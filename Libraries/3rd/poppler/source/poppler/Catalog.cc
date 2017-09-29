//========================================================================
//
// Catalog.cc
//
// Copyright 1996-2007 Glyph & Cog, LLC
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
// Copyright (C) 2005-2013, 2015, 2017 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2005 Jeff Muizelaar <jrmuizel@nit.ca>
// Copyright (C) 2005 Jonathan Blandford <jrb@redhat.com>
// Copyright (C) 2005 Marco Pesenti Gritti <mpg@redhat.com>
// Copyright (C) 2005, 2006, 2008 Brad Hards <bradh@frogmouth.net>
// Copyright (C) 2006, 2008, 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2007 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2008, 2011 Pino Toscano <pino@kde.org>
// Copyright (C) 2009 Ilya Gorenbein <igorenbein@finjan.com>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2013 Julien Nabet <serval2412@yahoo.fr>
// Copyright (C) 2013 Adrian Perez de Castro <aperez@igalia.com>
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 José Aliste <jaliste@src.gnome.org>
// Copyright (C) 2014 Ed Porras <ed@moto-research.com>
// Copyright (C) 2015 Even Rouault <even.rouault@spatialys.com>
// Copyright (C) 2016 Masamichi Hosoda <trueroad@trueroad.jp>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stddef.h>
#include <stdlib.h>
#include "goo/gmem.h"
#include "Object.h"
#include "PDFDoc.h"
#include "XRef.h"
#include "Array.h"
#include "Dict.h"
#include "Page.h"
#include "Error.h"
#include "Link.h"
#include "PageLabelInfo.h"
#include "Catalog.h"
#include "Form.h"
#include "OptionalContent.h"
#include "ViewerPreferences.h"
#include "FileSpec.h"
#include "StructTreeRoot.h"

#if MULTITHREADED
#  define catalogLocker()   MutexLocker locker(&mutex)
#else
#  define catalogLocker()
#endif
//------------------------------------------------------------------------
// Catalog
//------------------------------------------------------------------------

Catalog::Catalog(PDFDoc *docA) {
#if MULTITHREADED
  gInitMutex(&mutex);
#endif
  ok = gTrue;
  doc = docA;
  xref = doc->getXRef();
  pages = NULL;
  pageRefs = NULL;
  numPages = -1;
  pagesSize = 0;
  baseURI = NULL;
  pageLabelInfo = NULL;
  form = NULL;
  optContent = NULL;
  pageMode = pageModeNull;
  pageLayout = pageLayoutNull;
  destNameTree = NULL;
  embeddedFileNameTree = NULL;
  jsNameTree = NULL;
  viewerPrefs = NULL;
  structTreeRoot = NULL;

  pagesList = NULL;
  pagesRefList = NULL;
  attrsList = NULL;
  kidsIdxList = NULL;
  lastCachedPage = 0;
  markInfo = markInfoNull;

  Object catDict = xref->getCatalog();
  if (!catDict.isDict()) {
    error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
    ok = gFalse;
    return;
  }
  // get the AcroForm dictionary
  acroForm = catDict.dictLookup("AcroForm");

  // read base URI
  Object obj = catDict.dictLookup("URI");
  if (obj.isDict()) {
    Object obj2 = obj.dictLookup("Base");
    if (obj2.isString()) {
      baseURI = obj2.getString()->copy();
    }
  }

  // get the Optional Content dictionary
  Object optContentProps = catDict.dictLookup("OCProperties");
  if (optContentProps.isDict()) {
    optContent = new OCGs(&optContentProps, xref);
    if (!optContent->isOk ()) {
      delete optContent;
      optContent = NULL;
    }
  }

  // actions
  additionalActions = catDict.dictLookupNF("AA");

  // get the ViewerPreferences dictionary
  viewerPreferences = catDict.dictLookup("ViewerPreferences");
}

