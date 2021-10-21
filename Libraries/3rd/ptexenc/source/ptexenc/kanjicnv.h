/*
 *  KANJI Code conversion routines.
 */

#ifndef PTEXENC_KANJICNV_H
#define PTEXENC_KANJICNV_H

#include <kpathsea/types.h>
#if defined(MIKTEX) && defined(__cplusplus)
extern "C" {
#endif

#define HI(x)     ((((int)(x)) >> 8) & 0xff)
#define LO(x)     ( ((int)(x))       & 0xff)
#define HILO(x,y) ((LO(x) << 8) | LO(y))

extern boolean isJISkanji1(int c);
extern boolean isJISkanji2(int c);

extern boolean isEUCkanji1(int c);
extern boolean isEUCkanji2(int c);

extern boolean isSJISkanji1(int c);
extern boolean isSJISkanji2(int c);

/* EUC <=> JIS X 0208 code conversion */
extern int EUCtoJIS(int c);
extern int JIStoEUC(int c);

/* Shift JIS <=> JIS Kanji code conversion */
extern int SJIStoJIS(int c);
extern int JIStoSJIS(int c);

/* Shift JIS <=> EUC Kanji code conversion */
extern int SJIStoEUC(int c);
extern int EUCtoSJIS(int c);

/* KUTEN => JIS kanji code conversion */
extern int KUTENtoJIS(int c);

#if defined(MIKTEX) && defined(__cplusplus)
}
#endif
#endif /* PTEXENC_KANJICNV_H */
