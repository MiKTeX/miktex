//========================================================================
//
// PDFDoc.cc
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005, 2006, 2008 Brad Hards <bradh@frogmouth.net>
// Copyright (C) 2005, 2007-2009, 2011-2020 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2008 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2008, 2010 Pino Toscano <pino@kde.org>
// Copyright (C) 2008, 2010, 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009 Eric Toombs <ewtoombs@uwaterloo.ca>
// Copyright (C) 2009 Kovid Goyal <kovid@kovidgoyal.net>
// Copyright (C) 2009, 2011 Axel Struebing <axel.struebing@freenet.de>
// Copyright (C) 2010-2012, 2014 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2010 Jakub Wilk <jwilk@jwilk.net>
// Copyright (C) 2010 Ilya Gorenbein <igorenbein@finjan.com>
// Copyright (C) 2010 Srinivas Adicherla <srinivas.adicherla@geodesic.com>
// Copyright (C) 2010 Philip Lorenz <lorenzph+freedesktop@gmail.com>
// Copyright (C) 2011-2016 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2012, 2013 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2013, 2014, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013, 2018 Adam Reichold <adamreichold@myopera.com>
// Copyright (C) 2014 Bogdan Cristea <cristeab@gmail.com>
// Copyright (C) 2015 Li Junling <lijunling@sina.com>
// Copyright (C) 2015 André Guerreiro <aguerreiro1985@gmail.com>
// Copyright (C) 2015 André Esser <bepandre@hotmail.com>
// Copyright (C) 2016 Jakub Alba <jakubalba@gmail.com>
// Copyright (C) 2017 Jean Ghali <jghali@libertysurf.fr>
// Copyright (C) 2017 Fredrik Fornwall <fredrik@fornwall.net>
// Copyright (C) 2018 Ben Timby <btimby@gmail.com>
// Copyright (C) 2018 Evangelos Foutras <evangelos@foutrelis.com>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018 Evangelos Rigas <erigas@rnd2.org>
// Copyright (C) 2018 Philipp Knechtges <philipp-dev@knechtges.com>
// Copyright (C) 2019 Christian Persch <chpe@src.gnome.org>
// Copyright (C) 2020 Nelson Benítez León <nbenitezl@gmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>
#include <poppler-config.h>

#include <cctype>
#include <clocale>
#include <cstdio>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <ctime>
#include <regex>
#include <sys/stat.h>
#include "goo/glibc.h"
#include "goo/gstrtod.h"
#include "goo/GooString.h"
#include "goo/gfile.h"
#include "poppler-config.h"
#include "GlobalParams.h"
#include "Page.h"
#include "Catalog.h"
#include "Stream.h"
#include "XRef.h"
#include "Linearization.h"
#include "Link.h"
#include "OutputDev.h"
#include "Error.h"
#include "ErrorCodes.h"
#include "Lexer.h"
#include "Parser.h"
#include "SecurityHandler.h"
#include "Decrypt.h"
#include "Outline.h"
#include "PDFDoc.h"
#include "Hints.h"
#include "UTF.h"
#include "JSInfo.h"

//------------------------------------------------------------------------

#define headerSearchSize                                                                                                                                                                                                                       \
    1024 // read this many bytes at beginning of
         //   file to look for '%PDF'
#define pdfIdLength 32 // PDF Document IDs (PermanentId, UpdateId) length

#define linearizationSearchSize                                                                                                                                                                                                                \
    1024 // read this many bytes at beginning of
         // file to look for linearization
         // dictionary

#define xrefSearchSize                                                                                                                                                                                                                         \
    1024 // read this many bytes at end of file
         //   to look for 'startxref'

//------------------------------------------------------------------------
// PDFDoc
//------------------------------------------------------------------------

#define pdfdocLocker() std::unique_lock<std::recursive_mutex> locker(mutex)

void PDFDoc::init()
{
    ok = false;
    errCode = errNone;
    fileName = nullptr;
    file = nullptr;
    str = nullptr;
    xref = nullptr;
    linearization = nullptr;
    catalog = nullptr;
    hints = nullptr;
    outline = nullptr;
    startXRefPos = -1;
    secHdlr = nullptr;
    pageCache = nullptr;
}

PDFDoc::PDFDoc()
{
    init();
}

PDFDoc::PDFDoc(const GooString *fileNameA, const GooString *ownerPassword, const GooString *userPassword, void *guiDataA)
{
#ifdef _WIN32
    int n, i;
#endif

    init();

    fileName = fileNameA;
    guiData = guiDataA;
#ifdef _WIN32
    n = fileName->getLength();
    fileNameU = (wchar_t *)gmallocn(n + 1, sizeof(wchar_t));
    for (i = 0; i < n; ++i) {
        fileNameU[i] = (wchar_t)(fileName->getChar(i) & 0xff);
    }
    fileNameU[n] = L'\0';
#endif

    // try to open file
#ifdef _WIN32
    wchar_t *wFileName = (wchar_t *)utf8ToUtf16(fileName->c_str());
    file = GooFile::open(wFileName);
    gfree(wFileName);
#else
    file = GooFile::open(fileName);
#endif
    if (file == nullptr) {
        // fopen() has failed.
        // Keep a copy of the errno returned by fopen so that it can be
        // referred to later.
        fopenErrno = errno;
        error(errIO, -1, "Couldn't open file '{0:t}': {1:s}.", fileName, strerror(errno));
        errCode = errOpenFile;
        return;
    }

    // create stream
    str = new FileStream(file, 0, false, file->size(), Object(objNull));

    ok = setup(ownerPassword, userPassword);
}

#ifdef _WIN32
PDFDoc::PDFDoc(wchar_t *fileNameA, int fileNameLen, GooString *ownerPassword, GooString *userPassword, void *guiDataA)
{
    OSVERSIONINFO version;
    int i;

    init();

    guiData = guiDataA;

    // save both Unicode and 8-bit copies of the file name
    GooString *fileNameG = new GooString();
    fileNameU = (wchar_t *)gmallocn(fileNameLen + 1, sizeof(wchar_t));
    for (i = 0; i < fileNameLen; ++i) {
        fileNameG->append((char)fileNameA[i]);
        fileNameU[i] = fileNameA[i];
    }
    fileName = fileNameG;
    fileNameU[fileNameLen] = L'\0';

    // try to open file
    // NB: _wfopen is only available in NT
    version.dwOSVersionInfoSize = sizeof(version);
    GetVersionEx(&version);
    if (version.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        file = GooFile::open(fileNameU);
    } else {
        file = GooFile::open(fileName);
    }
    if (!file) {
        error(errIO, -1, "Couldn't open file '{0:t}'", fileName);
        errCode = errOpenFile;
        return;
    }

    // create stream
    str = new FileStream(file, 0, false, file->size(), Object(objNull));

    ok = setup(ownerPassword, userPassword);
}
#endif

PDFDoc::PDFDoc(BaseStream *strA, const GooString *ownerPassword, const GooString *userPassword, void *guiDataA)
{
#ifdef _WIN32
    int n, i;
#endif

    init();
    guiData = guiDataA;
    if (strA->getFileName()) {
        fileName = strA->getFileName()->copy();
#ifdef _WIN32
        n = fileName->getLength();
        fileNameU = (wchar_t *)gmallocn(n + 1, sizeof(wchar_t));
        for (i = 0; i < n; ++i) {
            fileNameU[i] = (wchar_t)(fileName->getChar(i) & 0xff);
        }
        fileNameU[n] = L'\0';
#endif
    } else {
        fileName = nullptr;
#ifdef _WIN32
        fileNameU = NULL;
#endif
    }
    str = strA;
    ok = setup(ownerPassword, userPassword);
}

bool PDFDoc::setup(const GooString *ownerPassword, const GooString *userPassword)
{
    pdfdocLocker();

    if (str->getLength() <= 0) {
        error(errSyntaxError, -1, "Document stream is empty");
        return false;
    }

    str->setPos(0, -1);
    if (str->getPos() < 0) {
        error(errSyntaxError, -1, "Document base stream is not seekable");
        return false;
    }

    str->reset();

    // check footer
    // Adobe does not seem to enforce %%EOF, so we do the same
    //  if (!checkFooter()) return false;

    // check header
    checkHeader();

    bool wasReconstructed = false;

    // read xref table
    xref = new XRef(str, getStartXRef(), getMainXRefEntriesOffset(), &wasReconstructed);
    if (!xref->isOk()) {
        if (wasReconstructed) {
            delete xref;
            startXRefPos = -1;
            xref = new XRef(str, getStartXRef(true), getMainXRefEntriesOffset(true), &wasReconstructed);
        }
        if (!xref->isOk()) {
            error(errSyntaxError, -1, "Couldn't read xref table");
            errCode = xref->getErrorCode();
            return false;
        }
    }

    // check for encryption
    if (!checkEncryption(ownerPassword, userPassword)) {
        errCode = errEncrypted;
        return false;
    }

    // read catalog
    catalog = new Catalog(this);
    if (catalog && !catalog->isOk()) {
        if (!wasReconstructed) {
            // try one more time to construct the Catalog, maybe the problem is damaged XRef
            delete catalog;
            delete xref;
            xref = new XRef(str, 0, 0, nullptr, true);
            catalog = new Catalog(this);
        }

        if (catalog && !catalog->isOk()) {
            error(errSyntaxError, -1, "Couldn't read page catalog");
            errCode = errBadCatalog;
            return false;
        }
    }

    // Extract PDF Subtype information
    extractPDFSubtype();

    // done
    return true;
}

