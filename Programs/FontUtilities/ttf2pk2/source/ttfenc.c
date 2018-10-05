/*
 *   ttfenc.c
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "newobj.h"
#include "ttf2tfm.h"
#include "ttfenc.h"
#include "errormsg.h"

EncodingScheme current_encoding_scheme;

struct encoding_table
{
  long code;
  const char *adobename;
};

struct encoding_table unicode_table[] =
{
  {-1,     ".notdef"},

  {0x0020, "space"},
  {0x0021, "exclam"},
  {0x0022, "quotedbl"},
  {0x0023, "numbersign"},
  {0x0024, "dollar"},
  {0x0025, "percent"},
  {0x0026, "ampersand"},
  {0x0027, "quotesingle"},
  {0x0028, "parenleft"},
  {0x0029, "parenright"},
  {0x002a, "asterisk"},
  {0x002b, "plus"},
  {0x002c, "comma"},
  {0x002d, "hyphen"},           /* cf "minus" */
  {0x002e, "period"},
  {0x002f, "slash"},

  {0x0030, "zero"},
  {0x0031, "one"},
  {0x0032, "two"},
  {0x0033, "three"},
  {0x0034, "four"},
  {0x0035, "five"},
  {0x0036, "six"},
  {0x0037, "seven"},
  {0x0038, "eight"},
  {0x0039, "nine"},
  {0x003a, "colon"},
  {0x003b, "semicolon"},
  {0x003c, "less"},
  {0x003d, "equal"},
  {0x003e, "greater"},
  {0x003f, "question"},

  {0x0040, "at"},
  {0x0041, "A"},
  {0x0042, "B"},
  {0x0043, "C"},
  {0x0044, "D"},
  {0x0045, "E"},
  {0x0046, "F"},
  {0x0047, "G"},
  {0x0048, "H"},
  {0x0049, "I"},
  {0x004a, "J"},
  {0x004b, "K"},
  {0x004c, "L"},
  {0x004d, "M"},
  {0x004e, "N"},
  {0x004f, "O"},

  {0x0050, "P"},
  {0x0051, "Q"},
  {0x0052, "R"},
  {0x0053, "S"},
  {0x0054, "T"},
  {0x0055, "U"},
  {0x0056, "V"},
  {0x0057, "W"},
  {0x0058, "X"},
  {0x0059, "Y"},
  {0x005a, "Z"},
  {0x005b, "bracketleft"},
  {0x005c, "backslash"},
  {0x005d, "bracketright"},
  {0x005e, "asciicircum"},
  {0x005f, "underscore"},

  {0x0060, "grave"},
  {0x0061, "a"},
  {0x0062, "b"},
  {0x0063, "c"},
  {0x0064, "d"},
  {0x0065, "e"},
  {0x0066, "f"},
  {0x0067, "g"},
  {0x0068, "h"},
  {0x0069, "i"},
  {0x006a, "j"},
  {0x006b, "k"},
  {0x006c, "l"},
  {0x006d, "m"},
  {0x006e, "n"},
  {0x006f, "o"},

  {0x0070, "p"},
  {0x0071, "q"},
  {0x0072, "r"},
  {0x0073, "s"},
  {0x0074, "t"},
  {0x0075, "u"},
  {0x0076, "v"},
  {0x0077, "w"},
  {0x0078, "x"},
  {0x0079, "y"},
  {0x007a, "z"},
  {0x007b, "braceleft"},
  {0x007c, "bar"},
  {0x007d, "braceright"},
  {0x007e, "asciitilde"},

  {0x00a0, "nbspace"},          /* cf "space" */
  {0x00a1, "exclamdown"},
  {0x00a2, "cent"},
  {0x00a3, "sterling"},
  {0x00a4, "currency"},
  {0x00a5, "yen"},
  {0x00a6, "brokenbar"},
  {0x00a7, "section"},
  {0x00a8, "dieresis"},
  {0x00a9, "copyright"},        /* "copyrightserif" "copyrightsans" */
  {0x00aa, "ordfeminine"},
  {0x00ab, "guillemotleft"},
  {0x00ac, "logicalnot"},
  {0x00ad, "sfthyphen"},        /* cf "hyphen" */
  {0x00ae, "registered"},       /* "registeredserif" "registeredsans" */
  {0x00af, "overscore"},

  {0x00b0, "degree"},
  {0x00b1, "plusminus"},
  {0x00b2, "twosuperior"},
  {0x00b3, "threesuperior"},
  {0x00b4, "acute"},
  {0x00b5, "mu1"},              /* "micro" */
  {0x00b6, "paragraph"},
  {0x00b7, "middot"},           /* cf "periodcentered" */
  {0x00b8, "cedilla"},
  {0x00b9, "onesuperior"},
  {0x00ba, "ordmasculine"},
  {0x00bb, "guillemotright"},
  {0x00bc, "onequarter"},
  {0x00bd, "onehalf"},
  {0x00be, "threequarters"},
  {0x00bf, "questiondown"},

  {0x00c0, "Agrave"},
  {0x00c1, "Aacute"},
  {0x00c2, "Acircumflex"},
  {0x00c3, "Atilde"},
  {0x00c4, "Adieresis"},
  {0x00c5, "Aring"},
  {0x00c6, "AE"},
  {0x00c7, "Ccedilla"},
  {0x00c8, "Egrave"},
  {0x00c9, "Eacute"},
  {0x00ca, "Ecircumflex"},
  {0x00cb, "Edieresis"},
  {0x00cc, "Igrave"},
  {0x00cd, "Iacute"},
  {0x00ce, "Icircumflex"},
  {0x00cf, "Idieresis"},

  {0x00d0, "Eth"},
  {0x00d1, "Ntilde"},
  {0x00d2, "Ograve"},
  {0x00d3, "Oacute"},
  {0x00d4, "Ocircumflex"},
  {0x00d5, "Otilde"},
  {0x00d6, "Odieresis"},
  {0x00d7, "multiply"},
  {0x00d8, "Oslash"},
  {0x00d9, "Ugrave"},
  {0x00da, "Uacute"},
  {0x00db, "Ucircumflex"},
  {0x00dc, "Udieresis"},
  {0x00dd, "Yacute"},
  {0x00de, "Thorn"},
  {0x00df, "germandbls"},

  {0x00e0, "agrave"},
  {0x00e1, "aacute"},
  {0x00e2, "acircumflex"},
  {0x00e3, "atilde"},
  {0x00e4, "adieresis"},
  {0x00e5, "aring"},
  {0x00e6, "ae"},
  {0x00e7, "ccedilla"},
  {0x00e8, "egrave"},
  {0x00e9, "eacute"},
  {0x00ea, "ecircumflex"},
  {0x00eb, "edieresis"},
  {0x00ec, "igrave"},
  {0x00ed, "iacute"},
  {0x00ee, "icircumflex"},
  {0x00ef, "idieresis"},

  {0x00f0, "eth"},
  {0x00f1, "ntilde"},
  {0x00f2, "ograve"},
  {0x00f3, "oacute"},
  {0x00f4, "ocircumflex"},
  {0x00f5, "otilde"},
  {0x00f6, "odieresis"},
  {0x00f7, "divide"},
  {0x00f8, "oslash"},
  {0x00f9, "ugrave"},
  {0x00fa, "uacute"},
  {0x00fb, "ucircumflex"},
  {0x00fc, "udieresis"},
  {0x00fd, "yacute"},
  {0x00fe, "thorn"},
  {0x00ff, "ydieresis"},

  {0x0100, "Amacron"},
  {0x0101, "amacron"},
  {0x0102, "Abreve"},
  {0x0103, "abreve"},
  {0x0104, "Aogonek"},
  {0x0105, "aogonek"},
  {0x0106, "Cacute"},
  {0x0107, "cacute"},
  {0x0108, "Ccircumflex"},
  {0x0109, "ccircumflex"},
  {0x010a, "Cdot"},
  {0x010b, "cdot"},
  {0x010c, "Ccaron"},
  {0x010d, "ccaron"},
  {0x010e, "Dcaron"},
  {0x010f, "dcaron"},

  {0x0110, "Dslash"},
  {0x0111, "dmacron"},
  {0x0112, "Emacron"},
  {0x0113, "emacron"},
  {0x0114, "Ebreve"},
  {0x0115, "ebreve"},
  {0x0116, "Edot"},
  {0x0117, "edot"},
  {0x0118, "Eogonek"},
  {0x0119, "eogonek"},
  {0x011a, "Ecaron"},
  {0x011b, "ecaron"},
  {0x011c, "Gcircumflex"},
  {0x011d, "gcircumflex"},
  {0x011e, "Gbreve"},
  {0x011f, "gbreve"},

  {0x0120, "Gdot"},
  {0x0121, "gdot"},
  {0x0122, "Gcedilla"},
  {0x0123, "gcedilla"},
  {0x0124, "Hcircumflex"},
  {0x0125, "hcircumflex"},
  {0x0126, "Hbar"},
  {0x0127, "hbar"},
  {0x0128, "Itilde"},
  {0x0129, "itilde"},
  {0x012a, "Imacron"},
  {0x012b, "imacron"},
  {0x012c, "Ibreve"},
  {0x012d, "ibreve"},
  {0x012e, "Iogonek"},
  {0x012f, "iogonek"},

  {0x0130, "Idot"},
  {0x0131, "dotlessi"},
  {0x0132, "IJ"},
  {0x0133, "ij"},
  {0x0134, "Jcircumflex"},
  {0x0135, "jcircumflex"},
  {0x0136, "Kcedilla"},
  {0x0137, "kcedilla"},
  {0x0138, "kgreenlandic"},
  {0x0139, "Lacute"},
  {0x013a, "lacute"},
  {0x013b, "Lcedilla"},
  {0x013c, "lcedilla"},
  {0x013d, "Lcaron"},
  {0x013e, "lcaron"},
  {0x013f, "Ldot"},

  {0x0140, "ldot"},
  {0x0141, "Lslash"},
  {0x0142, "lslash"},
  {0x0143, "Nacute"},
  {0x0144, "nacute"},
  {0x0145, "Ncedilla"},
  {0x0146, "ncedilla"},
  {0x0147, "Ncaron"},
  {0x0148, "ncaron"},
  {0x0149, "napostrophe"},
  {0x014a, "Eng"},
  {0x014b, "eng"},
  {0x014c, "Omacron"},
  {0x014d, "omacron"},
  {0x014e, "Obreve"},
  {0x014f, "obreve"},

  {0x0150, "Odblacute"},
  {0x0151, "odblacute"},
  {0x0152, "OE"},
  {0x0153, "oe"},
  {0x0154, "Racute"},
  {0x0155, "racute"},
  {0x0156, "Rcedilla"},
  {0x0157, "rcedilla"},
  {0x0158, "Rcaron"},
  {0x0159, "rcaron"},
  {0x015a, "Sacute"},
  {0x015b, "sacute"},
  {0x015c, "Scircumflex"},
  {0x015d, "scircumflex"},
  {0x015e, "Scedilla"},
  {0x015f, "scedilla"},

  {0x0160, "Scaron"},
  {0x0161, "scaron"},
  {0x0162, "Tcedilla"},
  {0x0163, "tcedilla"},
  {0x0164, "Tcaron"},
  {0x0165, "tcaron"},
  {0x0166, "Tbar"},
  {0x0167, "tbar"},
  {0x0168, "Utilde"},
  {0x0169, "utilde"},
  {0x016a, "Umacron"},
  {0x016b, "umacron"},
  {0x016c, "Ubreve"},
  {0x016d, "ubreve"},
  {0x016e, "Uring"},
  {0x016f, "uring"},

  {0x0170, "Udblacute"},
  {0x0171, "udblacute"},
  {0x0172, "Uogonek"},
  {0x0173, "uogonek"},
  {0x0174, "Wcircumflex"},
  {0x0175, "wcircumflex"},
  {0x0176, "Ycircumflex"},
  {0x0177, "ycircumflex"},
  {0x0178, "Ydieresis"},
  {0x0179, "Zacute"},
  {0x017a, "zacute"},
  {0x017b, "Zdot"},
  {0x017c, "zdot"},
  {0x017d, "Zcaron"},
  {0x017e, "zcaron"},
  {0x017f, "longs"},

  {0x0192, "florin"},

  {0x01fa, "Aringacute"},
  {0x01fb, "aringacute"},
  {0x01fc, "AEacute"},
  {0x01fd, "aeacute"},
  {0x01fe, "Oslashacute"},
  {0x01ff, "oslashacute"},

  {0x02c6, "circumflex"},
  {0x02c7, "caron"},
  {0x02c9, "macron"},           /* cf "overscore" */

  {0x02d8, "breve"},
  {0x02d9, "dotaccent"},
  {0x02da, "ring"},
  {0x02db, "ogonek"},
  {0x02dc, "tilde"},
  {0x02dd, "hungarumlaut"},

  {0x037e, "semicolon"},

  {0x0384, "tonos"},
  {0x0385, "dieresistonos"},
  {0x0386, "Alphatonos"},
  {0x0387, "anoteleia"},
  {0x0388, "Epsilontonos"},
  {0x0389, "Etatonos"},
  {0x038a, "Iotatonos"},
  {0x038c, "Omicrontonos"},
  {0x038e, "Upsilontonos"},
  {0x038f, "Omegatonos"},

  {0x0390, "iotadieresistonos"},
  {0x0391, "Alpha"},
  {0x0392, "Beta"},
  {0x0393, "Gamma"},
  {0x0394, "Delta"},
  {0x0395, "Epsilon"},
  {0x0396, "Zeta"},
  {0x0397, "Eta"},
  {0x0398, "Theta"},
  {0x0399, "Iota"},
  {0x039a, "Kappa"},
  {0x039b, "Lambda"},
  {0x039c, "Mu"},
  {0x039d, "Nu"},
  {0x039e, "Xi"},
  {0x039f, "Omicron"},

  {0x03a0, "Pi"},
  {0x03a1, "Rho"},
  {0x03a2, "Sigma"},
  {0x03a3, "Tau"},
  {0x03a4, "Upsilon"},
  {0x03a5, "Phi"},
  {0x03a6, "Chi"},
  {0x03a7, "Psi"},
  {0x03a8, "Omega"},            /* cf "Ohm" */
  {0x03aa, "Iotadieresis"},
  {0x03ab, "Upsilondieresis"},
  {0x03ac, "alphatonos"},
  {0x03ad, "epsilontonos"},
  {0x03ae, "etatonos"},
  {0x03af, "iotatonos"},

  {0x03b0, "upsilondieresistonos"},
  {0x03b1, "alpha"},
  {0x03b2, "beta"},
  {0x03b3, "gamma"},
  {0x03b4, "delta"},
  {0x03b5, "epsilon"},
  {0x03b6, "zeta"},
  {0x03b7, "eta"},
  {0x03b8, "theta"},
  {0x03b9, "iota"},
  {0x03ba, "kappa"},
  {0x03bb, "lambda"},
  {0x03bc, "mu"},
  {0x03bd, "nu"},
  {0x03be, "xi"},
  {0x03bf, "omicron"},

  {0x03c0, "pi"},
  {0x03c1, "rho"},
  {0x03c2, "sigma1"},
  {0x03c3, "sigma"},
  {0x03c4, "tau"},
  {0x03c5, "upsilon"},
  {0x03c6, "phi"},
  {0x03c7, "chi"},
  {0x03c8, "psi"},
  {0x03c9, "omega"},
  {0x03ca, "iotadieresis"},
  {0x03cb, "upsilondieresis"},
  {0x03cc, "omicrontonos"},
  {0x03cd, "upsilontonos"},
  {0x03ce, "omegatonos"},

  {0x0401, "afii10023"},
  {0x0402, "afii10051"},
  {0x0403, "afii10052"},
  {0x0404, "afii10053"},
  {0x0405, "afii10054"},
  {0x0406, "afii10055"},
  {0x0407, "afii10056"},
  {0x0408, "afii10057"},
  {0x0409, "afii10058"},
  {0x040a, "afii10059"},
  {0x040b, "afii10060"},
  {0x040c, "afii10061"},
  {0x040e, "afii10062"},
  {0x040f, "afii10145"},

  {0x0410, "afii10017"},
  {0x0411, "afii10018"},
  {0x0412, "afii10019"},
  {0x0413, "afii10020"},
  {0x0414, "afii10021"},
  {0x0415, "afii10022"},
  {0x0416, "afii10024"},
  {0x0417, "afii10025"},
  {0x0418, "afii10026"},
  {0x0419, "afii10027"},
  {0x041a, "afii10028"},
  {0x041b, "afii10029"},
  {0x041c, "afii10030"},
  {0x041d, "afii10031"},
  {0x041e, "afii10032"},
  {0x041f, "afii10033"},

  {0x0420, "afii10034"},
  {0x0421, "afii10035"},
  {0x0422, "afii10036"},
  {0x0423, "afii10037"},
  {0x0424, "afii10038"},
  {0x0425, "afii10039"},
  {0x0426, "afii10040"},
  {0x0427, "afii10041"},
  {0x0428, "afii10042"},
  {0x0429, "afii10043"},
  {0x042a, "afii10044"},
  {0x042b, "afii10045"},
  {0x042c, "afii10046"},
  {0x042d, "afii10047"},
  {0x042e, "afii10048"},
  {0x042f, "afii10049"},

  {0x0430, "afii10065"},
  {0x0431, "afii10066"},
  {0x0432, "afii10067"},
  {0x0433, "afii10068"},
  {0x0434, "afii10069"},
  {0x0435, "afii10070"},
  {0x0436, "afii10072"},
  {0x0437, "afii10073"},
  {0x0438, "afii10074"},
  {0x0439, "afii10075"},
  {0x043a, "afii10076"},
  {0x043b, "afii10077"},
  {0x043c, "afii10078"},
  {0x043d, "afii10079"},
  {0x043e, "afii10080"},
  {0x043f, "afii10081"},

  {0x0440, "afii10082"},
  {0x0441, "afii10083"},
  {0x0442, "afii10084"},
  {0x0443, "afii10085"},
  {0x0444, "afii10086"},
  {0x0445, "afii10087"},
  {0x0446, "afii10088"},
  {0x0447, "afii10089"},
  {0x0448, "afii10090"},
  {0x0449, "afii10091"},
  {0x044a, "afii10092"},
  {0x044b, "afii10093"},
  {0x044c, "afii10094"},
  {0x044d, "afii10095"},
  {0x044e, "afii10096"},
  {0x044f, "afii10097"},

  {0x0451, "afii10071"},
  {0x0452, "afii10099"},
  {0x0453, "afii10100"},
  {0x0454, "afii10101"},
  {0x0455, "afii10102"},
  {0x0456, "afii10103"},
  {0x0457, "afii10104"},
  {0x0458, "afii10105"},
  {0x0459, "afii10106"},
  {0x045a, "afii10107"},
  {0x045b, "afii10108"},
  {0x045c, "afii10109"},
  {0x045e, "afii10110"},
  {0x045f, "afii10193"},

  {0x0490, "afii10050"},
  {0x0491, "afii10098"},

  {0x1e80, "Wgrave"},
  {0x1e81, "wgrave"},
  {0x1e82, "Wacute"},
  {0x1e83, "wacute"},
  {0x1e84, "Wdieresis"},
  {0x1e85, "wdieresis"},

  {0x1ef2, "Ygrave"},
  {0x1ef3, "ygrave"},

  {0x2013, "endash"},
  {0x2014, "emdash"},
  {0x2015, "afii00208"},        /* horizontal bar */
  {0x2017, "underscoredbl"},
  {0x2018, "quoteleft"},
  {0x2019, "quoteright"},
  {0x201a, "quotesinglbase"},
  {0x201b, "quotereversed"},
  {0x201c, "quotedblleft"},
  {0x201d, "quotedblright"},
  {0x201e, "quotedblbase"},
    
  {0x2020, "dagger"},
  {0x2021, "daggerdbl"},
  {0x2022, "bullet"},
  {0x2026, "ellipsis"},
  {0x2030, "perthousand"},
  {0x2032, "minute"},
  {0x2033, "second"},
  {0x2039, "guilsinglleft"},
  {0x203a, "guilsinglright"},
  {0x203c, "exclamdbl"},
  {0x203e, "radicalex"},

  {0x2044, "fraction"},         /* cf U+2215 */

  {0x207f, "nsuperior"},

  {0x20a3, "franc"},
  {0x20a4, "afii08941"},        /* lira sign */
  {0x20a7, "peseta"},

  {0x2105, "afii61248"},        /* care of */

  {0x2113, "afii61289"},        /* script small l */
  {0x2116, "afii61352"},        /* numero sign */

  {0x2122, "trademark"},
  {0x2126, "Ohm"},
  {0x212e, "estimated"},

  {0x215b, "oneeighth"},
  {0x215c, "threeeighths"},
  {0x215d, "fiveeighths"},
  {0x215e, "seveneighths"},

  {0x2190, "arrowleft"},
  {0x2191, "arrowup"},
  {0x2192, "arrowright"},
  {0x2193, "arrowdown"},
  {0x2194, "arrowboth"},
  {0x2195, "arrowupdn"},

  {0x21a8, "arrowupdnbse"},

  {0x2202, "partialdiff"},
  {0x2206, "increment"},
  {0x220f, "product"},

  {0x2211, "summation"},
  {0x2212, "minus"},
  {0x2215, "fraction"},         /* cf U+2044 */
  {0x2219, "periodcentered"},
  {0x221a, "radical"},
  {0x221e, "infinity"},
  {0x221f, "orthogonal"},

  {0x2229, "intersection"},
  {0x222b, "integral"},

  {0x2248, "approxequal"},

  {0x2260, "notequal"},
  {0x2261, "equivalence"},
  {0x2264, "lessequal"},
  {0x2265, "greaterequal"},

  {0x2302, "house"},

  {0x2310, "revlogicalnot"},

  {0x2320, "integraltp"},
  {0x2321, "integralbt"},

  {0x2500, "SF100000"},
  {0x2502, "SF110000"},
  {0x250c, "SF010000"},

  {0x2510, "SF030000"},
  {0x2514, "SF020000"},
  {0x2518, "SF040000"},
  {0x251c, "SF080000"},

  {0x2524, "SF090000"},
  {0x252c, "SF060000"},

  {0x2534, "SF070000"},
  {0x253c, "SF050000"},

  {0x2550, "SF430000"},
  {0x2551, "SF240000"},
  {0x2552, "SF510000"},
  {0x2553, "SF520000"},
  {0x2554, "SF390000"},
  {0x2555, "SF220000"},
  {0x2556, "SF210000"},
  {0x2557, "SF250000"},
  {0x2558, "SF500000"},
  {0x2559, "SF490000"},
  {0x255a, "SF380000"},
  {0x255b, "SF280000"},
  {0x255c, "SF270000"},
  {0x255d, "SF260000"},
  {0x255e, "SF360000"},
  {0x255f, "SF370000"},

  {0x2560, "SF420000"},
  {0x2561, "SF190000"},
  {0x2562, "SF200000"},
  {0x2563, "SF230000"},
  {0x2564, "SF470000"},
  {0x2565, "SF480000"},
  {0x2566, "SF410000"},
  {0x2567, "SF450000"},
  {0x2568, "SF460000"},
  {0x2569, "SF400000"},
  {0x256a, "SF540000"},
  {0x256b, "SF530000"},
  {0x256c, "SF440000"},

  {0x2580, "upblock"},
  {0x2584, "dnblock"},
  {0x2588, "block"},
  {0x258c, "lfblock"},

  {0x2590, "rtblock"},
  {0x2591, "ltshade"},
  {0x2592, "shade"},
  {0x2593, "dkshade"},

  {0x25a0, "filledbox"},
  {0x25a1, "H22073"},
  {0x25aa, "H18543"},
  {0x25ab, "H18551"},
  {0x25ac, "filledrect"},

  {0x25b2, "triagup"},
  {0x25ba, "triagrt"},
  {0x25bc, "triagdn"},

  {0x25c4, "triaglf"},
  {0x25ca, "lozenge"},
  {0x25cb, "circle"},
  {0x25cf, "H18533"},

  {0x25d8, "invbullet"},
  {0x25d9, "invcircle"},

  {0x25e6, "openbullet"},

  {0x263a, "smileface"},
  {0x263b, "invsmileface"},
  {0x263c, "sun"},

  {0x2640, "female"},
  {0x2642, "male"},

  {0x2660, "spade"},
  {0x2663, "club"},
  {0x2665, "heart"},
  {0x2666, "diamond"},
  {0x266a, "musicalnote"},
  {0x266b, "musicalnotedbl"},

  {0xf000, "applelogo"},    
  {0xf001, "fi"},
  {0xf002, "fl"},
  {0xf004, "commaaccent"},
  {0xf005, "undercommaaccent"},
  {0xfb01, "fi"},
  {0xfb02, "fl"},
};