Catalog::~Catalog() {
  delete kidsIdxList;
  if (attrsList) {
    std::vector<PageAttrs *>::iterator it;
    for (it = attrsList->begin() ; it != attrsList->end(); ++it ) {
      delete *it;
    }
    delete attrsList;
  }
  delete pagesRefList;
  delete pagesList;
  if (pages) {
    for (int i = 0; i < pagesSize; ++i) {
      if (pages[i]) {
	delete pages[i];
      }
    }
    gfree(pages);
  }
  gfree(pageRefs);
  delete destNameTree;
  delete embeddedFileNameTree;
  delete jsNameTree;
  if (baseURI) {
    delete baseURI;
  }
  delete pageLabelInfo;
  delete form;
  delete optContent;
  delete viewerPrefs;
  delete structTreeRoot;
#if MULTITHREADED
  gDestroyMutex(&mutex);
#endif
}

GooString *Catalog::readMetadata() {
  catalogLocker();
  if (metadata.isNone()) {
    Object catDict = xref->getCatalog();
    if (catDict.isDict()) {
      metadata = catDict.dictLookup("Metadata");
    } else {
      error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
      metadata.setToNull();
    }
  }

  if (!metadata.isStream()) {
    return nullptr;
  }
  Object obj = metadata.streamGetDict()->lookup("Subtype");
  if (!obj.isName("XML")) {
    error(errSyntaxWarning, -1, "Unknown Metadata type: '{0:s}'",
	  obj.isName() ? obj.getName() : "???");
  }
  GooString *s = new GooString();
  metadata.getStream()->fillGooString(s);
  metadata.streamClose();
  return s;
}

Page *Catalog::getPage(int i)
{
  if (i < 1) return NULL;

  catalogLocker();
  if (i > lastCachedPage) {
     GBool cached = cachePageTree(i);
     if ( cached == gFalse) {
       return NULL;
     }
  }
  return pages[i-1];
}

Ref *Catalog::getPageRef(int i)
{
  if (i < 1) return NULL;

  catalogLocker();
  if (i > lastCachedPage) {
     GBool cached = cachePageTree(i);
     if ( cached == gFalse) {
       return NULL;
     }
  }
  return &pageRefs[i-1];
}

