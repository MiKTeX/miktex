// -*- related-file-name: "../include/efont/cff.hh" -*-

/* cff.{cc,hh} -- Compact Font Format fonts
 *
 * Copyright (c) 1998-2019 Eddie Kohler
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
#include <efont/cff.hh>
#include <lcdf/error.hh>
#include <efont/t1item.hh>
#include <lcdf/straccum.hh>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <efont/t1unparser.hh>

#ifndef static_assert
#define static_assert(c, msg) switch ((int) (c)) case 0: case (c):
#endif

namespace Efont {

const char * const Cff::operator_names[] = {
    "version", "Notice", "FullName", "FamilyName",
    "Weight", "FontBBox", "BlueValues", "OtherBlues",
    "FamilyBlues", "FamilyOtherBlues", "StdHW", "StdVW",
    "UNKNOWN_12", "UniqueID", "XUID", "charset",
    "Encoding", "CharStrings", "Private", "Subrs",
    "defaultWidthX", "nominalWidthX", "UNKNOWN_22", "UNKNOWN_23",
    "UNKNOWN_24", "UNKNOWN_25", "UNKNOWN_26", "UNKNOWN_27",
    "UNKNOWN_28", "UNKNOWN_29", "UNKNOWN_30", "UNKNOWN_31",
    "Copyright", "isFixedPitch", "ItalicAngle", "UnderlinePosition",
    "UnderlineThickness", "PaintType", "CharstringType", "FontMatrix",
    "StrokeWidth", "BlueScale", "BlueShift", "BlueFuzz",
    "StemSnapH", "StemSnapV", "ForceBold", "UNKNOWN_12_15",
    "UNKNOWN_12_16", "LanguageGroup", "ExpansionFactor", "initialRandomSeed",
    "SyntheticBase", "PostScript", "BaseFontName", "BaseFontBlend",
    "UNKNOWN_12_24", "UNKNOWN_12_25", "UNKNOWN_12_26", "UNKNOWN_12_27",
    "UNKNOWN_12_28", "UNKNOWN_12_29", "ROS", "CIDFontVersion",
    "CIDFontRevision", "CIDFontType", "CIDCount", "UIDBase",
    "FDArray", "FDSelect", "FontName"
};

const int Cff::operator_types[] = {
    tSID, tSID, tSID, tSID,     // version, Notice, FullName, FamilyName
    tSID, tArray4, tP+tArray, tP+tArray, // Weight, FontBBox, BlueValues, OtherBlues
    tP+tArray, tP+tArray, tP+tNumber, tP+tNumber, // FamBlues, FamOthBlues, StdHW, StdVW
    tNone, tNumber, tArray, tOffset, // escape, UniqueID, XUID, charset
    tOffset, tOffset, tPrivateType, tP+tLocalOffset, // Encoding, CharStrings, Private, Subrs
    tP+tNumber, tP+tNumber, tNone, tNone, // defaultWX, nominalWX, 22, 23
    tNone, tNone, tNone, tNone, // 24, 25, 26, 27
    tNone, tNone, tNone, tNone, // 28, 29, 30, 31
    tSID, tBoolean, tNumber, tNumber, // Copyright, isFixedPitch, ItalicAngle, UnderlinePosition
    tNumber, tNumber, tNumber, tArray6, // UnderlineThickness, PaintType, CharstringType, FontMatrix
    tNumber, tP+tNumber, tP+tNumber, tP+tNumber, // StrokeWidth, BlueScale, BlueShift, BlueFuzz
    tP+tArray, tP+tArray, tP+tBoolean, tNone, // StemSnapH, StemSnapV, ForceBold, 12 15
    tNone, tP+tNumber, tP+tNumber, tP+tNumber, // 12 16, LanguageGroup, ExpansionFactor, initialRandomSeed
    tNumber, tSID, tSID, tArray, // SyntheticBase, PostScript, BaseFontName, BaseFontBlend
    tNone, tNone, tNone, tNone, // 12 24, 12 25, 12 26, 12 27
    tNone, tNone, tArray, tNumber, // 12 28, 12 29, ROS, CIDFontVersion
    tNumber, tNumber, tNumber, tNumber, // CIDFontRevision, CIDFontType, CIDCount, UIDBase
    tOffset, tOffset, tSID      // FDArray, FDSelect, FontName
};

static PermString::Initializer initializer;
static const char * const standard_strings[] = {
    // Automatically generated from Appendix A of the CFF specification; do
    // not edit. Size should be 391.
    ".notdef", "space", "exclam", "quotedbl", "numbersign", "dollar",
    "percent", "ampersand", "quoteright", "parenleft", "parenright",
    "asterisk", "plus", "comma", "hyphen", "period", "slash", "zero", "one",
    "two", "three", "four", "five", "six", "seven", "eight", "nine", "colon",
    "semicolon", "less", "equal", "greater", "question", "at", "A", "B", "C",
    "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R",
    "S", "T", "U", "V", "W", "X", "Y", "Z", "bracketleft", "backslash",
    "bracketright", "asciicircum", "underscore", "quoteleft", "a", "b", "c",
    "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r",
    "s", "t", "u", "v", "w", "x", "y", "z", "braceleft", "bar", "braceright",
    "asciitilde", "exclamdown", "cent", "sterling", "fraction", "yen",
    "florin", "section", "currency", "quotesingle", "quotedblleft",
    "guillemotleft", "guilsinglleft", "guilsinglright", "fi", "fl", "endash",
    "dagger", "daggerdbl", "periodcentered", "paragraph", "bullet",
    "quotesinglbase", "quotedblbase", "quotedblright", "guillemotright",
    "ellipsis", "perthousand", "questiondown", "grave", "acute", "circumflex",
    "tilde", "macron", "breve", "dotaccent", "dieresis", "ring", "cedilla",
    "hungarumlaut", "ogonek", "caron", "emdash", "AE", "ordfeminine", "Lslash",
    "Oslash", "OE", "ordmasculine", "ae", "dotlessi", "lslash", "oslash", "oe",
    "germandbls", "onesuperior", "logicalnot", "mu", "trademark", "Eth",
    "onehalf", "plusminus", "Thorn", "onequarter", "divide", "brokenbar",
    "degree", "thorn", "threequarters", "twosuperior", "registered", "minus",
    "eth", "multiply", "threesuperior", "copyright", "Aacute", "Acircumflex",
    "Adieresis", "Agrave", "Aring", "Atilde", "Ccedilla", "Eacute",
    "Ecircumflex", "Edieresis", "Egrave", "Iacute", "Icircumflex", "Idieresis",
    "Igrave", "Ntilde", "Oacute", "Ocircumflex", "Odieresis", "Ograve",
    "Otilde", "Scaron", "Uacute", "Ucircumflex", "Udieresis", "Ugrave",
    "Yacute", "Ydieresis", "Zcaron", "aacute", "acircumflex", "adieresis",
    "agrave", "aring", "atilde", "ccedilla", "eacute", "ecircumflex",
    "edieresis", "egrave", "iacute", "icircumflex", "idieresis", "igrave",
    "ntilde", "oacute", "ocircumflex", "odieresis", "ograve", "otilde",
    "scaron", "uacute", "ucircumflex", "udieresis", "ugrave", "yacute",
    "ydieresis", "zcaron", "exclamsmall", "Hungarumlautsmall",
    "dollaroldstyle", "dollarsuperior", "ampersandsmall", "Acutesmall",
    "parenleftsuperior", "parenrightsuperior", "twodotenleader",
    "onedotenleader", "zerooldstyle", "oneoldstyle", "twooldstyle",
    "threeoldstyle", "fouroldstyle", "fiveoldstyle", "sixoldstyle",
    "sevenoldstyle", "eightoldstyle", "nineoldstyle", "commasuperior",
    "threequartersemdash", "periodsuperior", "questionsmall", "asuperior",
    "bsuperior", "centsuperior", "dsuperior", "esuperior", "isuperior",
    "lsuperior", "msuperior", "nsuperior", "osuperior", "rsuperior",
    "ssuperior", "tsuperior", "ff", "ffi", "ffl", "parenleftinferior",
    "parenrightinferior", "Circumflexsmall", "hyphensuperior", "Gravesmall",
    "Asmall", "Bsmall", "Csmall", "Dsmall", "Esmall", "Fsmall", "Gsmall",
    "Hsmall", "Ismall", "Jsmall", "Ksmall", "Lsmall", "Msmall", "Nsmall",
    "Osmall", "Psmall", "Qsmall", "Rsmall", "Ssmall", "Tsmall", "Usmall",
    "Vsmall", "Wsmall", "Xsmall", "Ysmall", "Zsmall", "colonmonetary",
    "onefitted", "rupiah", "Tildesmall", "exclamdownsmall", "centoldstyle",
    "Lslashsmall", "Scaronsmall", "Zcaronsmall", "Dieresissmall", "Brevesmall",
    "Caronsmall", "Dotaccentsmall", "Macronsmall", "figuredash",
    "hypheninferior", "Ogoneksmall", "Ringsmall", "Cedillasmall",
    "questiondownsmall", "oneeighth", "threeeighths", "fiveeighths",
    "seveneighths", "onethird", "twothirds", "zerosuperior", "foursuperior",
    "fivesuperior", "sixsuperior", "sevensuperior", "eightsuperior",
    "ninesuperior", "zeroinferior", "oneinferior", "twoinferior",
    "threeinferior", "fourinferior", "fiveinferior", "sixinferior",
    "seveninferior", "eightinferior", "nineinferior", "centinferior",
    "dollarinferior", "periodinferior", "commainferior", "Agravesmall",
    "Aacutesmall", "Acircumflexsmall", "Atildesmall", "Adieresissmall",
    "Aringsmall", "AEsmall", "Ccedillasmall", "Egravesmall", "Eacutesmall",
    "Ecircumflexsmall", "Edieresissmall", "Igravesmall", "Iacutesmall",
    "Icircumflexsmall", "Idieresissmall", "Ethsmall", "Ntildesmall",
    "Ogravesmall", "Oacutesmall", "Ocircumflexsmall", "Otildesmall",
    "Odieresissmall", "OEsmall", "Oslashsmall", "Ugravesmall", "Uacutesmall",
    "Ucircumflexsmall", "Udieresissmall", "Yacutesmall", "Thornsmall",
    "Ydieresissmall", "001.000", "001.001", "001.002", "001.003", "Black",
    "Bold", "Book", "Light", "Medium", "Regular", "Roman", "Semibold"
};
static PermString standard_permstrings[Cff::NSTANDARD_STRINGS];
static HashMap<PermString, int> standard_permstrings_map(-1);

static const int standard_encoding[] = {
    // Automatically generated from Appendix B of the CFF specification; do
    // not edit. Size should be 256.
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
    9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
    19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
    59, 60, 61, 62, 63, 64, 65, 66, 67, 68,
    69, 70, 71, 72, 73, 74, 75, 76, 77, 78,
    79, 80, 81, 82, 83, 84, 85, 86, 87, 88,
    89, 90, 91, 92, 93, 94, 95, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 96, 97, 98, 99, 100, 101, 102, 103, 104,
    105, 106, 107, 108, 109, 110, 0, 111, 112, 113,
    114, 0, 115, 116, 117, 118, 119, 120, 121, 122,
    0, 123, 0, 124, 125, 126, 127, 128, 129, 130,
    131, 0, 132, 133, 0, 134, 135, 136, 137, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 138, 0, 139, 0, 0,
    0, 0, 140, 141, 142, 143, 0, 0, 0, 0,
    0, 144, 0, 0, 0, 145, 0, 0, 146, 147,
    148, 149, 0, 0, 0, 0
};

static const int expert_encoding[] = {
    // Automatically generated from Appendix B of the CFF specification; do
    // not edit. Size should be 256.
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 229, 230, 0, 231, 232, 233, 234,
    235, 236, 237, 238, 13, 14, 15, 99, 239, 240,
    241, 242, 243, 244, 245, 246, 247, 248, 27, 28,
    249, 250, 251, 252, 0, 253, 254, 255, 256, 257,
    0, 0, 0, 258, 0, 0, 259, 260, 261, 262,
    0, 0, 263, 264, 265, 0, 266, 109, 110, 267,
    268, 269, 0, 270, 271, 272, 273, 274, 275, 276,
    277, 278, 279, 280, 281, 282, 283, 284, 285, 286,
    287, 288, 289, 290, 291, 292, 293, 294, 295, 296,
    297, 298, 299, 300, 301, 302, 303, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 304, 305, 306, 0, 0, 307, 308, 309, 310,
    311, 0, 312, 0, 0, 313, 0, 0, 314, 315,
    0, 0, 316, 317, 318, 0, 0, 0, 158, 155,
    163, 319, 320, 321, 322, 323, 324, 325, 0, 0,
    326, 150, 164, 169, 327, 328, 329, 330, 331, 332,
    333, 334, 335, 336, 337, 338, 339, 340, 341, 342,
    343, 344, 345, 346, 347, 348, 349, 350, 351, 352,
    353, 354, 355, 356, 357, 358, 359, 360, 361, 362,
    363, 364, 365, 366, 367, 368, 369, 370, 371, 372,
    373, 374, 375, 376, 377, 378
};

static const int iso_adobe_charset[] = {
    // Automatically generated from Appendix C of the CFF specification; do
    // not edit.
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
    50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
    60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
    70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
    90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
    100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
    110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129,
    130, 131, 132, 133, 134, 135, 136, 137, 138, 139,
    140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169,
    170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189,
    190, 191, 192, 193, 194, 195, 196, 197, 198, 199,
    200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
    210, 211, 212, 213, 214, 215, 216, 217, 218, 219,
    220, 221, 222, 223, 224, 225, 226, 227, 228
};

static const int expert_charset[] = {
    // Automatically generated from Appendix C of the CFF specification; do
    // not edit.
    0, 1, 229, 230, 231, 232, 233, 234, 235, 236,
    237, 238, 13, 14, 15, 99, 239, 240, 241, 242,
    243, 244, 245, 246, 247, 248, 27, 28, 249, 250,
    251, 252, 253, 254, 255, 256, 257, 258, 259, 260,
    261, 262, 263, 264, 265, 266, 109, 110, 267, 268,
    269, 270, 271, 272, 273, 274, 275, 276, 277, 278,
    279, 280, 281, 282, 283, 284, 285, 286, 287, 288,
    289, 290, 291, 292, 293, 294, 295, 296, 297, 298,
    299, 300, 301, 302, 303, 304, 305, 306, 307, 308,
    309, 310, 311, 312, 313, 314, 315, 316, 317, 318,
    158, 155, 163, 319, 320, 321, 322, 323, 324, 325,
    326, 150, 164, 169, 327, 328, 329, 330, 331, 332,
    333, 334, 335, 336, 337, 338, 339, 340, 341, 342,
    343, 344, 345, 346, 347, 348, 349, 350, 351, 352,
    353, 354, 355, 356, 357, 358, 359, 360, 361, 362,
    363, 364, 365, 366, 367, 368, 369, 370, 371, 372,
    373, 374, 375, 376, 377, 378
};

static const int expert_subset_charset[] = {
    // Automatically generated from Appendix C of the CFF specification; do
    // not edit.
    0, 1, 231, 232, 235, 236, 237, 238, 13, 14,
    15, 99, 239, 240, 241, 242, 243, 244, 245, 246,
    247, 248, 27, 28, 249, 250, 251, 253, 254, 255,
    256, 257, 258, 259, 260, 261, 262, 263, 264, 265,
    266, 109, 110, 267, 268, 269, 270, 272, 300, 301,
    302, 305, 314, 315, 158, 155, 163, 320, 321, 322,
    323, 324, 325, 326, 150, 164, 169, 327, 328, 329,
    330, 331, 332, 333, 334, 335, 336, 337, 338, 339,
    340, 341, 342, 343, 344, 345, 346
};

static const uint8_t default_dict_cff_data[] = {
    // CFF header
    1, 0,                       // format major and minor version
    4,                          // header size
    4,                          // absolute offset size

    // Name INDEX
    0, 1,                       // one element in index
    1,                          // offset size
    1, 14,                      // offset array
    '%', 'D', 'E', 'F', 'A', 'U', 'L', 'T', 'D', 'I', 'C', 'T', '%',

    // Top DICT INDEX
    0, 1,                       // one element in index
    1,                          // offset size
    1, 92,                      // offset array
    // and the DICT
    139, 12, 1,                 // isFixedPitch false
    139, 12, 2,                 // ItalicAngle 0
    39, 12, 3,                  // UnderlinePosition -100
    189, 12, 4,                 // UnderlineThickness 50
    139, 12, 5,                 // PaintType 0
    141, 12, 6,                 // CharstringType 2
    30, 0x0A, 0x00, 0x1F, 139, 139, 30, 0x0A, 0x00, 0x1F, 139, 139, 12, 7,
                                // FontMatrix 0.001 0 0 0.001 0 0
    139, 139, 139, 139, 5,      // FontBBox 0 0 0 0
    139, 12, 8,                 // StrokeWidth 0
    139, 15,                    // charset 0
    139, 16,                    // Encoding 0
    139, 12, 31,                // CIDFontVersion 0
    139, 12, 32,                // CIDFontRevision 0
    139, 12, 33,                // CIDFontType 0
    28, 34, 16, 12, 34,         // CIDCount 8720
    30, 0x0A, 0x03, 0x96, 0x25, 0xFF, 12, 9,    // BlueScale 0.039625
    146, 12, 10,                // BlueShift 7
    140, 12, 11,                // BlueFuzz 1
    139, 12, 14,                // ForceBold false
    139, 12, 17,                // LanguageGroup 0
    30, 0x0A, 0x06, 0xFF, 12, 18,               // ExpansionFactor 0.06
    139, 12, 19,                // initialRandomSeed 0
    139, 20,                    // defaultWidthX 0
    139, 21,                    // nominalWidthX 0

    // String INDEX
    0, 0,

    // Gsubr INDEX
    0, 0
};

static const Cff::Dict&
default_dict()
{
    static Cff cff(String::make_stable((const char*) default_dict_cff_data, sizeof(default_dict_cff_data)), 0, ErrorHandler::default_handler());
    return static_cast<Cff::Font*>(cff.font())->top_dict();
}


#define POS_GT(pos1, pos2)      ((unsigned)(pos1) > (unsigned)(pos2))


Cff::Cff(const String& s, unsigned units_per_em, ErrorHandler* errh)
    : _data_string(s), _data(reinterpret_cast<const uint8_t *>(_data_string.data())), _len(_data_string.length()),
      _strings_map(-2), _units_per_em(units_per_em)
{
    static_assert((sizeof(standard_strings) / sizeof(standard_strings[0])) == NSTANDARD_STRINGS,
                  "NSTANDARD_STRINGS defined incorrectly");
    static_assert((sizeof(standard_encoding) / sizeof(standard_encoding[0])) == 256,
                  "standard_encoding has wrong size");
    static_assert((sizeof(expert_encoding) / sizeof(expert_encoding[0])) == 256,
                  "expert_encoding has wrong size");
    _error = parse_header(errh ? errh : ErrorHandler::silent_handler());
}

Cff::~Cff()
{
    for (int i = 0; i < _gsubrs_cs.size(); i++)
        delete _gsubrs_cs[i];
    for (int i = 0; i < _fonts.size(); ++i)
        delete _fonts[i];
}

/*
 * Parsing the file header
 */

