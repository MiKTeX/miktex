/*****
 * env.cc
 * Andy Hammerlindl 2002/6/20
 *
 * Keeps track of the namespaces of variables and types when traversing
 * the abstract syntax.
 *****/

#include "env.h"
#include "record.h"
#include "genv.h"
#include "builtin.h"

using namespace types;

namespace trans {

// Instances of this class are passed to types::ty objects so that they can call
// back to env when checking casting of subtypes.
class envCaster : public caster {
  protoenv &e;
  symbol name;
public:
  envCaster(protoenv &e, symbol name)
    : e(e), name(name) {}

  access *operator() (ty *target, ty *source) {
    return e.lookupCast(target, source, name);
  }

  bool castable(ty *target, ty *source) {
    return e.castable(target, source, name);
  }
};
  
access *protoenv::baseLookupCast(ty *target, ty *source, symbol name) {
  static identAccess id;

  assert(target->kind != ty_overloaded &&
         source->kind != ty_overloaded);

  // If errors already exist, don't report more.  This may, however, cause
  // problems with resoving the signature of an overloaded function.  The
  // abstract syntax should check if any of the parameters had an error before
  // finding the signature.
  if (target->kind == ty_error || source->kind == ty_error)
    return &id;
  else if (equivalent(target,source))
    return &id;
  else {
    varEntry *v=lookupVarByType(name,new function(target,source));
    return v ? v->getLocation() : 0;
  }
}

access *protoenv::lookupCast(ty *target, ty *source, symbol name) {
  access *a=baseLookupCast(target, source, name);
  if (a)
    return a;

  envCaster ec(*this, name);
  return source->castTo(target, ec);
}

bool protoenv::castable(ty *target, ty *source, symbol name) {
  struct castTester : public tester {
    protoenv &e;
    symbol name;

    castTester(protoenv &e, symbol name)
      : e(e), name(name) {}

    bool base(ty *t, ty *s) {
      access *a=e.baseLookupCast(t, s, name);
      if (a)
        return true;

      envCaster ec(e, name);
      return s->castable(t, ec);
    }
  };

  castTester ct(*this, name);
  return ct.test(target,source);
}

bool protoenv::fastCastable(ty *target, ty *source) {
  assert(target->kind != types::ty_overloaded);
  assert(target->kind != types::ty_error);
  assert(source->kind != types::ty_error);

  // To avoid memory allocation, fill one static variable with new parameters
  // in each call.
  // Warning: This is not re-entrant if asy ever goes multi-threaded.
  static types::function castFunc(primVoid(), primVoid());
  castFunc.result = target;

  if (source->kind == types::ty_overloaded) {
    bool result = false;
    types::ty_vector& v = ((overloaded *)source)->sub;
    for (size_t i = 0; i < v.size(); ++i) {
      castFunc.sig.formals[0].t = v[i];
      if (lookupVarByType(symbol::castsym, &castFunc)) {
        result = true;
        break;
      }
    }
    //assert(result == castable(target, source, symbol::castsym));
    //cout << "fc OVERLOADED " << (result ? "CAST" : "FAIL") << endl;
    return result;
  }
  //else cout << "fc SIMPLE" << endl;

  // Don't test for equivalent, as that is already done by the castScore
  // code.  Assert disabled for speed.
#if 0
  assert(!equivalent(target, source));
#endif

  castFunc.sig.formals[0].t = source;

  if (lookupVarByType(symbol::castsym, &castFunc))
    return true;

  // Test for generic casts of null.  This should be moved to a types.h
  // routine.
  return source->kind == ty_null && target->isReference();
}

access *protoenv::fastLookupCast(ty *target, ty *source) {
  assert(target->kind != types::ty_overloaded);
  assert(target->kind != types::ty_error);
  assert(source->kind != types::ty_overloaded);
  assert(source->kind != types::ty_error);

  // Warning: This is not re-entrant.
  static types::function castFunc(primVoid(), primVoid());
  castFunc.result = target;
  castFunc.sig.formals[0].t = source;

  varEntry *ve = lookupVarByType(symbol::castsym, &castFunc);
  if (ve)
    return ve->getLocation();

  // Fall back on slow routine.
  return lookupCast(target, source, symbol::castsym);
}


ty *protoenv::castTarget(ty *target, ty *source, symbol name) {
  struct resolver : public collector {
    protoenv &e;
    symbol name;

    resolver(protoenv &e, symbol name)
      : e(e), name(name) {}

    types::ty *base(types::ty *target, types::ty *source) {
      return e.castable(target, source, name) ? target : 0;
    }
  };
          
  resolver r(*this, name);
  return r.collect(target, source);
} 

ty *protoenv::castSource(ty *target, ty *source, symbol name) {
  struct resolver : public collector {
    protoenv &e;
    symbol name;

    resolver(protoenv &e, symbol name)
      : e(e), name(name) {}

    types::ty *base(types::ty *target, types::ty *source) {
      return e.castable(target, source, name) ? source : 0;
    }
  };
          
  resolver r(*this, name);
  return r.collect(target, source);
} 

void protoenv::addArrayOps(array *a)
{
  trans::addArrayOps(ve, a);
}

void protoenv::addRecordOps(record *r)
{
  trans::addRecordOps(ve, r);
}

void protoenv::addFunctionOps(function *f)
{
  trans::addFunctionOps(ve, f);
}

env::env(genv &ge)
  : protoenv(venv::file_env_tag()), ge(ge)
{
  // NOTE: May want to make this initial environment into a "builtin" module,
  // and then import the builtin module.
  base_tenv(te);
  base_venv(ve);
}

env::~env()
{
}

record *env::getModule(symbol id, string filename)
{
  return ge.getModule(id, filename);
}

}
