//========================================================================
//
// Catalog.h
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
// Copyright (C) 2005, 2007, 2009-2011, 2013, 2017-2020 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2005 Jonathan Blandford <jrb@redhat.com>
// Copyright (C) 2005, 2006, 2008 Brad Hards <bradh@frogmouth.net>
// Copyright (C) 2007 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2008, 2011 Pino Toscano <pino@kde.org>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2013 Adrian Perez de Castro <aperez@igalia.com>
// Copyright (C) 2013, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 José Aliste <jaliste@src.gnome.org>
// Copyright (C) 2016 Masamichi Hosoda <trueroad@trueroad.jp>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2020 Oliver Sander <oliver.sander@tu-dresden.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef CATALOG_H
#define CATALOG_H

#include "poppler-config.h"
#include "Object.h"
#include "Link.h"

#include <vector>
#include <memory>

class PDFDoc;
class XRef;
class Object;
class Page;
class PageAttrs;
struct Ref;
class PageLabelInfo;
class Form;
class OCGs;
class ViewerPreferences;
class FileSpec;
class StructTreeRoot;

//------------------------------------------------------------------------
// NameTree
//------------------------------------------------------------------------

class NameTree
{
public:
    NameTree();
    ~NameTree();

    NameTree(const NameTree &) = delete;
    NameTree &operator=(const NameTree &) = delete;

    void init(XRef *xref, Object *tree);
    Object lookup(const GooString *name);
    int numEntries() { return length; };
    // iterator accessor, note it returns a pointer to the internal object, do not free nor delete it
    Object *getValue(int i);
    GooString *getName(int i);

private:
    struct Entry
    {
        Entry(Array *array, int index);
        ~Entry();
        GooString name;
        Object value;
        static int cmpEntry(const void *voidEntry, const void *voidOtherEntry);
        static int cmp(const void *key, const void *entry);
    };

    void parse(const Object *tree, std::set<int> &seen);
    void addEntry(Entry *entry);

    XRef *xref;
    Entry **entries;
    int size, length; // size is the number of entries in
                      // the array of Entry*
                      // length is the number of real Entry
};

//------------------------------------------------------------------------
// Catalog
//------------------------------------------------------------------------

class Catalog
{
public:
    // Constructor.
    Catalog(PDFDoc *docA);

    // Destructor.
    ~Catalog();

    Catalog(const Catalog &) = delete;
    Catalog &operator=(const Catalog &) = delete;

    // Is catalog valid?
    bool isOk() { return ok; }

    // Get number of pages.
    int getNumPages();

    // Get a page.
    Page *getPage(int i);

    // Get the reference for a page object.
    Ref *getPageRef(int i);

    // Return base URI, or NULL if none.
    GooString *getBaseURI() { return baseURI; }

    // Return the contents of the metadata stream, or NULL if there is
    // no metadata.
    GooString *readMetadata();

    // Return the structure tree root object.
    StructTreeRoot *getStructTreeRoot();

    // Return values from the MarkInfo dictionary as flags in a bitfield.
    enum MarkInfoFlags
    {
        markInfoNull = 1 << 0,
        markInfoMarked = 1 << 1,
        markInfoUserProperties = 1 << 2,
        markInfoSuspects = 1 << 3,
    };
    unsigned int getMarkInfo();

    // Find a page, given its object ID.  Returns page number, or 0 if
    // not found.
    int findPage(const Ref pageRef);

    // Find a named destination.  Returns the link destination, or
    // NULL if <name> is not a destination.
    std::unique_ptr<LinkDest> findDest(const GooString *name);

    Object *getDests();

    // Get the number of named destinations in name-dict
    int numDests();

    // Get the i'th named destination name in name-dict
    const char *getDestsName(int i);

    // Get the i'th named destination link destination in name-dict
    std::unique_ptr<LinkDest> getDestsDest(int i);

    // Get the number of named destinations in name-tree
    int numDestNameTree() { return getDestNameTree()->numEntries(); }

    // Get the i'th named destination name in name-tree
    GooString *getDestNameTreeName(int i) { return getDestNameTree()->getName(i); }

    // Get the i'th named destination link destination in name-tree
    std::unique_ptr<LinkDest> getDestNameTreeDest(int i);

