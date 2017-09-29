//========================================================================
//
// Linearization.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010, 2012 Hib Eris <hib@hiberis.nl>
// Copyright 2015 Jason Crain <jason@aquaticape.us>
// Copyright 2017 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#include "Linearization.h"
#include "Parser.h"
#include "Lexer.h"

//------------------------------------------------------------------------
// Linearization
//------------------------------------------------------------------------

Linearization::Linearization (BaseStream *str)
{
  Parser *parser;

  str->reset();
  parser = new Parser(NULL,
      new Lexer(NULL, str->makeSubStream(str->getStart(), gFalse, 0, Object(objNull))),
      gFalse);
  Object obj1 = parser->getObj();
  Object obj2 = parser->getObj();
  Object obj3 = parser->getObj();
  linDict = parser->getObj();
  if (obj1.isInt() && obj2.isInt() && obj3.isCmd("obj") && linDict.isDict()) {
    Object obj5 = linDict.dictLookup("Linearized");
    if (!(obj5.isNum() && obj5.getNum() > 0)) {
       linDict.setToNull();
    }
  }
  delete parser;
}

Linearization:: ~Linearization()
{
}

Guint Linearization::getLength()
{
  if (!linDict.isDict()) return 0;

  int length;
  if (linDict.getDict()->lookupInt("L", NULL, &length) &&
      length > 0) {
    return length;
  } else {
    error(errSyntaxWarning, -1, "Length in linearization table is invalid");
    return 0;
  }
}

Guint Linearization::getHintsOffset()
{
  int hintsOffset;

  Object obj1, obj2;
  if (linDict.isDict() &&
      (obj1 = linDict.dictLookup("H"), obj1.isArray()) &&
      obj1.arrayGetLength()>=2 &&
      (obj2 = obj1.arrayGet(0), obj2.isInt()) &&
      obj2.getInt() > 0) {
    hintsOffset = obj2.getInt();
  } else {
    error(errSyntaxWarning, -1, "Hints table offset in linearization table is invalid");
    hintsOffset = 0;
  }

  return hintsOffset;
}

Guint Linearization::getHintsLength()
{
  int hintsLength;

  Object obj1, obj2;
  if (linDict.isDict() &&
      (obj1 = linDict.dictLookup("H"), obj1.isArray()) &&
      obj1.arrayGetLength()>=2 &&
      (obj2 = obj1.arrayGet(1), obj2.isInt()) &&
      obj2.getInt() > 0) {
    hintsLength = obj2.getInt();
  } else {
    error(errSyntaxWarning, -1, "Hints table length in linearization table is invalid");
    hintsLength = 0;
  }

  return hintsLength;
}

Guint Linearization::getHintsOffset2()
{
  int hintsOffset2 = 0; // default to 0

  Object obj1;
  if (linDict.isDict() &&
      (obj1 = linDict.dictLookup("H"), obj1.isArray()) &&
      obj1.arrayGetLength()>=4) {
    Object obj2 = obj1.arrayGet(2);
    if (obj2.isInt() && obj2.getInt() > 0) {
      hintsOffset2 = obj2.getInt();
    } else {
      error(errSyntaxWarning, -1, "Second hints table offset in linearization table is invalid");
      hintsOffset2 = 0;
    }
  }

  return hintsOffset2;
}

Guint Linearization::getHintsLength2()
{
  int hintsLength2 = 0; // default to 0

  Object obj1;
  if (linDict.isDict() &&
      (obj1 = linDict.dictLookup("H"), obj1.isArray()) &&
      obj1.arrayGetLength()>=4) {
    Object obj2 = obj1.arrayGet(3);
    if (obj2.isInt() && obj2.getInt() > 0) {
      hintsLength2 = obj2.getInt();
    } else {
      error(errSyntaxWarning, -1, "Second hints table length in linearization table is invalid");
      hintsLength2 = 0;
    }
  }

  return hintsLength2;
}

int Linearization::getObjectNumberFirst()
{
  int objectNumberFirst = 0;
  if (linDict.isDict() &&
      linDict.getDict()->lookupInt("O", NULL, &objectNumberFirst) &&
      objectNumberFirst > 0) {
    return objectNumberFirst;
  } else {
    error(errSyntaxWarning, -1, "Object number of first page in linearization table is invalid");
    return 0;
  }
}

Guint Linearization::getEndFirst()
{
  int pageEndFirst = 0;
  if (linDict.isDict() &&
      linDict.getDict()->lookupInt("E", NULL, &pageEndFirst) &&
      pageEndFirst > 0) {
    return pageEndFirst;
  } else {
    error(errSyntaxWarning, -1, "First page end offset in linearization table is invalid");
    return 0;
  }
}

int Linearization::getNumPages()
{
  int numPages = 0;
  if (linDict.isDict() &&
      linDict.getDict()->lookupInt("N", NULL, &numPages) &&
      numPages > 0) {
    return numPages;
  } else {
    error(errSyntaxWarning, -1, "Page count in linearization table is invalid");
    return 0;
  }
}

Guint Linearization::getMainXRefEntriesOffset()
{
  int mainXRefEntriesOffset = 0;
  if (linDict.isDict() &&
      linDict.getDict()->lookupInt("T", NULL, &mainXRefEntriesOffset) &&
      mainXRefEntriesOffset > 0) {
    return mainXRefEntriesOffset;
  } else {
    error(errSyntaxWarning, -1, "Main Xref offset in linearization table is invalid");
    return 0;
  }
}

int Linearization::getPageFirst()
{
  int pageFirst = 0; // Optional, defaults to 0.

  if (linDict.isDict()) {
    linDict.getDict()->lookupInt("P", NULL, &pageFirst);
  }

  if ((pageFirst < 0) || (pageFirst >= getNumPages())) {
    error(errSyntaxWarning, -1, "First page in linearization table is invalid");
    return 0;
  }

  return pageFirst;
}


