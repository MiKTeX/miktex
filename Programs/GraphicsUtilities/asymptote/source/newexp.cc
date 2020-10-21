/*****
 * newexp.cc
 * Andy Hammerlindl 2003/07/28
 *
 * Handles the abstract syntax for expressions the create new objects,
 * such as record, array, and function constructors.
 *****/

#include "newexp.h"
#include "stm.h"
#include "runtime.h"
#include "runarray.h"
#include "coenv.h"
#include "inst.h"

using namespace types;
using trans::coder;
using trans::coenv;
using vm::inst;

namespace absyntax {

void printFrame(frame *f) {
  if (f == 0) {
    cerr << '0';
  }
  else {
    cerr << f << " of ";
    printFrame(f->getParent());
  }
}

void newRecordExp::prettyprint(ostream &out, Int indent) {
  prettyname(out, "newRecordExp", indent);
}

bool newRecordExp::encodeLevel(position pos, coenv &e, trans::tyEntry *ent)
{
  record *r = dynamic_cast<record *>(ent->t);
  assert(r);

  // The level needed on which to allocate the record.
  frame *level = r->getLevel()->getParent();

  if (ent->v) {
    // Put the record on the stack.  For instance, in code like
    //   import imp;
    //   new imp.t;
    // we are putting the instance of imp on the stack, so we can use it to
    // allocate an instance of imp.t.
    ent->v->getLocation()->encode(trans::READ, pos, e.c);

    // Adjust to the right frame.  For instance, in the last new in
    //   struct A {
    //     struct B {
    //       static struct C {}
    //     }
    //     B b=new B;
    //   }
    //   A a=new A;
    //   new a.b.C;
    // we push a.b onto the stack, but need a as the enclosing frame for
    // allocating an instance of C.
    record *q = dynamic_cast<record *>(ent->v->getType());
    return e.c.encode(level, q->getLevel());
  }
  else
    return e.c.encode(level);
}

types::ty *newRecordExp::transFromTyEntry(position pos, coenv &e,
                                          trans::tyEntry *ent)
{
  types::ty *t = ent->t;
  if (t->kind == ty_error)
    return t;
  else if (t->kind != ty_record) {
    em.error(pos);
    em << "type '" << *t << "' is not a structure";
    return primError();
  }

  // Put the enclosing frame on the stack.
  if (!encodeLevel(pos, e, ent)) {
    em.error(pos);
    em << "allocation of struct '" << *t << "' is not in a valid scope";
    return primError();
  }

  record *r = dynamic_cast<record *>(t);
  assert(r);

  // Encode the allocation.
  e.c.encode(inst::makefunc,r->getInit());
  e.c.encode(inst::popcall);

  return t;
}

types::ty *newRecordExp::trans(coenv &e)
{
  return transFromTyEntry(getPos(), e, result->transAsTyEntry(e, 0));
}

types::ty *newRecordExp::getType(coenv &e)
{
  types::ty *t = result->trans(e, true);
  if (t->kind != ty_error && t->kind != ty_record)
    return primError();
  else
    return t;
}

void newArrayExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out,"newArrayExp",indent);

  celltype->prettyprint(out, indent+1);
  if (dimexps) dimexps->prettyprint(out, indent+1);
  if (dims) dims->prettyprint(out, indent+1);
  if (ai) ai->prettyprint(out, indent+1);
}

types::ty *newArrayExp::trans(coenv &e)
{
  types::ty *c = celltype->trans(e);
  if (c->kind == ty_void) {
    em.error(getPos());
    em << "cannot declare array of type void";
    return primError();
  }

  if (dims)
    c = dims->truetype(c);

  if (ai) {
    ai->transToType(e, c);
    return c;
  } else if (dimexps || dims) {
    if (dimexps) {
      for (size_t i = 0; i < dimexps->size(); ++i) {
        (*dimexps)[i]->transToType(e, types::primInt());
        c = new types::array(c);
      }
    }
    if (dims) {
      for (size_t i = 0; i < dims->size(); ++i) {
        e.c.encode(inst::intpush,0);
      }
    }
    e.c.encode(inst::intpush,
               (Int) ((dimexps ? dimexps->size():0)
                      + (dims ? dims->size():0)));
    e.c.encode(inst::builtin, run::newDeepArray);

    return c;
  } else {
    em.compiler(getPos());
    em << "new array expression must have either dims or dimexps";
    return primError();
  }
}

types::ty *newArrayExp::getType(coenv &e)
{
  types::ty *c = celltype->trans(e);
  if (c->kind == ty_void) {
    return primError();
  }
  if (dims)
    c = dims->truetype(c);

  if (dimexps) {
    Int depth = (Int)dimexps->size();
    while (depth > 0) {
      c = new types::array(c);
      depth--;
    }
  }

  return c;
}


} // namespace absyntax
