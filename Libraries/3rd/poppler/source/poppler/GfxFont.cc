//========================================================================
//
// GfxFont.cc
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
// Copyright (C) 2005, 2006, 2008-2010, 2012, 2014, 2015, 2017-2023 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2005, 2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2006 Takashi Iwai <tiwai@suse.de>
// Copyright (C) 2007 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2007 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2007 Koji Otani <sho@bbr.jp>
// Copyright (C) 2007 Ed Catmur <ed@catmur.co.uk>
// Copyright (C) 2008 Jonathan Kew <jonathan_kew@sil.org>
// Copyright (C) 2008 Ed Avis <eda@waniasset.com>
// Copyright (C) 2008, 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2009 Peter Kerzum <kerzum@yandex-team.ru>
// Copyright (C) 2009, 2010 David Benjamin <davidben@mit.edu>
// Copyright (C) 2011 Axel Strübing <axel.struebing@freenet.de>
// Copyright (C) 2011, 2012, 2014 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2012 Yi Yang <ahyangyi@gmail.com>
// Copyright (C) 2012 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
// Copyright (C) 2012, 2017 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2013-2016, 2018 Jason Crain <jason@aquaticape.us>
// Copyright (C) 2014 Olly Betts <olly@survex.com>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019 LE GARREC Vincent <legarrec.vincent@gmail.com>
// Copyright (C) 2021, 2022 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright (C) 2023 Khaled Hosny <khaled@aliftype.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cmath>
#include <climits>
#include <algorithm>
#include "goo/gmem.h"
#include "Error.h"
#include "Object.h"
#include "Dict.h"
#include "GlobalParams.h"
#include "CMap.h"
#include "CharCodeToUnicode.h"
#include "FontEncodingTables.h"
#include "BuiltinFont.h"
#include "UnicodeTypeTable.h"
#include <fofi/FoFiIdentifier.h>
#include <fofi/FoFiType1.h>
#include <fofi/FoFiType1C.h>
#include <fofi/FoFiTrueType.h>
#include "GfxFont.h"
#include "PSOutputDev.h"

//------------------------------------------------------------------------

struct Base14FontMapEntry
{
    const char *altName;
    const char *base14Name;
};

static const Base14FontMapEntry base14FontMap[] = { { "Arial", "Helvetica" },
                                                    { "Arial,Bold", "Helvetica-Bold" },
                                                    { "Arial,BoldItalic", "Helvetica-BoldOblique" },
                                                    { "Arial,Italic", "Helvetica-Oblique" },
                                                    { "Arial-Bold", "Helvetica-Bold" },
                                                    { "Arial-BoldItalic", "Helvetica-BoldOblique" },
                                                    { "Arial-BoldItalicMT", "Helvetica-BoldOblique" },
                                                    { "Arial-BoldMT", "Helvetica-Bold" },
                                                    { "Arial-Italic", "Helvetica-Oblique" },
                                                    { "Arial-ItalicMT", "Helvetica-Oblique" },
                                                    { "ArialMT", "Helvetica" },
                                                    { "Courier", "Courier" },
                                                    { "Courier,Bold", "Courier-Bold" },
                                                    { "Courier,BoldItalic", "Courier-BoldOblique" },
                                                    { "Courier,Italic", "Courier-Oblique" },
                                                    { "Courier-Bold", "Courier-Bold" },
                                                    { "Courier-BoldOblique", "Courier-BoldOblique" },
                                                    { "Courier-Oblique", "Courier-Oblique" },
                                                    { "CourierNew", "Courier" },
                                                    { "CourierNew,Bold", "Courier-Bold" },
                                                    { "CourierNew,BoldItalic", "Courier-BoldOblique" },
                                                    { "CourierNew,Italic", "Courier-Oblique" },
                                                    { "CourierNew-Bold", "Courier-Bold" },
                                                    { "CourierNew-BoldItalic", "Courier-BoldOblique" },
                                                    { "CourierNew-Italic", "Courier-Oblique" },
                                                    { "CourierNewPS-BoldItalicMT", "Courier-BoldOblique" },
                                                    { "CourierNewPS-BoldMT", "Courier-Bold" },
                                                    { "CourierNewPS-ItalicMT", "Courier-Oblique" },
                                                    { "CourierNewPSMT", "Courier" },
                                                    { "Helvetica", "Helvetica" },
                                                    { "Helvetica,Bold", "Helvetica-Bold" },
                                                    { "Helvetica,BoldItalic", "Helvetica-BoldOblique" },
                                                    { "Helvetica,Italic", "Helvetica-Oblique" },
                                                    { "Helvetica-Bold", "Helvetica-Bold" },
                                                    { "Helvetica-BoldItalic", "Helvetica-BoldOblique" },
                                                    { "Helvetica-BoldOblique", "Helvetica-BoldOblique" },
                                                    { "Helvetica-Italic", "Helvetica-Oblique" },
                                                    { "Helvetica-Oblique", "Helvetica-Oblique" },
                                                    { "Symbol", "Symbol" },
                                                    { "Symbol,Bold", "Symbol" },
                                                    { "Symbol,BoldItalic", "Symbol" },
                                                    { "Symbol,Italic", "Symbol" },
                                                    { "SymbolMT", "Symbol" },
                                                    { "SymbolMT,Bold", "Symbol" },
                                                    { "SymbolMT,BoldItalic", "Symbol" },
                                                    { "SymbolMT,Italic", "Symbol" },
                                                    { "Times-Bold", "Times-Bold" },
                                                    { "Times-BoldItalic", "Times-BoldItalic" },
                                                    { "Times-Italic", "Times-Italic" },
                                                    { "Times-Roman", "Times-Roman" },
                                                    { "TimesNewRoman", "Times-Roman" },
                                                    { "TimesNewRoman,Bold", "Times-Bold" },
                                                    { "TimesNewRoman,BoldItalic", "Times-BoldItalic" },
                                                    { "TimesNewRoman,Italic", "Times-Italic" },
                                                    { "TimesNewRoman-Bold", "Times-Bold" },
                                                    { "TimesNewRoman-BoldItalic", "Times-BoldItalic" },
                                                    { "TimesNewRoman-Italic", "Times-Italic" },
                                                    { "TimesNewRomanPS", "Times-Roman" },
                                                    { "TimesNewRomanPS-Bold", "Times-Bold" },
                                                    { "TimesNewRomanPS-BoldItalic", "Times-BoldItalic" },
                                                    { "TimesNewRomanPS-BoldItalicMT", "Times-BoldItalic" },
                                                    { "TimesNewRomanPS-BoldMT", "Times-Bold" },
                                                    { "TimesNewRomanPS-Italic", "Times-Italic" },
                                                    { "TimesNewRomanPS-ItalicMT", "Times-Italic" },
                                                    { "TimesNewRomanPSMT", "Times-Roman" },
                                                    { "TimesNewRomanPSMT,Bold", "Times-Bold" },
                                                    { "TimesNewRomanPSMT,BoldItalic", "Times-BoldItalic" },
                                                    { "TimesNewRomanPSMT,Italic", "Times-Italic" },
                                                    { "ZapfDingbats", "ZapfDingbats" } };

//------------------------------------------------------------------------

// index: {fixed:0, sans-serif:4, serif:8} + bold*2 + italic
// NB: must be in same order as psSubstFonts in PSOutputDev.cc
static const char *base14SubstFonts[14] = { "Courier", "Courier-Oblique", "Courier-Bold", "Courier-BoldOblique", "Helvetica", "Helvetica-Oblique", "Helvetica-Bold", "Helvetica-BoldOblique", "Times-Roman", "Times-Italic", "Times-Bold",
                                            "Times-BoldItalic",
                                            // the last two are never used for substitution
                                            "Symbol", "ZapfDingbats" };

//------------------------------------------------------------------------

static int parseCharName(char *charName, Unicode *uBuf, int uLen, bool names, bool ligatures, bool numeric, bool hex, bool variants);

//------------------------------------------------------------------------

static int readFromStream(void *data)
{
    return ((Stream *)data)->getChar();
}

//------------------------------------------------------------------------
// GfxFontLoc
//------------------------------------------------------------------------

GfxFontLoc::GfxFontLoc()
{
    fontNum = 0;
    substIdx = -1;
}

GfxFontLoc::~GfxFontLoc() = default;

GfxFontLoc::GfxFontLoc(GfxFontLoc &&other) noexcept = default;

GfxFontLoc &GfxFontLoc::operator=(GfxFontLoc &&other) noexcept = default;

void GfxFontLoc::setPath(GooString *pathA)
{
    path = pathA->toStr();
    delete pathA;
}

const GooString *GfxFontLoc::pathAsGooString() const
{
    return (const GooString *)(&path);
}

//------------------------------------------------------------------------
// GfxFont
//------------------------------------------------------------------------

std::unique_ptr<GfxFont> GfxFont::makeFont(XRef *xref, const char *tagA, Ref idA, Dict *fontDict)
{
    std::optional<std::string> name;
    Ref embFontIDA;
    GfxFontType typeA;

    // get base font name
    Object obj1 = fontDict->lookup("BaseFont");
    if (obj1.isName()) {
        name = obj1.getName();
    }

    // There is no BaseFont in Type 3 fonts, try fontDescriptor.FontName
    if (!name) {
        Object fontDesc = fontDict->lookup("FontDescriptor");
        if (fontDesc.isDict()) {
            Object obj2 = fontDesc.dictLookup("FontName");
            if (obj2.isName()) {
                name = obj2.getName();
            }
        }
    }

    // As a last resort try the Name key
    if (!name) {
        Object obj2 = fontDict->lookup("Name");
        if (obj2.isName()) {
            name = obj2.getName();
        }
    }

    // get embedded font ID and font type
    typeA = getFontType(xref, fontDict, &embFontIDA);

    // create the font object
    GfxFont *font;
    if (typeA < fontCIDType0) {
        font = new Gfx8BitFont(xref, tagA, idA, std::move(name), typeA, embFontIDA, fontDict);
    } else {
        font = new GfxCIDFont(xref, tagA, idA, std::move(name), typeA, embFontIDA, fontDict);
    }

    return std::unique_ptr<GfxFont>(font);
}

GfxFont::GfxFont(const char *tagA, Ref idA, std::optional<std::string> &&nameA, GfxFontType typeA, Ref embFontIDA) : tag(tagA), id(idA), name(std::move(nameA)), type(typeA)
{
    ok = false;
    embFontID = embFontIDA;
    embFontName = nullptr;
    family = nullptr;
    stretch = StretchNotDefined;
    weight = WeightNotDefined;
    hasToUnicode = false;
}

GfxFont::~GfxFont()
{
    delete family;
    if (embFontName) {
        delete embFontName;
    }
}

bool GfxFont::isSubset() const
{
    if (name) {
        unsigned int i;
        for (i = 0; i < name->size(); ++i) {
            if ((*name)[i] < 'A' || (*name)[i] > 'Z') {
                break;
            }
        }
        return i == 6 && name->size() > 7 && (*name)[6] == '+';
    }
    return false;
}

