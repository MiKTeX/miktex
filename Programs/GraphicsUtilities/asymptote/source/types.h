/*****
 * types.h
 * Andy Hammerlindl 2002/06/20
 *
 * Used by the compiler as a way to keep track of the type of a variable
 * or expression.
 *
 *****/

#ifndef TYPES_H
#define TYPES_H

#include <iostream>
#include <cstdio>
#include <cassert>

#include "errormsg.h"
#include "symbol.h"
#include "common.h"
#include "util.h"

#if defined(MIKTEX)
#include <iterator>
using std::inserter;
#endif
using std::ostream;

using sym::symbol;

// Forward declaration.
namespace trans {
class access;
class varEntry;
}
namespace absyntax {
class varinit;
extern varinit *Default;
}

namespace types {

enum ty_kind {
  ty_null,
  ty_record,    // "struct" in Asymptote language
  ty_function,
  ty_overloaded,
 
#define PRIMITIVE(name,Name,asyName) ty_##name,
#define PRIMERROR
#include "primitives.h"
#undef PRIMERROR
#undef PRIMITIVE

  ty_array
};

// Forward declarations.
class ty;
struct signature;
typedef mem::vector<ty *> ty_vector;
typedef ty_vector::iterator ty_iterator;

// Checks if two types are equal in the sense of the language.
// That is primitive types are equal if they are the same kind.
// Structures are equal if they come from the same struct definition.
// Arrays are equal if their cell types are equal.
bool equivalent(const ty *t1, const ty *t2);

// If special is true, this is the same as above.  If special is false, just the
// signatures are compared.
bool equivalent(const ty *t1, const ty *t2, bool special);

class caster {
public:
  virtual ~caster() {}
  virtual trans::access *operator() (ty *target, ty *source) = 0;
  virtual bool castable(ty *target, ty *source) = 0;
};

class ty : public gc {
public:
  const ty_kind kind;
  ty(ty_kind kind)
    : kind(kind) {}
  virtual ~ty();

  virtual void print (ostream& out) const;
  virtual void printVar (ostream& out, string name) const {
    print(out);
    out << " " << name;
  }


  // Returns true if the type is a user-defined type or the null type.
  // While the pair, path, etc. are stored by reference, this is
  // transparent to the user.
  virtual bool isReference() {
    return true;
  }

  virtual signature *getSignature() {
    return 0;
  }

  virtual const signature *getSignature() const {
    return 0;
  }

  virtual bool primitive() {
    return false;
  }

  bool isError() const { return kind == ty_error; }
  bool isNotError() const { return !isError(); }

  // The following are only used by the overloaded type, but it is so common
  // to test for an overloaded type then iterate over its types, that this
  // allows the code:
  // if (t->isOverloaded()) {
  //   for (ty_iterator i = t->begin(); i != t->end(); ++i) {
  //     ...
  //   }
  // }
  // For speed reasons, only begin has an assert to test if t is overloaded.
  bool isOverloaded() const {
    return kind == ty_overloaded;
  }
  bool isNotOverloaded() const { return !isOverloaded(); }
  ty_iterator begin();
  ty_iterator end();

  // If a default initializer is not stored in the environment, the abstract
  // syntax asks the type if it has a "default" default initializer, by calling
  // this method.
  virtual trans::access *initializer() {
    return 0;
  }

  // If a cast function is not stored in the environment, ask the type itself.
  // This handles null->record casting, and the like.  The caster is used as a 
  // callback to the environment for casts of subtypes.
  virtual trans::access *castTo(ty *, caster &) {
    return 0;
  }

  // Just checks if a cast is possible.
  virtual bool castable(ty *target, caster &c) {
    return castTo(target, c);
  }

  // For pair's x and y, and array's length, this is a special type of
  // "field".
  // In actually, it returns a function which takes the object as its
  // parameter and returns the necessary result.
  // These should not have public permission, as modifying them would
  // have strange results.
  virtual trans::varEntry *virtualField(symbol, signature *) {
    return 0;
  }

