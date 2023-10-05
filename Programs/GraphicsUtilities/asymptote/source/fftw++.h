/* Fast Fourier transform C++ header class for the FFTW3 Library
   Copyright (C) 2004-2022
   John C. Bowman, University of Alberta
   Malcolm Roberts, University of Strasbourg

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

#ifndef __fftwpp_h__
#define __fftwpp_h__ 1

#define __FFTWPP_H_VERSION__ 2.11

#if defined(MIKTEX)
#include <miktex/ExitThrows>
#endif
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <fftw3.h>
#include <cerrno>
#include <map>
#include <typeinfo>
#include <climits>

#include "seconds.h"
#include "parallel.h"

#ifndef __Complex_h__
#include <complex>
typedef std::complex<double> Complex;
#endif

#include "statistics.h"
#include "align.h"

namespace fftwpp {

// Obsolete names:
#define FFTWComplex ComplexAlign
#define FFTWdouble doubleAlign
#define FFTWdelete deleteAlign

// Return the memory alignment used by FFTW.
// Use of this function requires applying patches/fftw-3.3.8-alignment.patch
// to the FFTW source, recompiling, and reinstalling the FFW library.
extern "C" size_t fftw_alignment();

class fftw;

extern "C" fftw_plan Planner(fftw *F, Complex *in, Complex *out);
void loadWisdom();
void saveWisdom();
extern std::string wisdomName;

extern const char *inout;

class ThreadBase
{
public:
  size_t threads;
  size_t innerthreads;
  ThreadBase();
  ThreadBase(size_t threads) : threads(threads) {}
  void Threads(size_t nthreads) {threads=nthreads;}
  size_t Threads() {return threads;}
  size_t Innerthreads() {return innerthreads;}

  void multithread(size_t n) {
    if(n >= threads) {
      innerthreads=1;
    } else {
      innerthreads=threads;
      threads=1;
    }
  }

  int get_thread_num0() {
    return threads > 1 ? parallel::get_thread_num() : 0;
  }
};

inline size_t realsize(size_t n, bool inplace)
{
  return inplace ? 2*(n/2+1) : n;
}

inline size_t Inplace(Complex *in, Complex *out=NULL)
{
  return !out || in == out;
}

inline size_t Inplace(Complex *in, double *out)
{
  return Inplace(in,(Complex *) out);
}

inline size_t Inplace(double *in, Complex *out)
{
  return Inplace((Complex *) in,out);
}

class Doubles {
public:
  size_t rsize,csize;

  Doubles(size_t nx, size_t M,
          size_t istride, size_t ostride,
          size_t idist, size_t odist, bool inplace) {
    rsize=(realsize(nx,inplace)-2)*istride+(M-1)*idist+2;
    csize=2*(nx/2*ostride+(M-1)*odist+1);
    if(inplace)
      rsize=csize=std::max(rsize,csize);
  }
};

// Base clase for fft routines
//
class fftw : public ThreadBase {
protected:
  size_t doubles; // number of double precision values in output
  int sign;
  size_t threads;
  double norm;

  fftw_plan plan;
  bool inplace;

  size_t Dist(size_t n, size_t stride, size_t dist) {
    return dist ? dist : ((stride == 1) ? n : 1);
  }

  static const double twopi;

public:
  static size_t effort;
  static size_t maxthreads;
  static fftw_plan (*planner)(fftw *f, Complex *in, Complex *out);
  static bool wiser;

  virtual size_t Threads() {return threads;}

  static const char *oddshift;

  // In-place shift of Fourier origin to (nx/2,0) for even nx.
  static void Shift(Complex *data, size_t nx, size_t ny,
                    size_t threads) {
    size_t nyp=ny/2+1;
    size_t stop=nx*nyp;
    if(nx % 2 == 0) {
      size_t inc=2*nyp;
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(size_t i=nyp; i < stop; i += inc) {
        Complex *p=data+i;
        for(size_t j=0; j < nyp; j++) p[j]=-p[j];
      }
    } else {
      std::cerr << oddshift << std::endl;
      exit(1);
    }
  }

  // Out-of-place shift of Fourier origin to (nx/2,0) for even nx.
  static void Shift(double *data, size_t nx, size_t ny,
                    size_t threads) {
    if(nx % 2 == 0) {
      size_t stop=nx*ny;
      size_t inc=2*ny;
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(size_t i=ny; i < stop; i += inc) {
        double *p=data+i;
        for(size_t j=0; j < ny; j++) p[j]=-p[j];
      }
    } else {
      std::cerr << oddshift << std::endl;
      exit(1);
    }
  }

  // In-place shift of Fourier origin to (nx/2,ny/2,0) for even nx and ny.
  static void Shift(Complex *data, size_t nx, size_t ny,
                    size_t nz, size_t threads) {
    size_t nzp=nz/2+1;
    size_t nyzp=ny*nzp;
    if(nx % 2 == 0 && ny % 2 == 0) {
      size_t pinc=2*nzp;
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(size_t i=0; i < nx; i++) {
        Complex *pstart=data+i*nyzp;
        Complex *pstop=pstart+nyzp;
        for(Complex *p=pstart+(1-(i % 2))*nzp; p < pstop; p += pinc) {
          for(size_t k=0; k < nzp; k++) p[k]=-p[k];
        }
      }
    } else {
      std::cerr << oddshift << " or odd ny" << std::endl;
      exit(1);
    }
  }

  // Out-of-place shift of Fourier origin to (nx/2,ny/2,0) for even nx and ny.
  static void Shift(double *data, size_t nx, size_t ny,
                    size_t nz, size_t threads) {
    size_t nyz=ny*nz;
    if(nx % 2 == 0 && ny % 2 == 0) {
      size_t pinc=2*nz;
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(size_t i=0; i < nx; i++) {
        double *pstart=data+i*nyz;
        double *pstop=pstart+nyz;
        for(double *p=pstart+(1-(i % 2))*nz; p < pstop; p += pinc) {
          for(size_t k=0; k < nz; k++) p[k]=-p[k];
        }
      }
    } else {
      std::cerr << oddshift << " or odd ny" << std::endl;
      exit(1);
    }
  }

  fftw() : plan(NULL) {}
  fftw(size_t doubles, int sign, size_t threads,
       size_t n=0) :
    doubles(doubles), sign(sign), threads(threads),
    norm(1.0/(n ? n : doubles/2)), plan(NULL) {
#ifndef FFTWPP_SINGLE_THREAD
    fftw_init_threads();
#endif
  }

  virtual ~fftw() {
    if(plan)
      fftw_destroy_plan(plan);
  }

  virtual fftw_plan Plan(Complex *in, Complex *out) {return NULL;};

  inline void CheckAlign(Complex *p, const char *s) {
    if((size_t) p % sizeof(Complex) == 0) return;
    std::cerr << "WARNING: " << s << " array is not " << sizeof(Complex)
              << "-byte aligned: address " << p << std::endl;
  }

  void noplan() {
    std::cerr << "Unable to construct FFTW plan" << std::endl;
    exit(1);
  }

  static void planThreads(size_t threads) {
#ifndef FFTWPP_SINGLE_THREAD
    omp_set_num_threads(threads);
    fftw_plan_with_nthreads(threads);
#endif
  }

  inline Complex *CheckAlign(Complex *in, Complex *out, bool constructor=true)
  {
#ifndef NO_CHECK_ALIGN
    CheckAlign(in,constructor ? "constructor input" : "input");
    if(out) CheckAlign(out,constructor ? "constructor output" : "output");
    else out=in;
#else
    if(!out) out=in;
#endif
    return out;
  }

  void Setup(Complex *in, Complex *out=NULL) {
    bool alloc=!in;
    if(alloc) in=utils::ComplexAlign((doubles+1)/2);
    out=CheckAlign(in,out);
    inplace=(out==in);

    parallel::Threshold(threads);
    if(doubles < 2*threshold)
      threads=1;

    planThreads(threads);
    plan=(*planner)(this,in,out);
    if(!plan) noplan();

    if(alloc) Array::deleteAlign(in,(doubles+1)/2);
#ifdef FFTWPP_VERBOSE
    if(threads > 1)
      std::cout << "Using " << threads << " threads." << std::endl;
#endif
  }

  void Setup(Complex *in, double *out) {
    parallel::Threshold(threads);
    if(doubles < 4*threshold)
      threads=1;

    Setup(in,(Complex *) out);
  }

  void Setup(double *in, Complex *out=NULL) {
    parallel::Threshold(threads);
    if(doubles < 4*threshold)
      threads=1;

    Setup((Complex *) in,out);
  }

  virtual void Execute(Complex *in, Complex *out, bool=false) {
    fftw_execute_dft(plan,(fftw_complex *) in,(fftw_complex *) out);
  }

  Complex *Setout(Complex *in, Complex *out) {
    out=CheckAlign(in,out,false);
    if(inplace ^ (out == in)) {
      std::cerr << "ERROR: fft " << inout << std::endl;
      exit(1);
    }
    return out;
  }

  void fft(Complex *in, Complex *out=NULL) {
    out=Setout(in,out);
    Execute(in,out);
  }

  void fft(double *in, Complex *out=NULL) {
    fft((Complex *) in,out);
  }

  void fft(Complex *in, double *out) {
    fft(in,(Complex *) out);
  }

  void fft0(Complex *in, Complex *out=NULL) {
    out=Setout(in,out);
    Execute(in,out,true);
  }

  void fft0(double *in, Complex *out=NULL) {
    fft0((Complex *) in,out);
  }

  void fft0(Complex *in, double *out) {
    fft0(in,(Complex *) out);
  }

  void Normalize(Complex *out) {
    size_t stop=doubles/2;
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
    for(size_t i=0; i < stop; i++) out[i] *= norm;
  }

  void Normalize(double *out) {
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
    for(size_t i=0; i < doubles; i++) out[i] *= norm;
  }

  void fftNormalized(Complex *in, Complex *out=NULL)
  {
    out=Setout(in,out);
    Execute(in,out);
    Normalize(out);
  }

  void fftNormalized(Complex *in, double *out) {
    out=(double *) Setout(in,(Complex *) out);
    Execute(in,(Complex *) out);
    Normalize(out);
  }

  void fftNormalized(double *in, Complex *out) {
    out=Setout((Complex *) in,out);
    Execute((Complex *) in,out);
    Normalize(out);
  }

  void fft0Normalized(Complex *in, Complex *out=NULL)
  {
    out=Setout(in,out);
    Execute(in,out,true);
    Normalize(out);
  }

  void fft0Normalized(Complex *in, double *out) {
    out=(double *) Setout(in,(Complex *) out);
    Execute(in,(Complex *) out,true);
    Normalize(out);
  }

  void fft0Normalized(double *in, Complex *out) {
    out=Setout((Complex *) in,out);
    Execute((Complex *) in,out,true);
    Normalize(out);
  }

  template<class O>
  void Normalize(size_t nx, size_t M, size_t ostride,
                 size_t odist, O *out) {
    size_t stop=nx*ostride;
    O *outMdist=out+M*odist;
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
    for(size_t i=0; i < stop; i += ostride) {
      O *pstop=outMdist+i;
      for(O *p=out+i; p < pstop; p += odist) {
        *p *= norm;
      }
    }
  }

  template<class I, class O>
  void fftNormalized(size_t nx, size_t M, size_t ostride,
                     size_t odist, I *in, O *out) {
    out=(O *) Setout((Complex *) in,(Complex *) out);
    Execute((Complex *) in,(Complex *) out);
    Normalize(nx,M,ostride,odist,out);
  }

}; // class fftw

class Transpose {
  fftw_plan plan;
  bool inplace;
public:
  template<class T>
  Transpose(size_t rows, size_t cols, size_t length,
            T *in, T *out=NULL, size_t threads=fftw::maxthreads) {
    size_t size=sizeof(T);
    if(size % sizeof(double) != 0) {
      std::cerr << "ERROR: Transpose is not implemented for type of size "
                << size;
      exit(1);
    }
    plan=NULL;
    if(!out) out=in;
    inplace=(out==in);
    if(rows == 0 || cols == 0) return;
    size /= sizeof(double);
    length *= size;

    parallel::Threshold(threads);
    if(length*rows*cols/2 < threshold)
      threads=1;

    fftw::planThreads(threads);

    fftw_iodim dims[3];

    dims[0].n=rows;
    dims[0].is=cols*length;
    dims[0].os=length;

    dims[1].n=cols;
    dims[1].is=length;
    dims[1].os=rows*length;

    dims[2].n=length;
    dims[2].is=1;
    dims[2].os=1;

    // A plan with rank=0 is a transpose.
    plan=fftw_plan_guru_r2r(0,NULL,3,dims,(double *) in,(double *) out,
                            NULL,fftw::effort);
  }

  ~Transpose() {
    if(plan)
      fftw_destroy_plan(plan);
  }

  template<class T>
  void transpose(T *in, T *out=NULL) {
    if(!plan) return;
    if(!out) out=in;
    if(inplace ^ (out == in)) {
      std::cerr << "ERROR: Transpose " << inout << std::endl;
      exit(1);
    }
    fftw_execute_r2r(plan,(double *) in,(double*) out);
  }
};

template<class T, class L>
class Threadtable {
public:
  typedef std::map<T,size_t,L> Table;

  size_t Lookup(Table& table, T key) {
    typename Table::iterator p=table.find(key);
    return p == table.end() ? 0 : p->second;
  }

  void Store(Table& threadtable, T key, size_t t) {
    threadtable[key]=t;
  }
};

struct keytype {
  size_t nx;
  size_t M;
  size_t threads;
  bool inplace;
  keytype(size_t nx, size_t M, size_t threads,
          bool inplace) :
    nx(nx), M(M), threads(threads), inplace(inplace) {}
};

struct keyless {
  bool operator()(const keytype& a, const keytype& b) const {
    return a.nx < b.nx || (a.nx == b.nx &&
                           (a.M < b.M || (a.M == b.M &&
                                              (a.threads < b.threads ||
                                               (a.threads == b.threads &&
                                                a.inplace < b.inplace)))));
  }
};

// Compute the complex Fourier transform of n complex values.
// Before calling fft(), the arrays in and out (which may coincide) must be
// allocated as Complex[n].
//
// Out-of-place usage:
//
//   fft1d Forward(n,-1,in,out);
//   Forward.fft(in,out);
//
//   fft1d Backward(n,1,in,out);
//   Backward.fft(in,out);
//
//   fft1d Backward(n,1,in,out);
//   Backward.fftNormalized(in,out); // True inverse of Forward.fft(out,in);
//
// In-place usage:
//
//   fft1d Forward(n,-1);
//   Forward.fft(in);
//
//   fft1d Backward(n,1);
//   Backward.fft(in);
//
class fft1d : public fftw {
  size_t nx;
public:
  fft1d(size_t nx, int sign, Complex *in=NULL, Complex *out=NULL,
        size_t threads=maxthreads)
    : fftw(2*nx,sign,threads), nx(nx) {Setup(in,out);}

#ifdef __Array_h__
  fft1d(int sign, const Array::array1<Complex>& in,
        const Array::array1<Complex>& out=Array::NULL1,
        size_t threads=maxthreads)
    : fftw(2*in.Nx(),sign,threads), nx(in.Nx()) {Setup(in,out);}
#endif

  fftw_plan Plan(Complex *in, Complex *out) {
    return fftw_plan_dft_1d(nx,(fftw_complex *) in,(fftw_complex *) out,
                            sign,effort);
  }
};

template<class I, class O>
class fftwblock : public virtual fftw,
                  public virtual Threadtable<keytype,keyless> {
public:
  int nx;
  size_t M;
  size_t istride,ostride;
  size_t idist,odist;
  fftw_plan plan1,plan2;
  size_t T,Q,R;
  fftwblock() : plan1(NULL), plan2(NULL) {}

  fftwblock(size_t nx, size_t M,
            size_t istride, size_t ostride, size_t idist, size_t odist)
    : fftw(), nx(nx), M(M), istride(istride), ostride(ostride),
      idist(Dist(nx,istride,idist)), odist(Dist(nx,ostride,odist)),
      plan1(NULL), plan2(NULL) {}

  void init(Complex *in, Complex *out, size_t Threads,
            Table& threadtable) {
    T=1;
    Q=M;
    R=0;

    if(Threads > M && M > 1) Threads=M;

    threads=Threads;
    Setup(in,out);
    Threads=threads;

    size_t T0=Threads;
    if(T0 > 1) {
      size_t nxp=nx/2+1;
      size_t olength=0;
      size_t ilength=0;
      if(typeid(I) == typeid(double)) {
        ilength=nx;
        olength=nxp;
      }
      if(typeid(O) == typeid(double)) {
        ilength=nxp;
        olength=nx;
      }
      if(!inplace ||
         (ostride*olength*sizeof(O) <= idist*sizeof(I) &&
          odist*sizeof(O) >= istride*ilength*sizeof(I))) {
        T=T0;
        Q=T > 0 ? M/T : 0;
        R=M-Q*T;

        size_t data=Lookup(threadtable,keytype(nx,M,Threads,inplace));
        if(data == 1)
          T0=1;
        else {
          fftw_plan planFFTW=plan;
          threads=1;
          Setup(in,out);
          plan1=plan;
          if(data == T) {
            plan=NULL;
            return;
          }
          plan=planFFTW;
        }
      } else T0=1;
    }

    if(T0 == 1 || time(in,out)) { // Use FFTW's multithreading
      T=1;
      if(plan1) {
        fftw_destroy_plan(plan1);
        plan1=NULL;
        if(plan2) {
          fftw_destroy_plan(plan2);
          plan2=NULL;
        }
        threads=Threads;
        Store(threadtable,keytype(nx,M,Threads,inplace),T);
      }
    } else { // Do the multithreading ourselves
      T=T0;
      threads=T;
      Store(threadtable,keytype(nx,M,Threads,inplace),T);
    }
  }

  bool time(Complex *in, Complex *out) {
    utils::statistics S(true),ST(true);
    utils::statistics medianS(true),medianST(true);

    double eps=0.02;

    size_t T0=T;

    do {
      T=1; // FFTW
      utils::cpuTimer C;
      inplace ? fftNormalized(in,out) : fft(in,out);
      S.add(C.nanoseconds());

      T=T0; // BLOCK
      utils::cpuTimer CT;
      inplace ? fftNormalized(in,out) : fft(in,out);
      ST.add(CT.nanoseconds());

      if(S.count() >= 4 && ST.min() >= S.max())
        return true;

      if(S.count() >= 4 && S.min() >= ST.max())
        return false;

      medianS.add(S.median());
      medianST.add(ST.median());

    } while(S.count() < 5 || medianS.stderror() > eps*medianS.mean() ||
            medianST.stderror() > eps*medianST.mean());

    return S.median() <= ST.median();
  }


  fftw_plan Plan(int Q, fftw_complex *in, fftw_complex *out) {
    return fftw_plan_many_dft(1,&nx,Q,in,NULL,istride,idist,
                              out,NULL,ostride,odist,sign,effort);
  }

  fftw_plan Plan(int Q, double *in, fftw_complex *out) {
    return fftw_plan_many_dft_r2c(1,&nx,Q,in,NULL,istride,idist,
                                  out,NULL,ostride,odist,effort);
  }

  fftw_plan Plan(int Q, fftw_complex *in, double *out) {
    return fftw_plan_many_dft_c2r(1,&nx,Q,in,NULL,istride,idist,
                                  out,NULL,ostride,odist,effort);
  }

  fftw_plan Plan(Complex *in, Complex *out) {
    if(R > 0) {
      plan2=Plan(Q+1,(I *) in,(O *) out);
      if(!plan2) return NULL;
    }
    return Plan(Q,(I *) in,(O *) out);
  }

  void Execute(fftw_plan plan, fftw_complex *in, fftw_complex *out) {
    fftw_execute_dft(plan,in,out);
  }

  void Execute(fftw_plan plan, double *in, fftw_complex *out) {
    fftw_execute_dft_r2c(plan,in,out);
  }

  void Execute(fftw_plan plan, fftw_complex *in, double *out) {
    fftw_execute_dft_c2r(plan,in,out);
  }

  void Execute(Complex *in, Complex *out, bool=false) {
    if(T == 1)
      Execute(plan,(I *) in,(O *) out);
    else {
      size_t extra=T-R;
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(T)
#endif
      for(size_t i=0; i < T; ++i) {
        size_t iQ=i*Q;
        if(i < extra)
          Execute(plan1,(I *) in+iQ*idist,(O *) out+iQ*odist);
        else {
          size_t offset=iQ+i-extra;
          Execute(plan2,(I *) in+offset*idist,(O *) out+offset*odist);
        }
      }
    }
  }

  size_t Threads() {return std::max(T,threads);}

  ~fftwblock() {
    if(plan1)
      fftw_destroy_plan(plan1);
    if(plan2)
      fftw_destroy_plan(plan2);
  }
};

class Mfft1d : public fftwblock<fftw_complex,fftw_complex>,
               public virtual Threadtable<keytype,keyless> {
  static Table threadtable;
public:
  Mfft1d(size_t nx, int sign, size_t M=1,
         Complex *in=NULL, Complex *out=NULL,
         size_t threads=maxthreads) :
    fftw(2*((nx-1)+(M-1)*nx+1),sign,threads,nx),
    fftwblock<fftw_complex,fftw_complex>(nx,M,1,1,nx,nx) {
    init(in,out,threads,threadtable);
  }

    Mfft1d(size_t nx, int sign, size_t M, size_t stride=1,
           size_t dist=0, Complex *in=NULL, Complex *out=NULL,
           size_t threads=maxthreads) :
      fftw(2*((nx-1)*stride+(M-1)*Dist(nx,stride,dist)+1),sign,threads,nx),
      fftwblock<fftw_complex,fftw_complex>
      (nx,M,stride,stride,dist,dist) {
      init(in,out,threads,threadtable);
    }

      Mfft1d(size_t nx, int sign, size_t M,
             size_t istride, size_t ostride, size_t idist, size_t odist,
             Complex *in, Complex *out, size_t threads=maxthreads):
        fftw(std::max(2*((nx-1)*istride+(M-1)*Dist(nx,istride,idist)+1),
                      2*((nx-1)*ostride+(M-1)*Dist(nx,ostride,odist)+1)),sign,
             threads,nx),
        fftwblock<fftw_complex,fftw_complex>(nx,M,istride,ostride,idist,odist)
  {
    init(in,out,threads,threadtable);
  }
};

// Compute the complex Fourier transform of M complex vectors, each of
// length n.
// Before calling fft(), the arrays in and out (which may coincide) must be
// allocated as Complex[M*n].
//
// Out-of-place usage:
//
//   mfft1d Forward(n,-1,M,stride,dist,in,out);
//   Forward.fft(in,out);
//
//   mfft1d Forward(n,-1,M,istride,ostride,idist,odist,in,out);
//   Forward.fft(in,out);
//
// In-place usage:
//
//   mfft1d Forward(n,-1,M,stride,dist);
//   Forward.fft(in);
//
//
//
// Notes:
//   stride is the spacing between the elements of each Complex vector;
//   dist is the spacing between the first elements of the vectors.
//
//
class mfft1d {
  bool single;
  fft1d *fft1;
  Mfft1d *fftm;
public:
  mfft1d(size_t nx, int sign, size_t M=1,
         Complex *in=NULL, Complex *out=NULL,
         size_t threads=fftw::maxthreads) : single(M == 1) {
    if(single)
      fft1=new fft1d(nx,sign,in,out,threads);
    else
      fftm=new Mfft1d(nx,sign,M,in,out,threads);
  }

  mfft1d(size_t nx, int sign, size_t M, size_t stride=1,
         size_t dist=0, Complex *in=NULL, Complex *out=NULL,
         size_t threads=fftw::maxthreads) :
    single(M == 1 && stride == 1) {
    if(single)
      fft1=new fft1d(nx,sign,in,out,threads);
    else
      fftm=new Mfft1d(nx,sign,M,stride,dist,in,out,threads);
  }

  mfft1d(size_t nx, int sign, size_t M,
         size_t istride, size_t ostride, size_t idist, size_t odist,
         Complex *in, Complex *out, size_t threads=fftw::maxthreads) :
    single(M == 1 && istride == 1 && ostride == 1) {
    if(single)
      fft1=new fft1d(nx,sign,in,out,threads);
    else
      fftm=new Mfft1d(nx,sign,M,istride,ostride,idist,odist,in,out,threads);
  }

  size_t Threads() {
    return single ? fft1->Threads() : fftm->Threads();
  }

  template<class I>
  void fft(I in) {
    single ? fft1->fft(in) : fftm->fft(in);
  }

  template<class I, class O>
  void fft(I in, O out) {
    single ? fft1->fft(in,out) : fftm->fft(in,out);
  }

  template<class I>
  void fftNormalized(I in) {
    single ? fft1->fftNormalized(in) : fftm->fftNormalized(in);
  }

  template<class I, class O>
  void fftNormalized(I in, O out) {
    single ? fft1->fftNormalized(in,out) : fftm->fftNormalized(in,out);
  }

  template<class O>
  void Normalize(O out) {
    single ? fft1->Normalize(out) : fftm->Normalize(out);
  }

  ~mfft1d() {
    if(single)
      delete fft1;
    else
      delete fftm;
  }
};

// Compute the complex Fourier transform of n real values, using phase sign -1.
// Before calling fft(), the array in must be allocated as double[n] and
// the array out must be allocated as Complex[n/2+1]. The arrays in and out
// may coincide, allocated as Complex[n/2+1].
//
// Out-of-place usage:
//
//   rcfft1d Forward(n,in,out);
//   Forward.fft(in,out);
//
// In-place usage:
//
//   rcfft1d Forward(n);
//   Forward.fft(out);
//
// Notes:
//   in contains the n real values stored as a Complex array;
//   out contains the first n/2+1 Complex Fourier values.
//
class rcfft1d : public fftw {
  size_t nx;
public:
  rcfft1d(size_t nx, Complex *out=NULL, size_t threads=maxthreads)
    : fftw(2*(nx/2+1),-1,threads,nx), nx(nx) {Setup(out,(double*) NULL);}

  rcfft1d(size_t nx, double *in, Complex *out=NULL,
          size_t threads=maxthreads)
    : fftw(2*(nx/2+1),-1,threads,nx), nx(nx) {Setup(in,out);}

  fftw_plan Plan(Complex *in, Complex *out) {
    return fftw_plan_dft_r2c_1d(nx,(double *) in,(fftw_complex *) out, effort);
  }

  void Execute(Complex *in, Complex *out, bool=false) {
    fftw_execute_dft_r2c(plan,(double *) in,(fftw_complex *) out);
  }
};

// Compute the real inverse Fourier transform of the n/2+1 Complex values
// corresponding to the non-negative part of the frequency spectrum, using
// phase sign +1.
// Before calling fft(), the array in must be allocated as Complex[n/2+1]
// and the array out must be allocated as double[n]. The arrays in and out
// may coincide, allocated as Complex[n/2+1].
//
// Out-of-place usage (input destroyed):
//
//   crfft1d Backward(n,in,out);
//   Backward.fft(in,out);
//
// In-place usage:
//
//   crfft1d Backward(n);
//   Backward.fft(in);
//
// Notes:
//   in contains the first n/2+1 Complex Fourier values.
//   out contains the n real values stored as a Complex array;
//
class crfft1d : public fftw {
  size_t nx;
public:
  crfft1d(size_t nx, double *out=NULL, size_t threads=maxthreads)
    : fftw(2*(nx/2+1),1,threads,nx), nx(nx) {Setup(out);}

  crfft1d(size_t nx, Complex *in, double *out=NULL,
          size_t threads=maxthreads)
    : fftw(realsize(nx,Inplace(in,out)),1,threads,nx), nx(nx) {Setup(in,out);}

  fftw_plan Plan(Complex *in, Complex *out) {
    return fftw_plan_dft_c2r_1d(nx,(fftw_complex *) in,(double *) out,effort);
  }

  void Execute(Complex *in, Complex *out, bool=false) {
    fftw_execute_dft_c2r(plan,(fftw_complex *) in,(double *) out);
  }
};

class Mrcfft1d : public fftwblock<double,fftw_complex>,
                 public virtual Threadtable<keytype,keyless> {
  static Table threadtable;
public:
  Mrcfft1d(size_t nx, size_t M, size_t istride, size_t ostride,
           size_t idist, size_t odist, double *in=NULL, Complex *out=NULL,
           size_t threads=maxthreads)
    : fftw(Doubles(nx,M,istride,ostride,idist,odist,Inplace(in,out)).csize,
           -1,threads,nx),
      fftwblock<double,fftw_complex>
    (nx,M,istride,ostride,idist,odist) {
    init((Complex *) in,out,threads,threadtable);
  }

  void Normalize(Complex *out) {
    fftw::Normalize<Complex>(nx/2+1,M,ostride,odist,out);
  }

  void fftNormalized(double *in, Complex *out=NULL) {
    fftw::fftNormalized<double,Complex>(nx/2+1,M,ostride,odist,in,out);
  }
};

// Compute the real Fourier transform of M real vectors, each of length n,
// using phase sign -1. Before calling fft(), the array in must be
// allocated as double[M*n] and the array out must be allocated as
// Complex[M*(n/2+1)]. The arrays in and out may coincide,
// allocated as Complex[M*(n/2+1)].
//
// Out-of-place usage:
//
//   mrcfft1d Forward(n,M,istride,ostride,idist,odist,in,out);
//   Forward.fft(in,out);
//
// In-place usage:
//
//   mrcfft1d Forward(n,M,istride,ostride,idist,odist);
//   Forward.fft(out);
//
// Notes:
//   istride is the spacing between the elements of each real vector;
//   ostride is the spacing between the elements of each Complex vector;
//   idist is the spacing between the first elements of the real vectors;
//   odist is the spacing between the first elements of the Complex vectors;
//   in contains the n real values stored as a Complex array;
//   out contains the first n/2+1 Complex Fourier values.
//
class mrcfft1d {
  bool single;
  rcfft1d *fft1;
  Mrcfft1d *fftm;
public:
  mrcfft1d(size_t nx, size_t M, size_t istride, size_t ostride,
           size_t idist, size_t odist, double *in=NULL, Complex *out=NULL,
           size_t threads=fftw::maxthreads) :
    single(M == 1 && istride == 1 && ostride == 1) {
    if(single)
      fft1=new rcfft1d(nx,in,out,threads);
    else
      fftm=new Mrcfft1d(nx,M,istride,ostride,idist,odist,in,out,threads);
  }

  size_t Threads() {
    return single ? fft1->Threads() : fftm->Threads();
  }

  template<class I>
  void fft(I in) {
    single ? fft1->fft(in) : fftm->fft(in);
  }

  template<class I, class O>
  void fft(I in, O out) {
    single ? fft1->fft(in,out) : fftm->fft(in,out);
  }

  void Normalize(Complex *out) {
    single ? fft1->Normalize(out) : fftm->Normalize(out);
  }

  template<class I>
  void fftNormalized(I in) {
    single ? fft1->fftNormalized(in) : fftm->fftNormalized(in);
  }

  template<class I, class O>
  void fftNormalized(I in, O out=NULL) {
    single ? fft1->fftNormalized(in,out) : fftm->fftNormalized(in,out);
  }

  ~mrcfft1d() {
    if(single)
      delete fft1;
    else
      delete fftm;
  }
};

class Mcrfft1d : public fftwblock<fftw_complex,double>,
                 public virtual Threadtable<keytype,keyless> {
  static Table threadtable;
public:
  Mcrfft1d(size_t nx, size_t M, size_t istride, size_t ostride,
           size_t idist, size_t odist, Complex *in=NULL, double *out=NULL,
           size_t threads=maxthreads)
    : fftw(Doubles(nx,M,ostride,istride,odist,idist,Inplace(in,out)).rsize,
           1,threads,nx),
      fftwblock<fftw_complex,double>
    (nx,M,istride,ostride,idist,odist) {
    init(in,(Complex *) out,threads,threadtable);
  }

  void Normalize(double *out) {
    fftw::Normalize<double>(nx,M,ostride,odist,out);
  }

  void fftNormalized(Complex *in, double *out=NULL) {
    fftw::fftNormalized<Complex,double>(nx,M,ostride,odist,in,out);
  }
};

// Compute the real inverse Fourier transform of M complex vectors, each of
// length n/2+1, corresponding to the non-negative parts of the frequency
// spectra, using phase sign +1. Before calling fft(), the array in must be
// allocated as Complex[M*(n/2+1)] and the array out must be allocated as
// double[M*n]. The arrays in and out may coincide,
// allocated as Complex[M*(n/2+1)].
//
// Out-of-place usage (input destroyed):
//
//   mcrfft1d Backward(n,M,istride,ostride,idist,odist,in,out);
//   Backward.fft(in,out);
//
// In-place usage:
//
//   mcrfft1d Backward(n,M,istride,ostride,idist,odist);
//   Backward.fft(out);
//
// Notes:
//   stride is the spacing between the elements of each Complex vector;
//   dist is the spacing between the first elements of the vectors;
//   in contains the first n/2+1 Complex Fourier values;
//   out contains the n real values stored as a Complex array.
//
class mcrfft1d {
  bool single;
  crfft1d *fft1;
  Mcrfft1d *fftm;
public:
  mcrfft1d(size_t nx, size_t M, size_t istride, size_t ostride,
           size_t idist, size_t odist, Complex *in=NULL, double *out=NULL,
           size_t threads=fftw::maxthreads) :
    single(M == 1 && istride == 1 && ostride == 1) {
    if(single)
      fft1=new crfft1d(nx,in,out,threads);
    else
      fftm=new Mcrfft1d(nx,M,istride,ostride,idist,odist,in,out,threads);
  }

  size_t Threads() {
    return single ? fft1->Threads() : fftm->Threads();
  }

  template<class I>
  void fft(I in) {
    single ? fft1->fft(in) : fftm->fft(in);
  }

  template<class I, class O>
  void fft(I in, O out) {
    single ? fft1->fft(in,out) : fftm->fft(in,out);
  }

  void Normalize(double *out) {
    single ? fft1->Normalize(out) : fftm->Normalize(out);
  }

  template<class I>
  void fftNormalized(I in) {
    single ? fft1->fftNormalized(in) : fftm->fftNormalized(in);
  }

  template<class I, class O>
  void fftNormalized(I in, O out=NULL) {
    single ? fft1->fftNormalized(in,out) : fftm->fftNormalized(in,out);
  }

  ~mcrfft1d() {
    if(single)
      delete fft1;
    else
      delete fftm;
  }
};

// Compute the complex two-dimensional Fourier transform of nx times ny
// complex values. Before calling fft(), the arrays in and out (which may
// coincide) must be allocated as Complex[nx*ny].
//
// Out-of-place usage:
//
//   fft2d Forward(nx,ny,-1,in,out);
//   Forward.fft(in,out);
//
//   fft2d Backward(nx,ny,1,in,out);
//   Backward.fft(in,out);
//
//   fft2d Backward(nx,ny,1,in,out);
//   Backward.fftNormalized(in,out); // True inverse of Forward.fft(out,in);
//
// In-place usage:
//
//   fft2d Forward(nx,ny,-1);
//   Forward.fft(in);
//
//   fft2d Backward(nx,ny,1);
//   Backward.fft(in);
//
// Note:
//   in[ny*i+j] contains the ny Complex values for each i=0,...,nx-1.
//
class fft2d : public fftw {
  size_t nx;
  size_t ny;
public:
  fft2d(size_t nx, size_t ny, int sign, Complex *in=NULL,
        Complex *out=NULL, size_t threads=maxthreads)
    : fftw(2*nx*ny,sign,threads), nx(nx), ny(ny) {Setup(in,out);}

#ifdef __Array_h__
  fft2d(int sign, const Array::array2<Complex>& in,
        const Array::array2<Complex>& out=Array::NULL2,
        size_t threads=maxthreads)
    : fftw(2*in.Size(),sign,threads), nx(in.Nx()), ny(in.Ny()) {
    Setup(in,out);
  }
#endif

  fftw_plan Plan(Complex *in, Complex *out) {
    return fftw_plan_dft_2d(nx,ny,(fftw_complex *) in,(fftw_complex *) out,
                            sign,effort);
  }

  void Execute(Complex *in, Complex *out, bool=false) {
    fftw_execute_dft(plan,(fftw_complex *) in,(fftw_complex *) out);
  }
};

// Compute the complex two-dimensional Fourier transform of nx times ny real
// values, using phase sign -1.
// Before calling fft(), the array in must be allocated as double[nx*ny] and
// the array out must be allocated as Complex[nx*(ny/2+1)]. The arrays in
// and out may coincide, allocated as Complex[nx*(ny/2+1)].
//
// Out-of-place usage:
//
//   rcfft2d Forward(nx,ny,in,out);
//   Forward.fft(in,out);       // Origin of Fourier domain at (0,0)
//   Forward.fft0(in,out);      // Origin of Fourier domain at (nx/2,0);
//                                 input destroyed.
//
// In-place usage:
//
//   rcfft2d Forward(nx,ny);
//   Forward.fft(in);           // Origin of Fourier domain at (0,0)
//   Forward.fft0(in);          // Origin of Fourier domain at (nx/2,0)
//
// Notes:
//   in contains the nx*ny real values stored as a Complex array;
//   out contains the upper-half portion (ky >= 0) of the Complex transform.
//
class rcfft2d : public fftw {
  size_t nx;
  size_t ny;
public:
  rcfft2d(size_t nx, size_t ny, Complex *out=NULL,
          size_t threads=maxthreads)
    : fftw(2*nx*(ny/2+1),-1,threads,nx*ny), nx(nx), ny(ny) {Setup(out);}

  rcfft2d(size_t nx, size_t ny, double *in, Complex *out=NULL,
          size_t threads=maxthreads)
    : fftw(2*nx*(ny/2+1),-1,threads,nx*ny), nx(nx), ny(ny) {
    Setup(in,out);
  }

  fftw_plan Plan(Complex *in, Complex *out) {
    return fftw_plan_dft_r2c_2d(nx,ny,(double *) in,(fftw_complex *) out,
                                effort);
  }

  void Execute(Complex *in, Complex *out, bool shift=false) {
    if(shift) {
      if(inplace) Shift(in,nx,ny,threads);
      else Shift((double *) in,nx,ny,threads);
    }
    fftw_execute_dft_r2c(plan,(double *) in,(fftw_complex *) out);
  }

  // Set Nyquist modes of even shifted transforms to zero.
  void deNyquist(Complex *f) {
    size_t nyp=ny/2+1;
    if(nx % 2 == 0)
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(size_t j=0; j < nyp; ++j)
        f[j]=0.0;
    if(ny % 2 == 0)
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(size_t i=0; i < nx; ++i)
        f[(i+1)*nyp-1]=0.0;
  }
};

// Compute the real two-dimensional inverse Fourier transform of the
// nx*(ny/2+1) Complex values corresponding to the spectral values in the
// half-plane ky >= 0, using phase sign +1.
// Before calling fft(), the array in must be allocated as
// Complex[nx*(ny/2+1)] and the array out must be allocated as
// double[nx*ny]. The arrays in and out may coincide,
// allocated as Complex[nx*(ny/2+1)].
//
// Out-of-place usage (input destroyed):
//
//   crfft2d Backward(nx,ny,in,out);
//   Backward.fft(in,out);      // Origin of Fourier domain at (0,0)
//   Backward.fft0(in,out);     // Origin of Fourier domain at (nx/2,0)
//
// In-place usage:
//
//   crfft2d Backward(nx,ny);
//   Backward.fft(in);          // Origin of Fourier domain at (0,0)
//   Backward.fft0(in);         // Origin of Fourier domain at (nx/2,0)
//
// Notes:
//   in contains the upper-half portion (ky >= 0) of the Complex transform;
//   out contains the nx*ny real values stored as a Complex array.
//
class crfft2d : public fftw {
  size_t nx;
  size_t ny;
public:
  crfft2d(size_t nx, size_t ny, double *out=NULL,
          size_t threads=maxthreads) :
    fftw(2*nx*(ny/2+1),1,threads,nx*ny), nx(nx), ny(ny) {Setup(out);}

  crfft2d(size_t nx, size_t ny, Complex *in, double *out=NULL,
          size_t threads=maxthreads)
    : fftw(nx*realsize(ny,Inplace(in,out)),1,threads,nx*ny), nx(nx), ny(ny) {
    Setup(in,out);
  }

  fftw_plan Plan(Complex *in, Complex *out) {
    return fftw_plan_dft_c2r_2d(nx,ny,(fftw_complex *) in,(double *) out,
                                effort);
  }

  void Execute(Complex *in, Complex *out, bool shift=false) {
    fftw_execute_dft_c2r(plan,(fftw_complex *) in,(double *) out);
    if(shift) {
      if(inplace) Shift(out,nx,ny,threads);
      else Shift((double *) out,nx,ny,threads);
    }
  }

  // Set Nyquist modes of even shifted transforms to zero.
  void deNyquist(Complex *f) {
    size_t nyp=ny/2+1;
    if(nx % 2 == 0)
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(size_t j=0; j < nyp; ++j)
        f[j]=0.0;
    if(ny % 2 == 0)
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(size_t i=0; i < nx; ++i)
        f[(i+1)*nyp-1]=0.0;
  }
};

// Compute the complex three-dimensional Fourier transform of
// nx times ny times nz complex values. Before calling fft(), the arrays in
// and out (which may coincide) must be allocated as Complex[nx*ny*nz].
//
// Out-of-place usage:
//
//   fft3d Forward(nx,ny,nz,-1,in,out);
//   Forward.fft(in,out);
//
//   fft3d Backward(nx,ny,nz,1,in,out);
//   Backward.fft(in,out);
//
//   fft3d Backward(nx,ny,nz,1,in,out);
//   Backward.fftNormalized(in,out); // True inverse of Forward.fft(out,in);
//
// In-place usage:
//
//   fft3d Forward(nx,ny,nz,-1);
//   Forward.fft(in);
//
//   fft3d Backward(nx,ny,nz,1);
//   Backward.fft(in);
//
// Note:
//   in[nz*(ny*i+j)+k] contains the (i,j,k)th Complex value,
//   indexed by i=0,...,nx-1, j=0,...,ny-1, and k=0,...,nz-1.
//
class fft3d : public fftw {
  size_t nx;
  size_t ny;
  size_t nz;
public:
  fft3d(size_t nx, size_t ny, size_t nz,
        int sign, Complex *in=NULL, Complex *out=NULL,
        size_t threads=maxthreads)
    : fftw(2*nx*ny*nz,sign,threads), nx(nx), ny(ny), nz(nz) {Setup(in,out);}

#ifdef __Array_h__
  fft3d(int sign, const Array::array3<Complex>& in,
        const Array::array3<Complex>& out=Array::NULL3,
        size_t threads=maxthreads)
    : fftw(2*in.Size(),sign,threads), nx(in.Nx()), ny(in.Ny()), nz(in.Nz())
  {Setup(in,out);}
#endif

  fftw_plan Plan(Complex *in, Complex *out) {
    return fftw_plan_dft_3d(nx,ny,nz,(fftw_complex *) in,
                            (fftw_complex *) out, sign, effort);
  }
};

// Compute the complex two-dimensional Fourier transform of
// nx times ny times nz real values, using phase sign -1.
// Before calling fft(), the array in must be allocated as double[nx*ny*nz]
// and the array out must be allocated as Complex[nx*ny*(nz/2+1)]. The
// arrays in and out may coincide, allocated as Complex[nx*ny*(nz/2+1)].
//
// Out-of-place usage:
//
//   rcfft3d Forward(nx,ny,nz,in,out);
//   Forward.fft(in,out);       // Origin of Fourier domain at (0,0)
//   Forward.fft0(in,out);      // Origin of Fourier domain at (nx/2,ny/2,0);
//                                 input destroyed
// In-place usage:
//
//   rcfft3d Forward(nx,ny,nz);
//   Forward.fft(in);           // Origin of Fourier domain at (0,0)
//   Forward.fft0(in);          // Origin of Fourier domain at (nx/2,ny/2,0)
//
// Notes:
//   in contains the nx*ny*nz real values stored as a Complex array;
//   out contains the upper-half portion (kz >= 0) of the Complex transform.
//
class rcfft3d : public fftw {
  size_t nx;
  size_t ny;
  size_t nz;
public:
  rcfft3d(size_t nx, size_t ny, size_t nz, Complex *out=NULL,
          size_t threads=maxthreads)
    : fftw(2*nx*ny*(nz/2+1),-1,threads,nx*ny*nz), nx(nx), ny(ny), nz(nz) {
    Setup(out);
  }

  rcfft3d(size_t nx, size_t ny, size_t nz, double *in,
          Complex *out=NULL, size_t threads=maxthreads)
    : fftw(2*nx*ny*(nz/2+1),-1,threads,nx*ny*nz),
      nx(nx), ny(ny), nz(nz) {Setup(in,out);}

  fftw_plan Plan(Complex *in, Complex *out) {
    return fftw_plan_dft_r2c_3d(nx,ny,nz,(double *) in,(fftw_complex *) out,
                                effort);
  }

  void Execute(Complex *in, Complex *out, bool shift=false) {
    if(shift) {
      if(inplace) Shift(in,nx,ny,nz,threads);
      else Shift((double *) in,nx,ny,nz,threads);
    }
    fftw_execute_dft_r2c(plan,(double *) in,(fftw_complex *) out);
  }

  // Set Nyquist modes of even shifted transforms to zero.
  void deNyquist(Complex *f) {
    size_t nzp=nz/2+1;
    size_t yz=ny*nzp;
    if(nx % 2 == 0) {
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(size_t k=0; k < yz; ++k)
        f[k]=0.0;
    }

    if(ny % 2 == 0) {
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(size_t i=0; i < nx; ++i) {
        size_t iyz=i*yz;
        for(size_t k=0; k < nzp; ++k)
          f[iyz+k]=0.0;
      }
    }

    if(nz % 2 == 0)
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(size_t i=0; i < nx; ++i)
        for(size_t j=0; j < ny; ++j)
          f[i*yz+(j+1)*nzp-1]=0.0;
  }
};

// Compute the real two-dimensional inverse Fourier transform of the
// nx*ny*(nz/2+1) Complex values corresponding to the spectral values in the
// half-plane kz >= 0, using phase sign +1.
// Before calling fft(), the array in must be allocated as
// Complex[nx*ny*(nz+1)/2] and the array out must be allocated as
// double[nx*ny*nz]. The arrays in and out may coincide,
// allocated as Complex[nx*ny*(nz/2+1)].
//
// Out-of-place usage (input destroyed):
//
//   crfft3d Backward(nx,ny,nz,in,out);
//   Backward.fft(in,out);      // Origin of Fourier domain at (0,0)
//   Backward.fft0(in,out);     // Origin of Fourier domain at (nx/2,ny/2,0)
//
// In-place usage:
//
//   crfft3d Backward(nx,ny,nz);
//   Backward.fft(in);          // Origin of Fourier domain at (0,0)
//   Backward.fft0(in);         // Origin of Fourier domain at (nx/2,ny/2,0)
//
// Notes:
//   in contains the upper-half portion (kz >= 0) of the Complex transform;
//   out contains the nx*ny*nz real values stored as a Complex array.
//
class crfft3d : public fftw {
  size_t nx;
  size_t ny;
  size_t nz;
public:
  crfft3d(size_t nx, size_t ny, size_t nz, double *out=NULL,
          size_t threads=maxthreads)
    : fftw(2*nx*ny*(nz/2+1),1,threads,nx*ny*nz), nx(nx), ny(ny), nz(nz)
  {Setup(out);}

  crfft3d(size_t nx, size_t ny, size_t nz, Complex *in,
          double *out=NULL, size_t threads=maxthreads)
    : fftw(nx*ny*(realsize(nz,Inplace(in,out))),1,threads,nx*ny*nz),
      nx(nx), ny(ny), nz(nz) {Setup(in,out);}

  fftw_plan Plan(Complex *in, Complex *out) {
    return fftw_plan_dft_c2r_3d(nx,ny,nz,(fftw_complex *) in,(double *) out,
                                effort);
  }

  void Execute(Complex *in, Complex *out, bool shift=false) {
    fftw_execute_dft_c2r(plan,(fftw_complex *) in,(double *) out);
    if(shift) {
      if(inplace) Shift(out,nx,ny,nz,threads);
      else Shift((double *) out,nx,ny,nz,threads);
    }
  }

  // Set Nyquist modes of even shifted transforms to zero.
  void deNyquist(Complex *f) {
    size_t nzp=nz/2+1;
    size_t yz=ny*nzp;
    if(nx % 2 == 0) {
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(size_t k=0; k < yz; ++k)
        f[k]=0.0;
    }

    if(ny % 2 == 0) {
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(size_t i=0; i < nx; ++i) {
        size_t iyz=i*yz;
        for(size_t k=0; k < nzp; ++k)
          f[iyz+k]=0.0;
      }
    }

    if(nz % 2 == 0)
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(size_t i=0; i < nx; ++i)
        for(size_t j=0; j < ny; ++j)
          f[i*yz+(j+1)*nzp-1]=0.0;
  }
};

}

#endif
