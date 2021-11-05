/*
   kanji.h: Handling 2byte char, and so on.
*/
#ifndef KANJI_H
#define KANJI_H
#include "cpascal.h"
#include <ptexenc/ptexenc.h>
#include <ptexenc/unicode.h>
#ifdef eupTeX
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

/* (e)upTeX does not allow file names with 0x5c */
#define not_kanji_char_seq(a,b) (1)
#define notkanjicharseq not_kanji_char_seq

/* functions */
#define XXHi(x) BYTE1(x)
#define XHi(x) BYTE2(x)
#define Hi(x) BYTE3(x)
#define Lo(x) BYTE4(x)

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
extern integer multilenbuffchar (integer c);

extern void init_default_kanji (const_string file_str, const_string internal_str);
extern void init_default_kanji_select (void);
/* for upTeX, e-upTeX, upBibTeX, upDVItype, upPLtoTF, and upTFtoPL */
#define initkanji() init_default_kanji_select()
/* for upDVItype */
#define setpriorfileenc() set_prior_file_enc()

/* number of rest of multi-char for kcode_pos */
#define nrestmultichr(x)  ( (x)!=0 ? ((x) / 8) + 2 - ((x) % 8) : -1 )

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
   return input_line2(f, buffer, pos, bufferSize, lastChar);
}
#else
#ifdef UPBIBTEX
#define inputline2(fp,buff,pos,size,ptr) input_line2(fp,buff,pos,size,ptr)
#else
#define inputline2(fp,buff,pos,size) input_line2(fp,buff,pos,size,NULL)
#endif
#endif

extern void init_kanji (const_string file_str, const_string internal_str);
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
