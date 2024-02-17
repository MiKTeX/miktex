/* kpsewhich -- standalone path lookup and variable expansion for Kpathsea.
   Ideas from Thomas Esser, Pierre MacKay, and many others.

   Copyright 1995-2018 Karl Berry & Olaf Weber.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; if not, see <http://www.gnu.org/licenses/>.  */

#include <kpathsea/config.h>
#include <kpathsea/c-ctype.h>
#include <kpathsea/c-pathch.h>
#include <kpathsea/expand.h>
#include <kpathsea/getopt.h>
#include <kpathsea/line.h>
#include <kpathsea/pathsearch.h>
#include <kpathsea/proginit.h>
#include <kpathsea/str-list.h>
#include <kpathsea/tex-file.h>
#include <kpathsea/tex-glyph.h>
#include <kpathsea/variable.h>
#include <kpathsea/version.h>
#if defined(MIKTEX)
#include <assert.h>
//#  define assert MIKTEX_ASSERT
#endif

/* For variable and path expansion.  (-expand-var, -expand-path,
   -show-path, etc.)  */
string var_to_expand = NULL;
string braces_to_expand = NULL;
string path_to_expand = NULL;
string path_to_show = NULL;
string var_to_value = NULL;
string var_to_brace_value = NULL;

/* Base resolution. (-D, -dpi) */
unsigned dpi = 600;

/* The engine name, for '$engine' construct in texmf.cnf.  (-engine) */
string engine = NULL;

/* Interactively ask for names to look up?  (-interactive) */
boolean interactive = false;

/* The device name, for $MAKETEX_MODE.  (-mode) */
string mode = NULL;

/* Search the disk as well as ls-R?  (-must-exist, -mktex) */
boolean must_exist = false;

/* The program name, for `.PROG' construct in texmf.cnf.  (-program) */
string progname = NULL;

/* Safe input and output names to check.  (-safe-in-name and -safe-out-name) */
string safe_in_name = NULL;
string safe_out_name = NULL;

/* Return all matches, not just the first one?  (-all) */
boolean show_all = false;

/* Only match files in given subdirs.  (-subdir) */
str_list_type subdir_paths;

/* The file type and path for lookups.  (-format, -path) */
kpse_file_format_type user_format = kpse_last_format;
string user_format_string;
string user_path;



/* Define one-word abbreviations for those format types which
   can otherwise only be specified by strings containing spaces.  */

typedef struct
{
  const_string abbr;
  kpse_file_format_type format;
} format_abbr_type;

static format_abbr_type format_abbrs[]
  = { { "bitmapfont", kpse_any_glyph_format },
      { "mpsupport", kpse_mpsupport_format },
      { "doc", kpse_texdoc_format },
      { "source", kpse_texsource_format },
      { "trofffont", kpse_troff_font_format },
      { "dvipsconfig", kpse_dvips_config_format },
      { "web2c", kpse_web2c_format },
      { "othertext", kpse_program_text_format },
      { "otherbin", kpse_program_binary_format },
      { "miscfont", kpse_miscfonts_format },
      { "cmap", kpse_cmap_format },
      { "pdftexconfig", kpse_pdftex_config_format },
      { NULL, kpse_last_format } };

/* The function to look up STR in the abbr table above.
   This is called only on a user-specified format string.
   Return `kpse_last_format' if no match.  */

static kpse_file_format_type
format_abbr (const_string str)
{
  kpse_file_format_type ret = kpse_last_format;
  unsigned a = 0;

  while (format_abbrs[a].abbr != NULL) {
    if (STREQ (str, format_abbrs[a].abbr)) {
      ret = format_abbrs[a].format;
      break;
    }
    a++;
  }

  return ret;
}



/* Return the <number> substring in `<name>.<number><stuff>', if S has
   that form.  If it doesn't, return 0.  */

static unsigned
find_dpi (string s)
{
  unsigned dpi_number = 0;
  const_string extension = find_suffix (s);

  if (extension != NULL)
    sscanf (extension, "%u", &dpi_number);

  return dpi_number;
}



