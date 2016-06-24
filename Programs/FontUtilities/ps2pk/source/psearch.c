/* FILE:    psearch.c
 * PURPOSE: PATH search module
 * VERSION: 1.7 (December 2014)
 * AUTHOR:  Piet Tutelaers
 * (see README for license)
 */

#define _POSIX_SOURCE 1
#ifdef ultrix
#define __POSIX		/* otherwise MAXNAMLEN undefined */
#endif

#include <sys/types.h>
#include <sys/stat.h>   /* struct stat */
#include <dirent.h>	/* opendir(), readdir(), closedir() */
#include "ctype.h"	/* isalpha() */
#include <stdio.h>	/* sprintf(), fclose() */
#include "stdlib.h"	/* getenv(), malloc() */
#include <stdarg.h>	/* va_start(), va_arg(), va_end() */
#include "string.h"	/* strcat(), strchr(), strcmp(), strcpy(), strlen(),
			   strncmp(), strncpy(), strstr() */
#include "basics.h"	/* fatal() */
#include "strexpr.h"	/* strexpr() */
#include "filenames.h"	/* equal(), extension() */
#include "texfiles.h"	/* four(), one(), two() */

#ifdef UNIX
#include <pwd.h>	/* getpwnam() */

int  tilde = 1;    		/* Substitute ~ and ~user */
static char *home = NULL;

/*
 * During scanning of paths we often need to replace ~user with user's
 * HOME directory and ~ with HOME directory. Subtilde() copies the actual
 * value of `~' or `~user' from <path> into <fn>. The pointers <fn> and
 * <path> are `updated'.
 */
void subtilde(char **fn, char **path)
{  char pattern[32], *p; int i;

   i = 0; p = *path;
   while (*p != '\0' && *p != DIRSEP && *p != PATHSEP)
      pattern[i++]  = *p++ ;
   pattern[i] = '\0' ;
   if (pattern[0] != '~') return;
   if (pattern[1] == '\0') {
      if (home == NULL) 
         home = getenv("HOME");
      if (home == NULL) fatal("You don't have a HOME!\n");
      sprintf(*fn, "%s\0", home);
      *fn += strlen(*fn);
      *path = p;
   }
   else {
      struct passwd *pw;
      if (pw = getpwnam(&pattern[1])) {
         sprintf(*fn, "%s\0", pw->pw_dir);
         *fn += strlen(*fn); *path = p;
      }
      else
         fatal("%s: no such user\n", &pattern[1]) ;
   }
}
#else
static int  tilde = 0;    /* Don't substitute ~ and ~user */
static void subtilde(char **fn, char **path) {}
#endif

/*
 * Return a path constructed by <env> and <deFault>. If <env> is empty
 * the returned path equals <deFault>, otherwise the returned path is
 * <env> in which empty path elements are replaced by <deFault>. Double
 * path elements are NOT removed.
 */
char *path(char *deFault, char *env) {
   char *e, *p, *q; int len, left = MAXPATHLEN;

   if (env == NULL) return deFault;

   p = (char *) malloc(MAXPATHLEN);
   if (p == NULL) fatal("Out of memory\n");

   q = p; e = env;
   while (*e) {
      if (*e == PATHSEP) {
         if (left == MAXPATHLEN) {
	    len = strlen(deFault);
            if (len > left-1) fatal("'%s' path too long!\n", env);
	    strcpy(q, deFault);
	    q += len; left -= len;
	    *q++ = *e++; left--;
	 }
	 else if (*(e+1) == PATHSEP || *(e+1) == '\0') {
	    *q++ = *e++; left--;
            len = strlen(deFault);
	    if (len > left) fatal("'%s' path too long!\n", env);     
            strcpy(q, deFault);
	    q += len; left -= len;
	 }
	 else {
	   *q++ = *e++; left--;
	 }
      }
      else {
         *q++ = *e++; left--;
      }
      if (left == 0) fatal("'%s' path too long!\n", env);
   }
   *q = '\0';
   return p;
}

/*
 * Rsearch() searches <file> through <path>. Recursion introduced by
 * RECURSIVE string (system dependant) will be followed. Search stops
 * after first match.
 */
