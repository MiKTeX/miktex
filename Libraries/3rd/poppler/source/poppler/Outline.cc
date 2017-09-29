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
// Copyright (C) 2008, 2016, 2017 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Nick Jones <nick.jones@network-box.com>
// Copyright (C) 2016 Jason Crain <jason@aquaticape.us>
// Copyright (C) 2017 Adrian Johnson <ajohnson@redneon.com>
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
  items = nullptr;
  if (!outlineObj->isDict()) {
    return;
  }
  Object first = outlineObj->dictLookupNF("First");
  items = OutlineItem::readItemList(&first, xref);
}

Outline::~Outline() {
  if (items) {
    deleteGooList(items, OutlineItem);
  }
}

//------------------------------------------------------------------------

OutlineItem::OutlineItem(Dict *dict, XRef *xrefA) {
  Object obj1;

  xref = xrefA;
  title = NULL;
  action = NULL;
  kids = NULL;


  obj1 = dict->lookup("Title");
  if (obj1.isString()) {
    GooString *s = obj1.getString();
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

  firstRef = dict->lookupNF("First");
  lastRef = dict->lookupNF("Last");
  nextRef = dict->lookupNF("Next");

  startsOpen = gFalse;
  obj1 = dict->lookup("Count");
  if (obj1.isInt()) {
    if (obj1.getInt() > 0) {
      startsOpen = gTrue;
    }
  }
}

OutlineItem::~OutlineItem() {
  close();
  if (title) {
    gfree(title);
  }
  if (action) {
    delete action;
  }
}

GooList *OutlineItem::readItemList(Object *firstItemRef, XRef *xrefA) {
  GooList *items;
  char* alreadyRead;
  OutlineItem *item;
  Object *p;

  items = new GooList();

  alreadyRead = (char *)gmalloc(xrefA->getNumObjects());
  memset(alreadyRead, 0, xrefA->getNumObjects());

  p = firstItemRef;
  while (p->isRef() && 
	 (p->getRefNum() >= 0) && 
         (p->getRefNum() < xrefA->getNumObjects()) && 
         !alreadyRead[p->getRefNum()]) {
    Object obj = p->fetch(xrefA);
    if (!obj.isDict()) {
      break;
    }
    alreadyRead[p->getRefNum()] = 1;
    item = new OutlineItem(obj.getDict(), xrefA);
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
