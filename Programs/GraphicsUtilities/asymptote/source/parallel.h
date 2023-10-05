#pragma once

#ifndef _OPENMP
#ifndef SINGLE_THREAD
#define SINGLE_THREAD
#endif
#endif

#ifndef SINGLE_THREAD
#include <omp.h>
#endif

extern size_t threshold;

namespace parallel {
extern size_t lastThreads;
inline int get_thread_num()
{
#ifdef SINGLE_THREAD
  return 0;
#else
  return omp_get_thread_num();
#endif
}

inline int get_max_threads()
{
#ifdef SINGLE_THREAD
  return 1;
#else
  return omp_get_max_threads();
#endif
}
}

#ifndef SINGLE_THREAD
#define PARALLEL(code)                                  \
  if(threads > 1) {                                     \
    _Pragma("omp parallel for num_threads(threads)")    \
      code                                              \
      } else {code}
#else
#define PARALLEL(code) {code}
#endif

#ifndef SINGLE_THREAD
#define OMPIF(condition,directive,code)    \
  if(threads > 1 && condition) {             \
    _Pragma(directive)                             \
      code                                   \
      } else {code}
#else
#define OMPIF(condition,directive,code) {code}
#endif

#define PARALLELIF(condition,code) \
  OMPIF(condition,"omp parallel for num_threads(threads)",code)

namespace parallel {

void Threshold(size_t threads);

}
