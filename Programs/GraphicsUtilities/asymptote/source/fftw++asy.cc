#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LIBFFTW3
#include "fftw++.h"
#include "fftw++.cc"

extern "C" double __log_finite(double x) {return log(x);}

#endif
