/*****
 * stack.h
 * Andy Hammerlindl 2002/06/27
 * 
 * The general stack machine  used to run compiled camp code.
 *****/

#ifndef STACK_H
#define STACK_H

#include <iostream>

#include "errormsg.h"
#include "vm.h"
#include "item.h"
#include "absyn.h"

namespace vm {

struct func;
class program;
struct lambda;
class importInitMap;

struct bpinfo : public gc {
  fileinfo f;
  absyntax::runnable *r;
  
  bpinfo(const string& filename, size_t lineNum,
         absyntax::runnable *r=NULL) :
    f(fileinfo(filename,lineNum)), r(r) {}
};
  
inline bool operator == (const bpinfo& a, const bpinfo& b)
{
  return a.f == b.f;
}
  
extern mem::list<bpinfo> bplist;
  
class runnable;
  
extern bool indebugger;  
  
class stack {
public:
  typedef frame* vars_t;

  struct importInitMap {
    virtual ~importInitMap() {}
    virtual lambda *operator[](string) = 0;
  };

private:
  // stack for operands
  typedef mem::vector<item> stack_t;
  stack_t theStack;

  void draw(ostream& out);

  // The initializer functions for imports, indexed by name.
  importInitMap *initMap;

  // The stack stores a map of initialized imported modules by name, so that
  // each module is initialized only once and each import refers to the same
  // instance.
  typedef mem::map<CONST string,frame *> importInstanceMap;
  importInstanceMap instMap;
  
  // One can associate an environment to embedded code while running.
  trans::coenv *e;

  // Debugger variables:
  char debugOp;
  position lastPos, breakPos;
  bool newline;
  
  // Move arguments from stack to frame.
  void marshall(size_t args, stack::vars_t vars);

public:
  stack() : e(0), debugOp(0), lastPos(nullPos),
            breakPos(nullPos), newline(false) {};
  
  virtual ~stack() {};

  void setInitMap(importInitMap *i) {
    initMap=i;
  }

  void setEnvironment(trans::coenv *e) {
    this->e=e;
  }

  trans::coenv *getEnvironment() {
    return e;
  }

  // Runs a lambda.  If vars is non-null, it is used to store the variables of
  // the lambda.  Otherwise, the method allocates a closure only if needed.
  void runWithOrWithoutClosure(lambda *l, vars_t vars, vars_t parent);

  // Executes a function on top of the stack.
  void run(func *f);

  void breakpoint(absyntax::runnable *r=NULL);
  void debug();
  
  // Put an import (indexed by name) on top of the stack, initializing it if
  // necessary.
  void load(string index);

  // These are so that built-in functions can easily manipulate the stack
  void push(item next) {
    theStack.push_back(next);
  }
  template <typename T>
  void push(T next) {
    push((item)next);
  }
  item top() {
    return theStack.back();
  }
  item pop() {
    item ret = theStack.back();
    theStack.pop_back();
    return ret;
  }
  template <typename T>
  T pop()
  {
    return get<T>(pop());
  }
};

inline item pop(stack* s)
{
  return s->pop();
}

template <typename T>
inline T pop(stack* s)
{
  return get<T>(pop(s));
}
  
template <typename T>
inline T pop(stack* s, T defval)
{
  item it=pop(s);
  return isdefault(it) ? defval : get<T>(it);
}
  
class interactiveStack : public stack {
  vars_t globals;
  size_t globals_size;
public:
  interactiveStack();
    
  // Run a codelet, a small piece of code that uses globals as its frame.
  void run(lambda *codelet);
};

} // namespace vm

#endif // STACK_H
  
