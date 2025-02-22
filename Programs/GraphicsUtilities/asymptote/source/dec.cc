/*****
 * dec.cc
 * Andy Hammerlindl 2002/8/29
 *
 * Represents the abstract syntax tree for declarations in the language.
 * Also included is an abstract syntax for types as they are most often
 * used with declarations.
 *****/

#include "errormsg.h"
#include "coenv.h"
#include "dec.h"
#include "fundec.h"
#include "newexp.h"
#include "stm.h"
#include "exp.h"
#include "modifier.h"
#include "runtime.h"
#include "locate.h"
#include "asyparser.h"
#include "builtin.h"  // for trans::addRecordOps

namespace absyntax {

using namespace trans;
using namespace types;

using mem::list;

symbol intSymbol() {
  const static symbol* intSymbol = new symbol(symbol::literalTrans("int"));
  return *intSymbol;
}

bool usableInTemplate(ty *t) {
  assert(t);
  if (t->primitive()) return true;
  assert(t->kind != ty_null);
  assert(t->kind != ty_overloaded);

  if (t->kind == ty_record) {
    record* r= dynamic_cast<record*>(t);
    assert(r);
    assert(r->getLevel());
    if (!r->getLevel()->getParent()) return false;// r is actually a module
    if (!r->getLevel()->getParent()->getParent()) {
      return true;// r is a top-level record, or all nestings are static
    }
    return false; // r is nested non-statically
  }
  if (t->kind == ty_function) {
    function* f= dynamic_cast<function*>(t);
    assert(f);
    // Check the types of the result and all the parameters.
    if (!usableInTemplate(f->result)) return false;
    const signature& sig= *f->getSignature();
    for (const types::formal& f : sig.formals) {
      if (!usableInTemplate(f.t)) return false;
    }
    if (sig.hasRest() && !usableInTemplate(sig.getRest().t)) return false;
    return true;
  }
  if (t->kind == ty_array) {
    array* a= dynamic_cast<array*>(t);
    assert(a);
    return usableInTemplate(a->celltype);
  }
  // We should have already handled all the cases.
  assert(false);
  return false;
}


trans::tyEntry *astType::transAsTyEntry(coenv &e, record *where)
{
  return new trans::tyEntry(trans(e, false), nullptr, where, getPos());
}


void nameTy::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "nameTy",indent, getPos());

  id->prettyprint(out, indent+1);
}

void addNameOps(coenv &e, record *r, record *qt, varEntry *qv, position pos) {
  for (auto au : qt->e.ve.getAutoUnravels()) {
    symbol auName = au.first;
    varEntry *v = au.second;
    if (!v->checkPerm(READ, e.c)) {
      em.error(pos);
      em << "cannot access '" << auName << "' in current scope";
      continue;
    }
    if (r && e.c.getPermission() != PUBLIC) {
      // Add an additional restriction to v based on c.getPermission().
      v = new varEntry(*v, e.c.getPermission(), r);
    }
    varEntry *qqv = qualifyVarEntry(qv, v);
    auto enter= [&](trans::venv& ve) {
      // Add op only if it does not already exist.
      if (!ve.lookByType(auName, qqv->getType())) {
        ve.enter(auName, qqv);
      }
    };
    if (r) {
      enter(r->e.ve);
    }
    enter(e.e.ve);
  }
}

void nameTy::addOps(coenv &e, record *r, AutounravelOption opt)
{
  if (opt == AutounravelOption::Apply)
  {
    if (record* qt= dynamic_cast<record*>(id->getType(e, true)); qt)
    {
      varEntry* qv= id->getVarEntry(e);
      addNameOps(e, r, qt, qv, getPos());
    }
  }
}

types::ty *nameTy::trans(coenv &e, bool tacit)
{
  return id->typeTrans(e, tacit);
}

trans::tyEntry *nameTy::transAsTyEntry(coenv &e, record *)
{
  return id->tyEntryTrans(e);
}

nameTy::operator string() const
{
  return static_cast<string>(id->getName());
}

void dimensions::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "dimensions (" << depth << ")\n";
}

types::array *dimensions::truetype(types::ty *base, bool tacit)
{
  if (!tacit && base->kind == ty_void) {
    em.error(getPos());
    em << "cannot declare array of type void";
  }

  assert(depth >= 1);
  size_t d=depth;
  types::array *a=new types::array(base); d--;
  for (; d > 0; d--) {
    a = new types::array(a);
  }
  return a;
}


void arrayTy::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "arrayTy",indent, getPos());

  cell->prettyprint(out, indent+1);
  dims->prettyprint(out, indent+1);
}

// NOTE: Can this be merged with trans somehow?
void arrayTy::addOps(coenv &e, record *r, AutounravelOption)
{
  types::ty *t=trans(e, true);

  // Only add ops if it is an array (and not, say, an error)
  if (t->kind == types::ty_array) {
    types::array *at=dynamic_cast<types::array *>(t);
    assert(at);
    e.e.addArrayOps(at);
    if (r)
      r->e.addArrayOps(at);
  }
}

