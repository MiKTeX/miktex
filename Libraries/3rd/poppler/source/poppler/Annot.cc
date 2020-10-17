//========================================================================
//
// Annot.cc
//
// Copyright 2000-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006 Scott Turner <scotty1024@mac.com>
// Copyright (C) 2007, 2008 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2007-2013, 2015-2020 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2007-2013, 2018 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2007, 2008 Iñigo Martínez <inigomartinez@gmail.com>
// Copyright (C) 2007 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2008, 2011 Pino Toscano <pino@kde.org>
// Copyright (C) 2008 Michael Vrable <mvrable@cs.ucsd.edu>
// Copyright (C) 2008 Hugo Mercier <hmercier31@gmail.com>
// Copyright (C) 2009 Ilya Gorenbein <igorenbein@finjan.com>
// Copyright (C) 2011, 2013, 2019 José Aliste <jaliste@src.gnome.org>
// Copyright (C) 2012, 2013 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2012, 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2012, 2015 Tobias Koenig <tokoe@kdab.com>
// Copyright (C) 2013 Peter Breitenlohner <peb@mppmu.mpg.de>
// Copyright (C) 2013, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2014, 2015 Marek Kasik <mkasik@redhat.com>
// Copyright (C) 2014 Jiri Slaby <jirislaby@gmail.com>
// Copyright (C) 2014 Anuj Khare <khareanuj18@gmail.com>
// Copyright (C) 2015 Petr Gajdos <pgajdos@suse.cz>
// Copyright (C) 2015 Philipp Reinkemeier <philipp.reinkemeier@offis.de>
// Copyright (C) 2015 Tamas Szekeres <szekerest@gmail.com>
// Copyright (C) 2017 Hans-Ulrich Jüttner <huj@froreich-bioscientia.de>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright 2018 Andre Heinecke <aheinecke@intevation.de>
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2018 Dileep Sankhla <sankhla.dileep96@gmail.com>
// Copyright (C) 2018, 2019 Tobias Deiminger <haxtibal@posteo.de>
// Copyright (C) 2018-2020 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright (C) 2019 Umang Malik <umang99m@gmail.com>
// Copyright (C) 2019 João Netto <joaonetto901@gmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <cstdlib>
#include <cmath>
#include <cassert>
#include "goo/gmem.h"
#include "goo/gstrtod.h"
#include "Error.h"
#include "Object.h"
#include "Catalog.h"
#include "Gfx.h"
#include "Lexer.h"
#include "PDFDoc.h"
#include "Page.h"
#include "Annot.h"
#include "GfxFont.h"
#include "CharCodeToUnicode.h"
#include "PDFDocEncoding.h"
#include "Form.h"
#include "Error.h"
#include "XRef.h"
#include "Movie.h"
#include "OptionalContent.h"
#include "Sound.h"
#include "FileSpec.h"
#include "DateInfo.h"
#include "Link.h"
#include <cstring>
#include <algorithm>

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

#define fieldFlagReadOnly 0x00000001
#define fieldFlagRequired 0x00000002
#define fieldFlagNoExport 0x00000004
#define fieldFlagMultiline 0x00001000
#define fieldFlagPassword 0x00002000
#define fieldFlagNoToggleToOff 0x00004000
#define fieldFlagRadio 0x00008000
#define fieldFlagPushbutton 0x00010000
#define fieldFlagCombo 0x00020000
#define fieldFlagEdit 0x00040000
#define fieldFlagSort 0x00080000
#define fieldFlagFileSelect 0x00100000
#define fieldFlagMultiSelect 0x00200000
#define fieldFlagDoNotSpellCheck 0x00400000
#define fieldFlagDoNotScroll 0x00800000
#define fieldFlagComb 0x01000000
#define fieldFlagRichText 0x02000000
#define fieldFlagRadiosInUnison 0x02000000
#define fieldFlagCommitOnSelChange 0x04000000

#define fieldQuadLeft 0
#define fieldQuadCenter 1
#define fieldQuadRight 2

// distance of Bezier control point from center for circle approximation
// = (4 * (sqrt(2) - 1) / 3) * r
#define bezierCircle 0.55228475

static AnnotLineEndingStyle parseAnnotLineEndingStyle(const GooString *string)
{
    if (string != nullptr) {
        if (!string->cmp("Square")) {
            return annotLineEndingSquare;
        } else if (!string->cmp("Circle")) {
            return annotLineEndingCircle;
        } else if (!string->cmp("Diamond")) {
            return annotLineEndingDiamond;
        } else if (!string->cmp("OpenArrow")) {
            return annotLineEndingOpenArrow;
        } else if (!string->cmp("ClosedArrow")) {
            return annotLineEndingClosedArrow;
        } else if (!string->cmp("Butt")) {
            return annotLineEndingButt;
        } else if (!string->cmp("ROpenArrow")) {
            return annotLineEndingROpenArrow;
        } else if (!string->cmp("RClosedArrow")) {
            return annotLineEndingRClosedArrow;
        } else if (!string->cmp("Slash")) {
            return annotLineEndingSlash;
        } else {
            return annotLineEndingNone;
        }
    } else {
        return annotLineEndingNone;
    }
}

static const char *convertAnnotLineEndingStyle(AnnotLineEndingStyle style)
{
    switch (style) {
    case annotLineEndingSquare:
        return "Square";
    case annotLineEndingCircle:
        return "Circle";
    case annotLineEndingDiamond:
        return "Diamond";
    case annotLineEndingOpenArrow:
        return "OpenArrow";
    case annotLineEndingClosedArrow:
        return "ClosedArrow";
    case annotLineEndingButt:
        return "Butt";
    case annotLineEndingROpenArrow:
        return "ROpenArrow";
    case annotLineEndingRClosedArrow:
        return "RClosedArrow";
    case annotLineEndingSlash:
        return "Slash";
    default:
        return "None";
    }
}

static AnnotExternalDataType parseAnnotExternalData(Dict *dict)
{
    AnnotExternalDataType type;

    Object obj1 = dict->lookup("Subtype");
    if (obj1.isName()) {
        const char *typeName = obj1.getName();

        if (!strcmp(typeName, "Markup3D")) {
            type = annotExternalDataMarkup3D;
        } else {
            type = annotExternalDataMarkupUnknown;
        }
    } else {
        type = annotExternalDataMarkupUnknown;
    }

    return type;
}

static std::unique_ptr<PDFRectangle> parseDiffRectangle(Array *array, PDFRectangle *rect)
{
    if (array->getLength() == 4) {
        // deltas
        const double dx1 = array->get(0).getNumWithDefaultValue(0);
        const double dy1 = array->get(1).getNumWithDefaultValue(0);
        const double dx2 = array->get(2).getNumWithDefaultValue(0);
        const double dy2 = array->get(3).getNumWithDefaultValue(0);

        // checking that the numbers are valid (i.e. >= 0),
        // and that applying the differences still give us a valid rect
        if (dx1 >= 0 && dy1 >= 0 && dx2 >= 0 && dy2 && (rect->x2 - rect->x1 - dx1 - dx2) >= 0 && (rect->y2 - rect->y1 - dy1 - dy2) >= 0) {
            auto newRect = std::make_unique<PDFRectangle>();
            newRect->x1 = rect->x1 + dx1;
            newRect->y1 = rect->y1 + dy1;
            newRect->x2 = rect->x2 - dx2;
            newRect->y2 = rect->y2 - dy2;
            return newRect;
        }
    }
    return nullptr;
}

static std::unique_ptr<LinkAction> getAdditionalAction(Annot::AdditionalActionsType type, Object *additionalActions, PDFDoc *doc)
{
    Object additionalActionsObject = additionalActions->fetch(doc->getXRef());

    if (additionalActionsObject.isDict()) {
        const char *key =
                (type == Annot::actionCursorEntering ? "E"
                                                     : type == Annot::actionCursorLeaving ? "X"
                                                                                          : type == Annot::actionMousePressed ? "D"
                                                                                                                              : type == Annot::actionMouseReleased ? "U"
                                                                                                                                                                   : type == Annot::actionFocusIn ? "Fo"
                                                                                                                                                                                                  : type == Annot::actionFocusOut
                                                                 ? "Bl"
                                                                 : type == Annot::actionPageOpening ? "PO"
                                                                                                    : type == Annot::actionPageClosing ? "PC" : type == Annot::actionPageVisible ? "PV" : type == Annot::actionPageInvisible ? "PI" : nullptr);

        Object actionObject = additionalActionsObject.dictLookup(key);
        if (actionObject.isDict())
            return LinkAction::parseAction(&actionObject, doc->getCatalog()->getBaseURI());
    }

    return nullptr;
}

static const char *getFormAdditionalActionKey(Annot::FormAdditionalActionsType type)
{
    return (type == Annot::actionFieldModified ? "K" : type == Annot::actionFormatField ? "F" : type == Annot::actionValidateField ? "V" : type == Annot::actionCalculateField ? "C" : nullptr);
}

//------------------------------------------------------------------------
// AnnotBorderEffect
//------------------------------------------------------------------------

AnnotBorderEffect::AnnotBorderEffect(Dict *dict)
{
    Object obj1;

    obj1 = dict->lookup("S");
    if (obj1.isName()) {
        const char *effectName = obj1.getName();

        if (!strcmp(effectName, "C"))
            effectType = borderEffectCloudy;
        else
            effectType = borderEffectNoEffect;
    } else {
        effectType = borderEffectNoEffect;
    }

    if (effectType == borderEffectCloudy) {
        intensity = dict->lookup("I").getNumWithDefaultValue(0);
    } else {
        intensity = 0;
    }
}

//------------------------------------------------------------------------
// AnnotPath
//------------------------------------------------------------------------

AnnotPath::AnnotPath() = default;

AnnotPath::AnnotPath(Array *array)
{
    parsePathArray(array);
}

AnnotPath::AnnotPath(std::vector<AnnotCoord> &&coordsA)
{
    coords = std::move(coordsA);
}

AnnotPath::~AnnotPath() = default;

double AnnotPath::getX(int coord) const
{
    if (coord >= 0 && coord < getCoordsLength())
        return coords[coord].getX();
    return 0;
}

double AnnotPath::getY(int coord) const
{
    if (coord >= 0 && coord < getCoordsLength())
        return coords[coord].getY();
    return 0;
}

AnnotCoord *AnnotPath::getCoord(int coord)
{
    if (coord >= 0 && coord < getCoordsLength())
        return &coords[coord];
    return nullptr;
}

void AnnotPath::parsePathArray(Array *array)
{
    if (array->getLength() % 2) {
        error(errSyntaxError, -1, "Bad Annot Path");
        return;
    }

    const auto tempLength = array->getLength() / 2;
    std::vector<AnnotCoord> tempCoords;
    tempCoords.reserve(tempLength);
    for (int i = 0; i < tempLength; i++) {
        double x = 0, y = 0;

        Object obj1 = array->get(i * 2);
        if (obj1.isNum()) {
            x = obj1.getNum();
        } else {
            return;
        }

        obj1 = array->get((i * 2) + 1);
        if (obj1.isNum()) {
            y = obj1.getNum();
        } else {
            return;
        }

        tempCoords.emplace_back(x, y);
    }

    coords = std::move(tempCoords);
}

//------------------------------------------------------------------------
// AnnotCalloutLine
//------------------------------------------------------------------------

AnnotCalloutLine::AnnotCalloutLine(double x1, double y1, double x2, double y2) : coord1(x1, y1), coord2(x2, y2) { }

//------------------------------------------------------------------------
// AnnotCalloutMultiLine
//------------------------------------------------------------------------

AnnotCalloutMultiLine::AnnotCalloutMultiLine(double x1, double y1, double x2, double y2, double x3, double y3) : AnnotCalloutLine(x1, y1, x2, y2), coord3(x3, y3) { }

//------------------------------------------------------------------------
// AnnotQuadrilateral
//------------------------------------------------------------------------

AnnotQuadrilaterals::AnnotQuadrilaterals(Array *array, PDFRectangle *rect)
{
    int arrayLength = array->getLength();
    int quadsLength = 0;
    double quadArray[8];

    // default values
    quadrilateralsLength = 0;

    if ((arrayLength % 8) == 0) {
        int i;

        quadsLength = arrayLength / 8;
        auto quads = std::make_unique<AnnotQuadrilateral[]>(quadsLength);
        for (i = 0; i < quadsLength; i++) {
            for (int j = 0; j < 8; j++) {
                Object obj = array->get(i * 8 + j);
                if (obj.isNum()) {
                    quadArray[j] = obj.getNum();
                } else {
                    error(errSyntaxError, -1, "Invalid QuadPoint in annot");
                    return;
                }
            }

            quads[i] = AnnotQuadrilateral(quadArray[0], quadArray[1], quadArray[2], quadArray[3], quadArray[4], quadArray[5], quadArray[6], quadArray[7]);
        }

        quadrilateralsLength = quadsLength;
        quadrilaterals = std::move(quads);
    }
}

AnnotQuadrilaterals::AnnotQuadrilaterals(std::unique_ptr<AnnotQuadrilateral[]> &&quads, int quadsLength)
{
    quadrilaterals = std::move(quads);
    quadrilateralsLength = quadsLength;
}

AnnotQuadrilaterals::~AnnotQuadrilaterals() = default;

double AnnotQuadrilaterals::getX1(int quadrilateral)
{
    if (quadrilateral >= 0 && quadrilateral < quadrilateralsLength)
        return quadrilaterals[quadrilateral].coord1.getX();
    return 0;
}

double AnnotQuadrilaterals::getY1(int quadrilateral)
{
    if (quadrilateral >= 0 && quadrilateral < quadrilateralsLength)
        return quadrilaterals[quadrilateral].coord1.getY();
    return 0;
}

double AnnotQuadrilaterals::getX2(int quadrilateral)
{
    if (quadrilateral >= 0 && quadrilateral < quadrilateralsLength)
        return quadrilaterals[quadrilateral].coord2.getX();
    return 0;
}

double AnnotQuadrilaterals::getY2(int quadrilateral)
{
    if (quadrilateral >= 0 && quadrilateral < quadrilateralsLength)
        return quadrilaterals[quadrilateral].coord2.getY();
    return 0;
}

double AnnotQuadrilaterals::getX3(int quadrilateral)
{
    if (quadrilateral >= 0 && quadrilateral < quadrilateralsLength)
        return quadrilaterals[quadrilateral].coord3.getX();
    return 0;
}

double AnnotQuadrilaterals::getY3(int quadrilateral)
{
    if (quadrilateral >= 0 && quadrilateral < quadrilateralsLength)
        return quadrilaterals[quadrilateral].coord3.getY();
    return 0;
}

double AnnotQuadrilaterals::getX4(int quadrilateral)
{
    if (quadrilateral >= 0 && quadrilateral < quadrilateralsLength)
        return quadrilaterals[quadrilateral].coord4.getX();
    return 0;
}

double AnnotQuadrilaterals::getY4(int quadrilateral)
{
    if (quadrilateral >= 0 && quadrilateral < quadrilateralsLength)
        return quadrilaterals[quadrilateral].coord4.getY();
    return 0;
}

AnnotQuadrilaterals::AnnotQuadrilateral::AnnotQuadrilateral() = default;

AnnotQuadrilaterals::AnnotQuadrilateral::AnnotQuadrilateral(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4) : coord1(x1, y1), coord2(x2, y2), coord3(x3, y3), coord4(x4, y4) { }

//------------------------------------------------------------------------
// AnnotBorder
//------------------------------------------------------------------------
AnnotBorder::AnnotBorder()
{
    width = 1;
    dashLength = 0;
    dash = nullptr;
    style = borderSolid;
}

bool AnnotBorder::parseDashArray(Object *dashObj)
{
    bool correct = true;
    const int tempLength = dashObj->arrayGetLength();
    double *tempDash = (double *)gmallocn(tempLength, sizeof(double));

    // TODO: check not all zero (Line Dash Pattern Page 217 PDF 8.1)
    for (int i = 0; i < tempLength && i < DASH_LIMIT && correct; i++) {
        const Object obj1 = dashObj->arrayGet(i);
        if (obj1.isNum()) {
            tempDash[i] = obj1.getNum();

            correct = tempDash[i] >= 0;
        } else {
            correct = false;
        }
    }

    if (correct) {
        dashLength = tempLength;
        dash = tempDash;
        style = borderDashed;
    } else {
        gfree(tempDash);
    }

    return correct;
}

AnnotBorder::~AnnotBorder()
{
    if (dash)
        gfree(dash);
}

//------------------------------------------------------------------------
// AnnotBorderArray
//------------------------------------------------------------------------

AnnotBorderArray::AnnotBorderArray()
{
    horizontalCorner = 0;
    verticalCorner = 0;
}

AnnotBorderArray::AnnotBorderArray(Array *array)
{
    Object obj1;
    int arrayLength = array->getLength();

    bool correct = true;
    if (arrayLength == 3 || arrayLength == 4) {
        // implementation note 81 in Appendix H.

        obj1 = array->get(0);
        if (obj1.isNum())
            horizontalCorner = obj1.getNum();
        else
            correct = false;

        obj1 = array->get(1);
        if (obj1.isNum())
            verticalCorner = obj1.getNum();
        else
            correct = false;

        obj1 = array->get(2);
        if (obj1.isNum())
            width = obj1.getNum();
        else
            correct = false;

        if (arrayLength == 4) {
            obj1 = array->get(3);
            if (obj1.isArray())
                correct = parseDashArray(&obj1);
            else
                correct = false;
        }
    } else {
        correct = false;
    }

    if (!correct) {
        width = 0;
    }
}

Object AnnotBorderArray::writeToObject(XRef *xref) const
{
    Array *borderArray = new Array(xref);
    borderArray->add(Object(horizontalCorner));
    borderArray->add(Object(verticalCorner));
    borderArray->add(Object(width));

    if (dashLength > 0) {
        Array *a = new Array(xref);

        for (int i = 0; i < dashLength; i++)
            a->add(Object(dash[i]));

        borderArray->add(Object(a));
    }

    return Object(borderArray);
}

//------------------------------------------------------------------------
// AnnotBorderBS
//------------------------------------------------------------------------

AnnotBorderBS::AnnotBorderBS() { }

AnnotBorderBS::AnnotBorderBS(Dict *dict)
{
    Object obj1, obj2;

    // acroread 8 seems to need both W and S entries for
    // any border to be drawn, even though the spec
    // doesn't claim anything of that sort. We follow
    // that behaviour by verifying both entries exist
    // otherwise we set the borderWidth to 0
    // --jrmuizel
    obj1 = dict->lookup("W");
    obj2 = dict->lookup("S");
    if (obj1.isNum() && obj2.isName()) {
        const char *styleName = obj2.getName();

        width = obj1.getNum();

        if (!strcmp(styleName, "S")) {
            style = borderSolid;
        } else if (!strcmp(styleName, "D")) {
            style = borderDashed;
        } else if (!strcmp(styleName, "B")) {
            style = borderBeveled;
        } else if (!strcmp(styleName, "I")) {
            style = borderInset;
        } else if (!strcmp(styleName, "U")) {
            style = borderUnderlined;
        } else {
            style = borderSolid;
        }
    } else {
        width = 0;
    }

    if (style == borderDashed) {
        obj1 = dict->lookup("D");
        if (obj1.isArray())
            parseDashArray(&obj1);

        if (!dash) {
            dashLength = 1;
            dash = (double *)gmallocn(dashLength, sizeof(double));
            dash[0] = 3;
        }
    }
}

const char *AnnotBorderBS::getStyleName() const
{
    switch (style) {
    case borderSolid:
        return "S";
    case borderDashed:
        return "D";
    case borderBeveled:
        return "B";
    case borderInset:
        return "I";
    case borderUnderlined:
        return "U";
    }

    return "S";
}

Object AnnotBorderBS::writeToObject(XRef *xref) const
{
    Dict *dict = new Dict(xref);
    dict->set("W", Object(width));
    dict->set("S", Object(objName, getStyleName()));
    if (style == borderDashed && dashLength > 0) {
        Array *a = new Array(xref);

        for (int i = 0; i < dashLength; i++)
            a->add(Object(dash[i]));
        dict->set("D", Object(a));
    }
    return Object(dict);
}

//------------------------------------------------------------------------
// AnnotColor
//------------------------------------------------------------------------

AnnotColor::AnnotColor()
{
    length = 0;
}

AnnotColor::AnnotColor(double gray)
{
    length = 1;

    values[0] = gray;
}

AnnotColor::AnnotColor(double r, double g, double b)
{
    length = 3;

    values[0] = r;
    values[1] = g;
    values[2] = b;
}

AnnotColor::AnnotColor(double c, double m, double y, double k)
{
    length = 4;

    values[0] = c;
    values[1] = m;
    values[2] = y;
    values[3] = k;
}

// If <adjust> is +1, color is brightened;
// if <adjust> is -1, color is darkened;
// otherwise color is not modified.
AnnotColor::AnnotColor(Array *array, int adjust)
{
    int i;

    length = array->getLength();
    if (length > 4)
        length = 4;

    for (i = 0; i < length; i++) {
        Object obj1 = array->get(i);
        if (obj1.isNum()) {
            values[i] = obj1.getNum();

            if (values[i] < 0 || values[i] > 1)
                values[i] = 0;
        } else {
            values[i] = 0;
        }
    }

    if (adjust != 0)
        adjustColor(adjust);
}

void AnnotColor::adjustColor(int adjust)
{
    int i;

    if (length == 4) {
        adjust = -adjust;
    }
    if (adjust > 0) {
        for (i = 0; i < length; ++i) {
            values[i] = 0.5 * values[i] + 0.5;
        }
    } else if (adjust < 0) {
        for (i = 0; i < length; ++i) {
            values[i] = 0.5 * values[i];
        }
    }
}

Object AnnotColor::writeToObject(XRef *xref) const
{
    if (length == 0) {
        return Object(objNull); // Transparent (no color)
    } else {
        Array *a = new Array(xref);
        for (int i = 0; i < length; ++i)
            a->add(Object(values[i]));
        return Object(a);
    }
}

//------------------------------------------------------------------------
// DefaultAppearance
//------------------------------------------------------------------------

DefaultAppearance::DefaultAppearance(Object &&fontNameA, double fontPtSizeA, std::unique_ptr<AnnotColor> fontColorA) : fontName(std::move(fontNameA)), fontPtSize(fontPtSizeA), fontColor(std::move(fontColorA)) { }

DefaultAppearance::DefaultAppearance(GooString *da)
{
    fontPtSize = -1;

    if (da) {
        std::vector<GooString *> *daToks = new std::vector<GooString *>();
        int i = FormFieldText::tokenizeDA(da, daToks, "Tf");

        if (i >= 1) {
            fontPtSize = gatof((*daToks)[i - 1]->c_str());
        }
        if (i >= 2) {
            // We are expecting a name, therefore the first letter should be '/'.
            const GooString *fontToken = (*daToks)[i - 2];
            if (fontToken && fontToken->getLength() > 1 && fontToken->getChar(0) == '/') {
                // The +1 is here to skip the leading '/'.
                fontName = Object(objName, fontToken->c_str() + 1);
            }
        }
        // Scan backwards: we are looking for the last set value
        for (i = daToks->size() - 1; i >= 0; --i) {
            if (!fontColor) {
                if (!((*daToks)[i])->cmp("g") && i >= 1) {
                    fontColor = std::make_unique<AnnotColor>(gatof(((*daToks)[i - 1])->c_str()));
                } else if (!((*daToks)[i])->cmp("rg") && i >= 3) {
                    fontColor = std::make_unique<AnnotColor>(gatof(((*daToks)[i - 3])->c_str()), gatof(((*daToks)[i - 2])->c_str()), gatof(((*daToks)[i - 1])->c_str()));
                } else if (!((*daToks)[i])->cmp("k") && i >= 4) {
                    fontColor = std::make_unique<AnnotColor>(gatof(((*daToks)[i - 4])->c_str()), gatof(((*daToks)[i - 3])->c_str()), gatof(((*daToks)[i - 2])->c_str()), gatof(((*daToks)[i - 1])->c_str()));
                }
            }
        }
        for (auto entry : *daToks) {
            delete entry;
        }
        delete daToks;
    }
}

void DefaultAppearance::setFontName(Object &&fontNameA)
{
    fontName = std::move(fontNameA);
}

void DefaultAppearance::setFontPtSize(double fontPtSizeA)
{
    fontPtSize = fontPtSizeA;
}

void DefaultAppearance::setFontColor(std::unique_ptr<AnnotColor> fontColorA)
{
    fontColor = std::move(fontColorA);
}

GooString *DefaultAppearance::toAppearanceString() const
{
    AnnotAppearanceBuilder appearBuilder;
    if (fontColor) {
        appearBuilder.setDrawColor(fontColor.get(), true);
    }
    appearBuilder.setTextFont(fontName, fontPtSize);
    return appearBuilder.buffer()->copy();
}

//------------------------------------------------------------------------
// AnnotIconFit
//------------------------------------------------------------------------

AnnotIconFit::AnnotIconFit(Dict *dict)
{
    Object obj1;

    obj1 = dict->lookup("SW");
    if (obj1.isName()) {
        const char *scaleName = obj1.getName();

        if (!strcmp(scaleName, "B")) {
            scaleWhen = scaleBigger;
        } else if (!strcmp(scaleName, "S")) {
            scaleWhen = scaleSmaller;
        } else if (!strcmp(scaleName, "N")) {
            scaleWhen = scaleNever;
        } else {
            scaleWhen = scaleAlways;
        }
    } else {
        scaleWhen = scaleAlways;
    }

    obj1 = dict->lookup("S");
    if (obj1.isName()) {
        const char *scaleName = obj1.getName();

        if (!strcmp(scaleName, "A")) {
            scale = scaleAnamorphic;
        } else {
            scale = scaleProportional;
        }
    } else {
        scale = scaleProportional;
    }

    obj1 = dict->lookup("A");
    if (obj1.isArray() && obj1.arrayGetLength() == 2) {
        left = obj1.arrayGet(0).getNumWithDefaultValue(0);
        bottom = obj1.arrayGet(1).getNumWithDefaultValue(0);

        if (left < 0 || left > 1)
            left = 0.5;

        if (bottom < 0 || bottom > 1)
            bottom = 0.5;

    } else {
        left = bottom = 0.5;
    }

    fullyBounds = dict->lookup("FB").getBoolWithDefaultValue(false);
}

//------------------------------------------------------------------------
// AnnotAppearance
//------------------------------------------------------------------------

AnnotAppearance::AnnotAppearance(PDFDoc *docA, Object *dict)
{
    assert(dict->isDict());
    doc = docA;
    appearDict = dict->copy();
}

AnnotAppearance::~AnnotAppearance() { }

Object AnnotAppearance::getAppearanceStream(AnnotAppearanceType type, const char *state)
{
    Object apData;

    // Obtain dictionary or stream associated to appearance type
    switch (type) {
    case appearRollover:
        apData = appearDict.dictLookupNF("R").copy();
        if (apData.isNull())
            apData = appearDict.dictLookupNF("N").copy();
        break;
    case appearDown:
        apData = appearDict.dictLookupNF("D").copy();
        if (apData.isNull())
            apData = appearDict.dictLookupNF("N").copy();
        break;
    case appearNormal:
        apData = appearDict.dictLookupNF("N").copy();
        break;
    }

    if (apData.isDict() && state)
        return apData.dictLookupNF(state).copy();
    else if (apData.isRef())
        return apData;

    return Object();
}

std::unique_ptr<GooString> AnnotAppearance::getStateKey(int i)
{
    const Object &obj1 = appearDict.dictLookupNF("N");
    if (obj1.isDict())
        return std::make_unique<GooString>(obj1.dictGetKey(i));
    return nullptr;
}

int AnnotAppearance::getNumStates()
{
    int res = 0;
    const Object &obj1 = appearDict.dictLookupNF("N");
    if (obj1.isDict())
        res = obj1.dictGetLength();
    return res;
}

// Test if stateObj (a Ref or a Dict) points to the specified stream
bool AnnotAppearance::referencesStream(const Object *stateObj, Ref refToStream)
{
    if (stateObj->isRef()) {
        const Ref r = stateObj->getRef();
        if (r == refToStream) {
            return true;
        }
    } else if (stateObj->isDict()) { // Test each value
        const int size = stateObj->dictGetLength();
        for (int i = 0; i < size; ++i) {
            const Object &obj1 = stateObj->dictGetValNF(i);
            if (obj1.isRef()) {
                const Ref r = obj1.getRef();
                if (r == refToStream) {
                    return true;
                }
            }
        }
    }
    return false; // Not found
}

// Test if this AnnotAppearance references the specified stream
bool AnnotAppearance::referencesStream(Ref refToStream)
{
    bool found;

    // Scan each state's ref/subdictionary
    const Object &objN = appearDict.dictLookupNF("N");
    found = referencesStream(&objN, refToStream);
    if (found)
        return true;

    const Object &objR = appearDict.dictLookupNF("R");
    found = referencesStream(&objR, refToStream);
    if (found)
        return true;

    const Object &objD = appearDict.dictLookupNF("D");
    found = referencesStream(&objD, refToStream);
    return found;
}

// If this is the only annotation in the document that references the
// specified appearance stream, remove the appearance stream
void AnnotAppearance::removeStream(Ref refToStream)
{
    const int lastpage = doc->getNumPages();
    for (int pg = 1; pg <= lastpage; ++pg) { // Scan all annotations in the document
        Page *page = doc->getPage(pg);
        if (!page) {
            error(errSyntaxError, -1, "Failed check for shared annotation stream at page {0:d}", pg);
            continue;
        }
        Annots *annots = page->getAnnots();
        for (int i = 0; i < annots->getNumAnnots(); ++i) {
            AnnotAppearance *annotAp = annots->getAnnot(i)->getAppearStreams();
            if (annotAp && annotAp != this && annotAp->referencesStream(refToStream)) {
                return; // Another annotation points to the stream -> Don't delete it
            }
        }
    }

    // TODO: stream resources (e.g. font), AP name tree
    doc->getXRef()->removeIndirectObject(refToStream);
}

// Removes stream if obj is a Ref, or removes pointed streams if obj is a Dict
void AnnotAppearance::removeStateStreams(const Object *state)
{
    if (state->isRef()) {
        removeStream(state->getRef());
    } else if (state->isDict()) {
        const int size = state->dictGetLength();
        for (int i = 0; i < size; ++i) {
            const Object &obj2 = state->dictGetValNF(i);
            if (obj2.isRef()) {
                removeStream(obj2.getRef());
            }
        }
    }
}

void AnnotAppearance::removeAllStreams()
{
    const Object &objN = appearDict.dictLookupNF("N");
    removeStateStreams(&objN);
    const Object &objR = appearDict.dictLookupNF("R");
    removeStateStreams(&objR);
    const Object &objD = appearDict.dictLookupNF("D");
    removeStateStreams(&objD);
}

//------------------------------------------------------------------------
// AnnotAppearanceCharacs
//------------------------------------------------------------------------

