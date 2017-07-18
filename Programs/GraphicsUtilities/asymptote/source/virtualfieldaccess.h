/*****
 * virtualfieldaccess.h
 * Andy Hammerlindl 2009/07/23
 *
 * Implements the access subclass used to read and write virtual fields.
 *****/

#include "access.h"

namespace trans {
// In code such as
//     pair z; write(z.x);
// to evaluate the expression z.x, first z is pushed onto the stack, then as
// it is not a record, instead of accessing its field in the usual way for a
// record, a builtin function is called which pops z off the stack and
// replaces it with z.x.  virtualFieldAccess provides the access for the
// virtualField 'x', and 'getter' is the access to the builtin function which
// replaces z with z.x.
//
// If the virtual field z.x were mutable, then setter would access a builtin
// function, which pops a real number and then z off of the stack, sets the
// z.x to that new value, and puts the value back on the stack.  In this case,
// pairs are immutable, but other virtual fields may not be.
//
// Some virtual fields are functions, such as a.push for an array a.  In rare
// cases, code such as
//     void f(int) = a.push;
// requires an object representing a.push, and so a getter for the field must
// be provided.  Most of the time, however, these fields are just called.
// As an optimization, a caller access can be provided.  If this access is
// given, then a call to the virtualFieldAccess just translates into a call to
// caller.
class virtualFieldAccess : public access {
  access *getter;
  access *setter;
  access *caller;

public:
  virtualFieldAccess(access *getter,
                     access *setter = 0,
                     access *caller = 0)
    : getter(getter), setter(setter) {}

  virtualFieldAccess(vm::bltin getter,
                     vm::bltin setter = 0,
                     vm::bltin caller = 0)
    : getter(new bltinAccess(getter)),
      setter(setter ? new bltinAccess(setter) : 0),
      caller(caller ? new bltinAccess(caller) : 0) {}

  void encode(action act, position pos, coder &e);
  void encode(action act, position pos, coder &e, frame *);

  // Attempting to WRITE a read-only field will give an error, but if the
  // error is caught at a higher level, a better error message (including the
  // name of the field) can be given.  This function allows such handling.
  bool readonly() {
    return (bool)setter;
  }
};

} // namespace trans
