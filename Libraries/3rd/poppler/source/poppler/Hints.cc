//========================================================================
//
// Hints.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010, 2012, 2013 Hib Eris <hib@hiberis.nl>
// Copyright 2010, 2011, 2013, 2014, 2016-2019, 2021, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright 2010, 2013 Pino Toscano <pino@kde.org>
// Copyright 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright 2014 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright 2016 Jeffrey Morlan <jmmorlan@sonic.net>
// Copyright 2019 LE GARREC Vincent <legarrec.vincent@gmail.com>
// Copyright 2019 Adam Reichold <adam.reichold@t-online.de>
//
//========================================================================

#include <config.h>

#include "Hints.h"

#include "Linearization.h"
#include "Object.h"
#include "Stream.h"
#include "XRef.h"
#include "Parser.h"
#include "Lexer.h"
#include "SecurityHandler.h"

#include <climits>

class StreamBitReader
{
public:
    explicit StreamBitReader(Stream *strA) : str(strA), inputBits(0), isAtEof(false) { }

    void resetInputBits() { inputBits = 0; }

    bool atEOF() const { return isAtEof; }

    unsigned int readBit()
    {
        unsigned int bit;
        int c;

        if (inputBits == 0) {
            if ((c = str->getChar()) == EOF) {
                isAtEof = true;
                return (unsigned int)-1;
            }
            bitsBuffer = c;
            inputBits = 8;
        }
        bit = (bitsBuffer >> (inputBits - 1)) & 1;
        --inputBits;
        return bit;
    }

    unsigned int readBits(int n)
    {
        unsigned int bit, bits;

        if (n < 0) {
            return -1;
        }
        if (n == 0) {
            return 0;
        }

        if (n == 1) {
            return readBit();
        }

        bit = readBit();
        if (bit == (unsigned int)-1) {
            return -1;
        }

        bit = bit << (n - 1);

        bits = readBits(n - 1);
        if (bits == (unsigned int)-1) {
            return -1;
        }

        return bit | bits;
    }

private:
    Stream *str;
    int inputBits;
    char bitsBuffer;
    bool isAtEof;
};

//------------------------------------------------------------------------
// Hints
//------------------------------------------------------------------------

Hints::Hints(BaseStream *str, Linearization *linearization, XRef *xref, SecurityHandler *secHdlr)
{
    mainXRefEntriesOffset = linearization->getMainXRefEntriesOffset();
    nPages = linearization->getNumPages();
    pageFirst = linearization->getPageFirst();
    pageEndFirst = linearization->getEndFirst();
    pageObjectFirst = linearization->getObjectNumberFirst();
    if (pageObjectFirst < 0 || pageObjectFirst >= xref->getNumObjects()) {
        error(errSyntaxWarning, -1, "Invalid reference for first page object ({0:d}) in linearization table ", pageObjectFirst);
        pageObjectFirst = 0;
    }
    XRefEntry *pageObjectFirstXRefEntry = xref->getEntry(pageObjectFirst);
    if (!pageObjectFirstXRefEntry) {
        error(errSyntaxWarning, -1, "No XRef entry for first page object");
        pageOffsetFirst = 0;
    } else {
        pageOffsetFirst = pageObjectFirstXRefEntry->offset;
    }

    if (nPages >= INT_MAX / (int)sizeof(unsigned int)) {
        error(errSyntaxWarning, -1, "Invalid number of pages ({0:d}) for hints table", nPages);
        nPages = 0;
    }
    nObjects = (unsigned int *)gmallocn_checkoverflow(nPages, sizeof(unsigned int));
    pageObjectNum = (int *)gmallocn_checkoverflow(nPages, sizeof(int));
    xRefOffset = (unsigned int *)gmallocn_checkoverflow(nPages, sizeof(unsigned int));
    pageLength = (unsigned int *)gmallocn_checkoverflow(nPages, sizeof(unsigned int));
    pageOffset = (Goffset *)gmallocn_checkoverflow(nPages, sizeof(Goffset));
    numSharedObject = (unsigned int *)gmallocn_checkoverflow(nPages, sizeof(unsigned int));
    sharedObjectId = (unsigned int **)gmallocn_checkoverflow(nPages, sizeof(unsigned int *));
    if (!nObjects || !pageObjectNum || !xRefOffset || !pageLength || !pageOffset || !numSharedObject || !sharedObjectId) {
        error(errSyntaxWarning, -1, "Failed to allocate memory for hints table");
        nPages = 0;
    }

    if (nPages != 0) {
        memset(pageLength, 0, nPages * sizeof(unsigned int));
        memset(pageOffset, 0, nPages * sizeof(unsigned int));
        memset(numSharedObject, 0, nPages * sizeof(unsigned int));
        memset(pageObjectNum, 0, nPages * sizeof(int));
    }

    groupLength = nullptr;
    groupOffset = nullptr;
    groupHasSignature = nullptr;
    groupNumObjects = nullptr;
    groupXRefOffset = nullptr;

    ok = true;
    readTables(str, linearization, xref, secHdlr);
}

