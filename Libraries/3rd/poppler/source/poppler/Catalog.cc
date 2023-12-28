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
// Copyright (C) 2005-2013, 2015, 2017-2023 Albert Astals Cid <aacid@kde.org>
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
// Copyright (C) 2013, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 José Aliste <jaliste@src.gnome.org>
// Copyright (C) 2014 Ed Porras <ed@moto-research.com>
// Copyright (C) 2015 Even Rouault <even.rouault@spatialys.com>
// Copyright (C) 2016 Masamichi Hosoda <trueroad@trueroad.jp>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2020 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright (C) 2020 Katarina Behrens <Katarina.Behrens@cib.de>
// Copyright (C) 2020 Thorsten Behrens <Thorsten.Behrens@CIB.de>
// Copyright (C) 2020 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by Technische Universität Dresden
// Copyright (C) 2021 RM <rm+git@arcsin.org>
// Copyright (C) 2023 Ilaï Deutel <idtl@google.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <cstddef>
#include <cstdlib>
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

//------------------------------------------------------------------------
// Catalog
//------------------------------------------------------------------------

#define catalogLocker() const std::scoped_lock locker(mutex)

Catalog::Catalog(PDFDoc *docA)
{
    ok = true;
    doc = docA;
    xref = doc->getXRef();
    numPages = -1;
    pageLabelInfo = nullptr;
    form = nullptr;
    optContent = nullptr;
    pageMode = pageModeNull;
    pageLayout = pageLayoutNull;
    destNameTree = nullptr;
    embeddedFileNameTree = nullptr;
    jsNameTree = nullptr;
    viewerPrefs = nullptr;
    structTreeRoot = nullptr;

    pagesList = nullptr;
    pagesRefList = nullptr;
    attrsList = nullptr;
    kidsIdxList = nullptr;
    markInfo = markInfoNull;

    Object catDict = xref->getCatalog();
    if (!catDict.isDict()) {
        error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
        ok = false;
        return;
    }
    // get the AcroForm dictionary
    acroForm = catDict.getDict()->lookup("AcroForm");

    // read base URI
    Object obj = catDict.getDict()->lookupEnsureEncryptedIfNeeded("URI");
    if (obj.isDict()) {
        Object obj2 = obj.getDict()->lookupEnsureEncryptedIfNeeded("Base");
        if (obj2.isString()) {
            baseURI = obj2.getString()->toStr();
        }
    }

    // get the Optional Content dictionary
    Object optContentProps = catDict.dictLookup("OCProperties");
    if (optContentProps.isDict()) {
        optContent = new OCGs(&optContentProps, xref);
        if (!optContent->isOk()) {
            delete optContent;
            optContent = nullptr;
        }
    }

    // actions
    additionalActions = catDict.dictLookupNF("AA").copy();

    // get the ViewerPreferences dictionary
    viewerPreferences = catDict.dictLookup("ViewerPreferences");

    const Object version = catDict.dictLookup("Version");
    if (version.isName()) {
        const int res = sscanf(version.getName(), "%d.%d", &catalogPdfMajorVersion, &catalogPdfMinorVersion);
        if (res != 2) {
            catalogPdfMajorVersion = -1;
            catalogPdfMinorVersion = -1;
        }
    }
}

Catalog::~Catalog()
{
    delete kidsIdxList;
    if (attrsList) {
        std::vector<PageAttrs *>::iterator it;
        for (it = attrsList->begin(); it != attrsList->end(); ++it) {
            delete *it;
        }
        delete attrsList;
    }
    delete pagesRefList;
    delete pagesList;
    delete destNameTree;
    delete embeddedFileNameTree;
    delete jsNameTree;
    delete pageLabelInfo;
    delete form;
    delete optContent;
    delete viewerPrefs;
    delete structTreeRoot;
}

std::unique_ptr<GooString> Catalog::readMetadata()
{
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
        return {};
    }
    Object obj = metadata.streamGetDict()->lookup("Subtype");
    if (!obj.isName("XML")) {
        error(errSyntaxWarning, -1, "Unknown Metadata type: '{0:s}'", obj.isName() ? obj.getName() : "???");
    }
    std::unique_ptr<GooString> s = std::make_unique<GooString>();
    metadata.getStream()->fillGooString(s.get());
    metadata.streamClose();
    return s;
}

