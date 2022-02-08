/*****
 * exp.h
 * Andy Hammerlindl 2002/8/19
 *
 * Represents the abstract syntax tree for the expressions in the
 * language.  this is translated into virtual machine code using trans()
 * and with the aid of the environment class.
 *****/

#ifndef EXP_H
#define EXP_H

#include "types.h"
#include "symbol.h"
#include "absyn.h"
#include "varinit.h"
#include "name.h"
#include "guideflags.h"

namespace trans {
class coenv;
class coder;

struct label_t;
typedef label_t *label;

class application;
}

namespace absyntax {

using trans::coenv;
using trans::label;
using trans::application;
using trans::access;
using sym::symbol;
using types::record;
using types::array;

class exp : public varinit {
protected:
  // The cached type (from a call to cgetType).
  types::ty *ct;
public:
  exp(position pos)
    : varinit(pos), ct(0) {}

  void prettyprint(ostream &out, Int indent) = 0;

  // When reporting errors with function calls, it is nice to say "no
  // function f(int)" instead of "no function matching signature
  // (int)."  Hence, this method returns the name of the expression if
  // there is one.
  virtual symbol getName()
  {
    return symbol::nullsym;
  }

  // Checks if the expression can be used as the right side of a scale
  // expression.  ie. 3sin(x)
  // If a "non-scalable" expression is scaled a warning is issued.
  virtual bool scalable() { return true; }

  // Specifies if the value of the expression should be written to interactive
  // prompt if typed as a stand-alone expression.  For example:
  // > 2+3;
  // should write 5, but
  // > x=2+3;
  // shouldn't.  (These choices are largely aesthetic)
  virtual bool writtenToPrompt() { return true; }

  // Translates the expression to the given target type.  This should only be
  // called with a type returned by getType().  It does not perform implicit
  // casting.
  virtual void transAsType(coenv &e, types::ty *target);

  // Translates the expression to the given target type, possibly using an
  // implicit cast.
  void transToType(coenv &e, types::ty *target);

  // Translates the expression and returns the resultant type.
  // For some expressions, this will be ambiguous and return an error.
  // Trans may only return ty_error, if it (or one of its recursively
  // called children in the syntax tree) reported an error to em.
  virtual types::ty *trans(coenv &) = 0;

  // getType() figures out the type of the expression without translating
  // the code into the virtual machine language or reporting errors to em.
  // This must follow a few rules to ensure proper translation:
  //   1. If this returns a valid type, t, trans(e) must return t or
  //      report an error, and transToType(e, t) must run either reporting
  //      an error or reporting no error and yielding the same result as
  //      trans(e).
  //   2. If this returns a superposition of types (ie. for overloaded
  //      functions), trans must not return a singular type, and every
  //      type in the superposition must run without error properly
  //      if fed to transAsType(e, t).
  //   3. If this returns ty_error, then so must a call to trans(e) and any
  //      call to trans, transAsType, or transToType must report an error
  //      to em.
  //   4. Any call to transAsType(e, t) with a type that is not returned by
  //      getType() (or one of the subtypes in case of a superposition)
  //      must report an error.
  //      Any call to transToType(e, t) with a type that is not returned by
  //      getType() (or one of the subtypes in case of a superposition)
  //      or any type not implicitly castable from the above must report an
  //      error.
  virtual types::ty *getType(coenv &) = 0;

  // This is an optimization which works in some cases to by-pass the slow
  // overloaded function resolution provided by the application class.
  //
  // If an expression is called with arguments given by sig, getCallee must
  // either return 0 (the default), or if it returns a varEntry, the varEntry
  // must correspond to the function which would be called after normal
  // function resolution.
  //
  // The callee must produce no side effects as there are no guarantees when
  // the varEntry will be translated.
  virtual trans::varEntry *getCallee(coenv &e, types::signature *sig) {
//#define DEBUG_GETAPP
#if DEBUG_GETAPP
    cout << "exp fail" << endl;
    cout << "exp fail at " << getPos() << endl;
    prettyprint(cout, 2);
#endif
    return 0;
  }

