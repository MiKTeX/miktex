/*
 *   ttf2pk.c
 *
 *   This file is part of the ttf2pk package, released under the
 *   GNU GPL version 2 or any later version.
 *
 *   Copyright 1997-1999, 2000, 2002 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 *   Copyright 2009-2013 by
 *     Peter Breitenlohner <tex-live@tug.org>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>         /* for size_t */
#include <string.h>
#include <math.h>
#include <ctype.h>

#if !defined(MIKTEX)
#ifdef MIKTEX
#include <miktex.h>
#endif
#endif

#include "ttf2tfm.h"
#include "newobj.h"
#include "pklib.h"
#include "ttfenc.h"
#include "ttflib.h"
#include "errormsg.h"
#include "filesrch.h"
#include "parse.h"
#include "subfont.h"


char ident[] = "ttf2pk version 2.0";
const char *progname = "ttf2pk";             /* for error/warning messages */

static const char *cfg_filename = "ttf2pk.cfg";
char *mapfiles;

Boolean have_sfd = False;
Boolean have_pid = False;
Boolean have_eid = False;


/*
 *   Checks for an equal sign surrounded by whitespace.
 */

static char *
strip_equal(char *s, char *os, char *p)
{
  while (isspace((unsigned char)*p))
    p++;
  if (*p != '=')
    boops(os, p - s, "Missing `='.");
  p++;
  while (isspace((unsigned char)*p))
    p++;
  return p;
}


#define USAGE "\
  Convert a TrueType font to TeX's PK format.\n\
\n\
-q                  suppress informational output\n\
-n                  only use `.pk' as extension\n\
-t                  test for <font> (returns 0 on success)\n\
--help              print this message and exit\n\
--version           print version number and exit\n\
\n\
Report bugs to tex-k@tug.org.\n\
"

static void
usage(void)
{
  fputs("Usage: ttf2pk [-q] [-n] <font> <dpi>\n", stdout);
  fputs("       ttf2pk -t [-q] <font>\n", stdout);
  fputs(USAGE, stdout);
  exit(0);
}


#define VERSION "\
Copyright (C) 1997-1999, 2000, 2002, 2009 Frederic Loyer and Werner Lemberg.\n\
              2009-2013 Peter Breitenlohner\n\
There is NO warranty.  You may redistribute this software\n\
under the terms of the GNU General Public License\n\
and the gsftopk copyright.\n\
\n\
For more information about these matters, see the files\n\
named COPYING and pklib.c.\n\
\n\
Primary authors of ttf2pk: F. Loyer and W. Lemberg.\n\
\n\
ttf2pk is partially based on gsftopk from P. Vojta\n\
and the FreeType project from\n\
David Turner, Robert Wilhelm, and Werner Lemberg\n\
"

static void
version(void)
{
  fputs(ident, stdout);
  fprintf(stdout, " (%s)\n", TeX_search_version());
  fputs(VERSION, stdout);
  exit(0);
}


static int
compare(Font *fnt, char *s, char *key)
{
  char c;
  char *p;
  char *temp, *temp1;
  int value;
  int sfd_begin, postfix_begin;


  /*
   *   We isolate the fontname.
   */

  while (isspace((unsigned char)*s))
    s++;

  p = s;
  while (*p && !isspace((unsigned char)*p))
    p++;

  c = *p;
  *p = '\0';
  temp = newstring(s);
  *p = c;

  /*
   *   We search for a subfont definition file name.
   */

  handle_sfd(temp, &sfd_begin, &postfix_begin);

  if (sfd_begin == -1)
    value = strcmp(temp, key);
  else
  {
    size_t len, len1, len2;


    /*
     *   The sfd file will be only searched if prefix and postfix match.
     */

    len = strlen(key);
    len1 = strlen(temp);
    len2 = strlen(temp + postfix_begin);

    if (len1 + len2 >= len)
      value = -1;
    else if (!strncmp(temp, key, len1) &&
             !strcmp(temp + postfix_begin, key + (len - len2)))
    {
      c = key[len - len2];
      key[len - len2] = '\0';
      temp1 = newstring(key + len1);
      key[len - len2] = c;

      if (fnt->sfdname)
        free(fnt->sfdname);
      fnt->sfdname = newstring(temp + sfd_begin);

      /*
       *   If the sfd file can't be opened the search is continued.
       */

      value = !init_sfd(fnt, False);

      if (!value)
      {
        value = -1;

        while (get_sfd(fnt, True))
        {
          if (!strcmp(fnt->subfont_name, temp1))
          {
            value = 0;                              /* success */
            have_sfd = True;
            break;
          }
        }

        close_sfd();
      }

      free(temp1);
    }
    else
      value = -1;
  }

  free(temp);

  return value;
}