struct encoding_table mac_table[] =
{
  {-1,     ".notdef"},
  {0x0000, ".notdef"},          /* null */
  {0x0008, ".notdef"},          /* backspace */
  {0x0009, ".notdef"},          /* horizontal tabulation */
  {0x000d, ".notdef"},          /* carriage return */
  {0x001d, ".notdef"},          /* group separator */
  {0x0020, "space"},
  {0x0021, "exclam"},
  {0x0022, "quotedbl"},
  {0x0023, "numbersign"},
  {0x0024, "dollar"},
  {0x0025, "percent"},
  {0x0026, "ampersand"},
  {0x0027, "quotesingle"},
  {0x0028, "parenleft"},
  {0x0029, "parenright"},
  {0x002a, "asterisk"},
  {0x002b, "plus"},
  {0x002c, "comma"},
  {0x002d, "hyphen"},
  {0x002e, "period"},
  {0x002f, "slash"},

  {0x0030, "zero"},
  {0x0031, "one"},
  {0x0032, "two"},
  {0x0033, "three"},
  {0x0034, "four"},
  {0x0035, "five"},
  {0x0036, "six"},
  {0x0037, "seven"},
  {0x0038, "eight"},
  {0x0039, "nine"},
  {0x003a, "colon"},
  {0x003b, "semicolon"},
  {0x003c, "less"},
  {0x003d, "equal"},
  {0x003e, "greater"},
  {0x003f, "question"},

