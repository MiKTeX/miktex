/*
 *  KANJI Code conversion routines.
 *  (for upTeX and e-upTeX)
 */

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif
#include "kanji.h"

#define CS_TOKEN_FLAG  0x1FFFFFFF
#define CJK_CHAR_LIMIT  0x1000000
#define CJK_TOKEN_FLAG   0xFFFFFF
#define KCAT_KANJI     16
#define KCAT_HANGUL    19

/* TOKEN */
boolean check_kanji (integer c)
{
    if (c >= CS_TOKEN_FLAG) return false;
    else if (!(XXHi(c)>=KCAT_KANJI && XXHi(c)<=KCAT_HANGUL)) return false;
    else return is_char_kanji(c);
}

boolean is_char_ascii(integer c)
{
    return (0 <= c && c < 0x100);
}

boolean is_char_kanji(integer c)
{
    if (is_internalUPTEX()) 
        return (c >= 0);
    else
        return iskanji1(Hi(c)) && iskanji2(Lo(c));
}

boolean ismultiprn(integer c)
{
    int i, j;

    for (i=2; i<=4; i++) for (j=1; j<=i; j++) {
        if (ismultichr(i,j,c)) return true;
    }
    return false;
}

integer calc_pos(integer c)
{
    unsigned char c1, c2;

    if(c>=0 && c<=255) return(c);
    c1 = Hi(c);
    c2 = Lo(c);

    c1 = (c1 % 4) * 64;  /* c1 = 0, 64, 128, 192 */
    c2 = c2 % 64;        /* c2 = 0..63 */
    return(c1 + c2);     /* ret = 0..255 */
}

