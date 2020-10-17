/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf poppler/TimesItalicWidths.gperf  */
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

#line 1 "poppler/TimesItalicWidths.gperf"

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

const struct BuiltinFontWidth *TimesItalicWidthsLookup(register const char *str, register size_t len)
{
    static const struct BuiltinFontWidth wordlist[] = { { "", 0 },
#line 90 "poppler/TimesItalicWidths.gperf"
                                                        { "e", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 70 "poppler/TimesItalicWidths.gperf"
                                                        { "R", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 57 "poppler/TimesItalicWidths.gperf"
                                                        { "K", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 49 "poppler/TimesItalicWidths.gperf"
                                                        { "D", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 115 "poppler/TimesItalicWidths.gperf"
                                                        { "t", 278 },
#line 191 "poppler/TimesItalicWidths.gperf"
                                                        { "ae", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 102 "poppler/TimesItalicWidths.gperf"
                                                        { "n", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 207 "poppler/TimesItalicWidths.gperf"
                                                        { "eacute", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 216 "poppler/TimesItalicWidths.gperf"
                                                        { "Racute", 611 },
                                                        { "", 0 },
#line 85 "poppler/TimesItalicWidths.gperf"
                                                        { "a", 500 },
#line 206 "poppler/TimesItalicWidths.gperf"
                                                        { "at", 920 },
                                                        { "", 0 },
#line 308 "poppler/TimesItalicWidths.gperf"
                                                        { "cent", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 161 "poppler/TimesItalicWidths.gperf"
                                                        { "oe", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 145 "poppler/TimesItalicWidths.gperf"
                                                        { "nacute", 500 },
                                                        { "", 0 },
#line 254 "poppler/TimesItalicWidths.gperf"
                                                        { "Delta", 612 },
                                                        { "", 0 },
#line 273 "poppler/TimesItalicWidths.gperf"
                                                        { "acute", 333 },
#line 112 "poppler/TimesItalicWidths.gperf"
                                                        { "aacute", 500 },
#line 47 "poppler/TimesItalicWidths.gperf"
                                                        { "C", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 88 "poppler/TimesItalicWidths.gperf"
                                                        { "c", 444 },
                                                        { "", 0 },
#line 173 "poppler/TimesItalicWidths.gperf"
                                                        { "one", 500 },
#line 285 "poppler/TimesItalicWidths.gperf"
                                                        { "Cacute", 667 },
                                                        { "", 0 },
#line 300 "poppler/TimesItalicWidths.gperf"
                                                        { "cacute", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 98 "poppler/TimesItalicWidths.gperf"
                                                        { "j", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 210 "poppler/TimesItalicWidths.gperf"
                                                        { "Dcroat", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 249 "poppler/TimesItalicWidths.gperf"
                                                        { "oacute", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 72 "poppler/TimesItalicWidths.gperf"
                                                        { "caron", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 104 "poppler/TimesItalicWidths.gperf"
                                                        { "o", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 317 "poppler/TimesItalicWidths.gperf"
                                                        { "ecaron", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 321 "poppler/TimesItalicWidths.gperf"
                                                        { "Rcaron", 611 },
#line 290 "poppler/TimesItalicWidths.gperf"
                                                        { "seven", 500 },
#line 306 "poppler/TimesItalicWidths.gperf"
                                                        { "sacute", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 224 "poppler/TimesItalicWidths.gperf"
                                                        { "Dcaron", 722 },
                                                        { "", 0 },
#line 252 "poppler/TimesItalicWidths.gperf"
                                                        { "tcaron", 300 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 26 "poppler/TimesItalicWidths.gperf"
                                                        { "colon", 333 },
#line 278 "poppler/TimesItalicWidths.gperf"
                                                        { "ncaron", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 125 "poppler/TimesItalicWidths.gperf"
                                                        { "commaaccent", 250 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 135 "poppler/TimesItalicWidths.gperf"
                                                        { "semicolon", 333 },
#line 19 "poppler/TimesItalicWidths.gperf"
                                                        { "comma", 250 },
#line 235 "poppler/TimesItalicWidths.gperf"
                                                        { "degree", 400 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 118 "poppler/TimesItalicWidths.gperf"
                                                        { "Ccaron", 667 },
                                                        { "", 0 },
#line 140 "poppler/TimesItalicWidths.gperf"
                                                        { "ccaron", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 113 "poppler/TimesItalicWidths.gperf"
                                                        { "s", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 231 "poppler/TimesItalicWidths.gperf"
                                                        { "racute", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 79 "poppler/TimesItalicWidths.gperf"
                                                        { "X", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 30 "poppler/TimesItalicWidths.gperf"
                                                        { "ntilde", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 205 "poppler/TimesItalicWidths.gperf"
                                                        { "tilde", 333 },
#line 324 "poppler/TimesItalicWidths.gperf"
                                                        { "atilde", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 196 "poppler/TimesItalicWidths.gperf"
                                                        { "nine", 500 },
#line 24 "poppler/TimesItalicWidths.gperf"
                                                        { "edotaccent", 444 },
#line 105 "poppler/TimesItalicWidths.gperf"
                                                        { "ordfeminine", 276 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 158 "poppler/TimesItalicWidths.gperf"
                                                        { "eight", 500 },
#line 150 "poppler/TimesItalicWidths.gperf"
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
#line 276 "poppler/TimesItalicWidths.gperf"
                                                        { "iacute", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 170 "poppler/TimesItalicWidths.gperf"
                                                        { "otilde", 500 },
#line 292 "poppler/TimesItalicWidths.gperf"
                                                        { "ordmasculine", 310 },
#line 213 "poppler/TimesItalicWidths.gperf"
                                                        { "eth", 500 },
                                                        { "", 0 },
#line 42 "poppler/TimesItalicWidths.gperf"
                                                        { "three", 500 },
#line 225 "poppler/TimesItalicWidths.gperf"
                                                        { "dcroat", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 281 "poppler/TimesItalicWidths.gperf"
                                                        { "Rcommaaccent", 611 },
#line 185 "poppler/TimesItalicWidths.gperf"
                                                        { "Eacute", 611 },
#line 322 "poppler/TimesItalicWidths.gperf"
                                                        { "Kcommaaccent", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 218 "poppler/TimesItalicWidths.gperf"
                                                        { "uacute", 500 },
#line 103 "poppler/TimesItalicWidths.gperf"
                                                        { "tcommaaccent", 278 },
                                                        { "", 0 },
#line 166 "poppler/TimesItalicWidths.gperf"
                                                        { "copyright", 760 },
#line 43 "poppler/TimesItalicWidths.gperf"
                                                        { "numbersign", 500 },
#line 15 "poppler/TimesItalicWidths.gperf"
                                                        { "rcaron", 389 },
#line 32 "poppler/TimesItalicWidths.gperf"
                                                        { "ncommaaccent", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 110 "poppler/TimesItalicWidths.gperf"
                                                        { "r", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 264 "poppler/TimesItalicWidths.gperf"
                                                        { "lacute", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 23 "poppler/TimesItalicWidths.gperf"
                                                        { "dotaccent", 333 },
#line 234 "poppler/TimesItalicWidths.gperf"
                                                        { "thorn", 500 },
#line 242 "poppler/TimesItalicWidths.gperf"
                                                        { "dcaron", 544 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 146 "poppler/TimesItalicWidths.gperf"
                                                        { "macron", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 203 "poppler/TimesItalicWidths.gperf"
                                                        { "Ccedilla", 667 },
#line 274 "poppler/TimesItalicWidths.gperf"
                                                        { "section", 500 },
#line 223 "poppler/TimesItalicWidths.gperf"
                                                        { "ccedilla", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 20 "poppler/TimesItalicWidths.gperf"
                                                        { "cedilla", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 25 "poppler/TimesItalicWidths.gperf"
                                                        { "asciitilde", 541 },
#line 89 "poppler/TimesItalicWidths.gperf"
                                                        { "d", 500 },
#line 239 "poppler/TimesItalicWidths.gperf"
                                                        { "percent", 833 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 288 "poppler/TimesItalicWidths.gperf"
                                                        { "germandbls", 500 },
                                                        { "", 0 },
#line 138 "poppler/TimesItalicWidths.gperf"
                                                        { "lozenge", 471 },
                                                        { "", 0 },
#line 316 "poppler/TimesItalicWidths.gperf"
                                                        { "less", 675 },
                                                        { "", 0 },
#line 97 "poppler/TimesItalicWidths.gperf"
                                                        { "dagger", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 258 "poppler/TimesItalicWidths.gperf"
                                                        { "grave", 333 },
#line 301 "poppler/TimesItalicWidths.gperf"
                                                        { "Ecaron", 611 },
#line 222 "poppler/TimesItalicWidths.gperf"
                                                        { "scommaaccent", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 160 "poppler/TimesItalicWidths.gperf"
                                                        { "endash", 500 },
#line 174 "poppler/TimesItalicWidths.gperf"
                                                        { "emacron", 444 },
#line 201 "poppler/TimesItalicWidths.gperf"
                                                        { "threequarters", 750 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 232 "poppler/TimesItalicWidths.gperf"
                                                        { "Tcaron", 556 },
                                                        { "", 0 },
#line 228 "poppler/TimesItalicWidths.gperf"
                                                        { "scedilla", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 101 "poppler/TimesItalicWidths.gperf"
                                                        { "m", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 245 "poppler/TimesItalicWidths.gperf"
                                                        { "summation", 600 },
#line 310 "poppler/TimesItalicWidths.gperf"
                                                        { "logicalnot", 675 },
#line 44 "poppler/TimesItalicWidths.gperf"
                                                        { "lcaron", 300 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 172 "poppler/TimesItalicWidths.gperf"
                                                        { "parenleft", 333 },
#line 139 "poppler/TimesItalicWidths.gperf"
                                                        { "parenright", 333 },
#line 95 "poppler/TimesItalicWidths.gperf"
                                                        { "i", 278 },
#line 305 "poppler/TimesItalicWidths.gperf"
                                                        { "amacron", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 194 "poppler/TimesItalicWidths.gperf"
                                                        { "Uacute", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 208 "poppler/TimesItalicWidths.gperf"
                                                        { "underscore", 500 },
#line 92 "poppler/TimesItalicWidths.gperf"
                                                        { "g", 500 },
#line 297 "poppler/TimesItalicWidths.gperf"
                                                        { "rcommaaccent", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 37 "poppler/TimesItalicWidths.gperf"
                                                        { "space", 250 },
#line 28 "poppler/TimesItalicWidths.gperf"
                                                        { "dollar", 500 },
                                                        { "", 0 },
#line 272 "poppler/TimesItalicWidths.gperf"
                                                        { "threesuperior", 300 },
#line 188 "poppler/TimesItalicWidths.gperf"
                                                        { "edieresis", 444 },
#line 236 "poppler/TimesItalicWidths.gperf"
                                                        { "registered", 760 },
#line 78 "poppler/TimesItalicWidths.gperf"
                                                        { "W", 833 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 64 "poppler/TimesItalicWidths.gperf"
                                                        { "omacron", 500 },
#line 36 "poppler/TimesItalicWidths.gperf"
                                                        { "yen", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 50 "poppler/TimesItalicWidths.gperf"
                                                        { "E", 611 },
                                                        { "", 0 },
#line 293 "poppler/TimesItalicWidths.gperf"
                                                        { "dotlessi", 278 },
                                                        { "", 0 },
#line 327 "poppler/TimesItalicWidths.gperf"
                                                        { "Edotaccent", 611 },
#line 71 "poppler/TimesItalicWidths.gperf"
                                                        { "Aacute", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 186 "poppler/TimesItalicWidths.gperf"
                                                        { "adieresis", 500 },
                                                        { "", 0 },
#line 117 "poppler/TimesItalicWidths.gperf"
                                                        { "u", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 144 "poppler/TimesItalicWidths.gperf"
                                                        { "daggerdbl", 500 },
                                                        { "", 0 },
#line 280 "poppler/TimesItalicWidths.gperf"
                                                        { "yacute", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 74 "poppler/TimesItalicWidths.gperf"
                                                        { "T", 556 },
#line 130 "poppler/TimesItalicWidths.gperf"
                                                        { "gcommaaccent", 500 },
#line 275 "poppler/TimesItalicWidths.gperf"
                                                        { "dieresis", 333 },
                                                        { "", 0 },
#line 51 "poppler/TimesItalicWidths.gperf"
                                                        { "onequarter", 750 },
#line 328 "poppler/TimesItalicWidths.gperf"
                                                        { "onesuperior", 300 },
#line 237 "poppler/TimesItalicWidths.gperf"
                                                        { "radical", 453 },
#line 190 "poppler/TimesItalicWidths.gperf"
                                                        { "Eth", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 94 "poppler/TimesItalicWidths.gperf"
                                                        { "h", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 193 "poppler/TimesItalicWidths.gperf"
                                                        { "odieresis", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 100 "poppler/TimesItalicWidths.gperf"
                                                        { "l", 278 },
#line 65 "poppler/TimesItalicWidths.gperf"
                                                        { "Tcommaaccent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 136 "poppler/TimesItalicWidths.gperf"
                                                        { "oslash", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 137 "poppler/TimesItalicWidths.gperf"
                                                        { "lessequal", 549 },
#line 159 "poppler/TimesItalicWidths.gperf"
                                                        { "exclamdown", 389 },
#line 35 "poppler/TimesItalicWidths.gperf"
                                                        { "zacute", 389 },
#line 269 "poppler/TimesItalicWidths.gperf"
                                                        { "lcommaaccent", 278 },
                                                        { "", 0 },
#line 209 "poppler/TimesItalicWidths.gperf"
                                                        { "Euro", 500 },
                                                        { "", 0 },
#line 291 "poppler/TimesItalicWidths.gperf"
                                                        { "Sacute", 500 },
#line 323 "poppler/TimesItalicWidths.gperf"
                                                        { "greater", 675 },
#line 244 "poppler/TimesItalicWidths.gperf"
                                                        { "two", 500 },
                                                        { "", 0 },
#line 220 "poppler/TimesItalicWidths.gperf"
                                                        { "Thorn", 611 },
#line 256 "poppler/TimesItalicWidths.gperf"
                                                        { "asciicircum", 422 },
#line 126 "poppler/TimesItalicWidths.gperf"
                                                        { "hungarumlaut", 333 },
                                                        { "", 0 },
#line 212 "poppler/TimesItalicWidths.gperf"
                                                        { "zero", 500 },
                                                        { "", 0 },
#line 40 "poppler/TimesItalicWidths.gperf"
                                                        { "emdash", 889 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 116 "poppler/TimesItalicWidths.gperf"
                                                        { "divide", 675 },
                                                        { "", 0 },
#line 271 "poppler/TimesItalicWidths.gperf"
                                                        { "ohungarumlaut", 500 },
#line 262 "poppler/TimesItalicWidths.gperf"
                                                        { "ampersand", 778 },
                                                        { "", 0 },
#line 164 "poppler/TimesItalicWidths.gperf"
                                                        { "ecircumflex", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 106 "poppler/TimesItalicWidths.gperf"
                                                        { "ring", 333 },
                                                        { "", 0 },
#line 320 "poppler/TimesItalicWidths.gperf"
                                                        { "period", 250 },
                                                        { "", 0 },
#line 318 "poppler/TimesItalicWidths.gperf"
                                                        { "guilsinglleft", 333 },
#line 155 "poppler/TimesItalicWidths.gperf"
                                                        { "guilsinglright", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 307 "poppler/TimesItalicWidths.gperf"
                                                        { "imacron", 278 },
                                                        { "", 0 },
#line 61 "poppler/TimesItalicWidths.gperf"
                                                        { "periodcentered", 250 },
                                                        { "", 0 },
#line 227 "poppler/TimesItalicWidths.gperf"
                                                        { "Oacute", 722 },
                                                        { "", 0 },
#line 294 "poppler/TimesItalicWidths.gperf"
                                                        { "sterling", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 299 "poppler/TimesItalicWidths.gperf"
                                                        { "acircumflex", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 33 "poppler/TimesItalicWidths.gperf"
                                                        { "minus", 675 },
#line 312 "poppler/TimesItalicWidths.gperf"
                                                        { "Atilde", 611 },
#line 148 "poppler/TimesItalicWidths.gperf"
                                                        { "Emacron", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 257 "poppler/TimesItalicWidths.gperf"
                                                        { "aring", 500 },
#line 261 "poppler/TimesItalicWidths.gperf"
                                                        { "Iacute", 333 },
#line 183 "poppler/TimesItalicWidths.gperf"
                                                        { "umacron", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 221 "poppler/TimesItalicWidths.gperf"
                                                        { "zcaron", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 133 "poppler/TimesItalicWidths.gperf"
                                                        { "Scaron", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 248 "poppler/TimesItalicWidths.gperf"
                                                        { "ocircumflex", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 189 "poppler/TimesItalicWidths.gperf"
                                                        { "idieresis", 278 },
                                                        { "", 0 },
#line 157 "poppler/TimesItalicWidths.gperf"
                                                        { "quotesingle", 214 },
#line 277 "poppler/TimesItalicWidths.gperf"
                                                        { "quotedblbase", 556 },
                                                        { "", 0 },
#line 268 "poppler/TimesItalicWidths.gperf"
                                                        { "quotesinglbase", 333 },
                                                        { "", 0 },
#line 107 "poppler/TimesItalicWidths.gperf"
                                                        { "p", 500 },
#line 132 "poppler/TimesItalicWidths.gperf"
                                                        { "greaterequal", 549 },
                                                        { "", 0 },
#line 326 "poppler/TimesItalicWidths.gperf"
                                                        { "quoteleft", 333 },
#line 179 "poppler/TimesItalicWidths.gperf"
                                                        { "quoteright", 333 },
                                                        { "", 0 },
#line 154 "poppler/TimesItalicWidths.gperf"
                                                        { "quotedblleft", 556 },
#line 304 "poppler/TimesItalicWidths.gperf"
                                                        { "quotedblright", 556 },
#line 169 "poppler/TimesItalicWidths.gperf"
                                                        { "Edieresis", 611 },
                                                        { "", 0 },
#line 128 "poppler/TimesItalicWidths.gperf"
                                                        { "Nacute", 667 },
#line 131 "poppler/TimesItalicWidths.gperf"
                                                        { "mu", 500 },
                                                        { "", 0 },
#line 198 "poppler/TimesItalicWidths.gperf"
                                                        { "udieresis", 500 },
                                                        { "", 0 },
#line 270 "poppler/TimesItalicWidths.gperf"
                                                        { "Yacute", 556 },
#line 253 "poppler/TimesItalicWidths.gperf"
                                                        { "eogonek", 444 },
#line 80 "poppler/TimesItalicWidths.gperf"
                                                        { "question", 500 },
                                                        { "", 0 },
#line 313 "poppler/TimesItalicWidths.gperf"
                                                        { "breve", 333 },
#line 77 "poppler/TimesItalicWidths.gperf"
                                                        { "V", 611 },
#line 39 "poppler/TimesItalicWidths.gperf"
                                                        { "questiondown", 500 },
                                                        { "", 0 },
#line 266 "poppler/TimesItalicWidths.gperf"
                                                        { "plus", 675 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 149 "poppler/TimesItalicWidths.gperf"
                                                        { "ellipsis", 889 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 319 "poppler/TimesItalicWidths.gperf"
                                                        { "exclam", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 219 "poppler/TimesItalicWidths.gperf"
                                                        { "braceleft", 400 },
#line 303 "poppler/TimesItalicWidths.gperf"
                                                        { "braceright", 400 },
#line 156 "poppler/TimesItalicWidths.gperf"
                                                        { "hyphen", 333 },
#line 48 "poppler/TimesItalicWidths.gperf"
                                                        { "aogonek", 500 },
#line 314 "poppler/TimesItalicWidths.gperf"
                                                        { "bar", 275 },
                                                        { "", 0 },
#line 311 "poppler/TimesItalicWidths.gperf"
                                                        { "zdotaccent", 389 },
#line 153 "poppler/TimesItalicWidths.gperf"
                                                        { "lslash", 278 },
#line 86 "poppler/TimesItalicWidths.gperf"
                                                        { "Gcommaaccent", 722 },
#line 309 "poppler/TimesItalicWidths.gperf"
                                                        { "currency", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 75 "poppler/TimesItalicWidths.gperf"
                                                        { "U", 722 },
#line 27 "poppler/TimesItalicWidths.gperf"
                                                        { "onehalf", 750 },
#line 109 "poppler/TimesItalicWidths.gperf"
                                                        { "uhungarumlaut", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 147 "poppler/TimesItalicWidths.gperf"
                                                        { "Otilde", 722 },
                                                        { "", 0 },
#line 287 "poppler/TimesItalicWidths.gperf"
                                                        { "guillemotleft", 500 },
#line 202 "poppler/TimesItalicWidths.gperf"
                                                        { "guillemotright", 500 },
                                                        { "", 0 },
#line 247 "poppler/TimesItalicWidths.gperf"
                                                        { "Lacute", 556 },
#line 163 "poppler/TimesItalicWidths.gperf"
                                                        { "Umacron", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 18 "poppler/TimesItalicWidths.gperf"
                                                        { "Zacute", 556 },
                                                        { "", 0 },
#line 295 "poppler/TimesItalicWidths.gperf"
                                                        { "notequal", 549 },
#line 143 "poppler/TimesItalicWidths.gperf"
                                                        { "trademark", 980 },
                                                        { "", 0 },
#line 265 "poppler/TimesItalicWidths.gperf"
                                                        { "Ncaron", 667 },
#line 200 "poppler/TimesItalicWidths.gperf"
                                                        { "Scommaaccent", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 181 "poppler/TimesItalicWidths.gperf"
                                                        { "perthousand", 1000 },
                                                        { "", 0 },
#line 240 "poppler/TimesItalicWidths.gperf"
                                                        { "six", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 302 "poppler/TimesItalicWidths.gperf"
                                                        { "icircumflex", 278 },
                                                        { "", 0 },
#line 214 "poppler/TimesItalicWidths.gperf"
                                                        { "Scedilla", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 93 "poppler/TimesItalicWidths.gperf"
                                                        { "bullet", 350 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 108 "poppler/TimesItalicWidths.gperf"
                                                        { "q", 500 },
#line 289 "poppler/TimesItalicWidths.gperf"
                                                        { "Amacron", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 127 "poppler/TimesItalicWidths.gperf"
                                                        { "Idotaccent", 333 },
#line 141 "poppler/TimesItalicWidths.gperf"
                                                        { "Ecircumflex", 611 },
                                                        { "", 0 },
#line 315 "poppler/TimesItalicWidths.gperf"
                                                        { "fraction", 167 },
#line 180 "poppler/TimesItalicWidths.gperf"
                                                        { "Udieresis", 722 },
                                                        { "", 0 },
#line 176 "poppler/TimesItalicWidths.gperf"
                                                        { "ucircumflex", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 197 "poppler/TimesItalicWidths.gperf"
                                                        { "five", 500 },
                                                        { "", 0 },
#line 14 "poppler/TimesItalicWidths.gperf"
                                                        { "Ntilde", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 267 "poppler/TimesItalicWidths.gperf"
                                                        { "uring", 500 },
#line 45 "poppler/TimesItalicWidths.gperf"
                                                        { "A", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 84 "poppler/TimesItalicWidths.gperf"
                                                        { "four", 500 },
                                                        { "", 0 },
#line 187 "poppler/TimesItalicWidths.gperf"
                                                        { "egrave", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 241 "poppler/TimesItalicWidths.gperf"
                                                        { "paragraph", 523 },
                                                        { "", 0 },
#line 29 "poppler/TimesItalicWidths.gperf"
                                                        { "Lcaron", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 325 "poppler/TimesItalicWidths.gperf"
                                                        { "brokenbar", 275 },
                                                        { "", 0 },
#line 199 "poppler/TimesItalicWidths.gperf"
                                                        { "Zcaron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 165 "poppler/TimesItalicWidths.gperf"
                                                        { "Adieresis", 611 },
                                                        { "", 0 },
#line 122 "poppler/TimesItalicWidths.gperf"
                                                        { "y", 444 },
#line 16 "poppler/TimesItalicWidths.gperf"
                                                        { "kcommaaccent", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 76 "poppler/TimesItalicWidths.gperf"
                                                        { "agrave", 500 },
                                                        { "", 0 },
#line 69 "poppler/TimesItalicWidths.gperf"
                                                        { "Uhungarumlaut", 722 },
#line 204 "poppler/TimesItalicWidths.gperf"
                                                        { "ydieresis", 444 },
#line 168 "poppler/TimesItalicWidths.gperf"
                                                        { "slash", 278 },
#line 229 "poppler/TimesItalicWidths.gperf"
                                                        { "ogonek", 333 },
#line 151 "poppler/TimesItalicWidths.gperf"
                                                        { "AE", 889 },
#line 192 "poppler/TimesItalicWidths.gperf"
                                                        { "asterisk", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 111 "poppler/TimesItalicWidths.gperf"
                                                        { "twosuperior", 300 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 53 "poppler/TimesItalicWidths.gperf"
                                                        { "G", 722 },
#line 58 "poppler/TimesItalicWidths.gperf"
                                                        { "iogonek", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 17 "poppler/TimesItalicWidths.gperf"
                                                        { "Ncommaaccent", 667 },
                                                        { "", 0 },
#line 21 "poppler/TimesItalicWidths.gperf"
                                                        { "plusminus", 675 },
                                                        { "", 0 },
#line 230 "poppler/TimesItalicWidths.gperf"
                                                        { "ograve", 500 },
                                                        { "", 0 },
#line 129 "poppler/TimesItalicWidths.gperf"
                                                        { "quotedbl", 420 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 233 "poppler/TimesItalicWidths.gperf"
                                                        { "Eogonek", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 120 "poppler/TimesItalicWidths.gperf"
                                                        { "w", 667 },
#line 259 "poppler/TimesItalicWidths.gperf"
                                                        { "uogonek", 500 },
                                                        { "", 0 },
#line 59 "poppler/TimesItalicWidths.gperf"
                                                        { "backslash", 278 },
#line 81 "poppler/TimesItalicWidths.gperf"
                                                        { "equal", 675 },
                                                        { "", 0 },
#line 38 "poppler/TimesItalicWidths.gperf"
                                                        { "Omacron", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 121 "poppler/TimesItalicWidths.gperf"
                                                        { "x", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 298 "poppler/TimesItalicWidths.gperf"
                                                        { "Zdotaccent", 556 },
#line 152 "poppler/TimesItalicWidths.gperf"
                                                        { "Ucircumflex", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 68 "poppler/TimesItalicWidths.gperf"
                                                        { "Q", 722 },
#line 296 "poppler/TimesItalicWidths.gperf"
                                                        { "Imacron", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 250 "poppler/TimesItalicWidths.gperf"
                                                        { "Uring", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 123 "poppler/TimesItalicWidths.gperf"
                                                        { "z", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 22 "poppler/TimesItalicWidths.gperf"
                                                        { "circumflex", 333 },
                                                        { "", 0 },
#line 251 "poppler/TimesItalicWidths.gperf"
                                                        { "Lcommaaccent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 73 "poppler/TimesItalicWidths.gperf"
                                                        { "S", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 175 "poppler/TimesItalicWidths.gperf"
                                                        { "Odieresis", 722 },
                                                        { "", 0 },
#line 284 "poppler/TimesItalicWidths.gperf"
                                                        { "Acircumflex", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 67 "poppler/TimesItalicWidths.gperf"
                                                        { "P", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 238 "poppler/TimesItalicWidths.gperf"
                                                        { "Aring", 611 },
#line 96 "poppler/TimesItalicWidths.gperf"
                                                        { "Oslash", 722 },
#line 114 "poppler/TimesItalicWidths.gperf"
                                                        { "OE", 944 },
                                                        { "", 0 },
#line 171 "poppler/TimesItalicWidths.gperf"
                                                        { "Idieresis", 333 },
                                                        { "", 0 },
#line 62 "poppler/TimesItalicWidths.gperf"
                                                        { "M", 833 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 282 "poppler/TimesItalicWidths.gperf"
                                                        { "fi", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 56 "poppler/TimesItalicWidths.gperf"
                                                        { "J", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 263 "poppler/TimesItalicWidths.gperf"
                                                        { "igrave", 278 },
                                                        { "", 0 },
#line 255 "poppler/TimesItalicWidths.gperf"
                                                        { "Ohungarumlaut", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 243 "poppler/TimesItalicWidths.gperf"
                                                        { "Uogonek", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 87 "poppler/TimesItalicWidths.gperf"
                                                        { "b", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 167 "poppler/TimesItalicWidths.gperf"
                                                        { "Egrave", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 182 "poppler/TimesItalicWidths.gperf"
                                                        { "Ydieresis", 556 },
                                                        { "", 0 },
#line 195 "poppler/TimesItalicWidths.gperf"
                                                        { "ugrave", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 211 "poppler/TimesItalicWidths.gperf"
                                                        { "multiply", 675 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 66 "poppler/TimesItalicWidths.gperf"
                                                        { "O", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 31 "poppler/TimesItalicWidths.gperf"
                                                        { "Aogonek", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 279 "poppler/TimesItalicWidths.gperf"
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
#line 226 "poppler/TimesItalicWidths.gperf"
                                                        { "Ocircumflex", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 283 "poppler/TimesItalicWidths.gperf"
                                                        { "fl", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 55 "poppler/TimesItalicWidths.gperf"
                                                        { "I", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 286 "poppler/TimesItalicWidths.gperf"
                                                        { "Icircumflex", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 54 "poppler/TimesItalicWidths.gperf"
                                                        { "H", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 134 "poppler/TimesItalicWidths.gperf"
                                                        { "Lslash", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 99 "poppler/TimesItalicWidths.gperf"
                                                        { "k", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 184 "poppler/TimesItalicWidths.gperf"
                                                        { "abreve", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 217 "poppler/TimesItalicWidths.gperf"
                                                        { "partialdiff", 476 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 52 "poppler/TimesItalicWidths.gperf"
                                                        { "F", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 178 "poppler/TimesItalicWidths.gperf"
                                                        { "Ugrave", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 91 "poppler/TimesItalicWidths.gperf"
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
#line 119 "poppler/TimesItalicWidths.gperf"
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
#line 63 "poppler/TimesItalicWidths.gperf"
                                                        { "N", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 41 "poppler/TimesItalicWidths.gperf"
                                                        { "Agrave", 611 },
#line 34 "poppler/TimesItalicWidths.gperf"
                                                        { "Iogonek", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 82 "poppler/TimesItalicWidths.gperf"
                                                        { "Y", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 46 "poppler/TimesItalicWidths.gperf"
                                                        { "B", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 177 "poppler/TimesItalicWidths.gperf"
                                                        { "bracketleft", 389 },
#line 260 "poppler/TimesItalicWidths.gperf"
                                                        { "bracketright", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 142 "poppler/TimesItalicWidths.gperf"
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
#line 215 "poppler/TimesItalicWidths.gperf"
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
#line 60 "poppler/TimesItalicWidths.gperf"
                                                        { "L", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 246 "poppler/TimesItalicWidths.gperf"
                                                        { "Igrave", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 83 "poppler/TimesItalicWidths.gperf"
                                                        { "Z", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 162 "poppler/TimesItalicWidths.gperf"
                                                        { "Abreve", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 124 "poppler/TimesItalicWidths.gperf"
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
#line 330 "poppler/TimesItalicWidths.gperf"