GBool Catalog::cachePageTree(int page)
{
  if (pagesList == NULL) {

    Ref pagesRef;

    Object catDict = xref->getCatalog();

    if (catDict.isDict()) {
      Object pagesDictRef = catDict.dictLookupNF("Pages");
      if (pagesDictRef.isRef() &&
          pagesDictRef.getRefNum() >= 0 &&
          pagesDictRef.getRefNum() < xref->getNumObjects()) {
        pagesRef = pagesDictRef.getRef();
      } else {
        error(errSyntaxError, -1, "Catalog dictionary does not contain a valid \"Pages\" entry");
        return gFalse;
      }
    } else {
      error(errSyntaxError, -1, "Could not find catalog dictionary");
      return gFalse;
    }

    Object obj = catDict.dictLookup("Pages");
    // This should really be isDict("Pages"), but I've seen at least one
    // PDF file where the /Type entry is missing.
    if (!obj.isDict()) {
      error(errSyntaxError, -1, "Top-level pages object is wrong type ({0:s})", obj.getTypeName());
      return gFalse;
    }

    pagesSize = getNumPages();
    pages = (Page **)gmallocn_checkoverflow(pagesSize, sizeof(Page *));
    pageRefs = (Ref *)gmallocn_checkoverflow(pagesSize, sizeof(Ref));
    if (pages == NULL || pageRefs == NULL ) {
      error(errSyntaxError, -1, "Cannot allocate page cache");
      pagesSize = 0;
      return gFalse;
    }
    for (int i = 0; i < pagesSize; ++i) {
      pages[i] = NULL;
      pageRefs[i].num = -1;
      pageRefs[i].gen = -1;
    }

    attrsList = new std::vector<PageAttrs *>();
    attrsList->push_back(new PageAttrs(NULL, obj.getDict()));
    pagesList = new std::vector<Object>();
    pagesList->push_back(std::move(obj));
    pagesRefList = new std::vector<Ref>();
    pagesRefList->push_back(pagesRef);
    kidsIdxList = new std::vector<int>();
    kidsIdxList->push_back(0);
    lastCachedPage = 0;

  }

  while(1) {

    if (page <= lastCachedPage) return gTrue;

    if (pagesList->empty()) return gFalse;

    Object pagesDict = pagesList->back().copy();
    Object kids = pagesDict.dictLookup("Kids");
    if (!kids.isArray()) {
      error(errSyntaxError, -1, "Kids object (page {0:d}) is wrong type ({1:s})",
            lastCachedPage+1, kids.getTypeName());
      return gFalse;
    }

    int kidsIdx = kidsIdxList->back();
    if (kidsIdx >= kids.arrayGetLength()) {
       pagesList->pop_back();
       pagesRefList->pop_back();
       delete attrsList->back();
       attrsList->pop_back();
       kidsIdxList->pop_back();
       if (!kidsIdxList->empty()) kidsIdxList->back()++;
       continue;
    }

    Object kidRef = kids.arrayGetNF(kidsIdx);
    if (!kidRef.isRef()) {
      error(errSyntaxError, -1, "Kid object (page {0:d}) is not an indirect reference ({1:s})",
            lastCachedPage+1, kidRef.getTypeName());
      return gFalse;
    }

    GBool loop = gFalse;;
    for (size_t i = 0; i < pagesRefList->size(); i++) {
      if (((*pagesRefList)[i]).num == kidRef.getRefNum()) {
         loop = gTrue;
         break;
      }
    }
    if (loop) {
      error(errSyntaxError, -1, "Loop in Pages tree");
      kidsIdxList->back()++;
      continue;
    }

    Object kid = kids.arrayGet(kidsIdx);
    if (kid.isDict("Page") || (kid.isDict() && !kid.getDict()->hasKey("Kids"))) {
      PageAttrs *attrs = new PageAttrs(attrsList->back(), kid.getDict());
      Page *p = new Page(doc, lastCachedPage+1, &kid,
                     kidRef.getRef(), attrs, form);
      if (!p->isOk()) {
        error(errSyntaxError, -1, "Failed to create page (page {0:d})", lastCachedPage+1);
        delete p;
        return gFalse;
      }

      if (lastCachedPage >= numPages) {
        error(errSyntaxError, -1, "Page count in top-level pages object is incorrect");
        return gFalse;
      }

      pages[lastCachedPage] = p;
      pageRefs[lastCachedPage].num = kidRef.getRefNum();
      pageRefs[lastCachedPage].gen = kidRef.getRefGen();

      lastCachedPage++;
      kidsIdxList->back()++;

    // This should really be isDict("Pages"), but I've seen at least one
    // PDF file where the /Type entry is missing.
    } else if (kid.isDict()) {
      attrsList->push_back(new PageAttrs(attrsList->back(), kid.getDict()));
      pagesRefList->push_back(kidRef.getRef());
      pagesList->push_back(std::move(kid));
      kidsIdxList->push_back(0);
    } else {
      error(errSyntaxError, -1, "Kid object (page {0:d}) is wrong type ({1:s})",
            lastCachedPage+1, kid.getTypeName());
      kidsIdxList->back()++;
    }
  }

  return gFalse;
}

int Catalog::findPage(int num, int gen) {
  int i;

  for (i = 0; i < getNumPages(); ++i) {
    Ref *ref = getPageRef(i+1);
    if (ref != NULL && ref->num == num && ref->gen == gen)
      return i + 1;
  }
  return 0;
}

LinkDest *Catalog::findDest(GooString *name) {
  // try named destination dictionary then name tree
  if (getDests()->isDict()) {
    Object obj1 = getDests()->dictLookup(name->getCString());
    return createLinkDest(&obj1);
  }

  catalogLocker();
  Object obj2 = getDestNameTree()->lookup(name);
  return createLinkDest(&obj2);
}

LinkDest *Catalog::createLinkDest(Object *obj)
{
  LinkDest *dest = nullptr;
  if (obj->isArray()) {
    dest = new LinkDest(obj->getArray());
  } else if (obj->isDict()) {
    Object obj2 = obj->dictLookup("D");
    if (obj2.isArray())
      dest = new LinkDest(obj2.getArray());
    else
      error(errSyntaxWarning, -1, "Bad named destination value");
  } else {
    error(errSyntaxWarning, -1, "Bad named destination value");
  }
  if (dest && !dest->isOk()) {
    delete dest;
    dest = NULL;
  }

  return dest;
}

int Catalog::numDests()
{
  Object *obj;

  obj= getDests();
  if (!obj->isDict()) {
    return 0;
  }
  return obj->dictGetLength();
}