int
Cff::parse_header(ErrorHandler *errh)
{
    if (_gsubrs_index.error() >= 0) // already done
        return 0;

    // parse header
    if (_len == 0)
        return errh->error("not a PostScript-flavored OpenType font"), -EFAULT;
    if (_len < HEADER_SIZE)
        return errh->error("CFF file corrupted (too small)"), -EFAULT;
    if (_data[0] != 1)          // major version number
        return errh->error("bad major version number %d", _data[0]), -ERANGE;
    int hdrSize = _data[2], offSize = _data[3];
    if (hdrSize < 4 || hdrSize > _len || offSize < 1 || offSize > 4)
        return errh->error("corrupted file header"), -EINVAL;
    int name_index_pos = hdrSize;

    // parse name INDEX
    IndexIterator niter(_data, name_index_pos, _len, errh, "Name INDEX");
    if (niter.error() < 0)
        return niter.error();
    _name_index.clear();
    for (; niter; niter++) {
        const uint8_t *d0 = niter[0];
        const uint8_t *d1 = niter[1];
        if (d0 == d1 || d0[0] == 0)
            _name_index.push_back(PermString());
        else
            _name_index.push_back(PermString(reinterpret_cast<const char *>(d0), d1 - d0));
    }
    int top_dict_index_pos = niter.index_end() - _data;

    // check top DICT INDEX
    _top_dict_index = IndexIterator(_data, top_dict_index_pos, _len, errh, "Top DICT INDEX");
    if (_top_dict_index.error() < 0)
        return _top_dict_index.error();
    else if (_top_dict_index.nitems() != nfonts())
        return errh->error("invalid font: Top DICT INDEX has %d elements, but there are %d fonts", _top_dict_index.nitems(), nfonts()), -EINVAL;
    int string_index_pos = _top_dict_index.index_end() - _data;

    // check strings INDEX
    _strings_index = IndexIterator(_data, string_index_pos, _len, errh, "Strings INDEX");
    if (_strings_index.error() < 0)
        return _strings_index.error();
    else if (NSTANDARD_STRINGS + _strings_index.nitems() - 1 > MAX_SID)
        return errh->error("too many strings defined in font"), -EINVAL;
    _strings.assign(_strings_index.nitems(), PermString());
    int global_subr_index_pos = _strings_index.index_end() - _data;

    // check gsubr INDEX
    _gsubrs_index = IndexIterator(_data, global_subr_index_pos, _len, errh, "Gsubrs INDEX");
    if (_gsubrs_index.error() < 0)
        return _gsubrs_index.error();
    _gsubrs_cs.assign(ngsubrs(), 0);

    return 0;
}