  // Same result as getType, but caches the result so that subsequent
  // calls are faster.  For this to work correctly, the expression should
  // only be used in one place, so the environment doesn't change between
  // calls.
  virtual types::ty *cgetType(coenv &e) {
#ifdef DEBUG_CACHE
    testCachedType(e);
#endif
    return ct ? ct : ct = getType(e);
  }

  void testCachedType(coenv &e);

  // The expression is being written.  Translate code such that the value
  // (represented by the exp value) is stored into the address represented by
  // this expression.
  // In terms of side-effects, this expression must be evaluated (once) before
  // value is evaluated (once).
  virtual void transWrite(coenv &e, types::ty *t, exp *value) {
    em.error(getPos());
    em << "expression cannot be used as an address";

    // Translate the value for errors.
    value->transToType(e, t);
  }

  // Translates code for calling a function.  The arguments, in the order they
  // appear in the function's signature, must all be on the stack.
  virtual void transCall(coenv &e, types::ty *target);

  // transConditionalJump must produce code equivalent to the following:
  // Evaluate the expression as a boolean.  If the result equals cond, jump to
  // the label dest, otherwise do not jump.  In either case, no value is left
  // on the stack.
  virtual void transConditionalJump(coenv &e, bool cond, label dest);

  // This is used to ensure the proper order and number of evaluations.  When
  // called, it immediately translates code to perform the side-effects
  // consistent with a corresponding call to transAsType(e, target).
  //
  // The return value, called an evaluation for lack of a better name, is
  // another expression that responds to the trans methods exactly as would the
  // original expression, but without producing side-effects.  It is also no
  // longer overloaded, due to the resolution effected by giving a target type
  // to evaluate().
  //
  // The methods transAsType, transWrite, and transCall of the evaluation must
  // be called with the same target type as the original call to evaluate.
  // When evaluate() is called during the translation of a function, that
  // function must still be in translation when the evaluation is translated.
  //
  // The base implementation uses a tempExp (see below).  This is
  // sufficient for most expressions.
  virtual exp *evaluate(coenv &e, types::ty *target);

  // NOTE: could add a "side-effects" method which says if the expression has
  // side-effects.  This might allow some small optimizations in translating.
};

class tempExp : public exp {
  access *a;
  types::ty *t;

public:
  tempExp(coenv &e, varinit *v, types::ty *t);

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);

  types::ty *getType(coenv &) {
    return t;
  }
};

// Wrap a varEntry so that it can be used as an expression.
// Translating the varEntry must cause no side-effects.
class varEntryExp : public exp {
  trans::varEntry *v;
public:
  varEntryExp(position pos, trans::varEntry *v)
    : exp(pos), v(v) {}
  varEntryExp(position pos, types::ty *t, access *a);
  varEntryExp(position pos, types::ty *t, vm::bltin f);

  void prettyprint(ostream &out, Int indent);

  types::ty *getType(coenv &);
  types::ty *trans(coenv &e);
  trans::varEntry *getCallee(coenv &e, types::signature *sig);

  void transAct(action act, coenv &e, types::ty *target);
  void transAsType(coenv &e, types::ty *target);
  void transWrite(coenv &e, types::ty *t, exp *value);
  void transCall(coenv &e, types::ty *target);
};

class nameExp : public exp {
  name *value;

public:
  nameExp(position pos, name *value)
    : exp(pos), value(value) {}

  nameExp(position pos, symbol id)
    : exp(pos), value(new simpleName(pos, id)) {}

  nameExp(position pos, string s)
    : exp(pos), value(new simpleName(pos, symbol::trans(s))) {}

  void prettyprint(ostream &out, Int indent) override;
  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;

  symbol getName() override
  {
    return value->getName();
  }

  void transAsType(coenv &e, types::ty *target) override {
    value->varTrans(trans::READ, e, target);

    // After translation, the cached type is no longer needed and should be
    // garbage collected.  This could presumably be done in every class derived
    // from exp, but here it is most important as nameExp can have heavily
    // overloaded types cached.
    ct=0;
  }

