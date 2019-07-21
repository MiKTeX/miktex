/* Copyright (C) 2000-2012 by George Williams */
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
#include "ustring.h"
#include "utype.h"
#include <stddef.h>

static int u_strlen(register const unichar_t *str) {
    register int len = 0;

    while ( *str++!='\0' )
	++len;
return( len );
}

unichar_t *uc_copy(const char *pt) {
    unichar_t *res, *rpt;
    int n;

    if(!pt)
return((unichar_t *)0);

    n = strlen(pt);
#ifdef MEMORY_MASK
    if ( (n+1)*sizeof(unichar_t)>=MEMORY_MASK )
	n = MEMORY_MASK/sizeof(unichar_t)-1;
#endif
    res = (unichar_t *) xmalloc((n+1)*sizeof(unichar_t));
    for ( rpt=res; --n>=0 ; *rpt++ = *(unsigned char *) pt++ );
    *rpt = '\0';
return(res);
}

static char *u2utf8_strcpy(char *utf8buf,const unichar_t *ubuf) {
    char *pt = utf8buf;

    while ( *ubuf ) {
	if ( *ubuf<0x80 )
	    *pt++ = *ubuf;
	else if ( *ubuf<0x800 ) {
	    *pt++ = 0xc0 | (*ubuf>>6);
	    *pt++ = 0x80 | (*ubuf&0x3f);
	} else if ( *ubuf < 0x10000 ) {
	    *pt++ = 0xe0 | (*ubuf>>12);
	    *pt++ = 0x80 | ((*ubuf>>6)&0x3f);
	    *pt++ = 0x80 | (*ubuf&0x3f);
	} else {
	    uint32 val = *ubuf-0x10000;
	    int u = ((val&0xf0000)>>16)+1, z=(val&0x0f000)>>12, y=(val&0x00fc0)>>6, x=val&0x0003f;
	    *pt++ = 0xf0 | (u>>2);
	    *pt++ = 0x80 | ((u&3)<<4) | z;
	    *pt++ = 0x80 | y;
	    *pt++ = 0x80 | x;
	}
	++ubuf;
    }
    *pt = '\0';
return( utf8buf );
}

static char *latin1_2_utf8_strcpy(char *utf8buf,const char *lbuf) {
    char *pt = utf8buf;
    const unsigned char *lpt = (const unsigned char *) lbuf;

    while ( *lpt ) {
	if ( *lpt<0x80 )
	    *pt++ = *lpt;
	else {
	    *pt++ = 0xc0 | (*lpt>>6);
	    *pt++ = 0x80 | (*lpt&0x3f);
	}
	++lpt;
    }
    *pt = '\0';
return( utf8buf );
}

char *latin1_2_utf8_copy(const char *lbuf) {
    int len;
    char *utf8buf;

    if ( lbuf==NULL )
return( NULL );

    len = strlen(lbuf);
    utf8buf = (char *) xmalloc(2*len+1);
return( latin1_2_utf8_strcpy(utf8buf,lbuf));
}

char *u2utf8_copy(const unichar_t *ubuf) {
    int len;
    char *utf8buf;

    if ( ubuf==NULL )
return( NULL );

    len = u_strlen(ubuf);
    utf8buf = (char *) xmalloc((len+1)*4);
return( u2utf8_strcpy(utf8buf,ubuf));
}

int32 utf8_ildb(const char **_text) {
    int32 val= -1;
    int ch;
    const uint8 *text = (const uint8 *) *_text;
    /* Increment and load character */

    if ( (ch = *text++)<0x80 ) {
	val = ch;
    } else if ( ch<=0xbf ) {
	/* error */
    } else if ( ch<=0xdf ) {
	if ( *text>=0x80 && *text<0xc0 )
	    val = ((ch&0x1f)<<6) | (*text++&0x3f);
    } else if ( ch<=0xef ) {
	if ( *text>=0x80 && *text<0xc0 && text[1]>=0x80 && text[1]<0xc0 ) {
	    val = ((ch&0xf)<<12) | ((text[0]&0x3f)<<6) | (text[1]&0x3f);
	    text += 2;
	}
    } else {
	int w = ( ((ch&0x7)<<2) | ((text[0]&0x30)>>4) )-1, w2;
	w = (w<<6) | ((text[0]&0xf)<<2) | ((text[1]&0x30)>>4);
	w2 = ((text[1]&0xf)<<6) | (text[2]&0x3f);
	val = w*0x400 + w2 + 0x10000;
	if ( *text<0x80 || text[1]<0x80 || text[2]<0x80 ||
		*text>=0xc0 || text[1]>=0xc0 || text[2]>=0xc0 )
	    val = -1;
	else
	    text += 3;
    }
    *_text = (const char *) text;
return( val );
}

char *utf8_idpb(char *utf8_text,uint32 ch) {
    /* Increment and deposit character */
    if ( ch>=17*65536 )
return( utf8_text );

    if ( ch<=127 )
	*utf8_text++ = ch;
    else if ( ch<=0x7ff ) {
	*utf8_text++ = 0xc0 | (ch>>6);
	*utf8_text++ = 0x80 | (ch&0x3f);
    } else if ( ch<=0xffff ) {
	*utf8_text++ = 0xe0 | (ch>>12);
	*utf8_text++ = 0x80 | ((ch>>6)&0x3f);
	*utf8_text++ = 0x80 | (ch&0x3f);
    } else {
	uint32 val = ch-0x10000;
	int u = ((val&0xf0000)>>16)+1, z=(val&0x0f000)>>12, y=(val&0x00fc0)>>6, x=val&0x0003f;
	*utf8_text++ = 0xf0 | (u>>2);
	*utf8_text++ = 0x80 | ((u&3)<<4) | z;
	*utf8_text++ = 0x80 | y;
	*utf8_text++ = 0x80 | x;
    }
return( utf8_text );
}

int utf8_valid(const char *str) {
    /* Is this a valid utf8 string? */
    int ch;

    while ( (ch=utf8_ildb(&str))!='\0' )
	if ( ch==-1 )
return( false );

return( true );
}