std::string GfxFont::getNameWithoutSubsetTag() const
{
    if (!name) {
        return {};
    }

    if (!isSubset()) {
        return *name;
    }

    return name->substr(7);
}

// This function extracts three pieces of information:
// 1. the "expected" font type, i.e., the font type implied by
//    Font.Subtype, DescendantFont.Subtype, and
//    FontDescriptor.FontFile3.Subtype
// 2. the embedded font object ID
// 3. the actual font type - determined by examining the embedded font
//    if there is one, otherwise equal to the expected font type
// If the expected and actual font types don't match, a warning
// message is printed.  The expected font type is not used for
// anything else.
GfxFontType GfxFont::getFontType(XRef *xref, Dict *fontDict, Ref *embID)
{
    GfxFontType t, expectedType;
    FoFiIdentifierType fft;
    Dict *fontDict2;
    bool isType0, err;

    t = fontUnknownType;
    *embID = Ref::INVALID();
    err = false;

    Object subtype = fontDict->lookup("Subtype");
    expectedType = fontUnknownType;
    isType0 = false;
    if (subtype.isName("Type1") || subtype.isName("MMType1")) {
        expectedType = fontType1;
    } else if (subtype.isName("Type1C")) {
        expectedType = fontType1C;
    } else if (subtype.isName("Type3")) {
        expectedType = fontType3;
    } else if (subtype.isName("TrueType")) {
        expectedType = fontTrueType;
    } else if (subtype.isName("Type0")) {
        isType0 = true;
    } else {
        error(errSyntaxWarning, -1, "Unknown font type: '{0:s}'", subtype.isName() ? subtype.getName() : "???");
    }

    fontDict2 = fontDict;
    Object obj1 = fontDict->lookup("DescendantFonts");
    Object obj2; // Do not move to inside the if
                 // we need it around so that fontDict2 remains valid
    if (obj1.isArray()) {
        if (obj1.arrayGetLength() == 0) {
            error(errSyntaxWarning, -1, "Empty DescendantFonts array in font");
        } else {
            obj2 = obj1.arrayGet(0);
            if (obj2.isDict()) {
                if (!isType0) {
                    error(errSyntaxWarning, -1, "Non-CID font with DescendantFonts array");
                }
                fontDict2 = obj2.getDict();
                subtype = fontDict2->lookup("Subtype");
                if (subtype.isName("CIDFontType0")) {
                    if (isType0) {
                        expectedType = fontCIDType0;
                    }
                } else if (subtype.isName("CIDFontType2")) {
                    if (isType0) {
                        expectedType = fontCIDType2;
                    }
                }
            }
        }
    }

    Object fontDesc = fontDict2->lookup("FontDescriptor");
    if (fontDesc.isDict()) {
        Object obj3 = fontDesc.dictLookupNF("FontFile").copy();
        if (obj3.isRef()) {
            *embID = obj3.getRef();
            if (expectedType != fontType1) {
                err = true;
            }
        }
        if (*embID == Ref::INVALID() && (obj3 = fontDesc.dictLookupNF("FontFile2").copy(), obj3.isRef())) {
            *embID = obj3.getRef();
            if (isType0) {
                expectedType = fontCIDType2;
            } else if (expectedType != fontTrueType) {
                err = true;
            }
        }
        if (*embID == Ref::INVALID() && (obj3 = fontDesc.dictLookupNF("FontFile3").copy(), obj3.isRef())) {
            *embID = obj3.getRef();
            Object obj4 = obj3.fetch(xref);
            if (obj4.isStream()) {
                subtype = obj4.streamGetDict()->lookup("Subtype");
                if (subtype.isName("Type1")) {
                    if (expectedType != fontType1) {
                        err = true;
                        expectedType = isType0 ? fontCIDType0 : fontType1;
                    }
                } else if (subtype.isName("Type1C")) {
                    if (expectedType == fontType1) {
                        expectedType = fontType1C;
                    } else if (expectedType != fontType1C) {
                        err = true;
                        expectedType = isType0 ? fontCIDType0C : fontType1C;
                    }
                } else if (subtype.isName("TrueType")) {
                    if (expectedType != fontTrueType) {
                        err = true;
                        expectedType = isType0 ? fontCIDType2 : fontTrueType;
                    }
                } else if (subtype.isName("CIDFontType0C")) {
                    if (expectedType == fontCIDType0) {
                        expectedType = fontCIDType0C;
                    } else {
                        err = true;
                        expectedType = isType0 ? fontCIDType0C : fontType1C;
                    }
                } else if (subtype.isName("OpenType")) {
                    if (expectedType == fontTrueType) {
                        expectedType = fontTrueTypeOT;
                    } else if (expectedType == fontType1) {
                        expectedType = fontType1COT;
                    } else if (expectedType == fontCIDType0) {
                        expectedType = fontCIDType0COT;
                    } else if (expectedType == fontCIDType2) {
                        expectedType = fontCIDType2OT;
                    } else {
                        err = true;
                    }
                } else {
                    error(errSyntaxError, -1, "Unknown font type '{0:s}'", subtype.isName() ? subtype.getName() : "???");
                }
            }
        }
    }

    t = fontUnknownType;
    if (*embID != Ref::INVALID()) {
        Object obj3(*embID);
        Object obj4 = obj3.fetch(xref);
        if (obj4.isStream()) {
            obj4.streamReset();
            fft = FoFiIdentifier::identifyStream(&readFromStream, obj4.getStream());
            obj4.streamClose();
            switch (fft) {
            case fofiIdType1PFA:
            case fofiIdType1PFB:
                t = fontType1;
                break;
            case fofiIdCFF8Bit:
                t = isType0 ? fontCIDType0C : fontType1C;
                break;
            case fofiIdCFFCID:
                t = fontCIDType0C;
                break;
            case fofiIdTrueType:
            case fofiIdTrueTypeCollection:
                t = isType0 ? fontCIDType2 : fontTrueType;
                break;
            case fofiIdOpenTypeCFF8Bit:
                t = isType0 ? fontCIDType0COT : fontType1COT;
                break;
            case fofiIdOpenTypeCFFCID:
                t = fontCIDType0COT;
                break;
            default:
                error(errSyntaxError, -1, "Embedded font file may be invalid");
                break;
            }
        }
    }

    if (t == fontUnknownType) {
        t = expectedType;
    }

    if (t != expectedType) {
        err = true;
    }

    if (err) {
        error(errSyntaxWarning, -1, "Mismatch between font type and embedded font file");
    }

    return t;
}

void GfxFont::readFontDescriptor(XRef *xref, Dict *fontDict)
{
    double t;

    // assume Times-Roman by default (for substitution purposes)
    flags = fontSerif;

    missingWidth = 0;

    Object obj1 = fontDict->lookup("FontDescriptor");
    if (obj1.isDict()) {

        // get flags
        Object obj2 = obj1.dictLookup("Flags");
        if (obj2.isInt()) {
            flags = obj2.getInt();
        }

        // get name
        obj2 = obj1.dictLookup("FontName");
        if (obj2.isName()) {
            embFontName = new GooString(obj2.getName());
        }
        if (embFontName == nullptr) {
            // get name with typo
            obj2 = obj1.dictLookup("Fontname");
            if (obj2.isName()) {
                embFontName = new GooString(obj2.getName());
                error(errSyntaxWarning, -1, "The file uses Fontname instead of FontName please notify the creator that the file is broken");
            }
        }

        // get family
        obj2 = obj1.dictLookup("FontFamily");
        if (obj2.isString()) {
            family = new GooString(obj2.getString());
        }

        // get stretch
        obj2 = obj1.dictLookup("FontStretch");
        if (obj2.isName()) {
            if (strcmp(obj2.getName(), "UltraCondensed") == 0) {
                stretch = UltraCondensed;
            } else if (strcmp(obj2.getName(), "ExtraCondensed") == 0) {
                stretch = ExtraCondensed;
            } else if (strcmp(obj2.getName(), "Condensed") == 0) {
                stretch = Condensed;
            } else if (strcmp(obj2.getName(), "SemiCondensed") == 0) {
                stretch = SemiCondensed;
            } else if (strcmp(obj2.getName(), "Normal") == 0) {
                stretch = Normal;
            } else if (strcmp(obj2.getName(), "SemiExpanded") == 0) {
                stretch = SemiExpanded;
            } else if (strcmp(obj2.getName(), "Expanded") == 0) {
                stretch = Expanded;
            } else if (strcmp(obj2.getName(), "ExtraExpanded") == 0) {
                stretch = ExtraExpanded;
            } else if (strcmp(obj2.getName(), "UltraExpanded") == 0) {
                stretch = UltraExpanded;
            } else {
                error(errSyntaxWarning, -1, "Invalid Font Stretch");
            }
        }

        // get weight
        obj2 = obj1.dictLookup("FontWeight");
        if (obj2.isNum()) {
            if (obj2.getNum() == 100) {
                weight = W100;
            } else if (obj2.getNum() == 200) {
                weight = W200;
            } else if (obj2.getNum() == 300) {
                weight = W300;
            } else if (obj2.getNum() == 400) {
                weight = W400;
            } else if (obj2.getNum() == 500) {
                weight = W500;
            } else if (obj2.getNum() == 600) {
                weight = W600;
            } else if (obj2.getNum() == 700) {
                weight = W700;
            } else if (obj2.getNum() == 800) {
                weight = W800;
            } else if (obj2.getNum() == 900) {
                weight = W900;
            } else {
                error(errSyntaxWarning, -1, "Invalid Font Weight");
            }
        }

        // look for MissingWidth
        obj2 = obj1.dictLookup("MissingWidth");
        if (obj2.isNum()) {
            missingWidth = obj2.getNum();
        }

        // get Ascent and Descent
        obj2 = obj1.dictLookup("Ascent");
        if (obj2.isNum()) {
            t = 0.001 * obj2.getNum();
            // some broken font descriptors specify a negative ascent
            if (t < 0) {
                t = -t;
            }
            // some broken font descriptors set ascent and descent to 0;
            // others set it to ridiculous values (e.g., 32768)
            if (t != 0 && t < 3) {
                ascent = t;
            }
        }
        obj2 = obj1.dictLookup("Descent");
        if (obj2.isNum()) {
            t = 0.001 * obj2.getNum();
            // some broken font descriptors specify a positive descent
            if (t > 0) {
                t = -t;
            }
            // some broken font descriptors set ascent and descent to 0
            if (t != 0 && t > -3) {
                descent = t;
            }
        }

        // font FontBBox
        obj2 = obj1.dictLookup("FontBBox");
        if (obj2.isArray()) {
            for (int i = 0; i < 4 && i < obj2.arrayGetLength(); ++i) {
                Object obj3 = obj2.arrayGet(i);
                if (obj3.isNum()) {
                    fontBBox[i] = 0.001 * obj3.getNum();
                }
            }
        }
    }
}