  types::ty *trans(coenv &e) override {
    types::ty *t=cgetType(e);
    if (t->kind == types::ty_error) {
      em.error(getPos());
      em << "no matching variable \'" << *value << "\'";
      return types::primError();
    }
    if (t->kind == types::ty_overloaded) {
      em.error(getPos());
      em << "use of variable \'" << *value << "\' is ambiguous";
      return types::primError();
    }
    else {
      transAsType(e, t);
      return t;
    }
  }

  types::ty *getType(coenv &e) override {
    types::ty *t=value->varGetType(e);
    return t ? t : types::primError();
  }

  trans::varEntry *getCallee(coenv &e, types::signature *sig) override {
#ifdef DEBUG_GETAPP
    cout << "nameExp" << endl;
#endif
    return value->getCallee(e, sig);
  }

  void transWrite(coenv &e, types::ty *target, exp *newValue) override {
    newValue->transToType(e, target);
    this->value->varTrans(trans::WRITE, e, target);

    ct=0;  // See note in transAsType.
  }

  void transCall(coenv &e, types::ty *target) override {
    value->varTrans(trans::CALL, e, target);

    ct=0;  // See note in transAsType.
  }

  exp *evaluate(coenv &, types::ty *) override {
    // Names have no side-effects.
    return this;
  }
};

// Most fields accessed are handled as parts of qualified names, but in cases
// like f().x or (new t).x, a separate expression is needed.
class fieldExp : public nameExp {
  exp *object;
  symbol field;

  // fieldExp has a lot of common functionality with qualifiedName, so we
  // essentially hack qualifiedName, by making our object expression look
  // like a name.
  class pseudoName : public name {
    exp *object;

  public:
    pseudoName(exp *object)
      : name(object->getPos()), object(object) {}

    // As a variable:
    void varTrans(trans::action act, coenv &e, types::ty *target) {
      assert(act == trans::READ);
      object->transToType(e, target);
    }
    types::ty *varGetType(coenv &e) {
      return object->getType(e);
    }
    trans::varEntry *getCallee(coenv &, types::signature *) {
#ifdef DEBUG_GETAPP
      cout << "pseudoName" << endl;
#endif
      return 0;
    }

    // As a type:
    types::ty *typeTrans(coenv &, bool tacit = false) {
      if (!tacit) {
        em.error(getPos());
        em << "expression is not a type";
      }
      return types::primError();
    }

    trans::varEntry *getVarEntry(coenv &) {
      em.compiler(getPos());
      em << "expression cannot be used as part of a type";
      return 0;
    }

    trans::tyEntry *tyEntryTrans(coenv &) {
      em.compiler(getPos());
      em << "expression cannot be used as part of a type";
      return 0;
    }

    trans::frame *tyFrameTrans(coenv &) {
      return 0;
    }

    void prettyprint(ostream &out, Int indent);
    void print(ostream& out) const {
      out << "<exp>";
    }

    symbol getName() const {
      return object->getName();
    }

    AsymptoteLsp::SymbolLit getLit() const
    {
      return AsymptoteLsp::SymbolLit(static_cast<std::string>(object->getName()));
    }
  };

  // Try to get this into qualifiedName somehow.
  types::ty *getObject(coenv &e);

public:
  fieldExp(position pos, exp *object, symbol field)
    : nameExp(pos, new qualifiedName(pos,
                                     new pseudoName(object),
                                     field)),
      object(object), field(field) {}

  void prettyprint(ostream &out, Int indent);

  symbol getName()
  {
    return field;
  }

  exp *evaluate(coenv &e, types::ty *) {
    // Evaluate the object.
    return new fieldExp(getPos(),
                        new tempExp(e, object, getObject(e)),
                        field);
  }
};

class arrayExp : public exp {
protected:
  exp *set;

  array *getArrayType(coenv &e);
  array *transArray(coenv &e);

public:
  arrayExp(position pos, exp *set)
    : exp(pos), set(set) {}
};


class subscriptExp : public arrayExp {
  exp *index;

public:
  subscriptExp(position pos, exp *set, exp *index)
    : arrayExp(pos, set), index(index) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  types::ty *getType(coenv &e);
  void transWrite(coenv &e, types::ty *t, exp *value);

