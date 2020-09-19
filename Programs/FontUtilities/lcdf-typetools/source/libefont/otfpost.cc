// -*- related-file-name: "../include/efont/otfpost.hh" -*-

/* otfpost.{cc,hh} -- OpenType post table
 *
 * Copyright (c) 2006-2019 Eddie Kohler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version. This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <efont/otfpost.hh>
#include <lcdf/error.hh>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <efont/otfdata.hh>     // for ntohl()

#define USHORT_AT(d)            (Data::u16_aligned(d))
#define SHORT_AT(d)             (Data::s16_aligned(d))
#define ULONG_AT(d)             (Data::u32_aligned(d))
#define LONG_AT(d)              (Data::s32_aligned(d))

namespace Efont { namespace OpenType {

static const char * const mac_names[] = {
    ".notdef", ".null", "nonmarkingreturn", "space",            // 0-3
    "exclam", "quotedbl", "numbersign", "dollar",               // 4-7
    "percent", "ampersand", "quotesingle", "parenleft",         // 8-11
    "parenright", "asterisk", "plus", "comma",                  // 12-15
    "hyphen", "period", "slash", "zero",                        // 16-19
    "one", "two", "three", "four",                              // 20-23
    "five", "six", "seven", "eight",                            // 24-27
    "nine", "colon", "semicolon", "less",                       // 28-31
    "equal", "greater", "question", "at",                       // 32-35
    "A", "B", "C", "D",                                         // 36-39
    "E", "F", "G", "H",                                         // 40-43
    "I", "J", "K", "L",                                         // 44-47
    "M", "N", "O", "P",                                         // 48-51
    "Q", "R", "S", "T",                                         // 52-55
    "U", "V", "W", "X",                                         // 56-59
    "Y", "Z", "bracketleft", "backslash",                       // 60-63
    "bracketright", "asciicircum", "underscore", "grave",       // 64-67
    "a", "b", "c", "d",                                         // 68-71
    "e", "f", "g", "h",                                         // 72-75
    "i", "j", "k", "l",                                         // 76-79
    "m", "n", "o", "p",                                         // 80-83
    "q", "r", "s", "t",                                         // 84-87
    "u", "v", "w", "x",                                         // 88-91
    "y", "z", "braceleft", "bar",                               // 92-95
    "braceright", "asciitilde", "Adieresis", "Aring",           // 96-99
    "Ccedilla", "Eacute", "Ntilde", "Odieresis",                // 100-103
    "Udieresis", "aacute", "agrave", "acircumflex",             // 104-107
    "adieresis", "atilde", "aring", "ccedilla",                 // 108-111
    "eacute", "egrave", "ecircumflex", "edieresis",             // 112-115
    "iacute", "igrave", "icircumflex", "idieresis",             // 116-119
    "ntilde", "oacute", "ograve", "ocircumflex",                // 120-123
    "odieresis", "otilde", "uacute", "ugrave",                  // 124-127
    "ucircumflex", "udieresis", "dagger", "degree",             // 128-131
    "cent", "sterling", "section", "bullet",                    // 132-135
    "paragraph", "germandbls", "registered", "copyright",       // 136-139
    "trademark", "acute", "dieresis", "notequal",               // 140-143
    "AE", "Oslash", "infinity", "plusminus",                    // 144-147
    "lessequal", "greaterequal", "yen", "mu",                   // 148-151
    "partialdiff", "summation", "product", "pi",                // 152-155
    "integral", "ordfeminine", "ordmasculine", "Omega",         // 156-159
    "ae", "oslash", "questiondown", "exclamdown",               // 160-163
    "logicalnot", "radical", "florin", "approxequal",           // 164-167
    "Delta", "guillemotleft", "guillemotright", "ellipsis",     // 168-171
    "nonbreakingspace", "Agrave", "Atilde", "Otilde",           // 172-175
    "OE", "oe", "endash", "emdash",                             // 176-179
    "quotedblleft", "quotedblright", "quoteleft", "quoteright", // 180-183
    "divide", "lozenge", "ydieresis", "Ydieresis",              // 184-187
    "fraction", "currency", "guilsinglleft", "guilsinglright",  // 188-191
    "fi", "fl", "daggerdbl", "periodcentered",                  // 192-195
    "quotesinglbase", "quotedblbase", "perthousand", "Acircumflex", // 196-199
    "Ecircumflex", "Aacute", "Edieresis", "Egrave",             // 200-203
    "Iacute", "Icircumflex", "Idieresis", "Igrave",             // 204-207
    "Oacute", "Ocircumflex", "apple", "Ograve",                 // 208-211
    "Uacute", "Ucircumflex", "Ugrave", "dotlessi",              // 212-215
    "circumflex", "tilde", "macron", "breve",                   // 216-219
    "dotaccent", "ring", "cedilla", "hungarumlaut",             // 220-223
    "ogonek", "caron", "Lslash", "lslash",                      // 224-227
    "Scaron", "scaron", "Zcaron", "zcaron",                     // 228-231
    "brokenbar", "Eth", "eth", "Yacute",                        // 232-235
    "yacute", "Thorn", "thorn", "minus",                        // 236-239
    "multiply", "onesuperior", "twosuperior", "threesuperior",  // 240-243
    "onehalf", "onequarter", "threequarters", "franc",          // 244-247
    "Gbreve", "gbreve", "Idotaccent", "Scedilla",               // 248-251
    "scedilla", "Cacute", "cacute", "Ccaron",                   // 252-255
    "ccaron", "dcroat"                                          // 256-257
};


Post::Post(const String &s, ErrorHandler *errh)
    : _str(s), _version(0)
{
    _str.align_long();
    _error = parse_header(errh ? errh : ErrorHandler::silent_handler());
}

int
Post::parse_header(ErrorHandler *errh)
{
    // HEADER FORMAT:
    // 0        FIXED   version
    // 4        FIXED   italicAngle
    // 8        FWORD   underlinePosition
    // 10       FWORD   underlineThickness
    // 12       ULONG   isFixedPitch
    // 16       ULONG   minMemType42
    // 20       ULONG   maxMemType42
    // 24       ULONG   minMemType1
    // 28       ULONG   maxMemType1
    int len = _str.length();
    const uint8_t *data = _str.udata();
    if (HEADER_SIZE > len)
        return errh->error("OTF post table too small"), -EFAULT;
    _version = USHORT_AT(data); // ignore minor version number
                                // except that version 2.5 isn't compatible
    if (_version < 1 || _version > 3
        || (_version == 2 && USHORT_AT(data + 2) == 0x5000))
        return errh->error("bad post version number"), -ERANGE;
    if (_version == 2) {
        // VERSION 2.0 GLYPH NAMES FORMAT:
        // 32   USHORT  numberOfGlyphs
        // 34   USHORT  glyphNameIndex[mumberOfGlyphs]
        //      CHAR    names[...]
        if (HEADER_SIZE + 2 > len
            || ((_nglyphs = USHORT_AT(data + HEADER_SIZE)),
                HEADER_SIZE + 2 + 2 * _nglyphs > len))
            return errh->error("OTF post table too small for glyph map"), -EFAULT;
        int pos = HEADER_SIZE + 2 + 2 * _nglyphs;
        while (pos < len && pos + data[pos] < len) {
            _extend_glyph_names.push_back(pos);
            pos += 1 + data[pos];
        }
        const uint8_t *gni = data + HEADER_SIZE + 2;
        for (int i = 0, g; i < _nglyphs; ++i, gni += 2)
            if ((g = USHORT_AT(gni)) >= _extend_glyph_names.size() + N_MAC_GLYPHS)
                return errh->error("bad glyph name index in post");
    } else if (_version == 1)
        _nglyphs = N_MAC_GLYPHS;
    else
        _nglyphs = -1;

    return 0;
}

double
Post::italic_angle() const
{
    if (error() < 0)
        return -1;
    return (double) LONG_AT(_str.udata() + 4) / 65536.;
}

bool
Post::is_fixed_pitch() const
{
    if (error() < 0)
        return false;
    return ULONG_AT(_str.udata() + 12) != 0;
}

bool
Post::glyph_names(Vector<PermString> &gnames) const
{
    gnames.clear();
    if (error() < 0)
        return false;
    if (_version == 1) {
        for (int i = 0; i < N_MAC_GLYPHS; i++)
            gnames.push_back(mac_names[i]);
        return true;
    } else if (_version == 2) {
        const uint8_t *data = _str.udata();
        const uint8_t *gni = data + HEADER_SIZE + 2;
        for (int i = 0; i < _nglyphs; i++, gni += 2) {
            int g = USHORT_AT(gni);
            if (g < N_MAC_GLYPHS)
                gnames.push_back(mac_names[g]);
            else {
                const uint8_t *n = data + _extend_glyph_names[g - N_MAC_GLYPHS];
                gnames.push_back(PermString((const char *) n + 1, *n));
            }
        }
        return true;
    } else
        return false;
}

}}