static char *rsearch(char *path, char *file)
{
   struct stat status;
   DIR *D;
   struct dirent *de;
   int i;
   char *res = NULL, *ext, *file_ext,
        head[MAXPATHLEN], target[MAXPATHLEN], *tail;
   char fn[MAXPATHLEN];

   /* determine head and tail of path */
   i = 0;
   while (*(path+i) != '\0' && strncmp(path+i, RECURSIVE, 2) != 0) {
      head[i] = *(path+i); i++;
   }
   head[i] = '\0';
   if (*(path+i) == '\0') tail = NULL;
   else tail = path+i+2;

   /* Check if we have found <file> */
   if (tail == NULL || *tail == '\0') {
      sprintf(fn, "%s%c%s", head, DIRSEP, file);
      if (stat(fn, &status) == 0) {
	 res = malloc(strlen(fn)+1);
	 if (res == NULL) fatal("Out of memory\n");
	 strcpy(res, fn);
	 return res;
      }
      /* No recursion, so stop */
      if (tail == NULL) return NULL;
   }

   if ((D = opendir(head)) == NULL) return NULL;
   file_ext = extension(file);

   /* now read directories and probe files */
   while ((de = readdir(D)) != NULL) {
      /* Ignore `.' and `..' directories */
      if (strcmp(de->d_name, ".") == 0) continue;
      if (strcmp(de->d_name, "..") == 0) continue;

      /* Check if we have a directory */
      sprintf(fn, "%s%c%s", head, DIRSEP, de->d_name);
      (void) stat(fn, &status);
      if (!S_ISDIR(status.st_mode)) {
         /* if we are looking for a file with an extension and we find
          * files with the same or a different extension we may consider
          * this directory as a wrong place unless the user has defined
          * funny directory search paths
          */
         if (file_ext && (ext = extension(de->d_name))) {
            if (equal(ext, ".pk")) break;
            if (equal(ext, ".vf")) break;
            if (equal(ext, ".mf")) break;
            if (equal(ext, ".tfm")) break;
            if (equal(ext, ".afm")) break;
         }
         continue;
      }

      /* Directories with no real subdirectories can be skipped
       * unless the file we are looking for is there. Unfortunately
       * this shortcut only works for UNIX systems. On other systems
       * the above check on extensions may help.
       */
#if defined(UNIX)
      if (status.st_nlink == 2) {
	 if (strchr(file, DIRSEP)) continue;
	 if ((*tail == '\0') && (res = rsearch(fn, file))) break;
	 continue;
      }
#endif

      /* path of the form "/aap/noot/mies//" */
      if (*tail == '\0') {
         sprintf(fn, "%s%c%s%s", head, DIRSEP, de->d_name, RECURSIVE);
	 if (res = rsearch(fn, file)) break;
	 continue;
      }
      
      /* path of the form "/aap/noot//mies" */

      /* search /aap/noot/<d_name>// <tail>/<file> */
      sprintf(fn, "%s%c%s%s", head, DIRSEP, de->d_name, RECURSIVE);
      sprintf(target, "%s%c%s", tail, DIRSEP, file);
      if (res = rsearch(fn, target)) break;
   }
   closedir(D);
   return res;
}

/* Is <name> an absolute filename? */
int absname(char *name) {

   if (*name == DIRSEP) return 1;
#ifdef MSDOS
   if (*(name+1) == ':' && *(name+2) == DIRSEP && isalpha(*name)) return 1;
#endif
   return 0;
}

/*
 * Look for a <file> in <path>. Return absolute filename if found.
 * If not found return NULL (if <terminate> is 0) or terminate program
 * with appropriate message (if <terminate> is not equal to 0).
 */
char *search_file(char *path, char *file, int terminate) {
   char *fn, pe[MAXPATHLEN], *ppe, *ppath;
   struct stat status;

   if (path == NULL || *path == '\0' || absname(file)) {
      if (stat(file, &status) == 0) return file;
      if (terminate) fatal("File <%s> not found\n", file);
      return NULL;
   }
   ppath = path;
   while (*ppath) {
      /* copy next pathelement into pe */
      ppe = pe;
      while (*ppath != PATHSEP && *ppath != '\0') {
         if (tilde && *ppath == '~')
            subtilde(&ppe, &ppath);
         else
            *ppe++ = *ppath++;
      }
      *ppe = '\0';
      if (*ppath == PATHSEP) ppath++;
      if (fn = rsearch(pe, file)) return fn;
   }
   if (terminate) fatal("File <%s> not found in path \"%s\"\n", file, path);
   return NULL;
}

/*
 * Now some functions for PK font search
 */

/*
 * replace <pattern> in <cs> by <value>. If the <pattern> is %b or %d
 * the value will be converted to a decimal value.
 */
static void replace(char *cs, char *pattern, void *value) {
   char temp[MAXPATHLEN], *p, *q; int len;

   while (q = strstr(cs, pattern)) {
      strcpy(temp, q+strlen(pattern));
      switch (*(pattern+1)) {
      case 'b':
      case 'd':
         sprintf(q, "%d", *((int *)value));
         len = strlen(q);
         break;
      case 'F':
         sprintf(q, "%-8s", (char *) value);
         len = strlen (q);
         break;
      case 'f':
         sprintf(q, "%s", (char *) value);
         len = strlen (q);
         break;
      default:
         sprintf(q, "%s", (char *) value);
         len = strlen (q);
         break;
      }
      strcpy(q+len, temp);
   }
}

