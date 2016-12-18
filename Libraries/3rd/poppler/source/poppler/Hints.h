//========================================================================
//
// Hints.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
// Copyright 2010, 2013, 2016 Albert Astals Cid <aacid@kde.org>
// Copyright 2013 Adrian Johnson <ajohnson@redneon.com>
//
//========================================================================

#ifndef HINTS_H
#define HINTS_H

#include <string.h>
#include "goo/gtypes.h"
#include <vector>
#include "PDFDoc.h"

class Stream;
class BaseStream;
class Linearization;
class XRef;

//------------------------------------------------------------------------
// Hints
//------------------------------------------------------------------------

class Hints {
public:

  Hints(BaseStream *str, Linearization *linearization, XRef *xref, SecurityHandler *secHdlr);
  ~Hints();

  GBool isOk() const;

  int getPageObjectNum(int page);
  Goffset getPageOffset(int page);
  std::vector<ByteRange>* getPageRanges(int page);

private:

  void readTables(BaseStream *str, Linearization *linearization, XRef *xref, SecurityHandler *secHdlr);
  GBool readPageOffsetTable(Stream *str);
  GBool readSharedObjectsTable(Stream *str);

  Guint hintsOffset;
  Guint hintsLength;
  Guint hintsOffset2;
  Guint hintsLength2;
  Guint mainXRefEntriesOffset;

  int nPages;
  int pageFirst;
  int pageObjectFirst;
  Goffset pageOffsetFirst;
  Guint pageEndFirst;

  Guint nObjectLeast;
  Guint objectOffsetFirst;
  Guint nBitsDiffObjects;
  Guint pageLengthLeast;
  Guint nBitsDiffPageLength;
  Guint OffsetStreamLeast;
  Guint nBitsOffsetStream;
  Guint lengthStreamLeast;
  Guint nBitsLengthStream;
  Guint nBitsNumShared;
  Guint nBitsShared;
  Guint nBitsNumerator;
  Guint denominator;

  Guint *nObjects;
  int *pageObjectNum;
  Guint *xRefOffset;
  Guint *pageLength;
  Goffset *pageOffset;
  Guint *numSharedObject;
  Guint **sharedObjectId;

  Guint *groupLength;
  Guint *groupOffset;
  Guint *groupHasSignature;
  Guint *groupNumObjects;
  Guint *groupXRefOffset;
  GBool ok;
};

#endif
