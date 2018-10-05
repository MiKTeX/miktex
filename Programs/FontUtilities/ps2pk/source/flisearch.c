/*
 * FILE:    psearch.c
 * PURPOSE: search files through possible recursive path
 * VERSION: 1.0 (Oct. 1995)
 * AUTHOR:  Piet Tutelaers
 */

#include <stdio.h>
#include <string.h>		/* strcmp() */
#include "basics.h"		/* fatal(), msg() */
#include "filenames.h"		/* */
#include "psearch.h"		/* search_flipath() */

char *fontname; int dpi, margin = 0;

int main(int argc, char *argv[]) {
   char *path, *file, *fn;
   int matching(char *, int);

   /* get path and file arguments */
   if (argc != 4) fatal("Usage: psearch path fontname dpi\n");
   path = argv[1]; fontname = argv[2]; dpi = atoi(argv[3]);

   /* find <file> in <path> */
   fn = search_flipath(path, matching);
   if (fn == NULL) msg("%s at %d dpi not found\n", fontname, dpi);
   else msg("%s: %s at %d dpi found\n", fn, fontname, dpi);
   exit(0);
}

/*
 * Function used to search for one single font in FLI path
 */
int matching(char *font, int size) {

   if ((strcmp(font, fontname) == 0) && (size == dpi ||
      (size >= dpi - margin && size <= dpi+margin))) 
      return 1;
   else
      return 0;
}

