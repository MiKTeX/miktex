/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf poppler/HelveticaBoldWidths.gperf  */
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

#line 1 "poppler/HelveticaBoldWidths.gperf"

#include <string.h>
#include "BuiltinFontWidth.h"

#define TOTAL_KEYWORDS 316
#define MIN_WORD_LENGTH 1
#define MAX_WORD_LENGTH 15
#define MIN_HASH_VALUE 1
#define MAX_HASH_VALUE 1001
/* maximum key range = 1001, duplicates = 0 */

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
    static const unsigned short asso_values[] = { 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002,
                                                  1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002,
                                                  1002, 1002, 1002, 1002, 1002, 1002, 1002, 270,  440,  28,   480,  315,  435,  5,    430,  405,  385,  35,   3,    330,  460,  355,  325,  310,  500,  350,  160,  370,  300,
                                                  275,  145,  415,  420,  1002, 1002, 1002, 1002, 1002, 1002, 20,   320,  30,   115,  0,    395,  140,  165,  135,  70,   380,  170,  130,  15,   45,   215,  260,  100,  65,
                                                  10,   155,  335,  290,  305,  280,  150,  1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002,
                                                  1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002,
                                                  1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002,
                                                  1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002,
                                                  1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002 };
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

const struct BuiltinFontWidth *HelveticaBoldWidthsLookup(register const char *str, register size_t len)
{
    static const struct BuiltinFontWidth wordlist[] = { { "", 0 },
#line 90 "poppler/HelveticaBoldWidths.gperf"
                                                        { "e", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 60 "poppler/HelveticaBoldWidths.gperf"
                                                        { "L", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 53 "poppler/HelveticaBoldWidths.gperf"
                                                        { "G", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 115 "poppler/HelveticaBoldWidths.gperf"
                                                        { "t", 333 },
#line 191 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ae", 889 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 102 "poppler/HelveticaBoldWidths.gperf"
                                                        { "n", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 208 "poppler/HelveticaBoldWidths.gperf"
                                                        { "eacute", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 248 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Lacute", 611 },
                                                        { "", 0 },
#line 85 "poppler/HelveticaBoldWidths.gperf"
                                                        { "a", 556 },
#line 207 "poppler/HelveticaBoldWidths.gperf"
                                                        { "at", 975 },
                                                        { "", 0 },
#line 309 "poppler/HelveticaBoldWidths.gperf"
                                                        { "cent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 161 "poppler/HelveticaBoldWidths.gperf"
                                                        { "oe", 944 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 145 "poppler/HelveticaBoldWidths.gperf"
                                                        { "nacute", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 274 "poppler/HelveticaBoldWidths.gperf"
                                                        { "acute", 333 },
#line 112 "poppler/HelveticaBoldWidths.gperf"
                                                        { "aacute", 556 },
#line 47 "poppler/HelveticaBoldWidths.gperf"
                                                        { "C", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 88 "poppler/HelveticaBoldWidths.gperf"
                                                        { "c", 556 },
                                                        { "", 0 },
#line 173 "poppler/HelveticaBoldWidths.gperf"
                                                        { "one", 556 },
#line 286 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Cacute", 722 },
                                                        { "", 0 },
#line 301 "poppler/HelveticaBoldWidths.gperf"
                                                        { "cacute", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 57 "poppler/HelveticaBoldWidths.gperf"
                                                        { "K", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 250 "poppler/HelveticaBoldWidths.gperf"
                                                        { "oacute", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 72 "poppler/HelveticaBoldWidths.gperf"
                                                        { "caron", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 104 "poppler/HelveticaBoldWidths.gperf"
                                                        { "o", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 318 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ecaron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 29 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Lcaron", 611 },
#line 291 "poppler/HelveticaBoldWidths.gperf"
                                                        { "seven", 556 },
#line 307 "poppler/HelveticaBoldWidths.gperf"
                                                        { "sacute", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 253 "poppler/HelveticaBoldWidths.gperf"
                                                        { "tcaron", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 26 "poppler/HelveticaBoldWidths.gperf"
                                                        { "colon", 333 },
#line 279 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ncaron", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 125 "poppler/HelveticaBoldWidths.gperf"
                                                        { "commaaccent", 250 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 135 "poppler/HelveticaBoldWidths.gperf"
                                                        { "semicolon", 333 },
#line 19 "poppler/HelveticaBoldWidths.gperf"
                                                        { "comma", 278 },
#line 236 "poppler/HelveticaBoldWidths.gperf"
                                                        { "degree", 400 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 118 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Ccaron", 722 },
                                                        { "", 0 },
#line 140 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ccaron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 113 "poppler/HelveticaBoldWidths.gperf"
                                                        { "s", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 232 "poppler/HelveticaBoldWidths.gperf"
                                                        { "racute", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 98 "poppler/HelveticaBoldWidths.gperf"
                                                        { "j", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 30 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ntilde", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 205 "poppler/HelveticaBoldWidths.gperf"
                                                        { "tilde", 333 },
#line 325 "poppler/HelveticaBoldWidths.gperf"
                                                        { "atilde", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 196 "poppler/HelveticaBoldWidths.gperf"
                                                        { "nine", 556 },
#line 24 "poppler/HelveticaBoldWidths.gperf"
                                                        { "edotaccent", 556 },
#line 105 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ordfeminine", 370 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 158 "poppler/HelveticaBoldWidths.gperf"
                                                        { "eight", 556 },
#line 150 "poppler/HelveticaBoldWidths.gperf"
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
#line 277 "poppler/HelveticaBoldWidths.gperf"
                                                        { "iacute", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 170 "poppler/HelveticaBoldWidths.gperf"
                                                        { "otilde", 611 },
#line 293 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ordmasculine", 365 },
#line 214 "poppler/HelveticaBoldWidths.gperf"
                                                        { "eth", 611 },
                                                        { "", 0 },
#line 42 "poppler/HelveticaBoldWidths.gperf"
                                                        { "three", 556 },
#line 226 "poppler/HelveticaBoldWidths.gperf"
                                                        { "dcroat", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 252 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Lcommaaccent", 611 },
#line 35 "poppler/HelveticaBoldWidths.gperf"
                                                        { "zacute", 500 },
#line 86 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Gcommaaccent", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 219 "poppler/HelveticaBoldWidths.gperf"
                                                        { "uacute", 611 },
#line 103 "poppler/HelveticaBoldWidths.gperf"
                                                        { "tcommaaccent", 333 },
                                                        { "", 0 },
#line 166 "poppler/HelveticaBoldWidths.gperf"
                                                        { "copyright", 737 },
#line 43 "poppler/HelveticaBoldWidths.gperf"
                                                        { "numbersign", 556 },
#line 15 "poppler/HelveticaBoldWidths.gperf"
                                                        { "rcaron", 389 },
#line 32 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ncommaaccent", 611 },
                                                        { "", 0 },
#line 213 "poppler/HelveticaBoldWidths.gperf"
                                                        { "zero", 556 },
                                                        { "", 0 },
#line 110 "poppler/HelveticaBoldWidths.gperf"
                                                        { "r", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 265 "poppler/HelveticaBoldWidths.gperf"
                                                        { "lacute", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 23 "poppler/HelveticaBoldWidths.gperf"
                                                        { "dotaccent", 333 },
#line 235 "poppler/HelveticaBoldWidths.gperf"
                                                        { "thorn", 611 },
#line 243 "poppler/HelveticaBoldWidths.gperf"
                                                        { "dcaron", 743 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 146 "poppler/HelveticaBoldWidths.gperf"
                                                        { "macron", 333 },
#line 323 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Kcommaaccent", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 203 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Ccedilla", 722 },
#line 275 "poppler/HelveticaBoldWidths.gperf"
                                                        { "section", 556 },
#line 224 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ccedilla", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 20 "poppler/HelveticaBoldWidths.gperf"
                                                        { "cedilla", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 25 "poppler/HelveticaBoldWidths.gperf"
                                                        { "asciitilde", 584 },
#line 89 "poppler/HelveticaBoldWidths.gperf"
                                                        { "d", 611 },
#line 240 "poppler/HelveticaBoldWidths.gperf"
                                                        { "percent", 889 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 289 "poppler/HelveticaBoldWidths.gperf"
                                                        { "germandbls", 611 },
                                                        { "", 0 },
#line 138 "poppler/HelveticaBoldWidths.gperf"
                                                        { "lozenge", 494 },
                                                        { "", 0 },
#line 317 "poppler/HelveticaBoldWidths.gperf"
                                                        { "less", 584 },
                                                        { "", 0 },
#line 97 "poppler/HelveticaBoldWidths.gperf"
                                                        { "dagger", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 259 "poppler/HelveticaBoldWidths.gperf"
                                                        { "grave", 333 },
#line 222 "poppler/HelveticaBoldWidths.gperf"
                                                        { "zcaron", 500 },
#line 223 "poppler/HelveticaBoldWidths.gperf"
                                                        { "scommaaccent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 160 "poppler/HelveticaBoldWidths.gperf"
                                                        { "endash", 556 },
#line 174 "poppler/HelveticaBoldWidths.gperf"
                                                        { "emacron", 556 },
#line 201 "poppler/HelveticaBoldWidths.gperf"
                                                        { "threequarters", 834 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 233 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Tcaron", 611 },
                                                        { "", 0 },
#line 229 "poppler/HelveticaBoldWidths.gperf"
                                                        { "scedilla", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 101 "poppler/HelveticaBoldWidths.gperf"
                                                        { "m", 889 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 246 "poppler/HelveticaBoldWidths.gperf"
                                                        { "summation", 600 },
#line 311 "poppler/HelveticaBoldWidths.gperf"
                                                        { "logicalnot", 584 },
#line 44 "poppler/HelveticaBoldWidths.gperf"
                                                        { "lcaron", 400 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 172 "poppler/HelveticaBoldWidths.gperf"
                                                        { "parenleft", 333 },
#line 139 "poppler/HelveticaBoldWidths.gperf"
                                                        { "parenright", 333 },
#line 95 "poppler/HelveticaBoldWidths.gperf"
                                                        { "i", 278 },
#line 306 "poppler/HelveticaBoldWidths.gperf"
                                                        { "amacron", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 209 "poppler/HelveticaBoldWidths.gperf"
                                                        { "underscore", 556 },
#line 92 "poppler/HelveticaBoldWidths.gperf"
                                                        { "g", 611 },
#line 298 "poppler/HelveticaBoldWidths.gperf"
                                                        { "rcommaaccent", 389 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 37 "poppler/HelveticaBoldWidths.gperf"
                                                        { "space", 278 },
#line 28 "poppler/HelveticaBoldWidths.gperf"
                                                        { "dollar", 556 },
                                                        { "", 0 },
#line 273 "poppler/HelveticaBoldWidths.gperf"
                                                        { "threesuperior", 333 },
#line 188 "poppler/HelveticaBoldWidths.gperf"
                                                        { "edieresis", 556 },
#line 237 "poppler/HelveticaBoldWidths.gperf"
                                                        { "registered", 737 },
#line 79 "poppler/HelveticaBoldWidths.gperf"
                                                        { "X", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 64 "poppler/HelveticaBoldWidths.gperf"
                                                        { "omacron", 611 },
#line 36 "poppler/HelveticaBoldWidths.gperf"
                                                        { "yen", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 123 "poppler/HelveticaBoldWidths.gperf"
                                                        { "z", 500 },
                                                        { "", 0 },
#line 294 "poppler/HelveticaBoldWidths.gperf"
                                                        { "dotlessi", 278 },
#line 134 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Lslash", 611 },
#line 312 "poppler/HelveticaBoldWidths.gperf"
                                                        { "zdotaccent", 500 },
#line 71 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Aacute", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 186 "poppler/HelveticaBoldWidths.gperf"
                                                        { "adieresis", 556 },
                                                        { "", 0 },
#line 117 "poppler/HelveticaBoldWidths.gperf"
                                                        { "u", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 144 "poppler/HelveticaBoldWidths.gperf"
                                                        { "daggerdbl", 556 },
                                                        { "", 0 },
#line 281 "poppler/HelveticaBoldWidths.gperf"
                                                        { "yacute", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 74 "poppler/HelveticaBoldWidths.gperf"
                                                        { "T", 611 },
#line 130 "poppler/HelveticaBoldWidths.gperf"
                                                        { "gcommaaccent", 611 },
#line 276 "poppler/HelveticaBoldWidths.gperf"
                                                        { "dieresis", 333 },
                                                        { "", 0 },
#line 51 "poppler/HelveticaBoldWidths.gperf"
                                                        { "onequarter", 834 },
#line 329 "poppler/HelveticaBoldWidths.gperf"
                                                        { "onesuperior", 333 },
#line 238 "poppler/HelveticaBoldWidths.gperf"
                                                        { "radical", 549 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 94 "poppler/HelveticaBoldWidths.gperf"
                                                        { "h", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 193 "poppler/HelveticaBoldWidths.gperf"
                                                        { "odieresis", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 100 "poppler/HelveticaBoldWidths.gperf"
                                                        { "l", 278 },
#line 65 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Tcommaaccent", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 136 "poppler/HelveticaBoldWidths.gperf"
                                                        { "oslash", 611 },
                                                        { "", 0 },
#line 245 "poppler/HelveticaBoldWidths.gperf"
                                                        { "two", 556 },
#line 137 "poppler/HelveticaBoldWidths.gperf"
                                                        { "lessequal", 549 },
#line 159 "poppler/HelveticaBoldWidths.gperf"
                                                        { "exclamdown", 333 },
#line 185 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Eacute", 667 },
#line 270 "poppler/HelveticaBoldWidths.gperf"
                                                        { "lcommaaccent", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 324 "poppler/HelveticaBoldWidths.gperf"
                                                        { "greater", 584 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 221 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Thorn", 667 },
#line 257 "poppler/HelveticaBoldWidths.gperf"
                                                        { "asciicircum", 584 },
#line 126 "poppler/HelveticaBoldWidths.gperf"
                                                        { "hungarumlaut", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 40 "poppler/HelveticaBoldWidths.gperf"
                                                        { "emdash", 1000 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 116 "poppler/HelveticaBoldWidths.gperf"
                                                        { "divide", 584 },
                                                        { "", 0 },
#line 272 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ohungarumlaut", 611 },
#line 263 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ampersand", 722 },
                                                        { "", 0 },
#line 164 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ecircumflex", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 106 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ring", 333 },
                                                        { "", 0 },
#line 321 "poppler/HelveticaBoldWidths.gperf"
                                                        { "period", 278 },
                                                        { "", 0 },
#line 319 "poppler/HelveticaBoldWidths.gperf"
                                                        { "guilsinglleft", 333 },
#line 155 "poppler/HelveticaBoldWidths.gperf"
                                                        { "guilsinglright", 333 },
                                                        { "", 0 },
#line 292 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Sacute", 667 },
#line 308 "poppler/HelveticaBoldWidths.gperf"
                                                        { "imacron", 278 },
                                                        { "", 0 },
#line 61 "poppler/HelveticaBoldWidths.gperf"
                                                        { "periodcentered", 278 },
                                                        { "", 0 },
#line 228 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Oacute", 778 },
                                                        { "", 0 },
#line 295 "poppler/HelveticaBoldWidths.gperf"
                                                        { "sterling", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 300 "poppler/HelveticaBoldWidths.gperf"
                                                        { "acircumflex", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 33 "poppler/HelveticaBoldWidths.gperf"
                                                        { "minus", 584 },
#line 313 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Atilde", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 258 "poppler/HelveticaBoldWidths.gperf"
                                                        { "aring", 556 },
#line 194 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Uacute", 722 },
#line 183 "poppler/HelveticaBoldWidths.gperf"
                                                        { "umacron", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 302 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Ecaron", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 249 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ocircumflex", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 189 "poppler/HelveticaBoldWidths.gperf"
                                                        { "idieresis", 278 },
#line 314 "poppler/HelveticaBoldWidths.gperf"
                                                        { "breve", 333 },
#line 157 "poppler/HelveticaBoldWidths.gperf"
                                                        { "quotesingle", 238 },
#line 278 "poppler/HelveticaBoldWidths.gperf"
                                                        { "quotedblbase", 500 },
                                                        { "", 0 },
#line 269 "poppler/HelveticaBoldWidths.gperf"
                                                        { "quotesinglbase", 278 },
                                                        { "", 0 },
#line 107 "poppler/HelveticaBoldWidths.gperf"
                                                        { "p", 611 },
#line 132 "poppler/HelveticaBoldWidths.gperf"
                                                        { "greaterequal", 549 },
                                                        { "", 0 },
#line 327 "poppler/HelveticaBoldWidths.gperf"
                                                        { "quoteleft", 278 },
#line 179 "poppler/HelveticaBoldWidths.gperf"
                                                        { "quoteright", 278 },
                                                        { "", 0 },
#line 154 "poppler/HelveticaBoldWidths.gperf"
                                                        { "quotedblleft", 500 },
#line 305 "poppler/HelveticaBoldWidths.gperf"
                                                        { "quotedblright", 500 },
#line 220 "poppler/HelveticaBoldWidths.gperf"
                                                        { "braceleft", 389 },
#line 304 "poppler/HelveticaBoldWidths.gperf"
                                                        { "braceright", 389 },
#line 262 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Iacute", 278 },
#line 131 "poppler/HelveticaBoldWidths.gperf"
                                                        { "mu", 611 },
#line 315 "poppler/HelveticaBoldWidths.gperf"
                                                        { "bar", 280 },
#line 198 "poppler/HelveticaBoldWidths.gperf"
                                                        { "udieresis", 611 },
                                                        { "", 0 },
#line 133 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Scaron", 667 },
#line 254 "poppler/HelveticaBoldWidths.gperf"
                                                        { "eogonek", 556 },
#line 80 "poppler/HelveticaBoldWidths.gperf"
                                                        { "question", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 271 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Yacute", 667 },
#line 39 "poppler/HelveticaBoldWidths.gperf"
                                                        { "questiondown", 611 },
                                                        { "", 0 },
#line 267 "poppler/HelveticaBoldWidths.gperf"
                                                        { "plus", 584 },
                                                        { "", 0 },
#line 18 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Zacute", 611 },
                                                        { "", 0 },
#line 149 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ellipsis", 1000 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 320 "poppler/HelveticaBoldWidths.gperf"
                                                        { "exclam", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 156 "poppler/HelveticaBoldWidths.gperf"
                                                        { "hyphen", 333 },
#line 48 "poppler/HelveticaBoldWidths.gperf"
                                                        { "aogonek", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 328 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Edotaccent", 667 },
#line 153 "poppler/HelveticaBoldWidths.gperf"
                                                        { "lslash", 278 },
                                                        { "", 0 },
#line 310 "poppler/HelveticaBoldWidths.gperf"
                                                        { "currency", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 206 "poppler/HelveticaBoldWidths.gperf"
                                                        { "dbldaggerumlaut", 556 },
#line 187 "poppler/HelveticaBoldWidths.gperf"
                                                        { "egrave", 556 },
#line 27 "poppler/HelveticaBoldWidths.gperf"
                                                        { "onehalf", 834 },
#line 109 "poppler/HelveticaBoldWidths.gperf"
                                                        { "uhungarumlaut", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 147 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Otilde", 778 },
                                                        { "", 0 },
#line 288 "poppler/HelveticaBoldWidths.gperf"
                                                        { "guillemotleft", 556 },
#line 202 "poppler/HelveticaBoldWidths.gperf"
                                                        { "guillemotright", 556 },
                                                        { "", 0 },
#line 93 "poppler/HelveticaBoldWidths.gperf"
                                                        { "bullet", 350 },
                                                        { "", 0 },
#line 190 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Eth", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 128 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Nacute", 722 },
                                                        { "", 0 },
#line 296 "poppler/HelveticaBoldWidths.gperf"
                                                        { "notequal", 549 },
#line 143 "poppler/HelveticaBoldWidths.gperf"
                                                        { "trademark", 1000 },
                                                        { "", 0 },
#line 76 "poppler/HelveticaBoldWidths.gperf"
                                                        { "agrave", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 181 "poppler/HelveticaBoldWidths.gperf"
                                                        { "perthousand", 1000 },
                                                        { "", 0 },
#line 241 "poppler/HelveticaBoldWidths.gperf"
                                                        { "six", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 303 "poppler/HelveticaBoldWidths.gperf"
                                                        { "icircumflex", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 199 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Zcaron", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 210 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Euro", 556 },
                                                        { "", 0 },
#line 108 "poppler/HelveticaBoldWidths.gperf"
                                                        { "q", 611 },
#line 290 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Amacron", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 255 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Delta", 612 },
#line 231 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ograve", 611 },
                                                        { "", 0 },
#line 316 "poppler/HelveticaBoldWidths.gperf"
                                                        { "fraction", 167 },
#line 326 "poppler/HelveticaBoldWidths.gperf"
                                                        { "brokenbar", 280 },
                                                        { "", 0 },
#line 176 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ucircumflex", 611 },
#line 200 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Scommaaccent", 667 },
                                                        { "", 0 },
#line 197 "poppler/HelveticaBoldWidths.gperf"
                                                        { "five", 556 },
                                                        { "", 0 },
#line 217 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Racute", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 268 "poppler/HelveticaBoldWidths.gperf"
                                                        { "uring", 611 },
#line 45 "poppler/HelveticaBoldWidths.gperf"
                                                        { "A", 722 },
                                                        { "", 0 },
#line 215 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Scedilla", 667 },
#line 84 "poppler/HelveticaBoldWidths.gperf"
                                                        { "four", 556 },
                                                        { "", 0 },
#line 211 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Dcroat", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 242 "poppler/HelveticaBoldWidths.gperf"
                                                        { "paragraph", 556 },
                                                        { "", 0 },
#line 78 "poppler/HelveticaBoldWidths.gperf"
                                                        { "W", 944 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 266 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Ncaron", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 165 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Adieresis", 722 },
#line 127 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Idotaccent", 278 },
#line 122 "poppler/HelveticaBoldWidths.gperf"
                                                        { "y", 556 },
#line 16 "poppler/HelveticaBoldWidths.gperf"
                                                        { "kcommaaccent", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 111 "poppler/HelveticaBoldWidths.gperf"
                                                        { "twosuperior", 333 },
#line 148 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Emacron", 667 },
                                                        { "", 0 },
#line 204 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ydieresis", 556 },
#line 168 "poppler/HelveticaBoldWidths.gperf"
                                                        { "slash", 278 },
#line 230 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ogonek", 333 },
                                                        { "", 0 },
#line 192 "poppler/HelveticaBoldWidths.gperf"
                                                        { "asterisk", 389 },
                                                        { "", 0 },
#line 299 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Zdotaccent", 611 },
#line 225 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Dcaron", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 59 "poppler/HelveticaBoldWidths.gperf"
                                                        { "backslash", 278 },
                                                        { "", 0 },
#line 120 "poppler/HelveticaBoldWidths.gperf"
                                                        { "w", 778 },
#line 58 "poppler/HelveticaBoldWidths.gperf"
                                                        { "iogonek", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 21 "poppler/HelveticaBoldWidths.gperf"
                                                        { "plusminus", 584 },
                                                        { "", 0 },
#line 14 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Ntilde", 722 },
                                                        { "", 0 },
#line 129 "poppler/HelveticaBoldWidths.gperf"
                                                        { "quotedbl", 474 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 322 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Rcaron", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 77 "poppler/HelveticaBoldWidths.gperf"
                                                        { "V", 667 },
#line 260 "poppler/HelveticaBoldWidths.gperf"
                                                        { "uogonek", 611 },
                                                        { "", 0 },
#line 169 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Edieresis", 667 },
#line 81 "poppler/HelveticaBoldWidths.gperf"
                                                        { "equal", 584 },
                                                        { "", 0 },
#line 38 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Omacron", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 121 "poppler/HelveticaBoldWidths.gperf"
                                                        { "x", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 264 "poppler/HelveticaBoldWidths.gperf"
                                                        { "igrave", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 68 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Q", 778 },
#line 163 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Umacron", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 50 "poppler/HelveticaBoldWidths.gperf"
                                                        { "E", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 22 "poppler/HelveticaBoldWidths.gperf"
                                                        { "circumflex", 333 },
#line 195 "poppler/HelveticaBoldWidths.gperf"
                                                        { "ugrave", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 87 "poppler/HelveticaBoldWidths.gperf"
                                                        { "b", 611 },
#line 17 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Ncommaaccent", 722 },
                                                        { "", 0 },
#line 175 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Odieresis", 778 },
                                                        { "", 0 },
#line 285 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Acircumflex", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 67 "poppler/HelveticaBoldWidths.gperf"
                                                        { "P", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 239 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Aring", 722 },
#line 96 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Oslash", 778 },
#line 297 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Imacron", 278 },
                                                        { "", 0 },
#line 180 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Udieresis", 722 },
                                                        { "", 0 },
#line 62 "poppler/HelveticaBoldWidths.gperf"
                                                        { "M", 833 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 124 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Gbreve", 778 },
#line 283 "poppler/HelveticaBoldWidths.gperf"
                                                        { "fi", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 119 "poppler/HelveticaBoldWidths.gperf"
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
#line 184 "poppler/HelveticaBoldWidths.gperf"
                                                        { "abreve", 556 },
#line 282 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Rcommaaccent", 722 },
#line 256 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Ohungarumlaut", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 141 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Ecircumflex", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 171 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Idieresis", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 69 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Uhungarumlaut", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 73 "poppler/HelveticaBoldWidths.gperf"
                                                        { "S", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 182 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Ydieresis", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 212 "poppler/HelveticaBoldWidths.gperf"
                                                        { "multiply", 584 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 66 "poppler/HelveticaBoldWidths.gperf"
                                                        { "O", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 31 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Aogonek", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 280 "poppler/HelveticaBoldWidths.gperf"
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
#line 227 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Ocircumflex", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 284 "poppler/HelveticaBoldWidths.gperf"
                                                        { "fl", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 75 "poppler/HelveticaBoldWidths.gperf"
                                                        { "U", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 152 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Ucircumflex", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 41 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Agrave", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 251 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Uring", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 99 "poppler/HelveticaBoldWidths.gperf"
                                                        { "k", 556 },
#line 234 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Eogonek", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 56 "poppler/HelveticaBoldWidths.gperf"
                                                        { "J", 556 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 218 "poppler/HelveticaBoldWidths.gperf"
                                                        { "partialdiff", 494 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 287 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Icircumflex", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 91 "poppler/HelveticaBoldWidths.gperf"
                                                        { "f", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 167 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Egrave", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 142 "poppler/HelveticaBoldWidths.gperf"
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
#line 55 "poppler/HelveticaBoldWidths.gperf"
                                                        { "I", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 244 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Uogonek", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 177 "poppler/HelveticaBoldWidths.gperf"
                                                        { "bracketleft", 333 },
#line 261 "poppler/HelveticaBoldWidths.gperf"
                                                        { "bracketright", 333 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 82 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Y", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 216 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Ograve", 778 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 83 "poppler/HelveticaBoldWidths.gperf"
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
#line 178 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Ugrave", 722 },
#line 34 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Iogonek", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 54 "poppler/HelveticaBoldWidths.gperf"
                                                        { "H", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 52 "poppler/HelveticaBoldWidths.gperf"
                                                        { "F", 611 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 46 "poppler/HelveticaBoldWidths.gperf"
                                                        { "B", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 247 "poppler/HelveticaBoldWidths.gperf"
                                                        { "Igrave", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 151 "poppler/HelveticaBoldWidths.gperf"
                                                        { "AE", 1000 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 63 "poppler/HelveticaBoldWidths.gperf"
                                                        { "N", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 162 "poppler/HelveticaBoldWidths.gperf"
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
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 49 "poppler/HelveticaBoldWidths.gperf"
                                                        { "D", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 114 "poppler/HelveticaBoldWidths.gperf"
                                                        { "OE", 1000 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 70 "poppler/HelveticaBoldWidths.gperf"
                                                        { "R", 722 } };

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
#line 331 "poppler/HelveticaBoldWidths.gperf"
