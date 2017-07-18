/*****
 * virtualfieldaccess.cc
 * Andy Hammerlindl 2009/07/23
 *
 * Implements the access subclass used to read and write virtual fields.
 *****/

#include "virtualfieldaccess.h"
#include "coder.h"

namespace trans {

void virtualFieldAccess::encode(action act, position pos, coder &e)
{
  switch(act) {
    case CALL:
      if (caller) {
        caller->encode(CALL, pos, e);
      } else {
        this->encode(READ, pos, e);
        e.encode(inst::popcall);
      }
      return;
    case READ:
      assert(getter);
      getter->encode(CALL, pos, e);
      return;
    case WRITE:
      if (setter)
        setter->encode(CALL, pos, e);
      else {
        em.error(pos);
        em << "virtual field is read-only";
      }
      return;
  }
}

void virtualFieldAccess::encode(action act, position pos, coder &e, frame *)
{
  e.encode(inst::pop);
  encode(act, pos, e);
}

} // namespace trans