    // Get the number of embedded files
    int numEmbeddedFiles() { return getEmbeddedFileNameTree()->numEntries(); }

    // Get the i'th file embedded (at the Document level) in the document
    FileSpec *embeddedFile(int i);

    // Is there an embedded file with the given name?
    bool hasEmbeddedFile(const std::string &fileName);

    // Adds and embeddedFile
    // If there is already an existing embedded file with the given fileName
    // it gets replaced, if that's not what you want check hasEmbeddedFile first
    void addEmbeddedFile(GooFile *file, const std::string &fileName);

    // Get the number of javascript scripts
    int numJS() { return getJSNameTree()->numEntries(); }
    GooString *getJSName(int i) { return getJSNameTree()->getName(i); }

    // Get the i'th JavaScript script (at the Document level) in the document
    GooString *getJS(int i);

    // Convert between page indices and page labels.
    bool labelToIndex(GooString *label, int *index);
    bool indexToLabel(int index, GooString *label);

    Object *getOutline();

    Object *getAcroForm() { return &acroForm; }

    OCGs *getOptContentConfig() { return optContent; }

    enum FormType
    {
        NoForm,
        AcroForm,
        XfaForm
    };

    FormType getFormType();
    Form *getForm();

    ViewerPreferences *getViewerPreferences();

    enum PageMode
    {
        pageModeNone,
        pageModeOutlines,
        pageModeThumbs,
        pageModeFullScreen,
        pageModeOC,
        pageModeAttach,
        pageModeNull
    };
    enum PageLayout
    {
        pageLayoutNone,
        pageLayoutSinglePage,
        pageLayoutOneColumn,
        pageLayoutTwoColumnLeft,
        pageLayoutTwoColumnRight,
        pageLayoutTwoPageLeft,
        pageLayoutTwoPageRight,
        pageLayoutNull
    };

    // Returns the page mode.
    PageMode getPageMode();
    PageLayout getPageLayout();

    enum DocumentAdditionalActionsType
    {
        actionCloseDocument, ///< Performed before closing the document
        actionSaveDocumentStart, ///< Performed before saving the document
        actionSaveDocumentFinish, ///< Performed after saving the document
        actionPrintDocumentStart, ///< Performed before printing the document
        actionPrintDocumentFinish, ///< Performed after printing the document
    };

    std::unique_ptr<LinkAction> getAdditionalAction(DocumentAdditionalActionsType type);

private:
    // Get page label info.
    PageLabelInfo *getPageLabelInfo();

    PDFDoc *doc;
    XRef *xref; // the xref table for this PDF file
    std::vector<std::pair<std::unique_ptr<Page>, Ref>> pages;
    std::vector<Object> *pagesList;
    std::vector<Ref> *pagesRefList;
    std::vector<PageAttrs *> *attrsList;
    std::vector<int> *kidsIdxList;
    Form *form;
    ViewerPreferences *viewerPrefs;
    int numPages; // number of pages
    Object dests; // named destination dictionary
    Object names; // named names dictionary
    NameTree *destNameTree; // named destination name-tree
    NameTree *embeddedFileNameTree; // embedded file name-tree
    NameTree *jsNameTree; // Java Script name-tree
    GooString *baseURI; // base URI for URI-type links
    Object metadata; // metadata stream
    StructTreeRoot *structTreeRoot; // structure tree root
    unsigned int markInfo; // Flags from MarkInfo dictionary
    Object outline; // outline dictionary
    Object acroForm; // AcroForm dictionary
    Object viewerPreferences; // ViewerPreference dictionary
    OCGs *optContent; // Optional Content groups
    bool ok; // true if catalog is valid
    PageLabelInfo *pageLabelInfo; // info about page labels
    PageMode pageMode; // page mode
    PageLayout pageLayout; // page layout
    Object additionalActions; // page additional actions

    bool cachePageTree(int page); // Cache first <page> pages.
    Object *findDestInTree(Object *tree, GooString *name, Object *obj);

    Object *getNames();
    NameTree *getDestNameTree();
    NameTree *getEmbeddedFileNameTree();
    NameTree *getJSNameTree();
    std::unique_ptr<LinkDest> createLinkDest(Object *obj);

    mutable std::recursive_mutex mutex;
};

#endif
