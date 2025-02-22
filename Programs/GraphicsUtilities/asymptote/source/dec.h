/*****
 * dec.h
 * Andy Hammerlindl 2002/8/29
 *
 * Represents the abstract syntax tree for declarations in the language.
 * Also included is abstract syntax for types as they are most often
 * used with declarations.
 *****/

#ifndef DEC_H
#define DEC_H

#include "symbol.h"
#include "absyn.h"
#include "name.h"
#include "varinit.h"
#include "modifier.h"

namespace trans {
class coenv;
class genv;
class protoenv;
class varEntry;
class access;
}

namespace types {
class ty;
struct formal;
struct signature;
struct function;
}

namespace vm {
struct lambda;
}
namespace absyntax {

using trans::genv;
using trans::coenv;
using trans::protoenv;
using trans::varEntry;
using trans::access;
using sym::symbol;

class vardec;

enum class AutounravelOption {
  Apply,
  DoNotApply,
};

class astType : public absyn {
public:
  astType(position pos)
    : absyn(pos) {}

  virtual void prettyprint(ostream &out, Int indent) = 0;

  // If we introduced a new type, automatically add corresponding functions for
  // that type.
  virtual void
  addOps(coenv&, record*, AutounravelOption opt= AutounravelOption::Apply)
  {}

  // Returns the internal representation of the type.  This method can
  // be called by exp::getType which does not report errors, so tacit is
  // needed to silence errors in this case.
  virtual types::ty *trans(coenv &e, bool tacit = false) = 0;

  virtual trans::tyEntry *transAsTyEntry(coenv &e, record *where);

  virtual operator string() const = 0;
#ifdef USEGC
  operator std::string() const {
    return mem::stdString(this->operator string());
  }
#endif
};

class nameTy : public astType {
  name *id;

public:
  nameTy(position pos, name *id)
    : astType(pos), id(id) {}

  nameTy(name *id)
    : astType(id->getPos()), id(id) {}

  void prettyprint(ostream &out, Int indent) override;

  void
  addOps(coenv& e, record* r,
         AutounravelOption opt= AutounravelOption::Apply) override;
  types::ty *trans(coenv &e, bool tacit = false) override;
  trans::tyEntry *transAsTyEntry(coenv &e, record *where) override;

  virtual operator string() const override;
};

class dimensions : public absyn {
  size_t depth;
public:
  dimensions(position pos)
    : absyn(pos), depth(1) {}

  void prettyprint(ostream &out, Int indent);

  void increase()
  { depth++; }

  size_t size() {
    return depth;
  }

  types::array *truetype(types::ty *base, bool tacit=false);
};

class arrayTy : public astType {
  astType *cell;
  dimensions *dims;

public:
  arrayTy(position pos, astType *cell, dimensions *dims)
    : astType(pos), cell(cell), dims(dims) {}

  arrayTy(name *id, dimensions *dims)
    : astType(dims->getPos()), cell(new nameTy(id)), dims(dims) {}

  void prettyprint(ostream &out, Int indent) override;

  void
  addOps(coenv& e, record* r,
         AutounravelOption opt= AutounravelOption::Apply) override;

  types::ty *trans(coenv &e, bool tacit = false) override;

  operator string() const override;
};

// Similar to varEntryExp, this helper class always translates to the same
// fixed type.
class tyEntryTy : public astType {
  trans::tyEntry *ent;
public:
  tyEntryTy(position pos, trans::tyEntry *ent)
    : astType(pos), ent(ent) {}

  tyEntryTy(position pos, types::ty *t);

  void prettyprint(ostream &out, Int indent) override;

  types::ty *trans(coenv &e, bool tacit = false) override;
  trans::tyEntry *transAsTyEntry(coenv &, record *) override {
    return ent;
  }
  operator string() const override;
};

// Runnable is anything that can be executed by the program, including
// any declaration or statement.
class runnable : public absyn {
public:
  runnable(position pos)
    : absyn(pos) {}

  virtual void prettyprint(ostream &out, Int indent) = 0;

  void markTrans(coenv &e)
  {
    markPos(e);
    trans(e);
  }

  /* Translates the stm or dec as if it were in a function definition. */
  virtual void trans(coenv &e) {
    transAsField(e, 0);
  }

