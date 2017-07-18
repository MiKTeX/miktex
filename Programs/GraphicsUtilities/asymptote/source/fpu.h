#ifndef FPU_H
#define FPU_H

#ifdef HAVE_FENV_H
#ifdef _GNU_SOURCE
#define HAVE_FEENABLEEXCEPT
#endif
#endif

#ifdef HAVE_FEENABLEEXCEPT
#include <fenv.h>

inline int fpu_exceptions() {
  int excepts=0;
#ifdef FE_INVALID    
  excepts |= FE_INVALID;
#endif    
#ifdef FE_DIVBYZERO
  excepts |= FE_DIVBYZERO;
#endif  
#ifdef FE_OVERFLOW
  excepts |= FE_OVERFLOW;
#endif  
  return excepts;
}

inline void fpu_trap(bool trap=true)
{
  // Conditionally trap FPU exceptions on NaN, zero divide and overflow.
  if(trap) feenableexcept(fpu_exceptions());
  else fedisableexcept(fpu_exceptions());
}

#else

inline void fpu_trap(bool=true) {}

#endif

#endif
