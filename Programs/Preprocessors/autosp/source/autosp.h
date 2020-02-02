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
