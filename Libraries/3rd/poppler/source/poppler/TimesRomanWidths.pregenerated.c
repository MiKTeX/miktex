/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf poppler/TimesRomanWidths.gperf  */
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

#line 1 "poppler/TimesRomanWidths.gperf"

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

const struct BuiltinFontWidth *TimesRomanWidthsLookup(register const char *str, register size_t len)
{
    static const struct BuiltinFontWidth wordlist[] = { { "", 0 },
#line 90 "poppler/TimesRomanWidths.gperf"
                                                        { "e", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 70 "poppler/TimesRomanWidths.gperf"
                                                        { "R", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 57 "poppler/TimesRomanWidths.gperf"
                                                        { "K", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 49 "poppler/TimesRomanWidths.gperf"
                                                        { "D", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 115 "poppler/TimesRomanWidths.gperf"
                                                        { "t", 278 },
#line 191 "poppler/TimesRomanWidths.gperf"
                                                        { "ae", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 102 "poppler/TimesRomanWidths.gperf"
                                                        { "n", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 207 "poppler/TimesRomanWidths.gperf"
                                                        { "eacute", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 216 "poppler/TimesRomanWidths.gperf"
                                                        { "Racute", 667 },
                                                        { "", 0 },
#line 85 "poppler/TimesRomanWidths.gperf"
                                                        { "a", 444 },
#line 206 "poppler/TimesRomanWidths.gperf"
                                                        { "at", 921 },
                                                        { "", 0 },
#line 308 "poppler/TimesRomanWidths.gperf"
                                                        { "cent", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 161 "poppler/TimesRomanWidths.gperf"
                                                        { "oe", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 145 "poppler/TimesRomanWidths.gperf"
                                                        { "nacute", 500 },
                                                        { "", 0 },
#line 254 "poppler/TimesRomanWidths.gperf"
                                                        { "Delta", 612 },
                                                        { "", 0 },
#line 273 "poppler/TimesRomanWidths.gperf"
                                                        { "acute", 333 },
#line 112 "poppler/TimesRomanWidths.gperf"
                                                        { "aacute", 444 },
#line 47 "poppler/TimesRomanWidths.gperf"
                                                        { "C", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 88 "poppler/TimesRomanWidths.gperf"
                                                        { "c", 444 },
                                                        { "", 0 },
#line 173 "poppler/TimesRomanWidths.gperf"
                                                        { "one", 500 },
#line 285 "poppler/TimesRomanWidths.gperf"
                                                        { "Cacute", 667 },
                                                        { "", 0 },
#line 300 "poppler/TimesRomanWidths.gperf"
                                                        { "cacute", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 98 "poppler/TimesRomanWidths.gperf"
                                                        { "j", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 210 "poppler/TimesRomanWidths.gperf"
                                                        { "Dcroat", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 249 "poppler/TimesRomanWidths.gperf"
                                                        { "oacute", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 72 "poppler/TimesRomanWidths.gperf"
                                                        { "caron", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 104 "poppler/TimesRomanWidths.gperf"
                                                        { "o", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 317 "poppler/TimesRomanWidths.gperf"
                                                        { "ecaron", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 321 "poppler/TimesRomanWidths.gperf"
                                                        { "Rcaron", 667 },
#line 290 "poppler/TimesRomanWidths.gperf"
                                                        { "seven", 500 },
#line 306 "poppler/TimesRomanWidths.gperf"
                                                        { "sacute", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 224 "poppler/TimesRomanWidths.gperf"
                                                        { "Dcaron", 722 },
                                                        { "", 0 },
#line 252 "poppler/TimesRomanWidths.gperf"
                                                        { "tcaron", 326 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 26 "poppler/TimesRomanWidths.gperf"
                                                        { "colon", 278 },
#line 278 "poppler/TimesRomanWidths.gperf"
                                                        { "ncaron", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 125 "poppler/TimesRomanWidths.gperf"
                                                        { "commaaccent", 250 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 135 "poppler/TimesRomanWidths.gperf"
                                                        { "semicolon", 278 },
#line 19 "poppler/TimesRomanWidths.gperf"
                                                        { "comma", 250 },
#line 235 "poppler/TimesRomanWidths.gperf"
                                                        { "degree", 400 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 118 "poppler/TimesRomanWidths.gperf"
                                                        { "Ccaron", 667 },
                                                        { "", 0 },
#line 140 "poppler/TimesRomanWidths.gperf"
                                                        { "ccaron", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 113 "poppler/TimesRomanWidths.gperf"
                                                        { "s", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 231 "poppler/TimesRomanWidths.gperf"
                                                        { "racute", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 79 "poppler/TimesRomanWidths.gperf"
                                                        { "X", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 30 "poppler/TimesRomanWidths.gperf"
                                                        { "ntilde", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 205 "poppler/TimesRomanWidths.gperf"
                                                        { "tilde", 333 },
#line 324 "poppler/TimesRomanWidths.gperf"
                                                        { "atilde", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 196 "poppler/TimesRomanWidths.gperf"
                                                        { "nine", 500 },
#line 24 "poppler/TimesRomanWidths.gperf"
                                                        { "edotaccent", 444 },
#line 105 "poppler/TimesRomanWidths.gperf"
                                                        { "ordfeminine", 276 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 158 "poppler/TimesRomanWidths.gperf"
                                                        { "eight", 500 },
#line 150 "poppler/TimesRomanWidths.gperf"
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
#line 276 "poppler/TimesRomanWidths.gperf"
                                                        { "iacute", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 170 "poppler/TimesRomanWidths.gperf"
                                                        { "otilde", 500 },
#line 292 "poppler/TimesRomanWidths.gperf"
                                                        { "ordmasculine", 310 },
#line 213 "poppler/TimesRomanWidths.gperf"
                                                        { "eth", 500 },
                                                        { "", 0 },
#line 42 "poppler/TimesRomanWidths.gperf"
                                                        { "three", 500 },
#line 225 "poppler/TimesRomanWidths.gperf"
                                                        { "dcroat", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 281 "poppler/TimesRomanWidths.gperf"
                                                        { "Rcommaaccent", 667 },
#line 185 "poppler/TimesRomanWidths.gperf"
                                                        { "Eacute", 611 },
#line 322 "poppler/TimesRomanWidths.gperf"
                                                        { "Kcommaaccent", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 218 "poppler/TimesRomanWidths.gperf"
                                                        { "uacute", 500 },
#line 103 "poppler/TimesRomanWidths.gperf"
                                                        { "tcommaaccent", 278 },
                                                        { "", 0 },
#line 166 "poppler/TimesRomanWidths.gperf"
                                                        { "copyright", 760 },
#line 43 "poppler/TimesRomanWidths.gperf"
                                                        { "numbersign", 500 },
#line 15 "poppler/TimesRomanWidths.gperf"
                                                        { "rcaron", 333 },
#line 32 "poppler/TimesRomanWidths.gperf"
                                                        { "ncommaaccent", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 110 "poppler/TimesRomanWidths.gperf"
                                                        { "r", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 264 "poppler/TimesRomanWidths.gperf"
                                                        { "lacute", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 23 "poppler/TimesRomanWidths.gperf"
                                                        { "dotaccent", 333 },
#line 234 "poppler/TimesRomanWidths.gperf"
                                                        { "thorn", 500 },
#line 242 "poppler/TimesRomanWidths.gperf"
                                                        { "dcaron", 588 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 146 "poppler/TimesRomanWidths.gperf"
                                                        { "macron", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 203 "poppler/TimesRomanWidths.gperf"
                                                        { "Ccedilla", 667 },
#line 274 "poppler/TimesRomanWidths.gperf"
                                                        { "section", 500 },
#line 223 "poppler/TimesRomanWidths.gperf"
                                                        { "ccedilla", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 20 "poppler/TimesRomanWidths.gperf"
                                                        { "cedilla", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 25 "poppler/TimesRomanWidths.gperf"
                                                        { "asciitilde", 541 },
#line 89 "poppler/TimesRomanWidths.gperf"
                                                        { "d", 500 },
#line 239 "poppler/TimesRomanWidths.gperf"
                                                        { "percent", 833 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 288 "poppler/TimesRomanWidths.gperf"
                                                        { "germandbls", 500 },
                                                        { "", 0 },
#line 138 "poppler/TimesRomanWidths.gperf"
                                                        { "lozenge", 471 },
                                                        { "", 0 },
#line 316 "poppler/TimesRomanWidths.gperf"
                                                        { "less", 564 },
                                                        { "", 0 },
#line 97 "poppler/TimesRomanWidths.gperf"
                                                        { "dagger", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 258 "poppler/TimesRomanWidths.gperf"
                                                        { "grave", 333 },
#line 301 "poppler/TimesRomanWidths.gperf"
                                                        { "Ecaron", 611 },
#line 222 "poppler/TimesRomanWidths.gperf"
                                                        { "scommaaccent", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 160 "poppler/TimesRomanWidths.gperf"
                                                        { "endash", 500 },
#line 174 "poppler/TimesRomanWidths.gperf"
                                                        { "emacron", 444 },
#line 201 "poppler/TimesRomanWidths.gperf"
                                                        { "threequarters", 750 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 232 "poppler/TimesRomanWidths.gperf"
                                                        { "Tcaron", 611 },
                                                        { "", 0 },
#line 228 "poppler/TimesRomanWidths.gperf"
                                                        { "scedilla", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 101 "poppler/TimesRomanWidths.gperf"
                                                        { "m", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 245 "poppler/TimesRomanWidths.gperf"
                                                        { "summation", 600 },
#line 310 "poppler/TimesRomanWidths.gperf"
                                                        { "logicalnot", 564 },
#line 44 "poppler/TimesRomanWidths.gperf"
                                                        { "lcaron", 344 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 172 "poppler/TimesRomanWidths.gperf"
                                                        { "parenleft", 333 },
#line 139 "poppler/TimesRomanWidths.gperf"
                                                        { "parenright", 333 },
#line 95 "poppler/TimesRomanWidths.gperf"
                                                        { "i", 278 },
#line 305 "poppler/TimesRomanWidths.gperf"
                                                        { "amacron", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 194 "poppler/TimesRomanWidths.gperf"
                                                        { "Uacute", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 208 "poppler/TimesRomanWidths.gperf"
                                                        { "underscore", 500 },
#line 92 "poppler/TimesRomanWidths.gperf"
                                                        { "g", 500 },
#line 297 "poppler/TimesRomanWidths.gperf"
                                                        { "rcommaaccent", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 37 "poppler/TimesRomanWidths.gperf"
                                                        { "space", 250 },
#line 28 "poppler/TimesRomanWidths.gperf"
                                                        { "dollar", 500 },
                                                        { "", 0 },
#line 272 "poppler/TimesRomanWidths.gperf"
                                                        { "threesuperior", 300 },
#line 188 "poppler/TimesRomanWidths.gperf"
                                                        { "edieresis", 444 },
#line 236 "poppler/TimesRomanWidths.gperf"
                                                        { "registered", 760 },
#line 78 "poppler/TimesRomanWidths.gperf"
                                                        { "W", 944 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 64 "poppler/TimesRomanWidths.gperf"
                                                        { "omacron", 500 },
#line 36 "poppler/TimesRomanWidths.gperf"
                                                        { "yen", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 50 "poppler/TimesRomanWidths.gperf"
                                                        { "E", 611 },
                                                        { "", 0 },
#line 293 "poppler/TimesRomanWidths.gperf"
                                                        { "dotlessi", 278 },
                                                        { "", 0 },
#line 327 "poppler/TimesRomanWidths.gperf"
                                                        { "Edotaccent", 611 },
#line 71 "poppler/TimesRomanWidths.gperf"
                                                        { "Aacute", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 186 "poppler/TimesRomanWidths.gperf"
                                                        { "adieresis", 444 },
                                                        { "", 0 },
#line 117 "poppler/TimesRomanWidths.gperf"
                                                        { "u", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 144 "poppler/TimesRomanWidths.gperf"
                                                        { "daggerdbl", 500 },
                                                        { "", 0 },
#line 280 "poppler/TimesRomanWidths.gperf"
                                                        { "yacute", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 74 "poppler/TimesRomanWidths.gperf"
                                                        { "T", 611 },
#line 130 "poppler/TimesRomanWidths.gperf"
                                                        { "gcommaaccent", 500 },
#line 275 "poppler/TimesRomanWidths.gperf"
                                                        { "dieresis", 333 },
                                                        { "", 0 },
#line 51 "poppler/TimesRomanWidths.gperf"
                                                        { "onequarter", 750 },
#line 328 "poppler/TimesRomanWidths.gperf"
                                                        { "onesuperior", 300 },
#line 237 "poppler/TimesRomanWidths.gperf"
                                                        { "radical", 453 },
#line 190 "poppler/TimesRomanWidths.gperf"
                                                        { "Eth", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 94 "poppler/TimesRomanWidths.gperf"
                                                        { "h", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 193 "poppler/TimesRomanWidths.gperf"
                                                        { "odieresis", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 100 "poppler/TimesRomanWidths.gperf"
                                                        { "l", 278 },
#line 65 "poppler/TimesRomanWidths.gperf"
                                                        { "Tcommaaccent", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 136 "poppler/TimesRomanWidths.gperf"
                                                        { "oslash", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 137 "poppler/TimesRomanWidths.gperf"
                                                        { "lessequal", 549 },
#line 159 "poppler/TimesRomanWidths.gperf"
                                                        { "exclamdown", 333 },
#line 35 "poppler/TimesRomanWidths.gperf"
                                                        { "zacute", 444 },
#line 269 "poppler/TimesRomanWidths.gperf"
                                                        { "lcommaaccent", 278 },
                                                        { "", 0 },
#line 209 "poppler/TimesRomanWidths.gperf"
                                                        { "Euro", 500 },
                                                        { "", 0 },
#line 291 "poppler/TimesRomanWidths.gperf"
                                                        { "Sacute", 556 },
#line 323 "poppler/TimesRomanWidths.gperf"
                                                        { "greater", 564 },
#line 244 "poppler/TimesRomanWidths.gperf"
                                                        { "two", 500 },
                                                        { "", 0 },
#line 220 "poppler/TimesRomanWidths.gperf"
                                                        { "Thorn", 556 },
#line 256 "poppler/TimesRomanWidths.gperf"
                                                        { "asciicircum", 469 },
#line 126 "poppler/TimesRomanWidths.gperf"
                                                        { "hungarumlaut", 333 },
                                                        { "", 0 },
#line 212 "poppler/TimesRomanWidths.gperf"
                                                        { "zero", 500 },
                                                        { "", 0 },
#line 40 "poppler/TimesRomanWidths.gperf"
                                                        { "emdash", 1000 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 116 "poppler/TimesRomanWidths.gperf"
                                                        { "divide", 564 },
                                                        { "", 0 },
#line 271 "poppler/TimesRomanWidths.gperf"
                                                        { "ohungarumlaut", 500 },
#line 262 "poppler/TimesRomanWidths.gperf"
                                                        { "ampersand", 778 },
                                                        { "", 0 },
#line 164 "poppler/TimesRomanWidths.gperf"
                                                        { "ecircumflex", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 106 "poppler/TimesRomanWidths.gperf"
                                                        { "ring", 333 },
                                                        { "", 0 },
#line 320 "poppler/TimesRomanWidths.gperf"
                                                        { "period", 250 },
                                                        { "", 0 },
#line 318 "poppler/TimesRomanWidths.gperf"
                                                        { "guilsinglleft", 333 },
#line 155 "poppler/TimesRomanWidths.gperf"
                                                        { "guilsinglright", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 307 "poppler/TimesRomanWidths.gperf"
                                                        { "imacron", 278 },
                                                        { "", 0 },
#line 61 "poppler/TimesRomanWidths.gperf"
                                                        { "periodcentered", 250 },
                                                        { "", 0 },
#line 227 "poppler/TimesRomanWidths.gperf"
                                                        { "Oacute", 722 },
                                                        { "", 0 },
#line 294 "poppler/TimesRomanWidths.gperf"
                                                        { "sterling", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 299 "poppler/TimesRomanWidths.gperf"
                                                        { "acircumflex", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 33 "poppler/TimesRomanWidths.gperf"
                                                        { "minus", 564 },
#line 312 "poppler/TimesRomanWidths.gperf"
                                                        { "Atilde", 722 },
#line 148 "poppler/TimesRomanWidths.gperf"
                                                        { "Emacron", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 257 "poppler/TimesRomanWidths.gperf"
                                                        { "aring", 444 },
#line 261 "poppler/TimesRomanWidths.gperf"
                                                        { "Iacute", 333 },
#line 183 "poppler/TimesRomanWidths.gperf"
                                                        { "umacron", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 221 "poppler/TimesRomanWidths.gperf"
                                                        { "zcaron", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 133 "poppler/TimesRomanWidths.gperf"
                                                        { "Scaron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 248 "poppler/TimesRomanWidths.gperf"
                                                        { "ocircumflex", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 189 "poppler/TimesRomanWidths.gperf"
                                                        { "idieresis", 278 },
                                                        { "", 0 },
#line 157 "poppler/TimesRomanWidths.gperf"
                                                        { "quotesingle", 180 },
#line 277 "poppler/TimesRomanWidths.gperf"
                                                        { "quotedblbase", 444 },
                                                        { "", 0 },
#line 268 "poppler/TimesRomanWidths.gperf"
                                                        { "quotesinglbase", 333 },
                                                        { "", 0 },
#line 107 "poppler/TimesRomanWidths.gperf"
                                                        { "p", 500 },
#line 132 "poppler/TimesRomanWidths.gperf"
                                                        { "greaterequal", 549 },
                                                        { "", 0 },
#line 326 "poppler/TimesRomanWidths.gperf"
                                                        { "quoteleft", 333 },
#line 179 "poppler/TimesRomanWidths.gperf"
                                                        { "quoteright", 333 },
                                                        { "", 0 },
#line 154 "poppler/TimesRomanWidths.gperf"
                                                        { "quotedblleft", 444 },
#line 304 "poppler/TimesRomanWidths.gperf"
                                                        { "quotedblright", 444 },
#line 169 "poppler/TimesRomanWidths.gperf"
                                                        { "Edieresis", 611 },
                                                        { "", 0 },
#line 128 "poppler/TimesRomanWidths.gperf"
                                                        { "Nacute", 722 },
#line 131 "poppler/TimesRomanWidths.gperf"
                                                        { "mu", 500 },
                                                        { "", 0 },
#line 198 "poppler/TimesRomanWidths.gperf"
                                                        { "udieresis", 500 },
                                                        { "", 0 },
#line 270 "poppler/TimesRomanWidths.gperf"
                                                        { "Yacute", 722 },
#line 253 "poppler/TimesRomanWidths.gperf"
                                                        { "eogonek", 444 },
#line 80 "poppler/TimesRomanWidths.gperf"
                                                        { "question", 444 },
                                                        { "", 0 },
#line 313 "poppler/TimesRomanWidths.gperf"
                                                        { "breve", 333 },
#line 77 "poppler/TimesRomanWidths.gperf"
                                                        { "V", 722 },
#line 39 "poppler/TimesRomanWidths.gperf"
                                                        { "questiondown", 444 },
                                                        { "", 0 },
#line 266 "poppler/TimesRomanWidths.gperf"
                                                        { "plus", 564 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 149 "poppler/TimesRomanWidths.gperf"
                                                        { "ellipsis", 1000 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 319 "poppler/TimesRomanWidths.gperf"
                                                        { "exclam", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 219 "poppler/TimesRomanWidths.gperf"
                                                        { "braceleft", 480 },
#line 303 "poppler/TimesRomanWidths.gperf"
                                                        { "braceright", 480 },
#line 156 "poppler/TimesRomanWidths.gperf"
                                                        { "hyphen", 333 },
#line 48 "poppler/TimesRomanWidths.gperf"
                                                        { "aogonek", 444 },
#line 314 "poppler/TimesRomanWidths.gperf"
                                                        { "bar", 200 },
                                                        { "", 0 },
#line 311 "poppler/TimesRomanWidths.gperf"
                                                        { "zdotaccent", 444 },
#line 153 "poppler/TimesRomanWidths.gperf"
                                                        { "lslash", 278 },
#line 86 "poppler/TimesRomanWidths.gperf"
                                                        { "Gcommaaccent", 722 },
#line 309 "poppler/TimesRomanWidths.gperf"
                                                        { "currency", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 75 "poppler/TimesRomanWidths.gperf"
                                                        { "U", 722 },
#line 27 "poppler/TimesRomanWidths.gperf"
                                                        { "onehalf", 750 },
#line 109 "poppler/TimesRomanWidths.gperf"
                                                        { "uhungarumlaut", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 147 "poppler/TimesRomanWidths.gperf"
                                                        { "Otilde", 722 },
                                                        { "", 0 },
#line 287 "poppler/TimesRomanWidths.gperf"
                                                        { "guillemotleft", 500 },
#line 202 "poppler/TimesRomanWidths.gperf"
                                                        { "guillemotright", 500 },
                                                        { "", 0 },
#line 247 "poppler/TimesRomanWidths.gperf"
                                                        { "Lacute", 611 },
#line 163 "poppler/TimesRomanWidths.gperf"
                                                        { "Umacron", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 18 "poppler/TimesRomanWidths.gperf"
                                                        { "Zacute", 611 },
                                                        { "", 0 },
#line 295 "poppler/TimesRomanWidths.gperf"
                                                        { "notequal", 549 },
#line 143 "poppler/TimesRomanWidths.gperf"
                                                        { "trademark", 980 },
                                                        { "", 0 },
#line 265 "poppler/TimesRomanWidths.gperf"
                                                        { "Ncaron", 722 },
#line 200 "poppler/TimesRomanWidths.gperf"
                                                        { "Scommaaccent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 181 "poppler/TimesRomanWidths.gperf"
                                                        { "perthousand", 1000 },
                                                        { "", 0 },
#line 240 "poppler/TimesRomanWidths.gperf"
                                                        { "six", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 302 "poppler/TimesRomanWidths.gperf"
                                                        { "icircumflex", 278 },
                                                        { "", 0 },
#line 214 "poppler/TimesRomanWidths.gperf"
                                                        { "Scedilla", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 93 "poppler/TimesRomanWidths.gperf"
                                                        { "bullet", 350 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 108 "poppler/TimesRomanWidths.gperf"
                                                        { "q", 500 },
#line 289 "poppler/TimesRomanWidths.gperf"
                                                        { "Amacron", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 127 "poppler/TimesRomanWidths.gperf"
                                                        { "Idotaccent", 333 },
#line 141 "poppler/TimesRomanWidths.gperf"
                                                        { "Ecircumflex", 611 },
                                                        { "", 0 },
#line 315 "poppler/TimesRomanWidths.gperf"
                                                        { "fraction", 167 },
#line 180 "poppler/TimesRomanWidths.gperf"
                                                        { "Udieresis", 722 },
                                                        { "", 0 },
#line 176 "poppler/TimesRomanWidths.gperf"
                                                        { "ucircumflex", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 197 "poppler/TimesRomanWidths.gperf"
                                                        { "five", 500 },
                                                        { "", 0 },
#line 14 "poppler/TimesRomanWidths.gperf"
                                                        { "Ntilde", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 267 "poppler/TimesRomanWidths.gperf"
                                                        { "uring", 500 },
#line 45 "poppler/TimesRomanWidths.gperf"
                                                        { "A", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 84 "poppler/TimesRomanWidths.gperf"
                                                        { "four", 500 },
                                                        { "", 0 },
#line 187 "poppler/TimesRomanWidths.gperf"
                                                        { "egrave", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 241 "poppler/TimesRomanWidths.gperf"
                                                        { "paragraph", 453 },
                                                        { "", 0 },
#line 29 "poppler/TimesRomanWidths.gperf"
                                                        { "Lcaron", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 325 "poppler/TimesRomanWidths.gperf"
                                                        { "brokenbar", 200 },
                                                        { "", 0 },
#line 199 "poppler/TimesRomanWidths.gperf"
                                                        { "Zcaron", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 165 "poppler/TimesRomanWidths.gperf"
                                                        { "Adieresis", 722 },
                                                        { "", 0 },
#line 122 "poppler/TimesRomanWidths.gperf"
                                                        { "y", 500 },
#line 16 "poppler/TimesRomanWidths.gperf"
                                                        { "kcommaaccent", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 76 "poppler/TimesRomanWidths.gperf"
                                                        { "agrave", 444 },
                                                        { "", 0 },
#line 69 "poppler/TimesRomanWidths.gperf"
                                                        { "Uhungarumlaut", 722 },
#line 204 "poppler/TimesRomanWidths.gperf"
                                                        { "ydieresis", 500 },
#line 168 "poppler/TimesRomanWidths.gperf"
                                                        { "slash", 278 },
#line 229 "poppler/TimesRomanWidths.gperf"
                                                        { "ogonek", 333 },
#line 151 "poppler/TimesRomanWidths.gperf"
                                                        { "AE", 889 },
#line 192 "poppler/TimesRomanWidths.gperf"
                                                        { "asterisk", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 111 "poppler/TimesRomanWidths.gperf"
                                                        { "twosuperior", 300 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 53 "poppler/TimesRomanWidths.gperf"
                                                        { "G", 722 },
#line 58 "poppler/TimesRomanWidths.gperf"
                                                        { "iogonek", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 17 "poppler/TimesRomanWidths.gperf"
                                                        { "Ncommaaccent", 722 },
                                                        { "", 0 },
#line 21 "poppler/TimesRomanWidths.gperf"
                                                        { "plusminus", 564 },
                                                        { "", 0 },
#line 230 "poppler/TimesRomanWidths.gperf"
                                                        { "ograve", 500 },
                                                        { "", 0 },
#line 129 "poppler/TimesRomanWidths.gperf"
                                                        { "quotedbl", 408 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 233 "poppler/TimesRomanWidths.gperf"
                                                        { "Eogonek", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 120 "poppler/TimesRomanWidths.gperf"
                                                        { "w", 722 },
#line 259 "poppler/TimesRomanWidths.gperf"
                                                        { "uogonek", 500 },
                                                        { "", 0 },
#line 59 "poppler/TimesRomanWidths.gperf"
                                                        { "backslash", 278 },
#line 81 "poppler/TimesRomanWidths.gperf"
                                                        { "equal", 564 },
                                                        { "", 0 },
#line 38 "poppler/TimesRomanWidths.gperf"
                                                        { "Omacron", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 121 "poppler/TimesRomanWidths.gperf"
                                                        { "x", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 298 "poppler/TimesRomanWidths.gperf"
                                                        { "Zdotaccent", 611 },
#line 152 "poppler/TimesRomanWidths.gperf"
                                                        { "Ucircumflex", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 68 "poppler/TimesRomanWidths.gperf"
                                                        { "Q", 722 },
#line 296 "poppler/TimesRomanWidths.gperf"
                                                        { "Imacron", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 250 "poppler/TimesRomanWidths.gperf"
                                                        { "Uring", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 123 "poppler/TimesRomanWidths.gperf"
                                                        { "z", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 22 "poppler/TimesRomanWidths.gperf"
                                                        { "circumflex", 333 },
                                                        { "", 0 },
#line 251 "poppler/TimesRomanWidths.gperf"
                                                        { "Lcommaaccent", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 73 "poppler/TimesRomanWidths.gperf"
                                                        { "S", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 175 "poppler/TimesRomanWidths.gperf"
                                                        { "Odieresis", 722 },
                                                        { "", 0 },
#line 284 "poppler/TimesRomanWidths.gperf"
                                                        { "Acircumflex", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 67 "poppler/TimesRomanWidths.gperf"
                                                        { "P", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 238 "poppler/TimesRomanWidths.gperf"
                                                        { "Aring", 722 },
#line 96 "poppler/TimesRomanWidths.gperf"
                                                        { "Oslash", 722 },
#line 114 "poppler/TimesRomanWidths.gperf"
                                                        { "OE", 889 },
                                                        { "", 0 },
#line 171 "poppler/TimesRomanWidths.gperf"
                                                        { "Idieresis", 333 },
                                                        { "", 0 },
#line 62 "poppler/TimesRomanWidths.gperf"
                                                        { "M", 889 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 282 "poppler/TimesRomanWidths.gperf"
                                                        { "fi", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 56 "poppler/TimesRomanWidths.gperf"
                                                        { "J", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 263 "poppler/TimesRomanWidths.gperf"
                                                        { "igrave", 278 },
                                                        { "", 0 },
#line 255 "poppler/TimesRomanWidths.gperf"
                                                        { "Ohungarumlaut", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 243 "poppler/TimesRomanWidths.gperf"
                                                        { "Uogonek", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 87 "poppler/TimesRomanWidths.gperf"
                                                        { "b", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 167 "poppler/TimesRomanWidths.gperf"
                                                        { "Egrave", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 182 "poppler/TimesRomanWidths.gperf"
                                                        { "Ydieresis", 722 },
                                                        { "", 0 },
#line 195 "poppler/TimesRomanWidths.gperf"
                                                        { "ugrave", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 211 "poppler/TimesRomanWidths.gperf"
                                                        { "multiply", 564 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 66 "poppler/TimesRomanWidths.gperf"
                                                        { "O", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 31 "poppler/TimesRomanWidths.gperf"
                                                        { "Aogonek", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 279 "poppler/TimesRomanWidths.gperf"
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
#line 226 "poppler/TimesRomanWidths.gperf"
                                                        { "Ocircumflex", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 283 "poppler/TimesRomanWidths.gperf"
                                                        { "fl", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 55 "poppler/TimesRomanWidths.gperf"
                                                        { "I", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 286 "poppler/TimesRomanWidths.gperf"
                                                        { "Icircumflex", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 54 "poppler/TimesRomanWidths.gperf"
                                                        { "H", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 134 "poppler/TimesRomanWidths.gperf"
                                                        { "Lslash", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 99 "poppler/TimesRomanWidths.gperf"
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
#line 184 "poppler/TimesRomanWidths.gperf"
                                                        { "abreve", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 217 "poppler/TimesRomanWidths.gperf"
                                                        { "partialdiff", 476 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 52 "poppler/TimesRomanWidths.gperf"
                                                        { "F", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 178 "poppler/TimesRomanWidths.gperf"
                                                        { "Ugrave", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 91 "poppler/TimesRomanWidths.gperf"
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
#line 119 "poppler/TimesRomanWidths.gperf"
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
#line 63 "poppler/TimesRomanWidths.gperf"
                                                        { "N", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 41 "poppler/TimesRomanWidths.gperf"
                                                        { "Agrave", 722 },
#line 34 "poppler/TimesRomanWidths.gperf"
                                                        { "Iogonek", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 82 "poppler/TimesRomanWidths.gperf"
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
#line 46 "poppler/TimesRomanWidths.gperf"
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
#line 177 "poppler/TimesRomanWidths.gperf"
                                                        { "bracketleft", 333 },
#line 260 "poppler/TimesRomanWidths.gperf"
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
#line 142 "poppler/TimesRomanWidths.gperf"
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
#line 215 "poppler/TimesRomanWidths.gperf"
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
#line 60 "poppler/TimesRomanWidths.gperf"
                                                        { "L", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 246 "poppler/TimesRomanWidths.gperf"
                                                        { "Igrave", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 83 "poppler/TimesRomanWidths.gperf"
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
#line 162 "poppler/TimesRomanWidths.gperf"
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
#line 124 "poppler/TimesRomanWidths.gperf"
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
#line 330 "poppler/TimesRomanWidths.gperf"