char *Catalog::getDestsName(int i)
{
  Object *obj;

  obj= getDests();
  if (!obj->isDict()) {
    return NULL;
  }
  return obj->dictGetKey(i);
}

LinkDest *Catalog::getDestsDest(int i)
{
  Object *obj = getDests();
  if (!obj->isDict()) {
    return NULL;
  }
  Object obj1 = obj->dictGetVal(i);
  return createLinkDest(&obj1);
}

LinkDest *Catalog::getDestNameTreeDest(int i)
{
  Object obj;

  catalogLocker();
  Object *aux = getDestNameTree()->getValue(i);
  if (aux) {
    obj = aux->fetch(xref);
  }
  return createLinkDest(&obj);
}

FileSpec *Catalog::embeddedFile(int i)
{
    Object efDict;
    catalogLocker();
    Object *obj = getEmbeddedFileNameTree()->getValue(i);
    FileSpec *embeddedFile = 0;
    if (obj->isRef()) {
      Object fsDict = obj->fetch(xref);
      embeddedFile = new FileSpec(&fsDict);
    } else if (obj->isDict()) {
      embeddedFile = new FileSpec(obj);
    } else {
      Object null;
      embeddedFile = new FileSpec(&null);
    }
    return embeddedFile;
}

GooString *Catalog::getJS(int i)
{
  Object obj;
  // getJSNameTree()->getValue(i) returns a shallow copy of the object so we
  // do not need to free it
  catalogLocker();
  Object *aux = getJSNameTree()->getValue(i);
  if (aux) {
    obj = aux->fetch(xref);
  }

  if (!obj.isDict()) {
    return nullptr;
  }
  Object obj2 = obj.dictLookup("S");
  if (!obj2.isName()) {
    return nullptr;
  }
  if (strcmp(obj2.getName(), "JavaScript")) {
    return nullptr;
  }
  obj2 = obj.dictLookup("JS");
  GooString *js = nullptr;
  if (obj2.isString()) {
    js = new GooString(obj2.getString());
  }
  else if (obj2.isStream()) {
    Stream *stream = obj2.getStream();
    js = new GooString();
    stream->fillGooString(js);
  }
  return js;
}

Catalog::PageMode Catalog::getPageMode() {

  catalogLocker();
  if (pageMode == pageModeNull) {

    pageMode = pageModeNone;

    Object catDict = xref->getCatalog();
    if (!catDict.isDict()) {
      error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
      return pageMode;
    }

    Object obj = catDict.dictLookup("PageMode");
    if (obj.isName()) {
      if (obj.isName("UseNone"))
        pageMode = pageModeNone;
      else if (obj.isName("UseOutlines"))
        pageMode = pageModeOutlines;
      else if (obj.isName("UseThumbs"))
        pageMode = pageModeThumbs;
      else if (obj.isName("FullScreen"))
        pageMode = pageModeFullScreen;
      else if (obj.isName("UseOC"))
        pageMode = pageModeOC;
      else if (obj.isName("UseAttachments"))
        pageMode = pageModeAttach;
    }
  }
  return pageMode;
}

Catalog::PageLayout Catalog::getPageLayout() {

  catalogLocker();
  if (pageLayout == pageLayoutNull) {

    pageLayout = pageLayoutNone;

    Object catDict = xref->getCatalog();
    if (!catDict.isDict()) {
      error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
      return pageLayout;
    }

    pageLayout = pageLayoutNone;
    Object obj = catDict.dictLookup("PageLayout");
    if (obj.isName()) {
      if (obj.isName("SinglePage"))
        pageLayout = pageLayoutSinglePage;
      if (obj.isName("OneColumn"))
        pageLayout = pageLayoutOneColumn;
      if (obj.isName("TwoColumnLeft"))
        pageLayout = pageLayoutTwoColumnLeft;
      if (obj.isName("TwoColumnRight"))
        pageLayout = pageLayoutTwoColumnRight;
      if (obj.isName("TwoPageLeft"))
        pageLayout = pageLayoutTwoPageLeft;
      if (obj.isName("TwoPageRight"))
        pageLayout = pageLayoutTwoPageRight;
    }
  }
  return pageLayout;
}

NameTree::NameTree()
{
  size = 0;
  length = 0;
  entries = NULL;
}

