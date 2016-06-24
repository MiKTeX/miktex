/* Copyright (C) 2000-2008 by George Williams */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.

 * The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fontforgevw.h"
#include <ustring.h>
#include <utype.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <gfile.h>
#include "plugins.h"
#include "encoding.h"

Encoding *default_encoding = NULL;

static int32 tex_base_encoding[] = {
    0x0000, 0x02d9, 0xfb01, 0xfb02, 0x2044, 0x02dd, 0x0141, 0x0142,
    0x02db, 0x02da, 0x000a, 0x02d8, 0x2212, 0x000d, 0x017d, 0x017e,
    0x02c7, 0x0131, 0xf6be, 0xfb00, 0xfb03, 0xfb04, 0x2260, 0x221e,
    0x2264, 0x2265, 0x2202, 0x2211, 0x220f, 0x03c0, 0x0060, 0x0027,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x2019,
    0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
    0x2018, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
    0x20ac, 0x222b, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
    0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0x2126, 0x221a, 0x2248,
    0x0090, 0x0091, 0x0092, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
    0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0x2206, 0x25ca, 0x0178,
    0x0000, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
    0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x002d, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
    0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
    0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
    0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
    0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
    0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
    0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
    0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
    0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff
};

static int32 unicode_from_MacSymbol[] = {
  0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
  0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
  0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
  0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
  0x0020, 0x0021, 0x2200, 0x0023, 0x2203, 0x0025, 0x0026, 0x220d,
  0x0028, 0x0029, 0x2217, 0x002b, 0x002c, 0x2212, 0x002e, 0x002f,
  0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
  0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
  0x2245, 0x0391, 0x0392, 0x03a7, 0x0394, 0x0395, 0x03a6, 0x0393,
  0x0397, 0x0399, 0x03d1, 0x039a, 0x039b, 0x039c, 0x039d, 0x039f,
  0x03a0, 0x0398, 0x03a1, 0x03a3, 0x03a4, 0x03a5, 0x03c2, 0x03a9,
  0x039e, 0x03a8, 0x0396, 0x005b, 0x2234, 0x005d, 0x22a5, 0x005f,
  0xf8e5, 0x03b1, 0x03b2, 0x03c7, 0x03b4, 0x03b5, 0x03c6, 0x03b3,
  0x03b7, 0x03b9, 0x03d5, 0x03ba, 0x03bb, 0x03bc, 0x03bd, 0x03bf,
  0x03c0, 0x03b8, 0x03c1, 0x03c3, 0x03c4, 0x03c5, 0x03d6, 0x03c9,
  0x03be, 0x03c8, 0x03b6, 0x007b, 0x007c, 0x007d, 0x223c, 0x007f,
  0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
  0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
  0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
  0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
  0x0000, 0x03d2, 0x2032, 0x2264, 0x2044, 0x221e, 0x0192, 0x2663,
  0x2666, 0x2665, 0x2660, 0x2194, 0x2190, 0x2191, 0x2192, 0x2193,
  0x00b0, 0x00b1, 0x2033, 0x2265, 0x00d7, 0x221d, 0x2202, 0x2022,
  0x00f7, 0x2260, 0x2261, 0x2248, 0x2026, 0xf8e6, 0xf8e7, 0x21b5,
  0x2135, 0x2111, 0x211c, 0x2118, 0x2297, 0x2295, 0x2205, 0x2229,
  0x222a, 0x2283, 0x2287, 0x2284, 0x2282, 0x2286, 0x2208, 0x2209,
  0x2220, 0x2207, 0x00ae, 0x00a9, 0x2122, 0x220f, 0x221a, 0x22c5,
  0x00ac, 0x2227, 0x2228, 0x21d4, 0x21d0, 0x21d1, 0x21d2, 0x21d3,
  0x22c4, 0x2329, 0xf8e8, 0xf8e9, 0xf8ea, 0x2211, 0xf8eb, 0xf8ec,
  0xf8ed, 0xf8ee, 0xf8ef, 0xf8f0, 0xf8f1, 0xf8f2, 0xf8f3, 0xf8f4,
  0xf8ff, 0x232a, 0x222b, 0x2320, 0xf8f5, 0x2321, 0xf8f6, 0xf8f7,
  0xf8f8, 0xf8f9, 0xf8fa, 0xf8fb, 0xf8fc, 0xf8fd, 0xf8fe, 0x02c7
};

/* I don't think iconv provides encodings for zapfdingbats nor jis201 */
/*  Perhaps I should list them here for compatability, but I think I'll just */
/*  leave them out. I doubt they get used.				     */
static Encoding texbase = { "TeX-Base-Encoding", 256, tex_base_encoding, NULL, NULL, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, { 0, 0 }, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, 0, 0  };
       Encoding custom = { "Custom", 0, NULL, NULL, &texbase,			  1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, { 0, 0 }, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, 0, 0 };