Page *Catalog::getPage(int i)
{
    if (i < 1) {
        return nullptr;
    }

    catalogLocker();
    if (std::size_t(i) > pages.size()) {
        bool cached = cachePageTree(i);
        if (cached == false) {
            return nullptr;
        }
    }
    return pages[i - 1].first.get();
}

Ref *Catalog::getPageRef(int i)
{
    if (i < 1) {
        return nullptr;
    }

    catalogLocker();
    if (std::size_t(i) > pages.size()) {
        bool cached = cachePageTree(i);
        if (cached == false) {
            return nullptr;
        }
    }
    return &pages[i - 1].second;
}

bool Catalog::cachePageTree(int page)
{
    if (pagesList == nullptr) {

        Ref pagesRef;

        Object catDict = xref->getCatalog();

        if (catDict.isDict()) {
            const Object &pagesDictRef = catDict.dictLookupNF("Pages");
            if (pagesDictRef.isRef() && pagesDictRef.getRefNum() >= 0 && pagesDictRef.getRefNum() < xref->getNumObjects()) {
                pagesRef = pagesDictRef.getRef();
            } else {
                error(errSyntaxError, -1, "Catalog dictionary does not contain a valid \"Pages\" entry");
                return false;
            }
        } else {
            error(errSyntaxError, -1, "Could not find catalog dictionary");
            return false;
        }

        Object obj = catDict.dictLookup("Pages");
        // This should really be isDict("Pages"), but I've seen at least one
        // PDF file where the /Type entry is missing.
        if (!obj.isDict()) {
            error(errSyntaxError, -1, "Top-level pages object is wrong type ({0:s})", obj.getTypeName());
            return false;
        }

        pages.clear();
        attrsList = new std::vector<PageAttrs *>();
        attrsList->push_back(new PageAttrs(nullptr, obj.getDict()));
        pagesList = new std::vector<Object>();
        pagesList->push_back(std::move(obj));
        pagesRefList = new std::vector<Ref>();
        pagesRefList->push_back(pagesRef);
        kidsIdxList = new std::vector<int>();
        kidsIdxList->push_back(0);
    }

    while (true) {

        if (std::size_t(page) <= pages.size()) {
            return true;
        }

        if (pagesList->empty()) {
            return false;
        }

        Object kids = pagesList->back().dictLookup("Kids");
        if (!kids.isArray()) {
            error(errSyntaxError, -1, "Kids object (page {0:uld}) is wrong type ({1:s})", pages.size() + 1, kids.getTypeName());
            return false;
        }

        int kidsIdx = kidsIdxList->back();
        if (kidsIdx >= kids.arrayGetLength()) {
            pagesList->pop_back();
            pagesRefList->pop_back();
            delete attrsList->back();
            attrsList->pop_back();
            kidsIdxList->pop_back();
            if (!kidsIdxList->empty()) {
                kidsIdxList->back()++;
            }
            continue;
        }

        const Object &kidRef = kids.arrayGetNF(kidsIdx);
        if (!kidRef.isRef()) {
            error(errSyntaxError, -1, "Kid object (page {0:uld}) is not an indirect reference ({1:s})", pages.size() + 1, kidRef.getTypeName());
            return false;
        }

        bool loop = false;
        ;
        for (const Ref &pageRef : *pagesRefList) {
            if (pageRef.num == kidRef.getRefNum()) {
                loop = true;
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
            auto p = std::make_unique<Page>(doc, pages.size() + 1, std::move(kid), kidRef.getRef(), attrs, form);
            if (!p->isOk()) {
                error(errSyntaxError, -1, "Failed to create page (page {0:uld})", pages.size() + 1);
                return false;
            }

            if (pages.size() >= std::size_t(numPages)) {
                error(errSyntaxError, -1, "Page count in top-level pages object is incorrect");
                return false;
            }

            pages.emplace_back(std::move(p), kidRef.getRef());

            kidsIdxList->back()++;

            // This should really be isDict("Pages"), but I've seen at least one
            // PDF file where the /Type entry is missing.
        } else if (kid.isDict()) {
            attrsList->push_back(new PageAttrs(attrsList->back(), kid.getDict()));
            pagesRefList->push_back(kidRef.getRef());
            pagesList->push_back(std::move(kid));
            kidsIdxList->push_back(0);
        } else {
            error(errSyntaxError, -1, "Kid object (page {0:uld}) is wrong type ({1:s})", pages.size() + 1, kid.getTypeName());
            kidsIdxList->back()++;
        }
    }

    return false;
}

int Catalog::findPage(const Ref pageRef)
{
    int i;

    for (i = 0; i < getNumPages(); ++i) {
        Ref *ref = getPageRef(i + 1);
        if (ref != nullptr && *ref == pageRef) {
            return i + 1;
        }
    }
    return 0;
}

std::unique_ptr<LinkDest> Catalog::findDest(const GooString *name)
{
    // try named destination dictionary then name tree
    if (getDests()->isDict()) {
        Object obj1 = getDests()->dictLookup(name->c_str());
        return createLinkDest(&obj1);
    }

    catalogLocker();
    Object obj2 = getDestNameTree()->lookup(name);
    return createLinkDest(&obj2);
}

std::unique_ptr<LinkDest> Catalog::createLinkDest(Object *obj)
{
    std::unique_ptr<LinkDest> dest;
    if (obj->isArray()) {
        dest = std::make_unique<LinkDest>(obj->getArray());
    } else if (obj->isDict()) {
        Object obj2 = obj->dictLookup("D");
        if (obj2.isArray()) {
            dest = std::make_unique<LinkDest>(obj2.getArray());
        } else {
            error(errSyntaxWarning, -1, "Bad named destination value");
        }
    } else {
        error(errSyntaxWarning, -1, "Bad named destination value");
    }
    if (dest && !dest->isOk()) {
        dest.reset();
    }

    return dest;
}

int Catalog::numDests()
{
    Object *obj;

    obj = getDests();
    if (!obj->isDict()) {
        return 0;
    }
    return obj->dictGetLength();
}

const char *Catalog::getDestsName(int i)
{
    Object *obj;

    obj = getDests();
    if (!obj->isDict()) {
        return nullptr;
    }
    return obj->dictGetKey(i);
}

std::unique_ptr<LinkDest> Catalog::getDestsDest(int i)
{
    Object *obj = getDests();
    if (!obj->isDict()) {
        return nullptr;
    }
    Object obj1 = obj->dictGetVal(i);
    return createLinkDest(&obj1);
}

std::unique_ptr<LinkDest> Catalog::getDestNameTreeDest(int i)
{
    Object obj;

    catalogLocker();
    Object *aux = getDestNameTree()->getValue(i);
    if (aux) {
        obj = aux->fetch(xref);
    }
    return createLinkDest(&obj);
}

std::unique_ptr<FileSpec> Catalog::embeddedFile(int i)
{
    catalogLocker();
    Object *obj = getEmbeddedFileNameTree()->getValue(i);
    if (obj->isRef()) {
        Object fsDict = obj->fetch(xref);
        return std::make_unique<FileSpec>(&fsDict);
    } else if (obj->isDict()) {
        return std::make_unique<FileSpec>(obj);
    } else {
        Object null;
        return std::make_unique<FileSpec>(&null);
    }
}

bool Catalog::hasEmbeddedFile(const std::string &fileName)
{
    NameTree *ef = getEmbeddedFileNameTree();
    for (int i = 0; i < ef->numEntries(); ++i) {
        if (fileName == ef->getName(i)->toStr()) {
            return true;
        }
    }
    return false;
}

void Catalog::addEmbeddedFile(GooFile *file, const std::string &fileName)
{
    catalogLocker();

    const Ref fileSpecRef = xref->addIndirectObject(FileSpec::newFileSpecObject(xref, file, fileName));

    Object catDict = xref->getCatalog();
    Ref namesObjRef;
    Object namesObj = catDict.getDict()->lookup("Names", &namesObjRef);
    if (!namesObj.isDict()) {
        // Need to create the names Dict
        catDict.dictSet("Names", Object(new Dict(xref)));
        namesObj = catDict.getDict()->lookup("Names");

        // Trigger getting the names dict again when needed
        names = Object();
    }

    Dict *namesDict = namesObj.getDict();

    // We create a new EmbeddedFiles nametree, this replaces the existing one (if any), but it's not a problem
    Object embeddedFilesObj = Object(new Dict(xref));
    const Ref embeddedFilesRef = xref->addIndirectObject(embeddedFilesObj);

    Array *embeddedFilesNamesArray = new Array(xref);

    // This flattens out the existing EmbeddedFiles nametree (if any), should not be a problem
    NameTree *ef = getEmbeddedFileNameTree();
    bool fileAlreadyAdded = false;
    for (int i = 0; i < ef->numEntries(); ++i) {
        const GooString *efNameI = ef->getName(i);

        // we need to add the file if it has not been added yet and the name is smaller or equal lexicographically
        // than the current item
        const bool sameFileName = fileName == efNameI->toStr();
        const bool addFile = !fileAlreadyAdded && (sameFileName || fileName < efNameI->toStr());
        if (addFile) {
            // If the new name is smaller lexicographically than an existing file add it in its correct position
            embeddedFilesNamesArray->add(Object(new GooString(fileName)));
            embeddedFilesNamesArray->add(Object(fileSpecRef));
            fileAlreadyAdded = true;
        }
        if (sameFileName) {
            // If the new name is the same lexicographically than an existing file then don't add the existing file (i.e. replace)
            continue;
        }
        embeddedFilesNamesArray->add(Object(efNameI->copy()));
        embeddedFilesNamesArray->add(ef->getValue(i)->copy());
    }

    if (!fileAlreadyAdded) {
        // The new file is bigger lexicographically than the existing ones
        embeddedFilesNamesArray->add(Object(new GooString(fileName)));
        embeddedFilesNamesArray->add(Object(fileSpecRef));
    }

    embeddedFilesObj.dictSet("Names", Object(embeddedFilesNamesArray));
    namesDict->set("EmbeddedFiles", Object(embeddedFilesRef));

    if (namesObjRef != Ref::INVALID()) {
        xref->setModifiedObject(&namesObj, namesObjRef);
    } else {
        xref->setModifiedObject(&catDict, { xref->getRootNum(), xref->getRootGen() });
    }

    // recreate Nametree on next call that uses it
    delete embeddedFileNameTree;
    embeddedFileNameTree = nullptr;
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
    } else if (obj2.isStream()) {
        Stream *stream = obj2.getStream();
        js = new GooString();
        stream->fillGooString(js);
    }
    return js;
}

