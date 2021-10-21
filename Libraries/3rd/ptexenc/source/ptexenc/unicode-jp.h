/*
  unicode-jp.h -- JIS X 0208 <=> UCS-2 converter
  written by N. Tsuchimura
*/

#ifndef PTEXENC_UNICODE_JP_H
#define PTEXENC_UNICODE_JP_H

#include <kpathsea/types.h>
#if defined(MIKTEX) && defined(__cplusplus)
extern "C" {
#endif

/* convert a JIS X 0208 char to UCS-2 */
extern int JIStoUCS2(int jis);

/* convert a UCS-2 char to JIS X 0208 */
extern int UCS2toJIS(int ucs2);

/* for U+3099 or  U+309A */
extern int get_voiced_sound(int ucs2, boolean semi);

#define U_BOM			0xFEFF
#define U_REPLACEMENT_CHARACTER	0xFFFD
#define U_VOICED		0x3099
#define U_SEMI_VOICED		0x309A

#if defined(MIKTEX) && defined(__cplusplus)
}
#endif
#endif /* PTEXENC_UNICODE_JP_H */
