/* pstops.c
 * Rearrange pages in conforming PS file
 *
 * (c) Reuben Thomas 2012-2014
 * (c) Angus J. C. Duggan 1991-1997
 * See file LICENSE for details.
 */

#include "config.h"

#include <unistd.h>
#include <string.h>
#include "progname.h"
#include "binary-io.h"

#include "psutil.h"
#include "psspec.h"

const char *syntax = "[-q] [-b] [-wWIDTH] [-hHEIGHT] [-dLWIDTH] [-pPAPER] PAGESPECS [INFILE [OUTFILE]]\n";

const char *argerr_message = "%page specification error:\n"
  "  pagespecs = [modulo:]spec\n"
  "  spec      = [-]pageno[@scale][L|R|U|H|V][(xoff,yoff)][,spec|+spec]\n"
  "                modulo >= 1, 0 <= pageno < modulo\n";

static int modulo = 1;
static int pagesperspec = 1;

static PageSpec *parsespecs(char *str)
{
   PageSpec *head, *tail;
   unsigned long spec_count = 0;
   long num = -1;

   head = tail = newspec();
   while (*str) {
      if (isdigit((unsigned char)*str)) {
	 num = parseint(&str);
      } else {
	 switch (*str++) {
	 case ':':
	    if (spec_count || head != tail || num < 1) argerror();
	    modulo = num;
	    num = -1;
	    break;
	 case '-':
	    tail->flags ^= REVERSED;
	    break;
	 case '@':
	    tail->scale *= parsedouble(&str);
	    tail->flags |= SCALE;
	    break;
	 case 'l': case 'L':
	    tail->rotate += 90;
	    tail->flags |= ROTATE;
	    break;
	 case 'r': case 'R':
	    tail->rotate -= 90;
	    tail->flags |= ROTATE;
	    break;
	 case 'u': case 'U':
	    tail->rotate += 180;
	    tail->flags |= ROTATE;
	    break;
	 case 'h': case 'H':
	    tail->flags ^= HFLIP;
	    break;
	 case 'v': case 'V':
	    tail->flags ^= VFLIP;
	    break;
	 case '(':
	    tail->xoff += parsedimen(&str);
	    if (*str++ != ',') argerror();
	    tail->yoff += parsedimen(&str);
	    if (*str++ != ')') argerror();
	    tail->flags |= OFFSET;
	    break;
	 case '+':
	    tail->flags |= ADD_NEXT;
	 case ',':
	    if (num < 0 || num >= modulo) argerror();
	    if ((tail->flags & ADD_NEXT) == 0)
	       pagesperspec++;
	    tail->pageno = num;
	    tail->next = newspec();
	    tail = tail->next;
	    num = -1;
	    break;
	 default:
	    argerror();
	 }
	 spec_count++;
      }
   }
   if (num >= modulo)
      argerror();
   else if (num >= 0)
      tail->pageno = num;
   return (head);
}

int
main(int argc, char *argv[])
{
   PageSpec *specs = NULL;
   int nobinding = 0;
   double draw = 0;
   int opt;

   set_program_name (argv[0]);

   if (!paper_size(NULL, &width, &height))
     die("could not get default paper size");

   verbose = 1;

   while((opt = getopt(argc, argv, "qd::bw:h:p:v0123456789")) != EOF) {
     switch(opt) {
     case 'q':	/* quiet */
       verbose = 0;
       break;
     case 'd':	/* draw borders */
       if(optarg)
         draw = singledimen(optarg);
       else
         draw = 1;
       break;
     case 'b':	/* no bind operator */
       nobinding = 1;
       break;
     case 'w':	/* page width */
       width = singledimen(optarg);
       break;
     case 'h':	/* page height */
       height = singledimen(optarg);
       break;
     case 'p':	/* paper type */
       if (!paper_size(optarg, &width, &height))
         die("paper size '%s' not recognised", optarg);
       break;
     case 'v':	/* version */
       usage();
     case '0':
     case '1':
     case '2':
     case '3':
     case '4':
     case '5':
     case '6':
     case '7':
     case '8':
     case '9':
       if (specs == NULL) {
         char *spec_txt = malloc((optarg ? strlen(optarg) : 0) + 3);
         if(!spec_txt) die("no memory for spec allocation");
         spec_txt[0] = '-';
         spec_txt[1] = opt;
         spec_txt[2] = 0;
         if (optarg) strcat(spec_txt, optarg);
         specs = parsespecs(spec_txt);
         free(spec_txt);
       } else {
         usage();
       }
       break;
     default:
       usage();
       break;
     }
   }

   if (specs == NULL) {
     if(optind == argc) usage();
     specs = parsespecs(argv[optind]);
     optind++;
   }

   infile = stdin;
   outfile = stdout;

   /* Be defensive */
   if((argc - optind) < 0 || (argc - optind) > 2) usage();

   if (optind != argc) {
     /* User specified an input file */
     if ((infile = fopen(argv[optind], "rb")) == NULL)
       die("can't open input file %s", argv[optind]);
     optind++;
   }

   if (optind != argc) {
     /* User specified an output file */
     if ((outfile = fopen(argv[optind], "wb")) == NULL)
       die("can't open output file %s", argv[optind]);
     optind++;
   }

   if (optind != argc || specs == NULL) usage();

   if (infile == stdin && set_binary_mode(fileno(stdin), O_BINARY) < 0)
     die("can't reset stdin to binary mode");
   if (outfile == stdout && set_binary_mode(fileno(stdout), O_BINARY) < 0)
     die("can't reset stdout to binary mode");

   if ((infile=seekable(infile))==NULL)
      die("can't seek input");

   pstops(modulo, pagesperspec, nobinding, specs, draw);

   return 0;
}
