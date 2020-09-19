char version[12] = "2019-01-08";

/*  Copyright (C) 2017-19 R. D. Tennent School of Computing,
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
 *  You should have received a copy of the GNU General
 *  Public License along with this program; if not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street,
 *  Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*  tex2aspc  - converts conventional MusiXTeX code to autosp-compatible format   */

/*  Usage:  tex2aspc infile[.tex] [outfile[.aspc]]
 *          tex2aspc [-v | --version | -h | --help]
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
# define MAX_STAFFS 9


PRIVATE char infilename[SHORT_LEN];
PRIVATE char *infilename_n = infilename;
PRIVATE char outfilename[SHORT_LEN];
PRIVATE char *outfilename_n = outfilename;
PRIVATE FILE *infile, *outfile;

PRIVATE char line[LINE_LEN];            /* line of input                      */

PRIVATE int ninstr = 1;                 /* number of instruments              */
PRIVATE int staffs[MAX_STAFFS] = {0,1}; /* number of staffs for ith instrument*/
PRIVATE int nstaffs = 1;                /* number of staffs                   */
PRIVATE char terminator[MAX_STAFFS];    /* one of '&' "|', '$'                */

PRIVATE char notes[MAX_STAFFS][LINE_LEN];  
                                        /* accumulate commands to be output   */
PRIVATE char *n_notes[MAX_STAFFS];
PRIVATE bool outputs = false;           /* are there commands to be output?   */



PRIVATE void
usage ()
{
  printf ("Usage: tex2aspc [-v | --version | -h | --help]\n");
  printf ("       tex2aspc infile[.tex] [outfile[.aspc]]\n");
}


PRIVATE
void analyze_notes (char **ln) 
/* append material from *ln to \en to notes[*] and initialize terminator[*]. */
{
  int i; char *s; char *t;  
  int newlines = 0;
  s = *ln+1;  /* skip "/"  */
  while (isalpha(*s)) {s++;}  /* skip rest of the initial command  */
  while (true)
  { /* look for \en */
    t = strstr(s, "\\en");
    if (t != NULL) break;
    /* replace EOL by a blank and append another line of input */
    { char new_line[LINE_LEN];  
      char *nl;
      if (fgets (new_line, LINE_LEN, infile) == NULL)
        error ("Unexpected EOF.");
      if (strlen (new_line) == LINE_LEN-1) 
        error ("Line too long.");
      nl = new_line;
      while (*nl == ' ') nl++;  /* avoid spaces */
      t = strpbrk (s, "\n%");
      if (t == NULL) error ("Missing EOL.");
      if (*t == '\n') 
      { *t = ' '; t++;}
      *t = '\0';
      if (append (line, &t, nl, LINE_LEN) >= LINE_LEN)
        error ("Line too long.");
      newlines++;
    }
  }
  /* t -> "\en"  */
  if ( prefix("\\enotes", t) ) *ln = t+7;
  else *ln = t+3;
  *t = '$'; /* replace '\' by '$' (a distinctive terminating character) */
  for (i=1; i <= nstaffs; i++)      
  { char *tt;
    while (*s == ' ') s++;  /* skip spaces  */
    tt = strpbrk (s, "|&$");
    if (tt == NULL) error ("can't parse note-spacing command.");
    while (s < tt)
    { *n_notes[i] = *s; 
      n_notes[i]++; s++;
    }
    *n_notes[i] = '\0';
    terminator[i] = *s;
    if (*s != '$') s++;
  }
  lineno = lineno + newlines;
}

PRIVATE
void process_command (char **ln)
{ char *s;
  if ( prefix("\\def\\nbinstrument", *ln) ||
       prefix("\\instrumentnumber", *ln)  )
  { 
    s = strpbrk (*ln, "123456789");
    if ( s == NULL ) error ("instrument number command unreadable.");
    ninstr = atoi (s);
    while (*ln <= s) { putc (**ln, outfile); (*ln)++;}
  }

  else if ( prefix("\\setstaffs", *ln) )
  { /* for now, just record in staffs[i]  */
    int n, p;
    s = strpbrk (*ln, "123456789");
    if ( s == NULL ) error ("\\setstaffs command unreadable.");
    n = (int)(*s) - (int)('0'); /* instrument number  */
    s = strpbrk (s+1, "123456789");
    if ( s == NULL ) error ("\\setstaffs command unreadable.");
    p = (int)(*s) - (int)('0'); /* number of staffs */
    staffs[n] = p;
    while (*ln <= s) { putc (**ln, outfile); (*ln)++;}
  }

  else if ( prefix("\\startpiece", *ln)     ||
            prefix("\\startextract", *ln)   ) 
  { int i, j; 
    nstaffs = 1;
    for (j=2; j <= staffs[1]; j++)
      nstaffs++; 
    for (i=2; i <= ninstr; i++) 
    { nstaffs++; 
      for (j=2; j <= staffs[i]; j++)
        nstaffs++; 
    }   
    fputs (*ln, outfile);
    *ln =*ln + strlen(*ln);
  }

  else if ( prefix("\\nnnotes", *ln) ||
            prefix("\\nnotes", *ln) ||
            prefix("\\notes", *ln) || 
            prefix("\\Notes", *ln) ||
            prefix("\\NOtes", *ln) ||
            prefix("\\NOTes", *ln) ||
            prefix("\\NOTEs", *ln) ) 
  { char *s;
    analyze_notes (ln);
    outputs = true;
    s = strchr (*ln, '\n');
    if (s == NULL) s = *ln + strlen (*ln);
    *ln = s+1;
  }

  else 
  { /* if there is output, generate it and re-initialize n_notes[*]  */
    if (outputs)
    { int i;
      fputs ("\\anotes\n", outfile);
      for (i=1; i <= nstaffs; i++)
      {
        fputs ("  ", outfile);
        fputs (notes[i], outfile);
        if (terminator[i] == '$')
          fputs ("\\en", outfile);
        else
          fputc (terminator[i], outfile);
        fputc ('\n', outfile);
        n_notes[i] = notes[i];
        *n_notes[i] = '\0';
      }
      outputs = false;
    }
    fputs (*ln, outfile);
    *ln =*ln + strlen(*ln);
  }

}

