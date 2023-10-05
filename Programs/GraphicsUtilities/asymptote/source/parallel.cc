#include <iostream>
#include <cstdlib>
#include <cstdint>

#include "parallel.h"

#ifndef __Complex_h__
#include <complex>
typedef std::complex<double> Complex;
#endif

#include "statistics.h"
#include "align.h"
#include <chrono>

namespace utils {
int ALIGNMENT=2*sizeof(Complex); // Must be a multiple of sizeof(Complex)
}

using namespace utils;

size_t threshold=SIZE_MAX;

namespace parallel {

size_t lastThreads=SIZE_MAX;

const size_t maxThreshold=1 << 24;

size_t parallelLoop(Complex *A, size_t m, size_t threads)
{
  auto T0=std::chrono::steady_clock::now();
  PARALLEL(
    for(size_t k=0; k < m; ++k)
      A[k]=k;
    );
  PARALLEL(
    for(size_t k=0; k < m; ++k)
      A[k] *= k;
    );
  auto T1=std::chrono::steady_clock::now();

  auto elapsed=std::chrono::duration_cast<std::chrono::nanoseconds>
    (T1-T0);
  return elapsed.count();
}

size_t measureThreshold(size_t threads)
{
  if(threads > 1) {
    for(size_t m=1; m < maxThreshold; m *= 2) {
      Complex *A=ComplexAlign(m);
      if(!A)
        break;
      if(parallelLoop(A,m,threads) < parallelLoop(A,m,1))
        return m;
      deleteAlign(A);
    }
  }
  return maxThreshold;
}

void Threshold(size_t threads)
{
  if(threads > 1 && threads < lastThreads) {
    statistics S(true);
    for(size_t i=0; i < 10; ++i)
      S.add(measureThreshold(threads));
    threshold=S.median();
    lastThreads=threads;
  }
}

}