  /* This can be overridden, to specify a special way of translating the code
   * when it is run at the top of the interactive prompt.
   */
  virtual void interactiveTrans(coenv &e) {
    trans(e);
  }

  void markTransAsField(coenv &e, record *r)
  {
    markPos(e);
    transAsField(e,r);
  }

  /* Translate the runnable as in the lowest lexical scope of a record
   * definition.  If it is simply a statement, it will be added to the
   * record's initializer.  A declaration, however, will also have to
   * add a new type or field to the record.
   */
  virtual void transAsField(coenv &e, record *) = 0;

  virtual vm::lambda *transAsCodelet(coenv &e);

  // For functions that return a value, we must guarantee that they end
  // with a return statement.  This checks for that condition.
  virtual bool returns()
  { return false; }

  // Returns true if it is syntactically allowable to modify this
  // runnable by a PUBLIC or PRIVATE modifier.
  virtual bool allowPermissions()
  { return false; }
};

// Forward declaration.
class formals;

class namedTy : public gc {
public:
  symbol dest;
  types::ty *t;
  position pos;
  namedTy(position pos, symbol dest, types::ty *t)
    : dest(dest), t(t), pos(pos) {}
};

class receiveTypedefDec;

class block : public runnable {
public:
  mem::list<runnable *> stms;

  // If the runnables should be interpreted in their own scope.
  bool scope;

protected:
  void prettystms(ostream &out, Int indent);

private:
  // If the first runnable exists and is a receiveTypedefDec*, return it;
  // otherwise return nullptr.
  receiveTypedefDec* getTypedefDec();

public:
  block(position pos, bool scope=true)
    : runnable(pos), scope(scope) {}

  // To ensure list deallocates properly.
  virtual ~block() {}

  void add(runnable *r) {
    stms.push_back(r);
  }

  void prettyprint(ostream &out, Int indent) override;

  void trans(coenv &e) override;

  void transAsField(coenv &e, record *r) override;

  bool transAsTemplatedField(
    coenv &e, record *r, mem::vector<absyntax::namedTy*>* args
  );

  types::record *transAsFile(genv& ge, symbol id);

  types::record *transAsTemplatedFile(
      genv& ge,
      symbol id,
      mem::vector<absyntax::namedTy*> *args
  );

  // If the block can be interpreted as a single vardec, return that vardec
  // (otherwise 0).
  vardec *asVardec();

  // A block is guaranteed to return iff one of the runnables is guaranteed to
  // return.
  // This is conservative in that
  //
  // int f(int x)
  // {
  //   if (x==1) return 0;
  //   if (x!=1) return 1;
  // }
  //
  // is not guaranteed to return.
  bool returns() override;

  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;
};

class modifierList : public absyn {
  mem::list<trans::permission> perms;
  mem::list<trans::modifier> mods;

public:
  modifierList(position pos)
    : absyn(pos) {}

  virtual ~modifierList()
  {}

  void prettyprint(ostream &out, Int indent);

  void add(trans::permission p)
  {
    perms.push_back(p);
  }

  void add(trans::modifier m)
  {
    mods.push_back(m);
  }

  /* True if a static or dynamic modifier is present.
   */
  bool staticSet();

  /* Says if the modifiers indicate static or dynamic. Prints error if
   * there are duplicates.
   */
  trans::modifier getModifier();

  /* Says if it is declared public, private, or read-only (default).
   * Prints error if there are duplicates.
   */
  trans::permission getPermission();
};

// Modifiers of static or dynamic can change the way declarations and
// statements are encoded.
class modifiedRunnable : public runnable {
  modifierList *mods;
  runnable *body;

public:
  modifiedRunnable(position pos, modifierList *mods, runnable *body)
    : runnable(pos), mods(mods), body(body)  {}

  modifiedRunnable(position pos, trans::permission perm, runnable *body)
    : runnable(pos), mods(new modifierList(pos)), body(body) {
    mods->add(perm);
  }

  void prettyprint(ostream &out, Int indent);

  void transAsField(coenv &e, record *r);

  bool returns()
  { return body->returns(); }
};


class decidstart : public absyn {
protected:
  symbol id;
  dimensions *dims;

public:
  decidstart(position pos, symbol id, dimensions *dims = 0)
    : absyn(pos), id(id), dims(dims) {}