int
Cff::sid(PermString s)
{
    if (!s)                     // XXX?
        return -1;

    // check standard strings
    if (standard_permstrings_map["a"] < 0)
        for (int i = 0; i < NSTANDARD_STRINGS; i++) {
            if (!standard_permstrings[i])
                standard_permstrings[i] = PermString(standard_strings[i]);
            standard_permstrings_map.insert(standard_permstrings[i], i);
        }
    int sid = standard_permstrings_map[s];
    if (sid >= 0)
        return sid;

    // check user strings
    sid = _strings_map[s];
    if (sid >= -1)
        return sid;

    for (int i = 0; i < _strings.size(); i++)
        if (!_strings[i] && s.length() == _strings_index[i+1] - _strings_index[i] && memcmp(s.c_str(), _strings_index[i], s.length()) == 0) {
            _strings[i] = s;
            _strings_map.insert(s, i + NSTANDARD_STRINGS);
            return i + NSTANDARD_STRINGS;
        }

    _strings_map.insert(s, -1);
    return -1;
}

String
Cff::sid_string(int sid) const
{
    if (sid < 0)
        return String();
    else if (sid < NSTANDARD_STRINGS)
        return String(sid_permstring(sid));
    else {
        sid -= NSTANDARD_STRINGS;
        if (sid >= _strings.size())
            return String();
        else if (_strings[sid])
            return String(_strings[sid]);
        else
            return String(reinterpret_cast<const char *>(_strings_index[sid]), _strings_index[sid + 1] - _strings_index[sid]);
    }
}