PDFDoc::~PDFDoc()
{
    if (pageCache) {
        for (int i = 0; i < getNumPages(); i++) {
            if (pageCache[i]) {
                delete pageCache[i];
            }
        }
        gfree(pageCache);
    }
    delete secHdlr;
    if (outline) {
        delete outline;
    }
    if (catalog) {
        delete catalog;
    }
    if (xref) {
        delete xref;
    }
    if (hints) {
        delete hints;
    }
    if (linearization) {
        delete linearization;
    }
    if (str) {
        delete str;
    }
    if (file) {
        delete file;
    }
    if (fileName) {
        delete fileName;
    }
#ifdef _WIN32
    if (fileNameU) {
        gfree(fileNameU);
    }
#endif
}

// Check for a %%EOF at the end of this stream
bool PDFDoc::checkFooter()
{
    // we look in the last 1024 chars because Adobe does the same
    char *eof = new char[1025];
    Goffset pos = str->getPos();
    str->setPos(1024, -1);
    int i, ch;
    for (i = 0; i < 1024; i++) {
        ch = str->getChar();
        if (ch == EOF)
            break;
        eof[i] = ch;
    }
    eof[i] = '\0';

    bool found = false;
    for (i = i - 5; i >= 0; i--) {
        if (strncmp(&eof[i], "%%EOF", 5) == 0) {
            found = true;
            break;
        }
    }
    if (!found) {
        error(errSyntaxError, -1, "Document has not the mandatory ending %%EOF");
        errCode = errDamaged;
        delete[] eof;
        return false;
    }
    delete[] eof;
    str->setPos(pos);
    return true;
}

// Check for a PDF header on this stream.  Skip past some garbage
// if necessary.
void PDFDoc::checkHeader()
{
    char hdrBuf[headerSearchSize + 1];
    char *p;
    char *tokptr;
    int i;
    int bytesRead;

    pdfMajorVersion = 0;
    pdfMinorVersion = 0;

    // read up to headerSearchSize bytes from the beginning of the document
    for (i = 0; i < headerSearchSize; ++i) {
        const int c = str->getChar();
        if (c == EOF)
            break;
        hdrBuf[i] = c;
    }
    bytesRead = i;
    hdrBuf[bytesRead] = '\0';

    // find the start of the PDF header if it exists and parse the version
    bool headerFound = false;
    for (i = 0; i < bytesRead - 5; ++i) {
        if (!strncmp(&hdrBuf[i], "%PDF-", 5)) {
            headerFound = true;
            break;
        }
    }
    if (!headerFound) {
        error(errSyntaxWarning, -1, "May not be a PDF file (continuing anyway)");
        return;
    }
    str->moveStart(i);
    if (!(p = strtok_r(&hdrBuf[i + 5], " \t\n\r", &tokptr))) {
        error(errSyntaxWarning, -1, "May not be a PDF file (continuing anyway)");
        return;
    }
    sscanf(p, "%d.%d", &pdfMajorVersion, &pdfMinorVersion);
    // We don't do the version check. Don't add it back in.
}

bool PDFDoc::checkEncryption(const GooString *ownerPassword, const GooString *userPassword)
{
    bool encrypted;
    bool ret;

    Object encrypt = xref->getTrailerDict()->dictLookup("Encrypt");
    if ((encrypted = encrypt.isDict())) {
        if ((secHdlr = SecurityHandler::make(this, &encrypt))) {
            if (secHdlr->isUnencrypted()) {
                // no encryption
                ret = true;
            } else if (secHdlr->checkEncryption(ownerPassword, userPassword)) {
                // authorization succeeded
                xref->setEncryption(secHdlr->getPermissionFlags(), secHdlr->getOwnerPasswordOk(), secHdlr->getFileKey(), secHdlr->getFileKeyLength(), secHdlr->getEncVersion(), secHdlr->getEncRevision(), secHdlr->getEncAlgorithm());
                ret = true;
            } else {
                // authorization failed
                ret = false;
            }
        } else {
            // couldn't find the matching security handler
            ret = false;
        }
    } else {
        // document is not encrypted
        ret = true;
    }
    return ret;
}

static PDFSubtypePart pdfPartFromString(PDFSubtype subtype, GooString *pdfSubtypeVersion)
{
    const std::regex regex("PDF/(?:A|X|VT|E|UA)-([[:digit:]])(?:[[:alpha:]]{1,2})?:?([[:digit:]]{4})?");
    std::smatch match;
    const std::string &pdfsubver = pdfSubtypeVersion->toStr();
    PDFSubtypePart subtypePart = subtypePartNone;

    if (std::regex_search(pdfsubver, match, regex)) {
        int date = 0;
        const int part = std::stoi(match.str(1));

        if (match[2].matched) {
            date = std::stoi(match.str(2));
        }

        switch (subtype) {
        case subtypePDFX:
            switch (part) {
            case 1:
                switch (date) {
                case 2001:
                default:
                    subtypePart = subtypePart1;
                    break;
                case 2003:
                    subtypePart = subtypePart4;
                    break;
                }
                break;
            case 2:
                subtypePart = subtypePart5;
                break;
            case 3:
                switch (date) {
                case 2002:
                default:
                    subtypePart = subtypePart3;
                    break;
                case 2003:
                    subtypePart = subtypePart6;
                    break;
                }
                break;
            case 4:
                subtypePart = subtypePart7;
                break;
            case 5:
                subtypePart = subtypePart8;
                break;
            }
            break;
        default:
            subtypePart = (PDFSubtypePart)part;
            break;
        }
    }

    return subtypePart;
}

static PDFSubtypeConformance pdfConformanceFromString(GooString *pdfSubtypeVersion)
{
    const std::regex regex("PDF/(?:A|X|VT|E|UA)-[[:digit:]]([[:alpha:]]+)");
    std::smatch match;
    const std::string &pdfsubver = pdfSubtypeVersion->toStr();
    PDFSubtypeConformance pdfConf = subtypeConfNone;

    // match contains the PDF conformance (A, B, G, N, P, PG or U)
    if (std::regex_search(pdfsubver, match, regex)) {
        GooString *conf = new GooString(match.str(1));
        // Convert to lowercase as the conformance may appear in both cases
        conf->lowerCase();
        if (conf->cmp("a") == 0) {
            pdfConf = subtypeConfA;
        } else if (conf->cmp("b") == 0) {
            pdfConf = subtypeConfB;
        } else if (conf->cmp("g") == 0) {
            pdfConf = subtypeConfG;
        } else if (conf->cmp("n") == 0) {
            pdfConf = subtypeConfN;
        } else if (conf->cmp("p") == 0) {
            pdfConf = subtypeConfP;
        } else if (conf->cmp("pg") == 0) {
            pdfConf = subtypeConfPG;
        } else if (conf->cmp("u") == 0) {
            pdfConf = subtypeConfU;
        } else {
            pdfConf = subtypeConfNone;
        }
        delete conf;
    }

    return pdfConf;
}

void PDFDoc::extractPDFSubtype()
{
    pdfSubtype = subtypeNull;
    pdfPart = subtypePartNull;
    pdfConformance = subtypeConfNull;

    GooString *pdfSubtypeVersion = nullptr;
    // Find PDF InfoDict subtype key if any
    if ((pdfSubtypeVersion = getDocInfoStringEntry("GTS_PDFA1Version"))) {
        pdfSubtype = subtypePDFA;
    } else if ((pdfSubtypeVersion = getDocInfoStringEntry("GTS_PDFEVersion"))) {
        pdfSubtype = subtypePDFE;
    } else if ((pdfSubtypeVersion = getDocInfoStringEntry("GTS_PDFUAVersion"))) {
        pdfSubtype = subtypePDFUA;
    } else if ((pdfSubtypeVersion = getDocInfoStringEntry("GTS_PDFVTVersion"))) {
        pdfSubtype = subtypePDFVT;
    } else if ((pdfSubtypeVersion = getDocInfoStringEntry("GTS_PDFXVersion"))) {
        pdfSubtype = subtypePDFX;
    } else {
        pdfSubtype = subtypeNone;
        pdfPart = subtypePartNone;
        pdfConformance = subtypeConfNone;
        return;
    }

    // Extract part from version string
    pdfPart = pdfPartFromString(pdfSubtype, pdfSubtypeVersion);

    // Extract conformance from version string
    pdfConformance = pdfConformanceFromString(pdfSubtypeVersion);

    delete pdfSubtypeVersion;
}

static void addSignatureFieldsToVector(FormField *ff, std::vector<FormFieldSignature *> &res)
{
    if (ff->getNumChildren() == 0) {
        if (ff->getType() == formSignature) {
            res.push_back(static_cast<FormFieldSignature *>(ff));
        }
    } else {
        for (int i = 0; i < ff->getNumChildren(); ++i) {
            FormField *children = ff->getChildren(i);
            addSignatureFieldsToVector(children, res);
        }
    }
}

std::vector<FormFieldSignature *> PDFDoc::getSignatureFields()
{
    std::vector<FormFieldSignature *> res;

    const Form *f = catalog->getForm();
    if (!f)
        return res;

    const int nRootFields = f->getNumFields();
    for (int i = 0; i < nRootFields; ++i) {
        FormField *ff = f->getRootField(i);
        addSignatureFieldsToVector(ff, res);
    }
    return res;
}

