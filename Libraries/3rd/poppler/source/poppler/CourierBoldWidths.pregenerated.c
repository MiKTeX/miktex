/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf poppler/CourierBoldWidths.gperf  */
/* Computed positions: -k'1-2,5,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) && ('-' == 45) && ('.' == 46) && ('/' == 47)           \
      && ('0' == 48) && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) && ('=' == 61) && ('>' == 62)         \
      && ('?' == 63) && ('A' == 65) && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) && ('N' == 78)         \
      && ('O' == 79) && ('P' == 80) && ('Q' == 81) && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93)        \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) && ('k' == 107) && ('l' == 108)               \
      && ('m' == 109) && ('n' == 110) && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122)          \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#    error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "poppler/CourierBoldWidths.gperf"

#include <string.h>
#include "BuiltinFontWidth.h"

#define TOTAL_KEYWORDS 315
#define MIN_WORD_LENGTH 1
#define MAX_WORD_LENGTH 14
#define MIN_HASH_VALUE 1
#define MAX_HASH_VALUE 1041
/* maximum key range = 1041, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#    ifdef __cplusplus
inline
#    endif
#endif
        static unsigned int
        hash(register const char *str, register size_t len)
{
    static const unsigned short asso_values[] = { 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042,
                                                  1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042,
                                                  1042, 1042, 1042, 1042, 1042, 1042, 1042, 270,  415,  28,   8,    150,  390,  290,  375,  370,  335,  5,    455,  330,  405,  355,  325,  310,  3,    320,  160,  240,  225,
                                                  145,  70,   410,  460,  1042, 1042, 1042, 1042, 1042, 1042, 20,   345,  30,   115,  0,    395,  140,  165,  135,  35,   380,  170,  130,  15,   45,   215,  260,  100,  65,
                                                  10,   155,  400,  300,  305,  280,  315,  1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042,
                                                  1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042,
                                                  1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042,
                                                  1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042,
                                                  1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042, 1042 };
    register unsigned int hval = len;

    switch (hval) {
    default:
        hval += asso_values[(unsigned char)str[4]];
    /*FALLTHROUGH*/
    case 4:
    case 3:
    case 2:
        hval += asso_values[(unsigned char)str[1]];
    /*FALLTHROUGH*/
    case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
    return hval + asso_values[(unsigned char)str[len - 1]];
}

