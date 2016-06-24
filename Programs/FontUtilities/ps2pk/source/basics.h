/* FILE:    basics.h
 * PURPOSE: basic definitions
 * AUTHOR:  Piet Tutelaers
 * (see README for license)
 */

#ifdef HAVE_CONFIG_H
#include "c-auto.h"
#endif

#ifndef NOBASICS

#if defined(KPATHSEA)
#  include <kpathsea/config.h>
#  include <kpathsea/c-pathch.h>
#  include <kpathsea/c-fopen.h>
#  define PATHSEP ENV_SEP
#  define DIRSEP  DIR_SEP
#  define RB FOPEN_RBIN_MODE
#  define WB FOPEN_WBIN_MODE
#  if defined(DOSISH)
#    define ESCAPECHAR '!'
#    define RECURSIVE "!!"
#  endif /* DOSISH */
#  define basename ps2pk_basename
#elif defined(MSDOS) || defined(WIN32)
#  define PATHSEP ';'
#  define DIRSEP '\\'
#  define ESCAPECHAR '!'
#  define RECURSIVE "!!"
#  define PSRES_NAME	"psres.dpr"
#  define RB "rb"
#  define WB "wb"
#endif
 
#ifndef PATHSEP
#define PATHSEP ':'
#endif

#ifndef DIRSEP
#define DIRSEP '/'
#endif

#ifndef ESCAPECHAR
#define ESCAPECHAR '\\'
#endif

#ifndef RECURSIVE
#define RECURSIVE "//"
#endif

/* TeX PS Resource database name */
#ifndef PSRES_NAME
#define PSRES_NAME	"PSres.upr"
#endif

#ifndef RB
#define RB "r"
#endif

#ifndef WB
#define WB "w"
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif

#ifndef MAXSTRLEN
#define MAXSTRLEN 256
#endif

#ifndef IS_DIR_SEP
#define IS_DIR_SEP(c) ((c) == DIRSEP)
#endif

#define NOBASICS
#endif

#include <stdarg.h>

#if defined __GNUC__ && __GNUC__ >=3
__attribute__((__noreturn__))
#elif defined _MSC_VER && 1200 <= _MSC_VER
__declspec (noreturn)
#endif
void fatal(const char *fmt, ...);
void msg(const char *fmt, ...);

extern char *encfile, *afmfile;

/* For debugging purposes it is handy to have a fopen() function that
 * shows which files are opened.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  /* struct stat */
#include <sys/stat.h>   /* stat() */