AnnotAppearanceCharacs::AnnotAppearanceCharacs(Dict *dict)
{
    Object obj1;

    obj1 = dict->lookup("R");
    if (obj1.isInt()) {
        rotation = obj1.getInt();
    } else {
        rotation = 0;
    }

    obj1 = dict->lookup("BC");
    if (obj1.isArray()) {
        Array *colorComponents = obj1.getArray();
        if (colorComponents->getLength() > 0) {
            borderColor = std::make_unique<AnnotColor>(colorComponents);
        }
    }

    obj1 = dict->lookup("BG");
    if (obj1.isArray()) {
        Array *colorComponents = obj1.getArray();
        if (colorComponents->getLength() > 0) {
            backColor = std::make_unique<AnnotColor>(colorComponents);
        }
    }

    obj1 = dict->lookup("CA");
    if (obj1.isString()) {
        normalCaption = std::make_unique<GooString>(obj1.getString());
    }

    obj1 = dict->lookup("RC");
    if (obj1.isString()) {
        rolloverCaption = std::make_unique<GooString>(obj1.getString());
    }

    obj1 = dict->lookup("AC");
    if (obj1.isString()) {
        alternateCaption = std::make_unique<GooString>(obj1.getString());
    }

    obj1 = dict->lookup("IF");
    if (obj1.isDict()) {
        iconFit = std::make_unique<AnnotIconFit>(obj1.getDict());
    }

    obj1 = dict->lookup("TP");
    if (obj1.isInt()) {
        position = (AnnotAppearanceCharacsTextPos)obj1.getInt();
    } else {
        position = captionNoIcon;
    }
}

AnnotAppearanceCharacs::~AnnotAppearanceCharacs() = default;

//------------------------------------------------------------------------
// AnnotAppearanceBBox
//------------------------------------------------------------------------

AnnotAppearanceBBox::AnnotAppearanceBBox(PDFRectangle *rect)
{
    origX = rect->x1;
    origY = rect->y1;
    borderWidth = 0;

    // Initially set the same size as rect
    minX = 0;
    minY = 0;
    maxX = rect->x2 - rect->x1;
    maxY = rect->y2 - rect->y1;
}

void AnnotAppearanceBBox::extendTo(double x, double y)
{
    if (x < minX) {
        minX = x;
    } else if (x > maxX) {
        maxX = x;
    }
    if (y < minY) {
        minY = y;
    } else if (y > maxY) {
        maxY = y;
    }
}

void AnnotAppearanceBBox::getBBoxRect(double bbox[4]) const
{
    bbox[0] = minX - borderWidth;
    bbox[1] = minY - borderWidth;
    bbox[2] = maxX + borderWidth;
    bbox[3] = maxY + borderWidth;
}

double AnnotAppearanceBBox::getPageXMin() const
{
    return origX + minX - borderWidth;
}

double AnnotAppearanceBBox::getPageYMin() const
{
    return origY + minY - borderWidth;
}

double AnnotAppearanceBBox::getPageXMax() const
{
    return origX + maxX + borderWidth;
}

double AnnotAppearanceBBox::getPageYMax() const
{
    return origY + maxY + borderWidth;
}

//------------------------------------------------------------------------
// Annot
//------------------------------------------------------------------------

#define annotLocker() std::unique_lock<std::recursive_mutex> locker(mutex)

Annot::Annot(PDFDoc *docA, PDFRectangle *rectA)
{

    refCnt = 1;
    flags = flagUnknown;
    type = typeUnknown;

    Array *a = new Array(docA->getXRef());
    a->add(Object(rectA->x1));
    a->add(Object(rectA->y1));
    a->add(Object(rectA->x2));
    a->add(Object(rectA->y2));

    annotObj = Object(new Dict(docA->getXRef()));
    annotObj.dictSet("Type", Object(objName, "Annot"));
    annotObj.dictSet("Rect", Object(a));

    ref = docA->getXRef()->addIndirectObject(&annotObj);

    initialize(docA, annotObj.getDict());
}

Annot::Annot(PDFDoc *docA, Object &&dictObject)
{
    refCnt = 1;
    hasRef = false;
    flags = flagUnknown;
    type = typeUnknown;
    annotObj = std::move(dictObject);
    initialize(docA, annotObj.getDict());
}

Annot::Annot(PDFDoc *docA, Object &&dictObject, const Object *obj)
{
    refCnt = 1;
    if (obj->isRef()) {
        hasRef = true;
        ref = obj->getRef();
    } else {
        hasRef = false;
    }
    flags = flagUnknown;
    type = typeUnknown;
    annotObj = std::move(dictObject);
    initialize(docA, annotObj.getDict());
}

void Annot::initialize(PDFDoc *docA, Dict *dict)
{
    Object apObj, asObj, obj1;

    ok = true;
    doc = docA;

    appearance.setToNull();

    //----- parse the rectangle
    rect = std::make_unique<PDFRectangle>();
    obj1 = dict->lookup("Rect");
    if (obj1.isArray() && obj1.arrayGetLength() == 4) {
        rect->x1 = obj1.arrayGet(0).getNumWithDefaultValue(0);
        rect->y1 = obj1.arrayGet(1).getNumWithDefaultValue(0);
        rect->x2 = obj1.arrayGet(2).getNumWithDefaultValue(1);
        rect->y2 = obj1.arrayGet(3).getNumWithDefaultValue(1);

        if (rect->x1 > rect->x2) {
            double t = rect->x1;
            rect->x1 = rect->x2;
            rect->x2 = t;
        }

        if (rect->y1 > rect->y2) {
            double t = rect->y1;
            rect->y1 = rect->y2;
            rect->y2 = t;
        }
    } else {
        rect->x1 = rect->y1 = 0;
        rect->x2 = rect->y2 = 1;
        error(errSyntaxError, -1, "Bad bounding box for annotation");
        ok = false;
    }

    obj1 = dict->lookup("Contents");
    if (obj1.isString()) {
        contents.reset(obj1.getString()->copy());
    } else {
        contents = std::make_unique<GooString>();
    }

    // Note: This value is overwritten by Annots ctor
    const Object &pObj = dict->lookupNF("P");
    if (pObj.isRef()) {
        const Ref pRef = pObj.getRef();

        page = doc->getCatalog()->findPage(pRef);
    } else {
        page = 0;
    }

    obj1 = dict->lookup("NM");
    if (obj1.isString()) {
        name.reset(obj1.getString()->copy());
    }

    obj1 = dict->lookup("M");
    if (obj1.isString()) {
        modified.reset(obj1.getString()->copy());
    }

    //----- get the flags
    obj1 = dict->lookup("F");
    if (obj1.isInt()) {
        flags |= obj1.getInt();
    } else {
        flags = flagUnknown;
    }

    //----- get the annotation appearance dictionary
    apObj = dict->lookup("AP");
    if (apObj.isDict()) {
        appearStreams = std::make_unique<AnnotAppearance>(doc, &apObj);
    }

    //----- get the appearance state
    asObj = dict->lookup("AS");
    if (asObj.isName()) {
        appearState = std::make_unique<GooString>(asObj.getName());
    } else if (appearStreams && appearStreams->getNumStates() != 0) {
        error(errSyntaxError, -1, "Invalid or missing AS value in annotation containing one or more appearance subdictionaries");
        // AS value is required in this case, but if the
        // N dictionary contains only one entry
        // take it as default appearance.
        if (appearStreams->getNumStates() == 1) {
            appearState = appearStreams->getStateKey(0);
        }
    }
    if (!appearState) {
        appearState = std::make_unique<GooString>("Off");
    }

    //----- get the annotation appearance
    if (appearStreams) {
        appearance = appearStreams->getAppearanceStream(AnnotAppearance::appearNormal, appearState->c_str());
    }

    //----- parse the border style
    // According to the spec if neither the Border nor the BS entry is present,
    // the border shall be drawn as a solid line with a width of 1 point. But acroread
    // seems to ignore the Border entry for annots that can't have a BS entry. So, we only
    // follow this rule for annots tha can have a BS entry.
    obj1 = dict->lookup("Border");
    if (obj1.isArray()) {
        border = std::make_unique<AnnotBorderArray>(obj1.getArray());
    }

    obj1 = dict->lookup("C");
    if (obj1.isArray()) {
        color = std::make_unique<AnnotColor>(obj1.getArray());
    }

    obj1 = dict->lookup("StructParent");
    if (obj1.isInt()) {
        treeKey = obj1.getInt();
    } else {
        treeKey = 0;
    }

    oc = dict->lookupNF("OC").copy();
}

void Annot::getRect(double *x1, double *y1, double *x2, double *y2) const
{
    *x1 = rect->x1;
    *y1 = rect->y1;
    *x2 = rect->x2;
    *y2 = rect->y2;
}

void Annot::setRect(PDFRectangle *rectA)
{
    setRect(rectA->x1, rectA->y1, rectA->x2, rectA->y2);
}

void Annot::setRect(double x1, double y1, double x2, double y2)
{
    if (x1 < x2) {
        rect->x1 = x1;
        rect->x2 = x2;
    } else {
        rect->x1 = x2;
        rect->x2 = x1;
    }

    if (y1 < y2) {
        rect->y1 = y1;
        rect->y2 = y2;
    } else {
        rect->y1 = y2;
        rect->y2 = y1;
    }

    Array *a = new Array(doc->getXRef());
    a->add(Object(rect->x1));
    a->add(Object(rect->y1));
    a->add(Object(rect->x2));
    a->add(Object(rect->y2));

    update("Rect", Object(a));
    invalidateAppearance();
}

bool Annot::inRect(double x, double y) const
{
    return rect->contains(x, y);
}

void Annot::update(const char *key, Object &&value)
{
    annotLocker();
    /* Set M to current time, unless we are updating M itself */
    if (strcmp(key, "M") != 0) {
        modified.reset(timeToDateString(nullptr));

        annotObj.dictSet("M", Object(modified->copy()));
    }

    annotObj.dictSet(const_cast<char *>(key), std::move(value));

    doc->getXRef()->setModifiedObject(&annotObj, ref);
}

void Annot::setContents(GooString *new_content)
{
    annotLocker();

    if (new_content) {
        contents = std::make_unique<GooString>(new_content);
        // append the unicode marker <FE FF> if needed
        if (!contents->hasUnicodeMarker()) {
            contents->prependUnicodeMarker();
        }
    } else {
        contents = std::make_unique<GooString>();
    }

    update("Contents", Object(contents->copy()));
}

void Annot::setName(GooString *new_name)
{
    annotLocker();

    if (new_name) {
        name = std::make_unique<GooString>(new_name);
    } else {
        name = std::make_unique<GooString>();
    }

    update("NM", Object(name->copy()));
}

void Annot::setModified(GooString *new_modified)
{
    annotLocker();

    if (new_modified) {
        modified = std::make_unique<GooString>(new_modified);
        update("M", Object(modified->copy()));
    } else {
        modified.reset(nullptr);
        update("M", Object(objNull));
    }
}

void Annot::setFlags(unsigned int new_flags)
{
    annotLocker();
    flags = new_flags;
    update("F", Object(int(flags)));
}

void Annot::setBorder(std::unique_ptr<AnnotBorder> &&new_border)
{
    annotLocker();

    if (new_border) {
        Object obj1 = new_border->writeToObject(doc->getXRef());
        update(new_border->getType() == AnnotBorder::typeArray ? "Border" : "BS", std::move(obj1));
        border = std::move(new_border);
    } else {
        border = nullptr;
    }
    invalidateAppearance();
}

void Annot::setColor(std::unique_ptr<AnnotColor> &&new_color)
{
    annotLocker();

    if (new_color) {
        Object obj1 = new_color->writeToObject(doc->getXRef());
        update("C", std::move(obj1));
        color = std::move(new_color);
    } else {
        color = nullptr;
    }
    invalidateAppearance();
}

void Annot::setPage(int pageIndex, bool updateP)
{
    annotLocker();
    Page *pageobj = doc->getPage(pageIndex);
    Object obj1(objNull);

    if (pageobj) {
        const Ref pageRef = pageobj->getRef();
        obj1 = Object(pageRef);
        page = pageIndex;
    } else {
        page = 0;
    }

    if (updateP) {
        update("P", std::move(obj1));
    }
}

void Annot::setAppearanceState(const char *state)
{
    annotLocker();
    if (!state)
        return;

    appearState = std::make_unique<GooString>(state);
    appearBBox = nullptr;

    update("AS", Object(objName, state));

    // The appearance state determines the current appearance stream
    if (appearStreams) {
        appearance = appearStreams->getAppearanceStream(AnnotAppearance::appearNormal, appearState->c_str());
    } else {
        appearance.setToNull();
    }
}

void Annot::invalidateAppearance()
{
    annotLocker();
    if (appearStreams) { // Remove existing appearance streams
        appearStreams->removeAllStreams();
    }
    appearStreams = nullptr;
    appearState = nullptr;
    appearBBox = nullptr;
    appearance.setToNull();

    Object obj2 = annotObj.dictLookup("AP");
    if (!obj2.isNull())
        update("AP", Object(objNull)); // Remove AP

    obj2 = annotObj.dictLookup("AS");
    if (!obj2.isNull())
        update("AS", Object(objNull)); // Remove AS
}

double Annot::getXMin()
{
    return rect->x1;
}

double Annot::getYMin()
{
    return rect->y1;
}

double Annot::getXMax()
{
    return rect->x2;
}

double Annot::getYMax()
{
    return rect->y2;
}

void Annot::readArrayNum(Object *pdfArray, int key, double *value)
{
    Object valueObject = pdfArray->arrayGet(key);
    if (valueObject.isNum()) {
        *value = valueObject.getNum();
    } else {
        *value = 0;
        ok = false;
    }
}

void Annot::removeReferencedObjects()
{
    // Remove appearance streams (if any)
    invalidateAppearance();
}

void Annot::incRefCnt()
{
    refCnt++;
}

void Annot::decRefCnt()
{
    if (--refCnt == 0) {
        delete this;
    }
}

Annot::~Annot() { }

void AnnotAppearanceBuilder::setDrawColor(const AnnotColor *drawColor, bool fill)
{
    const double *values = drawColor->getValues();

    switch (drawColor->getSpace()) {
    case AnnotColor::colorCMYK:
        appearBuf->appendf("{0:.5f} {1:.5f} {2:.5f} {3:.5f} {4:c}\n", values[0], values[1], values[2], values[3], fill ? 'k' : 'K');
        break;
    case AnnotColor::colorRGB:
        appearBuf->appendf("{0:.5f} {1:.5f} {2:.5f} {3:s}\n", values[0], values[1], values[2], fill ? "rg" : "RG");
        break;
    case AnnotColor::colorGray:
        appearBuf->appendf("{0:.5f} {1:c}\n", values[0], fill ? 'g' : 'G');
        break;
    case AnnotColor::colorTransparent:
    default:
        break;
    }
}

void AnnotAppearanceBuilder::setTextFont(const Object &fontName, double fontSize)
{
    if (fontName.isName() && strlen(fontName.getName()) > 0)
        appearBuf->appendf("/{0:s} {1:.2f} Tf\n", fontName.getName(), fontSize);
}

void AnnotAppearanceBuilder::setLineStyleForBorder(const AnnotBorder *border)
{
    int i, dashLength;
    double *dash;

    switch (border->getStyle()) {
    case AnnotBorder::borderDashed:
        appearBuf->append("[");
        dashLength = border->getDashLength();
        dash = border->getDash();
        for (i = 0; i < dashLength; ++i)
            appearBuf->appendf(" {0:.2f}", dash[i]);
        appearBuf->append(" ] 0 d\n");
        break;
    default:
        appearBuf->append("[] 0 d\n");
        break;
    }
    appearBuf->appendf("{0:.2f} w\n", border->getWidth());
}

// Draw an (approximate) circle of radius <r> centered at (<cx>, <cy>).
// If <fill> is true, the circle is filled; otherwise it is stroked.
void AnnotAppearanceBuilder::drawCircle(double cx, double cy, double r, bool fill)
{
    appearBuf->appendf("{0:.2f} {1:.2f} m\n", cx + r, cy);
    appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n", cx + r, cy + bezierCircle * r, cx + bezierCircle * r, cy + r, cx, cy + r);
    appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n", cx - bezierCircle * r, cy + r, cx - r, cy + bezierCircle * r, cx - r, cy);
    appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n", cx - r, cy - bezierCircle * r, cx - bezierCircle * r, cy - r, cx, cy - r);
    appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n", cx + bezierCircle * r, cy - r, cx + r, cy - bezierCircle * r, cx + r, cy);
    appearBuf->append(fill ? "f\n" : "s\n");
}

// Draw the top-left half of an (approximate) circle of radius <r>
// centered at (<cx>, <cy>).
void AnnotAppearanceBuilder::drawCircleTopLeft(double cx, double cy, double r)
{
    double r2;

    r2 = r / sqrt(2.0);
    appearBuf->appendf("{0:.2f} {1:.2f} m\n", cx + r2, cy + r2);
    appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n", cx + (1 - bezierCircle) * r2, cy + (1 + bezierCircle) * r2, cx - (1 - bezierCircle) * r2, cy + (1 + bezierCircle) * r2, cx - r2, cy + r2);
    appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n", cx - (1 + bezierCircle) * r2, cy + (1 - bezierCircle) * r2, cx - (1 + bezierCircle) * r2, cy - (1 - bezierCircle) * r2, cx - r2, cy - r2);
    appearBuf->append("S\n");
}

// Draw the bottom-right half of an (approximate) circle of radius <r>
// centered at (<cx>, <cy>).
void AnnotAppearanceBuilder::drawCircleBottomRight(double cx, double cy, double r)
{
    double r2;

    r2 = r / sqrt(2.0);
    appearBuf->appendf("{0:.2f} {1:.2f} m\n", cx - r2, cy - r2);
    appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n", cx - (1 - bezierCircle) * r2, cy - (1 + bezierCircle) * r2, cx + (1 - bezierCircle) * r2, cy - (1 + bezierCircle) * r2, cx + r2, cy - r2);
    appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n", cx + (1 + bezierCircle) * r2, cy - (1 - bezierCircle) * r2, cx + (1 + bezierCircle) * r2, cy + (1 - bezierCircle) * r2, cx + r2, cy + r2);
    appearBuf->append("S\n");
}

void AnnotAppearanceBuilder::drawLineEndSquare(double x, double y, double size, bool fill, const Matrix &m)
{
    const double halfSize { size / 2. };
    const double x1[3] { x - size, x - size, x };
    const double y1[3] { y + halfSize, y - halfSize, y - halfSize };
    double tx, ty;

    m.transform(x, y + halfSize, &tx, &ty);
    appendf("{0:.2f} {1:.2f} m\n", tx, ty);
    for (int i = 0; i < 3; i++) {
        m.transform(x1[i], y1[i], &tx, &ty);
        appendf("{0:.2f} {1:.2f} l\n", tx, ty);
    }
    appearBuf->append(fill ? "b\n" : "s\n");
}

void AnnotAppearanceBuilder::drawLineEndCircle(double x, double y, double size, bool fill, const Matrix &m)
{
    const double halfSize { size / 2. };
    const double x1[4] { x, x - halfSize - bezierCircle * halfSize, x - size, x - halfSize + bezierCircle * halfSize };
    const double x2[4] { x - halfSize + bezierCircle * halfSize, x - size, x - halfSize - bezierCircle * halfSize, x };
    const double x3[4] { x - halfSize, x - size, x - halfSize, x };
    const double y1[4] { y + bezierCircle * halfSize, y + halfSize, y - bezierCircle * halfSize, y - halfSize };
    const double y2[4] { y + halfSize, y + bezierCircle * halfSize, y - halfSize, y - bezierCircle * halfSize };
    const double y3[4] { y + halfSize, y, y - halfSize, y };
    double tx[3];
    double ty[3];

    m.transform(x, y, &tx[0], &ty[0]);
    appearBuf->appendf("{0:.2f} {1:.2f} m\n", tx[0], ty[0]);
    for (int i = 0; i < 4; i++) {
        m.transform(x1[i], y1[i], &tx[0], &ty[0]);
        m.transform(x2[i], y2[i], &tx[1], &ty[1]);
        m.transform(x3[i], y3[i], &tx[2], &ty[2]);
        appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n", tx[0], ty[0], tx[1], ty[1], tx[2], ty[2]);
    }
    appearBuf->append(fill ? "b\n" : "s\n");
}

void AnnotAppearanceBuilder::drawLineEndDiamond(double x, double y, double size, bool fill, const Matrix &m)
{
    const double halfSize { size / 2. };
    const double x1[3] { x - halfSize, x - size, x - halfSize };
    const double y1[3] { y + halfSize, y, y - halfSize };
    double tx, ty;

    m.transform(x, y, &tx, &ty);
    appendf("{0:.2f} {1:.2f} m\n", tx, ty);
    for (int i = 0; i < 3; i++) {
        m.transform(x1[i], y1[i], &tx, &ty);
        appendf("{0:.2f} {1:.2f} l\n", tx, ty);
    }
    appearBuf->append(fill ? "b\n" : "s\n");
}

void AnnotAppearanceBuilder::drawLineEndArrow(double x, double y, double size, int orientation, bool isOpen, bool fill, const Matrix &m)
{
    const double alpha { M_PI / 6. };
    const double xOffs { orientation * size };
    const double yOffs { tan(alpha) * size };
    double tx, ty;

    m.transform(x - xOffs, y + yOffs, &tx, &ty);
    appendf("{0:.2f} {1:.2f} m\n", tx, ty);
    m.transform(x, y, &tx, &ty);
    appendf("{0:.2f} {1:.2f} l\n", tx, ty);
    m.transform(x - xOffs, y - yOffs, &tx, &ty);
    appendf("{0:.2f} {1:.2f} l\n", tx, ty);

    if (isOpen) {
        appearBuf->append("S\n");
    } else {
        appearBuf->append(fill ? "b\n" : "s\n");
    }
}

void AnnotAppearanceBuilder::drawLineEndSlash(double x, double y, double size, const Matrix &m)
{
    const double halfSize { size / 2. };
    const double xOffs { cos(M_PI / 3.) * halfSize };
    double tx, ty;

    m.transform(x - xOffs, y - halfSize, &tx, &ty);
    appendf("{0:.2f} {1:.2f} m\n", tx, ty);
    m.transform(x + xOffs, y + halfSize, &tx, &ty);
    appendf("{0:.2f} {1:.2f} l\n", tx, ty);
    appearBuf->append("S\n");
}

void AnnotAppearanceBuilder::drawLineEnding(AnnotLineEndingStyle endingStyle, double x, double y, double size, bool fill, const Matrix &m)
{
    switch (endingStyle) {
    case annotLineEndingSquare:
        drawLineEndSquare(x, y, size, fill, m);
        break;
    case annotLineEndingCircle:
        drawLineEndCircle(x, y, size, fill, m);
        break;
    case annotLineEndingDiamond:
        drawLineEndDiamond(x, y, size, fill, m);
        break;
    case annotLineEndingOpenArrow:
        drawLineEndArrow(x, y, size, 1, true, fill, m);
        break;
    case annotLineEndingClosedArrow:
        drawLineEndArrow(x, y, size, 1, false, fill, m);
        break;
    case annotLineEndingButt: {
        const double halfSize { size / 2. };
        double tx, ty;
        m.transform(x, y + halfSize, &tx, &ty);
        appendf("{0:.2f} {1:.2f} m\n", tx, ty);
        m.transform(x, y - halfSize, &tx, &ty);
        appendf("{0:.2f} {1:.2f} l S\n", tx, ty);
    } break;
    case annotLineEndingROpenArrow:
        drawLineEndArrow(x, y, size, -1, true, fill, m);
        break;
    case annotLineEndingRClosedArrow:
        drawLineEndArrow(x, y, size, -1, false, fill, m);
        break;
    case annotLineEndingSlash:
        drawLineEndSlash(x, y, size, m);
        break;
    default:
        break;
    }
}

double AnnotAppearanceBuilder::lineEndingXShorten(AnnotLineEndingStyle endingStyle, double size)
{
    switch (endingStyle) {
    case annotLineEndingCircle:
    case annotLineEndingClosedArrow:
    case annotLineEndingDiamond:
    case annotLineEndingSquare:
        return size;
    default:
        break;
    }
    return 0;
}

double AnnotAppearanceBuilder::lineEndingXExtendBBox(AnnotLineEndingStyle endingStyle, double size)
{
    switch (endingStyle) {
    case annotLineEndingRClosedArrow:
    case annotLineEndingROpenArrow:
        return size;
    case annotLineEndingSlash:
        return cos(M_PI / 3.) * size / 2.;
    default:
        break;
    }
    return 0;
}

Object Annot::createForm(const GooString *appearBuf, double *bbox, bool transparencyGroup, Dict *resDict)
{
    return createForm(appearBuf, bbox, transparencyGroup, resDict ? Object(resDict) : Object());
}

Object Annot::createForm(const GooString *appearBuf, double *bbox, bool transparencyGroup, Object &&resDictObject)
{
    Dict *appearDict = new Dict(doc->getXRef());
    appearDict->set("Length", Object(appearBuf->getLength()));
    appearDict->set("Subtype", Object(objName, "Form"));

    Array *a = new Array(doc->getXRef());
    a->add(Object(bbox[0]));
    a->add(Object(bbox[1]));
    a->add(Object(bbox[2]));
    a->add(Object(bbox[3]));
    appearDict->set("BBox", Object(a));
    if (transparencyGroup) {
        Dict *d = new Dict(doc->getXRef());
        d->set("S", Object(objName, "Transparency"));
        appearDict->set("Group", Object(d));
    }
    if (resDictObject.isDict())
        appearDict->set("Resources", std::move(resDictObject));

    Stream *mStream = new AutoFreeMemStream(copyString(appearBuf->c_str()), 0, appearBuf->getLength(), Object(appearDict));
    return Object(mStream);
}

Dict *Annot::createResourcesDict(const char *formName, Object &&formStream, const char *stateName, double opacity, const char *blendMode)
{
    Dict *gsDict = new Dict(doc->getXRef());
    if (opacity != 1) {
        gsDict->set("CA", Object(opacity));
        gsDict->set("ca", Object(opacity));
    }
    if (blendMode)
        gsDict->set("BM", Object(objName, blendMode));
    Dict *stateDict = new Dict(doc->getXRef());
    stateDict->set(stateName, Object(gsDict));
    Dict *formDict = new Dict(doc->getXRef());
    formDict->set(formName, std::move(formStream));

    Dict *resDict = new Dict(doc->getXRef());
    resDict->set("ExtGState", Object(stateDict));
    resDict->set("XObject", Object(formDict));

    return resDict;
}

Object Annot::getAppearanceResDict()
{
    Object obj1, obj2;

    // Fetch appearance's resource dict (if any)
    obj1 = appearance.fetch(doc->getXRef());
    if (obj1.isStream()) {
        obj2 = obj1.streamGetDict()->lookup("Resources");
        if (obj2.isDict()) {
            return obj2;
        }
    }

    return Object(objNull);
}

bool Annot::isVisible(bool printing)
{
    // check the flags
    if ((flags & flagHidden) || (printing && !(flags & flagPrint)) || (!printing && (flags & flagNoView))) {
        return false;
    }

    // check the OC
    OCGs *optContentConfig = doc->getCatalog()->getOptContentConfig();
    if (optContentConfig) {
        if (!optContentConfig->optContentIsVisible(&oc))
            return false;
    }

    return true;
}

int Annot::getRotation() const
{
    Page *pageobj = doc->getPage(page);
    assert(pageobj != nullptr);

    if (flags & flagNoRotate) {
        return (360 - pageobj->getRotate()) % 360;
    } else {
        return 0;
    }
}

