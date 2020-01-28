/*****
 * exp.cc
 * andy hammerlindl 2002/8/19
 *
 * represents the abstract syntax tree for the expressions in the
 * language.  this is translated into virtual machine code using trans()
 * and with the aid of the environment class.
 *****/

#include "exp.h"
#include "errormsg.h"
#include "runtime.h"
#include "runmath.h"
#include "runpicture.h"
#include "runarray.h"
#include "runpair.h"
#include "runtriple.h"
#include "runpath.h"
#include "coenv.h"
#include "application.h"
#include "dec.h"
#include "stm.h"
#include "inst.h"
#include "opsymbols.h"
#include "process.h"

//void runCode(absyntax::block *code);
  
namespace absyntax {

using namespace types;
using namespace trans;
using vm::inst;
using mem::vector;


#if 0
void exp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "exp",indent);
}
#endif

void exp::transAsType(coenv &e, types::ty *target) {
  trans(e);
//  types::ty *t=trans(e);
//  assert(t->kind==ty_error || equivalent(t,target));
}

void exp::transToType(coenv &e, types::ty *target)
{
  types::ty *ct=cgetType(e);

  if (equivalent(target, ct)) {
    transAsType(e, target);
    return;
  }

  // See if the cast can be handled by the fastLookupCast method, which does
  // less memory allocation.
  if (ct->kind != ty_overloaded &&
      ct->kind != ty_error &&
      target->kind != ty_error) {
    access *a = e.e.fastLookupCast(target, ct);
    if (a) {
      transAsType(e, ct);
      a->encode(trans::CALL, getPos(), e.c);
      return;
    }
  }

  types::ty *source = e.e.castSource(target, ct, symbol::castsym);
  if (source==0) {
    if (target->kind != ty_error) {
      types::ty *sources=cgetType(e);
      em.error(getPos());

      em << "cannot cast ";
      if (sources->kind==ty_overloaded)
        em << "expression";
      else
        em << "'" << *sources << "'";
      em << " to '" << *target << "'";
    }
  }
  else if (source->kind==ty_overloaded) {
    if (target->kind != ty_error) {
      em.error(getPos());
      em << "expression is ambiguous in cast to '" << *target << "'";
    }
  }
  else {
    transAsType(e, source);
    e.implicitCast(getPos(), target, source);
  }
}

void exp::testCachedType(coenv &e) {
  if (ct != 0) {
    types::ty *t = getType(e);
    if (!equivalent(t, ct)) {
      em.compiler(getPos());
      em << "cached type '" << *ct 
         << "' doesn't match actual type '" << *t << "'";
      em.sync();
    }
  }
}

void exp::transCall(coenv &e, types::ty *target)
{
  transAsType(e, target);
  e.c.encode(inst::popcall);
}

void exp::transConditionalJump(coenv &e, bool cond, label dest) {
  transToType(e, primBoolean());
  e.c.useLabel(cond ? inst::cjmp : inst::njmp, dest);
}

exp *exp::evaluate(coenv &e, types::ty *target) {
  return new tempExp(e, this, target);
}


tempExp::tempExp(coenv &e, varinit *v, types::ty *t)
  : exp(v->getPos()), a(e.c.allocLocal()), t(t)
{
  v->transToType(e, t);
  a->encode(WRITE, getPos(), e.c);
  e.c.encodePop();
}

void tempExp::prettyprint(ostream &out, Int indent) {
  prettyname(out, "tempExp", indent);
}

types::ty *tempExp::trans(coenv &e) {
  a->encode(READ, getPos(), e.c);
  return t;
}


varEntryExp::varEntryExp(position pos, types::ty *t, access *a)
  : exp(pos), v(new trans::varEntry(t, a, 0, position())) {}
varEntryExp::varEntryExp(position pos, types::ty *t, vm::bltin f)
  : exp(pos), v(new trans::varEntry(t, new bltinAccess(f), 0, position())) {}

void varEntryExp::prettyprint(ostream &out, Int indent) {
  prettyname(out, "varEntryExp", indent);
}

types::ty *varEntryExp::getType(coenv &) {
  return v->getType();
}

types::ty *varEntryExp::trans(coenv &e) {
  v->encode(READ, getPos(), e.c);
  return getType(e);
}

trans::varEntry *varEntryExp::getCallee(coenv &e, types::signature *sig) {
  return equivalent(sig, v->getType()->getSignature()) ? v : 0;
}

