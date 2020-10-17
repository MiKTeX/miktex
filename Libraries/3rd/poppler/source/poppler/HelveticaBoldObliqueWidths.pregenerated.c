/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf poppler/HelveticaBoldObliqueWidths.gperf  */
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

#line 1 "poppler/HelveticaBoldObliqueWidths.gperf"

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

const struct BuiltinFontWidth *HelveticaBoldObliqueWidthsLookup(register const char *str, register size_t len)
{
    static const struct BuiltinFontWidth wordlist[] = { { "", 0 },
#line 90 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "e", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 70 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "R", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 57 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "K", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 49 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "D", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 115 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "t", 333 },
#line 191 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ae", 889 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 102 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "n", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 207 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "eacute", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 216 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Racute", 722 },
                                                        { "", 0 },
#line 85 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "a", 556 },
#line 206 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "at", 975 },
                                                        { "", 0 },
#line 308 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "cent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 161 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "oe", 944 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 145 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "nacute", 611 },
                                                        { "", 0 },
#line 254 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Delta", 612 },
                                                        { "", 0 },
#line 273 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "acute", 333 },
#line 112 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "aacute", 556 },
#line 47 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "C", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 88 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "c", 556 },
                                                        { "", 0 },
#line 173 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "one", 556 },
#line 285 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Cacute", 722 },
                                                        { "", 0 },
#line 300 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "cacute", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 98 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "j", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 210 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Dcroat", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 249 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "oacute", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 72 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "caron", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 104 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "o", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 317 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ecaron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 321 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Rcaron", 722 },
#line 290 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "seven", 556 },
#line 306 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "sacute", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 224 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Dcaron", 722 },
                                                        { "", 0 },
#line 252 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "tcaron", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 26 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "colon", 333 },
#line 278 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ncaron", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 125 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "commaaccent", 250 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 135 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "semicolon", 333 },
#line 19 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "comma", 278 },
#line 235 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "degree", 400 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 118 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Ccaron", 722 },
                                                        { "", 0 },
#line 140 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ccaron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 113 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "s", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 231 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "racute", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 79 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "X", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 30 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ntilde", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 205 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "tilde", 333 },
#line 324 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "atilde", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 196 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "nine", 556 },
#line 24 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "edotaccent", 556 },
#line 105 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ordfeminine", 370 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 158 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "eight", 556 },
#line 150 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "scaron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 276 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "iacute", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 170 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "otilde", 611 },
#line 292 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ordmasculine", 365 },
#line 213 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "eth", 611 },
                                                        { "", 0 },
#line 42 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "three", 556 },
#line 225 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "dcroat", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 281 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Rcommaaccent", 722 },
#line 185 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Eacute", 667 },
#line 322 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Kcommaaccent", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 218 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "uacute", 611 },
#line 103 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "tcommaaccent", 333 },
                                                        { "", 0 },
#line 166 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "copyright", 737 },
#line 43 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "numbersign", 556 },
#line 15 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "rcaron", 389 },
#line 32 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ncommaaccent", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 110 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "r", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 264 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "lacute", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 23 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "dotaccent", 333 },
#line 234 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "thorn", 611 },
#line 242 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "dcaron", 743 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 146 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "macron", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 203 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Ccedilla", 722 },
#line 274 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "section", 556 },
#line 223 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ccedilla", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 20 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "cedilla", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 25 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "asciitilde", 584 },
#line 89 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "d", 611 },
#line 239 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "percent", 889 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 288 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "germandbls", 611 },
                                                        { "", 0 },
#line 138 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "lozenge", 494 },
                                                        { "", 0 },
#line 316 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "less", 584 },
                                                        { "", 0 },
#line 97 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "dagger", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 258 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "grave", 333 },
#line 301 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Ecaron", 667 },
#line 222 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "scommaaccent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 160 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "endash", 556 },
#line 174 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "emacron", 556 },
#line 201 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "threequarters", 834 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 232 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Tcaron", 611 },
                                                        { "", 0 },
#line 228 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "scedilla", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 101 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "m", 889 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 245 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "summation", 600 },
#line 310 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "logicalnot", 584 },
#line 44 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "lcaron", 400 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 172 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "parenleft", 333 },
#line 139 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "parenright", 333 },
#line 95 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "i", 278 },
#line 305 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "amacron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 194 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Uacute", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 208 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "underscore", 556 },
#line 92 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "g", 611 },
#line 297 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "rcommaaccent", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 37 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "space", 278 },
#line 28 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "dollar", 556 },
                                                        { "", 0 },
#line 272 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "threesuperior", 333 },
#line 188 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "edieresis", 556 },
#line 236 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "registered", 737 },
#line 78 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "W", 944 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 64 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "omacron", 611 },
#line 36 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "yen", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 50 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "E", 667 },
                                                        { "", 0 },
#line 293 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "dotlessi", 278 },
                                                        { "", 0 },
#line 327 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Edotaccent", 667 },
#line 71 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Aacute", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 186 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "adieresis", 556 },
                                                        { "", 0 },
#line 117 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "u", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 144 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "daggerdbl", 556 },
                                                        { "", 0 },
#line 280 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "yacute", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 74 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "T", 611 },
#line 130 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "gcommaaccent", 611 },
#line 275 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "dieresis", 333 },
                                                        { "", 0 },
#line 51 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "onequarter", 834 },
#line 328 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "onesuperior", 333 },
#line 237 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "radical", 549 },
#line 190 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Eth", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 94 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "h", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 193 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "odieresis", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 100 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "l", 278 },
#line 65 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Tcommaaccent", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 136 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "oslash", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 137 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "lessequal", 549 },
#line 159 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "exclamdown", 333 },
#line 35 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "zacute", 500 },
#line 269 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "lcommaaccent", 278 },
                                                        { "", 0 },
#line 209 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Euro", 556 },
                                                        { "", 0 },
#line 291 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Sacute", 667 },
#line 323 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "greater", 584 },
#line 244 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "two", 556 },
                                                        { "", 0 },
#line 220 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Thorn", 667 },
#line 256 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "asciicircum", 584 },
#line 126 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "hungarumlaut", 333 },
                                                        { "", 0 },
#line 212 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "zero", 556 },
                                                        { "", 0 },
#line 40 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "emdash", 1000 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 116 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "divide", 584 },
                                                        { "", 0 },
#line 271 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ohungarumlaut", 611 },
#line 262 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ampersand", 722 },
                                                        { "", 0 },
#line 164 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ecircumflex", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 106 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ring", 333 },
                                                        { "", 0 },
#line 320 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "period", 278 },
                                                        { "", 0 },
#line 318 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "guilsinglleft", 333 },
#line 155 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "guilsinglright", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 307 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "imacron", 278 },
                                                        { "", 0 },
#line 61 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "periodcentered", 278 },
                                                        { "", 0 },
#line 227 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Oacute", 778 },
                                                        { "", 0 },
#line 294 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "sterling", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 299 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "acircumflex", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 33 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "minus", 584 },
#line 312 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Atilde", 722 },
#line 148 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Emacron", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 257 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "aring", 556 },
#line 261 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Iacute", 278 },
#line 183 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "umacron", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 221 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "zcaron", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 133 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Scaron", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 248 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ocircumflex", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 189 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "idieresis", 278 },
                                                        { "", 0 },
#line 157 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "quotesingle", 238 },
#line 277 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "quotedblbase", 500 },
                                                        { "", 0 },
#line 268 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "quotesinglbase", 278 },
                                                        { "", 0 },
#line 107 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "p", 611 },
#line 132 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "greaterequal", 549 },
                                                        { "", 0 },
#line 326 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "quoteleft", 278 },
#line 179 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "quoteright", 278 },
                                                        { "", 0 },
#line 154 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "quotedblleft", 500 },
#line 304 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "quotedblright", 500 },
#line 169 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Edieresis", 667 },
                                                        { "", 0 },
#line 128 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Nacute", 722 },
#line 131 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "mu", 611 },
                                                        { "", 0 },
#line 198 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "udieresis", 611 },
                                                        { "", 0 },
#line 270 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Yacute", 667 },
#line 253 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "eogonek", 556 },
#line 80 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "question", 611 },
                                                        { "", 0 },
#line 313 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "breve", 333 },
#line 77 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "V", 667 },
#line 39 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "questiondown", 611 },
                                                        { "", 0 },
#line 266 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "plus", 584 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 149 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ellipsis", 1000 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 319 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "exclam", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 219 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "braceleft", 389 },
#line 303 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "braceright", 389 },
#line 156 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "hyphen", 333 },
#line 48 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "aogonek", 556 },
#line 314 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "bar", 280 },
                                                        { "", 0 },
#line 311 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "zdotaccent", 500 },
#line 153 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "lslash", 278 },
#line 86 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Gcommaaccent", 778 },
#line 309 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "currency", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 75 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "U", 722 },
#line 27 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "onehalf", 834 },
#line 109 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "uhungarumlaut", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 147 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Otilde", 778 },
                                                        { "", 0 },
#line 287 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "guillemotleft", 556 },
#line 202 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "guillemotright", 556 },
                                                        { "", 0 },
#line 247 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Lacute", 611 },
#line 163 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Umacron", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 18 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Zacute", 611 },
                                                        { "", 0 },
#line 295 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "notequal", 549 },
#line 143 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "trademark", 1000 },
                                                        { "", 0 },
#line 265 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Ncaron", 722 },
#line 200 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Scommaaccent", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 181 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "perthousand", 1000 },
                                                        { "", 0 },
#line 240 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "six", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 302 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "icircumflex", 278 },
                                                        { "", 0 },
#line 214 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Scedilla", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 93 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "bullet", 350 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 108 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "q", 611 },
#line 289 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Amacron", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 127 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Idotaccent", 278 },
#line 141 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Ecircumflex", 667 },
                                                        { "", 0 },
#line 315 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "fraction", 167 },
#line 180 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Udieresis", 722 },
                                                        { "", 0 },
#line 176 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ucircumflex", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 197 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "five", 556 },
                                                        { "", 0 },
#line 14 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Ntilde", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 267 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "uring", 611 },
#line 45 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "A", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 84 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "four", 556 },
                                                        { "", 0 },
#line 187 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "egrave", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 241 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "paragraph", 556 },
                                                        { "", 0 },
#line 29 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Lcaron", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 325 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "brokenbar", 280 },
                                                        { "", 0 },
#line 199 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Zcaron", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 165 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Adieresis", 722 },
                                                        { "", 0 },
#line 122 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "y", 556 },
#line 16 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "kcommaaccent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 76 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "agrave", 556 },
                                                        { "", 0 },
#line 69 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Uhungarumlaut", 722 },
#line 204 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ydieresis", 556 },
#line 168 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "slash", 278 },
#line 229 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ogonek", 333 },
#line 151 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "AE", 1000 },
#line 192 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "asterisk", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 111 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "twosuperior", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 53 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "G", 778 },
#line 58 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "iogonek", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 17 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Ncommaaccent", 722 },
                                                        { "", 0 },
#line 21 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "plusminus", 584 },
                                                        { "", 0 },
#line 230 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ograve", 611 },
                                                        { "", 0 },
#line 129 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "quotedbl", 474 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 233 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Eogonek", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 120 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "w", 778 },
#line 259 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "uogonek", 611 },
                                                        { "", 0 },
#line 59 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "backslash", 278 },
#line 81 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "equal", 584 },
                                                        { "", 0 },
#line 38 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Omacron", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 121 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "x", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 298 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Zdotaccent", 611 },
#line 152 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Ucircumflex", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 68 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Q", 778 },
#line 296 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Imacron", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 250 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Uring", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 123 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "z", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 22 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "circumflex", 333 },
                                                        { "", 0 },
#line 251 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Lcommaaccent", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 73 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "S", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 175 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Odieresis", 778 },
                                                        { "", 0 },
#line 284 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Acircumflex", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 67 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "P", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 238 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Aring", 722 },
#line 96 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Oslash", 778 },
#line 114 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "OE", 1000 },
                                                        { "", 0 },
#line 171 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Idieresis", 278 },
                                                        { "", 0 },
#line 62 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "M", 833 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 282 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "fi", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 56 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "J", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 263 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "igrave", 278 },
                                                        { "", 0 },
#line 255 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Ohungarumlaut", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 243 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Uogonek", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 87 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "b", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 167 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Egrave", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 182 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Ydieresis", 667 },
                                                        { "", 0 },
#line 195 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "ugrave", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 211 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "multiply", 584 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 66 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "O", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 31 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Aogonek", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 279 "poppler/HelveticaBoldObliqueWidths.gperf"
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
#line 226 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Ocircumflex", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 283 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "fl", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 55 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "I", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 286 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Icircumflex", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 54 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "H", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 134 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Lslash", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 99 "poppler/HelveticaBoldObliqueWidths.gperf"
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
#line 184 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "abreve", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 217 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "partialdiff", 494 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 52 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "F", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 178 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Ugrave", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 91 "poppler/HelveticaBoldObliqueWidths.gperf"
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
#line 119 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "v", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 63 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "N", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 41 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Agrave", 722 },
#line 34 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Iogonek", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 82 "poppler/HelveticaBoldObliqueWidths.gperf"
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
#line 46 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "B", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 177 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "bracketleft", 333 },
#line 260 "poppler/HelveticaBoldObliqueWidths.gperf"
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
#line 142 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "gbreve", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 215 "poppler/HelveticaBoldObliqueWidths.gperf"
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
#line 60 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "L", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 246 "poppler/HelveticaBoldObliqueWidths.gperf"
                                                        { "Igrave", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 83 "poppler/HelveticaBoldObliqueWidths.gperf"
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
#line 162 "poppler/HelveticaBoldObliqueWidths.gperf"
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
#line 124 "poppler/HelveticaBoldObliqueWidths.gperf"
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
#line 330 "poppler/HelveticaBoldObliqueWidths.gperf"