  {0x0040, "at"},
  {0x0041, "A"},
  {0x0042, "B"},
  {0x0043, "C"},
  {0x0044, "D"},
  {0x0045, "E"},
  {0x0046, "F"},
  {0x0047, "G"},
  {0x0048, "H"},
  {0x0049, "I"},
  {0x004a, "J"},
  {0x004b, "K"},
  {0x004c, "L"},
  {0x004d, "M"},
  {0x004e, "N"},
  {0x004f, "O"},

  {0x0050, "P"},
  {0x0051, "Q"},
  {0x0052, "R"},
  {0x0053, "S"},
  {0x0054, "T"},
  {0x0055, "U"},
  {0x0056, "V"},
  {0x0057, "W"},
  {0x0058, "X"},
  {0x0059, "Y"},
  {0x005a, "Z"},
  {0x005b, "bracketleft"},
  {0x005c, "backslash"},
  {0x005d, "bracketright"},
  {0x005e, "asciicircum"},
  {0x005f, "underscore"},

  {0x0060, "grave"},
  {0x0061, "a"},
  {0x0062, "b"},
  {0x0063, "c"},
  {0x0064, "d"},
  {0x0065, "e"},
  {0x0066, "f"},
  {0x0067, "g"},
  {0x0068, "h"},
  {0x0069, "i"},
  {0x006a, "j"},
  {0x006b, "k"},
  {0x006c, "l"},
  {0x006d, "m"},
  {0x006e, "n"},
  {0x006f, "o"},

