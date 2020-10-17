//========================================================================
//
// Outline.cc
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
// Copyright (C) 2008, 2016-2019 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Nick Jones <nick.jones@network-box.com>
// Copyright (C) 2016 Jason Crain <jason@aquaticape.us>
// Copyright (C) 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019, 2020 Oliver Sander <oliver.sander@tu-dresden.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include "goo/gmem.h"
#include "goo/GooString.h"
#include "XRef.h"
#include "Link.h"
#include "PDFDocEncoding.h"
#include "Outline.h"
#include "UTF.h"

//------------------------------------------------------------------------

Outline::Outline(const Object *outlineObj, XRef *xref)
{
    items = nullptr;
    if (!outlineObj->isDict()) {
        return;
    }
    const Object &first = outlineObj->dictLookupNF("First");
    items = OutlineItem::readItemList(nullptr, &first, xref);
}

Outline::~Outline()
{
    if (items) {
        for (auto entry : *items) {
            delete entry;
        }
        delete items;
    }
}

//------------------------------------------------------------------------

OutlineItem::OutlineItem(const Dict *dict, int refNumA, OutlineItem *parentA, XRef *xrefA)
{
    Object obj1;

    refNum = refNumA;
    parent = parentA;
    xref = xrefA;
    title = nullptr;
    kids = nullptr;

    obj1 = dict->lookup("Title");
    if (obj1.isString()) {
        const GooString *s = obj1.getString();
        titleLen = TextStringToUCS4(s, &title);
    } else {
        titleLen = 0;
    }

    obj1 = dict->lookup("Dest");
    if (!obj1.isNull()) {
        action = LinkAction::parseDest(&obj1);
    } else {
        obj1 = dict->lookup("A");
        if (!obj1.isNull()) {
            action = LinkAction::parseAction(&obj1);
        }
    }

    firstRef = dict->lookupNF("First").copy();
    lastRef = dict->lookupNF("Last").copy();
    nextRef = dict->lookupNF("Next").copy();

    startsOpen = false;
    obj1 = dict->lookup("Count");
    if (obj1.isInt()) {
        if (obj1.getInt() > 0) {
            startsOpen = true;
        }
    }
}

OutlineItem::~OutlineItem()
{
    close();
    if (title) {
        gfree(title);
    }
}

std::vector<OutlineItem *> *OutlineItem::readItemList(OutlineItem *parent, const Object *firstItemRef, XRef *xrefA)
{
    auto items = new std::vector<OutlineItem *>();

    char *alreadyRead = (char *)gmalloc(xrefA->getNumObjects());
    memset(alreadyRead, 0, xrefA->getNumObjects());

    OutlineItem *parentO = parent;
    while (parentO) {
        alreadyRead[parentO->refNum] = 1;
        parentO = parentO->parent;
    }

    const Object *p = firstItemRef;
    while (p->isRef() && (p->getRefNum() >= 0) && (p->getRefNum() < xrefA->getNumObjects()) && !alreadyRead[p->getRefNum()]) {
        Object obj = p->fetch(xrefA);
        if (!obj.isDict()) {
            break;
        }
        alreadyRead[p->getRefNum()] = 1;
        OutlineItem *item = new OutlineItem(obj.getDict(), p->getRefNum(), parent, xrefA);
        items->push_back(item);
        p = &item->nextRef;
    }

    gfree(alreadyRead);

    if (items->empty()) {
        delete items;
        items = nullptr;
    }

    return items;
}

void OutlineItem::open()
{
    if (!kids) {
        kids = readItemList(this, &firstRef, xref);
    }
}

void OutlineItem::close()
{
    if (kids) {
        for (auto entry : *kids) {
            delete entry;
        }
        delete kids;
        kids = nullptr;
    }
}