static Encoding original = { "Original", 0, NULL, NULL, &custom,		  1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, { 0, 0 }, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, 0, 0  };
static Encoding unicodebmp = { "UnicodeBmp", 65536, NULL, NULL, &original, 	  1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, { 0, 0 }, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, 0, 0  };
static Encoding unicodefull = { "UnicodeFull", 17*65536, NULL, NULL, &unicodebmp, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, { 0, 0 }, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, 0, 0  };
static Encoding adobestd = { "AdobeStandard", 256, unicode_from_adobestd, AdobeStandardEncoding, &unicodefull,
										  1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, { 0, 0 }, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, 0, 0  };
static Encoding symbol = { "Symbol", 256, unicode_from_MacSymbol, NULL, &adobestd,1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, { 0, 0 }, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, 0, 0  };

Encoding *enclist = &symbol;

const char *FindUnicharName(void) {
    /* Iconv and libiconv use different names for UCS2. Just great. Perhaps */
    /*  different versions of each use still different names? */
    /* Even worse, both accept UCS-2, but under iconv it means native byte */
    /*  ordering and under libiconv it means big-endian */
    iconv_t test;
    static char *goodname = NULL;
#ifdef UNICHAR_16
    static char *names[] = { "UCS-2-INTERNAL", "UCS-2", "UCS2", "ISO-10646/UCS2", "UNICODE", NULL };
    static char *namesle[] = { "UCS-2LE", "UNICODELITTLE", NULL };
    static char *namesbe[] = { "UCS-2BE", "UNICODEBIG", NULL };
#else
    static char *names[] = { "UCS-4-INTERNAL", "UCS-4", "UCS4", "ISO-10646-UCS-4", "UTF-32", NULL };
    static char *namesle[] = { "UCS-4LE", "UTF-32LE", NULL };
    static char *namesbe[] = { "UCS-4BE", "UTF-32BE", NULL };
#endif
    char **testnames;
    int i;
    union {
	short s;
	char c[2];
    } u;

    if ( goodname!=NULL )
return( goodname );

    u.c[0] = 0x1; u.c[1] = 0x2;
    if ( u.s==0x201 ) {		/* Little endian */
	testnames = namesle;
    } else {
	testnames = namesbe;
    }
    for ( i=0; testnames[i]!=NULL; ++i ) {
	test = iconv_open(testnames[i],"ISO-8859-1");
	if ( test!=(iconv_t) -1 && test!=NULL ) {
	    iconv_close(test);
	    goodname = testnames[i];
    break;
	}
    }

    if ( goodname==NULL ) {
	for ( i=0; names[i]!=NULL; ++i ) {
	    test = iconv_open(names[i],"ISO-8859-1");
	    if ( test!=(iconv_t) -1 && test!=NULL ) {
		iconv_close(test);
		goodname = names[i];
	break;
	    }
	}
    }

    if ( goodname==NULL ) {
#ifdef UNICHAR_16
	IError( "I can't figure out your version of iconv(). I need a name for the UCS-2 encoding and I can't find one. Reconfigure --without-iconv. Bye.");
#else
	IError( "I can't figure out your version of iconv(). I need a name for the UCS-4 encoding and I can't find one. Reconfigure --without-iconv. Bye.");
#endif
	exit( 1 );
    }

    test = iconv_open(goodname,"Mac");
    if ( test==(iconv_t) -1 || test==NULL ) {
	IError( "Your version of iconv does not support the \"Mac Roman\" encoding.\nIf this causes problems, reconfigure --without-iconv." );
    } else
	iconv_close(test);

    /* I really should check for ISO-2022-JP, KR, CN, and all the other encodings */
    /*  I might find in a ttf 'name' table. But those tables take too long to build */
return( goodname );
}

static int TryEscape( Encoding *enc,char *escape_sequence ) {
    char from[20], ucs[20];
    size_t fromlen, tolen;
    ICONV_CONST char *fpt;
    char *upt;
    int i, j, low;
    int esc_len = strlen(escape_sequence);

    strcpy(from,escape_sequence);

    enc->has_2byte = false;
    low = -1;
    for ( i=0; i<256; ++i ) if ( i!=escape_sequence[0] ) {
	for ( j=0; j<256; ++j ) {
	    from[esc_len] = i; from[esc_len+1] = j; from[esc_len+2] = 0;
	    fromlen = esc_len+2;
	    fpt = from;
	    upt = ucs;
	    tolen = sizeof(ucs);
	    if ( iconv( enc->tounicode , &fpt, &fromlen, &upt, &tolen )!= (size_t) (-1) &&
		    upt-ucs==sizeof(unichar_t) /* Exactly one character */ ) {
		if ( low==-1 ) {
		    enc->low_page = low = i;
		    enc->has_2byte = true;
		}
		enc->high_page = i;
	break;
	    }
	}
    }
    if ( enc->low_page==enc->high_page )
	enc->has_2byte = false;
    if ( enc->has_2byte ) {
	strcpy(enc->iso_2022_escape, escape_sequence);
	enc->iso_2022_escape_len = esc_len;
    }
return( enc->has_2byte );
}

