% luainit.w
%
% Copyright 2006-2016 Taco Hoekwater <taco@@luatex.org>
%
% This file is part of LuaTeX.
%
% LuaTeX is free software; you can redistribute it and/or modify it under
% the terms of the GNU General Public License as published by the Free
% Software Foundation; either version 2 of the License, or (at your
% option) any later version.
%
% LuaTeX is distributed in the hope that it will be useful, but WITHOUT
% ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
% FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
% License for more details.
%
% You should have received a copy of the GNU General Public License along
% with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

@ @c

#include "ptexlib.h"

#include <kpathsea/c-stat.h>

#include "lua/luatex-api.h"

/* internalized strings: see luatex-api.h */
set_make_keys;

#if defined(MIKTEX)
#  include <miktex/Core/Paths.h> // MIKTEX_PREFIX
#endif

@
This file is getting a bit messy, but it is not simple to fix unilaterally.

Better to wait until Karl has some time (after texlive 2008) so we can
synchronize with kpathsea. One problem, for instance, is that I would
like to resolve the full executable path.  |kpse_set_program_name()| does
that, indirectly (by setting SELFAUTOLOC in the environment), but it
does much more, making it hard to use for our purpose.

In fact, it sets three C variables:

  |kpse_invocation_name|  |kpse_invocation_short_name|  |kpse->program_name|

and five environment variables:

  SELFAUTOLOC  SELFAUTODIR  SELFAUTOPARENT  SELFAUTOGRANDPARENT  progname

@c
const_string LUATEX_IHELP[] = {
    "Usage: " my_name " --lua=FILE [OPTION]... [TEXNAME[.tex]] [COMMANDS]",
    "   or: " my_name " --lua=FILE [OPTION]... \\FIRST-LINE",
    "   or: " my_name " --lua=FILE [OPTION]... &FMT ARGS",
    "  Run " MyName " on TEXNAME, usually creating TEXNAME.pdf.",
    "  Any remaining COMMANDS are processed as luatex input, after TEXNAME is read.",
    "",
    "  Alternatively, if the first non-option argument begins with a backslash,",
    "  " my_name " interprets all non-option arguments as an input line.",
    "",
    "  Alternatively, if the first non-option argument begins with a &, the",
    "  next word is taken as the FMT to read, overriding all else.  Any",
    "  remaining arguments are processed as above.",
    "",
    "  If no arguments or options are specified, prompt for input.",
    "",
    "  The following regular options are understood: ",
    "",
#if defined(MIKTEX)
    "   --aux-directory=DIR           use DIR as the directory to write auxiliary files to",
#endif
    "   --credits                     display credits and exit",
    "   --debug-format                enable format debugging",
#if defined(MIKTEX)
    "   --disable-installer           disable the package installer (do not automatically install missing files)",
#endif
    "   --draftmode                   switch on draft mode (generates no output PDF)",
#if defined(MIKTEX)
    "   --enable-installer            enable the package installer (automatically install missing files)",
#endif
    "   --[no-]file-line-error        disable/enable file:line:error style messages",
    "   --[no-]file-line-error-style  aliases of --[no-]file-line-error",
    "   --fmt=FORMAT                  load the format file FORMAT",
    "   --halt-on-error               stop processing at the first error",
    "   --help                        display help and exit",
#if defined(MIKTEX)
    "   --initialize                  be ini" my_name ", for dumping formats",
#else
    "   --ini                         be ini" my_name ", for dumping formats",
#endif
    "   --interaction=STRING          set interaction mode (STRING=batchmode/nonstopmode/scrollmode/errorstopmode)",
    "   --jobname=STRING              set the job name to STRING",
    "   --kpathsea-debug=NUMBER       set path searching debugging flags according to the bits of NUMBER",
    "   --lua=FILE                    load and execute a lua initialization script",
    "   --[no-]mktex=FMT              disable/enable mktexFMT generation (FMT=tex/tfm)",
    "   --nosocket                    disable the lua socket library",
    "   --output-comment=STRING       use STRING for DVI file comment instead of date (no effect for PDF)",
    "   --output-directory=DIR        use existing DIR as the directory to write files in",
    "   --output-format=FORMAT        use FORMAT for job output; FORMAT is 'dvi' or 'pdf'",
    "   --progname=STRING             set the program name to STRING",
    "   --recorder                    enable filename recorder",
    "   --safer                       disable easily exploitable lua commands",
    "   --[no-]shell-escape           disable/enable system commands",
    "   --shell-restricted            restrict system commands to a list of commands given in texmf.cnf",
    "   --synctex=NUMBER              enable synctex",
    "   --utc                         init time to UTC",
    "   --version                     display version and exit",
    "",
    "Alternate behaviour models can be obtained by special switches",
    "",
    "  --luaonly                      run a lua file, then exit",
    "  --luaconly                     byte-compile a lua file, then exit",
    "  --luahashchars                 the bits used by current Lua interpreter for strings hashing",
#ifdef LuajitTeX
    "  --jiton                        turns the JIT compiler on (default off)",
    "  --jithash=STRING               choose the hash function for the lua strings (lua51|luajit20: default lua51)",
#endif
    "",
    "See the reference manual for more information about the startup process.",
    NULL
};

