/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf poppler/HelveticaObliqueWidths.gperf  */
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

#line 1 "poppler/HelveticaObliqueWidths.gperf"

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

const struct BuiltinFontWidth *HelveticaObliqueWidthsLookup(register const char *str, register size_t len)
{
    static const struct BuiltinFontWidth wordlist[] = { { "", 0 },
#line 90 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "e", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 70 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "R", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 57 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "K", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 49 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "D", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 115 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "t", 278 },
#line 191 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ae", 889 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 102 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "n", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 207 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "eacute", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 216 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Racute", 722 },
                                                        { "", 0 },
#line 85 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "a", 556 },
#line 206 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "at", 1015 },
                                                        { "", 0 },
#line 308 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "cent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 161 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "oe", 944 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 145 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "nacute", 556 },
                                                        { "", 0 },
#line 254 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Delta", 612 },
                                                        { "", 0 },
#line 273 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "acute", 333 },
#line 112 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "aacute", 556 },
#line 47 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "C", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 88 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "c", 500 },
                                                        { "", 0 },
#line 173 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "one", 556 },
#line 285 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Cacute", 722 },
                                                        { "", 0 },
#line 300 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "cacute", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 98 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "j", 222 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 210 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Dcroat", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 249 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "oacute", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 72 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "caron", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 104 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "o", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 317 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ecaron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 321 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Rcaron", 722 },
#line 290 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "seven", 556 },
#line 306 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "sacute", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 224 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Dcaron", 722 },
                                                        { "", 0 },
#line 252 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "tcaron", 317 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 26 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "colon", 278 },
#line 278 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ncaron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 125 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "commaaccent", 250 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 135 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "semicolon", 278 },
#line 19 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "comma", 278 },
#line 235 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "degree", 400 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 118 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Ccaron", 722 },
                                                        { "", 0 },
#line 140 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ccaron", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 113 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "s", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 231 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "racute", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 79 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "X", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 30 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ntilde", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 205 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "tilde", 333 },
#line 324 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "atilde", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 196 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "nine", 556 },
#line 24 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "edotaccent", 556 },
#line 105 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ordfeminine", 370 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 158 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "eight", 556 },
#line 150 "poppler/HelveticaObliqueWidths.gperf"
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
#line 276 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "iacute", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 170 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "otilde", 556 },
#line 292 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ordmasculine", 365 },
#line 213 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "eth", 556 },
                                                        { "", 0 },
#line 42 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "three", 556 },
#line 225 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "dcroat", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 281 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Rcommaaccent", 722 },
#line 185 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Eacute", 667 },
#line 322 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Kcommaaccent", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 218 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "uacute", 556 },
#line 103 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "tcommaaccent", 278 },
                                                        { "", 0 },
#line 166 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "copyright", 737 },
#line 43 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "numbersign", 556 },
#line 15 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "rcaron", 333 },
#line 32 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ncommaaccent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 110 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "r", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 264 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "lacute", 222 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 23 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "dotaccent", 333 },
#line 234 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "thorn", 556 },
#line 242 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "dcaron", 643 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 146 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "macron", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 203 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Ccedilla", 722 },
#line 274 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "section", 556 },
#line 223 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ccedilla", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 20 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "cedilla", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 25 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "asciitilde", 584 },
#line 89 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "d", 556 },
#line 239 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "percent", 889 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 288 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "germandbls", 611 },
                                                        { "", 0 },
#line 138 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "lozenge", 471 },
                                                        { "", 0 },
#line 316 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "less", 584 },
                                                        { "", 0 },
#line 97 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "dagger", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 258 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "grave", 333 },
#line 301 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Ecaron", 667 },
#line 222 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "scommaaccent", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 160 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "endash", 556 },
#line 174 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "emacron", 556 },
#line 201 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "threequarters", 834 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 232 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Tcaron", 611 },
                                                        { "", 0 },
#line 228 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "scedilla", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 101 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "m", 833 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 245 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "summation", 600 },
#line 310 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "logicalnot", 584 },
#line 44 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "lcaron", 299 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 172 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "parenleft", 333 },
#line 139 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "parenright", 333 },
#line 95 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "i", 222 },
#line 305 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "amacron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 194 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Uacute", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 208 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "underscore", 556 },
#line 92 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "g", 556 },
#line 297 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "rcommaaccent", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 37 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "space", 278 },
#line 28 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "dollar", 556 },
                                                        { "", 0 },
#line 272 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "threesuperior", 333 },
#line 188 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "edieresis", 556 },
#line 236 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "registered", 737 },
#line 78 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "W", 944 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 64 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "omacron", 556 },
#line 36 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "yen", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 50 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "E", 667 },
                                                        { "", 0 },
#line 293 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "dotlessi", 278 },
                                                        { "", 0 },
#line 327 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Edotaccent", 667 },
#line 71 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Aacute", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 186 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "adieresis", 556 },
                                                        { "", 0 },
#line 117 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "u", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 144 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "daggerdbl", 556 },
                                                        { "", 0 },
#line 280 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "yacute", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 74 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "T", 611 },
#line 130 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "gcommaaccent", 556 },
#line 275 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "dieresis", 333 },
                                                        { "", 0 },
#line 51 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "onequarter", 834 },
#line 328 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "onesuperior", 333 },
#line 237 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "radical", 453 },
#line 190 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Eth", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 94 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "h", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 193 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "odieresis", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 100 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "l", 222 },
#line 65 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Tcommaaccent", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 136 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "oslash", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 137 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "lessequal", 549 },
#line 159 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "exclamdown", 333 },
#line 35 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "zacute", 500 },
#line 269 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "lcommaaccent", 222 },
                                                        { "", 0 },
#line 209 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Euro", 556 },
                                                        { "", 0 },
#line 291 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Sacute", 667 },
#line 323 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "greater", 584 },
#line 244 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "two", 556 },
                                                        { "", 0 },
#line 220 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Thorn", 667 },
#line 256 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "asciicircum", 469 },
#line 126 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "hungarumlaut", 333 },
                                                        { "", 0 },
#line 212 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "zero", 556 },
                                                        { "", 0 },
#line 40 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "emdash", 1000 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 116 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "divide", 584 },
                                                        { "", 0 },
#line 271 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ohungarumlaut", 556 },
#line 262 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ampersand", 667 },
                                                        { "", 0 },
#line 164 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ecircumflex", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 106 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ring", 333 },
                                                        { "", 0 },
#line 320 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "period", 278 },
                                                        { "", 0 },
#line 318 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "guilsinglleft", 333 },
#line 155 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "guilsinglright", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 307 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "imacron", 278 },
                                                        { "", 0 },
#line 61 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "periodcentered", 278 },
                                                        { "", 0 },
#line 227 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Oacute", 778 },
                                                        { "", 0 },
#line 294 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "sterling", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 299 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "acircumflex", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 33 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "minus", 584 },
#line 312 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Atilde", 667 },
#line 148 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Emacron", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 257 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "aring", 556 },
#line 261 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Iacute", 278 },
#line 183 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "umacron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 221 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "zcaron", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 133 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Scaron", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 248 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ocircumflex", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 189 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "idieresis", 278 },
                                                        { "", 0 },
#line 157 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "quotesingle", 191 },
#line 277 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "quotedblbase", 333 },
                                                        { "", 0 },
#line 268 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "quotesinglbase", 222 },
                                                        { "", 0 },
#line 107 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "p", 556 },
#line 132 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "greaterequal", 549 },
                                                        { "", 0 },
#line 326 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "quoteleft", 222 },
#line 179 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "quoteright", 222 },
                                                        { "", 0 },
#line 154 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "quotedblleft", 333 },
#line 304 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "quotedblright", 333 },
#line 169 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Edieresis", 667 },
                                                        { "", 0 },
#line 128 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Nacute", 722 },
#line 131 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "mu", 556 },
                                                        { "", 0 },
#line 198 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "udieresis", 556 },
                                                        { "", 0 },
#line 270 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Yacute", 667 },
#line 253 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "eogonek", 556 },
#line 80 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "question", 556 },
                                                        { "", 0 },
#line 313 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "breve", 333 },
#line 77 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "V", 667 },
#line 39 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "questiondown", 611 },
                                                        { "", 0 },
#line 266 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "plus", 584 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 149 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ellipsis", 1000 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 319 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "exclam", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 219 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "braceleft", 334 },
#line 303 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "braceright", 334 },
#line 156 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "hyphen", 333 },
#line 48 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "aogonek", 556 },
#line 314 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "bar", 260 },
                                                        { "", 0 },
#line 311 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "zdotaccent", 500 },
#line 153 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "lslash", 222 },
#line 86 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Gcommaaccent", 778 },
#line 309 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "currency", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 75 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "U", 722 },
#line 27 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "onehalf", 834 },
#line 109 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "uhungarumlaut", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 147 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Otilde", 778 },
                                                        { "", 0 },
#line 287 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "guillemotleft", 556 },
#line 202 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "guillemotright", 556 },
                                                        { "", 0 },
#line 247 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Lacute", 556 },
#line 163 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Umacron", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 18 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Zacute", 611 },
                                                        { "", 0 },
#line 295 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "notequal", 549 },
#line 143 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "trademark", 1000 },
                                                        { "", 0 },
#line 265 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Ncaron", 722 },
#line 200 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Scommaaccent", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 181 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "perthousand", 1000 },
                                                        { "", 0 },
#line 240 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "six", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 302 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "icircumflex", 278 },
                                                        { "", 0 },
#line 214 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Scedilla", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 93 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "bullet", 350 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 108 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "q", 556 },
#line 289 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Amacron", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 127 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Idotaccent", 278 },
#line 141 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Ecircumflex", 667 },
                                                        { "", 0 },
#line 315 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "fraction", 167 },
#line 180 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Udieresis", 722 },
                                                        { "", 0 },
#line 176 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ucircumflex", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 197 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "five", 556 },
                                                        { "", 0 },
#line 14 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Ntilde", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 267 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "uring", 556 },
#line 45 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "A", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 84 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "four", 556 },
                                                        { "", 0 },
#line 187 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "egrave", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 241 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "paragraph", 537 },
                                                        { "", 0 },
#line 29 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Lcaron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 325 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "brokenbar", 260 },
                                                        { "", 0 },
#line 199 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Zcaron", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 165 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Adieresis", 667 },
                                                        { "", 0 },
#line 122 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "y", 500 },
#line 16 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "kcommaaccent", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 76 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "agrave", 556 },
                                                        { "", 0 },
#line 69 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Uhungarumlaut", 722 },
#line 204 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ydieresis", 500 },
#line 168 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "slash", 278 },
#line 229 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ogonek", 333 },
#line 151 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "AE", 1000 },
#line 192 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "asterisk", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 111 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "twosuperior", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 53 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "G", 778 },
#line 58 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "iogonek", 222 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 17 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Ncommaaccent", 722 },
                                                        { "", 0 },
#line 21 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "plusminus", 584 },
                                                        { "", 0 },
#line 230 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ograve", 556 },
                                                        { "", 0 },
#line 129 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "quotedbl", 355 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 233 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Eogonek", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 120 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "w", 722 },
#line 259 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "uogonek", 556 },
                                                        { "", 0 },
#line 59 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "backslash", 278 },
#line 81 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "equal", 584 },
                                                        { "", 0 },
#line 38 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Omacron", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 121 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "x", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 298 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Zdotaccent", 611 },
#line 152 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Ucircumflex", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 68 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Q", 778 },
#line 296 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Imacron", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 250 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Uring", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 123 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "z", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 22 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "circumflex", 333 },
                                                        { "", 0 },
#line 251 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Lcommaaccent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 73 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "S", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 175 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Odieresis", 778 },
                                                        { "", 0 },
#line 284 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Acircumflex", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 67 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "P", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 238 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Aring", 667 },
#line 96 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Oslash", 778 },
#line 114 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "OE", 1000 },
                                                        { "", 0 },
#line 171 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Idieresis", 278 },
                                                        { "", 0 },
#line 62 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "M", 833 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 282 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "fi", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 56 "poppler/HelveticaObliqueWidths.gperf"
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
#line 263 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "igrave", 278 },
                                                        { "", 0 },
#line 255 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Ohungarumlaut", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 243 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Uogonek", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 87 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "b", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 167 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Egrave", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 182 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Ydieresis", 667 },
                                                        { "", 0 },
#line 195 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "ugrave", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 211 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "multiply", 584 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 66 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "O", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 31 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Aogonek", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 279 "poppler/HelveticaObliqueWidths.gperf"
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
#line 226 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Ocircumflex", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 283 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "fl", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 55 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "I", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 286 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Icircumflex", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 54 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "H", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 134 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Lslash", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 99 "poppler/HelveticaObliqueWidths.gperf"
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
#line 184 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "abreve", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 217 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "partialdiff", 476 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 52 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "F", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 178 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Ugrave", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 91 "poppler/HelveticaObliqueWidths.gperf"
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
#line 119 "poppler/HelveticaObliqueWidths.gperf"
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
#line 63 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "N", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 41 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Agrave", 667 },
#line 34 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Iogonek", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 82 "poppler/HelveticaObliqueWidths.gperf"
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
#line 46 "poppler/HelveticaObliqueWidths.gperf"
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
#line 177 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "bracketleft", 278 },
#line 260 "poppler/HelveticaObliqueWidths.gperf"
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
#line 142 "poppler/HelveticaObliqueWidths.gperf"
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
#line 215 "poppler/HelveticaObliqueWidths.gperf"
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
#line 60 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "L", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 246 "poppler/HelveticaObliqueWidths.gperf"
                                                        { "Igrave", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 83 "poppler/HelveticaObliqueWidths.gperf"
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
#line 162 "poppler/HelveticaObliqueWidths.gperf"
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
#line 124 "poppler/HelveticaObliqueWidths.gperf"
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
#line 330 "poppler/HelveticaObliqueWidths.gperf"
