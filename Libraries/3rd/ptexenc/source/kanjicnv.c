/*
 *  KANJI Code conversion routines.
 */

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif
#include <ptexenc/c-auto.h>
#include <ptexenc/kanjicnv.h>

boolean isJISkanji1(int c)
{
    c &= 0xff;
    return (0x21 <= c && c <= 0x7e);
}

boolean isJISkanji2(int c)
{
    c &= 0xff;
    return (0x21 <= c && c <= 0x7e);
}

boolean isEUCkanji1(int c)
{
    c &= 0xff;
    return (0xa1 <= c && c <= 0xfe);
}

boolean isEUCkanji2(int c)
{
    c &= 0xff;
    return (0xa1 <= c && c <= 0xfe);
}

boolean isSJISkanji1(int c)
{
    c &= 0xff;
    return ((0x81 <= c && c <= 0x9f) || (0xe0 <= c && c <= 0xfc));
}

boolean isSJISkanji2(int c)
{
    c &= 0xff;
    return (0x40 <= c && c <= 0xfc && c != 0x7f);
}

/* EUC <=> JIS X 0208 code conversion */
int EUCtoJIS(int kcode)
{
    if ((kcode<=0 || kcode>0x10000)) return 0;
    if (!isEUCkanji1(HI(kcode))) return 0;
    if (!isEUCkanji2(LO(kcode))) return 0;
    return (kcode & 0x7f7f);
}

int JIStoEUC(int kcode)
{
    if ((kcode<=0 || kcode>0x10000)) return 0;
    if (!isJISkanji1(HI(kcode))) return 0;
    if (!isJISkanji2(LO(kcode))) return 0;
    return (kcode | 0x8080);
}

/* Shift JIS <=> JIS Kanji code conversion */
int SJIStoJIS(int kcode)
{
    int byte1, byte2;

    if ((kcode<=0 || kcode>0x10000)) return 0;
    byte1 = HI(kcode); if (!isSJISkanji1(byte1)) return 0;
    byte2 = LO(kcode); if (!isSJISkanji2(byte2)) return 0;
    byte1 -= ( byte1>=0xa0 ) ? 0xc1 : 0x81;
    kcode = ((byte1<<1) + 0x21)<<8;
    if ( byte2 >= 0x9f ) {
        kcode += 0x0100;
        kcode |= (byte2 - 0x7e) & 0xff;
    } else {
        kcode |= (byte2 - ((byte2<=0x7e) ? 0x1f : 0x20 )) & 0xff;
    }
    return kcode;
}

int JIStoSJIS(int kcode)
{
    int high, low;
    int nh,   nl;

    if ((kcode<=0 || kcode>0x10000)) return 0;
    high = HI(kcode); if (!isJISkanji1(high)) return 0;
    low  = LO(kcode); if (!isJISkanji2(low)) return 0;
    nh = ((high-0x21)>>1) + 0x81;
    if (nh > 0x9f) nh += 0x40;
    if (high & 1) {
        nl = low + 0x1f;
        if (low > 0x5f) nl++;
    } else
        nl = low + 0x7e;
    return HILO(nh, nl);
}

/* Shift JIS <=> EUC Kanji code conversion */
int SJIStoEUC(int kcode)
{
    return JIStoEUC(SJIStoJIS(kcode));
}

int EUCtoSJIS(int kcode)
{
    return JIStoSJIS(EUCtoJIS(kcode));
}

/* KUTEN to JIS kanji code conversion */
int KUTENtoJIS(int kcode)
{
    /* in case of undefined in kuten code table */
    if (HI(kcode) == 0 || HI(kcode) > 94) return 0;
    if (LO(kcode) == 0 || LO(kcode) > 94) return 0;

    return kcode + 0x2020;
}