types::ty *arrayTy::trans(coenv &e, bool tacit)
{
  types::ty *ct = cell->trans(e, tacit);
  assert(ct);

  // Don't make an array of errors.
  if (ct->kind == types::ty_error)
    return ct;

  types::array *t = dims->truetype(ct,tacit);
  assert(t);

  return t;
}

arrayTy::operator string() const
{
  stringstream ss;
  ss << static_cast<string>(*cell);
  for (size_t i = 0; i < dims->size(); i++)
  {
    ss << "[]";
  }
  return ss.str();
}

tyEntryTy::tyEntryTy(position pos, types::ty* t)
    : astType(pos), ent(new trans::tyEntry(t, nullptr, nullptr, nullPos))
{}

void tyEntryTy::prettyprint(ostream &out, Int indent)
{
  prettyindent(out,indent);
  out << "tyEntryTy: " << *(ent->t) << "\n";
}

types::ty *tyEntryTy::trans(coenv &, bool) {
  return ent->t;
}

tyEntryTy::operator string() const
{
  return "<unknown-type>";
}

vm::lambda *runnable::transAsCodelet(coenv &e)
{
  coder c=e.c.newCodelet(getPos());
  coenv ce(c, e.e);
  markTrans(ce);
  return c.close();
}


void block::prettystms(ostream &out, Int indent)
{
  for (list<runnable *>::iterator p = stms.begin(); p != stms.end(); ++p)
    (*p)->prettyprint(out, indent);
}

void block::prettyprint(ostream &out, Int indent)
{
  prettyname(out,"block",indent,getPos());
  prettystms(out, indent+1);
}

// Uses RAII to ensure scope is ended when function returns.
class Scope {
  coenv* e;
public:
  Scope(coenv &e, bool scope) : e(scope ? &e : nullptr) {
    if (this->e) this->e->e.beginScope();
  }
  ~Scope() {
    if (this->e) e->e.endScope();
  }
};


void block::trans(coenv &e)
{
  Scope scopeHolder(e, scope);
  for (list<runnable *>::iterator p = stms.begin(); p != stms.end(); ++p) {
    (*p)->markTrans(e);
  }
}

void block::transAsField(coenv &e, record *r)
{
  Scope scopeHolder(e, scope);
  for (list<runnable *>::iterator p = stms.begin(); p != stms.end(); ++p) {
    (*p)->markTransAsField(e, r);
    if (em.errors() && !settings::debug)
      break;
  }
}

bool block::transAsTemplatedField(
  coenv &e, record *r, mem::vector<absyntax::namedTy*>* args
) {
  Scope scopeHolder(e, scope);
  receiveTypedefDec *dec = getTypedefDec();
  if (dec == nullptr) {
    em.error(getPos());
    em << "expected 'typedef import(<types>);'";
    em.sync(true);
    return false;
  }
  if(!dec->transAsParamMatcher(e, r, args/*, caller*/))
    return false;

  auto p = stms.begin();
  // Start with second statement since the first was a receiveTypedefDec.
  while (++p != stms.end()) {
    (*p)->markTransAsField(e, r);
    if (em.errors() && !settings::debug) {
      return false;
    }
  }
  em.sync();
  return true;
}

receiveTypedefDec* block::getTypedefDec()
{
  auto p= stms.begin();

  // Check for an empty file
  if (p == stms.end()) return nullptr;

  return dynamic_cast<receiveTypedefDec*>(*p);
}

record *block::transAsFile(genv& ge, symbol id)
{
  // Create the new module.
  record *r = new record(id, new frame(id,0,0));

  // Create coder and environment to translate the module.
  // File-level modules have dynamic fields by default.
  coder c(getPos(), r, 0);
  env e(ge);
  coenv ce(c, e);

  if (settings::getSetting<bool>("autoplain")) {
    autoplainRunnable()->transAsField(ce, r);
  }
  // If the file starts with a template declaration and it was accessed
  // or imported without template arguments, further translation is
  // likely futile.
  if (getTypedefDec() != nullptr) {
    if(!settings::getSetting<bool>("listvariables")) {
      em.error(getPos());
      em << "templated module access requires template parameters";
      em.sync();
    }
    return r;
  }

  // Translate the abstract syntax.
  transAsField(ce, r);
  ce.c.closeRecord();

  em.sync();
  if (em.errors()) return nullptr;

  return r;
}

