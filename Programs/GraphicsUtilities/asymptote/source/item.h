/*****
 * item.h
 * Tom Prince and John Bowman 2005/04/12
 *
 * Descibes the items that are used by the virtual machine.
 *****/

#ifndef ITEM_H
#define ITEM_H

#include "common.h"
#include <cfloat>
#include <cmath>

#if COMPACT
#include <cassert>
#else
#include <typeinfo>
#endif

namespace vm {

class item;
class bad_item_value {};

template<typename T>
T get(const item&);

#if COMPACT
// Identify a default argument.
extern const Int DefaultValue;

// Identify an undefined item.
extern const Int Undefined;

// Values for true and false unlikely to match other values.
extern const Int BoolTruthValue;
extern const Int BoolFalseValue;

inline Int valueFromBool(bool b) {
  return b ? BoolTruthValue : BoolFalseValue;
}
#endif

extern const item Default;

class item : public gc {
private:

#if !COMPACT
  const std::type_info *kind;
#endif

  union {
    Int i;
    double x;
#if !COMPACT
    bool b;
#endif
    void *p;
  };

public:
#if COMPACT
  bool empty() const
  {return i >= Undefined;}

  item() : i(Undefined) {}

  item(Int i)
    : i(i) {}
  item(int i)
    : i(i) {}
  item(double x)
    : x(x) {}
  item(bool b)
    : i(valueFromBool(b)) {}

  item& operator= (int a)
  { i=a; return *this; }
  item& operator= (unsigned int a)
  { i=a; return *this; }
  item& operator= (Int a)
  { i=a; return *this; }
  item& operator= (double a)
  { x=a; return *this; }
  item& operator= (bool b)
  { i=valueFromBool(b); return *this; }

  template<class T>
  item(T *p)
    : p((void *) p) {
    assert(!empty());
  }

  template<class T>
  item(const T &p)
    : p(new(UseGC) T(p)) {
    assert(!empty());
  }

  template<class T>
  item& operator= (T *a)
  { p=(void *) a; return *this; }

  template<class T>
  item& operator= (const T &it)
  { p=new(UseGC) T(it); return *this; }
#else
  bool empty() const
  {return *kind == typeid(void);}

  item()
    : kind(&typeid(void)) {}

  item(Int i)
    : kind(&typeid(Int)), i(i) {}
  item(int i)
    : kind(&typeid(Int)), i(i) {}
  item(double x)
    : kind(&typeid(double)), x(x) {}
  item(bool b)
    : kind(&typeid(bool)), b(b) {}

  item& operator= (int a)
  { kind=&typeid(Int); i=a; return *this; }
  item& operator= (unsigned int a)
  { kind=&typeid(Int); i=a; return *this; }
  item& operator= (Int a)
  { kind=&typeid(Int); i=a; return *this; }
  item& operator= (double a)
  { kind=&typeid(double); x=a; return *this; }
  item& operator= (bool a)
  { kind=&typeid(bool); b=a; return *this; }

  template<class T>
  item(T *p)
    : kind(&typeid(T)), p((void *) p) {}

  template<class T>
  item(const T &p)
    : kind(&typeid(T)), p(new(UseGC) T(p)) {}

  template<class T>
  item& operator= (T *a)
  { kind=&typeid(T); p=(void *) a; return *this; }

  template<class T>
  item& operator= (const T &it)
  { kind=&typeid(T); p=new(UseGC) T(it); return *this; }

  const std::type_info &type() const
  { return *kind; }
#endif

  template<typename T>
  friend inline T get(const item&);

  friend inline bool isdefault(const item&);

  friend ostream& operator<< (ostream& out, const item& i);

private:
  template <typename T>
  struct help;

  template <typename T>
  struct help<T*> {
    static T* unwrap(const item& it)
    {
#if COMPACT
      if(!it.empty())
        return (T*) it.p;
#else
      if(*it.kind == typeid(T))
        return (T*) it.p;
#endif
      throw vm::bad_item_value();
    }
  };

  template <typename T>
  struct help {
    static T& unwrap(const item& it)
    {
#if COMPACT
      if(!it.empty())
        return *(T*) it.p;
#else
      if(*it.kind == typeid(T))
        return *(T*) it.p;
#endif
      throw vm::bad_item_value();
    }
  };
};

#ifdef SIMPLE_FRAME
// In the simple implementation, a frame is just an array of items.
typedef item frame;
#else
class frame : public gc {
#ifdef DEBUG_FRAME
  string name;
  Int parentIndex;
#endif
  typedef mem::vector<item> internal_vars_t;
  internal_vars_t vars;

  // Allow the stack direct access to vars.
  friend class stack;
public:
#ifdef DEBUG_FRAME
  frame(string name, Int parentIndex, size_t size)
    : name(name), parentIndex(parentIndex), vars(size)
  {}

  string getName() { return name; }

  Int getParentIndex() { return parentIndex; }
#else
  frame(size_t size)
    : vars(size)
  {}
#endif

  item& operator[] (size_t n)
  { return vars[n]; }
  item operator[] (size_t n) const
  { return vars[n]; }

  size_t size()
  { return vars.size(); }

  // Extends vars to ensure it has a place for any variable indexed up to n.
  void extend(size_t n) {
    if(vars.size() < n)
      vars.resize(n);
  }
};
#endif


template<typename T>
inline T get(const item& it)
{
  return item::help<T>::unwrap(it);
}

template <>
inline int get<int>(const item&)
{
  throw vm::bad_item_value();
}

template <>
inline Int get<Int>(const item& it)
{
#if COMPACT
  if(!it.empty())
    return it.i;
#else
  if(*it.kind == typeid(Int))
    return it.i;
#endif
  throw vm::bad_item_value();
}

template <>
inline double get<double>(const item& it)
{
#if COMPACT
  if(!it.empty())
    return it.x;
#else
  if(*it.kind == typeid(double))
    return it.x;
#endif
  throw vm::bad_item_value();
}

template <>
inline bool get<bool>(const item& it)
{
#if COMPACT
  if(it.i == BoolTruthValue)
    return true;
  if(it.i == BoolFalseValue)
    return false;
#else
  if(*it.kind == typeid(bool))
    return it.b;
#endif
  throw vm::bad_item_value();
}

#if !COMPACT
// This serves as the object for representing a default argument.
struct default_t : public gc {};
#endif

inline bool isdefault(const item& it)
{
#if COMPACT
  return it.i == DefaultValue;
#else
  return *it.kind == typeid(default_t);
#endif
}

ostream& operator<< (ostream& out, const item& i);

} // namespace vm

#endif // ITEM_H