  // varGetType for virtual fields.
  // Unless you are using functions for virtual fields, the base implementation
  // should work fine.
  virtual ty *virtualFieldGetType(symbol id);

#if 0
  // Returns the type.  In case of functions, return the equivalent type
  // but with no default values for parameters.
  virtual ty *stripDefaults()
  {
    return this;
  }
#endif

  // Returns true if the other type is equivalent to this one.
  // The general function equivalent should be preferably used, as it properly
  // handles overloaded type comparisons.
  virtual bool equiv(const ty *other) const
  {
    return this==other;
  }


  // Returns a number for the type for use in a hash table.  Equivalent types
  // must yield the same number.
  virtual size_t hash() const = 0;
};

class primitiveTy : public ty {
public:
  primitiveTy(ty_kind kind)
    : ty(kind) {}
  
  bool primitive() {
    return true;
  }

  bool isReference() {
    return false;
  }
  
  ty *virtualFieldGetType(symbol );
  trans::varEntry *virtualField(symbol, signature *);

  bool equiv(const ty *other) const
  {
    return this->kind==other->kind;
  }

  size_t hash() const {
    return (size_t)kind + 47;
  }
};

class nullTy : public primitiveTy {
public:
  nullTy()
    : primitiveTy(ty_null) {}
  
  bool isReference() {
    return true;
  }

  trans::access *castTo(ty *target, caster &);

  size_t hash() const {
    return (size_t)kind + 47;
  }
};

// Ostream output, just defer to print.
inline ostream& operator<< (ostream& out, const ty& t)
{ t.print(out); return out; }

struct array : public ty {
  ty *celltype;
  ty *pushtype;
  ty *poptype;
  ty *appendtype;
  ty *inserttype;
  ty *deletetype;

  array(ty *celltype)
    : ty(ty_array), celltype(celltype), pushtype(0), poptype(0),
      appendtype(0), inserttype(0), deletetype(0) {}

  virtual bool isReference() {
    return true;
  }

  bool equiv(const ty *other) const {
    return other->kind==ty_array &&
      equivalent(this->celltype,((array *)other)->celltype);
  }

  size_t hash() const {
    return 1007 * celltype->hash();
  }

  Int depth() {
    if (array *cell=dynamic_cast<array *>(celltype))
      return cell->depth() + 1;
    else
      return 1;
  }

  void print(ostream& out) const
  { out << *celltype << "[]"; }

  ty *pushType();
  ty *popType();
  ty *appendType();
  ty *insertType();
  ty *deleteType();

  // Initialize to an empty array by default.
  trans::access *initializer();

  // NOTE: General vectorization of casts would be here.

  // Add length and push as virtual fields.
  ty *virtualFieldGetType(symbol id);
  trans::varEntry *virtualField(symbol id, signature *sig);
};

/* Base types */
#define PRIMITIVE(name,Name,asyName) \
  ty *prim##Name(); \
  ty *name##Array(); \
  ty *name##Array2(); \
  ty *name##Array3();
#define PRIMERROR
#include "primitives.h"
#undef PRIMERROR
#undef PRIMITIVE

ty *primNull();


struct formal {
  ty *t;
  symbol name;
  bool defval;
  bool Explicit;
  
  formal(ty *t,
         symbol name=symbol::nullsym,
         bool optional=false,
         bool Explicit=false)
    : t(t), name(name),
      defval(optional), Explicit(Explicit) {}

  // string->symbol translation is costly if done too many times.  This
  // constructor has been disabled to make this cost more visible to the
  // programmer.
#if 0
  formal(ty *t,
         const char *name,
         bool optional=false,
         bool Explicit=false)
    : t(t), name(symbol::trans(name)),
      defval(optional ? absyntax::Default : 0), Explicit(Explicit) {}
#endif

  friend ostream& operator<< (ostream& out, const formal& f);
};

bool equivalent(const formal& f1, const formal& f2);
bool argumentEquivalent(const formal &f1, const formal& f2);

typedef mem::vector<formal> formal_vector;

// Holds the parameters of a function and if they have default values
// (only applicable in some cases).
struct signature : public gc {
  formal_vector formals;