void Annot::draw(Gfx *gfx, bool printing)
{
    annotLocker();
    if (!isVisible(printing))
        return;

    // draw the appearance stream
    Object obj = appearance.fetch(gfx->getXRef());
    gfx->drawAnnot(&obj, nullptr, color.get(), rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
}

//------------------------------------------------------------------------
// AnnotPopup
//------------------------------------------------------------------------

AnnotPopup::AnnotPopup(PDFDoc *docA, PDFRectangle *rectA) : Annot(docA, rectA)
{
    type = typePopup;

    annotObj.dictSet("Subtype", Object(objName, "Popup"));
    initialize(docA, annotObj.getDict());
}

AnnotPopup::AnnotPopup(PDFDoc *docA, Object &&dictObject, const Object *obj) : Annot(docA, std::move(dictObject), obj)
{
    type = typePopup;
    initialize(docA, annotObj.getDict());
}

AnnotPopup::~AnnotPopup() { }

void AnnotPopup::initialize(PDFDoc *docA, Dict *dict)
{
    const Object &parentObj = dict->lookupNF("Parent");
    if (parentObj.isRef()) {
        parentRef = parentObj.getRef();
    } else {
        parentRef = Ref::INVALID();
    }

    open = dict->lookup("Open").getBoolWithDefaultValue(false);
}

void AnnotPopup::setParent(Annot *parentA)
{
    parentRef = parentA->getRef();
    update("Parent", Object(parentRef));
}

void AnnotPopup::setOpen(bool openA)
{
    open = openA;
    update("Open", Object(open));
}

//------------------------------------------------------------------------
// AnnotMarkup
//------------------------------------------------------------------------
AnnotMarkup::AnnotMarkup(PDFDoc *docA, PDFRectangle *rectA) : Annot(docA, rectA)
{
    initialize(docA, annotObj.getDict());
}

AnnotMarkup::AnnotMarkup(PDFDoc *docA, Object &&dictObject, const Object *obj) : Annot(docA, std::move(dictObject), obj)
{
    initialize(docA, annotObj.getDict());
}

AnnotMarkup::~AnnotMarkup() = default;

void AnnotMarkup::initialize(PDFDoc *docA, Dict *dict)
{
    Object obj1;

    obj1 = dict->lookup("T");
    if (obj1.isString()) {
        label.reset(obj1.getString()->copy());
    }

    Object popupObj = dict->lookup("Popup");
    const Object &obj2 = dict->lookupNF("Popup");
    if (popupObj.isDict() && obj2.isRef()) {
        popup = std::make_unique<AnnotPopup>(docA, std::move(popupObj), &obj2);
    }

    opacity = dict->lookup("CA").getNumWithDefaultValue(1.0);

    obj1 = dict->lookup("CreationDate");
    if (obj1.isString()) {
        date.reset(obj1.getString()->copy());
    }

    const Object &irtObj = dict->lookupNF("IRT");
    if (irtObj.isRef()) {
        inReplyTo = irtObj.getRef();
    } else {
        inReplyTo = Ref::INVALID();
    }

    obj1 = dict->lookup("Subj");
    if (obj1.isString()) {
        subject.reset(obj1.getString()->copy());
    }

    obj1 = dict->lookup("RT");
    if (obj1.isName()) {
        const char *replyName = obj1.getName();

        if (!strcmp(replyName, "R")) {
            replyTo = replyTypeR;
        } else if (!strcmp(replyName, "Group")) {
            replyTo = replyTypeGroup;
        } else {
            replyTo = replyTypeR;
        }
    } else {
        replyTo = replyTypeR;
    }

    obj1 = dict->lookup("ExData");
    if (obj1.isDict()) {
        exData = parseAnnotExternalData(obj1.getDict());
    } else {
        exData = annotExternalDataMarkupUnknown;
    }
}

void AnnotMarkup::setLabel(GooString *new_label)
{
    if (new_label) {
        label = std::make_unique<GooString>(new_label);
        // append the unicode marker <FE FF> if needed
        if (!label->hasUnicodeMarker()) {
            label->prependUnicodeMarker();
        }
    } else {
        label = std::make_unique<GooString>();
    }

    update("T", Object(label->copy()));
}

void AnnotMarkup::setPopup(std::unique_ptr<AnnotPopup> &&new_popup)
{
    // If there exists an old popup annotation that is already
    // associated with a page, then we need to remove that
    // popup annotation from the page. Otherwise we would have
    // dangling references to it.
    if (popup && popup->getPageNum() != 0) {
        Page *pageobj = doc->getPage(popup->getPageNum());
        if (pageobj) {
            pageobj->removeAnnot(popup.get());
        }
    }

    if (new_popup) {
        const Ref popupRef = new_popup->getRef();
        update("Popup", Object(popupRef));

        new_popup->setParent(this);
        popup = std::move(new_popup);

        // If this annotation is already added to a page, then we
        // add the new popup annotation to the same page.
        if (page != 0) {
            Page *pageobj = doc->getPage(page);
            assert(pageobj != nullptr); // pageobj should exist in doc (see setPage())

            pageobj->addAnnot(popup.get());
        }
    } else {
        popup = nullptr;
    }
}

void AnnotMarkup::setOpacity(double opacityA)
{
    opacity = opacityA;
    update("CA", Object(opacity));
    invalidateAppearance();
}

void AnnotMarkup::setDate(GooString *new_date)
{
    if (new_date) {
        date = std::make_unique<GooString>(new_date);
        update("CreationDate", Object(date->copy()));
    } else {
        date.reset(nullptr);
        update("CreationDate", Object(objNull));
    }
}

void AnnotMarkup::removeReferencedObjects()
{
    Page *pageobj = doc->getPage(page);
    assert(pageobj != nullptr); // We're called when removing an annot from a page

    // Remove popup
    if (popup) {
        pageobj->removeAnnot(popup.get());
    }

    Annot::removeReferencedObjects();
}

//------------------------------------------------------------------------
// AnnotText
//------------------------------------------------------------------------

AnnotText::AnnotText(PDFDoc *docA, PDFRectangle *rectA) : AnnotMarkup(docA, rectA)
{
    type = typeText;
    flags |= flagNoZoom | flagNoRotate;

    annotObj.dictSet("Subtype", Object(objName, "Text"));
    initialize(docA, annotObj.getDict());
}

AnnotText::AnnotText(PDFDoc *docA, Object &&dictObject, const Object *obj) : AnnotMarkup(docA, std::move(dictObject), obj)
{

    type = typeText;
    flags |= flagNoZoom | flagNoRotate;
    initialize(docA, annotObj.getDict());
}

AnnotText::~AnnotText() = default;

void AnnotText::initialize(PDFDoc *docA, Dict *dict)
{
    Object obj1;

    open = dict->lookup("Open").getBoolWithDefaultValue(false);

    obj1 = dict->lookup("Name");
    if (obj1.isName()) {
        icon = std::make_unique<GooString>(obj1.getName());
    } else {
        icon = std::make_unique<GooString>("Note");
    }

    obj1 = dict->lookup("StateModel");
    if (obj1.isString()) {
        const GooString *modelName = obj1.getString();

        Object obj2 = dict->lookup("State");
        if (obj2.isString()) {
            const GooString *stateName = obj2.getString();

            if (!stateName->cmp("Marked")) {
                state = stateMarked;
            } else if (!stateName->cmp("Unmarked")) {
                state = stateUnmarked;
            } else if (!stateName->cmp("Accepted")) {
                state = stateAccepted;
            } else if (!stateName->cmp("Rejected")) {
                state = stateRejected;
            } else if (!stateName->cmp("Cancelled")) {
                state = stateCancelled;
            } else if (!stateName->cmp("Completed")) {
                state = stateCompleted;
            } else if (!stateName->cmp("None")) {
                state = stateNone;
            } else {
                state = stateUnknown;
            }
        } else {
            state = stateUnknown;
        }

        if (!modelName->cmp("Marked")) {
            switch (state) {
            case stateUnknown:
                state = stateMarked;
                break;
            case stateAccepted:
            case stateRejected:
            case stateCancelled:
            case stateCompleted:
            case stateNone:
                state = stateUnknown;
                break;
            default:
                break;
            }
        } else if (!modelName->cmp("Review")) {
            switch (state) {
            case stateUnknown:
                state = stateNone;
                break;
            case stateMarked:
            case stateUnmarked:
                state = stateUnknown;
                break;
            default:
                break;
            }
        } else {
            state = stateUnknown;
        }
    } else {
        state = stateUnknown;
    }
}

void AnnotText::setOpen(bool openA)
{
    open = openA;
    update("Open", Object(open));
}

void AnnotText::setIcon(GooString *new_icon)
{
    if (new_icon && icon->cmp(new_icon) == 0)
        return;

    if (new_icon) {
        icon = std::make_unique<GooString>(new_icon);
    } else {
        icon = std::make_unique<GooString>("Note");
    }

    update("Name", Object(objName, icon->c_str()));
    invalidateAppearance();
}

#define ANNOT_TEXT_AP_NOTE                                                                                                                                                                                                                     \
    "3.602 24 m 20.398 24 l 22.387 24 24 22.387 24 20.398 c 24 3.602 l 24\n"                                                                                                                                                                   \
    "1.613 22.387 0 20.398 0 c 3.602 0 l 1.613 0 0 1.613 0 3.602 c 0 20.398\n"                                                                                                                                                                 \
    "l 0 22.387 1.613 24 3.602 24 c h\n"                                                                                                                                                                                                       \
    "3.602 24 m f\n"                                                                                                                                                                                                                           \
    "0.533333 0.541176 0.521569 RG 2 w\n"                                                                                                                                                                                                      \
    "1 J\n"                                                                                                                                                                                                                                    \
    "1 j\n"                                                                                                                                                                                                                                    \
    "[] 0.0 d\n"                                                                                                                                                                                                                               \
    "4 M 9 18 m 4 18 l 4 7 4 4 6 3 c 20 3 l 18 4 18 7 18 18 c 17 18 l S\n"                                                                                                                                                                     \
    "1.5 w\n"                                                                                                                                                                                                                                  \
    "0 j\n"                                                                                                                                                                                                                                    \
    "10 16 m 14 21 l S\n"                                                                                                                                                                                                                      \
    "1.85625 w\n"                                                                                                                                                                                                                              \
    "1 j\n"                                                                                                                                                                                                                                    \
    "15.07 20.523 m 15.07 19.672 14.379 18.977 13.523 18.977 c 12.672 18.977\n"                                                                                                                                                                \
    "11.977 19.672 11.977 20.523 c 11.977 21.379 12.672 22.07 13.523 22.07 c\n"                                                                                                                                                                \
    "14.379 22.07 15.07 21.379 15.07 20.523 c h\n"                                                                                                                                                                                             \
    "15.07 20.523 m S\n"                                                                                                                                                                                                                       \
    "1 w\n"                                                                                                                                                                                                                                    \
    "0 j\n"                                                                                                                                                                                                                                    \
    "6.5 13.5 m 15.5 13.5 l S\n"                                                                                                                                                                                                               \
    "6.5 10.5 m 13.5 10.5 l S\n"                                                                                                                                                                                                               \
    "6.801 7.5 m 15.5 7.5 l S\n"                                                                                                                                                                                                               \
    "0.729412 0.741176 0.713725 RG 2 w\n"                                                                                                                                                                                                      \
    "1 j\n"                                                                                                                                                                                                                                    \
    "9 19 m 4 19 l 4 8 4 5 6 4 c 20 4 l 18 5 18 8 18 19 c 17 19 l S\n"                                                                                                                                                                         \
    "1.5 w\n"                                                                                                                                                                                                                                  \
    "0 j\n"                                                                                                                                                                                                                                    \
    "10 17 m 14 22 l S\n"                                                                                                                                                                                                                      \
    "1.85625 w\n"                                                                                                                                                                                                                              \
    "1 j\n"                                                                                                                                                                                                                                    \
    "15.07 21.523 m 15.07 20.672 14.379 19.977 13.523 19.977 c 12.672 19.977\n"                                                                                                                                                                \
    "11.977 20.672 11.977 21.523 c 11.977 22.379 12.672 23.07 13.523 23.07 c\n"                                                                                                                                                                \
    "14.379 23.07 15.07 22.379 15.07 21.523 c h\n"                                                                                                                                                                                             \
    "15.07 21.523 m S\n"                                                                                                                                                                                                                       \
    "1 w\n"                                                                                                                                                                                                                                    \
    "0 j\n"                                                                                                                                                                                                                                    \
    "6.5 14.5 m 15.5 14.5 l S\n"                                                                                                                                                                                                               \
    "6.5 11.5 m 13.5 11.5 l S\n"                                                                                                                                                                                                               \
    "6.801 8.5 m 15.5 8.5 l S\n"

#define ANNOT_TEXT_AP_COMMENT                                                                                                                                                                                                                  \
    "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"                                                                                                                                                                   \
    "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"                                                                                                                                                                 \
    "l 1 21.523 2.477 23 4.301 23 c h\n"                                                                                                                                                                                                       \
    "4.301 23 m f\n"                                                                                                                                                                                                                           \
    "0.533333 0.541176 0.521569 RG 2 w\n"                                                                                                                                                                                                      \
    "0 J\n"                                                                                                                                                                                                                                    \
    "1 j\n"                                                                                                                                                                                                                                    \
    "[] 0.0 d\n"                                                                                                                                                                                                                               \
    "4 M 8 20 m 16 20 l 18.363 20 20 18.215 20 16 c 20 13 l 20 10.785 18.363 9\n"                                                                                                                                                              \
    "16 9 c 13 9 l 8 3 l 8 9 l 8 9 l 5.637 9 4 10.785 4 13 c 4 16 l 4 18.215\n"                                                                                                                                                                \
    "5.637 20 8 20 c h\n"                                                                                                                                                                                                                      \
    "8 20 m S\n"                                                                                                                                                                                                                               \
    "0.729412 0.741176 0.713725 RG 8 21 m 16 21 l 18.363 21 20 19.215 20 17\n"                                                                                                                                                                 \
    "c 20 14 l 20 11.785 18.363 10\n"                                                                                                                                                                                                          \
    "16 10 c 13 10 l 8 4 l 8 10 l 8 10 l 5.637 10 4 11.785 4 14 c 4 17 l 4\n"                                                                                                                                                                  \
    "19.215 5.637 21 8 21 c h\n"                                                                                                                                                                                                               \
    "8 21 m S\n"

#define ANNOT_TEXT_AP_KEY                                                                                                                                                                                                                      \
    "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"                                                                                                                                                                   \
    "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"                                                                                                                                                                 \
    "l 1 21.523 2.477 23 4.301 23 c h\n"                                                                                                                                                                                                       \
    "4.301 23 m f\n"                                                                                                                                                                                                                           \
    "0.533333 0.541176 0.521569 RG 2 w\n"                                                                                                                                                                                                      \
    "1 J\n"                                                                                                                                                                                                                                    \
    "0 j\n"                                                                                                                                                                                                                                    \
    "[] 0.0 d\n"                                                                                                                                                                                                                               \
    "4 M 11.895 18.754 m 13.926 20.625 17.09 20.496 18.961 18.465 c 20.832\n"                                                                                                                                                                  \
    "16.434 20.699 13.27 18.668 11.398 c 17.164 10.016 15.043 9.746 13.281\n"                                                                                                                                                                  \
    "10.516 c 12.473 9.324 l 11.281 10.078 l 9.547 8.664 l 9.008 6.496 l\n"                                                                                                                                                                    \
    "7.059 6.059 l 6.34 4.121 l 5.543 3.668 l 3.375 4.207 l 2.938 6.156 l\n"                                                                                                                                                                   \
    "10.57 13.457 l 9.949 15.277 10.391 17.367 11.895 18.754 c h\n"                                                                                                                                                                            \
    "11.895 18.754 m S\n"                                                                                                                                                                                                                      \
    "1.5 w\n"                                                                                                                                                                                                                                  \
    "16.059 15.586 m 16.523 15.078 17.316 15.043 17.824 15.512 c 18.332\n"                                                                                                                                                                     \
    "15.98 18.363 16.77 17.895 17.277 c 17.43 17.785 16.637 17.816 16.129\n"                                                                                                                                                                   \
    "17.352 c 15.621 16.883 15.59 16.094 16.059 15.586 c h\n"                                                                                                                                                                                  \
    "16.059 15.586 m S\n"                                                                                                                                                                                                                      \
    "0.729412 0.741176 0.713725 RG 2 w\n"                                                                                                                                                                                                      \
    "11.895 19.754 m 13.926 21.625 17.09 21.496 18.961 19.465 c 20.832\n"                                                                                                                                                                      \
    "17.434 20.699 14.27 18.668 12.398 c 17.164 11.016 15.043 10.746 13.281\n"                                                                                                                                                                 \
    "11.516 c 12.473 10.324 l 11.281 11.078 l 9.547 9.664 l 9.008 7.496 l\n"                                                                                                                                                                   \
    "7.059 7.059 l 6.34 5.121 l 5.543 4.668 l 3.375 5.207 l 2.938 7.156 l\n"                                                                                                                                                                   \
    "10.57 14.457 l 9.949 16.277 10.391 18.367 11.895 19.754 c h\n"                                                                                                                                                                            \
    "11.895 19.754 m S\n"                                                                                                                                                                                                                      \
    "1.5 w\n"                                                                                                                                                                                                                                  \
    "16.059 16.586 m 16.523 16.078 17.316 16.043 17.824 16.512 c 18.332\n"                                                                                                                                                                     \
    "16.98 18.363 17.77 17.895 18.277 c 17.43 18.785 16.637 18.816 16.129\n"                                                                                                                                                                   \
    "18.352 c 15.621 17.883 15.59 17.094 16.059 16.586 c h\n"                                                                                                                                                                                  \
    "16.059 16.586 m S\n"

#define ANNOT_TEXT_AP_HELP                                                                                                                                                                                                                     \
    "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"                                                                                                                                                                   \
    "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"                                                                                                                                                                 \
    "l 1 21.523 2.477 23 4.301 23 c h\n"                                                                                                                                                                                                       \
    "4.301 23 m f\n"                                                                                                                                                                                                                           \
    "0.533333 0.541176 0.521569 RG 2.5 w\n"                                                                                                                                                                                                    \
    "1 J\n"                                                                                                                                                                                                                                    \
    "1 j\n"                                                                                                                                                                                                                                    \
    "[] 0.0 d\n"                                                                                                                                                                                                                               \
    "4 M 8.289 16.488 m 8.824 17.828 10.043 18.773 11.473 18.965 c 12.902 19.156\n"                                                                                                                                                            \
    "14.328 18.559 15.195 17.406 c 16.062 16.254 16.242 14.723 15.664 13.398\n"                                                                                                                                                                \
    "c S\n"                                                                                                                                                                                                                                    \
    "0 j\n"                                                                                                                                                                                                                                    \
    "12 8 m 12 12 16 11 16 15 c S\n"                                                                                                                                                                                                           \
    "1.539286 w\n"                                                                                                                                                                                                                             \
    "1 j\n"                                                                                                                                                                                                                                    \
    "q 1 0 0 -0.999991 0 24 cm\n"                                                                                                                                                                                                              \
    "12.684 20.891 m 12.473 21.258 12.004 21.395 11.629 21.196 c 11.254\n"                                                                                                                                                                     \
    "20.992 11.105 20.531 11.297 20.149 c 11.488 19.77 11.945 19.61 12.332\n"                                                                                                                                                                  \
    "19.789 c 12.719 19.969 12.891 20.426 12.719 20.817 c S Q\n"                                                                                                                                                                               \
    "0.729412 0.741176 0.713725 RG 2.5 w\n"                                                                                                                                                                                                    \
    "8.289 17.488 m 9.109 19.539 11.438 20.535 13.488 19.711 c 15.539 18.891\n"                                                                                                                                                                \
    "16.535 16.562 15.711 14.512 c 15.699 14.473 15.684 14.438 15.664 14.398\n"                                                                                                                                                                \
    "c S\n"                                                                                                                                                                                                                                    \
    "0 j\n"                                                                                                                                                                                                                                    \
    "12 9 m 12 13 16 12 16 16 c S\n"                                                                                                                                                                                                           \
    "1.539286 w\n"                                                                                                                                                                                                                             \
    "1 j\n"                                                                                                                                                                                                                                    \
    "q 1 0 0 -0.999991 0 24 cm\n"                                                                                                                                                                                                              \
    "12.684 19.891 m 12.473 20.258 12.004 20.395 11.629 20.195 c 11.254\n"                                                                                                                                                                     \
    "19.992 11.105 19.531 11.297 19.149 c 11.488 18.77 11.945 18.61 12.332\n"                                                                                                                                                                  \
    "18.789 c 12.719 18.969 12.891 19.426 12.719 19.817 c S Q\n"

#define ANNOT_TEXT_AP_NEW_PARAGRAPH                                                                                                                                                                                                            \
    "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"                                                                                                                                                                   \
    "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"                                                                                                                                                                 \
    "l 1 21.523 2.477 23 4.301 23 c h\n"                                                                                                                                                                                                       \
    "4.301 23 m f\n"                                                                                                                                                                                                                           \
    "0.533333 0.541176 0.521569 RG 4 w\n"                                                                                                                                                                                                      \
    "0 J\n"                                                                                                                                                                                                                                    \
    "2 j\n"                                                                                                                                                                                                                                    \
    "[] 0.0 d\n"                                                                                                                                                                                                                               \
    "4 M q 1 0 0 -1 0 24 cm\n"                                                                                                                                                                                                                 \
    "9.211 11.988 m 8.449 12.07 7.711 11.707 7.305 11.059 c 6.898 10.41\n"                                                                                                                                                                     \
    "6.898 9.59 7.305 8.941 c 7.711 8.293 8.449 7.93 9.211 8.012 c S Q\n"                                                                                                                                                                      \
    "1.004413 w\n"                                                                                                                                                                                                                             \
    "1 J\n"                                                                                                                                                                                                                                    \
    "1 j\n"                                                                                                                                                                                                                                    \
    "q 1 0 0 -0.991232 0 24 cm\n"                                                                                                                                                                                                              \
    "18.07 11.511 m 15.113 10.014 l 12.199 11.602 l 12.711 8.323 l 10.301\n"                                                                                                                                                                   \
    "6.045 l 13.574 5.517 l 14.996 2.522 l 16.512 5.474 l 19.801 5.899 l\n"                                                                                                                                                                    \
    "17.461 8.252 l 18.07 11.511 l h\n"                                                                                                                                                                                                        \
    "18.07 11.511 m S Q\n"                                                                                                                                                                                                                     \
    "2 w\n"                                                                                                                                                                                                                                    \
    "0 j\n"                                                                                                                                                                                                                                    \
    "11 17 m 10 17 l 10 3 l S\n"                                                                                                                                                                                                               \
    "14 3 m 14 13 l S\n"                                                                                                                                                                                                                       \
    "0.729412 0.741176 0.713725 RG 4 w\n"                                                                                                                                                                                                      \
    "0 J\n"                                                                                                                                                                                                                                    \
    "2 j\n"                                                                                                                                                                                                                                    \
    "q 1 0 0 -1 0 24 cm\n"                                                                                                                                                                                                                     \
    "9.211 10.988 m 8.109 11.105 7.125 10.309 7.012 9.211 c 6.895 8.109\n"                                                                                                                                                                     \
    "7.691 7.125 8.789 7.012 c 8.93 6.996 9.07 6.996 9.211 7.012 c S Q\n"                                                                                                                                                                      \
    "1.004413 w\n"                                                                                                                                                                                                                             \
    "1 J\n"                                                                                                                                                                                                                                    \
    "1 j\n"                                                                                                                                                                                                                                    \
    "q 1 0 0 -0.991232 0 24 cm\n"                                                                                                                                                                                                              \
    "18.07 10.502 m 15.113 9.005 l 12.199 10.593 l 12.711 7.314 l 10.301\n"                                                                                                                                                                    \
    "5.036 l 13.574 4.508 l 14.996 1.513 l 16.512 4.465 l 19.801 4.891 l\n"                                                                                                                                                                    \
    "17.461 7.243 l 18.07 10.502 l h\n"                                                                                                                                                                                                        \
    "18.07 10.502 m S Q\n"                                                                                                                                                                                                                     \
    "2 w\n"                                                                                                                                                                                                                                    \
    "0 j\n"                                                                                                                                                                                                                                    \
    "11 18 m 10 18 l 10 4 l S\n"                                                                                                                                                                                                               \
    "14 4 m 14 14 l S\n"

#define ANNOT_TEXT_AP_PARAGRAPH                                                                                                                                                                                                                \
    "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"                                                                                                                                                                   \
    "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"                                                                                                                                                                 \
    "l 1 21.523 2.477 23 4.301 23 c h\n"                                                                                                                                                                                                       \
    "4.301 23 m f\n"                                                                                                                                                                                                                           \
    "0.533333 0.541176 0.521569 RG 2 w\n"                                                                                                                                                                                                      \
    "1 J\n"                                                                                                                                                                                                                                    \
    "1 j\n"                                                                                                                                                                                                                                    \
    "[] 0.0 d\n"                                                                                                                                                                                                                               \
    "4 M 15 3 m 15 18 l 11 18 l 11 3 l S\n"                                                                                                                                                                                                    \
    "4 w\n"                                                                                                                                                                                                                                    \
    "q 1 0 0 -1 0 24 cm\n"                                                                                                                                                                                                                     \
    "9.777 10.988 m 8.746 10.871 7.973 9.988 8 8.949 c 8.027 7.91 8.844\n"                                                                                                                                                                     \
    "7.066 9.879 7.004 c S Q\n"                                                                                                                                                                                                                \
    "0.729412 0.741176 0.713725 RG 2 w\n"                                                                                                                                                                                                      \
    "15 4 m 15 19 l 11 19 l 11 4 l S\n"                                                                                                                                                                                                        \
    "4 w\n"                                                                                                                                                                                                                                    \
    "q 1 0 0 -1 0 24 cm\n"                                                                                                                                                                                                                     \
    "9.777 9.988 m 8.746 9.871 7.973 8.988 8 7.949 c 8.027 6.91 8.844 6.066\n"                                                                                                                                                                 \
    "9.879 6.004 c S Q\n"

#define ANNOT_TEXT_AP_INSERT                                                                                                                                                                                                                   \
    "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"                                                                                                                                                                   \
    "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"                                                                                                                                                                 \
    "l 1 21.523 2.477 23 4.301 23 c h\n"                                                                                                                                                                                                       \
    "4.301 23 m f\n"                                                                                                                                                                                                                           \
    "0.533333 0.541176 0.521569 RG 2 w\n"                                                                                                                                                                                                      \
    "1 J\n"                                                                                                                                                                                                                                    \
    "0 j\n"                                                                                                                                                                                                                                    \
    "[] 0.0 d\n"                                                                                                                                                                                                                               \
    "4 M 12 18.012 m 20 18 l S\n"                                                                                                                                                                                                              \
    "9 10 m 17 10 l S\n"                                                                                                                                                                                                                       \
    "12 14.012 m 20 14 l S\n"                                                                                                                                                                                                                  \
    "12 6.012 m 20 6.012 l S\n"                                                                                                                                                                                                                \
    "4 12 m 6 10 l 4 8 l S\n"                                                                                                                                                                                                                  \
    "4 12 m 4 8 l S\n"                                                                                                                                                                                                                         \
    "0.729412 0.741176 0.713725 RG 12 19.012 m 20 19 l S\n"                                                                                                                                                                                    \
    "9 11 m 17 11 l S\n"                                                                                                                                                                                                                       \
    "12 15.012 m 20 15 l S\n"                                                                                                                                                                                                                  \
    "12 7.012 m 20 7.012 l S\n"                                                                                                                                                                                                                \
    "4 13 m 6 11 l 4 9 l S\n"                                                                                                                                                                                                                  \
    "4 13 m 4 9 l S\n"

#define ANNOT_TEXT_AP_CROSS                                                                                                                                                                                                                    \
    "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"                                                                                                                                                                   \
    "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"                                                                                                                                                                 \
    "l 1 21.523 2.477 23 4.301 23 c h\n"                                                                                                                                                                                                       \
    "4.301 23 m f\n"                                                                                                                                                                                                                           \
    "0.533333 0.541176 0.521569 RG 2.5 w\n"                                                                                                                                                                                                    \
    "1 J\n"                                                                                                                                                                                                                                    \
    "0 j\n"                                                                                                                                                                                                                                    \
    "[] 0.0 d\n"                                                                                                                                                                                                                               \
    "4 M 18 5 m 6 17 l S\n"                                                                                                                                                                                                                    \
    "6 5 m 18 17 l S\n"                                                                                                                                                                                                                        \
    "0.729412 0.741176 0.713725 RG 18 6 m 6 18 l S\n"                                                                                                                                                                                          \
    "6 6 m 18 18 l S\n"

#define ANNOT_TEXT_AP_CIRCLE                                                                                                                                                                                                                   \
    "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"                                                                                                                                                                   \
    "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"                                                                                                                                                                 \
    "l 1 21.523 2.477 23 4.301 23 c h\n"                                                                                                                                                                                                       \
    "4.301 23 m f\n"                                                                                                                                                                                                                           \
    "0.533333 0.541176 0.521569 RG 2.5 w\n"                                                                                                                                                                                                    \
    "1 J\n"                                                                                                                                                                                                                                    \
    "1 j\n"                                                                                                                                                                                                                                    \
    "[] 0.0 d\n"                                                                                                                                                                                                                               \
    "4 M 19.5 11.5 m 19.5 7.359 16.141 4 12 4 c 7.859 4 4.5 7.359 4.5 11.5 c 4.5\n"                                                                                                                                                            \
    "15.641 7.859 19 12 19 c 16.141 19 19.5 15.641 19.5 11.5 c h\n"                                                                                                                                                                            \
    "19.5 11.5 m S\n"                                                                                                                                                                                                                          \
    "0.729412 0.741176 0.713725 RG 19.5 12.5 m 19.5 8.359 16.141 5 12 5 c\n"                                                                                                                                                                   \
    "7.859 5 4.5 8.359 4.5 12.5 c 4.5\n"                                                                                                                                                                                                       \
    "16.641 7.859 20 12 20 c 16.141 20 19.5 16.641 19.5 12.5 c h\n"                                                                                                                                                                            \
    "19.5 12.5 m S\n"

void AnnotText::draw(Gfx *gfx, bool printing)
{
    double ca = 1;

    if (!isVisible(printing))
        return;

    annotLocker();
    if (appearance.isNull()) {
        ca = opacity;

        AnnotAppearanceBuilder appearBuilder;

        appearBuilder.append("q\n");
        if (color)
            appearBuilder.setDrawColor(color.get(), true);
        else
            appearBuilder.append("1 1 1 rg\n");
        if (!icon->cmp("Note"))
            appearBuilder.append(ANNOT_TEXT_AP_NOTE);
        else if (!icon->cmp("Comment"))
            appearBuilder.append(ANNOT_TEXT_AP_COMMENT);
        else if (!icon->cmp("Key"))
            appearBuilder.append(ANNOT_TEXT_AP_KEY);
        else if (!icon->cmp("Help"))
            appearBuilder.append(ANNOT_TEXT_AP_HELP);
        else if (!icon->cmp("NewParagraph"))
            appearBuilder.append(ANNOT_TEXT_AP_NEW_PARAGRAPH);
        else if (!icon->cmp("Paragraph"))
            appearBuilder.append(ANNOT_TEXT_AP_PARAGRAPH);
        else if (!icon->cmp("Insert"))
            appearBuilder.append(ANNOT_TEXT_AP_INSERT);
        else if (!icon->cmp("Cross"))
            appearBuilder.append(ANNOT_TEXT_AP_CROSS);
        else if (!icon->cmp("Circle"))
            appearBuilder.append(ANNOT_TEXT_AP_CIRCLE);
        appearBuilder.append("Q\n");

        // Force 24x24 rectangle
        PDFRectangle fixedRect(rect->x1, rect->y2 - 24, rect->x1 + 24, rect->y2);
        appearBBox = std::make_unique<AnnotAppearanceBBox>(&fixedRect);
        double bbox[4];
        appearBBox->getBBoxRect(bbox);
        if (ca == 1) {
            appearance = createForm(appearBuilder.buffer(), bbox, false, nullptr);
        } else {
            Object aStream = createForm(appearBuilder.buffer(), bbox, true, nullptr);

            GooString appearBuf("/GS0 gs\n/Fm0 Do");
            Dict *resDict = createResourcesDict("Fm0", std::move(aStream), "GS0", ca, nullptr);
            appearance = createForm(&appearBuf, bbox, false, resDict);
        }
    }

    // draw the appearance stream
    Object obj = appearance.fetch(gfx->getXRef());
    if (appearBBox) {
        gfx->drawAnnot(&obj, nullptr, color.get(), appearBBox->getPageXMin(), appearBBox->getPageYMin(), appearBBox->getPageXMax(), appearBBox->getPageYMax(), getRotation());
    } else {
        gfx->drawAnnot(&obj, nullptr, color.get(), rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
    }
}

//------------------------------------------------------------------------
// AnnotLink
//------------------------------------------------------------------------
AnnotLink::AnnotLink(PDFDoc *docA, PDFRectangle *rectA) : Annot(docA, rectA)
{
    type = typeLink;
    annotObj.dictSet("Subtype", Object(objName, "Link"));
    initialize(docA, annotObj.getDict());
}

AnnotLink::AnnotLink(PDFDoc *docA, Object &&dictObject, const Object *obj) : Annot(docA, std::move(dictObject), obj)
{

    type = typeLink;
    initialize(docA, annotObj.getDict());
}

AnnotLink::~AnnotLink() = default;

void AnnotLink::initialize(PDFDoc *docA, Dict *dict)
{
    Object obj1;

    // look for destination
    obj1 = dict->lookup("Dest");
    if (!obj1.isNull()) {
        action = LinkAction::parseDest(&obj1);
        // look for action
    } else {
        obj1 = dict->lookup("A");
        if (obj1.isDict()) {
            action = LinkAction::parseAction(&obj1, doc->getCatalog()->getBaseURI());
        }
    }

    obj1 = dict->lookup("H");
    if (obj1.isName()) {
        const char *effect = obj1.getName();

        if (!strcmp(effect, "N")) {
            linkEffect = effectNone;
        } else if (!strcmp(effect, "I")) {
            linkEffect = effectInvert;
        } else if (!strcmp(effect, "O")) {
            linkEffect = effectOutline;
        } else if (!strcmp(effect, "P")) {
            linkEffect = effectPush;
        } else {
            linkEffect = effectInvert;
        }
    } else {
        linkEffect = effectInvert;
    }
    /*
    obj1 = dict->lookup("PA");
    if (obj1.isDict()) {
      uriAction = NULL;
    } else {
      uriAction = NULL;
    }
    obj1.free();
    */
    obj1 = dict->lookup("QuadPoints");
    if (obj1.isArray()) {
        quadrilaterals = std::make_unique<AnnotQuadrilaterals>(obj1.getArray(), rect.get());
    }

    obj1 = dict->lookup("BS");
    if (obj1.isDict()) {
        border = std::make_unique<AnnotBorderBS>(obj1.getDict());
    } else if (!border) {
        border = std::make_unique<AnnotBorderBS>();
    }
}

void AnnotLink::draw(Gfx *gfx, bool printing)
{
    if (!isVisible(printing))
        return;

    annotLocker();
    // draw the appearance stream
    Object obj = appearance.fetch(gfx->getXRef());
    gfx->drawAnnot(&obj, border.get(), color.get(), rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
}

//------------------------------------------------------------------------
// AnnotFreeText
//------------------------------------------------------------------------
#if defined(MIKTEX)
const double AnnotFreeText::undefinedFontPtSize = 10.;
#else
const double AnnotFreeText::undefinedFontPtSize = 10.;
#endif

AnnotFreeText::AnnotFreeText(PDFDoc *docA, PDFRectangle *rectA, const DefaultAppearance &da) : AnnotMarkup(docA, rectA)
{
    type = typeFreeText;

    GooString *daStr = da.toAppearanceString();
    annotObj.dictSet("Subtype", Object(objName, "FreeText"));
    annotObj.dictSet("DA", Object(daStr));

    initialize(docA, annotObj.getDict());
}

AnnotFreeText::AnnotFreeText(PDFDoc *docA, Object &&dictObject, const Object *obj) : AnnotMarkup(docA, std::move(dictObject), obj)
{
    type = typeFreeText;
    initialize(docA, annotObj.getDict());
}

AnnotFreeText::~AnnotFreeText() = default;

void AnnotFreeText::initialize(PDFDoc *docA, Dict *dict)
{
    Object obj1;

    obj1 = dict->lookup("DA");
    if (obj1.isString()) {
        appearanceString.reset(obj1.getString()->copy());
    } else {
        appearanceString = std::make_unique<GooString>();
        error(errSyntaxWarning, -1, "Bad appearance for annotation");
    }

    obj1 = dict->lookup("Q");
    if (obj1.isInt()) {
        quadding = (AnnotFreeTextQuadding)obj1.getInt();
    } else {
        quadding = quaddingLeftJustified;
    }

    obj1 = dict->lookup("DS");
    if (obj1.isString()) {
        styleString.reset(obj1.getString()->copy());
    }

    obj1 = dict->lookup("CL");
    if (obj1.isArray() && obj1.arrayGetLength() >= 4) {
        const double x1 = obj1.arrayGet(0).getNumWithDefaultValue(0);
        const double y1 = obj1.arrayGet(1).getNumWithDefaultValue(0);
        const double x2 = obj1.arrayGet(2).getNumWithDefaultValue(0);
        const double y2 = obj1.arrayGet(3).getNumWithDefaultValue(0);

        if (obj1.arrayGetLength() == 6) {
            const double x3 = obj1.arrayGet(4).getNumWithDefaultValue(0);
            const double y3 = obj1.arrayGet(5).getNumWithDefaultValue(0);
            calloutLine = std::make_unique<AnnotCalloutMultiLine>(x1, y1, x2, y2, x3, y3);
        } else {
            calloutLine = std::make_unique<AnnotCalloutLine>(x1, y1, x2, y2);
        }
    }

    obj1 = dict->lookup("IT");
    if (obj1.isName()) {
        const char *intentName = obj1.getName();

        if (!strcmp(intentName, "FreeText")) {
            intent = intentFreeText;
        } else if (!strcmp(intentName, "FreeTextCallout")) {
            intent = intentFreeTextCallout;
        } else if (!strcmp(intentName, "FreeTextTypeWriter")) {
            intent = intentFreeTextTypeWriter;
        } else {
            intent = intentFreeText;
        }
    } else {
        intent = intentFreeText;
    }

    obj1 = dict->lookup("BS");
    if (obj1.isDict()) {
        border = std::make_unique<AnnotBorderBS>(obj1.getDict());
    } else if (!border) {
        border = std::make_unique<AnnotBorderBS>();
    }

    obj1 = dict->lookup("BE");
    if (obj1.isDict()) {
        borderEffect = std::make_unique<AnnotBorderEffect>(obj1.getDict());
    }

    obj1 = dict->lookup("RD");
    if (obj1.isArray()) {
        rectangle = parseDiffRectangle(obj1.getArray(), rect.get());
    }

    obj1 = dict->lookup("LE");
    if (obj1.isName()) {
        GooString styleName(obj1.getName());
        endStyle = parseAnnotLineEndingStyle(&styleName);
    } else {
        endStyle = annotLineEndingNone;
    }
}

void AnnotFreeText::setContents(GooString *new_content)
{
    Annot::setContents(new_content);
    invalidateAppearance();
}

void AnnotFreeText::setDefaultAppearance(const DefaultAppearance &da)
{
    appearanceString = std::unique_ptr<GooString>(da.toAppearanceString());

    update("DA", Object(appearanceString->copy()));
    invalidateAppearance();
}

void AnnotFreeText::setQuadding(AnnotFreeTextQuadding new_quadding)
{
    quadding = new_quadding;
    update("Q", Object((int)quadding));
    invalidateAppearance();
}

void AnnotFreeText::setStyleString(GooString *new_string)
{
    if (new_string) {
        styleString = std::make_unique<GooString>(new_string);
        // append the unicode marker <FE FF> if needed
        if (!styleString->hasUnicodeMarker()) {
            styleString->prependUnicodeMarker();
        }
    } else {
        styleString = std::make_unique<GooString>();
    }

    update("DS", Object(styleString->copy()));
}

void AnnotFreeText::setCalloutLine(AnnotCalloutLine *line)
{
    Object obj1;
    if (line == nullptr) {
        obj1.setToNull();
        calloutLine = nullptr;
    } else {
        double x1 = line->getX1(), y1 = line->getY1();
        double x2 = line->getX2(), y2 = line->getY2();
        obj1 = Object(new Array(doc->getXRef()));
        obj1.arrayAdd(Object(x1));
        obj1.arrayAdd(Object(y1));
        obj1.arrayAdd(Object(x2));
        obj1.arrayAdd(Object(y2));

        AnnotCalloutMultiLine *mline = dynamic_cast<AnnotCalloutMultiLine *>(line);
        if (mline) {
            double x3 = mline->getX3(), y3 = mline->getY3();
            obj1.arrayAdd(Object(x3));
            obj1.arrayAdd(Object(y3));
            calloutLine = std::make_unique<AnnotCalloutMultiLine>(x1, y1, x2, y2, x3, y3);
        } else {
            calloutLine = std::make_unique<AnnotCalloutLine>(x1, y1, x2, y2);
        }
    }

    update("CL", std::move(obj1));
    invalidateAppearance();
}

void AnnotFreeText::setIntent(AnnotFreeTextIntent new_intent)
{
    const char *intentName;

    intent = new_intent;
    if (new_intent == intentFreeText)
        intentName = "FreeText";
    else if (new_intent == intentFreeTextCallout)
        intentName = "FreeTextCallout";
    else // intentFreeTextTypeWriter
        intentName = "FreeTextTypeWriter";
    update("IT", Object(objName, intentName));
}

std::unique_ptr<DefaultAppearance> AnnotFreeText::getDefaultAppearance() const
{
    return std::make_unique<DefaultAppearance>(appearanceString.get());
}

static GfxFont *createAnnotDrawFont(XRef *xref, Dict *fontResDict, const char *resourceName = "AnnotDrawFont", const char *fontname = "Helvetica")
{
    const Ref dummyRef = { -1, -1 };

    Dict *fontDict = new Dict(xref);
    fontDict->add("BaseFont", Object(objName, fontname));
    fontDict->add("Subtype", Object(objName, "Type0"));
    fontDict->add("Encoding", Object(objName, "WinAnsiEncoding"));

    Dict *fontsDict = new Dict(xref);
    fontsDict->add(resourceName, Object(fontDict));

    fontResDict->add("Font", Object(fontsDict));

    return GfxFont::makeFont(xref, resourceName, dummyRef, fontDict);
}

void AnnotFreeText::generateFreeTextAppearance()
{
    double borderWidth, ca = opacity;

    AnnotAppearanceBuilder appearBuilder;
    appearBuilder.append("q\n");

    borderWidth = border->getWidth();
    if (borderWidth > 0)
        appearBuilder.setLineStyleForBorder(border.get());

    // Box size
    const double width = rect->x2 - rect->x1;
    const double height = rect->y2 - rect->y1;

    // Parse some properties from the appearance string
    DefaultAppearance da { appearanceString.get() };

    // Default values
    if (!da.getFontName().isName())
        da.setFontName(Object(objName, "AnnotDrawFont"));
    if (da.getFontPtSize() <= 0)
        da.setFontPtSize(undefinedFontPtSize);
    if (!da.getFontColor())
        da.setFontColor(std::make_unique<AnnotColor>(0, 0, 0));
    if (!contents)
        contents = std::make_unique<GooString>();

    // Draw box
    bool doFill = (color && color->getSpace() != AnnotColor::colorTransparent);
    bool doStroke = (borderWidth != 0);
    if (doFill || doStroke) {
        if (doStroke) {
            appearBuilder.setDrawColor(da.getFontColor(), false); // Border color: same as font color
        }
        appearBuilder.appendf("{0:.2f} {0:.2f} {1:.2f} {2:.2f} re\n", borderWidth / 2, width - borderWidth, height - borderWidth);
        if (doFill) {
            appearBuilder.setDrawColor(color.get(), true);
            appearBuilder.append(doStroke ? "B\n" : "f\n");
        } else {
            appearBuilder.append("S\n");
        }
    }

    // Setup text clipping
    const double textmargin = borderWidth * 2;
    const double textwidth = width - 2 * textmargin;
    appearBuilder.appendf("{0:.2f} {0:.2f} {1:.2f} {2:.2f} re W n\n", textmargin, textwidth, height - 2 * textmargin);

    GfxFont *font = nullptr;

    // look for font name in the default resources
    Form *form = doc->getCatalog()->getForm(); // form is owned by catalog, no need to clean it up

    Object resourceObj;
    if (form && form->getDefaultResourcesObj() && form->getDefaultResourcesObj()->isDict()) {
        resourceObj = form->getDefaultResourcesObj()->copy(); // No real copy, but increment refcount of /DR Dict

        Dict *resDict = resourceObj.getDict();
        Object fontResources = resDict->lookup("Font"); // The 'Font' subdictionary

        if (!fontResources.isDict()) {
            error(errSyntaxWarning, -1, "Font subdictionary is not a dictionary");
        } else {
            // Get the font dictionary for the actual requested font
            Ref fontReference;
            Object fontDictionary = fontResources.getDict()->lookup(da.getFontName().getName(), &fontReference);

            if (fontDictionary.isDict()) {
                font = GfxFont::makeFont(doc->getXRef(), da.getFontName().getName(), fontReference, fontDictionary.getDict());
            } else {
                error(errSyntaxWarning, -1, "Font dictionary is not a dictionary");
            }
        }
    }

    // if fontname is not in the default resources, create a Helvetica fake font
    if (!font) {
        Dict *fontResDict = new Dict(doc->getXRef());
        resourceObj = Object(fontResDict);
        font = createAnnotDrawFont(doc->getXRef(), fontResDict, da.getFontName().getName());
    }

    // Set font state
    appearBuilder.setDrawColor(da.getFontColor(), true);
    appearBuilder.appendf("BT 1 0 0 1 {0:.2f} {1:.2f} Tm\n", textmargin, height - textmargin - da.getFontPtSize() * font->getDescent());
    appearBuilder.setTextFont(da.getFontName(), da.getFontPtSize());

    int i = 0;
    double xposPrev = 0;
    while (i < contents->getLength()) {
        GooString out;
        double linewidth, xpos;
        layoutText(contents.get(), &out, &i, font, &linewidth, textwidth / da.getFontPtSize(), nullptr, false);
        linewidth *= da.getFontPtSize();
        switch (quadding) {
        case quaddingCentered:
            xpos = (textwidth - linewidth) / 2;
            break;
        case quaddingRightJustified:
            xpos = textwidth - linewidth;
            break;
        default: // quaddingLeftJustified:
            xpos = 0;
            break;
        }
        appearBuilder.appendf("{0:.2f} {1:.2f} Td\n", xpos - xposPrev, -da.getFontPtSize());
        appearBuilder.writeString(out);
        appearBuilder.append("Tj\n");
        xposPrev = xpos;
    }

    font->decRefCnt();
    appearBuilder.append("ET Q\n");

    double bbox[4];
    bbox[0] = bbox[1] = 0;
    bbox[2] = rect->x2 - rect->x1;
    bbox[3] = rect->y2 - rect->y1;

    if (ca == 1) {
        appearance = createForm(appearBuilder.buffer(), bbox, false, std::move(resourceObj));
    } else {
        Object aStream = createForm(appearBuilder.buffer(), bbox, true, std::move(resourceObj));

        GooString appearBuf("/GS0 gs\n/Fm0 Do");
        Dict *resDict = createResourcesDict("Fm0", std::move(aStream), "GS0", ca, nullptr);
        appearance = createForm(&appearBuf, bbox, false, resDict);
    }
}

void AnnotFreeText::draw(Gfx *gfx, bool printing)
{
    if (!isVisible(printing))
        return;

    annotLocker();
    if (appearance.isNull()) {
        generateFreeTextAppearance();
    }

    // draw the appearance stream
    Object obj = appearance.fetch(gfx->getXRef());
    gfx->drawAnnot(&obj, nullptr, color.get(), rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
}

// Before retrieving the res dict, regenerate the appearance stream if needed,
// because AnnotFreeText::draw needs to store font info in the res dict
Object AnnotFreeText::getAppearanceResDict()
{
    if (appearance.isNull()) {
        generateFreeTextAppearance();
    }
    return Annot::getAppearanceResDict();
}

//------------------------------------------------------------------------
// AnnotLine
//------------------------------------------------------------------------

AnnotLine::AnnotLine(PDFDoc *docA, PDFRectangle *rectA) : AnnotMarkup(docA, rectA)
{
    type = typeLine;
    annotObj.dictSet("Subtype", Object(objName, "Line"));

    initialize(docA, annotObj.getDict());
}

AnnotLine::AnnotLine(PDFDoc *docA, Object &&dictObject, const Object *obj) : AnnotMarkup(docA, std::move(dictObject), obj)
{
    type = typeLine;
    initialize(docA, annotObj.getDict());
}

AnnotLine::~AnnotLine() = default;

void AnnotLine::initialize(PDFDoc *docA, Dict *dict)
{
    Object obj1;

    obj1 = dict->lookup("L");
    if (obj1.isArray() && obj1.arrayGetLength() == 4) {
        const double x1 = obj1.arrayGet(0).getNumWithDefaultValue(0);
        const double y1 = obj1.arrayGet(1).getNumWithDefaultValue(0);
        const double x2 = obj1.arrayGet(2).getNumWithDefaultValue(0);
        const double y2 = obj1.arrayGet(3).getNumWithDefaultValue(0);

        coord1 = std::make_unique<AnnotCoord>(x1, y1);
        coord2 = std::make_unique<AnnotCoord>(x2, y2);
    } else {
        coord1 = std::make_unique<AnnotCoord>();
        coord2 = std::make_unique<AnnotCoord>();
    }

    obj1 = dict->lookup("LE");
    if (obj1.isArray() && obj1.arrayGetLength() == 2) {
        Object obj2;

        obj2 = obj1.arrayGet(0);
        if (obj2.isName()) {
            GooString leName(obj2.getName());
            startStyle = parseAnnotLineEndingStyle(&leName);
        } else {
            startStyle = annotLineEndingNone;
        }

        obj2 = obj1.arrayGet(1);
        if (obj2.isName()) {
            GooString leName(obj2.getName());
            endStyle = parseAnnotLineEndingStyle(&leName);
        } else {
            endStyle = annotLineEndingNone;
        }

    } else {
        startStyle = endStyle = annotLineEndingNone;
    }

    obj1 = dict->lookup("IC");
    if (obj1.isArray()) {
        interiorColor = std::make_unique<AnnotColor>(obj1.getArray());
    }

    leaderLineLength = dict->lookup("LL").getNumWithDefaultValue(0);

    leaderLineExtension = dict->lookup("LLE").getNumWithDefaultValue(0);
    if (leaderLineExtension < 0)
        leaderLineExtension = 0;

    caption = dict->lookup("Cap").getBoolWithDefaultValue(false);

    obj1 = dict->lookup("IT");
    if (obj1.isName()) {
        const char *intentName = obj1.getName();

        if (!strcmp(intentName, "LineArrow")) {
            intent = intentLineArrow;
        } else if (!strcmp(intentName, "LineDimension")) {
            intent = intentLineDimension;
        } else {
            intent = intentLineArrow;
        }
    } else {
        intent = intentLineArrow;
    }

    leaderLineOffset = dict->lookup("LLO").getNumWithDefaultValue(0);
    if (leaderLineOffset < 0)
        leaderLineOffset = 0;

    obj1 = dict->lookup("CP");
    if (obj1.isName()) {
        const char *captionName = obj1.getName();

        if (!strcmp(captionName, "Inline")) {
            captionPos = captionPosInline;
        } else if (!strcmp(captionName, "Top")) {
            captionPos = captionPosTop;
        } else {
            captionPos = captionPosInline;
        }
    } else {
        captionPos = captionPosInline;
    }

    obj1 = dict->lookup("Measure");
    if (obj1.isDict()) {
        measure = nullptr;
    } else {
        measure = nullptr;
    }

    obj1 = dict->lookup("CO");
    if (obj1.isArray() && (obj1.arrayGetLength() == 2)) {
        captionTextHorizontal = obj1.arrayGet(0).getNumWithDefaultValue(0);
        captionTextVertical = obj1.arrayGet(1).getNumWithDefaultValue(0);
    } else {
        captionTextHorizontal = captionTextVertical = 0;
    }

    obj1 = dict->lookup("BS");
    if (obj1.isDict()) {
        border = std::make_unique<AnnotBorderBS>(obj1.getDict());
    } else if (!border) {
        border = std::make_unique<AnnotBorderBS>();
    }
}

void AnnotLine::setContents(GooString *new_content)
{
    Annot::setContents(new_content);
    if (caption)
        invalidateAppearance();
}

void AnnotLine::setVertices(double x1, double y1, double x2, double y2)
{
    coord1 = std::make_unique<AnnotCoord>(x1, y1);
    coord2 = std::make_unique<AnnotCoord>(x2, y2);

    Array *lArray = new Array(doc->getXRef());
    lArray->add(Object(x1));
    lArray->add(Object(y1));
    lArray->add(Object(x2));
    lArray->add(Object(y2));

    update("L", Object(lArray));
    invalidateAppearance();
}

void AnnotLine::setStartEndStyle(AnnotLineEndingStyle start, AnnotLineEndingStyle end)
{
    startStyle = start;
    endStyle = end;

    Array *leArray = new Array(doc->getXRef());
    leArray->add(Object(objName, convertAnnotLineEndingStyle(startStyle)));
    leArray->add(Object(objName, convertAnnotLineEndingStyle(endStyle)));

    update("LE", Object(leArray));
    invalidateAppearance();
}

void AnnotLine::setInteriorColor(std::unique_ptr<AnnotColor> &&new_color)
{
    if (new_color) {
        Object obj1 = new_color->writeToObject(doc->getXRef());
        update("IC", std::move(obj1));
        interiorColor = std::move(new_color);
    } else {
        interiorColor = nullptr;
    }
    invalidateAppearance();
}

void AnnotLine::setLeaderLineLength(double len)
{
    leaderLineLength = len;
    update("LL", Object(len));
    invalidateAppearance();
}

void AnnotLine::setLeaderLineExtension(double len)
{
    leaderLineExtension = len;
    update("LLE", Object(len));

    // LL is required if LLE is present
    update("LL", Object(leaderLineLength));
    invalidateAppearance();
}

void AnnotLine::setCaption(bool new_cap)
{
    caption = new_cap;
    update("Cap", Object(new_cap));
    invalidateAppearance();
}

void AnnotLine::setIntent(AnnotLineIntent new_intent)
{
    const char *intentName;

    intent = new_intent;
    if (new_intent == intentLineArrow)
        intentName = "LineArrow";
    else // intentLineDimension
        intentName = "LineDimension";
    update("IT", Object(objName, intentName));
}

void AnnotLine::generateLineAppearance()
{
    double borderWidth, ca = opacity;
    bool fill = false;

    appearBBox = std::make_unique<AnnotAppearanceBBox>(rect.get());
    AnnotAppearanceBuilder appearBuilder;
    appearBuilder.append("q\n");
    if (color) {
        appearBuilder.setDrawColor(color.get(), false);
    }
    if (interiorColor) {
        appearBuilder.setDrawColor(interiorColor.get(), true);
        fill = true;
    }
    appearBuilder.setLineStyleForBorder(border.get());
    borderWidth = border->getWidth();
    appearBBox->setBorderWidth(std::max(1., borderWidth));

    const double x1 = coord1->getX();
    const double y1 = coord1->getY();
    const double x2 = coord2->getX();
    const double y2 = coord2->getY();

    // Main segment length
    const double main_len = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));

    // Main segment becomes positive x direction, coord1 becomes (0,0)
    Matrix matr;
    const double angle = atan2(y2 - y1, x2 - x1);
    matr.m[0] = matr.m[3] = cos(angle);
    matr.m[1] = sin(angle);
    matr.m[2] = -matr.m[1];
    matr.m[4] = x1 - rect->x1;
    matr.m[5] = y1 - rect->y1;

    double tx, ty, captionwidth = 0, captionheight = 0;
    AnnotLineCaptionPos actualCaptionPos = captionPos;
    const double fontsize = 9;
    const double captionhmargin = 2; // Left and right margin (inline caption only)
    const double captionmaxwidth = main_len - 2 * captionhmargin;
    const double lineendingSize = std::min(6. * borderWidth, main_len / 2);

    Dict *fontResDict;
    GfxFont *font;

    // Calculate caption width and height
    if (caption) {
        fontResDict = new Dict(doc->getXRef());
        font = createAnnotDrawFont(doc->getXRef(), fontResDict);
        int lines = 0;
        int i = 0;
        while (i < contents->getLength()) {
            GooString out;
            double linewidth;
            layoutText(contents.get(), &out, &i, font, &linewidth, 0, nullptr, false);
            linewidth *= fontsize;
            if (linewidth > captionwidth) {
                captionwidth = linewidth;
            }
            ++lines;
        }
        captionheight = lines * fontsize;
        // If text is longer than available space, turn into captionPosTop
        if (captionwidth > captionmaxwidth) {
            actualCaptionPos = captionPosTop;
        }
    } else {
        fontResDict = nullptr;
        font = nullptr;
    }

    // Draw main segment
    matr.transform(AnnotAppearanceBuilder::lineEndingXShorten(startStyle, lineendingSize), leaderLineLength, &tx, &ty);
    appearBuilder.appendf("{0:.2f} {1:.2f} m\n", tx, ty);
    appearBBox->extendTo(tx, ty);

    if (captionwidth != 0 && actualCaptionPos == captionPosInline) { // Break in the middle
        matr.transform((main_len - captionwidth) / 2 - captionhmargin, leaderLineLength, &tx, &ty);
        appearBuilder.appendf("{0:.2f} {1:.2f} l S\n", tx, ty);

        matr.transform((main_len + captionwidth) / 2 + captionhmargin, leaderLineLength, &tx, &ty);
        appearBuilder.appendf("{0:.2f} {1:.2f} m\n", tx, ty);
    }

    matr.transform(main_len - AnnotAppearanceBuilder::lineEndingXShorten(endStyle, lineendingSize), leaderLineLength, &tx, &ty);
    appearBuilder.appendf("{0:.2f} {1:.2f} l S\n", tx, ty);
    appearBBox->extendTo(tx, ty);

    if (startStyle != annotLineEndingNone) {
        const double extendX { -AnnotAppearanceBuilder::lineEndingXExtendBBox(startStyle, lineendingSize) };
        appearBuilder.drawLineEnding(startStyle, 0, leaderLineLength, -lineendingSize, fill, matr);
        matr.transform(extendX, leaderLineLength + lineendingSize / 2., &tx, &ty);
        appearBBox->extendTo(tx, ty);
        matr.transform(extendX, leaderLineLength - lineendingSize / 2., &tx, &ty);
        appearBBox->extendTo(tx, ty);
    }

    if (endStyle != annotLineEndingNone) {
        const double extendX { AnnotAppearanceBuilder::lineEndingXExtendBBox(endStyle, lineendingSize) };
        appearBuilder.drawLineEnding(endStyle, main_len, leaderLineLength, lineendingSize, fill, matr);
        matr.transform(main_len + extendX, leaderLineLength + lineendingSize / 2., &tx, &ty);
        appearBBox->extendTo(tx, ty);
        matr.transform(main_len + extendX, leaderLineLength - lineendingSize / 2., &tx, &ty);
        appearBBox->extendTo(tx, ty);
    }

    // Draw caption text
    if (caption) {
        double tlx = (main_len - captionwidth) / 2, tly; // Top-left coords
        if (actualCaptionPos == captionPosInline) {
            tly = leaderLineLength + captionheight / 2;
        } else {
            tly = leaderLineLength + captionheight + 2 * borderWidth;
        }

        tlx += captionTextHorizontal;
        tly += captionTextVertical;

        // Adjust bounding box
        matr.transform(tlx, tly - captionheight, &tx, &ty);
        appearBBox->extendTo(tx, ty);
        matr.transform(tlx + captionwidth, tly - captionheight, &tx, &ty);
        appearBBox->extendTo(tx, ty);
        matr.transform(tlx + captionwidth, tly, &tx, &ty);
        appearBBox->extendTo(tx, ty);
        matr.transform(tlx, tly, &tx, &ty);
        appearBBox->extendTo(tx, ty);

        // Setup text state (reusing transformed top-left coord)
        appearBuilder.appendf("0 g BT /AnnotDrawFont {0:.2f} Tf\n", fontsize); // Font color: black
        appearBuilder.appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} Tm\n", matr.m[0], matr.m[1], matr.m[2], matr.m[3], tx, ty);
        appearBuilder.appendf("0 {0:.2f} Td\n", -fontsize * font->getDescent());
        // Draw text
        int i = 0;
        double xposPrev = 0;
        while (i < contents->getLength()) {
            GooString out;
            double linewidth, xpos;
            layoutText(contents.get(), &out, &i, font, &linewidth, 0, nullptr, false);
            linewidth *= fontsize;
            xpos = (captionwidth - linewidth) / 2;
            appearBuilder.appendf("{0:.2f} {1:.2f} Td\n", xpos - xposPrev, -fontsize);
            appearBuilder.writeString(out);
            appearBuilder.append("Tj\n");
            xposPrev = xpos;
        }
        appearBuilder.append("ET\n");
        font->decRefCnt();
    }

    // Draw leader lines
    double ll_len = fabs(leaderLineLength) + leaderLineExtension;
    double sign = leaderLineLength >= 0 ? 1 : -1;
    if (ll_len != 0) {
        matr.transform(0, 0, &tx, &ty);
        appearBuilder.appendf("{0:.2f} {1:.2f} m\n", tx, ty);
        appearBBox->extendTo(tx, ty);
        matr.transform(0, sign * ll_len, &tx, &ty);
        appearBuilder.appendf("{0:.2f} {1:.2f} l S\n", tx, ty);
        appearBBox->extendTo(tx, ty);

        matr.transform(main_len, 0, &tx, &ty);
        appearBuilder.appendf("{0:.2f} {1:.2f} m\n", tx, ty);
        appearBBox->extendTo(tx, ty);
        matr.transform(main_len, sign * ll_len, &tx, &ty);
        appearBuilder.appendf("{0:.2f} {1:.2f} l S\n", tx, ty);
        appearBBox->extendTo(tx, ty);
    }

    appearBuilder.append("Q\n");

    double bbox[4];
    appearBBox->getBBoxRect(bbox);
    if (ca == 1) {
        appearance = createForm(appearBuilder.buffer(), bbox, false, fontResDict);
    } else {
        Object aStream = createForm(appearBuilder.buffer(), bbox, true, fontResDict);

        GooString appearBuf("/GS0 gs\n/Fm0 Do");
        Dict *resDict = createResourcesDict("Fm0", std::move(aStream), "GS0", ca, nullptr);
        appearance = createForm(&appearBuf, bbox, false, resDict);
    }
}

void AnnotLine::draw(Gfx *gfx, bool printing)
{
    if (!isVisible(printing))
        return;

    annotLocker();
    if (appearance.isNull()) {
        generateLineAppearance();
    }

    // draw the appearance stream
    Object obj = appearance.fetch(gfx->getXRef());
    if (appearBBox) {
        gfx->drawAnnot(&obj, nullptr, color.get(), appearBBox->getPageXMin(), appearBBox->getPageYMin(), appearBBox->getPageXMax(), appearBBox->getPageYMax(), getRotation());
    } else {
        gfx->drawAnnot(&obj, nullptr, color.get(), rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
    }
}

// Before retrieving the res dict, regenerate the appearance stream if needed,
// because AnnotLine::draw may need to store font info in the res dict
Object AnnotLine::getAppearanceResDict()
{
    if (appearance.isNull()) {
        generateLineAppearance();
    }
    return Annot::getAppearanceResDict();
}

//------------------------------------------------------------------------
// AnnotTextMarkup
//------------------------------------------------------------------------
AnnotTextMarkup::AnnotTextMarkup(PDFDoc *docA, PDFRectangle *rectA, AnnotSubtype subType) : AnnotMarkup(docA, rectA)
{
    switch (subType) {
    case typeHighlight:
        annotObj.dictSet("Subtype", Object(objName, "Highlight"));
        break;
    case typeUnderline:
        annotObj.dictSet("Subtype", Object(objName, "Underline"));
        break;
    case typeSquiggly:
        annotObj.dictSet("Subtype", Object(objName, "Squiggly"));
        break;
    case typeStrikeOut:
        annotObj.dictSet("Subtype", Object(objName, "StrikeOut"));
        break;
    default:
        assert(0 && "Invalid subtype for AnnotTextMarkup\n");
    }

    // Store dummy quadrilateral with null coordinates
    Array *quadPoints = new Array(doc->getXRef());
    for (int i = 0; i < 4 * 2; ++i) {
        quadPoints->add(Object(0.));
    }
    annotObj.dictSet("QuadPoints", Object(quadPoints));

    initialize(docA, annotObj.getDict());
}

AnnotTextMarkup::AnnotTextMarkup(PDFDoc *docA, Object &&dictObject, const Object *obj) : AnnotMarkup(docA, std::move(dictObject), obj)
{
    // the real type will be read in initialize()
    type = typeHighlight;
    initialize(docA, annotObj.getDict());
}

AnnotTextMarkup::~AnnotTextMarkup() = default;

void AnnotTextMarkup::initialize(PDFDoc *docA, Dict *dict)
{
    Object obj1;

    obj1 = dict->lookup("Subtype");
    if (obj1.isName()) {
        GooString typeName(obj1.getName());
        if (!typeName.cmp("Highlight")) {
            type = typeHighlight;
        } else if (!typeName.cmp("Underline")) {
            type = typeUnderline;
        } else if (!typeName.cmp("Squiggly")) {
            type = typeSquiggly;
        } else if (!typeName.cmp("StrikeOut")) {
            type = typeStrikeOut;
        }
    }

    obj1 = dict->lookup("QuadPoints");
    if (obj1.isArray()) {
        quadrilaterals = std::make_unique<AnnotQuadrilaterals>(obj1.getArray(), rect.get());
    } else {
        error(errSyntaxError, -1, "Bad Annot Text Markup QuadPoints");
        ok = false;
    }
}

void AnnotTextMarkup::setType(AnnotSubtype new_type)
{
    const char *typeName = nullptr; /* squelch bogus compiler warning */

    switch (new_type) {
    case typeHighlight:
        typeName = "Highlight";
        break;
    case typeUnderline:
        typeName = "Underline";
        break;
    case typeSquiggly:
        typeName = "Squiggly";
        break;
    case typeStrikeOut:
        typeName = "StrikeOut";
        break;
    default:
        assert(!"Invalid subtype");
    }

    type = new_type;
    update("Subtype", Object(objName, typeName));
    invalidateAppearance();
}

void AnnotTextMarkup::setQuadrilaterals(AnnotQuadrilaterals *quadPoints)
{
    Array *a = new Array(doc->getXRef());

    for (int i = 0; i < quadPoints->getQuadrilateralsLength(); ++i) {
        a->add(Object(quadPoints->getX1(i)));
        a->add(Object(quadPoints->getY1(i)));
        a->add(Object(quadPoints->getX2(i)));
        a->add(Object(quadPoints->getY2(i)));
        a->add(Object(quadPoints->getX3(i)));
        a->add(Object(quadPoints->getY3(i)));
        a->add(Object(quadPoints->getX4(i)));
        a->add(Object(quadPoints->getY4(i)));
    }

    quadrilaterals = std::make_unique<AnnotQuadrilaterals>(a, rect.get());

    annotObj.dictSet("QuadPoints", Object(a));
    invalidateAppearance();
}

void AnnotTextMarkup::draw(Gfx *gfx, bool printing)
{
    double ca = 1;
    int i;

    if (!isVisible(printing))
        return;

    annotLocker();
    if (appearance.isNull() || type == typeHighlight) {
        bool blendMultiply = true;
        ca = opacity;

        AnnotAppearanceBuilder appearBuilder;
        appearBuilder.append("q\n");

        /* Adjust BBox */
        appearBBox = std::make_unique<AnnotAppearanceBBox>(rect.get());
        for (i = 0; i < quadrilaterals->getQuadrilateralsLength(); ++i) {
            appearBBox->extendTo(quadrilaterals->getX1(i) - rect->x1, quadrilaterals->getY1(i) - rect->y1);
            appearBBox->extendTo(quadrilaterals->getX2(i) - rect->x1, quadrilaterals->getY2(i) - rect->y1);
            appearBBox->extendTo(quadrilaterals->getX3(i) - rect->x1, quadrilaterals->getY3(i) - rect->y1);
            appearBBox->extendTo(quadrilaterals->getX4(i) - rect->x1, quadrilaterals->getY4(i) - rect->y1);
        }

        switch (type) {
        case typeUnderline:
            if (color) {
                appearBuilder.setDrawColor(color.get(), false);
            }
            appearBuilder.append("[] 0 d 1 w\n");

            for (i = 0; i < quadrilaterals->getQuadrilateralsLength(); ++i) {
                double x3, y3, x4, y4;

                x3 = quadrilaterals->getX3(i);
                y3 = quadrilaterals->getY3(i);
                x4 = quadrilaterals->getX4(i);
                y4 = quadrilaterals->getY4(i);

                appearBuilder.appendf("{0:.2f} {1:.2f} m\n", x3, y3);
                appearBuilder.appendf("{0:.2f} {1:.2f} l\n", x4, y4);
                appearBuilder.append("S\n");
            }
            break;
        case typeStrikeOut:
            if (color) {
                appearBuilder.setDrawColor(color.get(), false);
            }
            blendMultiply = false;
            appearBuilder.append("[] 0 d 1 w\n");

            for (i = 0; i < quadrilaterals->getQuadrilateralsLength(); ++i) {
                double x1, y1, x2, y2;
                double x3, y3, x4, y4;

                x1 = quadrilaterals->getX1(i);
                y1 = quadrilaterals->getY1(i);
                x2 = quadrilaterals->getX2(i);
                y2 = quadrilaterals->getY2(i);

                x3 = quadrilaterals->getX3(i);
                y3 = quadrilaterals->getY3(i);
                x4 = quadrilaterals->getX4(i);
                y4 = quadrilaterals->getY4(i);

                appearBuilder.appendf("{0:.2f} {1:.2f} m\n", (x1 + x3) / 2., (y1 + y3) / 2.);
                appearBuilder.appendf("{0:.2f} {1:.2f} l\n", (x2 + x4) / 2., (y2 + y4) / 2.);
                appearBuilder.append("S\n");
            }
            break;
        case typeSquiggly:
            if (color) {
                appearBuilder.setDrawColor(color.get(), false);
            }
            appearBuilder.append("[] 0 d 1 w\n");

            for (i = 0; i < quadrilaterals->getQuadrilateralsLength(); ++i) {
                double x1, y1, x2, y3;
                double h6;

                x1 = quadrilaterals->getX1(i);
                y1 = quadrilaterals->getY1(i);
                x2 = quadrilaterals->getX2(i);
                y3 = quadrilaterals->getY3(i);
                h6 = (y1 - y3) / 6.0;

                appearBuilder.appendf("{0:.2f} {1:.2f} m\n", x1, y3 + h6);
                bool down = false;
                do {
                    down = !down; // Zigzag line
                    x1 += 2;
                    appearBuilder.appendf("{0:.2f} {1:.2f} l\n", x1, y3 + (down ? 0 : h6));
                } while (x1 < x2);
                appearBuilder.append("S\n");
            }
            break;
        default:
        case typeHighlight:
            if (color)
                appearBuilder.setDrawColor(color.get(), true);

            double biggestBorder = 0;
            for (i = 0; i < quadrilaterals->getQuadrilateralsLength(); ++i) {
                double x1, y1, x2, y2, x3, y3, x4, y4;
                double h4;

                x1 = quadrilaterals->getX1(i);
                y1 = quadrilaterals->getY1(i);
                x2 = quadrilaterals->getX2(i);
                y2 = quadrilaterals->getY2(i);
                x3 = quadrilaterals->getX3(i);
                y3 = quadrilaterals->getY3(i);
                x4 = quadrilaterals->getX4(i);
                y4 = quadrilaterals->getY4(i);
                h4 = fabs(y1 - y3) / 4.0;

                if (h4 > biggestBorder) {
                    biggestBorder = h4;
                }

                appearBuilder.appendf("{0:.2f} {1:.2f} m\n", x3, y3);
                appearBuilder.appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n", x3 - h4, y3 + h4, x1 - h4, y1 - h4, x1, y1);
                appearBuilder.appendf("{0:.2f} {1:.2f} l\n", x2, y2);
                appearBuilder.appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n", x2 + h4, y2 - h4, x4 + h4, y4 + h4, x4, y4);
                appearBuilder.append("f\n");
            }
            appearBBox->setBorderWidth(biggestBorder);
            break;
        }
        appearBuilder.append("Q\n");

        double bbox[4];
        bbox[0] = appearBBox->getPageXMin();
        bbox[1] = appearBBox->getPageYMin();
        bbox[2] = appearBBox->getPageXMax();
        bbox[3] = appearBBox->getPageYMax();
        Object aStream = createForm(appearBuilder.buffer(), bbox, true, nullptr);

        GooString appearBuf("/GS0 gs\n/Fm0 Do");
        Dict *resDict = createResourcesDict("Fm0", std::move(aStream), "GS0", 1, blendMultiply ? "Multiply" : nullptr);
        if (ca == 1) {
            appearance = createForm(&appearBuf, bbox, false, resDict);
        } else {
            aStream = createForm(&appearBuf, bbox, true, resDict);

            Dict *resDict2 = createResourcesDict("Fm0", std::move(aStream), "GS0", ca, nullptr);
            appearance = createForm(&appearBuf, bbox, false, resDict2);
        }
    }

    // draw the appearance stream
    Object obj = appearance.fetch(gfx->getXRef());
    if (appearBBox) {
        gfx->drawAnnot(&obj, nullptr, color.get(), appearBBox->getPageXMin(), appearBBox->getPageYMin(), appearBBox->getPageXMax(), appearBBox->getPageYMax(), getRotation());
    } else {
        gfx->drawAnnot(&obj, nullptr, color.get(), rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
    }
}

//------------------------------------------------------------------------
// AnnotWidget
//------------------------------------------------------------------------

AnnotWidget::AnnotWidget(PDFDoc *docA, Object &&dictObject, const Object *obj) : Annot(docA, std::move(dictObject), obj)
{
    type = typeWidget;
    field = nullptr;
    initialize(docA, annotObj.getDict());
}

AnnotWidget::AnnotWidget(PDFDoc *docA, Object *dictObject, Object *obj, FormField *fieldA) : Annot(docA, dictObject->copy(), obj)
{
    type = typeWidget;
    field = fieldA;
    initialize(docA, dictObject->getDict());
}

AnnotWidget::~AnnotWidget() = default;

void AnnotWidget::initialize(PDFDoc *docA, Dict *dict)
{
    Object obj1;

    form = doc->getCatalog()->getForm();

    obj1 = dict->lookup("H");
    if (obj1.isName()) {
        const char *modeName = obj1.getName();

        if (!strcmp(modeName, "N")) {
            mode = highlightModeNone;
        } else if (!strcmp(modeName, "O")) {
            mode = highlightModeOutline;
        } else if (!strcmp(modeName, "P") || !strcmp(modeName, "T")) {
            mode = highlightModePush;
        } else {
            mode = highlightModeInvert;
        }
    } else {
        mode = highlightModeInvert;
    }

    obj1 = dict->lookup("MK");
    if (obj1.isDict()) {
        appearCharacs = std::make_unique<AnnotAppearanceCharacs>(obj1.getDict());
    }

    obj1 = dict->lookup("A");
    if (obj1.isDict()) {
        action = LinkAction::parseAction(&obj1, doc->getCatalog()->getBaseURI());
    }

    additionalActions = dict->lookupNF("AA").copy();

    obj1 = dict->lookup("Parent");
    if (obj1.isDict()) {
        parent = nullptr;
    } else {
        parent = nullptr;
    }

    obj1 = dict->lookup("BS");
    if (obj1.isDict()) {
        border = std::make_unique<AnnotBorderBS>(obj1.getDict());
    }

    updatedAppearanceStream = Ref::INVALID();
}

std::unique_ptr<LinkAction> AnnotWidget::getAdditionalAction(AdditionalActionsType additionalActionType)
{
    return ::getAdditionalAction(additionalActionType, &additionalActions, doc);
}

std::unique_ptr<LinkAction> AnnotWidget::getFormAdditionalAction(FormAdditionalActionsType formAdditionalActionType)
{
    Object additionalActionsObject = additionalActions.fetch(doc->getXRef());

    if (additionalActionsObject.isDict()) {
        const char *key = getFormAdditionalActionKey(formAdditionalActionType);

        Object actionObject = additionalActionsObject.dictLookup(key);
        if (actionObject.isDict())
            return LinkAction::parseAction(&actionObject, doc->getCatalog()->getBaseURI());
    }

    return nullptr;
}

bool AnnotWidget::setFormAdditionalAction(FormAdditionalActionsType formAdditionalActionType, const GooString &js)
{
    Object additionalActionsObject = additionalActions.fetch(doc->getXRef());

    if (!additionalActionsObject.isDict()) {
        additionalActionsObject = Object(new Dict(doc->getXRef()));
        annotObj.dictSet("AA", additionalActionsObject.copy());
    }

    additionalActionsObject.dictSet(getFormAdditionalActionKey(formAdditionalActionType), LinkJavaScript::createObject(doc->getXRef(), js));

    if (additionalActions.isRef()) {
        doc->getXRef()->setModifiedObject(&additionalActionsObject, additionalActions.getRef());
    } else if (hasRef) {
        doc->getXRef()->setModifiedObject(&annotObj, ref);
    } else {
        error(errInternal, -1, "AnnotWidget::setFormAdditionalAction, where neither additionalActions is ref nor annotobj itself is ref");
        return false;
    }
    return true;
}

void AnnotWidget::setNewAppearance(Object &&newAppearance)
{
    if (!newAppearance.isNull()) {
        appearStreams = std::make_unique<AnnotAppearance>(doc, &newAppearance);
        update("AP", std::move(newAppearance));
    }

    if (appearStreams)
        appearance = appearStreams->getAppearanceStream(AnnotAppearance::appearNormal, appearState->c_str());
}

// Grand unified handler for preparing text strings to be drawn into form
// fields.  Takes as input a text string (in PDFDocEncoding or UTF-16).
// Converts some or all of this string to the appropriate encoding for the
// specified font, and computes the width of the text.  Can optionally stop
// converting when a specified width has been reached, to perform line-breaking
// for multi-line fields.
//
// Parameters:
//   text: input text string to convert
//   outBuf: buffer for writing re-encoded string
//   i: index at which to start converting; will be updated to point just after
//      last character processed
//   font: the font which will be used for display
//   width: computed width (unscaled by font size) will be stored here
//   widthLimit: if non-zero, stop converting to keep width under this value
//      (should be scaled down by font size)
//   charCount: count of number of characters will be stored here
//   noReencode: if set, do not try to translate the character encoding
//      (useful for Zapf Dingbats or other unusual encodings)
//      can only be used with simple fonts, not CID-keyed fonts
//
// TODO: Handle surrogate pairs in UTF-16.
//       Should be able to generate output for any CID-keyed font.
//       Doesn't handle vertical fonts--should it?
void Annot::layoutText(const GooString *text, GooString *outBuf, int *i, const GfxFont *font, double *width, double widthLimit, int *charCount, bool noReencode)
{
    CharCode c;
    Unicode uChar;
    const Unicode *uAux;
    double w = 0.0;
    int uLen, n;
    double dx, dy, ox, oy;

    if (width != nullptr)
        *width = 0.0;
    if (charCount != nullptr)
        *charCount = 0;

    if (!text) {
        return;
    }
    bool unicode = text->hasUnicodeMarker();
    bool spacePrev; // previous character was a space

    // State for backtracking when more text has been processed than fits within
    // widthLimit.  We track for both input (text) and output (outBuf) the offset
    // to the first character to discard.
    //
    // We keep track of several points:
    //   1 - end of previous completed word which fits
    //   2 - previous character which fit
    int last_i1, last_i2, last_o1, last_o2;

    if (unicode && text->getLength() % 2 != 0) {
        error(errSyntaxError, -1, "AnnotWidget::layoutText, bad unicode string");
        return;
    }

    // skip Unicode marker on string if needed
    if (unicode && *i == 0)
        *i = 2;

    // Start decoding and copying characters, until either:
    //   we reach the end of the string
    //   we reach the maximum width
    //   we reach a newline character
    // As we copy characters, keep track of the last full word to fit, so that we
    // can backtrack if we exceed the maximum width.
    last_i1 = last_i2 = *i;
    last_o1 = last_o2 = 0;
    spacePrev = false;
    outBuf->clear();

    while (*i < text->getLength()) {
        last_i2 = *i;
        last_o2 = outBuf->getLength();

        if (unicode) {
            uChar = (unsigned char)(text->getChar(*i)) << 8;
            uChar += (unsigned char)(text->getChar(*i + 1));
            *i += 2;
        } else {
            if (noReencode)
                uChar = text->getChar(*i) & 0xff;
            else
                uChar = pdfDocEncoding[text->getChar(*i) & 0xff];
            *i += 1;
        }

        // Explicit line break?
        if (uChar == '\r' || uChar == '\n') {
            // Treat a <CR><LF> sequence as a single line break
            if (uChar == '\r' && *i < text->getLength()) {
                if (unicode && text->getChar(*i) == '\0' && text->getChar(*i + 1) == '\n')
                    *i += 2;
                else if (!unicode && text->getChar(*i) == '\n')
                    *i += 1;
            }

            break;
        }

        if (noReencode) {
            outBuf->append(uChar);
        } else {
            const CharCodeToUnicode *ccToUnicode = font->getToUnicode();
            if (!ccToUnicode) {
                // This assumes an identity CMap.
                outBuf->append((uChar >> 8) & 0xff);
                outBuf->append(uChar & 0xff);
            } else if (ccToUnicode->mapToCharCode(&uChar, &c, 1)) {
                if (font->isCIDFont()) {
                    // TODO: This assumes an identity CMap.  It should be extended to
                    // handle the general case.
                    outBuf->append((c >> 8) & 0xff);
                    outBuf->append(c & 0xff);
                } else {
                    // 8-bit font
                    outBuf->append(c);
                }
            } else {
                error(errSyntaxError, -1, "AnnotWidget::layoutText, cannot convert U+{0:04uX}", uChar);
            }
        }

        // If we see a space, then we have a linebreak opportunity.
        if (uChar == ' ') {
            last_i1 = *i;
            if (!spacePrev)
                last_o1 = last_o2;
            spacePrev = true;
        } else {
            spacePrev = false;
        }

        // Compute width of character just output
        if (outBuf->getLength() > last_o2) {
            dx = 0.0;
            font->getNextChar(outBuf->c_str() + last_o2, outBuf->getLength() - last_o2, &c, &uAux, &uLen, &dx, &dy, &ox, &oy);
            w += dx;
        }

        // Current line over-full now?
        if (widthLimit > 0.0 && w > widthLimit) {
            if (last_o1 > 0) {
                // Back up to the previous word which fit, if there was a previous
                // word.
                *i = last_i1;
                outBuf->del(last_o1, outBuf->getLength() - last_o1);
            } else if (last_o2 > 0) {
                // Otherwise, back up to the previous character (in the only word on
                // this line)
                *i = last_i2;
                outBuf->del(last_o2, outBuf->getLength() - last_o2);
            } else {
                // Otherwise, we were trying to fit the first character; include it
                // anyway even if it overflows the space--no updates to make.
            }
            break;
        }
    }

    // If splitting input into lines because we reached the width limit, then
    // consume any remaining trailing spaces that would go on this line from the
    // input.  If in doing so we reach a newline, consume that also.  This code
    // won't run if we stopped at a newline above, since in that case w <=
    // widthLimit still.
    if (widthLimit > 0.0 && w > widthLimit) {
        if (unicode) {
            while (*i < text->getLength() && text->getChar(*i) == '\0' && text->getChar(*i + 1) == ' ')
                *i += 2;
            if (*i < text->getLength() && text->getChar(*i) == '\0' && text->getChar(*i + 1) == '\r')
                *i += 2;
            if (*i < text->getLength() && text->getChar(*i) == '\0' && text->getChar(*i + 1) == '\n')
                *i += 2;
        } else {
            while (*i < text->getLength() && text->getChar(*i) == ' ')
                *i += 1;
            if (*i < text->getLength() && text->getChar(*i) == '\r')
                *i += 1;
            if (*i < text->getLength() && text->getChar(*i) == '\n')
                *i += 1;
        }
    }

    // Compute the actual width and character count of the final string, based on
    // breakpoint, if this information is requested by the caller.
    if (width != nullptr || charCount != nullptr) {
        const char *s = outBuf->c_str();
        int len = outBuf->getLength();

        while (len > 0) {
            dx = 0.0;
            n = font->getNextChar(s, len, &c, &uAux, &uLen, &dx, &dy, &ox, &oy);

            if (n == 0) {
                break;
            }

            if (width != nullptr)
                *width += dx;
            if (charCount != nullptr)
                *charCount += 1;

            s += n;
            len -= n;
        }
    }
}

// Copy the given string to appearBuf, adding parentheses around it and
// escaping characters as appropriate.
void AnnotAppearanceBuilder::writeString(const GooString &str)
{
    char c;
    int i;

    appearBuf->append('(');

    for (i = 0; i < str.getLength(); ++i) {
        c = str.getChar(i);
        if (c == '(' || c == ')' || c == '\\') {
            appearBuf->append('\\');
            appearBuf->append(c);
        } else if (c < 0x20) {
            appearBuf->appendf("\\{0:03o}", (unsigned char)c);
        } else {
            appearBuf->append(c);
        }
    }

    appearBuf->append(')');
}

// Draw the variable text or caption for a field.
bool AnnotAppearanceBuilder::drawText(const GooString *text, const GooString *da, const GfxResources *resources, const AnnotBorder *border, const AnnotAppearanceCharacs *appearCharacs, const PDFRectangle *rect, bool multiline, int comb,
                                      int quadding, bool txField, bool forceZapfDingbats, XRef *xref, bool *addedDingbatsResource, bool password)
{
    std::vector<GooString *> *daToks;
    GooString *tok;
    GooString convertedText;
    const GfxFont *font;
    double dx, dy;
    double fontSize, borderWidth, x, xPrev, y, w, wMax;
    int tfPos, tmPos, j;
    int rot;
    bool freeText = false; // true if text should be freed before return
    GfxFont *fontToFree = nullptr;

    //~ if there is no MK entry, this should use the existing content stream,
    //~ and only replace the marked content portion of it
    //~ (this is only relevant for Tx fields)

    // parse the default appearance string
    tfPos = tmPos = -1;
    if (da) {
        daToks = new std::vector<GooString *>();
        int i = 0;
        while (i < da->getLength()) {
            while (i < da->getLength() && Lexer::isSpace(da->getChar(i))) {
                ++i;
            }
            if (i < da->getLength()) {
                for (j = i + 1; j < da->getLength() && !Lexer::isSpace(da->getChar(j)); ++j)
                    ;
                daToks->push_back(new GooString(da, i, j - i));
                i = j;
            }
        }
        for (i = 2; i < (int)daToks->size(); ++i) {
            if (i >= 2 && !((*daToks)[i])->cmp("Tf")) {
                tfPos = i - 2;
            } else if (i >= 6 && !((*daToks)[i])->cmp("Tm")) {
                tmPos = i - 6;
            }
        }
    } else {
        daToks = nullptr;
    }

    // force ZapfDingbats
    if (forceZapfDingbats) {
        assert(xref != nullptr);
        assert(addedDingbatsResource != nullptr);
        *addedDingbatsResource = false;

        if (tfPos >= 0) {
            tok = (*daToks)[tfPos];
            if (tok->cmp("/ZaDb")) {
                tok->clear();
                tok->append("/ZaDb");
            }
        }
    }
    // get the font and font size
    font = nullptr;
    fontSize = 0;
    if (tfPos >= 0) {
        tok = (*daToks)[tfPos];
        if (tok->getLength() >= 1 && tok->getChar(0) == '/') {
            if (!resources || !(font = resources->lookupFont(tok->c_str() + 1))) {
                if (forceZapfDingbats) {
                    // We are forcing ZaDb but the font does not exist
                    // so create a fake one
                    Ref r = Ref::INVALID(); // dummy Ref, it's not used at all in this codepath
                    Dict *d = new Dict(xref);
                    fontToFree = new Gfx8BitFont(xref, "ZaDb", r, new GooString("ZapfDingbats"), fontType1, r, d);
                    delete d;
                    font = fontToFree;
                    *addedDingbatsResource = true;
                } else {
                    error(errSyntaxError, -1, "Unknown font in field's DA string");
                }
            }
        } else {
            error(errSyntaxError, -1, "Invalid font name in 'Tf' operator in field's DA string");
        }
        tok = (*daToks)[tfPos + 1];
        fontSize = gatof(tok->c_str());
    } else {
        error(errSyntaxError, -1, "Missing 'Tf' operator in field's DA string");
    }
    if (!font) {
        if (daToks) {
            for (auto entry : *daToks) {
                delete entry;
            }
            delete daToks;
        }
        return false;
    }

    // get the border width
    borderWidth = border ? border->getWidth() : 0;

    // for a password field, replace all characters with asterisks
    if (password) {
        int len;
        if (text->hasUnicodeMarker())
            len = (text->getLength() - 2) / 2;
        else
            len = text->getLength();

        GooString *newText = new GooString;
        for (int i = 0; i < len; ++i)
            newText->append('*');
        text = newText;
        freeText = true;
    }

    // setup
    if (txField) {
        appearBuf->append("/Tx BMC\n");
    }
    appearBuf->append("q\n");
    rot = appearCharacs ? appearCharacs->getRotation() : 0;
    switch (rot) {
    case 90:
        appearBuf->appendf("0 1 -1 0 {0:.2f} 0 cm\n", rect->x2 - rect->x1);
        dx = rect->y2 - rect->y1;
        dy = rect->x2 - rect->x1;
        break;
    case 180:
        appearBuf->appendf("-1 0 0 -1 {0:.2f} {1:.2f} cm\n", rect->x2 - rect->x1, rect->y2 - rect->y1);
        dx = rect->x2 - rect->y2;
        dy = rect->y2 - rect->y1;
        break;
    case 270:
        appearBuf->appendf("0 -1 1 0 0 {0:.2f} cm\n", rect->y2 - rect->y1);
        dx = rect->y2 - rect->y1;
        dy = rect->x2 - rect->x1;
        break;
    default: // assume rot == 0
        dx = rect->x2 - rect->x1;
        dy = rect->y2 - rect->y1;
        break;
    }
    appearBuf->append("BT\n");
    // multi-line text
    if (multiline) {
        // note: the comb flag is ignored in multiline mode

        wMax = dx - 2 * borderWidth - 4;

        // compute font autosize
        if (fontSize == 0) {
            for (fontSize = 20; fontSize > 1; --fontSize) {
                y = dy - 3;
                int i = 0;
                while (i < text->getLength()) {
                    Annot::layoutText(text, &convertedText, &i, font, &w, wMax / fontSize, nullptr, forceZapfDingbats);
                    y -= fontSize;
                }
                // approximate the descender for the last line
                if (y >= 0.33 * fontSize) {
                    break;
                }
            }
            if (tfPos >= 0) {
                tok = (*daToks)[tfPos + 1];
                tok->clear();
                tok->appendf("{0:.2f}", fontSize);
            }
        }

        // starting y coordinate
        // (note: each line of text starts with a Td operator that moves
        // down a line)
        y = dy - 3;

        // set the font matrix
        if (tmPos >= 0) {
            tok = (*daToks)[tmPos + 4];
            tok->clear();
            tok->append('0');
            tok = (*daToks)[tmPos + 5];
            tok->clear();
            tok->appendf("{0:.2f}", y);
        }

        // write the DA string
        if (daToks) {
            for (const GooString *daTok : *daToks) {
                appearBuf->append(daTok)->append(' ');
            }
        }

        // write the font matrix (if not part of the DA string)
        if (tmPos < 0) {
            appearBuf->appendf("1 0 0 1 0 {0:.2f} Tm\n", y);
        }

        // write a series of lines of text
        int i = 0;
        xPrev = 0;
        while (i < text->getLength()) {
            Annot::layoutText(text, &convertedText, &i, font, &w, wMax / fontSize, nullptr, forceZapfDingbats);
            w *= fontSize;

            // compute text start position
            switch (quadding) {
            case quaddingLeftJustified:
            default:
                x = borderWidth + 2;
                break;
            case quaddingCentered:
                x = (dx - w) / 2;
                break;
            case quaddingRightJustified:
                x = dx - borderWidth - 2 - w;
                break;
            }

            // draw the line
            appearBuf->appendf("{0:.2f} {1:.2f} Td\n", x - xPrev, -fontSize);
            writeString(convertedText);
            appearBuf->append(" Tj\n");

            // next line
            xPrev = x;
        }

        // single-line text
    } else {
        //~ replace newlines with spaces? - what does Acrobat do?

        // comb formatting
        if (comb > 0) {
            int charCount;

            // compute comb spacing
            w = (dx - 2 * borderWidth) / comb;

            // compute font autosize
            if (fontSize == 0) {
                fontSize = dy - 2 * borderWidth;
                if (w < fontSize) {
                    fontSize = w;
                }
                fontSize = floor(fontSize);
                if (tfPos >= 0) {
                    tok = (*daToks)[tfPos + 1];
                    tok->clear();
                    tok->appendf("{0:.2f}", fontSize);
                }
            }

            int i = 0;
            Annot::layoutText(text, &convertedText, &i, font, nullptr, 0.0, &charCount, forceZapfDingbats);
            if (charCount > comb)
                charCount = comb;

            // compute starting text cell
            switch (quadding) {
            case quaddingLeftJustified:
            default:
                x = borderWidth;
                break;
            case quaddingCentered:
                x = borderWidth + (comb - charCount) / 2.0 * w;
                break;
            case quaddingRightJustified:
                x = borderWidth + (comb - charCount) * w;
                break;
            }
            y = 0.5 * dy - 0.4 * fontSize;

            // set the font matrix
            if (tmPos >= 0) {
                tok = (*daToks)[tmPos + 4];
                tok->clear();
                tok->appendf("{0:.2f}", x);
                tok = (*daToks)[tmPos + 5];
                tok->clear();
                tok->appendf("{0:.2f}", y);
            }

            // write the DA string
            if (daToks) {
                for (i = 0; i < (int)daToks->size(); ++i) {
                    appearBuf->append((*daToks)[i])->append(' ');
                }
            }

            // write the font matrix (if not part of the DA string)
            if (tmPos < 0) {
                appearBuf->appendf("1 0 0 1 {0:.2f} {1:.2f} Tm\n", x, y);
            }

            // write the text string
            const char *s = convertedText.c_str();
            int len = convertedText.getLength();
            i = 0;
            xPrev = w; // so that first character is placed properly
            while (i < comb && len > 0) {
                CharCode code;
                const Unicode *uAux;
                int uLen, n;
                double char_dx, char_dy, ox, oy;

                char_dx = 0.0;
                n = font->getNextChar(s, len, &code, &uAux, &uLen, &char_dx, &char_dy, &ox, &oy);
                char_dx *= fontSize;

                // center each character within its cell, by advancing the text
                // position the appropriate amount relative to the start of the
                // previous character
                x = 0.5 * (w - char_dx);
                appearBuf->appendf("{0:.2f} 0 Td\n", x - xPrev + w);

                GooString charBuf(s, n);
                writeString(charBuf);
                appearBuf->append(" Tj\n");

                i++;
                s += n;
                len -= n;
                xPrev = x;
            }

            // regular (non-comb) formatting
        } else {
            int ii = 0;
            Annot::layoutText(text, &convertedText, &ii, font, &w, 0.0, nullptr, forceZapfDingbats);

            // compute font autosize
            if (fontSize == 0) {
                fontSize = dy - 2 * borderWidth;
                if (w > 0) {
                    const double fontSize2 = (dx - 4 - 2 * borderWidth) / w;
                    if (fontSize2 < fontSize) {
                        fontSize = fontSize2;
                    }
                }
                fontSize = floor(fontSize);
                if (tfPos >= 0) {
                    tok = (*daToks)[tfPos + 1];
                    tok->clear();
                    tok->appendf("{0:.2f}", fontSize);
                }
            }

            // compute text start position
            w *= fontSize;
            switch (quadding) {
            case quaddingLeftJustified:
            default:
                x = borderWidth + 2;
                break;
            case quaddingCentered:
                x = (dx - w) / 2;
                break;
            case quaddingRightJustified:
                x = dx - borderWidth - 2 - w;
                break;
            }
            y = 0.5 * dy - 0.4 * fontSize;

            // set the font matrix
            if (tmPos >= 0) {
                tok = (*daToks)[tmPos + 4];
                tok->clear();
                tok->appendf("{0:.2f}", x);
                tok = (*daToks)[tmPos + 5];
                tok->clear();
                tok->appendf("{0:.2f}", y);
            }

            // write the DA string
            if (daToks) {
                for (const GooString *daTok : *daToks) {
                    appearBuf->append(daTok)->append(' ');
                }
            }

            // write the font matrix (if not part of the DA string)
            if (tmPos < 0) {
                appearBuf->appendf("1 0 0 1 {0:.2f} {1:.2f} Tm\n", x, y);
            }

            // write the text string
            writeString(convertedText);
            appearBuf->append(" Tj\n");
        }
    }
    // cleanup
    appearBuf->append("ET\n");
    appearBuf->append("Q\n");
    if (txField) {
        appearBuf->append("EMC\n");
    }
    if (daToks) {
        for (auto entry : *daToks) {
            delete entry;
        }
        delete daToks;
    }
    if (freeText) {
        delete text;
    }
    if (fontToFree) {
        fontToFree->decRefCnt();
    }

    return true;
}

// Draw the variable text or caption for a field.
bool AnnotAppearanceBuilder::drawListBox(const FormFieldChoice *fieldChoice, const AnnotBorder *border, const PDFRectangle *rect, const GooString *da, const GfxResources *resources, int quadding)
{
    std::vector<GooString *> *daToks;
    GooString *tok;
    GooString convertedText;
    const GfxFont *font;
    double fontSize, borderWidth, x, y, w, wMax;
    int tfPos, tmPos, i, j;

    //~ if there is no MK entry, this should use the existing content stream,
    //~ and only replace the marked content portion of it
    //~ (this is only relevant for Tx fields)

    // parse the default appearance string
    tfPos = tmPos = -1;
    if (da) {
        daToks = new std::vector<GooString *>();
        i = 0;
        while (i < da->getLength()) {
            while (i < da->getLength() && Lexer::isSpace(da->getChar(i))) {
                ++i;
            }
            if (i < da->getLength()) {
                for (j = i + 1; j < da->getLength() && !Lexer::isSpace(da->getChar(j)); ++j)
                    ;
                daToks->push_back(new GooString(da, i, j - i));
                i = j;
            }
        }
        for (std::size_t k = 2; k < daToks->size(); ++k) {
            if (k >= 2 && !((*daToks)[k])->cmp("Tf")) {
                tfPos = k - 2;
            } else if (k >= 6 && !((*daToks)[k])->cmp("Tm")) {
                tmPos = k - 6;
            }
        }
    } else {
        daToks = nullptr;
    }

    // get the font and font size
    font = nullptr;
    fontSize = 0;
    if (tfPos >= 0) {
        tok = (*daToks)[tfPos];
        if (tok->getLength() >= 1 && tok->getChar(0) == '/') {
            if (!resources || !(font = resources->lookupFont(tok->c_str() + 1))) {
                error(errSyntaxError, -1, "Unknown font in field's DA string");
            }
        } else {
            error(errSyntaxError, -1, "Invalid font name in 'Tf' operator in field's DA string");
        }
        tok = (*daToks)[tfPos + 1];
        fontSize = gatof(tok->c_str());
    } else {
        error(errSyntaxError, -1, "Missing 'Tf' operator in field's DA string");
    }
    if (!font) {
        if (daToks) {
            for (auto entry : *daToks) {
                delete entry;
            }
            delete daToks;
        }
        return false;
    }

    // get the border width
    borderWidth = border ? border->getWidth() : 0;

    // compute font autosize
    if (fontSize == 0) {
        wMax = 0;
        for (i = 0; i < fieldChoice->getNumChoices(); ++i) {
            j = 0;
            if (fieldChoice->getChoice(i) == nullptr) {
                error(errSyntaxError, -1, "Invalid annotation listbox");
                if (daToks) {
                    for (auto entry : *daToks) {
                        delete entry;
                    }
                    delete daToks;
                }
                return false;
            }
            Annot::layoutText(fieldChoice->getChoice(i), &convertedText, &j, font, &w, 0.0, nullptr, false);
            if (w > wMax) {
                wMax = w;
            }
        }
        fontSize = rect->y2 - rect->y1 - 2 * borderWidth;
        const double fontSize2 = (rect->x2 - rect->x1 - 4 - 2 * borderWidth) / wMax;
        if (fontSize2 < fontSize) {
            fontSize = fontSize2;
        }
        fontSize = floor(fontSize);
        if (tfPos >= 0) {
            tok = (*daToks)[tfPos + 1];
            tok->clear();
            tok->appendf("{0:.2f}", fontSize);
        }
    }
    // draw the text
    y = rect->y2 - rect->y1 - 1.1 * fontSize;
    for (i = fieldChoice->getTopIndex(); i < fieldChoice->getNumChoices(); ++i) {
        // setup
        appearBuf->append("q\n");

        // draw the background if selected
        if (fieldChoice->isSelected(i)) {
            appearBuf->append("0 g f\n");
            appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} re f\n", borderWidth, y - 0.2 * fontSize, rect->x2 - rect->x1 - 2 * borderWidth, 1.1 * fontSize);
        }

        // setup
        appearBuf->append("BT\n");

        // compute text width and start position
        j = 0;
        Annot::layoutText(fieldChoice->getChoice(i), &convertedText, &j, font, &w, 0.0, nullptr, false);
        w *= fontSize;
        switch (quadding) {
        case quaddingLeftJustified:
        default:
            x = borderWidth + 2;
            break;
        case quaddingCentered:
            x = (rect->x2 - rect->x1 - w) / 2;
            break;
        case quaddingRightJustified:
            x = rect->x2 - rect->x1 - borderWidth - 2 - w;
            break;
        }

        // set the font matrix
        if (tmPos >= 0) {
            tok = (*daToks)[tmPos + 4];
            tok->clear();
            tok->appendf("{0:.2f}", x);
            tok = (*daToks)[tmPos + 5];
            tok->clear();
            tok->appendf("{0:.2f}", y);
        }

        // write the DA string
        if (daToks) {
            for (const GooString *daTok : *daToks) {
                appearBuf->append(daTok)->append(' ');
            }
        }

        // write the font matrix (if not part of the DA string)
        if (tmPos < 0) {
            appearBuf->appendf("1 0 0 1 {0:.2f} {1:.2f} Tm\n", x, y);
        }

        // change the text color if selected
        if (fieldChoice->isSelected(i)) {
            appearBuf->append("1 g\n");
        }

        // write the text string
        writeString(convertedText);
        appearBuf->append(" Tj\n");

        // cleanup
        appearBuf->append("ET\n");
        appearBuf->append("Q\n");

        // next line
        y -= 1.1 * fontSize;
    }

    if (daToks) {
        for (auto entry : *daToks) {
            delete entry;
        }
        delete daToks;
    }

    return true;
}

void AnnotAppearanceBuilder::drawFieldBorder(const FormField *field, const AnnotBorder *border, const AnnotAppearanceCharacs *appearCharacs, const PDFRectangle *rect)
{
    int dashLength;
    double *dash;
    AnnotColor adjustedColor;
    const double w = border->getWidth();

    const AnnotColor *aColor = appearCharacs->getBorderColor();
    if (!aColor)
        aColor = appearCharacs->getBackColor();
    if (!aColor)
        return;

    const double dx = rect->x2 - rect->x1;
    const double dy = rect->y2 - rect->y1;

    // radio buttons with no caption have a round border
    const bool hasCaption = appearCharacs->getNormalCaption() != nullptr;
    if (field->getType() == formButton && static_cast<const FormFieldButton *>(field)->getButtonType() == formButtonRadio && !hasCaption) {
        double r = 0.5 * (dx < dy ? dx : dy);
        switch (border->getStyle()) {
        case AnnotBorder::borderDashed:
            appearBuf->append("[");
            dashLength = border->getDashLength();
            dash = border->getDash();
            for (int i = 0; i < dashLength; ++i) {
                appearBuf->appendf(" {0:.2f}", dash[i]);
            }
            appearBuf->append("] 0 d\n");
            // fallthrough
        case AnnotBorder::borderSolid:
        case AnnotBorder::borderUnderlined:
            appearBuf->appendf("{0:.2f} w\n", w);
            setDrawColor(aColor, false);
            drawCircle(0.5 * dx, 0.5 * dy, r - 0.5 * w, false);
            break;
        case AnnotBorder::borderBeveled:
        case AnnotBorder::borderInset:
            appearBuf->appendf("{0:.2f} w\n", 0.5 * w);
            setDrawColor(aColor, false);
            drawCircle(0.5 * dx, 0.5 * dy, r - 0.25 * w, false);
            adjustedColor = AnnotColor(*aColor);
            adjustedColor.adjustColor(border->getStyle() == AnnotBorder::borderBeveled ? 1 : -1);
            setDrawColor(&adjustedColor, false);
            drawCircleTopLeft(0.5 * dx, 0.5 * dy, r - 0.75 * w);
            adjustedColor = AnnotColor(*aColor);
            adjustedColor.adjustColor(border->getStyle() == AnnotBorder::borderBeveled ? -1 : 1);
            setDrawColor(&adjustedColor, false);
            drawCircleBottomRight(0.5 * dx, 0.5 * dy, r - 0.75 * w);
            break;
        }
    } else {
        switch (border->getStyle()) {
        case AnnotBorder::borderDashed:
            appearBuf->append("[");
            dashLength = border->getDashLength();
            dash = border->getDash();
            for (int i = 0; i < dashLength; ++i) {
                appearBuf->appendf(" {0:.2f}", dash[i]);
            }
            appearBuf->append("] 0 d\n");
            // fallthrough
        case AnnotBorder::borderSolid:
            appearBuf->appendf("{0:.2f} w\n", w);
            setDrawColor(aColor, false);
            appearBuf->appendf("{0:.2f} {0:.2f} {1:.2f} {2:.2f} re s\n", 0.5 * w, dx - w, dy - w);
            break;
        case AnnotBorder::borderBeveled:
        case AnnotBorder::borderInset:
            adjustedColor = AnnotColor(*aColor);
            adjustedColor.adjustColor(border->getStyle() == AnnotBorder::borderBeveled ? 1 : -1);
            setDrawColor(&adjustedColor, true);
            appearBuf->append("0 0 m\n");
            appearBuf->appendf("0 {0:.2f} l\n", dy);
            appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx, dy);
            appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx - w, dy - w);
            appearBuf->appendf("{0:.2f} {1:.2f} l\n", w, dy - w);
            appearBuf->appendf("{0:.2f} {0:.2f} l\n", w);
            appearBuf->append("f\n");
            adjustedColor = AnnotColor(*aColor);
            adjustedColor.adjustColor(border->getStyle() == AnnotBorder::borderBeveled ? -1 : 1);
            setDrawColor(&adjustedColor, true);
            appearBuf->append("0 0 m\n");
            appearBuf->appendf("{0:.2f} 0 l\n", dx);
            appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx, dy);
            appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx - w, dy - w);
            appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx - w, w);
            appearBuf->appendf("{0:.2f} {0:.2f} l\n", w);
            appearBuf->append("f\n");
            break;
        case AnnotBorder::borderUnderlined:
            appearBuf->appendf("{0:.2f} w\n", w);
            setDrawColor(aColor, false);
            appearBuf->appendf("0 0 m {0:.2f} 0 l s\n", dx);
            break;
        }

        // clip to the inside of the border
        appearBuf->appendf("{0:.2f} {0:.2f} {1:.2f} {2:.2f} re W n\n", w, dx - 2 * w, dy - 2 * w);
    }
}

