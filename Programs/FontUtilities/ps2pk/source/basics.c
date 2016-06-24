/* FILE:    basics.c
 * PURPOSE: basic functions
 * AUTHOR:  Piet Tutelaers
 * (see README for license)
 */

#include "basics.h"

/* Give up ... */
void fatal(const char *fmt, ...)
{  va_list args;

   va_start(args, fmt);
   vfprintf(stderr, fmt, args);
   va_end(args);
   exit(1);
}

/* Give a message ... */
void msg(const char *fmt, ...)
{  va_list args;

   va_start(args, fmt);
   vfprintf(stderr, fmt, args);
   fflush(stderr);
   va_end(args);
}
