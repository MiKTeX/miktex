//========================================================================
//
// Linearization.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010, 2012 Hib Eris <hib@hiberis.nl>
// Copyright 2015 Jason Crain <jason@aquaticape.us>
// Copyright 2017, 2019 Albert Astals Cid <aacid@kde.org>
// Copyright 2019 Adam Reichold <adam.reichold@t-online.de>
// Copyright 2019 Even Rouault <even.rouault@spatialys.com>
//
//========================================================================

#include "Linearization.h"
#include "Parser.h"
#include "Lexer.h"

//------------------------------------------------------------------------
// Linearization
//------------------------------------------------------------------------

Linearization::Linearization(BaseStream *str)
{
    Parser *parser;

    str->reset();
    parser = new Parser(nullptr, str->makeSubStream(str->getStart(), false, 0, Object(objNull)), false);
    Object obj1 = parser->getObj();
    Object obj2 = parser->getObj();
    Object obj3 = parser->getObj();
    linDict = parser->getObj();
    if (obj1.isInt() && obj2.isInt() && obj3.isCmd("obj") && linDict.isDict()) {
        Object obj5 = linDict.dictLookup("Linearized");
        if (!(obj5.isNum() && obj5.getNum() > 0)) {
            linDict.setToNull();
        }
    } else {
        linDict.setToNull();
    }
    delete parser;
}

Linearization::~Linearization() { }

unsigned int Linearization::getLength() const
{
    if (!linDict.isDict())
        return 0;

    int length;
    if (linDict.getDict()->lookupInt("L", nullptr, &length) && length > 0) {
        return length;
    } else {
        error(errSyntaxWarning, -1, "Length in linearization table is invalid");
        return 0;
    }
}

unsigned int Linearization::getHintsOffset() const
{
    int hintsOffset;

    Object obj1, obj2;
    if (linDict.isDict() && (obj1 = linDict.dictLookup("H"), obj1.isArray()) && obj1.arrayGetLength() >= 2 && (obj2 = obj1.arrayGet(0), obj2.isInt()) && obj2.getInt() > 0) {
        hintsOffset = obj2.getInt();
    } else {
        error(errSyntaxWarning, -1, "Hints table offset in linearization table is invalid");
        hintsOffset = 0;
    }

    return hintsOffset;
}

unsigned int Linearization::getHintsLength() const
{
    int hintsLength;

    Object obj1, obj2;
    if (linDict.isDict() && (obj1 = linDict.dictLookup("H"), obj1.isArray()) && obj1.arrayGetLength() >= 2 && (obj2 = obj1.arrayGet(1), obj2.isInt()) && obj2.getInt() > 0) {
        hintsLength = obj2.getInt();
    } else {
        error(errSyntaxWarning, -1, "Hints table length in linearization table is invalid");
        hintsLength = 0;
    }

    return hintsLength;
}

unsigned int Linearization::getHintsOffset2() const
{
    int hintsOffset2 = 0; // default to 0

    Object obj1;
    if (linDict.isDict() && (obj1 = linDict.dictLookup("H"), obj1.isArray()) && obj1.arrayGetLength() >= 4) {
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

unsigned int Linearization::getHintsLength2() const
{
    int hintsLength2 = 0; // default to 0

    Object obj1;
    if (linDict.isDict() && (obj1 = linDict.dictLookup("H"), obj1.isArray()) && obj1.arrayGetLength() >= 4) {
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

int Linearization::getObjectNumberFirst() const
{
    int objectNumberFirst = 0;
    if (linDict.isDict() && linDict.getDict()->lookupInt("O", nullptr, &objectNumberFirst) && objectNumberFirst > 0) {
        return objectNumberFirst;
    } else {
        error(errSyntaxWarning, -1, "Object number of first page in linearization table is invalid");
        return 0;
    }
}

unsigned int Linearization::getEndFirst() const
{
    int pageEndFirst = 0;
    if (linDict.isDict() && linDict.getDict()->lookupInt("E", nullptr, &pageEndFirst) && pageEndFirst > 0) {
        return pageEndFirst;
    } else {
        error(errSyntaxWarning, -1, "First page end offset in linearization table is invalid");
        return 0;
    }
}

int Linearization::getNumPages() const
{
    int numPages = 0;
    if (linDict.isDict() && linDict.getDict()->lookupInt("N", nullptr, &numPages) && numPages > 0) {
        return numPages;
    } else {
        error(errSyntaxWarning, -1, "Page count in linearization table is invalid");
        return 0;
    }
}

unsigned int Linearization::getMainXRefEntriesOffset() const
{
    int mainXRefEntriesOffset = 0;
    if (linDict.isDict() && linDict.getDict()->lookupInt("T", nullptr, &mainXRefEntriesOffset) && mainXRefEntriesOffset > 0) {
        return mainXRefEntriesOffset;
    } else {
        error(errSyntaxWarning, -1, "Main Xref offset in linearization table is invalid");
        return 0;
    }
}

int Linearization::getPageFirst() const
{
    int pageFirst = 0; // Optional, defaults to 0.

    if (linDict.isDict()) {
        linDict.getDict()->lookupInt("P", nullptr, &pageFirst);
    }

    if ((pageFirst < 0) || (pageFirst >= getNumPages())) {
        error(errSyntaxWarning, -1, "First page in linearization table is invalid");
        return 0;
    }

    return pageFirst;
}
