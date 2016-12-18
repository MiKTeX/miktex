//========================================================================
//
// Hints.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010, 2012 Hib Eris <hib@hiberis.nl>
// Copyright 2010, 2011, 2013, 2014, 2016 Albert Astals Cid <aacid@kde.org>
// Copyright 2010, 2013 Pino Toscano <pino@kde.org>
// Copyright 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright 2014 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright 2016 Jeffrey Morlan <jmmorlan@sonic.net>
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

#include <limits.h>

class StreamBitReader {
public:
  StreamBitReader(Stream *strA)
    : str(strA)
    , inputBits(0)
    , isAtEof(gFalse)
  {
  }

  void resetInputBits()
  {
    inputBits = 0;
  }

  GBool atEOF() const
  {
    return isAtEof;
  }

  Guint readBit()
  {
    Guint bit;
    int c;

    if (inputBits == 0) {
      if ((c = str->getChar()) == EOF) {
        isAtEof = gTrue;
        return (Guint) -1;
      }
      bitsBuffer = c;
      inputBits = 8;
    }
    bit = (bitsBuffer >> (inputBits - 1)) & 1;
    --inputBits;
    return bit;
  }

  Guint readBits(int n)
  {
    Guint bit, bits;

    if (n < 0) return -1;
    if (n == 0) return 0;

    if (n == 1)
      return readBit();

    bit = readBit();
    if (bit == (Guint) -1)
      return -1;

    bit = bit << (n-1);

    bits = readBits(n - 1);
    if (bits == (Guint) -1)
      return -1;

    return bit | bits;
  }

private:
  Stream *str;
  int inputBits;
  char bitsBuffer;
  GBool isAtEof;
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
    error(errSyntaxWarning, -1,
      "Invalid reference for first page object ({0:d}) in linearization table ",
      pageObjectFirst);
    pageObjectFirst = 0;
  }
  XRefEntry *pageObjectFirstXRefEntry = xref->getEntry(pageObjectFirst);
  if (!pageObjectFirstXRefEntry) {
      error(errSyntaxWarning, -1, "No XRef entry for first page object");
      pageOffsetFirst = 0;
  } else {
      pageOffsetFirst = pageObjectFirstXRefEntry->offset;
  }

  if (nPages >= INT_MAX / (int)sizeof(Guint)) {
     error(errSyntaxWarning, -1, "Invalid number of pages ({0:d}) for hints table", nPages);
     nPages = 0;
  }
  nObjects = (Guint *) gmallocn_checkoverflow(nPages, sizeof(Guint));
  pageObjectNum = (int *) gmallocn_checkoverflow(nPages, sizeof(int));
  xRefOffset = (Guint *) gmallocn_checkoverflow(nPages, sizeof(Guint));
  pageLength = (Guint *) gmallocn_checkoverflow(nPages, sizeof(Guint));
  pageOffset = (Goffset *) gmallocn_checkoverflow(nPages, sizeof(Goffset));
  numSharedObject = (Guint *) gmallocn_checkoverflow(nPages, sizeof(Guint));
  sharedObjectId = (Guint **) gmallocn_checkoverflow(nPages, sizeof(Guint*));
  if (!nObjects || !pageObjectNum || !xRefOffset || !pageLength || !pageOffset ||
      !numSharedObject || !sharedObjectId) {
    error(errSyntaxWarning, -1, "Failed to allocate memory for hints table");
    nPages = 0;
  }

  memset(pageLength, 0, nPages * sizeof(Guint));
  memset(pageOffset, 0, nPages * sizeof(Guint));
  memset(numSharedObject, 0, nPages * sizeof(Guint));
  memset(pageObjectNum, 0, nPages * sizeof(int));

  groupLength = NULL;
  groupOffset = NULL;
  groupHasSignature = NULL;
  groupNumObjects = NULL;
  groupXRefOffset = NULL;

  ok = gTrue;
  readTables(str, linearization, xref, secHdlr);
}