Hints::~Hints()
{
    gfree(nObjects);
    gfree(pageObjectNum);
    gfree(xRefOffset);
    gfree(pageLength);
    gfree(pageOffset);
    for (int i = 0; i < nPages; i++) {
        if (numSharedObject[i]) {
            gfree(sharedObjectId[i]);
        }
    }
    gfree(sharedObjectId);
    gfree(numSharedObject);

    gfree(groupLength);
    gfree(groupOffset);
    gfree(groupHasSignature);
    gfree(groupNumObjects);
    gfree(groupXRefOffset);
}

void Hints::readTables(BaseStream *str, Linearization *linearization, XRef *xref, SecurityHandler *secHdlr)
{
    hintsOffset = linearization->getHintsOffset();
    hintsLength = linearization->getHintsLength();
    hintsOffset2 = linearization->getHintsOffset2();
    hintsLength2 = linearization->getHintsLength2();

    const unsigned int bufLength = hintsLength + hintsLength2;

    if (bufLength == 0) {
        ok = false;
        return;
    }

    std::vector<char> buf(bufLength);
    char *p = &buf[0];

    if (hintsOffset && hintsLength) {
        std::unique_ptr<Stream> s(str->makeSubStream(hintsOffset, false, hintsLength, Object(objNull)));
        s->reset();
        for (unsigned int i = 0; i < hintsLength; i++) {
            const int c = s->getChar();
            if (unlikely(c == EOF)) {
                error(errSyntaxWarning, -1, "Found EOF while reading hints");
                ok = false;
                return;
            }
            *p++ = c;
        }
    }

    if (hintsOffset2 && hintsLength2) {
        std::unique_ptr<Stream> s(str->makeSubStream(hintsOffset2, false, hintsLength2, Object(objNull)));
        s->reset();
        for (unsigned int i = 0; i < hintsLength2; i++) {
            const int c = s->getChar();
            if (unlikely(c == EOF)) {
                error(errSyntaxWarning, -1, "Found EOF while reading hints2");
                ok = false;
                return;
            }
            *p++ = c;
        }
    }

    MemStream *memStream = new MemStream(&buf[0], 0, bufLength, Object(objNull));

    Parser *parser = new Parser(xref, memStream, true);

    int num, gen;
    Object obj;
    if ((obj = parser->getObj(), obj.isInt()) && (num = obj.getInt(), obj = parser->getObj(), obj.isInt()) && (gen = obj.getInt(), obj = parser->getObj(), obj.isCmd("obj"))
        && (obj = parser->getObj(false, secHdlr ? secHdlr->getFileKey() : nullptr, secHdlr ? secHdlr->getEncAlgorithm() : cryptRC4, secHdlr ? secHdlr->getFileKeyLength() : 0, num, gen, 0, true), obj.isStream())) {
        Stream *hintsStream = obj.getStream();
        Dict *hintsDict = obj.streamGetDict();

        int sharedStreamOffset = 0;
        if (hintsDict->lookupInt("S", nullptr, &sharedStreamOffset) && sharedStreamOffset > 0) {

            hintsStream->reset();
            ok = readPageOffsetTable(hintsStream);

            if (ok) {
                hintsStream->reset();
                for (int i = 0; i < sharedStreamOffset; i++) {
                    hintsStream->getChar();
                }
                ok = readSharedObjectsTable(hintsStream);
            }
        } else {
            error(errSyntaxWarning, -1, "Invalid shared object hint table offset");
            ok = false;
        }
    } else {
        error(errSyntaxWarning, -1, "Failed parsing hints table object");
        ok = false;
    }

    delete parser;
}