void PDFDoc::displayPage(OutputDev *out, int page, double hDPI, double vDPI, int rotate, bool useMediaBox, bool crop, bool printing, bool (*abortCheckCbk)(void *data), void *abortCheckCbkData,
                         bool (*annotDisplayDecideCbk)(Annot *annot, void *user_data), void *annotDisplayDecideCbkData, bool copyXRef)
{
    if (globalParams->getPrintCommands()) {
        printf("***** page %d *****\n", page);
    }

    if (getPage(page))
        getPage(page)->display(out, hDPI, vDPI, rotate, useMediaBox, crop, printing, abortCheckCbk, abortCheckCbkData, annotDisplayDecideCbk, annotDisplayDecideCbkData, copyXRef);
}

void PDFDoc::displayPages(OutputDev *out, int firstPage, int lastPage, double hDPI, double vDPI, int rotate, bool useMediaBox, bool crop, bool printing, bool (*abortCheckCbk)(void *data), void *abortCheckCbkData,
                          bool (*annotDisplayDecideCbk)(Annot *annot, void *user_data), void *annotDisplayDecideCbkData)
{
    int page;

    for (page = firstPage; page <= lastPage; ++page) {
        displayPage(out, page, hDPI, vDPI, rotate, useMediaBox, crop, printing, abortCheckCbk, abortCheckCbkData, annotDisplayDecideCbk, annotDisplayDecideCbkData);
    }
}

void PDFDoc::displayPageSlice(OutputDev *out, int page, double hDPI, double vDPI, int rotate, bool useMediaBox, bool crop, bool printing, int sliceX, int sliceY, int sliceW, int sliceH, bool (*abortCheckCbk)(void *data),
                              void *abortCheckCbkData, bool (*annotDisplayDecideCbk)(Annot *annot, void *user_data), void *annotDisplayDecideCbkData, bool copyXRef)
{
    if (getPage(page))
        getPage(page)->displaySlice(out, hDPI, vDPI, rotate, useMediaBox, crop, sliceX, sliceY, sliceW, sliceH, printing, abortCheckCbk, abortCheckCbkData, annotDisplayDecideCbk, annotDisplayDecideCbkData, copyXRef);
}

Links *PDFDoc::getLinks(int page)
{
    Page *p = getPage(page);
    if (!p) {
        return new Links(nullptr);
    }
    return p->getLinks();
}

void PDFDoc::processLinks(OutputDev *out, int page)
{
    if (getPage(page))
        getPage(page)->processLinks(out);
}

Linearization *PDFDoc::getLinearization()
{
    if (!linearization) {
        linearization = new Linearization(str);
        linearizationState = 0;
    }
    return linearization;
}

bool PDFDoc::checkLinearization()
{
    if (linearization == nullptr)
        return false;
    if (linearizationState == 1)
        return true;
    if (linearizationState == 2)
        return false;
    if (!hints) {
        hints = new Hints(str, linearization, getXRef(), secHdlr);
    }
    if (!hints->isOk()) {
        linearizationState = 2;
        return false;
    }
    for (int page = 1; page <= linearization->getNumPages(); page++) {
        Ref pageRef;

        pageRef.num = hints->getPageObjectNum(page);
        if (!pageRef.num) {
            linearizationState = 2;
            return false;
        }

        // check for bogus ref - this can happen in corrupted PDF files
        if (pageRef.num < 0 || pageRef.num >= xref->getNumObjects()) {
            linearizationState = 2;
            return false;
        }

        pageRef.gen = xref->getEntry(pageRef.num)->gen;
        Object obj = xref->fetch(pageRef);
        if (!obj.isDict("Page")) {
            linearizationState = 2;
            return false;
        }
    }
    linearizationState = 1;
    return true;
}

bool PDFDoc::isLinearized(bool tryingToReconstruct)
{
    if ((str->getLength()) && (getLinearization()->getLength() == str->getLength()))
        return true;
    else {
        if (tryingToReconstruct)
            return getLinearization()->getLength() > 0;
        else
            return false;
    }
}

void PDFDoc::setDocInfoModified(Object *infoObj)
{
    Object infoObjRef = getDocInfoNF();
    xref->setModifiedObject(infoObj, infoObjRef.getRef());
}

void PDFDoc::setDocInfoStringEntry(const char *key, GooString *value)
{
    bool removeEntry = !value || value->getLength() == 0 || value->hasJustUnicodeMarker();
    if (removeEntry) {
        delete value;
    }

    Object infoObj = getDocInfo();
    if (infoObj.isNull() && removeEntry) {
        // No info dictionary, so no entry to remove.
        return;
    }

    infoObj = createDocInfoIfNoneExists();
    if (removeEntry) {
        infoObj.dictSet(key, Object(objNull));
    } else {
        infoObj.dictSet(key, Object(value));
    }

    if (infoObj.dictGetLength() == 0) {
        // Info dictionary is empty. Remove it altogether.
        removeDocInfo();
    } else {
        setDocInfoModified(&infoObj);
    }
}

GooString *PDFDoc::getDocInfoStringEntry(const char *key)
{
    Object infoObj = getDocInfo();
    if (!infoObj.isDict()) {
        return nullptr;
    }

    Object entryObj = infoObj.dictLookup(key);

    GooString *result;

    if (entryObj.isString()) {
        result = entryObj.takeString();
    } else {
        result = nullptr;
    }

    return result;
}

static bool get_id(const GooString *encodedidstring, GooString *id)
{
    const char *encodedid = encodedidstring->c_str();
    char pdfid[pdfIdLength + 1];
    int n;

    if (encodedidstring->getLength() != pdfIdLength / 2)
        return false;

    n = sprintf(pdfid, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", encodedid[0] & 0xff, encodedid[1] & 0xff, encodedid[2] & 0xff, encodedid[3] & 0xff, encodedid[4] & 0xff, encodedid[5] & 0xff, encodedid[6] & 0xff,
                encodedid[7] & 0xff, encodedid[8] & 0xff, encodedid[9] & 0xff, encodedid[10] & 0xff, encodedid[11] & 0xff, encodedid[12] & 0xff, encodedid[13] & 0xff, encodedid[14] & 0xff, encodedid[15] & 0xff);
    if (n != pdfIdLength)
        return false;

    id->Set(pdfid, pdfIdLength);
    return true;
}

bool PDFDoc::getID(GooString *permanent_id, GooString *update_id) const
{
    Object obj = xref->getTrailerDict()->dictLookup("ID");

    if (obj.isArray() && obj.arrayGetLength() == 2) {
        if (permanent_id) {
            Object obj2 = obj.arrayGet(0);
            if (obj2.isString()) {
                if (!get_id(obj2.getString(), permanent_id)) {
                    return false;
                }
            } else {
                error(errSyntaxError, -1, "Invalid permanent ID");
                return false;
            }
        }

        if (update_id) {
            Object obj2 = obj.arrayGet(1);
            if (obj2.isString()) {
                if (!get_id(obj2.getString(), update_id)) {
                    return false;
                }
            } else {
                error(errSyntaxError, -1, "Invalid update ID");
                return false;
            }
        }

        return true;
    }

    return false;
}

Hints *PDFDoc::getHints()
{
    if (!hints && isLinearized()) {
        hints = new Hints(str, getLinearization(), getXRef(), secHdlr);
    }

    return hints;
}

