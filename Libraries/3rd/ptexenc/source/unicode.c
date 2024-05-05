/*
  unicode.c -- utilities for Unicode
  written by N. Tsuchimura
*/

#if defined(MIKTEX_WINDOWS)
#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>
#endif
#include <ptexenc/c-auto.h>
#include <ptexenc/unicode.h>
#include <ptexenc/kanjicnv.h>

#include <stdio.h>  /* for fprintf() */

/* determine if UTF-8 character or not */
boolean isUTF8(int length, int nth, int c)
{
    c &= 0xff;
    switch (length * 8 + nth) {
    case 011: return (0x00 <= c && c < 0x80);
    case 021: return (0xc2 <= c && c < 0xe0);
    case 031: return (0xe0 <= c && c < 0xf0);
    case 041: return (0xf0 <= c && c < 0xf5);
    case 022:
    case 032: case 033:
    case 042: case 043: case 044:
        return (0x80 <= c && c < 0xc0);
    default:
        fprintf(stderr, "isUTF8: unexpected param length=%d, nth=%d\n",
                length, nth);
    }        
    return false;
}


int UTF8length(int first_byte)
{
    first_byte &= 0xff;
    if (first_byte < 0x80) return 1;
    if (first_byte < 0xc2) return -2;  /* illegal */
    if (first_byte < 0xe0) return 2;
    if (first_byte < 0xf0) return 3;
    if (first_byte < 0xf5) return 4;
    return -1; /* reserved/undefined */
}


/* with strict range check */
#define DEFINE_UTF8SLENGTH(SUFF,TYPE) \
int UTF8Slength ## SUFF(TYPE *buff, int buff_len) \
{ \
    int i, len; \
    len = UTF8length(buff[0]); \
    if (len < 0) return -2; /* illegal */ \
    if (len > buff_len) return -3; /* overflow */ \
    for (i=0; i<len; i++) { \
        if (!isUTF8(len, 1+i, buff[i])) return -1; /* not UTF-8 */ \
    } \
    return len; \
}

DEFINE_UTF8SLENGTH(, unsigned char)
DEFINE_UTF8SLENGTH(short, unsigned short)


/* WITHOUT strict range check */
long UTF8StoUCS(unsigned char *s)
{
    switch (UTF8length(s[0])) {
    case 1: return s[0];
    case 2: return UTF8BtoUCS(s[0], s[1]);
    case 3: return UTF8CtoUCS(s[0], s[1], s[2]);
    case 4: return UTF8DtoUCS(s[0], s[1], s[2], s[3]);
    default: return 0; /* error */
    }
}

/* used only for debug */
#ifdef DEBUG
int UCStoUTF8S(long ucs, unsigned char *s)
{
    if (ucs < 0x80) {
	*s = ucs;
	return 1;
    } else if (ucs < 0x800) {
	*s++ = UCStoUTF8B1(ucs);
	*s++ = UCStoUTF8B2(ucs);
	return 2;
    } else if (ucs < 0xFFFF) {
	*s++ = UCStoUTF8C1(ucs);
	*s++ = UCStoUTF8C2(ucs);
	*s++ = UCStoUTF8C3(ucs);
	return 3;
    } else if (ucs < 0x10FFFF) {
	*s++ = UCStoUTF8D1(ucs);
	*s++ = UCStoUTF8D2(ucs);
	*s++ = UCStoUTF8D3(ucs);
	*s++ = UCStoUTF8D4(ucs);
	return 4;
    }
    return 0; /* unsupported */
}
#endif /* DEBUG */


long UCStoUTF8(long ucs)
{
    if (ucs < 0x80)     return ucs;
    if (ucs < 0x800)    return LONG(0,0,
				    UCStoUTF8B1(ucs),
				    UCStoUTF8B2(ucs));
    if (ucs < 0xFFFF)   return LONG(0,
				    UCStoUTF8C1(ucs),
				    UCStoUTF8C2(ucs),
				    UCStoUTF8C3(ucs));
    if (ucs < 0x10FFFF) return LONG(UCStoUTF8D1(ucs),
				    UCStoUTF8D2(ucs),
				    UCStoUTF8D3(ucs),
				    UCStoUTF8D4(ucs));
    return 0; /* unsupported */
}


#define UCS_MAX 0x110000L

/* using over U+10.FFFF Area */
long UCStoUPTEX (long ucs)
{
    return ucs;
}