  {0x0070, "p"},
  {0x0071, "q"},
  {0x0072, "r"},
  {0x0073, "s"},
  {0x0074, "t"},
  {0x0075, "u"},
  {0x0076, "v"},
  {0x0077, "w"},
  {0x0078, "x"},
  {0x0079, "y"},
  {0x007a, "z"},
  {0x007b, "braceleft"},
  {0x007c, "bar"},
  {0x007d, "braceright"},
  {0x007e, "asciitilde"},

  {0x0080, "Adieresis"},
  {0x0081, "Aring"},
  {0x0082, "Ccedilla"},
  {0x0083, "Eacute"},
  {0x0084, "Ntilde"},
  {0x0085, "Odieresis"},
  {0x0086, "Udieresis"},
  {0x0087, "aacute"},
  {0x0088, "agrave"},
  {0x0089, "acircumflex"},
  {0x008a, "adieresis"},
  {0x008b, "atilde"},
  {0x008c, "aring"},
  {0x008d, "ccedilla"},
  {0x008e, "eacute"},
  {0x008f, "egrave"},

  {0x0090, "ecircumflex"},
  {0x0091, "edieresis"},
  {0x0092, "iacute"},
  {0x0093, "igrave"},
  {0x0094, "icircumflex"},
  {0x0095, "idieresis"},
  {0x0096, "ntilde"},
  {0x0097, "oacute"},
  {0x0098, "ograve"},
  {0x0099, "ocircumflex"},
  {0x009a, "odieresis"},
  {0x009b, "otilde"},
  {0x009c, "uacute"},
  {0x009d, "ugrave"},
  {0x009e, "ucircumflex"},
  {0x009f, "udieresis"},