  // The number of keyword-only formals.  These formals always come after the
  // regular formals.
  size_t numKeywordOnly;

  // Formal for the rest parameter.  If there is no rest parameter, then the
  // type is null.
  formal rest;

  bool isOpen;

  signature()
    : numKeywordOnly(0), rest(0), isOpen(false)
  {}

  static const struct OPEN_t {} OPEN;

  explicit signature(OPEN_t) : numKeywordOnly(0), rest(0), isOpen(true) {}

  signature(signature &sig)
    : formals(sig.formals), numKeywordOnly(sig.numKeywordOnly),
      rest(sig.rest), isOpen(sig.isOpen)
  {}

  virtual ~signature() {}

  void add(formal f) {
    formals.push_back(f);
  }

  void addKeywordOnly(formal f) {
    add(f);
    ++numKeywordOnly;
  }

  void addRest(formal f) {
    rest=f;
  }

  bool hasRest() const {
    return rest.t;
  }
  size_t getNumFormals() const {
    return rest.t ? formals.size() + 1 : formals.size();
  }

  formal& getFormal(size_t n) { 
    assert(n < formals.size());
    return formals[n];
  }
  const formal& getFormal(size_t n) const {
    assert(n < formals.size());
    return formals[n];
  }

  formal& getRest() {
    return rest;
  }
  const formal& getRest() const {
    return rest;
  }

  bool formalIsKeywordOnly(size_t n) const
  {
    assert(n < formals.size());
    return n >= formals.size() - numKeywordOnly;
  }

  friend ostream& operator<< (ostream& out, const signature& s);

  friend bool equivalent(const signature *s1, const signature *s2);

  // Check if a signature of argument types (as opposed to formal parameters)
  // are equivalent.  Here, the arguments, if named, must have the same names,
  // and (for simplicity) no overloaded arguments are allowed.
  friend bool argumentEquivalent(const signature *s1, const signature *s2);
#if 0
  friend bool castable(signature *target, signature *source);
  friend Int numFormalsMatch(signature *s1, signature *s2);
#endif

  size_t hash() const;
};

struct function : public ty {
  ty *result;
  signature sig;

  function(ty *result)
    : ty(ty_function), result(result) {}
  function(ty *result, signature::OPEN_t)
    : ty(ty_function), result(result), sig(signature::OPEN) {}
  function(ty *result, signature *sig)
    : ty(ty_function), result(result), sig(*sig) {}
  function(ty *result, formal f1)
    : ty(ty_function), result(result) {
    add(f1);
  }
  function(ty *result, formal f1, formal f2)
    : ty(ty_function), result(result) {
    add(f1);
    add(f2);
  }
  function(ty *result, formal f1, formal f2, formal f3)
    : ty(ty_function), result(result) {
    add(f1);
    add(f2);
    add(f3);
  }
  function(ty *result, formal f1, formal f2, formal f3, formal f4)
    : ty(ty_function), result(result) {
    add(f1);
    add(f2);
    add(f3);
    add(f4);
  }
  virtual ~function() {}

  void add(formal f) {
    sig.add(f);
  }

  void addRest(formal f) {
    sig.addRest(f);
  }

  virtual bool isReference() {
    return true;
  }

  bool equiv(const ty *other) const
  {
    if (other->kind==ty_function) {
      function *that=(function *)other;
      return equivalent(this->result,that->result) &&
        equivalent(&this->sig,&that->sig);
    }
    else return false;
  }

  size_t hash() const {
    return sig.hash()*0x1231+result->hash();
  }

  void print(ostream& out) const
  { out << *result << sig; }

  void printVar (ostream& out, string name) const {
    result->printVar(out,name);
    out << sig;
  }

  ty *getResult() {
    return result;
  }
  
  signature *getSignature() {
    return &sig;
  }

  const signature *getSignature() const {
    return &sig;
  }

#if 0
  ty *stripDefaults();
#endif