PermString
Cff::sid_permstring(int sid) const
{
    if (sid < 0)
        return PermString();
    else if (sid < NSTANDARD_STRINGS) {
        if (!standard_permstrings[sid])
            standard_permstrings[sid] = PermString(standard_strings[sid]);
        return standard_permstrings[sid];
    } else {
        sid -= NSTANDARD_STRINGS;
        if (sid >= _strings.size())
            return PermString();
        else if (_strings[sid])
            return _strings[sid];
        else {
            PermString s = PermString(reinterpret_cast<const char *>(_strings_index[sid]), _strings_index[sid + 1] - _strings_index[sid]);
            _strings[sid] = s;
            _strings_map.insert(s, sid + NSTANDARD_STRINGS);
            return s;
        }
    }
}

Cff::FontParent *
Cff::font(PermString font_name, ErrorHandler *errh)
{
    if (!errh)
        errh = ErrorHandler::silent_handler();

    if (!ok())
        return errh->error("invalid CFF"), (FontParent *) 0;

    // search for a font named 'font_name'
    int findex;
    for (findex = 0; findex < _name_index.size(); ++findex) {
        if (_name_index[findex]
            && (!font_name || font_name == _name_index[findex]))
            break;
    }
    if (findex >= _name_index.size()) {
        if (!font_name)
            errh->error("no fonts in CFF");
        else
            errh->error("font %<%s%> not found", font_name.c_str());
        return 0;
    }

    // return font
    for (int i = 0; i < _fonts.size(); ++i)
        if (_fonts[i]->_font_index == findex)
            return _fonts[i];

    int td_offset = _top_dict_index[findex] - _data;
    int td_length = _top_dict_index[findex + 1] - _top_dict_index[findex];
    Dict top_dict(this, td_offset, td_length, errh, "Top DICT");
    if (!top_dict.ok())
        return 0;

    Cff::FontParent* fp;
    if (top_dict.has_first(oROS))
        fp = new Cff::CIDFont(this, _name_index[findex], top_dict, errh);
    else
        fp = new Cff::Font(this, _name_index[findex], top_dict, errh);
    fp->_font_index = findex;
    _fonts.push_back(fp);
    return fp;
}

static inline int
subr_bias(int charstring_type, int nsubrs)
{
    if (charstring_type == 1)
        return 0;
    else if (nsubrs < 1240)
        return 107;
    else if (nsubrs < 33900)
        return 1131;
    else
        return 32768;
}

Charstring *
Cff::gsubr(int i)
{
    i += subr_bias(2, ngsubrs());
    if (i < 0 || i >= ngsubrs())
        return 0;
    if (!_gsubrs_cs[i]) {
        const uint8_t *s1 = _gsubrs_index[i];
        int slen = _gsubrs_index[i + 1] - s1;
        String cs = data_string().substring(s1 - data(), slen);
        if (slen == 0)
            return 0;
        else
            _gsubrs_cs[i] = new Type2Charstring(cs);
    }
    return _gsubrs_cs[i];
}


/*****
 * Cff::Charset
 **/

Cff::Charset::Charset(const Cff *cff, int pos, int nglyphs, int max_sid, ErrorHandler *errh)
{
    assign(cff, pos, nglyphs, max_sid, errh);
}

void
Cff::Charset::assign(const Cff *cff, int pos, int nglyphs, int max_sid, ErrorHandler *errh)
{
    if (!errh)
        errh = ErrorHandler::silent_handler();

    _sids.reserve(nglyphs);

    if (pos == 0)
        assign(iso_adobe_charset, sizeof(iso_adobe_charset) / sizeof(int), nglyphs);
    else if (pos == 1)
        assign(expert_charset, sizeof(expert_charset) / sizeof(int), nglyphs);
    else if (pos == 2)
        assign(expert_subset_charset, sizeof(expert_subset_charset) / sizeof(int), nglyphs);
    else
        _error = parse(cff, pos, nglyphs, max_sid, errh);

    if (_error >= 0)
        for (int g = 0; g < _sids.size(); g++) {
            if (_gids[_sids[g]] >= 0) {
                errh->error("glyph %<%s%> in charset twice", cff->sid_permstring(_sids[g]).c_str());
                _error = -EEXIST;
            }
            _gids[_sids[g]] = g;
        }
}

void
Cff::Charset::assign(const int *data, int size, int nglyphs)
{
    if (size < nglyphs)
        size = nglyphs;
    _sids.resize(size);
    memcpy(&_sids[0], data, sizeof(const int) * size);
    _gids.resize(data[size-1] + 1, -1);
    _error = 0;
}

int
Cff::Charset::parse(const Cff *cff, int pos, int nglyphs, int max_sid, ErrorHandler *errh)
{
    const uint8_t *data = cff->data();
    int len = cff->length();

    if (pos + 1 > len)
        return errh->error("charset position out of range"), -EFAULT;

    _sids.push_back(0);
    int actual_max_sid = 0;

    int format = data[pos];
    if (format == 0) {
        if (pos + 1 + (nglyphs - 1) * 2 > len)
            return errh->error("charset [format 0] out of range"), -EFAULT;
        const uint8_t *p = data + pos + 1;
        for (; _sids.size() < nglyphs; p += 2) {
            int sid = (p[0] << 8) | p[1];
            if (sid > actual_max_sid)
                actual_max_sid = sid;
            _sids.push_back(sid);
        }

    } else if (format == 1) {
        const uint8_t *p = data + pos + 1;
        for (; _sids.size() < nglyphs; p += 3) {
            if (p + 3 > data + len)
                return errh->error("charset [format 1] out of range"), -EFAULT;
            int sid = (p[0] << 8) | p[1];
            int n = p[2];
            if (sid + n > actual_max_sid)
                actual_max_sid = sid + n;
            for (int i = 0; i <= n; i++)
                _sids.push_back(sid + i);
        }

    } else if (format == 2) {
        const uint8_t *p = data + pos + 1;
        for (; _sids.size() < nglyphs; p += 4) {
            if (p + 4 > data + len)
                return errh->error("charset [format 2] out of range"), -EFAULT;
            int sid = (p[0] << 8) | p[1];
            int n = (p[2] << 8) | p[3];
            if (sid + n > actual_max_sid)
                actual_max_sid = sid + n;
            for (int i = 0; i <= n; i++)
                _sids.push_back(sid + i);
        }

    } else
        return errh->error("unknown charset format %d", format), -EINVAL;

    if (max_sid >= 0 && actual_max_sid > max_sid)
        return errh->error("charset [format %d] uses bad SID %d", format, actual_max_sid), -EINVAL;
    _sids.resize(nglyphs);
    _gids.resize(actual_max_sid + 1, -1);
    return 0;
}


