/****
 * common.h
 *
 * Definitions common to all files.
 *****/

#ifndef COMMON_H
#define COMMON_H

#undef NDEBUG

#include <iostream>
#include <climits>

#ifdef __CYGWIN__
#undef LONG_LONG_MAX
#define LONG_LONG_MAX __LONG_LONG_MAX__
#undef LONG_LONG_MIN
#define LONG_LONG_MIN (-LONG_LONG_MAX-1)
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if !defined(FOR_SHARED) && defined(HAVE_LIBGLU) && \
  ((defined(HAVE_LIBGL) && defined(HAVE_LIBGLUT)) || defined(HAVE_LIBOSMESA))
#define HAVE_GL
#endif

#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif

#include "memory.h"

#if defined(MIKTEX) && defined(_MSC_VER)
#  define LONG_LONG_MAX LLONG_MAX
#  define LONG_LONG_MIN LLONG_MIN
#endif
#if defined(HAVE_LONG_LONG) && defined(LONG_LONG_MAX) && defined(LONG_LONG_MIN)
#define Int_MAX2 LONG_LONG_MAX
#define Int_MIN LONG_LONG_MIN
typedef long long Int;
typedef unsigned long long unsignedInt;
#else
#undef HAVE_LONG_LONG
#ifdef HAVE_LONG
#define Int_MAX2 LONG_MAX
#define Int_MIN LONG_MIN
typedef long Int;
typedef unsigned long unsignedInt;
#else
#define Int_MAX2 INT_MAX
#define Int_MIN INT_MIN
typedef int Int;
typedef unsigned int unsignedInt;
#endif
#endif

#ifndef COMPACT
#if Int_MAX2 >= 0x7fffffffffffffffLL
#define COMPACT 1
#else
#define COMPACT 0
#endif
#endif

#if COMPACT
// Reserve highest two values for DefaultValue and Undefined states.
#define Int_MAX (Int_MAX2-2)
#define int_MAX (LONG_MAX-2)
#else
#define Int_MAX Int_MAX2
#define int_MAX LONG_MAX
#endif

#define int_MIN LONG_MIN

#if !defined(MIKTEX_WINDOWS)
#define RANDOM_MAX 0x7FFFFFFF
#endif

using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::istream;
using std::ostream;

using mem::string;
using mem::stringstream;
using mem::istringstream;
using mem::ostringstream;
using mem::stringbuf;

#endif 
