/*****
 * stm.h
 * Andy Hammerlindl 2002/8/30
 *
 * Statements are objects in the language that do something on their
 * own.  Statements are different from declarations in that statements
 * do not modify the environment.  Translation of a statements puts the
 * stack code to run it into the instruction stream.
 *****/

#ifndef STM_H
#define STM_H

#include "types.h"
#include "symbol.h"
#include "dec.h"

namespace trans {
class coenv;
}

namespace absyntax {

using trans::coenv;
using sym::symbol;

class stm : public runnable {
public:
  stm(position pos)
    : runnable(pos) {}

  void prettyprint(ostream &out, Int indent);

  void transAsField(coenv &e, record *) {
    // Ignore the record.
    trans(e);
  }

  void trans(coenv &e) = 0;
};

class emptyStm : public stm {
public:
  emptyStm(position pos)
    : stm(pos) {}

  void prettyprint(ostream &out, Int indent);

  void trans(coenv &) {}
};

// Wrapper around a block to use it as a statement.
class blockStm : public stm {
  block *base;

public:
  blockStm(position pos, block *base)
    : stm(pos), base(base) {}

  void prettyprint(ostream &out, Int indent);

  void trans(coenv &e) {
    return base->trans(e);
  }

  // A block is guaranteed to return iff its last statement is
  // guaranteed to return. 
  bool returns() {
    return base->returns();
  }
    
};

// A statement that consist of a single expression to evaluate.
class expStm : public stm {
  exp *body;

public:
  expStm(position pos, exp *body)
    : stm(pos), body(body) {}

  void prettyprint(ostream &out, Int indent);

  void trans(coenv &e);

  // Should be called when running an expStm at the interactive prompt.
  // The code will "write" the value of the expression at the prompt if
  // possible.
  void interactiveTrans(coenv &e);
};

class ifStm : public stm {
  exp *test;
  stm *onTrue;
  stm *onFalse;

public:
  ifStm(position pos, exp *test, stm* onTrue, stm* onFalse = 0)
    : stm(pos), test(test), onTrue(onTrue), onFalse(onFalse) {}

  void prettyprint(ostream &out, Int indent);

  void trans(coenv &e);

  // An if statement is guaranteed to return iff both its pieces are
  // guaranteed to return.
  bool returns() {
    if (onTrue == 0 || onFalse == 0)
      return false;
    return onTrue->returns() && onFalse->returns();
  }
};

class whileStm : public stm {
  exp *test;
  stm *body;

public:
  whileStm(position pos, exp *test, stm *body)
    : stm(pos), test(test), body(body) {}

  void prettyprint(ostream &out, Int indent);

  void trans(coenv &e);
};

class doStm : public stm {
  stm *body;
  exp *test;

public:
  doStm(position pos, stm *body, exp *test)
    : stm(pos), body(body), test(test) {}

  void prettyprint(ostream &out, Int indent);

  void trans(coenv &e);
};

class forStm : public stm {
  runnable *init;
  exp *test;
  runnable *update;
  stm *body;

public:
  forStm(position pos, runnable *init, exp *test, runnable *update, stm *body)
    : stm(pos), init(init), test(test), update(update), body(body) {}

  void prettyprint(ostream &out, Int indent);

  void trans(coenv &e);
};

class extendedForStm : public stm {
  ty *start;
  symbol var;
  exp *set;

  stm *body;

public:
  extendedForStm(position pos, ty *start, symbol var, exp *set, stm *body)
    : stm(pos), start(start), var(var), set(set), body(body) {}

  void prettyprint(ostream &out, Int indent);

  void trans(coenv &e);
};


class breakStm : public stm {
public:
  breakStm(position pos)
    : stm(pos) {}

  void prettyprint(ostream &out, Int indent);

  void trans(coenv &e);
};

class continueStm : public stm {
public:
  continueStm(position pos)
    : stm(pos) {}

  void prettyprint(ostream &out, Int indent);

  void trans(coenv &e);
};

class returnStm : public stm {
  exp *value;
  
public:
  returnStm(position pos, exp *value = 0)
    : stm(pos), value(value) {}

  void prettyprint(ostream &out, Int indent);

  void trans(coenv &e);

  // A return statement is, of course, guaranteed to return.
  bool returns() {
    return true;
  }
};


// Used at the start of for loops.
class stmExpList : public stm {
  mem::list<stm *> stms;

public:
  stmExpList(position pos) 
    : stm(pos) {}

  // To ensure list deallocates properly.
  virtual ~stmExpList() {}

  void add(stm *s) {
    stms.push_back(s);
  }

  void prettyprint(ostream &out, Int indent);

  void trans(coenv &e);
};

} // namespace absyntax

#endif
