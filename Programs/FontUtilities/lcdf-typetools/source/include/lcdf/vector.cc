#ifndef LCDF_VECTOR_CC
#define LCDF_VECTOR_CC

/*
 * vector.{cc,hh} -- simple array template class
 * Eddie Kohler
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 * Copyright (c) 2001-2003 International Computer Science Institute
 * Copyright (c) 1999-2019 Eddie Kohler
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

/* #include <lcdf/vector.hh> */

template <class T>
Vector<T>::Vector(const Vector<T> &x)
    : _l(0), _n(0), _capacity(0)
{
    *this = x;
}

template <class T>
Vector<T>::~Vector()
{
    for (size_type i = 0; i < _n; i++)
	_l[i].~T();
    delete[] (unsigned char *)_l;
}

template <class T> Vector<T> &
Vector<T>::operator=(const Vector<T> &o)
{
    if (&o != this) {
	for (size_type i = 0; i < _n; i++)
	    _l[i].~T();
#ifdef VALGRIND_MAKE_MEM_NOACCESS
	if (_l && _n)
	    VALGRIND_MAKE_MEM_NOACCESS(_l, _n * sizeof(T));
#endif
	_n = 0;
	if (reserve(o._n)) {
	    _n = o._n;
#ifdef VALGRIND_MAKE_MEM_UNDEFINED
	    if (_l && _n)
		VALGRIND_MAKE_MEM_UNDEFINED(_l, _n * sizeof(T));
#endif
	    for (size_type i = 0; i < _n; i++)
		new(velt(i)) T(o._l[i]);
	}
    }
    return *this;
}

template <class T> Vector<T> &
Vector<T>::assign(size_type n, const T &x)
{
    if (&x >= begin() && &x < end()) {
	T x_copy(x);
	return assign(n, x_copy);
    } else {
	resize(0, x);
	resize(n, x);
	return *this;
    }
}

template <class T> typename Vector<T>::iterator
Vector<T>::insert(iterator it, const T &x)
{
    assert(it >= begin() && it <= end());
    if (&x >= begin() && &x < end()) {
	T x_copy(x);
	return insert(it, x_copy);
    }
    if (_n == _capacity) {
	size_type pos = it - begin();
	if (!reserve(RESERVE_GROW))
	    return end();
	it = begin() + pos;
    }
#ifdef VALGRIND_MAKE_MEM_UNDEFINED
    VALGRIND_MAKE_MEM_UNDEFINED(velt(_n), sizeof(T));
#endif
    for (iterator j = end(); j > it; ) {
	--j;
	new((void*) (j + 1)) T(*j);
	j->~T();
#ifdef VALGRIND_MAKE_MEM_UNDEFINED
	VALGRIND_MAKE_MEM_UNDEFINED(j, sizeof(T));
#endif
    }
    new((void*) it) T(x);
    _n++;
    return it;
}

template <class T> typename Vector<T>::iterator
Vector<T>::erase(iterator a, iterator b)
{
    if (b > a) {
	assert(a >= begin() && b <= end());
	iterator i = a, j = b;
	for (; j < end(); i++, j++) {
	    i->~T();
#ifdef VALGRIND_MAKE_MEM_UNDEFINED
	    VALGRIND_MAKE_MEM_UNDEFINED(i, sizeof(T));
#endif
	    new((void*) i) T(*j);
	}
	for (; i < end(); i++)
	    i->~T();
	_n -= b - a;
#ifdef VALGRIND_MAKE_MEM_NOACCESS
	VALGRIND_MAKE_MEM_NOACCESS(_l + _n, (b - a) * sizeof(T));
#endif
	return a;
    } else
	return b;
}

template <class T> bool
Vector<T>::reserve_and_push_back(size_type want, const T *push_x)
{
    if (push_x && push_x >= begin() && push_x < end()) {
	T x_copy(*push_x);
	return reserve_and_push_back(want, &x_copy);
    }

    if (want < 0)
	want = (_capacity > 0 ? _capacity * 2 : 4);

    if (want > _capacity) {
	T *new_l = (T *)new unsigned char[sizeof(T) * want];
	if (!new_l)
	    return false;
#ifdef VALGRIND_MAKE_MEM_NOACCESS
	VALGRIND_MAKE_MEM_NOACCESS(new_l + _n, (want - _n) * sizeof(T));
#endif

	for (size_type i = 0; i < _n; i++) {
	    new(velt(new_l, i)) T(_l[i]);
	    _l[i].~T();
	}
	delete[] (unsigned char *)_l;

	_l = new_l;
	_capacity = want;
    }

    if (push_x)
	push_back(*push_x);
    return true;
}

template <class T> void
Vector<T>::resize(size_type n, const T &x)
{
    if (&x >= begin() && &x < end()) {
	T x_copy(x);
	resize(n, x_copy);
    } else if (n <= _capacity || reserve(n)) {
	for (size_type i = n; i < _n; i++)
	    _l[i].~T();
#ifdef VALGRIND_MAKE_MEM_NOACCESS
	if (n < _n)
	    VALGRIND_MAKE_MEM_NOACCESS(_l + n, (_n - n) * sizeof(T));
	if (_n < n)
	    VALGRIND_MAKE_MEM_UNDEFINED(_l + _n, (n - _n) * sizeof(T));
#endif
	for (size_type i = _n; i < n; i++)
	    new(velt(i)) T(x);
	_n = n;
    }
}

template <class T> void
Vector<T>::swap(Vector<T> &x)
{
    T *l = _l;
    _l = x._l;
    x._l = l;

    size_type n = _n;
    _n = x._n;
    x._n = n;

    size_type cap = _capacity;
    _capacity = x._capacity;
    x._capacity = cap;
}

#endif