/* using over U+10.FFFF Area */
long UPTEXtoUCS (long uptex)
{
    long vs[2];
    if ((uptex>=0x220000 && uptex<=0x2FFFFF) || /* for 2-codepoint sequence */
         uptex>=0x400000) {               /* for SVS, IVS */
        if (UVS_divide_code(uptex, vs) == 0) return 0;
        return uptex;
    }
    if (uptex>=UCS_MAX*2) return uptex;
    if (uptex>=UCS_MAX)   return uptex % UCS_MAX; /* for OTF package */
    return uptex;
}

int
UVS_get_codepoint_length(long ucv)
{
    int len = 0;

    if (ucv <  0x220000) {
        len = 1;
    }
    else if (ucv >= 0x25E6E6 && ucv <= 0x25FFFF) {    /* RGI Emoji Flag Sequence */
        if ((ucv & 0xFF) < 0xE6) return 0;  /* illegal value */
        len = 2;
    }
    else if (ucv >= 0x800000 && ucv <= 0x80007F) {    /* Emoji Keycap Sequence */
        ucv &= 0x7F;
        if ( ucv==0x23 || ucv==0x2A ||
            (ucv>=0x30 && ucv<=0x39) ) len = 3;
        else return 0;  /* illegal value */
    }
    else if (ucv <  0x300000) {
        len = 2;
    }
    else if (ucv >= 0x400000 && ucv <= 0x43FFFFF) {
        len = 2;
    }

    return len;
}

long
UVS_get_codepoint_in_sequence(long ucv, int n)
{
    long uvs, cp[3], len;

    cp[1] = cp[2] = 0;

    if (ucv <  0x220000) {
        cp[0] =   ucv % 0x110000;
        len = 1;
    }
    else if (ucv >= 0x25E6E6 && ucv <= 0x25FFFF) {    /* RGI Emoji Flag Sequence */
        if ((ucv & 0xFF) < 0xE6) return 0;  /* illegal value */
        cp[0] = ((ucv >> 8) & 0xFF) + 0x1F100;
        cp[1] =  (ucv       & 0xFF) + 0x1F100;
        len = 2;
    }
    else if (ucv >= 0x800000 && ucv <= 0x80007F) {    /* Emoji Keycap Sequence */
        cp[0] =   ucv & 0x7F;
        cp[1] =   0xFE0F;
        cp[2] =   0x20E3;
        len = 3;
    }
    else if (ucv <  0x300000) {
        len = 2;
        uvs = ucv >> 16;
        if (uvs >= 0x22 && uvs <= 0x25) {    /* (Semi-)Voiced Sound Mark */
            cp[0] =   ucv & 0x1FFFF;
            cp[1] = ((uvs - 0x22) >> 1) + 0x3099;
        }
        if (uvs >= 0x26 && uvs <= 0x2F) {    /* Emoji Modifier Fitzpatrick */
            cp[0] =   ucv & 0x1FFFF;
            cp[1] = ((uvs - 0x26) >> 1) + 0x1F3FB;
        }
    }
    else if (ucv >= 0x400000) {
        len = 2;
        uvs = ucv >> 16;
        if (uvs >= 0x40 && uvs <= 0x7F) {    /* SVS, VS1 .. VS16 */
            cp[0] =   ucv & 0x3FFFF;
            cp[1] = ((uvs - 0x40) >> 2) + 0xFE00;
        }
        if (uvs >= 0x80 && uvs <= 0x43F) {   /* IVS, VS17 .. VS256 */
            cp[0] =   ucv & 0x3FFFF;
            cp[1] = ((uvs - 0x80) >> 2) + 0xE0100;
        }
    }

    if (n<0)
        return len;
    if (n>=1 && n<=3) {
        return cp[n-1];
    }

    /* Unsupported */
    return 0;
}

