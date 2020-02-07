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

extern bool debug;
extern bool dottedbeamnotes;          /* dotted beam notes ignored?        */

extern char infilename[SHORT_LEN];
extern char *infilename_n;
extern char outfilename[SHORT_LEN];
extern char *outfilename_n;
extern char logfilename[SHORT_LEN];
extern char *logfilename_n;
extern FILE *infile, *outfile, *logfile;


extern void process_score ();
