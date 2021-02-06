/*
 *   ttf2tfm.c
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000, 2002 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>.
 *   Copyright 2009-2013 by
 *     Peter Breitenlohner <tex-live@tug.org>
 */

/*
 *   This program converts TTF files to TeX TFM files, and optionally
 *   to TeX VPL files that retain all kerning and ligature information.
 *   Both files make the characters not normally encoded by TeX available
 *   by character codes greater than 0x7F.
 */

/*
 *   Adapted from afm2tfm by F. Loyer <loyer@ensta.fr>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>         /* for size_t */
#include <string.h>

#if !defined(MIKTEX)
#ifdef MIKTEX
#include <miktex.h>
#endif
#endif

#include "ttf2tfm.h"
#include "newobj.h"
#include "ttfenc.h"
#include "ligkern.h"
#include "texenc.h"
#include "ttfaux.h"
#include "tfmaux.h"
#include "vplaux.h"
#include "errormsg.h"
#include "filesrch.h"
#include "parse.h"
#include "subfont.h"


char ident[] = "ttf2tfm version 2.0";
const char *progname = "ttf2tfm";            /* for error/warning messages */

/* command line options */

static char    makevpl;                 /* can be 1 or 2 */
static Boolean pedantic;
static Boolean quiet;
static Boolean forceoctal;


/*
 *   Re-encode the TTF font.
 */

static void
handlereencoding(Font *fnt)
{
  int i;
  ttfinfo *ti;
  const char *p;


  if (fnt->inencname)
  {
    fnt->inencoding = readencoding(&(fnt->inencname), fnt, True);

    /* reset all pointers in the mapping table */
    for (i = 0; i <= 0xFF; i++)
      if (NULL != (ti = fnt->inencptrs[i]))
      {
        ti->incode = -1;
        fnt->inencptrs[i] = NULL;
      }

    /*
     *   Reencode TTF <--> raw TeX.  Only these code points will be used
     *   for the output encoding.
     */

    for (i = 0; i <= 0xFF; i++)
    {
      p = fnt->inencoding->vec[i];
      if (p && *p)
      {
        if ((ti = findadobe(p, fnt->charlist)))
        {
          if (ti->incode >= 0)
          {
            warning("Character `%s' encoded twice in input encoding\n"
                    "         (positions %x and %x; the latter is ignored).",
                    p, ti->incode, i);
            fnt->inencoding->vec[i] = ".notdef";
            continue;
          }
          if (ti->charcode >= 0)
          {
            ti->incode = i;
            fnt->inencptrs[i] = ti;
          }
        }
        else
        {
          warning("Cannot find character `%s'\n"
                  "         specified in input encoding.", p);
        }
      }
    }
    fnt->codingscheme = fnt->inencoding->name;
  }

  if (!quiet)
  {
    if (fnt->inencname)
      printf("\nUsing %s as input encoding.\n", fnt->inencname);
    else
    {
      printf(
        "\nUsing the first 256 glyphs in the following input encoding:\n\n");
      for (i = 0; i <= 0xFF; i++)
      {
        if ((ti = fnt->inencptrs[i]))
          printf("  0x%02x    %s\n", i, ti->adobename);
      }
      printf("\n");
    }
  }

  if (fnt->outencname)
    fnt->outencoding = readencoding(&(fnt->outencname), fnt, False);
  else
    fnt->outencoding = readencoding(NULL, fnt, False);
}


