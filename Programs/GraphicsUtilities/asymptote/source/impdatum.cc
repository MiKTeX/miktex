#include <cstring>

#include "stack.h"
#include "env.h"
#include "exp.h"
#include "stm.h"
#include "refaccess.h"

using std::strlen;

using namespace absyntax;
using namespace trans;

using vm::item;
using vm::get;


#include "policy.h"

coenv &coenvInOngoingProcess();
void runInOngoingProcess(absyntax::runnable *r);

void runExp(absyntax::exp *e)
{
  absyntax::expStm s(nullPos, e);
  runInOngoingProcess(&s);
}

class ImpDatum;
class ImpArguments;

ImpDatum *datumError(const char *msg);

// Expression used for non-item datums.
class errorExp : public absyntax::exp {
public:
  errorExp() : exp(nullPos) {}

  void prettyprint(ostream &out, Int indent)
  {
    absyntax::prettyname(out, "errorExp", indent, getPos());
  }

  void complain() {
    em.error(nullPos); em << "cannot use datum as expression";
  }

  types::ty *getType(coenv &) { return types::primError(); }
  types::ty *trans(coenv &e) { complain(); return getType(e); }

  void transAsType(coenv &e, types::ty *target) { complain(); }
};

// Abstract base class for Datum types.
class ImpDatum {
public:
  virtual operator handle_typ() { return (handle_typ)(this); }

  virtual int_typ toInt() {
    datumError("cannot convert to integer");

    // Return a weird value that will hopefully be noticed.
    return -777777;
  }

  virtual bool toBool() {
    datumError("cannot convert to bool");
    return false;
  }

  virtual double toDouble() {
    datumError("cannot convert to double");
    return -777e77;
  }

  virtual string_typ toString() {
    datumError("cannot convert to string");

    string_typ s = { "XXXXX", 5 };
    return s;
  }

  virtual absyntax::exp *getExp() {
    datumError("invalid use of datum");
    return new errorExp;
  }

  // How to access a field of the datum.
  virtual absyntax::exp *getFieldExp(symbol id) {
    assert(id);
    return new fieldExp(nullPos, this->getExp(), id);
  }

  virtual ImpDatum *getField(const char *name);

  virtual ImpDatum *getCell(ImpDatum *index) {
    return datumError("cannot index datatype");
  }

  virtual void addField(const char *name, ImpDatum *init)
  {
    datumError("cannot set field of datatype");
  }

};

// An ever-growing list of handles, used to avoid garbage collecting the data.
// TODO: Implement effective releaseHandle.
mem::vector<handle_typ> handles;

handle_typ wrap(ImpDatum *d)
{
  handle_typ h = (handle_typ)(d);
  handles.push_back(h);
  return h;
}

ImpDatum *unwrap(handle_typ handle)
{
  assert(handle != 0);
  return (ImpDatum *)(handle);
}

class ErrorDatum : public ImpDatum {
};

error_callback_typ errorCallback = 0;

ImpDatum *datumError(const char *msg)
{
  static ErrorDatum ed;

  if (errorCallback) {
    string_typ s = { msg, strlen(msg) };
    errorCallback(s);
  }
  else {
    cerr << msg << '\n';
  }

  return &ed;
}

handle_typ imp_copyHandle(handle_typ handle)
{
  //cout << "+";
  // For now, don't do anything.
  return handle;
}

void imp_releaseHandle()
{
  //cout << "-";
  // Do nothing, for now.
}

// A datum representing a value in Asymptote.  Both the runtime representation
// of the value and its type are stored.
class ItemDatum : public ImpDatum {
  item i;
  types::ty *t;

public:
  // Every itemDatum has a fixed (non-overloaded) type, t
  ItemDatum(types::ty *t) : t(t) {
    assert(t);
    assert(t->isNotOverloaded());
    assert(t->isNotError());
  }

  // An expression that can be used to get and set the datum.
  // The value should only be set once, when the datum is created, and not
  // changed.
  absyntax::exp *getExp() {
    // It may be faster to create this once on start, but then the datum will
    // require more space.  For now, we create the access and expression on
    // demand.
    return new varEntryExp(nullPos, t, new itemRefAccess(&i));
  }

  int_typ toInt() {
    // TODO: Decide if we want to use casting.
    if (t->kind == types::ty_Int)
      return static_cast<int_typ>(get<Int>(i));
    else
      return ImpDatum::toInt();
  }

  bool toBool() {
    if (t->kind == types::ty_boolean)
      return get<bool>(i);
    else
      return ImpDatum::toBool();
  }

  double toDouble() {
    if (t->kind == types::ty_real)
      return get<double>(i);
    else
      return ImpDatum::toDouble();
  }

  string_typ toString() {
    if (t->kind == types::ty_string) {
      // TODO: Fix for strings containing NUL.
      string *s = get<string *>(i);
      string_typ st = { s->c_str(), s->length() };
      return st;
    }
    else
      return ImpDatum::toString();
  }
};

