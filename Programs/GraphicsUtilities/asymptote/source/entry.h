/*****
 * entry.h
 * Andy Hammerlindl 2002/08/29
 *
 * All variables, built-in functions and user-defined functions reside
 * within the same namespace.  To keep track of all these, a table of
 * "entries" is used.
 *****/

#ifndef ENTRY_H
#define ENTRY_H

#include <iostream>

#include "common.h"
#include "frame.h"
#include "table.h"
#include "types.h"
#include "modifier.h"

using sym::symbol;
using types::ty;
using types::signature;

// Forward declaration.
namespace types {
class record;
}
using types::record;

namespace trans {

// An entry is associated to a name in the (variable or type) environment, and
// has permission based on the enclosing records where it was defined or
// imported.
class entry : public gc {
  struct pr {
    permission perm;
    record *r;

    pr(permission perm, record *r)
      : perm(perm), r(r) {}

    // Returns true if the permission allows access in this context.
    bool check(action act, coder &c);

    // Reports an error if permission is not allowed.
    void report(action act, position pos, coder &c);
  };
  
  mem::list<pr> perms;

  void addPerm(permission perm, record *r) {
    // Only store restrictive permissions.
    if (perm != PUBLIC && r)
      perms.push_back(pr(perm,r));
  }

  // The record where the variable or type is defined, or 0 if the entry is
  // not a field.
  record *where;

  // The location (file and line number) where the entry was defined.
  position pos;
  
public:
  entry(record *where, position pos) : where(where), pos(pos) {}
  entry(permission perm, record *r, record *where, position pos)
    : where(where), pos(pos) {
    addPerm(perm, r);
  }

  // (Non-destructively) merges two entries, appending permission lists.
  // The 'where' member is taken from the second entry.
  entry(entry &e1, entry &e2);
  
  // Create an entry with one more permission in the list.
  entry(entry &base, permission perm, record *r);

  bool checkPerm(action act, coder &c);
  void reportPerm(action act, position pos, coder &c);

  record *whereDefined() {
    return where;
  }
  
  position getPos() {
    return pos;
  }
};
    
class varEntry : public entry {
  ty *t;
  access *location;

public:
  varEntry(ty *t, access *location, record *where, position pos)
    : entry(where, pos), t(t), location(location) {}

  varEntry(ty *t, access *location, permission perm, record *r,
           record *where, position pos)
    : entry(perm, r, where, pos), t(t), location(location) {}

  // (Non-destructively) merges two varEntries, creating a qualified varEntry.
  varEntry(varEntry &qv, varEntry &v);

  ty *getType()
  { return t; }

  signature *getSignature()
  {
    return t->getSignature();
  }

  access *getLocation()
  { return location; }

  frame *getLevel();

  // Encodes the access, but also checks permissions.
  void encode(action act, position pos, coder &c);
  void encode(action act, position pos, coder &c, frame *top);
};

varEntry *qualifyVarEntry(varEntry *qv, varEntry *v);

// As looked-up types can be allocated in a new expression, we need to know
// what frame they should be allocated on.  Type entries store this extra
// information along with the type.
class tyEntry : public entry {
public:
  ty *t;
  varEntry *v;  // NOTE: Name isn't very descriptive.

  tyEntry(ty *t, varEntry *v, record *where, position pos)
    : entry(where, pos), t(t), v(v) {}

  tyEntry(tyEntry *base, permission perm, record *r)
    : entry(*base, perm, r), t(base->t), v(base->v) {}

  // Records need a varEntry that refers back to the qualifier qv; i.e. in
  // the last new of the code
  //   struct A {
  //     struct B {}
  //   }
  //   A a=new A;
  //   unravel a;
  //   new B;
  // we need to put a's frame on the stack before allocating an instance of B.
  // NOTE: A possible optimization could be to only qualify the varEntry if
  // the type is a record, as other types don't use the varEntry.
private:
  tyEntry(tyEntry *base, varEntry *qv)
    : entry(*base, *qv), t(base->t), v(qualifyVarEntry(qv, base->v)) {}

public:
  // Since the constructor can only be used when qv is non-null it is private
  // for safety reasons, and we provide this method instead.
  friend tyEntry *qualifyTyEntry(varEntry *qv, tyEntry *ent);
};

inline tyEntry *qualifyTyEntry(varEntry *qv, tyEntry *ent) {
  return qv ? new tyEntry(ent, qv) : ent;
}

// The type environment.
class tenv : public sym::table<tyEntry *> {
  bool add(symbol dest, names_t::value_type &x, varEntry *qualifier,
           coder &c);
public:
  // Add the entries in one environment to another, if qualifier is
  // non-null, it is a record and the source environment is its types.  The
  // coder is used to see which entries are accessible and should be added.
  void add(tenv& source, varEntry *qualifier, coder &c);