bool Hints::readPageOffsetTable(Stream *str)
{
    if (nPages < 1) {
        error(errSyntaxWarning, -1, "Invalid number of pages reading page offset hints table");
        return false;
    }

    StreamBitReader sbr(str);

    nObjectLeast = sbr.readBits(32);
    if (nObjectLeast < 1) {
        error(errSyntaxWarning, -1, "Invalid least number of objects reading page offset hints table");
        nPages = 0;
        return false;
    }

    objectOffsetFirst = sbr.readBits(32);
    if (objectOffsetFirst >= hintsOffset) {
        objectOffsetFirst += hintsLength;
    }

    nBitsDiffObjects = sbr.readBits(16);
    if (nBitsDiffObjects > 32) {
        error(errSyntaxWarning, -1, "Invalid number of bits needed to represent the difference between the greatest and least number of objects in a page");
        nPages = 0;
        return false;
    }

    pageLengthLeast = sbr.readBits(32);

    nBitsDiffPageLength = sbr.readBits(16);

    OffsetStreamLeast = sbr.readBits(32);

    nBitsOffsetStream = sbr.readBits(16);

    lengthStreamLeast = sbr.readBits(32);

    nBitsLengthStream = sbr.readBits(16);

    nBitsNumShared = sbr.readBits(16);

    nBitsShared = sbr.readBits(16);

    nBitsNumerator = sbr.readBits(16);

    denominator = sbr.readBits(16);

    if ((nBitsDiffPageLength > 32) || (nBitsOffsetStream > 32) || (nBitsLengthStream > 32) || (nBitsNumShared > 32) || (nBitsShared > 32) || (nBitsNumerator > 32)) {
        error(errSyntaxWarning, -1, "Invalid number of bits reading page offset hints table");
        return false;
    }

    for (int i = 0; i < nPages && !sbr.atEOF(); i++) {
        nObjects[i] = nObjectLeast + sbr.readBits(nBitsDiffObjects);
    }
    if (sbr.atEOF()) {
        return false;
    }

    nObjects[0] = 0;
    xRefOffset[0] = mainXRefEntriesOffset + 20;
    for (int i = 1; i < nPages; i++) {
        xRefOffset[i] = xRefOffset[i - 1] + 20 * nObjects[i - 1];
    }

    pageObjectNum[0] = 1;
    for (int i = 1; i < nPages; i++) {
        pageObjectNum[i] = pageObjectNum[i - 1] + nObjects[i - 1];
    }
    pageObjectNum[0] = pageObjectFirst;

    sbr.resetInputBits(); // reset on byte boundary. Not in specs!
    for (int i = 0; i < nPages && !sbr.atEOF(); i++) {
        pageLength[i] = pageLengthLeast + sbr.readBits(nBitsDiffPageLength);
    }
    if (sbr.atEOF()) {
        return false;
    }

    sbr.resetInputBits(); // reset on byte boundary. Not in specs!
    numSharedObject[0] = sbr.readBits(nBitsNumShared);
    numSharedObject[0] = 0; // Do not trust the read value to be 0.
    sharedObjectId[0] = nullptr;
    for (int i = 1; i < nPages && !sbr.atEOF(); i++) {
        numSharedObject[i] = sbr.readBits(nBitsNumShared);
        if (numSharedObject[i] >= INT_MAX / (int)sizeof(unsigned int)) {
            error(errSyntaxWarning, -1, "Invalid number of shared objects");
            numSharedObject[i] = 0;
            return false;
        }
        sharedObjectId[i] = (unsigned int *)gmallocn_checkoverflow(numSharedObject[i], sizeof(unsigned int));
        if (numSharedObject[i] && !sharedObjectId[i]) {
            error(errSyntaxWarning, -1, "Failed to allocate memory for shared object IDs");
            numSharedObject[i] = 0;
            return false;
        }
    }
    if (sbr.atEOF()) {
        return false;
    }

    sbr.resetInputBits(); // reset on byte boundary. Not in specs!
    for (int i = 1; i < nPages; i++) {
        for (unsigned int j = 0; j < numSharedObject[i] && !sbr.atEOF(); j++) {
            sharedObjectId[i][j] = sbr.readBits(nBitsShared);
        }
    }

    pageOffset[0] = pageOffsetFirst;
    // find pageOffsets.
    for (int i = 1; i < nPages; i++) {
        pageOffset[i] = pageOffset[i - 1] + pageLength[i - 1];
    }

    return !sbr.atEOF();
}