/*****
 * Cff::FDSelect
 **/

void
Cff::FDSelect::assign(const Cff *cff, int pos, int nglyphs, ErrorHandler *errh)
{
    if (!errh)
        errh = ErrorHandler::silent_handler();
    if (_my_fds)
        delete[] _fds;
    _fds = 0;
    _my_fds = false;
    _nglyphs = nglyphs;
    _error = parse(cff, pos, nglyphs, errh);
}

Cff::FDSelect::~FDSelect()
{
    if (_my_fds)
        delete[] _fds;
}

int
Cff::FDSelect::parse(const Cff *cff, int pos, int nglyphs, ErrorHandler *errh)
{
    const uint8_t *data = cff->data();
    int len = cff->length();

    if (pos + 1 > len)
        return errh->error("FDSelect position out of range"), -EFAULT;

    int format = data[pos];
    if (format == 0) {
        if (pos + 1 + nglyphs > len)
            return errh->error("FDSelect [format 0] out of range"), -EFAULT;
        _fds = data + pos + 1;
        _my_fds = false;
        return 0;

    } else if (format == 3) {
        int nranges = (data[pos+1] << 8) | data[pos+2];
        if (pos + 3 + 3*nranges + 2 > len)
            return errh->error("FDSelect [format 3] out of range"), -EFAULT;

        const uint8_t *p = data + pos + 3;
        int last_glyph = (p[3*nranges] << 8) | p[3*nranges + 1];
        if (p[0] || p[1] || last_glyph != nglyphs)
            return errh->error("FDSelect [format 3] bad values"), -EINVAL;

        _fds = new uint8_t[nglyphs];
        _my_fds = true;
        int curglyph = 0;
        for (; curglyph < nglyphs; p += 3) {
            int nextglyph = (p[3] << 8) | p[4];
            if (nextglyph > nglyphs || nextglyph < curglyph)
                return errh->error("FDSelect [format 3] sorting error"), -EINVAL;
            memset(const_cast<uint8_t *>(_fds + curglyph), p[2], nextglyph - curglyph);
            curglyph = nextglyph;
        }
        return 0;

    } else
        return errh->error("unknown charset format %d", format), -EINVAL;
}


/*****
 * Cff::IndexIterator
 **/

Cff::IndexIterator::IndexIterator(const uint8_t *data, int pos, int len, ErrorHandler *errh, const char *index_name)
    : _contents(0), _offset(0), _last_offset(0)
{
    if (!errh)
        errh = ErrorHandler::silent_handler();

    // check header
    int nitems = 0;
    if (POS_GT(pos + 2, len)) {
        errh->error("%s: position out of range", index_name);
        _offsize = -EFAULT;
    } else if (data[pos] == 0 && data[pos + 1] == 0) {
        _contents = data + pos + 2;
        _offsize = 0;
    } else if (POS_GT(pos + 3, len)) {
        errh->error("%s: position out of range", index_name);
        _offsize = -EFAULT;
    } else if ((_offsize = data[pos + 2]), (_offsize < 1 || _offsize > 4)) {
        errh->error("%s: offset size %d out of range", index_name, _offsize);
        _offsize = -EINVAL;
    } else {
        nitems = (data[pos] << 8) | data[pos + 1];
        if (POS_GT(pos + 3 + (nitems + 1) * _offsize, len)) {
            errh->error("%s: data out of range", index_name);
            _offsize = -EFAULT;
        } else {
            _offset = data + pos + 3;
            _last_offset = _offset + nitems * _offsize;
            _contents = _last_offset + _offsize - 1;
        }
    }

    // check items in offset array
    uint32_t max_doff_allowed = len - (pos + 2 + (nitems + 1) * _offsize);
    uint32_t last_doff = 1;
    for (const uint8_t *o = _offset; o <= _last_offset && _offsize > 0; o += _offsize) {
        uint32_t doff = offset_at(o);
        if (doff > max_doff_allowed) {
            errh->error("%s: element out of range", index_name);
            _offsize = -EFAULT;
        } else if (doff < last_doff) {
            errh->error("%s: garbled elements", index_name);
            break;
        }
        last_doff = doff;
    }
}

const uint8_t *
Cff::IndexIterator::index_end() const
{
    if (_offsize <= 0)
        return _contents;
    else
        return _contents + offset_at(_last_offset);
}

int
Cff::IndexIterator::nitems() const
{
    if (_offsize <= 0)
        return 0;
    else
        return (_last_offset - _offset) / _offsize;
}



/*****
 * Cff::Dict
 **/

Cff::Dict::Dict()
    : _cff(0), _pos(0), _error(-ENOENT)
{
}

Cff::Dict::Dict(Cff *cff, int pos, int dict_len, ErrorHandler *errh, const char *dict_name)
{
    assign(cff, pos, dict_len, errh, dict_name);
}

int
Cff::Dict::assign(Cff *cff, int pos, int dict_len, ErrorHandler *errh, const char *dict_name)
{
    _cff = cff;
    _pos = pos;
    _operators.clear();
    _pointers.clear();
    _operands.clear();

    if (!errh)
        errh = ErrorHandler::silent_handler();

    const uint8_t *data = cff->data() + pos;
    const uint8_t *end_data = data + dict_len;

    _pointers.push_back(0);
    while (data < end_data)
        switch (data[0]) {

          case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
          case 8: case 9: case 10: case 11: case 13: case 14: case 15:
          case 16: case 17: case 18: case 19: case 20: case 21:
            _operators.push_back(data[0]);
            _pointers.push_back(_operands.size());
            data++;
            break;

          case 22: case 23: case 24: case 25: case 26: case 27: case 31:
          case 255:             // reserved
            errh->error("%s: reserved operator %d", dict_name, data[0]);
            return (_error = -ERANGE);

          case 12:
            if (data + 1 >= end_data)
                goto runoff;
            _operators.push_back(32 + data[1]);
            _pointers.push_back(_operands.size());
            data += 2;
            break;

          case 28: {
              if (data + 2 >= end_data)
                  goto runoff;
              int16_t val = (data[1] << 8) | data[2];
              _operands.push_back(val);
              data += 3;
              break;
          }

          case 29: {
              if (data + 4 >= end_data)
                  goto runoff;
              int32_t val = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];
              _operands.push_back(val);
              data += 5;
              break;
          }

          case 30: {
              char buf[1024];
              int pos = 0;
              if (data + 1 >= end_data)
                  goto runoff;
              for (data++; data < end_data && pos < 1020; data++) {
                  int d = *data;
                  for (int i = 0; i < 2; i++, d <<= 4) {
                      int digit = (d >> 4) & 0xF;
                      switch (digit) {
                        case 10:
                          buf[pos++] = '.';
                          break;
                        case 11:
                          buf[pos++] = 'E';
                          break;
                        case 12:
                          buf[pos++] = 'E';
                          buf[pos++] = '-';
                          break;
                        case 13:
                          errh->error("%s: bad digit in real number", dict_name);
                          goto invalid;
                        case 14:
                          buf[pos++] = '-';
                          break;
                        case 15:
                          goto found;
                        default:
                          buf[pos++] = digit + '0';
                          break;
                      }
                  }
              }
              // number not found
              goto runoff;
            found:
              char *endptr;
              buf[pos] = '\0';
              _operands.push_back(strtod(buf, &endptr));
              if (*endptr) {
                  errh->error("%s: real number syntax error", dict_name);
                  goto invalid;
              }
              data++;
              break;
          }

          case 247: case 248: case 249: case 250: {
              if (data + 1 >= end_data)
                  goto runoff;
              int val = ((data[0] - 247) << 8) + data[1] + 108;
              _operands.push_back(val);
              data += 2;
              break;
          }

          case 251: case 252: case 253: case 254: {
              if (data + 1 >= end_data)
                  goto runoff;
              int val = -((data[0] - 251) << 8) - data[1] - 108;
              _operands.push_back(val);
              data += 2;
              break;
          }

          default:
            _operands.push_back(data[0] - 139);
            data++;
            break;

        }

    // not closed by an operator?
    if (_pointers.back() != _operands.size()) {
        errh->error("%s: not closed by an operator", dict_name);
        goto invalid;
    }

    return (_error = 0);

  runoff:
    errh->error("%s: runoff end of DICT", dict_name);
    return (_error = -EFAULT);

  invalid:
    return (_error = -EINVAL);
}