const struct BuiltinFontWidth *CourierBoldWidthsLookup(register const char *str, register size_t len)
{
    static const struct BuiltinFontWidth wordlist[] = { { "", 0 },
#line 90 "poppler/CourierBoldWidths.gperf"
                                                        { "e", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 70 "poppler/CourierBoldWidths.gperf"
                                                        { "R", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 57 "poppler/CourierBoldWidths.gperf"
                                                        { "K", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 49 "poppler/CourierBoldWidths.gperf"
                                                        { "D", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 115 "poppler/CourierBoldWidths.gperf"
                                                        { "t", 600 },
#line 191 "poppler/CourierBoldWidths.gperf"
                                                        { "ae", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 102 "poppler/CourierBoldWidths.gperf"
                                                        { "n", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 207 "poppler/CourierBoldWidths.gperf"
                                                        { "eacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 216 "poppler/CourierBoldWidths.gperf"
                                                        { "Racute", 600 },
                                                        { "", 0 },
#line 85 "poppler/CourierBoldWidths.gperf"
                                                        { "a", 600 },
#line 206 "poppler/CourierBoldWidths.gperf"
                                                        { "at", 600 },
                                                        { "", 0 },
#line 308 "poppler/CourierBoldWidths.gperf"
                                                        { "cent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 161 "poppler/CourierBoldWidths.gperf"
                                                        { "oe", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 145 "poppler/CourierBoldWidths.gperf"
                                                        { "nacute", 600 },
                                                        { "", 0 },
#line 254 "poppler/CourierBoldWidths.gperf"
                                                        { "Delta", 600 },
                                                        { "", 0 },
#line 273 "poppler/CourierBoldWidths.gperf"
                                                        { "acute", 600 },
#line 112 "poppler/CourierBoldWidths.gperf"
                                                        { "aacute", 600 },
#line 47 "poppler/CourierBoldWidths.gperf"
                                                        { "C", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 88 "poppler/CourierBoldWidths.gperf"
                                                        { "c", 600 },
                                                        { "", 0 },
#line 173 "poppler/CourierBoldWidths.gperf"
                                                        { "one", 600 },
#line 285 "poppler/CourierBoldWidths.gperf"
                                                        { "Cacute", 600 },
                                                        { "", 0 },
#line 300 "poppler/CourierBoldWidths.gperf"
                                                        { "cacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 98 "poppler/CourierBoldWidths.gperf"
                                                        { "j", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 210 "poppler/CourierBoldWidths.gperf"
                                                        { "Dcroat", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 249 "poppler/CourierBoldWidths.gperf"
                                                        { "oacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 72 "poppler/CourierBoldWidths.gperf"
                                                        { "caron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 104 "poppler/CourierBoldWidths.gperf"
                                                        { "o", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 317 "poppler/CourierBoldWidths.gperf"
                                                        { "ecaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 321 "poppler/CourierBoldWidths.gperf"
                                                        { "Rcaron", 600 },
#line 290 "poppler/CourierBoldWidths.gperf"
                                                        { "seven", 600 },
#line 306 "poppler/CourierBoldWidths.gperf"
                                                        { "sacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 224 "poppler/CourierBoldWidths.gperf"
                                                        { "Dcaron", 600 },
                                                        { "", 0 },
#line 252 "poppler/CourierBoldWidths.gperf"
                                                        { "tcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 26 "poppler/CourierBoldWidths.gperf"
                                                        { "colon", 600 },
#line 278 "poppler/CourierBoldWidths.gperf"
                                                        { "ncaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 125 "poppler/CourierBoldWidths.gperf"
                                                        { "commaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 135 "poppler/CourierBoldWidths.gperf"
                                                        { "semicolon", 600 },
#line 19 "poppler/CourierBoldWidths.gperf"
                                                        { "comma", 600 },
#line 235 "poppler/CourierBoldWidths.gperf"
                                                        { "degree", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 118 "poppler/CourierBoldWidths.gperf"
                                                        { "Ccaron", 600 },
                                                        { "", 0 },
#line 140 "poppler/CourierBoldWidths.gperf"
                                                        { "ccaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 113 "poppler/CourierBoldWidths.gperf"
                                                        { "s", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 231 "poppler/CourierBoldWidths.gperf"
                                                        { "racute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 79 "poppler/CourierBoldWidths.gperf"
                                                        { "X", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 30 "poppler/CourierBoldWidths.gperf"
                                                        { "ntilde", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 205 "poppler/CourierBoldWidths.gperf"
                                                        { "tilde", 600 },
#line 324 "poppler/CourierBoldWidths.gperf"
                                                        { "atilde", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 196 "poppler/CourierBoldWidths.gperf"
                                                        { "nine", 600 },
#line 24 "poppler/CourierBoldWidths.gperf"
                                                        { "edotaccent", 600 },
#line 105 "poppler/CourierBoldWidths.gperf"
                                                        { "ordfeminine", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 158 "poppler/CourierBoldWidths.gperf"
                                                        { "eight", 600 },
#line 150 "poppler/CourierBoldWidths.gperf"
                                                        { "scaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 276 "poppler/CourierBoldWidths.gperf"
                                                        { "iacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 170 "poppler/CourierBoldWidths.gperf"
                                                        { "otilde", 600 },
#line 292 "poppler/CourierBoldWidths.gperf"
                                                        { "ordmasculine", 600 },
#line 213 "poppler/CourierBoldWidths.gperf"
                                                        { "eth", 600 },
                                                        { "", 0 },
#line 42 "poppler/CourierBoldWidths.gperf"
                                                        { "three", 600 },
#line 225 "poppler/CourierBoldWidths.gperf"
                                                        { "dcroat", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 281 "poppler/CourierBoldWidths.gperf"
                                                        { "Rcommaaccent", 600 },
#line 185 "poppler/CourierBoldWidths.gperf"
                                                        { "Eacute", 600 },
#line 322 "poppler/CourierBoldWidths.gperf"
                                                        { "Kcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 218 "poppler/CourierBoldWidths.gperf"
                                                        { "uacute", 600 },
#line 103 "poppler/CourierBoldWidths.gperf"
                                                        { "tcommaaccent", 600 },
                                                        { "", 0 },
#line 166 "poppler/CourierBoldWidths.gperf"
                                                        { "copyright", 600 },
#line 43 "poppler/CourierBoldWidths.gperf"
                                                        { "numbersign", 600 },
#line 15 "poppler/CourierBoldWidths.gperf"
                                                        { "rcaron", 600 },
#line 32 "poppler/CourierBoldWidths.gperf"
                                                        { "ncommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 110 "poppler/CourierBoldWidths.gperf"
                                                        { "r", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 264 "poppler/CourierBoldWidths.gperf"
                                                        { "lacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 23 "poppler/CourierBoldWidths.gperf"
                                                        { "dotaccent", 600 },
#line 234 "poppler/CourierBoldWidths.gperf"
                                                        { "thorn", 600 },
#line 242 "poppler/CourierBoldWidths.gperf"
                                                        { "dcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 146 "poppler/CourierBoldWidths.gperf"
                                                        { "macron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 203 "poppler/CourierBoldWidths.gperf"
                                                        { "Ccedilla", 600 },
#line 274 "poppler/CourierBoldWidths.gperf"
                                                        { "section", 600 },
#line 223 "poppler/CourierBoldWidths.gperf"
                                                        { "ccedilla", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 20 "poppler/CourierBoldWidths.gperf"
                                                        { "cedilla", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 25 "poppler/CourierBoldWidths.gperf"
                                                        { "asciitilde", 600 },
#line 89 "poppler/CourierBoldWidths.gperf"
                                                        { "d", 600 },
#line 239 "poppler/CourierBoldWidths.gperf"
                                                        { "percent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 288 "poppler/CourierBoldWidths.gperf"
                                                        { "germandbls", 600 },
                                                        { "", 0 },
#line 138 "poppler/CourierBoldWidths.gperf"
                                                        { "lozenge", 600 },
                                                        { "", 0 },
#line 316 "poppler/CourierBoldWidths.gperf"
                                                        { "less", 600 },
                                                        { "", 0 },
#line 97 "poppler/CourierBoldWidths.gperf"
                                                        { "dagger", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 258 "poppler/CourierBoldWidths.gperf"
                                                        { "grave", 600 },
#line 301 "poppler/CourierBoldWidths.gperf"
                                                        { "Ecaron", 600 },
#line 222 "poppler/CourierBoldWidths.gperf"
                                                        { "scommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 160 "poppler/CourierBoldWidths.gperf"
                                                        { "endash", 600 },
#line 174 "poppler/CourierBoldWidths.gperf"
                                                        { "emacron", 600 },
#line 201 "poppler/CourierBoldWidths.gperf"
                                                        { "threequarters", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 232 "poppler/CourierBoldWidths.gperf"
                                                        { "Tcaron", 600 },
                                                        { "", 0 },
#line 228 "poppler/CourierBoldWidths.gperf"
                                                        { "scedilla", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 101 "poppler/CourierBoldWidths.gperf"
                                                        { "m", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 245 "poppler/CourierBoldWidths.gperf"
                                                        { "summation", 600 },
#line 310 "poppler/CourierBoldWidths.gperf"
                                                        { "logicalnot", 600 },
#line 44 "poppler/CourierBoldWidths.gperf"
                                                        { "lcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 172 "poppler/CourierBoldWidths.gperf"
                                                        { "parenleft", 600 },
#line 139 "poppler/CourierBoldWidths.gperf"
                                                        { "parenright", 600 },
#line 95 "poppler/CourierBoldWidths.gperf"
                                                        { "i", 600 },
#line 305 "poppler/CourierBoldWidths.gperf"
                                                        { "amacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 194 "poppler/CourierBoldWidths.gperf"
                                                        { "Uacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 208 "poppler/CourierBoldWidths.gperf"
                                                        { "underscore", 600 },
#line 92 "poppler/CourierBoldWidths.gperf"
                                                        { "g", 600 },
#line 297 "poppler/CourierBoldWidths.gperf"
                                                        { "rcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 37 "poppler/CourierBoldWidths.gperf"
                                                        { "space", 600 },
#line 28 "poppler/CourierBoldWidths.gperf"
                                                        { "dollar", 600 },
                                                        { "", 0 },
#line 272 "poppler/CourierBoldWidths.gperf"
                                                        { "threesuperior", 600 },
#line 188 "poppler/CourierBoldWidths.gperf"
                                                        { "edieresis", 600 },
#line 236 "poppler/CourierBoldWidths.gperf"
                                                        { "registered", 600 },
#line 78 "poppler/CourierBoldWidths.gperf"
                                                        { "W", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 64 "poppler/CourierBoldWidths.gperf"
                                                        { "omacron", 600 },
#line 36 "poppler/CourierBoldWidths.gperf"
                                                        { "yen", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 50 "poppler/CourierBoldWidths.gperf"
                                                        { "E", 600 },
                                                        { "", 0 },
#line 293 "poppler/CourierBoldWidths.gperf"
                                                        { "dotlessi", 600 },
                                                        { "", 0 },
#line 327 "poppler/CourierBoldWidths.gperf"
                                                        { "Edotaccent", 600 },
#line 71 "poppler/CourierBoldWidths.gperf"
                                                        { "Aacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 186 "poppler/CourierBoldWidths.gperf"
                                                        { "adieresis", 600 },
                                                        { "", 0 },
#line 117 "poppler/CourierBoldWidths.gperf"
                                                        { "u", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 144 "poppler/CourierBoldWidths.gperf"
                                                        { "daggerdbl", 600 },
                                                        { "", 0 },
#line 280 "poppler/CourierBoldWidths.gperf"
                                                        { "yacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 74 "poppler/CourierBoldWidths.gperf"
                                                        { "T", 600 },
#line 130 "poppler/CourierBoldWidths.gperf"
                                                        { "gcommaaccent", 600 },
#line 275 "poppler/CourierBoldWidths.gperf"
                                                        { "dieresis", 600 },
                                                        { "", 0 },
#line 51 "poppler/CourierBoldWidths.gperf"
                                                        { "onequarter", 600 },
#line 328 "poppler/CourierBoldWidths.gperf"
                                                        { "onesuperior", 600 },
#line 237 "poppler/CourierBoldWidths.gperf"
                                                        { "radical", 600 },
#line 190 "poppler/CourierBoldWidths.gperf"
                                                        { "Eth", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 94 "poppler/CourierBoldWidths.gperf"
                                                        { "h", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 193 "poppler/CourierBoldWidths.gperf"
                                                        { "odieresis", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 100 "poppler/CourierBoldWidths.gperf"
                                                        { "l", 600 },
#line 65 "poppler/CourierBoldWidths.gperf"
                                                        { "Tcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 136 "poppler/CourierBoldWidths.gperf"
                                                        { "oslash", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 137 "poppler/CourierBoldWidths.gperf"
                                                        { "lessequal", 600 },
#line 159 "poppler/CourierBoldWidths.gperf"
                                                        { "exclamdown", 600 },
#line 35 "poppler/CourierBoldWidths.gperf"
                                                        { "zacute", 600 },
#line 269 "poppler/CourierBoldWidths.gperf"
                                                        { "lcommaaccent", 600 },
                                                        { "", 0 },
#line 209 "poppler/CourierBoldWidths.gperf"
                                                        { "Euro", 600 },
                                                        { "", 0 },
#line 291 "poppler/CourierBoldWidths.gperf"
                                                        { "Sacute", 600 },
#line 323 "poppler/CourierBoldWidths.gperf"
                                                        { "greater", 600 },
#line 244 "poppler/CourierBoldWidths.gperf"
                                                        { "two", 600 },
                                                        { "", 0 },
#line 220 "poppler/CourierBoldWidths.gperf"
                                                        { "Thorn", 600 },
#line 256 "poppler/CourierBoldWidths.gperf"
                                                        { "asciicircum", 600 },
#line 126 "poppler/CourierBoldWidths.gperf"
                                                        { "hungarumlaut", 600 },
                                                        { "", 0 },
#line 212 "poppler/CourierBoldWidths.gperf"
                                                        { "zero", 600 },
                                                        { "", 0 },
#line 40 "poppler/CourierBoldWidths.gperf"
                                                        { "emdash", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 116 "poppler/CourierBoldWidths.gperf"
                                                        { "divide", 600 },
                                                        { "", 0 },
#line 271 "poppler/CourierBoldWidths.gperf"
                                                        { "ohungarumlaut", 600 },
#line 262 "poppler/CourierBoldWidths.gperf"
                                                        { "ampersand", 600 },
                                                        { "", 0 },
#line 164 "poppler/CourierBoldWidths.gperf"
                                                        { "ecircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 106 "poppler/CourierBoldWidths.gperf"
                                                        { "ring", 600 },
                                                        { "", 0 },
#line 320 "poppler/CourierBoldWidths.gperf"
                                                        { "period", 600 },
                                                        { "", 0 },
#line 318 "poppler/CourierBoldWidths.gperf"
                                                        { "guilsinglleft", 600 },
#line 155 "poppler/CourierBoldWidths.gperf"
                                                        { "guilsinglright", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 307 "poppler/CourierBoldWidths.gperf"
                                                        { "imacron", 600 },
                                                        { "", 0 },
#line 61 "poppler/CourierBoldWidths.gperf"
                                                        { "periodcentered", 600 },
                                                        { "", 0 },
#line 227 "poppler/CourierBoldWidths.gperf"
                                                        { "Oacute", 600 },
                                                        { "", 0 },
#line 294 "poppler/CourierBoldWidths.gperf"
                                                        { "sterling", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 299 "poppler/CourierBoldWidths.gperf"
                                                        { "acircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 33 "poppler/CourierBoldWidths.gperf"
                                                        { "minus", 600 },
#line 312 "poppler/CourierBoldWidths.gperf"
                                                        { "Atilde", 600 },
#line 148 "poppler/CourierBoldWidths.gperf"
                                                        { "Emacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 257 "poppler/CourierBoldWidths.gperf"
                                                        { "aring", 600 },
#line 261 "poppler/CourierBoldWidths.gperf"
                                                        { "Iacute", 600 },
#line 183 "poppler/CourierBoldWidths.gperf"
                                                        { "umacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 221 "poppler/CourierBoldWidths.gperf"
                                                        { "zcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 133 "poppler/CourierBoldWidths.gperf"
                                                        { "Scaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 248 "poppler/CourierBoldWidths.gperf"
                                                        { "ocircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 189 "poppler/CourierBoldWidths.gperf"
                                                        { "idieresis", 600 },
                                                        { "", 0 },
#line 157 "poppler/CourierBoldWidths.gperf"
                                                        { "quotesingle", 600 },
#line 277 "poppler/CourierBoldWidths.gperf"
                                                        { "quotedblbase", 600 },
                                                        { "", 0 },
#line 268 "poppler/CourierBoldWidths.gperf"
                                                        { "quotesinglbase", 600 },
                                                        { "", 0 },
#line 107 "poppler/CourierBoldWidths.gperf"
                                                        { "p", 600 },
#line 132 "poppler/CourierBoldWidths.gperf"
                                                        { "greaterequal", 600 },
                                                        { "", 0 },
#line 326 "poppler/CourierBoldWidths.gperf"
                                                        { "quoteleft", 600 },
#line 179 "poppler/CourierBoldWidths.gperf"
                                                        { "quoteright", 600 },
                                                        { "", 0 },
#line 154 "poppler/CourierBoldWidths.gperf"
                                                        { "quotedblleft", 600 },
#line 304 "poppler/CourierBoldWidths.gperf"
                                                        { "quotedblright", 600 },
#line 169 "poppler/CourierBoldWidths.gperf"
                                                        { "Edieresis", 600 },
                                                        { "", 0 },
#line 128 "poppler/CourierBoldWidths.gperf"
                                                        { "Nacute", 600 },
#line 131 "poppler/CourierBoldWidths.gperf"
                                                        { "mu", 600 },
                                                        { "", 0 },
#line 198 "poppler/CourierBoldWidths.gperf"
                                                        { "udieresis", 600 },
                                                        { "", 0 },
#line 270 "poppler/CourierBoldWidths.gperf"
                                                        { "Yacute", 600 },
#line 253 "poppler/CourierBoldWidths.gperf"
                                                        { "eogonek", 600 },
#line 80 "poppler/CourierBoldWidths.gperf"
                                                        { "question", 600 },
                                                        { "", 0 },
#line 313 "poppler/CourierBoldWidths.gperf"
                                                        { "breve", 600 },
#line 77 "poppler/CourierBoldWidths.gperf"
                                                        { "V", 600 },
#line 39 "poppler/CourierBoldWidths.gperf"
                                                        { "questiondown", 600 },
                                                        { "", 0 },
#line 266 "poppler/CourierBoldWidths.gperf"
                                                        { "plus", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 149 "poppler/CourierBoldWidths.gperf"
                                                        { "ellipsis", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 319 "poppler/CourierBoldWidths.gperf"
                                                        { "exclam", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 219 "poppler/CourierBoldWidths.gperf"
                                                        { "braceleft", 600 },
#line 303 "poppler/CourierBoldWidths.gperf"
                                                        { "braceright", 600 },
#line 156 "poppler/CourierBoldWidths.gperf"
                                                        { "hyphen", 600 },
#line 48 "poppler/CourierBoldWidths.gperf"
                                                        { "aogonek", 600 },
#line 314 "poppler/CourierBoldWidths.gperf"
                                                        { "bar", 600 },
                                                        { "", 0 },
#line 311 "poppler/CourierBoldWidths.gperf"
                                                        { "zdotaccent", 600 },
#line 153 "poppler/CourierBoldWidths.gperf"
                                                        { "lslash", 600 },
#line 86 "poppler/CourierBoldWidths.gperf"
                                                        { "Gcommaaccent", 600 },
#line 309 "poppler/CourierBoldWidths.gperf"
                                                        { "currency", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 75 "poppler/CourierBoldWidths.gperf"
                                                        { "U", 600 },
#line 27 "poppler/CourierBoldWidths.gperf"
                                                        { "onehalf", 600 },
#line 109 "poppler/CourierBoldWidths.gperf"
                                                        { "uhungarumlaut", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 147 "poppler/CourierBoldWidths.gperf"
                                                        { "Otilde", 600 },
                                                        { "", 0 },
#line 287 "poppler/CourierBoldWidths.gperf"
                                                        { "guillemotleft", 600 },
#line 202 "poppler/CourierBoldWidths.gperf"
                                                        { "guillemotright", 600 },
                                                        { "", 0 },
#line 247 "poppler/CourierBoldWidths.gperf"
                                                        { "Lacute", 600 },
#line 163 "poppler/CourierBoldWidths.gperf"
                                                        { "Umacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 18 "poppler/CourierBoldWidths.gperf"
                                                        { "Zacute", 600 },
                                                        { "", 0 },
#line 295 "poppler/CourierBoldWidths.gperf"
                                                        { "notequal", 600 },
#line 143 "poppler/CourierBoldWidths.gperf"
                                                        { "trademark", 600 },
                                                        { "", 0 },
#line 265 "poppler/CourierBoldWidths.gperf"
                                                        { "Ncaron", 600 },
#line 200 "poppler/CourierBoldWidths.gperf"
                                                        { "Scommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 181 "poppler/CourierBoldWidths.gperf"
                                                        { "perthousand", 600 },
                                                        { "", 0 },
#line 240 "poppler/CourierBoldWidths.gperf"
                                                        { "six", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 302 "poppler/CourierBoldWidths.gperf"
                                                        { "icircumflex", 600 },
                                                        { "", 0 },
#line 214 "poppler/CourierBoldWidths.gperf"
                                                        { "Scedilla", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 93 "poppler/CourierBoldWidths.gperf"
                                                        { "bullet", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 108 "poppler/CourierBoldWidths.gperf"
                                                        { "q", 600 },
#line 289 "poppler/CourierBoldWidths.gperf"
                                                        { "Amacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 127 "poppler/CourierBoldWidths.gperf"
                                                        { "Idotaccent", 600 },
#line 141 "poppler/CourierBoldWidths.gperf"
                                                        { "Ecircumflex", 600 },
                                                        { "", 0 },
#line 315 "poppler/CourierBoldWidths.gperf"
                                                        { "fraction", 600 },
#line 180 "poppler/CourierBoldWidths.gperf"
                                                        { "Udieresis", 600 },
                                                        { "", 0 },
#line 176 "poppler/CourierBoldWidths.gperf"
                                                        { "ucircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 197 "poppler/CourierBoldWidths.gperf"
                                                        { "five", 600 },
                                                        { "", 0 },
#line 14 "poppler/CourierBoldWidths.gperf"
                                                        { "Ntilde", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 267 "poppler/CourierBoldWidths.gperf"
                                                        { "uring", 600 },
#line 45 "poppler/CourierBoldWidths.gperf"
                                                        { "A", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 84 "poppler/CourierBoldWidths.gperf"
                                                        { "four", 600 },
                                                        { "", 0 },
#line 187 "poppler/CourierBoldWidths.gperf"
                                                        { "egrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 241 "poppler/CourierBoldWidths.gperf"
                                                        { "paragraph", 600 },
                                                        { "", 0 },
#line 29 "poppler/CourierBoldWidths.gperf"
                                                        { "Lcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 325 "poppler/CourierBoldWidths.gperf"
                                                        { "brokenbar", 600 },
                                                        { "", 0 },
#line 199 "poppler/CourierBoldWidths.gperf"
                                                        { "Zcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 165 "poppler/CourierBoldWidths.gperf"
                                                        { "Adieresis", 600 },
                                                        { "", 0 },
#line 122 "poppler/CourierBoldWidths.gperf"
                                                        { "y", 600 },
#line 16 "poppler/CourierBoldWidths.gperf"
                                                        { "kcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 76 "poppler/CourierBoldWidths.gperf"
                                                        { "agrave", 600 },
                                                        { "", 0 },
#line 69 "poppler/CourierBoldWidths.gperf"
                                                        { "Uhungarumlaut", 600 },
#line 204 "poppler/CourierBoldWidths.gperf"
                                                        { "ydieresis", 600 },
#line 168 "poppler/CourierBoldWidths.gperf"
                                                        { "slash", 600 },
#line 229 "poppler/CourierBoldWidths.gperf"
                                                        { "ogonek", 600 },
#line 151 "poppler/CourierBoldWidths.gperf"
                                                        { "AE", 600 },
#line 192 "poppler/CourierBoldWidths.gperf"
                                                        { "asterisk", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 111 "poppler/CourierBoldWidths.gperf"
                                                        { "twosuperior", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 53 "poppler/CourierBoldWidths.gperf"
                                                        { "G", 600 },
#line 58 "poppler/CourierBoldWidths.gperf"
                                                        { "iogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 17 "poppler/CourierBoldWidths.gperf"
                                                        { "Ncommaaccent", 600 },
                                                        { "", 0 },
#line 21 "poppler/CourierBoldWidths.gperf"
                                                        { "plusminus", 600 },
                                                        { "", 0 },
#line 230 "poppler/CourierBoldWidths.gperf"
                                                        { "ograve", 600 },
                                                        { "", 0 },
#line 129 "poppler/CourierBoldWidths.gperf"
                                                        { "quotedbl", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 233 "poppler/CourierBoldWidths.gperf"
                                                        { "Eogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 120 "poppler/CourierBoldWidths.gperf"
                                                        { "w", 600 },
#line 259 "poppler/CourierBoldWidths.gperf"
                                                        { "uogonek", 600 },
                                                        { "", 0 },
#line 59 "poppler/CourierBoldWidths.gperf"
                                                        { "backslash", 600 },
#line 81 "poppler/CourierBoldWidths.gperf"
                                                        { "equal", 600 },
                                                        { "", 0 },
#line 38 "poppler/CourierBoldWidths.gperf"
                                                        { "Omacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 121 "poppler/CourierBoldWidths.gperf"
                                                        { "x", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 298 "poppler/CourierBoldWidths.gperf"
                                                        { "Zdotaccent", 600 },
#line 152 "poppler/CourierBoldWidths.gperf"
                                                        { "Ucircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 68 "poppler/CourierBoldWidths.gperf"
                                                        { "Q", 600 },
#line 296 "poppler/CourierBoldWidths.gperf"
                                                        { "Imacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 250 "poppler/CourierBoldWidths.gperf"
                                                        { "Uring", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 123 "poppler/CourierBoldWidths.gperf"
                                                        { "z", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 22 "poppler/CourierBoldWidths.gperf"
                                                        { "circumflex", 600 },
                                                        { "", 0 },
#line 251 "poppler/CourierBoldWidths.gperf"
                                                        { "Lcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 73 "poppler/CourierBoldWidths.gperf"
                                                        { "S", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 175 "poppler/CourierBoldWidths.gperf"
                                                        { "Odieresis", 600 },
                                                        { "", 0 },
#line 284 "poppler/CourierBoldWidths.gperf"
                                                        { "Acircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 67 "poppler/CourierBoldWidths.gperf"
                                                        { "P", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 238 "poppler/CourierBoldWidths.gperf"
                                                        { "Aring", 600 },
#line 96 "poppler/CourierBoldWidths.gperf"
                                                        { "Oslash", 600 },
#line 114 "poppler/CourierBoldWidths.gperf"
                                                        { "OE", 600 },
                                                        { "", 0 },
#line 171 "poppler/CourierBoldWidths.gperf"
                                                        { "Idieresis", 600 },
                                                        { "", 0 },
#line 62 "poppler/CourierBoldWidths.gperf"
                                                        { "M", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 282 "poppler/CourierBoldWidths.gperf"
                                                        { "fi", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 56 "poppler/CourierBoldWidths.gperf"
                                                        { "J", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 263 "poppler/CourierBoldWidths.gperf"
                                                        { "igrave", 600 },
                                                        { "", 0 },
#line 255 "poppler/CourierBoldWidths.gperf"
                                                        { "Ohungarumlaut", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 243 "poppler/CourierBoldWidths.gperf"
                                                        { "Uogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 87 "poppler/CourierBoldWidths.gperf"
                                                        { "b", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 167 "poppler/CourierBoldWidths.gperf"
                                                        { "Egrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 182 "poppler/CourierBoldWidths.gperf"
                                                        { "Ydieresis", 600 },
                                                        { "", 0 },
#line 195 "poppler/CourierBoldWidths.gperf"
                                                        { "ugrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 211 "poppler/CourierBoldWidths.gperf"
                                                        { "multiply", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 66 "poppler/CourierBoldWidths.gperf"
                                                        { "O", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 31 "poppler/CourierBoldWidths.gperf"
                                                        { "Aogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 279 "poppler/CourierBoldWidths.gperf"
                                                        { "florin", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 226 "poppler/CourierBoldWidths.gperf"
                                                        { "Ocircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 283 "poppler/CourierBoldWidths.gperf"
                                                        { "fl", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 55 "poppler/CourierBoldWidths.gperf"
                                                        { "I", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 286 "poppler/CourierBoldWidths.gperf"
                                                        { "Icircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 54 "poppler/CourierBoldWidths.gperf"
                                                        { "H", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 134 "poppler/CourierBoldWidths.gperf"
                                                        { "Lslash", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 99 "poppler/CourierBoldWidths.gperf"
                                                        { "k", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 184 "poppler/CourierBoldWidths.gperf"
                                                        { "abreve", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 217 "poppler/CourierBoldWidths.gperf"
                                                        { "partialdiff", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 52 "poppler/CourierBoldWidths.gperf"
                                                        { "F", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 178 "poppler/CourierBoldWidths.gperf"
                                                        { "Ugrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 91 "poppler/CourierBoldWidths.gperf"
                                                        { "f", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 119 "poppler/CourierBoldWidths.gperf"
                                                        { "v", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 63 "poppler/CourierBoldWidths.gperf"
                                                        { "N", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 41 "poppler/CourierBoldWidths.gperf"
                                                        { "Agrave", 600 },
#line 34 "poppler/CourierBoldWidths.gperf"
                                                        { "Iogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 82 "poppler/CourierBoldWidths.gperf"
                                                        { "Y", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 46 "poppler/CourierBoldWidths.gperf"
                                                        { "B", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 177 "poppler/CourierBoldWidths.gperf"
                                                        { "bracketleft", 600 },
#line 260 "poppler/CourierBoldWidths.gperf"
                                                        { "bracketright", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 142 "poppler/CourierBoldWidths.gperf"
                                                        { "gbreve", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 215 "poppler/CourierBoldWidths.gperf"
                                                        { "Ograve", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 60 "poppler/CourierBoldWidths.gperf"
                                                        { "L", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 246 "poppler/CourierBoldWidths.gperf"
                                                        { "Igrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 83 "poppler/CourierBoldWidths.gperf"
                                                        { "Z", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 162 "poppler/CourierBoldWidths.gperf"
                                                        { "Abreve", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 124 "poppler/CourierBoldWidths.gperf"
                                                        { "Gbreve", 600 } };

    if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH) {
        register unsigned int key = hash(str, len);

        if (key <= MAX_HASH_VALUE) {
            register const char *s = wordlist[key].name;

            if (*str == *s && !strcmp(str + 1, s + 1))
                return &wordlist[key];
        }
    }
    return 0;
}
#line 330 "poppler/CourierBoldWidths.gperf"