record* block::transAsTemplatedFile(
        genv& ge, symbol id, mem::vector<absyntax::namedTy*>* args
)
{


  // Create the new module.
  record *r = new record(id, new frame(id, 0, 0));

  // Create coder and environment to translate the module.
  // File-level modules have dynamic fields by default.
  coder c(getPos(), r, 0);
  env e(ge);
  coenv ce(c, e);

  // Import `plain` before even translating "typedef import" since the latter
  // might change the meanings of symbols provided by `plain`.
  if (settings::getSetting<bool>("autoplain")) {
    autoplainRunnable()->transAsField(ce, r);
  }

  // Translate the abstract syntax.
  bool succeeded = transAsTemplatedField(ce, r, args);
  ce.c.closeRecord();
  if (!succeeded) {
    return nullptr;
  }

  return r;
}


bool block::returns() {
  // Search for a returning runnable, starting at the end for efficiency.
  for (list<runnable *>::reverse_iterator p=stms.rbegin();
       p != stms.rend();
       ++p)
    if ((*p)->returns())
      return true;
  return false;
}

vardec *block::asVardec()
{
  vardec *var = 0;
  for (list<runnable *>::iterator p=stms.begin();
       p != stms.end();
       ++p)
    {
      vardec *v = dynamic_cast<vardec *>(*p);
      if (v) {
        if (var)
          // Multiple vardecs.
          return 0;
        var = v;
      }
      else if (!dynamic_cast<emptyStm *>(*p))
        // Failure due to another runnable in the block.
        return 0;
    }

  return var;
}



void dec::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "dec", indent, getPos());
}


void modifierList::prettyprint(ostream &out, Int indent)
{
  prettyindent(out,indent);
  out << "modifierList (";

  for (list<modifier>::iterator p = mods.begin(); p != mods.end(); ++p) {
    if (p != mods.begin())
      out << ", ";
    switch (*p) {
      case EXPLICIT_STATIC:
        out << "static";
        break;
#if 0
      case EXPLICIT_DYNAMIC:
        out << "dynamic";
        break;
#endif
      default:
        out << "invalid code";
    }
  }

  for (list<permission>::iterator p = perms.begin(); p != perms.end(); ++p) {
    if (p != perms.begin() || !mods.empty())
      out << ", ";
    switch (*p) {
      case PUBLIC:
        out << "public";
        break;
      case PRIVATE:
        out << "private";
        break;
      default:
        out << "invalid code";
    }
  }

  out << ")\n";
}

bool modifierList::staticSet()
{
  return !mods.empty();
}

modifier modifierList::getModifier()
{
  assert(staticSet());
  int numAutounravel = 0;
  int numStatic = 0;
  for (modifier m : mods) {
    switch (m) {
      case AUTOUNRAVEL:
        ++numAutounravel;
        break;
      case EXPLICIT_STATIC:
      case DEFAULT_STATIC:
        ++numStatic;
        break;
      default:
        em.compiler(getPos());
        em << "invalid modifier";
    }
  }
  if (numAutounravel > 1) {
    em.error(getPos());
    em << "too many autounravel modifiers";
  }
  if (numStatic > 1) {
    em.error(getPos());
    em << "too many static modifiers";
  }
  if (numAutounravel) {
    return AUTOUNRAVEL;
  } else {
    return mods.front();
  }
}

permission modifierList::getPermission()
{
  if (perms.size() > 1) {
    em.error(getPos());
    em << "too many modifiers";
  }

  return perms.empty() ? DEFAULT_PERM : perms.front();
}


void modifiedRunnable::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "modifierRunnable",indent, getPos());

  mods->prettyprint(out, indent+1);
  body->prettyprint(out, indent+1);
}

void modifiedRunnable::transAsField(coenv &e, record *r)
{
  if (mods->staticSet()) {
    modifier mod = mods->getModifier();
    if (e.c.isTopLevel()) {
      if (mod == AUTOUNRAVEL) {
        em.error(getPos());
        em << "top-level fields cannot be autounraveled";
        return;
      } else {
        em.warning(getPos());
        em << "static modifier is meaningless at top level";
      }
    }
    e.c.pushModifier(mod);
  }

  permission p = mods->getPermission();
#if 0 // This is innocuous
  if (p != DEFAULT_PERM && (!r || !body->allowPermissions())) {
    em.warning(pos);
    em << "permission modifier is meaningless";
  }
#endif
  e.c.setPermission(p);

  body->transAsField(e,r);

  e.c.clearPermission();
  if (mods->staticSet())
    e.c.popModifier();
}


void decidstart::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "decidstart '" << id << "'\n";

  if (dims)
    dims->prettyprint(out, indent+1);
}

types::ty *decidstart::getType(types::ty *base, coenv &, bool)
{
  return dims ? dims->truetype(base) : base;
}

trans::tyEntry *decidstart::getTyEntry(trans::tyEntry *base, coenv &e,
                                       record *where)
{
  return dims ? new trans::tyEntry(
                        getType(base->t, e, false), nullptr, where, getPos()
                )
              : base;
}

void decidstart::addOps(types::ty *base, coenv &e, record *r)
{
  if (dims) {
    array *a=dims->truetype(base);
    e.e.addArrayOps(a);
    if (r)
      r->e.addArrayOps(a);
  }
}




  void fundecidstart::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "fundecidstart '" << id << "'\n";

  if (dims)
    dims->prettyprint(out, indent+1);
  if (params)
    params->prettyprint(out, indent+1);
}

