/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf poppler/TimesBoldWidths.gperf  */
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

#line 1 "poppler/TimesBoldWidths.gperf"

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

const struct BuiltinFontWidth *TimesBoldWidthsLookup(register const char *str, register size_t len)
{
    static const struct BuiltinFontWidth wordlist[] = { { "", 0 },
#line 90 "poppler/TimesBoldWidths.gperf"
                                                        { "e", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 70 "poppler/TimesBoldWidths.gperf"
                                                        { "R", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 57 "poppler/TimesBoldWidths.gperf"
                                                        { "K", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 49 "poppler/TimesBoldWidths.gperf"
                                                        { "D", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 115 "poppler/TimesBoldWidths.gperf"
                                                        { "t", 333 },
#line 191 "poppler/TimesBoldWidths.gperf"
                                                        { "ae", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 102 "poppler/TimesBoldWidths.gperf"
                                                        { "n", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 207 "poppler/TimesBoldWidths.gperf"
                                                        { "eacute", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 216 "poppler/TimesBoldWidths.gperf"
                                                        { "Racute", 722 },
                                                        { "", 0 },
#line 85 "poppler/TimesBoldWidths.gperf"
                                                        { "a", 500 },
#line 206 "poppler/TimesBoldWidths.gperf"
                                                        { "at", 930 },
                                                        { "", 0 },
#line 308 "poppler/TimesBoldWidths.gperf"
                                                        { "cent", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 161 "poppler/TimesBoldWidths.gperf"
                                                        { "oe", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 145 "poppler/TimesBoldWidths.gperf"
                                                        { "nacute", 556 },
                                                        { "", 0 },
#line 254 "poppler/TimesBoldWidths.gperf"
                                                        { "Delta", 612 },
                                                        { "", 0 },
#line 273 "poppler/TimesBoldWidths.gperf"
                                                        { "acute", 333 },
#line 112 "poppler/TimesBoldWidths.gperf"
                                                        { "aacute", 500 },
#line 47 "poppler/TimesBoldWidths.gperf"
                                                        { "C", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 88 "poppler/TimesBoldWidths.gperf"
                                                        { "c", 444 },
                                                        { "", 0 },
#line 173 "poppler/TimesBoldWidths.gperf"
                                                        { "one", 500 },
#line 285 "poppler/TimesBoldWidths.gperf"
                                                        { "Cacute", 722 },
                                                        { "", 0 },
#line 300 "poppler/TimesBoldWidths.gperf"
                                                        { "cacute", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 98 "poppler/TimesBoldWidths.gperf"
                                                        { "j", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 210 "poppler/TimesBoldWidths.gperf"
                                                        { "Dcroat", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 249 "poppler/TimesBoldWidths.gperf"
                                                        { "oacute", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 72 "poppler/TimesBoldWidths.gperf"
                                                        { "caron", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 104 "poppler/TimesBoldWidths.gperf"
                                                        { "o", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 317 "poppler/TimesBoldWidths.gperf"
                                                        { "ecaron", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 321 "poppler/TimesBoldWidths.gperf"
                                                        { "Rcaron", 722 },
#line 290 "poppler/TimesBoldWidths.gperf"
                                                        { "seven", 500 },
#line 306 "poppler/TimesBoldWidths.gperf"
                                                        { "sacute", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 224 "poppler/TimesBoldWidths.gperf"
                                                        { "Dcaron", 722 },
                                                        { "", 0 },
#line 252 "poppler/TimesBoldWidths.gperf"
                                                        { "tcaron", 416 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 26 "poppler/TimesBoldWidths.gperf"
                                                        { "colon", 333 },
#line 278 "poppler/TimesBoldWidths.gperf"
                                                        { "ncaron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 125 "poppler/TimesBoldWidths.gperf"
                                                        { "commaaccent", 250 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 135 "poppler/TimesBoldWidths.gperf"
                                                        { "semicolon", 333 },
#line 19 "poppler/TimesBoldWidths.gperf"
                                                        { "comma", 250 },
#line 235 "poppler/TimesBoldWidths.gperf"
                                                        { "degree", 400 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 118 "poppler/TimesBoldWidths.gperf"
                                                        { "Ccaron", 722 },
                                                        { "", 0 },
#line 140 "poppler/TimesBoldWidths.gperf"
                                                        { "ccaron", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 113 "poppler/TimesBoldWidths.gperf"
                                                        { "s", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 231 "poppler/TimesBoldWidths.gperf"
                                                        { "racute", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 79 "poppler/TimesBoldWidths.gperf"
                                                        { "X", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 30 "poppler/TimesBoldWidths.gperf"
                                                        { "ntilde", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 205 "poppler/TimesBoldWidths.gperf"
                                                        { "tilde", 333 },
#line 324 "poppler/TimesBoldWidths.gperf"
                                                        { "atilde", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 196 "poppler/TimesBoldWidths.gperf"
                                                        { "nine", 500 },
#line 24 "poppler/TimesBoldWidths.gperf"
                                                        { "edotaccent", 444 },
#line 105 "poppler/TimesBoldWidths.gperf"
                                                        { "ordfeminine", 300 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 158 "poppler/TimesBoldWidths.gperf"
                                                        { "eight", 500 },
#line 150 "poppler/TimesBoldWidths.gperf"
                                                        { "scaron", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 276 "poppler/TimesBoldWidths.gperf"
                                                        { "iacute", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 170 "poppler/TimesBoldWidths.gperf"
                                                        { "otilde", 500 },
#line 292 "poppler/TimesBoldWidths.gperf"
                                                        { "ordmasculine", 330 },
#line 213 "poppler/TimesBoldWidths.gperf"
                                                        { "eth", 500 },
                                                        { "", 0 },
#line 42 "poppler/TimesBoldWidths.gperf"
                                                        { "three", 500 },
#line 225 "poppler/TimesBoldWidths.gperf"
                                                        { "dcroat", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 281 "poppler/TimesBoldWidths.gperf"
                                                        { "Rcommaaccent", 722 },
#line 185 "poppler/TimesBoldWidths.gperf"
                                                        { "Eacute", 667 },
#line 322 "poppler/TimesBoldWidths.gperf"
                                                        { "Kcommaaccent", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 218 "poppler/TimesBoldWidths.gperf"
                                                        { "uacute", 556 },
#line 103 "poppler/TimesBoldWidths.gperf"
                                                        { "tcommaaccent", 333 },
                                                        { "", 0 },
#line 166 "poppler/TimesBoldWidths.gperf"
                                                        { "copyright", 747 },
#line 43 "poppler/TimesBoldWidths.gperf"
                                                        { "numbersign", 500 },
#line 15 "poppler/TimesBoldWidths.gperf"
                                                        { "rcaron", 444 },
#line 32 "poppler/TimesBoldWidths.gperf"
                                                        { "ncommaaccent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 110 "poppler/TimesBoldWidths.gperf"
                                                        { "r", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 264 "poppler/TimesBoldWidths.gperf"
                                                        { "lacute", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 23 "poppler/TimesBoldWidths.gperf"
                                                        { "dotaccent", 333 },
#line 234 "poppler/TimesBoldWidths.gperf"
                                                        { "thorn", 556 },
#line 242 "poppler/TimesBoldWidths.gperf"
                                                        { "dcaron", 672 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 146 "poppler/TimesBoldWidths.gperf"
                                                        { "macron", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 203 "poppler/TimesBoldWidths.gperf"
                                                        { "Ccedilla", 722 },
#line 274 "poppler/TimesBoldWidths.gperf"
                                                        { "section", 500 },
#line 223 "poppler/TimesBoldWidths.gperf"
                                                        { "ccedilla", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 20 "poppler/TimesBoldWidths.gperf"
                                                        { "cedilla", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 25 "poppler/TimesBoldWidths.gperf"
                                                        { "asciitilde", 520 },
#line 89 "poppler/TimesBoldWidths.gperf"
                                                        { "d", 556 },
#line 239 "poppler/TimesBoldWidths.gperf"
                                                        { "percent", 1000 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 288 "poppler/TimesBoldWidths.gperf"
                                                        { "germandbls", 556 },
                                                        { "", 0 },
#line 138 "poppler/TimesBoldWidths.gperf"
                                                        { "lozenge", 494 },
                                                        { "", 0 },
#line 316 "poppler/TimesBoldWidths.gperf"
                                                        { "less", 570 },
                                                        { "", 0 },
#line 97 "poppler/TimesBoldWidths.gperf"
                                                        { "dagger", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 258 "poppler/TimesBoldWidths.gperf"
                                                        { "grave", 333 },
#line 301 "poppler/TimesBoldWidths.gperf"
                                                        { "Ecaron", 667 },
#line 222 "poppler/TimesBoldWidths.gperf"
                                                        { "scommaaccent", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 160 "poppler/TimesBoldWidths.gperf"
                                                        { "endash", 500 },
#line 174 "poppler/TimesBoldWidths.gperf"
                                                        { "emacron", 444 },
#line 201 "poppler/TimesBoldWidths.gperf"
                                                        { "threequarters", 750 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 232 "poppler/TimesBoldWidths.gperf"
                                                        { "Tcaron", 667 },
                                                        { "", 0 },
#line 228 "poppler/TimesBoldWidths.gperf"
                                                        { "scedilla", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 101 "poppler/TimesBoldWidths.gperf"
                                                        { "m", 833 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 245 "poppler/TimesBoldWidths.gperf"
                                                        { "summation", 600 },
#line 310 "poppler/TimesBoldWidths.gperf"
                                                        { "logicalnot", 570 },
#line 44 "poppler/TimesBoldWidths.gperf"
                                                        { "lcaron", 394 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 172 "poppler/TimesBoldWidths.gperf"
                                                        { "parenleft", 333 },
#line 139 "poppler/TimesBoldWidths.gperf"
                                                        { "parenright", 333 },
#line 95 "poppler/TimesBoldWidths.gperf"
                                                        { "i", 278 },
#line 305 "poppler/TimesBoldWidths.gperf"
                                                        { "amacron", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 194 "poppler/TimesBoldWidths.gperf"
                                                        { "Uacute", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 208 "poppler/TimesBoldWidths.gperf"
                                                        { "underscore", 500 },
#line 92 "poppler/TimesBoldWidths.gperf"
                                                        { "g", 500 },
#line 297 "poppler/TimesBoldWidths.gperf"
                                                        { "rcommaaccent", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 37 "poppler/TimesBoldWidths.gperf"
                                                        { "space", 250 },
#line 28 "poppler/TimesBoldWidths.gperf"
                                                        { "dollar", 500 },
                                                        { "", 0 },
#line 272 "poppler/TimesBoldWidths.gperf"
                                                        { "threesuperior", 300 },
#line 188 "poppler/TimesBoldWidths.gperf"
                                                        { "edieresis", 444 },
#line 236 "poppler/TimesBoldWidths.gperf"
                                                        { "registered", 747 },
#line 78 "poppler/TimesBoldWidths.gperf"
                                                        { "W", 1000 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 64 "poppler/TimesBoldWidths.gperf"
                                                        { "omacron", 500 },
#line 36 "poppler/TimesBoldWidths.gperf"
                                                        { "yen", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 50 "poppler/TimesBoldWidths.gperf"
                                                        { "E", 667 },
                                                        { "", 0 },
#line 293 "poppler/TimesBoldWidths.gperf"
                                                        { "dotlessi", 278 },
                                                        { "", 0 },
#line 327 "poppler/TimesBoldWidths.gperf"
                                                        { "Edotaccent", 667 },
#line 71 "poppler/TimesBoldWidths.gperf"
                                                        { "Aacute", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 186 "poppler/TimesBoldWidths.gperf"
                                                        { "adieresis", 500 },
                                                        { "", 0 },
#line 117 "poppler/TimesBoldWidths.gperf"
                                                        { "u", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 144 "poppler/TimesBoldWidths.gperf"
                                                        { "daggerdbl", 500 },
                                                        { "", 0 },
#line 280 "poppler/TimesBoldWidths.gperf"
                                                        { "yacute", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 74 "poppler/TimesBoldWidths.gperf"
                                                        { "T", 667 },
#line 130 "poppler/TimesBoldWidths.gperf"
                                                        { "gcommaaccent", 500 },
#line 275 "poppler/TimesBoldWidths.gperf"
                                                        { "dieresis", 333 },
                                                        { "", 0 },
#line 51 "poppler/TimesBoldWidths.gperf"
                                                        { "onequarter", 750 },
#line 328 "poppler/TimesBoldWidths.gperf"
                                                        { "onesuperior", 300 },
#line 237 "poppler/TimesBoldWidths.gperf"
                                                        { "radical", 549 },
#line 190 "poppler/TimesBoldWidths.gperf"
                                                        { "Eth", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 94 "poppler/TimesBoldWidths.gperf"
                                                        { "h", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 193 "poppler/TimesBoldWidths.gperf"
                                                        { "odieresis", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 100 "poppler/TimesBoldWidths.gperf"
                                                        { "l", 278 },
#line 65 "poppler/TimesBoldWidths.gperf"
                                                        { "Tcommaaccent", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 136 "poppler/TimesBoldWidths.gperf"
                                                        { "oslash", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 137 "poppler/TimesBoldWidths.gperf"
                                                        { "lessequal", 549 },
#line 159 "poppler/TimesBoldWidths.gperf"
                                                        { "exclamdown", 333 },
#line 35 "poppler/TimesBoldWidths.gperf"
                                                        { "zacute", 444 },
#line 269 "poppler/TimesBoldWidths.gperf"
                                                        { "lcommaaccent", 278 },
                                                        { "", 0 },
#line 209 "poppler/TimesBoldWidths.gperf"
                                                        { "Euro", 500 },
                                                        { "", 0 },
#line 291 "poppler/TimesBoldWidths.gperf"
                                                        { "Sacute", 556 },
#line 323 "poppler/TimesBoldWidths.gperf"
                                                        { "greater", 570 },
#line 244 "poppler/TimesBoldWidths.gperf"
                                                        { "two", 500 },
                                                        { "", 0 },
#line 220 "poppler/TimesBoldWidths.gperf"
                                                        { "Thorn", 611 },
#line 256 "poppler/TimesBoldWidths.gperf"
                                                        { "asciicircum", 581 },
#line 126 "poppler/TimesBoldWidths.gperf"
                                                        { "hungarumlaut", 333 },
                                                        { "", 0 },
#line 212 "poppler/TimesBoldWidths.gperf"
                                                        { "zero", 500 },
                                                        { "", 0 },
#line 40 "poppler/TimesBoldWidths.gperf"
                                                        { "emdash", 1000 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 116 "poppler/TimesBoldWidths.gperf"
                                                        { "divide", 570 },
                                                        { "", 0 },
#line 271 "poppler/TimesBoldWidths.gperf"
                                                        { "ohungarumlaut", 500 },
#line 262 "poppler/TimesBoldWidths.gperf"
                                                        { "ampersand", 833 },
                                                        { "", 0 },
#line 164 "poppler/TimesBoldWidths.gperf"
                                                        { "ecircumflex", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 106 "poppler/TimesBoldWidths.gperf"
                                                        { "ring", 333 },
                                                        { "", 0 },
#line 320 "poppler/TimesBoldWidths.gperf"
                                                        { "period", 250 },
                                                        { "", 0 },
#line 318 "poppler/TimesBoldWidths.gperf"
                                                        { "guilsinglleft", 333 },
#line 155 "poppler/TimesBoldWidths.gperf"
                                                        { "guilsinglright", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 307 "poppler/TimesBoldWidths.gperf"
                                                        { "imacron", 278 },
                                                        { "", 0 },
#line 61 "poppler/TimesBoldWidths.gperf"
                                                        { "periodcentered", 250 },
                                                        { "", 0 },
#line 227 "poppler/TimesBoldWidths.gperf"
                                                        { "Oacute", 778 },
                                                        { "", 0 },
#line 294 "poppler/TimesBoldWidths.gperf"
                                                        { "sterling", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 299 "poppler/TimesBoldWidths.gperf"
                                                        { "acircumflex", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 33 "poppler/TimesBoldWidths.gperf"
                                                        { "minus", 570 },
#line 312 "poppler/TimesBoldWidths.gperf"
                                                        { "Atilde", 722 },
#line 148 "poppler/TimesBoldWidths.gperf"
                                                        { "Emacron", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 257 "poppler/TimesBoldWidths.gperf"
                                                        { "aring", 500 },
#line 261 "poppler/TimesBoldWidths.gperf"
                                                        { "Iacute", 389 },
#line 183 "poppler/TimesBoldWidths.gperf"
                                                        { "umacron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 221 "poppler/TimesBoldWidths.gperf"
                                                        { "zcaron", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 133 "poppler/TimesBoldWidths.gperf"
                                                        { "Scaron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 248 "poppler/TimesBoldWidths.gperf"
                                                        { "ocircumflex", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 189 "poppler/TimesBoldWidths.gperf"
                                                        { "idieresis", 278 },
                                                        { "", 0 },
#line 157 "poppler/TimesBoldWidths.gperf"
                                                        { "quotesingle", 278 },
#line 277 "poppler/TimesBoldWidths.gperf"
                                                        { "quotedblbase", 500 },
                                                        { "", 0 },
#line 268 "poppler/TimesBoldWidths.gperf"
                                                        { "quotesinglbase", 333 },
                                                        { "", 0 },
#line 107 "poppler/TimesBoldWidths.gperf"
                                                        { "p", 556 },
#line 132 "poppler/TimesBoldWidths.gperf"
                                                        { "greaterequal", 549 },
                                                        { "", 0 },
#line 326 "poppler/TimesBoldWidths.gperf"
                                                        { "quoteleft", 333 },
#line 179 "poppler/TimesBoldWidths.gperf"
                                                        { "quoteright", 333 },
                                                        { "", 0 },
#line 154 "poppler/TimesBoldWidths.gperf"
                                                        { "quotedblleft", 500 },
#line 304 "poppler/TimesBoldWidths.gperf"
                                                        { "quotedblright", 500 },
#line 169 "poppler/TimesBoldWidths.gperf"
                                                        { "Edieresis", 667 },
                                                        { "", 0 },
#line 128 "poppler/TimesBoldWidths.gperf"
                                                        { "Nacute", 722 },
#line 131 "poppler/TimesBoldWidths.gperf"
                                                        { "mu", 556 },
                                                        { "", 0 },
#line 198 "poppler/TimesBoldWidths.gperf"
                                                        { "udieresis", 556 },
                                                        { "", 0 },
#line 270 "poppler/TimesBoldWidths.gperf"
                                                        { "Yacute", 722 },
#line 253 "poppler/TimesBoldWidths.gperf"
                                                        { "eogonek", 444 },
#line 80 "poppler/TimesBoldWidths.gperf"
                                                        { "question", 500 },
                                                        { "", 0 },
#line 313 "poppler/TimesBoldWidths.gperf"
                                                        { "breve", 333 },
#line 77 "poppler/TimesBoldWidths.gperf"
                                                        { "V", 722 },
#line 39 "poppler/TimesBoldWidths.gperf"
                                                        { "questiondown", 500 },
                                                        { "", 0 },
#line 266 "poppler/TimesBoldWidths.gperf"
                                                        { "plus", 570 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 149 "poppler/TimesBoldWidths.gperf"
                                                        { "ellipsis", 1000 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 319 "poppler/TimesBoldWidths.gperf"
                                                        { "exclam", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 219 "poppler/TimesBoldWidths.gperf"
                                                        { "braceleft", 394 },
#line 303 "poppler/TimesBoldWidths.gperf"
                                                        { "braceright", 394 },
#line 156 "poppler/TimesBoldWidths.gperf"
                                                        { "hyphen", 333 },
#line 48 "poppler/TimesBoldWidths.gperf"
                                                        { "aogonek", 500 },
#line 314 "poppler/TimesBoldWidths.gperf"
                                                        { "bar", 220 },
                                                        { "", 0 },
#line 311 "poppler/TimesBoldWidths.gperf"
                                                        { "zdotaccent", 444 },
#line 153 "poppler/TimesBoldWidths.gperf"
                                                        { "lslash", 278 },
#line 86 "poppler/TimesBoldWidths.gperf"
                                                        { "Gcommaaccent", 778 },
#line 309 "poppler/TimesBoldWidths.gperf"
                                                        { "currency", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 75 "poppler/TimesBoldWidths.gperf"
                                                        { "U", 722 },
#line 27 "poppler/TimesBoldWidths.gperf"
                                                        { "onehalf", 750 },
#line 109 "poppler/TimesBoldWidths.gperf"
                                                        { "uhungarumlaut", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 147 "poppler/TimesBoldWidths.gperf"
                                                        { "Otilde", 778 },
                                                        { "", 0 },
#line 287 "poppler/TimesBoldWidths.gperf"
                                                        { "guillemotleft", 500 },
#line 202 "poppler/TimesBoldWidths.gperf"
                                                        { "guillemotright", 500 },
                                                        { "", 0 },
#line 247 "poppler/TimesBoldWidths.gperf"
                                                        { "Lacute", 667 },
#line 163 "poppler/TimesBoldWidths.gperf"
                                                        { "Umacron", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 18 "poppler/TimesBoldWidths.gperf"
                                                        { "Zacute", 667 },
                                                        { "", 0 },
#line 295 "poppler/TimesBoldWidths.gperf"
                                                        { "notequal", 549 },
#line 143 "poppler/TimesBoldWidths.gperf"
                                                        { "trademark", 1000 },
                                                        { "", 0 },
#line 265 "poppler/TimesBoldWidths.gperf"
                                                        { "Ncaron", 722 },
#line 200 "poppler/TimesBoldWidths.gperf"
                                                        { "Scommaaccent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 181 "poppler/TimesBoldWidths.gperf"
                                                        { "perthousand", 1000 },
                                                        { "", 0 },
#line 240 "poppler/TimesBoldWidths.gperf"
                                                        { "six", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 302 "poppler/TimesBoldWidths.gperf"
                                                        { "icircumflex", 278 },
                                                        { "", 0 },
#line 214 "poppler/TimesBoldWidths.gperf"
                                                        { "Scedilla", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 93 "poppler/TimesBoldWidths.gperf"
                                                        { "bullet", 350 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 108 "poppler/TimesBoldWidths.gperf"
                                                        { "q", 556 },
#line 289 "poppler/TimesBoldWidths.gperf"
                                                        { "Amacron", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 127 "poppler/TimesBoldWidths.gperf"
                                                        { "Idotaccent", 389 },
#line 141 "poppler/TimesBoldWidths.gperf"
                                                        { "Ecircumflex", 667 },
                                                        { "", 0 },
#line 315 "poppler/TimesBoldWidths.gperf"
                                                        { "fraction", 167 },
#line 180 "poppler/TimesBoldWidths.gperf"
                                                        { "Udieresis", 722 },
                                                        { "", 0 },
#line 176 "poppler/TimesBoldWidths.gperf"
                                                        { "ucircumflex", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 197 "poppler/TimesBoldWidths.gperf"
                                                        { "five", 500 },
                                                        { "", 0 },
#line 14 "poppler/TimesBoldWidths.gperf"
                                                        { "Ntilde", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 267 "poppler/TimesBoldWidths.gperf"
                                                        { "uring", 556 },
#line 45 "poppler/TimesBoldWidths.gperf"
                                                        { "A", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 84 "poppler/TimesBoldWidths.gperf"
                                                        { "four", 500 },
                                                        { "", 0 },
#line 187 "poppler/TimesBoldWidths.gperf"
                                                        { "egrave", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 241 "poppler/TimesBoldWidths.gperf"
                                                        { "paragraph", 540 },
                                                        { "", 0 },
#line 29 "poppler/TimesBoldWidths.gperf"
                                                        { "Lcaron", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 325 "poppler/TimesBoldWidths.gperf"
                                                        { "brokenbar", 220 },
                                                        { "", 0 },
#line 199 "poppler/TimesBoldWidths.gperf"
                                                        { "Zcaron", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 165 "poppler/TimesBoldWidths.gperf"
                                                        { "Adieresis", 722 },
                                                        { "", 0 },
#line 122 "poppler/TimesBoldWidths.gperf"
                                                        { "y", 500 },
#line 16 "poppler/TimesBoldWidths.gperf"
                                                        { "kcommaaccent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 76 "poppler/TimesBoldWidths.gperf"
                                                        { "agrave", 500 },
                                                        { "", 0 },
#line 69 "poppler/TimesBoldWidths.gperf"
                                                        { "Uhungarumlaut", 722 },
#line 204 "poppler/TimesBoldWidths.gperf"
                                                        { "ydieresis", 500 },
#line 168 "poppler/TimesBoldWidths.gperf"
                                                        { "slash", 278 },
#line 229 "poppler/TimesBoldWidths.gperf"
                                                        { "ogonek", 333 },
#line 151 "poppler/TimesBoldWidths.gperf"
                                                        { "AE", 1000 },
#line 192 "poppler/TimesBoldWidths.gperf"
                                                        { "asterisk", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 111 "poppler/TimesBoldWidths.gperf"
                                                        { "twosuperior", 300 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 53 "poppler/TimesBoldWidths.gperf"
                                                        { "G", 778 },
#line 58 "poppler/TimesBoldWidths.gperf"
                                                        { "iogonek", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 17 "poppler/TimesBoldWidths.gperf"
                                                        { "Ncommaaccent", 722 },
                                                        { "", 0 },
#line 21 "poppler/TimesBoldWidths.gperf"
                                                        { "plusminus", 570 },
                                                        { "", 0 },
#line 230 "poppler/TimesBoldWidths.gperf"
                                                        { "ograve", 500 },
                                                        { "", 0 },
#line 129 "poppler/TimesBoldWidths.gperf"
                                                        { "quotedbl", 555 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 233 "poppler/TimesBoldWidths.gperf"
                                                        { "Eogonek", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 120 "poppler/TimesBoldWidths.gperf"
                                                        { "w", 722 },
#line 259 "poppler/TimesBoldWidths.gperf"
                                                        { "uogonek", 556 },
                                                        { "", 0 },
#line 59 "poppler/TimesBoldWidths.gperf"
                                                        { "backslash", 278 },
#line 81 "poppler/TimesBoldWidths.gperf"
                                                        { "equal", 570 },
                                                        { "", 0 },
#line 38 "poppler/TimesBoldWidths.gperf"
                                                        { "Omacron", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 121 "poppler/TimesBoldWidths.gperf"
                                                        { "x", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 298 "poppler/TimesBoldWidths.gperf"
                                                        { "Zdotaccent", 667 },
#line 152 "poppler/TimesBoldWidths.gperf"
                                                        { "Ucircumflex", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 68 "poppler/TimesBoldWidths.gperf"
                                                        { "Q", 778 },
#line 296 "poppler/TimesBoldWidths.gperf"
                                                        { "Imacron", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 250 "poppler/TimesBoldWidths.gperf"
                                                        { "Uring", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 123 "poppler/TimesBoldWidths.gperf"
                                                        { "z", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 22 "poppler/TimesBoldWidths.gperf"
                                                        { "circumflex", 333 },
                                                        { "", 0 },
#line 251 "poppler/TimesBoldWidths.gperf"
                                                        { "Lcommaaccent", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 73 "poppler/TimesBoldWidths.gperf"
                                                        { "S", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 175 "poppler/TimesBoldWidths.gperf"
                                                        { "Odieresis", 778 },
                                                        { "", 0 },
#line 284 "poppler/TimesBoldWidths.gperf"
                                                        { "Acircumflex", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 67 "poppler/TimesBoldWidths.gperf"
                                                        { "P", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 238 "poppler/TimesBoldWidths.gperf"
                                                        { "Aring", 722 },
#line 96 "poppler/TimesBoldWidths.gperf"
                                                        { "Oslash", 778 },
#line 114 "poppler/TimesBoldWidths.gperf"
                                                        { "OE", 1000 },
                                                        { "", 0 },
#line 171 "poppler/TimesBoldWidths.gperf"
                                                        { "Idieresis", 389 },
                                                        { "", 0 },
#line 62 "poppler/TimesBoldWidths.gperf"
                                                        { "M", 944 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 282 "poppler/TimesBoldWidths.gperf"
                                                        { "fi", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 56 "poppler/TimesBoldWidths.gperf"
                                                        { "J", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 263 "poppler/TimesBoldWidths.gperf"
                                                        { "igrave", 278 },
                                                        { "", 0 },
#line 255 "poppler/TimesBoldWidths.gperf"
                                                        { "Ohungarumlaut", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 243 "poppler/TimesBoldWidths.gperf"
                                                        { "Uogonek", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 87 "poppler/TimesBoldWidths.gperf"
                                                        { "b", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 167 "poppler/TimesBoldWidths.gperf"
                                                        { "Egrave", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 182 "poppler/TimesBoldWidths.gperf"
                                                        { "Ydieresis", 722 },
                                                        { "", 0 },
#line 195 "poppler/TimesBoldWidths.gperf"
                                                        { "ugrave", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 211 "poppler/TimesBoldWidths.gperf"
                                                        { "multiply", 570 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 66 "poppler/TimesBoldWidths.gperf"
                                                        { "O", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 31 "poppler/TimesBoldWidths.gperf"
                                                        { "Aogonek", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 279 "poppler/TimesBoldWidths.gperf"
                                                        { "florin", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 226 "poppler/TimesBoldWidths.gperf"
                                                        { "Ocircumflex", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 283 "poppler/TimesBoldWidths.gperf"
                                                        { "fl", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 55 "poppler/TimesBoldWidths.gperf"
                                                        { "I", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 286 "poppler/TimesBoldWidths.gperf"
                                                        { "Icircumflex", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 54 "poppler/TimesBoldWidths.gperf"
                                                        { "H", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 134 "poppler/TimesBoldWidths.gperf"
                                                        { "Lslash", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 99 "poppler/TimesBoldWidths.gperf"
                                                        { "k", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 184 "poppler/TimesBoldWidths.gperf"
                                                        { "abreve", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 217 "poppler/TimesBoldWidths.gperf"
                                                        { "partialdiff", 494 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 52 "poppler/TimesBoldWidths.gperf"
                                                        { "F", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 178 "poppler/TimesBoldWidths.gperf"
                                                        { "Ugrave", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 91 "poppler/TimesBoldWidths.gperf"
                                                        { "f", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 119 "poppler/TimesBoldWidths.gperf"
                                                        { "v", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 63 "poppler/TimesBoldWidths.gperf"
                                                        { "N", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 41 "poppler/TimesBoldWidths.gperf"
                                                        { "Agrave", 722 },
#line 34 "poppler/TimesBoldWidths.gperf"
                                                        { "Iogonek", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 82 "poppler/TimesBoldWidths.gperf"
                                                        { "Y", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 46 "poppler/TimesBoldWidths.gperf"
                                                        { "B", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 177 "poppler/TimesBoldWidths.gperf"
                                                        { "bracketleft", 333 },
#line 260 "poppler/TimesBoldWidths.gperf"
                                                        { "bracketright", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 142 "poppler/TimesBoldWidths.gperf"
                                                        { "gbreve", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 215 "poppler/TimesBoldWidths.gperf"
                                                        { "Ograve", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 60 "poppler/TimesBoldWidths.gperf"
                                                        { "L", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 246 "poppler/TimesBoldWidths.gperf"
                                                        { "Igrave", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 83 "poppler/TimesBoldWidths.gperf"
                                                        { "Z", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 162 "poppler/TimesBoldWidths.gperf"
                                                        { "Abreve", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 124 "poppler/TimesBoldWidths.gperf"
                                                        { "Gbreve", 778 } };

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
#line 330 "poppler/TimesBoldWidths.gperf"
