//========================================================================
//
// XRef.cc
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
// Copyright (C) 2005 Dan Sheridan <dan.sheridan@postman.org.uk>
// Copyright (C) 2005 Brad Hards <bradh@frogmouth.net>
// Copyright (C) 2006, 2008, 2010, 2012-2014, 2016-2023 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2007-2008 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2007 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009, 2010 Ilya Gorenbein <igorenbein@finjan.com>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2012, 2013, 2016 Thomas Freitag <Thomas.Freitag@kabelmail.de>
// Copyright (C) 2012, 2013 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2013, 2014, 2017, 2019 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 Pino Toscano <pino@kde.org>
// Copyright (C) 2016 Jakub Alba <jakubalba@gmail.com>
// Copyright (C) 2018, 2019 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2018 Tobias Deiminger <haxtibal@posteo.de>
// Copyright (C) 2019 LE GARREC Vincent <legarrec.vincent@gmail.com>
// Copyright (C) 2020 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by Technische Universität Dresden
// Copyright (C) 2010 William Bader <william@newspapersystems.com>
// Copyright (C) 2021 Mahmoud Khalil <mahmoudkhalil11@gmail.com>
// Copyright (C) 2021 Georgiy Sgibnev <georgiy@sgibnev.com>. Work sponsored by lab50.net.
// Copyright (C) 2023 g10 Code GmbH, Author: Sune Stolborg Vuorela <sune@vuorela.dk>
// Copyright (C) 2023 Ilaï Deutel <idtl@google.com>
// Copyright (C) 2023 Even Rouault <even.rouault@spatialys.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>
#include "poppler-config.h"

#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <cmath>
#include <cctype>
#include <climits>
#include <cfloat>
#include <limits>
#include "goo/gfile.h"
#include "goo/gmem.h"
#include "Object.h"
#include "Stream.h"
#include "Lexer.h"
#include "Parser.h"
#include "Dict.h"
#include "Error.h"
#include "ErrorCodes.h"
#include "XRef.h"

//------------------------------------------------------------------------
// Permission bits
// Note that the PDF spec uses 1 base (eg bit 3 is 1<<2)
//------------------------------------------------------------------------

#define permPrint (1 << 2) // bit 3
#define permChange (1 << 3) // bit 4
#define permCopy (1 << 4) // bit 5
#define permNotes (1 << 5) // bit 6
#define permFillForm (1 << 8) // bit 9
#define permAccessibility (1 << 9) // bit 10
#define permAssemble (1 << 10) // bit 11
#define permHighResPrint (1 << 11) // bit 12
#define defPermFlags 0xfffc

//------------------------------------------------------------------------
// ObjectStream
//------------------------------------------------------------------------

class ObjectStream
{
public:
    // Create an object stream, using object number <objStrNum>,
    // generation 0.
    ObjectStream(XRef *xref, int objStrNumA, int recursion = 0);

    bool isOk() { return ok; }

    ~ObjectStream();

    ObjectStream(const ObjectStream &) = delete;
    ObjectStream &operator=(const ObjectStream &) = delete;

    // Return the object number of this object stream.
    int getObjStrNum() { return objStrNum; }

    // Get the <objIdx>th object from this stream, which should be
    // object number <objNum>, generation 0.
    Object getObject(int objIdx, int objNum);

private:
    int objStrNum; // object number of the object stream
    int nObjects; // number of objects in the stream
    Object *objs; // the objects (length = nObjects)
    int *objNums; // the object numbers (length = nObjects)
    bool ok;
};

ObjectStream::ObjectStream(XRef *xref, int objStrNumA, int recursion)
{
    Stream *str;
    Parser *parser;
    Goffset *offsets;
    Object objStr, obj1;
    Goffset first;
    int i;

    objStrNum = objStrNumA;
    nObjects = 0;
    objs = nullptr;
    objNums = nullptr;
    ok = false;

    objStr = xref->fetch(objStrNum, 0, recursion);
    if (!objStr.isStream()) {
        return;
    }

    obj1 = objStr.streamGetDict()->lookup("N", recursion);
    if (!obj1.isInt()) {
        return;
    }
    nObjects = obj1.getInt();
    if (nObjects <= 0) {
        return;
    }

    obj1 = objStr.streamGetDict()->lookup("First", recursion);
    if (!obj1.isInt() && !obj1.isInt64()) {
        return;
    }
    if (obj1.isInt()) {
        first = obj1.getInt();
    } else {
        first = obj1.getInt64();
    }
    if (first < 0) {
        return;
    }

    // this is an arbitrary limit to avoid integer overflow problems
    // in the 'new Object[nObjects]' call (Acrobat apparently limits
    // object streams to 100-200 objects)
    if (nObjects > 1000000) {
        error(errSyntaxError, -1, "Too many objects in an object stream");
        return;
    }
    objs = new Object[nObjects];
    objNums = (int *)gmallocn(nObjects, sizeof(int));
    offsets = (Goffset *)gmallocn(nObjects, sizeof(Goffset));

    // parse the header: object numbers and offsets
    objStr.streamReset();
    str = new EmbedStream(objStr.getStream(), Object(objNull), true, first);
    parser = new Parser(xref, str, false);
    for (i = 0; i < nObjects; ++i) {
        obj1 = parser->getObj();
        Object obj2 = parser->getObj();
        if (!obj1.isInt() || !(obj2.isInt() || obj2.isInt64())) {
            delete parser;
            gfree(offsets);
            return;
        }
        objNums[i] = obj1.getInt();
        if (obj2.isInt()) {
            offsets[i] = obj2.getInt();
        } else {
            offsets[i] = obj2.getInt64();
        }
        if (objNums[i] < 0 || offsets[i] < 0 || (i > 0 && offsets[i] < offsets[i - 1])) {
            delete parser;
            gfree(offsets);
            return;
        }
    }
    while (str->getChar() != EOF) {
        ;
    }
    delete parser;

    // skip to the first object - this shouldn't be necessary because
    // the First key is supposed to be equal to offsets[0], but just in
    // case...
    for (Goffset pos = first; pos < offsets[0]; ++pos) {
        objStr.getStream()->getChar();
    }

    // parse the objects
    for (i = 0; i < nObjects; ++i) {
        if (i == nObjects - 1) {
            str = new EmbedStream(objStr.getStream(), Object(objNull), false, 0);
        } else {
            str = new EmbedStream(objStr.getStream(), Object(objNull), true, offsets[i + 1] - offsets[i]);
        }
        parser = new Parser(xref, str, false);
        objs[i] = parser->getObj();
        while (str->getChar() != EOF) {
            ;
        }
        delete parser;
    }

    gfree(offsets);
    ok = true;
}

ObjectStream::~ObjectStream()
{
    delete[] objs;
    gfree(objNums);
}

Object ObjectStream::getObject(int objIdx, int objNum)
{
    if (objIdx < 0 || objIdx >= nObjects || objNum != objNums[objIdx]) {
        return Object(objNull);
    }
    return objs[objIdx].copy();
}

//------------------------------------------------------------------------
// XRef
//------------------------------------------------------------------------

#define xrefLocker() const std::scoped_lock locker(mutex)

