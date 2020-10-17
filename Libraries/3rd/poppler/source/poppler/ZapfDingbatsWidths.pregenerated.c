/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf poppler/ZapfDingbatsWidths.gperf  */
/* Computed positions: -k'2-4' */

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

#line 1 "poppler/ZapfDingbatsWidths.gperf"

#include <string.h>
#include "BuiltinFontWidth.h"

#define TOTAL_KEYWORDS 202
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 5
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 402
/* maximum key range = 401, duplicates = 0 */

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
    static const unsigned short asso_values[] = { 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403,
                                                  403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 35,  18,  180, 160, 140, 120, 100, 80,  60,  15,  195, 185, 217, 207, 218, 203, 0,   5,   10,  200,
                                                  195, 190, 185, 149, 15,  403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 0,   403, 403, 403, 403,
                                                  403, 403, 403, 403, 403, 403, 0,   403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 0,   403, 403, 403, 403, 403, 403, 403, 403,
                                                  403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403,
                                                  403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403,
                                                  403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403,
                                                  403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403, 403 };
    register unsigned int hval = len;

    switch (hval) {
    default:
        hval += asso_values[(unsigned char)str[3] + 9];
    /*FALLTHROUGH*/
    case 3:
        hval += asso_values[(unsigned char)str[2]];
    /*FALLTHROUGH*/
    case 2:
        hval += asso_values[(unsigned char)str[1] + 15];
        break;
    }
    return hval;
}

