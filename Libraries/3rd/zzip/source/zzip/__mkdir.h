#ifndef __ZZIP_INTERNAL_MKDIR_H
#define __ZZIP_INTERNAL_MKDIR_H

#include <zzip/conf.h>

#  ifdef ZZIP_HAVE_DIRECT_H
#  define _zzip_mkdir(a,b) mkdir(a)
#  else
#  define _zzip_mkdir      mkdir
#  endif

#endif
