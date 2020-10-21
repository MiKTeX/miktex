/*****
 * array.h
 * Tom Prince 2005/06/18
 *
 * Array type used by virtual machine.
 *****/

#ifndef ARRAY_H
#define ARRAY_H

#include "vm.h"
#include "common.h"
#include "item.h"

namespace vm {

extern const char *dereferenceNullArray;

// Arrays are vectors with push and pop functions.
class array : public mem::vector<item> {
  bool cycle;

  void setNonBridgingSlice(size_t l, size_t r, mem::vector<item> *a);
  void setBridgingSlice(size_t l, size_t r, mem::vector<item> *a);
public:
  array() : cycle(false) {}

  array(size_t n)
    : mem::vector<item>(n), cycle(false)
  {}

  array(size_t n, item i, size_t depth);

  void push(item i)
  {
    push_back(i);
  }

  item pop()
  {
    item i=back();
    pop_back();
    return i;
  }

  template <typename T>
  T read(size_t i) const
  {
    return get<T>((*this)[i]);
  }

  array *slice(Int left, Int right);
  void setSlice(Int left, Int right, array *a);

  void cyclic(bool b) {
    cycle=b;
  }

  bool cyclic() const {
    return cycle;
  }

  array *copyToDepth(size_t depth);
};

template <typename T>
inline T read(const array *a, size_t i)
{
  return a->array::read<T>(i);
}

template <typename T>
inline T read(const array &a, size_t i)
{
  return a.array::read<T>(i);
}

inline size_t checkArray(const vm::array *a)
{
  if(a == 0) vm::error(dereferenceNullArray);
  return a->size();
}

inline void checkEqual(size_t i, size_t j) {
  if(i == j) return;
  ostringstream buf;
  buf << "operation attempted on arrays of different lengths: "
      << i << " != " << j;
  vm::error(buf);
}

inline size_t checkArrays(const vm::array *a, const vm::array *b)
{
  size_t asize=checkArray(a);
  size_t bsize=checkArray(b);
  checkEqual(asize,bsize);
  return asize;
}

// Copies an item to a depth d.  If d == 0 then the item is just returned
// without copying, otherwise, the array and its subarrays are copied to
// depth d.
item copyItemToDepth(item i, size_t depth);

} // namespace vm

#endif // ARRAY_H
