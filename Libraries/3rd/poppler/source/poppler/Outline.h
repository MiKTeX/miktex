//========================================================================
//
// Outline.h
//
// Copyright 2002-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005 Marco Pesenti Gritti <mpg@redhat.com>
// Copyright (C) 2016, 2018 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2019, 2020 Oliver Sander <oliver.sander@tu-dresden.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef OUTLINE_H
#define OUTLINE_H

#include <memory>
#include "Object.h"
#include "CharTypes.h"

class GooString;
class XRef;
class LinkAction;
class OutlineItem;

//------------------------------------------------------------------------

class Outline
{
public:
    Outline(const Object *outlineObj, XRef *xref);
    ~Outline();

    Outline(const Outline &) = delete;
    Outline &operator=(const Outline &) = delete;

    const std::vector<OutlineItem *> *getItems() const { return items; }

private:
    std::vector<OutlineItem *> *items; // nullptr if document has no outline,
};

//------------------------------------------------------------------------

class OutlineItem
{
public:
    OutlineItem(const Dict *dict, int refNumA, OutlineItem *parentA, XRef *xrefA);
    ~OutlineItem();

    OutlineItem(const OutlineItem &) = delete;
    OutlineItem &operator=(const OutlineItem &) = delete;

    static std::vector<OutlineItem *> *readItemList(OutlineItem *parent, const Object *firstItemRef, XRef *xrefA);

    void open();
    void close();

    const Unicode *getTitle() const { return title; }
    int getTitleLength() const { return titleLen; }
    // OutlineItem keeps the ownership of the action
    const LinkAction *getAction() const { return action.get(); }
    bool isOpen() const { return startsOpen; }
    bool hasKids() const { return firstRef.isRef(); }
    const std::vector<OutlineItem *> *getKids() const { return kids; }

private:
    int refNum;
    OutlineItem *parent;
    XRef *xref;
    Unicode *title;
    int titleLen;
    std::unique_ptr<LinkAction> action;
    Object firstRef;
    Object lastRef;
    Object nextRef;
    bool startsOpen;
    std::vector<OutlineItem *> *kids; // nullptr if this item is closed or has no kids
};

#endif