  // Adds entries of the name src in source as the name dest, returning true if
  // any were added.
  bool add(symbol src, symbol dest,
           tenv& source, varEntry *qualifier, coder &c);
};

#if 0 //{{{
 /* This version of venv is provided for compiling on systems which do not
  * have some form of STL hash table.  It will eventually be removed.
  * See the hash version below for documentation on the functions.
  */
/*NOHASH*/ class venv : public sym::table<varEntry*> {
/*NOHASH*/ public:
/*NOHASH*/   venv() {}
/*NOHASH*/ 
/*NOHASH*/   struct file_env_tag {};
/*NOHASH*/   venv(file_env_tag) {}
/*NOHASH*/ 
/*NOHASH*/   void add(venv& source, varEntry *qualifier, coder &c);
/*NOHASH*/ 
/*NOHASH*/   bool add(symbol src, symbol dest,
/*NOHASH*/            venv& source, varEntry *qualifier, coder &c);
/*NOHASH*/ 
/*NOHASH*/   varEntry *lookByType(symbol name, ty *t);
/*NOHASH*/ 
/*NOHASH*/   varEntry *lookBySignature(symbol name, signature *sig) {
/*NOHASH*/     // This optimization is not implemented for the NOHASH version.
/*NOHASH*/     return 0;
/*NOHASH*/   }
/*NOHASH*/ 
/*NOHASH*/   ty *getType(symbol name);
/*NOHASH*/ 
/*NOHASH*/   friend std::ostream& operator<< (std::ostream& out,
/*NOHASH*/                                    const venv& ve);
/*NOHASH*/   
/*NOHASH*/   void list(record *module=0);
/*NOHASH*/ };

//}}}
#else

// For speed reasons, many asserts are only tested when DEBUG_CACHE is set.
#ifdef DEBUG_CACHE
#define DEBUG_CACHE_ASSERT(x) assert(x)
#else
#define DEBUG_CACHE_ASSERT(x) (void)(x)
#endif

// The hash table which is at the core of the variable environment venv.
class core_venv : public gc {

public:
  // The cells of the table
  struct cell {
    symbol name;
    varEntry *ent;

    bool empty() const {
      return name == 0;
    }

    bool isATomb() const {
      DEBUG_CACHE_ASSERT(!empty());
      return ent == 0;
    }

    bool filled() const {
      return !empty() and !isATomb();
    }

    bool matches(symbol name, const ty *t) {
      DEBUG_CACHE_ASSERT(name.special());
      DEBUG_CACHE_ASSERT(t);

      if (this->name != name)
        return false;
      if (!this->ent)
        return false;
      return equivalent(this->ent->getType(), t);
    }

    bool matches(symbol name, const signature *sig) {
      DEBUG_CACHE_ASSERT(!name.special());

      if (this->name != name)
        return false;
      if (!this->ent)
        return false;
      return equivalent(this->ent->getSignature(), sig);
    }

    void storeNew(symbol name, varEntry *ent) {
      DEBUG_CACHE_ASSERT(empty() || isATomb());

      this->name = name;
      this->ent = ent;
    }

    varEntry *replaceWith(symbol name, varEntry *ent) {
      this->name = name;

      varEntry *old = this->ent;
      this->ent = ent;
      return old;
    }

    void remove() {
      this->ent = 0;
    }
  };

private:
  size_t capacity;
  size_t size;
  size_t mask;
  cell *table;

  void initTable(size_t capacity);

  void resize();

  cell& cellByIndex(size_t i);

  const cell& cellByIndex(size_t i) const;

  varEntry *storeNew(cell& cell, symbol name, varEntry *ent);

  varEntry *storeNonSpecialAfterTomb(size_t tombIndex,
                                     symbol name, varEntry *ent);
  varEntry *storeSpecialAfterTomb(size_t tombIndex,
                                  symbol name, varEntry *ent);

public:
  core_venv(size_t capacity) {
    initTable(capacity);
  }

  bool empty() const { return size == 0; }
  void clear();

  void confirm_size();

  // Store an entry into the table.  If this shadows a previous entry, the old
  // entry is returned, otherwise 0 is returned.
  varEntry *storeNonSpecial(symbol name, varEntry *ent);
  varEntry *storeSpecial(symbol name, varEntry *ent);
  varEntry *store(symbol name, varEntry *ent);

  // Lookup an entry in the table.
  varEntry *lookupNonSpecial(symbol name, const signature *sig);
  varEntry *lookupSpecial(symbol name, const ty *t);
  varEntry *lookup(symbol name, const ty *t);

  // Remove an entry from the table.
  void removeNonSpecial(symbol name, const signature *sig);
  void removeSpecial(symbol name, const ty *t);
  void remove(symbol name, const ty *t);

  // Features for iterating over the entire table.
  class const_iterator {
    const core_venv& core;
    size_t index;

  public:
    const_iterator(const core_venv& core, size_t index)
      : core(core), index(index) {}

    const cell& operator * () const {
      return core.cellByIndex(index);
    }
    const cell* operator -> () const {
      return &core.cellByIndex(index);
    }

    const_iterator& operator ++ () {
      // Advance to the next filled cell, or stop at the end of the array.
      do {
        ++index;
      } while (!(*this)->filled() && index < core.capacity);

      DEBUG_CACHE_ASSERT((*this)->filled() || (*this) == core.end());

      return *this;
    }

