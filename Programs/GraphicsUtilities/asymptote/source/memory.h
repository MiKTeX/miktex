/****
 * memory.h
 *
 * Interface to the Boehm Garbage Collector.
 *****/

#ifndef MEMORY_H
#define MEMORY_H

#include <list>
#include <vector>
#include <stack>
#include <map>
#include <deque>
#include <string>
#include <sstream>
#include <unordered_map>

#ifdef USEGC

#define GC_THREADS
#ifdef __clang__
#define GC_ATTR_EXPLICIT
#define GC_NOEXCEPT
#endif
#include <gc.h>

#ifdef GC_DEBUG
extern "C" {
#include <gc_backptr.h>
}
#endif

void* asy_malloc(size_t n);
void* asy_malloc_atomic(size_t n);

#undef GC_MALLOC
#undef GC_MALLOC_ATOMIC

#define GC_MALLOC(sz) asy_malloc(sz)
#define GC_MALLOC_ATOMIC(sz) asy_malloc_atomic(sz)

#include <gc_allocator.h>
#include <gc_cpp.h>

#define gc_allocator gc_allocator_ignore_off_page

#else // USEGC

using std::allocator;
#define gc_allocator allocator

class gc {};
class gc_cleanup {};

enum GCPlacement {UseGC, NoGC, PointerFreeGC};

inline void* operator new(size_t size, GCPlacement) {
  return operator new(size);
}

inline void* operator new[](size_t size, GCPlacement) {
                           return operator new(size);
                         }

template<class T>
struct GC_type_traits {};

#define GC_DECLARE_PTRFREE(T)                   \
  template<> struct GC_type_traits<T> {}

#endif // USEGC

namespace mem {

#define GC_CONTAINER(KIND)                                              \
  template <typename T>                                                 \
  struct KIND : public std::KIND<T, gc_allocator<T> >, public gc {      \
    KIND() : std::KIND<T, gc_allocator<T> >() {}                        \
    KIND(size_t n) : std::KIND<T, gc_allocator<T> >(n) {}               \
    KIND(size_t n, const T& t) : std::KIND<T, gc_allocator<T> >(n,t) {} \
  }

GC_CONTAINER(list);
GC_CONTAINER(vector);
GC_CONTAINER(deque);

template <typename T, typename Container = vector<T> >
struct stack : public std::stack<T, Container>, public gc {};

template <typename T, typename S >
struct pair : public std::pair<T, S>, public gc {
  pair(T t, S s) : std::pair<T,S>(t,s) {}
};

#define PAIR_ALLOC gc_allocator<std::pair<const Key,T> > /* space */

#undef GC_CONTAINER

#define GC_CONTAINER(KIND)                                              \
  template <typename Key,                                               \
            typename T,                                                 \
            typename Compare = std::less<Key> >                         \
  struct KIND : public std::KIND<Key,T,Compare,PAIR_ALLOC>, public gc   \
  {                                                                     \
    KIND() : std::KIND<Key,T,Compare,PAIR_ALLOC> () {}                  \
  }

GC_CONTAINER(map);
GC_CONTAINER(multimap);

#undef GC_CONTAINER

#define GC_CONTAINER(KIND)                              \
  template <typename Key, typename T,                   \
            typename Hash = std::hash<Key>,             \
            typename Eq = std::equal_to<Key> >          \
  struct KIND : public                                  \
  std::KIND<Key,T,Hash,Eq,PAIR_ALLOC>, public gc {      \
    KIND() : std::KIND<Key,T,Hash,Eq,PAIR_ALLOC> () {}  \
    KIND(size_t n)                                      \
      : std::KIND<Key,T,Hash,Eq,PAIR_ALLOC> (n) {}      \
  }

GC_CONTAINER(unordered_map);
GC_CONTAINER(unordered_multimap);

#undef GC_CONTAINER

#undef PAIR_ALLOC

#ifdef USEGC
typedef std::basic_string<char,std::char_traits<char>,
                          gc_allocator<char> > string;
typedef std::basic_stringstream<char,std::char_traits<char>,
                                gc_allocator<char> > stringstream;
typedef std::basic_istringstream<char,std::char_traits<char>,
                                 gc_allocator<char> > istringstream;
typedef std::basic_ostringstream<char,std::char_traits<char>,
                                 gc_allocator<char> > ostringstream;
typedef std::basic_stringbuf<char,std::char_traits<char>,
                             gc_allocator<char> > stringbuf;

void compact(int x);
#else
inline void compact(int) {}
typedef std::string string;
typedef std::stringstream stringstream;
typedef std::istringstream istringstream;
typedef std::ostringstream ostringstream;
typedef std::stringbuf stringbuf;
#endif // USEGC

std::string stdString(string s);

} // namespace mem

#endif