CharCodeToUnicode *GfxFont::readToUnicodeCMap(Dict *fontDict, int nBits, CharCodeToUnicode *ctu)
{
    GooString *buf;

    Object obj1 = fontDict->lookup("ToUnicode");
    if (!obj1.isStream()) {
        return nullptr;
    }
    buf = new GooString();
    obj1.getStream()->fillGooString(buf);
    obj1.streamClose();
    if (ctu) {
        ctu->mergeCMap(buf, nBits);
    } else {
        ctu = CharCodeToUnicode::parseCMap(buf, nBits);
    }
    hasToUnicode = true;
    delete buf;
    return ctu;
}

std::optional<GfxFontLoc> GfxFont::locateFont(XRef *xref, PSOutputDev *ps, GooString *substituteFontName)
{
    SysFontType sysFontType;
    GooString *path, *base14Name;
    int substIdx, fontNum;
    bool embed;

    if (type == fontType3) {
        return std::nullopt;
    }

    //----- embedded font
    if (embFontID != Ref::INVALID()) {
        embed = true;
        Object refObj(embFontID);
        Object embFontObj = refObj.fetch(xref);
        if (!embFontObj.isStream()) {
            error(errSyntaxError, -1, "Embedded font object is wrong type");
            embed = false;
        }
        if (embed) {
            if (ps) {
                switch (type) {
                case fontType1:
                case fontType1C:
                case fontType1COT:
                    embed = ps->getEmbedType1();
                    break;
                case fontTrueType:
                case fontTrueTypeOT:
                    embed = ps->getEmbedTrueType();
                    break;
                case fontCIDType0C:
                case fontCIDType0COT:
                    embed = ps->getEmbedCIDPostScript();
                    break;
                case fontCIDType2:
                case fontCIDType2OT:
                    embed = ps->getEmbedCIDTrueType();
                    break;
                default:
                    break;
                }
            }
            if (embed) {
                GfxFontLoc fontLoc;
                fontLoc.locType = gfxFontLocEmbedded;
                fontLoc.fontType = type;
                fontLoc.embFontID = embFontID;
                return fontLoc;
            }
        }
    }

    //----- PS passthrough
    if (ps && !isCIDFont() && ps->getFontPassthrough()) {
        GfxFontLoc fontLoc;
        fontLoc.locType = gfxFontLocResident;
        fontLoc.fontType = fontType1;
        fontLoc.path = *name;
        return fontLoc;
    }

    //----- PS resident Base-14 font
    if (ps && !isCIDFont() && ((Gfx8BitFont *)this)->base14) {
        GfxFontLoc fontLoc;
        fontLoc.locType = gfxFontLocResident;
        fontLoc.fontType = fontType1;
        fontLoc.path = ((Gfx8BitFont *)this)->base14->base14Name;
        return fontLoc;
    }

    //----- external font file (fontFile, fontDir)
    if (name && (path = globalParams->findFontFile(*name))) {
        if (std::optional<GfxFontLoc> fontLoc = getExternalFont(path, isCIDFont())) {
            return fontLoc;
        }
    }

    //----- external font file for Base-14 font
    if (!ps && !isCIDFont() && ((Gfx8BitFont *)this)->base14) {
        base14Name = new GooString(((Gfx8BitFont *)this)->base14->base14Name);
        if ((path = globalParams->findBase14FontFile(base14Name, this, substituteFontName))) {
            if (std::optional<GfxFontLoc> fontLoc = getExternalFont(path, false)) {
                delete base14Name;
                return fontLoc;
            }
        }
        delete base14Name;
    }

    //----- system font
    if ((path = globalParams->findSystemFontFile(this, &sysFontType, &fontNum, substituteFontName))) {
        if (isCIDFont()) {
            if (sysFontType == sysFontTTF || sysFontType == sysFontTTC) {
                GfxFontLoc fontLoc;
                fontLoc.locType = gfxFontLocExternal;
                fontLoc.fontType = fontCIDType2;
                fontLoc.setPath(path);
                fontLoc.fontNum = fontNum;
                return fontLoc;
            }
        } else {
            GfxFontLoc fontLoc;
            fontLoc.setPath(path);
            fontLoc.locType = gfxFontLocExternal;
            if (sysFontType == sysFontTTF || sysFontType == sysFontTTC) {
                fontLoc.fontType = fontTrueType;
            } else if (sysFontType == sysFontPFA || sysFontType == sysFontPFB) {
                fontLoc.fontType = fontType1;
                fontLoc.fontNum = fontNum;
            }
            return fontLoc;
        }
        delete path;
    }

    if (!isCIDFont()) {

        //----- 8-bit font substitution
        if (flags & fontFixedWidth) {
            substIdx = 0;
        } else if (flags & fontSerif) {
            substIdx = 8;
        } else {
            substIdx = 4;
        }
        if (isBold()) {
            substIdx += 2;
        }
        if (isItalic()) {
            substIdx += 1;
        }
        const std::string substName = base14SubstFonts[substIdx];
        if (ps) {
            error(errSyntaxWarning, -1, "Substituting font '{0:s}' for '{1:s}'", base14SubstFonts[substIdx], name ? name->c_str() : "null");
            GfxFontLoc fontLoc;
            fontLoc.locType = gfxFontLocResident;
            fontLoc.fontType = fontType1;
            fontLoc.path = substName;
            fontLoc.substIdx = substIdx;
            return fontLoc;
        } else {
            path = globalParams->findFontFile(substName);
            if (path) {
                if (std::optional<GfxFontLoc> fontLoc = getExternalFont(path, false)) {
                    error(errSyntaxWarning, -1, "Substituting font '{0:s}' for '{1:s}'", base14SubstFonts[substIdx], name ? name->c_str() : "");
                    name = base14SubstFonts[substIdx];
                    fontLoc->substIdx = substIdx;
                    return fontLoc;
                }
            }
        }

        // failed to find a substitute font
        return std::nullopt;
    }

    // failed to find a substitute font
    return std::nullopt;
}

std::optional<GfxFontLoc> GfxFont::getExternalFont(GooString *path, bool cid)
{
    FoFiIdentifierType fft;
    GfxFontType fontType;

    fft = FoFiIdentifier::identifyFile(path->c_str());
    switch (fft) {
    case fofiIdType1PFA:
    case fofiIdType1PFB:
        fontType = fontType1;
        break;
    case fofiIdCFF8Bit:
        fontType = fontType1C;
        break;
    case fofiIdCFFCID:
        fontType = fontCIDType0C;
        break;
    case fofiIdTrueType:
    case fofiIdTrueTypeCollection:
        fontType = cid ? fontCIDType2 : fontTrueType;
        break;
    case fofiIdOpenTypeCFF8Bit:
        fontType = fontType1COT;
        break;
    case fofiIdOpenTypeCFFCID:
        fontType = fontCIDType0COT;
        break;
    case fofiIdUnknown:
    case fofiIdError:
    default:
        fontType = fontUnknownType;
        break;
    }
    if (fontType == fontUnknownType || (cid ? (fontType < fontCIDType0) : (fontType >= fontCIDType0))) {
        delete path;
        return std::nullopt;
    }
    GfxFontLoc fontLoc;
    fontLoc.locType = gfxFontLocExternal;
    fontLoc.fontType = fontType;
    fontLoc.setPath(path);
    return fontLoc;
}

std::optional<std::vector<unsigned char>> GfxFont::readEmbFontFile(XRef *xref)
{
    Stream *str;

    Object obj1(embFontID);
    Object obj2 = obj1.fetch(xref);
    if (!obj2.isStream()) {
        error(errSyntaxError, -1, "Embedded font file is not a stream");
        embFontID = Ref::INVALID();
        return {};
    }
    str = obj2.getStream();

    std::vector<unsigned char> buf = str->toUnsignedChars();
    str->close();

    return buf;
}

struct AlternateNameMap
{
    const char *name;
    const char *alt;
};

static const AlternateNameMap alternateNameMap[] = { { "fi", "f_i" }, { "fl", "f_l" }, { "ff", "f_f" }, { "ffi", "f_f_i" }, { "ffl", "f_f_l" }, { nullptr, nullptr } };

const char *GfxFont::getAlternateName(const char *name)
{
    const AlternateNameMap *map = alternateNameMap;
    while (map->name) {
        if (strcmp(name, map->name) == 0) {
            return map->alt;
        }
        map++;
    }
    return nullptr;
}

//------------------------------------------------------------------------
// Gfx8BitFont
//------------------------------------------------------------------------

// Parse character names of the form 'Axx', 'xx', 'Ann', 'ABnn', or
// 'nn', where 'A' and 'B' are any letters, 'xx' is two hex digits,
// and 'nn' is decimal digits.
static bool parseNumericName(const char *s, bool hex, unsigned int *u)
{
    char *endptr;

    // Strip leading alpha characters.
    if (hex) {
        int n = 0;

        // Get string length while ignoring junk at end.
        while (isalnum(s[n])) {
            ++n;
        }

        // Only 2 hex characters with optional leading alpha is allowed.
        if (n == 3 && isalpha(*s)) {
            ++s;
        } else if (n != 2) {
            return false;
        }
    } else {
        // Strip up to two alpha characters.
        for (int i = 0; i < 2 && isalpha(*s); ++i) {
            ++s;
        }
    }

    int v = strtol(s, &endptr, hex ? 16 : 10);

    if (endptr == s) {
        return false;
    }

    // Skip trailing junk characters.
    while (*endptr != '\0' && !isalnum(*endptr)) {
        ++endptr;
    }

    if (*endptr == '\0') {
        if (u) {
            *u = v;
        }
        return true;
    }
    return false;
}

// Returns true if the font has character names like xx or Axx which
// should be parsed for hex or decimal values.
static bool testForNumericNames(Dict *fontDict, bool hex)
{
    bool numeric = true;

    Object enc = fontDict->lookup("Encoding");
    if (!enc.isDict()) {
        return false;
    }

    Object diff = enc.dictLookup("Differences");
    if (!diff.isArray()) {
        return false;
    }

    for (int i = 0; i < diff.arrayGetLength() && numeric; ++i) {
        Object obj = diff.arrayGet(i);
        if (obj.isInt()) {
            // All sequences must start between character codes 0 and 5.
            if (obj.getInt() > 5) {
                numeric = false;
            }
        } else if (obj.isName()) {
            // All character names must successfully parse.
            if (!parseNumericName(obj.getName(), hex, nullptr)) {
                numeric = false;
            }
        } else {
            numeric = false;
        }
    }

    return numeric;
}

