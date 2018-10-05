/*
 * FILE:    basename.c
 * PURPOSE: a reimplementation of BSD's basename so that it can also be
 *          used as a function in other programs (see filenames.c and
 *          filenames.h)
 * AUTHOR:  Piet Tutelaers
 * VERSION: 1.0 (Sept. 1995)
 */
#include <stdio.h>	/* printf() */
#include "basics.h"	/* fatal() */
#include "filenames.h"	/* basename() */

main(int argc, char **argv)
{
   if (argc < 2 || argc > 3) 
      fatal("Usage: basename string [suffix]\n");

   if (argc == 2) 
      printf("%s\n", basename(argv[1], NULL));
   else
      printf("%s\n", basename(argv[1], argv[2]));
   exit(0);
}