  exp *evaluate(coenv &e, types::ty *) {
    return new subscriptExp(getPos(),
                            new tempExp(e, set, getArrayType(e)),
                            new tempExp(e, index, types::primInt()));
  }
};

class slice : public absyn {
  exp *left;
  exp *right;

public:
  slice(position pos, exp *left, exp *right)
    : absyn(pos), left(left), right(right) {}

  void prettyprint(ostream &out, Int indent);

  exp *getLeft() { return left; }
  exp *getRight() { return right; }

  // Translates code to put the left and right expressions on the stack (in that
  // order).  If left is omitted, zero is pushed on the stack in it's place.  If
  // right is omitted, nothing is pushed in its place.
  void trans(coenv &e);

  slice *evaluate(coenv &e) {
    return new slice(getPos(),
                     left ? new tempExp(e, left, types::primInt()) : 0,
                     right ? new tempExp(e, right, types::primInt()) : 0);
  }
};

class sliceExp : public arrayExp {
  slice *index;

public:
  sliceExp(position pos, exp *set, slice *index)
    : arrayExp(pos, set), index(index) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  types::ty *getType(coenv &e);
  void transWrite(coenv &e, types::ty *t, exp *value);

  exp *evaluate(coenv &e, types::ty *) {
    return new sliceExp(getPos(),
                        new tempExp(e, set, getArrayType(e)),
                        index->evaluate(e));
  }
};


// The expression "this," that evaluates to the lexically enclosing record.
class thisExp : public exp {
public:
  thisExp(position pos)
    : exp(pos) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  types::ty *getType(coenv &e);

  exp *evaluate(coenv &, types::ty *) {
    // this has no side-effects
    return this;
  }
};

class literalExp : public exp {
public:
  literalExp(position pos)
    : exp(pos) {}

  bool scalable() { return false; }

  exp *evaluate(coenv &, types::ty *) {
    // Literals are constant, they have no side-effects.
    return this;
  }
};

class intExp : public literalExp {
  Int value;

public:
  intExp(position pos, Int value)
    : literalExp(pos), value(value) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  types::ty *getType(coenv &) { return types::primInt(); }

  template<typename T>
  [[nodiscard]]
  T getValue() const
  {
    return static_cast<T>(value);
  }
};

class realExp : public literalExp {
protected:
  double value;

public:
  realExp(position pos, double value)
    : literalExp(pos), value(value) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  types::ty *getType(coenv &) { return types::primReal(); }

  template<typename T>
  [[nodiscard]]
  T getValue() const
  {
    return static_cast<T>(value);
  }
};


class stringExp : public literalExp {
  string str;

public:
  stringExp(position pos, string str)
    : literalExp(pos), str(str) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  types::ty *getType(coenv &) { return types::primString(); }

  const string& getString() { return str; }
};

class booleanExp : public literalExp {
  bool value;

public:
  booleanExp(position pos, bool value)
    : literalExp(pos), value(value) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  types::ty *getType(coenv &) { return types::primBoolean(); }
};

class cycleExp : public literalExp {

public:
  cycleExp(position pos)
    : literalExp(pos) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  types::ty *getType(coenv &) { return types::primCycleToken(); }
};

class newPictureExp : public literalExp {

public:
  newPictureExp(position pos)
    : literalExp(pos) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  types::ty *getType(coenv &) { return types::primPicture(); }
};

class nullPathExp : public literalExp {

public:
  nullPathExp(position pos)
    : literalExp(pos) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  types::ty *getType(coenv &) { return types::primPath(); }
};

class nullExp : public literalExp {

public:
  nullExp(position pos)
    : literalExp(pos) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  types::ty *getType(coenv &) { return types::primNull(); }
};

class quoteExp : public exp {
  runnable *value;

public:
  quoteExp(position pos, runnable *value)
    : exp(pos), value(value) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  types::ty *getType(coenv &) { return types::primCode(); }
};

// A list of expressions used in a function call.
class explist : public absyn {
  typedef mem::vector<exp *> expvector;
  expvector exps;

public:
  explist(position pos)
    : absyn(pos) {}