int PDFDoc::savePageAs(const GooString *name, int pageNo)
{
    FILE *f;
    OutStream *outStr;
    XRef *yRef, *countRef;

    if (file && file->modificationTimeChangedSinceOpen())
        return errFileChangedSinceOpen;

    int rootNum = getXRef()->getNumObjects() + 1;

    // Make sure that special flags are set, because we are going to read
    // all objects, including Unencrypted ones.
    xref->scanSpecialFlags();

    unsigned char *fileKey;
    CryptAlgorithm encAlgorithm;
    int keyLength;
    xref->getEncryptionParameters(&fileKey, &encAlgorithm, &keyLength);

    if (pageNo < 1 || pageNo > getNumPages() || !getCatalog()->getPage(pageNo)) {
        error(errInternal, -1, "Illegal pageNo: {0:d}({1:d})", pageNo, getNumPages());
        return errOpenFile;
    }
    const PDFRectangle *cropBox = nullptr;
    if (getCatalog()->getPage(pageNo)->isCropped()) {
        cropBox = getCatalog()->getPage(pageNo)->getCropBox();
    }
    replacePageDict(pageNo, getCatalog()->getPage(pageNo)->getRotate(), getCatalog()->getPage(pageNo)->getMediaBox(), cropBox);
    Ref *refPage = getCatalog()->getPageRef(pageNo);
    Object page = getXRef()->fetch(*refPage);

    if (!(f = openFile(name->c_str(), "wb"))) {
        error(errIO, -1, "Couldn't open file '{0:t}'", name);
        return errOpenFile;
    }
    outStr = new FileOutStream(f, 0);

    yRef = new XRef(getXRef()->getTrailerDict());

    if (secHdlr != nullptr && !secHdlr->isUnencrypted()) {
        yRef->setEncryption(secHdlr->getPermissionFlags(), secHdlr->getOwnerPasswordOk(), fileKey, keyLength, secHdlr->getEncVersion(), secHdlr->getEncRevision(), encAlgorithm);
    }
    countRef = new XRef();
    Object *trailerObj = getXRef()->getTrailerDict();
    if (trailerObj->isDict()) {
        markPageObjects(trailerObj->getDict(), yRef, countRef, 0, refPage->num, rootNum + 2);
    }
    yRef->add(0, 65535, 0, false);
    writeHeader(outStr, getPDFMajorVersion(), getPDFMinorVersion());

    // get and mark info dict
    Object infoObj = getXRef()->getDocInfo();
    if (infoObj.isDict()) {
        Dict *infoDict = infoObj.getDict();
        markPageObjects(infoDict, yRef, countRef, 0, refPage->num, rootNum + 2);
        if (trailerObj->isDict()) {
            Dict *trailerDict = trailerObj->getDict();
            const Object &ref = trailerDict->lookupNF("Info");
            if (ref.isRef()) {
                yRef->add(ref.getRef(), 0, true);
                if (getXRef()->getEntry(ref.getRef().num)->type == xrefEntryCompressed) {
                    yRef->getEntry(ref.getRef().num)->type = xrefEntryCompressed;
                }
            }
        }
    }

    // get and mark output intents etc.
    Object catObj = getXRef()->getCatalog();
    Dict *catDict = catObj.getDict();
    Object pagesObj = catDict->lookup("Pages");
    Object afObj = catDict->lookupNF("AcroForm").copy();
    if (!afObj.isNull()) {
        markAcroForm(&afObj, yRef, countRef, 0, refPage->num, rootNum + 2);
    }
    Dict *pagesDict = pagesObj.getDict();
    Object resourcesObj = pagesDict->lookup("Resources");
    if (resourcesObj.isDict())
        markPageObjects(resourcesObj.getDict(), yRef, countRef, 0, refPage->num, rootNum + 2);
    markPageObjects(catDict, yRef, countRef, 0, refPage->num, rootNum + 2);

    Dict *pageDict = page.getDict();
    if (resourcesObj.isNull() && !pageDict->hasKey("Resources")) {
        Object *resourceDictObject = getCatalog()->getPage(pageNo)->getResourceDictObject();
        if (resourceDictObject->isDict()) {
            resourcesObj = resourceDictObject->copy();
            markPageObjects(resourcesObj.getDict(), yRef, countRef, 0, refPage->num, rootNum + 2);
        }
    }
    markPageObjects(pageDict, yRef, countRef, 0, refPage->num, rootNum + 2);
    Object annotsObj = pageDict->lookupNF("Annots").copy();
    if (!annotsObj.isNull()) {
        markAnnotations(&annotsObj, yRef, countRef, 0, refPage->num, rootNum + 2);
    }
    yRef->markUnencrypted();
    writePageObjects(outStr, yRef, 0);

    yRef->add(rootNum, 0, outStr->getPos(), true);
    outStr->printf("%d 0 obj\n", rootNum);
    outStr->printf("<< /Type /Catalog /Pages %d 0 R", rootNum + 1);
    for (int j = 0; j < catDict->getLength(); j++) {
        const char *key = catDict->getKey(j);
        if (strcmp(key, "Type") != 0 && strcmp(key, "Catalog") != 0 && strcmp(key, "Pages") != 0) {
            if (j > 0)
                outStr->printf(" ");
            Object value = catDict->getValNF(j).copy();
            outStr->printf("/%s ", key);
            writeObject(&value, outStr, getXRef(), 0, nullptr, cryptRC4, 0, 0, 0);
        }
    }
    outStr->printf(">>\nendobj\n");

    yRef->add(rootNum + 1, 0, outStr->getPos(), true);
    outStr->printf("%d 0 obj\n", rootNum + 1);
    outStr->printf("<< /Type /Pages /Kids [ %d 0 R ] /Count 1 ", rootNum + 2);
    if (resourcesObj.isDict()) {
        outStr->printf("/Resources ");
        writeObject(&resourcesObj, outStr, getXRef(), 0, nullptr, cryptRC4, 0, 0, 0);
    }
    outStr->printf(">>\n");
    outStr->printf("endobj\n");

    yRef->add(rootNum + 2, 0, outStr->getPos(), true);
    outStr->printf("%d 0 obj\n", rootNum + 2);
    outStr->printf("<< ");
    for (int n = 0; n < pageDict->getLength(); n++) {
        if (n > 0)
            outStr->printf(" ");
        const char *key = pageDict->getKey(n);
        Object value = pageDict->getValNF(n).copy();
        if (strcmp(key, "Parent") == 0) {
            outStr->printf("/Parent %d 0 R", rootNum + 1);
        } else {
            outStr->printf("/%s ", key);
            writeObject(&value, outStr, getXRef(), 0, nullptr, cryptRC4, 0, 0, 0);
        }
    }
    outStr->printf(" >>\nendobj\n");

    Goffset uxrefOffset = outStr->getPos();
    Ref ref;
    ref.num = rootNum;
    ref.gen = 0;
    Object trailerDict = createTrailerDict(rootNum + 3, false, 0, &ref, getXRef(), name->c_str(), uxrefOffset);
    writeXRefTableTrailer(std::move(trailerDict), yRef, false /* do not write unnecessary entries */, uxrefOffset, outStr, getXRef());

    outStr->close();
    fclose(f);
    delete yRef;
    delete countRef;
    delete outStr;

    return errNone;
}

int PDFDoc::saveAs(const GooString *name, PDFWriteMode mode)
{
    FILE *f;
    OutStream *outStr;
    int res;

    if (!(f = openFile(name->c_str(), "wb"))) {
        error(errIO, -1, "Couldn't open file '{0:t}'", name);
        return errOpenFile;
    }
    outStr = new FileOutStream(f, 0);
    res = saveAs(outStr, mode);
    delete outStr;
    fclose(f);
    return res;
}

int PDFDoc::saveAs(OutStream *outStr, PDFWriteMode mode)
{
    if (file && file->modificationTimeChangedSinceOpen())
        return errFileChangedSinceOpen;

    if (!xref->isModified() && mode == writeStandard) {
        // simply copy the original file
        saveWithoutChangesAs(outStr);
    } else if (mode == writeForceRewrite) {
        saveCompleteRewrite(outStr);
    } else {
        saveIncrementalUpdate(outStr);
    }

    return errNone;
}

int PDFDoc::saveWithoutChangesAs(const GooString *name)
{
    FILE *f;
    OutStream *outStr;
    int res;

    if (!(f = openFile(name->c_str(), "wb"))) {
        error(errIO, -1, "Couldn't open file '{0:t}'", name);
        return errOpenFile;
    }

    outStr = new FileOutStream(f, 0);
    res = saveWithoutChangesAs(outStr);
    delete outStr;

    fclose(f);

    return res;
}

int PDFDoc::saveWithoutChangesAs(OutStream *outStr)
{
    int c;

    if (file && file->modificationTimeChangedSinceOpen())
        return errFileChangedSinceOpen;

    BaseStream *copyStr = str->copy();
    copyStr->reset();
    while ((c = copyStr->getChar()) != EOF) {
        outStr->put(c);
    }
    copyStr->close();
    delete copyStr;

    return errNone;
}

void PDFDoc::saveIncrementalUpdate(OutStream *outStr)
{
    XRef *uxref;
    int c;
    // copy the original file
    BaseStream *copyStr = str->copy();
    copyStr->reset();
    while ((c = copyStr->getChar()) != EOF) {
        outStr->put(c);
    }
    copyStr->close();
    delete copyStr;

    unsigned char *fileKey;
    CryptAlgorithm encAlgorithm;
    int keyLength;
    xref->getEncryptionParameters(&fileKey, &encAlgorithm, &keyLength);

    uxref = new XRef();
    uxref->add(0, 65535, 0, false);
    xref->lock();
    for (int i = 0; i < xref->getNumObjects(); i++) {
        if ((xref->getEntry(i)->type == xrefEntryFree) && (xref->getEntry(i)->gen == 0)) // we skip the irrelevant free objects
            continue;

        if (xref->getEntry(i)->getFlag(XRefEntry::Updated)) { // we have an updated object
            Ref ref;
            ref.num = i;
            ref.gen = xref->getEntry(i)->type == xrefEntryCompressed ? 0 : xref->getEntry(i)->gen;
            if (xref->getEntry(i)->type != xrefEntryFree) {
                Object obj1 = xref->fetch(ref, 1 /* recursion */);
                Goffset offset = writeObjectHeader(&ref, outStr);
                writeObject(&obj1, outStr, fileKey, encAlgorithm, keyLength, ref);
                writeObjectFooter(outStr);
                uxref->add(ref, offset, true);
            } else {
                uxref->add(ref, 0, false);
            }
        }
    }
    xref->unlock();
    // because of "uxref->add(0, 65535, 0, false);" uxref->getNumObjects() will
    // always be >= 1; if it is 1, it means there is nothing to update
    if (uxref->getNumObjects() == 1) {
        delete uxref;
        return;
    }

    Goffset uxrefOffset = outStr->getPos();
    int numobjects = xref->getNumObjects();
    const char *fileNameA = fileName ? fileName->c_str() : nullptr;
    Ref rootRef, uxrefStreamRef;
    rootRef.num = getXRef()->getRootNum();
    rootRef.gen = getXRef()->getRootGen();

    // Output a xref stream if there is a xref stream already
    bool xRefStream = xref->isXRefStream();

    if (xRefStream) {
        // Append an entry for the xref stream itself
        uxrefStreamRef.num = numobjects++;
        uxrefStreamRef.gen = 0;
        uxref->add(uxrefStreamRef, uxrefOffset, true);
    }

    Object trailerDict = createTrailerDict(numobjects, true, getStartXRef(), &rootRef, getXRef(), fileNameA, uxrefOffset);
    if (xRefStream) {
        writeXRefStreamTrailer(std::move(trailerDict), uxref, &uxrefStreamRef, uxrefOffset, outStr, getXRef());
    } else {
        writeXRefTableTrailer(std::move(trailerDict), uxref, false, uxrefOffset, outStr, getXRef());
    }

    delete uxref;
}

