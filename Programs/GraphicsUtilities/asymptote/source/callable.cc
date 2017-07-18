/*****
 * callable.cc
 * Tom Prince 2005/06/19
 *
 * Runtime representation of functions.
 *****/

#include "stack.h"
#include "callable.h"

namespace vm {

callable::~callable()
{}

void func::call(stack *s)
{
  s->run(this);
}

void nullfunc::print(ostream& out) {
  out << "nullfunc";
}

bool func::compare(callable* F)
{
  if (func* f=dynamic_cast<func*>(F))
    return (body == f->body) && (closure == f->closure);
  else return false;
}

void func::print(ostream& out) {
  out << "func with lambda";
#ifdef DEBUG_FRAME
  out << " " << body->name;
#endif
}

bool bfunc::compare(callable* F)
{
  if (bfunc* f=dynamic_cast<bfunc*>(F))
    return (func == f->func);
  else return false;
}

void bfunc::print(ostream& out) {
  out << "bltin";
#ifdef DEBUG_BLTIN
  out << " " << lookupBltin(func);
#endif
}

void thunk::call(stack *s)
{
  s->push(arg);
  func->call(s);
}

void thunk::print(ostream& out) {
  out << "thunk on " << arg << " with ";
  func->print(out);
}

nullfunc nullfunc::func;
void nullfunc::call(stack *)
{
  error("dereference of null function");
}

bool nullfunc::compare(callable* f)
{
  return f == &func;
}

} // namespace vm
