//========================================================================
//
// Array.cc
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2013, 2017, 2019, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2018, 2019 Adam Reichold <adam.reichold@t-online.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <cassert>

#include "Object.h"
#include "Array.h"

//------------------------------------------------------------------------
// Array
//------------------------------------------------------------------------

#define arrayLocker() const std::scoped_lock locker(mutex)

Array::Array(XRef *xrefA)
{
    xref = xrefA;
    ref = 1;
}

Array::~Array() { }

Array *Array::copy(XRef *xrefA) const
{
    arrayLocker();
    Array *a = new Array(xrefA);
    a->elems.reserve(elems.size());
    for (const auto &elem : elems) {
        a->elems.push_back(elem.copy());
    }
    return a;
}

Array *Array::deepCopy() const
{
    arrayLocker();
    Array *a = new Array(xref);
    a->elems.reserve(elems.size());
    for (const auto &elem : elems) {
        a->elems.push_back(elem.deepCopy());
    }
    return a;
}

void Array::add(Object &&elem)
{
    arrayLocker();
    elems.push_back(std::move(elem));
}

void Array::remove(int i)
{
    arrayLocker();
    if (i < 0 || std::size_t(i) >= elems.size()) {
        assert(i >= 0 && std::size_t(i) < elems.size());
        return;
    }
    elems.erase(elems.begin() + i);
}

Object Array::get(int i, int recursion) const
{
    if (i < 0 || std::size_t(i) >= elems.size()) {
        return Object(objNull);
    }
    return elems[i].fetch(xref, recursion);
}

Object Array::get(int i, Ref *returnRef, int recursion) const
{
    if (i < 0 || std::size_t(i) >= elems.size()) {
        *returnRef = Ref::INVALID();
        return Object(objNull);
    }
    if (elems[i].getType() == objRef) {
        *returnRef = elems[i].getRef();
    } else {
        *returnRef = Ref::INVALID();
    }
    return elems[i].fetch(xref, recursion);
}

const Object &Array::getNF(int i) const
{
    if (i < 0 || std::size_t(i) >= elems.size()) {
        static Object nullObj(objNull);
        return nullObj;
    }
    return elems[i];
}

bool Array::getString(int i, GooString *string) const
{
    const Object &obj = getNF(i);
    if (obj.isString()) {
        string->clear();
        string->append(obj.getString());
        return true;
    } else {
        return false;
    }
}