Gfx8BitFont::Gfx8BitFont(XRef *xref, const char *tagA, Ref idA, std::optional<std::string> &&nameA, GfxFontType typeA, Ref embFontIDA, Dict *fontDict) : GfxFont(tagA, idA, std::move(nameA), typeA, embFontIDA)
{
    const BuiltinFont *builtinFont;
    const char **baseEnc;
    bool baseEncFromFontFile;
    int len;
    FoFiType1 *ffT1;
    FoFiType1C *ffT1C;
    char *charName;
    bool missing, hex;
    bool numeric;
    Unicode toUnicode[256];
    Unicode uBuf[8];
    double mul;
    int firstChar, lastChar;
    unsigned short w;
    Object obj1;
    int n, a, b, m;

    ctu = nullptr;

    // do font name substitution for various aliases of the Base 14 font
    // names
    base14 = nullptr;
    if (name) {
        std::string name2 = *name;
        size_t i = 0;
        while (i < name2.size()) {
            if (name2[i] == ' ') {
                name2.erase(i, 1);
            } else {
                ++i;
            }
        }
        a = 0;
        b = sizeof(base14FontMap) / sizeof(Base14FontMapEntry);
        // invariant: base14FontMap[a].altName <= name2 < base14FontMap[b].altName
        while (b - a > 1) {
            m = (a + b) / 2;
            if (name2.compare(base14FontMap[m].altName) >= 0) {
                a = m;
            } else {
                b = m;
            }
        }
        if (name2 == base14FontMap[a].altName) {
            base14 = &base14FontMap[a];
        }
    }

    // is it a built-in font?
    builtinFont = nullptr;
    if (base14) {
        for (const BuiltinFont &bf : builtinFonts) {
            if (!strcmp(base14->base14Name, bf.name)) {
                builtinFont = &bf;
                break;
            }
        }
    }

    // default ascent/descent values
    if (builtinFont) {
        ascent = 0.001 * builtinFont->ascent;
        descent = 0.001 * builtinFont->descent;
        fontBBox[0] = 0.001 * builtinFont->bbox[0];
        fontBBox[1] = 0.001 * builtinFont->bbox[1];
        fontBBox[2] = 0.001 * builtinFont->bbox[2];
        fontBBox[3] = 0.001 * builtinFont->bbox[3];
    } else {
        ascent = 0.95;
        descent = -0.35;
        fontBBox[0] = fontBBox[1] = fontBBox[2] = fontBBox[3] = 0;
    }

    // get info from font descriptor
    readFontDescriptor(xref, fontDict);

    // for non-embedded fonts, don't trust the ascent/descent/bbox
    // values from the font descriptor
    if (builtinFont && embFontID == Ref::INVALID()) {
        ascent = 0.001 * builtinFont->ascent;
        descent = 0.001 * builtinFont->descent;
        fontBBox[0] = 0.001 * builtinFont->bbox[0];
        fontBBox[1] = 0.001 * builtinFont->bbox[1];
        fontBBox[2] = 0.001 * builtinFont->bbox[2];
        fontBBox[3] = 0.001 * builtinFont->bbox[3];
    }

    // get font matrix
    fontMat[0] = fontMat[3] = 1;
    fontMat[1] = fontMat[2] = fontMat[4] = fontMat[5] = 0;
    obj1 = fontDict->lookup("FontMatrix");
    if (obj1.isArray()) {
        for (int i = 0; i < 6 && i < obj1.arrayGetLength(); ++i) {
            Object obj2 = obj1.arrayGet(i);
            if (obj2.isNum()) {
                fontMat[i] = obj2.getNum();
            }
        }
    }

    // get Type 3 bounding box, font definition, and resources
    if (type == fontType3) {
        obj1 = fontDict->lookup("FontBBox");
        if (obj1.isArray()) {
            for (int i = 0; i < 4 && i < obj1.arrayGetLength(); ++i) {
                Object obj2 = obj1.arrayGet(i);
                if (obj2.isNum()) {
                    fontBBox[i] = obj2.getNum();
                }
            }
        }
        charProcs = fontDict->lookup("CharProcs");
        if (!charProcs.isDict()) {
            error(errSyntaxError, -1, "Missing or invalid CharProcs dictionary in Type 3 font");
            charProcs.setToNull();
        }
        resources = fontDict->lookup("Resources");
        if (!resources.isDict()) {
            resources.setToNull();
        }
    }

    //----- build the font encoding -----

    // Encodings start with a base encoding, which can come from
    // (in order of priority):
    //   1. FontDict.Encoding or FontDict.Encoding.BaseEncoding
    //        - MacRoman / MacExpert / WinAnsi / Standard
    //   2. embedded or external font file
    //   3. default:
    //        - builtin --> builtin encoding
    //        - TrueType --> WinAnsiEncoding
    //        - others --> StandardEncoding
    // and then add a list of differences (if any) from
    // FontDict.Encoding.Differences.

    // check FontDict for base encoding
    hasEncoding = false;
    usesMacRomanEnc = false;
    baseEnc = nullptr;
    baseEncFromFontFile = false;
    obj1 = fontDict->lookup("Encoding");
    if (obj1.isDict()) {
        Object obj2 = obj1.dictLookup("BaseEncoding");
        if (obj2.isName("MacRomanEncoding")) {
            hasEncoding = true;
            usesMacRomanEnc = true;
            baseEnc = macRomanEncoding;
        } else if (obj2.isName("MacExpertEncoding")) {
            hasEncoding = true;
            baseEnc = macExpertEncoding;
        } else if (obj2.isName("WinAnsiEncoding")) {
            hasEncoding = true;
            baseEnc = winAnsiEncoding;
        }
    } else if (obj1.isName("MacRomanEncoding")) {
        hasEncoding = true;
        usesMacRomanEnc = true;
        baseEnc = macRomanEncoding;
    } else if (obj1.isName("MacExpertEncoding")) {
        hasEncoding = true;
        baseEnc = macExpertEncoding;
    } else if (obj1.isName("WinAnsiEncoding")) {
        hasEncoding = true;
        baseEnc = winAnsiEncoding;
    }

    // check embedded font file for base encoding
    // (only for Type 1 fonts - trying to get an encoding out of a
    // TrueType font is a losing proposition)
    ffT1 = nullptr;
    ffT1C = nullptr;
    if (type == fontType1 && embFontID != Ref::INVALID()) {
        const std::optional<std::vector<unsigned char>> buf = readEmbFontFile(xref);
        if (buf) {
            if ((ffT1 = FoFiType1::make(buf->data(), buf->size()))) {
                const std::string fontName = ffT1->getName();
                if (!fontName.empty()) {
                    delete embFontName;
                    embFontName = new GooString(fontName);
                }
                if (!baseEnc) {
                    baseEnc = (const char **)ffT1->getEncoding();
                    baseEncFromFontFile = true;
                }
            }
        }
    } else if (type == fontType1C && embFontID != Ref::INVALID()) {
        const std::optional<std::vector<unsigned char>> buf = readEmbFontFile(xref);
        if (buf) {
            if ((ffT1C = FoFiType1C::make(buf->data(), buf->size()))) {
                if (ffT1C->getName()) {
                    if (embFontName) {
                        delete embFontName;
                    }
                    embFontName = new GooString(ffT1C->getName());
                }
                if (!baseEnc) {
                    baseEnc = (const char **)ffT1C->getEncoding();
                    baseEncFromFontFile = true;
                }
            }
        }
    }

    // get default base encoding
    if (!baseEnc) {
        if (builtinFont && embFontID == Ref::INVALID()) {
            baseEnc = builtinFont->defaultBaseEnc;
            hasEncoding = true;
        } else if (type == fontTrueType) {
            baseEnc = winAnsiEncoding;
        } else {
            baseEnc = standardEncoding;
        }
    }

    if (baseEncFromFontFile) {
        encodingName = "Builtin";
    } else if (baseEnc == winAnsiEncoding) {
        encodingName = "WinAnsi";
    } else if (baseEnc == macRomanEncoding) {
        encodingName = "MacRoman";
    } else if (baseEnc == macExpertEncoding) {
        encodingName = "MacExpert";
    } else if (baseEnc == symbolEncoding) {
        encodingName = "Symbol";
    } else if (baseEnc == zapfDingbatsEncoding) {
        encodingName = "ZapfDingbats";
    } else {
        encodingName = "Standard";
    }

    // copy the base encoding
    for (int i = 0; i < 256; ++i) {
        enc[i] = (char *)baseEnc[i];
        if ((encFree[i] = baseEncFromFontFile) && enc[i]) {
            enc[i] = copyString(baseEnc[i]);
        }
    }

    // some Type 1C font files have empty encodings, which can break the
    // T1C->T1 conversion (since the 'seac' operator depends on having
    // the accents in the encoding), so we fill in any gaps from
    // StandardEncoding
    if (type == fontType1C && embFontID != Ref::INVALID() && baseEncFromFontFile) {
        for (int i = 0; i < 256; ++i) {
            if (!enc[i] && standardEncoding[i]) {
                enc[i] = (char *)standardEncoding[i];
                encFree[i] = false;
            }
        }
    }

    // merge differences into encoding
    if (obj1.isDict()) {
        Object obj2 = obj1.dictLookup("Differences");
        if (obj2.isArray()) {
            encodingName = "Custom";
            hasEncoding = true;
            int code = 0;
            for (int i = 0; i < obj2.arrayGetLength(); ++i) {
                Object obj3 = obj2.arrayGet(i);
                if (obj3.isInt()) {
                    code = obj3.getInt();
                } else if (obj3.isName()) {
                    if (code >= 0 && code < 256) {
                        if (encFree[code]) {
                            gfree(enc[code]);
                        }
                        enc[code] = copyString(obj3.getName());
                        encFree[code] = true;
                        ++code;
                    }
                } else {
                    error(errSyntaxError, -1, "Wrong type in font encoding resource differences ({0:s})", obj3.getTypeName());
                }
            }
        }
    }
    delete ffT1;
    delete ffT1C;

    //----- build the mapping to Unicode -----

    // pass 1: use the name-to-Unicode mapping table
    missing = hex = false;
    bool isZapfDingbats = name && GooString::endsWith(*name, "ZapfDingbats");
    for (int code = 0; code < 256; ++code) {
        if ((charName = enc[code])) {
            if (isZapfDingbats) {
                // include ZapfDingbats names
                toUnicode[code] = globalParams->mapNameToUnicodeAll(charName);
            } else {
                toUnicode[code] = globalParams->mapNameToUnicodeText(charName);
            }
            if (!toUnicode[code] && strcmp(charName, ".notdef")) {
                // if it wasn't in the name-to-Unicode table, check for a
                // name that looks like 'Axx' or 'xx', where 'A' is any letter
                // and 'xx' is two hex digits
                if ((strlen(charName) == 3 && isalpha(charName[0]) && isxdigit(charName[1]) && isxdigit(charName[2])
                     && ((charName[1] >= 'a' && charName[1] <= 'f') || (charName[1] >= 'A' && charName[1] <= 'F') || (charName[2] >= 'a' && charName[2] <= 'f') || (charName[2] >= 'A' && charName[2] <= 'F')))
                    || (strlen(charName) == 2 && isxdigit(charName[0]) && isxdigit(charName[1]) &&
                        // Only check idx 1 to avoid misidentifying a decimal
                        // number like a0
                        ((charName[1] >= 'a' && charName[1] <= 'f') || (charName[1] >= 'A' && charName[1] <= 'F')))) {
                    hex = true;
                }
                missing = true;
            }
        } else {
            toUnicode[code] = 0;
        }
    }

    numeric = testForNumericNames(fontDict, hex);

    // construct the char code -> Unicode mapping object
    ctu = CharCodeToUnicode::make8BitToUnicode(toUnicode);

    // pass 1a: Expand ligatures in the Alphabetic Presentation Form
    // block (eg "fi", "ffi") to normal form
    for (int code = 0; code < 256; ++code) {
        if (unicodeIsAlphabeticPresentationForm(toUnicode[code])) {
            Unicode *normalized = unicodeNormalizeNFKC(&toUnicode[code], 1, &len, nullptr);
            if (len > 1) {
                ctu->setMapping((CharCode)code, normalized, len);
            }
            gfree(normalized);
        }
    }

    // pass 2: try to fill in the missing chars, looking for ligatures, numeric
    // references and variants
    if (missing) {
        for (int code = 0; code < 256; ++code) {
            if (!toUnicode[code]) {
                if ((charName = enc[code]) && strcmp(charName, ".notdef")
                    && (n = parseCharName(charName, uBuf, sizeof(uBuf) / sizeof(*uBuf),
                                          false, // don't check simple names (pass 1)
                                          true, // do check ligatures
                                          numeric, hex,
                                          true))) { // do check variants
                    ctu->setMapping((CharCode)code, uBuf, n);
                    continue;
                }

                // do a simple pass-through
                // mapping for unknown character names
                uBuf[0] = code;
                ctu->setMapping((CharCode)code, uBuf, 1);
            }
        }
    }

    // merge in a ToUnicode CMap, if there is one -- this overwrites
    // existing entries in ctu, i.e., the ToUnicode CMap takes
    // precedence, but the other encoding info is allowed to fill in any
    // holes
    readToUnicodeCMap(fontDict, 16, ctu);

    //----- get the character widths -----

    // initialize all widths
    for (double &width : widths) {
        width = missingWidth * 0.001;
    }

    // use widths from font dict, if present
    obj1 = fontDict->lookup("FirstChar");
    firstChar = obj1.isInt() ? obj1.getInt() : 0;
    if (firstChar < 0 || firstChar > 255) {
        firstChar = 0;
    }
    obj1 = fontDict->lookup("LastChar");
    lastChar = obj1.isInt() ? obj1.getInt() : 255;
    if (lastChar < 0 || lastChar > 255) {
        lastChar = 255;
    }
    mul = (type == fontType3) ? fontMat[0] : 0.001;
    obj1 = fontDict->lookup("Widths");
    if (obj1.isArray()) {
        flags |= fontFixedWidth;
        if (obj1.arrayGetLength() < lastChar - firstChar + 1) {
            lastChar = firstChar + obj1.arrayGetLength() - 1;
        }
        double firstNonZeroWidth = 0;
        for (int code = firstChar; code <= lastChar; ++code) {
            Object obj2 = obj1.arrayGet(code - firstChar);
            if (obj2.isNum()) {
                widths[code] = obj2.getNum() * mul;

                // Check if the font is fixed width
                if (firstNonZeroWidth == 0) {
                    firstNonZeroWidth = widths[code];
                }
                if (firstNonZeroWidth != 0 && widths[code] != 0 && fabs(widths[code] - firstNonZeroWidth) > 0.00001) {
                    flags &= ~fontFixedWidth;
                }
            }
        }

        // use widths from built-in font
    } else if (builtinFont) {
        // this is a kludge for broken PDF files that encode char 32
        // as .notdef
        if (builtinFont->getWidth("space", &w)) {
            widths[32] = 0.001 * w;
        }
        for (int code = 0; code < 256; ++code) {
            if (enc[code] && builtinFont->getWidth(enc[code], &w)) {
                widths[code] = 0.001 * w;
            }
        }

        // couldn't find widths -- use defaults
    } else {
        // this is technically an error -- the Widths entry is required
        // for all but the Base-14 fonts -- but certain PDF generators
        // apparently don't include widths for Arial and TimesNewRoman
        int i;
        if (isFixedWidth()) {
            i = 0;
        } else if (isSerif()) {
            i = 8;
        } else {
            i = 4;
        }
        if (isBold()) {
            i += 2;
        }
        if (isItalic()) {
            i += 1;
        }
        builtinFont = builtinFontSubst[i];
        // this is a kludge for broken PDF files that encode char 32
        // as .notdef
        if (builtinFont->getWidth("space", &w)) {
            widths[32] = 0.001 * w;
        }
        for (int code = 0; code < 256; ++code) {
            if (enc[code] && builtinFont->getWidth(enc[code], &w)) {
                widths[code] = 0.001 * w;
            }
        }
    }

    ok = true;
}