void PDFDoc::saveCompleteRewrite(OutStream *outStr)
{
    // Make sure that special flags are set, because we are going to read
    // all objects, including Unencrypted ones.
    xref->scanSpecialFlags();

    unsigned char *fileKey;
    CryptAlgorithm encAlgorithm;
    int keyLength;
    xref->getEncryptionParameters(&fileKey, &encAlgorithm, &keyLength);

    writeHeader(outStr, getPDFMajorVersion(), getPDFMinorVersion());
    XRef *uxref = new XRef();
    uxref->add(0, 65535, 0, false);
    xref->lock();
    for (int i = 0; i < xref->getNumObjects(); i++) {
        Ref ref;
        XRefEntryType type = xref->getEntry(i)->type;
        if (type == xrefEntryFree) {
            ref.num = i;
            ref.gen = xref->getEntry(i)->gen;
            /* the XRef class adds a lot of irrelevant free entries, we only want the significant one
                and we don't want the one with num=0 because it has already been added (gen = 65535)*/
            if (ref.gen > 0 && ref.num > 0)
                uxref->add(ref, 0, false);
        } else if (xref->getEntry(i)->getFlag(XRefEntry::DontRewrite)) {
            // This entry must not be written, put a free entry instead (with incremented gen)
            ref.num = i;
            ref.gen = xref->getEntry(i)->gen + 1;
            uxref->add(ref, 0, false);
        } else if (type == xrefEntryUncompressed) {
            ref.num = i;
            ref.gen = xref->getEntry(i)->gen;
            Object obj1 = xref->fetch(ref, 1 /* recursion */);
            Goffset offset = writeObjectHeader(&ref, outStr);
            // Write unencrypted objects in unencrypted form
            if (xref->getEntry(i)->getFlag(XRefEntry::Unencrypted)) {
                writeObject(&obj1, outStr, nullptr, cryptRC4, 0, 0, 0);
            } else {
                writeObject(&obj1, outStr, fileKey, encAlgorithm, keyLength, ref);
            }
            writeObjectFooter(outStr);
            uxref->add(ref, offset, true);
        } else if (type == xrefEntryCompressed) {
            ref.num = i;
            ref.gen = 0; // compressed entries have gen == 0
            Object obj1 = xref->fetch(ref, 1 /* recursion */);
            Goffset offset = writeObjectHeader(&ref, outStr);
            writeObject(&obj1, outStr, fileKey, encAlgorithm, keyLength, ref);
            writeObjectFooter(outStr);
            uxref->add(ref, offset, true);
        }
    }
    xref->unlock();
    Goffset uxrefOffset = outStr->getPos();
    writeXRefTableTrailer(uxrefOffset, uxref, true /* write all entries */, uxref->getNumObjects(), outStr, false /* complete rewrite */);
    delete uxref;
}

void PDFDoc::writeDictionnary(Dict *dict, OutStream *outStr, XRef *xRef, unsigned int numOffset, unsigned char *fileKey, CryptAlgorithm encAlgorithm, int keyLength, Ref ref, std::set<Dict *> *alreadyWrittenDicts)
{
    bool deleteSet = false;
    if (!alreadyWrittenDicts) {
        alreadyWrittenDicts = new std::set<Dict *>;
        deleteSet = true;
    }

    if (alreadyWrittenDicts->find(dict) != alreadyWrittenDicts->end()) {
        error(errSyntaxWarning, -1, "PDFDoc::writeDictionnary: Found recursive dicts");
        if (deleteSet)
            delete alreadyWrittenDicts;
        return;
    } else {
        alreadyWrittenDicts->insert(dict);
    }

    outStr->printf("<<");
    for (int i = 0; i < dict->getLength(); i++) {
        GooString keyName(dict->getKey(i));
        GooString *keyNameToPrint = keyName.sanitizedName(false /* non ps mode */);
        outStr->printf("/%s ", keyNameToPrint->c_str());
        delete keyNameToPrint;
        Object obj1 = dict->getValNF(i).copy();
        writeObject(&obj1, outStr, xRef, numOffset, fileKey, encAlgorithm, keyLength, ref, alreadyWrittenDicts);
    }
    outStr->printf(">> ");

    if (deleteSet) {
        delete alreadyWrittenDicts;
    }
}

void PDFDoc::writeStream(Stream *str, OutStream *outStr)
{
    outStr->printf("stream\r\n");
    str->reset();
    for (int c = str->getChar(); c != EOF; c = str->getChar()) {
        outStr->printf("%c", c);
    }
    outStr->printf("\r\nendstream\r\n");
}

void PDFDoc::writeRawStream(Stream *str, OutStream *outStr)
{
    Object obj1 = str->getDict()->lookup("Length");
    if (!obj1.isInt() && !obj1.isInt64()) {
        error(errSyntaxError, -1, "PDFDoc::writeRawStream, no Length in stream dict");
        return;
    }

    Goffset length;
    if (obj1.isInt())
        length = obj1.getInt();
    else
        length = obj1.getInt64();

    outStr->printf("stream\r\n");
    str->unfilteredReset();
    for (Goffset i = 0; i < length; i++) {
        int c = str->getUnfilteredChar();
        if (unlikely(c == EOF)) {
            error(errSyntaxError, -1, "PDFDoc::writeRawStream: EOF reading stream");
            break;
        }
        outStr->printf("%c", c);
    }
    str->reset();
    outStr->printf("\r\nendstream\r\n");
}

void PDFDoc::writeString(const GooString *s, OutStream *outStr, const unsigned char *fileKey, CryptAlgorithm encAlgorithm, int keyLength, Ref ref)
{
    // Encrypt string if encryption is enabled
    GooString *sEnc = nullptr;
    if (fileKey) {
        EncryptStream *enc = new EncryptStream(new MemStream(s->c_str(), 0, s->getLength(), Object(objNull)), fileKey, encAlgorithm, keyLength, ref);
        sEnc = new GooString();
        int c;
        enc->reset();
        while ((c = enc->getChar()) != EOF) {
            sEnc->append((char)c);
        }

        delete enc;
        s = sEnc;
    }

    // Write data
    if (s->hasUnicodeMarker()) {
        // unicode string don't necessary end with \0
        const char *c = s->c_str();
        outStr->printf("(");
        for (int i = 0; i < s->getLength(); i++) {
            char unescaped = *(c + i) & 0x000000ff;
            // escape if needed
            if (unescaped == '(' || unescaped == ')' || unescaped == '\\')
                outStr->printf("%c", '\\');
            outStr->printf("%c", unescaped);
        }
        outStr->printf(") ");
    } else {
        const char *c = s->c_str();
        outStr->printf("(");
        for (int i = 0; i < s->getLength(); i++) {
            char unescaped = *(c + i) & 0x000000ff;
            // escape if needed
            if (unescaped == '\r')
                outStr->printf("\\r");
            else if (unescaped == '\n')
                outStr->printf("\\n");
            else {
                if (unescaped == '(' || unescaped == ')' || unescaped == '\\') {
                    outStr->printf("%c", '\\');
                }
                outStr->printf("%c", unescaped);
            }
        }
        outStr->printf(") ");
    }

    delete sEnc;
}

Goffset PDFDoc::writeObjectHeader(Ref *ref, OutStream *outStr)
{
    Goffset offset = outStr->getPos();
    outStr->printf("%i %i obj\r\n", ref->num, ref->gen);
    return offset;
}

void PDFDoc::writeObject(Object *obj, OutStream *outStr, XRef *xRef, unsigned int numOffset, unsigned char *fileKey, CryptAlgorithm encAlgorithm, int keyLength, int objNum, int objGen, std::set<Dict *> *alreadyWrittenDicts)
{
    writeObject(obj, outStr, xRef, numOffset, fileKey, encAlgorithm, keyLength, { objNum, objGen }, alreadyWrittenDicts);
}