static void
assignchars(Font *fnt)
{
  register const char **p;
  register int i, j, k;
  register ttfinfo *ti;
  int nextfree = 0x80;


  /*
   *   First, we assign all those that match perfectly.
   */

  for (i = 0, p = fnt->outencoding->vec; i <= 0xFF; i++, p++)
    if ((ti = findmappedadobe(*p, fnt->inencptrs)))
    {
      if (ti->outcode >= 0)
        fnt->nextout[i] = ti->outcode;       /* linked list */
      ti->outcode = i;
      fnt->outencptrs[i] = ti;
    }
    else if (strcmp(*p, ".notdef") != 0)
      warning("Cannot map character `%s'\n"
              "         specified in output encoding.", *p);

  if (pedantic)
    goto end;

  /*
   *   Next, we assign all the others, retaining the TTF code positions,
   *   possibly multiplying assigned characters, unless the output encoding
   *   was precisely specified.
   */

  for (i = 0; i <= 0xFF; i++)
    if ((ti = fnt->inencptrs[i]) &&
         ti->charcode >= 0 && ti->charcode <= 0xFF &&
         ti->outcode < 0 && fnt->outencptrs[ti->charcode] == NULL)
    {
      ti->outcode = ti->charcode;
      fnt->outencptrs[ti->charcode] = ti;
    }

  /*
   *   Finally, we map all remaining characters into free locations beginning
   *   with 0x80.
   */

  for (i = 0; i <= 0xFF; i++)
    if ((ti = fnt->inencptrs[i]) && ti->outcode < 0)
    {
      while (fnt->outencptrs[nextfree])
      {
        nextfree = (nextfree + 1) & 0xFF;
        if (nextfree == 0x80)
          goto finishup;        /* all slots full */
      }
      ti->outcode = nextfree;
      fnt->outencptrs[nextfree] = ti;
    }

finishup:

  /*
   *   Now, if any of the characters are encoded multiple times, we want
   *   ti->outcode to be the first one assigned, since that is most likely
   *   to be the most important one.  So we reverse the above lists.
   */

  for (i = 0; i <= 0xFF; i++)
    if ((ti = fnt->inencptrs[i]) && ti->outcode >= 0)
    {
      k = -1;
      while (fnt->nextout[ti->outcode] >= 0)
      {
        j = fnt->nextout[ti->outcode];
        fnt->nextout[ti->outcode] = k;
        k = ti->outcode;
        ti->outcode = j;
      }
      fnt->nextout[ti->outcode] = k;
    }

end:

  if (!quiet)
  {
    printf("\nUsing the following output encoding:\n\n");
    for (i = 0; i <= 0xFF; i++)
    {
      if ((ti = fnt->outencptrs[i]))
        printf("  0x%02x    %s\n", i, ti->adobename);
    }
    printf("\n");
  }
}


#define MAX_SFLIST 10
static void
add_subfont_list(Font *fnt)
{
  if (fnt->subfont_list == NULL)
    fnt->subfont_list = (sflist *)mymalloc(MAX_SFLIST * sizeof(sflist));
  else if (fnt->subfont_num % MAX_SFLIST == 0)
    fnt->subfont_list = (sflist *)myrealloc(fnt->subfont_list,
      (fnt->subfont_num / MAX_SFLIST + 1) * MAX_SFLIST * sizeof(sflist));

  (fnt->subfont_list[fnt->subfont_num]).name =
      (char *)mymalloc(strlen(fnt->fullname)+1);
  strcpy((fnt->subfont_list[fnt->subfont_num]).name, fnt->fullname);
  (fnt->subfont_list[fnt->subfont_num]).cksum = fnt->cksum;

  fnt->subfont_num++;
}


static void
release_subfont_list(Font *fnt)
{
  register int i;

  for (i = 0; i < fnt->subfont_num; i++)
    free((fnt->subfont_list[i]).name);

  free(fnt->subfont_list);
}


#define VERSION "\
Copyright (C) 1997-1999, 2000, 2002 Frederic Loyer and Werner Lemberg.\n\
              2009-2013 Peter Breitenlohner\n\
There is NO warranty.  You may redistribute this software\n\
under the terms of the GNU General Public License\n\
and the Dvips copyright.\n\
\n\
For more information about these matters, see the files\n\
named COPYING and ttf2tfm.c.\n\
\n\
Primary authors of ttf2tfm: F. Loyer and W. Lemberg.\n\
\n\
ttf2tfm is based on afm2tfm from T. Rokicki\n\
and the FreeType project from\n\
David Turner, Robert Wilhelm, and Werner Lemberg.\n\
"