bool AnnotAppearanceBuilder::drawFormField(const FormField *field, const Form *form, const GfxResources *resources, const GooString *da, const AnnotBorder *border, const AnnotAppearanceCharacs *appearCharacs, const PDFRectangle *rect,
                                           const GooString *appearState, XRef *xref, bool *addedDingbatsResource)
{
    // draw the field contents
    switch (field->getType()) {
    case formButton:
        return drawFormFieldButton(static_cast<const FormFieldButton *>(field), resources, da, border, appearCharacs, rect, appearState, xref, addedDingbatsResource);
        break;
    case formText:
        return drawFormFieldText(static_cast<const FormFieldText *>(field), form, resources, da, border, appearCharacs, rect);
    case formChoice:
        return drawFormFieldChoice(static_cast<const FormFieldChoice *>(field), form, resources, da, border, appearCharacs, rect);
        break;
    case formSignature:
        //~unimp
        break;
    case formUndef:
    default:
        error(errSyntaxError, -1, "Unknown field type");
    }

    return false;
}

bool AnnotAppearanceBuilder::drawFormFieldButton(const FormFieldButton *field, const GfxResources *resources, const GooString *da, const AnnotBorder *border, const AnnotAppearanceCharacs *appearCharacs, const PDFRectangle *rect,
                                                 const GooString *appearState, XRef *xref, bool *addedDingbatsResource)
{
    const GooString *caption = nullptr;
    if (appearCharacs)
        caption = appearCharacs->getNormalCaption();

    switch (field->getButtonType()) {
    case formButtonRadio: {
        //~ Acrobat doesn't draw a caption if there is no AP dict (?)
        if (appearState && appearState->cmp("Off") != 0 && field->getState(appearState->c_str())) {
            if (caption) {
                return drawText(caption, da, resources, border, appearCharacs, rect, false, 0, fieldQuadCenter, false, true, xref, addedDingbatsResource, false);
            } else if (appearCharacs) {
                const AnnotColor *aColor = appearCharacs->getBorderColor();
                if (aColor) {
                    const double dx = rect->x2 - rect->x1;
                    const double dy = rect->y2 - rect->y1;
                    setDrawColor(aColor, true);
                    drawCircle(0.5 * dx, 0.5 * dy, 0.2 * (dx < dy ? dx : dy), true);
                }
                return true;
            }
        }
    } break;
    case formButtonPush:
        if (caption)
            return drawText(caption, da, resources, border, appearCharacs, rect, false, 0, fieldQuadCenter, false, false, xref, addedDingbatsResource, false);
        break;
    case formButtonCheck:
        if (appearState && appearState->cmp("Off") != 0) {
            if (!caption) {
                GooString checkMark("3");
                return drawText(&checkMark, da, resources, border, appearCharacs, rect, false, 0, fieldQuadCenter, false, true, xref, addedDingbatsResource, false);
            } else {
                return drawText(caption, da, resources, border, appearCharacs, rect, false, 0, fieldQuadCenter, false, true, xref, addedDingbatsResource, false);
            }
        }
        break;
    }

    return true;
}

