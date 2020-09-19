#ifndef LCDF_VECTOR_HH
#define LCDF_VECTOR_HH
#include <assert.h>
#include <stdlib.h>
#ifdef HAVE_NEW_HDR
# include <new>
#elif defined(HAVE_NEW_H)
# include <new.h>
#else
static inline void *operator new(size_t, void *v) { return v; }
#endif
#if HAVE_VALGRIND && HAVE_VALGRIND_MEMCHECK_H
# include <valgrind/memcheck.h>
#endif

template <class T>
class Vector { public:

    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef const T& const_access_type;

    typedef int size_type;
    enum { RESERVE_GROW = (size_type) -1 };

    typedef T* iterator;
    typedef const T* const_iterator;

    explicit Vector()
	: _l(0), _n(0), _capacity(0) {
    }
    explicit Vector(size_type n, const T &e)
	: _l(0), _n(0), _capacity(0) {
	resize(n, e);
    }
    // template <class In> ...
    Vector(const Vector<T> &x);
    ~Vector();

    Vector<T>& operator=(const Vector<T>&);
    Vector<T>& assign(size_type n, const T& e = T());
    // template <class In> ...

    // iterators
    iterator begin()			{ return _l; }
    const_iterator begin() const	{ return _l; }
    iterator end()			{ return _l + _n; }
    const_iterator end() const		{ return _l + _n; }

    // capacity
    size_type size() const		{ return _n; }
    void resize(size_type nn, const T& e = T());
    size_type capacity() const		{ return _capacity; }
    bool empty() const			{ return _n == 0; }
    bool reserve(size_type n)		{ return reserve_and_push_back(n, 0); }

    // element access
    T &operator[](size_type i) {
	assert((unsigned) i < (unsigned) _n);
	return _l[i];
    }
    const T &operator[](size_type i) const {
	assert((unsigned) i < (unsigned) _n);
	return _l[i];
    }
    T& at(size_type i)			{ return operator[](i); }
    const T& at(size_type i) const	{ return operator[](i); }
    T& front()				{ return operator[](0); }
    const T& front() const		{ return operator[](0); }
    T& back()				{ return operator[](_n - 1); }
    const T& back() const		{ return operator[](_n - 1); }
    T& at_u(size_type i)		{ return _l[i]; }
    const T& at_u(size_type i) const	{ return _l[i]; }

    // modifiers
    inline void push_back(const T& x);
    inline void pop_back();
    inline void push_front(const T& x);
    inline void pop_front();
    iterator insert(iterator it, const T& x);
    inline iterator erase(iterator it);
    iterator erase(iterator a, iterator b);
    void swap(Vector<T>& x);
    void clear()			{ erase(begin(), end()); }

 private:

    T *_l;
    size_type _n;
    size_type _capacity;

    void *velt(size_type i) const		{ return (void *)&_l[i]; }
    static void *velt(T *l, size_type i)	{ return (void *)&l[i]; }
    bool reserve_and_push_back(size_type n, const T *x);

};

template <class T> inline void
Vector<T>::push_back(const T& x)
{
    if (_n < _capacity) {
#ifdef VALGRIND_MAKE_MEM_UNDEFINED
	VALGRIND_MAKE_MEM_UNDEFINED(velt(_n), sizeof(T));
#endif
	new(velt(_n)) T(x);
	++_n;
    } else
	reserve_and_push_back(RESERVE_GROW, &x);
}

template <class T> inline void
Vector<T>::pop_back()
{
    assert(_n > 0);
    --_n;
    _l[_n].~T();
#ifdef VALGRIND_MAKE_MEM_NOACCESS
    VALGRIND_MAKE_MEM_NOACCESS(&_l[_n], sizeof(T));
#endif
}

template <class T> inline typename Vector<T>::iterator
Vector<T>::erase(iterator it)
{
    return (it < end() ? erase(it, it + 1) : it);
}

template <class T> inline void
Vector<T>::push_front(const T& x)
{
    insert(begin(), x);
}

template <class T> inline void
Vector<T>::pop_front()
{
    erase(begin());
}


template <>
class Vector<void*> { public:

    typedef void* value_type;
    typedef void*& reference;
    typedef void* const& const_reference;
    typedef void** pointer;
    typedef void* const* const_pointer;
    typedef void* const_access_type;

    typedef int size_type;
    enum { RESERVE_GROW = (size_type) -1 };

    typedef void** iterator;
    typedef void* const* const_iterator;

    explicit Vector()
	: _l(0), _n(0), _capacity(0) {
    }
    explicit Vector(size_type n, void* e)
	: _l(0), _n(0), _capacity(0) {
	resize(n, e);
    }
    Vector(const Vector<void*> &);
    ~Vector();

    Vector<void*> &operator=(const Vector<void*> &);
    Vector<void*> &assign(size_type n, void* x = 0);

    // iterators
    iterator begin()			{ return _l; }
    const_iterator begin() const	{ return _l; }
    iterator end()			{ return _l + _n; }
    const_iterator end() const		{ return _l + _n; }

