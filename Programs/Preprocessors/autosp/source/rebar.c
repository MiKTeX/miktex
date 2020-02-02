char version[12] = "2019-01-08";

/*  Copyright (C) 2015-19 R. D. Tennent School of Computing,
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

/*  rebar - corrects bar numbers in a MusiXTeX score
 */

# include "utils.h"

# define SHORT_LEN 256

PRIVATE char infilename[SHORT_LEN];
PRIVATE char *infilename_n = infilename;
PRIVATE char outfilename[SHORT_LEN];
PRIVATE char *outfilename_n = outfilename;

PRIVATE FILE *infile, *outfile;


PRIVATE int barno = 0;
PRIVATE int startbarno = 1;

PRIVATE char line[LINE_LEN];    /* line of input                                        */

PRIVATE void
usage (FILE *f)
{
  fprintf (f, "Usage: rebar [-v | --version | -h | --help]\n");
  fprintf (f, "       rebar [ infile.tex  [outfile[.tex] ] ]\n");
  fprintf (f, "       rebar [ infile.aspc [outfile[.aspc]] ]\n");
}

PRIVATE void
error_at (const char msg[])      /* abort with stderr message msg                        */
{
  fprintf (stderr, "Error on line %d: %s\n", lineno, msg);
  exit (EXIT_FAILURE);
}