bool AnnotAppearanceBuilder::drawFormFieldText(const FormFieldText *fieldText, const Form *form, const GfxResources *resources, const GooString *da, const AnnotBorder *border, const AnnotAppearanceCharacs *appearCharacs,
                                               const PDFRectangle *rect)
{
    VariableTextQuadding quadding;
    const GooString *contents;

    contents = fieldText->getAppearanceContent();
    if (contents) {
        quadding = fieldText->hasTextQuadding() ? fieldText->getTextQuadding() : form->getTextQuadding();

        int comb = 0;
        if (fieldText->isComb())
            comb = fieldText->getMaxLen();

        return drawText(contents, da, resources, border, appearCharacs, rect, fieldText->isMultiline(), comb, quadding, true, false, nullptr, nullptr, fieldText->isPassword());
    }

    return true;
}

bool AnnotAppearanceBuilder::drawFormFieldChoice(const FormFieldChoice *fieldChoice, const Form *form, const GfxResources *resources, const GooString *da, const AnnotBorder *border, const AnnotAppearanceCharacs *appearCharacs,
                                                 const PDFRectangle *rect)
{
    const GooString *selected;
    VariableTextQuadding quadding;

    quadding = fieldChoice->hasTextQuadding() ? fieldChoice->getTextQuadding() : form->getTextQuadding();

    if (fieldChoice->isCombo()) {
        selected = fieldChoice->getSelectedChoice();
        if (selected) {
            return drawText(selected, da, resources, border, appearCharacs, rect, false, 0, quadding, true, false, nullptr, nullptr, false);
            //~ Acrobat draws a popup icon on the right side
        }
        // list box
    } else {
        return drawListBox(fieldChoice, border, rect, da, resources, quadding);
    }

    return true;
}