/*
    "   --8bit                        ignored, input is assumed to be in UTF-8 encoding",
    "   --default-translate-file=FILE ignored, input is assumed to be in UTF-8 encoding",
    "   --etex                        ignored, the etex extensions are always active",
    "   --disable-write18             disable \\write18{SHELL COMMAND}",
    "   --enable-write18              enable \\write18{SHELL COMMAND}",
    "   --[no-]parse-first-line       ignored",
    "   --translate-file=FILE         ignored, input is assumed to be in UTF-8 encoding",
*/

@ The return value will be the directory of the executable, e.g.: \.{c:/TeX/bin}
@c
static char *ex_selfdir(char *argv0)
{
#if defined(WIN32) && ! defined(MIKTEX)
#if defined(__MINGW32__)
    char path[PATH_MAX], *fp;

    /* SearchPath() always gives back an absolute directory */
    if (SearchPath(NULL, argv0, ".exe", PATH_MAX, path, NULL) == 0)
        FATAL1("Can't determine where the executable %s is.\n", argv0);
    /* slashify the dirname */
    for (fp = path; fp && *fp; fp++)
        if (IS_DIR_SEP(*fp))
            *fp = DIR_SEP;
#else /* __MINGW32__ */
#define PATH_MAX 512
    char short_path[PATH_MAX], path[PATH_MAX], *fp;

    /* SearchPath() always gives back an absolute directory */
    if (SearchPath(NULL, argv0, ".exe", PATH_MAX, short_path, &fp) == 0)
        FATAL1("Can't determine where the executable %s is.\n", argv0);
    if (getlongpath(path, short_path, sizeof(path)) == 0) {
        FATAL1("This path points to an invalid file : %s\n", short_path);
    }
#endif /* __MINGW32__ */
   return xdirname(path);
#else /* WIN32 */
    return kpse_selfdir(argv0);
#endif
}

@ @c
static void prepare_cmdline(lua_State * L, char **av, int ac, int zero_offset)
{
    int i;
    char *s;
    luaL_checkstack(L, ac + 3, "too many arguments to script");
    lua_createtable(L, 0, 0);
    for (i = 0; i < ac; i++) {
        lua_pushstring(L, av[i]);
        lua_rawseti(L, -2, (i - zero_offset));
    }
    lua_setglobal(L, "arg");
    lua_getglobal(L, "os");
    s = ex_selfdir(argv[0]);
    lua_pushstring(L, s);
    xfree(s);
    lua_setfield(L, -2, "selfdir");
    return;
}


@ @c
int kpse_init = -1;

@ @c
string input_name = NULL;

static string user_progname = NULL;

char *startup_filename = NULL;
int lua_only = 0;
int lua_offset = 0;
unsigned char show_luahashchars = 0;

#ifdef LuajitTeX
int luajiton   = 0;
char *jithash_hashname = NULL;
#endif

int safer_option = 0;
int nosocket_option = 0;
int utc_option = 0;

@ Reading the options.

@ Test whether getopt found an option ``A''.
Assumes the option index is in the variable |option_index|, and the
option table in a variable |long_options|.

@c
#define ARGUMENT_IS(a) STREQ (long_options[option_index].name, a)

/*
    SunOS cc can't initialize automatic structs, so make this static.
*/

/*
    Nota Bene: we still intercept some options that other engines handle
    so that existing scripted usage will not fail.
*/

static struct option long_options[] = {
    {"fmt", 1, 0, 0},
    {"lua", 1, 0, 0},
    {"luaonly", 0, 0, 0},
    {"luahashchars", 0, 0, 0},
#ifdef LuajitTeX
    {"jiton", 0, 0, 0},
    {"jithash", 1, 0, 0},
#endif
    {"safer", 0, &safer_option, 1},
    {"utc", 0, &utc_option, 1},
    {"nosocket", 0, &nosocket_option, 1},
    {"help", 0, 0, 0},
#if defined(MIKTEX)
    {"initialize", 0, &ini_version, 1},
#else
    {"ini", 0, &ini_version, 1},
#endif
    {"interaction", 1, 0, 0},
    {"halt-on-error", 0, &haltonerrorp, 1},
    {"kpathsea-debug", 1, 0, 0},
#if defined(MIKTEX)
    {"alias", 1, 0, 0},
#endif
    {"progname", 1, 0, 0},
    {"version", 0, 0, 0},
    {"credits", 0, 0, 0},
    {"recorder", 0, 0, 0},
    {"etex", 0, 0, 0},
    {"output-comment", 1, 0, 0},
    {"output-directory", 1, 0, 0},
    {"draftmode", 0, 0, 0},
    {"output-format", 1, 0, 0},
    {"shell-escape", 0, &shellenabledp, 1},
    {"no-shell-escape", 0, &shellenabledp, -1},
    {"enable-write18", 0, &shellenabledp, 1},
    {"disable-write18", 0, &shellenabledp, -1},
    {"shell-restricted", 0, 0, 0},
    {"debug-format", 0, &debug_format_file, 1},
#if defined(MIKTEX)
    {"c-style-errors", 0, &filelineerrorstylep, 1},
    {"no-c-style-errors", 0, &filelineerrorstylep, -1},
#endif
    {"file-line-error-style", 0, &filelineerrorstylep, 1},
    {"no-file-line-error-style", 0, &filelineerrorstylep, -1},