/*
 * This function substitutes the <patterns> encountered in <str>
 * with their corresponding <value>. Recognized patterns are %b, %d,
 * %f, %F and %m. They will be replaced with their corresponding integer
 * (%b, %d) or string values (%f, %F and %m). These values can be used as
 * placeholders for basic resolution (%b), resolution (%d), font name
 * (%f or %F) or mode definition (%m). The %F is a special case for
 * representing the fontname leftadjusted in a string of eight characters.
 */

void substitute(char *str, char *patterns, ...) {
   va_list ap;
   char *p;
   char *svalue; int ivalue;

   /* get arguments ... */
   va_start(ap, patterns);
   for (p = patterns; *p; p++) {
      if (*p != '%') {
	 fatal("%s: invalid format\n");
      }
      switch (*++p) {
      case 'b':
         ivalue = va_arg(ap, int);
	 replace(str, "%b", &ivalue);
	 break;
      case 'd':
         ivalue = va_arg(ap, int);
	 replace(str, "%d", &ivalue);
	 break;
      case 'F':
         svalue = va_arg(ap, char *);
	 replace(str, "%F", svalue);
	 break;
      case 'f':
         svalue = va_arg(ap, char *);
	 replace(str, "%f", svalue);
	 break;
      case 'm':
         svalue = va_arg(ap, char *);
	 replace(str, "%m", svalue);
	 break;
      case 'p':
         svalue = va_arg(ap, char *);
	 replace(str, "%p", svalue);
	 break;
      default:
	 fatal("%s: invalid letter (%c) in format\n", patterns, *p);
      }
   }
   va_end(ap);
}

/* global font searching parameters */
static char *_mode = NULL;
static int _margin = 0, _bdpi = -1;
static char *_tolerance = NULL;

void init_pksearch(int bdpi, char *mode, int margin, char *tolerance) {
   _bdpi = bdpi;
   _mode = mode;
   _margin = margin;
   _tolerance = tolerance;
}

/* evaluate <expr> after substituting %d with <dpi>. Return the 
 * integer result or abort with message when invalid expression.
 */
int evaluate(char *expr, int dpi) {
   char expression[128];
   int result;

   if (expr == NULL || *expr == '\0') return 0;
   strcpy(expression, expr);
   substitute(expression, "%d", dpi);
   if (strexpr(&result, expression))
      fatal("%s: expression error\n", expression);
   return result;
}

#define INT(x) ((int)(x+0.5))

/*
 * Round <dpi> to a whole or half magstep if the difference is within
 * a certain _tolerance from it.
 */
int nearesthalf(int dpi) {
   double near = _bdpi, half = 1.095445115; int tolerance;

   tolerance = evaluate(_tolerance, dpi);
   if (tolerance == 0 || dpi == _bdpi) return dpi;
   if (dpi > INT(near)+tolerance)
      while (dpi > INT(near)+tolerance) near*=half;
   else if (dpi < INT(near)-tolerance)
      while (dpi < INT(near)-tolerance) near/=half;
   if (dpi >= INT(near)-tolerance && dpi <= INT(near)+tolerance)
      return INT(near);
   return dpi;
}

/*
 * Look for a PK <font> of resolution <dpi> in <path>. To avoid floating
 * around lots of different font sizes TeX prefers to work with whole
 * and half magsteps. So instead of using a 330 dpi font for a 11 point
 * version of a 300 dpi font we would prefer the 329 dpi size because
 * this size matches magstep 0.5 of 300 (329 = int(300 * 1.2 ** 0.5).
 * The process of truncating real font sizes to magsteps can be
 * controlled by the global parameter <tolerance> which may be an
 * expression of the actual value of <dpi> itself. The value dvips(1)
 * uses for <tolerance> is 1 + <dpi> / 500 allowing more tolerance in
 * bigger sizes. Due to round-off different DVI drivers choose different
 * <dpi> sizes. With <margin> we can define what font sizes are looked
 * at whenever size <dpi> is wanted. With <margin> set to zero only the
 * exact size is looked at, while a value of one would allow for a
 * round-off of one. Both <margin> and <tolerance> can be set before a
 * search starts otherwise they get their default values (zero) with
 * init_pksearch().
 *
 * When <path> does contain placeholders like %b, %d, %f, %p and %m
 * they will be substituted by their corresponding values _bdpi, <dpi>,
 * <font>, "pk" and _mode respectivally. This allows to define paths 
 * for a whole range of printers. Occurrences off ~ or * ~user in <path>
 * will be substituted by the corresponding HOME directories on UNIX.
 * 
 * The result of search_pkfile() is a pointer to the name of <font> when
 * one is found otherwise NULL.
 */