  virtual void prettyprint(ostream &out, Int indent) override;

  virtual types::ty *getType(types::ty *base, coenv &, bool = false);
  virtual trans::tyEntry *getTyEntry(trans::tyEntry *base, coenv &e,
                                     record *where);

  // If a new type is formed by adding dimensions (or a function signature)
  // after the id, this will add the standard functions for that new type.
  virtual void addOps(types::ty *base, coenv &e, record *r);

  virtual symbol getName()
  { return id; }

  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;
  void createSymMapWType(
      AsymptoteLsp::SymbolContext* symContext, absyntax::astType* base
  );
};

class fundecidstart : public decidstart {
  formals *params;

public:
  fundecidstart(position pos,
                symbol id,
                dimensions *dims = 0,
                formals *params = 0)
    : decidstart(pos, id, dims), params(params) {}

  void prettyprint(ostream &out, Int indent);

  types::ty *getType(types::ty *base, coenv &e, bool tacit = false);
  trans::tyEntry *getTyEntry(trans::tyEntry *base, coenv &e, record *where);
  void addOps(types::ty *base, coenv &e, record *r);
};

class decid : public absyn {
  decidstart *start;
  varinit *init;

  // Returns the default initializer for the type.
  access *defaultInit(coenv &e, types::ty *t);

public:
  decid(position pos, decidstart *start, varinit *init = 0)
    : absyn(pos), start(start), init(init) {}

  virtual void prettyprint(ostream &out, Int indent) override;

  virtual void transAsField(coenv &e, record *r, types::ty *base);

  // Translate, but add the names in as types rather than variables.
  virtual void transAsTypedefField(coenv &e, trans::tyEntry *base, record *r);

  decidstart *getStart() { return start; }

  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;
  void createSymMapWType(
      AsymptoteLsp::SymbolContext* symContext, absyntax::astType* base
  );
};

class decidlist : public absyn {
  mem::list<decid *> decs;

public:
  decidlist(position pos)
    : absyn(pos) {}

  virtual ~decidlist() {}

  void add(decid *p) {
    decs.push_back(p);
  }

  virtual void prettyprint(ostream &out, Int indent) override;

  virtual void transAsField(coenv &e, record *r, types::ty *base);

  // Translate, but add the names in as types rather than variables.
  virtual void transAsTypedefField(coenv &e, trans::tyEntry *base, record *r);

  // If the list consists of a single entry, return it.
  decid *singleEntry()
  {
    if (decs.size() == 1)
      return decs.front();
    else
      return 0;
  }

  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;
  void createSymMapWType(
      AsymptoteLsp::SymbolContext* symContext, absyntax::astType* base
  );
};

class dec : public runnable {
public:
  dec(position pos)
    : runnable(pos) {}

  void prettyprint(ostream &out, Int indent) override;

  // Declarations can be public or private.
  bool allowPermissions() override
  { return true; }
};

void createVar(position pos, coenv &e, record *r,
               symbol id, types::ty *t, varinit *init);

class vardec : public dec {
  astType *base;
  decidlist *decs;

public:
  vardec(position pos, astType *base, decidlist *decs)
    : dec(pos), base(base), decs(decs) {}

  vardec(position pos, astType *base, decid *di)
    : dec(pos), base(base), decs(new decidlist(pos))
  {
    decs->add(di);
  }
  void prettyprint(ostream &out, Int indent) override;

  void transAsField(coenv &e, record *r) override
  {
    base->addOps(e, r, AutounravelOption::DoNotApply);
    decs->transAsField(e, r, base->trans(e));
  }

  // Translate, but add the names in as types rather than variables.
  virtual void transAsTypedefField(coenv &e, record *r);

  // If the vardec encodes a single declaration, return the name of that
  // declaration (otherwise nullsym).
  symbol singleName();

  // If the vardec encodes a single declaration, return the type of that
  // declaration (otherwise 0).
  types::ty *singleGetType(coenv& e);

  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;
};

struct idpair : public absyn {
  symbol src; // The name of the module to access.
  symbol dest;  // What to call it in the local environment.
  bool valid; // If it parsed properly.

