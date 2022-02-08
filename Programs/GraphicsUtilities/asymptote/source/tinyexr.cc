/**
* @file tinyexr.cc
* @brief An implementation object file for tinyexr mandated by the repository.
*
* On Windows, use vcpkg to install zlib instead of nuget. On Linux, this should work naturally
*/

#include <zlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define TINYEXR_IMPLEMENTATION
#define TINYEXR_USE_MINIZ 0

#ifdef HAVE_PTHREAD
#define TINYEXR_USE_THREAD 1
#else
#define TINYEXR_USE_THREAD 0
#endif

#ifndef HAVE_STRNLEN
#include <cstring>
#include <iostream>

inline size_t strnlen(const char *s, size_t maxlen)
{
  const char *p=(const char *) memchr(s,0,maxlen);
  return p ? p-s : maxlen;
}
#endif

#include "cudareflect/tinyexr/tinyexr.h"