    /* Shorter option names for the above. */

    {"file-line-error", 0, &filelineerrorstylep, 1},
    {"no-file-line-error", 0, &filelineerrorstylep, -1},
#if defined(MIKTEX)
    {"job-name", 1, 0, 0},
#endif
    {"jobname", 1, 0, 0},
    {"parse-first-line", 0, &parsefirstlinep, 1},
    {"no-parse-first-line", 0, &parsefirstlinep, -1},
    {"translate-file", 1, 0, 0},
    {"default-translate-file", 1, 0, 0},
    {"8bit", 0, 0, 0},
    {"mktex", 1, 0, 0},
    {"no-mktex", 1, 0, 0},
#if defined(MIKTEX)
    {"disable-installer", 0, 0, 0},
    {"enable-installer", 0, 0, 0},
    {"aux-directory", 1, 0, 0},
#endif

    /* Synchronization: just like "interaction" above */

    {"synctex", 1, 0, 0},
    {0, 0, 0, 0}
};

@ @c
int lua_numeric_field_by_index(lua_State * L, int name_index, int dflt)
{
    register int i = dflt;
    lua_rawgeti(L, LUA_REGISTRYINDEX, name_index);      /* fetch the stringptr */
    lua_rawget(L, -2);
    if (lua_type(L, -1) == LUA_TNUMBER) {
        i = lua_roundnumber(L, -1);
    }
    lua_pop(L, 1);
    return i;
}

@ @c
unsigned int lua_unsigned_numeric_field_by_index(lua_State * L, int name_index, int dflt)
{
    register unsigned int i = dflt;
    lua_rawgeti(L, LUA_REGISTRYINDEX, name_index);      /* fetch the stringptr */
    lua_rawget(L, -2);
    if (lua_type(L, -1) == LUA_TNUMBER) {
        i = lua_uroundnumber(L, -1);
    }
    lua_pop(L, 1);
    return i;
}

@ @c
static int recorderoption = 0;

