/*
  unicode.h -- utilities for Unicode
  written by N. Tsuchimura
*/

#ifndef PTEXENC_UNICODE_H
#define PTEXENC_UNICODE_H

#include <kpathsea/types.h>
#include <ptexenc/ptexenc.h> /* for PTENCDLL */
#if defined(MIKTEX) && defined(__cplusplus)
extern "C" {
#endif

/* determine if UTF-8 character or not */
extern boolean isUTF8(int length, int nth, int c);

extern int UTF8length(int first_byte);
extern int UTF8Slength(unsigned char *buff, int buff_len);
extern long UTF8StoUCS(unsigned char *s);
extern long PTENCDLL UCStoUTF8(long ucs);

extern long UCStoUPTEX(long ucs);
extern long UPTEXtoUCS(long uptex);

#define LONG(a,b,c,d) ( ((long)(a)<<24) | ((long)(b)<<16) | ((c)<<8) | (d) )
#define BYTE1(x) (((x)>>24) & 0xff)
#define BYTE2(x) (((x)>>16) & 0xff)
#define BYTE3(x) (((x)>> 8) & 0xff)
#define BYTE4(x) ( (x)      & 0xff)
 

/* UTF-8 -> UCS */
#define UTF8BtoUCS(a,b)   ((((a)&0x1f) << 6) | ((b)&0x3f))
#define UTF8CtoUCS(a,b,c) ((((a)&0x0f) <<12) | (((b)&0x3f) << 6) | ((c)&0x3f))
#define UTF8DtoUCS(a,b,c,d) ((((long)(a)&0x07) <<18) | \
			     (((long)(b)&0x3f) <<12) | \
			     (((long)(c)&0x3f) << 6) | ((d)&0x3f))

/* UCS -> UTF-8 */
#define UCStoUTF8B1(x)  (0xc0 + (((x) >>  6) & 0x1f))
#define UCStoUTF8B2(x)  (0x80 + (((x)      ) & 0x3f))

#define UCStoUTF8C1(x)  (0xe0 + (((x) >> 12) & 0x0f))
#define UCStoUTF8C2(x)  (0x80 + (((x) >>  6) & 0x3f))
#define UCStoUTF8C3(x)  (0x80 + (((x)      ) & 0x3f))

#define UCStoUTF8D1(x)  (0xf0 + (((x) >> 18) & 0x07))
#define UCStoUTF8D2(x)  (0x80 + (((x) >> 12) & 0x3f))
#define UCStoUTF8D3(x)  (0x80 + (((x) >>  6) & 0x3f))
#define UCStoUTF8D4(x)  (0x80 + (((x)      ) & 0x3f))

/* UTF-32 over U+FFFF -> UTF-16 surrogate pair */
#define UTF32toUTF16HS(x)  (0xd800 + ((((x)-0x10000) >> 10) & 0x3ff))
#define UTF32toUTF16LS(x)  (0xdc00 + (  (x)                 & 0x3ff))

/* UTF-16 surrogate pair -> UTF-32 over U+FFFF */
#define UTF16StoUTF32(x,y) ((((x) & 0x3ff) << 10) + ((y) & 0x3ff) + 0x10000)

#if defined(MIKTEX) && defined(__cplusplus)
}
#endif
#endif /* PTEXENC_UNICODE_H */