/* Return true if FTRY (the candidate suffix) matches NAME.  If
   IS_FILENAME is true, the check is simply that FTRY is a suffix of
   NAME.  If false (that is, NAME is a format), then FTRY and NAME must
   be entirely equal.  */

static boolean
try_suffix (boolean is_filename, string name, unsigned name_len,
            const_string ftry)
{
  unsigned try_len;
  
  if (!ftry || ! *ftry) {
    return false;
  }
  
  try_len = strlen (ftry);
  if (try_len > name_len) {
    /* Candidate is longer than what we're looking for.  */
    return false;
  }
  if (!is_filename && try_len < name_len) {
    /* We're doing format names, not file names, and candidate is
       shorter than what we're looking for.  E.g., do not find `lua'
       when looking for `clua'.  */
    return false;
  }
  
  if (FILESTRCASEEQ (name + name_len - try_len, ftry)) {
    return true;
  }
  
  return false;
}



/* Use the file type from -format if that was previously determined
   (i.e., the user_format global variable), else guess dynamically from
   NAME.  Return kpse_last_format if undeterminable.  This function is
   also used to parse the -format string, a case we distinguish via
   is_filename being false.

   A few filenames have been hard-coded for format types that
   differ from what would be inferred from their extensions. */

static kpse_file_format_type
find_format (kpathsea kpse, string name, boolean is_filename)
{
  kpse_file_format_type ret = kpse_last_format;

  if (is_filename && user_format != kpse_last_format) {
    ret = user_format; /* just return what we already computed */

  } else if (FILESTRCASEEQ (name, "config.ps")) {
    ret = kpse_dvips_config_format;
  } else if (FILESTRCASEEQ (name, "fmtutil.cnf")) {
    ret = kpse_web2c_format;
  } else if (FILESTRCASEEQ (name, "glyphlist.txt")) {
    ret = kpse_fontmap_format;
  } else if (FILESTRCASEEQ (name, "mktex.cnf")) {
    ret = kpse_web2c_format;
  } else if (FILESTRCASEEQ (name, "pdfglyphlist.txt")) {
    ret = kpse_fontmap_format;
  } else if (FILESTRCASEEQ (name, "pdftex.cfg")) {
    ret = kpse_pdftex_config_format;
  } else if (FILESTRCASEEQ (name, "texglyphlist.txt")) {
    ret = kpse_fontmap_format;
  } else if (FILESTRCASEEQ (name, "texmf.cnf")) {
    ret = kpse_cnf_format;
  } else if (FILESTRCASEEQ (name, "updmap.cfg")) {
    ret = kpse_web2c_format;
  } else if (FILESTRCASEEQ (name, "XDvi")) {
    ret = kpse_program_text_format;
  } else {
    if (!is_filename) {
      /* Look for kpsewhich-specific format abbreviations.  */
      ret = format_abbr (name);
    }

    if (ret == kpse_last_format) {
      int f = 0;  /* kpse_file_format_type */
      unsigned name_len = strlen (name);

      while (f != kpse_last_format) {
        const_string *ext;
        const_string ftry;
        boolean found = false;

        if (!kpse->format_info[f].type)
          kpathsea_init_format (kpse, (kpse_file_format_type) f);

/* Just to abbreviate this lengthy call.  */
#define TRY_SUFFIX(ftry) try_suffix (is_filename, name, name_len, (ftry))

        if (!is_filename) {
          /* Allow the long name, but only in the format options.  We don't
             want a filename confused with a format name.  */
          ftry = kpse->format_info[f].type;
          found = TRY_SUFFIX (ftry);
        }
        for (ext = kpse->format_info[f].suffix; !found && ext && *ext; ext++) {
          found = TRY_SUFFIX (*ext);
        }
        for (ext=kpse->format_info[f].alt_suffix; !found && ext && *ext;ext++){
          found = TRY_SUFFIX (*ext);
        }

        if (found)
          break;
        f++;
      }
      ret = f;
    }
  }
  return ret;
}