static void parse_options(int ac, char **av)
{
#ifdef WIN32
/* save argc and argv */
    int sargc = argc;
    char **sargv = argv;
#endif
    int g;                      /* `getopt' return code.  */
    int option_index;
    char *firstfile = NULL;
    opterr = 0;                 /* dont whine */
#ifdef LuajitTeX
    if ((strstr(argv[0], "luajittexlua") != NULL) ||
        (strstr(argv[0], "texluajit") != NULL)) {
#else
    if ((strstr(argv[0], "luatexlua") != NULL) ||
        (strstr(argv[0], "texlua") != NULL)) {
#endif
        lua_only = 1;
        luainit = 1;
    }
    for (;;) {
        g = getopt_long_only(ac, av, "+", long_options, &option_index);

        if (g == -1)            /* End of arguments, exit the loop.  */
            break;
        if (g == '?')  {         /* Unknown option.  */
#if defined(MIKTEX)
          if (!luainit)
          {
            fprintf(stderr,"%s: unrecognized option '%s'\n", argv[0], argv[optind-1]);
            exit (1);
          }
#else
          if (!luainit)
            fprintf(stderr,"%s: unrecognized option '%s'\n", argv[0], argv[optind-1]);
#endif
          continue;
        }

#if !defined(MIKTEX)
        assert(g == 0);         /* We have no short option names.  */
#endif

        if (ARGUMENT_IS("luaonly")) {
            lua_only = 1;
            lua_offset = optind;
            luainit = 1;
        } else if (ARGUMENT_IS("lua")) {
            startup_filename = optarg;
            lua_offset = (optind - 1);
            luainit = 1;
#ifdef LuajitTeX
        } else if (ARGUMENT_IS("jiton")) {
            luajiton = 1;
        } else if (ARGUMENT_IS("jithash")) {
        size_t len = strlen(optarg);
        if (len<16) {
            jithash_hashname = optarg;
        } else {
            WARNING2("hash name truncated to 15 characters from %d. (%s)", (int) len, optarg);
            jithash_hashname = (string) xmalloc(16);
            strncpy(jithash_hashname, optarg, 15);
            jithash_hashname[15] = 0;
      }
#endif
        } else if (ARGUMENT_IS("luahashchars")) {
            show_luahashchars = 1;
        } else if (ARGUMENT_IS("kpathsea-debug")) {
            kpathsea_debug |= atoi(optarg);
#if defined(MIKTEX)
        } else if (ARGUMENT_IS("alias") || ARGUMENT_IS("progname")) {
#else
        } else if (ARGUMENT_IS("progname")) {
#endif
            user_progname = optarg;
#if defined(MIKTEX)
        } else if (ARGUMENT_IS("job-name") || ARGUMENT_IS("jobname")) {
#else
        } else if (ARGUMENT_IS("jobname")) {
#endif
            c_job_name = optarg;
        } else if (ARGUMENT_IS("fmt")) {
            dump_name = optarg;
        } else if (ARGUMENT_IS("output-directory")) {
#if defined(MIKTEX)
            miktex_web2c_set_output_directory(optarg);
#else
            output_directory = optarg;
#endif
        } else if (ARGUMENT_IS("output-comment")) {
            size_t len = strlen(optarg);
            if (len < 256) {
                output_comment = optarg;
            } else {
                WARNING2("Comment truncated to 255 characters from %d. (%s)", (int) len, optarg);
                output_comment = (string) xmalloc(256);
                strncpy(output_comment, optarg, 255);
                output_comment[255] = 0;
            }
        } else if (ARGUMENT_IS("shell-restricted")) {
            shellenabledp = 1;
            restrictedshell = 1;
        } else if (ARGUMENT_IS("output-format")) {
            output_mode_option = 1;
            if (strcmp(optarg, "dvi") == 0) {
                output_mode_value = 0;
            } else if (strcmp(optarg, "pdf") == 0) {
                output_mode_value = 1;
            } else {
                WARNING1("Ignoring unknown value `%s' for --output-format",optarg);
                output_mode_option = 0;
            }
        } else if (ARGUMENT_IS("draftmode")) {
            draft_mode_option = 1;
            draft_mode_value = 1;
        } else if (ARGUMENT_IS("mktex")) {
            kpse_maketex_option(optarg, true);
        } else if (ARGUMENT_IS("no-mktex")) {
            kpse_maketex_option(optarg, false);
        } else if (ARGUMENT_IS("interaction")) {
            /* These numbers match CPP defines */
            if (STREQ(optarg, "batchmode")) {
                interactionoption = 0;
            } else if (STREQ(optarg, "nonstopmode")) {
                interactionoption = 1;
            } else if (STREQ(optarg, "scrollmode")) {
                interactionoption = 2;
            } else if (STREQ(optarg, "errorstopmode")) {
                interactionoption = 3;
            } else {
                WARNING1("Ignoring unknown argument `%s' to --interaction", optarg);
            }
        } else if (ARGUMENT_IS("synctex")) {
            /* Synchronize TeXnology: catching the command line option as a long  */
            synctexoption = (int) strtol(optarg, NULL, 0);
        } else if (ARGUMENT_IS("recorder")) {
            recorderoption = 1 ;
#if defined(MIKTEX)
        }
        else if (ARGUMENT_IS("aux-directory"))
        {
          miktex_set_aux_directory(optarg);
        }
        else if (ARGUMENT_IS("disable-installer"))
        {
          miktex_enable_installer(0);
        }
        else if (ARGUMENT_IS("enable-installer"))
        {
          miktex_enable_installer(1);
#endif
        } else if (ARGUMENT_IS("help")) {
            usagehelp(LUATEX_IHELP, BUG_ADDRESS);
        } else if (ARGUMENT_IS("version")) {
            print_version_banner();
            /* *INDENT-OFF* */
            puts("\n\nExecute  '" my_name " --credits'  for credits and version details.\n\n"
                 "There is NO warranty. Redistribution of this software is covered by\n"
                 "the terms of the GNU General Public License, version 2 or (at your option)\n"
                 "any later version. For more information about these matters, see the file\n"
                 "named COPYING and the LuaTeX source.\n\n"
                 "LuaTeX is Copyright 2016 Taco Hoekwater and the LuaTeX Team.\n");
            /* *INDENT-ON* */
            uexit(0);
        } else if (ARGUMENT_IS("credits")) {
            char *versions;
            initversionstring(&versions);
            print_version_banner();
            /* *INDENT-OFF* */
            puts("\n\nThe LuaTeX team is Hans Hagen, Hartmut Henkel, Taco Hoekwater, Luigi Scarso.\n\n"
                 MyName " merges and builds upon (parts of) the code from these projects:\n\n"
                 "tex       : Donald Knuth\n"
                 "etex      : Peter Breitenlohner, Phil Taylor and friends\n"
                 "omega     : John Plaice and Yannis Haralambous\n"
                 "aleph     : Giuseppe Bilotta\n"
                 "pdftex    : Han The Thanh and friends\n"
                 "kpathsea  : Karl Berry, Olaf Weber and others\n"
                 "lua       : Roberto Ierusalimschy, Waldemar Celes and Luiz Henrique de Figueiredo\n"
                 "metapost  : John Hobby, Taco Hoekwater and friends\n"
                 "poppler   : Derek Noonburg, Kristian H\\ogsberg (partial)\n"
                 "fontforge : George Williams (partial)\n"
                 "luajit    : Mike Pall (used in LuajitTeX)\n");
            /* *INDENT-ON* */
            puts(versions);
#if defined(MIKTEX)
            miktex_show_library_versions();
#endif
            uexit(0);
        }
    }
    /* attempt to find |input_name| / |dump_name| */
    if (lua_only) {
        if (argv[optind]) {
            startup_filename = xstrdup(argv[optind]);
            lua_offset = optind;
        }
    } else if (argv[optind] && argv[optind][0] == '&') {
        dump_name = xstrdup(argv[optind] + 1);
    } else if (argv[optind] && argv[optind][0] != '\\') {
#if defined(MIKTEX_WINDOWS)
        {
          char * lpsz;
          for (lpsz = argv[optind]; *lpsz != 0; ++ lpsz)
          {
            if (*lpsz == '\\')
            {
              *lpsz = '/';
            }
          }
        }
#endif
        if (argv[optind][0] == '*') {
            input_name = xstrdup(argv[optind] + 1);
        } else {
            firstfile = xstrdup(argv[optind]);
            if ((strstr(firstfile, ".lua") ==
                 firstfile + strlen(firstfile) - 4)
                || (strstr(firstfile, ".luc") ==
                    firstfile + strlen(firstfile) - 4)
                || (strstr(firstfile, ".LUA") ==
                    firstfile + strlen(firstfile) - 4)
                || (strstr(firstfile, ".LUC") ==
                    firstfile + strlen(firstfile) - 4)) {
                if (startup_filename == NULL) {
                    startup_filename = firstfile;
                    lua_offset = optind;
                    lua_only = 1;
                    luainit = 1;
                }
            } else {
                input_name = firstfile;
            }
        }
#ifdef WIN32
    } else if (sargc > 1 && sargv[sargc-1] && sargv[sargc-1][0] != '-' &&
               sargv[sargc-1][0] != '\\') {
        if (sargv[sargc-1][0] == '&')
            dump_name = xstrdup(sargv[sargc-1] + 1);
        else  {
            if (sargv[sargc-1][0] == '*')
                input_name = xstrdup(sargv[sargc-1] + 1);
            else
                input_name = xstrdup(sargv[sargc-1]);
            sargv[sargc-1] = normalize_quotes(input_name, "argument");
        }
        if (safer_option)      /* --safer implies --nosocket */
            nosocket_option = 1;
        return;
#endif
    }
    if (safer_option)           /* --safer implies --nosocket */
        nosocket_option = 1;
    /* Finalize the input filename. */
    if (input_name != NULL) {
        argv[optind] = normalize_quotes(input_name, "argument");
    }
}

@ test for readability
@c
#define is_readable(a) (stat(a,&finfo)==0) && S_ISREG(finfo.st_mode) &&  \
  (f=fopen(a,"r")) != NULL && !fclose(f)

@ @c
static char *find_filename(char *name, const char *envkey)
{
    struct stat finfo;
    char *dirname = NULL;
    char *filename = NULL;
    FILE *f;
    if (is_readable(name)) {
        return name;
    } else {
        dirname = getenv(envkey);
        if ((dirname != NULL) && strlen(dirname)) {
            dirname = xstrdup(getenv(envkey));
            if (*(dirname + strlen(dirname) - 1) == '/') {
                *(dirname + strlen(dirname) - 1) = 0;
            }
            filename = xmalloc((unsigned) (strlen(dirname) + strlen(name) + 2));
            filename = concat3(dirname, "/", name);
            xfree(dirname);
            if (is_readable(filename)) {
                return filename;
            }
            xfree(filename);
        }
    }
    return NULL;
}

@ @c

static void init_kpse(void)
{
    if (!user_progname) {
        user_progname = dump_name;
    } else if (!dump_name) {
        dump_name = user_progname;
    }
    if (!user_progname) {
        if (ini_version) {
            if (input_name) {
                char *p = input_name + strlen(input_name) - 1;
                while (p >= input_name) {
                    if (IS_DIR_SEP (*p)) {
                        p++;
                        input_name = p;
                        break;
                    }
                    p--;
                }
                user_progname = remove_suffix (input_name);
            }
            if (!user_progname) {
#if defined(MIKTEX)
                user_progname = miktex_program_basename(argv[0]);
#else
                user_progname = kpse_program_basename(argv[0]);
#endif
            }
        } else {
            if (!dump_name) {
#if defined(MIKTEX)
                dump_name = miktex_program_basename(argv[0]);
#else
                dump_name = kpse_program_basename(argv[0]);
#endif
            }
            user_progname = dump_name;
        }
    }
    kpse_set_program_enabled(kpse_fmt_format, MAKE_TEX_FMT_BY_DEFAULT,
                             kpse_src_compile);

    kpse_set_program_name(argv[0], user_progname);
    init_shell_escape();        /* set up 'restrictedshell' */
    init_start_time();
    program_name_set = 1 ;
    if (recorderoption) {
        recorder_enabled = 1;
    }
}

@ @c
static void fix_dumpname(void)
{
    int dist;
    if (dump_name) {
        /* adjust array for Pascal and provide extension, if needed */
        dist = (int) (strlen(dump_name) - strlen(DUMP_EXT));
        if (strstr(dump_name, DUMP_EXT) == dump_name + dist)
            TEX_format_default = dump_name;
        else
            TEX_format_default = concat(dump_name, DUMP_EXT);
    } else {
        /* For |dump_name| to be NULL is a bug.  */
        if (!ini_version) {
          fprintf(stdout, "no format given, quitting\n");
          exit(1);
        }
    }
}

@ lua require patch

@ Auxiliary function for kpse search

@c
static const char *luatex_kpse_find_aux(lua_State *L, const char *name,
        kpse_file_format_type format, const char *errname)
{
    const char *filename;
    const char *altname;
    altname = luaL_gsub(L, name, ".", "/"); /* Lua convention */
    filename = kpse_find_file(altname, format, false);
    if (filename == NULL) {
        filename = kpse_find_file(name, format, false);
    }
    if (filename == NULL) {
        lua_pushfstring(L, "\n\t[kpse %s searcher] file not found: " LUA_QS, errname, name);
    }
    return filename;
}

@ The lua search function.

When kpathsea is not initialized, then it runs the
normal lua function that is saved in the registry, otherwise
it uses kpathsea.

two registry ref variables are needed: one for the actual lua
function, the other for its environment .

@c
static int lua_loader_function = 0;

static int luatex_kpse_lua_find(lua_State * L)
{
    const char *filename;
    const char *name;
    name = luaL_checkstring(L, 1);
    if (program_name_set == 0) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, lua_loader_function);
        lua_pushvalue(L, -2);
        lua_call(L, 1, 1);
        return 1;
    }
    filename = luatex_kpse_find_aux(L, name, kpse_lua_format, "lua");
    if (filename == NULL)
        return 1;               /* library not found in this path */
    if (luaL_loadfile(L, filename) != 0) {
        luaL_error(L, "error loading module %s from file %s:\n\t%s",
                   lua_tostring(L, 1), filename, lua_tostring(L, -1));
    }
    return 1;                   /* library loaded successfully */
}