static void
version(void)
{
  fputs(ident, stdout);
  fprintf(stdout, " (%s, %s)\n", TeX_search_version(), TL_VERSION);
  fputs(VERSION, stdout);
  exit(0);
}


#define USAGE "\
  Convert a TrueType font table to TeX's font metric format.\n\
\n\
-c REAL             use REAL for height of small caps made with -V [0.8]\n\
-e REAL             widen (extend) characters by a factor of REAL [1.0]\n\
-E INT              select INT as the TTF encoding ID [1]\n\
-f INT              select INT as the font index in a TTC [0]\n\
-l                  create 1st/2nd byte ligatures in subfonts\n\
-L LIGFILE[.sfd]    create 1st/2nd byte ligatures in subfonts using LIGFILE\n\
-n                  use PS names of TrueType font\n\
-N                  use only PS names and no cmap\n\
-o FILE[.ovp]       make an OVP file for conversion to OVF and OFM\n\
-O                  use octal for all character codes in the vpl file\n\
-p ENCFILE[.enc]    read ENCFILE for the TTF->raw TeX mapping\n\
-P INT              select INT as the TTF platform ID [3]\n\
-q                  suppress informational output\n\
-r OLDNAME NEWNAME  replace glyph name OLDNAME with NEWNAME\n\
-R RPLFILE[.rpl]    read RPLFILE containing glyph replacement names\n\
-s REAL             oblique (slant) characters by REAL, usually <<1 [0.0]\n\
-t ENCFILE[.enc]    read ENCFILE for the encoding of the vpl file\n\
-T ENCFILE[.enc]    equivalent to -p ENCFILE -t ENCFILE\n\
-u                  output only characters from encodings, nothing extra\n\
-v FILE[.vpl]       make a VPL file for conversion to VF\n\
-V SCFILE[.vpl]     like -v, but synthesize smallcaps as lowercase\n\
-w                  generate subfont enc. vectors containing glyph indices\n\
-x                  rotate subfont glyphs by 90 degrees\n\
-y REAL             move rotated glyphs down by a factor of REAL [0.25]\n\
--help              print this message and exit\n\
--version           print version number and exit\n\
\n\
Report bugs to " PACKAGE_BUGREPORT ".\n\
"

static void
usage(void)
{
  fputs("Usage: ttf2tfm FILE[.ttf|.ttc] [OPTION]... [FILE[.tfm]]\n", stdout);
  fputs(USAGE, stdout);
  exit(0);
}


