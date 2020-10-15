#ifndef __ZZIP_INTERNAL_FNMATCH_H
#define __ZZIP_INTERNAL_FNMATCH_H
/** included by fseeko.c, mmapped.c, memdisk.c */

#include <zzip/conf.h>
#include <stdio.h>
#include <string.h>

#ifdef ZZIP_HAVE_FNMATCH_H
# include <fnmatch.h>
# define _zzip_fnmatch fnmatch
# ifdef FNM_CASEFOLD
# define _zzip_FNM_CASEFOLD FNM_CASEFOLD
# else
# define _zzip_FNM_CASEFOLD 0
# endif
# ifdef FNM_NOESCAPE
# define _zzip_FNM_NOESCAPE FNM_NOESCAPE
# else
# define _zzip_FNM_NOESCAPE 0
# endif
# ifdef FNM_PATHNAME
# define _zzip_FNM_PATHNAME FNM_PATHNAME
# else
# define _zzip_FNM_PATHNAME 0
# endif
# ifdef FNM_PERIOD
# define _zzip_FNM_PERIOD FNM_PERIOD
# else
# define _zzip_FNM_PERIOD 0
# endif

#else
/* if your system does not have fnmatch, we fall back to strcmp: */
static int _zzip_fnmatch(char* pattern, char* string, int flags)
{ 
#   ifdef DBG2
    DBG1("<zzip:mmapped:strcmp>");
#   endif
    return strcmp (pattern, string); 
}

# define _zzip_FNM_CASEFOLD 0
# define _zzip_FNM_NOESCAPE 0
# define _zzip_FNM_PATHNAME 0
# define _zzip_FNM_PERIOD 0
#endif

#endif