void varEntryExp::transAct(action act, coenv &e, types::ty *target) {
  assert(equivalent(getType(e),target));
  v->encode(act, getPos(), e.c);
}
void varEntryExp::transAsType(coenv &e, types::ty *target) {
  transAct(READ, e, target);
}
void varEntryExp::transWrite(coenv &e, types::ty *target, exp *value) {
  value->transToType(e, target);
  transAct(WRITE, e, target);
}
void varEntryExp::transCall(coenv &e, types::ty *target) {
  transAct(trans::CALL, e, target);
}


void nameExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "nameExp",indent);

  value->prettyprint(out, indent+1);
}


void fieldExp::pseudoName::prettyprint(ostream &out, Int indent)
{
  // This should never be called.
  prettyindent(out, indent);
  out << "pseudoName" << "\n";

  object->prettyprint(out, indent+1);
}

void fieldExp::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "fieldExp '" << field << "'\n";

  object->prettyprint(out, indent+1);
}

types::ty *fieldExp::getObject(coenv& e)
{
  types::ty *t = object->cgetType(e);
  if (t->kind == ty_overloaded) {
    t=((overloaded *)t)->signatureless();
    if(!t) return primError();
  }
  return t;
}


array *arrayExp::getArrayType(coenv &e)
{
  types::ty *a = set->cgetType(e);
  if (a->kind == ty_overloaded) {
    a = ((overloaded *)a)->signatureless();
    if (!a)
      return 0;
  }

  switch (a->kind) {
    case ty_array:
      return (array *)a;
    case ty_error:
      return 0;
    default:
      return 0;
  }
}

array *arrayExp::transArray(coenv &e)
{
  types::ty *a = set->cgetType(e);
  if (a->kind == ty_overloaded) {
    a = ((overloaded *)a)->signatureless();
    if (!a) {
      em.error(set->getPos());
      em << "expression is not an array";
      return 0;
    }
  }

  set->transAsType(e, a);

  switch (a->kind) {
    case ty_array:
      return (array *)a;
    case ty_error:
      return 0;
    default:
      em.error(set->getPos());
      em << "expression is not an array";
      return 0;
  }
}

// Checks if the expression can be translated as an array.
bool isAnArray(coenv &e, exp *x)
{
  types::ty *t=x->cgetType(e);
  if (t->kind == ty_overloaded)
    t=dynamic_cast<overloaded *>(t)->signatureless();
  return t && t->kind==ty_array;
}


void subscriptExp::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "subscriptExp\n";

  set->prettyprint(out, indent+1);
  index->prettyprint(out, indent+1);
}

types::ty *subscriptExp::trans(coenv &e)
{
  array *a = transArray(e);
  if (!a)
    return primError();

  if (isAnArray(e, index)) {
    index->transToType(e, types::IntArray());
    e.c.encode(inst::builtin, run::arrayIntArray);
    return getArrayType(e);
  }
  else {
    index->transToType(e, types::primInt());
    e.c.encode(inst::builtin,
               a->celltype->kind==ty_array ? run::arrayArrayRead :
               run::arrayRead);
    return a->celltype;
  }
}

types::ty *subscriptExp::getType(coenv &e)
{
  array *a = getArrayType(e);
  return a ? (isAnArray(e, index) ? a : a->celltype) :
    primError();
}
     
void subscriptExp::transWrite(coenv &e, types::ty *t, exp *value)
{
  // Put array, index, and value on the stack in that order, then call
  // arrayWrite.
  array *a = transArray(e);
  if (!a)
    return;

  if (!equivalent(a->celltype, t))
  {
    em.error(getPos());
    em << "array expression cannot be used as an address";

    // Translate the value for errors.
    value->transToType(e, t);
    return;
  }

  index->transToType(e, types::primInt());

  value->transToType(e, t);

  e.c.encode(inst::builtin, run::arrayWrite);
}


void slice::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "slice", indent);
  if (left)
    left->prettyprint(out, indent+1);
  else
    prettyname(out, "left omitted", indent+1);
  if (right)
    right->prettyprint(out, indent+1);
  else
    prettyname(out, "right omitted", indent+1);
}

void slice::trans(coenv &e)
{
  if (left)
    left->transToType(e, types::primInt());
  else
    // If the left index is omitted it can be assumed to be zero.
    e.c.encode(inst::intpush, (Int)0);

  if (right)
    right->transToType(e, types::primInt());
}


void sliceExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "sliceExp", indent);
  set->prettyprint(out, indent+1);
  index->prettyprint(out, indent+1);
}