  virtual ~explist() {}

  virtual void add(exp *e) {
    exps.push_back(e);
  }

  virtual void prettyprint(ostream &out, Int indent);

  virtual size_t size() {
    return exps.size();
  }

  virtual exp * operator[] (size_t index) {
    return exps[index];
  }
};

struct argument {
  exp *val;
  symbol name;

  // No constructor due to the union in camp.y
#if 0
  argument(exp *val=0, symbol name=0)
    : val(val), name(name) {}
#endif

  void prettyprint(ostream &out, Int indent);
  void createSymMap(AsymptoteLsp::SymbolContext* symContext);
};

class arglist : public gc {
public:
  typedef mem::vector<argument> argvector;

  argvector args;
  argument rest;

  // As the language allows named arguments after rest arguments, store the
  // index of the rest argument in order to ensure proper left-to-right
  // execution.
  static const size_t DUMMY_REST_POSITION = 9999;
  size_t restPosition;

  arglist()
    : args(), rest(), restPosition(DUMMY_REST_POSITION) {}

  virtual ~arglist() {}

  virtual void addFront(argument a) {
    args.insert(args.begin(), a);
  }

  virtual void addFront(exp *val, symbol name=symbol::nullsym) {
    argument a; a.val=val; a.name=name;
    addFront(a);
  }

  virtual void add(argument a) {
    if (rest.val && !a.name) {
      em.error(a.val->getPos());
      em << "unnamed argument after rest argument";
      return;
    }
    args.push_back(a);
  }

  virtual void add(exp *val, symbol name=symbol::nullsym) {
    argument a; a.val=val; a.name=name;
    add(a);
  }

  virtual void addRest(argument a) {
    if (rest.val) {
      em.error(a.val->getPos());
      em << "additional rest argument";
      return;
    }

    rest = a;

    assert(restPosition == DUMMY_REST_POSITION);
    restPosition = size();
  }

  virtual void prettyprint(ostream &out, Int indent);

  virtual size_t size() {
    return args.size();
  }

  virtual argument& operator[] (size_t index) {
    return args[index];
  }

  virtual argument& getRest() {
    return rest;
  }

  virtual void createSymMap(AsymptoteLsp::SymbolContext* symContext);
};

// callExp has a global cache of resolved overloaded functions.  This clears
// this cache so the associated data can be garbage collected.
void clearCachedCalls();

class callExp : public exp {
protected:
  exp *callee;
  arglist *args;

private:
  // Per object caching - Cache the application when it's determined.
  application *cachedApp;

  // In special cases, no application object is needed and we can store the
  // varEntry used in advance.
  trans::varEntry *cachedVarEntry;

  types::signature *argTypes(coenv& e, bool *searchable);
  void reportArgErrors(coenv &e);
  application *resolve(coenv &e,
                       types::overloaded *o,
                       types::signature *source,
                       bool tacit);
  application *resolveWithCache(coenv &e,
                                types::overloaded *o,
                                types::signature *source,
                                bool tacit);
  void reportMismatch(types::function *ft,
                      types::signature *source);

  void reportNonFunction();

  // Caches either the application object used to apply the function to the
  // arguments, or in cases where the arguments match the function perfectly,
  // the varEntry of the callee (or neither in case of an error).  Returns
  // what getType should return.
  types::ty *cacheAppOrVarEntry(coenv &e, bool tacit);

  types::ty *transPerfectMatch(coenv &e);
public:
  callExp(position pos, exp *callee, arglist *args)
    : exp(pos), callee(callee), args(args),
      cachedApp(0), cachedVarEntry(0) { assert(args); }

  callExp(position pos, exp *callee)
    : exp(pos), callee(callee), args(new arglist()),
      cachedApp(0), cachedVarEntry(0) {}

  callExp(position pos, exp *callee, exp *arg1)
    : exp(pos), callee(callee), args(new arglist()),
      cachedApp(0), cachedVarEntry(0) {
    args->add(arg1);
  }