  // Initialized to null.
  trans::access *initializer();
};

// This is used in getType expressions when an overloaded variable is accessed.
class overloaded : public ty {
public:
  ty_vector sub;

  // Warning: The venv endScope routine relies heavily on the current
  // implementation of overloaded.
public:
  overloaded()
    : ty(ty_overloaded) {}
  overloaded(ty *t)
    : ty(ty_overloaded) { add(t); }
  virtual ~overloaded() {}

  bool equiv(const ty *other) const
  {
    for(ty_vector::const_iterator i=sub.begin();i!=sub.end();++i)
      if (equivalent(*i,other))
        return true;
    return false;
  }

  size_t hash() const {
    // Overloaded types should not be hashed.
    assert(False);
    return 0;
  }

  void add(ty *t) {
    if (t->kind == ty_overloaded) {
      overloaded *ot = (overloaded *)t;
      copy(ot->sub.begin(), ot->sub.end(),
           inserter(this->sub, this->sub.end()));
    }
    else
      sub.push_back(t);
  }

  // Only add a type distinct from the ones currently in the overloaded type.
  // If special is false, just the distinct signatures are added.
  void addDistinct(ty *t, bool special=false);

  // If there are less than two overloaded types, the type isn't really
  // overloaded.  This gives a more appropriate type in this case.
  ty *simplify() {
    switch (sub.size()) {
      case 0:
        return 0;
      case 1: {
        return sub.front();
      }
      default:
        return new overloaded(*this);
    }
  }

  // Returns the signature-less type of the set.
  ty *signatureless();

  // True if one of the subtypes is castable.
  bool castable(ty *target, caster &c);

  size_t size() const { return sub.size(); }

  // Use default printing for now.
};

inline ty_iterator ty::begin() {
  assert(this->isOverloaded());
  return ((overloaded *)this)->sub.begin();
}
inline ty_iterator ty::end() {
  return ((overloaded *)this)->sub.end();
}

// This is used to encapsulate iteration over the subtypes of an overloaded
// type.  The base method need only be implemented to handle non-overloaded
// types.
class collector {
public:
  virtual ~collector() {}
  virtual ty *base(ty *target, ty *source) = 0;

  virtual ty *collect(ty *target, ty *source) {
    if (overloaded *o=dynamic_cast<overloaded *>(target)) {
      ty_vector &sub=o->sub;

      overloaded *oo=new overloaded;
      for(ty_vector::iterator x = sub.begin(); x != sub.end(); ++x) {
        types::ty *t=collect(*x, source);
        if (t)
          oo->add(t);
      }

      return oo->simplify();
    }
    else if (overloaded *o=dynamic_cast<overloaded *>(source)) {
      ty_vector &sub=o->sub;

      overloaded *oo=new overloaded;
      for(ty_vector::iterator y = sub.begin(); y != sub.end(); ++y) {
        // NOTE: A possible speed optimization would be to replace this with a
        // call to base(), but this is only correct if we can guarantee that an
        // overloaded type has no overloaded sub-types.
        types::ty *t=collect(target, *y);
        if (t)
          oo->add(t);
      }

      return oo->simplify();
    }
    else
      return base(target, source);
  }
};

class tester {
public:
  virtual ~tester() {}
  virtual bool base(ty *target, ty *source) = 0;

  virtual bool test(ty *target, ty *source) {
    if (overloaded *o=dynamic_cast<overloaded *>(target)) {
      ty_vector &sub=o->sub;

      for(ty_vector::iterator x = sub.begin(); x != sub.end(); ++x)
        if (test(*x, source))
          return true;

      return false;
    }
    else if (overloaded *o=dynamic_cast<overloaded *>(source)) {
      ty_vector &sub=o->sub;

      for(ty_vector::iterator y = sub.begin(); y != sub.end(); ++y)
        if (base(target, *y))
          return true;

      return false;
    }
    else
      return base(target, source);
  }
};

} // namespace types

GC_DECLARE_PTRFREE(types::primitiveTy);
GC_DECLARE_PTRFREE(types::nullTy);

#endif