void PDFDoc::writeObject(Object *obj, OutStream *outStr, XRef *xRef, unsigned int numOffset, unsigned char *fileKey, CryptAlgorithm encAlgorithm, int keyLength, Ref ref, std::set<Dict *> *alreadyWrittenDicts)
{
    Array *array;

    switch (obj->getType()) {
    case objBool:
        outStr->printf("%s ", obj->getBool() ? "true" : "false");
        break;
    case objInt:
        outStr->printf("%i ", obj->getInt());
        break;
    case objInt64:
        outStr->printf("%lli ", obj->getInt64());
        break;
    case objReal: {
        GooString s;
        s.appendf("{0:.10g}", obj->getReal());
        outStr->printf("%s ", s.c_str());
        break;
    }
    case objString:
        writeString(obj->getString(), outStr, fileKey, encAlgorithm, keyLength, ref);
        break;
    case objName: {
        GooString name(obj->getName());
        GooString *nameToPrint = name.sanitizedName(false /* non ps mode */);
        outStr->printf("/%s ", nameToPrint->c_str());
        delete nameToPrint;
        break;
    }
    case objNull:
        outStr->printf("null ");
        break;
    case objArray:
        array = obj->getArray();
        outStr->printf("[");
        for (int i = 0; i < array->getLength(); i++) {
            Object obj1 = array->getNF(i).copy();
            writeObject(&obj1, outStr, xRef, numOffset, fileKey, encAlgorithm, keyLength, ref);
        }
        outStr->printf("] ");
        break;
    case objDict:
        writeDictionnary(obj->getDict(), outStr, xRef, numOffset, fileKey, encAlgorithm, keyLength, ref, alreadyWrittenDicts);
        break;
    case objStream: {
        // We can't modify stream with the current implementation (no write functions in Stream API)
        // => the only type of streams which that have been modified are internal streams (=strWeird)
        Stream *stream = obj->getStream();
        if (stream->getKind() == strWeird || stream->getKind() == strCrypt) {
            // we write the stream unencoded => TODO: write stream encoder

            // Encrypt stream
            EncryptStream *encStream = nullptr;
            bool removeFilter = true;
            if (stream->getKind() == strWeird && fileKey) {
                Object filter = stream->getDict()->lookup("Filter");
                if (!filter.isName("Crypt")) {
                    if (filter.isArray()) {
                        for (int i = 0; i < filter.arrayGetLength(); i++) {
                            Object filterEle = filter.arrayGet(i);
                            if (filterEle.isName("Crypt")) {
                                removeFilter = false;
                                break;
                            }
                        }
                        if (removeFilter) {
                            encStream = new EncryptStream(stream, fileKey, encAlgorithm, keyLength, ref);
                            encStream->setAutoDelete(false);
                            stream = encStream;
                        }
                    } else {
                        encStream = new EncryptStream(stream, fileKey, encAlgorithm, keyLength, ref);
                        encStream->setAutoDelete(false);
                        stream = encStream;
                    }
                } else {
                    removeFilter = false;
                }
            } else if (fileKey != nullptr) { // Encrypt stream
                encStream = new EncryptStream(stream, fileKey, encAlgorithm, keyLength, ref);
                encStream->setAutoDelete(false);
                stream = encStream;
            }

            stream->reset();
            // recalculate stream length
            Goffset tmp = 0;
            for (int c = stream->getChar(); c != EOF; c = stream->getChar()) {
                tmp++;
            }
            stream->getDict()->set("Length", Object(tmp));

            // Remove Stream encoding
            if (removeFilter) {
                stream->getDict()->remove("Filter");
            }
            stream->getDict()->remove("DecodeParms");

            writeDictionnary(stream->getDict(), outStr, xRef, numOffset, fileKey, encAlgorithm, keyLength, ref, alreadyWrittenDicts);
            writeStream(stream, outStr);
            delete encStream;
        } else if (fileKey != nullptr && stream->getKind() == strFile && static_cast<FileStream *>(stream)->getNeedsEncryptionOnSave()) {
            EncryptStream *encStream = new EncryptStream(stream, fileKey, encAlgorithm, keyLength, ref);
            encStream->setAutoDelete(false);
            writeDictionnary(encStream->getDict(), outStr, xRef, numOffset, fileKey, encAlgorithm, keyLength, ref, alreadyWrittenDicts);
            writeStream(encStream, outStr);
            delete encStream;
        } else {
            // raw stream copy
            FilterStream *fs = dynamic_cast<FilterStream *>(stream);
            if (fs) {
                BaseStream *bs = fs->getBaseStream();
                if (bs) {
                    Goffset streamEnd;
                    if (xRef->getStreamEnd(bs->getStart(), &streamEnd)) {
                        Goffset val = streamEnd - bs->getStart();
                        stream->getDict()->set("Length", Object(val));
                    }
                }
            }
            writeDictionnary(stream->getDict(), outStr, xRef, numOffset, fileKey, encAlgorithm, keyLength, ref, alreadyWrittenDicts);
            writeRawStream(stream, outStr);
        }
        break;
    }
    case objRef:
        outStr->printf("%i %i R ", obj->getRef().num + numOffset, obj->getRef().gen);
        break;
    case objCmd:
        outStr->printf("%s\n", obj->getCmd());
        break;
    case objError:
        outStr->printf("error\r\n");
        break;
    case objEOF:
        outStr->printf("eof\r\n");
        break;
    case objNone:
        outStr->printf("none\r\n");
        break;
    default:
        error(errUnimplemented, -1, "Unhandled objType : {0:d}, please report a bug with a testcase\r\n", obj->getType());
        break;
    }
}

void PDFDoc::writeObjectFooter(OutStream *outStr)
{
    outStr->printf("\r\nendobj\r\n");
}

Object PDFDoc::createTrailerDict(int uxrefSize, bool incrUpdate, Goffset startxRef, Ref *root, XRef *xRef, const char *fileName, Goffset fileSize)
{
    Dict *trailerDict = new Dict(xRef);
    trailerDict->set("Size", Object(uxrefSize));

    // build a new ID, as recommended in the reference, uses:
    // - current time
    // - file name
    // - file size
    // - values of entry in information dictionnary
    GooString message;
    char buffer[256];
    sprintf(buffer, "%i", (int)time(nullptr));
    message.append(buffer);

    if (fileName)
        message.append(fileName);

    sprintf(buffer, "%lli", (long long)fileSize);
    message.append(buffer);

    // info dict -- only use text string
    if (!xRef->getTrailerDict()->isNone()) {
        Object docInfo = xRef->getDocInfo();
        if (docInfo.isDict()) {
            for (int i = 0; i < docInfo.getDict()->getLength(); i++) {
                Object obj2 = docInfo.getDict()->getVal(i);
                if (obj2.isString()) {
                    message.append(obj2.getString());
                }
            }
        }
    }

    bool hasEncrypt = false;
    if (!xRef->getTrailerDict()->isNone()) {
        Object obj2 = xRef->getTrailerDict()->dictLookupNF("Encrypt").copy();
        if (!obj2.isNull()) {
            trailerDict->set("Encrypt", std::move(obj2));
            hasEncrypt = true;
        }
    }

    // calculate md5 digest
    unsigned char digest[16];
    md5((unsigned char *)message.c_str(), message.getLength(), digest);

    // create ID array
    // In case of encrypted files, the ID must not be changed because it's used to calculate the key
    if (incrUpdate || hasEncrypt) {
        // only update the second part of the array
        Object obj4 = xRef->getTrailerDict()->getDict()->lookup("ID");
        if (!obj4.isArray()) {
            error(errSyntaxWarning, -1, "PDFDoc::createTrailerDict original file's ID entry isn't an array. Trying to continue");
        } else {
            Array *array = new Array(xRef);
            // Get the first part of the ID
            array->add(obj4.arrayGet(0));
            array->add(Object(new GooString((const char *)digest, 16)));
            trailerDict->set("ID", Object(array));
        }
    } else {
        // new file => same values for the two identifiers
        Array *array = new Array(xRef);
        array->add(Object(new GooString((const char *)digest, 16)));
        array->add(Object(new GooString((const char *)digest, 16)));
        trailerDict->set("ID", Object(array));
    }

    trailerDict->set("Root", Object(*root));

    if (incrUpdate) {
        trailerDict->set("Prev", Object(startxRef));
    }

    if (!xRef->getTrailerDict()->isNone()) {
        Object obj5 = xRef->getDocInfoNF();
        if (!obj5.isNull()) {
            trailerDict->set("Info", std::move(obj5));
        }
    }

    return Object(trailerDict);
}

void PDFDoc::writeXRefTableTrailer(Object &&trailerDict, XRef *uxref, bool writeAllEntries, Goffset uxrefOffset, OutStream *outStr, XRef *xRef)
{
    uxref->writeTableToFile(outStr, writeAllEntries);
    outStr->printf("trailer\r\n");
    writeDictionnary(trailerDict.getDict(), outStr, xRef, 0, nullptr, cryptRC4, 0, { 0, 0 }, nullptr);
    outStr->printf("\r\nstartxref\r\n");
    outStr->printf("%lli\r\n", uxrefOffset);
    outStr->printf("%%%%EOF\r\n");
}

void PDFDoc::writeXRefStreamTrailer(Object &&trailerDict, XRef *uxref, Ref *uxrefStreamRef, Goffset uxrefOffset, OutStream *outStr, XRef *xRef)
{
    GooString stmData;

    // Fill stmData and some trailerDict fields
    uxref->writeStreamToBuffer(&stmData, trailerDict.getDict(), xRef);

    // Create XRef stream object and write it
    MemStream *mStream = new MemStream(stmData.c_str(), 0, stmData.getLength(), std::move(trailerDict));
    writeObjectHeader(uxrefStreamRef, outStr);
    Object obj1(static_cast<Stream *>(mStream));
    writeObject(&obj1, outStr, xRef, 0, nullptr, cryptRC4, 0, 0, 0);
    writeObjectFooter(outStr);

    outStr->printf("startxref\r\n");
    outStr->printf("%lli\r\n", uxrefOffset);
    outStr->printf("%%%%EOF\r\n");
}

void PDFDoc::writeXRefTableTrailer(Goffset uxrefOffset, XRef *uxref, bool writeAllEntries, int uxrefSize, OutStream *outStr, bool incrUpdate)
{
    const char *fileNameA = fileName ? fileName->c_str() : nullptr;
    // file size (doesn't include the trailer)
    unsigned int fileSize = 0;
    int c;
    str->reset();
    while ((c = str->getChar()) != EOF) {
        fileSize++;
    }
    str->close();
    Ref ref;
    ref.num = getXRef()->getRootNum();
    ref.gen = getXRef()->getRootGen();
    Object trailerDict = createTrailerDict(uxrefSize, incrUpdate, getStartXRef(), &ref, getXRef(), fileNameA, fileSize);
    writeXRefTableTrailer(std::move(trailerDict), uxref, writeAllEntries, uxrefOffset, outStr, getXRef());
}

void PDFDoc::writeHeader(OutStream *outStr, int major, int minor)
{
    outStr->printf("%%PDF-%d.%d\n", major, minor);
    outStr->printf("%%%c%c%c%c\n", 0xE2, 0xE3, 0xCF, 0xD3);
}

