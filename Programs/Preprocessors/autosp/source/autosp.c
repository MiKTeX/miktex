char version[12] = "2020-03-11";

/*  Copyright (C) 2014-20 R. D. Tennent School of Computing,
 *  Queen's University, rdt@cs.queensu.ca
 *
 *  This program is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU General
 *  Public License as published by the Free Software
 *  Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will
 *  be useful, but WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A
 *  PARTICULAR PURPOSE. See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General
 *  Public License along with this program; if not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street,
 *  Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*  autosp - preprocessor to generate note-spacing commands for MusiXTeX scores */

/*  Usage: autosp [-v | --version | -h | --help]
 *         autosp [-d | --dotted] [-l | --log] infile[.aspc] [outfile[.tex]]
 *
 *  Options 
 *    --dotted (-d) suppresses extra spacing for dotted beam notes.
 *    --log (-l) generate a log file
 *
 *
 *
 *  Key functions:
 *
 *    process_command, which processes commands
 *      \instrumentnumber
 *      \nbinstrument
 *      \setstaffs
 *      \startpiece
 *      \TransformNotes
 *      \def\vnotes
 *      \def\atnextbar
 *      \anotes etc.
 *      \bar
 *      \endpiece etc.
 *
 *    analyze_notes, which determines pointers notes[i] and 
 *      characters terminator[i] for each staff; '$' is used
 *      as the terminator for the last staff.
 *
 *    generate_notes, which repeatedly uses spacing_note (i) to
 *      determine spacings[i] of the next spacing note in 
 *      each staff; the current spacing is then the smallest
 *      of these.  If necessary, the current notes command
 *      is terminated by terminate_notes() and a new notes 
 *      command initiated by initialize_notes(). The current global 
 *      skip and note commands for each staff are then copied to 
 *      outstrings[i] using output_notes (i).  When necessary, 
 *      vspacing[i] is set to generate subsequent skips.
 *      Appoggiaturas and xtuplets are treated separately.
 *
 *    spacing_note (i), which searches for the next spacing note
 *      in the i'th staff, unless a "virtual note" (skip) is
 *      needed.
 *  
 *    output_notes (i), which copies a global_skip string and 
 *    outstrings[i] to output, using filter_output (i) to discard
 *    commas and global and large skips, and insert collective-coding 
 *    prefixes and closing braces.  
 *
 *    terminate_notes, which copies outstrings[i] to output,
 *    in the order determined by TransformNotes2.
 *
 *  Spacing values are expressed as sums of binary multiples of 
 *    SMALL_NOTE (by default, 1/256th); for example, half-note
 *    spacing = SP(2) = SMALL_NOTE/2. Special small or large 
 *    values are used for appoggiaturas and bar-rests.
 *      
 */

# include "utils.h"

#ifdef HAVE_CONFIG_H   /* for TeXLive */
#include <config.h>
#endif

# ifdef KPATHSEA
# include <kpathsea/getopt.h>
# else
# include <getopt.h>
# endif

# ifdef HAVE_STDBOOL_H
# include <stdbool.h>
# else
# ifndef HAVE__BOOL
# define _Bool signed char
# endif
# define bool    _Bool
# define true    1
# define false   0
# endif

# define PRIVATE static

# define SHORT_LEN 256

bool debug;
bool dottedbeamnotes;          /* dotted beam notes ignored?        */

char infilename[SHORT_LEN];
char *infilename_n;
char outfilename[SHORT_LEN];
char *outfilename_n;
char logfilename[SHORT_LEN];
char *logfilename_n;
FILE *infile, *outfile, *logfile;


void process_score ();

PRIVATE void
usage ()
{
  printf ("Usage: autosp [-v | --version | -h | --help]\n");
  printf ("       autosp [-d | --dotted] [-l | --log] infile[.aspc] [outfile[.tex]]\n");
  printf ("       autosp [-d | --dotted] [-l | --log] infile.tex outfile[.tex]\n");
}


