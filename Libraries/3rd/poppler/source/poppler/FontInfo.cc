//========================================================================
//
// FontInfo.cc
//
// Copyright (C) 2005, 2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2005-2008, 2010, 2017-2020, 2023 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2005 Brad Hards <bradh@frogmouth.net>
// Copyright (C) 2006 Kouhei Sutou <kou@cozmixng.org>
// Copyright (C) 2009 Pino Toscano <pino@kde.org>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2010, 2012 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2010, 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018, 2019 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019, 2021, 2022 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright (C) 2023 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
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
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <cmath>
#include "GlobalParams.h"
#include "Error.h"
#include "Object.h"
#include "Dict.h"
#include "GfxFont.h"
#include "Annot.h"
#include "PDFDoc.h"
#include "FontInfo.h"

FontInfoScanner::FontInfoScanner(PDFDoc *docA, int firstPage)
{
    doc = docA;
    currentPage = firstPage + 1;
}

FontInfoScanner::~FontInfoScanner() { }

std::vector<FontInfo *> FontInfoScanner::scan(int nPages)
{
    Page *page;
    Dict *resDict;
    Annots *annots;
    int lastPage;

    std::vector<FontInfo *> result;

    if (currentPage > doc->getNumPages()) {
        return result;
    }

    lastPage = currentPage + nPages;
    if (lastPage > doc->getNumPages() + 1) {
        lastPage = doc->getNumPages() + 1;
    }

    std::unique_ptr<XRef> xrefA(doc->getXRef()->copy());
    for (int pg = currentPage; pg < lastPage; ++pg) {
        page = doc->getPage(pg);
        if (!page) {
            continue;
        }

        if ((resDict = page->getResourceDictCopy(xrefA.get()))) {
            scanFonts(xrefA.get(), resDict, &result);
            delete resDict;
        }
        annots = page->getAnnots();
        for (Annot *annot : annots->getAnnots()) {
            Object obj1 = annot->getAppearanceResDict();
            if (obj1.isDict()) {
                scanFonts(xrefA.get(), obj1.getDict(), &result);
            }
        }
    }

    currentPage = lastPage;

    return result;
}

void FontInfoScanner::scanFonts(XRef *xrefA, Dict *resDict, std::vector<FontInfo *> *fontsList)
{
    GfxFontDict *gfxFontDict;

    // scan the fonts in this resource dictionary
    gfxFontDict = nullptr;
    const Object &fontObj = resDict->lookupNF("Font");
    if (fontObj.isRef()) {
        Object obj2 = fontObj.fetch(xrefA);
        if (obj2.isDict()) {
            Ref r = fontObj.getRef();
            gfxFontDict = new GfxFontDict(xrefA, &r, obj2.getDict());
        }
    } else if (fontObj.isDict()) {
        gfxFontDict = new GfxFontDict(xrefA, nullptr, fontObj.getDict());
    }
    if (gfxFontDict) {
        for (int i = 0; i < gfxFontDict->getNumFonts(); ++i) {
            if (const std::shared_ptr<GfxFont> &font = gfxFontDict->getFont(i)) {
                Ref fontRef = *font->getID();

                // add this font to the list if not already found
                if (fonts.insert(fontRef.num).second) {
                    fontsList->push_back(new FontInfo(font.get(), xrefA));
                }
            }
        }
        delete gfxFontDict;
    }

    // recursively scan any resource dictionaries in objects in this
    // resource dictionary
    const char *resTypes[] = { "XObject", "Pattern" };
    for (const char *resType : resTypes) {
        Ref objDictRef;
        Object objDict = resDict->lookup(resType, &objDictRef);
        if (!visitedObjects.insert(objDictRef)) {
            continue;
        }
        if (objDict.isDict()) {
            for (int i = 0; i < objDict.dictGetLength(); ++i) {
                Ref obj2Ref;
                const Object obj2 = objDict.getDict()->getVal(i, &obj2Ref);
                // check for an already-seen object
                if (!visitedObjects.insert(obj2Ref)) {
                    continue;
                }

                if (obj2.isStream()) {
                    Ref resourcesRef;
                    const Object resObj = obj2.streamGetDict()->lookup("Resources", &resourcesRef);
                    if (!visitedObjects.insert(resourcesRef)) {
                        continue;
                    }

                    if (resObj.isDict() && resObj.getDict() != resDict) {
                        scanFonts(xrefA, resObj.getDict(), fontsList);
                    }
                }
            }
        }
    }
}

FontInfo::FontInfo(GfxFont *font, XRef *xref)
{
    fontRef = *font->getID();

    // font name
    const std::optional<std::string> &origName = font->getName();
    if (origName) {
        name = *font->getName();
    }

    // font type
    type = (FontInfo::Type)font->getType();

    // check for an embedded font
    if (font->getType() == fontType3) {
        emb = true;
        embRef = Ref::INVALID();
    } else {
        emb = font->getEmbeddedFontID(&embRef);
    }

    if (!emb) {
        GooString substituteNameAux;
        const std::optional<GfxFontLoc> fontLoc = font->locateFont(xref, nullptr, &substituteNameAux);
        if (fontLoc && fontLoc->locType == gfxFontLocExternal) {
            file = fontLoc->path;
        }
        if (substituteNameAux.getLength() > 0) {
            substituteName = substituteNameAux.toStr();
        }
    }
    encoding = font->getEncodingName();

    // look for a ToUnicode map
    hasToUnicode = false;
    Object fontObj = xref->fetch(fontRef);
    if (fontObj.isDict()) {
        hasToUnicode = fontObj.dictLookup("ToUnicode").isStream();
    }

    // check for a font subset name: capital letters followed by a '+'
    // sign
    subset = font->isSubset();
}
