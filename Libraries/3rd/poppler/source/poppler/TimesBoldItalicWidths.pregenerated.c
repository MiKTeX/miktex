/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf poppler/TimesBoldItalicWidths.gperf  */
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

#line 1 "poppler/TimesBoldItalicWidths.gperf"

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

const struct BuiltinFontWidth *TimesBoldItalicWidthsLookup(register const char *str, register size_t len)
{
    static const struct BuiltinFontWidth wordlist[] = { { "", 0 },
#line 90 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "e", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 70 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "R", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 57 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "K", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 49 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "D", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 115 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "t", 278 },
#line 191 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ae", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 102 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "n", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 207 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "eacute", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 216 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Racute", 667 },
                                                        { "", 0 },
#line 85 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "a", 500 },
#line 206 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "at", 832 },
                                                        { "", 0 },
#line 308 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "cent", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 161 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "oe", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 145 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "nacute", 556 },
                                                        { "", 0 },
#line 254 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Delta", 612 },
                                                        { "", 0 },
#line 273 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "acute", 333 },
#line 112 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "aacute", 500 },
#line 47 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "C", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 88 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "c", 444 },
                                                        { "", 0 },
#line 173 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "one", 500 },
#line 285 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Cacute", 667 },
                                                        { "", 0 },
#line 300 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "cacute", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 98 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "j", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 210 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Dcroat", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 249 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "oacute", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 72 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "caron", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 104 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "o", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 317 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ecaron", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 321 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Rcaron", 667 },
#line 290 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "seven", 500 },
#line 306 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "sacute", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 224 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Dcaron", 722 },
                                                        { "", 0 },
#line 252 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "tcaron", 366 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 26 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "colon", 333 },
#line 278 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ncaron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 125 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "commaaccent", 250 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 135 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "semicolon", 333 },
#line 19 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "comma", 250 },
#line 235 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "degree", 400 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 118 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Ccaron", 667 },
                                                        { "", 0 },
#line 140 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ccaron", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 113 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "s", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 231 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "racute", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 79 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "X", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 30 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ntilde", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 205 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "tilde", 333 },
#line 324 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "atilde", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 196 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "nine", 500 },
#line 24 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "edotaccent", 444 },
#line 105 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ordfeminine", 266 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 158 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "eight", 500 },
#line 150 "poppler/TimesBoldItalicWidths.gperf"
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
#line 276 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "iacute", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 170 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "otilde", 500 },
#line 292 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ordmasculine", 300 },
#line 213 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "eth", 500 },
                                                        { "", 0 },
#line 42 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "three", 500 },
#line 225 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "dcroat", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 281 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Rcommaaccent", 667 },
#line 185 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Eacute", 667 },
#line 322 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Kcommaaccent", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 218 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "uacute", 556 },
#line 103 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "tcommaaccent", 278 },
                                                        { "", 0 },
#line 166 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "copyright", 747 },
#line 43 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "numbersign", 500 },
#line 15 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "rcaron", 389 },
#line 32 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ncommaaccent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 110 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "r", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 264 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "lacute", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 23 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "dotaccent", 333 },
#line 234 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "thorn", 500 },
#line 242 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "dcaron", 608 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 146 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "macron", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 203 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Ccedilla", 667 },
#line 274 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "section", 500 },
#line 223 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ccedilla", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 20 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "cedilla", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 25 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "asciitilde", 570 },
#line 89 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "d", 500 },
#line 239 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "percent", 833 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 288 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "germandbls", 500 },
                                                        { "", 0 },
#line 138 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "lozenge", 494 },
                                                        { "", 0 },
#line 316 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "less", 570 },
                                                        { "", 0 },
#line 97 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "dagger", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 258 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "grave", 333 },
#line 301 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Ecaron", 667 },
#line 222 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "scommaaccent", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 160 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "endash", 500 },
#line 174 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "emacron", 444 },
#line 201 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "threequarters", 750 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 232 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Tcaron", 611 },
                                                        { "", 0 },
#line 228 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "scedilla", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 101 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "m", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 245 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "summation", 600 },
#line 310 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "logicalnot", 606 },
#line 44 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "lcaron", 382 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 172 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "parenleft", 333 },
#line 139 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "parenright", 333 },
#line 95 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "i", 278 },
#line 305 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "amacron", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 194 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Uacute", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 208 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "underscore", 500 },
#line 92 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "g", 500 },
#line 297 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "rcommaaccent", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 37 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "space", 250 },
#line 28 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "dollar", 500 },
                                                        { "", 0 },
#line 272 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "threesuperior", 300 },
#line 188 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "edieresis", 444 },
#line 236 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "registered", 747 },
#line 78 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "W", 889 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 64 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "omacron", 500 },
#line 36 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "yen", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 50 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "E", 667 },
                                                        { "", 0 },
#line 293 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "dotlessi", 278 },
                                                        { "", 0 },
#line 327 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Edotaccent", 667 },
#line 71 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Aacute", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 186 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "adieresis", 500 },
                                                        { "", 0 },
#line 117 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "u", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 144 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "daggerdbl", 500 },
                                                        { "", 0 },
#line 280 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "yacute", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 74 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "T", 611 },
#line 130 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "gcommaaccent", 500 },
#line 275 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "dieresis", 333 },
                                                        { "", 0 },
#line 51 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "onequarter", 750 },
#line 328 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "onesuperior", 300 },
#line 237 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "radical", 549 },
#line 190 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Eth", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 94 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "h", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 193 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "odieresis", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 100 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "l", 278 },
#line 65 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Tcommaaccent", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 136 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "oslash", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 137 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "lessequal", 549 },
#line 159 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "exclamdown", 389 },
#line 35 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "zacute", 389 },
#line 269 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "lcommaaccent", 278 },
                                                        { "", 0 },
#line 209 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Euro", 500 },
                                                        { "", 0 },
#line 291 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Sacute", 556 },
#line 323 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "greater", 570 },
#line 244 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "two", 500 },
                                                        { "", 0 },
#line 220 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Thorn", 611 },
#line 256 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "asciicircum", 570 },
#line 126 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "hungarumlaut", 333 },
                                                        { "", 0 },
#line 212 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "zero", 500 },
                                                        { "", 0 },
#line 40 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "emdash", 1000 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 116 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "divide", 570 },
                                                        { "", 0 },
#line 271 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ohungarumlaut", 500 },
#line 262 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ampersand", 778 },
                                                        { "", 0 },
#line 164 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ecircumflex", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 106 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ring", 333 },
                                                        { "", 0 },
#line 320 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "period", 250 },
                                                        { "", 0 },
#line 318 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "guilsinglleft", 333 },
#line 155 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "guilsinglright", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 307 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "imacron", 278 },
                                                        { "", 0 },
#line 61 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "periodcentered", 250 },
                                                        { "", 0 },
#line 227 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Oacute", 722 },
                                                        { "", 0 },
#line 294 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "sterling", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 299 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "acircumflex", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 33 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "minus", 606 },
#line 312 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Atilde", 667 },
#line 148 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Emacron", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 257 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "aring", 500 },
#line 261 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Iacute", 389 },
#line 183 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "umacron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 221 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "zcaron", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 133 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Scaron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 248 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ocircumflex", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 189 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "idieresis", 278 },
                                                        { "", 0 },
#line 157 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "quotesingle", 278 },
#line 277 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "quotedblbase", 500 },
                                                        { "", 0 },
#line 268 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "quotesinglbase", 333 },
                                                        { "", 0 },
#line 107 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "p", 500 },
#line 132 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "greaterequal", 549 },
                                                        { "", 0 },
#line 326 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "quoteleft", 333 },
#line 179 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "quoteright", 333 },
                                                        { "", 0 },
#line 154 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "quotedblleft", 500 },
#line 304 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "quotedblright", 500 },
#line 169 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Edieresis", 667 },
                                                        { "", 0 },
#line 128 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Nacute", 722 },
#line 131 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "mu", 576 },
                                                        { "", 0 },
#line 198 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "udieresis", 556 },
                                                        { "", 0 },
#line 270 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Yacute", 611 },
#line 253 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "eogonek", 444 },
#line 80 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "question", 500 },
                                                        { "", 0 },
#line 313 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "breve", 333 },
#line 77 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "V", 667 },
#line 39 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "questiondown", 500 },
                                                        { "", 0 },
#line 266 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "plus", 570 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 149 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ellipsis", 1000 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 319 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "exclam", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 219 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "braceleft", 348 },
#line 303 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "braceright", 348 },
#line 156 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "hyphen", 333 },
#line 48 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "aogonek", 500 },
#line 314 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "bar", 220 },
                                                        { "", 0 },
#line 311 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "zdotaccent", 389 },
#line 153 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "lslash", 278 },
#line 86 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Gcommaaccent", 722 },
#line 309 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "currency", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 75 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "U", 722 },
#line 27 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "onehalf", 750 },
#line 109 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "uhungarumlaut", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 147 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Otilde", 722 },
                                                        { "", 0 },
#line 287 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "guillemotleft", 500 },
#line 202 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "guillemotright", 500 },
                                                        { "", 0 },
#line 247 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Lacute", 611 },
#line 163 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Umacron", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 18 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Zacute", 611 },
                                                        { "", 0 },
#line 295 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "notequal", 549 },
#line 143 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "trademark", 1000 },
                                                        { "", 0 },
#line 265 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Ncaron", 722 },
#line 200 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Scommaaccent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 181 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "perthousand", 1000 },
                                                        { "", 0 },
#line 240 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "six", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 302 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "icircumflex", 278 },
                                                        { "", 0 },
#line 214 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Scedilla", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 93 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "bullet", 350 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 108 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "q", 500 },
#line 289 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Amacron", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 127 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Idotaccent", 389 },
#line 141 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Ecircumflex", 667 },
                                                        { "", 0 },
#line 315 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "fraction", 167 },
#line 180 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Udieresis", 722 },
                                                        { "", 0 },
#line 176 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ucircumflex", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 197 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "five", 500 },
                                                        { "", 0 },
#line 14 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Ntilde", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 267 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "uring", 556 },
#line 45 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "A", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 84 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "four", 500 },
                                                        { "", 0 },
#line 187 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "egrave", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 241 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "paragraph", 500 },
                                                        { "", 0 },
#line 29 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Lcaron", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 325 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "brokenbar", 220 },
                                                        { "", 0 },
#line 199 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Zcaron", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 165 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Adieresis", 667 },
                                                        { "", 0 },
#line 122 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "y", 444 },
#line 16 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "kcommaaccent", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 76 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "agrave", 500 },
                                                        { "", 0 },
#line 69 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Uhungarumlaut", 722 },
#line 204 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ydieresis", 444 },
#line 168 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "slash", 278 },
#line 229 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ogonek", 333 },
#line 151 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "AE", 944 },
#line 192 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "asterisk", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 111 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "twosuperior", 300 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 53 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "G", 722 },
#line 58 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "iogonek", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 17 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Ncommaaccent", 722 },
                                                        { "", 0 },
#line 21 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "plusminus", 570 },
                                                        { "", 0 },
#line 230 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ograve", 500 },
                                                        { "", 0 },
#line 129 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "quotedbl", 555 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 233 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Eogonek", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 120 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "w", 667 },
#line 259 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "uogonek", 556 },
                                                        { "", 0 },
#line 59 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "backslash", 278 },
#line 81 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "equal", 570 },
                                                        { "", 0 },
#line 38 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Omacron", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 121 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "x", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 298 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Zdotaccent", 611 },
#line 152 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Ucircumflex", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 68 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Q", 722 },
#line 296 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Imacron", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 250 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Uring", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 123 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "z", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 22 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "circumflex", 333 },
                                                        { "", 0 },
#line 251 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Lcommaaccent", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 73 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "S", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 175 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Odieresis", 722 },
                                                        { "", 0 },
#line 284 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Acircumflex", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 67 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "P", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 238 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Aring", 667 },
#line 96 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Oslash", 722 },
#line 114 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "OE", 944 },
                                                        { "", 0 },
#line 171 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Idieresis", 389 },
                                                        { "", 0 },
#line 62 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "M", 889 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 282 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "fi", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 56 "poppler/TimesBoldItalicWidths.gperf"
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
#line 263 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "igrave", 278 },
                                                        { "", 0 },
#line 255 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Ohungarumlaut", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 243 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Uogonek", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 87 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "b", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 167 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Egrave", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 182 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Ydieresis", 611 },
                                                        { "", 0 },
#line 195 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "ugrave", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 211 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "multiply", 570 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 66 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "O", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 31 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Aogonek", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 279 "poppler/TimesBoldItalicWidths.gperf"
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
#line 226 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Ocircumflex", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 283 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "fl", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 55 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "I", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 286 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Icircumflex", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 54 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "H", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 134 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Lslash", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 99 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "k", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 184 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "abreve", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 217 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "partialdiff", 494 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 52 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "F", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 178 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Ugrave", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 91 "poppler/TimesBoldItalicWidths.gperf"
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
#line 119 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "v", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 63 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "N", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 41 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Agrave", 667 },
#line 34 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Iogonek", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 82 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Y", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 46 "poppler/TimesBoldItalicWidths.gperf"
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
#line 177 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "bracketleft", 333 },
#line 260 "poppler/TimesBoldItalicWidths.gperf"
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
#line 142 "poppler/TimesBoldItalicWidths.gperf"
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
#line 215 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Ograve", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 60 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "L", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 246 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Igrave", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 83 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Z", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 162 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Abreve", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 124 "poppler/TimesBoldItalicWidths.gperf"
                                                        { "Gbreve", 722 } };

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
#line 330 "poppler/TimesBoldItalicWidths.gperf"