XRef::XRef() : objStrs { 5 }
{
    ok = true;
    errCode = errNone;
    entries = nullptr;
    capacity = 0;
    size = 0;
    modified = false;
    streamEnds = nullptr;
    streamEndsLen = 0;
    mainXRefEntriesOffset = 0;
    xRefStream = false;
    scannedSpecialFlags = false;
    encrypted = false;
    permFlags = defPermFlags;
    ownerPasswordOk = false;
    rootNum = -1;
    strOwner = false;
    xrefReconstructed = false;
    encAlgorithm = cryptNone;
    keyLength = 0;
}

XRef::XRef(const Object *trailerDictA) : XRef {}
{
    if (trailerDictA->isDict()) {
        trailerDict = trailerDictA->copy();
    }
}

XRef::XRef(BaseStream *strA, Goffset pos, Goffset mainXRefEntriesOffsetA, bool *wasReconstructed, bool reconstruct, const std::function<void()> &xrefReconstructedCallback) : XRef {}
{
    Object obj;

    mainXRefEntriesOffset = mainXRefEntriesOffsetA;

    xrefReconstructedCb = xrefReconstructedCallback;

    // read the trailer
    str = strA;
    start = str->getStart();
    prevXRefOffset = mainXRefOffset = pos;

    if (reconstruct && !(ok = constructXRef(wasReconstructed))) {
        errCode = errDamaged;
        return;
    } else {
        // if there was a problem with the 'startxref' position, try to
        // reconstruct the xref table
        if (prevXRefOffset == 0) {
            if (!(ok = constructXRef(wasReconstructed))) {
                errCode = errDamaged;
                return;
            }

            // read the xref table
        } else {
            std::vector<Goffset> followedXRefStm;
            readXRef(&prevXRefOffset, &followedXRefStm, nullptr);

            // if there was a problem with the xref table,
            // try to reconstruct it
            if (!ok) {
                if (!(ok = constructXRef(wasReconstructed))) {
                    errCode = errDamaged;
                    return;
                }
            }
        }

        // set size to (at least) the size specified in trailer dict
        obj = trailerDict.dictLookupNF("Size").copy();
        if (!obj.isInt()) {
            error(errSyntaxWarning, -1, "No valid XRef size in trailer");
        } else {
            if (obj.getInt() > size) {
                if (resize(obj.getInt()) != obj.getInt()) {
                    if (!(ok = constructXRef(wasReconstructed))) {
                        errCode = errDamaged;
                        return;
                    }
                }
            }
        }

        // get the root dictionary (catalog) object
        obj = trailerDict.dictLookupNF("Root").copy();
        if (obj.isRef()) {
            rootNum = obj.getRefNum();
            rootGen = obj.getRefGen();
        } else {
            if (!(ok = constructXRef(wasReconstructed))) {
                errCode = errDamaged;
                return;
            }
        }
    }
    // now set the trailer dictionary's xref pointer so we can fetch
    // indirect objects from it
    trailerDict.getDict()->setXRef(this);
}

XRef::~XRef()
{
    for (int i = 0; i < size; i++) {
        if (entries[i].type == xrefEntryFree) {
            continue;
        }

        entries[i].obj.~Object();
    }
    gfree(entries);

    if (streamEnds) {
        gfree(streamEnds);
    }
    if (strOwner) {
        delete str;
    }
}

XRef *XRef::copy() const
{
    XRef *xref = new XRef();
    xref->str = str->copy();
    xref->strOwner = true;
    xref->encrypted = encrypted;
    xref->permFlags = permFlags;
    xref->ownerPasswordOk = ownerPasswordOk;
    xref->rootGen = rootGen;
    xref->rootNum = rootNum;

    xref->start = start;
    xref->prevXRefOffset = prevXRefOffset;
    xref->mainXRefEntriesOffset = mainXRefEntriesOffset;
    xref->xRefStream = xRefStream;
    xref->trailerDict = trailerDict.copy();
    xref->encAlgorithm = encAlgorithm;
    xref->encRevision = encRevision;
    xref->encVersion = encVersion;
    xref->permFlags = permFlags;
    xref->keyLength = keyLength;
    xref->permFlags = permFlags;
    for (int i = 0; i < 32; i++) {
        xref->fileKey[i] = fileKey[i];
    }

    if (xref->reserve(size) == 0) {
        error(errSyntaxError, -1, "unable to allocate {0:d} entries", size);
        delete xref;
        return nullptr;
    }
    xref->size = size;
    for (int i = 0; i < size; ++i) {
        xref->entries[i].offset = entries[i].offset;
        xref->entries[i].type = entries[i].type;
        // set the object to null, it will be fetched from the stream when needed
        new (&xref->entries[i].obj) Object(objNull);
        xref->entries[i].flags = entries[i].flags;
        xref->entries[i].gen = entries[i].gen;

        // If entry has been changed from the stream value we need to copy it
        // otherwise it's lost
        if (entries[i].getFlag(XRefEntry::Updated)) {
            xref->entries[i].obj = entries[i].obj.copy();
        }
    }
    xref->streamEndsLen = streamEndsLen;
    if (streamEndsLen != 0) {
        xref->streamEnds = (Goffset *)gmalloc(streamEndsLen * sizeof(Goffset));
        for (int i = 0; i < streamEndsLen; i++) {
            xref->streamEnds[i] = streamEnds[i];
        }
    }
    return xref;
}

int XRef::reserve(int newSize)
{
    if (newSize > capacity) {
        int newCapacity = 1024;
        if (capacity) {
            if (capacity <= INT_MAX / 2) {
                newCapacity = capacity * 2;
            } else {
                newCapacity = newSize;
            }
        }
        while (newSize > newCapacity) {
            if (newCapacity > INT_MAX / 2) {
                std::fputs("Too large XRef size\n", stderr);
                return 0;
            }
            newCapacity *= 2;
        }
        if (newCapacity >= INT_MAX / (int)sizeof(XRefEntry)) {
            std::fputs("Too large XRef size\n", stderr);
            return 0;
        }

        void *p = grealloc(entries, newCapacity * sizeof(XRefEntry),
                           /* checkoverflow=*/true);
        if (p == nullptr) {
            return 0;
        }

        entries = (XRefEntry *)p;
        capacity = newCapacity;
    }

    return capacity;
}

int XRef::resize(int newSize)
{
    if (newSize > size) {

        if (reserve(newSize) < newSize) {
            return size;
        }

        for (int i = size; i < newSize; ++i) {
            entries[i].offset = -1;
            entries[i].type = xrefEntryNone;
            new (&entries[i].obj) Object(objNull);
            entries[i].flags = 0;
            entries[i].gen = 0;
        }
    } else {
        for (int i = newSize; i < size; i++) {
            entries[i].obj.~Object();
        }
    }

    size = newSize;

    return size;
}

/* Read one xref table section.  Also reads the associated trailer
 * dictionary, and returns the prev pointer (if any).
 * Arguments:
 *   pos                Points to a Goffset containing the offset of the XRef
 *                      section to be read. If a prev pointer is found, *pos is
 *                      updated with its value
 *   followedXRefStm    Used in case of nested readXRef calls to spot circular
 *                      references in XRefStm pointers
 *   xrefStreamObjsNum  If not NULL, every time a XRef stream is encountered,
 *                      its object number is appended
 * Return value:
 *   true if a prev pointer is found, otherwise false
 */
