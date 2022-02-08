/*****
 * varinit.cc
 * Andy Hammerlindl 2005/07/01
 *
 * Variable initializer are syntax that finish code such as
 *   int var = ...
 * As such, they are translated to yield a certain type, the type of the
 * variable.  Expressions are a special case that can be translated without an
 * associated variable or its type.
 *****/

#include "varinit.h"
#include "coenv.h"
#include "runtime.h"
#include "runarray.h"

namespace absyntax {

using namespace types;
using namespace trans;

void definit::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "definit",indent, getPos());
}

void definit::transToType(coenv &e, types::ty *target)
{
  if (target->kind != ty_error) {
    access *a=e.e.lookupInitializer(target);

    if (a)
      a->encode(action::CALL, getPos(), e.c);
    else {
      em.error(getPos());
      em << "no default initializer for type '" << *target << "'";
    }
  }
}

void arrayinit::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "arrayinit",indent, getPos());

  for (mem::list<varinit *>::iterator p = inits.begin(); p != inits.end(); ++p)
    (*p)->prettyprint(out, indent+2);
  if (rest)
    rest->prettyprint(out, indent+1);
}

void arrayinit::transMaker(coenv &e, Int size, bool rest) {
  // Push the number of cells and call the array maker.
  e.c.encode(inst::intpush, size);
  e.c.encode(inst::builtin, rest ? run::newAppendedArray :
             run::newInitializedArray);
}

void arrayinit::transToType(coenv &e, types::ty *target)
{
  types::ty *celltype;
  if (target->kind != types::ty_array) {
    em.error(getPos());
    em << "array initializer used for non-array";
    celltype = types::primError();
  }
  else {
    celltype = ((types::array *)target)->celltype;
  }

  // Push the values on the stack.
  for (mem::list<varinit *>::iterator p = inits.begin(); p != inits.end(); ++p)
    (*p)->transToType(e, celltype);

  if (rest)
    rest->transToType(e, target);

  transMaker(e, (Int)inits.size(), (bool)rest);
}

} // namespace absyntax