bool dottedbeamnotes = false;   /* dotted beam notes ignored?        */
bool debug = false;

int main (int argc, char *argv[])
{
  int c;
# define NOPTS 5
  struct option longopts[NOPTS] =
  {  { "help", 0, NULL, 'h'},
     { "version", 0, NULL, 'v'},
     { "dotted", 0, NULL, 'd'},
     { "log", 0, NULL, 'l'},
     { NULL, 0, NULL, 0}
  };
  
  printf ("This is autosp, version %s.\n", version);
  printf ("Copyright (C) 2014-20  R. D. Tennent\n" );
  printf ("School of Computing, Queen's University, rdt@cs.queensu.ca\n" );
  printf ("License GNU GPL version 2 or later <http://gnu.org/licences/gpl.html>.\n" );
  printf ("There is NO WARRANTY, to the extent permitted by law.\n\n" );

  c = getopt_long (argc, argv, "hvdl", longopts, NULL);
  while (c != -1)
    {
      switch (c)
        {
        case 'h':
          usage ();
          printf ("Please report bugs to rdt@cs.queensu.ca.\n" );
          exit (0);
        case 'v':
          exit (0);
        case 'd':
          dottedbeamnotes = true;
          break;
        case 'l':
          debug = true;
          break;
        case '?':
          exit (EXIT_FAILURE);
        default:
          printf ("Function getopt returned character code 0%o.\n",
                  (unsigned int) c);
          exit (EXIT_FAILURE);
        }
      c = getopt_long (argc, argv, "hvd", longopts, NULL);
    }

  infilename[0] = '\0';
  infilename_n = infilename;
  if (optind < argc)
  { append (infilename, &infilename_n, argv[optind], sizeof (infilename));
    if (!suffix (".aspc", infilename)) 
      append (infilename, &infilename_n, ".aspc", sizeof (infilename));
  }
  else 
  {  usage();
     exit (EXIT_FAILURE);
  }
  infile = fopen (infilename, "r");
  if (infile == NULL )
  { printf ("Can't open %s\n", infilename);
    exit (EXIT_FAILURE);
  }

  printf ("Reading from %s.", infilename);

  optind++;
  outfilename[0] = '\0';
  outfilename_n = outfilename;
  if (optind < argc)  /* user-provided outfilename */
  {
    append (outfilename, &outfilename_n, argv[optind], sizeof (outfilename));
    if (!suffix (".tex", outfilename))
      append (outfilename, &outfilename_n, ".tex", sizeof (outfilename));
  }
  else if (suffix (".aspc", infilename)  )
  { /* output to infile.tex  */
    append (outfilename, &outfilename_n, infilename, sizeof (outfilename));
    outfilename_n -= 4;
    *outfilename_n = '\0';
    append (outfilename, &outfilename_n, "tex", sizeof (outfilename));
  }

  if (*outfilename == '\0')
  { 
    printf ("\nPlease provide a filename for the output.\n\n");
    exit (EXIT_FAILURE);
  }
  else
  {
    outfile = fopen (outfilename, "w");
    if (outfile == NULL)
    { printf ("Can't open %s\n", outfilename);
      exit (EXIT_FAILURE);
    }
    printf (" Writing to %s.", outfilename);
  }

  logfilename[0] = '\0';
  logfilename_n = logfilename;
  append (logfilename, &logfilename_n, infilename, sizeof (logfilename));
  if (suffix (".aspc", infilename) )
    logfilename_n -= 4;
  else
    logfilename_n -= 3;
  *logfilename_n = '\0';
  append (logfilename, &logfilename_n, "alog", sizeof (logfilename));
  if (debug)  
  { logfile = fopen (logfilename, "w");
    if (logfile == NULL)
    { printf ("Can't open %s\n", logfilename);
      exit (EXIT_FAILURE);
    }
    printf (" Log file %s.", logfilename);
  }
  printf ("\n\n");

  fprintf (outfile, "%%  Generated by autosp (%s).\n", version);

  process_score ();
  
  return 0;
}
