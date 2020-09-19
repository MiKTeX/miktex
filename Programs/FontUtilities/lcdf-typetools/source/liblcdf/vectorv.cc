/*
 * vectorv.cc -- template specialization for Vector<void*>
 * Eddie Kohler
 *
 * Copyright (c) 1999-2006 Massachusetts Institute of Technology
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <lcdf/vector.hh>
#include <string.h>

Vector<void*>::Vector(const Vector<void*> &o)
    : _l(0), _n(0), _capacity(0)
{
    *this = o;
}

Vector<void*>::~Vector()
{
    delete[] _l;
}

Vector<void*> &
Vector<void*>::operator=(const Vector<void*> &o)
{
    if (&o != this) {
        _n = 0;
        if (reserve(o._n)) {
            _n = o._n;
            memcpy(_l, o._l, sizeof(void *) * _n);
        }
    }
    return *this;
}

Vector<void*> &
Vector<void*>::assign(int n, void *e)
{
    _n = 0;
    resize(n, e);
    return *this;
}

bool
Vector<void*>::reserve(int want)
{
    if (want < 0)
        want = (_capacity > 0 ? _capacity * 2 : 4);
    if (want <= _capacity)
        return true;

    void **new_l = new void*[want];
    if (!new_l)
        return false;

    if (_n) {
        memcpy(new_l, _l, sizeof(void*) * _n);
    }
    delete[] _l;

    _l = new_l;
    _capacity = want;
    return true;
}

Vector<void*>::iterator
Vector<void*>::erase(iterator a, iterator b)
{
    if (b > a) {
        assert(a >= begin() && b <= end());
        memmove(a, b, (end() - b) * sizeof(void*));
        _n -= b - a;
        return a;
    } else
        return b;
}

void
Vector<void*>::resize(int nn, void *e)
{
    if (nn <= _capacity || reserve(nn)) {
        for (int i = _n; i < nn; i++)
            _l[i] = e;
        _n = nn;
    }
}

void
Vector<void*>::swap(Vector<void*>& o)
{
    void **l = _l;
    int n = _n;
    int cap = _capacity;
    _l = o._l;
    _n = o._n;
    _capacity = o._capacity;
    o._l = l;
    o._n = n;
    o._capacity = cap;
}