static void
handle_options(int argc, char *argv[], Font *fnt)
{
  register int lastext;
  register int i;
  size_t l;
  int arginc;
  char *temp;
  char c;
  char *vpl_name = NULL;
  Boolean have_capheight = 0;
  Boolean have_sfd = 0;
  int sfd_begin, postfix_begin;
  int base_name;
  stringlist* sl;


  /* scan first whether the -q switch is set */
  for (i = 1; i < argc; i++)
    if (argv[i][0] == '-' && argv[i][1] == 'q')
      quiet = True;

  if (!quiet)
    printf("This is %s\n", ident);

  /* Make VPL file identical to that created under Unix */
  fnt->titlebuf = (char *)mymalloc(strlen(progname) + strlen(argv[1]) +
                                   1 + 1);
  sprintf(fnt->titlebuf, "%s %s", progname, argv[1]);


  /*
   *   TrueType font name.
   */

  fnt->ttfname = newstring(argv[1]);

  /*
   *   The other arguments.  We delay the final processing of some switches
   *   until the tfm font name has been scanned -- if it contains two `@'s,
   *   many switches are ignored.
   */

  while (argc > 2 && *argv[2] == '-')
  {
    arginc = 2;
    i = argv[2][1];

    switch (i)
    {
    case 'v':
      makevpl = 1;
      if (argc <= 3)
        oops("Missing parameter for -v option.");
      if (vpl_name)
        free(vpl_name);
      vpl_name = newstring(argv[3]);
      handle_extension(&vpl_name, ".vpl");
      break;

    case 'V':
      makevpl = 2;
      if (argc <= 3)
        oops("Missing parameter for -V option.");
      if (vpl_name)
        free(vpl_name);
      vpl_name = newstring(argv[3]);
      handle_extension(&vpl_name, ".vpl");
      break;

    case 'f':
      if (argc <= 3)
        oops("Missing parameter for -f option.");
      if (sscanf(argv[3], "%lu", &(fnt->fontindex)) == 0)
        oops("Invalid font index.");
      fnt->fontindexparam = argv[3];
      break;

    case 'E':
      if (argc <= 3)
        oops("Missing parameter for -E option.");
      if (sscanf(argv[3], "%hu", &(fnt->eid)) == 0)
        oops("Invalid encoding ID.");
      fnt->eidparam = argv[3];
      break;

    case 'P':
      if (argc <= 3)
        oops("Missing parameter for -P option.");
      if (sscanf(argv[3], "%hu", &(fnt->pid)) == 0)
        oops("Invalid platform ID.");
      fnt->pidparam = argv[3];
      break;

    case 'e':
      if (argc <= 3)
        oops("Missing parameter for -e option.");
      if (sscanf(argv[3], "%f", &(fnt->efactor)) == 0 || fnt->efactor < 0.01)
        oops("Bad extension factor.");
      fnt->efactorparam = argv[3];
      break;

    case 'c':
      if (argc <= 3)
        oops("Missing parameter for -c option.");
      have_capheight = True;
      if (sscanf(argv[3], "%f", &(fnt->capheight)) == 0)
        fnt->capheight = 0;
      break;

    case 's':
      if (argc <= 3)
        oops("Missing parameter for -s option.");
      if (sscanf(argv[3], "%f", &(fnt->slant)) == 0)
        oops("Bad slant parameter.");
      fnt->slantparam = argv[3];
      break;

    case 'p':
      if (argc <= 3)
        oops("Missing parameter for -p option.");
      if (fnt->inencname)
        free(fnt->inencname);
      fnt->inencname = newstring(argv[3]);
      break;

    case 'T':
      if (argc <= 3)
        oops("Missing parameter for -T option.");
      if (fnt->inencname)
        free(fnt->inencname);
      if (fnt->outencname)
        free(fnt->outencname);
      fnt->inencname = newstring(argv[3]);
      fnt->outencname = newstring(argv[3]);
      break;

    case 't':
      if (argc <= 3)
        oops("Missing parameter for -T option.");
      if (fnt->outencname)
        free(fnt->outencname);
      fnt->outencname = newstring(argv[3]);
      break;

    case 'r':
      if (argc <= 4)
        oops("Not enough parameters for -r option.");
      sl = newstringlist();
      sl->old_name = newstring(argv[3]);
      sl->new_name = newstring(argv[4]);
      sl->single_replacement = True;
      sl->next = fnt->replacements;
      fnt->replacements = sl;
      arginc = 3;
      break;

    case 'R':
      if (argc <= 3)
        oops("Missing parameter for -R option.");
      if (fnt->replacementname)
        free(fnt->replacementname);
      fnt->replacementname = newstring(argv[3]);
      break;

    case 'y':
      if (argc <= 3)
        oops("Missing parameter for -y option.");
      if (sscanf(argv[3], "%f", &(fnt->y_offset)) == 0)
        oops("Invalid y-offset.");
      fnt->y_offsetparam = argv[3];
      break;

    case 'O':
      forceoctal = True;
      arginc = 1;
      break;

    case 'n':
      fnt->PSnames = Yes;
      arginc = 1;
      break;

    case 'N':
      fnt->PSnames = Only;
      arginc = 1;
      break;

    case 'u':
      pedantic = True;
      arginc = 1;
      break;

    case 'q':
      quiet = True;
      arginc = 1;
      break;

    case 'L':
      if (argc <= 3)
        oops("Missing parameter for -L option.");
      if (fnt->ligname)
        free(fnt->ligname);
      fnt->ligname = newstring(argv[3]);
      fnt->subfont_ligs = True;
      break;

    case 'l':
      fnt->subfont_ligs = True;
      arginc = 1;
      break;

    case 'w':
      fnt->write_enc = True;
      arginc = 1;
      break;

    case 'x':
      fnt->rotate = True;
      arginc = 1;
      break;

    case 'o':
      if (argc <= 3)
        oops("Missing parameter for -o option.");
      if (vpl_name)
        free(vpl_name);
      vpl_name = newstring(argv[3]);
      handle_extension(&vpl_name, ".ovp");
      break;

    default:
      if (argc <= 3 || argv[3][0] == '-')
      {
        warning("Unknown option `%s' will be ignored.\n", argv[2]);
        arginc = 1;
      }
      else
        warning("Unknown option `%s %s' will be ignored.\n",
                argv[2], argv[3]);
    }

    for (i = 0; i < arginc; i++)
    {
      l = strlen(fnt->titlebuf);
      fnt->titlebuf = (char *)myrealloc((void *)fnt->titlebuf,
                                        l + strlen(argv[2]) + 1 + 1);
      sprintf(fnt->titlebuf + strlen(fnt->titlebuf), " %s", argv[2]);
      argv++;
      argc--;
    }
  }

  /* Read replacement glyph name file */

  get_replacements(fnt);

  if (argc > 3 || (argc == 3 && *argv[2] == '-'))
    oops("Need at most two non-option arguments.");

  /*
   *   The tfm file name.
   */

  if (argc == 2)
    temp = newstring(fnt->ttfname);
  else
  {
    temp = newstring(argv[2]);
    l = strlen(fnt->titlebuf);
    fnt->titlebuf = (char *)myrealloc((void *)fnt->titlebuf,
                                      l + strlen(argv[2]) + 1 + 1);
    sprintf(fnt->titlebuf + strlen(fnt->titlebuf), " %s", argv[2]);
  }

  handle_sfd(temp, &sfd_begin, &postfix_begin);

  if (sfd_begin > -1)
  {
    have_sfd = True;
    i = sfd_begin - 2;
  }
  else
    i = strlen(temp) - 1;

  /*
   *   Now we search the beginning of the name without directory.
   */

  for (; i >= 0; i--)
    if (temp[i] == '/' || temp[i] == ':' || temp[i] == '\\')
      break;

  base_name = i + 1;

  /*
   *   We store the path (with the final directory separator).
   */

  if (base_name > 0)
  {
    c = temp[base_name];
    temp[base_name] = '\0';
    fnt->tfm_path = newstring(temp);
    temp[base_name] = c;
  }

  if (have_sfd)
  {
    /* the prefix and the sfd file name */

    if (temp[base_name])
      fnt->outname = newstring(temp + base_name);

    fnt->sfdname = newstring(temp + sfd_begin);
  }
  else
    postfix_begin = base_name;

  /*
   *   Get the extension.
   */

  lastext = -1;
  for (i = postfix_begin; temp[i]; i++)
    if (temp[i] == '.')
      lastext = i;

  if (argc == 2 && lastext >= 0)
  {
    temp[lastext] = '\0';       /* remove TTF file extension */
    lastext = -1;
  }

  if (lastext == -1)
    fnt->tfm_ext = newstring(".tfm");
  else
  {
    fnt->tfm_ext = newstring(temp + lastext);
    temp[lastext] = '\0';
  }

  if (have_sfd)
  {
    if (temp[postfix_begin])
      fnt->outname_postfix = newstring(temp + postfix_begin);
  }
  else
  {
    if (temp[base_name])
      fnt->outname = newstring(temp + base_name);
    else
      oops("Invalid tfm file name.");
  }


  /*
   *   Now we can process the remaining parameters.
   */

  if (have_sfd)
  {
    if (makevpl)
    {
      warning("Ignoring `-v' and `-V' switches for subfonts.");
      makevpl = 0;
    }
    if (vpl_name)
      if ((fnt->vplout = fopen(vpl_name, "wb")) == NULL)
        oops("Cannot open ovp output file.");
    if (have_capheight)
      warning("Ignoring `-c' switch for subfonts.");
    if (fnt->inencname || fnt->outencname)
    {
      warning("Ignoring `-p', `-t', and `-T' switches for subfonts.");
      fnt->inencname = NULL;
      fnt->outencname = NULL;
    }
    if (fnt->y_offsetparam && !fnt->rotate)
      warning("Ignoring `-y' switch for non-rotated subfonts.");
    if (fnt->PSnames)
    {
      warning("Ignoring `-n' or '-N' switch for subfonts.");
      fnt->PSnames = No;
    }

    init_sfd(fnt, True);
  }
  else
  {
    if (have_capheight && fnt->capheight < 0.01)
      oops("Bad small caps height.");

    if (vpl_name)
      if ((fnt->vplout = fopen(vpl_name, "wb")) == NULL)
        oops("Cannot open vpl output file.");
  
    if (fnt->subfont_ligs)
    {
      warning("Ignoring `-l' switch for non-subfont.");
      fnt->subfont_ligs = False;
    }

    if (fnt->rotate)
    {
      warning("Ignoring `-x' switch for non-subfont.");
      fnt->rotate = False;
    }

    if (fnt->write_enc)
    {
      warning("Ignoring `-w' switch for non-subfont.");
      fnt->write_enc = False;
    }

    if (fnt->y_offsetparam)
      warning("Ignoring `-y' switch for non-subfont.");
  }

  if (fnt->PSnames == Only)
  {
    if (fnt->pidparam || fnt->eidparam)
    {
      warning("Ignoring `-P' and `-E' options if `-N' switch is selected.");
      fnt->pidparam = NULL;
      fnt->eidparam = NULL;
    }
  }

  if (vpl_name)
    free(vpl_name);
  free(temp);
}