types::ty *fundecidstart::getType(types::ty *base, coenv &e, bool tacit)
{
  types::ty *result = decidstart::getType(base, e, tacit);

  if (params) {
    return params->getType(result, e, true, tacit);
  }
  else {
    types::ty *t = new function(base);
    return t;
  }
}

trans::tyEntry *fundecidstart::getTyEntry(trans::tyEntry *base, coenv &e,
                                          record *where)
{
  return new trans::tyEntry(getType(base->t,e,false), nullptr, where, getPos());
}

void fundecidstart::addOps(types::ty *base, coenv &e, record *r)
{
  decidstart::addOps(base, e, r);

  params->addOps(e, r);

  types::function *ft=dynamic_cast<types::function *>(getType(base, e, true));
  assert(ft);

}


void decid::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "decid",indent, getPos());

  start->prettyprint(out, indent+1);
  if (init)
    init->prettyprint(out, indent+1);
}


varEntry *makeVarEntryWhere(coenv &e, record *r, types::ty *t,
                            record *where, position pos)
{
  access *a = r ? r->allocField(e.c.isStatic()) :
    e.c.allocLocal();

  return r ? new varEntry(t, a, e.c.getPermission(), r, where, pos) :
    new varEntry(t, a, where, pos);
}

varEntry *makeVarEntry(position pos, coenv &e, record *r, types::ty *t) {
  return makeVarEntryWhere(e, r, t, r, pos);
}


// Defined in constructor.cc.
bool definesImplicitConstructor(coenv &e, record *r, varEntry *v, symbol id);
void addConstructorFromInitializer(position pos, coenv &e, record *r,
                                   varEntry *init);

void addVar(coenv &e, record *r, varEntry *v, symbol id)
{
  // Test for 'operator init' definitions that implicitly define constructors:
  if (definesImplicitConstructor(e, r, v, id))
    addConstructorFromInitializer(nullPos, e, r, v);

  // Add to the record so it can be accessed when qualified; add to the
  // environment so it can be accessed unqualified in the scope of the
  // record definition.
  if (r) {
    r->e.addVar(id, v);
    if (e.c.isAutoUnravel()) {
      r->e.ve.registerAutoUnravel(id, v);
    }
  }
  e.e.addVar(id, v);
}

void initializeVar(position pos, coenv &e, varEntry *v, varinit *init)
{
  types::ty *t=v->getType();

  if (init)
    init->transToType(e, t);
  else {
    definit d(pos);
    d.transToType(e, t);
  }

  v->getLocation()->encode(WRITE, pos, e.c);
  e.c.encodePop();
}

types::ty *inferType(position pos, coenv &e, varinit *init)
{
  if (!init) {
    em.error(pos);
    em << "inferred variable declaration without initializer";
    return primError();
  }

  exp *base = dynamic_cast<exp *>(init);
  bool Void=false;

  if (base) {
    types::ty *t = base->cgetType(e);
    Void=t->kind == ty_void;
    if (t->kind != ty_overloaded && !Void)
      return t;
  }

  em.error(pos);
  em << (Void ? "cannot infer from void" :
         "could not infer type of initializer");

  return primError();
}

void createVar(position pos, coenv &e, record *r,
               symbol id, types::ty *t, varinit *init)
{
  // I'm not sure how to handle inferred types in these cases.
  assert(t->kind != types::ty_inferred);

  varEntry *v=makeVarEntry(pos, e, r, t);
  addVar(e, r, v, id);
  initializeVar(pos, e, v, init);
}

void createVarOutOfOrder(position pos, coenv &e, record *r,
                         symbol id, types::ty *t, varinit *init)
{
  /* For declarations such as "var x = 5;", infer the type from the
   * initializer.
   */
  if (t->kind == types::ty_inferred)
    t = inferType(pos, e, init);

  varEntry *v=makeVarEntry(pos, e, r, t);
  initializeVar(pos, e, v, init);
  addVar(e, r, v, id);
}

void addTypeWithPermission(coenv &e, record *r, tyEntry *base, symbol id)
{
  // Only bother encoding permissions for private types.
  tyEntry *ent = (r && e.c.getPermission()==PRIVATE) ?
    new trans::tyEntry(base, PRIVATE, r) :
    base;

  if (r)
    r->e.addType(id, ent);
  e.e.addType(id, ent);
}


void decid::transAsField(coenv &e, record *r, types::ty *base)
{
  types::ty *t = start->getType(base, e);
  assert(t);
  if (t->kind == ty_void) {
    em.error(getPos());
    em << "cannot declare variable of type void";
  }

  start->addOps(base, e, r);

  createVarOutOfOrder(getPos(), e, r, start->getName(), t, init);
}