/* Return newly-allocated NULL-terminated list of strings from MATCHES
   that are prefixed with any of the subdirectories in SUBDIRS.  That
   is, for a string S in MATCHES, its dirname must end with one of the
   elements in SUBDIRS.  For instance, if subdir=foo/bar, that will
   match a string foo/bar/baz or /some/texmf/foo/bar/baz.

   We don't reallocate the actual strings, just the list elements.
   Perhaps later we will implement wildcards or // or something.  */

#if !defined(MIKTEX) && defined(WIN32)
static string *
kpathsea_subdir_match (kpathsea kpse, str_list_type subdirs,  string *matches)
{
  string *ret = XTALLOC1 (string);
  unsigned len = 1;
  unsigned e;
  unsigned m;
  string p;

  for (e = 0; e < STR_LIST_LENGTH (subdirs); e++) {
    for (p = STR_LIST_ELT (subdirs, e); *p; p++) {
      if (*p == '\\')
        *p = '/';
      else if (kpathsea_IS_KANJI(kpse, p))
        p++;
    }
  }

  for (m = 0; matches[m]; m++) {
    unsigned loc;
    string s = xstrdup (matches[m]);
    for (loc = strlen (s); loc > 0 && !IS_DIR_SEP_CH (s[loc-1]); loc--)
      ;
    while (loc > 0 && IS_DIR_SEP_CH (s[loc-1])) {
      loc--;
    }
    s[loc] = 0;  /* wipe out basename */

    for (e = 0; e < STR_LIST_LENGTH (subdirs); e++) {
      string subdir = STR_LIST_ELT (subdirs, e);
      unsigned subdir_len = strlen (subdir);
      while (subdir_len > 0 && IS_DIR_SEP_CH (subdir[subdir_len-1])) {
        subdir_len--;
        subdir[subdir_len] = 0; /* remove trailing slashes from subdir spec */
      }
      if (FILESTRCASEEQ (subdir, s + loc - subdir_len)) {
        /* matched, save this one.  */
        XRETALLOC (ret, len + 1, string);
        ret[len-1] = matches[m];
        len++;
      }
    }
    free (s);
  }
  ret[len-1] = NULL;
  return ret;
}

#else /* WIN32 */
static string *
subdir_match (str_list_type subdirs,  string *matches)
{
  string *ret = XTALLOC1 (string);
  unsigned len = 1;
  unsigned e;
  unsigned m;

  for (m = 0; matches[m]; m++) {
    unsigned loc;
    string s = xstrdup (matches[m]);
    for (loc = strlen (s); loc > 0 && !IS_DIR_SEP_CH (s[loc-1]); loc--)
      ;
    while (loc > 0 && IS_DIR_SEP_CH (s[loc-1])) {
      loc--;
    }
    s[loc] = 0;  /* wipe out basename */

    for (e = 0; e < STR_LIST_LENGTH (subdirs); e++) {
      string subdir = STR_LIST_ELT (subdirs, e);
      unsigned subdir_len = strlen (subdir);
      while (subdir_len > 0 && IS_DIR_SEP_CH (subdir[subdir_len-1])) {
        subdir_len--;
        subdir[subdir_len] = 0; /* remove trailing slashes from subdir spec */
      }
      if (FILESTRCASEEQ (subdir, s + loc - subdir_len)) {
        /* matched, save this one.  */
        XRETALLOC (ret, len + 1, string);
        ret[len-1] = matches[m];
        len++;
      }
    }
    free (s);
  }
  ret[len-1] = NULL;
  return ret;
}
#endif /* WIN32 */


/* Look up a single filename NAME, filtering by given subdirectories if
   -subdir was specified.  Print all matches if global `show_all' is
   true, else just the first match.  Return 0 if success, 1 if failure.  */