static void
add_mapfile(char *p)
{
  char *q;
 
  if (*p != '+')    /* if the first character is not '+' reset mapfiles */
  {
    if (mapfiles)
    {
      free(mapfiles);
      mapfiles = NULL;
    }
  }
  else
    while (isspace((unsigned char)*++p))
      ;
  for (q = p; *q != 0 && !isspace((unsigned char)*q); q++)
    ;
  *q = '\n';                        /* '\n' is the splitting character */
  if (mapfiles == NULL)
  {
    mapfiles = (char *)mymalloc(q - p + 2);
    *mapfiles = '\0';
  }
  else
    mapfiles = (char *)myrealloc(mapfiles,
                                 strlen(mapfiles) + q - p + 2);
  strncat(mapfiles, p, (unsigned)(q - p));
  strcat(mapfiles, "\n");
}


static void
read_config_file(Boolean quiet)
{
  FILE *config_file;
  char *p, *configline, *real_cfg_filename;


  real_cfg_filename = TeX_search_config_file(&cfg_filename);
  if (real_cfg_filename)
  {
    config_file = fopen(real_cfg_filename, "rt");
    if (config_file != NULL)
    {
      for (;;)
      {
        if (!(configline = get_line(config_file)))
          oops("Error while reading %s.", cfg_filename);
        if (!*configline)
          break;
        p = configline;
        while (isspace((unsigned char)*p))
          p++;
        /* ignore comments */
        if (*p == '*' || *p == '#' || *p == ';' || *p == '%')
          continue;
        if (strlen(p) > 4 && strncmp(p, "map", 3) == 0)
        {
          p += 3; /* move by the size of "map" */
          while (isspace((unsigned char)*p))
            p++;
          if (*p)
            add_mapfile(p);
        }
        else if (!quiet)
          warning("Unknown configuration option `%s'.", p);
      }
      fclose(config_file);
    }
    else if (!quiet)
      warning("Cannot open file `%s'.", cfg_filename);
  }
  else if (!quiet)
    warning("Cannot find file `%s'.\n"
            "Use `ttfonts.map' instead as map file.", cfg_filename);

  if (mapfiles == NULL)
    mapfiles = newstring("ttfonts.map\n");
}