types::ty *sliceExp::trans(coenv &e)
{
  array *a = transArray(e);
  if (!a)
    return primError();

  index->trans(e);

  e.c.encode(inst::builtin, index->getRight() ? run::arraySliceRead :
             run::arraySliceReadToEnd);

  return a;
}

types::ty *sliceExp::getType(coenv &e)
{
  array *a = getArrayType(e);
  return a ? a : primError();
}

void sliceExp::transWrite(coenv &e, types::ty *t, exp *value)
{
  array *a = transArray(e);
  if (!a)
    return;
  assert(equivalent(a, t));

  index->trans(e);

  value->transToType(e, t);

  e.c.encode(inst::builtin, index->getRight() ? run::arraySliceWrite :
             run::arraySliceWriteToEnd);
}

void thisExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "thisExp", indent);
}

types::ty *thisExp::trans(coenv &e)
{
  if (!e.c.encodeThis()) {
    em.error(getPos());
    em << "static use of 'this' expression";
  }
  return cgetType(e);
}

types::ty *thisExp::getType(coenv &e)
{
  return e.c.thisType();
}

void equalityExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "equalityExp", indent);
  callExp::prettyprint(out, indent+1);
}

types::ty *equalityExp::getType(coenv &e) {
  // Try to the resolve the expression as a function call first.
  types::ty *t = callExp::getType(e);
  assert(t);
  if (t->kind != ty_error)
    return t;
  else
    // Either an error or handled by the function equality methods.  In the
    // first case, we may return whatever we like, and the second case always
    // returns bool.  In either case, it is safe to return bool.
    return primBoolean();
}

// From a possibly overloaded type, if there is a unique function type, return
// it, otherwise 0.
types::ty *uniqueFunction(types::ty *t) {
  if (t->kind == types::ty_function)
    return t;

  if (t->isOverloaded()) {
    types::ty *ft = 0;
    for (ty_iterator i = t->begin(); i != t->end(); ++i)
    {
      if ((*i)->kind != types::ty_function) 
        continue;

      if (ft) {
        // Multiple function types.
        return 0;
      }

      ft = *i;
    }

    return ft;
  }

  // Not a function.
  return 0;
}

// From two possibly overloaded types, if there is a unique function type
// common to both, return it, otherwise 0.
types::ty *uniqueFunction(types::ty *t1, types::ty *t2) {
  if (t1->kind == types::ty_function)
    return equivalent(t1, t2) ? t1 : 0;

  if (t1->isOverloaded()) {
    types::ty *ft = 0;
    for (ty_iterator i = t1->begin(); i != t1->end(); ++i)
    {
      if ((*i)->kind != types::ty_function) 
        continue;

      if (!equivalent(*i, t2))
        continue;

      if (ft) {
        // Multiple function types.
        return 0;
      }

      ft = *i;
    }

    return ft;
  }

  // Not a function.
  return 0;
}

bltin bltinFromName(symbol name) {
  if (name == SYM_EQ)
    return run::boolFuncEq;
  assert(name == SYM_NEQ);
  return run::boolFuncNeq;
}

types::ty *equalityExp::trans(coenv &e) {
  // First, try to handle by normal function resolution.
  types::ty *t = callExp::getType(e);
  assert(t);
  if (t->kind != ty_error)
    return callExp::trans(e);

  // Then, check for the function equality case.
  exp *left = (*this->args)[0].val;
  exp *right = (*this->args)[1].val;

  types::ty *lt = left->getType(e);
  types::ty *rt = right->getType(e);

  // TODO: decide what null == null should do.

  // Check for function == null and null == function
  types::ty *ft = 0;
  if (rt->kind == types::ty_null)
    ft = uniqueFunction(lt);
  else if (lt->kind == types::ty_null)
    ft = uniqueFunction(rt);
  else
    ft = uniqueFunction(lt, rt);


  if (ft) {
    assert(ft->kind == ty_function);

    left->transToType(e, ft);
    right->transToType(e, ft);
    e.c.encode(inst::builtin, bltinFromName(callee->getName()));

    return primBoolean();
  } else {
    // Let callExp report a "no such function" error.
    types::ty *t = callExp::trans(e);
    assert(t->kind == ty_error);
    return t;
  }
}

void scaleExp::prettyprint(ostream &out, Int indent)
{
  exp *left=getLeft(); exp *right=getRight();

  prettyname(out, "scaleExp",indent);
  left->prettyprint(out, indent+1);
  right->prettyprint(out, indent+1);
}

