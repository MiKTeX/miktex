/* psselect.c
 * Rearrange pages in conforming PS file for printing in signatures
 *
 * (c) Reuben Thomas 2012-2014
 * (c) Angus J. C. Duggan 1991-1997
 * See file LICENSE for details.
 */

#include "config.h"

#include <unistd.h>
#include "progname.h"
#include "binary-io.h"

#include "psutil.h"

const char *syntax = "[-q] [-e] [-o] [-r] [-pPAGES] [INFILE [OUTFILE]]\n";

const char *argerr_message = "";

typedef struct pgrange {
   int first, last;
   struct pgrange *next;
} PageRange ;

static PageRange *makerange(int beg, int end, PageRange *next)
{
   PageRange *new;
   if ((new = (PageRange *)malloc(sizeof(PageRange))) == NULL)
      die("out of memory");
   new->first = beg;
   new->last = end;
   new->next = next;
   return (new);
}


static PageRange *addrange(char *str, PageRange *rp)
{
   int first=0;
   int sign;

   if(!str) return NULL;

   sign = (*str == '_' && ++str) ? -1 : 1;
   if (isdigit((unsigned char)*str)) {
      first = sign*atoi(str);
      while (isdigit((unsigned char)*str)) str++;
   }
   switch (*str) {
   case '\0':
      if (first || sign < 0)
	 return (makerange(first, first, rp));
      break;
   case ',':
      if (first || sign < 0)
	 return (addrange(str+1, makerange(first, first, rp)));
      break;
   case '-':
   case ':':
      str++;
      sign = (*str == '_' && ++str) ? -1 : 1;
      if (!first)
	 first = 1;
      if (isdigit((unsigned char)*str)) {
	 int last = sign*atoi(str);
	 while (isdigit((unsigned char)*str)) str++;
	 if (*str == '\0')
	   return (makerange(first, last, rp));
	 if (*str == ',')
	   return (addrange(str+1, makerange(first, last, rp)));
      } else if (*str == '\0')
	 return (makerange(first, -1, rp));
      else if (*str == ',')
	 return (addrange(str+1, makerange(first, -1, rp)));
   default: /* Avoid a compiler warning */
     break;
   }
   die("invalid page range");
   return (PageRange *)0 ;
}


int
main(int argc, char *argv[])
{
   int opt;
   int currentpg, maxpage = 0;
   int even = 0, odd = 0, reverse = 0;
   int pass, all;
   PageRange *pagerange = NULL;

   set_program_name (argv[0]);

   verbose = 1;

   while((opt = getopt(argc, argv, "eorqvp:")) != EOF) {
     switch(opt) {
     case 'e':	/* even pages */
       even = 1;
       break;
     case 'o':	/* odd pages */
       odd = 1;
       break;
     case 'r':	/* reverse */
       reverse = 1;
       break;
     case 'p':	/* page spec */
       pagerange = addrange(optarg, pagerange);
       break;
     case 'q':	/* quiet */
       verbose = 0;
       break;
     case 'v':	/* version */
     default:
       usage();
       break;
     }
   }

   infile = stdin;
   outfile = stdout;

   /* If we haven't gotten a page range yet, we better get one now */
   if (pagerange == NULL && !reverse && !even && !odd) {
     pagerange = addrange(argv[optind], NULL);
     optind++;
   }

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

   if(optind != argc) usage();

   if (infile == stdin && set_binary_mode(fileno(stdin), O_BINARY) < 0)
      die("can't reset stdin to binary mode");
   if (outfile == stdout && set_binary_mode(fileno(stdout), O_BINARY) < 0)
      die("can't reset stdout to binary mode");

   if ((infile=seekable(infile))==NULL)
      die("can't seek input");

   scanpages(NULL);

   /* select all pages or all in range if odd or even not set */
   all = !(odd || even);

   /* add default page range */
   if (!pagerange)
      pagerange = makerange(1, -1, NULL);

   /* reverse page list if not reversing pages (list constructed bottom up) */
   if (!reverse) {
      PageRange *revlist = NULL;
      PageRange *next = NULL;
      while (pagerange) {
	 next = pagerange->next;
	 pagerange->next = revlist;
	 revlist = pagerange;
	 pagerange = next;
      }
      pagerange = revlist;
   } else { /* swap start & end if reversing */
      PageRange *r;
      for (r = pagerange; r; r = r->next) {
         int temp = r->last;
         r->last = r->first;
         r->first = temp;
      }
   }

   { /* adjust for end-relative pageranges */
      PageRange *r;
      for (r = pagerange; r; r = r->next) {
	 if (r->first < 0) {
	    r->first += pages + 1;
	    if (r->first < 1)
	       r->first = 1;
	 }
	 if (r->last < 0) {
	    r->last += pages + 1;
	    if (r->last < 1)
	       r->last = 1;
	 }
      }
   }

   /* count pages on first pass, select pages on second pass */
   for (pass = 0; pass < 2; pass++) {
      PageRange *r;
      if (pass) {                           /* write header on second pass */
	 writeheader(maxpage, NULL);
	 writeprolog();
	 writesetup();
      }
      for (r = pagerange; r; r = r->next) {
	 if (r->last < r->first) {
	    for (currentpg = r->first; currentpg >= r->last; currentpg--) {
	       if (currentpg == 0 ||
		   (currentpg <= pages &&
		    ((currentpg&1) ? (odd || all) : (even || all)))) {
		  if (pass) {
		     if (currentpg)
		        writepage(currentpg-1);
		     else
		        writeemptypage() ;
		  } else
		     maxpage++;
	       }
	    }
	 } else {
	    for (currentpg = r->first; currentpg <= r->last; currentpg++) {
	       if (currentpg == 0 ||
		   (currentpg <= pages &&
		    ((currentpg&1) ? (odd || all) : (even || all)))) {
		  if (pass) {
		     if (currentpg)
		        writepage(currentpg-1);
		     else
		        writeemptypage() ;
		  } else
		     maxpage++;
	       }
	    }
	 }
      }
   }
   writetrailer();

   return 0;
}
