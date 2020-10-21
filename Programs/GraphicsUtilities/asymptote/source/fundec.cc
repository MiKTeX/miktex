/*****
 * fundec.h
 * Andy Hammerlindl 2002/8/29
 *
 * Defines the semantics for defining functions.  Both the newexp syntax, and
 * the abbreviated C-style function definition.
 *****/

#include "fundec.h"
#include "errormsg.h"
#include "coenv.h"
#include "stm.h"
#include "runtime.h"

namespace absyntax {

using namespace trans;
using namespace types;
using mem::list;

varinit *Default=new definit(nullPos);

void formal::prettyprint(ostream &out, Int indent)
{
  prettyname(out, keywordOnly ? "formal (keyword only)" : "formal", indent);

  base->prettyprint(out, indent+1);
  if (start) start->prettyprint(out, indent+1);
  if (defval) defval->prettyprint(out, indent+1);
}

types::formal formal::trans(coenv &e, bool encodeDefVal, bool tacit) {
  return types::formal(getType(e,tacit),
                       getName(),
                       encodeDefVal ? (bool) getDefaultValue() : 0,
                       getExplicit());
}

types::ty *formal::getType(coenv &e, bool tacit) {
  types::ty *bt = base->trans(e, tacit);
  types::ty *t = start ? start->getType(bt, e, tacit) : bt;
  if (t->kind == ty_void && !tacit) {
    em.error(getPos());
    em << "cannot declare parameters of type void";
    return primError();
  }

  return t;
}

void formal::addOps(coenv &e, record *r) {
  base->addOps(e, r);
  if (start)
    start->addOps(base->trans(e, true), e, r);
}

void formals::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "formals",indent);

  for (list<formal *>::iterator p = fields.begin(); p != fields.end(); ++p)
    (*p)->prettyprint(out, indent+1);
}

void formals::addToSignature(signature& sig,
                             coenv &e, bool encodeDefVal, bool tacit)
{
  for (list<formal *>::iterator p = fields.begin(); p != fields.end(); ++p) {
    formal& f=**p;
    types::formal tf = f.trans(e, encodeDefVal, tacit);

    if (f.isKeywordOnly())
      sig.addKeywordOnly(tf);
    else
      sig.add(tf);
  }

  if (rest) {
    if (!tacit && rest->getDefaultValue()) {
      em.error(rest->getPos());
      em << "rest parameters cannot have default values";
    }
    sig.addRest(rest->trans(e, encodeDefVal, tacit));
  }
}

// Returns the types of each parameter as a signature.
// encodeDefVal means that it will also encode information regarding
// the default values into the signature
signature *formals::getSignature(coenv &e, bool encodeDefVal, bool tacit)
{
  signature *sig = new signature;
  addToSignature(*sig,e,encodeDefVal,tacit);
  return sig;
}


// Returns the corresponding function type, assuming it has a return
// value of types::ty *result.
function *formals::getType(types::ty *result, coenv &e,
                           bool encodeDefVal,
                           bool tacit)
{
  function *ft = new function(result);
  addToSignature(ft->sig,e,encodeDefVal,tacit);
  return ft;
}

void formals::addOps(coenv &e, record *r)
{
  for(list<formal *>::iterator p = fields.begin(); p != fields.end(); ++p)
    (*p)->addOps(e, r);

  if (rest)
    rest->addOps(e, r);
}

// Another helper class. Does an assignment, but relying only on the
// destination for the type.
class basicAssignExp : public exp {
  varEntry *dest;
  varinit *value;
public:
  basicAssignExp(position pos, varEntry *dest, varinit *value)
    : exp(pos), dest(dest), value(value) {}

  void prettyprint(ostream &out, Int indent) {
    prettyname(out, "basicAssignExp", indent);
  }

  types::ty *getType(coenv &) {
    return dest->getType();
  }

  types::ty *trans(coenv &e) {
    // This doesn't handle overloaded types for the destination.
    value->transToType(e, getType(e));
    dest->encode(WRITE, pos, e.c);
    return getType(e);
  }
};