bool XRef::readXRef(Goffset *pos, std::vector<Goffset> *followedXRefStm, std::vector<int> *xrefStreamObjsNum)
{
    Parser *parser;
    Object obj;
    bool more;

    Goffset parsePos;
    if (unlikely(checkedAdd(start, *pos, &parsePos))) {
        ok = false;
        return false;
    }
    if (parsePos < 0) {
        ok = false;
        return false;
    }

    // start up a parser, parse one token
    parser = new Parser(nullptr, str->makeSubStream(parsePos, false, 0, Object(objNull)), true);
    obj = parser->getObj(true);

    // parse an old-style xref table
    if (obj.isCmd("xref")) {
        more = readXRefTable(parser, pos, followedXRefStm, xrefStreamObjsNum);

        // parse an xref stream
    } else if (obj.isInt()) {
        const int objNum = obj.getInt();
        if (obj = parser->getObj(true), !obj.isInt()) {
            goto err1;
        }
        if (obj = parser->getObj(true), !obj.isCmd("obj")) {
            goto err1;
        }
        if (obj = parser->getObj(), !obj.isStream()) {
            goto err1;
        }
        if (trailerDict.isNone()) {
            xRefStream = true;
        }
        if (xrefStreamObjsNum) {
            xrefStreamObjsNum->push_back(objNum);
        }
        more = readXRefStream(obj.getStream(), pos);

    } else {
        goto err1;
    }

    delete parser;
    return more;

err1:
    delete parser;
    ok = false;
    return false;
}

bool XRef::readXRefTable(Parser *parser, Goffset *pos, std::vector<Goffset> *followedXRefStm, std::vector<int> *xrefStreamObjsNum)
{
    XRefEntry entry;
    bool more;
    Object obj, obj2;
    Goffset pos2;
    int first, n;

    while (true) {
        obj = parser->getObj(true);
        if (obj.isCmd("trailer")) {
            break;
        }
        if (!obj.isInt()) {
            goto err0;
        }
        first = obj.getInt();
        obj = parser->getObj(true);
        if (!obj.isInt()) {
            goto err0;
        }
        n = obj.getInt();
        if (first < 0 || n < 0 || first > INT_MAX - n) {
            goto err0;
        }
        if (first + n > size) {
            if (resize(first + n) != first + n) {
                error(errSyntaxError, -1, "Invalid 'obj' parameters'");
                goto err0;
            }
        }
        for (int i = first; i < first + n; ++i) {
            obj = parser->getObj(true);
            if (obj.isInt()) {
                entry.offset = obj.getInt();
            } else if (obj.isInt64()) {
                entry.offset = obj.getInt64();
            } else {
                goto err0;
            }
            obj = parser->getObj(true);
            if (!obj.isInt()) {
                goto err0;
            }
            entry.gen = obj.getInt();
            entry.flags = 0;
            obj = parser->getObj(true);
            if (obj.isCmd("n")) {
                entry.type = xrefEntryUncompressed;
            } else if (obj.isCmd("f")) {
                entry.type = xrefEntryFree;
            } else {
                goto err0;
            }
            if (entries[i].offset == -1) {
                entries[i].offset = entry.offset;
                entries[i].gen = entry.gen;
                entries[i].type = entry.type;
                entries[i].flags = entry.flags;
                entries[i].obj.setToNull();

                // PDF files of patents from the IBM Intellectual Property
                // Network have a bug: the xref table claims to start at 1
                // instead of 0.
                if (i == 1 && first == 1 && entries[1].offset == 0 && entries[1].gen == 65535 && entries[1].type == xrefEntryFree) {
                    i = first = 0;
                    entries[0].offset = 0;
                    entries[0].gen = 65535;
                    entries[0].type = xrefEntryFree;
                    entries[0].flags = entries[1].flags;
                    entries[0].obj = std::move(entries[1].obj);

                    entries[1].offset = -1;
                    entries[1].obj.setToNull();
                }
            }
        }
    }

    // read the trailer dictionary
    obj = parser->getObj();
    if (!obj.isDict()) {
        goto err0;
    }

    // get the 'Prev' pointer
    obj2 = obj.getDict()->lookupNF("Prev").copy();
    if (obj2.isInt() || obj2.isInt64()) {
        if (obj2.isInt()) {
            pos2 = obj2.getInt();
        } else {
            pos2 = obj2.getInt64();
        }
        if (pos2 != *pos) {
            *pos = pos2;
            more = true;
        } else {
            error(errSyntaxWarning, -1, "Infinite loop in xref table");
            more = false;
        }
    } else if (obj2.isRef()) {
        // certain buggy PDF generators generate "/Prev NNN 0 R" instead
        // of "/Prev NNN"
        pos2 = (unsigned int)obj2.getRefNum();
        if (pos2 != *pos) {
            *pos = pos2;
            more = true;
        } else {
            error(errSyntaxWarning, -1, "Infinite loop in xref table");
            more = false;
        }
    } else {
        more = false;
    }

    // save the first trailer dictionary
    if (trailerDict.isNone()) {
        trailerDict = obj.copy();
    }

    // check for an 'XRefStm' key
    obj2 = obj.getDict()->lookup("XRefStm");
    if (obj2.isInt() || obj2.isInt64()) {
        if (obj2.isInt()) {
            pos2 = obj2.getInt();
        } else {
            pos2 = obj2.getInt64();
        }
        for (size_t i = 0; ok == true && i < followedXRefStm->size(); ++i) {
            if (followedXRefStm->at(i) == pos2) {
                ok = false;
            }
        }
        // Arbitrary limit because otherwise we exhaust the stack
        // calling readXRef + readXRefTable
        if (followedXRefStm->size() > 4096) {
            error(errSyntaxError, -1, "File has more than 4096 XRefStm, aborting");
            ok = false;
        }
        if (ok) {
            followedXRefStm->push_back(pos2);
            readXRef(&pos2, followedXRefStm, xrefStreamObjsNum);
        }
        if (!ok) {
            goto err0;
        }
    }

    return more;

err0:
    ok = false;
    return false;
}

