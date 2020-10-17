//========================================================================
//
// BuiltinFont.h
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2018, 2020 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef BUILTINFONT_H
#define BUILTINFONT_H

#include "BuiltinFontWidth.h"

//------------------------------------------------------------------------

using GetWidthFunction = const BuiltinFontWidth *(*)(const char *str, size_t len);

struct BuiltinFont
{
    const char *name;
    const char **defaultBaseEnc;
    short ascent;
    short descent;
    short bbox[4];
    GetWidthFunction f;

    bool getWidth(const char *n, unsigned short *w) const
    {
        const struct BuiltinFontWidth *bfw = f(n, strlen(n));
        if (!bfw) {
            return false;
        }

        *w = bfw->width;
        return true;
    }
};

//------------------------------------------------------------------------

extern "C" {
// define the gperf generated Lookup functions
const struct BuiltinFontWidth *CourierWidthsLookup(const char *str, size_t len);
const struct BuiltinFontWidth *CourierBoldWidthsLookup(const char *str, size_t len);
const struct BuiltinFontWidth *CourierBoldObliqueWidthsLookup(const char *str, size_t len);
const struct BuiltinFontWidth *CourierObliqueWidthsLookup(const char *str, size_t len);
const struct BuiltinFontWidth *HelveticaWidthsLookup(const char *str, size_t len);
const struct BuiltinFontWidth *HelveticaBoldWidthsLookup(const char *str, size_t len);
const struct BuiltinFontWidth *HelveticaBoldObliqueWidthsLookup(const char *str, size_t len);
const struct BuiltinFontWidth *HelveticaObliqueWidthsLookup(const char *str, size_t len);
const struct BuiltinFontWidth *SymbolWidthsLookup(const char *str, size_t len);
const struct BuiltinFontWidth *TimesBoldWidthsLookup(const char *str, size_t len);
const struct BuiltinFontWidth *TimesBoldItalicWidthsLookup(const char *str, size_t len);
const struct BuiltinFontWidth *TimesItalicWidthsLookup(const char *str, size_t len);
const struct BuiltinFontWidth *TimesRomanWidthsLookup(const char *str, size_t len);
const struct BuiltinFontWidth *ZapfDingbatsWidthsLookup(const char *str, size_t len);
}

static const BuiltinFont builtinFonts[] = { { "Courier", standardEncoding, 629, -157, { -23, -250, 715, 805 }, &CourierWidthsLookup },
                                            { "Courier-Bold", standardEncoding, 629, -157, { -113, -250, 749, 801 }, &CourierBoldWidthsLookup },
                                            { "Courier-BoldOblique", standardEncoding, 629, -157, { -57, -250, 869, 801 }, &CourierBoldObliqueWidthsLookup },
                                            { "Courier-Oblique", standardEncoding, 629, -157, { -27, -250, 849, 805 }, &CourierObliqueWidthsLookup },
                                            { "Helvetica", standardEncoding, 718, -207, { -166, -225, 1000, 931 }, &HelveticaWidthsLookup },
                                            { "Helvetica-Bold", standardEncoding, 718, -207, { -170, -228, 1003, 962 }, &HelveticaBoldWidthsLookup },
                                            { "Helvetica-BoldOblique", standardEncoding, 718, -207, { -174, -228, 1114, 962 }, &HelveticaBoldObliqueWidthsLookup },
                                            { "Helvetica-Oblique", standardEncoding, 718, -207, { -170, -225, 1116, 931 }, &HelveticaObliqueWidthsLookup },
                                            { "Symbol", symbolEncoding, 1010, -293, { -180, -293, 1090, 1010 }, &SymbolWidthsLookup },
                                            { "Times-Bold", standardEncoding, 683, -217, { -168, -218, 1000, 935 }, &TimesBoldWidthsLookup },
                                            { "Times-BoldItalic", standardEncoding, 683, -217, { -200, -218, 996, 921 }, &TimesBoldItalicWidthsLookup },
                                            { "Times-Italic", standardEncoding, 683, -217, { -169, -217, 1010, 883 }, &TimesItalicWidthsLookup },
                                            { "Times-Roman", standardEncoding, 683, -217, { -168, -218, 1000, 898 }, &TimesRomanWidthsLookup },
                                            { "ZapfDingbats", zapfDingbatsEncoding, 820, -143, { -1, -143, 981, 820 }, &ZapfDingbatsWidthsLookup } };

static const BuiltinFont *builtinFontSubst[] = { &builtinFonts[0], &builtinFonts[3], &builtinFonts[1],  &builtinFonts[2],  &builtinFonts[4], &builtinFonts[7],
                                                 &builtinFonts[5], &builtinFonts[6], &builtinFonts[12], &builtinFonts[11], &builtinFonts[9], &builtinFonts[10] };

//------------------------------------------------------------------------

#endif