  callExp(position pos, exp *callee, exp *arg1, exp *arg2)
    : exp(pos), callee(callee), args(new arglist()),
      cachedApp(0), cachedVarEntry(0) {
    args->add(arg1);
    args->add(arg2);
  }

  callExp(position pos, exp *callee, exp *arg1, exp *arg2, exp *arg3)
    : exp(pos), callee(callee), args(new arglist()),
      cachedApp(0), cachedVarEntry(0) {
    args->add(arg1);
    args->add(arg2);
    args->add(arg3);
  }

  void prettyprint(ostream &out, Int indent) override;
  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;

  using colorInfo = std::tuple<double, double, double>;

  /**
   * @return nullopt if callExp is not a color, pair<color, nullopt> if color is RGB,
   * and pair<color, alpha> if color is RGBA.
   */
  optional<std::tuple<colorInfo, optional<double>,
    AsymptoteLsp::posInFile, AsymptoteLsp::posInFile>> getColorInformation();

  types::ty *trans(coenv &e) override;
  types::ty *getType(coenv &e) override;

  // Returns true if the function call resolves uniquely without error.  Used
  // in implementing the special == and != operators for functions.
  virtual bool resolved(coenv &e);
};


class pairExp : public exp {
  exp *x;
  exp *y;

public:
  pairExp(position pos, exp *x, exp *y)
    : exp(pos), x(x), y(y) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  types::ty *getType(coenv &) { return types::primPair(); }
};

class tripleExp : public exp {
  exp *x;
  exp *y;
  exp *z;

public:
  tripleExp(position pos, exp *x, exp *y, exp *z)
    : exp(pos), x(x), y(y), z(z) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  types::ty *getType(coenv &) { return types::primTriple(); }
};

class transformExp : public exp {
  exp *x;
  exp *y;
  exp *xx,*xy,*yx,*yy;

public:
  transformExp(position pos, exp *x, exp *y, exp *xx, exp *xy, exp *yx,
               exp *yy)
    : exp(pos), x(x), y(y), xx(xx), xy(xy), yx(yx), yy(yy) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  types::ty *getType(coenv &) { return types::primTransform(); }
};

class castExp : public exp {
  ty *target;
  exp *castee;

  types::ty *tryCast(coenv &e, types::ty *t, types::ty *s,
                     symbol csym);
public:
  castExp(position pos, ty *target, exp *castee)
    : exp(pos), target(target), castee(castee) {}

  void prettyprint(ostream &out, Int indent) override;

  types::ty *trans(coenv &e) override;
  types::ty *getType(coenv &e) override;

  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;
};

class nullaryExp : public callExp {
public:
  nullaryExp(position pos, symbol op)
    : callExp(pos, new nameExp(pos, op)) {}
};

class unaryExp : public callExp {
public:
  unaryExp(position pos, exp *base, symbol op)
    : callExp(pos, new nameExp(pos, op), base) {}
};

class binaryExp : public callExp {
public:
  binaryExp(position pos, exp *left, symbol op, exp *right)
    : callExp(pos, new nameExp(pos, op), left, right) {}
};

class equalityExp : public callExp {
public:
  equalityExp(position pos, exp *left, symbol op, exp *right)
    : callExp(pos, new nameExp(pos, op), left, right) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  types::ty *getType(coenv &e);
};


// Scaling expressions such as 3sin(x).
class scaleExp : public binaryExp {
  exp *getLeft() {
    return (*this->args)[0].val;
  }
  exp *getRight() {
    return (*this->args)[1].val;
  }
public:
  scaleExp(position pos, exp *left, exp *right)
    : binaryExp(pos, left, symbol::trans("*"), right) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  //types::ty *getType(coenv &e);

  bool scalable() { return false; }
};

// Used for tension, which takes two real values, and a boolean to denote if it
// is a tension atleast case.
class ternaryExp : public callExp {
public:
  ternaryExp(position pos, exp *left, symbol op, exp *right, exp *last)
    : callExp(pos, new nameExp(pos, op), left, right, last) {}
};

// The a ? b : c ternary operator.
class conditionalExp : public exp {
  exp *test;
  exp *onTrue;
  exp *onFalse;

public:
  conditionalExp(position pos, exp *test, exp *onTrue, exp *onFalse)
    : exp(pos), test(test), onTrue(onTrue), onFalse(onFalse) {}

