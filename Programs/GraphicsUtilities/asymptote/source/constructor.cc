/*****
 * constructor.cc
 * Andy Hammerlindl 2007/05/12
 *
 * Using
 *
 *   void operator init(<args>)
 *
 * as a field in the definition structure named Foo implicitly creates a
 * function that could be explicitly defined with code similar to
 *
 *    static Foo Foo(<args>) {
 *      Foo a=new Foo;
 *      a.operator init(<args>);
 *      return a;
 *    }
 *
 * This function is usable within further code in the structure definition and,
 * after the end of the structure definition, is carried over into the enclosing
 * scope so that it lasts as long as the type definition.
 *****/

#include "stack.h"
#include "entry.h"
#include "coenv.h"
#include "dec.h"
#include "newexp.h"

namespace absyntax {

using namespace trans;
using namespace types;

// Defined in dec.cc.
varEntry *makeVarEntry(position pos, coenv &e, record *r, types::ty *t);

bool definesImplicitConstructor(coenv &e, record *r, varEntry *v, symbol id)
{
  if (id == symbol::initsym &&
      r != 0 &&
      v->getType()->kind == ty_function &&
      e.c.isStatic() == false &&
      e.c.isTopLevel() == false)
    {
      function *ft=dynamic_cast<function *>(v->getType());
      if (ft->getResult()->kind == ty_void)
        return true;
    }

  return false;
}

// Given the coenv of the body of the constructor, encode the neccessary
// instructions to make a new initialized object.
void transConstructorBody(position pos, coenv &e, record *r, varEntry *init)
{
  assert(r);
  assert(init);

  // Create a varEntry to hold the new object.  Foo a;
  varEntry *v=makeVarEntry(pos, e, 0 /* not a field */, r);

  // Initialize the object.  a=new Foo;
  newRecordExp::transFromTyEntry(pos, e, new tyEntry(r, 0, 0, position()));
  v->encode(WRITE, pos, e.c);
  e.c.encodePop();

  // Push the args onto the stack.
  size_t numArgs=init->getSignature()->getNumFormals();
  for (size_t i=0; i<numArgs; ++i) {
    access *a=e.c.accessFormal((Int)i);
    a->encode(READ, pos, e.c);
  }

  // Push the object on the stack.
  v->encode(READ, pos, e.c);

  // Call the 'operator init' field of the object.
  init->encode(action::CALL, pos, e.c, v->getLevel());

  // Push the object again.
  v->encode(READ, pos, e.c);

  // Return the initialized object.
  e.c.encode(inst::ret);
}

varEntry *constructorFromInitializer(position pos, coenv &e, record *r,
                                     varEntry *init)
{
  assert(r);

  types::function *ft=new types::function(r, init->getSignature());

  ostringstream out;
  ft->printVar(out, symbol::trans("<constructor>"));

  // Create a new function environment.
  coder fc = e.c.newFunction(pos, out.str(), ft);
  coenv fe(fc,e.e);

  // Translate the function.
  fe.e.beginScope();

  transConstructorBody(pos, fe, r, init);

  fe.e.endScope();

  // Put an instance of the new function on the stack.
  vm::lambda *l = fe.c.close();
  e.c.encode(inst::pushclosure);
  e.c.encode(inst::makefunc, l);

  // Save it into a varEntry.
  varEntry *v=makeVarEntry(pos, e, r, ft);
  v->encode(WRITE, pos, e.c);
  e.c.encodePop();

  return v;
}

void addConstructorFromInitializer(position pos, coenv &e, record *r,
                                   varEntry *init)
{
  assert(r);

  // Constructors are declared statically.
  e.c.pushModifier(EXPLICIT_STATIC);

  varEntry *v=constructorFromInitializer(pos, e, r, init);

  // Add the constructor function under the same name as the record.
  addVar(e, r, v, r->getName());

  // Add to the "post definition environment" of the record, so it will also be
  // added to the enclosing scope when the record definition ends.
  r->postdefenv.addVar(r->getName(), v);

  e.c.popModifier();
}

} // namespace absyntax