void decid::transAsTypedefField(coenv &e, trans::tyEntry *base, record *r)
{
  trans::tyEntry *ent = start->getTyEntry(base, e, r);
  assert(ent && ent->t);

  if (init) {
    em.error(getPos());
    em << "type definition cannot have initializer";
  }

  start->addOps(base->t, e, r);

  addTypeWithPermission(e, r, ent, start->getName());
}




void decidlist::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "decidlist",indent, getPos());

  for (list<decid *>::iterator p = decs.begin(); p != decs.end(); ++p)
    (*p)->prettyprint(out, indent+1);
}

void decidlist::transAsField(coenv &e, record *r, types::ty *base)
{
  for (list<decid *>::iterator p = decs.begin(); p != decs.end(); ++p)
    (*p)->transAsField(e, r, base);
}

void decidlist::transAsTypedefField(coenv &e, trans::tyEntry *base, record *r)
{
  for (list<decid *>::iterator p = decs.begin(); p != decs.end(); ++p)
    (*p)->transAsTypedefField(e, base, r);
}


void vardec::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "vardec",indent, getPos());

  base->prettyprint(out, indent+1);
  decs->prettyprint(out, indent+1);
}

void vardec::transAsTypedefField(coenv &e, record *r)
{
  base->addOps(e, r);
  decs->transAsTypedefField(e, base->transAsTyEntry(e, r), r);
}

symbol vardec::singleName()
{
  decid *did = decs->singleEntry();
  if (!did)
    return symbol::nullsym;
  return did->getStart()->getName();
}

types::ty *vardec::singleGetType(coenv &e)
{
  decid *did = decs->singleEntry();
  if (!did)
    return 0;
  return did->getStart()->getType(base->trans(e), e);
}




// Helper class for imports.  This essentially evaluates to the run::loadModule
// function.  However, that function returns different types of records
// depending on the filename given to it, so we cannot add it to the
// environment.  Instead, we explicitly tell it what types::record it is
// returning for each use.
class loadModuleExp : public exp {
  function *ft;

public:
  loadModuleExp(position pos, record* imp) : exp(pos)
  {
    ft= new function(imp, primString(), primInt());
  }

  void prettyprint(ostream &out, Int indent) {
    prettyname(out, "loadModuleExp", indent, getPos());
  }

  types::ty *trans(coenv &) {
    em.compiler(getPos());
    em << "trans called for loadModuleExp";
    return primError();
  }

  void transCall(coenv &e, types::ty *t) {
    assert(equivalent(t, ft));
    e.c.encode(inst::builtin, run::loadModule);
  }

  types::ty *getType(coenv &) {
    return ft;
  }

  exp *evaluate(coenv &, types::ty *) {
    // Don't alias.
    return this;
  }
};

// Creates a local variable to hold the import and translate the accessing of
// the import, but doesn't add the import to the environment.
varEntry *accessModule(position pos, coenv &e, record *r, symbol id)
{
  string filename=(string) id;
  bool tainted=settings::debug && em.errors();
  record *imp=e.e.getModule(id, filename);
  if (!imp) {
    if(!tainted) {
      em.error(pos);
      em << "could not load module '" << filename << "'";
      em.sync(true);
    }
    return 0;
  }
  else {
    // Create a varinit that evaluates to the module.
    // This is effectively the expression 'loadModule(filename, 0)'.
    callExp init(
            pos, new loadModuleExp(pos, imp), new stringExp(pos, filename),
            new intExp(pos, 0)
    );

    // The varEntry should have whereDefined()==0 as it is not defined inside
    // the record r.
    varEntry *v=makeVarEntryWhere(e, r, imp, 0, pos);
    initializeVar(pos, e, v, &init);
    return v;
  }
}



// Returns the number of items that will be added to the stack once the
// translation has been run (always either 0 or 1).
Int transPushParent(formal *f, coenv &e) {
  if (f->getType(e)->kind != types::ty_record) {
    return 0;
  }
  astType *astT = f->getAbsyntaxType();
  tyEntry *ent = astT->transAsTyEntry(e, nullptr);
  bool succeeded = e.c.encodeParent(f->getPos(), ent);
  if (!succeeded) {
    em.compiler(f->getPos());
    em << "failed to encode parent level";
    em.sync(true);
  }

  return 1;
}

// Translates formals into namedTys.
mem::vector<namedTy*> *computeTemplateArgs(formals *args, coenv &e) {
  auto *computedArgs = new mem::vector<namedTy*>();
  for (formal *f : *args) {
    symbol theName = f->getName();
    position sourcePos = f->getPos();
    if (theName == symbol::nullsym) {
      em.error(sourcePos);
      em << "expected typename=";
      em.sync(true);
      return nullptr;
    }
    types::ty *t = f->getType(e);
    if (!usableInTemplate(t)) {
      em.error(f->getAbsyntaxType()->getPos());
      em << "non-statically nested types cannot be used in templates";
      em.sync(true);
      return nullptr;
    }
    computedArgs->push_back(new namedTy(sourcePos, theName, t));
  }
  return computedArgs;
}

