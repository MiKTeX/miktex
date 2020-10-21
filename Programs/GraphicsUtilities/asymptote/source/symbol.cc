/*****
 * symbol.cc
 * Andy Hammerlindl 2002/06/18
 *
 * Creates symbols from strings so that multiple calls for a symbol of
 * the same string will return an identical object.
 *****/

#include <cstring>
#include <cstdlib>

using std::strlen;


#include "settings.h"
#include "symbol.h"

namespace sym {

const char USED = 1;
const char SKIP = 2;

struct symbolRecord {
  // When a symbol is entered into the table, its hash is computed.  If the
  // corresponding entry in the table is full, this value is incremented until
  // an empty slot is found.  hashplus stores the end value.
  // Each symbol has a unique hashplus value, even if there is a collision in
  // the original hashing function.
  uint hashplus;

  // Whether the cell of the table is empty, in use, or a "skip" entry due to
  // a resizing of the table.
  unsigned char flag;

  // Pointer to a copy of the string (allocated on the heap).  This string
  // will never be deallocated.  Symbols, in essence, last forever.
  char *s;
};

// The table size must be a power of two so that (h % tableSize) can be
// replaced by (h & tableMask).  1 << 15 was chosen based on the number of
// unique symbols (roughly 4000) which occured in all of the base modules.
const size_t SYMBOL_TABLE_BASE_CAPACITY = 1 << 15;
symbolRecord baseSymbolTable[SYMBOL_TABLE_BASE_CAPACITY];

symbolRecord *table = baseSymbolTable;
size_t tableCapacity = SYMBOL_TABLE_BASE_CAPACITY;
uint tableMask = 0;
size_t tableSize = 0;

symbolRecord &recordByHashplus(uint h)
{
  return table[h & tableMask];
}

GCInit symbol::initialize;
symbol symbol::nullsym;
symbol symbol::initsym;
symbol symbol::castsym;
symbol symbol::ecastsym;

const char *nullsymstr = "<nullsym>";

void initTable() {
  tableMask = (uint)(tableCapacity - 1);
  tableSize = 0;

  // Set every entry to empty.  (Is this faster than memsetting the whole
  // thing?)
  for (size_t i = 0; i < tableCapacity; ++i)
    table[i].flag = 0;

  // The zeroth entry is reserved for the "null" symbol.
  if (table == baseSymbolTable) {
    table[0].flag = USED;
    table[0].s = new char[strlen(nullsymstr) + 1];
    strcpy(table[0].s, nullsymstr);
    ++tableSize;

    symbol::nullsym.hashplus = 0;

    symbol::initsym = symbol::opTrans("init");
    symbol::castsym = symbol::opTrans("cast");
    symbol::ecastsym = symbol::opTrans("ecast");
  }
}

// Hashing constants found experimentally to reduce collision (a little).
const uint A = 25191, B = 16342, C = 1746, D = 18326;

// Hash the string into an integer.  Experimental testing has shown that
// hashing only the first few letters seems to be faster than hashing deeper
// into the string, even though this approach causes more hash collisions.
uint hash(const char *s, size_t len)
{
  uint h = s[0];
  if (len == 2)
    return h;
  h += A*s[1];
  if (len == 3)
    return h;
  h += B*s[2];
  if (len == 4)
    return h;
  h += C*s[3];
  if (len == 5)
    return h;
  h += D*s[4];
  return h+len;
}

/* Under normal circumstances, the initial table should be large enough for
 * all of the symbols used and will never be resized.  Just in case the
 * program encounters a large number of distinct symbols, we implement
 * resizing of the table.
 */
void resizeTable() {
  symbolRecord *oldTable = table;
  size_t oldSize = tableSize;
  size_t oldCapacity = tableCapacity;

  tableCapacity *= 4;
  table = new symbolRecord[tableCapacity];
  initTable();

  // The null symbol is a special case.
  table[0] = oldTable[0];
  ++tableSize;

#if 0
  printf("old:\n");
  for (size_t i = 0; i < oldCapacity; ++i) {
    symbolRecord &r = oldTable[i];

    if (r.flag != USED)
      continue;

    printf("  %u -> %s\n", r.hashplus, r.s);
  }
#endif

  for (size_t i = 1; i < oldCapacity; ++i) {
    symbolRecord &r = oldTable[i];

    if (r.flag != USED)
      continue;

    // Entries that were skipped over when this symbol was entered into the
    // old hash table may not appear in the same spot in the new hash table.
    // Put "SKIP" entries in their place, so that the symbol will still be
    // found.
    for (uint h = hash(r.s, strlen(r.s)+1); h < r.hashplus; ++h) {
      symbolRecord &skipr = recordByHashplus(h);
      if (skipr.flag == 0)
        skipr.flag = SKIP;
    }

    // Enter the symbol in its spot.
    symbolRecord &newr = recordByHashplus(r.hashplus);
    assert(newr.flag != USED);
    newr.flag = USED;
    newr.hashplus = r.hashplus;
    newr.s = r.s;
    ++tableSize;
  }

#if 0
  printf("new:\n");
  for (size_t i = 0; i < tableCapacity; ++i) {
    symbolRecord &r = table[i];

    if (r.flag != USED)
      continue;

    printf("  %u -> %s\n", r.hashplus, r.s);
  }
#endif

  assert(tableSize == oldSize);

  // Debugging resize.
  for (size_t i = 1; i < oldCapacity; ++i) {
    symbolRecord &r = oldTable[i];

    if (r.flag != USED)
      continue;

    symbolRecord &newr = recordByHashplus(r.hashplus);
    assert(newr.hashplus == r.hashplus);
    assert(newr.flag != 0);
    assert(newr.flag != SKIP);
    assert(newr.flag == USED);
    assert(newr.s = r.s);
    if (strncmp(r.s, "gensym", 6) != 0)
      assert(symbol::rawTrans(r.s, strlen(r.s)+1).hashplus == r.hashplus);
  }

#if 0
  // Diagnostics.
  uint empty=0, used=0, skip=0;
  for (size_t i = 0; i < tableCapacity; ++i) {
    symbolRecord &r = table[i];
    if (r.flag == 0) ++empty;
    else if (r.flag == USED) ++used;
    else if (r.flag == SKIP) ++skip;
    else assert("Unknown flag" == 0);
  }

  cout << "Resized symbol table.  "
       << "empty: " << empty
       << "used: " << used
       << "skip: " << skip
       << endl;
#endif
}

symbol symbolize(uint h) {
  symbol s;
  s.hashplus = h;
  return s;
}

// Handles the insertion of a new symbol into a table the has been resized (or
// needs resizing).
symbol advancedInsert(const char *s, size_t len)
{
  if (2*tableSize >= tableCapacity)
    resizeTable();

  uint hashplus = hash(s, len);

#if 1
  assert(s != 0);
  assert(len > 0);
  assert(2*tableSize <= tableCapacity);
#endif

  // We know the symbol is not in the table.  Just search for the first unused
  // entry (either empty or a skip entry) and insert there.
  for (;;) {
    symbolRecord &r = recordByHashplus(hashplus);

    if (r.flag != USED) {
      r.flag = USED;
      r.s = new char[len];
      memcpy(r.s, s, len);
      assert(r.s[len-1] == '\0');
      r.hashplus = hashplus;

      ++tableSize;
      assert(2*tableSize <= tableCapacity);

      return symbolize(hashplus);
    }

    ++hashplus;
  }

  assert("Unreachable code" == 0);
  return symbol::nullsym;
}

symbol symbol::gensym(string s) {
  // Gensym can be inserted as if it were a normal string not already in the
  // table.  advancedInsert handles this.
  s = "gensym " + s;
  return advancedInsert(s.c_str(), s.size() + 1);
}

symbol symbol::rawTrans(const char *s, size_t len)
{
  uint hashplus = sym::hash(s, len);

#if 1
  assert(s != 0);
  assert(len > 0);
  assert(2*tableSize <= tableCapacity);
#endif

  // Search through the table till we find the symbol already translated or
  // an empty field.
  for (;;) {
    symbolRecord &r = recordByHashplus(hashplus);

    // Translating pre-existing symbols is more common, so check for it first.
    if (r.hashplus == hashplus &&
        r.flag == USED &&
        strncmp(r.s, s, len) == 0) {
      return symbolize(hashplus);
    }

    // Then check for an empty entry, in which case the entry is added.
    if (r.flag == 0) {
      // Test if the table needs resizing before entering a new symbol, or if
      // the table has already been resized.  In either case, the symbol will
      // be added to a resized table which may contain skip entries, and a
      // more involved insertion routine is needed.
      if (2*tableSize >= SYMBOL_TABLE_BASE_CAPACITY)
        return advancedInsert(s, len);

      r.flag = USED;
      r.s = new char[len];
      memcpy(r.s, s, len);
      assert(r.s[len-1] == '\0');
      r.hashplus = hashplus;

      ++tableSize;
      assert(2*tableSize <= tableCapacity);

      return symbolize(hashplus);
    }

    // A case where a different symbol is in the spot, continue along the
    // table.
    ++hashplus;
  }

  assert("Unreachable code" == 0);
  return symbol::nullsym;
}

symbol::operator string () const {
  symbolRecord &r = recordByHashplus(this->hashplus);
  return (string)r.s;
}

ostream& operator<< (ostream& out, const symbol sym)
{
  symbolRecord &r = recordByHashplus(sym.hashplus);
  return out << r.s;
}

} // end namespace sym

/* Define all of operator symbols SYM_PLUS, etc. */
#define OPSYMBOL(str, name)                     \
  sym::symbol name = sym::symbol::opTrans(str)
#include "opsymbols.h"
#undef OPSYMBOL

/* Define all of the symbols of the type SYM(name) in selected files. */
#define ADDSYMBOL(name)                                                 \
  sym::symbol PRETRANSLATED_SYMBOL_##name = sym::symbol::literalTrans(#name)
#include "allsymbols.h"
#undef ADDSYMBOL