Gfx8BitFont::~Gfx8BitFont()
{
    int i;

    for (i = 0; i < 256; ++i) {
        if (encFree[i] && enc[i]) {
            gfree(enc[i]);
        }
    }
    ctu->decRefCnt();
}

// This function is in part a derived work of the Adobe Glyph Mapping
// Convention: http://www.adobe.com/devnet/opentype/archives/glyph.html
// Algorithmic comments are excerpted from that document to aid
// maintainability.
static int parseCharName(char *charName, Unicode *uBuf, int uLen, bool names, bool ligatures, bool numeric, bool hex, bool variants)
{
    if (uLen <= 0) {
        error(errInternal, -1,
              "Zero-length output buffer (recursion overflow?) in "
              "parseCharName, component \"{0:s}\"",
              charName);
        return 0;
    }
    // Step 1: drop all the characters from the glyph name starting with the
    // first occurrence of a period (U+002E FULL STOP), if any.
    if (variants) {
        char *var_part = strchr(charName, '.');
        if (var_part == charName) {
            return 0; // .notdef or similar
        } else if (var_part != nullptr) {
            // parse names of the form 7.oldstyle, P.swash, s.sc, etc.
            char *main_part = copyString(charName, var_part - charName);
            bool namesRecurse = true, variantsRecurse = false;
            int n = parseCharName(main_part, uBuf, uLen, namesRecurse, ligatures, numeric, hex, variantsRecurse);
            gfree(main_part);
            return n;
        }
    }
    // Step 2: split the remaining string into a sequence of components, using
    // underscore (U+005F LOW LINE) as the delimiter.
    if (ligatures && strchr(charName, '_')) {
        // parse names of the form A_a (e.g. f_i, T_h, l_quotesingle)
        char *lig_part, *lig_end, *lig_copy;
        int n = 0, m;
        lig_part = lig_copy = copyString(charName);
        do {
            if ((lig_end = strchr(lig_part, '_'))) {
                *lig_end = '\0';
            }
            if (lig_part[0] != '\0') {
                bool namesRecurse = true, ligaturesRecurse = false;
                if ((m = parseCharName(lig_part, uBuf + n, uLen - n, namesRecurse, ligaturesRecurse, numeric, hex, variants))) {
                    n += m;
                } else {
                    error(errSyntaxWarning, -1,
                          "Could not parse ligature component \"{0:s}\" of \"{1:s}\" in "
                          "parseCharName",
                          lig_part, charName);
                }
            }
            if (lig_end) {
                lig_part = lig_end + 1;
            }
        } while (lig_end && n < uLen);
        gfree(lig_copy);
        return n;
    }
    // Step 3: map each component to a character string according to the
    // procedure below, and concatenate those strings; the result is the
    // character string to which the glyph name is mapped.
    // 3.1. if the font is Zapf Dingbats (PostScript FontName ZapfDingbats), and
    // the component is in the ZapfDingbats list, then map it to the
    // corresponding character in that list.
    // 3.2. otherwise, if the component is in the Adobe Glyph List, then map it
    // to the corresponding character in that list.
    if (names && (uBuf[0] = globalParams->mapNameToUnicodeText(charName))) {
        return 1;
    }
    unsigned int n = strlen(charName);
    // 3.3. otherwise, if the component is of the form "uni" (U+0075 U+006E
    // U+0069) followed by a sequence of uppercase hexadecimal digits (0 .. 9,
    // A .. F, i.e. U+0030 .. U+0039, U+0041 .. U+0046), the length of that
    // sequence is a multiple of four, and each group of four digits represents
    // a number in the set {0x0000 .. 0xD7FF, 0xE000 .. 0xFFFF}, then interpret
    // each such number as a Unicode scalar value and map the component to the
    // string made of those scalar values. Note that the range and digit length
    // restrictions mean that the "uni" prefix can be used only with Unicode
    // values from the Basic Multilingual Plane (BMP).
    if (n >= 7 && (n % 4) == 3 && !strncmp(charName, "uni", 3)) {
        int i;
        unsigned int m;
        for (i = 0, m = 3; i < uLen && m < n; m += 4) {
            if (isxdigit(charName[m]) && isxdigit(charName[m + 1]) && isxdigit(charName[m + 2]) && isxdigit(charName[m + 3])) {
                unsigned int u;
                sscanf(charName + m, "%4x", &u);
                if (u <= 0xD7FF || (0xE000 <= u && u <= 0xFFFF)) {
                    uBuf[i++] = u;
                }
            }
        }
        return i;
    }
    // 3.4. otherwise, if the component is of the form "u" (U+0075) followed by
    // a sequence of four to six uppercase hexadecimal digits {0 .. 9, A .. F}
    // (U+0030 .. U+0039, U+0041 .. U+0046), and those digits represent a
    // number in {0x0000 .. 0xD7FF, 0xE000 .. 0x10FFFF}, then interpret this
    // number as a Unicode scalar value and map the component to the string
    // made of this scalar value.
    if (n >= 5 && n <= 7 && charName[0] == 'u' && isxdigit(charName[1]) && isxdigit(charName[2]) && isxdigit(charName[3]) && isxdigit(charName[4]) && (n <= 5 || isxdigit(charName[5])) && (n <= 6 || isxdigit(charName[6]))) {
        unsigned int u;
        sscanf(charName + 1, "%x", &u);
        if (u <= 0xD7FF || (0xE000 <= u && u <= 0x10FFFF)) {
            uBuf[0] = u;
            return 1;
        }
    }
    // Not in Adobe Glyph Mapping convention: look for names like xx
    // or Axx and parse for hex or decimal values.
    if (numeric && parseNumericName(charName, hex, uBuf)) {
        return 1;
    }
    // 3.5. otherwise, map the component to the empty string
    return 0;
}

int Gfx8BitFont::getNextChar(const char *s, int len, CharCode *code, Unicode const **u, int *uLen, double *dx, double *dy, double *ox, double *oy) const
{
    CharCode c;

    *code = c = (CharCode)(*s & 0xff);
    *uLen = ctu->mapToUnicode(c, u);
    *dx = widths[c];
    *dy = *ox = *oy = 0;
    return 1;
}

const CharCodeToUnicode *Gfx8BitFont::getToUnicode() const
{
    return ctu;
}

