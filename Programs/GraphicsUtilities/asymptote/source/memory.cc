/**
 * @file memory.cc
 * @brief Implementation of certain gc-related functions
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "memory.h"
#include <memory>

// asy_malloc functions + GC_throw_bad_alloc

#if defined(USEGC)

void* asy_malloc(size_t n)
{
#ifdef GC_DEBUG
  if(void *mem=GC_debug_malloc_ignore_off_page(n, GC_EXTRAS))
#else
  if(void *mem=GC_malloc_ignore_off_page(n))
#endif
    return mem;
  throw std::bad_alloc();
}

void* asy_malloc_atomic(size_t n)
{
#ifdef GC_DEBUG
  if(void *mem=GC_debug_malloc_atomic_ignore_off_page(n, GC_EXTRAS))
#else
  if(void *mem=GC_malloc_atomic_ignore_off_page(n))
#endif
    return mem;
  throw std::bad_alloc();
}

#if !defined(_WIN32)
GC_API void GC_CALL GC_throw_bad_alloc()
{
  throw std::bad_alloc();
}
#endif

#endif // defined(USEGC)

// compact & stdString functions

namespace mem
{
#if defined(USEGC)

void compact(int x)
{
  GC_set_dont_expand(x);
}

std::string stdString(string s)
{
  return std::string(s.c_str());
}

#else // defined(USEGC)

std::string stdString(string s)
{
  return s;
}

#endif // defined(USEGC)
}

// throw bad alloc