Encoding *_FindOrMakeEncoding(const char *name,int make_it) {
    Encoding *enc;
    char buffer[20];
    const char *iconv_name;
    Encoding temp;
    uint8 good[256];
    int i, j, any, all;
    char from[8], ucs[20];
    size_t fromlen, tolen;
    ICONV_CONST char *fpt;
    char *upt;
    /* iconv is not case sensitive */

    if ( strncasecmp(name,"iso8859_",8)==0 || strncasecmp(name,"koi8_",5)==0 ) {
	/* Fixup for old naming conventions */
	strncpy(buffer,name,sizeof(buffer));
	*strchr(buffer,'_') = '-';
	name = buffer;
    } else if ( strcasecmp(name,"iso-8859")==0 ) {
	/* Fixup for old naming conventions */
	strncpy(buffer,name,3);
	strncpy(buffer+3,name+4,sizeof(buffer)-3);
	name = buffer;
    } else if ( strcasecmp(name,"isolatin1")==0 ) {
        name = "iso8859-1";
    } else if ( strcasecmp(name,"isocyrillic")==0 ) {
        name = "iso8859-5";
    } else if ( strcasecmp(name,"isoarabic")==0 ) {
        name = "iso8859-6";
    } else if ( strcasecmp(name,"isogreek")==0 ) {
        name = "iso8859-7";
    } else if ( strcasecmp(name,"isohebrew")==0 ) {
        name = "iso8859-8";
    } else if ( strcasecmp(name,"isothai")==0 ) {
        name = "tis-620";	/* TIS doesn't define non-breaking space in 0xA0 */ 
    } else if ( strcasecmp(name,"latin0")==0 || strcasecmp(name,"latin9")==0 ) {
        name = "iso8859-15";	/* "latin-9" is supported (libiconv bug?) */ 
    } else if ( strcasecmp(name,"koi8r")==0 ) {
        name = "koi8-r";
    } else if ( strncasecmp(name,"jis201",6)==0 || strncasecmp(name,"jisx0201",8)==0 ) {
        name = "jis_x0201";
    } else if ( strcasecmp(name,"AdobeStandardEncoding")==0 || strcasecmp(name,"Adobe")==0 )
	name = "AdobeStandard";
    for ( enc=enclist; enc!=NULL; enc=enc->next )
	if ( strmatch(name,enc->enc_name)==0 ||
		(enc->iconv_name!=NULL && strmatch(name,enc->iconv_name)==0))
return( enc );
    if ( strmatch(name,"unicode")==0 || strmatch(name,"iso10646")==0 || strmatch(name,"iso10646-1")==0 )
return( &unicodebmp );
    if ( strmatch(name,"unicode4")==0 || strmatch(name,"ucs4")==0 )
return( &unicodefull );

    iconv_name = name;
    /* Mac seems to work ok */
    if ( strcasecmp(name,"win")==0 || strcasecmp(name,"ansi")==0 )
	iconv_name = "MS-ANSI";		/* "WINDOWS-1252";*/
    else if ( strcasecmp(name,"gb2312pk")==0 || strcasecmp(name,"gb2312packed")==0 )
    iconv_name = "EUC-CN";
    else if ( strcasecmp(name,"wansung")==0 )
	iconv_name = "EUC-KR";
    else if ( strcasecmp(name,"EUC-CN")==0 ) {
	iconv_name = name;
	name = "gb2312pk";
    } else if ( strcasecmp(name,"EUC-KR")==0 ) {
	iconv_name = name;
	name = "wansung";
    }

/* Escape sequences:					*/
/*	ISO-2022-CN:     \e $ ) A ^N			*/
/*	ISO-2022-KR:     \e $ ) C ^N			*/
/*	ISO-2022-JP:     \e $ B				*/
/*	ISO-2022-JP-2:   \e $ ( D			*/
/*	ISO-2022-JP-3:   \e $ ( O			*/ /* Capital "O", not zero */
/*	ISO-2022-CN-EXT: \e $ ) E ^N			*/ /* Not sure about this, also uses CN escape */

    memset(&temp,0,sizeof(temp));
    temp.builtin = true;
    temp.tounicode = iconv_open(FindUnicharName(),iconv_name);
    if ( temp.tounicode==(iconv_t) -1 || temp.tounicode==NULL )
return( NULL );			/* Iconv doesn't recognize this name */
    temp.fromunicode = iconv_open(iconv_name,FindUnicharName());
    if ( temp.fromunicode==(iconv_t) -1 || temp.fromunicode==NULL ) {
	/* This should never happen, but if it does... */
	iconv_close(temp.tounicode);
return( NULL );
    }

    memset(good,0,sizeof(good));
    any = false; all = true;
    for ( i=1; i<256; ++i ) {
	from[0] = i; from[1] = 0;
	fromlen = 1;
	fpt = from;
	upt = ucs;
	tolen = sizeof(ucs);
	if ( iconv( temp.tounicode , &fpt, &fromlen, &upt, &tolen )!= (size_t) (-1)) {
	    good[i] = true;
	    any = true;
	} else
	    all = false;
    }
    if ( any )
	temp.has_1byte = true;
    if ( all )
	temp.only_1byte = true;

    if ( !all ) {
	if ( strstr(iconv_name,"2022")==NULL ) {
	    for ( i=temp.has_1byte; i<256; ++i ) if ( !good[i] ) {
		for ( j=0; j<256; ++j ) {
		    from[0] = i; from[1] = j; from[2] = 0;
		    fromlen = 2;
		    fpt = from;
		    upt = ucs;
		    tolen = sizeof(ucs);
		    if ( iconv( temp.tounicode , &fpt, &fromlen, &upt, &tolen )!= (size_t) (-1) &&
			    upt-ucs==sizeof(unichar_t) /* Exactly one character */ ) {
			if ( temp.low_page==-1 )
			    temp.low_page = i;
			temp.high_page = i;
			temp.has_2byte = true;
		break;
		    }
		}
	    }
	    if ( temp.low_page==temp.high_page ) {
		temp.has_2byte = false;
		temp.low_page = temp.high_page = -1;
	    }
	}
	if ( !temp.has_2byte && !good[033]/* escape */ ) {
	    if ( strstr(iconv_name,"2022")!=NULL &&
		    strstr(iconv_name,"JP3")!=NULL &&
                      TryEscape( &temp,"\33$(O" )) {
		;
	    }
	    else if ( strstr(iconv_name,"2022")!=NULL &&
		    strstr(iconv_name,"JP2")!=NULL &&
		      TryEscape( &temp,"\33$(D" )) {
		;
	    }
	    else if ( strstr(iconv_name,"2022")!=NULL &&
		    strstr(iconv_name,"JP")!=NULL &&
		      TryEscape( &temp,"\33$B" )) {
		;
	    }
	    else if ( strstr(iconv_name,"2022")!=NULL &&
		    strstr(iconv_name,"KR")!=NULL &&
		      TryEscape( &temp,"\33$)C\16" )) {
		;
	    }
	    else if ( strstr(iconv_name,"2022")!=NULL &&
		    strstr(iconv_name,"CN")!=NULL &&
		      TryEscape( &temp,"\33$)A\16" )) {
		;
	    }
	}
    }
    if ( !temp.has_1byte && !temp.has_2byte )
return( NULL );
    if ( !make_it )
return( NULL );

    enc = chunkalloc(sizeof(Encoding));
    *enc = temp;
    enc->enc_name = copy(name);
    if ( iconv_name!=name )
	enc->iconv_name = copy(iconv_name);
    enc->next = enclist;
    enc->builtin = true;
    enclist = enc;
    if ( enc->has_2byte )
	enc->char_cnt = (enc->high_page<<8) + 256;
    else {
	enc->char_cnt = 256;
	enc->only_1byte = true;
    }
    if ( strstrmatch(iconv_name,"JP")!=NULL ||
	    strstrmatch(iconv_name,"sjis")!=NULL ||
	    strstrmatch(iconv_name,"cp932")!=NULL )
	enc->is_japanese = true;
    else if ( strstrmatch(iconv_name,"KR")!=NULL )
	enc->is_korean = true;
    else if ( strstrmatch(iconv_name,"CN")!=NULL )
	enc->is_simplechinese = true;
    else if ( strstrmatch(iconv_name,"BIG")!=NULL && strstrmatch(iconv_name,"5")!=NULL )
	enc->is_tradchinese = true;

    if ( strstrmatch(name,"ISO8859")!=NULL &&
	    strtol(name+strlen(name)-2,NULL,10)>=16 )
	/* Not in our menu, don't hide */;
    else if ( iconv_name!=name || strmatch(name,"mac")==0 || strstrmatch(name,"ISO8859")!=NULL ||
	    strmatch(name,"koi8-r")==0 || strmatch(name,"sjis")==0 ||
	    strmatch(name,"big5")==0 || strmatch(name,"big5hkscs")==0 )
	enc->hidden = true;

return( enc );
}