  void prettyprint(ostream &out, Int indent);

  void baseTransToType(coenv &e, types::ty *target);

  void transToType(coenv &e, types::ty *target);
  types::ty *trans(coenv &e);
  types::ty *getType(coenv &e);

};

class andOrExp : public exp {
protected:
  exp *left;
  symbol op;
  exp *right;

public:
  andOrExp(position pos, exp *left, symbol op, exp *right)
    : exp(pos), left(left), op(op), right(right) {}

  virtual types::ty *trans(coenv &e) override = 0;
  virtual types::ty *getType(coenv &) override {
    return types::primBoolean();
  }

  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override
  {
    left->createSymMap(symContext);
    right->createSymMap(symContext);
  }
};

class orExp : public andOrExp {
public:
  orExp(position pos, exp *left, symbol op, exp *right)
    : andOrExp(pos, left, op, right) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  void transConditionalJump(coenv &e, bool cond, label dest);
};

class andExp : public andOrExp {
public:
  andExp(position pos, exp *left, symbol op, exp *right)
    : andOrExp(pos, left, op, right) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  void transConditionalJump(coenv &e, bool cond, label dest);
};

class joinExp : public callExp {
public:
  joinExp(position pos, symbol op)
    : callExp(pos, new nameExp(pos, op)) {}

  void pushFront(exp *e) {
    args->addFront(e);
  }
  void pushBack(exp *e) {
    args->add(e);
  }

  void prettyprint(ostream &out, Int indent);
};

class specExp : public exp {
  symbol op;
  exp *arg;
  camp::side s;

public:
#if defined(MIKTEX) && defined(OUT)
#  undef OUT
#endif
  specExp(position pos, symbol op, exp *arg, camp::side s=camp::OUT)
    : exp(pos), op(op), arg(arg), s(s) {}

  void setSide(camp::side ss) {
    s=ss;
  }

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  types::ty *getType(coenv &e);
};

class assignExp : public exp {
protected:
  exp *dest;
  exp *value;

  // This is basically a hook to facilitate selfExp.  dest is given as an
  // argument since it will be a temporary in translation in order to avoid
  // multiple evaluation.
  virtual exp *ultimateValue(exp *) {
    return value;
  }

public:
  assignExp(position pos, exp *dest, exp *value)
    : exp(pos), dest(dest), value(value) {}

  void prettyprint(ostream &out, Int indent) override;

  // Don't write the result of an assignment to the prompt.
  bool writtenToPrompt() override { return false; }

  void transAsType(coenv &e, types::ty *target) override;
  types::ty *trans(coenv &e) override;
  types::ty *getType(coenv &e) override;

  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;
};

class selfExp : public assignExp {
  symbol op;

  exp *ultimateValue(exp *dest) override {
    return new binaryExp(getPos(), dest, op, value);
  }

public:
  selfExp(position pos, exp *dest, symbol op, exp *value)
    : assignExp(pos, dest, value), op(op) {}

  void prettyprint(ostream &out, Int indent) override;

  void transAsType(coenv &e, types::ty *target) override;
};

class prefixExp : public exp {
  exp *dest;
  symbol op;

public:
  prefixExp(position pos, exp *dest, symbol op)
    : exp(pos), dest(dest), op(op) {}

  void prettyprint(ostream &out, Int indent) override;

  bool scalable() override { return false; }

  // Don't write the result to the prompt.
  bool writtenToPrompt() override { return false; }

  types::ty *trans(coenv &e) override;
  types::ty *getType(coenv &e) override;
};

// Postfix expresions are illegal. This is caught here as we can give a
// more meaningful error message to the user, rather than a "parse
// error."
class postfixExp : public exp {
  exp *dest;
  symbol op;

public:
  postfixExp(position pos, exp *dest, symbol op)
    : exp(pos), dest(dest), op(op) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *trans(coenv &e);
  types::ty *getType(coenv &) { return types::primError(); }
};

} // namespace absyntax

#endif
