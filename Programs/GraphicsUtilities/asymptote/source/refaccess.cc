/*****
 * refaccess.cc
 * Andy Hammerlindl 2005/11/28
 *
 * An access which refers to a variable or other object in C++.
 *****/

#include "refaccess.h"

namespace trans {

using vm::item;
using vm::stack;
using vm::pop;

/* itemRefAccess */
void itemPointerRead(stack *s) {
  item *p=pop<item *>(s);
  s->push(*p);
}

void itemPointerWrite(stack *s) {
  item *p=pop<item *>(s);
  item value=pop(s);
  *p=value;
  s->push(value);
}

void itemRefAccess::encode(action act, position, coder &e)
{
  REGISTER_BLTIN(itemPointerRead, "itemPointerRead");
  REGISTER_BLTIN(itemPointerWrite, "itemPointerWrite");

  e.encode(inst::constpush, (item)ref);

  switch (act) {
    case READ:
      e.encode(inst::builtin, itemPointerRead);
      break;
    case WRITE:
      e.encode(inst::builtin, itemPointerWrite);
      break;
    case CALL:
      e.encode(inst::builtin, itemPointerRead);
      e.encode(inst::popcall);
      break;
  };
}

void itemRefAccess::encode(action act, position pos, coder &e, frame *)
{
  // Get rid of the useless top frame.
  e.encode(inst::pop);
  encode(act, pos, e);
}

}
