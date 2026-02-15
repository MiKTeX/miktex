#ifndef __align_h__
#define __align_h__ 1

#ifndef HAVE_POSIX_MEMALIGN

#ifdef __GLIBC_PREREQ
#if __GLIBC_PREREQ(2,3)
#define HAVE_POSIX_MEMALIGN
#endif
#else
#ifdef _POSIX_SOURCE
#define HAVE_POSIX_MEMALIGN
#endif
#endif

#endif

#ifdef __Array_h__

namespace Array {
static const array1<Complex> NULL1;
static const array2<Complex> NULL2;
static const array3<Complex> NULL3;
}

#else

#ifdef HAVE_POSIX_MEMALIGN
#ifdef _AIX
extern "C" int posix_memalign(void **memptr, size_t alignment, size_t size);
#endif
#else
namespace Array {

// Adapted from FFTW aligned malloc/free.  Assumes that malloc is at least
// sizeof(void*)-aligned. Allocated memory must be freed with free0.
inline int posix_memalign0(void **memptr, size_t alignment, size_t size)
{
  if(alignment % sizeof (void *) != 0 || (alignment & (alignment - 1)) != 0)
    return EINVAL;
  void *p0=malloc(size+alignment);
  if(!p0) return ENOMEM;
  void *p=(void *)(((size_t) p0+alignment)&~(alignment-1));
  *((void **) p-1)=p0;
  *memptr=p;
  return 0;
}

inline void free0(void *p)
{
  if(p) free(*((void **) p-1));
}

}
#endif

namespace Array {

template<class T>
inline void newAlign(T *&v, size_t len, size_t align)
{
  void *mem=NULL;
  const char *invalid="Invalid alignment requested";
  const char *nomem="Memory limits exceeded";
#ifdef HAVE_POSIX_MEMALIGN
  int rc=posix_memalign(&mem,align,len*sizeof(T));
#else
  int rc=posix_memalign0(&mem,align,len*sizeof(T));
#endif
  if(rc == EINVAL) std::cerr << invalid << std::endl;
  if(rc == ENOMEM) std::cerr << nomem << std::endl;
  v=(T *) mem;
  for(size_t i=0; i < len; i++) new(v+i) T;
}

template<class T>
inline void deleteAlign(T *v, size_t len)
{
  for(size_t i=len; i-- > 0;) v[i].~T();
#ifdef HAVE_POSIX_MEMALIGN
  free(v);
#else
  free0(v);
#endif
}
}

#endif

namespace utils {

extern size_t ALIGNMENT;

inline size_t ceilquotient(size_t a, size_t b)
{
  return (a+b-1)/b;
}

inline Complex *ComplexAlign(size_t size)
{
  if(size == 0) return NULL;
  Complex *v;
  Array::newAlign(v,size,ALIGNMENT);
  return v;
}

// Return a contiguous array v of n aligned buffers of length size.
// Deallocate with deleteAlign(v[0]); delete [] v;
inline Complex **ComplexAlign(size_t n, size_t size)
{
  if(n == 0 || size == 0) return NULL;
  Complex **v=new Complex*[n];
  size_t Size=ALIGNMENT*ceilquotient(size,ALIGNMENT);
  Complex *B=ComplexAlign((n-1)*Size+size);
  for(size_t i=0; i < n; ++i)
    v[i]=B+i*Size;
  return v;
}

inline double *doubleAlign(size_t size)
{
  double *v;
  Array::newAlign(v,size,ALIGNMENT);
  return v;
}

// Return a contiguous array v of n aligned buffers of length size.
// Deallocate with deleteAlign(v[0]); delete [] v;
inline double **doubleAlign(size_t n, size_t size)
{
  if(n == 0 || size == 0) return NULL;
  double **v=new double*[n];
  size_t Size=ALIGNMENT*ceilquotient(size,ALIGNMENT);
  double *B=doubleAlign((n-1)*Size+size);
  for(size_t i=0; i < n; ++i)
    v[i]=B+i*Size;
  return v;
}

// Extend n*sizeof(Complex) to a multiple of ALIGNMENT
inline size_t align(size_t n)
{
  return ceilquotient(n*sizeof(Complex),ALIGNMENT)*ALIGNMENT/sizeof(Complex);
}

template<class T>
inline void deleteAlign(T *p)
{
#ifdef HAVE_POSIX_MEMALIGN
  free(p);
#else
  Array::free0(p);
#endif
}

}

#endif
