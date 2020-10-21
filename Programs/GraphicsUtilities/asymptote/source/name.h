/*****
 * name.h
 * Andy Hammerlindl 2002/07/14
 *
 * Qualified names (such as x, f, builtin.sin, a.b.c.d, etc.) can be used
 * either as variables or a type names.  This class stores qualified
 * names used in nameExp and nameTy in the abstract syntax, and
 * implements the exp and type functions.
 *****/

#ifndef NAME_H
#define NAME_H

#include "absyn.h"
#include "types.h"
#include "frame.h"
#include "access.h"

namespace trans {
class coenv;
class varEntry;
class tyEntry;
}
namespace types {
class record;
}

namespace absyntax {

using trans::coenv;
using trans::action;
using types::record;
using std::ostream;

class name : public absyn {
public:
  name(position pos)
    : absyn(pos) {}

  // Helper function - ensures target and source match up, using casting in the
  // case of a read.  Issues errors on failure.
  void forceEquivalency(action act, coenv &e,
                        types::ty *target, types::ty *source);

  // Used for determining the type when the context does not establish
  // the name as a variable or a type.  First, the function looks for a
  // non-function variable fitting the description.  If one fits, the
  // type of the variable is returned.  Failing that, the function looks
  // for a fitting type and returns that.  If nothing is found, an
  // appropriate error is reported and ty_error is returned.
  // Because this is used only on qualifiers (ie. names to the left of a
  // dot), it does not look at function variables.
  // Tacit means that no error messages will be reported to the user.
  virtual types::ty *getType(coenv &e, bool tacit = false);

  // Pushes the highest level frame possible onto the stack.  Returning
  // the frame pushed.  If no frame can be pushed, returns 0.
  // NOTE: This duplicates some functionality with getVarEntry.
  virtual trans::frame *frameTrans(coenv &e);
  // Helper function for the case where the name is known to be a type.
  virtual trans::frame *tyFrameTrans(coenv &e) = 0;

  // Constructs the varEntry part of the tyEntry for the name.  Like
  // getType, this is called on the qualifier, instead of the full name.
  // This reports no errors, and returns 0 if there is no varEntry to
  // use.
  virtual trans::varEntry *getVarEntry(coenv &e) = 0;

  // As a variable:
  // Translates the name (much like an expression).
  virtual void varTrans(action act, coenv &e, types::ty *target) = 0;
  // Returns the possible variable types.  Unlike exp, returns 0 if none
  // match.
  virtual types::ty *varGetType(coenv &e) = 0;
  virtual trans::varEntry *getCallee(coenv &e, types::signature *sig) = 0;

  // As a type:
  // Determines the type, as used in a variable declaration.
  virtual types::ty *typeTrans(coenv &e, bool tacit = false) = 0;
  // Constructs the tyEntry of the name, needed so that we know the
  // parent frame for allocating new objects of that type.  Reports
  // errors as typeTrans() does with tacit=false.
  virtual trans::tyEntry *tyEntryTrans(coenv &e) = 0;

  virtual void prettyprint(ostream &out, Int indent) = 0;
  virtual void print(ostream& out) const {
    out << "<base name>";
  }

  virtual symbol getName() = 0;
};

inline ostream& operator<< (ostream& out, const name& n) {
  n.print(out);
  return out;
}

class simpleName : public name {
  symbol id;

public:
  simpleName(position pos, symbol id)
    : name(pos), id(id) {}

  trans::varEntry *getVarEntry(coenv &e);

  // As a variable:
  void varTrans(action act, coenv &e, types::ty *target);
  types::ty *varGetType(coenv &);
  trans::varEntry *getCallee(coenv &e, types::signature *sig);

  // As a type:
  types::ty *typeTrans(coenv &e, bool tacit = false);
  virtual trans::tyEntry *tyEntryTrans(coenv &e);
  trans::frame *tyFrameTrans(coenv &e);

  void prettyprint(ostream &out, Int indent);
  void print(ostream& out) const {
    out << id;
  }
  symbol getName() {
    return id;
  }
};


class qualifiedName : public name {
  name *qualifier;
  symbol id;

  // Gets the record type associated with the qualifier. Reports an
  // error and returns null if the type is not a record.
  record *castToRecord(types::ty *t, bool tacit = false);

  // Translates as a virtual field, if possible.  qt is the type of the
  // qualifier.  Return true if there was a matching virtual field.
  bool varTransVirtual(action act, coenv &e,
                       types::ty *target, types::ty *qt);

  // Translates as an ordinary (non-virtual) field of a record, r.
  void varTransField(action act, coenv &e,
                     types::ty *target, record *r);
public:
  qualifiedName(position pos, name *qualifier, symbol id)
    : name(pos), qualifier(qualifier), id(id) {}

  trans::varEntry *getVarEntry(coenv &e);

  // As a variable:
  void varTrans(action act, coenv &, types::ty *target);
  types::ty *varGetType(coenv &);
  trans::varEntry *getCallee(coenv &e, types::signature *sig);

  // As a type:
  types::ty *typeTrans(coenv &e, bool tacit = false);
  trans::tyEntry *tyEntryTrans(coenv &e);
  trans::frame *tyFrameTrans(coenv &e);

  void prettyprint(ostream &out, Int indent);
  void print(ostream& out) const {
    out << *qualifier << "." << id;
  }
  symbol getName() {
    return id;
  }
};

} // namespace absyntax

#endif
