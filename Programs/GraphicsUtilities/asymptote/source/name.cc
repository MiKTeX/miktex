/*****
 * name.cc
 * Andy Hammerlindl2002/07/14
 *
 * Qualified names (such as x, f, builtin.sin, a.b.c.d, etc.) can be used
 * either as variables or a type names.  This class stores qualified
 * names used in nameExp and nameTy in the abstract syntax, and
 * implements the exp and type functions.
 *****/

#include "name.h"
#include "frame.h"
#include "record.h"
#include "coenv.h"
#include "inst.h"

namespace absyntax {
using namespace types;
using trans::access;
using trans::qualifiedAccess;
using trans::action;
using trans::READ;
using trans::WRITE;
using trans::CALL;
using vm::inst;


types::ty *signatureless(types::ty *t) {
  if (overloaded *o=dynamic_cast<overloaded *>(t))
    return o->signatureless();
  else
    return (t && !t->getSignature()) ? t : 0;
}


void name::forceEquivalency(action act, coenv &e,
                            types::ty *target, types::ty *source)
{
  if (act == READ)
    e.implicitCast(getPos(), target, source);
  else if (!equivalent(target, source)) {
    em.compiler(getPos());
    em << "type mismatch in variable: "
       << *target
       << " vs " << *source;
  }
}

frame *name::frameTrans(coenv &e)
{
  if (types::ty *t=signatureless(varGetType(e))) {
    if (t->kind == types::ty_record) {
      varTrans(READ, e, t);
      return ((record *)t)->getLevel();
    }
    else
      return 0;
  }
  else
    return tyFrameTrans(e);
}
    

types::ty *name::getType(coenv &e, bool tacit)
{
  types::ty *t=signatureless(varGetType(e));
  if (!tacit && t && t->kind == ty_error)
    // Report errors associated with regarding the name as a variable.
    varTrans(trans::READ, e, t);
  return t ? t : typeTrans(e, tacit);
}


varEntry *simpleName::getVarEntry(coenv &e)
{
  types::ty *t=signatureless(varGetType(e));
  return t ? e.e.lookupVarByType(id, t) : 0;
}
  
void simpleName::varTrans(action act, coenv &e, types::ty *target)
{
  varEntry *v = e.e.lookupVarByType(id, target);
  
  if (v) {
    v->encode(act, getPos(), e.c);
    forceEquivalency(act, e, target, v->getType());
  }
  else {
    em.error(getPos());
    em << "no matching variable of name \'" << id << "\'";
  }
}

types::ty *simpleName::varGetType(coenv &e)
{
  return e.e.varGetType(id);
}

trans::varEntry *simpleName::getCallee(coenv &e, signature *sig)
{
  varEntry *ve = e.e.lookupVarBySignature(id, sig);
  return ve;
}

types::ty *simpleName::typeTrans(coenv &e, bool tacit)
{
  types::ty *t = e.e.lookupType(id);
  if (t) {
    return t;
  }
  else {
    if (!tacit) {
      em.error(getPos());
      em << "no type of name \'" << id << "\'";
    }
    return primError();
  }
}

tyEntry *simpleName::tyEntryTrans(coenv &e)
{
  tyEntry *ent = e.e.lookupTyEntry(id);
  if (!ent) {
    em.error(getPos());
    em << "no type of name \'" << id << "\'";
    return new tyEntry(primError(), 0, 0, position());
  }
  return ent;
}

frame *simpleName::tyFrameTrans(coenv &e)
{
  tyEntry *ent = e.e.lookupTyEntry(id);
  if (ent && ent->t->kind==types::ty_record && ent->v) {
    ent->v->encode(READ, getPos(), e.c);
    return ent->v->getLevel();
  }
  else 
    return 0;
}

void simpleName::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "simpleName '" << id << "'\n";
}


record *qualifiedName::castToRecord(types::ty *t, bool tacit)
{
  switch (t->kind) {
    case ty_overloaded:
      if (!tacit) {
        em.compiler(qualifier->getPos());
        em << "name::getType returned overloaded";
      }
      return 0;
    case ty_record:
      return (record *)t;
    case ty_error:
      return 0;
    default:
      if (!tacit) {
        em.error(qualifier->getPos());
        em << "type \'" << *t << "\' is not a structure";
      }
      return 0;
  }
}