bool XRef::readXRefStream(Stream *xrefStr, Goffset *pos)
{
    int w[3];
    bool more;
    Object obj;

    ok = false;

    Dict *dict = xrefStr->getDict();
    obj = dict->lookupNF("Size").copy();
    if (!obj.isInt()) {
        return false;
    }
    int newSize = obj.getInt();
    if (newSize < 0) {
        return false;
    }
    if (newSize > size) {
        if (resize(newSize) != newSize) {
            error(errSyntaxError, -1, "Invalid 'size' parameter");
            return false;
        }
    }

    obj = dict->lookupNF("W").copy();
    if (!obj.isArray() || obj.arrayGetLength() < 3) {
        return false;
    }
    for (int i = 0; i < 3; ++i) {
        Object obj2 = obj.arrayGet(i);
        if (!obj2.isInt()) {
            return false;
        }
        w[i] = obj2.getInt();
        if (w[i] < 0) {
            return false;
        }
    }
    if (w[0] > (int)sizeof(int) || w[1] > (int)sizeof(long long) || w[2] > (int)sizeof(long long)) {
        return false;
    }

    xrefStr->reset();
    const Object &idx = dict->lookupNF("Index");
    if (idx.isArray()) {
        for (int i = 0; i + 1 < idx.arrayGetLength(); i += 2) {
            obj = idx.arrayGet(i);
            if (!obj.isInt()) {
                return false;
            }
            int first = obj.getInt();
            obj = idx.arrayGet(i + 1);
            if (!obj.isInt()) {
                return false;
            }
            int n = obj.getInt();
            if (first < 0 || n < 0 || !readXRefStreamSection(xrefStr, w, first, n)) {
                return false;
            }
        }
    } else {
        if (!readXRefStreamSection(xrefStr, w, 0, newSize)) {
            return false;
        }
    }

    obj = dict->lookupNF("Prev").copy();
    if (obj.isInt() && obj.getInt() >= 0) {
        *pos = obj.getInt();
        more = true;
    } else if (obj.isInt64() && obj.getInt64() >= 0) {
        *pos = obj.getInt64();
        more = true;
    } else {
        more = false;
    }
    if (trailerDict.isNone()) {
        trailerDict = xrefStr->getDictObject()->copy();
    }

    ok = true;
    return more;
}

bool XRef::readXRefStreamSection(Stream *xrefStr, const int *w, int first, int n)
{
    unsigned long long offset, gen;
    int type, c, i, j;

    if (first > INT_MAX - n) {
        return false;
    }
    if (first + n < 0) {
        return false;
    }
    if (first + n > size) {
        if (resize(first + n) != size) {
            error(errSyntaxError, -1, "Invalid 'size' inside xref table");
            return false;
        }
        if (first + n > size) {
            error(errSyntaxError, -1, "Invalid 'first' or 'n' inside xref table");
            return false;
        }
    }
    for (i = first; i < first + n; ++i) {
        if (w[0] == 0) {
            type = 1;
        } else {
            for (type = 0, j = 0; j < w[0]; ++j) {
                if ((c = xrefStr->getChar()) == EOF) {
                    return false;
                }
                type = (type << 8) + c;
            }
        }
        for (offset = 0, j = 0; j < w[1]; ++j) {
            if ((c = xrefStr->getChar()) == EOF) {
                return false;
            }
            offset = (offset << 8) + c;
        }
        if (offset > (unsigned long long)GoffsetMax()) {
            error(errSyntaxError, -1, "Offset inside xref table too large for fseek");
            return false;
        }
        for (gen = 0, j = 0; j < w[2]; ++j) {
            if ((c = xrefStr->getChar()) == EOF) {
                return false;
            }
            gen = (gen << 8) + c;
        }
        if (gen > INT_MAX) {
            if (i == 0 && gen == std::numeric_limits<uint32_t>::max()) {
                // workaround broken generators
                gen = 65535;
            } else {
                error(errSyntaxError, -1, "Gen inside xref table too large (bigger than INT_MAX)");
                return false;
            }
        }
        if (entries[i].offset == -1) {
            switch (type) {
            case 0:
                entries[i].offset = offset;
                entries[i].gen = static_cast<int>(gen);
                entries[i].type = xrefEntryFree;
                break;
            case 1:
                entries[i].offset = offset;
                entries[i].gen = static_cast<int>(gen);
                entries[i].type = xrefEntryUncompressed;
                break;
            case 2:
                entries[i].offset = offset;
                entries[i].gen = static_cast<int>(gen);
                entries[i].type = xrefEntryCompressed;
                break;
            default:
                return false;
            }
        }
    }

    return true;
}

// Attempt to construct an xref table for a damaged file.
// Warning: Reconstruction of files where last XRef section is a stream
//          or where some objects are defined inside an object stream is not yet supported.
//          Existing data in XRef::entries may get corrupted if applied anyway.
bool XRef::constructXRef(bool *wasReconstructed, bool needCatalogDict)
{
    Parser *parser;
    char buf[256];
    Goffset pos;
    int num, gen;
    int streamEndsSize;
    char *p;
    bool gotRoot;
    char *token = nullptr;
    bool oneCycle = true;
    Goffset offset = 0;

    resize(0); // free entries properly
    gfree(entries);
    capacity = 0;
    size = 0;
    entries = nullptr;

    gotRoot = false;
    streamEndsLen = streamEndsSize = 0;

    if (wasReconstructed) {
        *wasReconstructed = true;
    }

    if (xrefReconstructedCb) {
        xrefReconstructedCb();
    }

    str->reset();
    while (true) {
        pos = str->getPos();
        if (!str->getLine(buf, 256)) {
            break;
        }
        p = buf;

        // skip whitespace
        while (*p && Lexer::isSpace(*p & 0xff)) {
            ++p;
        }

        oneCycle = true;
        offset = 0;

        while ((token = strstr(p, "endobj")) || oneCycle) {
            oneCycle = false;

            if (token) {
                oneCycle = true;
                token[0] = '\0';
                offset = token - p;
            }

            // got trailer dictionary
            if (!strncmp(p, "trailer", 7)) {
                parser = new Parser(nullptr, str->makeSubStream(pos + 7, false, 0, Object(objNull)), false);
                Object newTrailerDict = parser->getObj();
                if (newTrailerDict.isDict()) {
                    const Object &obj = newTrailerDict.dictLookupNF("Root");
                    if (obj.isRef() && (!gotRoot || !needCatalogDict) && rootNum != obj.getRefNum()) {
                        rootNum = obj.getRefNum();
                        rootGen = obj.getRefGen();
                        trailerDict = newTrailerDict.copy();
                        gotRoot = true;
                    }
                }
                delete parser;

                // look for object
            } else if (isdigit(*p & 0xff)) {
                num = atoi(p);
                if (num > 0) {
                    do {
                        ++p;
                    } while (*p && isdigit(*p & 0xff));
                    if ((*p & 0xff) == 0 || isspace(*p & 0xff)) {
                        if ((*p & 0xff) == 0) {
                            // new line, continue with next line!
                            str->getLine(buf, 256);
                            p = buf;
                        } else {
                            ++p;
                        }
                        while (*p && isspace(*p & 0xff)) {
                            ++p;
                        }
                        if (isdigit(*p & 0xff)) {
                            gen = atoi(p);
                            do {
                                ++p;
                            } while (*p && isdigit(*p & 0xff));
                            if ((*p & 0xff) == 0 || isspace(*p & 0xff)) {
                                if ((*p & 0xff) == 0) {
                                    // new line, continue with next line!
                                    str->getLine(buf, 256);
                                    p = buf;
                                } else {
                                    ++p;
                                }
                                while (*p && isspace(*p & 0xff)) {
                                    ++p;
                                }
                                if (!strncmp(p, "obj", 3)) {
                                    if (num >= size) {
                                        if (unlikely(num >= INT_MAX - 1 - 255)) {
                                            error(errSyntaxError, -1, "Bad object number");
                                            return false;
                                        }
                                        const int newSize = (num + 1 + 255) & ~255;
                                        if (newSize < 0) {
                                            error(errSyntaxError, -1, "Bad object number");
                                            return false;
                                        }
                                        if (resize(newSize) != newSize) {
                                            error(errSyntaxError, -1, "Invalid 'obj' parameters");
                                            return false;
                                        }
                                    }
                                    if (entries[num].type == xrefEntryFree || gen >= entries[num].gen) {
                                        entries[num].offset = pos - start;
                                        entries[num].gen = gen;
                                        entries[num].type = xrefEntryUncompressed;
                                    }
                                }
                            }
                        }
                    }
                }

            } else {
                char *endstream = strstr(p, "endstream");
                if (endstream) {
                    intptr_t endstreamPos = endstream - p;
                    if ((endstreamPos == 0 || Lexer::isSpace(p[endstreamPos - 1] & 0xff)) // endstream is either at beginning or preceeded by space
                        && (endstreamPos + 9 >= 256 || Lexer::isSpace(p[endstreamPos + 9] & 0xff))) // endstream is either at end or followed by space
                    {
                        if (streamEndsLen == streamEndsSize) {
                            streamEndsSize += 64;
                            if (streamEndsSize >= INT_MAX / (int)sizeof(int)) {
                                error(errSyntaxError, -1, "Invalid 'endstream' parameter.");
                                return false;
                            }
                            streamEnds = (Goffset *)greallocn(streamEnds, streamEndsSize, sizeof(Goffset));
                        }
                        streamEnds[streamEndsLen++] = pos + endstreamPos;
                    }
                }
            }
            if (token) {
                p = token + 6; // strlen( "endobj" ) = 6
                pos += offset + 6; // strlen( "endobj" ) = 6
                while (*p && Lexer::isSpace(*p & 0xff)) {
                    ++p;
                    ++pos;
                }
            }
        }
    }

    if (gotRoot) {
        return true;
    }

    error(errSyntaxError, -1, "Couldn't find trailer dictionary");
    return false;
}

