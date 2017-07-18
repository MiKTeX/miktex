/* Fast Fourier transform C++ header class for the FFTW3 Library
   Copyright (C) 2004-16
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

#define __FFTWPP_H_VERSION__ 2.02

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <fftw3.h>
#include <cerrno>
#include <map>

#ifndef _OPENMP
#ifndef FFTWPP_SINGLE_THREAD
#define FFTWPP_SINGLE_THREAD
#endif
#endif

#ifndef FFTWPP_SINGLE_THREAD
#include <omp.h>
#endif

inline int get_thread_num() 
{
#ifdef FFTWPP_SINGLE_THREAD
  return 0;
#else
  return omp_get_thread_num();
#endif  
}

inline int get_max_threads() 
{
#ifdef FFTWPP_SINGLE_THREAD
  return 1;
#else
  return omp_get_max_threads();
#endif  
}

#ifndef FFTWPP_SINGLE_THREAD
#define PARALLEL(code)                                  \
  if(threads > 1) {                                     \
    _Pragma("omp parallel for num_threads(threads)")    \
      code                                              \
      } else {                                          \
    code                                                \
      }
#else
#define PARALLEL(code)                          \
  {                                             \
    code                                        \
  }
#endif

#ifndef __Complex_h__
#include <complex>
typedef std::complex<double> Complex;
#endif

#include "seconds.h"
#include "statistics.h"
#include "align.h"

namespace fftwpp {

// Obsolete names:
#define FFTWComplex ComplexAlign
#define FFTWdouble doubleAlign
#define FFTWdelete deleteAlign

class fftw;

extern "C" fftw_plan Planner(fftw *F, Complex *in, Complex *out);
void LoadWisdom();
void SaveWisdom();

extern const char *inout;

struct threaddata {
  unsigned int threads;
  double mean;
  double stdev;
  threaddata() : threads(0), mean(0.0), stdev(0.0) {}
  threaddata(unsigned int threads, double mean, double stdev) :
    threads(threads), mean(mean), stdev(stdev) {}
};

class fftw;

class ThreadBase
{
protected:
  unsigned int threads;
  unsigned int innerthreads;
public:  
  ThreadBase();
  ThreadBase(unsigned int threads) : threads(threads) {}
  void Threads(unsigned int nthreads) {threads=nthreads;}
  unsigned int Threads() {return threads;}
  
  void multithread(unsigned int nx) {
    if(nx >= threads) {
      innerthreads=1;
    } else {
      innerthreads=threads;
      threads=1;
    }
  }
};

// Base clase for fft routines
//
class fftw : public ThreadBase {
protected:
  unsigned int doubles; // number of double precision values in dataset
  int sign;
  unsigned int threads;
  double norm;

  fftw_plan plan;
  bool inplace;
  
  unsigned int Dist(unsigned int n, size_t stride, size_t dist) {
    return dist ? dist : ((stride == 1) ? n : 1);
  }
  
  unsigned int realsize(unsigned int n, Complex *in, Complex *out=NULL) {
    return (!out || in == out) ? 2*(n/2+1) : n;
  }
  
  unsigned int realsize(unsigned int n, Complex *in, double *out) {
    return realsize(n,in,(Complex *) out);
  }
  
  unsigned int realsize(unsigned int n, double *in, Complex *out) {
    return realsize(n,(Complex *) in,out);
  }
  
  static const double twopi;
  
public:
  static unsigned int effort;
  static unsigned int maxthreads;
  static double testseconds;
  static const char *WisdomName;
  static fftw_plan (*planner)(fftw *f, Complex *in, Complex *out);
  
  virtual unsigned int Threads() {return threads;}
  
  static const char *oddshift;
  
  // Inplace shift of Fourier origin to (nx/2,0) for even nx.
  static void Shift(Complex *data, unsigned int nx, unsigned int ny,
                    unsigned int threads) {
    unsigned int nyp=ny/2+1;
    unsigned int stop=nx*nyp;
    if(nx % 2 == 0) {
      unsigned int inc=2*nyp;
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(unsigned int i=nyp; i < stop; i += inc) {
        Complex *p=data+i;
        for(unsigned int j=0; j < nyp; j++) p[j]=-p[j];
      }
    } else {
      std::cerr << oddshift << std::endl;
      exit(1);
    }
  }

  // Out-of-place shift of Fourier origin to (nx/2,0) for even nx.
  static void Shift(double *data, unsigned int nx, unsigned int ny,
                    unsigned int threads) {
    if(nx % 2 == 0) {
      unsigned int stop=nx*ny;
      unsigned int inc=2*ny;
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(unsigned int i=ny; i < stop; i += inc) {
        double *p=data+i;
        for(unsigned int j=0; j < ny; j++) p[j]=-p[j];
      }
    } else {
      std::cerr << oddshift << std::endl;
      exit(1);
    }
  }

  // Inplace shift of Fourier origin to (nx/2,ny/2,0) for even nx and ny.
  static void Shift(Complex *data, unsigned int nx, unsigned int ny,
                    unsigned int nz, unsigned int threads) {
    unsigned int nzp=nz/2+1;
    unsigned int nyzp=ny*nzp;
    if(nx % 2 == 0 && ny % 2 == 0) {
      unsigned int pinc=2*nzp;
      Complex *pstop=data;
      Complex *p=data;
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(unsigned i=0; i < nx; i++) {
        if(i % 2) p -= nzp;
        else p += nzp;
        pstop += nyzp;
        for(; p < pstop; p += pinc) {
          for(unsigned int k=0; k < nzp; k++) p[k]=-p[k];
        }
      }
    } else {
      std::cerr << oddshift << " or odd ny" << std::endl;
      exit(1);
    }
  }

  // Out-of-place shift of Fourier origin to (nx/2,ny/2,0) for even nx and ny.
  static void Shift(double *data, unsigned int nx, unsigned int ny,
                    unsigned int nz, unsigned int threads) {
    unsigned int nyz=ny*nz;
    if(nx % 2 == 0 && ny % 2 == 0) {
      unsigned int pinc=2*nz;
      double *pstop=data;
      double *p=data;
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(unsigned i=0; i < nx; i++) {
        if(i % 2) p -= nz;
        else p += nz;
        pstop += nyz;
        for(; p < pstop; p += pinc) {
          for(unsigned int k=0; k < nz; k++) p[k]=-p[k];
        }
      }
    } else {
      std::cerr << oddshift << " or odd ny" << std::endl;
      exit(1);
    }
  }
  
  fftw() : plan(NULL) {}
  fftw(unsigned int doubles, int sign, unsigned int threads,
       unsigned int n=0) :
    doubles(doubles), sign(sign), threads(threads), 
    norm(1.0/(n ? n : doubles/2)), plan(NULL) {
#ifndef FFTWPP_SINGLE_THREAD
    fftw_init_threads();
#endif      
  }
  
  virtual ~fftw() {
    if(plan) fftw_destroy_plan(plan);
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
  
  static void planThreads(unsigned int threads) {
#ifndef FFTWPP_SINGLE_THREAD
    omp_set_num_threads(threads);
    fftw_plan_with_nthreads(threads);
#endif    
  }
  
  threaddata time(fftw_plan plan1, fftw_plan planT, Complex *in, Complex *out,
                  unsigned int Threads) {
    utils::statistics S,ST;
    double stop=utils::totalseconds()+testseconds;
    threads=1;
    plan=plan1;
    fft(in,out);
    threads=Threads;
    plan=planT;
    fft(in,out);
    unsigned int N=1;
    for(;;) {
      double t0=utils::totalseconds();
      threads=1;
      plan=plan1;
      for(unsigned int i=0; i < N; ++i)
        fft(in,out);
      double t1=utils::totalseconds();
      threads=Threads;
      plan=planT;
      for(unsigned int i=0; i < N; ++i)
        fft(in,out);
      double t=utils::totalseconds();
      S.add(t1-t0);
      ST.add(t-t1);
      if(S.mean() < 100.0/CLOCKS_PER_SEC) N *= 2;
      if(S.count() >= 10) {
        double error=S.stdev();
        double diff=ST.mean()-S.mean();
        if(diff >= 0.0 || t > stop) {
          threads=1;
          plan=plan1;
          fftw_destroy_plan(planT);
          break;
        }
        if(diff < -error) {
          threads=Threads;
          fftw_destroy_plan(plan1);
          break;
        }
      }
    }
    return threaddata(threads,S.mean(),S.stdev());
  }
  
  virtual threaddata lookup(bool inplace, unsigned int threads) {
    return threaddata();
  }
  virtual void store(bool inplace, const threaddata& data) {}
  
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
  
  threaddata Setup(Complex *in, Complex *out=NULL) {
    bool alloc=!in;
    if(alloc) in=utils::ComplexAlign((doubles+1)/2);
    out=CheckAlign(in,out);
    inplace=(out==in);
    
    threaddata data;
    unsigned int Threads=threads;
    if(threads > 1) data=lookup(inplace,threads);
    threads=data.threads > 0 ? data.threads : 1;
    planThreads(threads);
    plan=(*planner)(this,in,out);
    if(!plan) noplan();
    
    fftw_plan planT;
    if(Threads > 1) {
      threads=Threads;
      planThreads(threads);
      planT=(*planner)(this,in,out);
    
      if(data.threads == 0) {
        if(planT)
          data=time(plan,planT,in,out,threads);
        else noplan();
        store(inplace,threaddata(threads,data.mean,data.stdev));
      }
    }
    
    if(alloc) Array::deleteAlign(in,(doubles+1)/2);
    return data;
  }
  
  threaddata Setup(Complex *in, double *out) {
    return Setup(in,(Complex *) out);
  }

  threaddata Setup(double *in, Complex *out=NULL) {
    return Setup((Complex *) in,out);
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
    unsigned int stop=doubles/2;
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
    for(unsigned int i=0; i < stop; i++) out[i] *= norm;
  }

  void Normalize(double *out) {
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
    for(unsigned int i=0; i < doubles; i++) out[i] *= norm;
  }
  
  virtual void fftNormalized(Complex *in, Complex *out=NULL, bool shift=false) 
  {
    out=Setout(in,out);
    Execute(in,out,shift);
    Normalize(out);
  }
  
  void fftNormalized(Complex *in, double *out, bool shift=false) {
    out=(double *) Setout(in,(Complex *) out);
    Execute(in,(Complex *) out,shift);
    Normalize(out);
  }
  
  void fftNormalized(double *in, Complex *out, bool shift=false) {
    fftNormalized((Complex *) in,out,shift);
  }
  
  template<class I, class O>
  void fft0Normalized(I in, O out) {
    fftNormalized(in,out,true);
  }
  
  template<class I, class O>
  void fftNormalized(unsigned int nx, unsigned int M, size_t ostride,
                     size_t odist, I *in, O *out=NULL, bool shift=false) {
    out=(O *) Setout((Complex *) in,(Complex *) out);
    Execute((Complex *) in,(Complex *) out,shift);

    unsigned int stop=nx*ostride;
    O *outMdist=out+M*odist;
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
    for(unsigned int i=0; i < stop; i += ostride) {
      O *pstop=outMdist+i;
      for(O *p=out+i; p < pstop; p += odist) {
        *p *= norm;
      }
    }
  }

}; // class fftw

class Transpose {
  fftw_plan plan;
  fftw_plan plan2;
  unsigned int a,b;
  unsigned int nlength,mlength;
  unsigned int ilast,jlast;
  unsigned int rows,cols;
  unsigned int threads;
  bool inplace;
  unsigned int size;
public:
  template<class T>
  Transpose(unsigned int rows, unsigned int cols, unsigned int length,
            T *in, T *out=NULL, unsigned int threads=fftw::maxthreads) :
    rows(rows), cols(cols), threads(threads) {
    size=sizeof(T);
    if(size % sizeof(double) != 0) {
      std::cerr << "ERROR: Transpose is not implemented for type of size " 
                << size;
      exit(1);
    }
    if(rows == 0 || cols == 0) return;
    size /= sizeof(double);
    length *= size;

    if(!out) out=in;
    inplace=(out==in);
    if(inplace) threads=1; // TODO: Generalize to inplace
    
    fftw_iodim dims[3];

    a=std::min(rows,threads);
    b=std::min(cols,threads/a);

    unsigned int n=utils::ceilquotient(rows,a);
    unsigned int m=utils::ceilquotient(cols,b);
    
    // If rows <= threads then a=rows and n=1.
    // If rows >= threads then b=1 and m=cols.
    
    nlength=n*length;
    mlength=m*length;
    
    dims[0].n=n; 
    dims[0].is=cols*length;
    dims[0].os=length;
    
    dims[1].n=m; 
    dims[1].is=length;
    dims[1].os=rows*length;

    dims[2].n=length;
    dims[2].is=1;
    dims[2].os=1;

    fftw::planThreads(1);

    // A plan with rank=0 is a transpose.
    plan=fftw_plan_guru_r2r(0,NULL,3,dims,(double *) in,(double *) out,
                            NULL,fftw::effort);
    plan2=NULL;
    ilast=a;
    jlast=b;
    
    if(n*a > rows) { // Only happens when rows > threads.
      a=utils::ceilquotient(rows,n);
      ilast=a-1;
      dims[0].n=rows-n*ilast;
      plan2=fftw_plan_guru_r2r(0,NULL,3,dims,(double *) in,(double *) out,
                               NULL,fftw::effort);
    } else { // Only happens when rows < threads.
      if(m*b > cols) {
        b=utils::ceilquotient(cols,m);
        jlast=b-1;
        dims[1].n=cols-m*jlast;
        plan2=fftw_plan_guru_r2r(0,NULL,3,dims,(double *) in,(double *) out,
                                 NULL,fftw::effort);
      }
    }
  }

  ~Transpose() {
    if(plan) fftw_destroy_plan(plan);
    if(plan2) fftw_destroy_plan(plan2);
  }
  
  template<class T>
  void transpose(T *in, T *out=NULL) {
    if(!out) out=in;
    if(inplace ^ (out == in)) {
      std::cerr << "ERROR: Transpose " << inout << std::endl;
      exit(1);
    }
#ifndef FFTWPP_SINGLE_THREAD
    if(a > 1) {
      if(b > 1) {
        int A=a, B=b;
#pragma omp parallel for num_threads(A)
        for(unsigned int i=0; i < a; ++i) {
          unsigned int I=i*nlength;
#pragma omp parallel for num_threads(B)
          for(unsigned int j=0; j < b; ++j) {
            unsigned int J=j*mlength;
            fftw_execute_r2r((i < ilast && j < jlast) ? plan : plan2,
                             (double *) in+cols*I+J,
                             (double *) out+rows*J+I);
          }
        }
      } else {
        int A=a;
#pragma omp parallel for num_threads(A)
        for(unsigned int i=0; i < a; ++i) {
          unsigned int I=i*nlength;
          fftw_execute_r2r(i < ilast ? plan : plan2,
                           (double *) in+cols*I,(double *) out+I);
        }
      }
    } else if(b > 1) {
      int B=b;
#pragma omp parallel for num_threads(B)
      for(unsigned int j=0; j < b; ++j) {
        unsigned int J=j*mlength;
        fftw_execute_r2r(j < jlast ? plan : plan2,
                         (double *) in+J,(double *) out+rows*J);
      }
    } else
#endif
      fftw_execute_r2r(plan,(double *) in,(double*) out);
  }
};

template<class T, class L>
class Threadtable {
public:
  typedef std::map<T,threaddata,L> Table;

  threaddata Lookup(Table& table, T key) {
    typename Table::iterator p=table.find(key);
    return p == table.end() ? threaddata() : p->second;
  }
  
  void Store(Table& threadtable, T key, const threaddata& data) {
    threadtable[key]=data;
  }
};

struct keytype1 {
  unsigned int nx;
  unsigned int threads;
  bool inplace;
  keytype1(unsigned int nx, unsigned int threads, bool inplace) : 
    nx(nx), threads(threads), inplace(inplace) {}
};
  
struct keyless1 {
  bool operator()(const keytype1& a, const keytype1& b) const {
    return a.nx < b.nx || (a.nx == b.nx &&
                           (a.threads < b.threads || (a.threads == b.threads &&
                                                      a.inplace < b.inplace)));
  }
};

struct keytype2 {
  unsigned int nx;
  unsigned int ny;
  unsigned int threads;
  bool inplace;
  keytype2(unsigned int nx, unsigned int ny, unsigned int threads,
           bool inplace) : 
    nx(nx), ny(ny), threads(threads), inplace(inplace) {}
};
  
struct keyless2 {
  bool operator()(const keytype2& a, const keytype2& b) const {
    return a.nx < b.nx || (a.nx == b.nx &&
                           (a.ny < b.ny || (a.ny == b.ny &&
                                            (a.threads < b.threads ||
                                             (a.threads == b.threads &&
                                              a.inplace < b.inplace)))));
  }
};

struct keytype3 {
  unsigned int nx;
  unsigned int ny;
  unsigned int nz;
  unsigned int threads;
  bool inplace;
  keytype3(unsigned int nx, unsigned int ny, unsigned int nz,
           unsigned int threads, bool inplace) : 
    nx(nx), ny(ny), nz(nz), threads(threads), inplace(inplace) {}
};
  
struct keyless3 {
  bool operator()(const keytype3& a, const keytype3& b) const {
    return a.nx < b.nx || (a.nx == b.nx && 
                           (a.ny < b.ny || (a.ny == b.ny &&
                                            (a.nz < b.nz ||
                                             (a.nz == b.nz &&
                                              (a.threads < b.threads ||
                                               (a.threads == b.threads &&
                                                a.inplace < b.inplace)))))));
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
class fft1d : public fftw, public Threadtable<keytype1,keyless1> {
  unsigned int nx;
  static Table threadtable;
public:  
  fft1d(unsigned int nx, int sign, Complex *in=NULL, Complex *out=NULL,
        unsigned int threads=maxthreads)
    : fftw(2*nx,sign,threads), nx(nx) {Setup(in,out);} 
  
#ifdef __Array_h__
  fft1d(int sign, const Array::array1<Complex>& in,
        const Array::array1<Complex>& out=Array::NULL1,
        unsigned int threads=maxthreads) 
    : fftw(2*in.Nx(),sign,threads), nx(in.Nx()) {Setup(in,out);} 
#endif  
  
  threaddata lookup(bool inplace, unsigned int threads) {
    return this->Lookup(threadtable,keytype1(nx,threads,inplace));
  }
  void store(bool inplace, const threaddata& data) {
    this->Store(threadtable,keytype1(nx,data.threads,inplace),data);
  }
  
  fftw_plan Plan(Complex *in, Complex *out) {
    return fftw_plan_dft_1d(nx,(fftw_complex *) in,(fftw_complex *) out,
                            sign,effort);
  }
};
  
template<class I, class O>
class fftwblock : public virtual fftw {
public:
  int nx;
  unsigned int M;
  size_t istride,ostride;
  size_t idist,odist;
  fftw_plan plan1,plan2;
  unsigned int T,Q,R;
  fftwblock(unsigned int nx, unsigned int M,
            size_t istride, size_t ostride, size_t idist, size_t odist,
            Complex *in, Complex *out, unsigned int Threads)
    : fftw(), nx(nx), M(M), istride(istride), ostride(ostride),
      idist(Dist(nx,istride,idist)), odist(Dist(nx,ostride,odist)),
      plan1(NULL), plan2(NULL) {
    T=1;
    Q=M;
    R=0;
    
    threaddata S1=Setup(in,out);
    fftw_plan planT1=plan;
    
    if(Threads > 1) {
      T=std::min(M,Threads);
      Q=T > 0 ? M/T : 0;
      R=M-Q*T;
      threads=Threads;
      threaddata ST=Setup(in,out);
    
      if(R > 0 && threads == 1 && plan1 != plan2) {
        fftw_destroy_plan(plan2);
        plan2=plan1;
      }

      if(ST.mean > S1.mean-S1.stdev) { // Use FFTW's multi-threading
        fftw_destroy_plan(plan);
        if(R > 0) {
          fftw_destroy_plan(plan2);
          plan2=NULL;
        }
        T=1;
        Q=M;
        R=0;
        plan=planT1;
        threads=S1.threads;
      } else {                         // Do the multi-threading ourselves
        fftw_destroy_plan(planT1);
        threads=ST.threads;
      }
    }
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
      if(threads == 1) plan1=plan2;
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
      unsigned int extra=T-R;
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(T)
#endif
      for(unsigned int i=0; i < T; ++i) {
        unsigned int iQ=i*Q;
        if(i < extra)
          Execute(plan,(I *) in+iQ*idist,(O *) out+iQ*odist);
        else {
          unsigned int offset=iQ+i-extra;
          Execute(plan2,(I *) in+offset*idist,(O *) out+offset*odist);
        }
      }
    }
  }
  
  unsigned int Threads() {return std::max(T,threads);}
  
  ~fftwblock() {
    if(plan2) fftw_destroy_plan(plan2);
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
// In-place usage:
//
//   mfft1d Forward(n,-1,M,stride,dist);
//   Forward.fft(in);
//
// Notes:
//   stride is the spacing between the elements of each Complex vector;
//   dist is the spacing between the first elements of the vectors.
//
//
class mfft1d : public fftwblock<fftw_complex,fftw_complex>,
               public Threadtable<keytype3,keyless3> {
  static Table threadtable;
public:  
  mfft1d(unsigned int nx, int sign, unsigned int M=1, size_t stride=1,
         size_t dist=0, Complex *in=NULL, Complex *out=NULL,
         unsigned int threads=maxthreads) :
    fftw(2*((nx-1)*stride+(M-1)*Dist(nx,stride,dist)+1),sign,threads,nx),
    fftwblock<fftw_complex,fftw_complex>
    (nx,M,stride,stride,dist,dist,in,out,threads) {} 
  
  mfft1d(unsigned int nx, int sign, unsigned int M,
         size_t istride, size_t ostride, size_t idist, size_t odist,
         Complex *in=NULL, Complex *out=NULL, unsigned int threads=maxthreads):
    fftw(std::max(2*((nx-1)*istride+(M-1)*Dist(nx,istride,idist)+1),
                  2*((nx-1)*ostride+(M-1)*Dist(nx,ostride,odist)+1)),sign,
         threads, nx),
    fftwblock<fftw_complex,fftw_complex>(nx,M,istride,ostride,idist,odist,in,
                                         out,threads) {} 
  
  threaddata lookup(bool inplace, unsigned int threads) {
    return Lookup(threadtable,keytype3(nx,Q,R,threads,inplace));
  }
  void store(bool inplace, const threaddata& data) {
    Store(threadtable,keytype3(nx,Q,R,data.threads,inplace),data);
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
class rcfft1d : public fftw, public Threadtable<keytype1,keyless1> {
  unsigned int nx;
  static Table threadtable;
public:  
  rcfft1d(unsigned int nx, Complex *out=NULL, unsigned int threads=maxthreads) 
    : fftw(2*(nx/2+1),-1,threads,nx), nx(nx) {Setup(out,(double*) NULL);}
  
  rcfft1d(unsigned int nx, double *in, Complex *out=NULL,
          unsigned int threads=maxthreads)  
    : fftw(2*(nx/2+1),-1,threads,nx), nx(nx) {Setup(in,out);}
  
  threaddata lookup(bool inplace, unsigned int threads) {
    return Lookup(threadtable,keytype1(nx,threads,inplace));
  }
  void store(bool inplace, const threaddata& data) {
    Store(threadtable,keytype1(nx,data.threads,inplace),data);
  }
  
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
class crfft1d : public fftw, public Threadtable<keytype1,keyless1> {
  unsigned int nx;
  static Table threadtable;
public:  
  crfft1d(unsigned int nx, double *out=NULL, unsigned int threads=maxthreads) 
    : fftw(2*(nx/2+1),1,threads,nx), nx(nx) {Setup(out);} 
  
  crfft1d(unsigned int nx, Complex *in, double *out=NULL, 
          unsigned int threads=maxthreads)
    : fftw(realsize(nx,in,out),1,threads,nx), nx(nx) {Setup(in,out);} 
  
  threaddata lookup(bool inplace, unsigned int threads) {
    return Lookup(threadtable,keytype1(nx,threads,inplace));
  }
  void store(bool inplace, const threaddata& data) {
    Store(threadtable,keytype1(nx,data.threads,inplace),data);
  }
  
  fftw_plan Plan(Complex *in, Complex *out) {
    return fftw_plan_dft_c2r_1d(nx,(fftw_complex *) in,(double *) out,effort);
  }
  
  void Execute(Complex *in, Complex *out, bool=false) {
    fftw_execute_dft_c2r(plan,(fftw_complex *) in,(double *) out);
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
class mrcfft1d : public fftwblock<double,fftw_complex>,
                 public Threadtable<keytype3,keyless3> {
  static Table threadtable;
public:
  mrcfft1d(unsigned int nx, unsigned int M,
           size_t istride, size_t ostride,
           size_t idist, size_t odist,
           double *in=NULL, Complex *out=NULL,
           unsigned int threads=maxthreads) 
    : fftw(std::max((realsize(nx,in,out)-2)*istride+(M-1)*idist+2,
                    2*(nx/2*ostride+(M-1)*odist+1)),-1,threads,nx),
      fftwblock<double,fftw_complex>
    (nx,M,istride,ostride,idist,odist,(Complex *) in,out,threads) {}
  
  threaddata lookup(bool inplace, unsigned int threads) {
    return Lookup(threadtable,keytype3(nx,Q,R,threads,inplace));
  }
  
  void store(bool inplace, const threaddata& data) {
    Store(threadtable,keytype3(nx,Q,R,data.threads,inplace),data);
  }
  
  void fftNormalized(double *in, Complex *out=NULL) {
    fftw::fftNormalized<double,Complex>(nx/2+1,M,ostride,odist,in,out,false);
  }
  
  void fft0Normalized(double *in, Complex *out=NULL) {
    fftw::fftNormalized<double,Complex>(nx/2+1,M,ostride,odist,in,out,true);
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
class mcrfft1d : public fftwblock<fftw_complex,double>,
                 public Threadtable<keytype3,keyless3> {
  static Table threadtable;
public:
  mcrfft1d(unsigned int nx, unsigned int M, size_t istride, size_t ostride,
           size_t idist, size_t odist, Complex *in=NULL, double *out=NULL,
           unsigned int threads=maxthreads) 
    : fftw(std::max(2*(nx/2*istride+(M-1)*idist+1),
                    (realsize(nx,in,out)-2)*ostride+(M-1)*odist+2),1,threads,nx),
      fftwblock<fftw_complex,double>
    (nx,M,istride,ostride,idist,odist,in,(Complex *) out,threads) {}
  
  threaddata lookup(bool inplace, unsigned int threads) {
    return Lookup(threadtable,keytype3(nx,Q,R,threads,inplace));
  }
  void store(bool inplace, const threaddata& data) {
    Store(threadtable,keytype3(nx,Q,R,data.threads,inplace),data);
  }
  void fftNormalized(Complex *in, double *out=NULL) {
    fftw::fftNormalized<Complex,double>(nx,M,ostride,odist,in,out,false);
  }
  void fft0Normalized(Complex *in, double *out=NULL) {
    fftw::fftNormalized<Complex,double>(nx,M,ostride,odist,in,out,true);
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
class fft2d : public fftw, public Threadtable<keytype2,keyless2> {
  unsigned int nx;
  unsigned int ny;
  static Table threadtable;
public:  
  fft2d(unsigned int nx, unsigned int ny, int sign, Complex *in=NULL,
        Complex *out=NULL, unsigned int threads=maxthreads) 
    : fftw(2*nx*ny,sign,threads), nx(nx), ny(ny) {Setup(in,out);} 
  
#ifdef __Array_h__
  fft2d(int sign, const Array::array2<Complex>& in,
        const Array::array2<Complex>& out=Array::NULL2, 
        unsigned int threads=maxthreads) 
    : fftw(2*in.Size(),sign,threads), nx(in.Nx()), ny(in.Ny()) {
    Setup(in,out);
  }
#endif  
  
  threaddata lookup(bool inplace, unsigned int threads) {
    return this->Lookup(threadtable,keytype2(nx,ny,threads,inplace));
  }
  void store(bool inplace, const threaddata& data) {
    this->Store(threadtable,keytype2(nx,ny,data.threads,inplace),data);
  }
  
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
  unsigned int nx;
  unsigned int ny;
public:  
  rcfft2d(unsigned int nx, unsigned int ny, Complex *out=NULL,
          unsigned int threads=maxthreads) 
    : fftw(2*nx*(ny/2+1),-1,threads,nx*ny), nx(nx), ny(ny) {Setup(out);} 
  
  rcfft2d(unsigned int nx, unsigned int ny, double *in, Complex *out=NULL,
          unsigned int threads=maxthreads) 
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
    unsigned int nyp=ny/2+1;
    if(nx % 2 == 0)
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(unsigned int j=0; j < nyp; ++j)
        f[j]=0.0;
    if(ny % 2 == 0)
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(unsigned int i=0; i < nx; ++i)
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
  unsigned int nx;
  unsigned int ny;
public:  
  crfft2d(unsigned int nx, unsigned int ny, double *out=NULL,
          unsigned int threads=maxthreads) :
    fftw(2*nx*(ny/2+1),1,threads,nx*ny), nx(nx), ny(ny) {Setup(out);} 
  
  crfft2d(unsigned int nx, unsigned int ny, Complex *in, double *out=NULL,
          unsigned int threads=maxthreads)
    : fftw(nx*realsize(ny,in,out),1,threads,nx*ny), nx(nx), ny(ny) {
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
    unsigned int nyp=ny/2+1;
    if(nx % 2 == 0)
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(unsigned int j=0; j < nyp; ++j)
        f[j]=0.0;
    if(ny % 2 == 0)
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(unsigned int i=0; i < nx; ++i)
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
  unsigned int nx;
  unsigned int ny;
  unsigned int nz;
public:  
  fft3d(unsigned int nx, unsigned int ny, unsigned int nz,
        int sign, Complex *in=NULL, Complex *out=NULL,
        unsigned int threads=maxthreads) 
    : fftw(2*nx*ny*nz,sign,threads), nx(nx), ny(ny), nz(nz) {Setup(in,out);} 
  
#ifdef __Array_h__
  fft3d(int sign, const Array::array3<Complex>& in,
        const Array::array3<Complex>& out=Array::NULL3,
        unsigned int threads=maxthreads) 
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
  unsigned int nx;
  unsigned int ny;
  unsigned int nz;
public:  
  rcfft3d(unsigned int nx, unsigned int ny, unsigned int nz, Complex *out=NULL,
          unsigned int threads=maxthreads)
    : fftw(2*nx*ny*(nz/2+1),-1,threads,nx*ny*nz), nx(nx), ny(ny), nz(nz) {
    Setup(out);
  } 
  
  rcfft3d(unsigned int nx, unsigned int ny, unsigned int nz, double *in,
          Complex *out=NULL, unsigned int threads=maxthreads) 
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
    unsigned int nzp=nz/2+1;
    unsigned int yz=ny*nzp;
    if(nx % 2 == 0) {
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(unsigned int k=0; k < yz; ++k)
        f[k]=0.0;
    }
    
    if(ny % 2 == 0) {
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(unsigned int i=0; i < nx; ++i) {
        unsigned int iyz=i*yz;
        for(unsigned int k=0; k < nzp; ++k)
          f[iyz+k]=0.0;
      }
    }
        
    if(nz % 2 == 0)
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(unsigned int i=0; i < nx; ++i)
        for(unsigned int j=0; j < ny; ++j)
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
  unsigned int nx;
  unsigned int ny;
  unsigned int nz;
public:  
  crfft3d(unsigned int nx, unsigned int ny, unsigned int nz, double *out=NULL,
          unsigned int threads=maxthreads) 
    : fftw(2*nx*ny*(nz/2+1),1,threads,nx*ny*nz), nx(nx), ny(ny), nz(nz)
  {Setup(out);} 
  
  crfft3d(unsigned int nx, unsigned int ny, unsigned int nz, Complex *in,
          double *out=NULL, unsigned int threads=maxthreads) 
    : fftw(nx*ny*(realsize(nz,in,out)),1,threads,nx*ny*nz), nx(nx), ny(ny),
      nz(nz) {Setup(in,out);} 
  
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
    unsigned int nzp=nz/2+1;
    unsigned int yz=ny*nzp;
    if(nx % 2 == 0) {
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(unsigned int k=0; k < yz; ++k)
        f[k]=0.0;
    }
    
    if(ny % 2 == 0) {
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(unsigned int i=0; i < nx; ++i) {
        unsigned int iyz=i*yz;
        for(unsigned int k=0; k < nzp; ++k)
          f[iyz+k]=0.0;
      }
    }
        
    if(nz % 2 == 0)
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif
      for(unsigned int i=0; i < nx; ++i)
        for(unsigned int j=0; j < ny; ++j)
          f[i*yz+(j+1)*nzp-1]=0.0;
  }
};

}

#endif
