#include <zconf.h>
#include <zlib.h>

#ifndef NOMINMAX
#  define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif


#ifdef min
#  undef min
#endif
#ifdef max
#  undef max
#endif
#ifdef near
#  undef near
#endif
#ifdef far
#  undef far
#endif

#include <Windows.h>