char *search_pkfile(char *path, char *texfont, int dpi) {
   char *ppe, *pkpath, pe[MAXPATHLEN];
   int del;
   char *pkname;
   struct stat status;

   for (del=0; del <= _margin ; del=del>0?-del:-del+1) {
      pkpath = path;
      while (*pkpath) {
         /* copy next pathelement into pe */
         ppe = pe;
         while (*pkpath != PATHSEP && *pkpath != '\0') {
            if (tilde && *pkpath == '~')
               subtilde(&ppe, &pkpath);
            else
               *ppe++ = *pkpath++;
         }
         *ppe = '\0';
         if (strchr(pe, '%')) {
            if (strstr(pe, "%d") && strstr(pe, "%f")) {
               /* try filename after replacing placeholders */
               substitute(pe, "%b%m%f%d%p",
		  _bdpi, _mode, texfont, dpi+del, "pk");
	       if (stat(pe, &status) == 0) {
                  pkname = malloc(strlen(pe)+1);
                  if (pkname == NULL) fatal("Out of memory\n");
                  strcpy(pkname, pe);
                  return pkname;
               }
            }
            else /* %f and %d are required! */
               fatal("%s: incomplete path element!\n", pe);
         }
         else {
	    sprintf(ppe, "%c%s.%dpk", DIRSEP, texfont, dpi+del);
            if (stat(pe, &status) == 0) {
               pkname = malloc(strlen(pe)+1);
               if (pkname == NULL) fatal("Out of memory\n");
               strcpy(pkname, pe);
               return pkname;
            }
         }
         if (*pkpath == PATHSEP) pkpath++;
      }
   }
   return NULL;
}

/*
 * Functions for handling emTeX's FLI files.
 */

/*
 * Look at the fonts in the font library by calling the function <font>.
 * This function can stop the search by returning a 1 or continue by
 * returning a zero. In this way we can look for just one font or for
 * a whole series of fonts. If the search is stopped the name of the 
 * current FLI file is returned otherwise a NULL pointer.
 */
char *search_flifile(char *fliname, int (*font)(char *, int)) {
   unsigned short len, numsizes, numfonts, version1, version2;
   char fontname[50];
   int i, size;

   FILE *FLI;

   FLI = fopen(fliname, RB);
   if (FLI == NULL) return NULL;

   for (i=0; i<4; i++) {
      fontname[i] = one(FLI);  /* read header */
   }
   version1 = one(FLI);
   version2 = one(FLI);
   if (strncmp(fontname,"FLIB",4)!=0 || version1 != 2 || version2 != 0)
      fatal("incorrect font library format");

   (void) two(FLI);        /* ignore directory length */
   numsizes = two(FLI);    /* number of sizes */
   numfonts = two(FLI);    /* number of fonts */
   len = two(FLI);         /* length of comment */
   for (i=0; i<len; i++)
      (void)one(FLI);      /* skip comment */

   for ( ;numsizes>0; numsizes--) { 
      /* for each font size in this library */
      (void)two(FLI);      /* length of size entry - ignore */
      numfonts = two(FLI); /* number of fonts */
			   /* DPI (fixed point 16.16) */
      size = (int)(four(FLI)/65536.0+0.5);
 
      for ( ;numfonts > 0; numfonts--) {
         /* read each entry */
         (void)four(FLI);           /* ignore length of font */
         (void)four(FLI);           /* offset to font */
         len = one(FLI);            /* length of name */
         for (i=0; i<len; i++)
            fontname[i] = one(FLI);
         fontname[len] = '\0';
	 if (font(fontname, size) == 1) {
            fclose(FLI);
	    return fliname;
	 };
      }
   }
   fclose(FLI);
   return NULL;
}

/*
 * Look for font libraries in <path> (ex.: c:\emtex\texfonts;fli_0;fli_1).
 * Run <font()> function for each .fli file found in <path>. Continue
 * if this function returns NULL, stop otherwise.
 */
char *search_flipath(char *flipath, int (*font)(char *, int)) {
   char fn[MAXPATHLEN], *pfn, *pfli, *fliname;
   int len;

   if (flipath == NULL) fatal("Invalid fli path\n");

   /* treat first element of flipath as a directory */
   pfn = fn;
   while (*flipath != PATHSEP && *flipath != '\0') {
      *pfn++ = *flipath++;
   }
   *pfn = '\0'; len = strlen(fn);
   if (len >= 1 && fn[len-1] != DIRSEP) *pfn++ = DIRSEP;

   /* and next elements as fli files */
   while (*flipath != '\0') {
      /* copy next pathelement into fn */
      pfli = pfn; flipath++;
      while (*flipath != PATHSEP && *flipath != '\0') {
         *pfli++ = *flipath++;
      }
      *pfli = '\0'; len = strlen(pfn);
      if (len > 4 && strcmp(pfli-4, ".fli") != 0)
         strcat(pfli, ".fli");
      if ((fliname = search_flifile(fn, font)) != NULL)
	 return fliname;
   }
   return NULL;
}