int *Gfx8BitFont::getCodeToGIDMap(FoFiTrueType *ff)
{
    int *map;
    int cmapPlatform, cmapEncoding;
    int unicodeCmap, macRomanCmap, msSymbolCmap, cmap;
    bool useMacRoman, useUnicode;
    char *charName;
    Unicode u;
    int code, i, n;

    map = (int *)gmallocn(256, sizeof(int));
    for (i = 0; i < 256; ++i) {
        map[i] = 0;
    }

    // To match up with the Adobe-defined behaviour, we choose a cmap
    // like this:
    // 1. If the PDF font has an encoding:
    //    1a. If the TrueType font has a Microsoft Unicode
    //        cmap or a non-Microsoft Unicode cmap, use it, and use the
    //        Unicode indexes, not the char codes.
    //    1b. If the PDF font specified MacRomanEncoding and the
    //        TrueType font has a Macintosh Roman cmap, use it, and
    //        reverse map the char names through MacRomanEncoding to
    //        get char codes.
    //    1c. If the PDF font is symbolic and the TrueType font has a
    //        Microsoft Symbol cmap, use it, and use char codes
    //        directly (possibly with an offset of 0xf000).
    //    1d. If the TrueType font has a Macintosh Roman cmap, use it,
    //        as in case 1a.
    // 2. If the PDF font does not have an encoding or the PDF font is
    //    symbolic:
    //    2a. If the TrueType font has a Macintosh Roman cmap, use it,
    //        and use char codes directly (possibly with an offset of
    //        0xf000).
    //    2b. If the TrueType font has a Microsoft Symbol cmap, use it,
    //        and use char codes directly (possible with an offset of
    //        0xf000).
    // 3. If none of these rules apply, use the first cmap and hope for
    //    the best (this shouldn't happen).
    unicodeCmap = macRomanCmap = msSymbolCmap = -1;
    for (i = 0; i < ff->getNumCmaps(); ++i) {
        cmapPlatform = ff->getCmapPlatform(i);
        cmapEncoding = ff->getCmapEncoding(i);
        if ((cmapPlatform == 3 && cmapEncoding == 1) || cmapPlatform == 0) {
            unicodeCmap = i;
        } else if (cmapPlatform == 1 && cmapEncoding == 0) {
            macRomanCmap = i;
        } else if (cmapPlatform == 3 && cmapEncoding == 0) {
            msSymbolCmap = i;
        }
    }
    cmap = 0;
    useMacRoman = false;
    useUnicode = false;
    if (hasEncoding || type == fontType1) {
        if (unicodeCmap >= 0) {
            cmap = unicodeCmap;
            useUnicode = true;
        } else if (usesMacRomanEnc && macRomanCmap >= 0) {
            cmap = macRomanCmap;
            useMacRoman = true;
        } else if ((flags & fontSymbolic) && msSymbolCmap >= 0) {
            cmap = msSymbolCmap;
        } else if ((flags & fontSymbolic) && macRomanCmap >= 0) {
            cmap = macRomanCmap;
        } else if (macRomanCmap >= 0) {
            cmap = macRomanCmap;
            useMacRoman = true;
        }
    } else {
        if (msSymbolCmap >= 0) {
            cmap = msSymbolCmap;
        } else if (macRomanCmap >= 0) {
            cmap = macRomanCmap;
        }
    }

    // reverse map the char names through MacRomanEncoding, then map the
    // char codes through the cmap
    if (useMacRoman) {
        for (i = 0; i < 256; ++i) {
            if ((charName = enc[i])) {
                if ((code = globalParams->getMacRomanCharCode(charName))) {
                    map[i] = ff->mapCodeToGID(cmap, code);
                }
            } else {
                map[i] = -1;
            }
        }

        // map Unicode through the cmap
    } else if (useUnicode) {
        const Unicode *uAux;
        for (i = 0; i < 256; ++i) {
            if (((charName = enc[i]) && (u = globalParams->mapNameToUnicodeAll(charName)))) {
                map[i] = ff->mapCodeToGID(cmap, u);
            } else {
                n = ctu->mapToUnicode((CharCode)i, &uAux);
                if (n > 0) {
                    map[i] = ff->mapCodeToGID(cmap, uAux[0]);
                } else {
                    map[i] = -1;
                }
            }
        }

        // map the char codes through the cmap, possibly with an offset of
        // 0xf000
    } else {
        for (i = 0; i < 256; ++i) {
            if (!(map[i] = ff->mapCodeToGID(cmap, i))) {
                map[i] = ff->mapCodeToGID(cmap, 0xf000 + i);
            }
        }
    }

    // try the TrueType 'post' table to handle any unmapped characters
    for (i = 0; i < 256; ++i) {
        if (map[i] <= 0 && (charName = enc[i])) {
            map[i] = ff->mapNameToGID(charName);
        }
    }

    return map;
}

Dict *Gfx8BitFont::getCharProcs()
{
    return charProcs.isDict() ? charProcs.getDict() : nullptr;
}

Object Gfx8BitFont::getCharProc(int code)
{
    if (enc[code] && charProcs.isDict()) {
        return charProcs.dictLookup(enc[code]);
    } else {
        return Object(objNull);
    }
}

Object Gfx8BitFont::getCharProcNF(int code)
{
    if (enc[code] && charProcs.isDict()) {
        return charProcs.dictLookupNF(enc[code]).copy();
    } else {
        return Object(objNull);
    }
}

Dict *Gfx8BitFont::getResources()
{
    return resources.isDict() ? resources.getDict() : nullptr;
}

//------------------------------------------------------------------------
// GfxCIDFont
//------------------------------------------------------------------------

struct cmpWidthExcepFunctor
{
    bool operator()(const GfxFontCIDWidthExcep w1, const GfxFontCIDWidthExcep w2) { return w1.first < w2.first; }
};

struct cmpWidthExcepVFunctor
{
    bool operator()(const GfxFontCIDWidthExcepV &w1, const GfxFontCIDWidthExcepV &w2) { return w1.first < w2.first; }
};