    // capacity
    size_type size() const		{ return _n; }
    void resize(size_type n, void* x = 0);
    size_type capacity() const		{ return _capacity; }
    bool empty() const			{ return _n == 0; }
    bool reserve(size_type n);

    // element access
    void*& operator[](size_type i)	{ assert(i>=0 && i<_n); return _l[i]; }
    void* operator[](size_type i) const	{ assert(i>=0 && i<_n); return _l[i]; }
    void*& at(size_type i)		{ return operator[](i); }
    void* at(size_type i) const		{ return operator[](i); }
    void*& front()			{ return operator[](0); }
    void* front() const			{ return operator[](0); }
    void*& back()			{ return operator[](_n - 1); }
    void* back() const			{ return operator[](_n - 1); }
    void*& at_u(size_type i)		{ return _l[i]; }
    void* at_u(size_type i) const	{ return _l[i]; }

    // modifiers
    inline void push_back(void* x);
    inline void pop_back();
    inline void push_front(void* x);
    inline void pop_front();
    iterator insert(iterator it, void* x);
    inline iterator erase(iterator it);
    iterator erase(iterator a, iterator b);
    void swap(Vector<void*> &x);
    void clear()			{ _n = 0; }

 private:

    void **_l;
    size_type _n;
    size_type _capacity;

};

inline void
Vector<void*>::push_back(void* x)
{
    if (_n < _capacity || reserve(RESERVE_GROW)) {
	_l[_n] = x;
	_n++;
    }
}

inline void
Vector<void*>::pop_back()
{
    assert(_n > 0);
    --_n;
#ifdef VALGRIND_MAKE_MEM_NOACCESS
    VALGRIND_MAKE_MEM_NOACCESS(&_l[_n], sizeof(void *));
#endif
}

inline Vector<void*>::iterator
Vector<void*>::erase(Vector<void*>::iterator it)
{
    return (it < end() ? erase(it, it + 1) : it);
}

inline void
Vector<void*>::push_front(void* x)
{
    insert(begin(), x);
}

inline void
Vector<void*>::pop_front()
{
    erase(begin());
}


template <class T>
class Vector<T*>: private Vector<void*> {

    typedef Vector<void*> Base;

 public:

    typedef T* value_type;
    typedef T*& reference;
    typedef T* const& const_reference;
    typedef T** pointer;
    typedef T* const* const_pointer;
    typedef T* const_access_type;

    typedef int size_type;
    enum { RESERVE_GROW = Base::RESERVE_GROW };

    typedef T** iterator;
    typedef T* const* const_iterator;

    explicit Vector()			: Base() { }
    explicit Vector(size_type n, T* x)	: Base(n, (void *)x) { }
    Vector(const Vector<T*>& x)		: Base(x) { }
    ~Vector()				{ }

    Vector<T*>& operator=(const Vector<T*>& x)
		{ Base::operator=(x); return *this; }
    Vector<T*>& assign(size_type n, T* x = 0)
		{ Base::assign(n, (void*)x); return *this; }

    // iterators
    const_iterator begin() const { return (const_iterator)(Base::begin()); }
    iterator begin()		{ return (iterator)(Base::begin()); }
    const_iterator end() const	{ return (const_iterator)(Base::end()); }
    iterator end()		{ return (iterator)(Base::end()); }

    // capacity
    size_type size() const	{ return Base::size(); }
    void resize(size_type n, T* x = 0) { Base::resize(n, (void*)x); }
    size_type capacity() const	{ return Base::capacity(); }
    bool empty() const		{ return Base::empty(); }
    bool reserve(size_type n)	{ return Base::reserve(n); }

    // element access
    T*& operator[](size_type i)	{ return (T*&)(Base::at(i)); }
    T* operator[](size_type i) const { return (T*)(Base::operator[](i)); }
    T*& at(size_type i)		{ return (T*&)(Base::operator[](i)); }
    T* at(size_type i) const	{ return (T*)(Base::at(i)); }
    T*& front()			{ return (T*&)(Base::front()); }
    T* front() const		{ return (T*)(Base::front()); }
    T*& back()			{ return (T*&)(Base::back()); }
    T* back() const		{ return (T*)(Base::back()); }
    T*& at_u(size_type i)	{ return (T*&)(Base::at_u(i)); }
    T* at_u(size_type i) const	{ return (T*)(Base::at_u(i)); }

    // modifiers
    void push_back(T* x)	{ Base::push_back((void*)x); }
    void pop_back()		{ Base::pop_back(); }
    void push_front(T* x)	{ Base::push_front((void*)x); }
    void pop_front()		{ Base::pop_front(); }
    iterator insert(iterator it, T* x) { return (iterator)Base::insert((void**)it, (void*)x); }
    iterator erase(iterator it)	{ return (iterator)Base::erase((void**)it); }
    iterator erase(iterator a, iterator b) { return (iterator)Base::erase((void**)a, (void**)b); }
    void swap(Vector<T*>& x)	{ Base::swap(x); }
    void clear()		{ Base::clear(); }

};

#include <lcdf/vector.cc>
#endif