ItemDatum *ItemDatumFromExp(types::ty *t, absyntax::exp *e)
{
  ItemDatum *d = new ItemDatum(t);
  assignExp ae(nullPos, d->getExp(), e);
  runExp(&ae);

  return d;
}

ItemDatum *ItemDatumFromInt(int_typ x)
{
  intExp ie(nullPos, static_cast<Int>(x));
  return ItemDatumFromExp(types::primInt(), &ie);
}

ItemDatum *ItemDatumFromBool(bool x)
{
  booleanExp be(nullPos, x);
  return ItemDatumFromExp(types::primBoolean(), &be);
}

ItemDatum *ItemDatumFromDouble(double x)
{
  realExp re(nullPos, x);
  return ItemDatumFromExp(types::primReal(), &re);
}

ItemDatum *ItemDatumFromString(string_typ x)
{
  mem::string s(x.buf, (size_t)x.length);
  stringExp se(nullPos, s);
  return ItemDatumFromExp(types::primString(), &se);
}


// If the interface is asked to return a field which is overloaded, a handle
// to and OverloadedDatum is returned.  No evaluation actually occurs.  The
// datum simply consists of the containing datum and the name of the field
// requested.  Subsequent use of the datum will resolve the overloading (or
// report an error).
class OverloadedDatum : public ImpDatum {
  ImpDatum *parent;
  symbol id;

public:
  OverloadedDatum(ImpDatum *parent, symbol id) : parent(parent), id(id)
  {
    assert(parent);
    assert(id);
  }

  absyntax::exp *getExp() {
    return parent->getFieldExp(id);
    return new fieldExp(nullPos, parent->getExp(), id);
  }
};

ImpDatum *ImpDatum::getField(const char *name)
{
  coenv &e = coenvInOngoingProcess();
  symbol id = symbol::trans(name);

  absyntax::exp *ex = getFieldExp(id);

  types::ty *t = ex->getType(e);
  if (t->isError())
    return datumError("no field of that name");

  if (t->isOverloaded())
    return new OverloadedDatum(this, id);

  // Create a new datum and assign the variable to it.
  ItemDatum *d = new ItemDatum(t);
  assignExp ae(nullPos, d->getExp(), ex);
  runExp(&ae);

  return d;
}

handle_typ imp_handleFromInt(int_typ x)
{
  return wrap(ItemDatumFromInt(x));
}

handle_typ imp_handleFromBool(int_typ x)
{
  if (x != 0 && x != 1)
    return wrap(datumError("invalid boolean value"));

  return wrap(ItemDatumFromBool(x == 1));
}

handle_typ imp_handleFromDouble(double x)
{
  return wrap(ItemDatumFromDouble(x));
}

int_typ imp_IntFromHandle(handle_typ handle)
{
  return unwrap(handle)->toInt();
}

int_typ imp_boolFromHandle(handle_typ handle)
{
  return unwrap(handle)->toBool() ? 1 : 0;
}

double imp_doubleFromHandle(handle_typ handle)
{
  return unwrap(handle)->toDouble();
}


handle_typ imp_handleFromString(string_typ x)
{
  return wrap(ItemDatumFromString(x));
}

string_typ imp_stringFromHandle(handle_typ handle)
{
  return unwrap(handle)->toString();
}

handle_typ imp_getField(handle_typ handle, const char *name)
{
  return wrap(unwrap(handle)->getField(name));
}

handle_typ imp_getCell(handle_typ handle, handle_typ index)
{
  return wrap(unwrap(handle)->getCell(unwrap(index)));
}

void imp_addField(handle_typ handle, const char *name, handle_typ init)
{
  unwrap(handle)->addField(name, unwrap(init));
}

class ImpArguments /* TODO: gc visible but not collected */ {
  arglist args;
public:

  ImpArguments() {}

  void add(const char *name, ImpDatum *arg, arg_rest_option isRest)
  {
    assert(isRest == NORMAL_ARG); // TODO: Implement rest.
    symbol id = (name && name[0]) ? symbol::trans(name) : symbol::nullsym;

    args.add(arg->getExp(), id);
  }

  arglist *getArgs() { return &args; }
};

arguments_typ wrapArgs(ImpArguments *args)
{
  return (arguments_typ)(args);
}
ImpArguments *unwrapArgs(arguments_typ args)
{
  return (ImpArguments *)(args);
}

arguments_typ imp_newArguments()
{
  return wrapArgs(new ImpArguments);
}

void imp_releaseArguments(arguments_typ args)
{
  // For now, do nothing.
}

void imp_addArgument(arguments_typ args, const char *name, handle_typ handle,
                     arg_rest_option isRest)
{
  unwrapArgs(args)->add(name, unwrap(handle), isRest);
}

