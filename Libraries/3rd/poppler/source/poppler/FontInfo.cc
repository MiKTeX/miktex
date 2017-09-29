//========================================================================
//
// FontInfo.cc
//
// Copyright (C) 2005, 2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2005-2008, 2010, 2017 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2005 Brad Hards <bradh@frogmouth.net>
// Copyright (C) 2006 Kouhei Sutou <kou@cozmixng.org>
// Copyright (C) 2009 Pino Toscano <pino@kde.org>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2010, 2012 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2010, 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

//========================================================================
//
// Based on code from pdffonts.cc
//
// Copyright 2001-2007 Glyph & Cog, LLC
//
//========================================================================

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "GlobalParams.h"
#include "Error.h"
#include "Object.h"
#include "Dict.h"
#include "GfxFont.h"
#include "Annot.h"
#include "PDFDoc.h"
#include "FontInfo.h"

FontInfoScanner::FontInfoScanner(PDFDoc *docA, int firstPage) {
  doc = docA;
  currentPage = firstPage + 1;
}

FontInfoScanner::~FontInfoScanner() {
}

GooList *FontInfoScanner::scan(int nPages) {
  GooList *result;
  Page *page;
  Dict *resDict;
  Annots *annots;
  int lastPage;

  if (currentPage > doc->getNumPages()) {
    return NULL;
  }
 
  result = new GooList();

  lastPage = currentPage + nPages;
  if (lastPage > doc->getNumPages() + 1) {
    lastPage = doc->getNumPages() + 1;
  }

  XRef *xrefA = doc->getXRef()->copy();
  for (int pg = currentPage; pg < lastPage; ++pg) {
    page = doc->getPage(pg);
    if (!page) continue;

    if ((resDict = page->getResourceDictCopy(xrefA))) {
      scanFonts(xrefA, resDict, result);
      delete resDict;
    }
    annots = page->getAnnots();
    for (int i = 0; i < annots->getNumAnnots(); ++i) {
      Object obj1 = annots->getAnnot(i)->getAppearanceResDict();
      if (obj1.isDict()) {
        scanFonts(xrefA, obj1.getDict(), result);
      }
    }
  }

  currentPage = lastPage;

  delete xrefA;
  return result;
}

void FontInfoScanner::scanFonts(XRef *xrefA, Dict *resDict, GooList *fontsList) {
  Ref r;
  GfxFontDict *gfxFontDict;
  GfxFont *font;

  // scan the fonts in this resource dictionary
  gfxFontDict = NULL;
  Object obj1 = resDict->lookupNF("Font");
  if (obj1.isRef()) {
    Object obj2 = obj1.fetch(xrefA);
    if (obj2.isDict()) {
      r = obj1.getRef();
      gfxFontDict = new GfxFontDict(xrefA, &r, obj2.getDict());
    }
  } else if (obj1.isDict()) {
    gfxFontDict = new GfxFontDict(xrefA, NULL, obj1.getDict());
  }
  if (gfxFontDict) {
    for (int i = 0; i < gfxFontDict->getNumFonts(); ++i) {
      if ((font = gfxFontDict->getFont(i))) {
        Ref fontRef = *font->getID();

        // add this font to the list if not already found
        if (fonts.find(fontRef.num) == fonts.end()) {
          fontsList->append(new FontInfo(font, xrefA));
          fonts.insert(fontRef.num);
        }
      }
    }
    delete gfxFontDict;
  }

  // recursively scan any resource dictionaries in objects in this
  // resource dictionary
  const char *resTypes[] = { "XObject", "Pattern" };
  for (Guint resType = 0; resType < sizeof(resTypes) / sizeof(resTypes[0]); ++resType) {
    Object objDict = resDict->lookup(resTypes[resType]);
    if (objDict.isDict()) {
      for (int i = 0; i < objDict.dictGetLength(); ++i) {
        obj1 = objDict.dictGetValNF(i);
        if (obj1.isRef()) {
          // check for an already-seen object
          const Ref r = obj1.getRef();
          if (visitedObjects.find(r.num) != visitedObjects.end()) {
            continue;
          }

          visitedObjects.insert(r.num);
        }

        Object obj2 = obj1.fetch(xrefA);
        if (obj2.isStream()) {
          Object resObj = obj2.streamGetDict()->lookup("Resources");
          if (resObj.isDict() && resObj.getDict() != resDict) {
            scanFonts(xrefA, resObj.getDict(), fontsList);
          }
        }
      }
    }
  }
}

FontInfo::FontInfo(GfxFont *font, XRef *xref) {
  GooString *origName;

  fontRef = *font->getID();

  // font name
  origName = font->getName();
  if (origName != NULL) {
    name = font->getName()->copy();
  } else {
    name = NULL;
  }

  // font type
  type = (FontInfo::Type)font->getType();

  // check for an embedded font
  if (font->getType() == fontType3) {
    emb = gTrue;
  } else {
    emb = font->getEmbeddedFontID(&embRef);
  }

  file = NULL;
  substituteName = NULL;
  if (!emb)
  {
    SysFontType dummy;
    int dummy2;
    GooString substituteNameAux;
    file = globalParams->findSystemFontFile(font, &dummy, &dummy2, &substituteNameAux);
    if (substituteNameAux.getLength() > 0)
	substituteName = substituteNameAux.copy();
  }
  encoding = font->getEncodingName()->copy();

  // look for a ToUnicode map
  hasToUnicode = gFalse;
  Object fontObj = xref->fetch(fontRef.num, fontRef.gen);
  if (fontObj.isDict()) {
    hasToUnicode = fontObj.dictLookup("ToUnicode").isStream();
  }

  // check for a font subset name: capital letters followed by a '+'
  // sign
  subset = gFalse;
  if (name) {
    int i;
    for (i = 0; i < name->getLength(); ++i) {
      if (name->getChar(i) < 'A' || name->getChar(i) > 'Z') {
	break;
      }
    }
    subset = i > 0 && i < name->getLength() && name->getChar(i) == '+';
  }
}

FontInfo::FontInfo(FontInfo& f) {
  name = f.name ? f.name->copy() : NULL;
  file = f.file ? f.file->copy() : NULL;
  encoding = f.encoding ? f.encoding->copy() : NULL;
  substituteName = f.substituteName ? f.substituteName->copy() : NULL;
  type = f.type;
  emb = f.emb;
  subset = f.subset;
  hasToUnicode = f.hasToUnicode;
  fontRef = f.fontRef;
  embRef = f.embRef;
}

FontInfo::~FontInfo() {
  delete name;
  delete file;
  delete encoding;
  if (substituteName)
    delete substituteName;
}