@ @c
static int clua_loader_function = 0;
extern int searcher_C_luatex (lua_State *L, const char *name, const char *filename);

static int luatex_kpse_clua_find(lua_State * L)
{
    const char *filename;
    const char *name;
    if (safer_option) {
        lua_pushliteral(L, "\n\t[C searcher disabled in safer mode]");
        return 1;               /* library not found in this path */
    }
    name = luaL_checkstring(L, 1);
    if (program_name_set == 0) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, clua_loader_function);
        lua_pushvalue(L, -2);
        lua_call(L, 1, 1);
        return 1;
    } else {
        const char *path_saved;
        char *prefix, *postfix, *p, *total;
        char *extensionless;
        char *temp_name;
        int j;
        filename = luatex_kpse_find_aux(L, name, kpse_clua_format, "C");
        if (filename == NULL)
           return 1;               /* library not found in this path */
        extensionless = strdup(filename);
        if (!extensionless)
            return 1;  /* allocation failure */
        /* Fix Issue 850: replace '.' with LUA_DIRSEP */
        temp_name = strdup(name);
        for(j=0; ; j++){
          if ((unsigned char)temp_name[j]=='\0') {
            break;
          }
          if ((unsigned char)temp_name[j]=='.'){
            temp_name[j]=LUA_DIRSEP[0];
          }
        }
        p = strstr(extensionless, temp_name);
        if (!p) return 1;  /* this would be exceedingly weird */
        *p = '\0';
        prefix = strdup(extensionless);
        if (!prefix) return 1;  /* allocation failure */
        postfix = strdup(p+strlen(name));
        if (!postfix) return 1;  /* allocation failure */
        total = malloc(strlen(prefix)+strlen(postfix)+2);
        if (!total) return 1;  /* allocation failure */
        snprintf(total,strlen(prefix)+strlen(postfix)+2, "%s?%s", prefix, postfix);
        /* save package.path */
        lua_getglobal(L,"package");
        lua_getfield(L,-1,"cpath");
        path_saved = lua_tostring(L,-1);
        lua_pop(L,1);
        /* set package.path = "?" */
        lua_pushstring(L,total);
        lua_setfield(L,-2,"cpath");
        lua_pop(L,1); /* pop "package" */
        /* run function */
        lua_rawgeti(L, LUA_REGISTRYINDEX, clua_loader_function);
        lua_pushstring(L, name);
        lua_call(L, 1, 1);
        /* restore package.path */
        lua_getglobal(L,"package");
        lua_pushstring(L,path_saved);
        lua_setfield(L,-2,"cpath");
        lua_pop(L,1); /* pop "package" */
        free(extensionless);
        free(total);
        free(temp_name);
        return 1;
    }
}