/* Ref. http://www.unicode.org/Public/UNIDATA/Blocks.txt */
/* # Blocks-13.0.0.txt                                   */
/* # Date: 2019-07-10, 19:06:00 GMT [KW]                 */
static long ucs_range[]={
      0x0000, /* Basic Latin					     */ /* 0x00 */
      0x0080, /* Latin-1 Supplement				     */
      0x0100, /* Latin Extended-A				     */
      0x0180, /* Latin Extended-B				     */
      0x0250, /* IPA Extensions				     */
      0x02B0, /* Spacing Modifier Letters			     */
      0x0300, /* Combining Diacritical Marks			     */
      0x0370, /* Greek and Coptic				     */
      0x0400, /* Cyrillic					     */
      0x0500, /* Cyrillic Supplement				     */
      0x0530, /* Armenian					     */
      0x0590, /* Hebrew					     */
      0x0600, /* Arabic					     */
      0x0700, /* Syriac					     */
      0x0750, /* Arabic Supplement				     */
      0x0780, /* Thaana					     */
      0x07C0, /* NKo						     */ /* 0x10 */
      0x0800, /* Samaritan					     */
      0x0840, /* Mandaic					     */
      0x0860, /* Syriac Supplement				     */
      0x08A0, /* Arabic Extended-A				     */
      0x0900, /* Devanagari					     */
      0x0980, /* Bengali					     */
      0x0A00, /* Gurmukhi					     */
      0x0A80, /* Gujarati					     */
      0x0B00, /* Oriya						     */
      0x0B80, /* Tamil						     */
      0x0C00, /* Telugu					     */
      0x0C80, /* Kannada					     */
      0x0D00, /* Malayalam					     */
      0x0D80, /* Sinhala					     */
      0x0E00, /* Thai						     */
      0x0E80, /* Lao						     */ /* 0x20 */
      0x0F00, /* Tibetan					     */
      0x1000, /* Myanmar					     */
      0x10A0, /* Georgian					     */
      0x1100, /* Hangul Jamo					     */
      0x1200, /* Ethiopic					     */
      0x1380, /* Ethiopic Supplement				     */
      0x13A0, /* Cherokee					     */
      0x1400, /* Unified Canadian Aboriginal Syllabics		     */
      0x1680, /* Ogham						     */
      0x16A0, /* Runic						     */
      0x1700, /* Tagalog					     */
      0x1720, /* Hanunoo					     */
      0x1740, /* Buhid						     */
      0x1760, /* Tagbanwa					     */
      0x1780, /* Khmer						     */
      0x1800, /* Mongolian					     */ /* 0x30 */
      0x18B0, /* Unified Canadian Aboriginal Syllabics Extended      */
      0x1900, /* Limbu						     */
      0x1950, /* Tai Le					     */
      0x1980, /* New Tai Lue					     */
      0x19E0, /* Khmer Symbols					     */
      0x1A00, /* Buginese					     */
      0x1A20, /* Tai Tham					     */
      0x1AB0, /* Combining Diacritical Marks Extended		     */
      0x1B00, /* Balinese					     */
      0x1B80, /* Sundanese					     */
      0x1BC0, /* Batak						     */
      0x1C00, /* Lepcha					     */
      0x1C50, /* Ol Chiki					     */
      0x1C80, /* Cyrillic Extended-C				     */
      0x1C90, /* Georgian Extended				     */
      0x1CC0, /* Sundanese Supplement				     */ /* 0x40 */
      0x1CD0, /* Vedic Extensions				     */
      0x1D00, /* Phonetic Extensions				     */
      0x1D80, /* Phonetic Extensions Supplement		     */
      0x1DC0, /* Combining Diacritical Marks Supplement	     */
      0x1E00, /* Latin Extended Additional			     */
      0x1F00, /* Greek Extended				     */
      0x2000, /* General Punctuation				     */
      0x2070, /* Superscripts and Subscripts			     */
      0x20A0, /* Currency Symbols				     */
      0x20D0, /* Combining Diacritical Marks for Symbols	     */
      0x2100, /* Letterlike Symbols				     */
      0x2150, /* Number Forms					     */
      0x2190, /* Arrows					     */
      0x2200, /* Mathematical Operators			     */
      0x2300, /* Miscellaneous Technical			     */
      0x2400, /* Control Pictures				     */ /* 0x50 */
      0x2440, /* Optical Character Recognition			     */
      0x2460, /* Enclosed Alphanumerics			     */
      0x2500, /* Box Drawing					     */
      0x2580, /* Block Elements				     */
      0x25A0, /* Geometric Shapes				     */
      0x2600, /* Miscellaneous Symbols				     */
      0x2700, /* Dingbats					     */
      0x27C0, /* Miscellaneous Mathematical Symbols-A		     */
      0x27F0, /* Supplemental Arrows-A				     */
      0x2800, /* Braille Patterns				     */
      0x2900, /* Supplemental Arrows-B				     */
      0x2980, /* Miscellaneous Mathematical Symbols-B		     */
      0x2A00, /* Supplemental Mathematical Operators		     */
      0x2B00, /* Miscellaneous Symbols and Arrows		     */
      0x2C00, /* Glagolitic					     */
      0x2C60, /* Latin Extended-C				     */ /* 0x60 */
      0x2C80, /* Coptic					     */
      0x2D00, /* Georgian Supplement				     */
      0x2D30, /* Tifinagh					     */
      0x2D80, /* Ethiopic Extended				     */
      0x2DE0, /* Cyrillic Extended-A				     */
      0x2E00, /* Supplemental Punctuation			     */
      0x2E80, /* CJK Radicals Supplement			     */
      0x2F00, /* Kangxi Radicals				     */
      0x2FF0, /* Ideographic Description Characters		     */
      0x3000, /* CJK Symbols and Punctuation			     */
      0x3040, /* Hiragana					     */
      0x30A0, /* Katakana					     */
      0x3100, /* Bopomofo					     */
      0x3130, /* Hangul Compatibility Jamo			     */
      0x3190, /* Kanbun					     */
      0x31A0, /* Bopomofo Extended				     */ /* 0x70 */
      0x31C0, /* CJK Strokes					     */
      0x31F0, /* Katakana Phonetic Extensions			     */
      0x3200, /* Enclosed CJK Letters and Months		     */
      0x3300, /* CJK Compatibility				     */
      0x3400, /* CJK Unified Ideographs Extension A		     */
      0x4DC0, /* Yijing Hexagram Symbols			     */
      0x4E00, /* CJK Unified Ideographs			     */
      0xA000, /* Yi Syllables					     */
      0xA490, /* Yi Radicals					     */
      0xA4D0, /* Lisu						     */
      0xA500, /* Vai						     */
      0xA640, /* Cyrillic Extended-B				     */
      0xA6A0, /* Bamum						     */
      0xA700, /* Modifier Tone Letters				     */
      0xA720, /* Latin Extended-D				     */
      0xA800, /* Syloti Nagri					     */ /* 0x80 */
      0xA830, /* Common Indic Number Forms			     */
      0xA840, /* Phags-pa					     */
      0xA880, /* Saurashtra					     */
      0xA8E0, /* Devanagari Extended				     */
      0xA900, /* Kayah Li					     */
      0xA930, /* Rejang					     */
      0xA960, /* Hangul Jamo Extended-A			     */
      0xA980, /* Javanese					     */
      0xA9E0, /* Myanmar Extended-B				     */
      0xAA00, /* Cham						     */
      0xAA60, /* Myanmar Extended-A				     */
      0xAA80, /* Tai Viet					     */
      0xAAE0, /* Meetei Mayek Extensions			     */
      0xAB00, /* Ethiopic Extended-A				     */
      0xAB30, /* Latin Extended-E				     */
      0xAB70, /* Cherokee Supplement				     */ /* 0x90 */
      0xABC0, /* Meetei Mayek					     */
      0xAC00, /* Hangul Syllables				     */
      0xD7B0, /* Hangul Jamo Extended-B			     */
      0xD800, /* High Surrogates				     */
      0xDB80, /* High Private Use Surrogates			     */
      0xDC00, /* Low Surrogates				     */
      0xE000, /* Private Use Area				     */
      0xF900, /* CJK Compatibility Ideographs			     */
      0xFB00, /* Alphabetic Presentation Forms			     */
      0xFB50, /* Arabic Presentation Forms-A			     */
      0xFE00, /* Variation Selectors				     */
      0xFE10, /* Vertical Forms				     */
      0xFE20, /* Combining Half Marks				     */
      0xFE30, /* CJK Compatibility Forms			     */
      0xFE50, /* Small Form Variants				     */
      0xFE70, /* Arabic Presentation Forms-B			     */ /* 0xa0 */
      0xFF00, /* Halfwidth and Fullwidth Forms			     */
      0xFFF0, /* Specials					     */
      0x10000, /* Linear B Syllabary				     */
      0x10080, /* Linear B Ideograms				     */
      0x10100, /* Aegean Numbers				     */
      0x10140, /* Ancient Greek Numbers			     */
      0x10190, /* Ancient Symbols				     */
      0x101D0, /* Phaistos Disc				     */
      0x10280, /* Lycian					     */
      0x102A0, /* Carian					     */
      0x102E0, /* Coptic Epact Numbers				     */
      0x10300, /* Old Italic					     */
      0x10330, /* Gothic					     */
      0x10350, /* Old Permic					     */
      0x10380, /* Ugaritic					     */
      0x103A0, /* Old Persian					     */ /* 0xb0 */
      0x10400, /* Deseret					     */
      0x10450, /* Shavian					     */
      0x10480, /* Osmanya					     */
      0x104B0, /* Osage					     */
      0x10500, /* Elbasan					     */
      0x10530, /* Caucasian Albanian				     */
      0x10600, /* Linear A					     */
      0x10800, /* Cypriot Syllabary				     */
      0x10840, /* Imperial Aramaic				     */
      0x10860, /* Palmyrene					     */
      0x10880, /* Nabataean					     */
      0x108E0, /* Hatran					     */
      0x10900, /* Phoenician					     */
      0x10920, /* Lydian					     */
      0x10980, /* Meroitic Hieroglyphs				     */
      0x109A0, /* Meroitic Cursive				     */ /* 0xc0 */
      0x10A00, /* Kharoshthi					     */
      0x10A60, /* Old South Arabian				     */
      0x10A80, /* Old North Arabian				     */
      0x10AC0, /* Manichaean					     */
      0x10B00, /* Avestan					     */
      0x10B40, /* Inscriptional Parthian			     */
      0x10B60, /* Inscriptional Pahlavi			     */
      0x10B80, /* Psalter Pahlavi				     */
      0x10C00, /* Old Turkic					     */
      0x10C80, /* Old Hungarian				     */
      0x10D00, /* Hanifi Rohingya				     */
      0x10E60, /* Rumi Numeral Symbols				     */
      0x10E80, /* Yezidi					     */
      0x10F00, /* Old Sogdian					     */
      0x10F30, /* Sogdian					     */
      0x10FB0, /* Chorasmian					     */ /* 0xd0 */
      0x10FE0, /* Elymaic					     */
      0x11000, /* Brahmi					     */
      0x11080, /* Kaithi					     */
      0x110D0, /* Sora Sompeng					     */
      0x11100, /* Chakma					     */
      0x11150, /* Mahajani					     */
      0x11180, /* Sharada					     */
      0x111E0, /* Sinhala Archaic Numbers			     */
      0x11200, /* Khojki					     */
      0x11280, /* Multani					     */
      0x112B0, /* Khudawadi					     */
      0x11300, /* Grantha					     */
      0x11400, /* Newa						     */
      0x11480, /* Tirhuta					     */
      0x11580, /* Siddham					     */
      0x11600, /* Modi						     */ /* 0xe0 */
      0x11660, /* Mongolian Supplement				     */
      0x11680, /* Takri					     */
      0x11700, /* Ahom						     */
      0x11800, /* Dogra					     */
      0x118A0, /* Warang Citi					     */
      0x11900, /* Dives Akuru					     */
      0x119A0, /* Nandinagari					     */
      0x11A00, /* Zanabazar Square				     */
      0x11A50, /* Soyombo					     */
      0x11AC0, /* Pau Cin Hau					     */
      0x11C00, /* Bhaiksuki					     */
      0x11C70, /* Marchen					     */
      0x11D00, /* Masaram Gondi				     */
      0x11D60, /* Gunjala Gondi				     */
      0x11EE0, /* Makasar					     */
      0x11FB0, /* Lisu Supplement				     */ /* 0xf0 */
      0x11FC0, /* Tamil Supplement				     */
      0x12000, /* Cuneiform					     */
      0x12400, /* Cuneiform Numbers and Punctuation		     */
      0x12480, /* Early Dynastic Cuneiform			     */
      0x13000, /* Egyptian Hieroglyphs				     */
      0x13430, /* Egyptian Hieroglyph Format Controls		     */
      0x14400, /* Anatolian Hieroglyphs			     */
      0x16800, /* Bamum Supplement				     */
      0x16A40, /* Mro						     */
      0x16AD0, /* Bassa Vah					     */
      0x16B00, /* Pahawh Hmong					     */
      0x16E40, /* Medefaidrin					     */
      0x16F00, /* Miao						     */
      0x16FE0, /* Ideographic Symbols and Punctuation		     */
      0x17000, /* Tangut					     */
      0x18800, /* Tangut Components				     */ /* 0x100 */
      0x18B00, /* Khitan Small Script				     */
      0x18D00, /* Tangut Supplement				     */
      0x1B000, /* Kana Supplement				     */
      0x1B100, /* Kana Extended-A				     */
      0x1B130, /* Small Kana Extension				     */
      0x1B170, /* Nushu					     */
      0x1BC00, /* Duployan					     */
      0x1BCA0, /* Shorthand Format Controls			     */
      0x1D000, /* Byzantine Musical Symbols			     */
      0x1D100, /* Musical Symbols				     */
      0x1D200, /* Ancient Greek Musical Notation		     */
      0x1D2E0, /* Mayan Numerals				     */
      0x1D300, /* Tai Xuan Jing Symbols			     */
      0x1D360, /* Counting Rod Numerals			     */
      0x1D400, /* Mathematical Alphanumeric Symbols		     */
      0x1D800, /* Sutton SignWriting				     */ /* 0x110 */
      0x1E000, /* Glagolitic Supplement			     */
      0x1E100, /* Nyiakeng Puachue Hmong			     */
      0x1E2C0, /* Wancho					     */
      0x1E800, /* Mende Kikakui				     */
      0x1E900, /* Adlam					     */
      0x1EC70, /* Indic Siyaq Numbers				     */
      0x1ED00, /* Ottoman Siyaq Numbers			     */
      0x1EE00, /* Arabic Mathematical Alphabetic Symbols	     */
      0x1F000, /* Mahjong Tiles				     */
      0x1F030, /* Domino Tiles					     */
      0x1F0A0, /* Playing Cards				     */
      0x1F100, /* Enclosed Alphanumeric Supplement		     */
      0x1F200, /* Enclosed Ideographic Supplement		     */
      0x1F300, /* Miscellaneous Symbols and Pictographs	     */
      0x1F600, /* Emoticons					     */
      0x1F650, /* Ornamental Dingbats				     */ /* 0x120 */
      0x1F680, /* Transport and Map Symbols			     */
      0x1F700, /* Alchemical Symbols				     */
      0x1F780, /* Geometric Shapes Extended			     */
      0x1F800, /* Supplemental Arrows-C			     */
      0x1F900, /* Supplemental Symbols and Pictographs		     */
      0x1FA00, /* Chess Symbols				     */
      0x1FA70, /* Symbols and Pictographs Extended-A		     */
      0x1FB00, /* Symbols for Legacy Computing			     */
      0x20000, /* CJK Unified Ideographs Extension B		     */
      0x2A700, /* CJK Unified Ideographs Extension C		     */
      0x2B740, /* CJK Unified Ideographs Extension D		     */
      0x2B820, /* CJK Unified Ideographs Extension E		     */
      0x2CEB0, /* CJK Unified Ideographs Extension F		     */
      0x2F800, /* CJK Compatibility Ideographs Supplement	     */
      0x30000, /* CJK Unified Ideographs Extension G		     */
      0x31350, /* reserved					     */ /* 0x130 */
      0x40000, /* reserved					     */
      0x50000, /* reserved					     */
      0x60000, /* reserved					     */
      0x70000, /* reserved					     */
      0x80000, /* reserved					     */
      0x90000, /* reserved					     */
      0xA0000, /* reserved					     */
      0xB0000, /* reserved					     */
      0xC0000, /* reserved					     */
      0xD0000, /* reserved					     */
      0xE0000, /* Tags						     */
      0xE0100, /* Variation Selectors Supplement		     */
      0xF0000, /* Supplementary Private Use Area-A		     */
      0x100000, /* Supplementary Private Use Area-B		     */
  /* Value over 0x10FFFF is illegal under Unicode,
     They are for some special use.  *** experimental ***  */
      0x110000, /* Reserved					     */
      0x120000, /* Reserved					     */ /* 0x140 */
      0x130000, /* Reserved					     */
      0x140000, /* Reserved					     */
      0x150000, /* Reserved					     */
      0x160000, /* Reserved					     */
      0x170000, /* Reserved					     */
      0x180000, /* Reserved					     */
      0x190000, /* Reserved					     */
      0x1A0000, /* Reserved					     */
      0x1B0000, /* Reserved					     */
      0x1C0000, /* Reserved					     */
      0x1D0000, /* Reserved					     */
      0x1E0000, /* Reserved					     */
      0x1F0000, /* Reserved					     */
      0x200000, /* Reserved					     */
      0x210000, /* Reserved					     */
      0x220000, /* Reserved					     */ /* 0x150 */
      CJK_CHAR_LIMIT
};