NameTree::~NameTree()
{
  int i;

  for (i = 0; i < length; i++)
    delete entries[i];

  gfree(entries);
}

NameTree::Entry::Entry(Array *array, int index) {
  if (!array->getString(index, &name)) {
    Object aux = array->get(index);
    if (aux.isString())
    {
      name.append(aux.getString());
    }
    else
      error(errSyntaxError, -1, "Invalid page tree");
  }
  value = array->getNF(index + 1);
}

NameTree::Entry::~Entry() {
}

void NameTree::addEntry(Entry *entry)
{
  if (length == size) {
    if (length == 0) {
      size = 8;
    } else {
      size *= 2;
    }
    entries = (Entry **) grealloc (entries, sizeof (Entry *) * size);
  }

  entries[length] = entry;
  ++length;
}

int NameTree::Entry::cmpEntry(const void *voidEntry, const void *voidOtherEntry)
{
  Entry *entry = *(NameTree::Entry **) voidEntry;
  Entry *otherEntry = *(NameTree::Entry **) voidOtherEntry;

  return entry->name.cmp(&otherEntry->name);
}

void NameTree::init(XRef *xrefA, Object *tree) {
  xref = xrefA;
  std::set<int> seen;
  parse(tree, seen);
  if (entries && length > 0) {
    qsort(entries, length, sizeof(Entry *), Entry::cmpEntry);
  }
}

void NameTree::parse(Object *tree, std::set<int> &seen) {
  if (!tree->isDict())
    return;

  // leaf node
  Object names = tree->dictLookup("Names");
  if (names.isArray()) {
    for (int i = 0; i < names.arrayGetLength(); i += 2) {
      NameTree::Entry *entry;

      entry = new Entry(names.getArray(), i);
      addEntry(entry);
    }
  }

  // root or intermediate node
  Object kids = tree->dictLookup("Kids");
  if (kids.isArray()) {
    for (int i = 0; i < kids.arrayGetLength(); ++i) {
      Object kidRef = kids.arrayGetNF(i);
      if (kidRef.isRef()) {
	const int numObj = kidRef.getRef().num;
	if (seen.find(numObj) != seen.end()) {
	  error(errSyntaxError, -1, "loop in NameTree (numObj: {0:d})", numObj);
	  continue;
	}
	seen.insert(numObj);
      }
      Object kid = kids.arrayGet(i);
      if (kid.isDict())
	parse(&kid, seen);
    }
  }
}

int NameTree::Entry::cmp(const void *voidKey, const void *voidEntry)
{
  GooString *key = (GooString *) voidKey;
  Entry *entry = *(NameTree::Entry **) voidEntry;

  return key->cmp(&entry->name);
}

Object NameTree::lookup(GooString *name)
{
  Entry **entry;

  entry = (Entry **) bsearch(name, entries,
			     length, sizeof(Entry *), Entry::cmp);
  if (entry != NULL) {
    return (*entry)->value.fetch(xref);
  } else {
    error(errSyntaxError, -1, "failed to look up ({0:s})", name->getCString());
    return Object(objNull);
  }
}

Object *NameTree::getValue(int index)
{
  if (index < length) {
    return &entries[index]->value;
  } else {
    return nullptr;
  }
}

GooString *NameTree::getName(int index)
{
    if (index < length) {
	return &entries[index]->name;
    } else {
	return NULL;
    }
}

GBool Catalog::labelToIndex(GooString *label, int *index)
{
  char *end;

  PageLabelInfo *pli = getPageLabelInfo();
  if (pli != NULL) {
    if (!pli->labelToIndex(label, index))
      return gFalse;
  } else {
    *index = strtol(label->getCString(), &end, 10) - 1;
    if (*end != '\0')
      return gFalse;
  }

  if (*index < 0 || *index >= getNumPages())
    return gFalse;

  return gTrue;
}

GBool Catalog::indexToLabel(int index, GooString *label)
{
  char buffer[32];

  if (index < 0 || index >= getNumPages())
    return gFalse;

  PageLabelInfo *pli = getPageLabelInfo();
  if (pli != NULL) {
    return pli->indexToLabel(index, label);
  } else {
    snprintf(buffer, sizeof (buffer), "%d", index + 1);
    label->append(buffer);	      
    return gTrue;
  }
}

