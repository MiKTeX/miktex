#include <c-auto.h>
#if defined(MIKTEX)
#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif
#include <miktex/Core/c/api.h>
#endif

#include <kpathsea/config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) && defined(WITH_SYSTEM_ICU)
#include <icu.h>
#else
#include <unicode/utypes.h>
#include <unicode/ustring.h>
#include <unicode/uchar.h>
#include <unicode/ucol.h>
#include <unicode/unorm2.h>
#endif

#define PAGE_COMPOSIT_DEPTH 10
#define MAXDEPTH 5

struct page {
	char *page;
	char *enc;
	int attr[PAGE_COMPOSIT_DEPTH];
};

struct index {
	int num;
	unsigned char words;
	UChar *dic[MAXDEPTH];
	UChar *org[MAXDEPTH];
	UChar *idx[MAXDEPTH];
	struct page *p;
	int lnum;
};

#define INITIALLENGTH 10

struct hanzi_index {
	UChar idx[INITIALLENGTH];
	UChar threshold[3];
};

/* convert.c */
UChar *u_xstrdup (const UChar *string);
void initkanatable(void);
int convert(UChar *buff1, UChar *buff2);
int pnumconv(char *page, int attr);
int dicread(const char *filename);

/* pageread.c */
int lastpage(const char *filename);

/* sort.c */
void init_icu_collator();
void wsort(struct index *ind, int num);
void pagesort(struct index *ind, int num);
int is_latin(UChar *c);
int is_numeric(UChar *c);
int is_jpn_kana(UChar *c);
int is_kor_hngl(UChar *c);
int is_hanzi(UChar *c);
int is_zhuyin(UChar *c);
int is_cyrillic(UChar *c);
int is_greek(UChar *c);
int is_brahmic(UChar *c);
int is_thai(UChar *c);
int is_lao(UChar *c);
int is_arabic(UChar *c);
int is_hebrew(UChar *c);
int is_type_mark_or_punct(UChar *c);
int is_type_symbol(UChar *c);
int chkcontinue(struct page *p, int num);
int ss_comp(UChar *s1, UChar *s2);

#define CH_UNKNOWN      0
#define CH_LATIN        1
#define CH_CYRILLIC     2
#define CH_GREEK        3
#define CH_KANA         4
#define CH_HANGUL       5
#define CH_HANZI        6
#define CH_ARABIC       7
#define CH_HEBREW       8
#define CH_DEVANAGARI   9
#define CH_BENGALI     10
#define CH_GURMUKHI    11
#define CH_GUJARATI    12
#define CH_ORIYA       13
#define CH_TAMIL       14
#define CH_TELUGU      15
#define CH_KANNADA     16
#define CH_MALAYALAM   17
#define CH_SINHALA     18
#define CH_THAI        19
#define CH_LAO         20
#define CH_SYMBOL   0x100
#define CH_NUMERIC  0x101
#define  is_any_script(a)  ((CH_LATIN<=(a) && (a)<=CH_LAO))
#define NUM_BRAHMIC    12
#define BR_DEVA   CH_DEVANAGARI - CH_DEVANAGARI
#define BR_BENG   CH_BENGALI    - CH_DEVANAGARI
#define BR_GURU   CH_GURMUKHI   - CH_DEVANAGARI
#define BR_GUJR   CH_GUJARATI   - CH_DEVANAGARI
#define BR_ORYA   CH_ORIYA      - CH_DEVANAGARI
#define BR_TAML   CH_TAMIL      - CH_DEVANAGARI
#define BR_TELU   CH_TELUGU     - CH_DEVANAGARI
#define BR_KNDA   CH_KANNADA    - CH_DEVANAGARI
#define BR_MLYM   CH_MALAYALAM  - CH_DEVANAGARI
#define BR_SINH   CH_SINHALA    - CH_DEVANAGARI
#define BR_THAI   CH_THAI       - CH_DEVANAGARI
#define BR_LAO    CH_LAO        - CH_DEVANAGARI

/* sort.c */
int charset(UChar *c);

/* styfile.c */
void styread(const char *filename);
void set_icu_attributes(void);

/* fread.c */
int idxread(char *filename, int start);
int multibyte_to_widechar(UChar *wcstr, int32_t size, char *mbstr);
int widechar_to_multibyte(char *mbstr, int32_t size, UChar *wcstr);

/* fwrite.c */
int fprintf2   (FILE *fp, const char *format, ...);
void warn_printf(FILE *fp, const char *format, ...);
void verb_printf(FILE *fp, const char *format, ...);

struct index;
void indwrite(char *filename, struct index *ind, int pagenum);

#define  multibytelen(a)  ((a)<0x80 ? 1 : ((a)<0xc2 ? -2 : ((a)<0xe0 ? 2 : ((a)<0xf0 ? 3 : ((a)<0xf5 ? 4 : -1)))))
#define  is_surrogate_pair(a)   (U16_IS_LEAD(*(a)) && U16_IS_TRAIL(*(a+1)))

#if !defined(MIKTEX)
#ifdef WIN32
#undef fprintf
#undef fputs
#undef fopen
#define fprintf fprintf2
#define fputs   win32_fputs
#define fopen   fsyscp_fopen
#endif
#endif