void XRef::setEncryption(int permFlagsA, bool ownerPasswordOkA, const unsigned char *fileKeyA, int keyLengthA, int encVersionA, int encRevisionA, CryptAlgorithm encAlgorithmA)
{
    int i;

    encrypted = true;
    permFlags = permFlagsA;
    ownerPasswordOk = ownerPasswordOkA;
    if (keyLengthA <= 32) {
        keyLength = keyLengthA;
    } else {
        keyLength = 32;
    }
    for (i = 0; i < keyLength; ++i) {
        fileKey[i] = fileKeyA[i];
    }
    encVersion = encVersionA;
    encRevision = encRevisionA;
    encAlgorithm = encAlgorithmA;
}

void XRef::getEncryptionParameters(unsigned char **fileKeyA, CryptAlgorithm *encAlgorithmA, int *keyLengthA)
{
    if (encrypted) {
        *fileKeyA = fileKey;
        *encAlgorithmA = encAlgorithm;
        *keyLengthA = keyLength;
    } else {
        // null encryption parameters
        *fileKeyA = nullptr;
        *encAlgorithmA = cryptRC4;
        *keyLengthA = 0;
    }
}

bool XRef::isRefEncrypted(Ref r)
{
    xrefLocker();

    const XRefEntry *e = getEntry(r.num);
    if (!e->obj.isNull()) { // check for updated object
        return false;
    }

    switch (e->type) {
    case xrefEntryUncompressed: {
        return encrypted && !e->getFlag(XRefEntry::Unencrypted);
    }

    case xrefEntryCompressed: {
        const Goffset objStrNum = e->offset;
        if (unlikely(objStrNum < 0 || objStrNum >= size)) {
            error(errSyntaxError, -1, "XRef::isRefEncrypted - Compressed object offset out of xref bounds");
            return false;
        }
        const Object objStr = fetch(static_cast<int>(e->offset), 0);
        return objStr.getStream()->isEncrypted();
    }

    default: {
    }
    }

    return false;
}

bool XRef::okToPrint(bool ignoreOwnerPW) const
{
    return (!ignoreOwnerPW && ownerPasswordOk) || (permFlags & permPrint);
}

// we can print at high res if we are only doing security handler revision
// 2 (and we are allowed to print at all), or with security handler rev
// 3 and we are allowed to print, and bit 12 is set.
bool XRef::okToPrintHighRes(bool ignoreOwnerPW) const
{
    if (encrypted) {
        if (2 == encRevision) {
            return (okToPrint(ignoreOwnerPW));
        } else if (encRevision >= 3) {
            return (okToPrint(ignoreOwnerPW) && (permFlags & permHighResPrint));
        } else {
            // something weird - unknown security handler version
            return false;
        }
    } else {
        return true;
    }
}

bool XRef::okToChange(bool ignoreOwnerPW) const
{
    return (!ignoreOwnerPW && ownerPasswordOk) || (permFlags & permChange);
}

bool XRef::okToCopy(bool ignoreOwnerPW) const
{
    return (!ignoreOwnerPW && ownerPasswordOk) || (permFlags & permCopy);
}

bool XRef::okToAddNotes(bool ignoreOwnerPW) const
{
    return (!ignoreOwnerPW && ownerPasswordOk) || (permFlags & permNotes);
}

bool XRef::okToFillForm(bool ignoreOwnerPW) const
{
    return (!ignoreOwnerPW && ownerPasswordOk) || (permFlags & permFillForm);
}

bool XRef::okToAccessibility(bool ignoreOwnerPW) const
{
    return (!ignoreOwnerPW && ownerPasswordOk) || (permFlags & permAccessibility);
}

bool XRef::okToAssemble(bool ignoreOwnerPW) const
{
    return (!ignoreOwnerPW && ownerPasswordOk) || (permFlags & permAssemble);
}

Object XRef::getCatalog()
{
    Object catalog = fetch(rootNum, rootGen);
    if (catalog.isDict()) {
        return catalog;
    }
    bool wasReconstructed = false;
    if (constructXRef(&wasReconstructed, true)) {
        catalog = fetch(rootNum, rootGen);
    }
    return catalog;
}

Object XRef::fetch(const Ref ref, int recursion)
{
    return fetch(ref.num, ref.gen, recursion);
}