// Creates a local variable to hold the import and translate the accessing of
// the import, but doesn't add the import to the environment.
varEntry *accessTemplatedModule(position pos, coenv &e, record *r, symbol id,
                                formals *args)
{
  string moduleName=(string) id;

  mem::vector<namedTy*> *computedArgs = computeTemplateArgs(args, e);
  if (!computedArgs) {
    return nullptr;
  }

  record *imp=e.e.getTemplatedModule(moduleName,computedArgs);
  if (!imp) {
    em.error(pos);
    em << "could not load module '" << id << "'";
    em.sync(true);
    return nullptr;
  }
  // Encode action: Push parents to the stack.
  Int numParents = 0;
  // We push parents in reverse order so that we can later pop them in
  // order, meaning that if more than one parameter has an error, the first
  // error will be reported rather than the last.
  for (auto p = args->rbegin(); p != args->rend(); ++p) {
    numParents += transPushParent(*p, e);
  }

  // Create a varinit that evaluates to the module.
  // This is effectively the expression 'loadModule(index, numParents)'.
  callExp init(
          pos, new loadModuleExp(pos, imp),
          new stringExp(pos, imp->getTemplateIndex()),
          new intExp(pos, numParents)
  );

  // The varEntry should have whereDefined()==nullptr as it is not defined
  // inside the record r.
  varEntry *v=makeVarEntryWhere(e, r, imp, nullptr, pos);
  initializeVar(pos, e, v, &init);
  return v;
}


void idpair::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "idpair (" << "'" << src << "' as " << dest << ")\n";
}

void idpair::transAsAccess(coenv &e, record *r)
{
  checkValidity();

  varEntry *v=accessModule(getPos(), e, r, src);
  if (v)
    addVar(e, r, v, dest);
}

tyEntry *idpair::transAsUnravel(coenv &e, record *r,
                                protoenv &source, varEntry *qualifier)
{
  checkValidity();

  if (r) {
    auto fieldsAdded = r->e.add(src, dest, source, qualifier, e.c)->varsAdded;
    if (e.c.isAutoUnravel()) {
      for (varEntry *v : fieldsAdded) {
        r->e.ve.registerAutoUnravel(dest, v);
      }
    }
  }
  protoenv::Added *added = e.e.add(src, dest, source, qualifier, e.c);
  if (added->empty()) {
    em.error(getPos());
    em << "no matching types or fields of name '" << src << "'";
  }
  return added->typeAdded;
}


void idpairlist::prettyprint(ostream &out, Int indent)
{
  for (list<idpair *>::iterator p=base.begin();
       p != base.end();
       ++p)
    (*p)->prettyprint(out, indent);
}

void idpairlist::transAsAccess(coenv &e, record *r)
{
  for (list<idpair *>::iterator p=base.begin();
       p != base.end();
       ++p)
    (*p)->transAsAccess(e,r);
}

mem::vector<tyEntry*> idpairlist::transAsUnravel(
  coenv &e, record *r, protoenv &source, varEntry *qualifier
) {
  mem::vector<tyEntry*> result;
  for (idpair *p : base)
  {
    tyEntry *typeAdded = p->transAsUnravel(e,r,source,qualifier);
    result.push_back(typeAdded);
  }
  return result;
}

idpairlist * const WILDCARD = 0;

void accessdec::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "accessdec", indent, getPos());
  base->prettyprint(out, indent+1);
}

void templateAccessDec::transAsField(coenv& e, record* r) {
  if (!this->checkValidity()) return;

  args->addOps(e, r);

  varEntry *v=accessTemplatedModule(getPos(), e, r, this->src, args);
  if (v)
    addVar(e, r, v, dest);
}

void typeParam::prettyprint(ostream &out, Int indent) {
  prettyindent(out, indent);
  out << "typeParam (" << paramSym <<  ")\n";
}

void recordInitializer(coenv &e, symbol id, record *r, position here)
{
  // This is almost equivalent to the code
  //   autounravel A operator init() { return new A; }
  // where A is the name of the record. The "almost" is because the code below
  // does not add the operator init to the environment, since in practice it
  // would be added to the outer environment, not the record's environment.
  // Since it is always added *after* any code in the record, we lose nothing
  // by not adding it to the environment.
  // Additionally, the "autounravel" is made low-priority (will not override
  // user-defined operator init) which is possible only for built-in functions.
  formals formals(here);
  simpleName recordName(here, id);
  nameTy result(here, &recordName);
  newRecordExp exp(here, &result);
  returnStm stm(here, &exp);
  fundef fun(here, &result, &formals, &stm);
  assert(r);
  {
    e.c.pushModifier(AUTOUNRAVEL);
    function *ft = fun.transType(e, false);
    assert(ft);

    symbol initSym=symbol::opTrans("init");
    varinit *init=fun.makeVarInit(ft);

    assert(ft->kind != types::ty_inferred);

    varEntry *v=makeVarEntry(here, e, r, ft);
    r->e.addVar(initSym, v);
    r->e.ve.registerAutoUnravel(initSym, v, trans::AutounravelPriority::OFFER);
    initializeVar(here, e, v, init);
    e.c.popModifier();
  }
}