types::ty *scaleExp::trans(coenv &e)
{
  exp *left=getLeft(); exp *right=getRight();

  types::ty *lt = left->cgetType(e);
  if (lt->kind != types::ty_Int && lt->kind != types::ty_real) {
    if (lt->kind != types::ty_error) {
      em.error(left->getPos());
      em << "only numeric constants can do implicit scaling";
    }
    right->trans(e);
    return types::primError();
  }

  if (!right->scalable()) {
    em.warning(right->getPos());
    em << "implicit scaling may be unintentional";
  }

  // Defer to the binaryExp for multiplication.
  return binaryExp::trans(e);
}


void intExp::prettyprint(ostream &out, Int indent)
{
  prettyindent(out,indent);
  out << "intExp: " << value << "\n";
}

types::ty *intExp::trans(coenv &e)
{
  e.c.encode(inst::intpush,value);
  
  return types::primInt();  
}


void realExp::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "realExp: " << value << "\n";
}

types::ty *realExp::trans(coenv &e)
{
  e.c.encode(inst::constpush,(item)value);
  
  return types::primReal();  
}

void stringExp::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "stringExp '" << str << "'\n";
}

types::ty *stringExp::trans(coenv &e)
{
  e.c.encode(inst::constpush,(item) string(str));
  
  return types::primString();  
}


void booleanExp::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "booleanExp: " << value << "\n";
}

types::ty *booleanExp::trans(coenv &e)
{
  e.c.encode(inst::constpush,(item)value);
  
  return types::primBoolean();  
}

void newPictureExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "newPictureExp",indent);
}

types::ty *newPictureExp::trans(coenv &e)
{
  e.c.encode(inst::builtin, run::newPicture);
  
  return types::primPicture();  
}

void cycleExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "cycleExp",indent);
}

types::ty *cycleExp::trans(coenv &e)
{
  e.c.encode(inst::builtin, run::newCycleToken);
  
  return types::primCycleToken();  
}

void nullPathExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "nullPathExp",indent);
}

types::ty *nullPathExp::trans(coenv &e)
{
  e.c.encode(inst::builtin, run::nullPath);
  
  return types::primPath();  
}

void nullExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "nullExp",indent);
}

types::ty *nullExp::trans(coenv &)
{
  // Things get put on the stack when ty_null
  // is cast to an appropriate type
  return types::primNull();  
}


void quoteExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "quoteExp", indent);
  value->prettyprint(out, indent+1);
}

types::ty *quoteExp::trans(coenv &e)
{
  e.c.encode(inst::constpush,(item)value);
  
  return types::primCode();  
}

void explist::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "explist",indent);
  for (expvector::iterator p = exps.begin();
       p != exps.end(); ++p)
    (*p)->prettyprint(out, indent+1);
}


void argument::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "explist";
  if (name)
    out << " '" << name << "'";
  out << '\n';

  val->prettyprint(out, indent+1);
}

void arglist::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "arglist",indent);
  for (argvector::iterator p = args.begin();
       p != args.end(); ++p)
    p->prettyprint(out, indent+1);
}

void callExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "callExp",indent);

  callee->prettyprint(out, indent+1);
  args->prettyprint(out, indent+1);
}

signature *callExp::argTypes(coenv &e, bool *searchable)
{
  signature *source=new signature;

  // The signature is searchable unless one of the arguments is overloaded or
  // named.
  *searchable = true;

  size_t n = args->size();
  
  for (size_t i = 0; i < n; i++) {
    if(string(args->args[i].name) == "KEY") {
      stringExp *s=dynamic_cast<stringExp*>(args->args[i].val);
      if(s) {
        if(getPos().filename() == processData().fileName)
          processData().xkey[getPos().LineColumn()]=Strdup(s->getString());
        args->args.erase(args->args.begin()+i);
        --n;
        if(i == n) break;
      }
    }

    argument a=(*args)[i];
    types::ty *t = a.val->cgetType(e);
    if (t->kind == types::ty_error)
      return 0;
    if (t->kind == types::ty_overloaded || a.name)
      *searchable = false;
    source->add(types::formal(t,a.name));
  }

  if (args->rest.val) {
    argument a=args->rest;
    types::ty *t = a.val->cgetType(e);
    if (t->kind == types::ty_error)
      return 0;
    if (t->kind == types::ty_overloaded || a.name)
      *searchable = false;
    source->addRest(types::formal(t,a.name));
  }

  return source;
}