#define NUCS_RANGE (sizeof(ucs_range)/sizeof(ucs_range[0]))

/* Return i, which satisfies a[i] <= x < a[i+1]. */
/* 'i' is serached between left <= i <= right. */
/* If no i satisfies such condition, return left-1. */
/* [Okumura et al.] An algorithm encyclopedia in Java, p.236. */
static int
binary_search (long x, long *a, int left, int right)
{
    right++;
    while (left < right) {
        int mid = (left + right) / 2;
        if (a[mid] <= x ) left = mid + 1;
        else             right = mid;
    }
    return left - 1;
}

#define FEMININE_ORDINAL_INDICATOR             0x00AA
#define MASCULINE_ORDINAL_INDICATOR            0x00BA
#define LATIN_CAPITAL_LETTER_A_WITH_GRAVE      0x00C0
#define LATIN_CAPITAL_LETTER_O_WITH_DIAERESIS  0x00D6
#define LATIN_CAPITAL_LETTER_O_WITH_STROKE     0x00D8
#define LATIN_SMALL_LETTER_O_WITH_DIAERESIS    0x00F6
#define LATIN_SMALL_LETTER_O_WITH_STROKE       0x00F8
#define LATIN_SMALL_LETTER_Y_WITH_DIAERESIS    0x00FF
#define FULLWIDTH_DIGIT_0    0xFF10
#define FULLWIDTH_DIGIT_9    0xFF19
#define FULLWIDTH_CAPITAL_A  0xFF21
#define FULLWIDTH_CAPITAL_Z  0xFF3A
#define FULLWIDTH_SMALL_A    0xFF41
#define FULLWIDTH_SMALL_Z    0xFF5A
#define HALFWIDTH_KATAKANA_WO         0xFF66
#define HALFWIDTH_KATAKANA_SMALL_TSU  0xFF6F
#define HALFWIDTH_KATAKANA_A          0xFF71
#define HALFWIDTH_KATAKANA_N          0xFF9D