bool typeParam::transAsParamMatcher(coenv &e, record *module, namedTy* arg) {
  symbol name = arg->dest;
  ty *t = arg->t;
  if (name != paramSym) {
    em.error(arg->pos);
    em << "template argument name does not match module: passed "
       << name
       << ", expected "
       << paramSym;
    return false;
  }
  if (t->kind != types::ty_record) {
    tyEntry *ent = new tyEntry(t, nullptr, module, getPos());
    addTypeWithPermission(e, module, ent, name);
    return true;
  }

  // We can now assume t is a record (i.e., asy struct).
  record *r = dynamic_cast<record *>(t);
  assert(r);
  //
  // Build a varEntry v for the parent level and encode the bytecode
  // to pop the parent off the stack into v. The varEntry needs a type,
  // but the only thing we use from the type is the level, so make a
  // new fake record.
  string fakeParentName;
# ifdef DEBUG_FRAME
  {
    ostringstream oss;
    oss << "<fake parent of " << name << " holding ";
    print(oss, r->getLevel()->getParent());
    oss << ">";
    fakeParentName = oss.str();
  }
# else
  fakeParentName = "<fake parent of " + static_cast<string>(name) + ">";
# endif
  record* fakeParent = new record(
          symbol::literalTrans(fakeParentName), r->getLevel()->getParent()
  );
  varEntry *v = makeVarEntryWhere(e, module, fakeParent, nullptr, getPos());
  // Encode bytecode to pop the parent off the stack into v.
  v->encode(WRITE, getPos(), e.c);
  e.c.encodePop();
  //
  // Build tyEntry, using v as ent->v.
  tyEntry *ent = new tyEntry(t, v, module, getPos());
  addTypeWithPermission(e, module, ent, name);

  // Add any autounravel fields.
  addNameOps(e, module, r, v, getPos());

  return true;
}

void typeParamList::prettyprint(ostream &out, Int indent) {
  for (auto p = params.begin(); p != params.end(); ++p) {
    (*p)->prettyprint(out, indent);
  }
}

void typeParamList::add(typeParam *tp) {
  params.push_back(tp);
}

// RAII class to set the permission of a coder to a new value, and then reset it
// to the old value when the object goes out of scope.
class PermissionSetter {
  coder &c;
  permission oldPerm;
public:
  PermissionSetter(coder &c, permission newPerm) : c(c), oldPerm(c.getPermission()) {
    c.setPermission(newPerm);
  }
  ~PermissionSetter() {
    c.setPermission(oldPerm);
  }
};

bool typeParamList::transAsParamMatcher(
  coenv &e, record *r, mem::vector<namedTy*> *args
) {

  // Check that the number of arguments passed matches the number of parameters.
  if (args->size() != params.size()) {
    position pos = getPos();
    if (args->size() >= 1) {
      pos = (*args)[0]->pos;
    }
    em.error(pos);
    if (args->size() > params.size()) {
      em << "too many types passed: got " << args->size() << ", expected "
         << params.size();
    } else {
      em << "too few types passed: got " << args->size() << ", expected "
         << params.size();
    }
    return false;
  }

  // Set the permission to PRIVATE while translating type parameters.
  PermissionSetter ps(e.c, PRIVATE);
  // Pop the parents off the stack. They were pushed in reverse order.
  // With this approach, the first error will be reported, rather than the last.
  for (size_t i = 0; i < params.size(); ++i) {
    bool succeeded = params[i]->transAsParamMatcher(e, r, (*args)[i]);
    if (!succeeded) return false;
  }
  return true;
}

bool receiveTypedefDec::transAsParamMatcher(
  coenv& e, record *r, mem::vector<namedTy*> *args
) {
  bool succeeded = params->transAsParamMatcher(e, r, args);
  return succeeded;
}

void receiveTypedefDec::transAsField(coenv& e, record *r) {
  em.error(getPos());
  em << "unexpected 'typedef import'";
  em.sync();
}


void fromdec::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "fromdec", indent, getPos());
  fields->prettyprint(out, indent+1);
}

void fromdec::transAsField(coenv &e, record *r)
{
  qualifier q=getQualifier(e,r);
  if (q.t) {
    if (fields==WILDCARD) {
      if (r)
        r->e.add(q.t->e, q.v, e.c);
      e.e.add(q.t->e, q.v, e.c);
    } else {
      auto typesAdded = fields->transAsUnravel(e, r, q.t->e, q.v);
      for (tyEntry *te : typesAdded) {
        if (te) {
          record *t = dynamic_cast<record*>(te->t);
          if (t) {
            addNameOps(e, r, t, te->v, getPos());
          }
        }
      }

    }
  }
}


