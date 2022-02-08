/****
 * common.h
 *
 * Definitions common to all files.
 *****/

#ifndef COMMON_H
#define COMMON_H

#undef NDEBUG

#include <iostream>
#include <memory>
#include <climits>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LSP
#include <boost/optional.hpp>
#include <boost/none.hpp>
using boost::optional;
using boost::make_optional;
#else
#include "optional.hpp"
#define boost nonstd
using nonstd::optional;
using nonstd::nullopt;
#endif

using std::make_pair;

#if !defined(FOR_SHARED) &&                                             \
  ((defined(HAVE_LIBGL) && defined(HAVE_LIBGLUT) && defined(HAVE_LIBGLM)) || \
   defined(HAVE_LIBOSMESA))
#define HAVE_GL
#endif

#if defined(HAVE_LIBREADLINE) || defined(HAVE_LIBEDIT)
#define HAVE_READLINE
#endif

#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif

#include "memory.h"

#if defined(HAVE_LONG_LONG) && defined(LLONG_MAX) && defined(LLONG_MIN)
#define Int_MAX2 LLONG_MAX
#define Int_MIN LLONG_MIN
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

#ifndef RANDOM_MAX
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

using std::shared_ptr;
using std::unique_ptr;
using std::make_shared;

static const struct ws_t {} ws={};

// Portable way of skipping whitespace
inline std::istream &operator >> (std::istream & s, const ws_t &ws) {
  if(!s.eof())
    s >> std::ws;
  return s;
}

#endif
