//========================================================================
//
// Dict.cc
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
// Copyright (C) 2006 Krzysztof Kowalczyk <kkowalczyk@gmail.com>
// Copyright (C) 2007-2008 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2008, 2010, 2013, 2014, 2017, 2019, 2020, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2010 Paweł Wiejacha <pawel.wiejacha@gmail.com>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2014 Scott West <scott.gregory.west@gmail.com>
// Copyright (C) 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <algorithm>

#include "XRef.h"
#include "Dict.h"

//------------------------------------------------------------------------
// Dict
//------------------------------------------------------------------------

#define dictLocker() const std::scoped_lock locker(mutex)

constexpr int SORT_LENGTH_LOWER_LIMIT = 32;

struct Dict::CmpDictEntry
{
    bool operator()(const DictEntry &lhs, const DictEntry &rhs) const { return lhs.first < rhs.first; }
    bool operator()(const DictEntry &lhs, const char *rhs) const { return lhs.first < rhs; }
    bool operator()(const char *lhs, const DictEntry &rhs) const { return lhs < rhs.first; }
};

Dict::Dict(XRef *xrefA)
{
    xref = xrefA;
    ref = 1;

    sorted = false;
}

Dict::Dict(const Dict *dictA)
{
    xref = dictA->xref;
    ref = 1;

    entries.reserve(dictA->entries.size());
    for (const auto &entry : dictA->entries) {
        entries.emplace_back(entry.first, entry.second.copy());
    }

    sorted = dictA->sorted.load();
}

Dict *Dict::copy(XRef *xrefA) const
{
    dictLocker();
    Dict *dictA = new Dict(this);
    dictA->xref = xrefA;
    for (auto &entry : dictA->entries) {
        if (entry.second.getType() == objDict) {
            entry.second = Object(entry.second.getDict()->copy(xrefA));
        }
    }
    return dictA;
}

Dict *Dict::deepCopy() const
{
    dictLocker();
    Dict *dictA = new Dict(xref);

    dictA->entries.reserve(entries.size());
    for (auto &entry : entries) {
        dictA->entries.emplace_back(entry.first, entry.second.deepCopy());
    }
    return dictA;
}

void Dict::add(const char *key, Object &&val)
{
    dictLocker();
    entries.emplace_back(key, std::move(val));
    sorted = false;
}

inline const Dict::DictEntry *Dict::find(const char *key) const
{
    if (entries.size() >= SORT_LENGTH_LOWER_LIMIT) {
        if (!sorted) {
            dictLocker();
            if (!sorted) {
                Dict *that = const_cast<Dict *>(this);

                std::sort(that->entries.begin(), that->entries.end(), CmpDictEntry {});
                that->sorted = true;
            }
        }
    }

    if (sorted) {
        const auto pos = std::lower_bound(entries.begin(), entries.end(), key, CmpDictEntry {});
        if (pos != entries.end() && pos->first == key) {
            return &*pos;
        }
    } else {
        const auto pos = std::find_if(entries.rbegin(), entries.rend(), [key](const DictEntry &entry) { return entry.first == key; });
        if (pos != entries.rend()) {
            return &*pos;
        }
    }
    return nullptr;
}

inline Dict::DictEntry *Dict::find(const char *key)
{
    return const_cast<DictEntry *>(const_cast<const Dict *>(this)->find(key));
}

void Dict::remove(const char *key)
{
    dictLocker();
    if (auto *entry = find(key)) {
        if (sorted) {
            const auto index = entry - &entries.front();
            entries.erase(entries.begin() + index);
        } else {
            swap(*entry, entries.back());
            entries.pop_back();
        }
    }
}

void Dict::set(const char *key, Object &&val)
{
    if (val.isNull()) {
        remove(key);
        return;
    }
    dictLocker();
    if (auto *entry = find(key)) {
        entry->second = std::move(val);
    } else {
        add(key, std::move(val));
    }
}

bool Dict::is(const char *type) const
{
    if (const auto *entry = find("Type")) {
        return entry->second.isName(type);
    }
    return false;
}

Object Dict::lookup(const char *key, int recursion) const
{
    if (const auto *entry = find(key)) {
        return entry->second.fetch(xref, recursion);
    }
    return Object(objNull);
}

Object Dict::lookup(const char *key, Ref *returnRef, int recursion) const
{
    if (const auto *entry = find(key)) {
        if (entry->second.getType() == objRef) {
            *returnRef = entry->second.getRef();
        } else {
            *returnRef = Ref::INVALID();
        }
        return entry->second.fetch(xref, recursion);
    }
    *returnRef = Ref::INVALID();
    return Object(objNull);
}

Object Dict::lookupEnsureEncryptedIfNeeded(const char *key) const
{
    const auto *entry = find(key);
    if (!entry) {
        return Object(objNull);
    }

    if (entry->second.getType() == objRef && xref->isEncrypted() && !xref->isRefEncrypted(entry->second.getRef())) {
        error(errSyntaxError, -1, "{0:s} is not encrypted and the document is. This may be a hacking attempt", key);
        return Object(objNull);
    }

    return entry->second.fetch(xref);
}

const Object &Dict::lookupNF(const char *key) const
{
    if (const auto *entry = find(key)) {
        return entry->second;
    }
    static Object nullObj(objNull);
    return nullObj;
}

bool Dict::lookupInt(const char *key, const char *alt_key, int *value) const
{
    auto obj1 = lookup(key);
    if (obj1.isNull() && alt_key != nullptr) {
        obj1 = lookup(alt_key);
    }
    if (obj1.isInt()) {
        *value = obj1.getInt();
        return true;
    }
    return false;
}

Object Dict::getVal(int i, Ref *returnRef) const
{
    const DictEntry &entry = entries[i];
    if (entry.second.getType() == objRef) {
        *returnRef = entry.second.getRef();
    } else {
        *returnRef = Ref::INVALID();
    }
    return entry.second.fetch(xref);
}

bool Dict::hasKey(const char *key) const
{
    return find(key) != nullptr;
}

std::string Dict::findAvailableKey(const std::string &suggestedKey)
{
    int i = 0;
    std::string res = suggestedKey;
    while (find(res.c_str())) {
        res = suggestedKey + std::to_string(i++);
    }
    return res;
}