void transDefault(coenv &e, position pos, varEntry *v, varinit *init) {
  // This roughly translates into the statement
  //   if (isDefault(x))
  //     x=init;
  // where x is the variable in v and isDefault is a function that tests
  // whether x is the default argument token.

  v->encode(READ, pos, e.c);

  label end = e.c.fwdLabel();
  e.c.useLabel(inst::jump_if_not_default, end);

  init->transToType(e, v->getType());
  v->encode(WRITE, pos, e.c);
  e.c.encodePop();

  e.c.defLabel(end);
}

void formal::transAsVar(coenv &e, Int index) {
  symbol name = getName();
  if (name) {
    trans::access *a = e.c.accessFormal(index);
    assert(a);

    // Suppress error messages because they will already be reported
    // when the formals are translated to yield the type earlier.
    types::ty *t = getType(e, true);
    varEntry *v = new varEntry(t, a, 0, getPos());

    // Translate the default argument before adding the formal to the
    // environment, consistent with the initializers of variables.
    if (defval)
      transDefault(e, getPos(), v, defval);

    e.e.addVar(name, v);
  }
}

void formals::trans(coenv &e)
{
  Int index = 0;

  for (list<formal *>::iterator p=fields.begin(); p!=fields.end(); ++p) {
    (*p)->transAsVar(e, index);
    ++index;
  }

  if (rest) {
    rest->transAsVar(e, index);
    ++index;
  }
}

void fundef::prettyprint(ostream &out, Int indent)
{
  result->prettyprint(out, indent+1);
  params->prettyprint(out, indent+1);
  body->prettyprint(out, indent+1);
}

function *fundef::transType(coenv &e, bool tacit) {
  bool encodeDefVal=true;
  return params->getType(result->trans(e, tacit), e, encodeDefVal, tacit);
}

function *fundef::transTypeAndAddOps(coenv &e, record *r, bool tacit) {
  result->addOps(e,r);
  params->addOps(e,r);

  function *ft=transType(e, tacit);
  e.e.addFunctionOps(ft);
  if (r)
    r->e.addFunctionOps(ft);

  return ft;
}

varinit *fundef::makeVarInit(function *ft) {
  struct initializer : public varinit {
    fundef *f;
    function *ft;

    initializer(fundef *f, function *ft)
      : varinit(f->getPos()), f(f), ft(ft) {}

    void prettyprint(ostream &out, Int indent) {
      prettyname(out, "initializer", indent);
    }

    void transToType(coenv &e, types::ty *target) {
      assert(ft==target);
      f->baseTrans(e, ft);
    }
  };

  return new initializer(this, ft);
}

void fundef::baseTrans(coenv &e, types::function *ft)
{
  string name = id ? string(id) : string("<anonymous function>");

  // Create a new function environment.
  coder fc = e.c.newFunction(getPos(), name, ft);
  coenv fe(fc,e.e);

  // Translate the function.
  fe.e.beginScope();
  params->trans(fe);

  body->trans(fe);

  types::ty *rt = ft->result;
  if (rt->kind != ty_void &&
      rt->kind != ty_error &&
      !body->returns()) {
    em.error(body->getPos());
    em << "function must return a value";
  }

  fe.e.endScope();

  // Put an instance of the new function on the stack.
  vm::lambda *l = fe.c.close();
  e.c.encode(inst::pushclosure);
  e.c.encode(inst::makefunc, l);
}

types::ty *fundef::trans(coenv &e) {
  // I don't see how addFunctionOps can be useful here.
  // For instance, in
  //
  //   new void() {} == null
  //
  // callExp has to search for == before translating either argument, and the
  // operator cannot be added before translation.  (getType() is not allowed to
  // manipulate the environment.)
  // A new function expression is assigned to a variable, given as a return
  // value, or used as an argument to a function.  In any of these
  //
  // We must still addOps though, for the return type and formals.  ex:
  //
  //   new guide[] (guide f(int)) {
  //     return sequence(f, 10);
  //   };
  function *ft=transTypeAndAddOps(e, (record *)0, false);
  assert(ft);

  baseTrans(e, ft);

  return ft;
}

void fundec::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "fundec '" << id << "'\n";

  fun.prettyprint(out, indent);
}

void fundec::trans(coenv &e)
{
  transAsField(e,0);
}

void fundec::transAsField(coenv &e, record *r)
{
  function *ft = fun.transTypeAndAddOps(e, r, false);
  assert(ft);

  createVar(getPos(), e, r, id, ft, fun.makeVarInit(ft));
}

} // namespace absyntax