  {0x00a0, "dagger"},
  {0x00a1, "degree"},
  {0x00a2, "cent"},
  {0x00a3, "sterling"},
  {0x00a4, "section"},
  {0x00a5, "bullet"},
  {0x00a6, "paragraph"},
  {0x00a7, "germandbls"},
  {0x00a8, "registered"},
  {0x00a9, "copyright"},
  {0x00aa, "trademark"},
  {0x00ab, "acute"},
  {0x00ac, "dieresis"},
  {0x00ad, "notequal"},
  {0x00ae, "AE"},
  {0x00af, "Oslash"},

  {0x00b0, "infinity"},
  {0x00b1, "plusminus"},
  {0x00b2, "lessequal"},
  {0x00b3, "greaterequal"},
  {0x00b4, "yen"},
  {0x00b5, "mu"},
  {0x00b6, "partialdiff"},
  {0x00b7, "summation"},
  {0x00b8, "product"},
  {0x00b9, "pi"},
  {0x00ba, "integral"},
  {0x00bb, "ordfeminine"},
  {0x00bc, "ordmasculine"},
  {0x00bd, "Omega"},
  {0x00be, "ae"},
  {0x00bf, "oslash"},

  {0x00c0, "questiondown"},
  {0x00c1, "exclamdown"},
  {0x00c2, "logicalnot"},
  {0x00c3, "radical"},
  {0x00c4, "florin"},
  {0x00c5, "approxequal"},
  {0x00c6, "Delta"},
  {0x00c7, "guillemotleft"},
  {0x00c8, "guillemotright"},
  {0x00c9, "ellipsis"},
  {0x00ca, "nbspace"},
  {0x00cb, "Agrave"},
  {0x00cc, "Atilde"},
  {0x00cd, "Otilde"},
  {0x00ce, "OE"},
  {0x00cf, "oe"},