static unsigned
lookup (kpathsea kpse, string name)
{
  int i;
  string ret = NULL;
  string *ret_list = NULL;

  if (user_path) {
    /* Translate ; to : if that's our ENV_SEP.  See cnf.c.  */
    if (IS_ENV_SEP (':')) {
      string loc;
      for (loc = user_path; *loc; loc++) {
        if (*loc == ';')
          *loc = ':';
      }
    }
    user_path = kpathsea_path_expand (kpse, user_path);
    if (show_all) {
      ret_list = kpathsea_all_path_search (kpse, user_path, name);
    } else {
      ret = kpathsea_path_search (kpse, user_path, name, must_exist);
    }

  } else {
    /* No user-specified search path, check user format or guess from NAME.  */
    kpse_file_format_type fmt = find_format (kpse, name, true);

    switch (fmt) {
      case kpse_pk_format:
      case kpse_gf_format:
      case kpse_any_glyph_format:
        {
          kpse_glyph_file_type glyph_ret;
          string temp = remove_suffix (name);
          /* Try to extract the resolution from the name.  */
          unsigned local_dpi = find_dpi (name);
          if (!local_dpi)
            local_dpi = dpi;
          if (show_all) {
            /* Because the whole glyph lookup thing is too complicated
               to try all the alternatives (cmr10.600pk vs.
               dpi600/cmr10.pk, just for starters).  Patches welcome :).  */
            WARNING1 ("kpsewhich: Ignoring --all for bitmap font, sorry: %s",
                      name);
          }
          ret = kpathsea_find_glyph (kpse, temp,
                                     local_dpi, fmt, &glyph_ret);
          if (temp != name)
            free (temp);
        }
        break;

      case kpse_last_format:
        /* If the suffix isn't recognized, assume it's a tex file. */
        fmt = kpse_tex_format;
        /* fall through */

      default:
        if (show_all) {
          ret_list = kpathsea_find_file_generic (kpse, name, fmt,
                                                 must_exist, true);
        } else {
          ret = kpathsea_find_file (kpse, name, fmt, must_exist);
        }
    }
  }

  /* Turn single return into a null-terminated list for uniform treatment.  */
  if (ret) {
    ret_list = XTALLOC (2, string);
    ret_list[0] = ret;
    ret_list[1] = NULL;
  }

  /* Filter by subdirectories, if specified.  */
  if (!STR_LIST_EMPTY (subdir_paths)) {
#if !defined(MIKTEX) && defined(WIN32)
    string *new_list = kpathsea_subdir_match (kpse, subdir_paths, ret_list);
#else
    string *new_list = subdir_match (subdir_paths, ret_list);
#endif /* WIN32 */
    free (ret_list);
    ret_list = new_list;
  }

  /* Print output.  */
  if (ret_list) {
    for (i = 0; ret_list[i]; i++)
#if defined(MIKTEX)
    {
      puts(ret_list[i]);
    }
#else
#ifdef WIN32
      kpathsea_win32_puts (kpse, ret_list[i]);
#else
      puts (ret_list[i]);
#endif
#endif
    /* Save whether we found anything */
    ret = ret_list[0];
    free (ret_list);
  }

  return ret == NULL;
}

/* Help message.  */