int 
main(int argc, char** argv)
{
  size_t l;
  unsigned int i;
  long index, code;
  FILE *map_file;
  char *configline, *oldconfigline, *p, *q;
  Font font;
  encoding *enc;
  long inenc_array[256];
  char *fontname;
  size_t fontname_len;
  char *pk_filename, *tfm_filename, *enc_filename;
  char *map_filename = NULL;
  char *real_ttfname, *real_map_filename;
  int dpi = 0, ptsize;
  Boolean hinting = True;
  Boolean quiet = False;
  Boolean no_dpi = False;
  Boolean testing = False;


#if !defined(MIKTEX)
#ifdef MIKTEX
  miktex_initialize();
#endif
#endif

  TeX_search_init(argv[0], "ttf2pk", "TTF2PK");

  if (argc == 1)
    oops("Need at least two arguments.\n"
         "Try `ttf2pk --help' for more information.");

  if (argc == 2)
  {
    if (strcmp(argv[1], "--help") == 0)
      usage();
    else if (strcmp(argv[1], "--version") == 0)
      version();
  }

  while (argv[1] && argv[1][0] == '-')
  {
    if (argv[1][1] == 'q')
      quiet = True;
    else if (argv[1][1] == 'n')
      no_dpi = True;
    else if (argv[1][1] == 't')
      testing = True;
    else
      oops("Unknown option `%s'.\n"
           "Try `ttf2pk --help' for more information.", argv[1]);

    argv++;
    argc--;
  }

  if (testing)
  {
    if (argc != 2)
      oops("Need exactly one parameter for `-t' option.\n"
           "Try `ttf2pk --help' for more information.");
  }
  else if (argc != 3)
    oops("Need at most two arguments.\n"
         "Try `ttf2pk --help' for more information.");

  if (!quiet)
    printf("This is %s\n", ident);

  if (!testing)
    if ((dpi = atoi(argv[2])) <= 50)
      oops("dpi value must be larger than 50.");

  fontname = argv[1];
  fontname_len = strlen(fontname);
  enc_filename = NULL;

  ptsize = 10;

  init_font_structure(&font);

  mapfiles = NULL;
  read_config_file(quiet);

  p = mapfiles;
  while (*p)
  {
    if (map_filename)
      free(map_filename);
    map_filename = p;
    while (*p != '\n')
      p++;
    *p++ = '\0';

    map_filename = newstring(map_filename);
    real_map_filename = TeX_search_map_file(&map_filename);
    if (!real_map_filename)
    {
      if (!quiet)
        warning("Cannot find file `%s'.", map_filename);
      continue;
    }

    map_file = fopen(real_map_filename, "rt");
    if (map_file == NULL)
    {
      if (!quiet)
        warning("Cannot open file `%s'.", map_filename);
      continue;
    }

    for (;;) 
    {
      if (!(configline = get_line(map_file)))
        oops("Error while reading %s.", map_filename);
      if (!*configline)
        break;
      if (compare(&font, configline, fontname) == 0)
      {
        fclose(map_file);
        goto font_found;
      }
    }

    fclose(map_file);
  }

  /*
   *   This is the only error message we suppress if the -q flag
   *   is set, making it possible to call ttf2pk silently.
   */
  if (!quiet)
    fprintf(stdout,
            "%s: ERROR: Cannot find font %s in map files.\n",
            progname, fontname);
  exit(2);

font_found:  
  if (testing)
  {
    if (!quiet)
      fprintf(stdout, "%s\n", configline);
    exit(0);
  }

  /*
   *   Parse the line from the config file.  We split the config line buffer
   *   into substrings according to the given options.
   */

  l = strlen(configline);
  if (configline[l - 1] == '\n')
    configline[l - 1] = '\0';     /* strip newline */

  oldconfigline = newstring(configline);

  p = configline;
  while (isspace((unsigned char)*p))
    p++;
  while (*p && !isspace((unsigned char)*p))
    p++;

  q = p;

  while (*p && isspace((unsigned char)*p))
    p++;
  if (!*p)
    boops(oldconfigline, q - configline, "TTF file missing.");

  font.ttfname = p;

  while (*p && !isspace((unsigned char)*p))
    p++;
  if (*p)
    *p++ = '\0';      

  for (; *p; p++)
  {
    if (isspace((unsigned char)*p))
      continue;

    if (!strncmp(p, "Slant", 5))
    {
      p = strip_equal(configline, oldconfigline, p + 5);
      if (sscanf(p, "%f", &(font.slant)) == 0)
        boops(oldconfigline, p - configline, "Bad `Slant' parameter.");
    }
    else if (!strncmp(p, "Encoding", 8))
    {
      if (have_sfd)
        boops(oldconfigline, p - configline,
              "No `Encoding' parameter allowed for subfonts.");

      p = strip_equal(configline, oldconfigline, p + 8);
      if (!*p)
        boops(oldconfigline, p - configline, "Bad `Encoding' parameter.");
      enc_filename = p;
    }
    else if (!strncmp(p, "Extend", 6))
    {
      p = strip_equal(configline, oldconfigline, p + 6);
      if (sscanf(p, "%f", &(font.efactor)) == 0)
        boops(oldconfigline, p - configline, "Bad `Extend' parameter.");
    }
    else if (!strncmp(p, "Fontindex", 9))
    {
      p = strip_equal(configline, oldconfigline, p + 9);
      if (sscanf(p, "%lu", &(font.fontindex)) < 0)
        boops(oldconfigline, p - configline, "Bad `Fontindex' parameter.");
    }
    else if (!strncmp(p, "Pid", 3))
    {
      p = strip_equal(configline, oldconfigline, p + 3);
      if (sscanf(p, "%hu", &(font.pid)) < 0)
        boops(oldconfigline, p - configline, "Bad `Pid' parameter.");
      have_pid = True;
    }
    else if (!strncmp(p, "Eid", 3))
    {
      p = strip_equal(configline, oldconfigline, p + 3);
      if (sscanf(p, "%hu", &(font.eid)) < 0)
        boops(oldconfigline, p - configline, "Bad `Eid' parameter.");
      have_eid = True;
    }
    else if (!strncmp(p, "Hinting", 7))
    {
      p = strip_equal(configline, oldconfigline, p + 7);
      if (p[1] == 'N' || p[1] == 'n' ||
          p[0] == 'Y' || p[1] == 'y' ||
          p[0] == '1')
        hinting = True;
      else if (p[1] == 'F' || p[1] == 'f' ||
               p[0] == 'N' || p[1] == 'n' ||
               p[0] == '0')
        hinting = False;
      else
        boops(oldconfigline, p - configline, "Bad `Hinting' parameter.");
    }
    else if (!strncmp(p, "PS", 2))
    {
      p = strip_equal(configline, oldconfigline, p + 2);
      if (p[1] != '\0' &&
          (p[2] == 'l' || p[2] == 'L'))
        font.PSnames = Only;
      else if (p[1] == 'N' || p[1] == 'n' ||
               p[0] == 'Y' || p[0] == 'y' ||
               p[0] == '1')
        font.PSnames = Yes;
      else if (p[1] == 'F' || p[1] == 'f' ||
               p[0] == 'N' || p[0] == 'n' ||
               p[0] == '0')
        font.PSnames = No;
      else
        boops(oldconfigline, p - configline, "Bad `PS' parameter.");

      if (have_sfd)
        boops(oldconfigline, p - configline,
              "No `PS' parameter allowed for subfonts.");
    }
    else if (!strncmp(p, "Rotate", 6))
    {
      p = strip_equal(configline, oldconfigline, p + 6);
      if (p[1] == 'N' || p[1] == 'n' ||
          p[0] == 'Y' || p[1] == 'y' ||
          p[0] == '1')
        font.rotate = True;
      else if (p[1] == 'F' || p[1] == 'f' ||
               p[0] == 'N' || p[1] == 'n' ||
               p[0] == '0')
        font.rotate = False;
      else
        boops(oldconfigline, p - configline, "Bad `Rotate' parameter.");

      if (!have_sfd)
        boops(oldconfigline, p - configline,
              "No `Rotate' parameter allowed for non-subfonts.");
    }
    else if (!strncmp(p, "Y-Offset", 8))
    {
      p = strip_equal(configline, oldconfigline, p + 8);
      if (sscanf(p, "%f", &(font.y_offset)) == 0)
        boops(oldconfigline, p - configline, "Bad `Y-Offset' parameter.");
    }
    else if (!strncmp(p, "Replacement", 11))
    {
      p = strip_equal(configline, oldconfigline, p + 11);
      if (!*p)
        boops(oldconfigline, p - configline, "Bad `Replacement' parameter.");
      font.replacementname = p;
    }
    else
    {
      char *new_name, *old_name;
      stringlist *sl;


      old_name = p;
      while (*p && !isspace((unsigned char)*p) && *p != '=')
        p++;

      q = p;
      p = strip_equal(configline, oldconfigline, p);
      *q = '\0';

      new_name = p;
      while (*p && !isspace((unsigned char)*p))
        p++;
      if (*p)
        *p++ = '\0';

      sl = newstringlist();
      sl->new_name = new_name;
      sl->old_name = old_name;
      sl->next = font.replacements;
      font.replacements = sl;

      p--;                      /* to make the next while loop work */
    }

    while (*p && !isspace((unsigned char)*p))
      p++;
    if (*p)
      *p = '\0';
  }

  if (font.PSnames == Only)
    if (have_pid || have_eid)
      boops(oldconfigline, 0,
            "No `Pid' or `Eid' parameters allowed if `PS=Only' is set.");

  font.replacementname = newstring(font.replacementname);
  get_replacements(&font);

  tfm_filename = newstring(fontname);
  TFMopen(&tfm_filename);

  pk_filename = mymalloc(fontname_len + 10);
  if (no_dpi)
    sprintf(pk_filename, "%s.pk", fontname);
  else
    sprintf(pk_filename, "%s.%dpk", fontname, dpi);
  PKopen(pk_filename, fontname, dpi);

  font.ttfname = newstring(font.ttfname);
  real_ttfname = TeX_search_ttf_file(&(font.ttfname));
  if (!real_ttfname)
    oops("Cannot find `%s'.", font.ttfname);
  TTFopen(real_ttfname, &font, dpi, ptsize, quiet);

  enc_filename = newstring(enc_filename);
  enc = readencoding(&enc_filename, &font, True);
  if (enc)
  {
    const char *name;


    restore_glyph(enc, &font);

    for (i = 0; i <= 0xFF; i++)
    {
      name = enc->vec[i];
      if (!font.PSnames)
      {
        code = adobename_to_code(name);
        if (code < 0 && strcmp(name, ".notdef") != 0)
          warning("Cannot map character `%s'.", name);
        inenc_array[i] = code;
      }
      else
      {
        /* we search the glyph index */
        index = TTFsearch_PS_name(name);
        if (index < 0)
          warning("Cannot map character `%s'.", name);
        inenc_array[i] = index | 0x1000000;
      }
    }
  }
  else
  {
    if (font.replacements)
      warning("Replacement glyphs will be ignored.");

    if (have_sfd)
      TTFget_subfont(&font, inenc_array);
    else
      /* get the table of glyph names too */
      enc = TTFget_first_glyphs(&font, inenc_array);
  }

  for (i = 0; i <= 0xFF; i++)
  {
    byte *bitmap;
    int w, h, hoff, voff;


    if ((code = inenc_array[i]) >= 0)
    {
      if (!quiet)
      {
        printf("Processing glyph %3ld   %s index 0x%04lx  %s\n",
               (long)i, (code >= 0x1000000) ? "glyph" : "code",
               (code & 0xFFFFFF), enc ? enc->vec[i] : "");
        fflush(stdout);
      }

      if (TTFprocess(&font, code,
                     &bitmap, &w, &h, &hoff, &voff, hinting, quiet))
        PKputglyph(i,
                   -hoff, -voff, w - hoff, h - voff,
                   w, h, bitmap);
      else
        warning("Cannot render glyph with %s index 0x%lx.",
                (code >= 0x1000000) ? "glyph" : "code",
                (code & 0xFFFFFF));
    }
  }

  PKclose();
  exit(0);      /* for safety reasons */
  return 0;     /* never reached */
}


/* end */