void AnnotWidget::generateFieldAppearance(bool *addedDingbatsResource)
{
    GfxResources *resources;
    const GooString *da;

    AnnotAppearanceBuilder appearBuilder;

    // draw the background
    if (appearCharacs) {
        const AnnotColor *aColor = appearCharacs->getBackColor();
        if (aColor) {
            appearBuilder.setDrawColor(aColor, true);
            appearBuilder.appendf("0 0 {0:.2f} {1:.2f} re f\n", rect->x2 - rect->x1, rect->y2 - rect->y1);
        }
    }

    // draw the border
    if (appearCharacs && border && border->getWidth() > 0)
        appearBuilder.drawFieldBorder(field, border.get(), appearCharacs.get(), rect.get());

    da = field->getDefaultAppearance();
    if (!da)
        da = form->getDefaultAppearance();

    resources = form->getDefaultResources();

    const bool success = appearBuilder.drawFormField(field, form, resources, da, border.get(), appearCharacs.get(), rect.get(), appearState.get(), doc->getXRef(), addedDingbatsResource);
    if (!success && da != form->getDefaultAppearance()) {
        da = form->getDefaultAppearance();
        appearBuilder.drawFormField(field, form, resources, da, border.get(), appearCharacs.get(), rect.get(), appearState.get(), doc->getXRef(), addedDingbatsResource);
    }

    const GooString *appearBuf = appearBuilder.buffer();
    // build the appearance stream dictionary
    Dict *appearDict = new Dict(doc->getXRef());
    appearDict->add("Length", Object(appearBuf->getLength()));
    appearDict->add("Subtype", Object(objName, "Form"));
    Array *bbox = new Array(doc->getXRef());
    bbox->add(Object(0));
    bbox->add(Object(0));
    bbox->add(Object(rect->x2 - rect->x1));
    bbox->add(Object(rect->y2 - rect->y1));
    appearDict->add("BBox", Object(bbox));

    // set the resource dictionary
    Object *resDict = form->getDefaultResourcesObj();
    if (resDict->isDict()) {
        appearDict->add("Resources", resDict->copy());
    }

    // build the appearance stream
    Stream *appearStream = new AutoFreeMemStream(copyString(appearBuf->c_str()), 0, appearBuf->getLength(), Object(appearDict));
    appearance = Object(appearStream);
}

void AnnotWidget::updateAppearanceStream()
{
    // If this the first time updateAppearanceStream() is called on this widget,
    // destroy the AP dictionary because we are going to create a new one.
    if (updatedAppearanceStream == Ref::INVALID()) {
        invalidateAppearance(); // Delete AP dictionary and all referenced streams
    }

    // There's no need to create a new appearance stream if NeedAppearances is
    // set, because it will be ignored next time anyway.
    if (form && form->getNeedAppearances())
        return;

    // Create the new appearance
    bool dummyAddDingbatsResource = false; // This is only update so if we didn't need to add
                                           // the dingbats resource we should not need it now
    generateFieldAppearance(&dummyAddDingbatsResource);

    // Fetch the appearance stream we've just created
    Object obj1 = appearance.fetch(doc->getXRef());

    // If this the first time updateAppearanceStream() is called on this widget,
    // create a new AP dictionary containing the new appearance stream.
    // Otherwise, just update the stream we had created previously.
    if (updatedAppearanceStream == Ref::INVALID()) {
        // Write the appearance stream
        updatedAppearanceStream = doc->getXRef()->addIndirectObject(&obj1);

        // Write the AP dictionary
        obj1 = Object(new Dict(doc->getXRef()));
        obj1.dictAdd("N", Object(updatedAppearanceStream));

        // Update our internal pointers to the appearance dictionary
        appearStreams = std::make_unique<AnnotAppearance>(doc, &obj1);

        update("AP", std::move(obj1));
    } else {
        // Replace the existing appearance stream
        doc->getXRef()->setModifiedObject(&obj1, updatedAppearanceStream);
    }
}