    friend bool operator == (const const_iterator& a, const const_iterator& b)
    {
      // For speed, we don't compare the hashtables.
      return a.index == b.index;
    }
    friend bool operator != (const const_iterator& a, const const_iterator& b)
    {
      // For speed, we don't compare the hashtables.
      return a.index != b.index;
    }
  };

  const_iterator begin() const {
    size_t index = 0;
    while (index < capacity && !cellByIndex(index).filled())
      ++index;
    return const_iterator(*this, index);
  }

  const_iterator end() const {
    return const_iterator(*this, capacity);
  }
};


// venv implemented with a hash table.
class venv {
  // A hash table used to quickly look up a variable once its name and type are
  // known.  Includes all scopes.
  core_venv core;

  // Record of added variables in the order they were added.
  struct addition {
    symbol name;
    ty *t;
    varEntry *shadowed;

    addition(symbol name, ty *t, varEntry *shadowed)
      : name(name), t(t), shadowed(shadowed) {}
  };
  typedef mem::stack<addition> addstack;
  addstack additions;

  // A scope can be recorded by the size of the addition stack at the time the
  // scope began.
  typedef mem::stack<size_t> scopestack;
  scopestack scopesizes;


  struct namehash {
    size_t operator()(const symbol name) const {
      return name.hash();
    }
  };
  struct nameeq {
    bool operator()(const symbol s, const symbol t) const {
      return s==t;
    }
  };

  struct namevalue {
    size_t maxFormals;
    ty *t;

    namevalue() : maxFormals(0), t(0) {}

    void addType(ty *s);
    
    void replaceType(ty *new_t, ty *old_t);

#if DEBUG_CACHE
    void popType(ty *tnew);
#else
    void popType();
#endif
  };

  // A dictionary indexed solely on the name, storing for each name the
  // current (possibly overloaded) type of the name.
  // The hash table implementation is slightly faster than the std::map binary
  // tree implementation, so we use it if we can.
#ifdef NOHASH
  typedef mem::map<symbol CONST, namevalue> namemap;
#else
  typedef mem::unordered_map<symbol, namevalue, namehash, nameeq> namemap;
#endif
  namemap names;


  // A sanity check.  For a given name, it checks that the type stored in the
  // names hash table exactly matches with all of the entries of that name
  // stored in the full hash table.
  void checkName(symbol name);

  void listValues(symbol name, record *module);


  // Helper function for endScope.
  void remove(const addition& a);

  // These are roughly the size the hashtables will be after loading the
  // builtin functions and plain module.
  static const size_t fileCoreSize=1 << 13;
  static const size_t fileNamesSize=1000;

  // The number of scopes begun (but not yet ended) when the venv was empty.
  size_t empty_scopes;
public:
  venv() :
    core(1 << 2), empty_scopes(0) {}

  // Most file level modules automatically import plain, so allocate hashtables
  // big enough to hold it in advance.
  struct file_env_tag {};
  venv(file_env_tag)
    : core(fileCoreSize),
#ifndef NOHASH
    names(fileNamesSize),
#endif
    empty_scopes(0) {}

  // Add a new variable definition.
  void enter(symbol name, varEntry *v);

  // Add the entries in one environment to another, if qualifier is
  // non-null, it is a record and entries of the source environment are its
  // fields.  The coder is necessary to check which variables are accessible and
  // should be added.
  void add(venv& source, varEntry *qualifier, coder &c);

  // Add all unshadowed variables from source of the name src as variables
  // named dest.  Returns true if at least one was added.
  bool add(symbol src, symbol dest,
           venv& source, varEntry *qualifier, coder &c);

  // Look for a function that exactly matches the type given.
  varEntry *lookByType(symbol name, ty *t) {
    return core.lookup(name, t);
  }

  // An optimization heuristic.  Try to guess the signature of a variable and
  // look it up.  This is allowed to return 0 even if the appropriate variable
  // exists.  If it returns a varEntry from an overloaded number of choices,
  // the returned function must be the one which would be called with
  // arguments given by sig, and its signature must be equivalent to sig.
  // For instance, in
  //   int f(int a, int b);
  //   int f(int a, int b, int c = 1);
  //   f(a,b);
  // looking up the signature of 'f' with arguments (int, int) must return 0
  // as there is an ambiguity.  The maxFormals field is used to ensure we
  // avoid such ambiguities.
  varEntry *lookBySignature(symbol name, signature *sig);

  // Get the (possibly overloaded) type of all variables associated to a
  // particular name.
  ty *getType(symbol name);

  void beginScope();
  void endScope();
  
  // Merges the top-level scope with the level immediately underneath it.
  void collapseScope();

  // Prints a list of the variables to the standard output.
  void list(record *module=0);

  // Adds to l, all names prefixed by start.
  void completions(mem::list<symbol>& l, string start);
};
#endif

} // namespace trans

#endif //ENTRY_H