const struct BuiltinFontWidth *ZapfDingbatsWidthsLookup(register const char *str, register size_t len)
{
    static const struct BuiltinFontWidth wordlist[] = { { "", 0 },        { "", 0 },
#line 84 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a1", 974 },    { "", 0 }, { "", 0 },
#line 36 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "space", 278 }, { "", 0 },
#line 85 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a2", 961 },    { "", 0 }, { "", 0 }, { "", 0 }, { "", 0 },
#line 86 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a3", 980 },    { "", 0 }, { "", 0 }, { "", 0 }, { "", 0 },
#line 92 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a9", 577 },
#line 112 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a19", 755 },
#line 100 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a197", 771 },  { "", 0 },
#line 104 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a11", 960 },
#line 166 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a117", 690 },
#line 122 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a29", 786 },
#line 101 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a198", 888 },  { "", 0 },
#line 114 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a21", 762 },
#line 167 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a118", 791 },
#line 132 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a39", 823 },
#line 102 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a199", 867 },  { "", 0 },
#line 124 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a31", 788 },
#line 168 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a119", 790 },
#line 43 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a99", 668 },
#line 93 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a190", 970 },  { "", 0 },
#line 34 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a91", 276 },
#line 163 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a110", 694 },
#line 103 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a10", 692 },
#line 150 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a107", 760 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 113 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a20", 846 },
#line 151 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a108", 760 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 123 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a30", 788 },
#line 152 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a109", 626 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 33 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a90", 390 },
#line 143 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a100", 668 },  { "", 0 }, { "", 0 }, { "", 0 }, { "", 0 },
#line 179 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a200", 696 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 111 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a18", 974 },
#line 81 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a187", 831 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 121 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a28", 754 },
#line 82 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a188", 873 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 131 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a38", 841 },
#line 83 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a189", 927 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 42 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a98", 392 },
#line 74 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a180", 867 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 110 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a17", 945 },
#line 71 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a177", 463 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 120 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a27", 759 },
#line 72 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a178", 883 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 130 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a37", 789 },
#line 73 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a179", 836 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 41 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a97", 392 },
#line 64 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a170", 834 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 109 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a16", 933 },
#line 61 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a167", 927 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 119 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a26", 760 },
#line 62 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a168", 928 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 129 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a36", 823 },
#line 63 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a169", 928 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 40 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a96", 334 },
#line 54 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a160", 894 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 108 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a15", 911 },
#line 51 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a157", 788 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 118 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a25", 763 },
#line 52 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a158", 788 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 128 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a35", 816 },
#line 53 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a159", 788 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 39 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a95", 334 },
#line 44 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a150", 788 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 107 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a14", 855 },
#line 30 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a147", 788 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 117 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a24", 677 },
#line 31 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a148", 788 },  { "", 0 },
#line 91 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a8", 537 },    { "", 0 },
#line 127 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a34", 794 },
#line 32 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a149", 788 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 38 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a94", 317 },
#line 23 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a140", 788 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 106 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a13", 549 },
#line 213 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a137", 788 },  { "", 0 }, { "", 0 },
#line 22 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a89", 390 },
#line 116 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a23", 571 },
#line 214 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a138", 788 },
#line 14 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a81", 438 },   { "", 0 }, { "", 0 },
#line 126 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a33", 793 },
#line 215 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a139", 788 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 37 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a93", 317 },
#line 206 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a130", 788 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 105 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a12", 939 },
#line 193 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a127", 788 },  { "", 0 }, { "", 0 },
#line 90 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a7", 552 },
#line 115 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a22", 761 },
#line 194 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a128", 788 },  { "", 0 }, { "", 0 },
#line 89 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a6", 494 },
#line 125 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a32", 790 },
#line 195 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a129", 788 },  { "", 0 }, { "", 0 },
#line 88 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a5", 789 },
#line 35 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a92", 276 },
#line 181 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a120", 788 },  { "", 0 }, { "", 0 },
#line 87 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a4", 719 },
#line 205 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a79", 784 },
#line 95 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a192", 748 },  { "", 0 },
#line 197 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a71", 791 },
#line 165 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a112", 776 },
#line 178 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a69", 791 },   { "", 0 }, { "", 0 },
#line 170 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a61", 789 },
#line 21 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a88", 234 },
#line 162 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a59", 815 },
#line 94 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a191", 918 },  { "", 0 },
#line 154 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a51", 768 },
#line 164 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a111", 595 },
#line 142 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a49", 695 },   { "", 0 }, { "", 0 },
#line 134 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a41", 816 },
#line 99 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a196", 748 },
#line 196 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a70", 785 },
#line 145 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a102", 544 },  { "", 0 },
#line 97 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a194", 771 },  { "", 0 },
#line 169 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a60", 789 },
#line 183 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a202", 974 },  { "", 0 }, { "", 0 },
#line 20 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a87", 234 },
#line 153 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a50", 776 },
#line 144 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a101", 732 },  { "", 0 },
#line 96 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a193", 836 },
#line 98 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a195", 888 },
#line 133 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a40", 833 },
#line 180 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a201", 874 },  { "", 0 }, { "", 0 },
#line 149 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a106", 667 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 147 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a104", 910 },
#line 191 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a206", 410 },
#line 204 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a78", 788 },
#line 76 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a182", 874 },  { "", 0 },
#line 187 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a204", 759 },
#line 19 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a86", 410 },
#line 177 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a68", 713 },   { "", 0 }, { "", 0 },
#line 146 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a103", 544 },
#line 148 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a105", 911 },
#line 161 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a58", 826 },
#line 75 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a181", 696 },  { "", 0 },
#line 185 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a203", 762 },
#line 189 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a205", 509 },
#line 141 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a48", 792 },   { "", 0 }, { "", 0 }, { "", 0 },
#line 80 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a186", 967 },
#line 203 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a77", 892 },
#line 66 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a172", 828 },  { "", 0 },
#line 78 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a184", 946 },
#line 18 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a85", 509 },
#line 176 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a67", 787 },   { "", 0 }, { "", 0 }, { "", 0 }, { "", 0 },
#line 160 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a57", 701 },
#line 65 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a171", 873 },  { "", 0 },
#line 77 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a183", 760 },
#line 79 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a185", 865 },
#line 140 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a47", 790 },   { "", 0 }, { "", 0 }, { "", 0 },
#line 70 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a176", 931 },
#line 202 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a76", 892 },
#line 56 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a162", 924 },  { "", 0 },
#line 68 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a174", 917 },
#line 17 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a84", 415 },
#line 175 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a66", 786 },   { "", 0 }, { "", 0 }, { "", 0 }, { "", 0 },
#line 159 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a56", 682 },
#line 55 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a161", 838 },  { "", 0 },
#line 67 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a173", 924 },
#line 69 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a175", 930 },
#line 139 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a46", 749 },   { "", 0 }, { "", 0 }, { "", 0 },
#line 60 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a166", 918 },
#line 201 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a75", 759 },
#line 46 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a152", 788 },  { "", 0 },
#line 58 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a164", 458 },
#line 16 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a83", 277 },
#line 174 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a65", 689 },   { "", 0 }, { "", 0 }, { "", 0 }, { "", 0 },
#line 158 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a55", 708 },
#line 45 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a151", 788 },  { "", 0 },
#line 57 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a163", 1016 },
#line 59 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a165", 924 },
#line 138 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a45", 723 },   { "", 0 }, { "", 0 }, { "", 0 },
#line 50 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a156", 788 },
#line 200 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a74", 762 },
#line 25 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a142", 788 },  { "", 0 },
#line 48 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a154", 788 },
#line 15 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a82", 138 },
#line 173 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a64", 696 },   { "", 0 }, { "", 0 }, { "", 0 }, { "", 0 },
#line 157 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a54", 707 },
#line 24 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a141", 788 },  { "", 0 },
#line 47 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a153", 788 },
#line 49 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a155", 788 },
#line 137 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a44", 744 },   { "", 0 }, { "", 0 }, { "", 0 },
#line 29 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a146", 788 },
#line 199 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a73", 761 },
#line 208 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a132", 788 },  { "", 0 },
#line 27 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a144", 788 },  { "", 0 },
#line 172 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a63", 687 },   { "", 0 }, { "", 0 }, { "", 0 }, { "", 0 },
#line 156 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a53", 759 },
#line 207 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a131", 788 },  { "", 0 },
#line 26 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a143", 788 },
#line 28 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a145", 788 },
#line 136 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a43", 923 },   { "", 0 }, { "", 0 }, { "", 0 },
#line 212 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a136", 788 },
#line 198 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a72", 873 },
#line 184 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a122", 788 },  { "", 0 },
#line 210 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a134", 788 },  { "", 0 },
#line 171 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a62", 707 },   { "", 0 }, { "", 0 }, { "", 0 }, { "", 0 },
#line 155 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a52", 792 },
#line 182 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a121", 788 },  { "", 0 },
#line 209 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a133", 788 },
#line 211 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a135", 788 },
#line 135 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a42", 831 },   { "", 0 }, { "", 0 }, { "", 0 },
#line 192 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a126", 788 },  { "", 0 }, { "", 0 }, { "", 0 },
#line 188 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a124", 788 },  { "", 0 }, { "", 0 }, { "", 0 }, { "", 0 }, { "", 0 }, { "", 0 }, { "", 0 }, { "", 0 }, { "", 0 },
#line 186 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a123", 788 },
#line 190 "poppler/ZapfDingbatsWidths.gperf"
                                                        { "a125", 788 } };

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
#line 217 "poppler/ZapfDingbatsWidths.gperf"