Object XRef::fetch(int num, int gen, int recursion, Goffset *endPos)
{
    XRefEntry *e;
    Object obj1, obj2, obj3;

    xrefLocker();
    // check for bogus ref - this can happen in corrupted PDF files
    if (num < 0 || num >= size) {
        goto err;
    }

    e = getEntry(num);
    if (!e->obj.isNull()) { // check for updated object
        return e->obj.copy();
    }

    switch (e->type) {

    case xrefEntryUncompressed: {
        if (e->gen != gen || e->offset < 0) {
            goto err;
        }
        Parser parser { this, str->makeSubStream(start + e->offset, false, 0, Object(objNull)), true };
        obj1 = parser.getObj(recursion);
        obj2 = parser.getObj(recursion);
        obj3 = parser.getObj(recursion);
        if (!obj1.isInt() || obj1.getInt() != num || !obj2.isInt() || obj2.getInt() != gen || !obj3.isCmd("obj")) {
            // some buggy pdf have obj1234 for ints that represent 1234
            // try to recover here
            if (obj1.isInt() && obj1.getInt() == num && obj2.isInt() && obj2.getInt() == gen && obj3.isCmd()) {
                const char *cmd = obj3.getCmd();
                if (strlen(cmd) > 3 && cmd[0] == 'o' && cmd[1] == 'b' && cmd[2] == 'j') {
                    char *end_ptr;
                    long longNumber = strtol(cmd + 3, &end_ptr, 0);
                    if (longNumber <= INT_MAX && longNumber >= INT_MIN && *end_ptr == '\0') {
                        int number = longNumber;
                        error(errSyntaxWarning, -1, "Cmd was not obj but {0:s}, assuming the creator meant obj {1:d}", cmd, number);
                        if (endPos) {
                            *endPos = parser.getPos();
                        }
                        return Object(number);
                    }
                }
            }
            goto err;
        }
        Object obj = parser.getObj(false, (encrypted && !e->getFlag(XRefEntry::Unencrypted)) ? fileKey : nullptr, encAlgorithm, keyLength, num, gen, recursion);
        if (endPos) {
            *endPos = parser.getPos();
        }
        return obj;
    }

    case xrefEntryCompressed: {
#if 0 // Adobe apparently ignores the generation number on compressed objects
    if (gen != 0) {
      goto err;
    }
#endif
        if (e->offset >= (unsigned int)size || (entries[e->offset].type != xrefEntryUncompressed && entries[e->offset].type != xrefEntryNone)) {
            error(errSyntaxError, -1, "Invalid object stream");
            goto err;
        }

        ObjectStream *objStr = objStrs.lookup(e->offset);
        if (!objStr) {
            objStr = new ObjectStream(this, static_cast<int>(e->offset), recursion + 1);
            if (!objStr->isOk()) {
                delete objStr;
                objStr = nullptr;
                goto err;
            } else {
                // XRef could be reconstructed in constructor of ObjectStream:
                e = getEntry(num);
                objStrs.put(e->offset, objStr);
            }
        }
        if (endPos) {
            *endPos = -1;
        }
        return objStr->getObject(e->gen, num);
    }

    default:
        goto err;
    }

err:
    if (!xRefStream && !xrefReconstructed) {
        // Check if there has been any updated object, if there has been we can't reconstruct because that would mean losing the changes
        bool xrefHasChanges = false;
        for (int i = 0; i < size; i++) {
            if (entries[i].getFlag(XRefEntry::Updated)) {
                xrefHasChanges = true;
                break;
            }
        }
        if (xrefHasChanges) {
            error(errInternal, -1, "xref num {0:d} not found but needed, document has changes, reconstruct aborted\n", num);
            // pretend we constructed the xref, otherwise we will do this check again and again
            xrefReconstructed = true;
            return Object(objNull);
        }

        error(errInternal, -1, "xref num {0:d} not found but needed, try to reconstruct\n", num);
        rootNum = -1;
        constructXRef(&xrefReconstructed);
        return fetch(num, gen, ++recursion, endPos);
    }
    if (endPos) {
        *endPos = -1;
    }
    return Object(objNull);
}

void XRef::lock()
{
    mutex.lock();
}

void XRef::unlock()
{
    mutex.unlock();
}

Object XRef::getDocInfo()
{
    return trailerDict.dictLookup("Info");
}

// Added for the pdftex project.
Object XRef::getDocInfoNF()
{
    return trailerDict.dictLookupNF("Info").copy();
}

Object XRef::createDocInfoIfNeeded(Ref *ref)
{
    Object obj = trailerDict.getDict()->lookup("Info", ref);
    getDocInfo();

    if (obj.isDict() && *ref != Ref::INVALID()) {
        // Info is valid if it's a dict and to pointed by an indirect reference
        return obj;
    }

    removeDocInfo();

    obj = Object(new Dict(this));
    *ref = addIndirectObject(obj);
    trailerDict.dictSet("Info", Object(*ref));

    return obj;
}

void XRef::removeDocInfo()
{
    Object infoObjRef = getDocInfoNF();
    if (infoObjRef.isNull()) {
        return;
    }

    trailerDict.dictRemove("Info");

    if (likely(infoObjRef.isRef())) {
        removeIndirectObject(infoObjRef.getRef());
    }
}

bool XRef::getStreamEnd(Goffset streamStart, Goffset *streamEnd)
{
    int a, b, m;

    if (streamEndsLen == 0 || streamStart > streamEnds[streamEndsLen - 1]) {
        return false;
    }

    a = -1;
    b = streamEndsLen - 1;
    // invariant: streamEnds[a] < streamStart <= streamEnds[b]
    while (b - a > 1) {
        m = (a + b) / 2;
        if (streamStart <= streamEnds[m]) {
            b = m;
        } else {
            a = m;
        }
    }
    *streamEnd = streamEnds[b];
    return true;
}

int XRef::getNumEntry(Goffset offset)
{
    if (size > 0) {
        int res = 0;
        Goffset resOffset = getEntry(0)->offset;
        XRefEntry *e;
        for (int i = 1; i < size; ++i) {
            e = getEntry(i, false);
            if (e->type != xrefEntryFree && e->offset < offset && e->offset >= resOffset) {
                res = i;
                resOffset = e->offset;
            }
        }
        return res;
    } else {
        return -1;
    }
}

void XRef::add(Ref ref, Goffset offs, bool used)
{
    add(ref.num, ref.gen, offs, used);
}

bool XRef::add(int num, int gen, Goffset offs, bool used)
{
    xrefLocker();
    if (num >= size) {
        if (num >= capacity) {
            entries = (XRefEntry *)greallocn_checkoverflow(entries, num + 1, sizeof(XRefEntry));
            if (unlikely(entries == nullptr)) {
                size = 0;
                capacity = 0;
                return false;
            }

            capacity = num + 1;
        }
        for (int i = size; i < num + 1; ++i) {
            entries[i].offset = -1;
            entries[i].type = xrefEntryFree;
            new (&entries[i].obj) Object(objNull);
            entries[i].flags = 0;
            entries[i].gen = 0;
        }
        size = num + 1;
    }
    XRefEntry *e = getEntry(num);
    e->gen = gen;
    e->obj.setToNull();
    e->flags = 0;
    if (used) {
        e->type = xrefEntryUncompressed;
        e->offset = offs;
    } else {
        e->type = xrefEntryFree;
        e->offset = 0;
    }
    return true;
}

void XRef::setModifiedObject(const Object *o, Ref r)
{
    xrefLocker();
    if (r.num < 0 || r.num >= size) {
        error(errInternal, -1, "XRef::setModifiedObject on unknown ref: {0:d}, {1:d}\n", r.num, r.gen);
        return;
    }
    XRefEntry *e = getEntry(r.num);
    if (unlikely(e->type == xrefEntryFree)) {
        error(errInternal, -1, "XRef::setModifiedObject on ref: {0:d}, {1:d} that is marked as free. This will cause a memory leak\n", r.num, r.gen);
    }
    e->obj = o->copy();
    e->setFlag(XRefEntry::Updated, true);
    setModified();
}