Encoding *FindOrMakeEncoding(const char *name) {
return( _FindOrMakeEncoding(name,true));
}


/* ************************************************************************** */
/* ****************************** CID Encodings ***************************** */
/* ************************************************************************** */
struct cidmap *cidmaps = NULL;

int CID2NameUni(struct cidmap *map,int cid, char *buffer, int len) {
    int enc = -1;
    const char *temp;

#if defined( _NO_SNPRINTF ) || defined( __VMS )
    if ( map==NULL )
	sprintf(buffer,"cid-%d", cid);
    else if ( cid<map->namemax && map->name[cid]!=NULL )
	strncpy(buffer,map->name[cid],len);
    else if ( cid==0 || (cid<map->namemax && map->unicode[cid]!=0 )) {
	if ( map->unicode==NULL || map->namemax==0 )
	    enc = 0;
	else
	    enc = map->unicode[cid];
	temp = StdGlyphName(buffer,enc,ui_none,(NameList *) -1);
	if ( temp!=buffer )
	    strcpy(buffer,temp);
    } else
	sprintf(buffer,"%s.%d", map->ordering, cid);
#else
    if ( map==NULL )
	snprintf(buffer,len,"cid-%d", cid);
    else if ( cid<map->namemax && map->name[cid]!=NULL )
	strncpy(buffer,map->name[cid],len);
    else if ( cid==0 )
	strcpy(buffer,".notdef");
    else if ( cid<map->namemax && map->unicode[cid]!=0 ) {
	if ( map->unicode==NULL || map->namemax==0 )
	    enc = 0;
	else
	    enc = map->unicode[cid];
	temp = StdGlyphName(buffer,enc,ui_none,(NameList *) -1);
	if ( temp!=buffer )
	    strcpy(buffer,temp);
    } else
	snprintf(buffer,len,"%s.%d", map->ordering, cid);
#endif
return( enc );
}