  {0x00d0, "endash"},
  {0x00d1, "emdash"},
  {0x00d2, "quotedblleft"},
  {0x00d3, "quotedblright"},
  {0x00d4, "quoteleft"},
  {0x00d5, "quoteright"},
  {0x00d6, "divide"},
  {0x00d7, "lozenge"},
  {0x00d8, "ydieresis"},
  {0x00d9, "Ydieresis"},
  {0x00da, "fraction"},
  {0x00db, "currency"},
  {0x00dc, "guilsinglleft"},
  {0x00dd, "guilsinglright"},
  {0x00de, "fi"},
  {0x00df, "fl"},

  {0x00e0, "daggerdbl"},
  {0x00e1, "periodcentered"},
  {0x00e2, "quotesinglbase"},
  {0x00e3, "quotedblbase"},
  {0x00e4, "perthousand"},
  {0x00e5, "Acircumflex"},
  {0x00e6, "Ecircumflex"},
  {0x00e7, "Aacute"},
  {0x00e8, "Edieresis"},
  {0x00e9, "Egrave"},
  {0x00ea, "Iacute"},
  {0x00eb, "Icircumflex"},
  {0x00ec, "Idieresis"},
  {0x00ed, "Igrave"},
  {0x00ee, "Oacute"},
  {0x00ef, "Ocircumflex"},