Ref XRef::addIndirectObject(const Object &o)
{
    int entryIndexToUse = -1;
    for (int i = 1; entryIndexToUse == -1 && i < size; ++i) {
        XRefEntry *e = getEntry(i, false /* complainIfMissing */);
        if (e->type == xrefEntryFree && e->gen < 65535) {
            entryIndexToUse = i;
        }
    }

    XRefEntry *e;
    if (entryIndexToUse == -1) {
        entryIndexToUse = size;
        add(entryIndexToUse, 0, 0, false);
        e = getEntry(entryIndexToUse);
    } else {
        // reuse a free entry
        e = getEntry(entryIndexToUse);
        // we don't touch gen number, because it should have been
        // incremented when the object was deleted
    }
    e->type = xrefEntryUncompressed;
    e->obj = o.copy();
    e->setFlag(XRefEntry::Updated, true);
    setModified();

    Ref r;
    r.num = entryIndexToUse;
    r.gen = e->gen;
    return r;
}

void XRef::removeIndirectObject(Ref r)
{
    xrefLocker();
    if (r.num < 0 || r.num >= size) {
        error(errInternal, -1, "XRef::removeIndirectObject on unknown ref: {0:d}, {1:d}\n", r.num, r.gen);
        return;
    }
    XRefEntry *e = getEntry(r.num);
    if (e->type == xrefEntryFree) {
        return;
    }
    e->obj.~Object();
    e->type = xrefEntryFree;
    if (likely(e->gen < 65535)) {
        e->gen++;
    }
    e->setFlag(XRefEntry::Updated, true);
    setModified();
}

Ref XRef::addStreamObject(Dict *dict, char *buffer, const Goffset bufferSize, StreamCompression compression)
{
    dict->add("Length", Object((int)bufferSize));
    AutoFreeMemStream *stream = new AutoFreeMemStream(buffer, 0, bufferSize, Object(dict));
    stream->setFilterRemovalForbidden(true);
    switch (compression) {
    case StreamCompression::None:;
        break;
    case StreamCompression::Compress:
        stream->getDict()->add("Filter", Object(objName, "FlateDecode"));
        break;
    }
    return addIndirectObject(Object((Stream *)stream));
}

Ref XRef::addStreamObject(Dict *dict, uint8_t *buffer, const Goffset bufferSize, StreamCompression compression)
{
    return addStreamObject(dict, (char *)buffer, bufferSize, compression);
}

void XRef::writeXRef(XRef::XRefWriter *writer, bool writeAllEntries)
{
    // create free entries linked-list
    if (getEntry(0)->gen != 65535) {
        error(errInternal, -1, "XRef::writeXRef, entry 0 of the XRef is invalid (gen != 65535)\n");
    }
    int lastFreeEntry = 0;
    for (int i = 0; i < size; i++) {
        if (getEntry(i)->type == xrefEntryFree) {
            getEntry(lastFreeEntry)->offset = i;
            lastFreeEntry = i;
        }
    }
    getEntry(lastFreeEntry)->offset = 0;

    if (writeAllEntries) {
        writer->startSection(0, size);
        for (int i = 0; i < size; i++) {
            XRefEntry *e = getEntry(i);
            if (e->gen > 65535) {
                e->gen = 65535; // cap generation number to 65535 (required by PDFReference)
            }
            writer->writeEntry(e->offset, e->gen, e->type);
        }
    } else {
        int i = 0;
        while (i < size) {
            int j;
            for (j = i; j < size; j++) { // look for consecutive entries
                if ((getEntry(j)->type == xrefEntryFree) && (getEntry(j)->gen == 0)) {
                    break;
                }
            }
            if (j - i != 0) {
                writer->startSection(i, j - i);
                for (int k = i; k < j; k++) {
                    XRefEntry *e = getEntry(k);
                    if (e->gen > 65535) {
                        e->gen = 65535; // cap generation number to 65535 (required by PDFReference)
                    }
                    writer->writeEntry(e->offset, e->gen, e->type);
                }
                i = j;
            } else {
                ++i;
            }
        }
    }
}

XRef::XRefTableWriter::XRefTableWriter(OutStream *outStrA)
{
    outStr = outStrA;
}

void XRef::XRefTableWriter::startSection(int first, int count)
{
    outStr->printf("%i %i\r\n", first, count);
}

void XRef::XRefTableWriter::writeEntry(Goffset offset, int gen, XRefEntryType type)
{
    outStr->printf("%010lli %05i %c\r\n", (long long)offset, gen, (type == xrefEntryFree) ? 'f' : 'n');
}

void XRef::writeTableToFile(OutStream *outStr, bool writeAllEntries)
{
    XRefTableWriter writer(outStr);
    outStr->printf("xref\r\n");
    writeXRef(&writer, writeAllEntries);
}

XRef::XRefStreamWriter::XRefStreamWriter(Array *indexA, GooString *stmBufA, int offsetSizeA)
{
    index = indexA;
    stmBuf = stmBufA;
    offsetSize = offsetSizeA;
}

void XRef::XRefStreamWriter::startSection(int first, int count)
{
    index->add(Object(first));
    index->add(Object(count));
}

void XRef::XRefStreamWriter::writeEntry(Goffset offset, int gen, XRefEntryType type)
{
    const int entryTotalSize = 1 + offsetSize + 2; /* type + offset + gen */
    char data[16];
    data[0] = (type == xrefEntryFree) ? 0 : 1;
    for (int i = offsetSize; i > 0; i--) {
        data[i] = offset & 0xff;
        offset >>= 8;
    }
    data[offsetSize + 1] = (gen >> 8) & 0xff;
    data[offsetSize + 2] = gen & 0xff;
    stmBuf->append(data, entryTotalSize);
}

XRef::XRefPreScanWriter::XRefPreScanWriter()
{
    hasOffsetsBeyond4GB = false;
}

void XRef::XRefPreScanWriter::startSection(int first, int count) { }

void XRef::XRefPreScanWriter::writeEntry(Goffset offset, int gen, XRefEntryType type)
{
    if (offset >= 0x100000000ll) {
        hasOffsetsBeyond4GB = true;
    }
}

void XRef::writeStreamToBuffer(GooString *stmBuf, Dict *xrefDict, XRef *xref)
{
    Array *index = new Array(xref);
    stmBuf->clear();

    // First pass: determine whether all offsets fit in 4 bytes or not
    XRefPreScanWriter prescan;
    writeXRef(&prescan, false);
    const int offsetSize = prescan.hasOffsetsBeyond4GB ? sizeof(Goffset) : 4;

    // Second pass: actually write the xref stream
    XRefStreamWriter writer(index, stmBuf, offsetSize);
    writeXRef(&writer, false);

    xrefDict->set("Type", Object(objName, "XRef"));
    xrefDict->set("Index", Object(index));
    Array *wArray = new Array(xref);
    wArray->add(Object(1));
    wArray->add(Object(offsetSize));
    wArray->add(Object(2));
    xrefDict->set("W", Object(wArray));
}