PRIVATE void
process_command (char **ln)
{ char *t;
  if ( prefix ("\\startbarno", *ln ) )
  {
    t = *ln + 11;
    if (*t == '=') t++;
    if ( sscanf (t, "%d", &startbarno) != 1) 
      error ("Expected startbarno");
    fprintf (outfile, "%s=%d", "\\startbarno", startbarno);
    t = strpbrk (t+1, "%\\\n");
    if (t == NULL) t = *ln + strlen (*ln);
    *ln = t;
  }
  else if ( prefix("\\startpiece", *ln) )
  { 
    barno = startbarno;
    fprintf (outfile, "%s%%%d\n", "\\startpiece", barno);
    t = strpbrk (*ln+1, "%\\\n");
    if (t == NULL) t = *ln + strlen (*ln);
    *ln = t;
  }
  else if ( prefix("\\startextract", *ln) )
  { 
    barno = startbarno;
    fprintf (outfile, "%s%%%d\n", "\\startextract", barno);
    t = strpbrk (*ln+1, "%\\\n");
    if (t == NULL) t = *ln + strlen (*ln);
    *ln = t;
  }
  else if ( prefix ("\\barno", *ln) )
  {
    t = *ln + 6;
     if (*t == '=') t++;
     if ( sscanf (t, "%d", &barno) != 1) 
       error ("Expected barno");
    t = strpbrk (*ln+6, "%\\\n");
    if (t == NULL) t = *ln + strlen (*ln);
    while (*ln < t)
    { fputc (**ln, outfile); 
      (*ln)++;
    }
  }
  else if ( prefix ("\\advance\\barno", *ln) )
  { 
    int barno_increment = 0;
    t = strpbrk (*ln, "-0123456789");
    if ( (*ln == NULL) || (sscanf (t, "%d", &barno_increment) != 1 ) )
      error ("Expected barno increment/decrement");
    barno = barno + barno_increment;
    t = strpbrk (*ln+13, "%\\\n");
    if (t == NULL) t = *ln + strlen (*ln);
    while (*ln < t)
    { fputc (**ln, outfile); 
      (*ln)++;
    }
  }
  else if ( prefix("\\bar", *ln) 
         || prefix("\\alaligne", *ln)
         || prefix("\\alapage", *ln)
         || prefix("\\changecontext", *ln)
         || prefix("\\Changecontext", *ln)
         || prefix("\\contpiece", *ln)
         || prefix("\\Contpiece", *ln)
         || prefix("\\xbar", *ln)  
         || prefix("\\leftrepeat", *ln)  
         || prefix("\\rightrepeat", *ln)  
         || prefix("\\leftrightrepeat", *ln)  )
  { 
    barno++;
    t = strpbrk (*ln+1, "%\\\n");
    if (t == NULL) t = *ln + strlen (*ln);
    while (*ln < t)
    {
      putc (**ln, outfile); 
      (*ln)++;
    }
    fprintf (outfile, "%%%d\n", barno);
    t = strpbrk (*ln+1, "\\\n");
    if (t != NULL && *t == '\n') 
      t++;   /* to prevent blank lines */
    else if (t == NULL) t = *ln + strlen (*ln); 
    *ln = t;
  }

  else  /* everything else */
  { t = strpbrk (*ln+1, "%\\");
    if (t == NULL || *t == '%') t = *ln + strlen (*ln);
    while (*ln < t) 
    { fputc (**ln, outfile); 
      (*ln)++;
    }
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


PRIVATE void
process_score (void)
/* process .tex file */
{
  int c;
  while ( (c = getc (infile)) != EOF )
  {
    ungetc (c, infile);
    lineno++;
    if ( fgets(line, LINE_LEN, infile) == NULL) 
      error_at("Unexpected EOF.");
    if (strlen (line) == LINE_LEN-1) 
      error_at("Line too long.");
    process_line (); 
  }     /* c == EOF  */
}     


int 
main (int argc, char *argv[])
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
  c = getopt_long (argc, argv, "hv", longopts, NULL);
  while (c != -1)
    {
      switch (c)
        {
        case 'h':
          fprintf (stdout, "This is rebar, version %s.\n", version);
          usage (stdout);
          fprintf (stdout, "Please report bugs to rdt@cs.queensu.ca.\n" );
          exit (0);
        case 'v':
          fprintf (stdout, "This is rebar, version %s.\n", version);
          exit (0);
        case '?':
          exit (EXIT_FAILURE);
        default:
          fprintf (stderr,"Function getopt returned character code 0%o.\n",
                  (unsigned int) c);
          exit (EXIT_FAILURE);
        }
      c = getopt_long (argc, argv, "hv", longopts, NULL);
    }
  fprintf (stderr, "This is rebar, version %s.\n", version);
  fprintf (stderr, "Copyright (C) 2015-16  R. D. Tennent\n" );
  fprintf (stderr, "School of Computing, Queen's University, rdt@cs.queensu.ca\n" );
  fprintf (stderr, "License GNU GPL version 2 or later <http://gnu.org/licences/gpl.html>.\n" );
  fprintf (stderr, "There is NO WARRANTY, to the extent permitted by law.\n\n" );

  if (optind < argc)
  { 
    infilename[0] = '\0';
    infilename_n = infilename;
    append (infilename, &infilename_n, argv[optind], sizeof (infilename));
    if ( !suffix (".aspc", infilename)  && !suffix (".tex", infilename) ) 
    { usage( stderr );
      exit (EXIT_FAILURE);
    }
    infile = fopen (infilename, "r");
    if (infile == NULL )
    { fprintf (stderr, "Can't open %s\n", infilename);
      exit (EXIT_FAILURE);
    }
    fprintf (stderr, "Reading from %s.", infilename);
  }
  else 
  { infile = stdin;
    fprintf (stderr, "Reading from stdin.");
  }

  optind++;
  if (optind < argc)  
  {
    outfilename[0] = '\0';
    outfilename_n = outfilename;
    append (outfilename, &outfilename_n, argv[optind], sizeof (outfilename));
    if (!suffix (".tex", outfilename) && !suffix (".aspc", outfilename) )
    { 
      if ( suffix (".tex", infilename) )
        append (outfilename, &outfilename_n, ".tex", sizeof (outfilename));
      else
        append (outfilename, &outfilename_n, ".aspc", sizeof (outfilename));
    }
    outfile = fopen (outfilename, "w");
    if (outfile == NULL)
    { fprintf (stderr,"Can't open %s\n", outfilename);
      exit (EXIT_FAILURE);
    }
    fprintf (stderr, " Writing to %s.\n\n", outfilename);
  }
  else 
  { outfile = stdout;
    fprintf (stderr, " Writing to stdout.\n\n");
  }

  process_score (); 

  return 0;
}