int NameUni2CID(struct cidmap *map,int uni, const char *name) {
    int i;

    if ( map==NULL )
return( -1 );
    if ( uni!=-1 ) {
	for ( i=0; i<map->namemax; ++i )
	  if ( map->unicode[i]==(unsigned)uni )
return( i );
    } else {
	for ( i=0; i<map->namemax; ++i )
	    if ( map->name[i]!=NULL && strcmp(map->name[i],name)==0 )
return( i );
    }
return( -1 );
}

int MaxCID(struct cidmap *map) {
return( map->cidmax );
}

static struct cidmap *MakeDummyMap(char *registry,char *ordering,int supplement) {
    struct cidmap *ret = galloc(sizeof(struct cidmap));

    ret->registry = copy(registry);
    ret->ordering = copy(ordering);
    ret->supplement = ret->maxsupple = supplement;
    ret->cidmax = ret->namemax = 0;
    ret->unicode = NULL; ret->name = NULL;
    ret->next = cidmaps;
    cidmaps = ret;
return( ret );
}

struct cidmap *FindCidMap(char *registry,char *ordering,int supplement,SplineFont *sf) {
return( MakeDummyMap(registry,ordering,supplement));
}

/* ************************** Reencoding  routines ************************** */


EncMap *EncMapFromEncoding(SplineFont *sf,Encoding *enc) {
    int i,j, extras, found, base, unmax;
    int *encoded, *unencoded;
    EncMap *map;
    struct altuni *altuni;
    SplineChar *sc;

    if ( enc==NULL )
return( NULL );

    base = enc->char_cnt;
    if ( enc->is_original )
	base = 0;
    else if ( enc->char_cnt<=256 )
	base = 256;
    else if ( enc->char_cnt<=0x10000 )
	base = 0x10000;
    if (base==0) 
return( NULL );
    encoded = galloc(base*sizeof(int));
    memset(encoded,-1,base*sizeof(int));
    unencoded = galloc(sf->glyphcnt*sizeof(int));
    unmax = sf->glyphcnt;

    for ( i=extras=0; i<sf->glyphcnt; ++i ) if ( (sc=sf->glyphs[i])!=NULL ) {
	found = false;
	if ( enc->psnames!=NULL ) {
	    for ( j=enc->char_cnt-1; j>=0; --j ) {
		if ( enc->psnames[j]!=NULL &&
			strcmp(enc->psnames[j],sc->name)==0 ) {
		    found = true;
		    encoded[j] = i;
		}
	    }
	}
	if ( !found ) {
	    if ( sc->unicodeenc!=-1 &&
		     sc->unicodeenc<unicode4_size &&
		     (j = EncFromUni(sc->unicodeenc,enc))!= -1 )
		encoded[j] = i;
	    else {
		/* I don't think extras can surpass unmax now, but it doesn't */
		/*  hurt to leave the code (it's from when we encoded duplicates see below) */
		if ( extras>=unmax ) unencoded = grealloc(unencoded,(unmax+=300)*sizeof(int));
		unencoded[extras++] = i;
	    }
	    for ( altuni=sc->altuni; altuni!=NULL; altuni=altuni->next ) {
		if ( altuni->unienc!=-1 &&
			 altuni->unienc<unicode4_size &&
			 altuni->vs==-1 &&
			 altuni->fid==0 &&
			 (j = EncFromUni(altuni->unienc,enc))!= -1 )
		    encoded[j] = i;
		/* I used to have code here to add these unencoded duplicates */
		/*  but I don't really see any reason to do so. The main unicode */
		/*  will occur, and any encoded duplicates so the glyph won't */
		/*  vanish */
	    }
	}
    }

    /* Some glyphs have both a pua encoding and an encoding in a non-bmp */
    /*  plane. Big5HK does and the AMS glyphs do */
    if ( enc->is_unicodefull && (sf->uni_interp == ui_trad_chinese ||
				 sf->uni_interp == ui_ams )) {
	extern const int cns14pua[], amspua[];
	const int *pua = sf->uni_interp == ui_ams? amspua : cns14pua;
	for ( i=0xe000; i<0xf8ff; ++i ) {
	    if ( pua[i-0xe000]!=0 )
		encoded[pua[i-0xe000]] = encoded[i];
	}
    }

    if ( enc->psnames != NULL ) {
	/* Names are more important than unicode code points for some encodings */
	/*  AdobeStandard for instance which won't work if you have a glyph  */
	/*  named "f_i" (must be "fi") even though the code point is correct */
	/* The code above would match f_i where AS requires fi, so force the */
	/*  names to be correct. */
	for ( j=0; j<enc->char_cnt; ++j ) {
	    if ( encoded[j]!=-1 && enc->psnames[j]!=NULL &&
		    strcmp(sf->glyphs[encoded[j]]->name,enc->psnames[j])!=0 ) {
		free(sf->glyphs[encoded[j]]->name);
		sf->glyphs[encoded[j]]->name = copy(enc->psnames[j]);
	    }
	}
    }

    map = chunkalloc(sizeof(EncMap));
    map->enccount = map->encmax = base + extras;
    map->map = galloc(map->enccount*sizeof(int));
    memcpy(map->map,encoded,base*sizeof(int));
    memcpy(map->map+base,unencoded,extras*sizeof(int));
    map->backmax = sf->glyphcnt;
    map->backmap = galloc(sf->glyphcnt*sizeof(int));
    memset(map->backmap,-1,sf->glyphcnt*sizeof(int));	/* Just in case there are some unencoded glyphs (duplicates perhaps) */
    for ( i = map->enccount-1; i>=0; --i ) if ( map->map[i]!=-1 )
	map->backmap[map->map[i]] = i;
    map->enc = enc;

    free(encoded);
    free(unencoded);

return( map );
}

