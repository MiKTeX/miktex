/* FILE:    filenames.c
 * PURPOSE: some handy functions for working with TeXfiles
 * AUTHOR:  Piet Tutelaers
 * (see README for license)
 */

#ifdef HAVE_CONFIG_H
#include "c-auto.h"
#endif

#ifdef KPATHSEA
#include <kpathsea/kpathsea.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "basics.h"	/* basic definitions and fatal() */
#include "filenames.h"

/* comparing names (system dependant) */
static int equal(const char *s, const char *t)
{
#  ifndef UNIX
   while (tolower(*s) == tolower(*t)) {
      if (*s == '\0')
         break;
      s++; t++;
   }
#  else
   while (*s == *t) {
      if (*s == '\0' || *t == '\0')
         break;
      s++; t++;
   }
#  endif
   if (*s == '\0' && *t == '\0')
      return 1;
   else
      return 0;
}

/*
 * Determine the extension
 */
char *extension(char *str) {
   char *p, *base;
   for (p = base = str; *p; ++p)
      /*      if (*p++ == DIRSEP) base = p; */
      if (IS_DIR_SEP(*p))
         base = p + 1;
   for (p = base ; *p; p++)
      if (*p == '.')
         break;
   return p;
}

/*
 * Construct filename by replacing the current extension (if available)
 * with supplied one.
 */
char *newname(char *name, const char *ext)
{  
   char *e, *nn; int len1, len2;

   e = extension(name);
   if (equal(ext, e))
      return name;
   len1 = strlen(name) - strlen(e);
   len2 = len1 + strlen(ext) + 1;
   nn = (char *) malloc(len2);
   if (nn == NULL)
      fatal("Out of memory\n");
   strncpy(nn, name, len1);
   strcpy(nn+len1, ext);
   return nn;
}

/*
 * Derived from BSD basename
 */
char *basename(char *str, const char *suffix){   
   char *p; int len = 0;
   const char *t;
   char *base;

   printf("basename of %s = ", str);
#ifdef KPATHSEA
   for (p = base = (NAME_BEGINS_WITH_DEVICE(str) ? str+2 : str);
	*p; p++) {
#else
   for (p = base = str; *p; p++) {
#endif
      /*      if (*p++ == DIRSEP) { base = p; len = 0; } */
      if (IS_DIR_SEP(*p)) {
         base = p+1;
         len = 0;
      } else len++;
   }
   printf("%s\n", base);
   if (suffix != NULL) {
      for (t = suffix; *t; ++t);
      do {
         len--; t--; p--;
         if (*t != *p) break;
         if (t == suffix) {
            char *bn;
            if (len == 0)
               return NULL;
            bn = malloc(len+1);
            if (bn == NULL)
               fatal("Out of memory\n");
            strncpy(bn, base, len);
            *(bn+len) = '\0'; /* RA */
            return bn;
         }
      } while (p > base);
   }
   return base;
}

/*
 * Return true if name can be the name of a PostScript resource
 * (no extension and no absolute pathname).
 */
int ps_resource(const char *name) {
   if (strchr(name, '.')) return 0 ;
#ifdef KPATHSEA
   if (kpse_absolute_p(name, true)) return 0;
#else
   if (strchr(name, DIRSEP)) return 0 ;
#endif
   return 1;
}