int Catalog::getNumPages()
{
  catalogLocker();
  if (numPages == -1)
  {
    Object catDict = xref->getCatalog();
    if (!catDict.isDict()) {
      error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
      return 0;
    }
    Object pagesDict = catDict.dictLookup("Pages");

    // This should really be isDict("Pages"), but I've seen at least one
    // PDF file where the /Type entry is missing.
    if (!pagesDict.isDict()) {
      error(errSyntaxError, -1, "Top-level pages object is wrong type ({0:s})",
          pagesDict.getTypeName());
      return 0;
    }

    Object obj = pagesDict.dictLookup("Count");
    // some PDF files actually use real numbers here ("/Count 9.0")
    if (!obj.isNum()) {
      if (pagesDict.dictIs("Page")) {
	Object pageRootRef = catDict.dictLookupNF("Pages");

	error(errSyntaxError, -1, "Pages top-level is a single Page. The document is malformed, trying to recover...");

	Dict *pageDict = pagesDict.getDict();
	if (pageRootRef.isRef()) {
	  const Ref pageRef = pageRootRef.getRef();
	  Page *p = new Page(doc, 1, &pagesDict, pageRef, new PageAttrs(NULL, pageDict), form);
	  if (p->isOk()) {
	    pages = (Page **)gmallocn(1, sizeof(Page *));
	    pageRefs = (Ref *)gmallocn(1, sizeof(Ref));

	    pages[0] = p;
	    pageRefs[0].num = pageRef.num;
	    pageRefs[0].gen = pageRef.gen;

	    numPages = 1;
	    lastCachedPage = 1;
	    pagesSize = 1;
	  } else {
	    delete p;
	    numPages = 0;
	  }
	} else {
	  numPages = 0;
	}
      } else {
	error(errSyntaxError, -1, "Page count in top-level pages object is wrong type ({0:s})",
	  obj.getTypeName());
	numPages = 0;
      }
    } else {
      numPages = (int)obj.getNum();
      if (numPages <= 0) {
        error(errSyntaxError, -1,
              "Invalid page count {0:d}", numPages);
        numPages = 0;
      } else if (numPages > xref->getNumObjects()) {
        error(errSyntaxError, -1,
              "Page count ({0:d}) larger than number of objects ({1:d})",
              numPages, xref->getNumObjects());
        numPages = 0;
      }

    }
  }

  return numPages;
}

PageLabelInfo *Catalog::getPageLabelInfo()
{
  catalogLocker();
  if (!pageLabelInfo) {
    Object catDict = xref->getCatalog();
    if (!catDict.isDict()) {
      error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
      return nullptr;
    }

    Object obj = catDict.dictLookup("PageLabels");
    if (obj.isDict()) {
      pageLabelInfo = new PageLabelInfo(&obj, getNumPages());
    }
  }

  return pageLabelInfo;
}

StructTreeRoot *Catalog::getStructTreeRoot()
{
  catalogLocker();
  if (!structTreeRoot) {
    Object catalog = xref->getCatalog();
    if (!catalog.isDict()) {
      error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catalog.getTypeName());
      return NULL;
    }

    Object root = catalog.dictLookup("StructTreeRoot");
    if (root.isDict("StructTreeRoot")) {
      structTreeRoot = new StructTreeRoot(doc, root.getDict());
    }
  }
  return structTreeRoot;
}

Guint Catalog::getMarkInfo()
{
  if (markInfo == markInfoNull) {
    markInfo = 0;

    catalogLocker();
    Object catDict = xref->getCatalog();

    if (catDict.isDict()) {
      Object markInfoDict = catDict.dictLookup("MarkInfo");
      if (markInfoDict.isDict()) {
        Object value = markInfoDict.dictLookup("Marked");
        if (value.isBool() && value.getBool())
          markInfo |= markInfoMarked;
        else if (!value.isNull())
          error(errSyntaxError, -1, "Marked object is wrong type ({0:s})", value.getTypeName());

        value = markInfoDict.dictLookup("Suspects");
        if (value.isBool() && value.getBool())
          markInfo |= markInfoSuspects;
        else if (!value.isNull())
          error(errSyntaxError, -1, "Suspects object is wrong type ({0:s})", value.getTypeName());

        value = markInfoDict.dictLookup("UserProperties");
        if (value.isBool() && value.getBool())
          markInfo |= markInfoUserProperties;
        else if (!value.isNull())
          error(errSyntaxError, -1, "UserProperties object is wrong type ({0:s})", value.getTypeName());
      } else if (!markInfoDict.isNull()) {
        error(errSyntaxError, -1, "MarkInfo object is wrong type ({0:s})", markInfoDict.getTypeName());
      }
    } else {
      error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
    }
  }
  return markInfo;
}