  {0x00f0, "apple"},
  {0x00f1, "Ograve"},
  {0x00f2, "Uacute"},
  {0x00f3, "Ucircumflex"},
  {0x00f4, "Ugrave"},
  {0x00f5, "dotlessi"},
  {0x00f6, "circumflex"},
  {0x00f7, "tilde"},
  {0x00f8, "macron"},
  {0x00f9, "breve"},
  {0x00fa, "dotaccent"},
  {0x00fb, "ring"},
  {0x00fc, "cedilla"},
  {0x00fd, "hungarumlaut"},
  {0x00fe, "ogonek"},
  {0x00ff, "caron"},
};


struct encoding_table *current_table;
size_t current_table_len;


void 
set_encoding_scheme(EncodingScheme e, Font *fnt)
{
  current_encoding_scheme = e;

  switch (e)
  {
  case encUnicode:
    current_table = unicode_table;
    current_table_len = sizeof (unicode_table) / sizeof (unicode_table[0]);
    break;

  case encMac:
    current_table = mac_table;
    current_table_len = sizeof (mac_table) / sizeof (mac_table[0]);
    break;

  case encFontSpecific:
    break;
  }
}


/*
 *   We return ".c0x<code point in hexadecimal representation>"
 *   if no name is found.
 *
 *   We return ".g0x<code point in hexadecimal representation>"
 *   if it's a glyph index (code >= 0x1000000).
 */

