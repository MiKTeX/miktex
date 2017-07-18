/*****
 * refaccess.h
 * Andy Hammerlindl 2003/12/03
 *
 * An access which refers to a variable or other object in C++.
 *****/

#ifndef REFACCESS_H
#define REFACCESS_H

#include "access.h"
#include "inst.h"
#include "coder.h"
#include "stack.h"

namespace trans {

// Access refers to a piece of data, represented by an item, somewhere in the
// C++ code.
class itemRefAccess : public access {
  vm::item *ref;

public:
  itemRefAccess(vm::item *ref)
    : ref(ref) {}

  void encode(action act, position pos, coder &e);
  void encode(action act, position pos, coder &e, frame *);
};

// Access refers to an arbitrary piece of data of type T.
template <class T>
class refAccess : public access {
  T *ref;

public:
  refAccess(T *ref)
    : ref(ref) {}

  void encode(action act, position pos, coder &e);
  void encode(action act, position pos, coder &e, frame *);
};

template <class T>
void pointerRead(vm::stack *s) {
  T *ptr=vm::pop<T *>(s);
  s->push(*ptr);
}

template <class T>
void pointerWrite(vm::stack *s) {
  T *ptr=vm::pop<T *>(s);
  T value=vm::pop<T>(s);
  *ptr=value;
  s->push(value);
}

template <class T>
void refAccess<T>::encode(action act, position, coder &e)
{
  // You may be able to use typeid(T).name() to get a better label.
  REGISTER_BLTIN((bltin) pointerRead<T>, "pointerRead");
  REGISTER_BLTIN((bltin) pointerWrite<T>, "pointerWrite");

  e.encode(vm::inst::constpush, (vm::item)ref);

  switch (act) {
    case READ:
      e.encode(vm::inst::builtin, (bltin) pointerRead<T>);
      break;
    case WRITE:
      e.encode(vm::inst::builtin, (bltin) pointerWrite<T>);
      break;
    case CALL:
      e.encode(vm::inst::builtin, (bltin) pointerRead<T>);
      e.encode(vm::inst::popcall);
      break;
  };
}

template <class T>
void refAccess<T>::encode(action act, position pos, coder &e, frame *)
{
  // Get rid of the useless top frame.
  e.encode(vm::inst::pop);
  encode(act, pos, e);
}

}
#endif
