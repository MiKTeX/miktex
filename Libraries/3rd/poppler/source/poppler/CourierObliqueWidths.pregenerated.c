/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf poppler/CourierObliqueWidths.gperf  */
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

#line 1 "poppler/CourierObliqueWidths.gperf"

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

const struct BuiltinFontWidth *CourierObliqueWidthsLookup(register const char *str, register size_t len)
{
    static const struct BuiltinFontWidth wordlist[] = { { "", 0 },
#line 90 "poppler/CourierObliqueWidths.gperf"
                                                        { "e", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 70 "poppler/CourierObliqueWidths.gperf"
                                                        { "R", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 57 "poppler/CourierObliqueWidths.gperf"
                                                        { "K", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 49 "poppler/CourierObliqueWidths.gperf"
                                                        { "D", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 115 "poppler/CourierObliqueWidths.gperf"
                                                        { "t", 600 },
#line 191 "poppler/CourierObliqueWidths.gperf"
                                                        { "ae", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 102 "poppler/CourierObliqueWidths.gperf"
                                                        { "n", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 207 "poppler/CourierObliqueWidths.gperf"
                                                        { "eacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 216 "poppler/CourierObliqueWidths.gperf"
                                                        { "Racute", 600 },
                                                        { "", 0 },
#line 85 "poppler/CourierObliqueWidths.gperf"
                                                        { "a", 600 },
#line 206 "poppler/CourierObliqueWidths.gperf"
                                                        { "at", 600 },
                                                        { "", 0 },
#line 308 "poppler/CourierObliqueWidths.gperf"
                                                        { "cent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 161 "poppler/CourierObliqueWidths.gperf"
                                                        { "oe", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 145 "poppler/CourierObliqueWidths.gperf"
                                                        { "nacute", 600 },
                                                        { "", 0 },
#line 254 "poppler/CourierObliqueWidths.gperf"
                                                        { "Delta", 600 },
                                                        { "", 0 },
#line 273 "poppler/CourierObliqueWidths.gperf"
                                                        { "acute", 600 },
#line 112 "poppler/CourierObliqueWidths.gperf"
                                                        { "aacute", 600 },
#line 47 "poppler/CourierObliqueWidths.gperf"
                                                        { "C", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 88 "poppler/CourierObliqueWidths.gperf"
                                                        { "c", 600 },
                                                        { "", 0 },
#line 173 "poppler/CourierObliqueWidths.gperf"
                                                        { "one", 600 },
#line 285 "poppler/CourierObliqueWidths.gperf"
                                                        { "Cacute", 600 },
                                                        { "", 0 },
#line 300 "poppler/CourierObliqueWidths.gperf"
                                                        { "cacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 98 "poppler/CourierObliqueWidths.gperf"
                                                        { "j", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 210 "poppler/CourierObliqueWidths.gperf"
                                                        { "Dcroat", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 249 "poppler/CourierObliqueWidths.gperf"
                                                        { "oacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 72 "poppler/CourierObliqueWidths.gperf"
                                                        { "caron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 104 "poppler/CourierObliqueWidths.gperf"
                                                        { "o", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 317 "poppler/CourierObliqueWidths.gperf"
                                                        { "ecaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 321 "poppler/CourierObliqueWidths.gperf"
                                                        { "Rcaron", 600 },
#line 290 "poppler/CourierObliqueWidths.gperf"
                                                        { "seven", 600 },
#line 306 "poppler/CourierObliqueWidths.gperf"
                                                        { "sacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 224 "poppler/CourierObliqueWidths.gperf"
                                                        { "Dcaron", 600 },
                                                        { "", 0 },
#line 252 "poppler/CourierObliqueWidths.gperf"
                                                        { "tcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 26 "poppler/CourierObliqueWidths.gperf"
                                                        { "colon", 600 },
#line 278 "poppler/CourierObliqueWidths.gperf"
                                                        { "ncaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 125 "poppler/CourierObliqueWidths.gperf"
                                                        { "commaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 135 "poppler/CourierObliqueWidths.gperf"
                                                        { "semicolon", 600 },
#line 19 "poppler/CourierObliqueWidths.gperf"
                                                        { "comma", 600 },
#line 235 "poppler/CourierObliqueWidths.gperf"
                                                        { "degree", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 118 "poppler/CourierObliqueWidths.gperf"
                                                        { "Ccaron", 600 },
                                                        { "", 0 },
#line 140 "poppler/CourierObliqueWidths.gperf"
                                                        { "ccaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 113 "poppler/CourierObliqueWidths.gperf"
                                                        { "s", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 231 "poppler/CourierObliqueWidths.gperf"
                                                        { "racute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 79 "poppler/CourierObliqueWidths.gperf"
                                                        { "X", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 30 "poppler/CourierObliqueWidths.gperf"
                                                        { "ntilde", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 205 "poppler/CourierObliqueWidths.gperf"
                                                        { "tilde", 600 },
#line 324 "poppler/CourierObliqueWidths.gperf"
                                                        { "atilde", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 196 "poppler/CourierObliqueWidths.gperf"
                                                        { "nine", 600 },
#line 24 "poppler/CourierObliqueWidths.gperf"
                                                        { "edotaccent", 600 },
#line 105 "poppler/CourierObliqueWidths.gperf"
                                                        { "ordfeminine", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 158 "poppler/CourierObliqueWidths.gperf"
                                                        { "eight", 600 },
#line 150 "poppler/CourierObliqueWidths.gperf"
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
#line 276 "poppler/CourierObliqueWidths.gperf"
                                                        { "iacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 170 "poppler/CourierObliqueWidths.gperf"
                                                        { "otilde", 600 },
#line 292 "poppler/CourierObliqueWidths.gperf"
                                                        { "ordmasculine", 600 },
#line 213 "poppler/CourierObliqueWidths.gperf"
                                                        { "eth", 600 },
                                                        { "", 0 },
#line 42 "poppler/CourierObliqueWidths.gperf"
                                                        { "three", 600 },
#line 225 "poppler/CourierObliqueWidths.gperf"
                                                        { "dcroat", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 281 "poppler/CourierObliqueWidths.gperf"
                                                        { "Rcommaaccent", 600 },
#line 185 "poppler/CourierObliqueWidths.gperf"
                                                        { "Eacute", 600 },
#line 322 "poppler/CourierObliqueWidths.gperf"
                                                        { "Kcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 218 "poppler/CourierObliqueWidths.gperf"
                                                        { "uacute", 600 },
#line 103 "poppler/CourierObliqueWidths.gperf"
                                                        { "tcommaaccent", 600 },
                                                        { "", 0 },
#line 166 "poppler/CourierObliqueWidths.gperf"
                                                        { "copyright", 600 },
#line 43 "poppler/CourierObliqueWidths.gperf"
                                                        { "numbersign", 600 },
#line 15 "poppler/CourierObliqueWidths.gperf"
                                                        { "rcaron", 600 },
#line 32 "poppler/CourierObliqueWidths.gperf"
                                                        { "ncommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 110 "poppler/CourierObliqueWidths.gperf"
                                                        { "r", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 264 "poppler/CourierObliqueWidths.gperf"
                                                        { "lacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 23 "poppler/CourierObliqueWidths.gperf"
                                                        { "dotaccent", 600 },
#line 234 "poppler/CourierObliqueWidths.gperf"
                                                        { "thorn", 600 },
#line 242 "poppler/CourierObliqueWidths.gperf"
                                                        { "dcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 146 "poppler/CourierObliqueWidths.gperf"
                                                        { "macron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 203 "poppler/CourierObliqueWidths.gperf"
                                                        { "Ccedilla", 600 },
#line 274 "poppler/CourierObliqueWidths.gperf"
                                                        { "section", 600 },
#line 223 "poppler/CourierObliqueWidths.gperf"
                                                        { "ccedilla", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 20 "poppler/CourierObliqueWidths.gperf"
                                                        { "cedilla", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 25 "poppler/CourierObliqueWidths.gperf"
                                                        { "asciitilde", 600 },
#line 89 "poppler/CourierObliqueWidths.gperf"
                                                        { "d", 600 },
#line 239 "poppler/CourierObliqueWidths.gperf"
                                                        { "percent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 288 "poppler/CourierObliqueWidths.gperf"
                                                        { "germandbls", 600 },
                                                        { "", 0 },
#line 138 "poppler/CourierObliqueWidths.gperf"
                                                        { "lozenge", 600 },
                                                        { "", 0 },
#line 316 "poppler/CourierObliqueWidths.gperf"
                                                        { "less", 600 },
                                                        { "", 0 },
#line 97 "poppler/CourierObliqueWidths.gperf"
                                                        { "dagger", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 258 "poppler/CourierObliqueWidths.gperf"
                                                        { "grave", 600 },
#line 301 "poppler/CourierObliqueWidths.gperf"
                                                        { "Ecaron", 600 },
#line 222 "poppler/CourierObliqueWidths.gperf"
                                                        { "scommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 160 "poppler/CourierObliqueWidths.gperf"
                                                        { "endash", 600 },
#line 174 "poppler/CourierObliqueWidths.gperf"
                                                        { "emacron", 600 },
#line 201 "poppler/CourierObliqueWidths.gperf"
                                                        { "threequarters", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 232 "poppler/CourierObliqueWidths.gperf"
                                                        { "Tcaron", 600 },
                                                        { "", 0 },
#line 228 "poppler/CourierObliqueWidths.gperf"
                                                        { "scedilla", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 101 "poppler/CourierObliqueWidths.gperf"
                                                        { "m", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 245 "poppler/CourierObliqueWidths.gperf"
                                                        { "summation", 600 },
#line 310 "poppler/CourierObliqueWidths.gperf"
                                                        { "logicalnot", 600 },
#line 44 "poppler/CourierObliqueWidths.gperf"
                                                        { "lcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 172 "poppler/CourierObliqueWidths.gperf"
                                                        { "parenleft", 600 },
#line 139 "poppler/CourierObliqueWidths.gperf"
                                                        { "parenright", 600 },
#line 95 "poppler/CourierObliqueWidths.gperf"
                                                        { "i", 600 },
#line 305 "poppler/CourierObliqueWidths.gperf"
                                                        { "amacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 194 "poppler/CourierObliqueWidths.gperf"
                                                        { "Uacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 208 "poppler/CourierObliqueWidths.gperf"
                                                        { "underscore", 600 },
#line 92 "poppler/CourierObliqueWidths.gperf"
                                                        { "g", 600 },
#line 297 "poppler/CourierObliqueWidths.gperf"
                                                        { "rcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 37 "poppler/CourierObliqueWidths.gperf"
                                                        { "space", 600 },
#line 28 "poppler/CourierObliqueWidths.gperf"
                                                        { "dollar", 600 },
                                                        { "", 0 },
#line 272 "poppler/CourierObliqueWidths.gperf"
                                                        { "threesuperior", 600 },
#line 188 "poppler/CourierObliqueWidths.gperf"
                                                        { "edieresis", 600 },
#line 236 "poppler/CourierObliqueWidths.gperf"
                                                        { "registered", 600 },
#line 78 "poppler/CourierObliqueWidths.gperf"
                                                        { "W", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 64 "poppler/CourierObliqueWidths.gperf"
                                                        { "omacron", 600 },
#line 36 "poppler/CourierObliqueWidths.gperf"
                                                        { "yen", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 50 "poppler/CourierObliqueWidths.gperf"
                                                        { "E", 600 },
                                                        { "", 0 },
#line 293 "poppler/CourierObliqueWidths.gperf"
                                                        { "dotlessi", 600 },
                                                        { "", 0 },
#line 327 "poppler/CourierObliqueWidths.gperf"
                                                        { "Edotaccent", 600 },
#line 71 "poppler/CourierObliqueWidths.gperf"
                                                        { "Aacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 186 "poppler/CourierObliqueWidths.gperf"
                                                        { "adieresis", 600 },
                                                        { "", 0 },
#line 117 "poppler/CourierObliqueWidths.gperf"
                                                        { "u", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 144 "poppler/CourierObliqueWidths.gperf"
                                                        { "daggerdbl", 600 },
                                                        { "", 0 },
#line 280 "poppler/CourierObliqueWidths.gperf"
                                                        { "yacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 74 "poppler/CourierObliqueWidths.gperf"
                                                        { "T", 600 },
#line 130 "poppler/CourierObliqueWidths.gperf"
                                                        { "gcommaaccent", 600 },
#line 275 "poppler/CourierObliqueWidths.gperf"
                                                        { "dieresis", 600 },
                                                        { "", 0 },
#line 51 "poppler/CourierObliqueWidths.gperf"
                                                        { "onequarter", 600 },
#line 328 "poppler/CourierObliqueWidths.gperf"
                                                        { "onesuperior", 600 },
#line 237 "poppler/CourierObliqueWidths.gperf"
                                                        { "radical", 600 },
#line 190 "poppler/CourierObliqueWidths.gperf"
                                                        { "Eth", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 94 "poppler/CourierObliqueWidths.gperf"
                                                        { "h", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 193 "poppler/CourierObliqueWidths.gperf"
                                                        { "odieresis", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 100 "poppler/CourierObliqueWidths.gperf"
                                                        { "l", 600 },
#line 65 "poppler/CourierObliqueWidths.gperf"
                                                        { "Tcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 136 "poppler/CourierObliqueWidths.gperf"
                                                        { "oslash", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 137 "poppler/CourierObliqueWidths.gperf"
                                                        { "lessequal", 600 },
#line 159 "poppler/CourierObliqueWidths.gperf"
                                                        { "exclamdown", 600 },
#line 35 "poppler/CourierObliqueWidths.gperf"
                                                        { "zacute", 600 },
#line 269 "poppler/CourierObliqueWidths.gperf"
                                                        { "lcommaaccent", 600 },
                                                        { "", 0 },
#line 209 "poppler/CourierObliqueWidths.gperf"
                                                        { "Euro", 600 },
                                                        { "", 0 },
#line 291 "poppler/CourierObliqueWidths.gperf"
                                                        { "Sacute", 600 },
#line 323 "poppler/CourierObliqueWidths.gperf"
                                                        { "greater", 600 },
#line 244 "poppler/CourierObliqueWidths.gperf"
                                                        { "two", 600 },
                                                        { "", 0 },
#line 220 "poppler/CourierObliqueWidths.gperf"
                                                        { "Thorn", 600 },
#line 256 "poppler/CourierObliqueWidths.gperf"
                                                        { "asciicircum", 600 },
#line 126 "poppler/CourierObliqueWidths.gperf"
                                                        { "hungarumlaut", 600 },
                                                        { "", 0 },
#line 212 "poppler/CourierObliqueWidths.gperf"
                                                        { "zero", 600 },
                                                        { "", 0 },
#line 40 "poppler/CourierObliqueWidths.gperf"
                                                        { "emdash", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 116 "poppler/CourierObliqueWidths.gperf"
                                                        { "divide", 600 },
                                                        { "", 0 },
#line 271 "poppler/CourierObliqueWidths.gperf"
                                                        { "ohungarumlaut", 600 },
#line 262 "poppler/CourierObliqueWidths.gperf"
                                                        { "ampersand", 600 },
                                                        { "", 0 },
#line 164 "poppler/CourierObliqueWidths.gperf"
                                                        { "ecircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 106 "poppler/CourierObliqueWidths.gperf"
                                                        { "ring", 600 },
                                                        { "", 0 },
#line 320 "poppler/CourierObliqueWidths.gperf"
                                                        { "period", 600 },
                                                        { "", 0 },
#line 318 "poppler/CourierObliqueWidths.gperf"
                                                        { "guilsinglleft", 600 },
#line 155 "poppler/CourierObliqueWidths.gperf"
                                                        { "guilsinglright", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 307 "poppler/CourierObliqueWidths.gperf"
                                                        { "imacron", 600 },
                                                        { "", 0 },
#line 61 "poppler/CourierObliqueWidths.gperf"
                                                        { "periodcentered", 600 },
                                                        { "", 0 },
#line 227 "poppler/CourierObliqueWidths.gperf"
                                                        { "Oacute", 600 },
                                                        { "", 0 },
#line 294 "poppler/CourierObliqueWidths.gperf"
                                                        { "sterling", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 299 "poppler/CourierObliqueWidths.gperf"
                                                        { "acircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 33 "poppler/CourierObliqueWidths.gperf"
                                                        { "minus", 600 },
#line 312 "poppler/CourierObliqueWidths.gperf"
                                                        { "Atilde", 600 },
#line 148 "poppler/CourierObliqueWidths.gperf"
                                                        { "Emacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 257 "poppler/CourierObliqueWidths.gperf"
                                                        { "aring", 600 },
#line 261 "poppler/CourierObliqueWidths.gperf"
                                                        { "Iacute", 600 },
#line 183 "poppler/CourierObliqueWidths.gperf"
                                                        { "umacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 221 "poppler/CourierObliqueWidths.gperf"
                                                        { "zcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 133 "poppler/CourierObliqueWidths.gperf"
                                                        { "Scaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 248 "poppler/CourierObliqueWidths.gperf"
                                                        { "ocircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 189 "poppler/CourierObliqueWidths.gperf"
                                                        { "idieresis", 600 },
                                                        { "", 0 },
#line 157 "poppler/CourierObliqueWidths.gperf"
                                                        { "quotesingle", 600 },
#line 277 "poppler/CourierObliqueWidths.gperf"
                                                        { "quotedblbase", 600 },
                                                        { "", 0 },
#line 268 "poppler/CourierObliqueWidths.gperf"
                                                        { "quotesinglbase", 600 },
                                                        { "", 0 },
#line 107 "poppler/CourierObliqueWidths.gperf"
                                                        { "p", 600 },
#line 132 "poppler/CourierObliqueWidths.gperf"
                                                        { "greaterequal", 600 },
                                                        { "", 0 },
#line 326 "poppler/CourierObliqueWidths.gperf"
                                                        { "quoteleft", 600 },
#line 179 "poppler/CourierObliqueWidths.gperf"
                                                        { "quoteright", 600 },
                                                        { "", 0 },
#line 154 "poppler/CourierObliqueWidths.gperf"
                                                        { "quotedblleft", 600 },
#line 304 "poppler/CourierObliqueWidths.gperf"
                                                        { "quotedblright", 600 },
#line 169 "poppler/CourierObliqueWidths.gperf"
                                                        { "Edieresis", 600 },
                                                        { "", 0 },
#line 128 "poppler/CourierObliqueWidths.gperf"
                                                        { "Nacute", 600 },
#line 131 "poppler/CourierObliqueWidths.gperf"
                                                        { "mu", 600 },
                                                        { "", 0 },
#line 198 "poppler/CourierObliqueWidths.gperf"
                                                        { "udieresis", 600 },
                                                        { "", 0 },
#line 270 "poppler/CourierObliqueWidths.gperf"
                                                        { "Yacute", 600 },
#line 253 "poppler/CourierObliqueWidths.gperf"
                                                        { "eogonek", 600 },
#line 80 "poppler/CourierObliqueWidths.gperf"
                                                        { "question", 600 },
                                                        { "", 0 },
#line 313 "poppler/CourierObliqueWidths.gperf"
                                                        { "breve", 600 },
#line 77 "poppler/CourierObliqueWidths.gperf"
                                                        { "V", 600 },
#line 39 "poppler/CourierObliqueWidths.gperf"
                                                        { "questiondown", 600 },
                                                        { "", 0 },
#line 266 "poppler/CourierObliqueWidths.gperf"
                                                        { "plus", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 149 "poppler/CourierObliqueWidths.gperf"
                                                        { "ellipsis", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 319 "poppler/CourierObliqueWidths.gperf"
                                                        { "exclam", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 219 "poppler/CourierObliqueWidths.gperf"
                                                        { "braceleft", 600 },
#line 303 "poppler/CourierObliqueWidths.gperf"
                                                        { "braceright", 600 },
#line 156 "poppler/CourierObliqueWidths.gperf"
                                                        { "hyphen", 600 },
#line 48 "poppler/CourierObliqueWidths.gperf"
                                                        { "aogonek", 600 },
#line 314 "poppler/CourierObliqueWidths.gperf"
                                                        { "bar", 600 },
                                                        { "", 0 },
#line 311 "poppler/CourierObliqueWidths.gperf"
                                                        { "zdotaccent", 600 },
#line 153 "poppler/CourierObliqueWidths.gperf"
                                                        { "lslash", 600 },
#line 86 "poppler/CourierObliqueWidths.gperf"
                                                        { "Gcommaaccent", 600 },
#line 309 "poppler/CourierObliqueWidths.gperf"
                                                        { "currency", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 75 "poppler/CourierObliqueWidths.gperf"
                                                        { "U", 600 },
#line 27 "poppler/CourierObliqueWidths.gperf"
                                                        { "onehalf", 600 },
#line 109 "poppler/CourierObliqueWidths.gperf"
                                                        { "uhungarumlaut", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 147 "poppler/CourierObliqueWidths.gperf"
                                                        { "Otilde", 600 },
                                                        { "", 0 },
#line 287 "poppler/CourierObliqueWidths.gperf"
                                                        { "guillemotleft", 600 },
#line 202 "poppler/CourierObliqueWidths.gperf"
                                                        { "guillemotright", 600 },
                                                        { "", 0 },
#line 247 "poppler/CourierObliqueWidths.gperf"
                                                        { "Lacute", 600 },
#line 163 "poppler/CourierObliqueWidths.gperf"
                                                        { "Umacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 18 "poppler/CourierObliqueWidths.gperf"
                                                        { "Zacute", 600 },
                                                        { "", 0 },
#line 295 "poppler/CourierObliqueWidths.gperf"
                                                        { "notequal", 600 },
#line 143 "poppler/CourierObliqueWidths.gperf"
                                                        { "trademark", 600 },
                                                        { "", 0 },
#line 265 "poppler/CourierObliqueWidths.gperf"
                                                        { "Ncaron", 600 },
#line 200 "poppler/CourierObliqueWidths.gperf"
                                                        { "Scommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 181 "poppler/CourierObliqueWidths.gperf"
                                                        { "perthousand", 600 },
                                                        { "", 0 },
#line 240 "poppler/CourierObliqueWidths.gperf"
                                                        { "six", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 302 "poppler/CourierObliqueWidths.gperf"
                                                        { "icircumflex", 600 },
                                                        { "", 0 },
#line 214 "poppler/CourierObliqueWidths.gperf"
                                                        { "Scedilla", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 93 "poppler/CourierObliqueWidths.gperf"
                                                        { "bullet", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 108 "poppler/CourierObliqueWidths.gperf"
                                                        { "q", 600 },
#line 289 "poppler/CourierObliqueWidths.gperf"
                                                        { "Amacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 127 "poppler/CourierObliqueWidths.gperf"
                                                        { "Idotaccent", 600 },
#line 141 "poppler/CourierObliqueWidths.gperf"
                                                        { "Ecircumflex", 600 },
                                                        { "", 0 },
#line 315 "poppler/CourierObliqueWidths.gperf"
                                                        { "fraction", 600 },
#line 180 "poppler/CourierObliqueWidths.gperf"
                                                        { "Udieresis", 600 },
                                                        { "", 0 },
#line 176 "poppler/CourierObliqueWidths.gperf"
                                                        { "ucircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 197 "poppler/CourierObliqueWidths.gperf"
                                                        { "five", 600 },
                                                        { "", 0 },
#line 14 "poppler/CourierObliqueWidths.gperf"
                                                        { "Ntilde", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 267 "poppler/CourierObliqueWidths.gperf"
                                                        { "uring", 600 },
#line 45 "poppler/CourierObliqueWidths.gperf"
                                                        { "A", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 84 "poppler/CourierObliqueWidths.gperf"
                                                        { "four", 600 },
                                                        { "", 0 },
#line 187 "poppler/CourierObliqueWidths.gperf"
                                                        { "egrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 241 "poppler/CourierObliqueWidths.gperf"
                                                        { "paragraph", 600 },
                                                        { "", 0 },
#line 29 "poppler/CourierObliqueWidths.gperf"
                                                        { "Lcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 325 "poppler/CourierObliqueWidths.gperf"
                                                        { "brokenbar", 600 },
                                                        { "", 0 },
#line 199 "poppler/CourierObliqueWidths.gperf"
                                                        { "Zcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 165 "poppler/CourierObliqueWidths.gperf"
                                                        { "Adieresis", 600 },
                                                        { "", 0 },
#line 122 "poppler/CourierObliqueWidths.gperf"
                                                        { "y", 600 },
#line 16 "poppler/CourierObliqueWidths.gperf"
                                                        { "kcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 76 "poppler/CourierObliqueWidths.gperf"
                                                        { "agrave", 600 },
                                                        { "", 0 },
#line 69 "poppler/CourierObliqueWidths.gperf"
                                                        { "Uhungarumlaut", 600 },
#line 204 "poppler/CourierObliqueWidths.gperf"
                                                        { "ydieresis", 600 },
#line 168 "poppler/CourierObliqueWidths.gperf"
                                                        { "slash", 600 },
#line 229 "poppler/CourierObliqueWidths.gperf"
                                                        { "ogonek", 600 },
#line 151 "poppler/CourierObliqueWidths.gperf"
                                                        { "AE", 600 },
#line 192 "poppler/CourierObliqueWidths.gperf"
                                                        { "asterisk", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 111 "poppler/CourierObliqueWidths.gperf"
                                                        { "twosuperior", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 53 "poppler/CourierObliqueWidths.gperf"
                                                        { "G", 600 },
#line 58 "poppler/CourierObliqueWidths.gperf"
                                                        { "iogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 17 "poppler/CourierObliqueWidths.gperf"
                                                        { "Ncommaaccent", 600 },
                                                        { "", 0 },
#line 21 "poppler/CourierObliqueWidths.gperf"
                                                        { "plusminus", 600 },
                                                        { "", 0 },
#line 230 "poppler/CourierObliqueWidths.gperf"
                                                        { "ograve", 600 },
                                                        { "", 0 },
#line 129 "poppler/CourierObliqueWidths.gperf"
                                                        { "quotedbl", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 233 "poppler/CourierObliqueWidths.gperf"
                                                        { "Eogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 120 "poppler/CourierObliqueWidths.gperf"
                                                        { "w", 600 },
#line 259 "poppler/CourierObliqueWidths.gperf"
                                                        { "uogonek", 600 },
                                                        { "", 0 },
#line 59 "poppler/CourierObliqueWidths.gperf"
                                                        { "backslash", 600 },
#line 81 "poppler/CourierObliqueWidths.gperf"
                                                        { "equal", 600 },
                                                        { "", 0 },
#line 38 "poppler/CourierObliqueWidths.gperf"
                                                        { "Omacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 121 "poppler/CourierObliqueWidths.gperf"
                                                        { "x", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 298 "poppler/CourierObliqueWidths.gperf"
                                                        { "Zdotaccent", 600 },
#line 152 "poppler/CourierObliqueWidths.gperf"
                                                        { "Ucircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 68 "poppler/CourierObliqueWidths.gperf"
                                                        { "Q", 600 },
#line 296 "poppler/CourierObliqueWidths.gperf"
                                                        { "Imacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 250 "poppler/CourierObliqueWidths.gperf"
                                                        { "Uring", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 123 "poppler/CourierObliqueWidths.gperf"
                                                        { "z", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 22 "poppler/CourierObliqueWidths.gperf"
                                                        { "circumflex", 600 },
                                                        { "", 0 },
#line 251 "poppler/CourierObliqueWidths.gperf"
                                                        { "Lcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 73 "poppler/CourierObliqueWidths.gperf"
                                                        { "S", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 175 "poppler/CourierObliqueWidths.gperf"
                                                        { "Odieresis", 600 },
                                                        { "", 0 },
#line 284 "poppler/CourierObliqueWidths.gperf"
                                                        { "Acircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 67 "poppler/CourierObliqueWidths.gperf"
                                                        { "P", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 238 "poppler/CourierObliqueWidths.gperf"
                                                        { "Aring", 600 },
#line 96 "poppler/CourierObliqueWidths.gperf"
                                                        { "Oslash", 600 },
#line 114 "poppler/CourierObliqueWidths.gperf"
                                                        { "OE", 600 },
                                                        { "", 0 },
#line 171 "poppler/CourierObliqueWidths.gperf"
                                                        { "Idieresis", 600 },
                                                        { "", 0 },
#line 62 "poppler/CourierObliqueWidths.gperf"
                                                        { "M", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 282 "poppler/CourierObliqueWidths.gperf"
                                                        { "fi", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 56 "poppler/CourierObliqueWidths.gperf"
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
#line 263 "poppler/CourierObliqueWidths.gperf"
                                                        { "igrave", 600 },
                                                        { "", 0 },
#line 255 "poppler/CourierObliqueWidths.gperf"
                                                        { "Ohungarumlaut", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 243 "poppler/CourierObliqueWidths.gperf"
                                                        { "Uogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 87 "poppler/CourierObliqueWidths.gperf"
                                                        { "b", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 167 "poppler/CourierObliqueWidths.gperf"
                                                        { "Egrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 182 "poppler/CourierObliqueWidths.gperf"
                                                        { "Ydieresis", 600 },
                                                        { "", 0 },
#line 195 "poppler/CourierObliqueWidths.gperf"
                                                        { "ugrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 211 "poppler/CourierObliqueWidths.gperf"
                                                        { "multiply", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 66 "poppler/CourierObliqueWidths.gperf"
                                                        { "O", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 31 "poppler/CourierObliqueWidths.gperf"
                                                        { "Aogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 279 "poppler/CourierObliqueWidths.gperf"
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
#line 226 "poppler/CourierObliqueWidths.gperf"
                                                        { "Ocircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 283 "poppler/CourierObliqueWidths.gperf"
                                                        { "fl", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 55 "poppler/CourierObliqueWidths.gperf"
                                                        { "I", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 286 "poppler/CourierObliqueWidths.gperf"
                                                        { "Icircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 54 "poppler/CourierObliqueWidths.gperf"
                                                        { "H", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 134 "poppler/CourierObliqueWidths.gperf"
                                                        { "Lslash", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 99 "poppler/CourierObliqueWidths.gperf"
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
#line 184 "poppler/CourierObliqueWidths.gperf"
                                                        { "abreve", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 217 "poppler/CourierObliqueWidths.gperf"
                                                        { "partialdiff", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 52 "poppler/CourierObliqueWidths.gperf"
                                                        { "F", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 178 "poppler/CourierObliqueWidths.gperf"
                                                        { "Ugrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 91 "poppler/CourierObliqueWidths.gperf"
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
#line 119 "poppler/CourierObliqueWidths.gperf"
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
#line 63 "poppler/CourierObliqueWidths.gperf"
                                                        { "N", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 41 "poppler/CourierObliqueWidths.gperf"
                                                        { "Agrave", 600 },
#line 34 "poppler/CourierObliqueWidths.gperf"
                                                        { "Iogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 82 "poppler/CourierObliqueWidths.gperf"
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
#line 46 "poppler/CourierObliqueWidths.gperf"
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
#line 177 "poppler/CourierObliqueWidths.gperf"
                                                        { "bracketleft", 600 },
#line 260 "poppler/CourierObliqueWidths.gperf"
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
#line 142 "poppler/CourierObliqueWidths.gperf"
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
#line 215 "poppler/CourierObliqueWidths.gperf"
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
#line 60 "poppler/CourierObliqueWidths.gperf"
                                                        { "L", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 246 "poppler/CourierObliqueWidths.gperf"
                                                        { "Igrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 83 "poppler/CourierObliqueWidths.gperf"
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
#line 162 "poppler/CourierObliqueWidths.gperf"
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
#line 124 "poppler/CourierObliqueWidths.gperf"
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
#line 330 "poppler/CourierObliqueWidths.gperf"