EncMap *CompactEncMap(EncMap *map, SplineFont *sf) {
    int i, inuse, gid;
    int32 *newmap;

    for ( i=inuse=0; i<map->enccount ; ++i )
	if ( (gid = map->map[i])!=-1 && SCWorthOutputting(sf->glyphs[gid]))
	    ++inuse;
    newmap = galloc(inuse*sizeof(int32));
    for ( i=inuse=0; i<map->enccount ; ++i )
	if ( (gid = map->map[i])!=-1 && SCWorthOutputting(sf->glyphs[gid]))
	    newmap[inuse++] = gid;
    free(map->map);
    map->map = newmap;
    map->enccount = inuse;
    map->encmax = inuse;
    map->enc = &custom;
    memset(map->backmap,-1,sf->glyphcnt*sizeof(int));
    for ( i=inuse-1; i>=0; --i )
	if ( (gid=map->map[i])!=-1 )
	    map->backmap[gid] = i;
return( map );
}


static int MapAddEncodingSlot(EncMap *map,int gid) {
    int enc;

    if ( map->enccount>=map->encmax )
	map->map = grealloc(map->map,(map->encmax+=10)*sizeof(int));
    enc = map->enccount++;
    map->map[enc] = gid;
    map->backmap[gid] = enc;
return( enc );
}

void FVAddEncodingSlot(FontViewBase *fv,int gid) {
    EncMap *map = fv->map;
    int enc;

    enc = MapAddEncodingSlot(map,gid);

    fv->selected = grealloc(fv->selected,map->enccount);
    fv->selected[enc] = 0;
    FVAdjustScrollBarRows(fv,enc);
}

static int MapAddEnc(SplineFont *sf,SplineChar *sc,EncMap *basemap, EncMap *map,int baseenc, int gid, FontViewBase *fv) {
    int any = false, enc;

    if ( gid>=map->backmax ) {
	map->backmap = grealloc(map->backmap,(map->backmax+=10)*sizeof(int));
	memset(map->backmap+map->backmax-10,-1,10*sizeof(int));
    }
    if ( map->enc->psnames!=NULL ) {
	/* Check for multiple encodings */
	for ( enc = map->enc->char_cnt-1; enc>=0; --enc ) {
	    if ( map->enc->psnames[enc]!=NULL && strcmp(sc->name,map->enc->psnames[enc])==0 ) {
		if ( !any ) {
		    map->backmap[gid] = enc;
		    any = true;
		}
		map->map[enc] = gid;
	    }
	}
    } else {
	enc = SFFindSlot(sf,map,sc->unicodeenc,sc->name);
	if ( enc!=-1 ) {
	    map->map[enc] = gid;
	    map->backmap[gid] = enc;
	    any = true;
	}
    }
    if ( basemap!=NULL && map->enc==basemap->enc && baseenc!=-1 ) {
	if ( baseenc>=map->enccount ) {
	    if ( map==fv->map )
		FVAddEncodingSlot(fv,gid);
	    else
		MapAddEncodingSlot(map,gid);
	} else {
	    map->map[baseenc] = gid;
	    if ( map->backmap[gid]==-1 )
		map->backmap[gid] = baseenc;
	}
	any = true;
    }
return( any );
}

