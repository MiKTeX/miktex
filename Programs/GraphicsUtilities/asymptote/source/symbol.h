/*****
 * symbol.h
 * Andy Hammerlindl 2002/06/18
 *
 * Creates symbols from strings so that multiple calls for a symbol of
 * the same string will return an identical object.
 *****/

#ifndef SYMBOL_H
#define SYMBOL_H

#include <iostream>
#include <cassert>

#include "common.h"

using std::ostream;

namespace sym {

void initTable();

struct GCInit {
#ifdef _AIX
  typedef char * GC_PTR;
#endif
  GCInit() {
#ifdef USEGC
    GC_set_free_space_divisor(2);
    mem::compact(0);
    GC_INIT();
#ifdef HAVE_PTHREAD
    GC_allow_register_threads();
#endif
#endif

    // Put the symbol table into a state where symbols can be translated.
    initTable();
  }
};

typedef unsigned int uint;

/* The symbol class, just a wrapper around the augmented hash value.  This
 * wrapper is so that
 *   cout << s << endl;
 * prints the symbol name instead of a meaningless integer.
 *
 * This is a lightweight class and should have no virtual functions for speed
 * reasons.
 */
struct symbol {
  // Is there any particular reason why this is in symbol?
  static GCInit initialize;

  uint hashplus;
#if 0
  symbol() {}
  symbol(uint h) : hashplus(h) {}
#endif

  static symbol nullsym;
  static symbol initsym;
  static symbol castsym;
  static symbol ecastsym;

  bool special() const {
    return *this == initsym || *this == castsym || *this == ecastsym;
  }
  bool notSpecial() const {
    return !special();
  }

  // Translate a string into a unique symbol, such that two strings are equal
  // if and only if their resulting symbols are equal.
  // len should be equal to strlen(s)+1
  static symbol rawTrans(const char *s, size_t len);

  static symbol literalTrans(string s) {
    return rawTrans(s.c_str(), s.size() + 1);
  }

  static symbol opTrans(string s) {
    return literalTrans("operator "+s);
  }

  static symbol trans(string s) {
    // Figure out whether it's an operator or an identifier by looking at the
    // first character.
    char c=s[0];
    return isalpha(c) || c == '_' ? literalTrans(s) : opTrans(s);
  }

  // Make a symbol that is guaranteed to be unique.  It will not match any other
  // symbol in the namespace.
  static symbol gensym(string s);

  size_t hash() const {
    return (size_t)this->hashplus;
  }

  friend bool operator== (symbol s1, symbol s2) {
    return s1.hashplus == s2.hashplus;
  }

  friend bool operator!= (symbol s1, symbol s2) {
    return s1.hashplus != s2.hashplus;
  }

  friend bool operator< (symbol s1, symbol s2) {
    return s1.hashplus < s2.hashplus;
  }

  operator bool () const { return this->hashplus != 0; }

  operator string () const;

#ifdef USEGC
  explicit operator std::string() const;
#endif

  friend ostream& operator<< (ostream& out, const symbol sym);
};

} // end namespace

#endif // SYMBOL_H