Catalog::PageMode Catalog::getPageMode()
{

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
            if (obj.isName("UseNone")) {
                pageMode = pageModeNone;
            } else if (obj.isName("UseOutlines")) {
                pageMode = pageModeOutlines;
            } else if (obj.isName("UseThumbs")) {
                pageMode = pageModeThumbs;
            } else if (obj.isName("FullScreen")) {
                pageMode = pageModeFullScreen;
            } else if (obj.isName("UseOC")) {
                pageMode = pageModeOC;
            } else if (obj.isName("UseAttachments")) {
                pageMode = pageModeAttach;
            }
        }
    }
    return pageMode;
}

Catalog::PageLayout Catalog::getPageLayout()
{

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
            if (obj.isName("SinglePage")) {
                pageLayout = pageLayoutSinglePage;
            }
            if (obj.isName("OneColumn")) {
                pageLayout = pageLayoutOneColumn;
            }
            if (obj.isName("TwoColumnLeft")) {
                pageLayout = pageLayoutTwoColumnLeft;
            }
            if (obj.isName("TwoColumnRight")) {
                pageLayout = pageLayoutTwoColumnRight;
            }
            if (obj.isName("TwoPageLeft")) {
                pageLayout = pageLayoutTwoPageLeft;
            }
            if (obj.isName("TwoPageRight")) {
                pageLayout = pageLayoutTwoPageRight;
            }
        }
    }
    return pageLayout;
}