#define USAGE "\n\
Standalone path lookup and expansion for the Kpathsea library.\n\
The default is to look up each FILENAME in turn and report its\n\
first match (if any) to standard output.\n\
\n\
When looking up format (.fmt/.base/.mem) files, it is usually necessary\n\
to also use -engine, or nothing will be returned; in particular,\n\
-engine=/ will return matching format files for any engine.\n\
\n\
-all                   output all matches, one per line (no effect with pk/gf).\n\
[-no]-casefold-search  fall back to case-insensitive search if no exact match.\n\
-debug=NUM             set debugging flags.\n\
-D, -dpi=NUM           use a base resolution of NUM; default 600.\n\
-engine=STRING         set engine name to STRING.\n\
-expand-braces=STRING  output variable and brace expansion of STRING.\n\
-expand-path=STRING    output complete path expansion of STRING.\n\
-expand-var=STRING     output variable expansion of STRING.\n\
-format=NAME           use file type NAME (list shown by -help-formats).\n\
-help                  display this message and exit.\n\
-help-formats          display information about all supported file formats.\n\
-interactive           ask for additional filenames to look up.\n\
[-no]-mktex=FMT        disable/enable mktexFMT generation (FMT=pk/mf/tex/tfm).\n\
-mode=STRING           set device name for $MAKETEX_MODE to STRING; no default.\n\
-must-exist            search the disk as well as ls-R if necessary.\n\
-path=STRING           search in the path STRING.\n\
-progname=STRING       set program name to STRING.\n\
-safe-in-name=STRING   check if STRING is ok to open for input.\n\
-safe-out-name=STRING  check if STRING is ok to open for output.\n\
-show-path=TYPE        output search path for file type TYPE\n\
                         (list shown by -help-formats).\n\
-subdir=STRING         only output matches whose directory ends with STRING.\n\
-var-brace-value=STRING output brace-expanded value of variable $STRING.\n\
-var-value=STRING       output variable-expanded value of variable $STRING.\n\
-version               display version information number and exit.\n \
"

static void
help_message (kpathsea kpse, string *argv)
{
  printf ("Usage: %s [OPTION]... [FILENAME]...\n", argv[0]);
  fputs (USAGE, stdout);
  putchar ('\n');
  fputs (kpathsea_bug_address, stdout);
  fputs ("Kpathsea home page: https://tug.org/kpathsea/\n", stdout);
  exit (0);
}

static void
help_formats (kpathsea kpse, string *argv)
{
  int f; /* kpse_file_format_type */

  /* Have to set this for init_format to work.  */
  kpathsea_set_program_name (kpse, argv[0], progname);

#if defined(MIKTEX)
  {
    char miktexBanner[200];
    miktex_get_miktex_banner(miktexBanner, sizeof(miktexBanner) / sizeof(miktexBanner[0]));
    puts(miktexBanner);
  }
#else
  puts (kpathsea_version_string); 
#endif
  puts ("\nRecognized Kpathsea format names and their (abbreviations) and suffixes:");
  for (f = 0; f < kpse_last_format; f++) {
    const_string *ext;

#if !defined(MIKTEX)
    const_string envvar_list = 
      kpathsea_init_format_return_varlist (kpse, (kpse_file_format_type) f);
#endif
    printf ("%s", kpse->format_info[f].type);

    /* Show abbreviation if we accept one.  We repeatedly go through the
       abbr list here, but it's so short, it doesn't matter.  */
    {
       unsigned a = 0;
       while (format_abbrs[a].abbr != NULL) {
         if (f == format_abbrs[a].format) {
           printf (" (%s)", format_abbrs[a].abbr);
           break;
         }
         a++;
       }
    }

    /* Regular suffixes.  */
    putchar (':');
    for (ext = kpse->format_info[f].suffix; ext && *ext; ext++) {
      putchar (' ');
#if defined(MIKTEX)
      fputs(*ext, stdout);
#else
#ifdef WIN32
      kpathsea_win32_fputs (kpse, *ext, stdout);
#else
      fputs (*ext, stdout);
#endif
#endif
    }

    if (kpse->format_info[f].alt_suffix) {
      /* leave extra space between default and alt suffixes */
      putchar (' ');
    }
    for (ext = kpse->format_info[f].alt_suffix; ext && *ext; ext++) {
      putchar (' ');
#if defined(MIKTEX)
      fputs(*ext, stdout);
#else
#ifdef WIN32
      kpathsea_win32_fputs (kpse, *ext, stdout);
#else
      fputs (*ext, stdout);
#endif
#endif
    }

#if !defined(MIKTEX)
    printf ("  [variables: %s]\n", envvar_list);
#endif

#if !defined(MIKTEX)
    printf ("  [original path (from %s) = %s]\n",
            kpse->format_info[f].path_source, kpse->format_info[f].raw_path);
#endif
  }

  fputs ("\nTo see paths after expansion, use --show-path=FMT.\n\n", stdout);
  fputs (kpathsea_bug_address, stdout);
  exit (0);
}


