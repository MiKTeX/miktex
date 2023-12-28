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
// Copyright (C) 2008, 2016-2019, 2021, 2023 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Nick Jones <nick.jones@network-box.com>
// Copyright (C) 2016 Jason Crain <jason@aquaticape.us>
// Copyright (C) 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019, 2020 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright (C) 2021 RM <rm+git@arcsin.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include "goo/gmem.h"
#include "goo/GooString.h"
#include "PDFDoc.h"
#include "XRef.h"
#include "Link.h"
#include "PDFDocEncoding.h"
#include "Outline.h"
#include "UTF.h"

//------------------------------------------------------------------------

Outline::Outline(Object *outlineObjA, XRef *xrefA, PDFDoc *docA)
{
    outlineObj = outlineObjA;
    xref = xrefA;
    doc = docA;
    items = nullptr;
    if (!outlineObj->isDict()) {
        return;
    }
    const Object &first = outlineObj->dictLookupNF("First");
    items = OutlineItem::readItemList(nullptr, &first, xref, doc);
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

static void insertChildHelper(const std::string &itemTitle, int destPageNum, unsigned int pos, Ref parentObjRef, PDFDoc *doc, XRef *xref, std::vector<OutlineItem *> &items)
{
    std::vector<OutlineItem *>::const_iterator it;
    if (pos >= items.size()) {
        it = items.end();
    } else {
        it = items.begin() + pos;
    }

    Array *a = new Array(xref);
    Ref *pageRef = doc->getCatalog()->getPageRef(destPageNum);
    if (pageRef != nullptr) {
        a->add(Object(*pageRef));
    } else {
        // if the page obj doesn't exist put the page number
        // PDF32000-2008 12.3.2.2 Para 2
        // as if it's a "Remote-Go-To Actions"
        // it's not strictly valid, but most viewers seem
        // to handle it without crashing
        // alternately, could put 0, or omit it
        a->add(Object(destPageNum - 1));
    }
    a->add(Object(objName, "Fit"));

    Object outlineItem = Object(new Dict(xref));

    GooString *g = new GooString(itemTitle);
    outlineItem.dictSet("Title", Object(g));
    outlineItem.dictSet("Dest", Object(a));
    outlineItem.dictSet("Count", Object(1));
    outlineItem.dictAdd("Parent", Object(parentObjRef));

    // add one to the main outline Object's count
    Object parentObj = xref->fetch(parentObjRef);
    int parentCount = parentObj.dictLookup("Count").getInt();
    parentObj.dictSet("Count", Object(parentCount + 1));
    xref->setModifiedObject(&parentObj, parentObjRef);

    Object prevItemObject;
    Object nextItemObject;

    Ref outlineItemRef = xref->addIndirectObject(outlineItem);

    // the next two statements fix up the parent object
    // for clarity we separate this out
    if (it == items.begin()) {
        // we will be the first item in the list
        // fix our parent
        parentObj.dictSet("First", Object(outlineItemRef));
    }
    if (it == items.end()) {
        // we will be the last item on the list
        // fix up our parent
        parentObj.dictSet("Last", Object(outlineItemRef));
    }

    if (it == items.end()) {
        if (!items.empty()) {
            // insert at the end, we handle this separately
            prevItemObject = xref->fetch((*(it - 1))->getRef());
            prevItemObject.dictSet("Next", Object(outlineItemRef));
            outlineItem.dictSet("Prev", Object((*(it - 1))->getRef()));
            xref->setModifiedObject(&prevItemObject, (*(it - 1))->getRef());
        }
    } else {
        nextItemObject = xref->fetch((*it)->getRef());
        nextItemObject.dictSet("Prev", Object(outlineItemRef));
        xref->setModifiedObject(&nextItemObject, (*it)->getRef());

        outlineItem.dictSet("Next", Object((*(it))->getRef()));

        if (it != items.begin()) {
            prevItemObject = xref->fetch((*(it - 1))->getRef());
            prevItemObject.dictSet("Next", Object(outlineItemRef));
            outlineItem.dictSet("Prev", Object((*(it - 1))->getRef()));
            xref->setModifiedObject(&prevItemObject, (*(it - 1))->getRef());
        }
    }

    OutlineItem *item = new OutlineItem(outlineItem.getDict(), outlineItemRef, nullptr, xref, doc);

    items.insert(it, item);
}

void Outline::insertChild(const std::string &itemTitle, int destPageNum, unsigned int pos)
{
    Ref outlineObjRef = xref->getCatalog().dictLookupNF("Outlines").getRef();
    insertChildHelper(itemTitle, destPageNum, pos, outlineObjRef, doc, xref, *items);
}

// ref is a valid reference to a list
// walk the list and free any children
// returns the number items deleted (just in case)
static int recursiveRemoveList(Ref ref, XRef *xref)
{
    int count = 0;
    bool done = false;

    Ref nextRef;
    Object tempObj;

    while (!done) {
        tempObj = xref->fetch(ref);

        if (!tempObj.isDict()) {
            // something horrible has happened
            break;
        }

        const Object &firstRef = tempObj.dictLookupNF("First");
        if (firstRef.isRef()) {
            count += recursiveRemoveList(firstRef.getRef(), xref);
        }

        const Object &nextObjRef = tempObj.dictLookupNF("Next");
        if (nextObjRef.isRef()) {
            nextRef = nextObjRef.getRef();
        } else {
            done = true;
        }
        xref->removeIndirectObject(ref);
        count++;
        ref = nextRef;
    }
    return count;
}

static void removeChildHelper(unsigned int pos, PDFDoc *doc, XRef *xref, std::vector<OutlineItem *> &items)
{
    std::vector<OutlineItem *>::const_iterator it;
    if (pos >= items.size()) {
        // position is out of range, do nothing
        return;
    } else {
        it = items.begin() + pos;
    }

    //  relink around this node
    Object itemObject = xref->fetch((*it)->getRef());
    Object parentObj = itemObject.dictLookup("Parent");
    Object prevItemObject = itemObject.dictLookup("Prev");
    Object nextItemObject = itemObject.dictLookup("Next");

    // delete 1 from the parent Count if it's positive
    Object countObj = parentObj.dictLookup("Count");
    int count = countObj.getInt();
    if (count > 0) {
        count--;
        parentObj.dictSet("Count", Object(count));
        xref->setModifiedObject(&parentObj, itemObject.dictLookupNF("Parent").getRef());
    }

    if (!prevItemObject.isNull() && !nextItemObject.isNull()) {
        // deletion is in the middle
        prevItemObject.dictSet("Next", Object((*(it + 1))->getRef()));
        xref->setModifiedObject(&prevItemObject, (*(it - 1))->getRef());

        nextItemObject.dictSet("Prev", Object((*(it - 1))->getRef()));
        xref->setModifiedObject(&nextItemObject, (*(it + 1))->getRef());
    } else if (prevItemObject.isNull() && nextItemObject.isNull()) {
        // deletion is only child
        parentObj.dictRemove("First");
        parentObj.dictRemove("Last");
        xref->setModifiedObject(&parentObj, itemObject.dictLookupNF("Parent").getRef());
    } else if (prevItemObject.isNull()) {
        // deletion at the front
        parentObj.dictSet("First", Object((*(it + 1))->getRef()));
        xref->setModifiedObject(&parentObj, itemObject.dictLookupNF("Parent").getRef());

        nextItemObject.dictRemove("Prev");
        xref->setModifiedObject(&nextItemObject, (*(it + 1))->getRef());
    } else {
        // deletion at the end
        parentObj.dictSet("Last", Object((*(it - 1))->getRef()));
        xref->setModifiedObject(&parentObj, itemObject.dictLookupNF("Parent").getRef());
        prevItemObject.dictRemove("Next");
        xref->setModifiedObject(&prevItemObject, (*(it - 1))->getRef());
    }

    // free any children
    const Object &firstRef = itemObject.dictLookupNF("First");
    if (firstRef.isRef()) {
        recursiveRemoveList(firstRef.getRef(), xref);
    }

    // free the pdf objects and the representation
    xref->removeIndirectObject((*it)->getRef());
    OutlineItem *oi = *it;
    items.erase(it);
    // deletion of the OutlineItem will delete all child
    // outline items in its destructor
    delete oi;
}

void Outline::removeChild(unsigned int pos)
{
    removeChildHelper(pos, doc, xref, *items);
}

//------------------------------------------------------------------------

int Outline::addOutlineTreeNodeList(const std::vector<OutlineTreeNode> &nodeList, Ref &parentRef, Ref &firstRef, Ref &lastRef)
{
    firstRef = Ref::INVALID();
    lastRef = Ref::INVALID();
    if (nodeList.empty()) {
        return 0;
    }

    int itemCount = 0;
    Ref prevNodeRef = Ref::INVALID();

    for (auto &node : nodeList) {

        Array *a = new Array(doc->getXRef());
        Ref *pageRef = doc->getCatalog()->getPageRef(node.destPageNum);
        if (pageRef != nullptr) {
            a->add(Object(*pageRef));
        } else {
            // if the page obj doesn't exist put the page number
            // PDF32000-2008 12.3.2.2 Para 2
            // as if it's a "Remote-Go-To Actions"
            // it's not strictly valid, but most viewers seem
            // to handle it without crashing
            // alternately, could put 0, or omit it
            a->add(Object(node.destPageNum - 1));
        }
        a->add(Object(objName, "Fit"));

        Object outlineItem = Object(new Dict(doc->getXRef()));
        Ref outlineItemRef = doc->getXRef()->addIndirectObject(outlineItem);

        if (firstRef == Ref::INVALID()) {
            firstRef = outlineItemRef;
        }
        lastRef = outlineItemRef;

        GooString *g = new GooString(node.title);
        outlineItem.dictSet("Title", Object(g));
        outlineItem.dictSet("Dest", Object(a));
        itemCount++;

        if (prevNodeRef != Ref::INVALID()) {
            outlineItem.dictSet("Prev", Object(prevNodeRef));

            // maybe easier way to fix up the previous object
            Object prevOutlineItem = xref->fetch(prevNodeRef);
            prevOutlineItem.dictSet("Next", Object(outlineItemRef));
            xref->setModifiedObject(&prevOutlineItem, prevNodeRef);
        }
        prevNodeRef = outlineItemRef;

        Ref firstChildRef;
        Ref lastChildRef;
        itemCount += addOutlineTreeNodeList(node.children, outlineItemRef, firstChildRef, lastChildRef);

        if (firstChildRef != Ref::INVALID()) {
            outlineItem.dictSet("First", Object(firstChildRef));
            outlineItem.dictSet("Last", Object(lastChildRef));
        }
        outlineItem.dictSet("Count", Object(itemCount));
        outlineItem.dictAdd("Parent", Object(parentRef));
    }
    return itemCount;
}

/* insert an outline into a PDF
   outline->setOutline({ {"page 1", 1,
                                         { { "1.1", 1, {} } }   },
                            {"page 2", 2, {} },
                            {"page 3", 3, {} },
                            {"page 4", 4,{ { "4.1", 4, {} },
                                           { "4.2", 4, {} },
                                         },
                            }
                       });
 */

void Outline::setOutline(const std::vector<OutlineTreeNode> &nodeList)
{
    // check if outlineObj is an object, if it's not make sure it exists
    if (!outlineObj->isDict()) {
        outlineObj = doc->getCatalog()->getCreateOutline();

        // make sure it was created
        if (!outlineObj->isDict()) {
            return;
        }
    }

    Ref outlineObjRef = xref->getCatalog().dictLookupNF("Outlines").getRef();
    Ref firstChildRef;
    Ref lastChildRef;

    // free any OutlineItem objects that will be replaced
    const Object &firstChildRefObj = outlineObj->dictLookupNF("First");
    if (firstChildRefObj.isRef()) {
        recursiveRemoveList(firstChildRefObj.getRef(), xref);
    }

    const int count = addOutlineTreeNodeList(nodeList, outlineObjRef, firstChildRef, lastChildRef);

    // modify the parent Outlines dict
    if (firstChildRef != Ref::INVALID()) {
        outlineObj->dictSet("First", Object(firstChildRef));
        outlineObj->dictSet("Last", Object(lastChildRef));
    } else {
        // nothing was inserted into the outline, so just remove the
        // child references in the top-level outline
        outlineObj->dictRemove("First");
        outlineObj->dictRemove("Last");
    }
    outlineObj->dictSet("Count", Object(count));
    xref->setModifiedObject(outlineObj, outlineObjRef);

    // reload the outline object from the xrefs

    if (items) {
        for (auto entry : *items) {
            delete entry;
        }
        delete items;
    }
    const Object &first = outlineObj->dictLookupNF("First");
    // we probably want to allow readItemList to create an empty list
    // but for now just check and do it ourselves here
    if (first.isRef()) {
        items = OutlineItem::readItemList(nullptr, &first, xref, doc);
    } else {
        items = new std::vector<OutlineItem *>();
    }
}

//------------------------------------------------------------------------

OutlineItem::OutlineItem(const Dict *dict, Ref refA, OutlineItem *parentA, XRef *xrefA, PDFDoc *docA)
{
    Object obj1;

    ref = refA;
    parent = parentA;
    xref = xrefA;
    doc = docA;
    title = nullptr;
    kids = nullptr;

    obj1 = dict->lookup("Title");
    if (obj1.isString()) {
        const GooString *s = obj1.getString();
        titleLen = TextStringToUCS4(s->toStr(), &title);
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
    if (kids) {
        for (auto entry : *kids) {
            delete entry;
        }
        delete kids;
        kids = nullptr;
    }
    if (title) {
        gfree(title);
    }
}

std::vector<OutlineItem *> *OutlineItem::readItemList(OutlineItem *parent, const Object *firstItemRef, XRef *xrefA, PDFDoc *docA)
{
    auto items = new std::vector<OutlineItem *>();

    // could be a hash (unordered_map) too for better avg case check
    // small number of objects expected, likely doesn't matter
    std::set<Ref> alreadyRead;

    OutlineItem *parentO = parent;
    while (parentO) {
        alreadyRead.insert(parentO->getRef());
        parentO = parentO->parent;
    }

    Object tempObj = firstItemRef->copy();
    while (tempObj.isRef() && (tempObj.getRefNum() >= 0) && (tempObj.getRefNum() < xrefA->getNumObjects()) && alreadyRead.find(tempObj.getRef()) == alreadyRead.end()) {
        Object obj = tempObj.fetch(xrefA);
        if (!obj.isDict()) {
            break;
        }
        alreadyRead.insert(tempObj.getRef());
        OutlineItem *item = new OutlineItem(obj.getDict(), tempObj.getRef(), parent, xrefA, docA);
        items->push_back(item);
        tempObj = obj.dictLookupNF("Next").copy();
    }
    return items;
}

void OutlineItem::open()
{
    if (!kids) {
        Object itemDict = xref->fetch(ref);
        if (itemDict.isDict()) {
            const Object &firstRef = itemDict.dictLookupNF("First");
            kids = readItemList(this, &firstRef, xref, doc);
        } else {
            kids = new std::vector<OutlineItem *>();
        }
    }
}

void OutlineItem::setTitle(const std::string &titleA)
{
    gfree(title);

    Object dict = xref->fetch(ref);
    GooString *g = new GooString(titleA);
    titleLen = TextStringToUCS4(g->toStr(), &title);
    dict.dictSet("Title", Object(g));
    xref->setModifiedObject(&dict, ref);
}

bool OutlineItem::setPageDest(int i)
{
    Object dict = xref->fetch(ref);
    Object obj1;

    if (i < 1) {
        return false;
    }

    obj1 = dict.dictLookup("Dest");
    if (!obj1.isNull()) {
        int arrayLength = obj1.arrayGetLength();
        for (int index = 0; index < arrayLength; index++) {
            obj1.arrayRemove(0);
        }
        obj1.arrayAdd(Object(i - 1));
        obj1.arrayAdd(Object(objName, "Fit"));

        // unique_ptr will destroy previous on assignment
        action = LinkAction::parseDest(&obj1);
    } else {
        obj1 = dict.dictLookup("A");
        if (!obj1.isNull()) {
            // RM 20210505 Implement
        } else {
        }
        return false;
    }

    xref->setModifiedObject(&dict, ref);
    return true;
}

void OutlineItem::insertChild(const std::string &itemTitle, int destPageNum, unsigned int pos)
{
    open();
    insertChildHelper(itemTitle, destPageNum, pos, ref, doc, xref, *kids);
}

void OutlineItem::removeChild(unsigned int pos)
{
    open();
    removeChildHelper(pos, doc, xref, *kids);
}

void OutlineItem::setStartsOpen(bool value)
{
    startsOpen = value;
    Object dict = xref->fetch(ref);
    Object obj1 = dict.dictLookup("Count");
    if (obj1.isInt()) {
        const int count = obj1.getInt();
        if ((count > 0 && !value) || (count < 0 && value)) {
            // states requires change of sign
            dict.dictSet("Count", Object(-count));
            xref->setModifiedObject(&dict, ref);
        }
    }
}

bool OutlineItem::hasKids()
{
    open();
    return !kids->empty();
}

const std::vector<OutlineItem *> *OutlineItem::getKids()
{
    open();

    if (!kids || kids->empty()) {
        return nullptr;
    } else {
        return kids;
    }
}