GfxCIDFont::GfxCIDFont(XRef *xref, const char *tagA, Ref idA, std::optional<std::string> &&nameA, GfxFontType typeA, Ref embFontIDA, Dict *fontDict) : GfxFont(tagA, idA, std::move(nameA), typeA, embFontIDA)
{
    Dict *desFontDict;
    Object desFontDictObj;
    Object obj1, obj2, obj3, obj4, obj5, obj6;
    int c1, c2;
    int excepsSize;

    ascent = 0.95;
    descent = -0.35;
    fontBBox[0] = fontBBox[1] = fontBBox[2] = fontBBox[3] = 0;
    collection = nullptr;
    ctu = nullptr;
    ctuUsesCharCode = true;
    widths.defWidth = 1.0;
    widths.defHeight = -1.0;
    widths.defVY = 0.880;
    widths.exceps = nullptr;
    widths.nExceps = 0;
    widths.excepsV = nullptr;
    widths.nExcepsV = 0;
    cidToGID = nullptr;
    cidToGIDLen = 0;

    // get the descendant font
    obj1 = fontDict->lookup("DescendantFonts");
    if (!obj1.isArray() || obj1.arrayGetLength() == 0) {
        error(errSyntaxError, -1, "Missing or empty DescendantFonts entry in Type 0 font");
        return;
    }
    desFontDictObj = obj1.arrayGet(0);
    if (!desFontDictObj.isDict()) {
        error(errSyntaxError, -1, "Bad descendant font in Type 0 font");
        return;
    }
    desFontDict = desFontDictObj.getDict();

    // get info from font descriptor
    readFontDescriptor(xref, desFontDict);

    //----- encoding info -----

    // char collection
    obj1 = desFontDict->lookup("CIDSystemInfo");
    if (!obj1.isDict()) {
        error(errSyntaxError, -1, "Missing CIDSystemInfo dictionary in Type 0 descendant font");
        return;
    }
    obj2 = obj1.dictLookup("Registry");
    obj3 = obj1.dictLookup("Ordering");
    if (!obj2.isString() || !obj3.isString()) {
        error(errSyntaxError, -1, "Invalid CIDSystemInfo dictionary in Type 0 descendant font");
        return;
    }
    collection = obj2.getString()->copy()->append('-')->append(obj3.getString());

    // look for a ToUnicode CMap
    if (!(ctu = readToUnicodeCMap(fontDict, 16, nullptr))) {
        ctuUsesCharCode = false;

        // use an identity mapping for the "Adobe-Identity" and
        // "Adobe-UCS" collections
        if (!collection->cmp("Adobe-Identity") || !collection->cmp("Adobe-UCS")) {
            ctu = CharCodeToUnicode::makeIdentityMapping();
        } else {
            // look for a user-supplied .cidToUnicode file
            if (!(ctu = globalParams->getCIDToUnicode(collection))) {
                // I'm not completely sure that this is the best thing to do
                // but it seems to produce better results when the .cidToUnicode
                // files from the poppler-data package are missing. At least
                // we know that assuming the Identity mapping is definitely wrong.
                //   -- jrmuizel
                static const char *knownCollections[] = {
                    "Adobe-CNS1", "Adobe-GB1", "Adobe-Japan1", "Adobe-Japan2", "Adobe-Korea1",
                };
                for (const char *knownCollection : knownCollections) {
                    if (collection->cmp(knownCollection) == 0) {
                        error(errSyntaxError, -1, "Missing language pack for '{0:t}' mapping", collection);
                        return;
                    }
                }
                error(errSyntaxError, -1, "Unknown character collection '{0:t}'", collection);
                // fall-through, assuming the Identity mapping -- this appears
                // to match Adobe's behavior
            }
        }
    }

    // encoding (i.e., CMap)
    obj1 = fontDict->lookup("Encoding");
    if (obj1.isNull()) {
        error(errSyntaxError, -1, "Missing Encoding entry in Type 0 font");
        return;
    }
    if (!(cMap = CMap::parse(nullptr, collection, &obj1))) {
        return;
    }
    if (cMap->getCMapName()) {
        encodingName = cMap->getCMapName()->toStr();
    } else {
        encodingName = "Custom";
    }

    // CIDToGIDMap (for embedded TrueType fonts)
    obj1 = desFontDict->lookup("CIDToGIDMap");
    if (obj1.isStream()) {
        cidToGIDLen = 0;
        unsigned int i = 64;
        cidToGID = (int *)gmallocn(i, sizeof(int));
        obj1.streamReset();
        while ((c1 = obj1.streamGetChar()) != EOF && (c2 = obj1.streamGetChar()) != EOF) {
            if (cidToGIDLen == i) {
                i *= 2;
                cidToGID = (int *)greallocn(cidToGID, i, sizeof(int));
            }
            cidToGID[cidToGIDLen++] = (c1 << 8) + c2;
        }
    } else if (!obj1.isName("Identity") && !obj1.isNull()) {
        error(errSyntaxError, -1, "Invalid CIDToGIDMap entry in CID font");
    }

    //----- character metrics -----

    // default char width
    obj1 = desFontDict->lookup("DW");
    if (obj1.isInt()) {
        widths.defWidth = obj1.getInt() * 0.001;
    }

    // char width exceptions
    obj1 = desFontDict->lookup("W");
    if (obj1.isArray()) {
        excepsSize = 0;
        int i = 0;
        while (i + 1 < obj1.arrayGetLength()) {
            obj2 = obj1.arrayGet(i);
            obj3 = obj1.arrayGet(i + 1);
            if (obj2.isInt() && obj3.isInt() && i + 2 < obj1.arrayGetLength()) {
                obj4 = obj1.arrayGet(i + 2);
                if (obj4.isNum()) {
                    if (widths.nExceps == excepsSize) {
                        excepsSize += 16;
                        widths.exceps = (GfxFontCIDWidthExcep *)greallocn(widths.exceps, excepsSize, sizeof(GfxFontCIDWidthExcep));
                    }
                    widths.exceps[widths.nExceps].first = obj2.getInt();
                    widths.exceps[widths.nExceps].last = obj3.getInt();
                    widths.exceps[widths.nExceps].width = obj4.getNum() * 0.001;
                    ++widths.nExceps;
                } else {
                    error(errSyntaxError, -1, "Bad widths array in Type 0 font");
                }
                i += 3;
            } else if (obj2.isInt() && obj3.isArray()) {
                if (widths.nExceps + obj3.arrayGetLength() > excepsSize) {
                    excepsSize = (widths.nExceps + obj3.arrayGetLength() + 15) & ~15;
                    widths.exceps = (GfxFontCIDWidthExcep *)greallocn(widths.exceps, excepsSize, sizeof(GfxFontCIDWidthExcep));
                }
                int j = obj2.getInt();
                if (likely(j < INT_MAX - obj3.arrayGetLength())) {
                    for (int k = 0; k < obj3.arrayGetLength(); ++k) {
                        obj4 = obj3.arrayGet(k);
                        if (obj4.isNum()) {
                            widths.exceps[widths.nExceps].first = j;
                            widths.exceps[widths.nExceps].last = j;
                            widths.exceps[widths.nExceps].width = obj4.getNum() * 0.001;
                            ++j;
                            ++widths.nExceps;
                        } else {
                            error(errSyntaxError, -1, "Bad widths array in Type 0 font");
                        }
                    }
                }
                i += 2;
            } else {
                error(errSyntaxError, -1, "Bad widths array in Type 0 font");
                ++i;
            }
        }
        std::sort(widths.exceps, widths.exceps + widths.nExceps, cmpWidthExcepFunctor());
    }

    // default metrics for vertical font
    obj1 = desFontDict->lookup("DW2");
    if (obj1.isArray() && obj1.arrayGetLength() == 2) {
        obj2 = obj1.arrayGet(0);
        if (obj2.isNum()) {
            widths.defVY = obj2.getNum() * 0.001;
        }
        obj2 = obj1.arrayGet(1);
        if (obj2.isNum()) {
            widths.defHeight = obj2.getNum() * 0.001;
        }
    }

    // char metric exceptions for vertical font
    obj1 = desFontDict->lookup("W2");
    if (obj1.isArray()) {
        excepsSize = 0;
        int i = 0;
        while (i + 1 < obj1.arrayGetLength()) {
            obj2 = obj1.arrayGet(i);
            obj3 = obj1.arrayGet(i + 1);
            if (obj2.isInt() && obj3.isInt() && i + 4 < obj1.arrayGetLength()) {
                if ((obj4 = obj1.arrayGet(i + 2), obj4.isNum()) && (obj5 = obj1.arrayGet(i + 3), obj5.isNum()) && (obj6 = obj1.arrayGet(i + 4), obj6.isNum())) {
                    if (widths.nExcepsV == excepsSize) {
                        excepsSize += 16;
                        widths.excepsV = (GfxFontCIDWidthExcepV *)greallocn(widths.excepsV, excepsSize, sizeof(GfxFontCIDWidthExcepV));
                    }
                    widths.excepsV[widths.nExcepsV].first = obj2.getInt();
                    widths.excepsV[widths.nExcepsV].last = obj3.getInt();
                    widths.excepsV[widths.nExcepsV].height = obj4.getNum() * 0.001;
                    widths.excepsV[widths.nExcepsV].vx = obj5.getNum() * 0.001;
                    widths.excepsV[widths.nExcepsV].vy = obj6.getNum() * 0.001;
                    ++widths.nExcepsV;
                } else {
                    error(errSyntaxError, -1, "Bad widths (W2) array in Type 0 font");
                }
                i += 5;
            } else if (obj2.isInt() && obj3.isArray()) {
                if (widths.nExcepsV + obj3.arrayGetLength() / 3 > excepsSize) {
                    excepsSize = (widths.nExcepsV + obj3.arrayGetLength() / 3 + 15) & ~15;
                    widths.excepsV = (GfxFontCIDWidthExcepV *)greallocn(widths.excepsV, excepsSize, sizeof(GfxFontCIDWidthExcepV));
                }
                int j = obj2.getInt();
                for (int k = 0; k < obj3.arrayGetLength(); k += 3) {
                    if ((obj4 = obj3.arrayGet(k), obj4.isNum()) && (obj5 = obj3.arrayGet(k + 1), obj5.isNum()) && (obj6 = obj3.arrayGet(k + 2), obj6.isNum())) {
                        widths.excepsV[widths.nExcepsV].first = j;
                        widths.excepsV[widths.nExcepsV].last = j;
                        widths.excepsV[widths.nExcepsV].height = obj4.getNum() * 0.001;
                        widths.excepsV[widths.nExcepsV].vx = obj5.getNum() * 0.001;
                        widths.excepsV[widths.nExcepsV].vy = obj6.getNum() * 0.001;
                        ++j;
                        ++widths.nExcepsV;
                    } else {
                        error(errSyntaxError, -1, "Bad widths (W2) array in Type 0 font");
                    }
                }
                i += 2;
            } else {
                error(errSyntaxError, -1, "Bad widths (W2) array in Type 0 font");
                ++i;
            }
        }
        std::sort(widths.excepsV, widths.excepsV + widths.nExcepsV, cmpWidthExcepVFunctor());
    }

    ok = true;
}

GfxCIDFont::~GfxCIDFont()
{
    if (collection) {
        delete collection;
    }
    if (ctu) {
        ctu->decRefCnt();
    }
    gfree(widths.exceps);
    gfree(widths.excepsV);
    if (cidToGID) {
        gfree(cidToGID);
    }
}

int GfxCIDFont::getNextChar(const char *s, int len, CharCode *code, Unicode const **u, int *uLen, double *dx, double *dy, double *ox, double *oy) const
{
    CID cid;
    CharCode dummy;
    double w, h, vx, vy;
    int n, a, b, m;

    if (!cMap) {
        *code = 0;
        *uLen = 0;
        *dx = *dy = *ox = *oy = 0;
        return 1;
    }

    *code = (CharCode)(cid = cMap->getCID(s, len, &dummy, &n));
    if (ctu) {
        if (hasToUnicode) {
            int i = 0, c = 0;
            while (i < n) {
                c = (c << 8) + (s[i] & 0xff);
                ++i;
            }
            *uLen = ctu->mapToUnicode(c, u);
        } else {
            *uLen = ctu->mapToUnicode(cid, u);
        }
    } else {
        *uLen = 0;
    }

    // horizontal
    if (cMap->getWMode() == 0) {
        w = getWidth(cid);
        h = vx = vy = 0;

        // vertical
    } else {
        w = 0;
        h = widths.defHeight;
        vx = getWidth(cid) / 2;
        vy = widths.defVY;
        if (widths.nExcepsV > 0 && cid >= widths.excepsV[0].first) {
            a = 0;
            b = widths.nExcepsV;
            // invariant: widths.excepsV[a].first <= cid < widths.excepsV[b].first
            while (b - a > 1) {
                m = (a + b) / 2;
                if (widths.excepsV[m].last <= cid) {
                    a = m;
                } else {
                    b = m;
                }
            }
            if (cid <= widths.excepsV[a].last) {
                h = widths.excepsV[a].height;
                vx = widths.excepsV[a].vx;
                vy = widths.excepsV[a].vy;
            }
        }
    }

    *dx = w;
    *dy = h;
    *ox = vx;
    *oy = vy;

    return n;
}

int GfxCIDFont::getWMode() const
{
    return cMap ? cMap->getWMode() : 0;
}

const CharCodeToUnicode *GfxCIDFont::getToUnicode() const
{
    return ctu;
}

const GooString *GfxCIDFont::getCollection() const
{
    return cMap ? cMap->getCollection() : nullptr;
}

int GfxCIDFont::mapCodeToGID(FoFiTrueType *ff, int cmapi, Unicode unicode, bool wmode)
{
    unsigned short gid = ff->mapCodeToGID(cmapi, unicode);
    if (wmode) {
        unsigned short vgid = ff->mapToVertGID(gid);
        if (vgid != 0) {
            gid = vgid;
        }
    }
    return gid;
}