  idpair(position pos, symbol id)
    : absyn(pos), src(id), dest(id), valid(true) {}

  idpair(position pos, symbol src, symbol as, symbol dest)
    : absyn(pos), src(src), dest(dest), valid(as==symbol::trans("as")) {}

  idpair(position pos, string src, symbol as, symbol dest)
    : absyn(pos), src(symbol::trans(src)), dest(dest),
      valid(as==symbol::trans("as")) {}

  void checkValidity() {
    if (!valid) {
      em.error(getPos());
      em << "expected 'as'";
    }
  }

  void prettyprint(ostream &out, Int indent) override;

  // Translates as: access src as dest;
  void transAsAccess(coenv &e, record *r);

  // Translates as: from _ unravel src as dest;
  // where _ is the qualifier record with source as its fields and types.
  trans::tyEntry *transAsUnravel(coenv &e, record *r,
                                 protoenv &source, varEntry *qualifier);

  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;
};

struct idpairlist : public gc {
  mem::list<idpair *> base;

  void add(idpair *x) {
    base.push_back(x);
  }

  void prettyprint(ostream &out, Int indent);

  void transAsAccess(coenv &e, record *r);

  mem::vector<trans::tyEntry*> transAsUnravel(
    coenv &e, record *r, protoenv &source, varEntry *qualifier
  );

  void createSymMap(AsymptoteLsp::SymbolContext* symContext);

  template<typename TFn>
  void processListFn(TFn const& fn)
  {
    for (auto* idp : base)
    {
      fn(idp->src, idp->dest);
    }
  }
};

extern idpairlist * const WILDCARD;


class accessdec : public dec {
  idpairlist *base;

public:
  accessdec(position pos, idpairlist *base)
    : dec(pos), base(base) {}

  void prettyprint(ostream &out, Int indent) override;

  void transAsField(coenv &e, record *r) override {
    base->transAsAccess(e,r);
  }

  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;
};

class badDec : public dec {
  position errorPos;
  string errorMessage;

public:
  badDec(position pos, position errorPos, string errorMessage)
      : dec(pos), errorPos(errorPos), errorMessage(errorMessage) {}

  void transAsField(coenv&, record*) override {
    em.error(errorPos);
    em << errorMessage;
  }
};

// Accesses the file with specified types added to the type environment.
class templateAccessDec : public dec {
  symbol src; // The name of the module to access.
  formals *args;
  symbol dest;  // What to call it in the local environment.
  bool valid;
  position expectedAsPos;

public:
  templateAccessDec(position pos, symbol src, formals* args, symbol as,
                    symbol dest, position asPos)
      : dec(pos), src(src), args(args), dest(dest),
        valid(as == symbol::trans("as")), expectedAsPos(asPos) {}

  bool checkValidity() {
    if (!valid) {
      em.error(expectedAsPos);
      em << "expected 'as'";
      return false;
    }
    return true;
  }

  void transAsField(coenv& e, record* r) override;
};

class typeParam : public absyn {
  const symbol paramSym;
public:
  typeParam(position pos, symbol paramSym)
    : absyn(pos), paramSym(paramSym) {}

  bool transAsParamMatcher(coenv &e, record *r, namedTy *arg);

  void prettyprint(ostream &out, Int indent);
};

class typeParamList : public absyn {
  mem::vector<typeParam*> params;

public:
  typeParamList(position pos) : absyn(pos) {}

  void add(typeParam *tp);

  bool transAsParamMatcher(coenv &e, record *r,
                           mem::vector<namedTy*> *args);

  void prettyprint(ostream &out, Int indent);
};


class receiveTypedefDec : public dec {
  typeParamList* params;

public:
  receiveTypedefDec(position pos, typeParamList* params)
    : dec(pos), params(params) {}

  void transAsField(coenv& e, record *r) override;
  bool transAsParamMatcher(
    coenv& e, record *r, mem::vector<namedTy*> *args
  );
};


// Abstract base class for
//   from _ access _;  (fromaccessdec)
// and
//   from _ unravel _;  (unraveldec)
class fromdec : public dec {
protected:
  struct qualifier {
    // The varEntry holds the location and the type of the highest framed
    // structure that can be put on the stack.  The record holds the actual
    // type of the qualifier.
    // For example:
    //   struct A {
    //     struct B {
    //       static int x;
    //     }
    //   }
    //   A a=new A;
    //   from a.B unravel x;
    //
    // Here, v->getType() will yield A and v->getLocation() will yield the
    // location of the variable a, but the record type t will be B.
    record *t;
    varEntry *v;

