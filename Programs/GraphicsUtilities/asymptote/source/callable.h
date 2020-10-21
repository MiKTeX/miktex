/*****
 * callable.h
 * Tom Prince 2005/06/19
 *
 * Runtime representation of functions.
 *****/

#ifndef CALLABLE_H
#define CALLABLE_H

#include "common.h"
#include "item.h"
#include "inst.h"

namespace vm {

class stack;
typedef void (*bltin)(stack *s);

struct callable : public gc
{
  virtual void call(stack *) = 0;
  virtual ~callable();
  virtual bool compare(callable*) { return false; }

  // For debugging:
  virtual void print(ostream& out) = 0;
};

class nullfunc : public callable
{
private:
  nullfunc() {}
  static nullfunc func;
public:
  virtual void call (stack*);
  virtual bool compare(callable*);
  static callable* instance() { return &func; }

  void print(ostream& out);
};

// How a function reference to a non-builtin function is stored.
struct func : public callable {
  lambda *body;
  frame *closure;
  func () : body(), closure() {}
  virtual void call (stack*);
  virtual bool compare(callable*);

  void print(ostream& out);
};

class bfunc : public callable
{
public:
  bfunc(bltin b) : func(b) {}
  virtual void call (stack *s) { func(s); }
  virtual bool compare(callable*);

  void print(ostream& out);
private:
  bltin func;
};

class thunk : public callable
{
public:
  thunk(callable *f, item i) : func(f), arg(i) {}
  virtual void call (stack*);

  void print(ostream& out);
private:
  callable *func;
  item arg;
};

inline ostream& operator<< (ostream& out, callable &c) {
  c.print(out);
  return out;
}

} // namespace vm

GC_DECLARE_PTRFREE(vm::nullfunc);

// I believe this is safe, as pointers to C++ functions do not point to
// the heap.
GC_DECLARE_PTRFREE(vm::bfunc);

#endif // CALLABLE_H
