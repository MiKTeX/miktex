/*
 * NAME
 *	pfb2pfa - convert a type1 pfb file (binary) into a pfa (ASCII) 
 * SYNOPSIS
 *	pfb2pfa [-v] pfbfile [pfafile]
 * DESCRIPTION
 *	Program converts a binary MSDOS representation for a type1 
 *	PostScript font into a readable ASCII version. The MSDOS 
 *	newline (\r) is converted into the UNIX newline (\n).
 *	The output is written in a file whose name is the name that
 *	is provided on the command line or the basename of the input
 *	file plus extension ".pfa".
 *
 *	With the -v option you get some information about what the 
 *	program is doing.
 * AUTHOR
 *	Piet Tutelaers
 */

#include "basics.h"	/* basic definitions and fatal() */
#include <stdarg.h>
#include <stdio.h>
#include "filenames.h"  /* newname() */

#if defined(MSDOS) || defined(DOSISH)
#define NEWLINE '\012'
#else
#define NEWLINE '\n'
#endif

#define HEX_PER_LINE 30

int main (int argc, char *argv[])
{  unsigned int t, l, i;
   unsigned int l1, l2, l3, l4; 
   short c, done, verbose = 0;
   FILE *pfb, *pfa;
   char *pfbname, *pfaname = NULL;
   const char *myname = "pfb2pfa";

   while (--argc > 0 && (*++argv)[0] == '-') {
      done=0;
      while ((!done) && (c = *++argv[0]))  /* allow -bcK like options */
      	 switch (c) {
      	 case 'v':
      	    verbose = 1; break;
      	 default:
      	    fatal("%s: %c invalid option\n", myname, c);
      	 }
      }

   if (argc < 1) {
     msg  ("pfb2pfa (ps2pk) version " PACKAGE_VERSION "\n");
     fatal("Usage: %s [-v] pfbfile [pfafile]\n", myname);
   }
   
   pfbname = argv[0]; argc--; argv++;
   if (argc < 1) pfaname = newname(pfbname, ".pfa");
   else if (argc == 1) pfaname = argv[0];
   else fatal("Usage: %s [-v] pfbfile [pfafile]\n", myname);

   pfb = fopen(pfbname, RB);
   if (pfb == NULL) fatal("Can't open %s\n", pfbname);
   
   pfa = fopen(pfaname, WB);
   if (pfa == NULL) fatal("Can't open %s\n", pfaname);
   
   while(!feof(pfb)) {
      if (getc(pfb) != 128)
	 fatal("%s: not a pfb file.\n", pfbname);
      t = getc(pfb);
      if (verbose) printf("Type: %d, ", t);
      switch (t) {
      case 1:
         l1 = getc(pfb); l2 = getc(pfb); l3 = getc(pfb); l4 = getc(pfb); 
         l = l1 | l2 << 8 | l3 << 16 | l4 << 24;
         /* printf("%2x %2x %2x %2x -> %x\n", l1, l2, l3, l4, l); */
         if (verbose) printf(" plain text, length %d\n", l);
	 for (i=0; i < l ; i++) {
            c = getc(pfb);
            if (c == '\r') putc(NEWLINE, pfa);
            else putc(c, pfa);
	  }
         break;
      case 2:
         l1 = getc(pfb); l2 = getc(pfb); l3 = getc(pfb); l4 = getc(pfb); 
         l = l1 | l2 << 8 | l3 << 16 | l4 << 24;
         /* printf("%2x %2x %2x %2x -> %x\n", l1, l2, l3, l4, l); */
         if (verbose) printf(" binary data, length %d\n", l);
         for(i = 0; i < l ;i++) {
	    fprintf(pfa, "%02x", getc(pfb));
	    if ((i+1) % HEX_PER_LINE == 0) putc(NEWLINE, pfa);
	 }
	 putc(NEWLINE, pfa);
         break;
      case 3:
         if (verbose) printf("End of file\n");
         exit(0);
         break;
      default:
	 fatal("Unknown field type: %d\n", t);
      }
   }
   fclose(pfa); fclose(pfb);
   return 0;
}