int
Cff::Dict::check(bool is_private, ErrorHandler *errh, const char *dict_name) const
{
    if (!errh)
        errh = ErrorHandler::silent_handler();
    int before_nerrors = errh->nerrors();

    // keep track of operator reuse
    Vector<int> operators_used;

    for (int i = 0; i < _operators.size(); i++) {
        int arity = _pointers[i+1] - _pointers[i];
        double num = (arity == 0 ? 0 : _operands[_pointers[i]]);
        double truncnum = floor(num);
        int op = _operators[i];
        int type = (op > oLastOperator ? tNone : operator_types[op]);

        // check reuse
        if (op >= operators_used.size())
            operators_used.resize(op + 1, 0);
        if (operators_used[op] && (type & tTypeMask) != tNone)
            errh->error("%s: operator %<%s%> specified twice", dict_name, operator_names[op]);
        operators_used[op]++;

        // check data
        switch (type & tTypeMask) {

          case tNone:
            if (op >= 32)
                errh->warning("%s: unknown operator %<12 %d%>", dict_name, op - 32);
            else
                errh->warning("%s: unknown operator %<%d%>", dict_name, op);
            continue;

          case tSID:
            if (arity != 1 || num != truncnum || num < 0 || num > _cff->max_sid())
                goto bad_data;
            break;

          case tFontNumber:
            if (arity != 1 || num != truncnum || num < 0 || num >= _cff->nfonts())
                goto bad_data;
            break;

          case tBoolean:
            if (arity != 1)
                goto bad_data;
            else if (num != 0 && num != 1)
                errh->warning("%s: data for Boolean operator %<%s%> not 0 or 1", dict_name, operator_names[op]);
            break;

          case tNumber:
            if (arity != 1)
                goto bad_data;
            break;

          case tOffset:
            if (arity != 1 || num != truncnum || num < 0 || num >= _cff->length())
                goto bad_data;
            break;

          case tLocalOffset:
            if (arity != 1 || num != truncnum || _pos + num < 0 || _pos + num >= _cff->length())
                goto bad_data;
            break;

          case tPrivateType: {
              if (arity != 2 || num != truncnum || num < 0)
                  goto bad_data;
              double off = _operands[_pointers[i] + 1];
              if (off < 0 || off + num > _cff->length())
                  goto bad_data;
              break;
          }

          case tArray2: case tArray3: case tArray4:
          case tArray5: case tArray6:
            if (arity != (type & tTypeMask) - tArray2 + 2)
                goto bad_data;
            break;

          case tArray:
            break;

        }

        // check dict location
        if (((type & tPrivate) != 0) != is_private)
            errh->warning("%s: operator %<%s%> in wrong DICT", dict_name, operator_names[op]);

        continue;

      bad_data:
        errh->error("%s: bad data for operator %<%s%>", dict_name, operator_names[op]);
    }

    return (errh->nerrors() != before_nerrors ? -1 : 0);
}

bool
Cff::Dict::has(DictOperator op) const
{
    for (int i = 0; i < _operators.size(); i++)
        if (_operators[i] == op)
            return true;
    return false;
}

bool
Cff::Dict::xvalue(DictOperator op, Vector<double> &out) const
{
    out.clear();
    for (int i = 0; i < _operators.size(); i++)
        if (_operators[i] == op) {
            for (int j = _pointers[i]; j < _pointers[i+1]; j++)
                out.push_back(_operands[j]);
            return true;
        }
    return false;
}

bool
Cff::Dict::xvalue(DictOperator op, int *val) const
{
    for (int i = 0; i < _operators.size(); i++)
        if (_operators[i] == op && _pointers[i] + 1 == _pointers[i+1]) {
            *val = (int) _operands[_pointers[i]];
            return true;
        }
    return false;
}

bool
Cff::Dict::xvalue(DictOperator op, double *val) const
{
    for (int i = 0; i < _operators.size(); i++)
        if (_operators[i] == op && _pointers[i] + 1 == _pointers[i+1]) {
            *val = _operands[_pointers[i]];
            return true;
        }
    return false;
}

bool
Cff::Dict::value(DictOperator op, Vector<double> &out) const
{
    return xvalue(op, out) || default_dict().xvalue(op, out);
}

bool
Cff::Dict::value(DictOperator op, int *val) const
{
    return xvalue(op, val) || default_dict().xvalue(op, val);
}

bool
Cff::Dict::value(DictOperator op, double *val) const
{
    return xvalue(op, val) || default_dict().xvalue(op, val);
}

void
Cff::Dict::unparse(ErrorHandler *errh, const char *dict_name) const
{
    StringAccum sa;
    for (int i = 0; i < _operators.size(); i++) {
        sa.clear();
        if (_pointers[i] + 1 == _pointers[i+1])
            sa << _operands[_pointers[i]];
        else {
            sa << "[";
            for (int j = _pointers[i]; j < _pointers[i+1]; j++)
                sa << _operands[j] << ' ';
            sa.pop_back();
            sa << "]";
        }
        errh->message("%s: %s %s", dict_name, operator_names[_operators[i]], sa.c_str());
    }
}


/*****
 * CffFontParent
 **/

static int
handle_private(Cff *cff, const Cff::Dict &top_dict, Cff::Dict &private_dict,
               double &default_width_x, double &nominal_width_x,
               Cff::IndexIterator &subrs_index, Vector<Charstring *> &subrs_cs,
               ErrorHandler *errh)
{
    Vector<double> private_info;
    top_dict.value(Cff::oPrivate, private_info);
    int private_offset = (int) private_info[1];
    private_dict.assign(cff, private_offset, (int) private_info[0], errh, "Private DICT");
    if (private_dict.error() < 0)
        return private_dict.error();
    else if (private_dict.check(true, errh, "Private DICT") < 0)
        return -EINVAL;
    //private_dict.unparse(errh, "Private DICT");

    private_dict.value(Cff::oDefaultWidthX, &default_width_x);
    private_dict.value(Cff::oNominalWidthX, &nominal_width_x);
    if (private_dict.has(Cff::oSubrs)) {
        int subrs_offset = 0;
        private_dict.value(Cff::oSubrs, &subrs_offset);
        subrs_index = Cff::IndexIterator(cff->data(), private_offset + subrs_offset, cff->length(), errh, "Subrs INDEX");
        if (subrs_index.error() < 0)
            return subrs_index.error();
    }
    subrs_cs.assign(subrs_index.nitems(), 0);
    return 0;
}