/*
 *   This routine prints out the line that needs to be added to ttfonts.map.
 */

static void
consttfonts(Font *fnt)
{
  if (!quiet)
    printf("\n");
  if (fnt->outname)
    printf("%s", fnt->outname);
  if (fnt->sfdname)
    printf("@%s@", fnt->sfdname);
  if (fnt->outname_postfix)
    printf("%s", fnt->outname_postfix);

  printf("   %s", fnt->ttfname);

  if (fnt->slantparam || fnt->efactorparam ||
      fnt->inencname ||
      fnt->pidparam || fnt->eidparam ||
      fnt->fontindexparam ||
      fnt->replacements ||
      fnt->replacementname ||
      fnt->PSnames ||
      fnt->rotate || fnt->y_offsetparam)
  {
    if (fnt->slantparam)
      printf(" Slant=%s", fnt->slantparam);
    if (fnt->efactorparam)
      printf(" Extend=%s", fnt->efactorparam);
    if (fnt->inencname)
      printf(" Encoding=%s", fnt->inencname);
    if (fnt->pidparam)
      printf(" Pid=%s", fnt->pidparam);
    if (fnt->eidparam)
      printf(" Eid=%s", fnt->eidparam);
    if (fnt->fontindexparam)
      printf(" Fontindex=%s", fnt->fontindexparam);
    if (fnt->PSnames)
      printf(" PS=%s", fnt->PSnames == Yes ? "Yes" : "Only");
    if (fnt->rotate)
      printf(" Rotate=Yes");
    if (fnt->y_offsetparam)
      printf(" Y-Offset=%s", fnt->y_offsetparam);
    if (fnt->replacementname && fnt->inencoding)
      printf(" Replacement=%s", fnt->replacementname);
    if (fnt->replacements && fnt->inencoding)
    {
      stringlist *sl;


      for (sl = fnt->replacements; sl; sl = sl->next)
        if (sl->single_replacement)
          printf(" %s=%s", sl->old_name, sl->new_name);
    }
  }
  printf("\n");
}