NameTree::NameTree()
{
    size = 0;
    length = 0;
    entries = nullptr;
}

NameTree::~NameTree()
{
    int i;

    for (i = 0; i < length; i++) {
        delete entries[i];
    }

    gfree(entries);
}

NameTree::Entry::Entry(Array *array, int index)
{
    if (!array->getString(index, &name)) {
        Object aux = array->get(index);
        if (aux.isString()) {
            name.append(aux.getString());
        } else {
            error(errSyntaxError, -1, "Invalid page tree");
        }
    }
    value = array->getNF(index + 1).copy();
}

NameTree::Entry::~Entry() { }

void NameTree::addEntry(Entry *entry)
{
    if (length == size) {
        if (length == 0) {
            size = 8;
        } else {
            size *= 2;
        }
        entries = (Entry **)grealloc(entries, sizeof(Entry *) * size);
    }

    entries[length] = entry;
    ++length;
}

int NameTree::Entry::cmpEntry(const void *voidEntry, const void *voidOtherEntry)
{
    Entry *entry = *(NameTree::Entry **)voidEntry;
    Entry *otherEntry = *(NameTree::Entry **)voidOtherEntry;

    return entry->name.cmp(&otherEntry->name);
}

void NameTree::init(XRef *xrefA, Object *tree)
{
    xref = xrefA;
    RefRecursionChecker seen;
    parse(tree, seen);
    if (entries && length > 0) {
        qsort(entries, length, sizeof(Entry *), Entry::cmpEntry);
    }
}

