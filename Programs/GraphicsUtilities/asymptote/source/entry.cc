/*****
 * entry.cc
 * Andy Hammerlindl 2002/08/29
 *
 * All variables, built-in functions and user-defined functions reside
 * within the same namespace.  To keep track of all these, table of
 * "entries" is used.
 *****/

#include <iostream>

#include <cmath>
#include <cstring>
#include <utility>
#include "entry.h"
#include "coder.h"

using std::memset;
using types::ty;
using types::signature;
using types::overloaded;
using types::ty_vector;
using types::ty_iterator;

namespace trans {

bool entry::pr::check(action act, coder &c) {
  // We assume PUBLIC permissions and one's without an associated record are not
  // stored.
  assert(perm!=PUBLIC && r!=0);
  return c.inTranslation(r->getLevel()) ||
    (perm == RESTRICTED && act != WRITE);
}

void entry::pr::report(action act, position pos, coder &c) {
  if (!c.inTranslation(r->getLevel())) {
    if (perm == PRIVATE) {
      em.error(pos);
      em << "accessing private field outside of structure";
    }
    else if (perm == RESTRICTED && act == WRITE) {
      em.error(pos);
      em << "modifying non-public field outside of structure";
    }
  }
}

entry::entry(entry &e1, entry &e2) : where(e2.where), pos(e2.pos) {
  perms.insert(perms.end(), e1.perms.begin(), e1.perms.end());
  perms.insert(perms.end(), e2.perms.begin(), e2.perms.end());
}

entry::entry(entry &base, permission perm, record *r)
  : where(base.where), pos(base.pos) {
  perms.insert(perms.end(), base.perms.begin(), base.perms.end());
  addPerm(perm, r);
}

bool entry::checkPerm(action act, coder &c) {
  for (mem::list<pr>::iterator p=perms.begin(); p != perms.end(); ++p)
    if (!p->check(act, c))
      return false;
  return true;
}

void entry::reportPerm(action act, position pos, coder &c) {
  for (mem::list<pr>::iterator p=perms.begin(); p != perms.end(); ++p)
    p->report(act, pos, c);
}


varEntry::varEntry(varEntry &qv, varEntry &v)
  : entry(qv,v), t(v.t),
    location(new qualifiedAccess(qv.location, qv.getLevel(), v.location)) {}

frame *varEntry::getLevel() {
  record *r=dynamic_cast<record *>(t);
  assert(r);
  return r->getLevel();
}

void varEntry::encode(action act, position pos, coder &c) {
  reportPerm(act, pos, c);
  getLocation()->encode(act, pos, c);
}

void varEntry::encode(action act, position pos, coder &c, frame *top) {
  reportPerm(act, pos, c);
  getLocation()->encode(act, pos, c, top);
}

varEntry *qualifyVarEntry(varEntry *qv, varEntry *v)
{
  return qv ? (v ? new varEntry(*qv,*v) : qv) : v;
}


bool tenv::add(symbol dest,
               names_t::value_type &x, varEntry *qualifier, coder &c)
{
  if (!x.second.empty()) {
    tyEntry *ent=x.second.front();
    if (ent->checkPerm(READ, c)) {
      enter(dest, qualifyTyEntry(qualifier, ent));
      return true;
    }
  }
  return false;
}

void tenv::add(tenv& source, varEntry *qualifier, coder &c) {
  // Enter each distinct (unshadowed) name,type pair.
  for(names_t::iterator p = source.names.begin(); p != source.names.end(); ++p)
    add(p->first, *p, qualifier, c);
}

bool tenv::add(symbol src, symbol dest,
               tenv& source, varEntry *qualifier, coder &c) {
  names_t::iterator p = source.names.find(src);
  if (p != source.names.end())
    return add(dest, *p, qualifier, c);
  else
    return false;
}

#if 0 //{{{
/*NOHASH*/ void venv::add(venv& source, varEntry *qualifier, coder &c)
/*NOHASH*/ {
/*NOHASH*/   // Enter each distinct (unshadowed) name,type pair.
/*NOHASH*/   for(names_t::iterator p = source.names.begin();
/*NOHASH*/       p != source.names.end();
/*NOHASH*/       ++p)
/*NOHASH*/     add(p->first, p->first, source, qualifier, c);
/*NOHASH*/ }
/*NOHASH*/ 
/*NOHASH*/ bool venv::add(symbol src, symbol dest,
/*NOHASH*/                venv& source, varEntry *qualifier, coder &c)
/*NOHASH*/ {
/*NOHASH*/   bool added=false;
/*NOHASH*/   name_t &list=source.names[src];
/*NOHASH*/   types::overloaded set; // To keep track of what is shadowed.
/*NOHASH*/   bool special = src.special();
/*NOHASH*/ 
/*NOHASH*/   for(name_iterator p = list.begin();
/*NOHASH*/       p != list.end();
/*NOHASH*/       ++p) {
/*NOHASH*/     varEntry *v=*p;
/*NOHASH*/     if (!equivalent(v->getType(), &set)) {
/*NOHASH*/       set.addDistinct(v->getType(), special);
/*NOHASH*/       if (v->checkPerm(READ, c)) {
/*NOHASH*/         enter(dest, qualifyVarEntry(qualifier, v));
/*NOHASH*/         added=true;
/*NOHASH*/       }
/*NOHASH*/     }
/*NOHASH*/   }
/*NOHASH*/   
/*NOHASH*/   return added;
/*NOHASH*/ }
/*NOHASH*/ 
/*NOHASH*/ varEntry *venv::lookByType(symbol name, ty *t)
/*NOHASH*/ {
/*NOHASH*/   // Find first applicable function.
/*NOHASH*/   name_t &list = names[name];
/*NOHASH*/   for(name_iterator p = list.begin();
/*NOHASH*/       p != list.end();
/*NOHASH*/       ++p) {
/*NOHASH*/     if (equivalent((*p)->getType(), t))
/*NOHASH*/       return *p;
/*NOHASH*/   }
/*NOHASH*/   return 0;
/*NOHASH*/ }
/*NOHASH*/ 
/*NOHASH*/ void venv::list(record *module)
/*NOHASH*/ {
/*NOHASH*/   bool where=settings::getSetting<bool>("where");
/*NOHASH*/   // List all functions and variables.
/*NOHASH*/   for(names_t::iterator N = names.begin(); N != names.end(); ++N) {
/*NOHASH*/     symbol s=N->first;
/*NOHASH*/     name_t &list=names[s];
/*NOHASH*/     for(name_iterator p = list.begin(); p != list.end(); ++p) {
/*NOHASH*/       if(!module || (*p)->whereDefined() == module) {
/*NOHASH*/         if(where) cout << (*p)->getPos();
/*NOHASH*/         (*p)->getType()->printVar(cout, s);
/*NOHASH*/         cout << ";\n";
/*NOHASH*/       }
/*NOHASH*/     }
/*NOHASH*/   }
/*NOHASH*/   flush(cout);
/*NOHASH*/ }
/*NOHASH*/ 
/*NOHASH*/ ty *venv::getType(symbol name)
/*NOHASH*/ {
/*NOHASH*/   types::overloaded set;
/*NOHASH*/ 
/*NOHASH*/   // Find all applicable functions in scope.
/*NOHASH*/   name_t &list = names[name];
/*NOHASH*/   bool special = name.special();
/*NOHASH*/   
/*NOHASH*/   for(name_iterator p = list.begin();
/*NOHASH*/       p != list.end();
/*NOHASH*/       ++p) {
/*NOHASH*/     set.addDistinct((*p)->getType(), special);
/*NOHASH*/   }
/*NOHASH*/ 
/*NOHASH*/   return set.simplify();
/*NOHASH*/ }
// }}}
#else

// To avoid writing qualifiers everywhere.
typedef core_venv::cell cell;

void core_venv::initTable(size_t capacity) {
  // Assert that capacity is a power of two.
  assert((capacity & (capacity-1)) == 0);

  this->capacity = capacity;
  size = 0;
  mask = capacity - 1;
  table = new (UseGC) cell[capacity];
  memset(table, 0, sizeof(cell) * capacity);
}

void core_venv::clear() {
  if (size != 0) {
    memset(table, 0, sizeof(cell) * capacity);
    size = 0;
  }
}

void core_venv::resize() {
  size_t oldCapacity = capacity;
  size_t oldSize = size;
  cell *oldTable = table;

  initTable(capacity * 4);

  for (size_t i = 0; i < oldCapacity; ++i) {
    cell& b = oldTable[i];
    if (!b.empty() && !b.isATomb()) {
      varEntry *old = store(b.name, b.ent);

      // We should not be shadowing anything when reconstructing the table.
      DEBUG_CACHE_ASSERT(old == 0);
    }
  }

  assert(size == oldSize);

#if DEBUG_CACHE
  confirm_size();

  for (size_t i = 0; i < oldCapacity; ++i) {
    cell& b = oldTable[i];
    if (!b.empty() && !b.isATomb()) {
      assert(lookup(b.name, b.ent->getType()) == b.ent);
    }
  }
#endif

  //cout << "resized from " << oldCapacity << " to " << capacity << endl;
}

cell& core_venv::cellByIndex(size_t i) {
  return table[i & mask];
}

const cell& core_venv::cellByIndex(size_t i) const {
  return table[i & mask];
}

void core_venv::confirm_size() {
  size_t sum = 0;
  for (size_t i = 0; i < capacity; ++i) {
    cell& b = table[i];
    if (!b.empty() && !b.isATomb())
      ++sum;
  }
  assert(sum == size);
}

varEntry *core_venv::storeNew(cell& cell, symbol name, varEntry *ent) {
  // Store the value into the cell.
  cell.storeNew(name, ent);

  // We now have a new entry.  Update the size.
  ++size;

  // Check if the hash table has grown too big and needs to be expanded.
  if (2*size > capacity)
    resize();

  // Nothing is shadowed.
  return 0;
}


varEntry *core_venv::storeNonSpecialAfterTomb(size_t tombIndex,
                                              symbol name, varEntry *ent) {
  DEBUG_CACHE_ASSERT(name.notSpecial());
  DEBUG_CACHE_ASSERT(ent);
  DEBUG_CACHE_ASSERT(ent->getType());

  signature *sig = ent->getSignature();

  for (size_t i = tombIndex+1; ; ++i)
  {
    cell& b = cellByIndex(i);

    if (b.empty())
      return storeNew(cellByIndex(tombIndex), name, ent);

    if (b.matches(name, sig))
      return b.replaceWith(name, ent);
  }
}

varEntry *core_venv::storeSpecialAfterTomb(size_t tombIndex,
                                           symbol name, varEntry *ent) {
  DEBUG_CACHE_ASSERT(name.special());
  DEBUG_CACHE_ASSERT(ent);
  DEBUG_CACHE_ASSERT(ent->getType());

  ty *t = ent->getType();

  for (size_t i = tombIndex+1; ; ++i)
  {
    cell& b = cellByIndex(i);

    if (b.empty())
      return storeNew(cellByIndex(tombIndex), name, ent);

    if (b.matches(name, t))
      return b.replaceWith(name, ent);
  }
}

size_t hashSig(const signature *sig) {
  return sig ? sig->hash() : 0;
}
size_t nonSpecialHash(symbol name, const signature *sig) {
  return name.hash() * 107 + hashSig(sig);
}
size_t nonSpecialHash(symbol name, const ty *t) {
  DEBUG_CACHE_ASSERT(t);
  return nonSpecialHash(name, t->getSignature());
}
size_t specialHash(symbol name, const ty *t) {
  DEBUG_CACHE_ASSERT(t);
  return name.hash() * 107 + t->hash();
}

varEntry *core_venv::storeNonSpecial(symbol name, varEntry *ent) {
  DEBUG_CACHE_ASSERT(name.notSpecial());
  DEBUG_CACHE_ASSERT(ent);
  DEBUG_CACHE_ASSERT(ent->getType());

  signature *sig = ent->getSignature();

  for (size_t i = nonSpecialHash(name, sig); ; ++i)
  {
    cell& b = cellByIndex(i);

    if (b.empty())
      return storeNew(b, name, ent);

    if (b.matches(name, sig))
      return b.replaceWith(name, ent);

    if (b.isATomb())
      return storeNonSpecialAfterTomb(i, name, ent);
  }
}

varEntry *core_venv::storeSpecial(symbol name, varEntry *ent) {
  DEBUG_CACHE_ASSERT(name.special());
  DEBUG_CACHE_ASSERT(ent);
  DEBUG_CACHE_ASSERT(ent->getType());

  ty *t = ent->getType();

  for (size_t i = specialHash(name, t); ; ++i)
  {
    cell& b = cellByIndex(i);

    if (b.empty())
      return storeNew(b, name, ent);

    if (b.matches(name, t))
      return b.replaceWith(name, ent);

    if (b.isATomb())
      return storeSpecialAfterTomb(i, name, ent);
  }
}

varEntry *core_venv::store(symbol name, varEntry *ent) {
  DEBUG_CACHE_ASSERT(ent);
  DEBUG_CACHE_ASSERT(ent->getType());

  return name.special() ? storeSpecial(name, ent) :
                          storeNonSpecial(name, ent);
}

varEntry *core_venv::lookupSpecial(symbol name, const ty *t) {
  DEBUG_CACHE_ASSERT(name.special());
  DEBUG_CACHE_ASSERT(t);

  for (size_t i = specialHash(name, t); ; ++i)
  {
    cell& b = cellByIndex(i);

    if (b.matches(name, t))
      return b.ent;

    if (b.empty())
      return 0;
  }
}

varEntry *core_venv::lookupNonSpecial(symbol name, const signature *sig) {
  DEBUG_CACHE_ASSERT(name.notSpecial());

  for (size_t i = nonSpecialHash(name, sig); ; ++i)
  {
    cell& b = cellByIndex(i);

    if (b.matches(name, sig))
      return b.ent;

    if (b.empty())
      return 0;
  }
}

varEntry *core_venv::lookup(symbol name, const ty *t) {
  DEBUG_CACHE_ASSERT(t);

  return name.special() ? lookupSpecial(name, t) :
                          lookupNonSpecial(name, t->getSignature());
}

void core_venv::removeNonSpecial(symbol name, const signature *sig) {
  DEBUG_CACHE_ASSERT(name.notSpecial());

  for (size_t i = nonSpecialHash(name, sig); ; ++i)
  {
    cell& b = cellByIndex(i);

    if (b.matches(name, sig)) {
      b.remove();
      --size;
      return;
    }

    DEBUG_CACHE_ASSERT(!b.empty());
  }
}

void core_venv::removeSpecial(symbol name, const ty *t) {
  DEBUG_CACHE_ASSERT(name.special());
  DEBUG_CACHE_ASSERT(t);

  for (size_t i = specialHash(name, t); ; ++i)
  {
    cell& b = cellByIndex(i);

    if (b.matches(name, t)) {
      b.remove();
      --size;
      return;
    }

    DEBUG_CACHE_ASSERT(!b.empty());
  }
}

void core_venv::remove(symbol name, const ty *t) {
  DEBUG_CACHE_ASSERT(t);

  if (name.special())
    removeSpecial(name, t);
  else
    removeNonSpecial(name, t->getSignature());
}


size_t numFormals(ty *t) {
  signature *sig = t->getSignature();
  return sig ? sig->getNumFormals() : 0;
}

void venv::checkName(symbol name)
{
#if 0
  // This size test is too slow, even for DEBUG_CACHE.
  core.confirm_size();
#endif

  // Get the type, and make it overloaded if it is not (for uniformity).
  overloaded o;
  ty *t = getType(name);
  if (!t)
    t = &o;
  if (!t->isOverloaded()) {
    o.add(t);
    t = &o;
  }
  assert(t->isOverloaded());

  size_t maxFormals = names[name].maxFormals;

  size_t size = 0;
  for (ty_iterator i = t->begin(); i != t->end(); ++i) {
    assert(numFormals(*i) <= maxFormals);
    varEntry *v = lookByType(name, *i);
    assert(v);
    assert(equivalent(v->getType(), *i));
    ++size;
  }

  size_t matches = 0;
  core_venv::const_iterator end = core.end();
  for (core_venv::const_iterator p = core.begin(); p != end; ++p) {
    if (p->name == name) {
      ++matches;

      varEntry *v=p->ent;
      assert(v);
      assert(equivalent(t, v->getType()));
    }
  }
  assert(matches == size);
}
    
void rightKind(ty *t) {
  if (t && t->isOverloaded()) {
    ty_vector& set=((overloaded *)t)->sub;
    assert(set.size() > 1);
  }
}

#ifdef DEBUG_CACHE
#define RIGHTKIND(t) (rightKind(t))
#define CHECKNAME(name) (checkName(name))
#else
#define RIGHTKIND(t) (void)(t)
#define CHECKNAME(name) (void)(name)
#endif

void venv::namevalue::addType(ty *s) {
  RIGHTKIND(t);

#ifdef DEBUG_CACHE
  assert(!s->isOverloaded());
#endif

  if (t == 0) {
    maxFormals = numFormals(s);
    t = s;
  } else {
    if (!t->isOverloaded())
      t = new overloaded(t);

#ifdef DEBUG_CACHE
    assert(t->isOverloaded());
    assert(!equivalent(t, s));
#endif

    ((overloaded *)t)->add(s);

    size_t n = numFormals(s);
    if (n > maxFormals)
      maxFormals = n;
  }

  RIGHTKIND(t);
}

void venv::namevalue::replaceType(ty *new_t, ty *old_t) {
#ifdef DEBUG_CACHE
  assert(t != 0);
  RIGHTKIND(t);
#endif

  // TODO: Test for equivalence.
  
  if (t->isOverloaded()) {
    for (ty_iterator i = t->begin(); i != t->end(); ++i) {
      if (equivalent(old_t, *i)) {
        *i = new_t;
        return;
      }
    }

    // An error, the type was not found.
    assert("unreachable code" == 0);

  } else {
#ifdef DEBUG_CACHE
    assert(equivalent(old_t, t));
#endif
    t = new_t;
  }

#ifdef DEBUG_CACHE
  assert(t != 0);
  RIGHTKIND(t);
#endif
}

#ifdef DEBUG_CACHE
void venv::namevalue::popType(ty *s)
#else
void venv::namevalue::popType()
#endif
{
#ifdef DEBUG_CACHE
  assert(t);
  RIGHTKIND(t);
  assert(!s->isOverloaded());
#endif

  if (t->isOverloaded()) {
    ty_vector& set=((overloaded *)t)->sub;

#ifdef DEBUG_CACHE
    assert(set.size() > 0);
    assert(equivalent(set.back(), s));
#endif

    // We are relying on the fact that this was the last type added to t, and
    // that type are added by pushing them on the end of the vector.
    set.pop_back();

    if (set.size() == 1)
      t = set.front();
  } else {
#ifdef DEBUG_CACHE
    assert(equivalent(t, s));
#endif
    t = 0;
  }

  RIGHTKIND(t);

  // Don't try to reduce numFormals as I doubt it is worth the cost of
  // recalculating. 
}

void venv::remove(const addition& a) {
  CHECKNAME(a.name);

  if (a.shadowed) {
    varEntry *popEnt = core.store(a.name, a.shadowed);

    DEBUG_CACHE_ASSERT(popEnt);

    // Unshadow the previously shadowed varEntry.
    names[a.name].replaceType(a.shadowed->getType(), popEnt->getType());
  }
  else {
    // Remove the (name,sig) key completely.
#if DEBUG_CACHE
    varEntry *popEnt = core.lookup(a.name, a.t);
    assert(popEnt);
    names[a.name].popType(popEnt->getType());
#else
    names[a.name].popType();
#endif

    core.remove(a.name, a.t);
  }

  CHECKNAME(a.name);
}

void venv::beginScope() {
  if (core.empty()) {
    assert(scopesizes.empty());
    ++empty_scopes;
  } else {
    scopesizes.push(additions.size());
  }
}

void venv::endScope() {
  if (scopesizes.empty()) {
    // The corresponding beginScope happened when the venv was empty, so
    // clear the hash tables to return to that state.
    core.clear();
    names.clear();

    assert(empty_scopes > 0);
    --empty_scopes;
  } else {
    size_t scopesize = scopesizes.top();
    assert(additions.size() >= scopesize);
    while (additions.size() > scopesize) {
      remove(additions.top());
      additions.pop();
    }
    scopesizes.pop();
  }
}

// Adds the definitions of the top-level scope to the level underneath,
// and then removes the top scope.
void venv::collapseScope() {
  if (scopesizes.empty()) {
    // Collapsing an empty scope.
    assert(empty_scopes > 0);
    --empty_scopes;
  } else {
    // As scopes are stored solely by the number of entries at the beginning
    // of the scope, popping the top size will put all of the entries into the
    // next scope down.
    scopesizes.pop();
  }
}


void venv::enter(symbol name, varEntry *v)
{
  CHECKNAME(name);

  // Store the new variable.  If it shadows an older variable, that varEntry
  // will be returned.
  varEntry *shadowed = core.store(name, v);

  ty *t = v->getType();

  // Record the addition, so it can be undone during endScope.
  if (!scopesizes.empty())
    additions.push(addition(name, t, shadowed));

  if (shadowed)
    // The new value shadows an old value.  They have the same signature, but
    // possibly different return types.  If necessary, update the type stored
    // by name.
    names[name].replaceType(t, shadowed->getType());
  else
    // Add to the names hash table.
    names[name].addType(t);

  CHECKNAME(name);
}


varEntry *venv::lookBySignature(symbol name, signature *sig) {
  // Rest arguments are complicated and rare.  Don't handle them here.
  if (sig->hasRest())
    return 0;

  // Likewise with the special operators.
  if (name.special())
    return 0;

  namevalue& nv = names[name];

  // Avoid ambiguities with default parameters.
  if (nv.maxFormals != sig->getNumFormals())
    return 0;

  // See if this exactly matches a function in the table.
  varEntry *ve = core.lookupNonSpecial(name, sig);
  
  if (!ve)
    return 0;

  // Keyword-only arguments may cause matching to fail.
  if (ve->getSignature()->numKeywordOnly > 0)
    return 0;

  // At this point, any function with an equivalent signature will be equal
  // to the result of the normal overloaded function resolution.  We may
  // safely return it.
  return ve;
}

void venv::add(venv& source, varEntry *qualifier, coder &c)
{
  core_venv::const_iterator end = source.core.end();
  for (core_venv::const_iterator p = source.core.begin(); p != end; ++p)
  {
    DEBUG_CACHE_ASSERT(p->filled());

    varEntry *v=p->ent;
    if (v->checkPerm(READ, c)) {
      enter(p->name, qualifyVarEntry(qualifier, v));
    }
  }
}

bool venv::add(symbol src, symbol dest,
               venv& source, varEntry *qualifier, coder &c)
{
  ty *t=source.getType(src);

  if (!t)
    return false;

  if (t->isOverloaded()) {
    bool added=false;
    for (ty_iterator i = t->begin(); i != t->end(); ++i)
    {
      varEntry *v=source.lookByType(src, *i);
      if (v->checkPerm(READ, c)) {
        enter(dest, qualifyVarEntry(qualifier, v));
        added=true;
      }
    }
    return added;
  }
  else {
    varEntry *v=source.lookByType(src, t);
    if (v->checkPerm(READ, c)) {
      enter(dest, qualifyVarEntry(qualifier, v));
      return true;
    }
    return false;
  }
}


ty *venv::getType(symbol name)
{
  return names[name].t;
}

void listValue(symbol name, varEntry *v, record *module)
{
  if (!module || v->whereDefined() == module)
  {
    if (settings::getSetting<bool>("where"))
      cout << v->getPos();

    v->getType()->printVar(cout, name);

    cout << ";\n";
  }
}

void venv::listValues(symbol name, record *module)
{
  ty *t=getType(name);

  if (t->isOverloaded())
    for (ty_iterator i = t->begin(); i != t->end(); ++i)
      listValue(name, lookByType(name, *i), module);
  else
    listValue(name, lookByType(name, t), module);

  flush(cout);
}

void venv::list(record *module)
{
  // List all functions and variables.
  for (namemap::iterator N = names.begin(); N != names.end(); ++N)
    listValues(N->first, module);
}

void venv::completions(mem::list<symbol >& l, string start)
{
  for(namemap::iterator N = names.begin(); N != names.end(); ++N)
    if (prefix(start, N->first) && N->second.t)
      l.push_back(N->first);
}

#endif /* not NOHASH */

} // namespace trans
