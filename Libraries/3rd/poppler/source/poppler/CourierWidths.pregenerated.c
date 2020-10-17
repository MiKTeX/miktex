/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf poppler/CourierWidths.gperf  */
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

#line 1 "poppler/CourierWidths.gperf"

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

const struct BuiltinFontWidth *CourierWidthsLookup(register const char *str, register size_t len)
{
    static const struct BuiltinFontWidth wordlist[] = { { "", 0 },
#line 90 "poppler/CourierWidths.gperf"
                                                        { "e", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 70 "poppler/CourierWidths.gperf"
                                                        { "R", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 57 "poppler/CourierWidths.gperf"
                                                        { "K", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 49 "poppler/CourierWidths.gperf"
                                                        { "D", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 115 "poppler/CourierWidths.gperf"
                                                        { "t", 600 },
#line 191 "poppler/CourierWidths.gperf"
                                                        { "ae", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 102 "poppler/CourierWidths.gperf"
                                                        { "n", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 207 "poppler/CourierWidths.gperf"
                                                        { "eacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 215 "poppler/CourierWidths.gperf"
                                                        { "Racute", 600 },
                                                        { "", 0 },
#line 85 "poppler/CourierWidths.gperf"
                                                        { "a", 600 },
#line 206 "poppler/CourierWidths.gperf"
                                                        { "at", 600 },
                                                        { "", 0 },
#line 308 "poppler/CourierWidths.gperf"
                                                        { "cent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 161 "poppler/CourierWidths.gperf"
                                                        { "oe", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 145 "poppler/CourierWidths.gperf"
                                                        { "nacute", 600 },
                                                        { "", 0 },
#line 254 "poppler/CourierWidths.gperf"
                                                        { "Delta", 600 },
                                                        { "", 0 },
#line 273 "poppler/CourierWidths.gperf"
                                                        { "acute", 600 },
#line 112 "poppler/CourierWidths.gperf"
                                                        { "aacute", 600 },
#line 47 "poppler/CourierWidths.gperf"
                                                        { "C", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 88 "poppler/CourierWidths.gperf"
                                                        { "c", 600 },
                                                        { "", 0 },
#line 173 "poppler/CourierWidths.gperf"
                                                        { "one", 600 },
#line 285 "poppler/CourierWidths.gperf"
                                                        { "Cacute", 600 },
                                                        { "", 0 },
#line 300 "poppler/CourierWidths.gperf"
                                                        { "cacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 98 "poppler/CourierWidths.gperf"
                                                        { "j", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 210 "poppler/CourierWidths.gperf"
                                                        { "Dcroat", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 249 "poppler/CourierWidths.gperf"
                                                        { "oacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 72 "poppler/CourierWidths.gperf"
                                                        { "caron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 104 "poppler/CourierWidths.gperf"
                                                        { "o", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 317 "poppler/CourierWidths.gperf"
                                                        { "ecaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 321 "poppler/CourierWidths.gperf"
                                                        { "Rcaron", 600 },
#line 289 "poppler/CourierWidths.gperf"
                                                        { "seven", 600 },
#line 306 "poppler/CourierWidths.gperf"
                                                        { "sacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 224 "poppler/CourierWidths.gperf"
                                                        { "Dcaron", 600 },
                                                        { "", 0 },
#line 252 "poppler/CourierWidths.gperf"
                                                        { "tcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 26 "poppler/CourierWidths.gperf"
                                                        { "colon", 600 },
#line 278 "poppler/CourierWidths.gperf"
                                                        { "ncaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 125 "poppler/CourierWidths.gperf"
                                                        { "commaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 135 "poppler/CourierWidths.gperf"
                                                        { "semicolon", 600 },
#line 19 "poppler/CourierWidths.gperf"
                                                        { "comma", 600 },
#line 235 "poppler/CourierWidths.gperf"
                                                        { "degree", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 118 "poppler/CourierWidths.gperf"
                                                        { "Ccaron", 600 },
                                                        { "", 0 },
#line 140 "poppler/CourierWidths.gperf"
                                                        { "ccaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 113 "poppler/CourierWidths.gperf"
                                                        { "s", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 231 "poppler/CourierWidths.gperf"
                                                        { "racute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 81 "poppler/CourierWidths.gperf"
                                                        { "X", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 30 "poppler/CourierWidths.gperf"
                                                        { "ntilde", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 205 "poppler/CourierWidths.gperf"
                                                        { "tilde", 600 },
#line 324 "poppler/CourierWidths.gperf"
                                                        { "atilde", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 197 "poppler/CourierWidths.gperf"
                                                        { "nine", 600 },
#line 24 "poppler/CourierWidths.gperf"
                                                        { "edotaccent", 600 },
#line 105 "poppler/CourierWidths.gperf"
                                                        { "ordfeminine", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 158 "poppler/CourierWidths.gperf"
                                                        { "eight", 600 },
#line 150 "poppler/CourierWidths.gperf"
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
#line 277 "poppler/CourierWidths.gperf"
                                                        { "iacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 170 "poppler/CourierWidths.gperf"
                                                        { "otilde", 600 },
#line 292 "poppler/CourierWidths.gperf"
                                                        { "ordmasculine", 600 },
#line 213 "poppler/CourierWidths.gperf"
                                                        { "eth", 600 },
                                                        { "", 0 },
#line 42 "poppler/CourierWidths.gperf"
                                                        { "three", 600 },
#line 225 "poppler/CourierWidths.gperf"
                                                        { "dcroat", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 281 "poppler/CourierWidths.gperf"
                                                        { "Rcommaaccent", 600 },
#line 185 "poppler/CourierWidths.gperf"
                                                        { "Eacute", 600 },
#line 322 "poppler/CourierWidths.gperf"
                                                        { "Kcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 218 "poppler/CourierWidths.gperf"
                                                        { "uacute", 600 },
#line 103 "poppler/CourierWidths.gperf"
                                                        { "tcommaaccent", 600 },
                                                        { "", 0 },
#line 166 "poppler/CourierWidths.gperf"
                                                        { "copyright", 600 },
#line 43 "poppler/CourierWidths.gperf"
                                                        { "numbersign", 600 },
#line 15 "poppler/CourierWidths.gperf"
                                                        { "rcaron", 600 },
#line 32 "poppler/CourierWidths.gperf"
                                                        { "ncommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 110 "poppler/CourierWidths.gperf"
                                                        { "r", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 263 "poppler/CourierWidths.gperf"
                                                        { "lacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 23 "poppler/CourierWidths.gperf"
                                                        { "dotaccent", 600 },
#line 234 "poppler/CourierWidths.gperf"
                                                        { "thorn", 600 },
#line 242 "poppler/CourierWidths.gperf"
                                                        { "dcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 146 "poppler/CourierWidths.gperf"
                                                        { "macron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 203 "poppler/CourierWidths.gperf"
                                                        { "Ccedilla", 600 },
#line 274 "poppler/CourierWidths.gperf"
                                                        { "section", 600 },
#line 223 "poppler/CourierWidths.gperf"
                                                        { "ccedilla", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 20 "poppler/CourierWidths.gperf"
                                                        { "cedilla", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 25 "poppler/CourierWidths.gperf"
                                                        { "asciitilde", 600 },
#line 89 "poppler/CourierWidths.gperf"
                                                        { "d", 600 },
#line 239 "poppler/CourierWidths.gperf"
                                                        { "percent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 288 "poppler/CourierWidths.gperf"
                                                        { "germandbls", 600 },
                                                        { "", 0 },
#line 138 "poppler/CourierWidths.gperf"
                                                        { "lozenge", 600 },
                                                        { "", 0 },
#line 316 "poppler/CourierWidths.gperf"
                                                        { "less", 600 },
                                                        { "", 0 },
#line 97 "poppler/CourierWidths.gperf"
                                                        { "dagger", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 258 "poppler/CourierWidths.gperf"
                                                        { "grave", 600 },
#line 301 "poppler/CourierWidths.gperf"
                                                        { "Ecaron", 600 },
#line 222 "poppler/CourierWidths.gperf"
                                                        { "scommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 160 "poppler/CourierWidths.gperf"
                                                        { "endash", 600 },
#line 174 "poppler/CourierWidths.gperf"
                                                        { "emacron", 600 },
#line 201 "poppler/CourierWidths.gperf"
                                                        { "threequarters", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 232 "poppler/CourierWidths.gperf"
                                                        { "Tcaron", 600 },
                                                        { "", 0 },
#line 226 "poppler/CourierWidths.gperf"
                                                        { "scedilla", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 101 "poppler/CourierWidths.gperf"
                                                        { "m", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 245 "poppler/CourierWidths.gperf"
                                                        { "summation", 600 },
#line 310 "poppler/CourierWidths.gperf"
                                                        { "logicalnot", 600 },
#line 44 "poppler/CourierWidths.gperf"
                                                        { "lcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 172 "poppler/CourierWidths.gperf"
                                                        { "parenleft", 600 },
#line 139 "poppler/CourierWidths.gperf"
                                                        { "parenright", 600 },
#line 95 "poppler/CourierWidths.gperf"
                                                        { "i", 600 },
#line 305 "poppler/CourierWidths.gperf"
                                                        { "amacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 194 "poppler/CourierWidths.gperf"
                                                        { "Uacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 208 "poppler/CourierWidths.gperf"
                                                        { "underscore", 600 },
#line 92 "poppler/CourierWidths.gperf"
                                                        { "g", 600 },
#line 297 "poppler/CourierWidths.gperf"
                                                        { "rcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 37 "poppler/CourierWidths.gperf"
                                                        { "space", 600 },
#line 28 "poppler/CourierWidths.gperf"
                                                        { "dollar", 600 },
                                                        { "", 0 },
#line 272 "poppler/CourierWidths.gperf"
                                                        { "threesuperior", 600 },
#line 188 "poppler/CourierWidths.gperf"
                                                        { "edieresis", 600 },
#line 236 "poppler/CourierWidths.gperf"
                                                        { "registered", 600 },
#line 78 "poppler/CourierWidths.gperf"
                                                        { "W", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 64 "poppler/CourierWidths.gperf"
                                                        { "omacron", 600 },
#line 36 "poppler/CourierWidths.gperf"
                                                        { "yen", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 50 "poppler/CourierWidths.gperf"
                                                        { "E", 600 },
                                                        { "", 0 },
#line 293 "poppler/CourierWidths.gperf"
                                                        { "dotlessi", 600 },
                                                        { "", 0 },
#line 327 "poppler/CourierWidths.gperf"
                                                        { "Edotaccent", 600 },
#line 71 "poppler/CourierWidths.gperf"
                                                        { "Aacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 186 "poppler/CourierWidths.gperf"
                                                        { "adieresis", 600 },
                                                        { "", 0 },
#line 117 "poppler/CourierWidths.gperf"
                                                        { "u", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 144 "poppler/CourierWidths.gperf"
                                                        { "daggerdbl", 600 },
                                                        { "", 0 },
#line 280 "poppler/CourierWidths.gperf"
                                                        { "yacute", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 74 "poppler/CourierWidths.gperf"
                                                        { "T", 600 },
#line 130 "poppler/CourierWidths.gperf"
                                                        { "gcommaaccent", 600 },
#line 275 "poppler/CourierWidths.gperf"
                                                        { "dieresis", 600 },
                                                        { "", 0 },
#line 51 "poppler/CourierWidths.gperf"
                                                        { "onequarter", 600 },
#line 328 "poppler/CourierWidths.gperf"
                                                        { "onesuperior", 600 },
#line 237 "poppler/CourierWidths.gperf"
                                                        { "radical", 600 },
#line 190 "poppler/CourierWidths.gperf"
                                                        { "Eth", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 94 "poppler/CourierWidths.gperf"
                                                        { "h", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 193 "poppler/CourierWidths.gperf"
                                                        { "odieresis", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 100 "poppler/CourierWidths.gperf"
                                                        { "l", 600 },
#line 65 "poppler/CourierWidths.gperf"
                                                        { "Tcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 136 "poppler/CourierWidths.gperf"
                                                        { "oslash", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 137 "poppler/CourierWidths.gperf"
                                                        { "lessequal", 600 },
#line 159 "poppler/CourierWidths.gperf"
                                                        { "exclamdown", 600 },
#line 35 "poppler/CourierWidths.gperf"
                                                        { "zacute", 600 },
#line 269 "poppler/CourierWidths.gperf"
                                                        { "lcommaaccent", 600 },
                                                        { "", 0 },
#line 209 "poppler/CourierWidths.gperf"
                                                        { "Euro", 600 },
                                                        { "", 0 },
#line 291 "poppler/CourierWidths.gperf"
                                                        { "Sacute", 600 },
#line 323 "poppler/CourierWidths.gperf"
                                                        { "greater", 600 },
#line 244 "poppler/CourierWidths.gperf"
                                                        { "two", 600 },
                                                        { "", 0 },
#line 220 "poppler/CourierWidths.gperf"
                                                        { "Thorn", 600 },
#line 256 "poppler/CourierWidths.gperf"
                                                        { "asciicircum", 600 },
#line 126 "poppler/CourierWidths.gperf"
                                                        { "hungarumlaut", 600 },
                                                        { "", 0 },
#line 211 "poppler/CourierWidths.gperf"
                                                        { "zero", 600 },
                                                        { "", 0 },
#line 40 "poppler/CourierWidths.gperf"
                                                        { "emdash", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 116 "poppler/CourierWidths.gperf"
                                                        { "divide", 600 },
                                                        { "", 0 },
#line 271 "poppler/CourierWidths.gperf"
                                                        { "ohungarumlaut", 600 },
#line 261 "poppler/CourierWidths.gperf"
                                                        { "ampersand", 600 },
                                                        { "", 0 },
#line 164 "poppler/CourierWidths.gperf"
                                                        { "ecircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 106 "poppler/CourierWidths.gperf"
                                                        { "ring", 600 },
                                                        { "", 0 },
#line 320 "poppler/CourierWidths.gperf"
                                                        { "period", 600 },
                                                        { "", 0 },
#line 318 "poppler/CourierWidths.gperf"
                                                        { "guilsinglleft", 600 },
#line 156 "poppler/CourierWidths.gperf"
                                                        { "guilsinglright", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 307 "poppler/CourierWidths.gperf"
                                                        { "imacron", 600 },
                                                        { "", 0 },
#line 61 "poppler/CourierWidths.gperf"
                                                        { "periodcentered", 600 },
                                                        { "", 0 },
#line 227 "poppler/CourierWidths.gperf"
                                                        { "Oacute", 600 },
                                                        { "", 0 },
#line 294 "poppler/CourierWidths.gperf"
                                                        { "sterling", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 299 "poppler/CourierWidths.gperf"
                                                        { "acircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 33 "poppler/CourierWidths.gperf"
                                                        { "minus", 600 },
#line 312 "poppler/CourierWidths.gperf"
                                                        { "Atilde", 600 },
#line 148 "poppler/CourierWidths.gperf"
                                                        { "Emacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 257 "poppler/CourierWidths.gperf"
                                                        { "aring", 600 },
#line 262 "poppler/CourierWidths.gperf"
                                                        { "Iacute", 600 },
#line 183 "poppler/CourierWidths.gperf"
                                                        { "umacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 221 "poppler/CourierWidths.gperf"
                                                        { "zcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 133 "poppler/CourierWidths.gperf"
                                                        { "Scaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 248 "poppler/CourierWidths.gperf"
                                                        { "ocircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 189 "poppler/CourierWidths.gperf"
                                                        { "idieresis", 600 },
                                                        { "", 0 },
#line 157 "poppler/CourierWidths.gperf"
                                                        { "quotesingle", 600 },
#line 276 "poppler/CourierWidths.gperf"
                                                        { "quotedblbase", 600 },
                                                        { "", 0 },
#line 268 "poppler/CourierWidths.gperf"
                                                        { "quotesinglbase", 600 },
                                                        { "", 0 },
#line 107 "poppler/CourierWidths.gperf"
                                                        { "p", 600 },
#line 132 "poppler/CourierWidths.gperf"
                                                        { "greaterequal", 600 },
                                                        { "", 0 },
#line 326 "poppler/CourierWidths.gperf"
                                                        { "quoteleft", 600 },
#line 179 "poppler/CourierWidths.gperf"
                                                        { "quoteright", 600 },
                                                        { "", 0 },
#line 154 "poppler/CourierWidths.gperf"
                                                        { "quotedblleft", 600 },
#line 304 "poppler/CourierWidths.gperf"
                                                        { "quotedblright", 600 },
#line 169 "poppler/CourierWidths.gperf"
                                                        { "Edieresis", 600 },
                                                        { "", 0 },
#line 128 "poppler/CourierWidths.gperf"
                                                        { "Nacute", 600 },
#line 131 "poppler/CourierWidths.gperf"
                                                        { "mu", 600 },
                                                        { "", 0 },
#line 198 "poppler/CourierWidths.gperf"
                                                        { "udieresis", 600 },
                                                        { "", 0 },
#line 270 "poppler/CourierWidths.gperf"
                                                        { "Yacute", 600 },
#line 253 "poppler/CourierWidths.gperf"
                                                        { "eogonek", 600 },
#line 80 "poppler/CourierWidths.gperf"
                                                        { "question", 600 },
                                                        { "", 0 },
#line 313 "poppler/CourierWidths.gperf"
                                                        { "breve", 600 },
#line 77 "poppler/CourierWidths.gperf"
                                                        { "V", 600 },
#line 39 "poppler/CourierWidths.gperf"
                                                        { "questiondown", 600 },
                                                        { "", 0 },
#line 266 "poppler/CourierWidths.gperf"
                                                        { "plus", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 149 "poppler/CourierWidths.gperf"
                                                        { "ellipsis", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 319 "poppler/CourierWidths.gperf"
                                                        { "exclam", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 219 "poppler/CourierWidths.gperf"
                                                        { "braceleft", 600 },
#line 302 "poppler/CourierWidths.gperf"
                                                        { "braceright", 600 },
#line 155 "poppler/CourierWidths.gperf"
                                                        { "hyphen", 600 },
#line 48 "poppler/CourierWidths.gperf"
                                                        { "aogonek", 600 },
#line 314 "poppler/CourierWidths.gperf"
                                                        { "bar", 600 },
                                                        { "", 0 },
#line 311 "poppler/CourierWidths.gperf"
                                                        { "zdotaccent", 600 },
#line 153 "poppler/CourierWidths.gperf"
                                                        { "lslash", 600 },
#line 86 "poppler/CourierWidths.gperf"
                                                        { "Gcommaaccent", 600 },
#line 309 "poppler/CourierWidths.gperf"
                                                        { "currency", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 75 "poppler/CourierWidths.gperf"
                                                        { "U", 600 },
#line 27 "poppler/CourierWidths.gperf"
                                                        { "onehalf", 600 },
#line 109 "poppler/CourierWidths.gperf"
                                                        { "uhungarumlaut", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 147 "poppler/CourierWidths.gperf"
                                                        { "Otilde", 600 },
                                                        { "", 0 },
#line 287 "poppler/CourierWidths.gperf"
                                                        { "guillemotleft", 600 },
#line 202 "poppler/CourierWidths.gperf"
                                                        { "guillemotright", 600 },
                                                        { "", 0 },
#line 247 "poppler/CourierWidths.gperf"
                                                        { "Lacute", 600 },
#line 163 "poppler/CourierWidths.gperf"
                                                        { "Umacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 18 "poppler/CourierWidths.gperf"
                                                        { "Zacute", 600 },
                                                        { "", 0 },
#line 295 "poppler/CourierWidths.gperf"
                                                        { "notequal", 600 },
#line 143 "poppler/CourierWidths.gperf"
                                                        { "trademark", 600 },
                                                        { "", 0 },
#line 265 "poppler/CourierWidths.gperf"
                                                        { "Ncaron", 600 },
#line 200 "poppler/CourierWidths.gperf"
                                                        { "Scommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 181 "poppler/CourierWidths.gperf"
                                                        { "perthousand", 600 },
                                                        { "", 0 },
#line 240 "poppler/CourierWidths.gperf"
                                                        { "six", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 303 "poppler/CourierWidths.gperf"
                                                        { "icircumflex", 600 },
                                                        { "", 0 },
#line 214 "poppler/CourierWidths.gperf"
                                                        { "Scedilla", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 93 "poppler/CourierWidths.gperf"
                                                        { "bullet", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 108 "poppler/CourierWidths.gperf"
                                                        { "q", 600 },
#line 290 "poppler/CourierWidths.gperf"
                                                        { "Amacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 127 "poppler/CourierWidths.gperf"
                                                        { "Idotaccent", 600 },
#line 141 "poppler/CourierWidths.gperf"
                                                        { "Ecircumflex", 600 },
                                                        { "", 0 },
#line 315 "poppler/CourierWidths.gperf"
                                                        { "fraction", 600 },
#line 180 "poppler/CourierWidths.gperf"
                                                        { "Udieresis", 600 },
                                                        { "", 0 },
#line 176 "poppler/CourierWidths.gperf"
                                                        { "ucircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 196 "poppler/CourierWidths.gperf"
                                                        { "five", 600 },
                                                        { "", 0 },
#line 14 "poppler/CourierWidths.gperf"
                                                        { "Ntilde", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 267 "poppler/CourierWidths.gperf"
                                                        { "uring", 600 },
#line 45 "poppler/CourierWidths.gperf"
                                                        { "A", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 84 "poppler/CourierWidths.gperf"
                                                        { "four", 600 },
                                                        { "", 0 },
#line 187 "poppler/CourierWidths.gperf"
                                                        { "egrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 241 "poppler/CourierWidths.gperf"
                                                        { "paragraph", 600 },
                                                        { "", 0 },
#line 29 "poppler/CourierWidths.gperf"
                                                        { "Lcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 325 "poppler/CourierWidths.gperf"
                                                        { "brokenbar", 600 },
                                                        { "", 0 },
#line 199 "poppler/CourierWidths.gperf"
                                                        { "Zcaron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 165 "poppler/CourierWidths.gperf"
                                                        { "Adieresis", 600 },
                                                        { "", 0 },
#line 122 "poppler/CourierWidths.gperf"
                                                        { "y", 600 },
#line 16 "poppler/CourierWidths.gperf"
                                                        { "kcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 76 "poppler/CourierWidths.gperf"
                                                        { "agrave", 600 },
                                                        { "", 0 },
#line 69 "poppler/CourierWidths.gperf"
                                                        { "Uhungarumlaut", 600 },
#line 204 "poppler/CourierWidths.gperf"
                                                        { "ydieresis", 600 },
#line 168 "poppler/CourierWidths.gperf"
                                                        { "slash", 600 },
#line 229 "poppler/CourierWidths.gperf"
                                                        { "ogonek", 600 },
#line 151 "poppler/CourierWidths.gperf"
                                                        { "AE", 600 },
#line 192 "poppler/CourierWidths.gperf"
                                                        { "asterisk", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 111 "poppler/CourierWidths.gperf"
                                                        { "twosuperior", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 53 "poppler/CourierWidths.gperf"
                                                        { "G", 600 },
#line 58 "poppler/CourierWidths.gperf"
                                                        { "iogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 17 "poppler/CourierWidths.gperf"
                                                        { "Ncommaaccent", 600 },
                                                        { "", 0 },
#line 21 "poppler/CourierWidths.gperf"
                                                        { "plusminus", 603 },
                                                        { "", 0 },
#line 230 "poppler/CourierWidths.gperf"
                                                        { "ograve", 600 },
                                                        { "", 0 },
#line 129 "poppler/CourierWidths.gperf"
                                                        { "quotedbl", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 233 "poppler/CourierWidths.gperf"
                                                        { "Eogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 120 "poppler/CourierWidths.gperf"
                                                        { "w", 600 },
#line 259 "poppler/CourierWidths.gperf"
                                                        { "uogonek", 600 },
                                                        { "", 0 },
#line 60 "poppler/CourierWidths.gperf"
                                                        { "backslash", 600 },
#line 79 "poppler/CourierWidths.gperf"
                                                        { "equal", 600 },
                                                        { "", 0 },
#line 38 "poppler/CourierWidths.gperf"
                                                        { "Omacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 121 "poppler/CourierWidths.gperf"
                                                        { "x", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 298 "poppler/CourierWidths.gperf"
                                                        { "Zdotaccent", 600 },
#line 152 "poppler/CourierWidths.gperf"
                                                        { "Ucircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 68 "poppler/CourierWidths.gperf"
                                                        { "Q", 600 },
#line 296 "poppler/CourierWidths.gperf"
                                                        { "Imacron", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 250 "poppler/CourierWidths.gperf"
                                                        { "Uring", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 123 "poppler/CourierWidths.gperf"
                                                        { "z", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 22 "poppler/CourierWidths.gperf"
                                                        { "circumflex", 600 },
                                                        { "", 0 },
#line 251 "poppler/CourierWidths.gperf"
                                                        { "Lcommaaccent", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 73 "poppler/CourierWidths.gperf"
                                                        { "S", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 175 "poppler/CourierWidths.gperf"
                                                        { "Odieresis", 600 },
                                                        { "", 0 },
#line 284 "poppler/CourierWidths.gperf"
                                                        { "Acircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 67 "poppler/CourierWidths.gperf"
                                                        { "P", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 238 "poppler/CourierWidths.gperf"
                                                        { "Aring", 600 },
#line 96 "poppler/CourierWidths.gperf"
                                                        { "Oslash", 600 },
#line 114 "poppler/CourierWidths.gperf"
                                                        { "OE", 600 },
                                                        { "", 0 },
#line 171 "poppler/CourierWidths.gperf"
                                                        { "Idieresis", 600 },
                                                        { "", 0 },
#line 62 "poppler/CourierWidths.gperf"
                                                        { "M", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 282 "poppler/CourierWidths.gperf"
                                                        { "fi", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 56 "poppler/CourierWidths.gperf"
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
#line 264 "poppler/CourierWidths.gperf"
                                                        { "igrave", 600 },
                                                        { "", 0 },
#line 255 "poppler/CourierWidths.gperf"
                                                        { "Ohungarumlaut", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 243 "poppler/CourierWidths.gperf"
                                                        { "Uogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 87 "poppler/CourierWidths.gperf"
                                                        { "b", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 167 "poppler/CourierWidths.gperf"
                                                        { "Egrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 182 "poppler/CourierWidths.gperf"
                                                        { "Ydieresis", 600 },
                                                        { "", 0 },
#line 195 "poppler/CourierWidths.gperf"
                                                        { "ugrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 212 "poppler/CourierWidths.gperf"
                                                        { "multiply", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 66 "poppler/CourierWidths.gperf"
                                                        { "O", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 31 "poppler/CourierWidths.gperf"
                                                        { "Aogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 279 "poppler/CourierWidths.gperf"
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
#line 228 "poppler/CourierWidths.gperf"
                                                        { "Ocircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 283 "poppler/CourierWidths.gperf"
                                                        { "fl", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 55 "poppler/CourierWidths.gperf"
                                                        { "I", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 286 "poppler/CourierWidths.gperf"
                                                        { "Icircumflex", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 54 "poppler/CourierWidths.gperf"
                                                        { "H", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 134 "poppler/CourierWidths.gperf"
                                                        { "Lslash", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 99 "poppler/CourierWidths.gperf"
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
#line 184 "poppler/CourierWidths.gperf"
                                                        { "abreve", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 217 "poppler/CourierWidths.gperf"
                                                        { "partialdiff", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 52 "poppler/CourierWidths.gperf"
                                                        { "F", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 178 "poppler/CourierWidths.gperf"
                                                        { "Ugrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 91 "poppler/CourierWidths.gperf"
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
#line 119 "poppler/CourierWidths.gperf"
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
#line 63 "poppler/CourierWidths.gperf"
                                                        { "N", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 41 "poppler/CourierWidths.gperf"
                                                        { "Agrave", 600 },
#line 34 "poppler/CourierWidths.gperf"
                                                        { "Iogonek", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 82 "poppler/CourierWidths.gperf"
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
#line 46 "poppler/CourierWidths.gperf"
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
#line 177 "poppler/CourierWidths.gperf"
                                                        { "bracketleft", 600 },
#line 260 "poppler/CourierWidths.gperf"
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
#line 142 "poppler/CourierWidths.gperf"
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
#line 216 "poppler/CourierWidths.gperf"
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
#line 59 "poppler/CourierWidths.gperf"
                                                        { "L", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 246 "poppler/CourierWidths.gperf"
                                                        { "Igrave", 600 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 83 "poppler/CourierWidths.gperf"
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
#line 162 "poppler/CourierWidths.gperf"
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
#line 124 "poppler/CourierWidths.gperf"
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
#line 330 "poppler/CourierWidths.gperf"