Cff::FontParent::FontParent(Cff* cff)
    : CharstringProgram(cff->units_per_em()), _cff(cff), _error(-1)
{
}

Charstring *
Cff::FontParent::charstring(const IndexIterator &iiter, int which) const
{
    const uint8_t *s1 = iiter[which];
    int slen = iiter[which + 1] - s1;
    String cs = _cff->data_string().substring(s1 - _cff->data(), slen);
    if (slen == 0)
        return 0;
    else if (_charstring_type == 1)
        return new Type1Charstring(cs);
    else
        return new Type2Charstring(cs);
}

Charstring *
Cff::FontParent::gsubr(int i) const
{
    return _cff->gsubr(i);
}

int
Cff::FontParent::gsubr_bias() const
{
    return Efont::subr_bias(2, ngsubrs_x());
}


/*****
 * CffFont
 **/

Cff::Font::Font(Cff *cff, PermString font_name, const Dict &top_dict, ErrorHandler *errh)
    : ChildFont(cff, 0, 2, top_dict, errh), _font_name(font_name),
      _t1encoding(0)
{
    assert(!_top_dict.has_first(oROS));
    if (_error < 0)
        return;

    // extract CharStrings
    // must use xvalue because we could be creating the default dict!
    int charstrings_offset = 0;
    _top_dict.xvalue(oCharStrings, &charstrings_offset);
    _charstrings_index = Cff::IndexIterator(cff->data(), charstrings_offset, cff->length(), errh, "CharStrings INDEX");
    if (_charstrings_index.error() < 0) {
        _error = _charstrings_index.error();
        return;
    }
    _charstrings_cs.assign(_charstrings_index.nitems(), 0);

    int charset = 0;
    _top_dict.xvalue(oCharset, &charset);
    _charset.assign(cff, charset, _charstrings_index.nitems(), cff->max_sid(), errh);
    if (_charset.error() < 0) {
        _error = _charset.error();
        return;
    }

    int Encoding = 0;
    _top_dict.xvalue(oEncoding, &Encoding);
    if (parse_encoding(Encoding, errh) < 0)
        return;

    // success!
    _error = 0;
}

Cff::Font::~Font()
{
    for (int i = 0; i < _charstrings_cs.size(); i++)
        delete _charstrings_cs[i];
    delete _t1encoding;
}

int
Cff::Font::parse_encoding(int pos, ErrorHandler *errh)
{
    _encoding_pos = pos;
    for (int i = 0; i < 256; i++)
        _encoding[i] = 0;

    // check for standard encodings
    if (pos == 0)
        return assign_standard_encoding(standard_encoding);
    else if (pos == 1)
        return assign_standard_encoding(expert_encoding);

    // otherwise, a custom encoding
    const uint8_t *data = _cff->data();
    int len = _cff->length();
    if (pos + 1 > len)
        return errh->error("Encoding position out of range"), -EFAULT;
    bool supplemented = (data[pos] & 0x80) != 0;
    int format = (data[pos] & 0x7F);

    int retval = 0;
    int endpos, g = 1;
    if (format == 0) {
        endpos = pos + 2 + data[pos + 1];
        if (endpos > len)
            return errh->error("Encoding[0] out of range"), -EFAULT;
        const uint8_t *p = data + pos + 2;
        int n = data[pos + 1];
        for (; g <= n; g++, p++) {
            int e = p[0];
            if (_encoding[e])
                retval = 1;
            _encoding[e] = g;
        }

    } else if (format == 1) {
        endpos = pos + 2 + data[pos + 1] * 2;
        if (endpos > len)
            return errh->error("Encoding[1] out of range"), -EFAULT;
        const uint8_t *p = data + pos + 2;
        int n = data[pos + 1];
        for (int i = 0; i < n; i++, p += 2) {
            int first = p[0];
            int nLeft = p[1];
            for (int e = first; e <= first + nLeft; e++) {
                if (_encoding[e])
                    retval = 1;
                _encoding[e] = g++;
            }
        }

    } else
        return errh->error("unknown Encoding format %d", format), -EINVAL;

    if (g > _charset.nglyphs())
        return errh->error("Encoding glyph %d out of range", g), -EINVAL;

    // check supplements
    if (supplemented) {
        if (endpos + data[endpos] * 3 > len)
            return -EINVAL;
        const uint8_t *p = data + endpos + 1;
        int n = data[endpos];
        for (int i = 0; i < n; i++, p += 3) {
            int e = p[0];
            int s = (p[1] << 8) | p[2];
            int g = _charset.sid_to_gid(s);
            if (_encoding[e])
                retval = 1;
            if (g < 0 || g >= _charset.nglyphs())
                return errh->error("Encoding glyph %d out of range", g), -EINVAL;
            _encoding[e] = g;
        }
    }

    // successfully done
    return retval;
}

int
Cff::Font::assign_standard_encoding(const int *standard_encoding)
{
    for (int i = 0; i < 256; i++)
        _encoding[i] = _charset.sid_to_gid(standard_encoding[i]);
    return 0;
}

void
Cff::Font::font_matrix(double matrix[6]) const
{
    Vector<double> t1d_matrix;
    if (dict_value(oFontMatrix, t1d_matrix) && t1d_matrix.size() == 6)
        memcpy(&matrix[0], &t1d_matrix[0], sizeof(double) * 6);
    else {
        matrix[0] = matrix[3] = 0.001;
        matrix[1] = matrix[2] = matrix[4] = matrix[5] = 0;
    }
}

PermString
Cff::Font::glyph_name(int gid) const
{
    if (gid >= 0 && gid < nglyphs())
        return _cff->sid_permstring(_charset.gid_to_sid(gid));
    else
        return PermString();
}

void
Cff::Font::glyph_names(Vector<PermString> &gnames) const
{
    gnames.resize(nglyphs());
    for (int i = 0; i < nglyphs(); i++)
        gnames[i] = _cff->sid_permstring(_charset.gid_to_sid(i));
}

Charstring *
Cff::Font::glyph(int gid) const
{
    if (gid < 0 || gid >= nglyphs())
        return 0;
    if (!_charstrings_cs[gid])
        _charstrings_cs[gid] = charstring(_charstrings_index, gid);
    return _charstrings_cs[gid];
}

Charstring *
Cff::Font::glyph(PermString name) const
{
    int gid = _charset.sid_to_gid(_cff->sid(name));
    if (gid < 0)
        return 0;
    if (!_charstrings_cs[gid])
        _charstrings_cs[gid] = charstring(_charstrings_index, gid);
    return _charstrings_cs[gid];
}

int
Cff::Font::glyphid(PermString name) const
{
    return _charset.sid_to_gid(_cff->sid(name));
}

Type1Encoding *
Cff::Font::type1_encoding() const
{
    if (_encoding_pos == 0)
        return Type1Encoding::standard_encoding();
    if (!_t1encoding)
        _t1encoding = type1_encoding_copy();
    return _t1encoding;
}

Type1Encoding *
Cff::Font::type1_encoding_copy() const
{
    if (_encoding_pos == 0)
        return Type1Encoding::standard_encoding();
    Type1Encoding *e = new Type1Encoding;
    for (int i = 0; i < 256; i++)
        if (_encoding[i])
            e->put(i, _cff->sid_permstring(_charset.gid_to_sid(_encoding[i])));
    return e;
}