@ Setting up the new search functions.

This replaces package.searchers[2] and package.searchers[3] with the
functions defined above.

@c
static void setup_lua_path(lua_State * L)
{
    lua_getglobal(L, "package");
#ifdef LuajitTeX
    lua_getfield(L, -1, "loaders");
#else
    lua_getfield(L, -1, "searchers");
#endif
    lua_rawgeti(L, -1, 2);      /* package.searchers[2] */
    lua_loader_function = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pushcfunction(L, luatex_kpse_lua_find);
    lua_rawseti(L, -2, 2);      /* replace the normal lua loader */

    lua_rawgeti(L, -1, 3);      /* package.searchers[3] */
    clua_loader_function = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pushcfunction(L, luatex_kpse_clua_find);
    lua_rawseti(L, -2, 3);      /* replace the normal lua lib loader */

    lua_pop(L, 2);              /* pop the array and table */
}

@ helper variables for the safe keeping of table ids

@c
int tex_table_id;
int pdf_table_id;
int token_table_id;
int node_table_id;

@ @c
int l_pack_type_index       [PACK_TYPE_SIZE] ;
int l_group_code_index      [GROUP_CODE_SIZE];
int l_local_par_index       [LOCAL_PAR_SIZE];
int l_math_style_name_index [MATH_STYLE_NAME_SIZE];
int l_dir_par_index         [DIR_PAR_SIZE];
int l_dir_text_index        [DIR_TEXT_SIZE];

int img_parms               [img_parms_max];
int img_pageboxes           [img_pageboxes_max];

