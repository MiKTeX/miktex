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
// Copyright (C) 2008, 2016 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Nick Jones <nick.jones@network-box.com>
// Copyright (C) 2016 Jason Crain <jason@aquaticape.us>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "goo/gmem.h"
#include "goo/GooString.h"
#include "goo/GooList.h"
#include "XRef.h"
#include "Link.h"
#include "PDFDocEncoding.h"
#include "Outline.h"
#include "UTF.h"

//------------------------------------------------------------------------

Outline::Outline(Object *outlineObj, XRef *xref) {
  Object first, last;

  items = NULL;
  if (!outlineObj->isDict()) {
    return;
  }
  items = OutlineItem::readItemList(outlineObj->dictLookupNF("First", &first), xref);
  first.free();
  last.free();
}

Outline::~Outline() {
  if (items) {
    deleteGooList(items, OutlineItem);
  }
}

//------------------------------------------------------------------------

OutlineItem::OutlineItem(Dict *dict, XRef *xrefA) {
  Object obj1;
  GooString *s;

  xref = xrefA;
  title = NULL;
  action = NULL;
  kids = NULL;

  if (dict->lookup("Title", &obj1)->isString()) {
    s = obj1.getString();
    titleLen = TextStringToUCS4(s, &title);
  } else {
    titleLen = 0;
  }
  obj1.free();

  if (!dict->lookup("Dest", &obj1)->isNull()) {
    action = LinkAction::parseDest(&obj1);
  } else {
      obj1.free();
    if (!dict->lookup("A", &obj1)->isNull()) {
        action = LinkAction::parseAction(&obj1);
  }
  }
  obj1.free();

  dict->lookupNF("First", &firstRef);
  dict->lookupNF("Last", &lastRef);
  dict->lookupNF("Next", &nextRef);

  startsOpen = gFalse;
  if (dict->lookup("Count", &obj1)->isInt()) {
    if (obj1.getInt() > 0) {
      startsOpen = gTrue;
    }
  }
  obj1.free();
}

OutlineItem::~OutlineItem() {
  close();
  if (title) {
    gfree(title);
  }
  if (action) {
    delete action;
  }
  firstRef.free();
  lastRef.free();
  nextRef.free();
}

GooList *OutlineItem::readItemList(Object *firstItemRef, XRef *xrefA) {
  GooList *items;
  char* alreadyRead;
  OutlineItem *item;
  Object obj;
  Object *p;

  items = new GooList();

  alreadyRead = (char *)gmalloc(xrefA->getNumObjects());
  memset(alreadyRead, 0, xrefA->getNumObjects());

  p = firstItemRef;
  while (p->isRef() && 
	 (p->getRefNum() >= 0) && 
         (p->getRefNum() < xrefA->getNumObjects()) && 
         !alreadyRead[p->getRefNum()]) {
    if (!p->fetch(xrefA, &obj)->isDict()) {
      obj.free();
      break;
    }
    alreadyRead[p->getRefNum()] = 1;
    item = new OutlineItem(obj.getDict(), xrefA);
    obj.free();
    items->append(item);
    p = &item->nextRef;
  }

  gfree(alreadyRead);

  if (!items->getLength()) {
    delete items;
    items = NULL;
  }

  return items;
}

void OutlineItem::open() {
  if (!kids) {
    kids = readItemList(&firstRef, xref);
  }
}

void OutlineItem::close() {
  if (kids) {
    deleteGooList(kids, OutlineItem);
    kids = NULL;
  }
}