int
main(int argc, char *argv[])
{
  Font font;
  ttfinfo *ti;


#if !defined(MIKTEX)
#ifdef MIKTEX
  miktex_initialize();
#endif
#endif

  init_font_structure(&font);

  TeX_search_init(argv[0], "ttf2tfm", "TTF2TFM");

  if (argc == 1)
  {
    fputs("ttf2tfm: Need at least one file argument.\n", stderr);
    fputs("Try `ttf2tfm --help' for more information.\n", stderr);
    exit(1);
  }
  if (argc == 2)
  {
    if (strcmp(argv[1], "--help") == 0)
      usage();
    else if (strcmp(argv[1], "--version") == 0)
      version();
  }

  handle_options(argc, argv, &font);

  if (font.sfdname)
  {
    while (get_sfd(&font, True))
    {
      char *temp, *ttemp;
      int i, start, end, len;


      get_tfm_fullname(&font);

      /*
       *   Extract base name of sfd file.
       */

      temp = newstring(font.sfdname);
      len = strlen(temp);

      start = 0;
      for (i = len - 1; i >= 0; i--)
        if (temp[i] == '/' || temp[i] == ':' || temp[i] == '\\')
        {
          start = i + 1;
          break;
        }

      end = len;
      for (i = len - 1; i >= 0; i--)
        if (temp[i] == '.')
        {
          end = i;
          break;
        }
      temp[end] = '\0';

      ttemp = (char *)mymalloc(strlen(temp + start) + 4 + 1);
      sprintf(ttemp, "CJK-%s", temp + start);
      font.codingscheme = ttemp;
      free(temp);

      readttf(&font, quiet, True);
      if (font.replacements)
        warning("Replacement glyphs will be ignored.");

      /* second try to get an xheight value */
      if (font.xheight == 0)
      {
        if (NULL != (ti = findadobe("x", font.charlist)))
          font.xheight = ti->ury;
        else if (font.pid == 3 && font.eid == 1 &&
                 NULL != (ti = findadobe(".c0x78", font.charlist)))
          font.xheight = ti->ury;
        else
          font.xheight = 400;
      }

      if (NULL != (ti = findadobe("space", font.charlist)))
        font.fontspace = ti->width;
      else if (NULL != (ti = findadobe(".c0x20", font.charlist)))
        font.fontspace = ti->width;
      else
        font.fontspace = transform(500, 0, font.efactor, font.slant);

      if (font.ligname)
        get_sfd(&font, False); /* read sfd files for ligatures */

      if (buildtfm(&font))
      {
        writetfm(&font);
        if (font.write_enc)
          writeenc(&font);
        if (font.vplout)
          add_subfont_list(&font);
      }
    }

    close_sfd();

    if (font.vplout)
    {
      writeovp(&font);
      fclose(font.vplout);
      release_subfont_list(&font);
    }
  }
  else
  {
    get_tfm_fullname(&font);

    readttf(&font, quiet, False);
    replace_glyphs(&font);

    /* second try to get an xheight value */
    if (font.xheight == 0)
    {
      if (NULL != (ti = findadobe("x", font.charlist)))
        font.xheight = ti->ury;
      else if (font.pid == 3 && font.eid == 1 &&
               NULL != (ti = findadobe(".c0x78", font.charlist)))
        font.xheight = ti->ury;
      else
        font.xheight = 400;
    }

    if (NULL != (ti = findadobe("space", font.charlist)))
      font.fontspace = ti->width;
    else if (NULL != (ti = findadobe(".c0x20", font.charlist)))
      font.fontspace = ti->width;
    else
      font.fontspace = transform(500, 0, font.efactor, font.slant);

    handlereencoding(&font);

    buildtfm(&font);
    writetfm(&font);
  }

  if (makevpl)
  {
    assignchars(&font);
    if (makevpl > 1)
      upmap(&font);
    writevpl(&font, makevpl, forceoctal);
    fclose(font.vplout);
  }

  consttfonts(&font);

  exit(0);      /* for safety reasons */
  return 0;     /* never reached */
}


/* end */