Hints::~Hints()
{
  gfree(nObjects);
  gfree(pageObjectNum);
  gfree(xRefOffset);
  gfree(pageLength);
  gfree(pageOffset);
  for (int i=0; i< nPages; i++) {
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

  Parser *parser;
  Object obj;

  int bufLength = hintsLength + hintsLength2;

  std::vector<char> buf(bufLength);
  char *p = &buf[0];

  obj.initNull();
  Stream *s = str->makeSubStream(hintsOffset, gFalse, hintsLength, &obj);
  s->reset();
  for (Guint i=0; i < hintsLength; i++) { *p++ = s->getChar(); }
  delete s;

  if (hintsOffset2 && hintsLength2) {
    obj.initNull();
    s = str->makeSubStream(hintsOffset2, gFalse, hintsLength2, &obj);
    s->reset();
    for (Guint i=0; i < hintsLength2; i++) { *p++ = s->getChar(); }
    delete s;
  }

  obj.initNull();
  MemStream *memStream = new MemStream (&buf[0], 0, bufLength, &obj);

  obj.initNull();
  parser = new Parser(xref, new Lexer(xref, memStream), gTrue);

  int num, gen;
  if (parser->getObj(&obj)->isInt() &&
     (num = obj.getInt(), obj.free(), parser->getObj(&obj)->isInt()) &&
     (gen = obj.getInt(), obj.free(), parser->getObj(&obj)->isCmd("obj")) &&
     (obj.free(), parser->getObj(&obj, gFalse,
         secHdlr ? secHdlr->getFileKey() : (Guchar *)NULL,
         secHdlr ? secHdlr->getEncAlgorithm() : cryptRC4,
         secHdlr ? secHdlr->getFileKeyLength() : 0,
         num, gen, 0, gTrue)->isStream())) {
    Stream *hintsStream = obj.getStream();
    Dict *hintsDict = obj.streamGetDict();

    int sharedStreamOffset = 0;
    if (hintsDict->lookupInt("S", NULL, &sharedStreamOffset) &&
        sharedStreamOffset > 0) {

        hintsStream->reset();
        ok = readPageOffsetTable(hintsStream);

        if (ok) {
          hintsStream->reset();
          for (int i=0; i<sharedStreamOffset; i++) hintsStream->getChar();
          ok = readSharedObjectsTable(hintsStream);
        }
    } else {
      error(errSyntaxWarning, -1, "Invalid shared object hint table offset");
    }
  } else {
    error(errSyntaxWarning, -1, "Failed parsing hints table object");
  }
  obj.free();

  delete parser;
}

GBool Hints::readPageOffsetTable(Stream *str)
{
  if (nPages < 1) {
    error(errSyntaxWarning, -1, "Invalid number of pages reading page offset hints table");
    return gFalse;
  }

  StreamBitReader sbr(str);

  nObjectLeast = sbr.readBits(32);
  if (nObjectLeast < 1) {
    error(errSyntaxWarning, -1, "Invalid least number of objects reading page offset hints table");
    nPages = 0;
    return gFalse;
  }

  objectOffsetFirst = sbr.readBits(32);
  if (objectOffsetFirst >= hintsOffset) objectOffsetFirst += hintsLength;

  nBitsDiffObjects = sbr.readBits(16);

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

  for (int i = 0; i < nPages && !sbr.atEOF(); i++) {
    nObjects[i] = nObjectLeast + sbr.readBits(nBitsDiffObjects);
  }
  if (sbr.atEOF())
    return gFalse;

  nObjects[0] = 0;
  xRefOffset[0] = mainXRefEntriesOffset + 20;
  for (int i=1; i<nPages; i++) {
    xRefOffset[i] = xRefOffset[i-1] + 20*nObjects[i-1];
  }

  pageObjectNum[0] = 1;
  for (int i=1; i<nPages; i++) {
    pageObjectNum[i] = pageObjectNum[i-1] + nObjects[i-1];
  }
  pageObjectNum[0] = pageObjectFirst;

  sbr.resetInputBits(); // reset on byte boundary. Not in specs!
  for (int i = 0; i < nPages && !sbr.atEOF(); i++) {
    pageLength[i] = pageLengthLeast + sbr.readBits(nBitsDiffPageLength);
  }
  if (sbr.atEOF())
    return gFalse;

  sbr.resetInputBits(); // reset on byte boundary. Not in specs!
  numSharedObject[0] = sbr.readBits(nBitsNumShared);
  numSharedObject[0] = 0; // Do not trust the read value to be 0.
  sharedObjectId[0] = NULL;
  for (int i = 1; i < nPages && !sbr.atEOF(); i++) {
    numSharedObject[i] = sbr.readBits(nBitsNumShared);
    if (numSharedObject[i] >= INT_MAX / (int)sizeof(Guint)) {
       error(errSyntaxWarning, -1, "Invalid number of shared objects");
       numSharedObject[i] = 0;
       return gFalse;
    }
    sharedObjectId[i] = (Guint *) gmallocn_checkoverflow(numSharedObject[i], sizeof(Guint));
    if (numSharedObject[i] && !sharedObjectId[i]) {
       error(errSyntaxWarning, -1, "Failed to allocate memory for shared object IDs");
       numSharedObject[i] = 0;
       return gFalse;
    }
  }
  if (sbr.atEOF())
    return gFalse;

  sbr.resetInputBits(); // reset on byte boundary. Not in specs!
  for (int i=1; i<nPages; i++) {
    for (Guint j = 0; j < numSharedObject[i] && !sbr.atEOF(); j++) {
      sharedObjectId[i][j] = sbr.readBits(nBitsShared);
    }
  }

  pageOffset[0] = pageOffsetFirst;
  // find pageOffsets.
  for (int i=1; i<nPages; i++) {
    pageOffset[i] = pageOffset[i-1] + pageLength[i-1];
  }

  return !sbr.atEOF();
}

GBool Hints::readSharedObjectsTable(Stream *str)
{
  StreamBitReader sbr(str);

  const Guint firstSharedObjectNumber = sbr.readBits(32);

  const Guint firstSharedObjectOffset = sbr.readBits(32) + hintsLength;

  const Guint nSharedGroupsFirst = sbr.readBits(32);

  const Guint nSharedGroups = sbr.readBits(32);

  const Guint nBitsNumObjects = sbr.readBits(16);

  const Guint groupLengthLeast = sbr.readBits(32);

  const Guint nBitsDiffGroupLength = sbr.readBits(16);

  if ((!nSharedGroups) || (nSharedGroups >= INT_MAX / (int)sizeof(Guint))) {
     error(errSyntaxWarning, -1, "Invalid number of shared object groups");
     return gFalse;
  }
  if ((!nSharedGroupsFirst) || (nSharedGroupsFirst > nSharedGroups)) {
     error(errSyntaxWarning, -1, "Invalid number of first page shared object groups");
     return gFalse;
  }
  if (nBitsNumObjects > 32 || nBitsDiffGroupLength > 32) {
     error(errSyntaxWarning, -1, "Invalid shared object groups bit length");
     return gFalse;
  }

  groupLength = (Guint *) gmallocn_checkoverflow(nSharedGroups, sizeof(Guint));
  groupOffset = (Guint *) gmallocn_checkoverflow(nSharedGroups, sizeof(Guint));
  groupHasSignature = (Guint *) gmallocn_checkoverflow(nSharedGroups, sizeof(Guint));
  groupNumObjects = (Guint *) gmallocn_checkoverflow(nSharedGroups, sizeof(Guint));
  groupXRefOffset = (Guint *) gmallocn_checkoverflow(nSharedGroups, sizeof(Guint));
  if (!groupLength || !groupOffset || !groupHasSignature ||
      !groupNumObjects || !groupXRefOffset) {
     error(errSyntaxWarning, -1, "Failed to allocate memory for shared object groups");
     return gFalse;
  }

  sbr.resetInputBits(); // reset on byte boundary. Not in specs!
  for (Guint i = 0; i < nSharedGroups && !sbr.atEOF(); i++) {
    groupLength[i] = groupLengthLeast + sbr.readBits(nBitsDiffGroupLength);
  }
  if (sbr.atEOF())
    return gFalse;

  groupOffset[0] = objectOffsetFirst;
  for (Guint i=1; i<nSharedGroupsFirst; i++) {
    groupOffset[i] = groupOffset[i-1] + groupLength[i-1];
  }
  if (nSharedGroups > nSharedGroupsFirst ) {
    groupOffset[nSharedGroupsFirst] = firstSharedObjectOffset;
    for (Guint i=nSharedGroupsFirst+1; i<nSharedGroups; i++) {
      groupOffset[i] = groupOffset[i-1] + groupLength[i-1];
    }
  }

  sbr.resetInputBits(); // reset on byte boundary. Not in specs!
  for (Guint i = 0; i < nSharedGroups && !sbr.atEOF(); i++) {
    groupHasSignature[i] = sbr.readBits(1);
  }
  if (sbr.atEOF())
    return gFalse;

  sbr.resetInputBits(); // reset on byte boundary. Not in specs!
  for (Guint i = 0; i < nSharedGroups && !sbr.atEOF(); i++) {
    if (groupHasSignature[i]) {
       sbr.readBits(128);
    }
  }
  if (sbr.atEOF())
    return gFalse;

  sbr.resetInputBits(); // reset on byte boundary. Not in specs!
  for (Guint i = 0; i < nSharedGroups && !sbr.atEOF(); i++) {
    groupNumObjects[i] =
       nBitsNumObjects ? 1 + sbr.readBits(nBitsNumObjects) : 1;
  }

  for (Guint i=0; i<nSharedGroupsFirst; i++) {
    groupNumObjects[i] = 0;
    groupXRefOffset[i] = 0;
  }
  if (nSharedGroups > nSharedGroupsFirst ) {
    groupXRefOffset[nSharedGroupsFirst] =
        mainXRefEntriesOffset + 20*firstSharedObjectNumber;
    for (Guint i=nSharedGroupsFirst+1; i<nSharedGroups; i++) {
      groupXRefOffset[i] = groupXRefOffset[i-1] + 20*groupNumObjects[i-1];
    }
  }

  return !sbr.atEOF();
}

GBool Hints::isOk() const
{
  return ok;
}

Goffset Hints::getPageOffset(int page)
{
  if ((page < 1) || (page > nPages)) return 0;

  if (page-1 > pageFirst)
    return pageOffset[page-1];
  else if (page-1 < pageFirst)
    return pageOffset[page];
  else
    return pageOffset[0];
}

std::vector<ByteRange>* Hints::getPageRanges(int page)
{
  if ((page < 1) || (page > nPages)) return NULL;

  int idx;
  if (page-1 > pageFirst)
     idx = page-1;
  else if (page-1 < pageFirst)
     idx = page;
  else
     idx = 0;

  ByteRange pageRange;
  std::vector<ByteRange> *v = new std::vector<ByteRange>;

  pageRange.offset = pageOffset[idx];
  pageRange.length = pageLength[idx];
  v->push_back(pageRange);

  pageRange.offset = xRefOffset[idx];
  pageRange.length = 20*nObjects[idx];
  v->push_back(pageRange);

  for (Guint j=0; j<numSharedObject[idx]; j++) {
     Guint k = sharedObjectId[idx][j];

     pageRange.offset = groupOffset[k];
     pageRange.length = groupLength[k];
     v->push_back(pageRange);

     pageRange.offset = groupXRefOffset[k];
     pageRange.length = 20*groupNumObjects[k];
     v->push_back(pageRange);
  }

  return v;
}

int Hints::getPageObjectNum(int page) {
  if ((page < 1) || (page > nPages)) return 0;

  if (page-1 > pageFirst)
    return pageObjectNum[page-1];
  else if (page-1 < pageFirst)
    return pageObjectNum[page];
  else
    return pageObjectNum[0];
}