/* Reading the options.  */

/* This macro tests whether getopt found an option ``A''.
   Assumes the option index is in the variable `option_index', and the
   option table in a variable `long_options'.  */
#define ARGUMENT_IS(a) STREQ (long_options[option_index].name, a)

/* SunOS cc can't initialize automatic structs.  */
static struct option long_options[]
  = { { "D",                    1, 0, 0 },
      { "all",                  0, (int *) &show_all, 1 },
      { "casefold-search",      0, 0, 0 },
      { "debug",                1, 0, 0 },
      { "dpi",                  1, 0, 0 },
      { "engine",               1, 0, 0 },
      { "expand-braces",        1, 0, 0 },
      { "expand-path",          1, 0, 0 },
      { "expand-var",           1, 0, 0 },
      { "format",               1, 0, 0 },
      { "help",                 0, 0, 0 },
      { "help-formats",         0, 0, 0 },
      { "interactive",          0, (int *) &interactive, 1 },
      { "mktex",                1, 0, 0 },
      { "mode",                 1, 0, 0 },
      { "must-exist",           0, (int *) &must_exist, 1 },
      { "path",                 1, 0, 0 },
      { "no-casefold-search",   0, 0, 0 },
      { "no-mktex",             1, 0, 0 },
      { "progname",             1, 0, 0 },
      { "safe-in-name",         1, 0, 0 },
      { "safe-out-name",        1, 0, 0 },
      { "subdir",               1, 0, 0 },
      { "show-path",            1, 0, 0 },
      { "var-brace-value",      1, 0, 0 },
      { "var-value",            1, 0, 0 },
      { "version",              0, 0, 0 },
      { 0, 0, 0, 0 } };