integer kcatcodekey(integer c)
{
    integer block;
    if (is_internalUPTEX()) {
        block = binary_search((long)c, ucs_range, 0, NUCS_RANGE-1);
        if (block==0x01) {
            /* Latin-1 Letters */
            if (   FEMININE_ORDINAL_INDICATOR ==c
               ||  MASCULINE_ORDINAL_INDICATOR==c
               || (LATIN_CAPITAL_LETTER_A_WITH_GRAVE <=c && c<=LATIN_CAPITAL_LETTER_O_WITH_DIAERESIS)
               || (LATIN_CAPITAL_LETTER_O_WITH_STROKE<=c && c<=LATIN_SMALL_LETTER_O_WITH_DIAERESIS  )
               || (LATIN_SMALL_LETTER_O_WITH_STROKE  <=c && c<=LATIN_SMALL_LETTER_Y_WITH_DIAERESIS  ) )
            return 0x1FD;
        }
        if (block==0xa1) {
            /* Fullwidth ASCII variants  except for U+FF01..FF0F, U+FF1A..FF20, U+FF3B..FF40, U+FF5B..FF5E */
            if (  (FULLWIDTH_DIGIT_0  <=c && c<=FULLWIDTH_DIGIT_9  )
               || (FULLWIDTH_CAPITAL_A<=c && c<=FULLWIDTH_CAPITAL_Z)
               || (FULLWIDTH_SMALL_A  <=c && c<=FULLWIDTH_SMALL_Z  ) )
            return 0x1FE;
        /* Halfwidth Katakana variants  except for U+FF65, U+FF70, U+FF9E..FF9F */
            if (  (HALFWIDTH_KATAKANA_WO <=c && c<=HALFWIDTH_KATAKANA_SMALL_TSU )
               || (HALFWIDTH_KATAKANA_A  <=c && c<=HALFWIDTH_KATAKANA_N  ) )
            return 0x1FF;
        }
        return block;
    } else {
        return Hi(toDVI(c));
    }
}

