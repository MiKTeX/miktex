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

#include <unicode/utypes.h>
#include <unicode/ustring.h>
#include <unicode/uchar.h>
#include <unicode/ucol.h>

#define PAGE_COMPOSIT_DEPTH 10

struct page {
	char *page;
	char *enc;
	int attr[PAGE_COMPOSIT_DEPTH];
};

struct index {
	int num;
	unsigned char words;
	UChar *dic[3];
	UChar *org[3];
	UChar *idx[3];
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
int is_devanagari(UChar *c);
int is_thai(UChar *c);
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
#define CH_DEVANAGARI   7
#define CH_THAI         8
#define CH_ARABIC       9
#define CH_HEBREW      10
#define CH_SYMBOL   0x100
#define CH_NUMERIC  0x101
#define  is_any_script(a)  ((CH_LATIN<=(a) && (a)<=CH_HEBREW))

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