static void
read_command_line (kpathsea kpse, int argc, string *argv)
{
  int g;   /* `getopt' return code.  */
  int option_index;

  for (;;) {
    g = getopt_long_only (argc, argv, "", long_options, &option_index);

    if (g == -1)
      break;

    if (g == '?')
      exit (1);  /* Unknown option.  */

    assert (g == 0); /* We have no short option names.  */

    if (ARGUMENT_IS ("casefold-search")) {
      /* We can't just a boolean for casefold-search because we want to
         distinguish it being set with an option vs. leaving the default
         (by default).  */
      xputenv ("texmf_casefold_search", "1");      

    } else if (ARGUMENT_IS ("debug")) {
      kpse->debug |= atoi (optarg);

    } else if (ARGUMENT_IS ("dpi") || ARGUMENT_IS ("D")) {
      dpi = atoi (optarg);

    } else if (ARGUMENT_IS ("engine")) {
      engine = optarg;

    } else if (ARGUMENT_IS ("expand-braces")) {
      braces_to_expand = optarg;

    } else if (ARGUMENT_IS ("expand-path")) {
      path_to_expand = optarg;

    } else if (ARGUMENT_IS ("expand-var")) {
      var_to_expand = optarg;

    } else if (ARGUMENT_IS ("format")) {
      user_format_string = optarg;

    } else if (ARGUMENT_IS ("help")) {
      help_message (kpse, argv);

    } else if (ARGUMENT_IS ("help-formats")) {
      help_formats (kpse, argv);

    } else if (ARGUMENT_IS ("mktex")) {
      kpathsea_maketex_option (kpse, optarg, true);
      must_exist = 1;  /* otherwise it never gets called */

    } else if (ARGUMENT_IS ("mode")) {
      mode = optarg;

    } else if (ARGUMENT_IS ("no-casefold-search")) {
      xputenv ("texmf_casefold_search", "0");      

    } else if (ARGUMENT_IS ("no-mktex")) {
      kpathsea_maketex_option (kpse, optarg, false);
      must_exist = 0;

    } else if (ARGUMENT_IS ("path")) {
      user_path = optarg;

    } else if (ARGUMENT_IS ("progname")) {
      progname = optarg;

    } else if (ARGUMENT_IS ("safe-in-name")) {
      safe_in_name = optarg;

    } else if (ARGUMENT_IS ("safe-out-name")) {
      safe_out_name = optarg;

    } else if (ARGUMENT_IS ("show-path")) {
      path_to_show = optarg;
      user_format_string = optarg;

    } else if (ARGUMENT_IS ("subdir")) {
      str_list_add (&subdir_paths, optarg);

    } else if (ARGUMENT_IS ("var-brace-value")) {
      var_to_brace_value = optarg;

    } else if (ARGUMENT_IS ("var-value")) {
      var_to_value = optarg;

    } else if (ARGUMENT_IS ("version")) {
#if defined(MIKTEX)
  {
    char miktexBanner[200];
    miktex_get_miktex_banner(miktexBanner, sizeof(miktexBanner) / sizeof(miktexBanner[0]));
    puts(miktexBanner);
  }
#else
      puts (kpathsea_version_string);
#endif
      puts ("Copyright 2018 Karl Berry & Olaf Weber.\n\
License LGPLv2.1+: GNU Lesser GPL version 2.1 or later <https://gnu.org/licenses/lgpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n");
      exit (0);
    }

    /* Else it was just a flag; getopt has already done the assignment.  */
  }

  if (user_path && user_format_string) {
    fprintf (stderr, "-path (%s) and -format (%s) are mutually exclusive.\n",
             user_path, user_format_string);
    fputs ("Try `kpsewhich --help' for more information.\n", stderr);
    exit (1);
  }

  if (optind == argc
      && !var_to_expand && !braces_to_expand && !path_to_expand
      && !path_to_show && !var_to_value && !var_to_brace_value
      && !safe_in_name && !safe_out_name) {
    fputs ("Missing argument. Try `kpsewhich --help' for more information.\n",
           stderr);
    exit (1);
  }
}



/* Initializations that may depend on the options.  */

static void
init_more (kpathsea kpse)
{
  if (engine)
    kpathsea_xputenv (kpse, "engine", engine);

  /* Disable all mktex programs unless they were explicitly enabled on our
     command line.  */
#define DISABLE_MKTEX(fmt) \
kpathsea_set_program_enabled (kpse, fmt, false, kpse_src_cmdline - 1)
  DISABLE_MKTEX (kpse_pk_format);
  DISABLE_MKTEX (kpse_mf_format);
  DISABLE_MKTEX (kpse_tex_format);
  DISABLE_MKTEX (kpse_tfm_format);
  DISABLE_MKTEX (kpse_fmt_format);
  DISABLE_MKTEX (kpse_ofm_format);
  DISABLE_MKTEX (kpse_ocp_format);

  /* NULL for no fallback font.  */
  kpathsea_init_prog (kpse, uppercasify (kpse->program_name), dpi, mode, NULL);

  /* Have to do this after setting the program name.  */
  if (user_format_string) {
    user_format = find_format (kpse, user_format_string, false);
    if (user_format == kpse_last_format) {
      WARNING1 ("kpsewhich: Ignoring unknown file type `%s'",
                user_format_string);
    }
  }
}