integer multilenbuffchar(integer c)
{
    c=toBUFF(c);
    if (BYTE1(c)) return 4;
    if (BYTE2(c)) return 3;
    if (BYTE3(c)) return 2;
    if (BYTE4(c)) return 1;
    return 0;
}

void init_kanji (const_string file_str, const_string internal_str)
{
    if (!set_enc_string (file_str, internal_str)) {
        fprintf (stderr, "Bad kanji encoding \"%s\" or \"%s\".\n",
                 file_str ? file_str  : "NULL",
                 internal_str ? internal_str : "NULL");
        uexit(1);
    }
}

void init_default_kanji (const_string file_str, const_string internal_str)
{
    char *p;

    init_kanji (file_str, internal_str);

    p = getenv ("PTEX_KANJI_ENC");
    if (p) {
        if (!set_enc_string (p, NULL))
            fprintf (stderr, "Ignoring bad kanji encoding \"%s\".\n", p);
    }

#if !defined(MIKTEX)
#ifdef WIN32
    p = kpse_var_value ("guess_input_kanji_encoding");
    if (p) {
        if (*p == '1' || *p == 'y' || *p == 't')
            infile_enc_auto = 1;
        free(p);
    }
#endif
#endif
}

void init_default_kanji_select(void)
{
#if defined(MIKTEX)
    enable_UPTEX(true);
    init_default_kanji("utf8", "uptex");
#else
    char *base;

    base = kpse_program_basename (argv[0]);

    if (FILESTRNCASEEQ(base, "p", 1) || FILESTRNCASEEQ(base, "ep", 2)) {

        enable_UPTEX (false); /* disable */
#if defined(WIN32)
/* pBibTeX is EUC only */
        if (FILESTRNCASEEQ(base, "pbibtex", 7)) {
            init_default_kanji(NULL, "euc");
        } else {
/* for pTeX, e-pTeX, pDVItype, pPLtoTF, and pTFtoPL */
            init_default_kanji(NULL, "sjis");
        }
#else
        init_default_kanji(NULL, "euc");
#endif

    } else {

/* for upTeX, e-upTeX, upBibTeX, upDVItype, upPLtoTF, and upTFtoPL */
        enable_UPTEX (true);  /* enable */
        init_default_kanji ("utf8", "uptex");

    }
#endif
}