application *callExp::resolve(coenv &e, overloaded *o, signature *source,
                              bool tacit) {
  app_list l=multimatch(e.e, o, source, *args);

  if (l.empty()) {
    //cerr << "l is empty\n";
    if (!tacit) {
      em.error(getPos());

      symbol s = callee->getName();
      if (s)
        em << "no matching function \'" << s;
      else
        em << "no matching function for signature \'";
      em << *source << "\'";
    }

    return 0;
  }
  else if (l.size() > 1) { // This may take O(n) time.
    //cerr << "l is full\n";
    if (!tacit) {
      em.error(getPos());

      symbol s = callee->getName();
      if(s)
        em << "call of function \'" << s;
      else
        em << "call with signature \'";
      em << *source << "\' is ambiguous";
    }

    return 0;
  }
  else {
    //cerr << "l is singleton\n";
    return l.front();
  }
}

bool hasNamedParameters(signature *sig) {
  for (size_t i=0; i < sig->getNumFormals(); ++i)
    if (sig->getFormal(i).name)
      return true;
  return false;
}

void callExp::reportMismatch(function *ft, signature *source)
{
  symbol s = callee->getName();
  const char *separator=ft->getSignature()->getNumFormals() > 1 ? "\n" : " ";

  em.error(getPos());
  em << "cannot call" << separator << "'" << *ft->getResult() << " ";
  if(s)
    em << s;
  em << *ft->getSignature() << "'" << separator;

  if (ft->getSignature()->isOpen && hasNamedParameters(source))
    em << "with named parameters";
  else
    switch(source->getNumFormals()) {
      case 0:
        em << "without parameters";
        break;
      case 1:
        em << "with parameter '" << *source << "'";
        break;
      default:
        em << "with parameters\n'" << *source << "'";
    }
}

void callExp::reportArgErrors(coenv &e)
{
  // Cycle through the parameters to report all errors.
  // NOTE: This may report inappropriate ambiguity errors. 
  for (size_t i = 0; i < args->size(); i++) {
    (*args)[i].val->trans(e);
  }
  if (args->rest.val)
    args->rest.val->trans(e);
}

void callExp::reportNonFunction() {
    em.error(getPos());
    symbol s = callee->getName();
    if (s)
      em << "\'" << s << "\' is not a function";
    else
      em << "called expression is not a function";
}

types::ty *callExp::cacheAppOrVarEntry(coenv &e, bool tacit)
{
  assert(cachedVarEntry == 0 && cachedApp == 0);

  // First figure out the signature of what we want to call.
  bool searchable;
  signature *source=argTypes(e, &searchable);

#ifdef DEBUG_GETAPP /* {{{ */
  cout << "getApp for ";
  if (callee->getName())
    cout << *callee->getName();
  else 
    cout << "unnamed";
  cout << " at " << getPos() << endl;
  cout << "searchable: " << searchable << endl;
#endif /* }}} */

  if (!source) {
    return primError();
  }

  // An attempt at speeding up compilation:  See if the source arguments match
  // the (possibly overloaded) function exactly.
  if (searchable) {
    varEntry *ve = callee->getCallee(e, source);

#ifdef DEBUG_GETAPP
    cout << "guessed: " << (ve!=0) << endl;
#endif

    if (ve) {
      cachedVarEntry = ve;
#ifndef DEBUG_CACHE
      // Normally DEBUG_CACHE is not defined and we return here for efficiency
      // reasons.  If DEBUG_CACHE is defined, we instead proceed to resolve
      // the function by the normal techniques and make sure we get the same
      // result.
      return ((function *)ve->getType())->getResult();
#endif
    }
  }

  // Figure out what function types we can call.
  types::ty *ft = callee->cgetType(e);

#ifdef DEBUG_GETAPP
  string name = callee->getName() ? string(*callee->getName()) :
                                    string("unnamed");
  if (!callee->getName())
    cout << getPos() << endl;
#endif

  switch (ft->kind) {
    case ty_error:
      if (!tacit)
        // Report callee errors.
        callee->trans(e);
      break;

    case ty_function:
      //cout << "name " << name << endl;
      cachedApp = application::match(e.e, (function *)ft, source, *args);
      if (!cachedApp && !tacit)
        reportMismatch((function *)ft, source);
      break;

    case ty_overloaded: {
#ifdef DEBUG_GETAPP
      int size = ((overloaded *)ft)->sub.size();
      for (int i = 0; i < size; ++i) cout << "name " << name << endl;
#endif
      cachedApp = resolve(e, (overloaded *)ft, source, tacit);
      break;
    }

    default:
      if (!tacit)
        reportNonFunction();
      break;
  }

#ifdef DEBUG_GETAPP
  cout << name << " " << *source << " --> "
       << *cachedApp->getType()->getSignature() << endl;
#endif

#if DEBUG_CACHE
  // Make sure cachedVarEntry is giving us the right function.
  if (cachedVarEntry)
    assert(equivalent(cachedVarEntry->getType(), cachedApp->getType()));
#endif

  // getType relies on this method for the type.
  return cachedApp ? cachedApp->getType()->getResult() : primError();
}