PRIVATE
void process_line ()
{
  char  *ln; 
  ln = &line[0];
  while ( *ln != '\0') 
  {
    while (*ln == ' ') { ln++; fputc (' ', outfile); }
    if (*ln == '%') {
      fprintf (outfile, "%s", ln);
      return;
    }
  process_command (&ln); 
  }     
}


PRIVATE
void process_score ()
{
  int c; int i;
  lineno = 0;
  for (i=1; i < MAX_STAFFS; i++) 
  {
    staffs[i] = 1; 
    n_notes[i] = notes[i];
    *n_notes[i] = '\0';
  }
  c = getc (infile);

  while ( c != EOF )
  {
    ungetc (c, infile);
    if ( fgets(line, LINE_LEN, infile) == NULL)  
      error ("Unexpected EOF.");
    if (strlen (line) == LINE_LEN-1) 
      error ("Line too long.");
    lineno++;
    process_line ();
    c = getc (infile);
  }     /* c == EOF  */
}     

int main (int argc, char *argv[])
{
  int c;
  char today[12];
  time_t mytime; 
# define NOPTS 3
  struct option longopts[NOPTS] =
  {  { "help", 0, NULL, 'h'},
     { "version", 0, NULL, 'v'},
     { NULL, 0, NULL, 0}
  };
  
  time (&mytime);
  strftime (today, 11, "%Y-%m-%d", localtime (&mytime) );
  printf ("This is tex2aspc, version %s.\n", version);
  printf ("Copyright (C) 2017  R. D. Tennent\n" );
  printf ("School of Computing, Queen's University, rdt@cs.queensu.ca\n" );
  printf ("License GNU GPL version 2 or later <http://gnu.org/licences/gpl.html>.\n" );
  printf ("There is NO WARRANTY, to the extent permitted by law.\n\n" );

  c = getopt_long (argc, argv, "hv", longopts, NULL);
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
        case '?':
          exit (EXIT_FAILURE);
        default:
          printf ("Function getopt returned character code 0%o.\n",
                  (unsigned int) c);
          exit (EXIT_FAILURE);
        }
      c = getopt_long (argc, argv, "hv", longopts, NULL);
    }

  infilename[0] = '\0';
  infilename_n = infilename;
  if (optind < argc)
    append (infilename, &infilename_n, argv[optind], sizeof (infilename));
  else 
  { usage();
    exit (EXIT_FAILURE);
  }
  infile = fopen (infilename, "r");
  if (infile == NULL)
  {
    if (suffix (".tex", infilename) )
    { printf ("Can't open %s\n", infilename);
      exit (EXIT_FAILURE);
    }
    else
    { append (infilename, &infilename_n, ".tex", sizeof (infilename));
      infile = fopen (infilename, "r");
      if (infile == NULL)
      { printf ("Can't open %s\n", infilename);
        exit (EXIT_FAILURE);
      }
    }
  }
  printf ("Reading from %s.", infilename);

  optind++;
  outfilename[0] = '\0';
  outfilename_n = outfilename;
  if (optind < argc)  /* user-provided outfilename */
  {
    append (outfilename, &outfilename_n, argv[optind], sizeof (outfilename));
    if (!suffix (".aspc", outfilename))
      append (outfilename, &outfilename_n, ".aspc", sizeof (outfilename));
  }
  else 
  { /* output to infile.aspc  */
    append (outfilename, &outfilename_n, infilename, sizeof (outfilename));
    outfilename_n -= 4;
    *outfilename_n = '\0';
    append (outfilename, &outfilename_n, ".aspc", sizeof (outfilename));
  }

  outfile = fopen (outfilename, "w");
  if (outfile == NULL)
  { printf ("Can't open %s\n", outfilename);
    exit (EXIT_FAILURE);
  }
  printf (" Writing to %s.", outfilename);

  printf ("\n\n");

  fprintf (outfile, "%%  Generated by tex2aspc (%s).\n", version);
    process_score (); 
  
  return 0;
}
