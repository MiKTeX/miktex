/*****
 * arrayop
 * John Bowman
 *
 * Array operations
 *****/
#ifndef ARRAYOP_H
#define ARRAYOP_H

#include "util.h"
#include "stack.h"
#include "array.h"
#include "types.h"
#include "fileio.h"
#include "callable.h"
#include "mathop.h"

namespace run {

using vm::pop;
using vm::read;
using vm::array;
using camp::tab;

vm::array *copyArray(vm::array *a);
vm::array *copyArray2(vm::array *a);
  
template<class T, class U, template <class S> class op>
void arrayOp(vm::stack *s)
{
  U b=pop<U>(s);
  array *a=pop<array*>(s);
  size_t size=checkArray(a);
  array *c=new array(size);
  for(size_t i=0; i < size; i++)
    (*c)[i]=op<T>()(read<T>(a,i),b,i);
  s->push(c);
}

template<class T, class U, template <class S> class op>
void opArray(vm::stack *s)
{
  array *a=pop<array*>(s);
  T b=pop<T>(s);
  size_t size=checkArray(a);
  array *c=new array(size);
  for(size_t i=0; i < size; i++)
    (*c)[i]=op<U>()(b,read<U>(a,i),i);
  s->push(c);
}

template<class T, template <class S> class op>
void arrayArrayOp(vm::stack *s)
{
  array *b=pop<array*>(s);
  array *a=pop<array*>(s);
  size_t size=checkArrays(a,b);
  array *c=new array(size);
  for(size_t i=0; i < size; i++)
    (*c)[i]=op<T>()(read<T>(a,i),read<T>(b,i),i);
  s->push(c);
}

template<class T>
void sumArray(vm::stack *s)
{
  array *a=pop<array*>(s);
  size_t size=checkArray(a);
  T sum=0;
  for(size_t i=0; i < size; i++)
    sum += read<T>(a,i);
  s->push(sum);
}

extern const char *arrayempty;
  
template<class T, template <class S> class op>
void binopArray(vm::stack *s)
{
  array *a=pop<array*>(s);
  size_t size=checkArray(a);
  if(size == 0) vm::error(arrayempty);
  T m=read<T>(a,0);
  for(size_t i=1; i < size; i++)
    m=op<T>()(m,read<T>(a,i));
  s->push(m);
}

template<class T, template <class S> class op>
void binopArray2(vm::stack *s)
{
  array *a=pop<array*>(s);
  size_t size=checkArray(a);
  bool empty=true;
  T m=0;
  for(size_t i=0; i < size; i++) {
    array *ai=read<array*>(a,i);
    size_t aisize=checkArray(ai);
    if(aisize) {
      if(empty) {
        m=read<T>(ai,0);
        empty=false;
      }
      for(size_t j=0; j < aisize; j++)
        m=op<T>()(m,read<T>(ai,j));
    }
  }
  if(empty) vm::error(arrayempty);
  s->push(m);
}

template<class T, template <class S> class op>
void binopArray3(vm::stack *s)
{
  array *a=pop<array*>(s);
  size_t size=checkArray(a);
  bool empty=true;
  T m=0;
  for(size_t i=0; i < size; i++) {
    array *ai=read<array*>(a,i);
    size_t aisize=checkArray(ai);
    for(size_t j=0; j < aisize; j++) {
      array *aij=read<array*>(ai,j);
      size_t aijsize=checkArray(aij);
      if(aijsize) {
        if(empty) {
          m=read<T>(aij,0);
          empty=false;
        }
        for(size_t k=0; k < aijsize; k++) {
          m=op<T>()(m,read<T>(aij,k));
        }
      }
    }
  }
  if(empty) vm::error(arrayempty);
  s->push(m);
}

template<class T, class U, template <class S> class op>
void array2Op(vm::stack *s)
{
  U b=pop<U>(s);
  array *a=pop<array*>(s);
  size_t size=checkArray(a);
  array *c=new array(size);
  for(size_t i=0; i < size; ++i) {
    array *ai=read<array*>(a,i);
    size_t aisize=checkArray(ai);
    array *ci=new array(aisize);
    (*c)[i]=ci;
    for(size_t j=0; j < aisize; j++)
      (*ci)[j]=op<T>()(read<T>(ai,j),b,0);
  }
  s->push(c);
}

template<class T, class U, template <class S> class op>
void opArray2(vm::stack *s)
{
  array *a=pop<array*>(s);
  T b=pop<T>(s);
  size_t size=checkArray(a);
  array *c=new array(size);
  for(size_t i=0; i < size; ++i) {
    array *ai=read<array*>(a,i);
    size_t aisize=checkArray(ai);
    array *ci=new array(aisize);
    (*c)[i]=ci;
    for(size_t j=0; j < aisize; j++)
      (*ci)[j]=op<U>()(read<U>(ai,j),b,0);
  }
  s->push(c);
}

template<class T, template <class S> class op>
void array2Array2Op(vm::stack *s)
{
  array *b=pop<array*>(s);
  array *a=pop<array*>(s);
  size_t size=checkArrays(a,b);
  array *c=new array(size);
  for(size_t i=0; i < size; ++i) {
    array *ai=read<array*>(a,i);
    array *bi=read<array*>(b,i);
    size_t aisize=checkArrays(ai,bi);
    array *ci=new array(aisize);
    (*c)[i]=ci;
    for(size_t j=0; j < aisize; j++)
      (*ci)[j]=op<T>()(read<T>(ai,j),read<T>(bi,j),0);
  }
  s->push(c);
}

template<class T>
bool Array2Equals(vm::stack *s)
{
  array *b=pop<array*>(s);
  array *a=pop<array*>(s);
  size_t n=checkArray(a);
  if(n != checkArray(b)) return false;
  if(n == 0) return true;
  size_t n0=checkArray(read<array*>(a,0));
  if(n0 != checkArray(read<array*>(b,0))) return false;
    
  for(size_t i=0; i < n; ++i) {
    array *ai=read<array*>(a,i);
    array *bi=read<array*>(b,i);
    for(size_t j=0; j < n0; ++j) {
      if(read<T>(ai,j) != read<T>(bi,j))
        return false;
    }
  }
  return true;
}

template<class T>
void array2Equals(vm::stack *s)
{
  s->push(Array2Equals<T>(s));
}

template<class T>
void array2NotEquals(vm::stack *s)
{
  s->push(!Array2Equals<T>(s));
}

template<class T>
void diagonal(vm::stack *s)
{
  array *a=pop<array*>(s);
  size_t n=checkArray(a);
  array *c=new array(n);
  for(size_t i=0; i < n; ++i) {
    array *ci=new array(n);
    (*c)[i]=ci;
    for(size_t j=0; j < i; ++j)
      (*ci)[j]=T();
    (*ci)[i]=read<T>(a,i);
    for(size_t j=i+1; j < n; ++j)
      (*ci)[j]=T();
  }
  s->push(c);
}

template<class T>
struct compare {
  bool operator() (const vm::item& a, const vm::item& b)
  {
    return vm::get<T>(a) < vm::get<T>(b);
  }
};

template<class T>
void sortArray(vm::stack *s)
{
  array *c=copyArray(pop<array*>(s));
  sort(c->begin(),c->end(),compare<T>());
  s->push(c);
}

template<class T>
struct compare2 {
  bool operator() (const vm::item& A, const vm::item& B)
  {
    array *a=vm::get<array*>(A);
    array *b=vm::get<array*>(B);
    size_t size=a->size();
    if(size != b->size()) return false;

    for(size_t j=0; j < size; j++) {
      if(read<T>(a,j) < read<T>(b,j)) return true;
      if(read<T>(a,j) > read<T>(b,j)) return false;
    }
    return false;
  }
};

// Sort the rows of a 2-dimensional array by the first column, breaking
// ties with successively higher columns.
template<class T>
void sortArray2(vm::stack *s)
{
  array *c=copyArray(pop<array*>(s));
  stable_sort(c->begin(),c->end(),compare2<T>());
  s->push(c);
}

// Search a sorted ordered array a of n elements for key, returning the index i
// if a[i] <= key < a[i+1], -1 if key is less than all elements of a, or
// n-1 if key is greater than or equal to the last element of a.
template<class T>
void searchArray(vm::stack *s)
{
  T key=pop<T>(s);
  array *a=pop<array*>(s);
  size_t size= a->size();
  if(size == 0 || key < read<T>(a,0)) {s->push(-1); return;}
  size_t u=size-1;
  if(key >= read<T>(a,u)) {s->push((Int) u); return;}
  size_t l=0;
        
  while (l < u) {
    size_t i=(l+u)/2;
    if(key < read<T>(a,i)) u=i;
    else if(key < read<T>(a,i+1)) {s->push((Int) i); return;}
    else l=i+1;
  }
  s->push(0);
}

extern string emptystring;
  
void writestring(vm::stack *s);
  
template<class T>
void write(vm::stack *s)
{
  array *a=pop<array*>(s);
  vm::callable *suffix=pop<vm::callable *>(s,NULL);
  T first=pop<T>(s);
  string S=pop<string>(s,emptystring);
  vm::item it=pop(s);
  bool defaultfile=isdefault(it);
  camp::ofile *f=defaultfile ? &camp::Stdout : vm::get<camp::ofile*>(it);
  if(!f->isOpen() || !f->enabled()) return;
  
  size_t size=checkArray(a);
  if(S != "") f->write(S);
  f->write(first);
  for(size_t i=0; i < size; ++i) {
    f->write(tab);
    f->write(read<T>(a,i));
  }
  if(f->text()) {
    if(suffix) {
      s->push(f);
      suffix->call(s);
    } else if(defaultfile) {
      try {
        f->writeline();
      } catch (quit&) {
      }
    }
  }
}

template<class T>
void writeArray(vm::stack *s)
{
  array *A=pop<array*>(s);
  array *a=pop<array*>(s);
  string S=pop<string>(s,emptystring);
  vm::item it=pop(s);
  bool defaultfile=isdefault(it);
  camp::ofile *f=defaultfile ? &camp::Stdout : vm::get<camp::ofile*>(it);
  if(!f->isOpen() || !f->enabled()) return;
  
  size_t asize=checkArray(a);
  size_t Asize=checkArray(A);
  if(f->Standard()) interact::lines=0;
  else if(!f->isOpen()) return;
  try {
    if(S != "") {f->write(S); f->writeline();}
  
    size_t i=0;
    bool cont=true;
    while(cont) {
      cont=false;
      bool first=true;
      if(i < asize) {
        vm::item& I=(*a)[i];
        if(defaultfile) cout << i << ":\t";
        if(!I.empty())
          f->write(vm::get<T>(I));
        cont=true;
        first=false;
      }
      unsigned count=0;
      for(size_t j=0; j < Asize; ++j) {
        array *Aj=read<array*>(A,j);
        size_t Ajsize=checkArray(Aj);
        if(i < Ajsize) {
          if(f->text()) {
            if(first && defaultfile) cout << i << ":\t";
            for(unsigned k=0; k <= count; ++k)
              f->write(tab);
            count=0;
          }
          vm::item& I=(*Aj)[i];
          if(!I.empty())
            f->write(vm::get<T>(I));
          cont=true;
          first=false;
        } else count++;
      }
      ++i;
      if(cont && f->text()) f->writeline();
    }
  } catch (quit&) {
  }
  f->flush();
}
  
template<class T>
void writeArray2(vm::stack *s)
{
  array *a=pop<array*>(s);
  vm::item it=pop(s);
  bool defaultfile=isdefault(it);
  camp::ofile *f=defaultfile ? &camp::Stdout : vm::get<camp::ofile*>(it);
  if(!f->isOpen() || !f->enabled()) return;
  
  size_t size=checkArray(a);
  if(f->Standard()) interact::lines=0;
  
  try {
    for(size_t i=0; i < size; i++) {
      vm::item& I=(*a)[i];
      if(!I.empty()) {
        array *ai=vm::get<array*>(I);
        size_t aisize=checkArray(ai);
        for(size_t j=0; j < aisize; j++) {
          if(j > 0 && f->text()) f->write(tab);
          vm::item& I=(*ai)[j];
          if(!I.empty())
            f->write(vm::get<T>(I));
        }
      }
      if(f->text()) f->writeline();
    }
  } catch (quit&) {
  }
  f->flush();
}

template<class T>
void writeArray3(vm::stack *s)
{
  array *a=pop<array*>(s);
  vm::item it=pop(s);
  bool defaultfile=isdefault(it);
  camp::ofile *f=defaultfile ? &camp::Stdout : vm::get<camp::ofile*>(it);
  if(!f->isOpen() || !f->enabled()) return;
  
  size_t size=checkArray(a);
  if(f->Standard()) interact::lines=0;
  
  try {
    for(size_t i=0; i < size;) {
      vm::item& I=(*a)[i];
      if(!I.empty()) {
        array *ai=vm::get<array*>(I);
        size_t aisize=checkArray(ai);
        for(size_t j=0; j < aisize; j++) {
          vm::item& I=(*ai)[j];
          if(!I.empty()) {
            array *aij=vm::get<array*>(I);
            size_t aijsize=checkArray(aij);
            for(size_t k=0; k < aijsize; k++) {
              if(k > 0 && f->text()) f->write(tab);
              vm::item& I=(*aij)[k];
              if(!I.empty())
                f->write(vm::get<T>(I));
            }
          }
          if(f->text()) f->writeline();
        }
      }
      ++i;
      if(i < size && f->text()) f->writeline();
    }
  } catch (quit&) {
  }
  f->flush();
}

template <class T, class S, T (*func)(S)>
void arrayFunc(vm::stack *s) 
{
  array *a=pop<array*>(s);
  size_t size=checkArray(a);
  array *c=new array(size);
  for(size_t i=0; i < size; i++)
    (*c)[i]=func(read<S>(a,i));
  s->push(c);
}

template <class T, class S, T (*func)(S)>
void arrayFunc2(vm::stack *s) 
{
  array *a=pop<array*>(s);
  size_t size=checkArray(a);
  array *c=new array(size);
  for(size_t i=0; i < size; ++i) {
    array *ai=read<array*>(a,i);
    size_t aisize=checkArray(ai);
    array *ci=new array(aisize);
    (*c)[i]=ci;
    for(size_t j=0; j < aisize; j++)
    (*ci)[j]=func(read<S>(ai,j));
  }
  s->push(c);
}

vm::array *Identity(Int n);
camp::triple operator *(const vm::array& a, const camp::triple& v);
double norm(double *a, size_t n);
double norm(camp::triple *a, size_t n);

inline size_t checkdimension(const vm::array *a, size_t dim)
{
  size_t size=checkArray(a);
  if(dim && size != dim) {
    ostringstream buf;
    buf << "array of length " << dim << " expected";
    vm::error(buf);
  }
  return size;
}

template<class T>
inline void copyArrayC(T* &dest, const vm::array *a, size_t dim=0,
                       GCPlacement placement=NoGC)
{
  size_t size=checkdimension(a,dim);
  dest=(placement == NoGC) ? new T[size] : new(placement) T[size];
  for(size_t i=0; i < size; i++) 
    dest[i]=vm::read<T>(a,i);
}

template<class T, class A>
inline void copyArrayC(T* &dest, const vm::array *a, T (*cast)(A),
                       size_t dim=0, GCPlacement placement=NoGC)
{
  size_t size=checkdimension(a,dim);
  dest=(placement == NoGC) ? new T[size] : new(placement) T[size];
  for(size_t i=0; i < size; i++) 
    dest[i]=cast(vm::read<A>(a,i));
}

template<typename T>
inline vm::array* copyCArray(const size_t n, const T* p)
{
  vm::array* a = new vm::array(n);
  for(size_t i=0; i < n; ++i) (*a)[i] = p[i];
  return a;
}

template<class T>
inline void copyArray2C(T* &dest, const vm::array *a, bool square=true,
                        size_t dim2=0, GCPlacement placement=NoGC)
{
  size_t n=checkArray(a);
  size_t m=(square || n == 0) ? n : checkArray(vm::read<vm::array*>(a,0));
  if(n > 0 && dim2 && m != dim2) {
    ostringstream buf;
    buf << "second matrix dimension must be " << dim2;
    vm::error(buf);
  }
  
  dest=(placement == NoGC) ? new T[n*m] : new(placement) T[n*m];
  for(size_t i=0; i < n; i++) {
    vm::array *ai=vm::read<vm::array*>(a,i);
    size_t aisize=checkArray(ai);
    if(aisize == m) {
      T *desti=dest+i*m;
      for(size_t j=0; j < m; j++) 
        desti[j]=vm::read<T>(ai,j);
    } else
      vm::error(square ? "matrix must be square" : 
                "matrix must be rectangular");
  }
}

template<typename T>
inline vm::array* copyCArray2(const size_t n, const size_t m, const T* p)
{
  vm::array* a=new vm::array(n);
  for(size_t i=0; i < n; ++i) {
    array *ai=new array(m);
    (*a)[i]=ai;
    for(size_t j=0; j < m; ++j) 
      (*ai)[j]=p[m*i+j];
  }
  return a;
}

} // namespace run

#endif // ARRAYOP_H