int lua_show_valid_list(lua_State *L, const char **list, int max)
{
    int i;
    lua_newtable(L);
    for (i = 0; i < max; i++) {
        lua_pushinteger(L,i+1);
        lua_pushstring(L, list[i]);
        lua_settable(L, -3);
    }
    return 1;
}

int lua_show_valid_keys(lua_State *L, int *list, int max)
{
    int i;
    lua_newtable(L);
    for (i = 0; i < max; i++) {
        lua_pushinteger(L,i+1);
        lua_rawgeti(L, LUA_REGISTRYINDEX, list[i]);
        lua_settable(L, -3);
    }
    return 1;
}

#if defined(WIN32) || defined(__MINGW32__) || defined(__CYGWIN__)
char **suffixlist;

#  define EXE_SUFFIXES ".com;.exe;.bat;.cmd;.vbs;.vbe;.js;.jse;.wsf;.wsh;.ws;.tcl;.py;.pyw"

@ @c
static void mk_suffixlist(void)
{
    char **p;
    char *q, *r, *v;
    int n;

#  if defined(__CYGWIN__)
    v = xstrdup(EXE_SUFFIXES);
#  else
    v = (char *) getenv("PATHEXT");
    if (v)                      /* strlwr() exists also in MingW */
        v = (char *) strlwr(xstrdup(v));
    else
        v = xstrdup(EXE_SUFFIXES);
#  endif

    q = v;
    n = 0;

    while ((r = strchr(q, ';')) != NULL) {
        n++;
        r++;
        q = r;
    }
    if (*q)
        n++;
    suffixlist = (char **) xmalloc((n + 2) * sizeof(char *));
    p = suffixlist;
    *p = xstrdup(".dll");
    p++;
    q = v;
    while ((r = strchr(q, ';')) != NULL) {
        *r = '\0';
        *p = xstrdup(q);
        p++;
        r++;
        q = r;
    }
    if (*q) {
        *p = xstrdup(q);
        p++;
    }
    *p = NULL;
    free(v);
}
#endif

