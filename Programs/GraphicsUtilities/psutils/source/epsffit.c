/* epsffit.c
 * Fit EPSF file into constrained size
 *
 * (c) Reuben Thomas 2012-2014
 * (c) Angus J. C. Duggan 1991-1997
 * See file LICENSE for details.
 *
 * Added filename spec (from Larry Weissman) 5 Feb 93
 * Accepts double %%BoundingBox input, outputs proper BB, 4 Jun 93. (I don't
 * like this; developers should read the Big Red Book before writing code which
 * outputs PostScript.
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "progname.h"
#include "binary-io.h"

#include "psutil.h"

const char *syntax = "[-c] [-r] [-a] [-m] [-s] LLX LLY URX URY [INFILE [OUTFILE]]\n";

const char *argerr_message = "";

int
main(int argc, char **argv)
{
   int bbfound = 0;              /* %%BoundingBox: found */
   int urx = 0, ury = 0, llx = 0, lly = 0;
   int furx, fury, fllx, flly;
   int showpage = 0, centre = 0, rotate = 0, aspect = 0, maximise = 0;
   char buf[BUFSIZ];
   FILE *input = stdin;
   FILE *output = stdout;
   int opt;

   set_program_name(argv[0]);

   while((opt = getopt(argc, argv, "csramv")) != EOF) {
     switch(opt) {
     case 'c': centre = 1; break;
     case 's': showpage = 1; break;
     case 'r': rotate = 1; break;
     case 'a': aspect = 1; break;
     case 'm': maximise = 1; break;
     case 'v':
     default:
       usage();
       break;
     }
   }

   if ((argc - optind) < 4 || (argc - optind) > 6) usage();

   fllx = atoi(argv[optind++]);
   flly = atoi(argv[optind++]);
   furx = atoi(argv[optind++]);
   fury = atoi(argv[optind++]);

   /* Be defensive */
   if((argc - optind) < 0 || (argc - optind) > 2) usage();

   if ((argc - optind) > 0) {
      if(!(input = fopen(argv[optind], "rb")))
	 die("can't open input file %s", argv[optind]);
      optind++;
   } else if (set_binary_mode(fileno(stdin), O_BINARY) < 0)
      die("can't reset stdin to binary mode");

   if ((argc - optind) > 0) {
      if(!(output = fopen(argv[optind], "wb")))
	 die("can't open output file %s", argv[optind]);
      optind++;
   } else if (set_binary_mode(fileno(stdout), O_BINARY) < 0)
      die("can't reset stdout to binary mode");

   while (fgets(buf, BUFSIZ, input)) {
      if (buf[0] == '%' && (buf[1] == '%' || buf[1] == '!')) {
	 /* still in comment section */
	 if (!strncmp(buf, "%%BoundingBox:", 14)) {
	    double illx, illy, iurx, iury;	/* input bbox parameters */
	    if (sscanf(buf, "%%%%BoundingBox:%lf %lf %lf %lf\n",
		       &illx, &illy, &iurx, &iury) == 4) {
	       bbfound = 1;
	       llx = (int)illx;	/* accept doubles, but convert to int */
	       lly = (int)illy;
	       urx = (int)(iurx+0.5);
	       ury = (int)(iury+0.5);
	    }
	 } else if (!strncmp(buf, "%%EndComments", 13)) {
	    strcpy(buf, "\n"); /* don't repeat %%EndComments */
	    break;
	 } else fputs(buf, output);
      } else break;
   }

   if (bbfound) { /* put BB, followed by scale&translate */
      int fwidth, fheight;
      double xscale, yscale;
      double xoffset = fllx, yoffset = flly;
      double width = urx-llx, height = ury-lly;

      if (maximise)
	 if ((width > height && fury-flly > furx-fllx) ||
	     (width < height && fury-flly < furx-fllx)) 
	    rotate = 1;

      if (rotate) {
	 fwidth = fury - flly;
	 fheight = furx - fllx;
      } else {
	 fwidth = furx - fllx;
	 fheight = fury - flly;
      }

      xscale = fwidth/width;
      yscale = fheight/height;

      if (!aspect) {       /* preserve aspect ratio ? */
	 xscale = yscale = MIN(xscale,yscale);
      }
      width *= xscale;     /* actual width and height after scaling */
      height *= yscale;
      if (centre) {
	 if (rotate) {
	    xoffset += (fheight - height)/2;
	    yoffset += (fwidth - width)/2;
	 } else {
	    xoffset += (fwidth - width)/2;
	    yoffset += (fheight - height)/2;
	 }
      }
      fprintf(output, 
	      "%%%%BoundingBox: %d %d %d %d\n", (int)xoffset, (int)yoffset,
	     (int)(xoffset+(rotate ? height : width)),
	     (int)(yoffset+(rotate ? width : height)));
      if (rotate) {  /* compensate for original image shift */
	 xoffset += height + lly * yscale;  /* displacement for rotation */
	 yoffset -= llx * xscale;
      } else {
	 xoffset -= llx * xscale;
	 yoffset -= lly * yscale;
      }
      fputs("%%EndComments\n", output);
      if (showpage)
	 fputs("save /showpage{}def /copypage{}def /erasepage{}def\n", output);
      else
	 fputs("%%BeginProcSet: epsffit 1 0\n", output);
      fputs("gsave\n", output);
      fprintf(output, "%.3f %.3f translate\n", xoffset, yoffset);
      if (rotate)
	 fputs("90 rotate\n", output);
      fprintf(output, "%.3f %.3f scale\n", xscale, yscale);
      if (!showpage)
	 fputs("%%EndProcSet\n", output);
   }
   do {
      fputs(buf, output);
   } while (fgets(buf, BUFSIZ, input));
   if (bbfound) {
      fputs("grestore\n", output);
      if (showpage)
	 fputs("restore showpage\n", output); /* just in case */
   } else
      die("no %%%%BoundingBox:");

   return 0;
}
