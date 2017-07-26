/*****
 * varinit.h
 * Andy Hammerlindl 2005/07/01
 *
 * Variable initializers are syntax that finish code such as
 *   Int var = ...
 * As such, they are translated to yield a certain type, the type of the
 * variable.  Expressions are a special case that can be translated without an
 * associated variable or its type.
 *****/

#ifndef VARINIT_H
#define VARINIT_H

#include "types.h"
#include "symbol.h"
#include "absyn.h"

namespace absyntax {

using trans::coenv;
using trans::access;
using sym::symbol;
using types::array;

class varinit : public absyn {
public:
  varinit(position pos)
    : absyn(pos) {}

  // This determines what instruction are needed to put the associated
  // value onto the stack, then adds those instructions to the current
  // lambda in e.
  // In some expressions and initializers, the target type needs to be
  // known in order to translate properly.  For most expressions, this is
  // kept to a minimum.
  // For expression, this also allows an implicit cast, hence the name.
  virtual void transToType(coenv &e, types::ty *target) = 0;
};

// A default initializer.  For example:
//   int a;
// is in some sense equivalent to
//   int a=0;
// where the definit for Int is a function that returns 0.
class definit : public varinit {
public:
  definit(position pos)
    : varinit(pos) {}

  void prettyprint(ostream &out, Int indent);

  void transToType(coenv &e, types::ty *target);
};

class arrayinit : public varinit {
  mem::list<varinit *> inits;

  varinit *rest;
public:
  arrayinit(position pos)
    : varinit(pos), rest(0) {}

  virtual ~arrayinit() 
  {}

  void prettyprint(ostream &out, Int indent);

  // Encodes the instructions to make an array from size elements on the stack.
  static void transMaker(coenv &e, Int size, bool rest);

  void transToType(coenv &e, types::ty *target);

  void add(varinit *init) {
    inits.push_back(init);
  }

  void addRest(varinit *init) {
    rest=init;
  }

  friend class joinExp;
};

} // namespace absyntax

#endif