void PDFDoc::markDictionnary(Dict *dict, XRef *xRef, XRef *countRef, unsigned int numOffset, int oldRefNum, int newRefNum, std::set<Dict *> *alreadyMarkedDicts)
{
    bool deleteSet = false;
    if (!alreadyMarkedDicts) {
        alreadyMarkedDicts = new std::set<Dict *>;
        deleteSet = true;
    }

    if (alreadyMarkedDicts->find(dict) != alreadyMarkedDicts->end()) {
        error(errSyntaxWarning, -1, "PDFDoc::markDictionnary: Found recursive dicts");
        if (deleteSet)
            delete alreadyMarkedDicts;
        return;
    } else {
        alreadyMarkedDicts->insert(dict);
    }

    for (int i = 0; i < dict->getLength(); i++) {
        const char *key = dict->getKey(i);
        if (strcmp(key, "Annots") != 0) {
            Object obj1 = dict->getValNF(i).copy();
            markObject(&obj1, xRef, countRef, numOffset, oldRefNum, newRefNum, alreadyMarkedDicts);
        } else {
            Object annotsObj = dict->getValNF(i).copy();
            if (!annotsObj.isNull()) {
                markAnnotations(&annotsObj, xRef, countRef, 0, oldRefNum, newRefNum, alreadyMarkedDicts);
            }
        }
    }

    if (deleteSet) {
        delete alreadyMarkedDicts;
    }
}

void PDFDoc::markObject(Object *obj, XRef *xRef, XRef *countRef, unsigned int numOffset, int oldRefNum, int newRefNum, std::set<Dict *> *alreadyMarkedDicts)
{
    Array *array;

    switch (obj->getType()) {
    case objArray:
        array = obj->getArray();
        for (int i = 0; i < array->getLength(); i++) {
            Object obj1 = array->getNF(i).copy();
            markObject(&obj1, xRef, countRef, numOffset, oldRefNum, newRefNum, alreadyMarkedDicts);
        }
        break;
    case objDict:
        markDictionnary(obj->getDict(), xRef, countRef, numOffset, oldRefNum, newRefNum, alreadyMarkedDicts);
        break;
    case objStream: {
        Stream *stream = obj->getStream();
        markDictionnary(stream->getDict(), xRef, countRef, numOffset, oldRefNum, newRefNum, alreadyMarkedDicts);
    } break;
    case objRef: {
        if (obj->getRef().num + (int)numOffset >= xRef->getNumObjects() || xRef->getEntry(obj->getRef().num + numOffset)->type == xrefEntryFree) {
            if (getXRef()->getEntry(obj->getRef().num)->type == xrefEntryFree) {
                return; // already marked as free => should be replaced
            }
            xRef->add(obj->getRef().num + numOffset, obj->getRef().gen, 0, true);
            if (getXRef()->getEntry(obj->getRef().num)->type == xrefEntryCompressed) {
                xRef->getEntry(obj->getRef().num + numOffset)->type = xrefEntryCompressed;
            }
        }
        if (obj->getRef().num + (int)numOffset >= countRef->getNumObjects() || countRef->getEntry(obj->getRef().num + numOffset)->type == xrefEntryFree) {
            countRef->add(obj->getRef().num + numOffset, 1, 0, true);
        } else {
            XRefEntry *entry = countRef->getEntry(obj->getRef().num + numOffset);
            entry->gen++;
            if (entry->gen > 9)
                break;
        }
        Object obj1 = getXRef()->fetch(obj->getRef());
        markObject(&obj1, xRef, countRef, numOffset, oldRefNum, newRefNum);
    } break;
    default:
        break;
    }
}

void PDFDoc::replacePageDict(int pageNo, int rotate, const PDFRectangle *mediaBox, const PDFRectangle *cropBox)
{
    Ref *refPage = getCatalog()->getPageRef(pageNo);
    Object page = getXRef()->fetch(*refPage);
    Dict *pageDict = page.getDict();
    pageDict->remove("MediaBoxssdf");
    pageDict->remove("MediaBox");
    pageDict->remove("CropBox");
    pageDict->remove("ArtBox");
    pageDict->remove("BleedBox");
    pageDict->remove("TrimBox");
    pageDict->remove("Rotate");
    Array *mediaBoxArray = new Array(getXRef());
    mediaBoxArray->add(Object(mediaBox->x1));
    mediaBoxArray->add(Object(mediaBox->y1));
    mediaBoxArray->add(Object(mediaBox->x2));
    mediaBoxArray->add(Object(mediaBox->y2));
    Object mediaBoxObject(mediaBoxArray);
    Object trimBoxObject = mediaBoxObject.copy();
    pageDict->add("MediaBox", std::move(mediaBoxObject));
    if (cropBox != nullptr) {
        Array *cropBoxArray = new Array(getXRef());
        cropBoxArray->add(Object(cropBox->x1));
        cropBoxArray->add(Object(cropBox->y1));
        cropBoxArray->add(Object(cropBox->x2));
        cropBoxArray->add(Object(cropBox->y2));
        Object cropBoxObject(cropBoxArray);
        trimBoxObject = cropBoxObject.copy();
        pageDict->add("CropBox", std::move(cropBoxObject));
    }
    pageDict->add("TrimBox", std::move(trimBoxObject));
    pageDict->add("Rotate", Object(rotate));
    getXRef()->setModifiedObject(&page, *refPage);
}

void PDFDoc::markPageObjects(Dict *pageDict, XRef *xRef, XRef *countRef, unsigned int numOffset, int oldRefNum, int newRefNum, std::set<Dict *> *alreadyMarkedDicts)
{
    pageDict->remove("OpenAction");
    pageDict->remove("Outlines");
    pageDict->remove("StructTreeRoot");

    for (int n = 0; n < pageDict->getLength(); n++) {
        const char *key = pageDict->getKey(n);
        Object value = pageDict->getValNF(n).copy();
        if (strcmp(key, "Parent") != 0 && strcmp(key, "Pages") != 0 && strcmp(key, "AcroForm") != 0 && strcmp(key, "Annots") != 0 && strcmp(key, "P") != 0 && strcmp(key, "Root") != 0) {
            markObject(&value, xRef, countRef, numOffset, oldRefNum, newRefNum, alreadyMarkedDicts);
        }
    }
}

bool PDFDoc::markAnnotations(Object *annotsObj, XRef *xRef, XRef *countRef, unsigned int numOffset, int oldPageNum, int newPageNum, std::set<Dict *> *alreadyMarkedDicts)
{
    bool modified = false;
    Object annots = annotsObj->fetch(getXRef());
    if (annots.isArray()) {
        Array *array = annots.getArray();
        for (int i = array->getLength() - 1; i >= 0; i--) {
            Object obj1 = array->get(i);
            if (obj1.isDict()) {
                Dict *dict = obj1.getDict();
                Object type = dict->lookup("Type");
                if (type.isName() && strcmp(type.getName(), "Annot") == 0) {
                    const Object &obj2 = dict->lookupNF("P");
                    if (obj2.isRef()) {
                        if (obj2.getRef().num == oldPageNum) {
                            const Object &obj3 = array->getNF(i);
                            if (obj3.isRef()) {
                                Ref r;
                                r.num = newPageNum;
                                r.gen = 0;
                                dict->set("P", Object(r));
                                getXRef()->setModifiedObject(&obj1, obj3.getRef());
                            }
                        } else if (obj2.getRef().num == newPageNum) {
                            continue;
                        } else {
                            Object page = getXRef()->fetch(obj2.getRef());
                            if (page.isDict()) {
                                Dict *pageDict = page.getDict();
                                Object pagetype = pageDict->lookup("Type");
                                if (!pagetype.isName() || strcmp(pagetype.getName(), "Page") != 0) {
                                    continue;
                                }
                            }
                            array->remove(i);
                            modified = true;
                            continue;
                        }
                    }
                }
                markPageObjects(dict, xRef, countRef, numOffset, oldPageNum, newPageNum, alreadyMarkedDicts);
            }
            obj1 = array->getNF(i).copy();
            if (obj1.isRef()) {
                if (obj1.getRef().num + (int)numOffset >= xRef->getNumObjects() || xRef->getEntry(obj1.getRef().num + numOffset)->type == xrefEntryFree) {
                    if (getXRef()->getEntry(obj1.getRef().num)->type == xrefEntryFree) {
                        continue; // already marked as free => should be replaced
                    }
                    xRef->add(obj1.getRef().num + numOffset, obj1.getRef().gen, 0, true);
                    if (getXRef()->getEntry(obj1.getRef().num)->type == xrefEntryCompressed) {
                        xRef->getEntry(obj1.getRef().num + numOffset)->type = xrefEntryCompressed;
                    }
                }
                if (obj1.getRef().num + (int)numOffset >= countRef->getNumObjects() || countRef->getEntry(obj1.getRef().num + numOffset)->type == xrefEntryFree) {
                    countRef->add(obj1.getRef().num + numOffset, 1, 0, true);
                } else {
                    XRefEntry *entry = countRef->getEntry(obj1.getRef().num + numOffset);
                    entry->gen++;
                }
            }
        }
    }
    if (annotsObj->isRef()) {
        if (annotsObj->getRef().num + (int)numOffset >= xRef->getNumObjects() || xRef->getEntry(annotsObj->getRef().num + numOffset)->type == xrefEntryFree) {
            if (getXRef()->getEntry(annotsObj->getRef().num)->type == xrefEntryFree) {
                return modified; // already marked as free => should be replaced
            }
            xRef->add(annotsObj->getRef().num + numOffset, annotsObj->getRef().gen, 0, true);
            if (getXRef()->getEntry(annotsObj->getRef().num)->type == xrefEntryCompressed) {
                xRef->getEntry(annotsObj->getRef().num + numOffset)->type = xrefEntryCompressed;
            }
        }
        if (annotsObj->getRef().num + (int)numOffset >= countRef->getNumObjects() || countRef->getEntry(annotsObj->getRef().num + numOffset)->type == xrefEntryFree) {
            countRef->add(annotsObj->getRef().num + numOffset, 1, 0, true);
        } else {
            XRefEntry *entry = countRef->getEntry(annotsObj->getRef().num + numOffset);
            entry->gen++;
        }
        getXRef()->setModifiedObject(&annots, annotsObj->getRef());
    }
    return modified;
}

