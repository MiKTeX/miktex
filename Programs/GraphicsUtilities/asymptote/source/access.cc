/*****
 * access.cc
 * Andy Hammerlindl 2003/12/03
 * Describes an "access," a representation of where a variable will be
 * stored at runtime, so that read, write, and call instructions can be
 * made.
 *****/

#include "access.h"
#include "frame.h"
#include "coder.h"
#include "callable.h"

using vm::item;

namespace trans {

/* access */
access::~access()
{}

/* identAccess */
void identAccess::encode(action act, position pos, coder& e)
{
  if (act != CALL) {
    access::encode(act, pos, e);
  }
  // else - do nothing
}


/* bltinAccess */
static void bltinError(position pos)
{
  em.error(pos);
  em << "built-in functions cannot be modified";
}

void bltinAccess::encode(action act, position pos, coder &e)
{
  switch (act) {
    case READ:
      e.encode(inst::constpush,(item)(vm::callable*)new vm::bfunc(f));
      break;
    case WRITE:
      bltinError(pos);
      break;
    case CALL:
      e.encode(inst::builtin, f);
      break;
  }
}

void bltinAccess::encode(action act, position pos, coder &e, frame *)
{
  e.encode(inst::pop);
  encode(act, pos, e);
}
  
/* callableAccess */
void callableAccess::encode(action act, position pos, coder &e)
{
  switch (act) {
    case READ:
      e.encode(inst::constpush, (item)f);
      break;
    case WRITE:
      bltinError(pos);
      break;
    case CALL:
      this->encode(READ, pos, e);
      e.encode(inst::popcall);
      break;
  }
}

void callableAccess::encode(action act, position pos, coder &e, frame *)
{
  e.encode(inst::pop);
  encode(act, pos, e);
}
  

/* frameAccess */
void frameAccess::encode(action act, position pos, coder &e)
{
  if (act == READ) {
    if (!e.encode(f)) {
      em.compiler(pos);
      em << "encoding frame out of context";
    }
  }
  else
    access::encode(act, pos, e);
}

void frameAccess::encode(action act, position pos, coder &e, frame *top)
{
  if (act == READ) {
    if (!e.encode(f, top)) {
      em.compiler(pos);
      em << "encoding frame out of context";
    }
  }
  else
    access::encode(act, pos, e, top);
}

/* localAccess */
static void frameError(position pos) {
  // A local variable is being used when its frame is not active.
  em.error(pos);
  em << "static use of dynamic variable";
}

void localAccess::encode(action act, position pos, coder &e)
{
  // Get the active frame of the virtual machine.
  frame *active = e.getFrame();
  if (level == active) {
    e.encode(act == WRITE ? inst::varsave : inst::varpush,
             offset);
  }
  else if (e.encode(level)) {
      e.encode(act == WRITE ? inst::fieldsave : inst::fieldpush,
               offset);
  }
  else {
    frameError(pos);
  }

  if (act == CALL)
    e.encode(inst::popcall);
}

void localAccess::encode(action act, position pos, coder &e, frame *top)
{
  if (e.encode(level,top)) {
    e.encode(act == WRITE ? inst::fieldsave : inst::fieldpush,
             offset);
    if (act == CALL)
      e.encode(inst::popcall);
  }
  else {
    frameError(pos);
  }
}


void qualifiedAccess::encode(action act, position pos, coder &e)
{
  qualifier->encode(READ, pos, e);
  field->encode(act, pos, e, qualifierLevel);
}

void qualifiedAccess::encode(action act, position pos, coder &e, frame *top)
{
  qualifier->encode(READ, pos, e, top);
  field->encode(act, pos, e, qualifierLevel);
}

    
} // namespace trans