#if defined(MIKTEX)
#  define main MIKTEXCEECALL Main
int
main(int argc, char **argv)
#else
int
main (int argc,  string *argv)
#endif
{
#if !defined(MIKTEX)
#ifdef WIN32
#define puts(s) kpathsea_win32_puts(kpse, (s))
  string *av, enc;
  int ac;
#endif /* WIN32 */
#endif
  unsigned unfound = 0;
  kpathsea kpse = kpathsea_new ();

  /* Read options, then dependent initializations.  */
  read_command_line (kpse, argc, argv);

  kpathsea_set_program_name (kpse, argv[0], progname);
#if !defined(MIKTEX)
#ifdef WIN32
  if(strstr(kpse->program_name,"xetex") || strstr(kpse->program_name,"xelatex")
     || strstr(kpse->program_name,"uptex") || strstr(kpse->program_name,"uplatex")
     || strstr(kpse->program_name,"pdftex") || strstr(kpse->program_name,"pdflatex")
     || strstr(kpse->program_name,"dvipdfm") || strstr(kpse->program_name,"extractbb")
     || strstr(kpse->program_name,"xbb") || strstr(kpse->program_name,"ebb")
     || strstr(kpse->program_name,"dvips") || strstr(kpse->program_name,"upmendex"))
  {
    if (strstr(kpse->program_name,"upmendex"))
      enc = "utf-8";
    else
      enc = kpathsea_var_value (kpse, "command_line_encoding");
    if (kpathsea_get_command_line_args_utf8(kpse, enc, &ac, &av)) {
      optind = 0;
      read_command_line (kpse, ac, av);
      argv = av;
      argc = ac;
    }
  }
#endif /* WIN32 */
#endif
  init_more (kpse);


  /* Perform actions.  */

  /* Variable expansion.  */
  if (var_to_expand)
    puts (kpathsea_var_expand (kpse, var_to_expand));

  /* Brace expansion. */
  if (braces_to_expand)
    puts (kpathsea_brace_expand (kpse, braces_to_expand));

  /* Path expansion. */
  if (path_to_expand)
    puts (kpathsea_path_expand (kpse, path_to_expand));

  /* Show a search path. */
  if (path_to_show) {
    if (user_format != kpse_last_format) {
      if (!kpse->format_info[user_format].type) {
        /* needed if arg was numeric */
        kpathsea_init_format (kpse, user_format);
      }
      puts (kpse->format_info[user_format].path);
    } else {
      WARNING1 ("kpsewhich: Unknown file type, cannot show path: %s",
                path_to_show);
    }
  }

  /* Var to value.  */
  if (var_to_value) {
    const_string value = kpathsea_var_value (kpse, var_to_value);
    if (!value) {
      unfound++;
      value = "";
    }
    puts (value);
  }

  /* Var to brace-expanded value. This is separate from --var-value for
     compatibility; people use --var-value for non-path values, where
     changing commas to colons, which brace expansion does, is not right.  */
  if (var_to_brace_value) {
    const_string value = kpathsea_var_value (kpse, var_to_brace_value);
    if (!value) {
      unfound++;
      value = "";
    } else {
      /* Sometimes users want the fully-expanded (as a string, no
         filesystem checks) value.  We can't call brace_expand as part of
         kpathsea_var_value, though, because unfortunately it is not
         reentrant.  We use var_value in lots of places in the source,
         and it clobbers the static buffer in the kpse structure.  */
      value = kpathsea_brace_expand (kpse, value);
    }
    puts (value);
  }

  if (safe_in_name) {
    if (!kpathsea_in_name_ok_silent (kpse, safe_in_name))
      unfound++;
  }

  if (safe_out_name) {
    if (!kpathsea_out_name_ok_silent (kpse, safe_out_name))
      unfound++;
  }

  /* --subdir must imply --all, since we filter here after doing the
     search, rather than inside the search itself.  */
  if (!STR_LIST_EMPTY (subdir_paths)) {
    show_all = 1;
  }

  /* Usual case: look up each given filename.  */
  for (; optind < argc; optind++) {
    unfound += lookup (kpse, argv[optind]);
  }

  if (interactive) {
    for (;;) {
      string name = read_line (stdin);
      if (!name || STREQ (name, "q") || STREQ (name, "quit"))
        break;
      unfound += lookup (kpse, name);
      free (name);
    }
  }

  kpathsea_finish (kpse);
  return unfound > 255 ? 1 : unfound;
}
