/*
   kanji.h: Handling 2byte char, and so on.
*/
#ifndef KANJI_H
#define KANJI_H
#include "cpascal.h"
#include <ptexenc/ptexenc.h>
#ifdef epTeX
#include <ptexenc/unicode.h>
#if defined(MIKTEX)
#define getintone(w) ((w).c4p_P2.hh.c4p_P1.lh)
#define setintone(w,a) ((w).c4p_P2.hh.c4p_P1.lh=(a))
#else
#define getintone(w) ((w).cint1)
#define setintone(w,a) ((w).cint1=(a))
#endif
#endif
#include <zlib.h>

#ifndef KANJI
#define KANJI
#endif

/* allow file names with 0x5c in (e)pTeX on windows */
#if !defined(MIKTEX) && defined(WIN32)
#include <kpathsea/knj.h>
#define not_kanji_char_seq(a,b) (!(is_cp932_system && isknj(a) && isknj2(b)))
#else
#define not_kanji_char_seq(a,b) (1)
#endif
#define notkanjicharseq not_kanji_char_seq

/* functions */
#define Hi(x) (((x) >> 8) & 0xff)
#define Lo(x) ((x) & 0xff)

extern boolean check_kanji (integer c);
#define checkkanji check_kanji
extern boolean is_char_ascii (integer c);
#define ischarascii is_char_ascii
extern boolean is_char_kanji (integer c);
#define ischarkanji is_char_kanji
extern boolean ismultiprn (integer c);
extern integer calc_pos (integer c);
#define calcpos calc_pos
extern integer kcatcodekey (integer c);

extern void init_kanji (const_string file_str, const_string internal_str);
extern void init_default_kanji (const_string file_str, const_string internal_str);
#if defined(MIKTEX)
#define initkanji() init_default_kanji(NULL, "euc")
#else
#ifdef PBIBTEX
/* pBibTeX is EUC only */
#define initkanji() init_default_kanji(NULL, "euc")
#elif defined(WIN32)
/* for pTeX, e-pTeX, pDVItype, pPLtoTF, and pTFtoPL */
#define initkanji() init_default_kanji(NULL, "sjis")
#else
#define initkanji() init_default_kanji(NULL, "euc")
#endif
#endif
/* for pDVItype */
#define setpriorfileenc() set_prior_file_enc()

#ifndef PRESERVE_PUTC
#undef putc
#define putc(c,fp) putc2(c,fp)
#endif /* !PRESERVE_PUTC */

#ifndef PRESERVE_FPUTS
#undef fputs
#define fputs(c,fp) fputs2(c,fp)
#endif /* !PRESERVE_FPUTS */

#if defined(MIKTEX)
inline long inputline2(FILE* f, unsigned char* buffer, long pos, const long bufferSize, int* lastChar = nullptr)
{
   return input_line2(f, buffer, nullptr, pos, bufferSize, lastChar);
}
#else
#ifdef PBIBTEX
#define inputline2(fp,buff,pos,size,ptr) input_line2(fp,buff,NULL,pos,size,ptr)
#else
#define inputline2(fp,buff,pos,size) input_line2(fp,buff,NULL,pos,size,NULL)
#endif
#endif

#if defined(MIKTEX)
extern void dump_kanji(FILE* fp);
extern void undump_kanji(FILE* fp);
#else
extern void dump_kanji (gzFile fp);
extern void undump_kanji (gzFile fp);
#endif
#define dumpkanji dump_kanji
#define undumpkanji undump_kanji

#endif /* not KANJI_H */