long
UVS_combine_code(long ucv, long uvs)
{
    if (  uvs == 0x20E3 &&
        ( ucv == 0x7C0023 || ucv == 0x7C002A ||    /* U+00xx U+FE0F U+20E3  */
         (ucv >= 0x7C0030 && ucv <= 0x7C0039))) {  /* Emoji Keycap Sequence */
        return 0x40000 + ucv;
    }

    if (ucv > 0x3FFFF)
        return 0;

    if (((ucv >= 0x03000 && ucv <= 0x031FF) ||
         (ucv >= 0x1AFF0 && ucv <= 0x1B16F)) &&
          uvs >=  0x3099 && uvs <= 0x309A) {     /* Kana (Semi-)Voiced Sound Mark */
        return ((uvs - 0x3099) << 17) + 0x220000 + ucv;
    }
    if (((ucv >= 0x02600 && ucv <= 0x027BF) ||
         (ucv >= 0x1F300 && ucv <= 0x1F9FF)) &&
          uvs >= 0x1F3FB && uvs <= 0x1F3FF) {    /* Emoji Modifier Fitzpatrick */
        return ((uvs - 0x1F3FB) << 17) + 0x260000 + ucv;
    }
    if ( (ucv >=0x1F1E6 && ucv <= 0x1F1FF) &&
         (uvs >=0x1F1E6 && uvs <= 0x1F1FF) ) {   /* RGI Emoji Flag Sequence */
        return ((ucv & 0xFF) << 8) + (uvs & 0xFF) + 0x250000;
    }
    if (ucv <= 0x2FFFF && uvs >= 0xFE00 && uvs <= 0xFE0F) { /* SVS, VS1 .. VS16 */
        return ((uvs - 0xFE00) << 18) + 0x400000 + ucv;
    }
    if (uvs >= 0xE0100) {  /* IVS */
        if (                   ucv <= 0x033FF  ||
            (ucv >= 0x04DC0 && ucv <= 0x04DFF) ||
            (ucv >= 0x0A000 && ucv <= 0x0F8FF) ||
            (ucv >= 0x0FB00 && ucv <= 0x1FFFF) ) return 0;
        if (ucv <= 0x0FFFF && uvs <=0xE01EF) { /* BMP, VS17 .. VS256 */
            return ((uvs - 0xE0100) << 18) + 0x800000 + ucv;
        }
        if (ucv <= 0x2FFFF && uvs <=0xE010F) { /* SIP, VS17 .. VS32 */
            return ((uvs - 0xE0100) << 18) + 0x800000 + ucv;
        }
        if (                  uvs <=0xE010F) { /* TIP, VS17 .. VS32 */
            return ((uvs - 0xE0100) << 18) + 0x800000 + ucv;
        }
    }
    /* Unsupported Combination */
    return 0;
}

long
UVS_divide_code(long code, long* uvs)
{
    long u, v, p;

    u = code & 0x1FFFF;  /* upto U+1FFFF */
    v = code >> 16;
    /* for Combining Katakana-Hiragana (Semi-)Voiced Sound Mark */
    if (((u>=0x03000 && u<=0x031FF) ||
         (u>=0x1AFF0 && u<=0x1B16F)) &&
          v>=0x22 && v<=0x25) {
        p = (v - 0x22) >> 1;  /* Voiced or Semi-Voiced */
        if (uvs) *uvs = 0x3099 + p;
        return u;
    }
    /* for Emoji Modifier Fitzpatrick */
    if (((u>=0x02600 && u<=0x027BF) ||
         (u>=0x1F300 && u<=0x1F9FF)) &&
          v>=0x26 && v<=0x2F) {
        p = (v - 0x26) >> 1;  /* Emoji Modifier Fitzpatrick Type-1..6 */
        if (uvs) *uvs = 0x1F3FB + p;
        return u;
    }
    /* for RGI Emoji Flag Sequence */
    if (  u>=0x1E6E6 && v==0x25 ) {
        if ((u & 0xFF) < 0xE6) goto Undefined;  /* illegal value */
        u = ((u >> 8) & 0xFF) + 0x1F100;
        v = ( u       & 0xFF) + 0x1F100; /* Regional Indicator Symbol Letter */
        if (!uvs) goto Undefined;
        *uvs = v;
        return u;
    }

    if (code<0x400000 || code>=0x4400000) {
      /* Undefined */
        goto Undefined;
    }

    /* for Variation Selector */
    u = code & 0x3FFFF;  /* upto U+3FFFF */
    v = code >> 18;
    p = u >> 16;
    if (v < 0x20) {  /* SVS     VS1 .. VS16 */
        if (p==3) goto Undefined;
        if (uvs) *uvs = v - 0x10 + 0xFE00;
        return u;
    } else
    if (v == 0x20 && u <= 0x7F ) { /* for Emoji Keycap Sequence, need space for U+20E3 */
        if (uvs) { *uvs = 0xFE0F; *(uvs+1) = 0x20E3; }
        return u;
    } else           /* IVS */
    if (v < 0x40) {          /* VS17 .. VS32 */
        if (p==1) goto Undefined;
        if (uvs) *uvs = v - 0x20 + 0xE0100;
        return u;
    } else if (v <= 0x1FF) { /* VS33 .. VS256 */
        if (p>0) goto Undefined;
        if (uvs) *uvs = v - 0x20 + 0xE0100;
        return u;
    }

 Undefined:
    /* Undefined */
    if (uvs) *uvs = 0;
    return 0;
}
