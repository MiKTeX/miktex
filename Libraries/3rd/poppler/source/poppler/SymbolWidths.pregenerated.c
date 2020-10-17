/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf poppler/SymbolWidths.gperf  */
/* Computed positions: -k'1-2,4-5,$' */

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

#line 1 "poppler/SymbolWidths.gperf"

#include <string.h>
#include "BuiltinFontWidth.h"

#define TOTAL_KEYWORDS 190
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 14
#define MIN_HASH_VALUE 5
#define MAX_HASH_VALUE 406
/* maximum key range = 402, duplicates = 0 */

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
    static const unsigned short asso_values[] = { 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407,
                                                  407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 80,  407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 60,  10,  5,   0,   45,  407, 25,  407, 25,
                                                  407, 0,   5,   30,  25,  15,  80,  407, 135, 5,   65,  70,  407, 407, 5,   407, 5,   407, 407, 407, 407, 407, 407, 0,   45,  100, 90,  15,  45,  145, 70,  5,   407, 30,  20,  105, 10,
                                                  70,  0,   50,  0,   25,  5,   145, 0,   220, 35,  0,   0,   407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407,
                                                  407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407,
                                                  407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407,
                                                  407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407, 407 };
    register unsigned int hval = len;

    switch (hval) {
    default:
        hval += asso_values[(unsigned char)str[4]];
    /*FALLTHROUGH*/
    case 4:
        hval += asso_values[(unsigned char)str[3]];
    /*FALLTHROUGH*/
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

const struct BuiltinFontWidth *SymbolWidthsLookup(register const char *str, register size_t len)
{
    static const struct BuiltinFontWidth wordlist[] = { { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 183 "poppler/SymbolWidths.gperf"
                                                        { "Kappa", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 109 "poppler/SymbolWidths.gperf"
                                                        { "pi", 549 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 180 "poppler/SymbolWidths.gperf"
                                                        { "Xi", 645 },
                                                        { "", 0 },
#line 144 "poppler/SymbolWidths.gperf"
                                                        { "zeta", 494 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 122 "poppler/SymbolWidths.gperf"
                                                        { "eta", 603 },
#line 133 "poppler/SymbolWidths.gperf"
                                                        { "Zeta", 611 },
#line 153 "poppler/SymbolWidths.gperf"
                                                        { "Delta", 612 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 203 "poppler/SymbolWidths.gperf"
                                                        { "Beta", 667 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 115 "poppler/SymbolWidths.gperf"
                                                        { "propersubset", 713 },
#line 174 "poppler/SymbolWidths.gperf"
                                                        { "psi", 686 },
#line 46 "poppler/SymbolWidths.gperf"
                                                        { "propersuperset", 713 },
#line 199 "poppler/SymbolWidths.gperf"
                                                        { "kappa", 549 },
#line 55 "poppler/SymbolWidths.gperf"
                                                        { "parenlefttp", 384 },
#line 121 "poppler/SymbolWidths.gperf"
                                                        { "parenrighttp", 384 },
#line 17 "poppler/SymbolWidths.gperf"
                                                        { "infinity", 713 },
#line 101 "poppler/SymbolWidths.gperf"
                                                        { "parenleft", 333 },
#line 82 "poppler/SymbolWidths.gperf"
                                                        { "parenright", 333 },
#line 23 "poppler/SymbolWidths.gperf"
                                                        { "parenleftbt", 384 },
#line 90 "poppler/SymbolWidths.gperf"
                                                        { "parenrightbt", 384 },
#line 140 "poppler/SymbolWidths.gperf"
                                                        { "perpendicular", 658 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 193 "poppler/SymbolWidths.gperf"
                                                        { "xi", 493 },
#line 185 "poppler/SymbolWidths.gperf"
                                                        { "bar", 200 },
#line 113 "poppler/SymbolWidths.gperf"
                                                        { "nine", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 97 "poppler/SymbolWidths.gperf"
                                                        { "intersection", 768 },
#line 111 "poppler/SymbolWidths.gperf"
                                                        { "Eta", 722 },
                                                        { "", 0 },
#line 22 "poppler/SymbolWidths.gperf"
                                                        { "apple", 790 },
                                                        { "", 0 },
#line 64 "poppler/SymbolWidths.gperf"
                                                        { "epsilon", 439 },
                                                        { "", 0 },
#line 93 "poppler/SymbolWidths.gperf"
                                                        { "angleleft", 329 },
#line 87 "poppler/SymbolWidths.gperf"
                                                        { "angleright", 329 },
                                                        { "", 0 },
#line 184 "poppler/SymbolWidths.gperf"
                                                        { "similar", 549 },
                                                        { "", 0 },
#line 26 "poppler/SymbolWidths.gperf"
                                                        { "beta", 549 },
#line 128 "poppler/SymbolWidths.gperf"
                                                        { "angle", 768 },
#line 127 "poppler/SymbolWidths.gperf"
                                                        { "partialdiff", 494 },
#line 179 "poppler/SymbolWidths.gperf"
                                                        { "reflexsubset", 713 },
#line 145 "poppler/SymbolWidths.gperf"
                                                        { "six", 500 },
#line 83 "poppler/SymbolWidths.gperf"
                                                        { "reflexsuperset", 713 },
                                                        { "", 0 },
#line 66 "poppler/SymbolWidths.gperf"
                                                        { "parenleftex", 384 },
#line 131 "poppler/SymbolWidths.gperf"
                                                        { "parenrightex", 384 },
#line 88 "poppler/SymbolWidths.gperf"
                                                        { "ellipsis", 1000 },
#line 159 "poppler/SymbolWidths.gperf"
                                                        { "plus", 549 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 40 "poppler/SymbolWidths.gperf"
                                                        { "phi", 521 },
#line 154 "poppler/SymbolWidths.gperf"
                                                        { "iota", 329 },
#line 171 "poppler/SymbolWidths.gperf"
                                                        { "seven", 500 },
                                                        { "", 0 },
#line 188 "poppler/SymbolWidths.gperf"
                                                        { "registersans", 790 },
#line 107 "poppler/SymbolWidths.gperf"
                                                        { "Chi", 722 },
#line 114 "poppler/SymbolWidths.gperf"
                                                        { "five", 500 },
#line 108 "poppler/SymbolWidths.gperf"
                                                        { "theta", 521 },
#line 167 "poppler/SymbolWidths.gperf"
                                                        { "florin", 500 },
#line 50 "poppler/SymbolWidths.gperf"
                                                        { "Epsilon", 611 },
                                                        { "", 0 },
#line 187 "poppler/SymbolWidths.gperf"
                                                        { "less", 549 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 98 "poppler/SymbolWidths.gperf"
                                                        { "Pi", 768 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 15 "poppler/SymbolWidths.gperf"
                                                        { "alpha", 631 },
#line 175 "poppler/SymbolWidths.gperf"
                                                        { "equivalence", 549 },
                                                        { "", 0 },
#line 102 "poppler/SymbolWidths.gperf"
                                                        { "one", 500 },
#line 139 "poppler/SymbolWidths.gperf"
                                                        { "Iota", 333 },
#line 62 "poppler/SymbolWidths.gperf"
                                                        { "heart", 753 },
#line 33 "poppler/SymbolWidths.gperf"
                                                        { "approxequal", 549 },
#line 160 "poppler/SymbolWidths.gperf"
                                                        { "proportional", 713 },
#line 100 "poppler/SymbolWidths.gperf"
                                                        { "registerserif", 790 },
#line 78 "poppler/SymbolWidths.gperf"
                                                        { "lessequal", 549 },
#line 92 "poppler/SymbolWidths.gperf"
                                                        { "eight", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 137 "poppler/SymbolWidths.gperf"
                                                        { "Ifraktur", 686 },
                                                        { "", 0 },
#line 49 "poppler/SymbolWidths.gperf"
                                                        { "equal", 549 },
#line 76 "poppler/SymbolWidths.gperf"
                                                        { "existential", 549 },
#line 60 "poppler/SymbolWidths.gperf"
                                                        { "Upsilon", 690 },
#line 168 "poppler/SymbolWidths.gperf"
                                                        { "Psi", 795 },
#line 70 "poppler/SymbolWidths.gperf"
                                                        { "therefore", 863 },
#line 161 "poppler/SymbolWidths.gperf"
                                                        { "delta", 494 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 53 "poppler/SymbolWidths.gperf"
                                                        { "four", 500 },
#line 32 "poppler/SymbolWidths.gperf"
                                                        { "Sigma", 592 },
                                                        { "", 0 },
#line 142 "poppler/SymbolWidths.gperf"
                                                        { "asteriskmath", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 37 "poppler/SymbolWidths.gperf"
                                                        { "three", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 150 "poppler/SymbolWidths.gperf"
                                                        { "carriagereturn", 658 },
#line 24 "poppler/SymbolWidths.gperf"
                                                        { "notelement", 713 },
                                                        { "", 0 },
#line 141 "poppler/SymbolWidths.gperf"
                                                        { "radical", 549 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 86 "poppler/SymbolWidths.gperf"
                                                        { "Gamma", 603 },
                                                        { "", 0 },
#line 80 "poppler/SymbolWidths.gperf"
                                                        { "lozenge", 494 },
#line 51 "poppler/SymbolWidths.gperf"
                                                        { "emptyset", 823 },
                                                        { "", 0 },
#line 45 "poppler/SymbolWidths.gperf"
                                                        { "sigma", 603 },
#line 138 "poppler/SymbolWidths.gperf"
                                                        { "degree", 400 },
#line 143 "poppler/SymbolWidths.gperf"
                                                        { "percent", 833 },
#line 105 "poppler/SymbolWidths.gperf"
                                                        { "rho", 549 },
                                                        { "", 0 },
#line 96 "poppler/SymbolWidths.gperf"
                                                        { "Theta", 741 },
#line 28 "poppler/SymbolWidths.gperf"
                                                        { "Lambda", 686 },
                                                        { "", 0 },
#line 117 "poppler/SymbolWidths.gperf"
                                                        { "trademarksans", 786 },
#line 91 "poppler/SymbolWidths.gperf"
                                                        { "radicalex", 500 },
#line 43 "poppler/SymbolWidths.gperf"
                                                        { "spade", 753 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 59 "poppler/SymbolWidths.gperf"
                                                        { "tau", 439 },
                                                        { "", 0 },
#line 195 "poppler/SymbolWidths.gperf"
                                                        { "Alpha", 722 },
                                                        { "", 0 },
#line 197 "poppler/SymbolWidths.gperf"
                                                        { "greater", 549 },
#line 29 "poppler/SymbolWidths.gperf"
                                                        { "Phi", 763 },
#line 126 "poppler/SymbolWidths.gperf"
                                                        { "zero", 500 },
#line 31 "poppler/SymbolWidths.gperf"
                                                        { "space", 250 },
#line 39 "poppler/SymbolWidths.gperf"
                                                        { "lambda", 549 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 73 "poppler/SymbolWidths.gperf"
                                                        { "semicolon", 278 },
#line 41 "poppler/SymbolWidths.gperf"
                                                        { "aleph", 823 },
#line 181 "poppler/SymbolWidths.gperf"
                                                        { "theta1", 631 },
#line 74 "poppler/SymbolWidths.gperf"
                                                        { "element", 713 },
#line 186 "poppler/SymbolWidths.gperf"
                                                        { "fraction", 167 },
#line 81 "poppler/SymbolWidths.gperf"
                                                        { "trademarkserif", 890 },
                                                        { "", 0 },
#line 61 "poppler/SymbolWidths.gperf"
                                                        { "bracelefttp", 494 },
#line 192 "poppler/SymbolWidths.gperf"
                                                        { "bracerighttp", 494 },
#line 173 "poppler/SymbolWidths.gperf"
                                                        { "notequal", 549 },
#line 130 "poppler/SymbolWidths.gperf"
                                                        { "braceleft", 480 },
#line 178 "poppler/SymbolWidths.gperf"
                                                        { "braceright", 480 },
#line 27 "poppler/SymbolWidths.gperf"
                                                        { "braceleftbt", 494 },
#line 169 "poppler/SymbolWidths.gperf"
                                                        { "bracerightbt", 494 },
#line 120 "poppler/SymbolWidths.gperf"
                                                        { "chi", 549 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 190 "poppler/SymbolWidths.gperf"
                                                        { "exclam", 333 },
#line 67 "poppler/SymbolWidths.gperf"
                                                        { "greaterequal", 549 },
#line 191 "poppler/SymbolWidths.gperf"
                                                        { "Upsilon1", 620 },
#line 20 "poppler/SymbolWidths.gperf"
                                                        { "plusminus", 549 },
#line 110 "poppler/SymbolWidths.gperf"
                                                        { "integraltp", 686 },
#line 194 "poppler/SymbolWidths.gperf"
                                                        { "period", 250 },
#line 75 "poppler/SymbolWidths.gperf"
                                                        { "upsilon", 576 },
#line 198 "poppler/SymbolWidths.gperf"
                                                        { "bracketlefttp", 384 },
#line 149 "poppler/SymbolWidths.gperf"
                                                        { "bracketrighttp", 384 },
#line 77 "poppler/SymbolWidths.gperf"
                                                        { "integralbt", 686 },
#line 104 "poppler/SymbolWidths.gperf"
                                                        { "bracketleft", 333 },
#line 157 "poppler/SymbolWidths.gperf"
                                                        { "bracketright", 333 },
#line 170 "poppler/SymbolWidths.gperf"
                                                        { "bracketleftbt", 384 },
#line 116 "poppler/SymbolWidths.gperf"
                                                        { "bracketrightbt", 384 },
#line 123 "poppler/SymbolWidths.gperf"
                                                        { "underscore", 500 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 176 "poppler/SymbolWidths.gperf"
                                                        { "universal", 713 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 134 "poppler/SymbolWidths.gperf"
                                                        { "braceex", 494 },
#line 201 "poppler/SymbolWidths.gperf"
                                                        { "integral", 274 },
#line 65 "poppler/SymbolWidths.gperf"
                                                        { "logicalor", 603 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 19 "poppler/SymbolWidths.gperf"
                                                        { "copyrightsans", 790 },
                                                        { "", 0 },
#line 182 "poppler/SymbolWidths.gperf"
                                                        { "logicalnot", 713 },
#line 63 "poppler/SymbolWidths.gperf"
                                                        { "divide", 549 },
                                                        { "", 0 },
#line 42 "poppler/SymbolWidths.gperf"
                                                        { "Tau", 611 },
#line 56 "poppler/SymbolWidths.gperf"
                                                        { "club", 753 },
#line 99 "poppler/SymbolWidths.gperf"
                                                        { "slash", 278 },
#line 165 "poppler/SymbolWidths.gperf"
                                                        { "second", 411 },
                                                        { "", 0 },
#line 132 "poppler/SymbolWidths.gperf"
                                                        { "Rfraktur", 795 },
#line 158 "poppler/SymbolWidths.gperf"
                                                        { "ampersand", 778 },
#line 119 "poppler/SymbolWidths.gperf"
                                                        { "integralex", 686 },
#line 84 "poppler/SymbolWidths.gperf"
                                                        { "sigma1", 439 },
                                                        { "", 0 },
#line 14 "poppler/SymbolWidths.gperf"
                                                        { "bracketleftex", 384 },
#line 156 "poppler/SymbolWidths.gperf"
                                                        { "bracketrightex", 384 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 162 "poppler/SymbolWidths.gperf"
                                                        { "copyrightserif", 790 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 79 "poppler/SymbolWidths.gperf"
                                                        { "phi1", 603 },
#line 38 "poppler/SymbolWidths.gperf"
                                                        { "numbersign", 500 },
#line 57 "poppler/SymbolWidths.gperf"
                                                        { "bullet", 460 },
#line 36 "poppler/SymbolWidths.gperf"
                                                        { "Omicron", 722 },
                                                        { "", 0 },
#line 106 "poppler/SymbolWidths.gperf"
                                                        { "circlemultiply", 768 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 48 "poppler/SymbolWidths.gperf"
                                                        { "question", 444 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 112 "poppler/SymbolWidths.gperf"
                                                        { "product", 823 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 16 "poppler/SymbolWidths.gperf"
                                                        { "union", 768 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 200 "poppler/SymbolWidths.gperf"
                                                        { "gradient", 713 },
                                                        { "", 0 },
#line 103 "poppler/SymbolWidths.gperf"
                                                        { "gamma", 411 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 151 "poppler/SymbolWidths.gperf"
                                                        { "suchthat", 439 },
                                                        { "", 0 },
#line 35 "poppler/SymbolWidths.gperf"
                                                        { "circleplus", 768 },
                                                        { "", 0 },
#line 172 "poppler/SymbolWidths.gperf"
                                                        { "braceleftmid", 494 },
#line 163 "poppler/SymbolWidths.gperf"
                                                        { "bracerightmid", 494 },
#line 71 "poppler/SymbolWidths.gperf"
                                                        { "notsubset", 713 },
#line 25 "poppler/SymbolWidths.gperf"
                                                        { "colon", 278 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 125 "poppler/SymbolWidths.gperf"
                                                        { "multiply", 549 },
                                                        { "", 0 },
#line 58 "poppler/SymbolWidths.gperf"
                                                        { "Omega", 768 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 89 "poppler/SymbolWidths.gperf"
                                                        { "Rho", 556 },
                                                        { "", 0 },
#line 18 "poppler/SymbolWidths.gperf"
                                                        { "comma", 250 },
#line 34 "poppler/SymbolWidths.gperf"
                                                        { "minute", 247 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 147 "poppler/SymbolWidths.gperf"
                                                        { "weierstrass", 987 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 47 "poppler/SymbolWidths.gperf"
                                                        { "omicron", 549 },
                                                        { "", 0 },
#line 148 "poppler/SymbolWidths.gperf"
                                                        { "summation", 713 },
#line 44 "poppler/SymbolWidths.gperf"
                                                        { "logicaland", 603 },
                                                        { "", 0 },
#line 21 "poppler/SymbolWidths.gperf"
                                                        { "arrowup", 603 },
#line 146 "poppler/SymbolWidths.gperf"
                                                        { "two", 500 },
                                                        { "", 0 },
#line 135 "poppler/SymbolWidths.gperf"
                                                        { "arrowdblup", 603 },
                                                        { "", 0 },
#line 85 "poppler/SymbolWidths.gperf"
                                                        { "nu", 521 },
                                                        { "", 0 },
#line 164 "poppler/SymbolWidths.gperf"
                                                        { "arrowleft", 987 },
#line 196 "poppler/SymbolWidths.gperf"
                                                        { "arrowright", 987 },
                                                        { "", 0 },
#line 129 "poppler/SymbolWidths.gperf"
                                                        { "arrowdblleft", 987 },
#line 177 "poppler/SymbolWidths.gperf"
                                                        { "arrowdblright", 987 },
#line 136 "poppler/SymbolWidths.gperf"
                                                        { "arrowdown", 603 },
#line 30 "poppler/SymbolWidths.gperf"
                                                        { "minus", 549 },
                                                        { "", 0 },
#line 94 "poppler/SymbolWidths.gperf"
                                                        { "arrowdbldown", 603 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 69 "poppler/SymbolWidths.gperf"
                                                        { "Nu", 722 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 54 "poppler/SymbolWidths.gperf"
                                                        { "Mu", 889 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 72 "poppler/SymbolWidths.gperf"
                                                        { "omega", 686 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 95 "poppler/SymbolWidths.gperf"
                                                        { "congruent", 549 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 124 "poppler/SymbolWidths.gperf"
                                                        { "Euro", 750 },
                                                        { "", 0 },
#line 152 "poppler/SymbolWidths.gperf"
                                                        { "arrowvertex", 603 },
#line 155 "poppler/SymbolWidths.gperf"
                                                        { "arrowhorizex", 1000 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 118 "poppler/SymbolWidths.gperf"
                                                        { "dotmath", 250 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 52 "poppler/SymbolWidths.gperf"
                                                        { "diamond", 753 },
                                                        { "", 0 },
#line 202 "poppler/SymbolWidths.gperf"
                                                        { "arrowboth", 1042 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 166 "poppler/SymbolWidths.gperf"
                                                        { "arrowdblboth", 1042 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 68 "poppler/SymbolWidths.gperf"
                                                        { "mu", 576 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
                                                        { "", 0 },
#line 189 "poppler/SymbolWidths.gperf"
                                                        { "omega1", 713 } };

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
#line 205 "poppler/SymbolWidths.gperf"