types::ty *callExp::transPerfectMatch(coenv &e) {
  // The varEntry of the callee.  (No longer needed after translation.)
  varEntry *ve = cachedVarEntry;
  cachedVarEntry = 0;
  assert(ve);

  // Translate the arguments in turn.
  for (size_t i = 0; i < args->size(); ++i)
    (*args)[i].val->trans(e);
  if (args->rest.val)
    args->rest.val->trans(e);

  // Call the function.
  ve->encode(trans::CALL, getPos(), e.c);

  // That's it.  Return the return type of the function.
  return ct ? ct : dynamic_cast<function *>(ve->getType())->getResult();
}

types::ty *callExp::trans(coenv &e)
{
  if (cachedVarEntry == 0 && cachedApp == 0)
    cacheAppOrVarEntry(e, false);

  if (cachedVarEntry)
    return transPerfectMatch(e);

  // The cached data is no longer needed after translation, so let it be
  // garbage collected.
  application *a = cachedApp;
  cachedApp=0;

  if (!a) {
    reportArgErrors(e);
    return primError();
  }

  // To simulate left-to-right order of evaluation, produce the
  // side-effects for the callee.
  assert(a);
  function *t=a->getType();
  assert(t);
  exp *temp=callee->evaluate(e, t);

  // Let the application handle the argument translation.
  a->transArgs(e);

  // Translate the call.
  temp->transCall(e, t);

  return t->result;
}

types::ty *callExp::getType(coenv &e)
{
  if (cachedApp)
    return cachedApp->getType()->getResult();
  if (cachedVarEntry) {
    function *ft = dynamic_cast<function *>(cachedVarEntry->getType());
    assert(ft);
    return ft->getResult();
  }
  return cacheAppOrVarEntry(e, true);
}

bool callExp::resolved(coenv &e) {
  if (cachedApp == 0 && cachedVarEntry == 0)
    cacheAppOrVarEntry(e, true);
  return cachedApp || cachedVarEntry;
}
  
    
void pairExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "pairExp",indent);

  x->prettyprint(out, indent+1);
  y->prettyprint(out, indent+1);
}

types::ty *pairExp::trans(coenv &e)
{
  x->transToType(e, types::primReal());
  y->transToType(e, types::primReal());

  e.c.encode(inst::builtin, run::realRealToPair);

  return types::primPair();
}

void tripleExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "tripleExp",indent);

  x->prettyprint(out, indent+1);
  y->prettyprint(out, indent+1);
  z->prettyprint(out, indent+1);
}

types::ty *tripleExp::trans(coenv &e)
{
  x->transToType(e, types::primReal());
  y->transToType(e, types::primReal());
  z->transToType(e, types::primReal());

  e.c.encode(inst::builtin, run::realRealRealToTriple);

  return types::primTriple();
}

void transformExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "transformExp",indent);

  x->prettyprint(out, indent+1);
  y->prettyprint(out, indent+1);
  xx->prettyprint(out, indent+1);
  xy->prettyprint(out, indent+1);
  yx->prettyprint(out, indent+1);
  yy->prettyprint(out, indent+1);
}

types::ty *transformExp::trans(coenv &e)
{
  x->transToType(e, types::primReal());
  y->transToType(e, types::primReal());
  xx->transToType(e, types::primReal());
  xy->transToType(e, types::primReal());
  yx->transToType(e, types::primReal());
  yy->transToType(e, types::primReal());

  e.c.encode(inst::builtin, run::real6ToTransform);

  return types::primTransform();
}

void castExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "castExp",indent);

  target->prettyprint(out, indent+1);
  castee->prettyprint(out, indent+1);
}

types::ty *castExp::tryCast(coenv &e, types::ty *t, types::ty *s,
                            symbol csym)
{
  types::ty *ss=e.e.castSource(t, s, csym);
  if (ss == 0) {
    return 0;
  }
  if (ss->kind == ty_overloaded) {
    em.error(getPos());
    em << "cast is ambiguous";
    return primError();
  }
  else {
    castee->transAsType(e, ss);

    access *a=e.e.lookupCast(t, ss, csym);
    assert(a);
    a->encode(trans::CALL, getPos(), e.c);
    return ss;
  }
}