bool Hints::readSharedObjectsTable(Stream *str)
{
    StreamBitReader sbr(str);

    const unsigned int firstSharedObjectNumber = sbr.readBits(32);

    const unsigned int firstSharedObjectOffset = sbr.readBits(32) + hintsLength;

    const unsigned int nSharedGroupsFirst = sbr.readBits(32);

    const unsigned int nSharedGroups = sbr.readBits(32);

    const unsigned int nBitsNumObjects = sbr.readBits(16);

    const unsigned int groupLengthLeast = sbr.readBits(32);

    const unsigned int nBitsDiffGroupLength = sbr.readBits(16);

    if ((!nSharedGroups) || (nSharedGroups >= INT_MAX / (int)sizeof(unsigned int))) {
        error(errSyntaxWarning, -1, "Invalid number of shared object groups");
        return false;
    }
    if ((!nSharedGroupsFirst) || (nSharedGroupsFirst > nSharedGroups)) {
        error(errSyntaxWarning, -1, "Invalid number of first page shared object groups");
        return false;
    }
    if (nBitsNumObjects > 32 || nBitsDiffGroupLength > 32) {
        error(errSyntaxWarning, -1, "Invalid shared object groups bit length");
        return false;
    }

    groupLength = (unsigned int *)gmallocn_checkoverflow(nSharedGroups, sizeof(unsigned int));
    groupOffset = (unsigned int *)gmallocn_checkoverflow(nSharedGroups, sizeof(unsigned int));
    groupHasSignature = (unsigned int *)gmallocn_checkoverflow(nSharedGroups, sizeof(unsigned int));
    groupNumObjects = (unsigned int *)gmallocn_checkoverflow(nSharedGroups, sizeof(unsigned int));
    groupXRefOffset = (unsigned int *)gmallocn_checkoverflow(nSharedGroups, sizeof(unsigned int));
    if (!groupLength || !groupOffset || !groupHasSignature || !groupNumObjects || !groupXRefOffset) {
        error(errSyntaxWarning, -1, "Failed to allocate memory for shared object groups");
        return false;
    }

    sbr.resetInputBits(); // reset on byte boundary. Not in specs!
    for (unsigned int i = 0; i < nSharedGroups && !sbr.atEOF(); i++) {
        groupLength[i] = groupLengthLeast + sbr.readBits(nBitsDiffGroupLength);
    }
    if (sbr.atEOF()) {
        return false;
    }

    groupOffset[0] = objectOffsetFirst;
    for (unsigned int i = 1; i < nSharedGroupsFirst; i++) {
        groupOffset[i] = groupOffset[i - 1] + groupLength[i - 1];
    }
    if (nSharedGroups > nSharedGroupsFirst) {
        groupOffset[nSharedGroupsFirst] = firstSharedObjectOffset;
        for (unsigned int i = nSharedGroupsFirst + 1; i < nSharedGroups; i++) {
            groupOffset[i] = groupOffset[i - 1] + groupLength[i - 1];
        }
    }

    sbr.resetInputBits(); // reset on byte boundary. Not in specs!
    for (unsigned int i = 0; i < nSharedGroups && !sbr.atEOF(); i++) {
        groupHasSignature[i] = sbr.readBits(1);
    }
    if (sbr.atEOF()) {
        return false;
    }

    sbr.resetInputBits(); // reset on byte boundary. Not in specs!
    for (unsigned int i = 0; i < nSharedGroups && !sbr.atEOF(); i++) {
        if (groupHasSignature[i]) {
            // readBits doesn't supports more than 32 bits.
            sbr.readBits(32);
            sbr.readBits(32);
            sbr.readBits(32);
            sbr.readBits(32);
        }
    }
    if (sbr.atEOF()) {
        return false;
    }

    sbr.resetInputBits(); // reset on byte boundary. Not in specs!
    for (unsigned int i = 0; i < nSharedGroups && !sbr.atEOF(); i++) {
        groupNumObjects[i] = nBitsNumObjects ? 1 + sbr.readBits(nBitsNumObjects) : 1;
    }

    for (unsigned int i = 0; i < nSharedGroupsFirst; i++) {
        groupNumObjects[i] = 0;
        groupXRefOffset[i] = 0;
    }
    if (nSharedGroups > nSharedGroupsFirst) {
        groupXRefOffset[nSharedGroupsFirst] = mainXRefEntriesOffset + 20 * firstSharedObjectNumber;
        for (unsigned int i = nSharedGroupsFirst + 1; i < nSharedGroups; i++) {
            groupXRefOffset[i] = groupXRefOffset[i - 1] + 20 * groupNumObjects[i - 1];
        }
    }

    return !sbr.atEOF();
}

bool Hints::isOk() const
{
    return ok;
}

Goffset Hints::getPageOffset(int page)
{
    if ((page < 1) || (page > nPages)) {
        return 0;
    }

    if (page - 1 > pageFirst) {
        return pageOffset[page - 1];
    } else if (page - 1 < pageFirst) {
        return pageOffset[page];
    } else {
        return pageOffset[0];
    }
}

int Hints::getPageObjectNum(int page)
{
    if ((page < 1) || (page > nPages)) {
        return 0;
    }

    if (page - 1 > pageFirst) {
        return pageObjectNum[page - 1];
    } else if (page - 1 < pageFirst) {
        return pageObjectNum[page];
    } else {
        return pageObjectNum[0];
    }
}
