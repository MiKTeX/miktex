/*****
 * access.h
 * Andy Hammerlindl 2003/12/03
 *
 * Describes an "access," a representation of where a variable will be
 * stored at runtime, so that read, write, and call instructions can be
 * made.
 *****/

#ifndef ACCESS_H
#define ACCESS_H

#include <cassert>

#include "errormsg.h"
#include "item.h"
#include "vm.h"

namespace vm {
struct callable;
}

namespace trans {

class frame;
class coder;

enum action {
  READ,
  WRITE,
  CALL
};

// These serves as the base class for the accesses.
class access : public gc {
protected:
  // Generic compiler access error - if the compiler functions properly,
  // none of these should be reachable by the user.
  void error(position pos)
  {
    em.compiler(pos);
    em << "invalid use of access";
  }

public:
  virtual ~access() = 0;

  // Encode a read/write/call of the access when nothing is on the stack.
  virtual void encode(action, position pos, coder &)
  {
    error(pos);
  }
  // Encode a read/write/call of the access when the frame "top" is on top
  // of the stack.
  virtual void encode(action, position pos, coder &, frame *)
  {
    error(pos);
  }
};

// This class represents identity conversions in casting.
class identAccess : public access
{
  virtual void encode(action act, position, coder&);
};

// Represents a function that is implemented by a built-in C++ function.
class bltinAccess : public access {
  vm::bltin f;

public:
  bltinAccess(vm::bltin f)
    : f(f) {}

  void encode(action act, position pos, coder &e);
  void encode(action act, position pos, coder &e, frame *);
};

// Similar to bltinAccess, but works for any callable.
class callableAccess : public access {
  vm::callable *f;

public:
  callableAccess(vm::callable *f)
    : f(f) {}

  void encode(action act, position pos, coder &e);
  void encode(action act, position pos, coder &e, frame *);
};

// An access that puts a frame on the top of the stack.
class frameAccess : public access {
  frame *f;

public:
  frameAccess(frame *f)
    : f(f) {}

  void encode(action act, position pos, coder &e);
  void encode(action act, position pos, coder &e, frame *top);
};

// Represents the access of a local variable.
class localAccess : public access {
  Int offset;
  frame *level;

public:
  localAccess(Int offset, frame *level)
    : offset(offset), level(level) {}

  void encode(action act, position pos, coder &e);
  void encode(action act, position pos, coder &e, frame *top);
};

class qualifiedAccess : public access {
  // The location and frame of the record.
  access *qualifier;
  frame *qualifierLevel;

  // The location of the field relative to the record.
  access *field;

public:
  qualifiedAccess(access *qualifier, frame *qualifierLevel, access *field)
    : qualifier(qualifier), qualifierLevel(qualifierLevel), field(field) {}

  void encode(action act, position pos, coder &e);
  void encode(action act, position pos, coder &e, frame *top);
};

} // namespace trans

#endif // ACCESS_H