void AnnotWidget::draw(Gfx *gfx, bool printing)
{
    if (!isVisible(printing))
        return;

    annotLocker();
    bool addDingbatsResource = false;

    // Only construct the appearance stream when
    // - annot doesn't have an AP or
    // - NeedAppearances is true
    if (field) {
        if (appearance.isNull() || (form && form->getNeedAppearances()))
            generateFieldAppearance(&addDingbatsResource);
    }

    // draw the appearance stream
    Object obj = appearance.fetch(gfx->getXRef());
    if (addDingbatsResource) {
        // We are forcing ZaDb but the font does not exist
        // so create a fake one
        Dict *fontDict = new Dict(gfx->getXRef());
        fontDict->add("BaseFont", Object(objName, "ZapfDingbats"));
        fontDict->add("Subtype", Object(objName, "Type1"));

        Dict *fontsDict = new Dict(gfx->getXRef());
        fontsDict->add("ZaDb", Object(fontDict));

        Dict *dict = new Dict(gfx->getXRef());
        dict->add("Font", Object(fontsDict));
        gfx->pushResources(dict);
        delete dict;
    }
    gfx->drawAnnot(&obj, nullptr, color.get(), rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
    if (addDingbatsResource) {
        gfx->popResources();
    }
}

//------------------------------------------------------------------------
// AnnotMovie
//------------------------------------------------------------------------
AnnotMovie::AnnotMovie(PDFDoc *docA, PDFRectangle *rectA, Movie *movieA) : Annot(docA, rectA)
{
    type = typeMovie;
    annotObj.dictSet("Subtype", Object(objName, "Movie"));

    movie.reset(movieA->copy());
    // TODO: create movie dict from movieA

    initialize(docA, annotObj.getDict());
}

AnnotMovie::AnnotMovie(PDFDoc *docA, Object &&dictObject, const Object *obj) : Annot(docA, std::move(dictObject), obj)
{
    type = typeMovie;
    initialize(docA, annotObj.getDict());
}

AnnotMovie::~AnnotMovie() = default;

void AnnotMovie::initialize(PDFDoc *docA, Dict *dict)
{
    Object obj1;

    obj1 = dict->lookup("T");
    if (obj1.isString()) {
        title.reset(obj1.getString()->copy());
    }

    Object movieDict = dict->lookup("Movie");
    if (movieDict.isDict()) {
        Object obj2 = dict->lookup("A");
        if (obj2.isDict())
            movie = std::make_unique<Movie>(&movieDict, &obj2);
        else
            movie = std::make_unique<Movie>(&movieDict);
        if (!movie->isOk()) {
            movie = nullptr;
            ok = false;
        }
    } else {
        error(errSyntaxError, -1, "Bad Annot Movie");
        ok = false;
    }
}

void AnnotMovie::draw(Gfx *gfx, bool printing)
{
    if (!isVisible(printing))
        return;

    annotLocker();
    if (appearance.isNull() && movie->getShowPoster()) {
        int width, height;
        Object poster = movie->getPoster();
        movie->getAspect(&width, &height);

        if (width != -1 && height != -1 && !poster.isNone()) {
            auto appearBuf = std::make_unique<GooString>();
            appearBuf->append("q\n");
            appearBuf->appendf("{0:d} 0 0 {1:d} 0 0 cm\n", width, height);
            appearBuf->append("/MImg Do\n");
            appearBuf->append("Q\n");

            Dict *imgDict = new Dict(gfx->getXRef());
            imgDict->set("MImg", std::move(poster));

            Dict *resDict = new Dict(gfx->getXRef());
            resDict->set("XObject", Object(imgDict));

            Dict *formDict = new Dict(gfx->getXRef());
            formDict->set("Length", Object(appearBuf->getLength()));
            formDict->set("Subtype", Object(objName, "Form"));
            formDict->set("Name", Object(objName, "FRM"));
            Array *bboxArray = new Array(gfx->getXRef());
            bboxArray->add(Object(0));
            bboxArray->add(Object(0));
            bboxArray->add(Object(width));
            bboxArray->add(Object(height));
            formDict->set("BBox", Object(bboxArray));
            Array *matrix = new Array(gfx->getXRef());
            matrix->add(Object(1));
            matrix->add(Object(0));
            matrix->add(Object(0));
            matrix->add(Object(1));
            matrix->add(Object(-width / 2));
            matrix->add(Object(-height / 2));
            formDict->set("Matrix", Object(matrix));
            formDict->set("Resources", Object(resDict));

            Stream *mStream = new AutoFreeMemStream(copyString(appearBuf->c_str()), 0, appearBuf->getLength(), Object(formDict));

            Dict *dict = new Dict(gfx->getXRef());
            dict->set("FRM", Object(mStream));

            Dict *resDict2 = new Dict(gfx->getXRef());
            resDict2->set("XObject", Object(dict));

            appearBuf = std::make_unique<GooString>();
            appearBuf->append("q\n");
            appearBuf->appendf("0 0 {0:d} {1:d} re W n\n", width, height);
            appearBuf->append("q\n");
            appearBuf->appendf("0 0 {0:d} {1:d} re W n\n", width, height);
            appearBuf->appendf("1 0 0 1 {0:d} {1:d} cm\n", width / 2, height / 2);
            appearBuf->append("/FRM Do\n");
            appearBuf->append("Q\n");
            appearBuf->append("Q\n");

            double bbox[4];
            bbox[0] = bbox[1] = 0;
            bbox[2] = width;
            bbox[3] = height;
            appearance = createForm(appearBuf.get(), bbox, false, resDict2);
        }
    }

    // draw the appearance stream
    Object obj = appearance.fetch(gfx->getXRef());
    gfx->drawAnnot(&obj, nullptr, color.get(), rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
}

//------------------------------------------------------------------------
// AnnotScreen
//------------------------------------------------------------------------
AnnotScreen::AnnotScreen(PDFDoc *docA, PDFRectangle *rectA) : Annot(docA, rectA)
{
    type = typeScreen;

    annotObj.dictSet("Subtype", Object(objName, "Screen"));
    initialize(docA, annotObj.getDict());
}

AnnotScreen::AnnotScreen(PDFDoc *docA, Object &&dictObject, const Object *obj) : Annot(docA, std::move(dictObject), obj)
{
    type = typeScreen;
    initialize(docA, annotObj.getDict());
}

AnnotScreen::~AnnotScreen() = default;

void AnnotScreen::initialize(PDFDoc *docA, Dict *dict)
{
    Object obj1;

    obj1 = dict->lookup("T");
    if (obj1.isString()) {
        title.reset(obj1.getString()->copy());
    }

    obj1 = dict->lookup("A");
    if (obj1.isDict()) {
        action = LinkAction::parseAction(&obj1, doc->getCatalog()->getBaseURI());
        if (action && action->getKind() == actionRendition && page == 0) {
            error(errSyntaxError, -1, "Invalid Rendition action: associated screen annotation without P");
            action = nullptr;
            ok = false;
        }
    }

    additionalActions = dict->lookupNF("AA").copy();

    obj1 = dict->lookup("MK");
    if (obj1.isDict()) {
        appearCharacs = std::make_unique<AnnotAppearanceCharacs>(obj1.getDict());
    }
}

std::unique_ptr<LinkAction> AnnotScreen::getAdditionalAction(AdditionalActionsType additionalActionType)
{
    if (additionalActionType == actionFocusIn || additionalActionType == actionFocusOut) // not defined for screen annotation
        return nullptr;

    return ::getAdditionalAction(additionalActionType, &additionalActions, doc);
}

//------------------------------------------------------------------------
// AnnotStamp
//------------------------------------------------------------------------
AnnotStamp::AnnotStamp(PDFDoc *docA, PDFRectangle *rectA) : AnnotMarkup(docA, rectA)
{
    type = typeStamp;
    annotObj.dictSet("Subtype", Object(objName, "Stamp"));
    initialize(docA, annotObj.getDict());
}

AnnotStamp::AnnotStamp(PDFDoc *docA, Object &&dictObject, const Object *obj) : AnnotMarkup(docA, std::move(dictObject), obj)
{
    type = typeStamp;
    initialize(docA, annotObj.getDict());
}

AnnotStamp::~AnnotStamp() = default;

void AnnotStamp::initialize(PDFDoc *docA, Dict *dict)
{
    Object obj1 = dict->lookup("Name");
    if (obj1.isName()) {
        icon = std::make_unique<GooString>(obj1.getName());
    } else {
        icon = std::make_unique<GooString>("Draft");
    }
}

void AnnotStamp::setIcon(GooString *new_icon)
{
    if (new_icon) {
        icon = std::make_unique<GooString>(new_icon);
    } else {
        icon = std::make_unique<GooString>();
    }

    update("Name", Object(objName, icon->c_str()));
    invalidateAppearance();
}

//------------------------------------------------------------------------
// AnnotGeometry
//------------------------------------------------------------------------
AnnotGeometry::AnnotGeometry(PDFDoc *docA, PDFRectangle *rectA, AnnotSubtype subType) : AnnotMarkup(docA, rectA)
{
    switch (subType) {
    case typeSquare:
        annotObj.dictSet("Subtype", Object(objName, "Square"));
        break;
    case typeCircle:
        annotObj.dictSet("Subtype", Object(objName, "Circle"));
        break;
    default:
        assert(0 && "Invalid subtype for AnnotGeometry\n");
    }

    initialize(docA, annotObj.getDict());
}

AnnotGeometry::AnnotGeometry(PDFDoc *docA, Object &&dictObject, const Object *obj) : AnnotMarkup(docA, std::move(dictObject), obj)
{
    // the real type will be read in initialize()
    type = typeSquare;
    initialize(docA, annotObj.getDict());
}

AnnotGeometry::~AnnotGeometry() = default;

void AnnotGeometry::initialize(PDFDoc *docA, Dict *dict)
{
    Object obj1;

    obj1 = dict->lookup("Subtype");
    if (obj1.isName()) {
        GooString typeName(obj1.getName());
        if (!typeName.cmp("Square")) {
            type = typeSquare;
        } else if (!typeName.cmp("Circle")) {
            type = typeCircle;
        }
    }

    obj1 = dict->lookup("IC");
    if (obj1.isArray()) {
        interiorColor = std::make_unique<AnnotColor>(obj1.getArray());
    }

    obj1 = dict->lookup("BS");
    if (obj1.isDict()) {
        border = std::make_unique<AnnotBorderBS>(obj1.getDict());
    } else if (!border) {
        border = std::make_unique<AnnotBorderBS>();
    }

    obj1 = dict->lookup("BE");
    if (obj1.isDict()) {
        borderEffect = std::make_unique<AnnotBorderEffect>(obj1.getDict());
    }

    obj1 = dict->lookup("RD");
    if (obj1.isArray()) {
        geometryRect = parseDiffRectangle(obj1.getArray(), rect.get());
    }
}

void AnnotGeometry::setType(AnnotSubtype new_type)
{
    const char *typeName = nullptr; /* squelch bogus compiler warning */

    switch (new_type) {
    case typeSquare:
        typeName = "Square";
        break;
    case typeCircle:
        typeName = "Circle";
        break;
    default:
        assert(!"Invalid subtype");
    }

    type = new_type;
    update("Subtype", Object(objName, typeName));
    invalidateAppearance();
}

void AnnotGeometry::setInteriorColor(std::unique_ptr<AnnotColor> &&new_color)
{
    if (new_color) {
        Object obj1 = new_color->writeToObject(doc->getXRef());
        update("IC", std::move(obj1));
        interiorColor = std::move(new_color);
    } else {
        interiorColor = nullptr;
    }
    invalidateAppearance();
}

void AnnotGeometry::draw(Gfx *gfx, bool printing)
{
    double ca = 1;

    if (!isVisible(printing))
        return;

    annotLocker();
    if (appearance.isNull()) {
        ca = opacity;

        AnnotAppearanceBuilder appearBuilder;
        appearBuilder.append("q\n");
        if (color)
            appearBuilder.setDrawColor(color.get(), false);

        double borderWidth = border->getWidth();
        appearBuilder.setLineStyleForBorder(border.get());

        if (interiorColor)
            appearBuilder.setDrawColor(interiorColor.get(), true);

        if (type == typeSquare) {
            appearBuilder.appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} re\n", borderWidth / 2.0, borderWidth / 2.0, (rect->x2 - rect->x1) - borderWidth, (rect->y2 - rect->y1) - borderWidth);
        } else {
            double width, height;
            double b;
            double x1, y1, x2, y2, x3, y3;

            width = rect->x2 - rect->x1;
            height = rect->y2 - rect->y1;
            b = borderWidth / 2.0;

            x1 = b;
            y1 = height / 2.0;
            appearBuilder.appendf("{0:.2f} {1:.2f} m\n", x1, y1);

            y1 += height / 4.0;
            x2 = width / 4.0;
            y2 = height - b;
            x3 = width / 2.0;
            y3 = y2;
            appearBuilder.appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n", x1, y1, x2, y2, x3, y3);
            x2 = width - b;
            y2 = y1;
            x1 = x3 + (width / 4.0);
            y1 = y3;
            x3 = x2;
            y3 = height / 2.0;
            appearBuilder.appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n", x1, y1, x2, y2, x3, y3);

            x2 = x1;
            y2 = b;
            x1 = x3;
            y1 = height / 4.0;
            x3 = width / 2.0;
            y3 = b;
            appearBuilder.appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n", x1, y1, x2, y2, x3, y3);

            x2 = b;
            y2 = y1;
            x1 = width / 4.0;
            y1 = b;
            x3 = b;
            y3 = height / 2.0;
            appearBuilder.appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n", x1, y1, x2, y2, x3, y3);
        }

        if (interiorColor && interiorColor->getSpace() != AnnotColor::colorTransparent)
            appearBuilder.append("b\n");
        else
            appearBuilder.append("S\n");

        appearBuilder.append("Q\n");

        double bbox[4];
        bbox[0] = bbox[1] = 0;
        bbox[2] = rect->x2 - rect->x1;
        bbox[3] = rect->y2 - rect->y1;
        if (ca == 1) {
            appearance = createForm(appearBuilder.buffer(), bbox, false, nullptr);
        } else {
            Object aStream = createForm(appearBuilder.buffer(), bbox, true, nullptr);

            GooString appearBuf("/GS0 gs\n/Fm0 Do");
            Dict *resDict = createResourcesDict("Fm0", std::move(aStream), "GS0", ca, nullptr);
            appearance = createForm(&appearBuf, bbox, false, resDict);
        }
    }

    // draw the appearance stream
    Object obj = appearance.fetch(gfx->getXRef());
    gfx->drawAnnot(&obj, nullptr, color.get(), rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
}

//------------------------------------------------------------------------
// AnnotPolygon
//------------------------------------------------------------------------
AnnotPolygon::AnnotPolygon(PDFDoc *docA, PDFRectangle *rectA, AnnotSubtype subType) : AnnotMarkup(docA, rectA)
{
    switch (subType) {
    case typePolygon:
        annotObj.dictSet("Subtype", Object(objName, "Polygon"));
        break;
    case typePolyLine:
        annotObj.dictSet("Subtype", Object(objName, "PolyLine"));
        break;
    default:
        assert(0 && "Invalid subtype for AnnotGeometry\n");
    }

    // Store dummy path with one null vertex only
    Array *a = new Array(doc->getXRef());
    a->add(Object(0.));
    a->add(Object(0.));
    annotObj.dictSet("Vertices", Object(a));

    initialize(docA, annotObj.getDict());
}

AnnotPolygon::AnnotPolygon(PDFDoc *docA, Object &&dictObject, const Object *obj) : AnnotMarkup(docA, std::move(dictObject), obj)
{
    // the real type will be read in initialize()
    type = typePolygon;
    initialize(docA, annotObj.getDict());
}

AnnotPolygon::~AnnotPolygon() = default;

void AnnotPolygon::initialize(PDFDoc *docA, Dict *dict)
{
    Object obj1;

    obj1 = dict->lookup("Subtype");
    if (obj1.isName()) {
        GooString typeName(obj1.getName());
        if (!typeName.cmp("Polygon")) {
            type = typePolygon;
        } else if (!typeName.cmp("PolyLine")) {
            type = typePolyLine;
        }
    }

    obj1 = dict->lookup("Vertices");
    if (obj1.isArray()) {
        vertices = std::make_unique<AnnotPath>(obj1.getArray());
    } else {
        vertices = std::make_unique<AnnotPath>();
        error(errSyntaxError, -1, "Bad Annot Polygon Vertices");
        ok = false;
    }

    obj1 = dict->lookup("LE");
    if (obj1.isArray() && obj1.arrayGetLength() == 2) {
        Object obj2 = obj1.arrayGet(0);
        if (obj2.isName()) {
            const GooString leName(obj2.getName());
            startStyle = parseAnnotLineEndingStyle(&leName);
        } else {
            startStyle = annotLineEndingNone;
        }
        obj2 = obj1.arrayGet(1);
        if (obj2.isName()) {
            const GooString leName(obj2.getName());
            endStyle = parseAnnotLineEndingStyle(&leName);
        } else {
            endStyle = annotLineEndingNone;
        }
    } else {
        startStyle = endStyle = annotLineEndingNone;
    }

    obj1 = dict->lookup("IC");
    if (obj1.isArray()) {
        interiorColor = std::make_unique<AnnotColor>(obj1.getArray());
    }

    obj1 = dict->lookup("BS");
    if (obj1.isDict()) {
        border = std::make_unique<AnnotBorderBS>(obj1.getDict());
    } else if (!border) {
        border = std::make_unique<AnnotBorderBS>();
    }

    obj1 = dict->lookup("BE");
    if (obj1.isDict()) {
        borderEffect = std::make_unique<AnnotBorderEffect>(obj1.getDict());
    }

    obj1 = dict->lookup("IT");
    if (obj1.isName()) {
        const char *intentName = obj1.getName();

        if (!strcmp(intentName, "PolygonCloud")) {
            intent = polygonCloud;
        } else if (!strcmp(intentName, "PolyLineDimension")) {
            intent = polylineDimension;
        } else {
            intent = polygonDimension;
        }
    } else {
        intent = polygonCloud;
    }
}

void AnnotPolygon::setType(AnnotSubtype new_type)
{
    const char *typeName = nullptr; /* squelch bogus compiler warning */

    switch (new_type) {
    case typePolygon:
        typeName = "Polygon";
        break;
    case typePolyLine:
        typeName = "PolyLine";
        break;
    default:
        assert(!"Invalid subtype");
    }

    type = new_type;
    update("Subtype", Object(objName, typeName));
    invalidateAppearance();
}

void AnnotPolygon::setVertices(AnnotPath *path)
{
    Array *a = new Array(doc->getXRef());
    for (int i = 0; i < path->getCoordsLength(); i++) {
        a->add(Object(path->getX(i)));
        a->add(Object(path->getY(i)));
    }

    vertices = std::make_unique<AnnotPath>(a);

    update("Vertices", Object(a));
    invalidateAppearance();
}

void AnnotPolygon::setStartEndStyle(AnnotLineEndingStyle start, AnnotLineEndingStyle end)
{
    startStyle = start;
    endStyle = end;

    Array *a = new Array(doc->getXRef());
    a->add(Object(objName, convertAnnotLineEndingStyle(startStyle)));
    a->add(Object(objName, convertAnnotLineEndingStyle(endStyle)));

    update("LE", Object(a));
    invalidateAppearance();
}

void AnnotPolygon::setInteriorColor(std::unique_ptr<AnnotColor> &&new_color)
{
    if (new_color) {
        Object obj1 = new_color->writeToObject(doc->getXRef());
        update("IC", std::move(obj1));
        interiorColor = std::move(new_color);
    }
    invalidateAppearance();
}

void AnnotPolygon::setIntent(AnnotPolygonIntent new_intent)
{
    const char *intentName;

    intent = new_intent;
    if (new_intent == polygonCloud)
        intentName = "PolygonCloud";
    else if (new_intent == polylineDimension)
        intentName = "PolyLineDimension";
    else // polygonDimension
        intentName = "PolygonDimension";
    update("IT", Object(objName, intentName));
}

void AnnotPolygon::generatePolyLineAppearance(AnnotAppearanceBuilder *appearBuilder)
{
    const bool fill = (bool)interiorColor;
    const double x1 = vertices->getX(0);
    const double y1 = vertices->getY(0);
    const double x2 = vertices->getX(1);
    const double y2 = vertices->getY(1);
    const double x3 = vertices->getX(vertices->getCoordsLength() - 2);
    const double y3 = vertices->getY(vertices->getCoordsLength() - 2);
    const double x4 = vertices->getX(vertices->getCoordsLength() - 1);
    const double y4 = vertices->getY(vertices->getCoordsLength() - 1);

    const double len_1 = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
    // length of last segment
    const double len_2 = sqrt((x4 - x3) * (x4 - x3) + (y4 - y3) * (y4 - y3));

    // segments become positive x direction, coord1 becomes (0,0).
    Matrix matr1, matr2;
    const double angle1 = atan2(y2 - y1, x2 - x1);
    const double angle2 = atan2(y4 - y3, x4 - x3);

    matr1.m[0] = matr1.m[3] = cos(angle1);
    matr1.m[1] = sin(angle1);
    matr1.m[2] = -matr1.m[1];
    matr1.m[4] = x1 - rect->x1;
    matr1.m[5] = y1 - rect->y1;

    matr2.m[0] = matr2.m[3] = cos(angle2);
    matr2.m[1] = sin(angle2);
    matr2.m[2] = -matr2.m[1];
    matr2.m[4] = x3 - rect->x1;
    matr2.m[5] = y3 - rect->y1;

    const double lineEndingSize1 { std::min(6. * border->getWidth(), len_1 / 2) };
    const double lineEndingSize2 { std::min(6. * border->getWidth(), len_2 / 2) };

    if (vertices->getCoordsLength() != 0) {
        double tx, ty;
        matr1.transform(AnnotAppearanceBuilder::lineEndingXShorten(startStyle, lineEndingSize1), 0, &tx, &ty);
        appearBuilder->appendf("{0:.2f} {1:.2f} m\n", tx, ty);
        appearBBox->extendTo(tx, ty);

        for (int i = 1; i < vertices->getCoordsLength() - 1; ++i) {
            appearBuilder->appendf("{0:.2f} {1:.2f} l\n", vertices->getX(i) - rect->x1, vertices->getY(i) - rect->y1);
            appearBBox->extendTo(vertices->getX(i) - rect->x1, vertices->getY(i) - rect->y1);
        }

        if (vertices->getCoordsLength() > 1) {
            matr2.transform(len_2 - AnnotAppearanceBuilder::lineEndingXShorten(endStyle, lineEndingSize2), 0, &tx, &ty);
            appearBuilder->appendf("{0:.2f} {1:.2f} l S\n", tx, ty);
            appearBBox->extendTo(tx, ty);
        }
    }

    if (startStyle != annotLineEndingNone) {
        const double extendX { -AnnotAppearanceBuilder::lineEndingXExtendBBox(startStyle, lineEndingSize1) };
        double tx, ty;
        appearBuilder->drawLineEnding(startStyle, 0, 0, -lineEndingSize1, fill, matr1);
        matr1.transform(extendX, lineEndingSize1 / 2., &tx, &ty);
        appearBBox->extendTo(tx, ty);
        matr1.transform(extendX, -lineEndingSize1 / 2., &tx, &ty);
        appearBBox->extendTo(tx, ty);
    }

    if (endStyle != annotLineEndingNone) {
        const double extendX { AnnotAppearanceBuilder::lineEndingXExtendBBox(endStyle, lineEndingSize2) };
        double tx, ty;
        appearBuilder->drawLineEnding(endStyle, len_2, 0, lineEndingSize2, fill, matr2);
        matr2.transform(len_2 + extendX, lineEndingSize2 / 2., &tx, &ty);
        appearBBox->extendTo(tx, ty);
        matr2.transform(len_2 + extendX, -lineEndingSize2 / 2., &tx, &ty);
        appearBBox->extendTo(tx, ty);
    }
}

void AnnotPolygon::draw(Gfx *gfx, bool printing)
{
    double ca = 1;

    if (!isVisible(printing))
        return;

    annotLocker();
    if (appearance.isNull()) {
        appearBBox = std::make_unique<AnnotAppearanceBBox>(rect.get());
        ca = opacity;

        AnnotAppearanceBuilder appearBuilder;
        appearBuilder.append("q\n");

        if (color) {
            appearBuilder.setDrawColor(color.get(), false);
        }

        appearBuilder.setLineStyleForBorder(border.get());
        appearBBox->setBorderWidth(std::max(1., border->getWidth()));

        if (interiorColor) {
            appearBuilder.setDrawColor(interiorColor.get(), true);
        }

        if (type == typePolyLine) {
            generatePolyLineAppearance(&appearBuilder);
        } else {
            if (vertices->getCoordsLength() != 0) {
                appearBuilder.appendf("{0:.2f} {1:.2f} m\n", vertices->getX(0) - rect->x1, vertices->getY(0) - rect->y1);
                appearBBox->extendTo(vertices->getX(0) - rect->x1, vertices->getY(0) - rect->y1);

                for (int i = 1; i < vertices->getCoordsLength(); ++i) {
                    appearBuilder.appendf("{0:.2f} {1:.2f} l\n", vertices->getX(i) - rect->x1, vertices->getY(i) - rect->y1);
                    appearBBox->extendTo(vertices->getX(i) - rect->x1, vertices->getY(i) - rect->y1);
                }

                if (interiorColor && interiorColor->getSpace() != AnnotColor::colorTransparent) {
                    appearBuilder.append("b\n");
                } else {
                    appearBuilder.append("s\n");
                }
            }
        }
        appearBuilder.append("Q\n");

        double bbox[4];
        appearBBox->getBBoxRect(bbox);
        if (ca == 1) {
            appearance = createForm(appearBuilder.buffer(), bbox, false, nullptr);
        } else {
            Object aStream = createForm(appearBuilder.buffer(), bbox, true, nullptr);

            GooString appearBuf("/GS0 gs\n/Fm0 Do");
            Dict *resDict = createResourcesDict("Fm0", std::move(aStream), "GS0", ca, nullptr);
            appearance = createForm(&appearBuf, bbox, false, resDict);
        }
    }

    // draw the appearance stream
    Object obj = appearance.fetch(gfx->getXRef());
    if (appearBBox) {
        gfx->drawAnnot(&obj, nullptr, color.get(), appearBBox->getPageXMin(), appearBBox->getPageYMin(), appearBBox->getPageXMax(), appearBBox->getPageYMax(), getRotation());
    } else {
        gfx->drawAnnot(&obj, nullptr, color.get(), rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
    }
}

//------------------------------------------------------------------------
// AnnotCaret
//------------------------------------------------------------------------
AnnotCaret::AnnotCaret(PDFDoc *docA, PDFRectangle *rectA) : AnnotMarkup(docA, rectA)
{
    type = typeCaret;

    annotObj.dictSet("Subtype", Object(objName, "Caret"));
    initialize(docA, annotObj.getDict());
}

AnnotCaret::AnnotCaret(PDFDoc *docA, Object &&dictObject, const Object *obj) : AnnotMarkup(docA, std::move(dictObject), obj)
{
    type = typeCaret;
    initialize(docA, annotObj.getDict());
}

AnnotCaret::~AnnotCaret() = default;

void AnnotCaret::initialize(PDFDoc *docA, Dict *dict)
{
    Object obj1;

    symbol = symbolNone;
    obj1 = dict->lookup("Sy");
    if (obj1.isName()) {
        GooString typeName(obj1.getName());
        if (!typeName.cmp("P")) {
            symbol = symbolP;
        } else if (!typeName.cmp("None")) {
            symbol = symbolNone;
        }
    }

    obj1 = dict->lookup("RD");
    if (obj1.isArray()) {
        caretRect = parseDiffRectangle(obj1.getArray(), rect.get());
    }
}

void AnnotCaret::setSymbol(AnnotCaretSymbol new_symbol)
{
    symbol = new_symbol;
    update("Sy", Object(objName, new_symbol == symbolP ? "P" : "None"));
    invalidateAppearance();
}

//------------------------------------------------------------------------
// AnnotInk
//------------------------------------------------------------------------
AnnotInk::AnnotInk(PDFDoc *docA, PDFRectangle *rectA) : AnnotMarkup(docA, rectA)
{
    type = typeInk;

    annotObj.dictSet("Subtype", Object(objName, "Ink"));

    // Store dummy path with one null vertex only
    Array *inkListArray = new Array(doc->getXRef());
    Array *vList = new Array(doc->getXRef());
    vList->add(Object(0.));
    vList->add(Object(0.));
    inkListArray->add(Object(vList));
    annotObj.dictSet("InkList", Object(inkListArray));

    initialize(docA, annotObj.getDict());
}

AnnotInk::AnnotInk(PDFDoc *docA, Object &&dictObject, const Object *obj) : AnnotMarkup(docA, std::move(dictObject), obj)
{
    type = typeInk;
    initialize(docA, annotObj.getDict());
}

AnnotInk::~AnnotInk()
{
    freeInkList();
}

void AnnotInk::initialize(PDFDoc *docA, Dict *dict)
{
    Object obj1;

    obj1 = dict->lookup("InkList");
    if (obj1.isArray()) {
        parseInkList(obj1.getArray());
    } else {
        inkListLength = 0;
        inkList = nullptr;
        error(errSyntaxError, -1, "Bad Annot Ink List");

        obj1 = dict->lookup("AP");
        // Although InkList is required, it should be ignored
        // when there is an AP entry in the Annot, so do not fail
        // when that happens
        if (!obj1.isDict()) {
            ok = false;
        }
    }

    obj1 = dict->lookup("BS");
    if (obj1.isDict()) {
        border = std::make_unique<AnnotBorderBS>(obj1.getDict());
    } else if (!border) {
        border = std::make_unique<AnnotBorderBS>();
    }
}

void AnnotInk::writeInkList(AnnotPath **paths, int n_paths, Array *dest_array)
{
    for (int i = 0; i < n_paths; ++i) {
        AnnotPath *path = paths[i];
        Array *a = new Array(doc->getXRef());
        for (int j = 0; j < path->getCoordsLength(); ++j) {
            a->add(Object(path->getX(j)));
            a->add(Object(path->getY(j)));
        }
        dest_array->add(Object(a));
    }
}

void AnnotInk::parseInkList(Array *array)
{
    inkListLength = array->getLength();
    inkList = (AnnotPath **)gmallocn((inkListLength), sizeof(AnnotPath *));
    memset(inkList, 0, inkListLength * sizeof(AnnotPath *));
    for (int i = 0; i < inkListLength; i++) {
        Object obj2 = array->get(i);
        if (obj2.isArray())
            inkList[i] = new AnnotPath(obj2.getArray());
    }
}

void AnnotInk::freeInkList()
{
    if (inkList) {
        for (int i = 0; i < inkListLength; ++i)
            delete inkList[i];
        gfree(inkList);
    }
}

void AnnotInk::setInkList(AnnotPath **paths, int n_paths)
{
    freeInkList();

    Array *a = new Array(doc->getXRef());
    writeInkList(paths, n_paths, a);

    parseInkList(a);
    annotObj.dictSet("InkList", Object(a));
    invalidateAppearance();
}

void AnnotInk::draw(Gfx *gfx, bool printing)
{
    double ca = 1;

    if (!isVisible(printing))
        return;

    annotLocker();
    if (appearance.isNull()) {
        appearBBox = std::make_unique<AnnotAppearanceBBox>(rect.get());
        ca = opacity;

        AnnotAppearanceBuilder appearBuilder;
        appearBuilder.append("q\n");

        if (color) {
            appearBuilder.setDrawColor(color.get(), false);
        }

        appearBuilder.setLineStyleForBorder(border.get());
        appearBBox->setBorderWidth(std::max(1., border->getWidth()));

        for (int i = 0; i < inkListLength; ++i) {
            const AnnotPath *path = inkList[i];
            if (path && path->getCoordsLength() != 0) {
                appearBuilder.appendf("{0:.2f} {1:.2f} m\n", path->getX(0) - rect->x1, path->getY(0) - rect->y1);
                appearBBox->extendTo(path->getX(0) - rect->x1, path->getY(0) - rect->y1);

                for (int j = 1; j < path->getCoordsLength(); ++j) {
                    appearBuilder.appendf("{0:.2f} {1:.2f} l\n", path->getX(j) - rect->x1, path->getY(j) - rect->y1);
                    appearBBox->extendTo(path->getX(j) - rect->x1, path->getY(j) - rect->y1);
                }

                appearBuilder.append("S\n");
            }
        }

        appearBuilder.append("Q\n");

        double bbox[4];
        appearBBox->getBBoxRect(bbox);
        if (ca == 1) {
            appearance = createForm(appearBuilder.buffer(), bbox, false, nullptr);
        } else {
            Object aStream = createForm(appearBuilder.buffer(), bbox, true, nullptr);

            GooString appearBuf("/GS0 gs\n/Fm0 Do");
            Dict *resDict = createResourcesDict("Fm0", std::move(aStream), "GS0", ca, nullptr);
            appearance = createForm(&appearBuf, bbox, false, resDict);
        }
    }

    // draw the appearance stream
    Object obj = appearance.fetch(gfx->getXRef());
    if (appearBBox) {
        gfx->drawAnnot(&obj, nullptr, color.get(), appearBBox->getPageXMin(), appearBBox->getPageYMin(), appearBBox->getPageXMax(), appearBBox->getPageYMax(), getRotation());
    } else {
        gfx->drawAnnot(&obj, nullptr, color.get(), rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
    }
}

//------------------------------------------------------------------------
// AnnotFileAttachment
//------------------------------------------------------------------------
AnnotFileAttachment::AnnotFileAttachment(PDFDoc *docA, PDFRectangle *rectA, GooString *filename) : AnnotMarkup(docA, rectA)
{
    type = typeFileAttachment;

    annotObj.dictSet("Subtype", Object(objName, "FileAttachment"));
    annotObj.dictSet("FS", Object(filename->copy()));

    initialize(docA, annotObj.getDict());
}

AnnotFileAttachment::AnnotFileAttachment(PDFDoc *docA, Object &&dictObject, const Object *obj) : AnnotMarkup(docA, std::move(dictObject), obj)
{
    type = typeFileAttachment;
    initialize(docA, annotObj.getDict());
}

AnnotFileAttachment::~AnnotFileAttachment() = default;

void AnnotFileAttachment::initialize(PDFDoc *docA, Dict *dict)
{
    Object objFS = dict->lookup("FS");
    if (objFS.isDict() || objFS.isString()) {
        file = std::move(objFS);
    } else {
        error(errSyntaxError, -1, "Bad Annot File Attachment");
        ok = false;
    }

    Object objName = dict->lookup("Name");
    if (objName.isName()) {
        name = std::make_unique<GooString>(objName.getName());
    } else {
        name = std::make_unique<GooString>("PushPin");
    }
}

#define ANNOT_FILE_ATTACHMENT_AP_PUSHPIN                                                                                                                                                                                                       \
    "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"                                                                                                                                                                   \
    "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"                                                                                                                                                                 \
    "l 1 21.523 2.477 23 4.301 23 c h\n"                                                                                                                                                                                                       \
    "4.301 23 m f\n"                                                                                                                                                                                                                           \
    "0.533333 0.541176 0.521569 RG 2 w\n"                                                                                                                                                                                                      \
    "1 J\n"                                                                                                                                                                                                                                    \
    "1 j\n"                                                                                                                                                                                                                                    \
    "[] 0.0 d\n"                                                                                                                                                                                                                               \
    "4 M 5 4 m 6 5 l S\n"                                                                                                                                                                                                                      \
    "2 w\n"                                                                                                                                                                                                                                    \
    "11 14 m 9 12 l 6 12 l 13 5 l 13 8 l 15 10 l 18 11 l 20 11 l 12 19 l 12\n"                                                                                                                                                                 \
    "17 l 11 14 l h\n"                                                                                                                                                                                                                         \
    "11 14 m S\n"                                                                                                                                                                                                                              \
    "3 w\n"                                                                                                                                                                                                                                    \
    "6 5 m 9 8 l S\n"                                                                                                                                                                                                                          \
    "0.729412 0.741176 0.713725 RG 2 w\n"                                                                                                                                                                                                      \
    "5 5 m 6 6 l S\n"                                                                                                                                                                                                                          \
    "2 w\n"                                                                                                                                                                                                                                    \
    "11 15 m 9 13 l 6 13 l 13 6 l 13 9 l 15 11 l 18 12 l 20 12 l 12 20 l 12\n"                                                                                                                                                                 \
    "18 l 11 15 l h\n"                                                                                                                                                                                                                         \
    "11 15 m S\n"                                                                                                                                                                                                                              \
    "3 w\n"                                                                                                                                                                                                                                    \
    "6 6 m 9 9 l S\n"

#define ANNOT_FILE_ATTACHMENT_AP_PAPERCLIP                                                                                                                                                                                                     \
    "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"                                                                                                                                                                   \
    "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"                                                                                                                                                                 \
    "l 1 21.523 2.477 23 4.301 23 c h\n"                                                                                                                                                                                                       \
    "4.301 23 m f\n"                                                                                                                                                                                                                           \
    "0.533333 0.541176 0.521569 RG 2 w\n"                                                                                                                                                                                                      \
    "1 J\n"                                                                                                                                                                                                                                    \
    "1 j\n"                                                                                                                                                                                                                                    \
    "[] 0.0 d\n"                                                                                                                                                                                                                               \
    "4 M 16.645 12.035 m 12.418 7.707 l 10.902 6.559 6.402 11.203 8.09 12.562 c\n"                                                                                                                                                             \
    "14.133 18.578 l 14.949 19.387 16.867 19.184 17.539 18.465 c 20.551\n"                                                                                                                                                                     \
    "15.23 l 21.191 14.66 21.336 12.887 20.426 12.102 c 13.18 4.824 l 12.18\n"                                                                                                                                                                 \
    "3.82 6.25 2.566 4.324 4.461 c 3 6.395 3.383 11.438 4.711 12.801 c 9.648\n"                                                                                                                                                                \
    "17.887 l S\n"                                                                                                                                                                                                                             \
    "0.729412 0.741176 0.713725 RG 16.645 13.035 m 12.418 8.707 l\n"                                                                                                                                                                           \
    "10.902 7.559 6.402 12.203 8.09 13.562 c\n"                                                                                                                                                                                                \
    "14.133 19.578 l 14.949 20.387 16.867 20.184 17.539 19.465 c 20.551\n"                                                                                                                                                                     \
    "16.23 l 21.191 15.66 21.336 13.887 20.426 13.102 c 13.18 5.824 l 12.18\n"                                                                                                                                                                 \
    "4.82 6.25 3.566 4.324 5.461 c 3 7.395 3.383 12.438 4.711 13.801 c 9.648\n"                                                                                                                                                                \
    "18.887 l S\n"

#define ANNOT_FILE_ATTACHMENT_AP_GRAPH                                                                                                                                                                                                         \
    "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"                                                                                                                                                                   \
    "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"                                                                                                                                                                 \
    "l 1 21.523 2.477 23 4.301 23 c h\n"                                                                                                                                                                                                       \
    "4.301 23 m f\n"                                                                                                                                                                                                                           \
    "0.533333 0.541176 0.521569 RG 1 w\n"                                                                                                                                                                                                      \
    "1 J\n"                                                                                                                                                                                                                                    \
    "0 j\n"                                                                                                                                                                                                                                    \
    "[] 0.0 d\n"                                                                                                                                                                                                                               \
    "4 M 18.5 15.5 m 18.5 13.086 l 16.086 15.5 l 18.5 15.5 l h\n"                                                                                                                                                                              \
    "18.5 15.5 m S\n"                                                                                                                                                                                                                          \
    "7 7 m 10 11 l 13 9 l 18 15 l S\n"                                                                                                                                                                                                         \
    "0.729412 0.741176 0.713725 RG 7 8 m 10 12 l 13 10 l 18 16 l S\n"                                                                                                                                                                          \
    "18.5 16.5 m 18.5 14.086 l 16.086 16.5 l 18.5 16.5 l h\n"                                                                                                                                                                                  \
    "18.5 16.5 m S\n"                                                                                                                                                                                                                          \
    "0.533333 0.541176 0.521569 RG 2 w\n"                                                                                                                                                                                                      \
    "1 j\n"                                                                                                                                                                                                                                    \
    "3 19 m 3 3 l 21 3 l S\n"                                                                                                                                                                                                                  \
    "0.729412 0.741176 0.713725 RG 3 20 m 3 4 l 21 4 l S\n"

#define ANNOT_FILE_ATTACHMENT_AP_TAG                                                                                                                                                                                                           \
    "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"                                                                                                                                                                   \
    "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"                                                                                                                                                                 \
    "l 1 21.523 2.477 23 4.301 23 c h\n"                                                                                                                                                                                                       \
    "4.301 23 m f\n"                                                                                                                                                                                                                           \
    "0.533333 0.541176 0.521569 RG 0.999781 w\n"                                                                                                                                                                                               \
    "1 J\n"                                                                                                                                                                                                                                    \
    "1 j\n"                                                                                                                                                                                                                                    \
    "[] 0.0 d\n"                                                                                                                                                                                                                               \
    "4 M q 1 0 0 -1 0 24 cm\n"                                                                                                                                                                                                                 \
    "8.492 8.707 m 8.492 9.535 7.82 10.207 6.992 10.207 c 6.164 10.207 5.492\n"                                                                                                                                                                \
    "9.535 5.492 8.707 c 5.492 7.879 6.164 7.207 6.992 7.207 c 7.82 7.207\n"                                                                                                                                                                   \
    "8.492 7.879 8.492 8.707 c h\n"                                                                                                                                                                                                            \
    "8.492 8.707 m S Q\n"                                                                                                                                                                                                                      \
    "2 w\n"                                                                                                                                                                                                                                    \
    "20.078 11.414 m 20.891 10.602 20.785 9.293 20.078 8.586 c 14.422 2.93 l\n"                                                                                                                                                                \
    "13.715 2.223 12.301 2.223 11.594 2.93 c 3.816 10.707 l 3.109 11.414\n"                                                                                                                                                                    \
    "2.402 17.781 3.816 19.195 c 5.23 20.609 11.594 19.902 12.301 19.195 c\n"                                                                                                                                                                  \
    "20.078 11.414 l h\n"                                                                                                                                                                                                                      \
    "20.078 11.414 m S\n"                                                                                                                                                                                                                      \
    "0.729412 0.741176 0.713725 RG 20.078 12.414 m\n"                                                                                                                                                                                          \
    "20.891 11.605 20.785 10.293 20.078 9.586 c 14.422 3.93 l\n"                                                                                                                                                                               \
    "13.715 3.223 12.301 3.223 11.594 3.93 c 3.816 11.707 l 3.109 12.414\n"                                                                                                                                                                    \
    "2.402 18.781 3.816 20.195 c 5.23 21.609 11.594 20.902 12.301 20.195 c\n"                                                                                                                                                                  \
    "20.078 12.414 l h\n"                                                                                                                                                                                                                      \
    "20.078 12.414 m S\n"                                                                                                                                                                                                                      \
    "0.533333 0.541176 0.521569 RG 1 w\n"                                                                                                                                                                                                      \
    "0 j\n"                                                                                                                                                                                                                                    \
    "11.949 13.184 m 16.191 8.941 l S\n"                                                                                                                                                                                                       \
    "0.729412 0.741176 0.713725 RG 11.949 14.184 m 16.191 9.941 l S\n"                                                                                                                                                                         \
    "0.533333 0.541176 0.521569 RG 14.07 6.82 m 9.828 11.062 l S\n"                                                                                                                                                                            \
    "0.729412 0.741176 0.713725 RG 14.07 7.82 m 9.828 12.062 l S\n"                                                                                                                                                                            \
    "0.533333 0.541176 0.521569 RG 6.93 15.141 m 8 20 14.27 20.5 16 20.5 c\n"                                                                                                                                                                  \
    "18.094 20.504 19.5 20 19.5 18 c 19.5 16.699 20.91 16.418 22.5 16.5 c S\n"                                                                                                                                                                 \
    "0.729412 0.741176 0.713725 RG 0.999781 w\n"                                                                                                                                                                                               \
    "1 j\n"                                                                                                                                                                                                                                    \
    "q 1 0 0 -1 0 24 cm\n"                                                                                                                                                                                                                     \
    "8.492 7.707 m 8.492 8.535 7.82 9.207 6.992 9.207 c 6.164 9.207 5.492\n"                                                                                                                                                                   \
    "8.535 5.492 7.707 c 5.492 6.879 6.164 6.207 6.992 6.207 c 7.82 6.207\n"                                                                                                                                                                   \
    "8.492 6.879 8.492 7.707 c h\n"                                                                                                                                                                                                            \
    "8.492 7.707 m S Q\n"                                                                                                                                                                                                                      \
    "1 w\n"                                                                                                                                                                                                                                    \
    "0 j\n"                                                                                                                                                                                                                                    \
    "6.93 16.141 m 8 21 14.27 21.5 16 21.5 c 18.094 21.504 19.5 21 19.5 19 c\n"                                                                                                                                                                \
    "19.5 17.699 20.91 17.418 22.5 17.5 c S\n"

void AnnotFileAttachment::draw(Gfx *gfx, bool printing)
{
    double ca = 1;

    if (!isVisible(printing))
        return;

    annotLocker();
    if (appearance.isNull()) {
        ca = opacity;

        AnnotAppearanceBuilder appearBuilder;

        appearBuilder.append("q\n");
        if (color)
            appearBuilder.setDrawColor(color.get(), true);
        else
            appearBuilder.append("1 1 1 rg\n");
        if (!name->cmp("PushPin"))
            appearBuilder.append(ANNOT_FILE_ATTACHMENT_AP_PUSHPIN);
        else if (!name->cmp("Paperclip"))
            appearBuilder.append(ANNOT_FILE_ATTACHMENT_AP_PAPERCLIP);
        else if (!name->cmp("Graph"))
            appearBuilder.append(ANNOT_FILE_ATTACHMENT_AP_GRAPH);
        else if (!name->cmp("Tag"))
            appearBuilder.append(ANNOT_FILE_ATTACHMENT_AP_TAG);
        appearBuilder.append("Q\n");

        double bbox[4];
        bbox[0] = bbox[1] = 0;
        bbox[2] = bbox[3] = 24;
        if (ca == 1) {
            appearance = createForm(appearBuilder.buffer(), bbox, false, nullptr);
        } else {
            Object aStream = createForm(appearBuilder.buffer(), bbox, true, nullptr);

            GooString appearBuf("/GS0 gs\n/Fm0 Do");
            Dict *resDict = createResourcesDict("Fm0", std::move(aStream), "GS0", ca, nullptr);
            appearance = createForm(&appearBuf, bbox, false, resDict);
        }
    }

    // draw the appearance stream
    Object obj = appearance.fetch(gfx->getXRef());
    gfx->drawAnnot(&obj, nullptr, color.get(), rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
}

//------------------------------------------------------------------------
// AnnotSound
//------------------------------------------------------------------------
AnnotSound::AnnotSound(PDFDoc *docA, PDFRectangle *rectA, Sound *soundA) : AnnotMarkup(docA, rectA)
{
    type = typeSound;

    annotObj.dictSet("Subtype", Object(objName, "Sound"));
    annotObj.dictSet("Sound", soundA->getObject()->copy());

    initialize(docA, annotObj.getDict());
}

AnnotSound::AnnotSound(PDFDoc *docA, Object &&dictObject, const Object *obj) : AnnotMarkup(docA, std::move(dictObject), obj)
{
    type = typeSound;
    initialize(docA, annotObj.getDict());
}

AnnotSound::~AnnotSound() = default;

void AnnotSound::initialize(PDFDoc *docA, Dict *dict)
{
    Object obj1 = dict->lookup("Sound");

    sound = Sound::parseSound(&obj1);
    if (!sound) {
        error(errSyntaxError, -1, "Bad Annot Sound");
        ok = false;
    }

    obj1 = dict->lookup("Name");
    if (obj1.isName()) {
        name = std::make_unique<GooString>(obj1.getName());
    } else {
        name = std::make_unique<GooString>("Speaker");
    }
}

#define ANNOT_SOUND_AP_SPEAKER                                                                                                                                                                                                                 \
    "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"                                                                                                                                                                   \
    "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"                                                                                                                                                                 \
    "l 1 21.523 2.477 23 4.301 23 c h\n"                                                                                                                                                                                                       \
    "4.301 23 m f\n"                                                                                                                                                                                                                           \
    "0.533333 0.541176 0.521569 RG 2 w\n"                                                                                                                                                                                                      \
    "0 J\n"                                                                                                                                                                                                                                    \
    "1 j\n"                                                                                                                                                                                                                                    \
    "[] 0.0 d\n"                                                                                                                                                                                                                               \
    "4 M 4 14 m 4.086 8.043 l 7 8 l 11 4 l 11 18 l 7 14 l 4 14 l h\n"                                                                                                                                                                          \
    "4 14 m S\n"                                                                                                                                                                                                                               \
    "1 w\n"                                                                                                                                                                                                                                    \
    "1 J\n"                                                                                                                                                                                                                                    \
    "0 j\n"                                                                                                                                                                                                                                    \
    "13.699 15.398 m 14.699 13.398 14.699 9.398 13.699 7.398 c S\n"                                                                                                                                                                            \
    "18.199 19.398 m 21.199 17.398 21.199 5.398 18.199 3.398 c S\n"                                                                                                                                                                            \
    "16 17.398 m 18 16.398 18 7.398 16 5.398 c S\n"                                                                                                                                                                                            \
    "0.729412 0.741176 0.713725 RG 2 w\n"                                                                                                                                                                                                      \
    "0 J\n"                                                                                                                                                                                                                                    \
    "1 j\n"                                                                                                                                                                                                                                    \
    "4 15 m 4.086 9.043 l 7 9 l 11 5 l 11 19 l 7 15 l 4 15 l h\n"                                                                                                                                                                              \
    "4 15 m S\n"                                                                                                                                                                                                                               \
    "1 w\n"                                                                                                                                                                                                                                    \
    "1 J\n"                                                                                                                                                                                                                                    \
    "0 j\n"                                                                                                                                                                                                                                    \
    "13.699 16 m 14.699 14 14.699 10 13.699 8 c S\n"                                                                                                                                                                                           \
    "18.199 20 m 21.199 18 21.199 6 18.199 4 c S\n"                                                                                                                                                                                            \
    "16 18 m 18 17 18 8 16 6 c S\n"

#define ANNOT_SOUND_AP_MIC                                                                                                                                                                                                                     \
    "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"                                                                                                                                                                   \
    "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"                                                                                                                                                                 \
    "l 1 21.523 2.477 23 4.301 23 c h\n"                                                                                                                                                                                                       \
    "4.301 23 m f\n"                                                                                                                                                                                                                           \
    "0.533333 0.541176 0.521569 RG 2 w\n"                                                                                                                                                                                                      \
    "1 J\n"                                                                                                                                                                                                                                    \
    "0 j\n"                                                                                                                                                                                                                                    \
    "[] 0.0 d\n"                                                                                                                                                                                                                               \
    "4 M 12 20 m 12 20 l 13.656 20 15 18.656 15 17 c 15 13 l 15 11.344 13.656 10\n"                                                                                                                                                            \
    "12 10 c 12 10 l 10.344 10 9 11.344 9 13 c 9 17 l 9 18.656 10.344 20 12\n"                                                                                                                                                                 \
    "20 c h\n"                                                                                                                                                                                                                                 \
    "12 20 m S\n"                                                                                                                                                                                                                              \
    "1 w\n"                                                                                                                                                                                                                                    \
    "17.5 14.5 m 17.5 11.973 l 17.5 8.941 15.047 6.5 12 6.5 c 8.953 6.5 6.5\n"                                                                                                                                                                 \
    "8.941 6.5 11.973 c 6.5 14.5 l S\n"                                                                                                                                                                                                        \
    "2 w\n"                                                                                                                                                                                                                                    \
    "0 J\n"                                                                                                                                                                                                                                    \
    "12 6.52 m 12 3 l S\n"                                                                                                                                                                                                                     \
    "1 J\n"                                                                                                                                                                                                                                    \
    "8 3 m 16 3 l S\n"                                                                                                                                                                                                                         \
    "0.729412 0.741176 0.713725 RG 12 21 m 12 21 l 13.656 21 15 19.656 15 18 c\n"                                                                                                                                                              \
    "15 14 l 15 12.344 13.656 11 12 11 c 12 11 l 10.344 11 9 12.344 9 14 c\n"                                                                                                                                                                  \
    "9 18 l 9 19.656 10.344 21 12 21 c h\n"                                                                                                                                                                                                    \
    "12 21 m S\n"                                                                                                                                                                                                                              \
    "1 w\n"                                                                                                                                                                                                                                    \
    "17.5 15.5 m 17.5 12.973 l 17.5 9.941 15.047 7.5 12 7.5 c 8.953 7.5 6.5\n"                                                                                                                                                                 \
    "9.941 6.5 12.973 c 6.5 15.5 l S\n"                                                                                                                                                                                                        \
    "2 w\n"                                                                                                                                                                                                                                    \
    "0 J\n"                                                                                                                                                                                                                                    \
    "12 7.52 m 12 4 l S\n"                                                                                                                                                                                                                     \
    "1 J\n"                                                                                                                                                                                                                                    \
    "8 4 m 16 4 l S\n"

void AnnotSound::draw(Gfx *gfx, bool printing)
{
    Object obj;
    double ca = 1;

    if (!isVisible(printing))
        return;

    annotLocker();
    if (appearance.isNull()) {
        ca = opacity;

        AnnotAppearanceBuilder appearBuilder;

        appearBuilder.append("q\n");
        if (color)
            appearBuilder.setDrawColor(color.get(), true);
        else
            appearBuilder.append("1 1 1 rg\n");
        if (!name->cmp("Speaker"))
            appearBuilder.append(ANNOT_SOUND_AP_SPEAKER);
        else if (!name->cmp("Mic"))
            appearBuilder.append(ANNOT_SOUND_AP_MIC);
        appearBuilder.append("Q\n");

        double bbox[4];
        bbox[0] = bbox[1] = 0;
        bbox[2] = bbox[3] = 24;
        if (ca == 1) {
            appearance = createForm(appearBuilder.buffer(), bbox, false, nullptr);
        } else {
            Object aStream = createForm(appearBuilder.buffer(), bbox, true, nullptr);

            GooString appearBuf("/GS0 gs\n/Fm0 Do");
            Dict *resDict = createResourcesDict("Fm0", std::move(aStream), "GS0", ca, nullptr);
            appearance = createForm(&appearBuf, bbox, false, resDict);
        }
    }

    // draw the appearance stream
    obj = appearance.fetch(gfx->getXRef());
    gfx->drawAnnot(&obj, nullptr, color.get(), rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
}

//------------------------------------------------------------------------
// Annot3D
//------------------------------------------------------------------------
Annot3D::Annot3D(PDFDoc *docA, PDFRectangle *rectA) : Annot(docA, rectA)
{
    type = type3D;

    annotObj.dictSet("Subtype", Object(objName, "3D"));

    initialize(docA, annotObj.getDict());
}

Annot3D::Annot3D(PDFDoc *docA, Object &&dictObject, const Object *obj) : Annot(docA, std::move(dictObject), obj)
{
    type = type3D;
    initialize(docA, annotObj.getDict());
}

Annot3D::~Annot3D() = default;

void Annot3D::initialize(PDFDoc *docA, Dict *dict)
{
    Object obj1 = dict->lookup("3DA");
    if (obj1.isDict()) {
        activation = std::make_unique<Activation>(obj1.getDict());
    }
}

Annot3D::Activation::Activation(Dict *dict)
{
    Object obj1;

    obj1 = dict->lookup("A");
    if (obj1.isName()) {
        const char *name = obj1.getName();

        if (!strcmp(name, "PO")) {
            aTrigger = aTriggerPageOpened;
        } else if (!strcmp(name, "PV")) {
            aTrigger = aTriggerPageVisible;
        } else if (!strcmp(name, "XA")) {
            aTrigger = aTriggerUserAction;
        } else {
            aTrigger = aTriggerUnknown;
        }
    } else {
        aTrigger = aTriggerUnknown;
    }

    obj1 = dict->lookup("AIS");
    if (obj1.isName()) {
        const char *name = obj1.getName();

        if (!strcmp(name, "I")) {
            aState = aStateEnabled;
        } else if (!strcmp(name, "L")) {
            aState = aStateDisabled;
        } else {
            aState = aStateUnknown;
        }
    } else {
        aState = aStateUnknown;
    }

    obj1 = dict->lookup("D");
    if (obj1.isName()) {
        const char *name = obj1.getName();

        if (!strcmp(name, "PC")) {
            dTrigger = dTriggerPageClosed;
        } else if (!strcmp(name, "PI")) {
            dTrigger = dTriggerPageInvisible;
        } else if (!strcmp(name, "XD")) {
            dTrigger = dTriggerUserAction;
        } else {
            dTrigger = dTriggerUnknown;
        }
    } else {
        dTrigger = dTriggerUnknown;
    }

    obj1 = dict->lookup("DIS");
    if (obj1.isName()) {
        const char *name = obj1.getName();

        if (!strcmp(name, "U")) {
            dState = dStateUninstantiaded;
        } else if (!strcmp(name, "I")) {
            dState = dStateInstantiated;
        } else if (!strcmp(name, "L")) {
            dState = dStateLive;
        } else {
            dState = dStateUnknown;
        }
    } else {
        dState = dStateUnknown;
    }

    displayToolbar = dict->lookup("TB").getBoolWithDefaultValue(true);

    displayNavigation = dict->lookup("NP").getBoolWithDefaultValue(false);
}

//------------------------------------------------------------------------
// AnnotRichMedia
//------------------------------------------------------------------------
AnnotRichMedia::AnnotRichMedia(PDFDoc *docA, PDFRectangle *rectA) : Annot(docA, rectA)
{
    type = typeRichMedia;

    annotObj.dictSet("Subtype", Object(objName, "RichMedia"));

    initialize(docA, annotObj.getDict());
}

AnnotRichMedia::AnnotRichMedia(PDFDoc *docA, Object &&dictObject, const Object *obj) : Annot(docA, std::move(dictObject), obj)
{
    type = typeRichMedia;
    initialize(docA, annotObj.getDict());
}

AnnotRichMedia::~AnnotRichMedia() = default;

void AnnotRichMedia::initialize(PDFDoc *docA, Dict *dict)
{
    Object obj1 = dict->lookup("RichMediaContent");
    if (obj1.isDict()) {
        content = std::make_unique<AnnotRichMedia::Content>(obj1.getDict());
    }

    obj1 = dict->lookup("RichMediaSettings");
    if (obj1.isDict()) {
        settings = std::make_unique<AnnotRichMedia::Settings>(obj1.getDict());
    }
}

AnnotRichMedia::Content *AnnotRichMedia::getContent() const
{
    return content.get();
}

AnnotRichMedia::Settings *AnnotRichMedia::getSettings() const
{
    return settings.get();
}

AnnotRichMedia::Settings::Settings(Dict *dict)
{
    Object obj1 = dict->lookup("Activation");
    if (obj1.isDict()) {
        activation = std::make_unique<AnnotRichMedia::Activation>(obj1.getDict());
    }

    obj1 = dict->lookup("Deactivation");
    if (obj1.isDict()) {
        deactivation = std::make_unique<AnnotRichMedia::Deactivation>(obj1.getDict());
    }
}

AnnotRichMedia::Settings::~Settings() = default;

AnnotRichMedia::Activation *AnnotRichMedia::Settings::getActivation() const
{
    return activation.get();
}

AnnotRichMedia::Deactivation *AnnotRichMedia::Settings::getDeactivation() const
{
    return deactivation.get();
}

AnnotRichMedia::Activation::Activation(Dict *dict)
{
    Object obj1 = dict->lookup("Condition");
    if (obj1.isName()) {
        const char *name = obj1.getName();

        if (!strcmp(name, "PO")) {
            condition = conditionPageOpened;
        } else if (!strcmp(name, "PV")) {
            condition = conditionPageVisible;
        } else if (!strcmp(name, "XA")) {
            condition = conditionUserAction;
        } else {
            condition = conditionUserAction;
        }
    } else {
        condition = conditionUserAction;
    }
}

AnnotRichMedia::Activation::Condition AnnotRichMedia::Activation::getCondition() const
{
    return condition;
}

AnnotRichMedia::Deactivation::Deactivation(Dict *dict)
{
    Object obj1 = dict->lookup("Condition");
    if (obj1.isName()) {
        const char *name = obj1.getName();

        if (!strcmp(name, "PC")) {
            condition = conditionPageClosed;
        } else if (!strcmp(name, "PI")) {
            condition = conditionPageInvisible;
        } else if (!strcmp(name, "XD")) {
            condition = conditionUserAction;
        } else {
            condition = conditionUserAction;
        }
    } else {
        condition = conditionUserAction;
    }
}

AnnotRichMedia::Deactivation::Condition AnnotRichMedia::Deactivation::getCondition() const
{
    return condition;
}

AnnotRichMedia::Content::Content(Dict *dict)
{
    Object obj1 = dict->lookup("Configurations");
    if (obj1.isArray()) {
        nConfigurations = obj1.arrayGetLength();

        configurations = (Configuration **)gmallocn(nConfigurations, sizeof(Configuration *));

        for (int i = 0; i < nConfigurations; ++i) {
            Object obj2 = obj1.arrayGet(i);
            if (obj2.isDict()) {
                configurations[i] = new AnnotRichMedia::Configuration(obj2.getDict());
            } else {
                configurations[i] = nullptr;
            }
        }
    } else {
        nConfigurations = 0;
        configurations = nullptr;
    }

    nAssets = 0;
    assets = nullptr;
    obj1 = dict->lookup("Assets");
    if (obj1.isDict()) {
        Object obj2 = obj1.getDict()->lookup("Names");
        if (obj2.isArray()) {
            const int length = obj2.arrayGetLength() / 2;

            assets = (Asset **)gmallocn(length, sizeof(Asset *));
            for (int i = 0; i < length; ++i) {
                Object objKey = obj2.arrayGet(2 * i);
                Object objVal = obj2.arrayGet(2 * i + 1);

                if (!objKey.isString() || objVal.isNull()) {
                    error(errSyntaxError, -1, "Bad Annot Asset");
                    continue;
                }

                assets[nAssets] = new AnnotRichMedia::Asset;
                assets[nAssets]->name = std::make_unique<GooString>(objKey.getString());
                assets[nAssets]->fileSpec = std::move(objVal);
                ++nAssets;
            }
        }
    }
}

AnnotRichMedia::Content::~Content()
{
    if (configurations) {
        for (int i = 0; i < nConfigurations; ++i)
            delete configurations[i];
        gfree(configurations);
    }

    if (assets) {
        for (int i = 0; i < nAssets; ++i)
            delete assets[i];
        gfree(assets);
    }
}

int AnnotRichMedia::Content::getConfigurationsCount() const
{
    return nConfigurations;
}

AnnotRichMedia::Configuration *AnnotRichMedia::Content::getConfiguration(int index) const
{
    if (index < 0 || index >= nConfigurations)
        return nullptr;

    return configurations[index];
}

int AnnotRichMedia::Content::getAssetsCount() const
{
    return nAssets;
}

AnnotRichMedia::Asset *AnnotRichMedia::Content::getAsset(int index) const
{
    if (index < 0 || index >= nAssets)
        return nullptr;

    return assets[index];
}

AnnotRichMedia::Asset::Asset() = default;

AnnotRichMedia::Asset::~Asset() = default;

const GooString *AnnotRichMedia::Asset::getName() const
{
    return name.get();
}

Object *AnnotRichMedia::Asset::getFileSpec() const
{
    return const_cast<Object *>(&fileSpec);
}

AnnotRichMedia::Configuration::Configuration(Dict *dict)
{
    Object obj1 = dict->lookup("Instances");
    if (obj1.isArray()) {
        nInstances = obj1.arrayGetLength();

        instances = (Instance **)gmallocn(nInstances, sizeof(Instance *));

        for (int i = 0; i < nInstances; ++i) {
            Object obj2 = obj1.arrayGet(i);
            if (obj2.isDict()) {
                instances[i] = new AnnotRichMedia::Instance(obj2.getDict());
            } else {
                instances[i] = nullptr;
            }
        }
    } else {
        instances = nullptr;
    }

    obj1 = dict->lookup("Name");
    if (obj1.isString()) {
        name = std::make_unique<GooString>(obj1.getString());
    }

    obj1 = dict->lookup("Subtype");
    if (obj1.isName()) {
        const char *subtypeName = obj1.getName();

        if (!strcmp(subtypeName, "3D")) {
            type = type3D;
        } else if (!strcmp(subtypeName, "Flash")) {
            type = typeFlash;
        } else if (!strcmp(subtypeName, "Sound")) {
            type = typeSound;
        } else if (!strcmp(subtypeName, "Video")) {
            type = typeVideo;
        } else {
            // determine from first non null instance
            type = typeFlash; // default in case all instances are null
            if (instances && nInstances > 0) {
                for (int i = 0; i < nInstances; ++i) {
                    AnnotRichMedia::Instance *instance = instances[i];
                    if (instance) {
                        switch (instance->getType()) {
                        case AnnotRichMedia::Instance::type3D:
                            type = type3D;
                            break;
                        case AnnotRichMedia::Instance::typeFlash:
                            type = typeFlash;
                            break;
                        case AnnotRichMedia::Instance::typeSound:
                            type = typeSound;
                            break;
                        case AnnotRichMedia::Instance::typeVideo:
                            type = typeVideo;
                            break;
                        }
                        // break the loop since we found the first non null instance
                        break;
                    }
                }
            }
        }
    }
}

AnnotRichMedia::Configuration::~Configuration()
{
    if (instances) {
        for (int i = 0; i < nInstances; ++i)
            delete instances[i];
        gfree(instances);
    }
}

int AnnotRichMedia::Configuration::getInstancesCount() const
{
    return nInstances;
}

AnnotRichMedia::Instance *AnnotRichMedia::Configuration::getInstance(int index) const
{
    if (index < 0 || index >= nInstances)
        return nullptr;

    return instances[index];
}

const GooString *AnnotRichMedia::Configuration::getName() const
{
    return name.get();
}

AnnotRichMedia::Configuration::Type AnnotRichMedia::Configuration::getType() const
{
    return type;
}

AnnotRichMedia::Instance::Instance(Dict *dict)
{
    Object obj1 = dict->lookup("Subtype");
    const char *name = obj1.isName() ? obj1.getName() : "";

    if (!strcmp(name, "3D")) {
        type = type3D;
    } else if (!strcmp(name, "Flash")) {
        type = typeFlash;
    } else if (!strcmp(name, "Sound")) {
        type = typeSound;
    } else if (!strcmp(name, "Video")) {
        type = typeVideo;
    } else {
        type = typeFlash;
    }

    obj1 = dict->lookup("Params");
    if (obj1.isDict()) {
        params = std::make_unique<AnnotRichMedia::Params>(obj1.getDict());
    }
}

AnnotRichMedia::Instance::~Instance() = default;

AnnotRichMedia::Instance::Type AnnotRichMedia::Instance::getType() const
{
    return type;
}

AnnotRichMedia::Params *AnnotRichMedia::Instance::getParams() const
{
    return params.get();
}

AnnotRichMedia::Params::Params(Dict *dict)
{
    Object obj1 = dict->lookup("FlashVars");
    if (obj1.isString()) {
        flashVars = std::make_unique<GooString>(obj1.getString());
    }
}

AnnotRichMedia::Params::~Params() = default;

const GooString *AnnotRichMedia::Params::getFlashVars() const
{
    return flashVars.get();
}

//------------------------------------------------------------------------
// Annots
//------------------------------------------------------------------------

Annots::Annots(PDFDoc *docA, int page, Object *annotsObj)
{
    Annot *annot;
    int i;

    doc = docA;

    if (annotsObj->isArray()) {
        for (i = 0; i < annotsObj->arrayGetLength(); ++i) {
            // get the Ref to this annot and pass it to Annot constructor
            // this way, it'll be possible for the annot to retrieve the corresponding
            // form widget
            Object obj1 = annotsObj->arrayGet(i);
            if (obj1.isDict()) {
                const Object &obj2 = annotsObj->arrayGetNF(i);
                annot = createAnnot(std::move(obj1), &obj2);
                if (annot) {
                    if (annot->isOk()) {
                        annot->setPage(page, false); // Don't change /P
                        appendAnnot(annot);
                    }
                    annot->decRefCnt();
                }
            }
        }
    }
}

void Annots::appendAnnot(Annot *annot)
{
    if (annot && annot->isOk()) {
        annots.push_back(annot);
        annot->incRefCnt();
    }
}

bool Annots::removeAnnot(Annot *annot)
{
    auto idx = std::find(annots.begin(), annots.end(), annot);

    if (idx == annots.end()) {
        return false;
    } else {
        annot->decRefCnt();
        annots.erase(idx);
        return true;
    }
}

Annot *Annots::createAnnot(Object &&dictObject, const Object *obj)
{
    Annot *annot = nullptr;
    Object obj1 = dictObject.dictLookup("Subtype");
    if (obj1.isName()) {
        const char *typeName = obj1.getName();

        if (!strcmp(typeName, "Text")) {
            annot = new AnnotText(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "Link")) {
            annot = new AnnotLink(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "FreeText")) {
            annot = new AnnotFreeText(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "Line")) {
            annot = new AnnotLine(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "Square")) {
            annot = new AnnotGeometry(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "Circle")) {
            annot = new AnnotGeometry(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "Polygon")) {
            annot = new AnnotPolygon(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "PolyLine")) {
            annot = new AnnotPolygon(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "Highlight")) {
            annot = new AnnotTextMarkup(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "Underline")) {
            annot = new AnnotTextMarkup(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "Squiggly")) {
            annot = new AnnotTextMarkup(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "StrikeOut")) {
            annot = new AnnotTextMarkup(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "Stamp")) {
            annot = new AnnotStamp(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "Caret")) {
            annot = new AnnotCaret(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "Ink")) {
            annot = new AnnotInk(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "FileAttachment")) {
            annot = new AnnotFileAttachment(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "Sound")) {
            annot = new AnnotSound(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "Movie")) {
            annot = new AnnotMovie(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "Widget")) {
            // Find the annot in forms
            if (obj->isRef()) {
                Form *form = doc->getCatalog()->getForm();
                if (form) {
                    FormWidget *widget = form->findWidgetByRef(obj->getRef());
                    if (widget) {
                        annot = widget->getWidgetAnnotation();
                        annot->incRefCnt();
                    }
                }
            }
            if (!annot)
                annot = new AnnotWidget(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "Screen")) {
            annot = new AnnotScreen(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "PrinterMark")) {
            annot = new Annot(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "TrapNet")) {
            annot = new Annot(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "Watermark")) {
            annot = new Annot(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "3D")) {
            annot = new Annot3D(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "RichMedia")) {
            annot = new AnnotRichMedia(doc, std::move(dictObject), obj);
        } else if (!strcmp(typeName, "Popup")) {
            /* Popup annots are already handled by markup annots
             * Here we only care about popup annots without a
             * markup annotation associated
             */
            Object obj2 = dictObject.dictLookup("Parent");
            if (obj2.isNull())
                annot = new AnnotPopup(doc, std::move(dictObject), obj);
            else
                annot = nullptr;
        } else {
            annot = new Annot(doc, std::move(dictObject), obj);
        }
    }

    return annot;
}

Annot *Annots::findAnnot(Ref *ref)
{
    for (auto *annot : annots) {
        if (annot->match(ref)) {
            return annot;
        }
    }
    return nullptr;
}

Annots::~Annots()
{
    for (auto *annot : annots) {
        annot->decRefCnt();
    }
}

//------------------------------------------------------------------------
// AnnotAppearanceBuilder
//------------------------------------------------------------------------

AnnotAppearanceBuilder::AnnotAppearanceBuilder() : appearBuf(new GooString()) { }

AnnotAppearanceBuilder::~AnnotAppearanceBuilder()
{
    delete appearBuf;
}

void AnnotAppearanceBuilder::append(const char *text)
{
    appearBuf->append(text);
}

void AnnotAppearanceBuilder::appendf(const char *fmt, ...) GOOSTRING_FORMAT
{
    va_list argList;

    va_start(argList, fmt);
    appearBuf->appendfv(fmt, argList);
    va_end(argList);
}

const GooString *AnnotAppearanceBuilder::buffer() const
{
    return appearBuf;
}