bool qualifiedName::varTransVirtual(action act, coenv &e,
                                    types::ty *target, types::ty *qt)
{
  varEntry *v = qt->virtualField(id, target->getSignature());
  if (v) {
    // Push qualifier onto stack.
    qualifier->varTrans(READ, e, qt);

    v->encode(act, getPos(), e.c);

    // A virtual field was used.
    return true;
  }

  // No virtual field.
  return false;
}

void qualifiedName::varTransField(action act, coenv &e,
                                  types::ty *target, record *r)
{
  varEntry *v = r->e.lookupVarByType(id, target);

  if (v) {
    frame *f = qualifier->frameTrans(e);
    if (f)
      v->encode(act, getPos(), e.c, f);
    else
      v->encode(act, getPos(), e.c);

    forceEquivalency(act, e, target, v->getType());
  }
  else {
    em.error(getPos());
    em << "no matching field of name \'" << id << "\' in \'" << *r << "\'";
  }
}

void qualifiedName::varTrans(action act, coenv &e, types::ty *target)
{
  types::ty *qt = qualifier->getType(e);

  // Use virtual fields if applicable.
  if (varTransVirtual(act, e, target, qt))
    return;

  record *r = castToRecord(qt);
  if (r)
    varTransField(act, e, target, r);
}

types::ty *qualifiedName::varGetType(coenv &e)
{
  types::ty *qt = qualifier->getType(e, true);

  // Look for virtual fields.
  types::ty *t = qt->virtualFieldGetType(id);
  if (t)
    return t;

  record *r = castToRecord(qt, true);
  return r ? r->e.varGetType(id) : 0;
}

trans::varEntry *qualifiedName::getCallee(coenv &e, signature *sig)
{
  // getTypeAsCallee is an optimization attempt.  We don't try optimizing the
  // rarer qualifiedName call case.
  // TODO: See if this is worth implementing.
  //cout << "FAIL BY QUALIFIED NAME" << endl;
  return 0;
}

trans::varEntry *qualifiedName::getVarEntry(coenv &e)
{
  varEntry *qv = qualifier->getVarEntry(e);

  types::ty *qt = qualifier->getType(e, true);
  record *r = castToRecord(qt, true);
  if (r) {
    types::ty *t = signatureless(r->e.varGetType(id));
    varEntry *v = t ? r->e.lookupVarByType(id, t) : 0;
    return trans::qualifyVarEntry(qv,v);
  }
  else
    return qv;
}

types::ty *qualifiedName::typeTrans(coenv &e, bool tacit)
{
  types::ty *rt = qualifier->getType(e, tacit);

  record *r = castToRecord(rt, tacit);
  if (!r)
    return primError();

  tyEntry *ent = r->e.lookupTyEntry(id);
  if (ent) {
    if (!tacit)
      ent->reportPerm(READ, getPos(), e.c);
    return ent->t;
  }
  else {
    if (!tacit) {
      em.error(getPos());
      em << "no matching field or type of name \'" << id << "\' in \'"
         << *r << "\'";
    }
    return primError();
  }
}

tyEntry *qualifiedName::tyEntryTrans(coenv &e)
{
  types::ty *rt = qualifier->getType(e, false);

  record *r = castToRecord(rt, false);
  if (!r)
    return new tyEntry(primError(), 0, 0, position());

  tyEntry *ent = r->e.lookupTyEntry(id);
  if (!ent) {
    em.error(getPos());
    em << "no matching type of name \'" << id << "\' in \'"
       << *r << "\'";
    return new tyEntry(primError(), 0, 0, position());
  }
  ent->reportPerm(READ, getPos(), e.c);

  return trans::qualifyTyEntry(qualifier->getVarEntry(e), ent);
}

frame *qualifiedName::tyFrameTrans(coenv &e)
{
  frame *f=qualifier->frameTrans(e);
  tyEntry *ent = e.e.lookupTyEntry(id);
  if (ent && ent->t->kind==types::ty_record && ent->v) {
    if (f)
      ent->v->encode(READ, getPos(), e.c, f);
    else
      ent->v->encode(READ, getPos(), e.c);
    return ent->v->getLevel();
  }
  else 
    return f;
}

void qualifiedName::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "qualifiedName '" << id << "'\n";

  qualifier->prettyprint(out, indent+1);
}

} // namespace absyntax