types::ty *castExp::trans(coenv &e)
{
  target->addOps(e, (record *)0);
  types::ty *t=target->trans(e);

  types::ty *s=castee->cgetType(e);

  if (!tryCast(e, t, s, symbol::ecastsym))
    if (!tryCast(e, t, s, symbol::castsym)) {
      em.error(getPos());
      em << "cannot cast '" << *s << "' to '" << *t << "'";
    }

  return t;
}

types::ty *castExp::getType(coenv &e)
{
  return target->trans(e, true);
}


void conditionalExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "conditionalExp",indent);

  test->prettyprint(out, indent+1);
  onTrue->prettyprint(out, indent+1);
  onFalse->prettyprint(out, indent+1);
}

void conditionalExp::baseTransToType(coenv &e, types::ty *target) {
  test->transToType(e, types::primBoolean());

  label tlabel = e.c.fwdLabel();
  e.c.useLabel(inst::cjmp,tlabel);

  onFalse->transToType(e, target);

  label end = e.c.fwdLabel();
  e.c.useLabel(inst::jmp,end);

  e.c.defLabel(tlabel);
  onTrue->transToType(e, target);

  e.c.defLabel(end);
}

void conditionalExp::transToType(coenv &e, types::ty *target)
{
  if (isAnArray(e, test)) {
    if (target->kind != ty_array) {
      em.error(getPos());
      em << "cannot cast vectorized conditional to '" << *target << "'";
    }
    test->transToType(e, types::booleanArray());
    onTrue->transToType(e, target);
    onFalse->transToType(e, target);
    e.c.encode(inst::builtin, run::arrayConditional);
  }
  else {
    baseTransToType(e, target);
  }
}

types::ty *promote(coenv &e, types::ty *x, types::ty *y)
{
  struct promoter : public collector {
    env &e;

    promoter(env &e)
      : e(e) {}

    types::ty *both (types::ty *x, types::ty *y) {
      overloaded *o=new overloaded;
      o->add(x); o->add(y);
      return o;
    }

    types::ty *base (types::ty *x, types::ty *y) {
      if (equivalent(x,y))
        return x;
      else {
        bool castToFirst=e.castable(x, y, symbol::castsym);
        bool castToSecond=e.castable(y, x, symbol::castsym);

        return (castToFirst && castToSecond) ? both(x,y) : 
          castToFirst ? x :
          castToSecond ? y :
          0;
      }
    }
  };

  promoter p(e.e);
  return p.collect(x,y);
}

types::ty *conditionalExp::trans(coenv &e)
{
  types::ty *tt=onTrue->cgetType(e);
  types::ty *ft=onFalse->cgetType(e);

  if (tt->kind==ty_error)
    return onTrue->trans(e);
  if (ft->kind==ty_error)
    return onFalse->trans(e);

  types::ty *t=promote(e, tt, ft);
  if (!t) {
    em.error(getPos());
    em << "types in conditional expression do not match";
    return primError();
  }
  else if (t->kind == ty_overloaded) {
    em.error(getPos());
    em << "type of conditional expression is ambiguous";
    return primError();
  }

  transToType(e,t);
  return t;
}

types::ty *conditionalExp::getType(coenv &e)
{
  types::ty *tt=onTrue->cgetType(e);
  types::ty *ft=onFalse->cgetType(e);
  if (tt->kind==ty_error || ft->kind==ty_error)
    return primError();

  types::ty *t = promote(e, tt, ft);
  return t ? t : primError();
}
 

void orExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "orExp", indent);

  left->prettyprint(out, indent+1);
  right->prettyprint(out, indent+1);
}

types::ty *orExp::trans(coenv &e)
{
  //     a || b
  // translates into
  //     a ? true : b
  booleanExp be(pos, true);
  conditionalExp ce(pos, left, &be, right);
  ce.baseTransToType(e, primBoolean());

  return getType(e);
}

void orExp::transConditionalJump(coenv &e, bool cond, label dest)
{
  if (cond == true) {
    left->transConditionalJump(e, true, dest);
    right->transConditionalJump(e, true, dest);
  } else { /* cond == false */
    label end = e.c.fwdLabel();

    left->transConditionalJump(e, true, end);
    right->transConditionalJump(e, false, dest);

    e.c.defLabel(end);
  }
}


void andExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "andExp", indent);

  left->prettyprint(out, indent+1);
  right->prettyprint(out, indent+1);
}