    qualifier(record *t, varEntry *v)
      : t(t), v(v) {}
  };

  // Return the qualifier from which the fields are taken.  If t==0, it is
  // assumed that an error occurred and was reported.
  virtual qualifier getQualifier(coenv &e, record *r) = 0;
  idpairlist *fields;

public:
  fromdec(position pos, idpairlist *fields)
    : dec(pos), fields(fields) {}

  void prettyprint(ostream &out, Int indent) override;

  void transAsField(coenv &e, record *r) override;
};

// An unravel declaration dumps fields and types of a record into the local
// scope.
class unraveldec : public fromdec {
  name *id;

  qualifier getQualifier(coenv &e, record *) override;
public:
  unraveldec(position pos, name *id, idpairlist *fields)
    : fromdec(pos, fields), id(id) {}

  void prettyprint(ostream &out, Int indent) override;

  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;
};

// A fromaccess declaration dumps fields and types of a module into the local
// scope.  It does not add the module as a variable in the local scope.
class fromaccessdec : public fromdec {
  symbol id;
  formals *templateArgs;

  qualifier getQualifier(coenv &e, record *r) override;
public:
  fromaccessdec(
      position pos, symbol id, idpairlist *fields, formals *templateArgs = 0
  ) : fromdec(pos, fields), id(id), templateArgs(templateArgs) {}

  void prettyprint(ostream &out, Int indent) override;

  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;
};

// An import declaration dumps fields and types of a module into the local
// scope.  It also adds the module as a variable in the local scope.
class importdec : public dec {
  block base;

public:
  importdec(position pos, idpair *id)
    : dec(pos), base(pos, false) {
    idpairlist *i=new idpairlist;
    i->add(id);
    base.add(new accessdec(pos, i));
    base.add(new unraveldec(pos, new simpleName(pos, id->dest), WILDCARD));
  }

  void trans(coenv &e) override {
    base.trans(e);
  }

  void transAsField(coenv &e, record *r) override {
    base.transAsField(e, r);
  }

  void prettyprint(ostream &out, Int indent) override;

  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;
};

// Parses the file given, and translates the resulting runnables as if they
// occurred at this place in the code.
class includedec : public dec {
  string filename;

public:
  includedec(position pos, string filename)
    : dec(pos), filename(filename) {}
  includedec(position pos, symbol id)
    : dec(pos), filename(id) {}

  void prettyprint(ostream &out, Int indent) override;
  void loadFailed(coenv &e);

  void transAsField(coenv &e, record *r) override;

  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;
};

// Types defined from others in `typedef` or `using` statement.
class typedec : public dec {
  vardec *body;

public:
  typedec(position pos, vardec *body)
    : dec(pos), body(body) {}

  typedec(position pos, decidstart *id_with_signature, astType *return_type)
    : dec(pos)
  {
    decid *di = new decid(id_with_signature->getPos(), id_with_signature);
    body = new vardec(return_type->getPos(), return_type, di);
  }

  void prettyprint(ostream &out, Int indent);

  void transAsField(coenv &e, record *r) {
    body->transAsTypedefField(e,r);
  }
};


// A struct declaration.
class recorddec : public dec {
  symbol id;
  block *body;

  void transRecordInitializer(coenv &e, record *parent);
  void addPostRecordEnvironment(coenv &e, record *r, record *parent);

public:
  recorddec(position pos, symbol id, block *body)
    : dec(pos), id(id), body(body) {}

  virtual ~recorddec()
  {}

  void prettyprint(ostream &out, Int indent) override;

  void transAsField(coenv &e, record *parent) override;

  void createSymMap(AsymptoteLsp::SymbolContext* symContext) override;
};

// Returns a runnable that facilitates the autoplain feature.
runnable *autoplainRunnable();

void addVar(coenv &e, record *r, varEntry *v, symbol id);

} // namespace absyntax

#endif