bool XRef::parseEntry(Goffset offset, XRefEntry *entry)
{
    bool r;

    if (unlikely(entry == nullptr)) {
        return false;
    }

    Parser parser(nullptr, str->makeSubStream(offset, false, 20, Object(objNull)), true);

    Object obj1, obj2, obj3;
    if (((obj1 = parser.getObj(), obj1.isInt()) || obj1.isInt64()) && (obj2 = parser.getObj(), obj2.isInt()) && (obj3 = parser.getObj(), obj3.isCmd("n") || obj3.isCmd("f"))) {
        if (obj1.isInt64()) {
            entry->offset = obj1.getInt64();
        } else {
            entry->offset = obj1.getInt();
        }
        entry->gen = obj2.getInt();
        entry->type = obj3.isCmd("n") ? xrefEntryUncompressed : xrefEntryFree;
        entry->obj.setToNull();
        entry->flags = 0;
        r = true;
    } else {
        r = false;
    }

    return r;
}

/* Traverse all XRef tables and, if untilEntryNum != -1, stop as soon as
 * untilEntryNum is found, or try to reconstruct the xref table if it's not
 * present in any xref.
 * If xrefStreamObjsNum is not NULL, it is filled with the list of the object
 * numbers of the XRef streams that have been traversed */
void XRef::readXRefUntil(int untilEntryNum, std::vector<int> *xrefStreamObjsNum)
{
    std::vector<Goffset> followedPrev;
    while (prevXRefOffset && (untilEntryNum == -1 || (untilEntryNum < size && entries[untilEntryNum].type == xrefEntryNone))) {
        bool followed = false;
        for (long long j : followedPrev) {
            if (j == prevXRefOffset) {
                followed = true;
                break;
            }
        }
        if (followed) {
            error(errSyntaxError, -1, "Circular XRef");
            if (!xRefStream && !(ok = constructXRef(nullptr))) {
                errCode = errDamaged;
            }
            break;
        }

        followedPrev.push_back(prevXRefOffset);

        std::vector<Goffset> followedXRefStm;
        if (!readXRef(&prevXRefOffset, &followedXRefStm, xrefStreamObjsNum)) {
            prevXRefOffset = 0;
        }

        // if there was a problem with the xref table, or we haven't found the entry
        // we were looking for, try to reconstruct the xref
        if (!ok || (!prevXRefOffset && untilEntryNum != -1 && entries[untilEntryNum].type == xrefEntryNone)) {
            if (!xRefStream && !(ok = constructXRef(nullptr))) {
                errCode = errDamaged;
                break;
            }
            break;
        }
    }
}

namespace {

struct DummyXRefEntry : XRefEntry
{
    DummyXRefEntry()
    {
        offset = -1;
        gen = 0;
        type = xrefEntryNone;
        flags = 0;
        obj = Object(objNull);
    }
};

DummyXRefEntry dummyXRefEntry;

}

XRefEntry *XRef::getEntry(int i, bool complainIfMissing)
{
    if (unlikely(i < 0)) {
        error(errInternal, -1, "Request for invalid XRef entry [{0:d}]", i);
        return &dummyXRefEntry;
    }

    if (i >= size || entries[i].type == xrefEntryNone) {

        if ((!xRefStream) && mainXRefEntriesOffset) {
            if (unlikely(i >= capacity)) {
                error(errInternal, -1, "Request for out-of-bounds XRef entry [{0:d}]", i);
                return &dummyXRefEntry;
            }

            if (!parseEntry(mainXRefEntriesOffset + 20 * i, &entries[i])) {
                error(errSyntaxError, -1, "Failed to parse XRef entry [{0:d}].", i);
                return &dummyXRefEntry;
            }
        } else {
            // Read XRef tables until the entry we're looking for is found
            readXRefUntil(i);

            // We might have reconstructed the xref
            // Check again i is in bounds
            if (unlikely(i >= size)) {
                return &dummyXRefEntry;
            }

            if (entries[i].type == xrefEntryNone) {
                if (complainIfMissing) {
                    error(errSyntaxError, -1, "Invalid XRef entry {0:d}", i);
                }
                entries[i].type = xrefEntryFree;
            }
        }
    }

    return &entries[i];
}

// Recursively sets the Unencrypted flag in all referenced xref entries
void XRef::markUnencrypted(Object *obj)
{
    Object obj1;

    switch (obj->getType()) {
    case objArray: {
        Array *array = obj->getArray();
        for (int i = 0; i < array->getLength(); i++) {
            obj1 = array->getNF(i).copy();
            markUnencrypted(&obj1);
        }
        break;
    }
    case objStream:
    case objDict: {
        Dict *dict;
        if (obj->getType() == objStream) {
            Stream *stream = obj->getStream();
            dict = stream->getDict();
        } else {
            dict = obj->getDict();
        }
        for (int i = 0; i < dict->getLength(); i++) {
            obj1 = dict->getValNF(i).copy();
            markUnencrypted(&obj1);
        }
        break;
    }
    case objRef: {
        const Ref ref = obj->getRef();
        XRefEntry *e = getEntry(ref.num);
        if (e->getFlag(XRefEntry::Unencrypted)) {
            return; // We've already been here: prevent infinite recursion
        }
        e->setFlag(XRefEntry::Unencrypted, true);
        obj1 = fetch(ref);
        markUnencrypted(&obj1);
        break;
    }
    default:
        break;
    }
}

void XRef::scanSpecialFlags()
{
    if (scannedSpecialFlags) {
        return;
    }
    scannedSpecialFlags = true;

    // "Rewind" the XRef linked list, so that readXRefUntil re-reads all XRef
    // tables/streams, even those that had already been parsed
    prevXRefOffset = mainXRefOffset;

    std::vector<int> xrefStreamObjNums;
    if (!streamEndsLen) { // don't do it for already reconstructed xref
        readXRefUntil(-1 /* read all xref sections */, &xrefStreamObjNums);
    }

    // Mark object streams as DontRewrite, because we write each object
    // individually in full rewrite mode.
    for (int i = 0; i < size; ++i) {
        if (entries[i].type == xrefEntryCompressed) {
            const Goffset objStmNum = entries[i].offset;
            if (unlikely(objStmNum < 0 || objStmNum >= size)) {
                error(errSyntaxError, -1, "Compressed object offset out of xref bounds");
            } else {
                getEntry(static_cast<int>(objStmNum))->setFlag(XRefEntry::DontRewrite, true);
            }
        }
    }

    // Mark XRef streams objects as Unencrypted and DontRewrite
    for (const int objNum : xrefStreamObjNums) {
        getEntry(objNum)->setFlag(XRefEntry::Unencrypted, true);
        getEntry(objNum)->setFlag(XRefEntry::DontRewrite, true);
    }

    // Mark objects referred from the Encrypt dict as Unencrypted
    markUnencrypted();
}

void XRef::markUnencrypted()
{
    // Mark objects referred from the Encrypt dict as Unencrypted
    const Object &obj = trailerDict.dictLookupNF("Encrypt");
    if (obj.isRef()) {
        XRefEntry *e = getEntry(obj.getRefNum());
        e->setFlag(XRefEntry::Unencrypted, true);
    }
}

XRef::XRefWriter::~XRefWriter() = default;