void SFAddGlyphAndEncode(SplineFont *sf,SplineChar *sc,EncMap *basemap, int baseenc) {
    int gid, mapfound = false;
    FontViewBase *fv;
    BDFFont *bdf;

    if ( sf->cidmaster==NULL ) {
	if ( sf->glyphcnt+1>=sf->glyphmax )
	    sf->glyphs = grealloc(sf->glyphs,(sf->glyphmax+=10)*sizeof(SplineChar *));
	gid = sf->glyphcnt++;
	for ( bdf = sf->bitmaps; bdf!=NULL; bdf=bdf->next ) {
	    if ( sf->glyphcnt+1>=bdf->glyphmax )
		bdf->glyphs = grealloc(bdf->glyphs,(bdf->glyphmax=sf->glyphmax)*sizeof(BDFChar *));
	    if ( sf->glyphcnt>bdf->glyphcnt ) {
		memset(bdf->glyphs+bdf->glyphcnt,0,(sf->glyphcnt-bdf->glyphcnt)*sizeof(BDFChar *));
		bdf->glyphcnt = sf->glyphcnt;
	    }
	}
	for ( fv=sf->fv; fv!=NULL; fv = fv->nextsame ) {
	    EncMap *map = fv->map;
	    if ( gid>=map->backmax )
		map->backmap = grealloc(map->backmap,(map->backmax=gid+10)*sizeof(int));
	    map->backmap[gid] = -1;
	}
    } else {
	gid = baseenc;
	if ( baseenc+1>=sf->glyphmax )
	    sf->glyphs = grealloc(sf->glyphs,(sf->glyphmax = baseenc+10)*sizeof(SplineChar *));
	if ( baseenc>=sf->glyphcnt ) {
	    memset(sf->glyphs+sf->glyphcnt,0,(baseenc+1-sf->glyphcnt)*sizeof(SplineChar *));
	    sf->glyphcnt = baseenc+1;
	    for ( bdf = sf->cidmaster->bitmaps; bdf!=NULL; bdf=bdf->next ) {
		if ( baseenc+1>=bdf->glyphmax )
		    bdf->glyphs = grealloc(bdf->glyphs,(bdf->glyphmax=baseenc+10)*sizeof(BDFChar *));
		if ( baseenc+1>bdf->glyphcnt ) {
		    memset(bdf->glyphs+bdf->glyphcnt,0,(baseenc+1-bdf->glyphcnt)*sizeof(BDFChar *));
		    bdf->glyphcnt = baseenc+1;
		}
	    }
	    for ( fv=sf->fv; fv!=NULL; fv = fv->nextsame ) if ( fv->sf==sf ) {
		EncMap *map = fv->map;
		if ( gid>=map->backmax )
		    map->backmap = grealloc(map->backmap,(map->backmax=gid+10)*sizeof(int));
		map->backmap[gid] = -1;
	    }
	}
    }
    sf->glyphs[gid] = NULL;
    for ( fv=sf->fv; fv!=NULL; fv = fv->nextsame ) {
	EncMap *map = fv->map;

	FVBiggerGlyphCache(fv,gid);

	if ( !MapAddEnc(sf,sc,basemap,map,baseenc,gid,fv) )
	    FVAddEncodingSlot(fv,gid);
	if ( map==basemap ) mapfound = true;
	if ( fv->normal!=NULL ) {
	    if ( !MapAddEnc(sf,sc,basemap,fv->normal,baseenc,gid,fv))
		MapAddEncodingSlot(fv->normal,gid);
	}
    }
    if ( !mapfound && basemap!=NULL )
	MapAddEnc(sf,sc,basemap,basemap,baseenc,gid,fv);
    sf->glyphs[gid] = sc;
    sc->orig_pos = gid;
    sc->parent = sf;
    SFHashGlyph(sf,sc);
}


int32 UniFromEnc(int enc, Encoding *encname) {
    char from[20];
    unichar_t to[20];
    ICONV_CONST char *fpt;
    char *tpt;
    size_t fromlen, tolen;

    if ( encname->is_custom || encname->is_original )
return( -1 );
    if ( enc>=encname->char_cnt )
return( -1 );
    if ( encname->is_unicodebmp || encname->is_unicodefull )
return( enc );
    if ( encname->unicode!=NULL )
return( encname->unicode[enc] );
    else if ( encname->tounicode ) {
	/* To my surprise, on RH9, doing a reset on conversion of CP1258->UCS2 */
	/* causes subsequent calls to return garbage */
	if ( encname->iso_2022_escape_len ) {
	    tolen = sizeof(to); fromlen = 0;
	    iconv(encname->tounicode,NULL,&fromlen,NULL,&tolen);	/* Reset state */
	}
	fpt = from; tpt = (char *) to; tolen = sizeof(to);
	if ( encname->has_1byte && enc<256 ) {
	    *(char *) fpt = enc;
	    fromlen = 1;
	} else if ( encname->has_2byte ) {
	    if ( encname->iso_2022_escape_len )
		strncpy(from,encname->iso_2022_escape,encname->iso_2022_escape_len );
	    fromlen = encname->iso_2022_escape_len;
	    from[fromlen++] = enc>>8;
	    from[fromlen++] = enc&0xff;
	}
	if ( iconv(encname->tounicode,&fpt,&fromlen,&tpt,&tolen)==(size_t) -1 )
return( -1 );
	if ( tpt-(char *) to == 0 ) {
	    /* This strange call appears to be what we need to make CP1258->UCS2 */
	    /*  work.  It's supposed to reset the state and give us the shift */
	    /*  out. As there is no state, and no shift out I have no idea why*/
	    /*  this works, but it does. */
	    if ( iconv(encname->tounicode,NULL,&fromlen,&tpt,&tolen)==(size_t) -1 )
return( -1 );
	}
	if ( tpt-(char *) to == sizeof(unichar_t) )
return( to[0] );
#ifdef UNICHAR_16
	else if ( tpt-(char *) to == 4 && to[0]>=0xd800 && to[0]<0xdc00 && to[1]>=0xdc00 )
return( ((to[0]-0xd800)<<10) + (to[1]-0xdc00) + 0x10000 );
#endif
    } else if ( encname->tounicode_func!=NULL ) {
return( (encname->tounicode_func)(enc) );
    }
return( -1 );
}