bool
Cff::Font::dict_has(DictOperator op) const
{
    return dict_of(op).has(op);
}

String
Cff::Font::dict_string(DictOperator op) const
{
    Vector<double> vec;
    dict_of(op).value(op, vec);
    if (vec.size() == 1 && vec[0] >= 0 && vec[0] <= _cff->max_sid())
        return _cff->sid_string((int) vec[0]);
    else
        return String();
}


/*****
 * Cff::CIDFont
 **/

Cff::CIDFont::CIDFont(Cff *cff, PermString font_name, const Dict &top_dict, ErrorHandler *errh)
    : FontParent(cff), _font_name(font_name), _top_dict(top_dict)
{
    assert(_top_dict.has_first(oROS));

    // parse top DICT
    _error = -EINVAL;
    if (_top_dict.check(false, errh, "Top DICT") < 0)
        return;
    else if (!_top_dict.has(oCharStrings)) {
        errh->error("font has no CharStrings dictionary");
        return;
    }
    //_top_dict.unparse(errh, "Top DICT");

    // extract offsets and information from TOP DICT
    _top_dict.value(oCharstringType, &_charstring_type);
    if (_charstring_type != 1 && _charstring_type != 2) {
        errh->error("unknown CharString type %d", _charstring_type);
        return;
    }

    int charstrings_offset = 0;
    _top_dict.value(oCharStrings, &charstrings_offset);
    _charstrings_index = Cff::IndexIterator(cff->data(), charstrings_offset, cff->length(), errh, "CharStrings INDEX");
    if (_charstrings_index.error() < 0) {
        _error = _charstrings_index.error();
        return;
    }
    _charstrings_cs.assign(_charstrings_index.nitems(), 0);

    int charset = 0;
    _top_dict.value(oCharset, &charset);
    _charset.assign(cff, charset, _charstrings_index.nitems(), -1, errh);
    if (_charset.error() < 0) {
        _error = _charset.error();
        return;
    }

    // extract information about child fonts
    int fdarray_offset = 0;
    if (!_top_dict.value(oFDArray, &fdarray_offset)) {
        errh->error("CID-keyed font missing FDArray");
        return;
    }
    IndexIterator fdarray_index(cff->data(), fdarray_offset, cff->length(), errh, "FDArray INDEX");
    for (; fdarray_index; fdarray_index++) {
        Dict d(cff, fdarray_index[0] - cff->data(), fdarray_index[1] - fdarray_index[0], errh, "Top DICT");
        if (!d.ok() || d.check(false, errh, "Top DICT") < 0) {
            _error = d.error();
            return;
        }
        _child_fonts.push_back(new ChildFont(cff, this, _charstring_type, d, errh));
        if (!_child_fonts.back()->ok())
            return;
    }

    int fdselect_offset = 0;
    if (!_top_dict.value(oFDSelect, &fdselect_offset)) {
        errh->error("CID-keyed font missing FDSelect");
        return;
    }
    _fdselect.assign(cff, fdselect_offset, _charstrings_cs.size(), errh);
    if (_fdselect.error() < 0)
        return;

    // success!
    _error = 0;
    set_parent_program(true);
}

Cff::CIDFont::~CIDFont()
{
    for (int i = 0; i < _charstrings_cs.size(); i++)
        delete _charstrings_cs[i];
    for (int i = 0; i < _child_fonts.size(); i++)
        delete _child_fonts[i];
}

void
Cff::CIDFont::font_matrix(double matrix[6]) const
{
    // XXX
    matrix[0] = matrix[3] = 0.001;
    matrix[1] = matrix[2] = matrix[4] = matrix[5] = 0;
}

const CharstringProgram *
Cff::CIDFont::child_program(int gid) const
{
    int fd = _fdselect.gid_to_fd(gid);
    if (fd >= 0 && fd < _child_fonts.size())
        return _child_fonts.at_u(fd);
    else
        return 0;
}

PermString
Cff::CIDFont::glyph_name(int gid) const
{
    if (gid >= 0 && gid < nglyphs())
        return permprintf("#%d", _charset.gid_to_sid(gid));
    else
        return PermString();
}

void
Cff::CIDFont::glyph_names(Vector<PermString> &gnames) const
{
    gnames.resize(nglyphs());
    for (int i = 0; i < nglyphs(); i++)
        gnames[i] = permprintf("#%d", _charset.gid_to_sid(i));
}

Charstring *
Cff::CIDFont::glyph(int gid) const
{
    if (gid < 0 || gid >= nglyphs())
        return 0;
    if (!_charstrings_cs[gid])
        _charstrings_cs[gid] = charstring(_charstrings_index, gid);
    return _charstrings_cs[gid];
}

int
Cff::CIDFont::glyphid(PermString name) const
{
    if (name.length() <= 1 || name[0] != '#' || !isdigit((unsigned char) name[1]))
        return -1;
    char *endptr;
    long cid = strtol(name.c_str() + 1, &endptr, 10);
    if (*endptr != 0)
        return -1;
    return _charset.sid_to_gid(cid);
}

Charstring *
Cff::CIDFont::glyph(PermString name) const
{
    return CIDFont::glyph(CIDFont::glyphid(name));
}


/*****
 * ChildFont
 **/

Cff::ChildFont::ChildFont(Cff *cff, Cff::CIDFont *parent, int charstring_type, const Dict &top_dict, ErrorHandler *errh)
    : FontParent(cff), _parent(parent), _top_dict(top_dict)
{
    if (!errh)
        errh = ErrorHandler::silent_handler();

    if (!cff->ok() || !_top_dict.ok()) {
        errh->error("invalid CFF");
        _error = -EINVAL;
        return;
    }

    // extract offsets and information from TOP DICT
    _charstring_type = charstring_type;
    _top_dict.value(oCharstringType, &_charstring_type);
    if (_charstring_type != 1 && _charstring_type != 2) {
        errh->error("unknown CharString type %d", _charstring_type);
        return;
    }

    // extract information from Private DICT
    if (_top_dict.has(oPrivate)
        && (_error = handle_private(cff, _top_dict, _private_dict, _default_width_x, _nominal_width_x, _subrs_index, _subrs_cs, errh)) < 0)
        return;

    // success!
    _error = 0;
}

Cff::ChildFont::~ChildFont()
{
    for (int i = 0; i < _subrs_cs.size(); i++)
        delete _subrs_cs[i];
}

Charstring *
Cff::ChildFont::charstring(const IndexIterator &iiter, int which) const
{
    const uint8_t *s1 = iiter[which];
    int slen = iiter[which + 1] - s1;
    String cs = _cff->data_string().substring(s1 - _cff->data(), slen);
    if (slen == 0)
        return 0;
    else if (_charstring_type == 1)
        return new Type1Charstring(cs);
    else
        return new Type2Charstring(cs);
}

Charstring *
Cff::ChildFont::subr(int i) const
{
    i += Efont::subr_bias(_charstring_type, nsubrs_x());
    if (i < 0 || i >= nsubrs_x())
        return 0;
    if (!_subrs_cs[i])
        _subrs_cs[i] = charstring(_subrs_index, i);
    return _subrs_cs[i];
}

int
Cff::ChildFont::subr_bias() const
{
    return Efont::subr_bias(_charstring_type, nsubrs_x());
}

double
Cff::ChildFont::global_width_x(bool is_nominal) const
{
    return (is_nominal ? _nominal_width_x : _default_width_x);
}

}