void NameTree::parse(const Object *tree, RefRecursionChecker &seen)
{
    if (!tree->isDict()) {
        return;
    }

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
    Ref ref;
    const Object kids = tree->getDict()->lookup("Kids", &ref);
    if (!seen.insert(ref)) {
        error(errSyntaxError, -1, "loop in NameTree (numObj: {0:d})", ref.num);
        return;
    }
    if (kids.isArray()) {
        for (int i = 0; i < kids.arrayGetLength(); ++i) {
            const Object kid = kids.getArray()->get(i, &ref);
            if (!seen.insert(ref)) {
                error(errSyntaxError, -1, "loop in NameTree (numObj: {0:d})", ref.num);
                continue;
            }
            if (kid.isDict()) {
                parse(&kid, seen);
            }
        }
    }
}

int NameTree::Entry::cmp(const void *voidKey, const void *voidEntry)
{
    GooString *key = (GooString *)voidKey;
    Entry *entry = *(NameTree::Entry **)voidEntry;

    return key->cmp(&entry->name);
}

Object NameTree::lookup(const GooString *name)
{
    Entry **entry;

    entry = (Entry **)bsearch(name, entries, length, sizeof(Entry *), Entry::cmp);
    if (entry != nullptr) {
        return (*entry)->value.fetch(xref);
    } else {
        error(errSyntaxError, -1, "failed to look up ({0:s})", name->c_str());
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

const GooString *NameTree::getName(int index) const
{
    if (index < length) {
        return &entries[index]->name;
    } else {
        return nullptr;
    }
}

bool Catalog::labelToIndex(GooString *label, int *index)
{
    char *end;

    PageLabelInfo *pli = getPageLabelInfo();
    if (pli != nullptr) {
        if (!pli->labelToIndex(label, index)) {
            return false;
        }
    } else {
        *index = strtol(label->c_str(), &end, 10) - 1;
        if (*end != '\0') {
            return false;
        }
    }

    if (*index < 0 || *index >= getNumPages()) {
        return false;
    }

    return true;
}

bool Catalog::indexToLabel(int index, GooString *label)
{
    char buffer[32];

    if (index < 0 || index >= getNumPages()) {
        return false;
    }

    PageLabelInfo *pli = getPageLabelInfo();
    if (pli != nullptr) {
        return pli->indexToLabel(index, label);
    } else {
        snprintf(buffer, sizeof(buffer), "%d", index + 1);
        label->append(buffer);
        return true;
    }
}

int Catalog::getNumPages()
{
    catalogLocker();
    if (numPages == -1) {
        Object catDict = xref->getCatalog();
        if (!catDict.isDict()) {
            error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
            return 0;
        }
        Object pagesDict = catDict.dictLookup("Pages");

        // This should really be isDict("Pages"), but I've seen at least one
        // PDF file where the /Type entry is missing.
        if (!pagesDict.isDict()) {
            error(errSyntaxError, -1, "Top-level pages object is wrong type ({0:s})", pagesDict.getTypeName());
            return 0;
        }

        Object obj = pagesDict.dictLookup("Count");
        // some PDF files actually use real numbers here ("/Count 9.0")
        if (!obj.isNum()) {
            if (pagesDict.dictIs("Page")) {
                const Object &pageRootRef = catDict.dictLookupNF("Pages");

                error(errSyntaxError, -1, "Pages top-level is a single Page. The document is malformed, trying to recover...");

                Dict *pageDict = pagesDict.getDict();
                if (pageRootRef.isRef()) {
                    const Ref pageRef = pageRootRef.getRef();
                    auto p = std::make_unique<Page>(doc, 1, std::move(pagesDict), pageRef, new PageAttrs(nullptr, pageDict), form);
                    if (p->isOk()) {
                        pages.emplace_back(std::move(p), pageRef);

                        numPages = 1;
                    } else {
                        numPages = 0;
                    }
                } else {
                    numPages = 0;
                }
            } else {
                error(errSyntaxError, -1, "Page count in top-level pages object is wrong type ({0:s})", obj.getTypeName());
                numPages = 0;
            }
        } else {
            if (obj.isInt()) {
                numPages = obj.getInt();
            } else if (obj.isInt64()) {
                numPages = obj.getInt64();
            } else {
                numPages = obj.getNum();
            }
            if (numPages <= 0) {
                error(errSyntaxError, -1, "Invalid page count {0:d}", numPages);
                numPages = 0;
            } else if (numPages > xref->getNumObjects()) {
                error(errSyntaxError, -1, "Page count ({0:d}) larger than number of objects ({1:d})", numPages, xref->getNumObjects());
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
            return nullptr;
        }

        Object root = catalog.dictLookup("StructTreeRoot");
        if (root.isDict("StructTreeRoot")) {
            structTreeRoot = new StructTreeRoot(doc, root.getDict());
        }
    }
    return structTreeRoot;
}

unsigned int Catalog::getMarkInfo()
{
    if (markInfo == markInfoNull) {
        markInfo = 0;

        catalogLocker();
        Object catDict = xref->getCatalog();

        if (catDict.isDict()) {
            Object markInfoDict = catDict.dictLookup("MarkInfo");
            if (markInfoDict.isDict()) {
                Object value = markInfoDict.dictLookup("Marked");
                if (value.isBool()) {
                    if (value.getBool()) {
                        markInfo |= markInfoMarked;
                    }
                } else if (!value.isNull()) {
                    error(errSyntaxError, -1, "Marked object is wrong type ({0:s})", value.getTypeName());
                }

                value = markInfoDict.dictLookup("Suspects");
                if (value.isBool() && value.getBool()) {
                    markInfo |= markInfoSuspects;
                } else if (!value.isNull()) {
                    error(errSyntaxError, -1, "Suspects object is wrong type ({0:s})", value.getTypeName());
                }

                value = markInfoDict.dictLookup("UserProperties");
                if (value.isBool() && value.getBool()) {
                    markInfo |= markInfoUserProperties;
                } else if (!value.isNull()) {
                    error(errSyntaxError, -1, "UserProperties object is wrong type ({0:s})", value.getTypeName());
                }
            } else if (!markInfoDict.isNull()) {
                error(errSyntaxError, -1, "MarkInfo object is wrong type ({0:s})", markInfoDict.getTypeName());
            }
        } else {
            error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
        }
    }
    return markInfo;
}

Object *Catalog::getCreateOutline()
{

    catalogLocker();
    Object catDict = xref->getCatalog();

    // If there is no Object in the outline variable,
    // check if there is an Outline dict in the catalog
    if (outline.isNone()) {
        if (catDict.isDict()) {
            Object outline_obj = catDict.dictLookup("Outlines");
            if (outline_obj.isDict()) {
                return &outline;
            }
        } else {
            // catalog is not a dict, give up?
            return &outline;
        }
    }

    // If there is an Object in variable, make sure it's a dict
    if (outline.isDict()) {
        return &outline;
    }

    // setup an empty outline dict
    outline = Object(new Dict(doc->getXRef()));
    outline.dictSet("Type", Object(objName, "Outlines"));
    outline.dictSet("Count", Object(0));

    const Ref outlineRef = doc->getXRef()->addIndirectObject(outline);
    catDict.dictAdd("Outlines", Object(outlineRef));
    xref->setModifiedObject(&catDict, { xref->getRootNum(), xref->getRootGen() });

    return &outline;
}

Object *Catalog::getOutline()
{
    catalogLocker();
    if (outline.isNone()) {
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
    if (dests.isNone()) {
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

Form *Catalog::getCreateForm()
{
    catalogLocker();
    if (!form) {

        Object catDict = xref->getCatalog();
        if (!catDict.isDict()) {
            error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
            return nullptr;
        }

        if (!acroForm.isDict()) {
            acroForm = Object(new Dict(xref));
            acroForm.dictSet("Fields", Object(new Array(xref)));

            const Ref newFormRef = xref->addIndirectObject(acroForm);
            catDict.dictSet("AcroForm", Object(newFormRef));

            xref->setModifiedObject(&catDict, { xref->getRootNum(), xref->getRootGen() });
        }
    }

    return getForm();
}

Form *Catalog::getForm()
{
    catalogLocker();
    if (!form) {
        if (acroForm.isDict()) {
            form = new Form(doc);
            // perform form-related loading after all widgets have been loaded
            form->postWidgetsLoad();
        }
    }

    return form;
}

void Catalog::addFormToAcroForm(const Ref formRef)
{
    catalogLocker();

    if (!acroForm.isDict()) {
        getCreateForm();
    }

    // append to field array
    Ref fieldRef;
    Object fieldArray = acroForm.getDict()->lookup("Fields", &fieldRef);
    fieldArray.getArray()->add(Object(formRef));

    setAcroFormModified();
}

void Catalog::setAcroFormModified()
{
    Object catDict = xref->getCatalog();
    Ref acroFormRef;
    catDict.getDict()->lookup("AcroForm", &acroFormRef);

    if (acroFormRef != Ref::INVALID()) {
        xref->setModifiedObject(&acroForm, acroFormRef);
    } else {
        xref->setModifiedObject(&catDict, { xref->getRootNum(), xref->getRootGen() });
    }
}

void Catalog::removeFormFromAcroForm(const Ref formRef)
{
    catalogLocker();

    Object catDict = xref->getCatalog();
    if (acroForm.isDict()) {
        // remove from field array
        Ref fieldRef;
        Object fieldArrayO = acroForm.getDict()->lookup("Fields", &fieldRef);
        Array *fieldArray = fieldArrayO.getArray();
        for (int i = 0; i < fieldArray->getLength(); ++i) {
            const Object &o = fieldArray->getNF(i);
            if (o.isRef() && o.getRef() == formRef) {
                fieldArray->remove(i);
                break;
            }
        }

        setAcroFormModified();
    }
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
    if (names.isNone()) {
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

std::unique_ptr<LinkAction> Catalog::getAdditionalAction(DocumentAdditionalActionsType type)
{
    Object additionalActionsObject = additionalActions.fetch(doc->getXRef());
    if (additionalActionsObject.isDict()) {
        const char *key = (type == actionCloseDocument                 ? "WC"
                                   : type == actionSaveDocumentStart   ? "WS"
                                   : type == actionSaveDocumentFinish  ? "DS"
                                   : type == actionPrintDocumentStart  ? "WP"
                                   : type == actionPrintDocumentFinish ? "DP"
                                                                       : nullptr);

        Object actionObject = additionalActionsObject.dictLookup(key);
        if (actionObject.isDict()) {
            return LinkAction::parseAction(&actionObject, doc->getCatalog()->getBaseURI());
        }
    }
    return nullptr;
}
