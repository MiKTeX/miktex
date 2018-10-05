/*
 * A simple memset() in case your ANSI C does not provide it
 */

#include "c-auto.h"

#if !defined(HAVE_MEMSET) && !defined(memset)
memset(void *s, int c, int length)
{  char *p = s;
  
   while (length--) *(p++) = c;
}
#endif