ImpDatum *callDatum(ImpDatum *callee, ImpArguments *args)
{
  coenv &e = coenvInOngoingProcess();

  callExp callex(nullPos, callee->getExp(), args->getArgs());

  types::ty *t = callex.getType(e);
  if (t->isError()) {
    // Run for errors.
    runExp(&callex); em.sync();
    return datumError("invalid call");
  }

  assert(t->isNotOverloaded()); // Calls are never overloaded.

  if (t->kind == types::ty_void) {
    // Execute the call and return 0 to indicate void.
    runExp(&callex);
    return 0;
  }
  else
    return ItemDatumFromExp(t, &callex);
}

handle_typ imp_call(handle_typ callee, arguments_typ args)
{
  return wrap(callDatum(unwrap(callee), unwrapArgs(args)));
}

class GlobalsDatum : public ImpDatum {
  typedef std::map<const char*, ImpDatum *> gmap;
  gmap base;

  virtual absyntax::exp *getFieldExp(symbol id)
  {
    // Fields of the globals datum are global variables.  Use the unqualified
    // name.
    return new nameExp(nullPos, id);
  }

  virtual void addField(const char *name, ImpDatum *init) {
    datumError("addField not yet re-implemented");
  }
};

class ImpState {
  //ImpArguments *params;

  ImpDatum *retval;
public:
  ImpState() : retval(0) {}

  ImpDatum *globals() {
    return new GlobalsDatum();
  }

  int_typ numParams() {
    /*if (params)
      return params->val.size();
      else */ {
      datumError("parameters accessed outside of function");
      return 0;
    }
  }

  ImpDatum *getParam(int_typ index) {
    /*if (params) {
      if (index >= 0 && index < static_cast<int_typ>(params->val.size()))
      return params->val[index];
      else
      return datumError("invalid index for parameter");
      }
      else */ {
      return datumError("parameters accessed outside of function");
    }
  }

  void setReturnValue(ImpDatum *retval)
  {
    /*if (params) {
      if (this->retval)
      datumError("return value set more than once");
      else
      this->retval = retval;
      }
      else */ {
      datumError("return value set outside of function");
    }
  }

  ImpDatum *getReturnValue()
  {
    return retval;
  }
};

state_typ wrapState(ImpState *s)
{
  return (state_typ)(s);
}
ImpState *unwrapState(state_typ s)
{
  return (ImpState *)(s);
}


handle_typ imp_globals(state_typ state)
{
  return wrap(unwrapState(state)->globals());
}

int_typ imp_numParams(state_typ state)
{
  return unwrapState(state)->numParams();
}

handle_typ imp_getParam(state_typ state, int_typ index)
{
  return wrap(unwrapState(state)->getParam(index));
}

void imp_setReturnValue(state_typ state, handle_typ handle)
{
  unwrapState(state)->setReturnValue(unwrap(handle));
}

state_typ cheatState()
{
  return wrapState(new ImpState());
}

#if 0
class FunctionDatum : public ImpDatum {
  function_typ f;
  void *data;

public:
  FunctionDatum(function_typ f, void *data) : f(f), data(data) {}

  ImpDatum *call(ImpArguments *args) {
    ImpState state(args);

    // Call the function.
    f(wrapState(&state),data);

    if (state.getReturnValue())
      return state.getReturnValue();
    else
      // TODO: Decide on datum for void return.
      return 0;
  }
};
#endif

handle_typ imp_handleFromFunction(const char *signature,
                                  function_typ f, void *data)
{
  // TODO: Re-implement.
  return 0; //wrap(new FunctionDatum(f, data));
}

void imp_setErrorCallback(error_callback_typ callback)
{
  errorCallback = callback;
}

extern policy_typ imp_policy;
policy_typ imp_policy =
{
  /* version = */ 101,
  imp_copyHandle,
  imp_releaseHandle,
  imp_handleFromInt,
  imp_handleFromBool,
  imp_handleFromDouble,
  imp_handleFromString,
  imp_handleFromFunction,
  imp_IntFromHandle,
  imp_boolFromHandle,
  imp_doubleFromHandle,
  imp_stringFromHandle,
  imp_getField,
  imp_getCell,
  imp_addField,
  imp_newArguments,
  imp_releaseArguments,
  imp_addArgument,
  imp_call,
  imp_globals,
  imp_numParams,
  imp_getParam,
  imp_setReturnValue,
  imp_setErrorCallback,
};

// Defined in process.cc
void init(bool resetpath=true);

extern "C" {

  policy_typ *_asy_getPolicy()
  {
    return &imp_policy;
  }

  state_typ _asy_getState()
  {
    static state_typ state = cheatState();

    // TODO: Make sure this runs once.
    char buf[] = "asymptote.so";
    char *argv [] = { buf };
    settings::setOptions(1,argv);

    // Ensures uptodate is not used.
    init();

    return state;
  }

}