int *GfxCIDFont::getCodeToGIDMap(FoFiTrueType *ff, int *codeToGIDLen)
{
#define N_UCS_CANDIDATES 2
    /* space characters */
    static const unsigned long spaces[] = { 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x00A0, 0x200B, 0x2060, 0x3000, 0xFEFF, 0 };
    static const char *adobe_cns1_cmaps[] = { "UniCNS-UTF32-V", "UniCNS-UCS2-V", "UniCNS-UTF32-H", "UniCNS-UCS2-H", nullptr };
    static const char *adobe_gb1_cmaps[] = { "UniGB-UTF32-V", "UniGB-UCS2-V", "UniGB-UTF32-H", "UniGB-UCS2-H", nullptr };
    static const char *adobe_japan1_cmaps[] = { "UniJIS-UTF32-V", "UniJIS-UCS2-V", "UniJIS-UTF32-H", "UniJIS-UCS2-H", nullptr };
    static const char *adobe_japan2_cmaps[] = { "UniHojo-UTF32-V", "UniHojo-UCS2-V", "UniHojo-UTF32-H", "UniHojo-UCS2-H", nullptr };
    static const char *adobe_korea1_cmaps[] = { "UniKS-UTF32-V", "UniKS-UCS2-V", "UniKS-UTF32-H", "UniKS-UCS2-H", nullptr };
    static struct CMapListEntry
    {
        const char *collection;
        const char *scriptTag;
        const char *languageTag;
        const char *toUnicodeMap;
        const char **CMaps;
    } CMapList[] = { {
                             "Adobe-CNS1",
                             "hani",
                             "CHN ",
                             "Adobe-CNS1-UCS2",
                             adobe_cns1_cmaps,
                     },
                     {
                             "Adobe-GB1",
                             "hani",
                             "CHN ",
                             "Adobe-GB1-UCS2",
                             adobe_gb1_cmaps,
                     },
                     {
                             "Adobe-Japan1",
                             "kana",
                             "JAN ",
                             "Adobe-Japan1-UCS2",
                             adobe_japan1_cmaps,
                     },
                     {
                             "Adobe-Japan2",
                             "kana",
                             "JAN ",
                             "Adobe-Japan2-UCS2",
                             adobe_japan2_cmaps,
                     },
                     {
                             "Adobe-Korea1",
                             "hang",
                             "KOR ",
                             "Adobe-Korea1-UCS2",
                             adobe_korea1_cmaps,
                     },
                     { nullptr, nullptr, nullptr, nullptr, nullptr } };
    Unicode *humap = nullptr;
    Unicode *vumap = nullptr;
    Unicode *tumap = nullptr;
    int *codeToGID = nullptr;
    int i;
    unsigned long code;
    int wmode;
    const char **cmapName;
    CMapListEntry *lp;
    int cmap;
    int cmapPlatform, cmapEncoding;
    Ref embID;

    *codeToGIDLen = 0;
    if (!ctu || !getCollection()) {
        return nullptr;
    }

    if (getEmbeddedFontID(&embID)) {
        if (getCollection()->cmp("Adobe-Identity") == 0) {
            return nullptr;
        }

        /* if this font is embedded font,
         * CIDToGIDMap should be embedded in PDF file
         * and already set. So return it.
         */
        *codeToGIDLen = getCIDToGIDLen();
        return getCIDToGID();
    }

    /* we use only unicode cmap */
    cmap = -1;
    for (i = 0; i < ff->getNumCmaps(); ++i) {
        cmapPlatform = ff->getCmapPlatform(i);
        cmapEncoding = ff->getCmapEncoding(i);
        if (cmapPlatform == 3 && cmapEncoding == 10) {
            /* UCS-4 */
            cmap = i;
            /* use UCS-4 cmap */
            break;
        } else if (cmapPlatform == 3 && cmapEncoding == 1) {
            /* Unicode */
            cmap = i;
        } else if (cmapPlatform == 0 && cmap < 0) {
            cmap = i;
        }
    }
    if (cmap < 0) {
        return nullptr;
    }

    wmode = getWMode();
    for (lp = CMapList; lp->collection != nullptr; lp++) {
        if (strcmp(lp->collection, getCollection()->c_str()) == 0) {
            break;
        }
    }
    const unsigned int n = 65536;
    humap = new Unicode[n * N_UCS_CANDIDATES];
    memset(humap, 0, sizeof(Unicode) * n * N_UCS_CANDIDATES);
    if (lp->collection != nullptr) {
        CharCodeToUnicode *tctu;
        GooString tname(lp->toUnicodeMap);

        if ((tctu = CharCodeToUnicode::parseCMapFromFile(&tname, 16)) != nullptr) {
            tumap = new Unicode[n];
            CharCode cid;
            for (cid = 0; cid < n; cid++) {
                int len;
                const Unicode *ucodes;

                len = tctu->mapToUnicode(cid, &ucodes);
                if (len == 1) {
                    tumap[cid] = ucodes[0];
                } else {
                    /* if not single character, ignore it */
                    tumap[cid] = 0;
                }
            }
            delete tctu;
        }
        vumap = new Unicode[n];
        memset(vumap, 0, sizeof(Unicode) * n);
        for (cmapName = lp->CMaps; *cmapName != nullptr; cmapName++) {
            GooString cname(*cmapName);

            std::shared_ptr<CMap> cnameCMap;
            if ((cnameCMap = globalParams->getCMap(getCollection(), &cname)) != nullptr) {
                if (cnameCMap->getWMode()) {
                    cnameCMap->setReverseMap(vumap, n, 1);
                } else {
                    cnameCMap->setReverseMap(humap, n, N_UCS_CANDIDATES);
                }
            }
        }
        ff->setupGSUB(lp->scriptTag, lp->languageTag);
    } else {
        if (getCollection()->cmp("Adobe-Identity") == 0) {
            error(errSyntaxError, -1, "non-embedded font using identity encoding: {0:s}", name ? name->c_str() : "(null)");
        } else {
            error(errSyntaxError, -1, "Unknown character collection {0:t}\n", getCollection());
        }
        if (ctu) {
            CharCode cid;
            for (cid = 0; cid < n; cid++) {
                const Unicode *ucode;

                if (ctu->mapToUnicode(cid, &ucode)) {
                    humap[cid * N_UCS_CANDIDATES] = ucode[0];
                } else {
                    humap[cid * N_UCS_CANDIDATES] = 0;
                }
                for (i = 1; i < N_UCS_CANDIDATES; i++) {
                    humap[cid * N_UCS_CANDIDATES + i] = 0;
                }
            }
        }
    }
    // map CID -> Unicode -> GID
    codeToGID = (int *)gmallocn(n, sizeof(int));
    for (code = 0; code < n; ++code) {
        Unicode unicode;
        unsigned long gid;

        unicode = 0;
        gid = 0;
        if (humap != nullptr) {
            for (i = 0; i < N_UCS_CANDIDATES && gid == 0 && (unicode = humap[code * N_UCS_CANDIDATES + i]) != 0; i++) {
                gid = mapCodeToGID(ff, cmap, unicode, false);
            }
        }
        if (gid == 0 && vumap != nullptr) {
            unicode = vumap[code];
            if (unicode != 0) {
                gid = mapCodeToGID(ff, cmap, unicode, true);
                if (gid == 0 && tumap != nullptr) {
                    if ((unicode = tumap[code]) != 0) {
                        gid = mapCodeToGID(ff, cmap, unicode, true);
                    }
                }
            }
        }
        if (gid == 0 && tumap != nullptr) {
            if ((unicode = tumap[code]) != 0) {
                gid = mapCodeToGID(ff, cmap, unicode, false);
            }
        }
        if (gid == 0) {
            /* special handling space characters */
            const unsigned long *p;

            if (humap != nullptr) {
                unicode = humap[code];
            }
            if (unicode != 0) {
                /* check if code is space character , so map code to 0x0020 */
                for (p = spaces; *p != 0; p++) {
                    if (*p == unicode) {
                        unicode = 0x20;
                        gid = mapCodeToGID(ff, cmap, unicode, wmode);
                        break;
                    }
                }
            }
        }
        codeToGID[code] = gid;
    }
    *codeToGIDLen = n;
    if (humap != nullptr) {
        delete[] humap;
    }
    if (tumap != nullptr) {
        delete[] tumap;
    }
    if (vumap != nullptr) {
        delete[] vumap;
    }
    return codeToGID;
}

double GfxCIDFont::getWidth(CID cid) const
{
    double w;
    int a, b, m;

    w = widths.defWidth;
    if (widths.nExceps > 0 && cid >= widths.exceps[0].first) {
        a = 0;
        b = widths.nExceps;
        // invariant: widths.exceps[a].first <= cid < widths.exceps[b].first
        while (b - a > 1) {
            m = (a + b) / 2;
            if (widths.exceps[m].first <= cid) {
                a = m;
            } else {
                b = m;
            }
        }
        if (cid <= widths.exceps[a].last) {
            w = widths.exceps[a].width;
        }
    }
    return w;
}

double GfxCIDFont::getWidth(char *s, int len) const
{
    int nUsed;
    CharCode c;

    CID cid = cMap->getCID(s, len, &c, &nUsed);
    return getWidth(cid);
}

//------------------------------------------------------------------------
// GfxFontDict
//------------------------------------------------------------------------

GfxFontDict::GfxFontDict(XRef *xref, Ref *fontDictRef, Dict *fontDict)
{
    Ref r;

    fonts.resize(fontDict->getLength());
    for (std::size_t i = 0; i < fonts.size(); ++i) {
        const Object &obj1 = fontDict->getValNF(i);
        Object obj2 = obj1.fetch(xref);
        if (obj2.isDict()) {
            if (obj1.isRef()) {
                r = obj1.getRef();
            } else if (fontDictRef) {
                // legal generation numbers are five digits, so we use a
                // 6-digit number here
                r.gen = 100000 + fontDictRef->num;
                r.num = i;
            } else {
                // no indirect reference for this font, or for the containing
                // font dict, so hash the font and use that
                r.gen = 100000;
                r.num = hashFontObject(&obj2);
            }
            fonts[i] = GfxFont::makeFont(xref, fontDict->getKey(i), r, obj2.getDict());
            if (fonts[i] && !fonts[i]->isOk()) {
                // XXX: it may be meaningful to distinguish between
                // NULL and !isOk() so that when we do lookups
                // we can tell the difference between a missing font
                // and a font that is just !isOk()
                fonts[i].reset();
            }
        } else {
            error(errSyntaxError, -1, "font resource is not a dictionary");
            fonts[i] = nullptr;
        }
    }
}

std::shared_ptr<GfxFont> GfxFontDict::lookup(const char *tag) const
{
    for (const auto &font : fonts) {
        if (font && font->matches(tag)) {
            return font;
        }
    }
    return nullptr;
}

// FNV-1a hash
class FNVHash
{
public:
    FNVHash() { h = 2166136261U; }

    void hash(char c)
    {
        h ^= c & 0xff;
        h *= 16777619;
    }

    void hash(const char *p, int n)
    {
        int i;
        for (i = 0; i < n; ++i) {
            hash(p[i]);
        }
    }

    int get31() { return (h ^ (h >> 31)) & 0x7fffffff; }

private:
    unsigned int h;
};

int GfxFontDict::hashFontObject(Object *obj)
{
    FNVHash h;

    hashFontObject1(obj, &h);
    return h.get31();
}

void GfxFontDict::hashFontObject1(const Object *obj, FNVHash *h)
{
    const GooString *s;
    const char *p;
    double r;
    int n, i;

    switch (obj->getType()) {
    case objBool:
        h->hash('b');
        h->hash(obj->getBool() ? 1 : 0);
        break;
    case objInt:
        h->hash('i');
        n = obj->getInt();
        h->hash((char *)&n, sizeof(int));
        break;
    case objReal:
        h->hash('r');
        r = obj->getReal();
        h->hash((char *)&r, sizeof(double));
        break;
    case objString:
        h->hash('s');
        s = obj->getString();
        h->hash(s->c_str(), s->getLength());
        break;
    case objName:
        h->hash('n');
        p = obj->getName();
        h->hash(p, (int)strlen(p));
        break;
    case objNull:
        h->hash('z');
        break;
    case objArray:
        h->hash('a');
        n = obj->arrayGetLength();
        h->hash((char *)&n, sizeof(int));
        for (i = 0; i < n; ++i) {
            const Object &obj2 = obj->arrayGetNF(i);
            hashFontObject1(&obj2, h);
        }
        break;
    case objDict:
        h->hash('d');
        n = obj->dictGetLength();
        h->hash((char *)&n, sizeof(int));
        for (i = 0; i < n; ++i) {
            p = obj->dictGetKey(i);
            h->hash(p, (int)strlen(p));
            const Object &obj2 = obj->dictGetValNF(i);
            hashFontObject1(&obj2, h);
        }
        break;
    case objStream:
        // this should never happen - streams must be indirect refs
        break;
    case objRef:
        h->hash('f');
        n = obj->getRefNum();
        h->hash((char *)&n, sizeof(int));
        n = obj->getRefGen();
        h->hash((char *)&n, sizeof(int));
        break;
    default:
        h->hash('u');
        break;
    }
}