types::ty *andExp::trans(coenv &e)
{
  //     a && b
  // translates into
  //     a ? b : false
  booleanExp be(pos, false);
  conditionalExp ce(pos, left, right, &be);
  ce.baseTransToType(e, primBoolean());

  return getType(e);
}

void andExp::transConditionalJump(coenv &e, bool cond, label dest)
{
  if (cond == true) {
    label end = e.c.fwdLabel();

    left->transConditionalJump(e, false, end);
    right->transConditionalJump(e, true, dest);

    e.c.defLabel(end);
  } else { /* cond == false */
    left->transConditionalJump(e, false, dest);
    right->transConditionalJump(e, false, dest);
  }
}

void joinExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "joinExp",indent);

  callee->prettyprint(out, indent+1);
  args->prettyprint(out, indent+1);
}


void specExp::prettyprint(ostream &out, Int indent)
{
  prettyindent(out,indent);
  out << "specExp '" << op << "' " 
      << (s==camp::OUT ? "out" :
          s==camp::IN  ? "in" :
          "invalid side") << '\n';

  arg->prettyprint(out, indent+1);
}

types::ty *specExp::trans(coenv &e)
{
  intExp ie(getPos(), (Int)s);
  binaryExp be(getPos(), arg, op, &ie);
  return be.trans(e);
}

types::ty *specExp::getType(coenv &e)
{
  intExp ie(getPos(), (Int)s);
  binaryExp be(getPos(), arg, op, &ie);
  return be.cgetType(e);
}

void assignExp::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "assignExp",indent);

  dest->prettyprint(out, indent+1);
  value->prettyprint(out, indent+1);
}

void assignExp::transAsType(coenv &e, types::ty *target)
{
#if 0
  // For left-to-right order, we have to evaluate the side-effects of the
  // destination first.
  exp *temp=dest->evaluate(e, target);
  ultimateValue(temp)->transToType(e, target);
  temp->transWrite(e, target);
#endif

  // All of the heavy work is handled by transWrite.
  dest->transWrite(e, target, value);
}

types::ty *assignExp::trans(coenv &e)
{
  exp *uvalue=ultimateValue(dest);
  types::ty *lt = dest->cgetType(e), *rt = uvalue->cgetType(e);

  if (lt->kind == ty_error)
    return dest->trans(e);
  if (rt->kind == ty_error)
    return uvalue->trans(e);

  types::ty *t = e.e.castTarget(lt, rt, symbol::castsym);
  if (!t) {
    em.error(getPos());
    em << "cannot convert '" << *rt << "' to '" << *lt << "' in assignment";
    return primError();
  }
  else if (t->kind == ty_overloaded) {
    em.error(getPos());
    em << "assignment is ambiguous";
    return primError();
  }
  else {
    transAsType(e, t);
    return t;
  }
}

types::ty *assignExp::getType(coenv &e)
{
  types::ty *lt = dest->cgetType(e), *rt = ultimateValue(dest)->cgetType(e);
  if (lt->kind==ty_error || rt->kind==ty_error)
    return primError();
  types::ty *t = e.e.castTarget(lt, rt, symbol::castsym);

  return t ? t : primError();
}


void selfExp::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "selfExp '" << op << "'\n";

  dest->prettyprint(out, indent+1);
  value->prettyprint(out, indent+1);
}

void selfExp::transAsType(coenv &e, types::ty *target)
{
  // Create a temp expression for the destination, so it is not evaluated
  // twice.
  exp *temp=dest->evaluate(e, target);
  temp->transWrite(e, target, ultimateValue(temp));
}

void prefixExp::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "prefixExp '" << op << "'\n";
  
  dest->prettyprint(out, indent+1);
}

types::ty *prefixExp::trans(coenv &e)
{
  // Convert into the operation and the assign.
  // NOTE: This can cause multiple evaluations.
  intExp ie(getPos(), 1);
  selfExp se(getPos(), dest, op, &ie);

  return se.trans(e);
}

types::ty *prefixExp::getType(coenv &e)
{
  // Convert into the operation and the assign.
  intExp ie(getPos(), 1);
  selfExp se(getPos(), dest, op, &ie);

  return se.getType(e);
}

void postfixExp::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "postfixExp <illegal> '" << op << "'\n";

  dest->prettyprint(out, indent+1);
}

types::ty *postfixExp::trans(coenv &)
{
  em.error(getPos());
  em << "postfix expressions are not allowed";
  return primError();
}


} // namespace absyntax