Object *Catalog::getOutline()
{
  catalogLocker();
  if (outline.isNone())
  {
     Object catDict = xref->getCatalog();
     if (catDict.isDict()) {
       outline = catDict.dictLookup("Outlines");
     } else {
       error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
       outline.setToNull();
     }
  }

  return &outline;
}

Object *Catalog::getDests()
{
  catalogLocker();
  if (dests.isNone())
  {
     Object catDict = xref->getCatalog();
     if (catDict.isDict()) {
       dests = catDict.dictLookup("Dests");
     } else {
       error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
       dests.setToNull();
     }
  }

  return &dests;
}

Catalog::FormType Catalog::getFormType()
{
  Object xfa;
  FormType res = NoForm;

  if (acroForm.isDict()) {
    xfa = acroForm.dictLookup("XFA");
    if (xfa.isStream() || xfa.isArray()) {
      res = XfaForm;
    } else {
      res = AcroForm;
    }
  }

  return res;
}

Form *Catalog::getForm()
{
  catalogLocker();
  if (!form) {
    if (acroForm.isDict()) {
      form = new Form(doc, &acroForm);
      // perform form-related loading after all widgets have been loaded
      form->postWidgetsLoad();
    }
  }

  return form;
}

ViewerPreferences *Catalog::getViewerPreferences()
{
  catalogLocker();
  if (!viewerPrefs) {
    if (viewerPreferences.isDict()) {
      viewerPrefs = new ViewerPreferences(viewerPreferences.getDict());
    }
  }

  return viewerPrefs;
}

Object *Catalog::getNames()
{
  if (names.isNone())
  {
     Object catDict = xref->getCatalog();
     if (catDict.isDict()) {
       names = catDict.dictLookup("Names");
     } else {
       error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
       names.setToNull();
     }
  }

  return &names;
}

NameTree *Catalog::getDestNameTree()
{
  if (!destNameTree) {

    destNameTree = new NameTree();

    if (getNames()->isDict()) {
       Object obj = getNames()->dictLookup("Dests");
       destNameTree->init(xref, &obj);
    }

  }

  return destNameTree;
}

NameTree *Catalog::getEmbeddedFileNameTree()
{
  if (!embeddedFileNameTree) {

    embeddedFileNameTree = new NameTree();

    if (getNames()->isDict()) {
       Object obj = getNames()->dictLookup("EmbeddedFiles");
       embeddedFileNameTree->init(xref, &obj);
    }

  }

  return embeddedFileNameTree;
}

NameTree *Catalog::getJSNameTree()
{
  if (!jsNameTree) {

    jsNameTree = new NameTree();

    if (getNames()->isDict()) {
       Object obj = getNames()->dictLookup("JavaScript");
       jsNameTree->init(xref, &obj);
    }

  }

  return jsNameTree;
}

LinkAction* Catalog::getAdditionalAction(DocumentAdditionalActionsType type) {
  LinkAction *linkAction = nullptr;
  Object additionalActionsObject = additionalActions.fetch(doc->getXRef());
  if (additionalActionsObject.isDict()) {
    const char *key = (type == actionCloseDocument ?       "WC" :
                       type == actionSaveDocumentStart ?   "WS" :
                       type == actionSaveDocumentFinish ?  "DS" :
                       type == actionPrintDocumentStart ?  "WP" :
                       type == actionPrintDocumentFinish ? "DP" : NULL);

    Object actionObject = additionalActionsObject.dictLookup(key);
    if (actionObject.isDict())
      linkAction = LinkAction::parseAction(&actionObject, doc->getCatalog()->getBaseURI());
  }
  return linkAction;
}
