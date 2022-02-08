/*****
 * fundec.h
 * Andy Hammerlindl 2002/8/29
 *
 * Defines the semantics for defining functions.  Both the newexp syntax, and
 * the abbreviated C-style function definition.
 *****/

#ifndef FUNDEC_H
#define FUNDEC_H

#include "dec.h"
#include "exp.h"

namespace absyntax {

class formal : public absyn {
  ty *base;
  decidstart *start;
  bool Explicit;
  varinit *defval;
  bool keywordOnly;

public:
  formal(position pos, ty *base, decidstart *start=0, varinit *defval=0,
         bool Explicit= false, bool keywordOnly=false)
    : absyn(pos), base(base), start(start), Explicit(Explicit),
      defval(defval), keywordOnly(keywordOnly) {}

  virtual void prettyprint(ostream &out, Int indent) override;

  // Build the corresponding types::formal to put into a signature.
  types::formal trans(coenv &e, bool encodeDefVal, bool tacit=false);

  // Add the formal parameter to the environment to prepare for the
  // function body's translation.
  virtual void transAsVar(coenv &e, Int index);

  types::ty *getType(coenv &e, bool tacit=false);

  virtual void addOps(coenv &e, record *r);

  varinit *getDefaultValue() {
    return defval;
  }

  symbol getName() {
    return start ? start->getName() : symbol::nullsym;
  }

  bool getExplicit() {
    return Explicit;
  }

  bool isKeywordOnly() {
    return keywordOnly;
  }

  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;
  std::pair<std::string, optional<std::string>> fnInfo() const;
};

class formals : public absyn {
  //friend class funheader;

  mem::list<formal *> fields;
  formal *rest;

  // If the list of formals contains at least one keyword-only formal.
  bool keywordOnly;

  void addToSignature(types::signature& sig,
                      coenv &e, bool encodeDefVal, bool tacit);
public:
  formals(position pos)
    : absyn(pos), rest(0), keywordOnly(false) {}

  virtual ~formals() {}

  virtual void prettyprint(ostream &out, Int indent) override;

  virtual void add(formal *f) {
    if (f->isKeywordOnly()) {
      keywordOnly = true;
    }
    else if (rest) {
      em.error(f->getPos());
      em << "normal parameter after rest parameter";
    }
    else if (keywordOnly) {
      em.error(f->getPos());
      em << "normal parameter after keyword-only parameter";
    }

    fields.push_back(f);
  }

  virtual void addRest(formal *f) {
    if (rest) {
      em.error(f->getPos());
      em << "additional rest parameter";
    }
    else if (f->isKeywordOnly()) {
      em.error(f->getPos());
      em << "rest parameter declared as keyword-only";
    }
    rest = f;
  }

  // Returns the types of each parameter as a signature.
  // encodeDefVal means that it will also encode information regarding
  // the default values into the signature
  types::signature *getSignature(coenv &e,
                                 bool encodeDefVal = false,
                                 bool tacit = false);

  // Returns the corresponding function type, assuming it has a return
  // value of "result."
  types::function *getType(types::ty *result, coenv &e,
                           bool encodeDefVal = false,
                           bool tacit = false);

  virtual void addOps(coenv &e, record *r);

  // Add the formal parameters to the environment to prepare for the
  // function body's translation.
  virtual void trans(coenv &e);

  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;
  void addArgumentsToFnInfo(AsymptoteLsp::FunctionInfo& fnInfo);
};

class fundef : public exp {
  ty *result;
  formals *params;
  stm *body;

  // If the fundef is part of a fundec, the name of the function is stored
  // here for debugging purposes.
  symbol id;

  friend class fundec;

public:
  fundef(position pos, ty *result, formals *params, stm *body)
    : exp(pos), result(result), params(params), body(body), id() {}

  virtual void prettyprint(ostream &out, Int indent) override;

  varinit *makeVarInit(types::function *ft);
  virtual void baseTrans(coenv &e, types::function *ft);
  virtual types::ty *trans(coenv &e) override;

  virtual types::function *transType(coenv &e, bool tacit);
  virtual types::function *transTypeAndAddOps(coenv &e, record *r, bool tacit);
  virtual types::ty *getType(coenv &e) override {
    return transType(e, true);
  }

  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;
  void addArgumentsToFnInfo(AsymptoteLsp::FunctionInfo& fnInfo);
};

class fundec : public dec {
  symbol id;
  fundef fun;

public:
  fundec(position pos, ty *result, symbol id, formals *params, stm *body)
    : dec(pos), id(id), fun(pos, result, params, body)
  { fun.id = id; }

  void prettyprint(ostream &out, Int indent) override;

  void trans(coenv &e) override;

  void transAsField(coenv &e, record *r) override;

  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;
};

} // namespace absyntax

#endif