@ @c
void lua_initialize(int ac, char **av)
{
    char *given_file = NULL;
    char *banner;
    /*int kpse_init;*/
    size_t len;
    int starttime;
    int utc;
    static char LC_CTYPE_C[] = "LC_CTYPE=C";
    static char LC_COLLATE_C[] = "LC_COLLATE=C";
    static char LC_NUMERIC_C[] = "LC_NUMERIC=C";
    static char engine_luatex[] = "engine=" my_name;
    /* Save to pass along to topenin.  */
    const char *fmt = "This is " MyName ", Version %s" WEB2CVERSION;
    argc = ac;
    argv = av;
    len = strlen(fmt) + strlen(luatex_version_string) ;
    banner = xmalloc(len);
    sprintf(banner, fmt, luatex_version_string);
    luatex_banner = banner;
#if defined(MIKTEX)
    kpse_invocation_name = miktex_program_basename(argv[0]);
#else
    kpse_invocation_name = kpse_program_basename(argv[0]);
#endif

    /* be 'luac' */
    if (argc >1) {
#ifdef LuajitTeX
        if (FILESTRCASEEQ(kpse_invocation_name, "texluajitc"))
            exit(luac_main(ac, av));
#if defined(MIKTEX)
        if (FILESTRCASEEQ(kpse_invocation_name, MIKTEX_PREFIX "texluajitc"))
            exit(luac_main(ac, av));
#endif
        if (STREQ(argv[1], "--luaconly") || STREQ(argv[1], "--luac")) {
            char *argv1 = xmalloc (strlen ("luajittex") + 1);
            av[1] = argv1;
            strcpy (av[1], "luajittex");
            exit(luac_main(--ac, ++av));
        }
#else
        if (FILESTRCASEEQ(kpse_invocation_name, "texluac"))
            exit(luac_main(ac, av));
#if defined(MIKTEX)
        if (FILESTRCASEEQ(kpse_invocation_name, MIKTEX_PREFIX "texluac"))
            exit(luac_main(ac, av));
#endif
        if (STREQ(argv[1], "--luaconly") || STREQ(argv[1], "--luac")) {
            strcpy(av[1], "luatex");
            exit(luac_main(--ac, ++av));
        }
#endif
    }
#if defined(WIN32) || defined(__MINGW32__) || defined(__CYGWIN__)
    mk_suffixlist();
#endif

    /* Must be initialized before options are parsed.  */
    interactionoption = 4;
    dump_name = NULL;

    /* 0 means "disable Synchronize TeXnology".
     synctexoption is a *.web variable.
     We initialize it to a weird value to catch the -synctex command line flag
     At runtime, if synctexoption is not |INT_MAX|, then it contains the command line option provided,
     otherwise no such option was given by the user. */
#define SYNCTEX_NO_OPTION INT_MAX
    synctexoption = SYNCTEX_NO_OPTION;

    /* parse commandline */
    parse_options(ac, av);
    if (lua_only)
        shellenabledp = true;

    /* make sure that the locale is 'sane' (for lua) */

    putenv(LC_CTYPE_C);
    putenv(LC_COLLATE_C);
    putenv(LC_NUMERIC_C);

    /* this is sometimes needed */
    putenv(engine_luatex);

    luainterpreter();

    /* init internalized strings */
    set_init_keys;

    lua_pushstring(Luas,"lua.functions");
    lua_newtable(Luas);
    lua_settable(Luas,LUA_REGISTRYINDEX);

    /* here start the key definitions */
    set_l_pack_type_index;
    set_l_group_code_index;
    set_l_local_par_index;
    set_l_math_style_name_index;
    set_l_dir_par_index;
    set_l_dir_text_index;

    set_l_img_keys_index;
    set_l_img_pageboxes_index;

    prepare_cmdline(Luas, argv, argc, lua_offset);      /* collect arguments */
    setup_lua_path(Luas);

    if (startup_filename != NULL) {
        given_file = xstrdup(startup_filename);
        if (lua_only) {
          xfree(startup_filename);
        }
        startup_filename = find_filename(given_file, "LUATEXDIR");
    }
    /* now run the file */
    if (startup_filename != NULL) {
        char *v1;
        /* hide the 'tex' and 'pdf' table */
        tex_table_id = hide_lua_table(Luas, "tex");
        token_table_id = hide_lua_table(Luas, "token");
        node_table_id = hide_lua_table(Luas, "node");
        pdf_table_id = hide_lua_table(Luas, "pdf");

        if (luaL_loadfile(Luas, startup_filename)) {
            fprintf(stdout, "%s\n", lua_tostring(Luas, -1));
            exit(1);
        }
        /* */
        init_tex_table(Luas);
        if (lua_pcall(Luas, 0, 0, 0)) {
            fprintf(stdout, "%s\n", lua_tostring(Luas, -1));
        lua_traceback(Luas);
            exit(1);
        }
        /* no filename? quit now! */
        if (!input_name) {
            get_lua_string("texconfig", "jobname", &input_name);
        }
        if (!dump_name) {
            get_lua_string("texconfig", "formatname", &dump_name);
        }
        if (lua_only) {
            if (given_file)
                free(given_file);
            /* this is not strictly needed but it pleases valgrind */
            lua_close(Luas);
            exit(0);
        }
        /* unhide the 'tex' and 'pdf' table */
        unhide_lua_table(Luas, "tex", tex_table_id);
        unhide_lua_table(Luas, "pdf", pdf_table_id);
        unhide_lua_table(Luas, "token", token_table_id);
        unhide_lua_table(Luas, "node", node_table_id);

        /* |kpse_init| */
        kpse_init = -1;
        get_lua_boolean("texconfig", "kpse_init", &kpse_init);

        if (kpse_init != 0) {
            luainit = 0;        /* re-enable loading of texmf.cnf values, see luatex.ch */
            init_kpse();
        }
        /* |prohibit_file_trace| (boolean) */
        tracefilenames = 1;
        get_lua_boolean("texconfig", "trace_file_names", &tracefilenames);

        /* |file_line_error| */
        filelineerrorstylep = false;
        get_lua_boolean("texconfig", "file_line_error", &filelineerrorstylep);

        /* |halt_on_error| */
        haltonerrorp = false;
        get_lua_boolean("texconfig", "halt_on_error", &haltonerrorp);

        /* |restrictedshell| */
        v1 = NULL;
        get_lua_string("texconfig", "shell_escape", &v1);
        if (v1) {
            if (*v1 == 't' || *v1 == 'y' || *v1 == '1') {
                shellenabledp = 1;
            } else if (*v1 == 'p') {
                shellenabledp = 1;
                restrictedshell = 1;
            }
            free(v1);
        }
        /* If shell escapes are restricted, get allowed cmds from cnf.  */
        if (shellenabledp && restrictedshell == 1) {
            v1 = NULL;
            get_lua_string("texconfig", "shell_escape_commands", &v1);
            if (v1) {
                mk_shellcmdlist(v1);
            free(v1);
            }
        }

        starttime = -1 ;
        get_lua_number("texconfig", "start_time", &starttime);
        if (starttime < 0) {
            /*
                We provide this one for compatibility reasons and therefore also in
                uppercase.
            */
            get_lua_number("texconfig", "SOURCE_DATE_EPOCH", &starttime);
        }
        if (starttime >= 0) {
            set_start_time(starttime);
        }

        utc = -1 ;
        get_lua_boolean("texconfig", "use_utc_time", &utc);
        if (utc >= 0 && utc <= 1) {
            utc_option = utc;
        }

        fix_dumpname();
    } else {
        if (luainit) {
            if (given_file) {
                fprintf(stdout, "%s file %s not found\n", (lua_only ? "Script" : "Configuration"), given_file);
                free(given_file);
            } else {
                fprintf(stdout, "No %s file given\n", (lua_only ? "script" : "configuration"));
            }
            exit(1);
        } else {
            /* init */
            init_kpse();
            fix_dumpname();
        }
    }
}

@ @c
void check_texconfig_init(void)
{
    if (Luas != NULL) {
        lua_getglobal(Luas, "texconfig");
        if (lua_istable(Luas, -1)) {
            lua_getfield(Luas, -1, "init");
            if (lua_isfunction(Luas, -1)) {
                int i = lua_pcall(Luas, 0, 0, 0);
                if (i != 0) {
                    /* Can't be more precise here, called before TeX initialization  */
                    fprintf(stderr, "This went wrong: %s\n", lua_tostring(Luas, -1));
                    error();
                }
            }
        }
    }
}
