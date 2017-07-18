/*****
 * castop.h
 * Tom Prince 2005/3/18
 *
 * Defines some runtime functions used by the stack machine.
 *
 *****/

#ifndef CASTOP_H
#define CASTOP_H

#include <cfloat>

#include "common.h"
#include "stack.h"
#include "fileio.h"
#include "lexical.h"
#include "mathop.h"
#include "array.h"

namespace run {

using vm::read;
using vm::pop;

template<class T, class S>
void cast(vm::stack *s)
{
  s->push((S) pop<T>(s));
}

void castDoubleInt(vm::stack *s)
{
  double x=pop<double>(s);
  s->push(Intcast(x));
}

template<class T>
void stringCast(vm::stack *s)
{
  ostringstream buf;
  buf.precision(DBL_DIG);
  buf << pop<T>(s);
  s->push(buf.str());
}

template<class T>
void castString(vm::stack *s)
{
  string *S=pop<string*>(s);
  if(S->empty()) {
    T x=0;
    s->push(x);
  } else {
    try {
      s->push(lexical::cast<T>(*S));
    } catch (lexical::bad_cast&) {
      s->push(vm::Default);
    }
  }
}

void initialized(vm::stack *s)
{
  s->push(!vm::isdefault(pop(s)));
}

template<class T, class S>
void arrayToArray(vm::stack *s)
{
  vm::array *a=pop<vm::array*>(s);
  size_t size=checkArray(a);
  vm::array *c=new vm::array(size);
  for(size_t i=0; i < size; i++)
    (*c)[i]=(S) read<T>(a,i);
  s->push(c);
}

template<class T, class S>
void array2ToArray2(vm::stack *s)
{
  vm::array *a=pop<vm::array*>(s);
  size_t size=checkArray(a);
  vm::array *c=new vm::array(size);
  for(size_t i=0; i < size; ++i) {
    vm::array *ai=vm::read<vm::array*>(a,i);
    size_t aisize=checkArray(ai);
    vm::array *ci=new vm::array(aisize);
    (*c)[i]=ci;
    for(size_t j=0; j < aisize; ++j)
      (*ci)[j]=(S) read<T>(ai,j);
  }
  s->push(c);
}

template<class T>
void read(vm::stack *s)
{
  camp::file *f = pop<camp::file*>(s);
  T val=T();
  if(f->isOpen()) {
    f->read(val);
    if(f->LineMode()) f->nexteol();
    if(interact::interactive) f->purgeStandard(val);
  }
  s->push(val);
}

inline Int Limit(Int nx) {return nx == 0 ? Int_MAX : nx;}
inline void reportEof(camp::file *f, Int count) 
{
  if(count > 0) {
    ostringstream buf;
    buf << "EOF after reading " << count
        << " values from file '" << f->filename() << "'.";
    vm::error(buf);
  }
}

template<class T>
void readArray(vm::stack *s, Int nx=-1, Int ny=-1, Int nz=-1)
{
  camp::file *f = pop<camp::file*>(s);
  vm::array *c=new vm::array(0);
  if(f->isOpen()) {
    if(nx != -1 && f->Nx() != -1) nx=f->Nx();
    if(nx == -2) {f->read(nx); f->Nx(-1); if(nx == 0) {s->push(c); return;}}
    if(ny != -1 && f->Ny() != -1) ny=f->Ny();
    if(ny == -2) {f->read(ny); f->Ny(-1); if(ny == 0) {s->push(c); return;}}
    if(nz != -1 && f->Nz() != -1) nz=f->Nz();
    if(nz == -2) {f->read(nz); f->Nz(-1); if(nz == 0) {s->push(c); return;}}
    T v;
    if(nx >= 0) {
      for(Int i=0; i < Limit(nx); i++) {
        if(ny >= 0) {
          vm::array *ci=new vm::array(0);
          for(Int j=0; j < Limit(ny); j++) {
            if(nz >= 0) {
              vm::array *cij=new vm::array(0);
              bool break2=false;
              for(Int k=0; k < Limit(nz); k++) {
                f->read(v);
                if(f->error()) {
                  if(nx && ny && nz) reportEof(f,(i*ny+j)*nz+k);
                  s->push(c);
                  return;
                }
                if(k == 0) {
                  if(j == 0) c->push(ci);
                  ci->push(cij);
                }
                cij->push(v);
                if(f->LineMode() && f->nexteol()) {
                  if(f->nexteol()) break2=true;
                  break;
                }
              }
              if(break2) break;
            } else {
              f->read(v);
              if(f->error()) {
                if(nx && ny) reportEof(f,i*ny+j);
                s->push(c);
                return;
              }
              if(j == 0) c->push(ci);
              ci->push(v);
              if(f->LineMode() && f->nexteol()) break;
            }
          }
        } else {
          f->read(v);
          if(f->error()) {
            if(nx) reportEof(f,i);
            s->push(c);
            return;
          }
          c->push(v);
          if(f->LineMode() && f->nexteol()) break;
        }
      }
    } else {
      for(;;) {
        f->read(v);
        if(f->error()) break;
        c->push(v);
        if(f->LineMode() && f->nexteol()) break;
      }
    }
    if(interact::interactive) f->purgeStandard(v);
  }
  s->push(c);
}

template<class T>
void readArray1(vm::stack *s)
{
  readArray<T>(s,0); 
}

template<class T>
void readArray2(vm::stack *s)
{
  readArray<T>(s,0,0); 
}

template<class T>
void readArray3(vm::stack *s)
{
  readArray<T>(s,0,0,0); 
}

} // namespace run

#endif // CASTOP_H

