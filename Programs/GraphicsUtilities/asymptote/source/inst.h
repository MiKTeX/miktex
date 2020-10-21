/*****
 * inst.h
 * Andy Hammerlindl 2002/06/27
 *
 * Descibes the items and instructions that are used by the virtual machine.
 *****/

#ifndef INST_H
#define INST_H

#include <iterator>
#include <iostream>

#include "errormsg.h"
#include "item.h"
#include "vm.h"

namespace vm {

// Forward declarations
struct inst; class stack; class program;

// A function "lambda," that is, the code that runs a function.
// It also needs the closure of the enclosing module or function to run.
struct lambda : public gc {
  // The instructions to follow.
  program *code;

  // The index of the link to the parent closure in the frame corresponding to
  // this function.
  size_t parentIndex;

  // The total number of items that will be stored in the closure of this
  // function.  Includes a link to the higher closure, the parameters, and the
  // local variables.
  // NOTE: In order to help garbage collection, this could be modified to
  // have one array store escaping items, and another to store non-
  // escaping items.
  size_t framesize;

  // States whether any of the variables escape the function, in which case a
  // closure needs to be allocated when the function is called.  It is
  // initially set to "maybe" and it is computed the first time the function
  // is called.
  enum { NEEDS_CLOSURE, DOESNT_NEED_CLOSURE, MAYBE_NEEDS_CLOSURE} closureReq;

#ifdef DEBUG_FRAME
  string name;

  lambda()
    : closureReq(MAYBE_NEEDS_CLOSURE), name("<unnamed>") {}
  virtual ~lambda() {}
#else
  lambda()
    : closureReq(MAYBE_NEEDS_CLOSURE) {}
#endif
};

// The code run is just a string of instructions.  The ops are actual commands
// to be run, but constants, labels, and other objects can be in the code.
struct inst : public gc {
  enum opcode {
#define OPCODE(name,type)  name,
#include "opcodes.h"
#undef OPCODE
  };
  opcode op;
  position pos;
  item ref;
};
template<typename T>
inline T get(const inst& it)
{ return get<T>(it.ref); }

} // namespace vm

#endif

