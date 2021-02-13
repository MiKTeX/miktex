/* wrapper.c -- Copyright (c) 2020 J. M. Spivey */
/*                             and Bob Tennent */

char xml2pmxVersion[12] = "2021-02-07";

#ifdef HAVE_CONFIG_H   /* for TeXLive */
#include <config.h>
#endif
#if defined(MIKTEX)
#include <miktex/Core/c/api.h>
#endif

# ifdef KPATHSEA
# include <kpathsea/getopt.h>
# else
# include <getopt.h>
# endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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


static void
usage ()
{
  printf ("Usage: xml2pmx [-v | --version | -h | --help]\n");
  printf ("       xml2pmx infile.xml outfile.pmx [p|d|v|s|l] .. \n");
}


int xmain(int argc, char **argv);

int main(int argc, char **argv) {
  int c;
# define NOPTS 3
  struct option longopts[NOPTS] =
  {  { "help", 0, NULL, 'h'},
     { "version", 0, NULL, 'v'},
     { NULL, 0, NULL, 0}
  };
  
  printf ("This is xml2pmx, version %s.\n", xml2pmxVersion);

  if (argc < 2) {
    usage();
    exit (0);
  }

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

  puts ( "Copyright (C) 2015-21  Dieter Gloetzel, Mike Spivey and Bob Tennent" );
  puts ( "License GNU GPL version 3 or later <http://gnu.org/licences/gpl.html>." );
  puts ( "There is NO WARRANTY, to the extent permitted by law." );
  puts ( "" );

  return xmain(argc, argv);
}
    