void PDFDoc::markAcroForm(Object *afObj, XRef *xRef, XRef *countRef, unsigned int numOffset, int oldRefNum, int newRefNum)
{
    bool modified = false;
    Object acroform = afObj->fetch(getXRef());
    if (acroform.isDict()) {
        Dict *dict = acroform.getDict();
        for (int i = 0; i < dict->getLength(); i++) {
            if (strcmp(dict->getKey(i), "Fields") == 0) {
                Object fields = dict->getValNF(i).copy();
                modified = markAnnotations(&fields, xRef, countRef, numOffset, oldRefNum, newRefNum);
            } else {
                Object obj = dict->getValNF(i).copy();
                markObject(&obj, xRef, countRef, numOffset, oldRefNum, newRefNum);
            }
        }
    }
    if (afObj->isRef()) {
        if (afObj->getRef().num + (int)numOffset >= xRef->getNumObjects() || xRef->getEntry(afObj->getRef().num + numOffset)->type == xrefEntryFree) {
            if (getXRef()->getEntry(afObj->getRef().num)->type == xrefEntryFree) {
                return; // already marked as free => should be replaced
            }
            xRef->add(afObj->getRef().num + numOffset, afObj->getRef().gen, 0, true);
            if (getXRef()->getEntry(afObj->getRef().num)->type == xrefEntryCompressed) {
                xRef->getEntry(afObj->getRef().num + numOffset)->type = xrefEntryCompressed;
            }
        }
        if (afObj->getRef().num + (int)numOffset >= countRef->getNumObjects() || countRef->getEntry(afObj->getRef().num + numOffset)->type == xrefEntryFree) {
            countRef->add(afObj->getRef().num + numOffset, 1, 0, true);
        } else {
            XRefEntry *entry = countRef->getEntry(afObj->getRef().num + numOffset);
            entry->gen++;
        }
        if (modified) {
            getXRef()->setModifiedObject(&acroform, afObj->getRef());
        }
    }
    return;
}

unsigned int PDFDoc::writePageObjects(OutStream *outStr, XRef *xRef, unsigned int numOffset, bool combine)
{
    unsigned int objectsCount = 0; // count the number of objects in the XRef(s)
    unsigned char *fileKey;
    CryptAlgorithm encAlgorithm;
    int keyLength;
    xRef->getEncryptionParameters(&fileKey, &encAlgorithm, &keyLength);

    for (int n = numOffset; n < xRef->getNumObjects(); n++) {
        if (xRef->getEntry(n)->type != xrefEntryFree) {
            Ref ref;
            ref.num = n;
            ref.gen = xRef->getEntry(n)->gen;
            objectsCount++;
            Object obj = getXRef()->fetch(ref.num - numOffset, ref.gen);
            Goffset offset = writeObjectHeader(&ref, outStr);
            if (combine) {
                writeObject(&obj, outStr, getXRef(), numOffset, nullptr, cryptRC4, 0, 0, 0);
            } else if (xRef->getEntry(n)->getFlag(XRefEntry::Unencrypted)) {
                writeObject(&obj, outStr, nullptr, cryptRC4, 0, 0, 0);
            } else {
                writeObject(&obj, outStr, fileKey, encAlgorithm, keyLength, ref);
            }
            writeObjectFooter(outStr);
            xRef->add(ref, offset, true);
        }
    }
    return objectsCount;
}

Outline *PDFDoc::getOutline()
{
    if (!outline) {
        pdfdocLocker();
        // read outline
        outline = new Outline(catalog->getOutline(), xref);
    }

    return outline;
}

PDFDoc *PDFDoc::ErrorPDFDoc(int errorCode, const GooString *fileNameA)
{
    PDFDoc *doc = new PDFDoc();
    doc->errCode = errorCode;
    doc->fileName = fileNameA;

    return doc;
}

long long PDFDoc::strToLongLong(const char *s)
{
    long long x, d;
    const char *p;

    x = 0;
    for (p = s; *p && isdigit(*p & 0xff); ++p) {
        d = *p - '0';
        if (x > (LLONG_MAX - d) / 10) {
            break;
        }
        x = 10 * x + d;
    }
    return x;
}

// Read the 'startxref' position.
Goffset PDFDoc::getStartXRef(bool tryingToReconstruct)
{
    if (startXRefPos == -1) {

        if (isLinearized(tryingToReconstruct)) {
            char buf[linearizationSearchSize + 1];
            int c, n, i;

            str->setPos(0);
            for (n = 0; n < linearizationSearchSize; ++n) {
                if ((c = str->getChar()) == EOF) {
                    break;
                }
                buf[n] = c;
            }
            buf[n] = '\0';

            // find end of first obj (linearization dictionary)
            startXRefPos = 0;
            for (i = 0; i < n; i++) {
                if (!strncmp("endobj", &buf[i], 6)) {
                    i += 6;
                    // skip whitespace
                    while (buf[i] && Lexer::isSpace(buf[i]))
                        ++i;
                    startXRefPos = i;
                    break;
                }
            }
        } else {
            char buf[xrefSearchSize + 1];
            const char *p;
            int c, n, i;

            // read last xrefSearchSize bytes
            int segnum = 0;
            int maxXRefSearch = 24576;
            if (str->getLength() < maxXRefSearch)
                maxXRefSearch = str->getLength();
            for (; (xrefSearchSize - 16) * segnum < maxXRefSearch; segnum++) {
                str->setPos((xrefSearchSize - 16) * segnum + xrefSearchSize, -1);
                for (n = 0; n < xrefSearchSize; ++n) {
                    if ((c = str->getChar()) == EOF) {
                        break;
                    }
                    buf[n] = c;
                }
                buf[n] = '\0';

                // find startxref
                for (i = n - 9; i >= 0; --i) {
                    if (!strncmp(&buf[i], "startxref", 9)) {
                        break;
                    }
                }
                if (i < 0) {
                    startXRefPos = 0;
                } else {
                    for (p = &buf[i + 9]; isspace(*p); ++p)
                        ;
                    startXRefPos = strToLongLong(p);
                    break;
                }
            }
        }
    }

    return startXRefPos;
}

Goffset PDFDoc::getMainXRefEntriesOffset(bool tryingToReconstruct)
{
    unsigned int mainXRefEntriesOffset = 0;

    if (isLinearized(tryingToReconstruct)) {
        mainXRefEntriesOffset = getLinearization()->getMainXRefEntriesOffset();
    }

    return mainXRefEntriesOffset;
}

int PDFDoc::getNumPages()
{
    if (isLinearized()) {
        int n;
        if ((n = getLinearization()->getNumPages())) {
            return n;
        }
    }

    return catalog->getNumPages();
}

Page *PDFDoc::parsePage(int page)
{
    Ref pageRef;

    pageRef.num = getHints()->getPageObjectNum(page);
    if (!pageRef.num) {
        error(errSyntaxWarning, -1, "Failed to get object num from hint tables for page {0:d}", page);
        return nullptr;
    }

    // check for bogus ref - this can happen in corrupted PDF files
    if (pageRef.num < 0 || pageRef.num >= xref->getNumObjects()) {
        error(errSyntaxWarning, -1, "Invalid object num ({0:d}) for page {1:d}", pageRef.num, page);
        return nullptr;
    }

    pageRef.gen = xref->getEntry(pageRef.num)->gen;
    Object obj = xref->fetch(pageRef);
    if (!obj.isDict("Page")) {
        error(errSyntaxWarning, -1, "Object ({0:d} {1:d}) is not a pageDict", pageRef.num, pageRef.gen);
        return nullptr;
    }
    Dict *pageDict = obj.getDict();

    return new Page(this, page, std::move(obj), pageRef, new PageAttrs(nullptr, pageDict), catalog->getForm());
}

Page *PDFDoc::getPage(int page)
{
    if ((page < 1) || page > getNumPages())
        return nullptr;

    if (isLinearized() && checkLinearization()) {
        pdfdocLocker();
        if (!pageCache) {
            pageCache = (Page **)gmallocn(getNumPages(), sizeof(Page *));
            for (int i = 0; i < getNumPages(); i++) {
                pageCache[i] = nullptr;
            }
        }
        if (!pageCache[page - 1]) {
            pageCache[page - 1] = parsePage(page);
        }
        if (pageCache[page - 1]) {
            return pageCache[page - 1];
        } else {
            error(errSyntaxWarning, -1, "Failed parsing page {0:d} using hint tables", page);
        }
    }

    return catalog->getPage(page);
}

bool PDFDoc::hasJavascript()
{
    JSInfo jsInfo(this);
    jsInfo.scanJS(getNumPages(), true);
    return jsInfo.containsJS();
}