const char *
code_to_adobename(long code)
{
  unsigned int n, n1 = 0, n2 = current_table_len - 1;
  char *p;


  if (current_encoding_scheme == encFontSpecific)
  {
    p = (char *)mymalloc(11);
    sprintf(p, ".%c0x%lx", (code >= 0x1000000) ? 'g' : 'c',
                           (code & 0xFFFFFF));
    return p;
  }

  while (n1 <= n2)
  {
    n = (n1 + n2) / 2;
    if (code < current_table[n].code)
      n2 = n - 1;
    else if (code > current_table[n].code)
      n1 = n + 1;
    else
      return current_table[n].adobename;
  }

  p = (char *)mymalloc(11);
  sprintf(p, ".%c0x%lx", (code >= 0x1000000) ? 'g' : 'c',
                         (code & 0xFFFFFF));
  return p;    
}


/*
 *   The first of two identical entries will win.
 */

long
adobename_to_code(const char *s)
{
  size_t i;
  long j;
  char p;
  char *e;


  if (s == NULL)
    return -1;

  if (current_encoding_scheme == encFontSpecific)
  {
    if (*(s++) != '.')
      return -1;

    p = *(s++);
    if (!(p == 'c' || p == 'g'))
      return -1;

    j = strtol(s, &e, 0);
    if (*e == '\0')
      return (p == 'g') ? (j | 0x1000000) : j;
    else
      return -1;
  }

  for (i = 0; i < current_table_len; i++)
  {
    if (strcmp(current_table[i].adobename, s) == 0)
      return current_table[i].code;
  }

  if (*(s++) != '.')
    return -1;

  p = *(s++);
  if (!(p == 'c' || p == 'g'))
    return -1;

  j = strtol(s, &e, 0);
  if (*e == '\0')
    return (p == 'g') ? (j | 0x1000000) : j;
  else
    return -1;
}


ttfinfo *
findglyph(unsigned short g, ttfinfo *p)
{
  register ttfinfo *ti;


  if (!p)
    return NULL;

  for (ti = p; ti; ti = ti->next)
   if (g == ti->glyphindex)
     return ti;

  return NULL;
}


ttfinfo *
findadobe(const char *p, ttfinfo *ap)
{
  register ttfinfo *ti;
  register long l = -1;
  register char c = '\0', d = '\0';


  if (!p)
    return NULL;

  if (p[0] == '.' &&
      (c = p[1]) && (c == 'c' || c == 'g') &&
      (d = p[2]) && '0' <= d && d <= '9')
    l = strtol(p + 2, NULL, 0);

  for (ti = ap; ti; ti = ti->next)
  {
    if (l >= 0)
    {
      if (c == 'c')
      {
        if (ti->charcode == l)
          return ti;
      }
      else
      {
        if (ti->glyphindex == l)
          return ti;
      }
    }
    else if (strcmp(p, ti->adobename) == 0)
      return ti;
  }

  return NULL;
}


ttfinfo *
findmappedadobe(const char *p, ttfinfo **array)
{
  register int i;
  register ttfinfo *ti;
  register long l = -1;
  register char c = '\0', d = '\0';


  if (!p)
    return NULL;

  if (p[0] == '.' &&
      (c = p[1]) && (c == 'c' || c == 'g') &&
      (d = p[2]) && '0' <= d && d <= '9')
    l = strtol(p + 2, NULL, 0);

  for (i = 0; i <= 0xFF; i++)
    if ((ti = array[i]))
    {
      if (l >= 0)
      {
        if (c == 'c')
        {
          if (ti->charcode == l)
            return ti;
        }
        else
        {
          if (ti->glyphindex == l)
            return ti;
        }
      }
      else if (strcmp(p, ti->adobename) == 0)
        return ti;
    }

  return NULL;
}


void
replace_glyphs(Font *fnt)
{
  stringlist *sl, *sl_old;
  ttfinfo *ti;


  for (sl = fnt->replacements, sl_old = NULL; sl; sl_old = sl, sl = sl->next)
  {
    if ((ti = findadobe(sl->old_name, fnt->charlist)))
      ti->adobename = sl->new_name;
    else
    {
      warning("Glyph name `%s' not found.", sl->old_name);
      warning("Replacement glyph name `%s' thus ignored.", sl->new_name);
      if (sl_old == NULL)
        fnt->replacements = sl->next;
      else
        sl_old->next = sl->next;
    }
  }
}


/* the opposite of replace_glyph() */

void
restore_glyph(encoding *enc, Font *fnt)
{
  stringlist *sl;
  int i;


  for (sl = fnt->replacements; sl; sl = sl->next)
  {
    for (i = 0; i <= 0xFF; i++)
    {
      if (strcmp(enc->vec[i], sl->new_name) == 0)
      {
        enc->vec[i] = sl->old_name;
        goto success;
      }
    }
    warning("Glyph name `%s' not found in encoding.", sl->new_name);
    warning("Replacement for glyph name `%s' thus ignored.", sl->old_name);

success:
    ;
  }
}


/* end */