int32 EncFromUni(int32 uni, Encoding *enc) {
    unichar_t from[20];
    unsigned char to[20];
    ICONV_CONST char *fpt;
    char *tpt;
    size_t fromlen, tolen;
    int i;

    if ( enc->is_custom || enc->is_original || enc->is_compact || uni==-1 )
return( -1 );
    if ( enc->is_unicodebmp || enc->is_unicodefull )
return( uni<enc->char_cnt ? uni : -1 );

    if ( enc->unicode!=NULL ) {
	for ( i=0; i<enc->char_cnt; ++i ) {
	    if ( enc->unicode[i]==uni )
return( i );
	}
return( -1 );
    } else if ( enc->fromunicode!=NULL ) {
	/* I don't see how there can be any state to reset in this direction */
	/*  So I don't reset it */
#ifdef UNICHAR_16
	if ( uni<0x10000 ) {
	    from[0] = uni;
	    fromlen = sizeof(unichar_t);
	} else {
	    uni -= 0x10000;
	    from[0] = 0xd800 + (uni>>10);
	    from[1] = 0xdc00 + (uni&0x3ff);
	    fromlen = 2*sizeof(unichar_t);
	}
#else
	from[0] = uni;
	fromlen = sizeof(unichar_t);
#endif
	fpt = (char *) from; tpt = (char *) to; tolen = sizeof(to);
	iconv(enc->fromunicode,NULL,NULL,NULL,NULL);	/* reset shift in/out, etc. */
	if ( iconv(enc->fromunicode,&fpt,&fromlen,&tpt,&tolen)==(size_t) -1 )
return( -1 );
	if ( tpt-(char *) to == 1 )
return( to[0] );
	if ( enc->iso_2022_escape_len!=0 ) {
	    if ( tpt-(char *) to == enc->iso_2022_escape_len+2 &&
		    strncmp((char *) to,enc->iso_2022_escape,enc->iso_2022_escape_len)==0 )
return( (to[enc->iso_2022_escape_len]<<8) | to[enc->iso_2022_escape_len+1] );
	} else {
	    if ( tpt-(char *) to == sizeof(unichar_t) )
return( (to[0]<<8) | to[1] );
	}
    } else if ( enc->fromunicode_func!=NULL ) {
return( (enc->fromunicode_func)(uni) );
    }
return( -1 );
}

int32 EncFromName(const char *name,enum uni_interp interp,Encoding *encname) {
    int i;
    if ( encname->psnames!=NULL ) {
	for ( i=0; i<encname->char_cnt; ++i )
	    if ( encname->psnames[i]!=NULL && strcmp(name,encname->psnames[i])==0 )
return( i );
    }
    i = UniFromName(name,interp,encname);
    if ( i==-1 && strlen(name)==4 ) {
	/* MS says use this kind of name, Adobe says use the one above */
	char *end;
	i = strtol(name,&end,16);
	if ( i<0 || i>0xffff || *end!='\0' )
return( -1 );
    }
return( EncFromUni(i,encname));
}

void SFExpandGlyphCount(SplineFont *sf, int newcnt) {
    int old = sf->glyphcnt;
    FontViewBase *fv;

    if ( old>=newcnt )
return;
    if ( sf->glyphmax<newcnt ) {
	sf->glyphs = grealloc(sf->glyphs,newcnt*sizeof(SplineChar *));
	sf->glyphmax = newcnt;
    }
    memset(sf->glyphs+sf->glyphcnt,0,(newcnt-sf->glyphcnt)*sizeof(SplineChar *));
    sf->glyphcnt = newcnt;

    for ( fv=sf->fv; fv!=NULL; fv=fv->nextsame ) {
	if ( fv->sf==sf ) {	/* Beware of cid keyed fonts which might look at a different subfont */
	    if ( fv->normal!=NULL )
    continue;			/* If compacted then we haven't added any glyphs so haven't changed anything */
	    /* Don't display any of these guys, so not mapped. */
	    /*  No change to selection, or to map->map, but change to backmap */
	    if ( newcnt>fv->map->backmax )
		fv->map->backmap = grealloc(fv->map->backmap,(fv->map->backmax = newcnt+5)*sizeof(int32));
	    memset(fv->map->backmap+old,-1,(newcnt-old)*sizeof(int32));
	}
    }
}
