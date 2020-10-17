/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf poppler/HelveticaWidths.gperf  */
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

#line 1 "poppler/HelveticaWidths.gperf"

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

const struct BuiltinFontWidth *HelveticaWidthsLookup(register const char *str, register size_t len)
{
    static const struct BuiltinFontWidth wordlist[] = { { "", 0 },
#line 90 "poppler/HelveticaWidths.gperf"
                                                        { "e", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 70 "poppler/HelveticaWidths.gperf"
                                                        { "R", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 57 "poppler/HelveticaWidths.gperf"
                                                        { "K", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 49 "poppler/HelveticaWidths.gperf"
                                                        { "D", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 115 "poppler/HelveticaWidths.gperf"
                                                        { "t", 278 },
#line 191 "poppler/HelveticaWidths.gperf"
                                                        { "ae", 889 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 102 "poppler/HelveticaWidths.gperf"
                                                        { "n", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 207 "poppler/HelveticaWidths.gperf"
                                                        { "eacute", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 216 "poppler/HelveticaWidths.gperf"
                                                        { "Racute", 722 },
                                                        { "", 0 },
#line 85 "poppler/HelveticaWidths.gperf"
                                                        { "a", 556 },
#line 206 "poppler/HelveticaWidths.gperf"
                                                        { "at", 1015 },
                                                        { "", 0 },
#line 308 "poppler/HelveticaWidths.gperf"
                                                        { "cent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 161 "poppler/HelveticaWidths.gperf"
                                                        { "oe", 944 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 145 "poppler/HelveticaWidths.gperf"
                                                        { "nacute", 556 },
                                                        { "", 0 },
#line 254 "poppler/HelveticaWidths.gperf"
                                                        { "Delta", 612 },
                                                        { "", 0 },
#line 273 "poppler/HelveticaWidths.gperf"
                                                        { "acute", 333 },
#line 112 "poppler/HelveticaWidths.gperf"
                                                        { "aacute", 556 },
#line 47 "poppler/HelveticaWidths.gperf"
                                                        { "C", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 88 "poppler/HelveticaWidths.gperf"
                                                        { "c", 500 },
                                                        { "", 0 },
#line 173 "poppler/HelveticaWidths.gperf"
                                                        { "one", 556 },
#line 285 "poppler/HelveticaWidths.gperf"
                                                        { "Cacute", 722 },
                                                        { "", 0 },
#line 300 "poppler/HelveticaWidths.gperf"
                                                        { "cacute", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 98 "poppler/HelveticaWidths.gperf"
                                                        { "j", 222 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 210 "poppler/HelveticaWidths.gperf"
                                                        { "Dcroat", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 249 "poppler/HelveticaWidths.gperf"
                                                        { "oacute", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 72 "poppler/HelveticaWidths.gperf"
                                                        { "caron", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 104 "poppler/HelveticaWidths.gperf"
                                                        { "o", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 317 "poppler/HelveticaWidths.gperf"
                                                        { "ecaron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 321 "poppler/HelveticaWidths.gperf"
                                                        { "Rcaron", 722 },
#line 290 "poppler/HelveticaWidths.gperf"
                                                        { "seven", 556 },
#line 306 "poppler/HelveticaWidths.gperf"
                                                        { "sacute", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 224 "poppler/HelveticaWidths.gperf"
                                                        { "Dcaron", 722 },
                                                        { "", 0 },
#line 252 "poppler/HelveticaWidths.gperf"
                                                        { "tcaron", 317 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 26 "poppler/HelveticaWidths.gperf"
                                                        { "colon", 278 },
#line 278 "poppler/HelveticaWidths.gperf"
                                                        { "ncaron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 125 "poppler/HelveticaWidths.gperf"
                                                        { "commaaccent", 250 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 135 "poppler/HelveticaWidths.gperf"
                                                        { "semicolon", 278 },
#line 19 "poppler/HelveticaWidths.gperf"
                                                        { "comma", 278 },
#line 235 "poppler/HelveticaWidths.gperf"
                                                        { "degree", 400 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 118 "poppler/HelveticaWidths.gperf"
                                                        { "Ccaron", 722 },
                                                        { "", 0 },
#line 140 "poppler/HelveticaWidths.gperf"
                                                        { "ccaron", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 113 "poppler/HelveticaWidths.gperf"
                                                        { "s", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 231 "poppler/HelveticaWidths.gperf"
                                                        { "racute", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 79 "poppler/HelveticaWidths.gperf"
                                                        { "X", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 30 "poppler/HelveticaWidths.gperf"
                                                        { "ntilde", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 205 "poppler/HelveticaWidths.gperf"
                                                        { "tilde", 333 },
#line 324 "poppler/HelveticaWidths.gperf"
                                                        { "atilde", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 196 "poppler/HelveticaWidths.gperf"
                                                        { "nine", 556 },
#line 24 "poppler/HelveticaWidths.gperf"
                                                        { "edotaccent", 556 },
#line 105 "poppler/HelveticaWidths.gperf"
                                                        { "ordfeminine", 370 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 158 "poppler/HelveticaWidths.gperf"
                                                        { "eight", 556 },
#line 150 "poppler/HelveticaWidths.gperf"
                                                        { "scaron", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 276 "poppler/HelveticaWidths.gperf"
                                                        { "iacute", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 170 "poppler/HelveticaWidths.gperf"
                                                        { "otilde", 556 },
#line 292 "poppler/HelveticaWidths.gperf"
                                                        { "ordmasculine", 365 },
#line 213 "poppler/HelveticaWidths.gperf"
                                                        { "eth", 556 },
                                                        { "", 0 },
#line 42 "poppler/HelveticaWidths.gperf"
                                                        { "three", 556 },
#line 225 "poppler/HelveticaWidths.gperf"
                                                        { "dcroat", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 281 "poppler/HelveticaWidths.gperf"
                                                        { "Rcommaaccent", 722 },
#line 185 "poppler/HelveticaWidths.gperf"
                                                        { "Eacute", 667 },
#line 322 "poppler/HelveticaWidths.gperf"
                                                        { "Kcommaaccent", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 218 "poppler/HelveticaWidths.gperf"
                                                        { "uacute", 556 },
#line 103 "poppler/HelveticaWidths.gperf"
                                                        { "tcommaaccent", 278 },
                                                        { "", 0 },
#line 166 "poppler/HelveticaWidths.gperf"
                                                        { "copyright", 737 },
#line 43 "poppler/HelveticaWidths.gperf"
                                                        { "numbersign", 556 },
#line 15 "poppler/HelveticaWidths.gperf"
                                                        { "rcaron", 333 },
#line 32 "poppler/HelveticaWidths.gperf"
                                                        { "ncommaaccent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 110 "poppler/HelveticaWidths.gperf"
                                                        { "r", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 264 "poppler/HelveticaWidths.gperf"
                                                        { "lacute", 222 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 23 "poppler/HelveticaWidths.gperf"
                                                        { "dotaccent", 333 },
#line 234 "poppler/HelveticaWidths.gperf"
                                                        { "thorn", 556 },
#line 242 "poppler/HelveticaWidths.gperf"
                                                        { "dcaron", 643 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 146 "poppler/HelveticaWidths.gperf"
                                                        { "macron", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 203 "poppler/HelveticaWidths.gperf"
                                                        { "Ccedilla", 722 },
#line 274 "poppler/HelveticaWidths.gperf"
                                                        { "section", 556 },
#line 223 "poppler/HelveticaWidths.gperf"
                                                        { "ccedilla", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 20 "poppler/HelveticaWidths.gperf"
                                                        { "cedilla", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 25 "poppler/HelveticaWidths.gperf"
                                                        { "asciitilde", 584 },
#line 89 "poppler/HelveticaWidths.gperf"
                                                        { "d", 556 },
#line 239 "poppler/HelveticaWidths.gperf"
                                                        { "percent", 889 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 288 "poppler/HelveticaWidths.gperf"
                                                        { "germandbls", 611 },
                                                        { "", 0 },
#line 138 "poppler/HelveticaWidths.gperf"
                                                        { "lozenge", 471 },
                                                        { "", 0 },
#line 316 "poppler/HelveticaWidths.gperf"
                                                        { "less", 584 },
                                                        { "", 0 },
#line 97 "poppler/HelveticaWidths.gperf"
                                                        { "dagger", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 258 "poppler/HelveticaWidths.gperf"
                                                        { "grave", 333 },
#line 301 "poppler/HelveticaWidths.gperf"
                                                        { "Ecaron", 667 },
#line 222 "poppler/HelveticaWidths.gperf"
                                                        { "scommaaccent", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 160 "poppler/HelveticaWidths.gperf"
                                                        { "endash", 556 },
#line 174 "poppler/HelveticaWidths.gperf"
                                                        { "emacron", 556 },
#line 201 "poppler/HelveticaWidths.gperf"
                                                        { "threequarters", 834 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 232 "poppler/HelveticaWidths.gperf"
                                                        { "Tcaron", 611 },
                                                        { "", 0 },
#line 228 "poppler/HelveticaWidths.gperf"
                                                        { "scedilla", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 101 "poppler/HelveticaWidths.gperf"
                                                        { "m", 833 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 245 "poppler/HelveticaWidths.gperf"
                                                        { "summation", 600 },
#line 310 "poppler/HelveticaWidths.gperf"
                                                        { "logicalnot", 584 },
#line 44 "poppler/HelveticaWidths.gperf"
                                                        { "lcaron", 299 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 172 "poppler/HelveticaWidths.gperf"
                                                        { "parenleft", 333 },
#line 139 "poppler/HelveticaWidths.gperf"
                                                        { "parenright", 333 },
#line 95 "poppler/HelveticaWidths.gperf"
                                                        { "i", 222 },
#line 305 "poppler/HelveticaWidths.gperf"
                                                        { "amacron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 194 "poppler/HelveticaWidths.gperf"
                                                        { "Uacute", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 208 "poppler/HelveticaWidths.gperf"
                                                        { "underscore", 556 },
#line 92 "poppler/HelveticaWidths.gperf"
                                                        { "g", 556 },
#line 297 "poppler/HelveticaWidths.gperf"
                                                        { "rcommaaccent", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 37 "poppler/HelveticaWidths.gperf"
                                                        { "space", 278 },
#line 28 "poppler/HelveticaWidths.gperf"
                                                        { "dollar", 556 },
                                                        { "", 0 },
#line 272 "poppler/HelveticaWidths.gperf"
                                                        { "threesuperior", 333 },
#line 188 "poppler/HelveticaWidths.gperf"
                                                        { "edieresis", 556 },
#line 236 "poppler/HelveticaWidths.gperf"
                                                        { "registered", 737 },
#line 78 "poppler/HelveticaWidths.gperf"
                                                        { "W", 944 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 64 "poppler/HelveticaWidths.gperf"
                                                        { "omacron", 556 },
#line 36 "poppler/HelveticaWidths.gperf"
                                                        { "yen", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 50 "poppler/HelveticaWidths.gperf"
                                                        { "E", 667 },
                                                        { "", 0 },
#line 293 "poppler/HelveticaWidths.gperf"
                                                        { "dotlessi", 278 },
                                                        { "", 0 },
#line 327 "poppler/HelveticaWidths.gperf"
                                                        { "Edotaccent", 667 },
#line 71 "poppler/HelveticaWidths.gperf"
                                                        { "Aacute", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 186 "poppler/HelveticaWidths.gperf"
                                                        { "adieresis", 556 },
                                                        { "", 0 },
#line 117 "poppler/HelveticaWidths.gperf"
                                                        { "u", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 144 "poppler/HelveticaWidths.gperf"
                                                        { "daggerdbl", 556 },
                                                        { "", 0 },
#line 280 "poppler/HelveticaWidths.gperf"
                                                        { "yacute", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 74 "poppler/HelveticaWidths.gperf"
                                                        { "T", 611 },
#line 130 "poppler/HelveticaWidths.gperf"
                                                        { "gcommaaccent", 556 },
#line 275 "poppler/HelveticaWidths.gperf"
                                                        { "dieresis", 333 },
                                                        { "", 0 },
#line 51 "poppler/HelveticaWidths.gperf"
                                                        { "onequarter", 834 },
#line 328 "poppler/HelveticaWidths.gperf"
                                                        { "onesuperior", 333 },
#line 237 "poppler/HelveticaWidths.gperf"
                                                        { "radical", 453 },
#line 190 "poppler/HelveticaWidths.gperf"
                                                        { "Eth", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 94 "poppler/HelveticaWidths.gperf"
                                                        { "h", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 193 "poppler/HelveticaWidths.gperf"
                                                        { "odieresis", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 100 "poppler/HelveticaWidths.gperf"
                                                        { "l", 222 },
#line 65 "poppler/HelveticaWidths.gperf"
                                                        { "Tcommaaccent", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 136 "poppler/HelveticaWidths.gperf"
                                                        { "oslash", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 137 "poppler/HelveticaWidths.gperf"
                                                        { "lessequal", 549 },
#line 159 "poppler/HelveticaWidths.gperf"
                                                        { "exclamdown", 333 },
#line 35 "poppler/HelveticaWidths.gperf"
                                                        { "zacute", 500 },
#line 269 "poppler/HelveticaWidths.gperf"
                                                        { "lcommaaccent", 222 },
                                                        { "", 0 },
#line 209 "poppler/HelveticaWidths.gperf"
                                                        { "Euro", 556 },
                                                        { "", 0 },
#line 291 "poppler/HelveticaWidths.gperf"
                                                        { "Sacute", 667 },
#line 323 "poppler/HelveticaWidths.gperf"
                                                        { "greater", 584 },
#line 244 "poppler/HelveticaWidths.gperf"
                                                        { "two", 556 },
                                                        { "", 0 },
#line 220 "poppler/HelveticaWidths.gperf"
                                                        { "Thorn", 667 },
#line 256 "poppler/HelveticaWidths.gperf"
                                                        { "asciicircum", 469 },
#line 126 "poppler/HelveticaWidths.gperf"
                                                        { "hungarumlaut", 333 },
                                                        { "", 0 },
#line 212 "poppler/HelveticaWidths.gperf"
                                                        { "zero", 556 },
                                                        { "", 0 },
#line 40 "poppler/HelveticaWidths.gperf"
                                                        { "emdash", 1000 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 116 "poppler/HelveticaWidths.gperf"
                                                        { "divide", 584 },
                                                        { "", 0 },
#line 271 "poppler/HelveticaWidths.gperf"
                                                        { "ohungarumlaut", 556 },
#line 262 "poppler/HelveticaWidths.gperf"
                                                        { "ampersand", 667 },
                                                        { "", 0 },
#line 164 "poppler/HelveticaWidths.gperf"
                                                        { "ecircumflex", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 106 "poppler/HelveticaWidths.gperf"
                                                        { "ring", 333 },
                                                        { "", 0 },
#line 320 "poppler/HelveticaWidths.gperf"
                                                        { "period", 278 },
                                                        { "", 0 },
#line 318 "poppler/HelveticaWidths.gperf"
                                                        { "guilsinglleft", 333 },
#line 155 "poppler/HelveticaWidths.gperf"
                                                        { "guilsinglright", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 307 "poppler/HelveticaWidths.gperf"
                                                        { "imacron", 278 },
                                                        { "", 0 },
#line 61 "poppler/HelveticaWidths.gperf"
                                                        { "periodcentered", 278 },
                                                        { "", 0 },
#line 227 "poppler/HelveticaWidths.gperf"
                                                        { "Oacute", 778 },
                                                        { "", 0 },
#line 294 "poppler/HelveticaWidths.gperf"
                                                        { "sterling", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 299 "poppler/HelveticaWidths.gperf"
                                                        { "acircumflex", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 33 "poppler/HelveticaWidths.gperf"
                                                        { "minus", 584 },
#line 312 "poppler/HelveticaWidths.gperf"
                                                        { "Atilde", 667 },
#line 148 "poppler/HelveticaWidths.gperf"
                                                        { "Emacron", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 257 "poppler/HelveticaWidths.gperf"
                                                        { "aring", 556 },
#line 261 "poppler/HelveticaWidths.gperf"
                                                        { "Iacute", 278 },
#line 183 "poppler/HelveticaWidths.gperf"
                                                        { "umacron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 221 "poppler/HelveticaWidths.gperf"
                                                        { "zcaron", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 133 "poppler/HelveticaWidths.gperf"
                                                        { "Scaron", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 248 "poppler/HelveticaWidths.gperf"
                                                        { "ocircumflex", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 189 "poppler/HelveticaWidths.gperf"
                                                        { "idieresis", 278 },
                                                        { "", 0 },
#line 157 "poppler/HelveticaWidths.gperf"
                                                        { "quotesingle", 191 },
#line 277 "poppler/HelveticaWidths.gperf"
                                                        { "quotedblbase", 333 },
                                                        { "", 0 },
#line 268 "poppler/HelveticaWidths.gperf"
                                                        { "quotesinglbase", 222 },
                                                        { "", 0 },
#line 107 "poppler/HelveticaWidths.gperf"
                                                        { "p", 556 },
#line 132 "poppler/HelveticaWidths.gperf"
                                                        { "greaterequal", 549 },
                                                        { "", 0 },
#line 326 "poppler/HelveticaWidths.gperf"
                                                        { "quoteleft", 222 },
#line 179 "poppler/HelveticaWidths.gperf"
                                                        { "quoteright", 222 },
                                                        { "", 0 },
#line 154 "poppler/HelveticaWidths.gperf"
                                                        { "quotedblleft", 333 },
#line 304 "poppler/HelveticaWidths.gperf"
                                                        { "quotedblright", 333 },
#line 169 "poppler/HelveticaWidths.gperf"
                                                        { "Edieresis", 667 },
                                                        { "", 0 },
#line 128 "poppler/HelveticaWidths.gperf"
                                                        { "Nacute", 722 },
#line 131 "poppler/HelveticaWidths.gperf"
                                                        { "mu", 556 },
                                                        { "", 0 },
#line 198 "poppler/HelveticaWidths.gperf"
                                                        { "udieresis", 556 },
                                                        { "", 0 },
#line 270 "poppler/HelveticaWidths.gperf"
                                                        { "Yacute", 667 },
#line 253 "poppler/HelveticaWidths.gperf"
                                                        { "eogonek", 556 },
#line 80 "poppler/HelveticaWidths.gperf"
                                                        { "question", 556 },
                                                        { "", 0 },
#line 313 "poppler/HelveticaWidths.gperf"
                                                        { "breve", 333 },
#line 77 "poppler/HelveticaWidths.gperf"
                                                        { "V", 667 },
#line 39 "poppler/HelveticaWidths.gperf"
                                                        { "questiondown", 611 },
                                                        { "", 0 },
#line 266 "poppler/HelveticaWidths.gperf"
                                                        { "plus", 584 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 149 "poppler/HelveticaWidths.gperf"
                                                        { "ellipsis", 1000 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 319 "poppler/HelveticaWidths.gperf"
                                                        { "exclam", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 219 "poppler/HelveticaWidths.gperf"
                                                        { "braceleft", 334 },
#line 303 "poppler/HelveticaWidths.gperf"
                                                        { "braceright", 334 },
#line 156 "poppler/HelveticaWidths.gperf"
                                                        { "hyphen", 333 },
#line 48 "poppler/HelveticaWidths.gperf"
                                                        { "aogonek", 556 },
#line 314 "poppler/HelveticaWidths.gperf"
                                                        { "bar", 260 },
                                                        { "", 0 },
#line 311 "poppler/HelveticaWidths.gperf"
                                                        { "zdotaccent", 500 },
#line 153 "poppler/HelveticaWidths.gperf"
                                                        { "lslash", 222 },
#line 86 "poppler/HelveticaWidths.gperf"
                                                        { "Gcommaaccent", 778 },
#line 309 "poppler/HelveticaWidths.gperf"
                                                        { "currency", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 75 "poppler/HelveticaWidths.gperf"
                                                        { "U", 722 },
#line 27 "poppler/HelveticaWidths.gperf"
                                                        { "onehalf", 834 },
#line 109 "poppler/HelveticaWidths.gperf"
                                                        { "uhungarumlaut", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 147 "poppler/HelveticaWidths.gperf"
                                                        { "Otilde", 778 },
                                                        { "", 0 },
#line 287 "poppler/HelveticaWidths.gperf"
                                                        { "guillemotleft", 556 },
#line 202 "poppler/HelveticaWidths.gperf"
                                                        { "guillemotright", 556 },
                                                        { "", 0 },
#line 247 "poppler/HelveticaWidths.gperf"
                                                        { "Lacute", 556 },
#line 163 "poppler/HelveticaWidths.gperf"
                                                        { "Umacron", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 18 "poppler/HelveticaWidths.gperf"
                                                        { "Zacute", 611 },
                                                        { "", 0 },
#line 295 "poppler/HelveticaWidths.gperf"
                                                        { "notequal", 549 },
#line 143 "poppler/HelveticaWidths.gperf"
                                                        { "trademark", 1000 },
                                                        { "", 0 },
#line 265 "poppler/HelveticaWidths.gperf"
                                                        { "Ncaron", 722 },
#line 200 "poppler/HelveticaWidths.gperf"
                                                        { "Scommaaccent", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 181 "poppler/HelveticaWidths.gperf"
                                                        { "perthousand", 1000 },
                                                        { "", 0 },
#line 240 "poppler/HelveticaWidths.gperf"
                                                        { "six", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 302 "poppler/HelveticaWidths.gperf"
                                                        { "icircumflex", 278 },
                                                        { "", 0 },
#line 214 "poppler/HelveticaWidths.gperf"
                                                        { "Scedilla", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 93 "poppler/HelveticaWidths.gperf"
                                                        { "bullet", 350 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 108 "poppler/HelveticaWidths.gperf"
                                                        { "q", 556 },
#line 289 "poppler/HelveticaWidths.gperf"
                                                        { "Amacron", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 127 "poppler/HelveticaWidths.gperf"
                                                        { "Idotaccent", 278 },
#line 141 "poppler/HelveticaWidths.gperf"
                                                        { "Ecircumflex", 667 },
                                                        { "", 0 },
#line 315 "poppler/HelveticaWidths.gperf"
                                                        { "fraction", 167 },
#line 180 "poppler/HelveticaWidths.gperf"
                                                        { "Udieresis", 722 },
                                                        { "", 0 },
#line 176 "poppler/HelveticaWidths.gperf"
                                                        { "ucircumflex", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 197 "poppler/HelveticaWidths.gperf"
                                                        { "five", 556 },
                                                        { "", 0 },
#line 14 "poppler/HelveticaWidths.gperf"
                                                        { "Ntilde", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 267 "poppler/HelveticaWidths.gperf"
                                                        { "uring", 556 },
#line 45 "poppler/HelveticaWidths.gperf"
                                                        { "A", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 84 "poppler/HelveticaWidths.gperf"
                                                        { "four", 556 },
                                                        { "", 0 },
#line 187 "poppler/HelveticaWidths.gperf"
                                                        { "egrave", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 241 "poppler/HelveticaWidths.gperf"
                                                        { "paragraph", 537 },
                                                        { "", 0 },
#line 29 "poppler/HelveticaWidths.gperf"
                                                        { "Lcaron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 325 "poppler/HelveticaWidths.gperf"
                                                        { "brokenbar", 260 },
                                                        { "", 0 },
#line 199 "poppler/HelveticaWidths.gperf"
                                                        { "Zcaron", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 165 "poppler/HelveticaWidths.gperf"
                                                        { "Adieresis", 667 },
                                                        { "", 0 },
#line 122 "poppler/HelveticaWidths.gperf"
                                                        { "y", 500 },
#line 16 "poppler/HelveticaWidths.gperf"
                                                        { "kcommaaccent", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 76 "poppler/HelveticaWidths.gperf"
                                                        { "agrave", 556 },
                                                        { "", 0 },
#line 69 "poppler/HelveticaWidths.gperf"
                                                        { "Uhungarumlaut", 722 },
#line 204 "poppler/HelveticaWidths.gperf"
                                                        { "ydieresis", 500 },
#line 168 "poppler/HelveticaWidths.gperf"
                                                        { "slash", 278 },
#line 229 "poppler/HelveticaWidths.gperf"
                                                        { "ogonek", 333 },
#line 151 "poppler/HelveticaWidths.gperf"
                                                        { "AE", 1000 },
#line 192 "poppler/HelveticaWidths.gperf"
                                                        { "asterisk", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 111 "poppler/HelveticaWidths.gperf"
                                                        { "twosuperior", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 53 "poppler/HelveticaWidths.gperf"
                                                        { "G", 778 },
#line 58 "poppler/HelveticaWidths.gperf"
                                                        { "iogonek", 222 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 17 "poppler/HelveticaWidths.gperf"
                                                        { "Ncommaaccent", 722 },
                                                        { "", 0 },
#line 21 "poppler/HelveticaWidths.gperf"
                                                        { "plusminus", 584 },
                                                        { "", 0 },
#line 230 "poppler/HelveticaWidths.gperf"
                                                        { "ograve", 556 },
                                                        { "", 0 },
#line 129 "poppler/HelveticaWidths.gperf"
                                                        { "quotedbl", 355 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 233 "poppler/HelveticaWidths.gperf"
                                                        { "Eogonek", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 120 "poppler/HelveticaWidths.gperf"
                                                        { "w", 722 },
#line 259 "poppler/HelveticaWidths.gperf"
                                                        { "uogonek", 556 },
                                                        { "", 0 },
#line 59 "poppler/HelveticaWidths.gperf"
                                                        { "backslash", 278 },
#line 81 "poppler/HelveticaWidths.gperf"
                                                        { "equal", 584 },
                                                        { "", 0 },
#line 38 "poppler/HelveticaWidths.gperf"
                                                        { "Omacron", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 121 "poppler/HelveticaWidths.gperf"
                                                        { "x", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 298 "poppler/HelveticaWidths.gperf"
                                                        { "Zdotaccent", 611 },
#line 152 "poppler/HelveticaWidths.gperf"
                                                        { "Ucircumflex", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 68 "poppler/HelveticaWidths.gperf"
                                                        { "Q", 778 },
#line 296 "poppler/HelveticaWidths.gperf"
                                                        { "Imacron", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 250 "poppler/HelveticaWidths.gperf"
                                                        { "Uring", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 123 "poppler/HelveticaWidths.gperf"
                                                        { "z", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 22 "poppler/HelveticaWidths.gperf"
                                                        { "circumflex", 333 },
                                                        { "", 0 },
#line 251 "poppler/HelveticaWidths.gperf"
                                                        { "Lcommaaccent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 73 "poppler/HelveticaWidths.gperf"
                                                        { "S", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 175 "poppler/HelveticaWidths.gperf"
                                                        { "Odieresis", 778 },
                                                        { "", 0 },
#line 284 "poppler/HelveticaWidths.gperf"
                                                        { "Acircumflex", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 67 "poppler/HelveticaWidths.gperf"
                                                        { "P", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 238 "poppler/HelveticaWidths.gperf"
                                                        { "Aring", 667 },
#line 96 "poppler/HelveticaWidths.gperf"
                                                        { "Oslash", 778 },
#line 114 "poppler/HelveticaWidths.gperf"
                                                        { "OE", 1000 },
                                                        { "", 0 },
#line 171 "poppler/HelveticaWidths.gperf"
                                                        { "Idieresis", 278 },
                                                        { "", 0 },
#line 62 "poppler/HelveticaWidths.gperf"
                                                        { "M", 833 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 282 "poppler/HelveticaWidths.gperf"
                                                        { "fi", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 56 "poppler/HelveticaWidths.gperf"
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
#line 263 "poppler/HelveticaWidths.gperf"
                                                        { "igrave", 278 },
                                                        { "", 0 },
#line 255 "poppler/HelveticaWidths.gperf"
                                                        { "Ohungarumlaut", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 243 "poppler/HelveticaWidths.gperf"
                                                        { "Uogonek", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 87 "poppler/HelveticaWidths.gperf"
                                                        { "b", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 167 "poppler/HelveticaWidths.gperf"
                                                        { "Egrave", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 182 "poppler/HelveticaWidths.gperf"
                                                        { "Ydieresis", 667 },
                                                        { "", 0 },
#line 195 "poppler/HelveticaWidths.gperf"
                                                        { "ugrave", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 211 "poppler/HelveticaWidths.gperf"
                                                        { "multiply", 584 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 66 "poppler/HelveticaWidths.gperf"
                                                        { "O", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 31 "poppler/HelveticaWidths.gperf"
                                                        { "Aogonek", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 279 "poppler/HelveticaWidths.gperf"
                                                        { "florin", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 226 "poppler/HelveticaWidths.gperf"
                                                        { "Ocircumflex", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 283 "poppler/HelveticaWidths.gperf"
                                                        { "fl", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 55 "poppler/HelveticaWidths.gperf"
                                                        { "I", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 286 "poppler/HelveticaWidths.gperf"
                                                        { "Icircumflex", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 54 "poppler/HelveticaWidths.gperf"
                                                        { "H", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 134 "poppler/HelveticaWidths.gperf"
                                                        { "Lslash", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 99 "poppler/HelveticaWidths.gperf"
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
#line 184 "poppler/HelveticaWidths.gperf"
                                                        { "abreve", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 217 "poppler/HelveticaWidths.gperf"
                                                        { "partialdiff", 476 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 52 "poppler/HelveticaWidths.gperf"
                                                        { "F", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 178 "poppler/HelveticaWidths.gperf"
                                                        { "Ugrave", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 91 "poppler/HelveticaWidths.gperf"
                                                        { "f", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 119 "poppler/HelveticaWidths.gperf"
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
#line 63 "poppler/HelveticaWidths.gperf"
                                                        { "N", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 41 "poppler/HelveticaWidths.gperf"
                                                        { "Agrave", 667 },
#line 34 "poppler/HelveticaWidths.gperf"
                                                        { "Iogonek", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 82 "poppler/HelveticaWidths.gperf"
                                                        { "Y", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 46 "poppler/HelveticaWidths.gperf"
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
#line 177 "poppler/HelveticaWidths.gperf"
                                                        { "bracketleft", 278 },
#line 260 "poppler/HelveticaWidths.gperf"
                                                        { "bracketright", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 142 "poppler/HelveticaWidths.gperf"
                                                        { "gbreve", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 215 "poppler/HelveticaWidths.gperf"
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
#line 60 "poppler/HelveticaWidths.gperf"
                                                        { "L", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 246 "poppler/HelveticaWidths.gperf"
                                                        { "Igrave", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 83 "poppler/HelveticaWidths.gperf"
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
#line 162 "poppler/HelveticaWidths.gperf"
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
#line 124 "poppler/HelveticaWidths.gperf"
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
#line 330 "poppler/HelveticaWidths.gperf"
