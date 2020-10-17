/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf poppler/CourierBoldObliqueWidths.gperf  */
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

#line 1 "poppler/CourierBoldObliqueWidths.gperf"

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

const struct BuiltinFontWidth *CourierBoldObliqueWidthsLookup(register const char *str, register size_t len)
{
    static const struct BuiltinFontWidth wordlist[] = { { "", 0 },
#line 90 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "e", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 70 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "R", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 57 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "K", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 49 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "D", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 115 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "t", 600 },
#line 191 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ae", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 102 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "n", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 207 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "eacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 216 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Racute", 600 },
                                                        { "", 0 },
#line 85 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "a", 600 },
#line 206 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "at", 600 },
                                                        { "", 0 },
#line 308 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "cent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 161 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "oe", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 145 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "nacute", 600 },
                                                        { "", 0 },
#line 254 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Delta", 600 },
                                                        { "", 0 },
#line 273 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "acute", 600 },
#line 112 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "aacute", 600 },
#line 47 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "C", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 88 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "c", 600 },
                                                        { "", 0 },
#line 173 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "one", 600 },
#line 285 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Cacute", 600 },
                                                        { "", 0 },
#line 300 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "cacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 98 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "j", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 210 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Dcroat", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 249 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "oacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 72 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "caron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 104 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "o", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 317 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ecaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 321 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Rcaron", 600 },
#line 290 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "seven", 600 },
#line 306 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "sacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 224 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Dcaron", 600 },
                                                        { "", 0 },
#line 252 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "tcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 26 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "colon", 600 },
#line 278 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ncaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 125 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "commaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 135 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "semicolon", 600 },
#line 19 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "comma", 600 },
#line 235 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "degree", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 118 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Ccaron", 600 },
                                                        { "", 0 },
#line 140 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ccaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 113 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "s", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 231 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "racute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 79 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "X", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 30 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ntilde", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 205 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "tilde", 600 },
#line 324 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "atilde", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 196 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "nine", 600 },
#line 24 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "edotaccent", 600 },
#line 105 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ordfeminine", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 158 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "eight", 600 },
#line 150 "poppler/CourierBoldObliqueWidths.gperf"
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
#line 276 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "iacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 170 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "otilde", 600 },
#line 292 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ordmasculine", 600 },
#line 213 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "eth", 600 },
                                                        { "", 0 },
#line 42 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "three", 600 },
#line 225 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "dcroat", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 281 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Rcommaaccent", 600 },
#line 185 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Eacute", 600 },
#line 322 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Kcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 218 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "uacute", 600 },
#line 103 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "tcommaaccent", 600 },
                                                        { "", 0 },
#line 166 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "copyright", 600 },
#line 43 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "numbersign", 600 },
#line 15 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "rcaron", 600 },
#line 32 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ncommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 110 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "r", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 264 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "lacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 23 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "dotaccent", 600 },
#line 234 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "thorn", 600 },
#line 242 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "dcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 146 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "macron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 203 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Ccedilla", 600 },
#line 274 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "section", 600 },
#line 223 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ccedilla", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 20 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "cedilla", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 25 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "asciitilde", 600 },
#line 89 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "d", 600 },
#line 239 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "percent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 288 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "germandbls", 600 },
                                                        { "", 0 },
#line 138 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "lozenge", 600 },
                                                        { "", 0 },
#line 316 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "less", 600 },
                                                        { "", 0 },
#line 97 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "dagger", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 258 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "grave", 600 },
#line 301 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Ecaron", 600 },
#line 222 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "scommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 160 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "endash", 600 },
#line 174 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "emacron", 600 },
#line 201 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "threequarters", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 232 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Tcaron", 600 },
                                                        { "", 0 },
#line 228 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "scedilla", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 101 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "m", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 245 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "summation", 600 },
#line 310 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "logicalnot", 600 },
#line 44 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "lcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 172 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "parenleft", 600 },
#line 139 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "parenright", 600 },
#line 95 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "i", 600 },
#line 305 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "amacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 194 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Uacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 208 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "underscore", 600 },
#line 92 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "g", 600 },
#line 297 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "rcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 37 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "space", 600 },
#line 28 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "dollar", 600 },
                                                        { "", 0 },
#line 272 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "threesuperior", 600 },
#line 188 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "edieresis", 600 },
#line 236 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "registered", 600 },
#line 78 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "W", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 64 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "omacron", 600 },
#line 36 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "yen", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 50 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "E", 600 },
                                                        { "", 0 },
#line 293 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "dotlessi", 600 },
                                                        { "", 0 },
#line 327 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Edotaccent", 600 },
#line 71 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Aacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 186 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "adieresis", 600 },
                                                        { "", 0 },
#line 117 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "u", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 144 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "daggerdbl", 600 },
                                                        { "", 0 },
#line 280 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "yacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 74 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "T", 600 },
#line 130 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "gcommaaccent", 600 },
#line 275 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "dieresis", 600 },
                                                        { "", 0 },
#line 51 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "onequarter", 600 },
#line 328 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "onesuperior", 600 },
#line 237 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "radical", 600 },
#line 190 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Eth", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 94 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "h", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 193 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "odieresis", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 100 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "l", 600 },
#line 65 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Tcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 136 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "oslash", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 137 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "lessequal", 600 },
#line 159 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "exclamdown", 600 },
#line 35 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "zacute", 600 },
#line 269 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "lcommaaccent", 600 },
                                                        { "", 0 },
#line 209 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Euro", 600 },
                                                        { "", 0 },
#line 291 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Sacute", 600 },
#line 323 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "greater", 600 },
#line 244 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "two", 600 },
                                                        { "", 0 },
#line 220 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Thorn", 600 },
#line 256 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "asciicircum", 600 },
#line 126 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "hungarumlaut", 600 },
                                                        { "", 0 },
#line 212 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "zero", 600 },
                                                        { "", 0 },
#line 40 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "emdash", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 116 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "divide", 600 },
                                                        { "", 0 },
#line 271 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ohungarumlaut", 600 },
#line 262 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ampersand", 600 },
                                                        { "", 0 },
#line 164 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ecircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 106 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ring", 600 },
                                                        { "", 0 },
#line 320 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "period", 600 },
                                                        { "", 0 },
#line 318 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "guilsinglleft", 600 },
#line 155 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "guilsinglright", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 307 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "imacron", 600 },
                                                        { "", 0 },
#line 61 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "periodcentered", 600 },
                                                        { "", 0 },
#line 227 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Oacute", 600 },
                                                        { "", 0 },
#line 294 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "sterling", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 299 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "acircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 33 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "minus", 600 },
#line 312 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Atilde", 600 },
#line 148 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Emacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 257 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "aring", 600 },
#line 261 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Iacute", 600 },
#line 183 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "umacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 221 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "zcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 133 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Scaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 248 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ocircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 189 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "idieresis", 600 },
                                                        { "", 0 },
#line 157 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "quotesingle", 600 },
#line 277 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "quotedblbase", 600 },
                                                        { "", 0 },
#line 268 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "quotesinglbase", 600 },
                                                        { "", 0 },
#line 107 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "p", 600 },
#line 132 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "greaterequal", 600 },
                                                        { "", 0 },
#line 326 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "quoteleft", 600 },
#line 179 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "quoteright", 600 },
                                                        { "", 0 },
#line 154 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "quotedblleft", 600 },
#line 304 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "quotedblright", 600 },
#line 169 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Edieresis", 600 },
                                                        { "", 0 },
#line 128 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Nacute", 600 },
#line 131 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "mu", 600 },
                                                        { "", 0 },
#line 198 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "udieresis", 600 },
                                                        { "", 0 },
#line 270 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Yacute", 600 },
#line 253 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "eogonek", 600 },
#line 80 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "question", 600 },
                                                        { "", 0 },
#line 313 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "breve", 600 },
#line 77 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "V", 600 },
#line 39 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "questiondown", 600 },
                                                        { "", 0 },
#line 266 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "plus", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 149 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ellipsis", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 319 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "exclam", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 219 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "braceleft", 600 },
#line 303 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "braceright", 600 },
#line 156 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "hyphen", 600 },
#line 48 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "aogonek", 600 },
#line 314 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "bar", 600 },
                                                        { "", 0 },
#line 311 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "zdotaccent", 600 },
#line 153 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "lslash", 600 },
#line 86 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Gcommaaccent", 600 },
#line 309 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "currency", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 75 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "U", 600 },
#line 27 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "onehalf", 600 },
#line 109 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "uhungarumlaut", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 147 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Otilde", 600 },
                                                        { "", 0 },
#line 287 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "guillemotleft", 600 },
#line 202 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "guillemotright", 600 },
                                                        { "", 0 },
#line 247 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Lacute", 600 },
#line 163 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Umacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 18 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Zacute", 600 },
                                                        { "", 0 },
#line 295 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "notequal", 600 },
#line 143 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "trademark", 600 },
                                                        { "", 0 },
#line 265 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Ncaron", 600 },
#line 200 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Scommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 181 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "perthousand", 600 },
                                                        { "", 0 },
#line 240 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "six", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 302 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "icircumflex", 600 },
                                                        { "", 0 },
#line 214 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Scedilla", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 93 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "bullet", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 108 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "q", 600 },
#line 289 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Amacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 127 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Idotaccent", 600 },
#line 141 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Ecircumflex", 600 },
                                                        { "", 0 },
#line 315 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "fraction", 600 },
#line 180 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Udieresis", 600 },
                                                        { "", 0 },
#line 176 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ucircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 197 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "five", 600 },
                                                        { "", 0 },
#line 14 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Ntilde", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 267 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "uring", 600 },
#line 45 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "A", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 84 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "four", 600 },
                                                        { "", 0 },
#line 187 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "egrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 241 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "paragraph", 600 },
                                                        { "", 0 },
#line 29 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Lcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 325 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "brokenbar", 600 },
                                                        { "", 0 },
#line 199 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Zcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 165 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Adieresis", 600 },
                                                        { "", 0 },
#line 122 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "y", 600 },
#line 16 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "kcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 76 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "agrave", 600 },
                                                        { "", 0 },
#line 69 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Uhungarumlaut", 600 },
#line 204 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ydieresis", 600 },
#line 168 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "slash", 600 },
#line 229 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ogonek", 600 },
#line 151 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "AE", 600 },
#line 192 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "asterisk", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 111 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "twosuperior", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 53 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "G", 600 },
#line 58 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "iogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 17 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Ncommaaccent", 600 },
                                                        { "", 0 },
#line 21 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "plusminus", 600 },
                                                        { "", 0 },
#line 230 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ograve", 600 },
                                                        { "", 0 },
#line 129 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "quotedbl", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 233 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Eogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 120 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "w", 600 },
#line 259 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "uogonek", 600 },
                                                        { "", 0 },
#line 59 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "backslash", 600 },
#line 81 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "equal", 600 },
                                                        { "", 0 },
#line 38 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Omacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 121 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "x", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 298 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Zdotaccent", 600 },
#line 152 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Ucircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 68 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Q", 600 },
#line 296 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Imacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 250 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Uring", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 123 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "z", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 22 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "circumflex", 600 },
                                                        { "", 0 },
#line 251 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Lcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 73 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "S", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 175 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Odieresis", 600 },
                                                        { "", 0 },
#line 284 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Acircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 67 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "P", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 238 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Aring", 600 },
#line 96 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Oslash", 600 },
#line 114 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "OE", 600 },
                                                        { "", 0 },
#line 171 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Idieresis", 600 },
                                                        { "", 0 },
#line 62 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "M", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 282 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "fi", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 56 "poppler/CourierBoldObliqueWidths.gperf"
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
#line 263 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "igrave", 600 },
                                                        { "", 0 },
#line 255 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Ohungarumlaut", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 243 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Uogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 87 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "b", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 167 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Egrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 182 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Ydieresis", 600 },
                                                        { "", 0 },
#line 195 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "ugrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 211 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "multiply", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 66 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "O", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 31 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Aogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 279 "poppler/CourierBoldObliqueWidths.gperf"
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
#line 226 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Ocircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 283 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "fl", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 55 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "I", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 286 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Icircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 54 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "H", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 134 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Lslash", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 99 "poppler/CourierBoldObliqueWidths.gperf"
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
#line 184 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "abreve", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 217 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "partialdiff", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 52 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "F", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 178 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Ugrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 91 "poppler/CourierBoldObliqueWidths.gperf"
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
#line 119 "poppler/CourierBoldObliqueWidths.gperf"
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
#line 63 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "N", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 41 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Agrave", 600 },
#line 34 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Iogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 82 "poppler/CourierBoldObliqueWidths.gperf"
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
#line 46 "poppler/CourierBoldObliqueWidths.gperf"
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
#line 177 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "bracketleft", 600 },
#line 260 "poppler/CourierBoldObliqueWidths.gperf"
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
#line 142 "poppler/CourierBoldObliqueWidths.gperf"
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
#line 215 "poppler/CourierBoldObliqueWidths.gperf"
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
#line 60 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "L", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 246 "poppler/CourierBoldObliqueWidths.gperf"
                                                        { "Igrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 83 "poppler/CourierBoldObliqueWidths.gperf"
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
#line 162 "poppler/CourierBoldObliqueWidths.gperf"
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
#line 124 "poppler/CourierBoldObliqueWidths.gperf"
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
#line 330 "poppler/CourierBoldObliqueWidths.gperf"