void unraveldec::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "unraveldec", indent, getPos());
  id->prettyprint(out, indent+1);
  idpairlist *f=this->fields;
  if(f) f->prettyprint(out, indent+1);
}

fromdec::qualifier unraveldec::getQualifier(coenv &e, record *)
{
  // getType is where errors in the qualifier are reported.
  record *qt=dynamic_cast<record *>(id->getType(e, false));
  if (!qt) {
    em.error(getPos());
    em << "qualifier is not a record";
  }

  return qualifier(qt,id->getVarEntry(e));
}

void fromaccessdec::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "fromaccessdec '" << id << "'\n";
  idpairlist *f=this->fields;
  if(f) f->prettyprint(out, indent+1);
}

fromdec::qualifier fromaccessdec::getQualifier(coenv &e, record *r)
{
  varEntry *v = 0;
  if (templateArgs) {
    v = accessTemplatedModule(getPos(), e, r, id, templateArgs);
  } else {
    v=accessModule(getPos(), e, r, id);
  }
  if (v) {
    record *qt=dynamic_cast<record *>(v->getType());
    if (!qt) {
      em.compiler(getPos());
      em << "qualifier is not a record";
    }
    return qualifier(qt, v);
  }
  else
    return qualifier(0,0);
}



void importdec::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "importdec", indent, getPos());
  base.prettyprint(out, indent+1);
}



void includedec::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "includedec ('" << filename << "')\n";
}

void includedec::loadFailed(coenv &)
{
  em.warning(getPos());
  em << "could not parse file of name '" << filename << "'";
  em.sync(true);
}

void includedec::transAsField(coenv &e, record *r)
{
  file *ast = parser::parseFile(filename,"Including");
  em.sync();

  // The runnables will be translated, one at a time, without any additional
  // scoping.
  ast->transAsField(e, r);
}




void typedec::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "typedec",indent, getPos());

  body->prettyprint(out, indent+1);
}


void recorddec::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "structdec '" << id << "'\n";

  body->prettyprint(out, indent+1);
}

void recorddec::transRecordInitializer(coenv &e, record *parent)
{
  recordInitializer(e,id,parent,getPos());
}

void recorddec::addPostRecordEnvironment(coenv &e, record *r, record *parent) {
  if (parent)
    parent->e.add(r->postdefenv, 0, e.c);
  e.e.add(r->postdefenv, 0, e.c);
  // Add the autounravel fields also.
  addNameOps(e, parent, r, nullptr, getPos());
}

void recorddec::transAsField(coenv &e, record *parent)
{
  if (e.c.isAutoUnravel()) {
    em.error(getPos());
    em << "types cannot be autounraveled";
  }
  record *r = parent ? parent->newRecord(id, e.c.isStatic()) :
    e.c.newRecord(id);

  addTypeWithPermission(
          e, parent, new trans::tyEntry(r, nullptr, parent, getPos()), id
  );
  trans::addRecordOps(r);

  // Start translating the initializer.
  coder c=e.c.newRecordInit(getPos(), r);
  coenv re(c,e.e);

  {
    // Make sure the autounraveled fields are limited to the record's scope.
    // If the scope is too broad, then user-provide autounravel overrides will
    // defer to already-defined ops when they should not.
    bool useScope = body->scope;
    // RAII: Close the scope when scopeHolder runs its destructor.
    Scope scopeHolder(re, useScope);
    // Autounravel the record ops into the record's environment.
    addNameOps(re, nullptr, r, nullptr, getPos());
    // We've already handled the scope ourselves, so tell `body` not to add an
    // additional scope when running `transAsField`.
    body->scope = false;
    // Translate the main body of the record.
    body->transAsField(re, r);
    // Restore the original value of the `scope` boolean. This probably makes no
    // difference but is included out of an abundance of caution.
    body->scope = useScope;
  }  // Close the scope.
  // After the record is translated, add a default initializer so that a
  // variable of the type of the record is initialized to a new instance by
  // default.
  transRecordInitializer(re, r);

  // This would normally be done right after transAsField, but we needed to add
  // the default initializer first.
  re.c.closeRecord();


  // Add types and variables defined during the record that should be added to
  // the enclosing environment.  These are the implicit constructors defined by
  // "operator init", as well as the autounravel fields (both builtin and user
  // defined).
  addPostRecordEnvironment(e, r, parent);
}

runnable *autoplainRunnable() {
  // Abstract syntax for the code:
  //   private import plain;
  position pos=nullPos;
  static importdec ap(pos, new idpair(pos, symbol::literalTrans("plain")));
  static modifiedRunnable mr(pos, trans::PRIVATE, &ap);

  return &mr;
}

} // namespace absyntax
