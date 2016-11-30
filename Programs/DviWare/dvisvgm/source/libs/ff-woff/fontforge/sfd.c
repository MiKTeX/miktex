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
#include "fontforge.h"
#include <utype.h>
#include <sys/stat.h>

#ifndef NAME_MAX
# ifndef  _POSIX_NAME_MAX
#  define _POSIX_NAME_MAX 512
# endif
# define NAME_MAX _POSIX_NAME_MAX
#endif

static const char *joins[] = { "miter", "round", "bevel", "inher", NULL };
static const char *caps[] = { "butt", "round", "square", "inher", NULL };
static const char *spreads[] = { "pad", "reflect", "repeat", NULL };

#define SFD_PTFLAG_TYPE_MASK          0x3
#define SFD_PTFLAG_IS_SELECTED        0x4
#define SFD_PTFLAG_NEXTCP_IS_DEFAULT  0x8
#define SFD_PTFLAG_PREVCP_IS_DEFAULT  0x10
#define SFD_PTFLAG_ROUND_IN_X         0x20
#define SFD_PTFLAG_ROUND_IN_Y         0x40
#define SFD_PTFLAG_INTERPOLATE        0x80
#define SFD_PTFLAG_INTERPOLATE_NEVER  0x100
#define SFD_PTFLAG_PREV_EXTREMA_MARKED_ACCEPTABLE  0x200
#define SFD_PTFLAG_FORCE_OPEN_PATH    0x400




/* I will retain this list in case there are still some really old sfd files */
/*  including numeric encodings.  This table maps them to string encodings */
static const char *charset_names[] = {
    "custom",
    "iso8859-1", "iso8859-2", "iso8859-3", "iso8859-4", "iso8859-5",
    "iso8859-6", "iso8859-7", "iso8859-8", "iso8859-9", "iso8859-10",
    "iso8859-11", "iso8859-13", "iso8859-14", "iso8859-15",
    "koi8-r",
    "jis201",
    "win", "mac", "symbol", "zapfding", "adobestandard",
    "jis208", "jis212", "ksc5601", "gb2312", "big5", "big5hkscs", "johab",
    "unicode", "unicode4", "sjis", "wansung", "gb2312pk", NULL};

static const char *unicode_interp_names[] = { "none", "adobe", "greek",
    "japanese", "tradchinese", "simpchinese", "korean", "ams", NULL };

/* sfdir files and extensions */
#define FONT_PROPS	"font.props"
#define STRIKE_PROPS	"strike.props"
#define EXT_CHAR	'.'
#define GLYPH_EXT	".glyph"
#define BITMAP_EXT	".bitmap"
#define STRIKE_EXT	".strike"
#define SUBFONT_EXT	".subfont"
#define INSTANCE_EXT	".instance"

signed char inbase64[256] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
        -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
        -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

static const char *end_tt_instrs = "EndTTInstrs";
static RefChar *SFDGetRef(FILE *sfd, int was_enc);
static AnchorPoint *SFDReadAnchorPoints(FILE *sfd,SplineChar *sc,AnchorPoint** alist, AnchorPoint *lastap);
static StemInfo *SFDReadHints(FILE *sfd);
static DStemInfo *SFDReadDHints( SplineFont *sf,FILE *sfd,int old );
extern uint8 *_IVParse(SplineFont *sf, char *text, int *len, void (*IVError)(void *,char *, int), void *iv);

static int PeekMatch(FILE *stream, const char * target) {
  // This returns 1 if target matches the next characters in the stream.
  int pos1 = 0;
  int lastread = getc(stream);
  while (target[pos1] != '\0' && lastread != EOF && lastread == target[pos1]) {
    pos1 ++; lastread = getc(stream);
  }

  int rewind_amount = pos1 + ((lastread == EOF) ? 0 : 1);
  fseek(stream, -rewind_amount, SEEK_CUR);
  return (target[pos1] == '\0');
}

/* Long lines can be broken by inserting \\\n (backslash newline) */
/*  into the line. I don't think this is ever ambiguous as I don't */
/*  think a line can end with backslash */
/* UPDATE: it can... that's handled in getquotedeol() below. */
static int nlgetc(FILE *sfd) {
    int ch, ch2;

    ch=getc(sfd);
    if ( ch!='\\' )
return( ch );
    ch2 = getc(sfd);
    if ( ch2=='\n' )
return( nlgetc(sfd));
    ungetc(ch2,sfd);
return( ch );
}

static char *SFDReadUTF7Str(FILE *sfd) {
    char *buffer = NULL, *pt, *end = NULL;
    int ch1, ch2, ch3, ch4, done, c;
    int prev_cnt=0, prev=0, in=0;

    ch1 = nlgetc(sfd);
    while ( isspace(ch1) && ch1!='\n' && ch1!='\r') ch1 = nlgetc(sfd);
    if ( ch1=='\n' || ch1=='\r' )
	ungetc(ch1,sfd);
    if ( ch1!='"' )
return( NULL );
    pt = 0;
    while ( (ch1=nlgetc(sfd))!=EOF && ch1!='"' ) {
	done = 0;
	if ( !done && !in ) {
	    if ( ch1=='+' ) {
		ch1 = nlgetc(sfd);
		if ( ch1=='-' ) {
		    ch1 = '+';
		    done = true;
		} else {
		    in = true;
		    prev_cnt = 0;
		}
	    } else
		done = true;
	}
	if ( !done ) {
	    if ( ch1=='-' ) {
		in = false;
	    } else if ( inbase64[ch1]==-1 ) {
		in = false;
		done = true;
	    } else {
		ch1 = inbase64[ch1];
		ch2 = inbase64[c = nlgetc(sfd)];
		if ( ch2==-1 ) {
		    ungetc(c, sfd);
		    ch2 = ch3 = ch4 = 0;
		} else {
		    ch3 = inbase64[c = nlgetc(sfd)];
		    if ( ch3==-1 ) {
			ungetc(c, sfd);
			ch3 = ch4 = 0;
		    } else {
			ch4 = inbase64[c = nlgetc(sfd)];
			if ( ch4==-1 ) {
			    ungetc(c, sfd);
			    ch4 = 0;
			}
		    }
		}
		ch1 = (ch1<<18) | (ch2<<12) | (ch3<<6) | ch4;
		if ( prev_cnt==0 ) {
		    prev = ch1&0xff;
		    ch1 >>= 8;
		    prev_cnt = 1;
		} else /* if ( prev_cnt == 1 ) */ {
		    ch1 |= (prev<<24);
		    prev = (ch1&0xffff);
		    ch1 = (ch1>>16)&0xffff;
		    prev_cnt = 2;
		}
		done = true;
	    }
	}
	if ( pt+10>=end ) {
	    if ( buffer==NULL ) {
		pt = buffer = malloc(400);
		end = buffer+400;
	    } else if (pt) {
		char *temp = realloc(buffer,end-buffer+400);
		pt = temp+(pt-buffer);
		end = temp+(end-buffer+400);
		buffer = temp;
	    }
	}
	if ( pt && done )
	    pt = utf8_idpb(pt,ch1,0);
	if ( prev_cnt==2 ) {
	    prev_cnt = 0;
	    if ( pt && prev!=0 )
		pt = utf8_idpb(pt,prev,0);
	}
	if ( pt==0 ) {
	    free(buffer);
	    return( NULL );
	}
    }
    if ( buffer==NULL )
return( NULL );
    *pt = '\0';
    pt = copy(buffer);
    free(buffer );
return( pt );
}

struct enc85 {
    FILE *sfd;
    unsigned char sofar[4];
    int pos;
    int ccnt;
};

static void *SFDUnPickle(FILE *sfd, int python_data_has_lists) {
    int ch, quoted;
    static int max = 0;
    static char *buf = NULL;
    char *pt, *end;
    int cnt;

    pt = buf; end = buf+max;
    while ( (ch=nlgetc(sfd))!='"' && ch!='\n' && ch!=EOF );
    if ( ch!='"' )
return( NULL );

    quoted = false;
    while ( ((ch=nlgetc(sfd))!='"' || quoted) && ch!=EOF ) {
	if ( !quoted && ch=='\\' )
	    quoted = true;
	else {
	    if ( pt>=end ) {
		cnt = pt-buf;
		buf = realloc(buf,(max+=200)+1);
		pt = buf+cnt;
		end = buf+max;
	    }
	    *pt++ = ch;
	    quoted = false;
	}
    }
    if ( pt==buf )
return( NULL );
    *pt='\0';
#ifdef _NO_PYTHON
return( copy(buf));
#else
return( PyFF_UnPickleMeToObjects(buf));
#endif
    /* buf is a static buffer, I don't free it, I'll reuse it next time */
}

/* ********************************* INPUT ********************************** */
#include "sfd1.h"

char *getquotedeol(FILE *sfd) {
    char *pt, *str, *end;
    int ch;

    pt = str = malloc(101); end = str+100;
    while ( isspace(ch = nlgetc(sfd)) && ch!='\r' && ch!='\n' );
    while ( ch!='\n' && ch!='\r' && ch!=EOF ) {
	if ( ch=='\\' ) {
	    /* We can't use nlgetc() here, because it would misinterpret */
	    /* double backslash at the end of line. Multiline strings,   */
	    /* broken with backslash + newline, are just handled above.  */
	    ch = getc(sfd);
	    if ( ch=='n' ) ch='\n';
	    /* else if ( ch=='\\' ) ch=='\\'; */ /* second backslash of '\\' */

	    /* FontForge doesn't write other escape sequences in this context. */
	    /* So any other value of ch is assumed impossible. */
	}
	if ( pt>=end ) {
	    pt = realloc(str,end-str+101);
	    end = pt+(end-str)+100;
	    str = pt;
	    pt = end-100;
	}
	*pt++ = ch;
	ch = nlgetc(sfd);
    }
    *pt='\0';
    /* these strings should be in utf8 now, but some old sfd files might have */
    /* latin1. Not a severe problems because they SHOULD be in ASCII. So any */
    /* non-ascii strings are erroneous anyway */
    if ( !utf8_valid(str) ) {
	pt = latin1_2_utf8_copy(str);
	free(str);
	str = pt;
    }
return( str );
}

static int geteol(FILE *sfd, char *tokbuf) {
    char *pt=tokbuf, *end = tokbuf+2000-2; int ch;

    while ( isspace(ch = nlgetc(sfd)) && ch!='\r' && ch!='\n' );
    while ( ch!='\n' && ch!='\r' && ch!=EOF ) {
	if ( pt<end ) *pt++ = ch;
	ch = nlgetc(sfd);
    }
    *pt='\0';
return( pt!=tokbuf?1:ch==EOF?-1: 0 );
}

static int getprotectedname(FILE *sfd, char *tokbuf) {
    char *pt=tokbuf, *end = tokbuf+100-2; int ch;

    while ( (ch = nlgetc(sfd))==' ' || ch=='\t' );
    while ( ch!=EOF && !isspace(ch) && ch!='[' && ch!=']' && ch!='{' && ch!='}' && ch!='<' && ch!='%' ) {
	if ( pt<end ) *pt++ = ch;
	ch = nlgetc(sfd);
    }
    if ( pt==tokbuf && ch!=EOF )
	*pt++ = ch;
    else
	ungetc(ch,sfd);
    *pt='\0';
return( pt!=tokbuf?1:ch==EOF?-1: 0 );
}

int getname(FILE *sfd, char *tokbuf) {
    int ch;

    while ( isspace(ch = nlgetc(sfd)));
    ungetc(ch,sfd);
return( getprotectedname(sfd,tokbuf));
}

static uint32 gettag(FILE *sfd) {
    int ch, quoted;
    uint32 tag;

    while ( (ch=nlgetc(sfd))==' ' );
    if ( (quoted = (ch=='\'')) ) ch = nlgetc(sfd);
    tag = (ch<<24)|(nlgetc(sfd)<<16);
    tag |= nlgetc(sfd)<<8;
    tag |= nlgetc(sfd);
    if ( quoted ) (void) nlgetc(sfd);
return( tag );
}

static int getint(FILE *sfd, int *val) {
    char tokbuf[100]; int ch;
    char *pt=tokbuf, *end = tokbuf+100-2;

    while ( isspace(ch = nlgetc(sfd)));
    if ( ch=='-' || ch=='+' ) {
	*pt++ = ch;
	ch = nlgetc(sfd);
    }
    while ( isdigit(ch)) {
	if ( pt<end ) *pt++ = ch;
	ch = nlgetc(sfd);
    }
    *pt='\0';
    ungetc(ch,sfd);
    *val = strtol(tokbuf,NULL,10);
return( pt!=tokbuf?1:ch==EOF?-1: 0 );
}

static int getlonglong(FILE *sfd, long long *val) {
    char tokbuf[100]; int ch;
    char *pt=tokbuf, *end = tokbuf+100-2;

    while ( isspace(ch = nlgetc(sfd)));
    if ( ch=='-' || ch=='+' ) {
	*pt++ = ch;
	ch = nlgetc(sfd);
    }
    while ( isdigit(ch)) {
	if ( pt<end ) *pt++ = ch;
	ch = nlgetc(sfd);
    }
    *pt='\0';
    ungetc(ch,sfd);
    *val = strtoll(tokbuf,NULL,10);
return( pt!=tokbuf?1:ch==EOF?-1: 0 );
}

static int gethex(FILE *sfd, uint32 *val) {
    char tokbuf[100]; int ch;
    char *pt=tokbuf, *end = tokbuf+100-2;

    while ( isspace(ch = nlgetc(sfd)));
    if ( ch=='#' )
	ch = nlgetc(sfd);
    if ( ch=='-' || ch=='+' ) {
	*pt++ = ch;
	ch = nlgetc(sfd);
    }
    if ( ch=='0' ) {
	ch = nlgetc(sfd);
	if ( ch=='x' || ch=='X' )
	    ch = nlgetc(sfd);
	else {
	    ungetc(ch,sfd);
	    ch = '0';
	}
    }
    while ( isdigit(ch) || (ch>='a' && ch<='f') || (ch>='A' && ch<='F')) {
	if ( pt<end ) *pt++ = ch;
	ch = nlgetc(sfd);
    }
    *pt='\0';
    ungetc(ch,sfd);
    *val = strtoul(tokbuf,NULL,16);
return( pt!=tokbuf?1:ch==EOF?-1: 0 );
}

static int gethexints(FILE *sfd, uint32 *val, int cnt) {
    int i, ch;

    for ( i=0; i<cnt; ++i ) {
	if ( i!=0 ) {
	    ch = nlgetc(sfd);
	    if ( ch!='.' ) ungetc(ch,sfd);
	}
	if ( !gethex(sfd,&val[i]))
return( false );
    }
return( true );
}

static int getsint(FILE *sfd, int16 *val) {
    int val2;
    int ret = getint(sfd,&val2);
    *val = val2;
return( ret );
}

static int getusint(FILE *sfd, uint16 *val) {
    int val2;
    int ret = getint(sfd,&val2);
    *val = val2;
return( ret );
}

static int getreal(FILE *sfd, real *val) {
    char tokbuf[100];
    int ch;
    char *pt=tokbuf, *end = tokbuf+100-2, *nend;

    while ( isspace(ch = nlgetc(sfd)));
    if ( ch!='e' && ch!='E' )		/* real's can't begin with exponants */
	while ( isdigit(ch) || ch=='-' || ch=='+' || ch=='e' || ch=='E' || ch=='.' || ch==',' ) {
	    if ( pt<end ) *pt++ = ch;
	    ch = nlgetc(sfd);
	}
    *pt='\0';
    ungetc(ch,sfd);
    *val = strtod(tokbuf,&nend);
    /* Beware of different locals! */
    if ( *nend!='\0' ) {
	if ( *nend=='.' )
	    *nend = ',';
	else if ( *nend==',' )
	    *nend = '.';
	*val = strtod(tokbuf,&nend);
    }
return( pt!=tokbuf && *nend=='\0'?1:ch==EOF?-1: 0 );
}

/* Don't use nlgetc here. We carefully control newlines when dumping in 85 */
/*  but backslashes can occur at end of line. */
static int Dec85(struct enc85 *dec) {
    int ch1, ch2, ch3, ch4, ch5;
    unsigned int val;

    if ( dec->pos<0 ) {
	while ( isspace(ch1=getc(dec->sfd)));
	if ( ch1=='z' ) {
	    dec->sofar[0] = dec->sofar[1] = dec->sofar[2] = dec->sofar[3] = 0;
	    dec->pos = 3;
	} else {
	    while ( isspace(ch2=getc(dec->sfd)));
	    while ( isspace(ch3=getc(dec->sfd)));
	    while ( isspace(ch4=getc(dec->sfd)));
	    while ( isspace(ch5=getc(dec->sfd)));
	    val = ((((ch1-'!')*85+ ch2-'!')*85 + ch3-'!')*85 + ch4-'!')*85 + ch5-'!';
	    dec->sofar[3] = val>>24;
	    dec->sofar[2] = val>>16;
	    dec->sofar[1] = val>>8;
	    dec->sofar[0] = val;
	    dec->pos = 3;
	}
    }
return( dec->sofar[dec->pos--] );
}

static void SFDGetType1(FILE *sfd) {
    /* We've read the OrigType1 token (this is now obselete, but parse it in case there are any old sfds) */
    int len;
    struct enc85 dec;

    memset(&dec,'\0', sizeof(dec)); dec.pos = -1;
    dec.sfd = sfd;

    getint(sfd,&len);
    while ( --len >= 0 )
	Dec85(&dec);
}

static void SFDGetTtfInstrs(FILE *sfd, SplineChar *sc) {
    /* We've read the TtfInstr token, it is followed by a byte count */
    /* and then the instructions in enc85 format */
    int i,len;
    struct enc85 dec;

    memset(&dec,'\0', sizeof(dec)); dec.pos = -1;
    dec.sfd = sfd;

    getint(sfd,&len);
    sc->ttf_instrs = malloc(len);
    sc->ttf_instrs_len = len;
    for ( i=0; i<len; ++i )
	sc->ttf_instrs[i] = Dec85(&dec);
}

static void tterr(void *UNUSED(rubbish), char *message, int UNUSED(pos)) {
    LogError(_("When loading tt instrs from sfd: %s\n"), message );
}

static void SFDGetTtInstrs(FILE *sfd, SplineChar *sc) {
    /* We've read the TtInstr token, it is followed by text versions of */
    /*  the instructions, slurp it all into a big buffer, and then parse that */
    char *buf=NULL, *pt=buf, *end=buf;
    int ch;
    int backlen = strlen(end_tt_instrs);
    int instr_len;

    while ( (ch=nlgetc(sfd))!=EOF ) {
	if ( pt>=end ) {
	    char *newbuf = realloc(buf,(end-buf+200));
	    pt = newbuf+(pt-buf);
	    end = newbuf+(end+200-buf);
	    buf = newbuf;
	}
	*pt++ = ch;
	if ( pt-buf>backlen && strncmp(pt-backlen,end_tt_instrs,backlen)==0 ) {
	    pt -= backlen;
    break;
	}
    }
    *pt = '\0';

    sc->ttf_instrs = _IVParse(sc->parent,buf,&instr_len,tterr,NULL);
    sc->ttf_instrs_len = instr_len;

    free(buf);
}

static struct ttf_table *SFDGetTtfTable(FILE *sfd, SplineFont *sf,struct ttf_table *lasttab[2]) {
    /* We've read the TtfTable token, it is followed by a tag and a byte count */
    /* and then the instructions in enc85 format */
    int i,len;
    int which;
    struct enc85 dec;
    struct ttf_table *tab = chunkalloc(sizeof(struct ttf_table));

    memset(&dec,'\0', sizeof(dec)); dec.pos = -1;
    dec.sfd = sfd;

    tab->tag = gettag(sfd);

    if ( tab->tag==CHR('f','p','g','m') || tab->tag==CHR('p','r','e','p') ||
	    tab->tag==CHR('c','v','t',' ') || tab->tag==CHR('m','a','x','p'))
	which = 0;
    else
	which = 1;

    getint(sfd,&len);
    tab->data = malloc(len);
    tab->len = len;
    for ( i=0; i<len; ++i )
	tab->data[i] = Dec85(&dec);

    if ( lasttab[which]!=NULL )
	lasttab[which]->next = tab;
    else if ( which==0 )
	sf->ttf_tables = tab;
    else
	sf->ttf_tab_saved = tab;
    lasttab[which] = tab;
return( tab );
}

static struct ttf_table *SFDGetShortTable(FILE *sfd, SplineFont *sf,struct ttf_table *lasttab[2]) {
    /* We've read the ShortTable token, it is followed by a tag and a word count */
    /* and then the (text) values of the words that make up the cvt table */
    int i,len, ch;
    uint8 *pt;
    int which, iscvt, started;
    struct ttf_table *tab = chunkalloc(sizeof(struct ttf_table));

    tab->tag = gettag(sfd);

    if ( tab->tag==CHR('f','p','g','m') || tab->tag==CHR('p','r','e','p') ||
	    tab->tag==CHR('c','v','t',' ') || tab->tag==CHR('m','a','x','p'))
	which = 0;
    else
	which = 1;
    iscvt = tab->tag==CHR('c','v','t',' ');

    getint(sfd,&len);
    pt = tab->data = malloc(2*len);
    tab->len = 2*len;
    started = false;
    for ( i=0; i<len; ++i ) {
	int num;
	getint(sfd,&num);
	*pt++ = num>>8;
	*pt++ = num&0xff;
	if ( iscvt ) {
	    ch = nlgetc(sfd);
	    if ( ch==' ' ) {
		if ( !started ) {
		    sf->cvt_names = calloc(len+1,sizeof(char *));
		    sf->cvt_names[len] = END_CVT_NAMES;
		    started = true;
		}
		sf->cvt_names[i] = SFDReadUTF7Str(sfd);
	    } else
		ungetc(ch,sfd);
	}
    }

    if ( lasttab[which]!=NULL )
	lasttab[which]->next = tab;
    else if ( which==0 )
	sf->ttf_tables = tab;
    else
	sf->ttf_tab_saved = tab;
    lasttab[which] = tab;
return( tab );
}

static struct ttf_table *SFDGetTtTable(FILE *sfd, SplineFont *sf,struct ttf_table *lasttab[2]) {
    /* We've read the TtTable token, it is followed by a tag */
    /* and then the instructions in text format */
    int ch;
    int which;
    struct ttf_table *tab = chunkalloc(sizeof(struct ttf_table));
    char *buf=NULL, *pt=buf, *end=buf;
    int backlen = strlen(end_tt_instrs);

    tab->tag = gettag(sfd);

    if ( tab->tag==CHR('f','p','g','m') || tab->tag==CHR('p','r','e','p') ||
	    tab->tag==CHR('c','v','t',' ') || tab->tag==CHR('m','a','x','p'))
	which = 0;
    else
	which = 1;

    while ( (ch=nlgetc(sfd))!=EOF ) {
	if ( pt>=end ) {
	    char *newbuf = realloc(buf,(end-buf+200));
	    pt = newbuf+(pt-buf);
	    end = newbuf+(end+200-buf);
	    buf = newbuf;
	}
	*pt++ = ch;
	if ( pt-buf>backlen && strncmp(pt-backlen,end_tt_instrs,backlen)==0 ) {
	    pt -= backlen;
    break;
	}
    }
    *pt = '\0';
    tab->data = _IVParse(sf,buf,(int*)&tab->len,tterr,NULL);
    free(buf);

    if ( lasttab[which]!=NULL )
	lasttab[which]->next = tab;
    else if ( which==0 )
	sf->ttf_tables = tab;
    else
	sf->ttf_tab_saved = tab;
    lasttab[which] = tab;
return( tab );
}

static int SFDCloseCheck(SplinePointList *spl,int order2) {
    if ( spl->first!=spl->last &&
	    RealNear(spl->first->me.x,spl->last->me.x) &&
	    RealNear(spl->first->me.y,spl->last->me.y)) {
	SplinePoint *oldlast = spl->last;
	spl->first->prevcp = oldlast->prevcp;
	spl->first->noprevcp = oldlast->noprevcp;
	oldlast->prev->from->next = NULL;
	spl->last = oldlast->prev->from;
	chunkfree(oldlast->prev,sizeof(*oldlast));
	chunkfree(oldlast->hintmask,sizeof(HintMask));
	chunkfree(oldlast,sizeof(*oldlast));
	SplineMake(spl->last,spl->first,order2);
	spl->last = spl->first;
return( true );
    }
return( false );
}

static void SFDGetHintMask(FILE *sfd,HintMask *hintmask) {
    int nibble = 0, ch;

    memset(hintmask,0,sizeof(HintMask));
    for (;;) {
	ch = nlgetc(sfd);
	if ( isdigit(ch))
	    ch -= '0';
	else if ( ch>='a' && ch<='f' )
	    ch -= 'a'-10;
	else if ( ch>='A' && ch<='F' )
	    ch -= 'A'-10;
	else {
	    ungetc(ch,sfd);
    break;
	}
	if ( nibble<2*HntMax/8 )
	    (*hintmask)[nibble>>1] |= ch<<(4*(1-(nibble&1)));
	++nibble;
    }
}

static void SFDGetSpiros(FILE *sfd,SplineSet *cur) {
    int ch;
    spiro_cp cp;

    ch = nlgetc(sfd);		/* S */
    ch = nlgetc(sfd);		/* p */
    ch = nlgetc(sfd);		/* i */
    ch = nlgetc(sfd);		/* r */
    ch = nlgetc(sfd);		/* o */
    while ( fscanf(sfd,"%lg %lg %c", &cp.x, &cp.y, &cp.ty )==3 ) {
	if ( cur!=NULL ) {
	    if ( cur->spiro_cnt>=cur->spiro_max )
		cur->spiros = realloc(cur->spiros,(cur->spiro_max+=10)*sizeof(spiro_cp));
	    cur->spiros[cur->spiro_cnt++] = cp;
	}
    }
    if ( cur!=NULL && (cur->spiros[cur->spiro_cnt-1].ty&0x7f)!=SPIRO_END ) {
	if ( cur->spiro_cnt>=cur->spiro_max )
	    cur->spiros = realloc(cur->spiros,(cur->spiro_max+=1)*sizeof(spiro_cp));
	memset(&cur->spiros[cur->spiro_cnt],0,sizeof(spiro_cp));
	cur->spiros[cur->spiro_cnt++].ty = SPIRO_END;
    }
    ch = nlgetc(sfd);
    if ( ch=='E' ) {
	ch = nlgetc(sfd);		/* n */
	ch = nlgetc(sfd);		/* d */
	ch = nlgetc(sfd);		/* S */
	ch = nlgetc(sfd);		/* p */
	ch = nlgetc(sfd);		/* i */
	ch = nlgetc(sfd);		/* r */
	ch = nlgetc(sfd);		/* o */
    } else
	ungetc(ch,sfd);
}

static SplineSet *SFDGetSplineSet(FILE *sfd,int order2) {
    SplinePointList *cur=NULL, *head=NULL;
    BasePoint current;
    real stack[100];
    int sp=0;
    SplinePoint *pt = NULL;
    int ch;
	int ch2;
    char tok[100];
    int ttfindex = 0;
    int lastacceptable;

    current.x = current.y = 0;
    lastacceptable = 0;
    while ( 1 ) {
	int have_read_val = 0;
	int val = 0;

	while ( getreal(sfd,&stack[sp])==1 )
	    if ( sp<99 )
		++sp;
	while ( isspace(ch=nlgetc(sfd)));
	if ( ch=='E' || ch=='e' || ch==EOF )
    break;
	if ( ch=='S' ) {
	    ungetc(ch,sfd);
	    SFDGetSpiros(sfd,cur);
    continue;
	} else if (( ch=='N' ) &&
	    nlgetc(sfd)=='a' &&	/* a */
	    nlgetc(sfd)=='m' &&	/* m */
	    nlgetc(sfd)=='e' &&	/* e */
	    nlgetc(sfd)=='d' ) /* d */ {
	    ch2 = nlgetc(sfd);		/* : */
		// We are either fetching a splineset name (Named:) or a point name (NamedP:).
		if (ch2=='P') { if ((nlgetc(sfd)==':') && (pt!=NULL)) { if (pt->name!=NULL) {free(pt->name);} pt->name = SFDReadUTF7Str(sfd); } }
		else if (ch2==':') { if (cur != NULL) cur->contour_name = SFDReadUTF7Str(sfd); else { char * freetmp = SFDReadUTF7Str(sfd); free(freetmp); freetmp = NULL; } }
        continue;
	} else if ( ch=='P' && PeekMatch(sfd,"ath") ) {
	    int flags;
	    nlgetc(sfd);		/* a */
	    nlgetc(sfd);		/* t */
	    nlgetc(sfd);		/* h */
	    if (PeekMatch(sfd,"Flags:")) {
	      nlgetc(sfd);		/* F */
	      nlgetc(sfd);		/* l */
	      nlgetc(sfd);		/* a */
	      nlgetc(sfd);		/* g */
	      nlgetc(sfd);		/* s */
	      nlgetc(sfd);		/* : */
	      getint(sfd,&flags);
	      if (cur != NULL) cur->is_clip_path = flags&1;
	    } else if (PeekMatch(sfd,"Start:")) {
	      nlgetc(sfd);		/* S */
	      nlgetc(sfd);		/* t */
	      nlgetc(sfd);		/* a */
	      nlgetc(sfd);		/* r */
	      nlgetc(sfd);		/* t */
	      nlgetc(sfd);		/* : */
	      getint(sfd,&flags);
	      if (cur != NULL) cur->start_offset = flags;
	    }
	}
	pt = NULL;
	if ( ch=='l' || ch=='m' ) {
	    if ( sp>=2 ) {
		current.x = stack[sp-2];
		current.y = stack[sp-1];
		sp -= 2;
		pt = chunkalloc(sizeof(SplinePoint));
		pt->me = current;
		pt->noprevcp = true; pt->nonextcp = true;
		if ( ch=='m' ) {
		    SplinePointList *spl = chunkalloc(sizeof(SplinePointList));
		    spl->first = spl->last = pt;
		    spl->start_offset = 0;
		    if ( cur!=NULL ) {
			if ( SFDCloseCheck(cur,order2))
			    --ttfindex;
			cur->next = spl;
		    } else
			head = spl;
		    cur = spl;
		} else {
		    if ( cur!=NULL && cur->first!=NULL && (cur->first!=cur->last || cur->first->next==NULL) ) {
			if ( cur->last->nextcpindex==0xfffe )
			    cur->last->nextcpindex = 0xffff;
			SplineMake(cur->last,pt,order2);
			cur->last->nonextcp = 1;
			pt->noprevcp = 1;
			cur->last = pt;
		    }
		}
	    } else
		sp = 0;
	} else if ( ch=='c' ) {
	    if ( sp>=6 ) {
		getint(sfd,&val);
		have_read_val = 1;


		current.x = stack[sp-2];
		current.y = stack[sp-1];
		real original_current_x = current.x;
		if( val & SFD_PTFLAG_FORCE_OPEN_PATH )
		{
		    // Find somewhere vacant to put the point.x for now
		    // we need to do this check in case we choose a point that is already
		    // on the spline and this connect back to that point instead of creating
		    // an open path
		    while( 1 )
		    {
			real offset = 0.1;
			current.x += offset;
			if( !cur || !SplinePointListContainsPointAtX( cur, current.x ))
			{
			    break;
			}
		    }
		}

		if ( cur!=NULL && cur->first!=NULL && (cur->first!=cur->last || cur->first->next==NULL) ) {
		    cur->last->nextcp.x = stack[sp-6];
		    cur->last->nextcp.y = stack[sp-5];
		    cur->last->nonextcp = false;
		    pt = chunkalloc(sizeof(SplinePoint));
		    pt->prevcp.x = stack[sp-4];
		    pt->prevcp.y = stack[sp-3];
		    pt->me = current;
		    pt->nonextcp = true;
		    if ( cur->last->nextcpindex==0xfffe )
			cur->last->nextcpindex = ttfindex++;
		    else if ( cur->last->nextcpindex!=0xffff )
			ttfindex = cur->last->nextcpindex+1;
		    SplineMake(cur->last,pt,order2);
		    cur->last = pt;
		    // pt->me is a copy of 'current' so we should now move
		    // the x coord of pt->me back to where it should be.
		    // The whole aim here is that this spline remains an open path
		    // when PTFLAG_FORCE_OPEN_PATH is set.
		    pt->me.x = original_current_x;
		}

		// Move the point back to the same location it was
		// but do not connect it back to the point that is
		// already there.
		if( val & SFD_PTFLAG_FORCE_OPEN_PATH )
		{
		    current.x = original_current_x;
		}

		sp -= 6;
	    } else
		sp = 0;
	}
	if ( pt!=NULL ) {
	    if( !have_read_val )
		getint(sfd,&val);

	    pt->pointtype = (val & SFD_PTFLAG_TYPE_MASK);
	    pt->selected  = (val & SFD_PTFLAG_IS_SELECTED) > 0;
	    pt->nextcpdef = (val & SFD_PTFLAG_NEXTCP_IS_DEFAULT) > 0;
	    pt->prevcpdef = (val & SFD_PTFLAG_PREVCP_IS_DEFAULT) > 0;
	    pt->roundx    = (val & SFD_PTFLAG_ROUND_IN_X) > 0;
	    pt->roundy    = (val & SFD_PTFLAG_ROUND_IN_Y) > 0;
	    pt->dontinterpolate = (val & SFD_PTFLAG_INTERPOLATE_NEVER) > 0;
	    if ( pt->prev!=NULL )
		pt->prev->acceptableextrema = (val & SFD_PTFLAG_PREV_EXTREMA_MARKED_ACCEPTABLE) > 0;
	    else
		lastacceptable = (val & SFD_PTFLAG_PREV_EXTREMA_MARKED_ACCEPTABLE) > 0;
	    if ( val&0x80 )
		pt->ttfindex = 0xffff;
	    else
		pt->ttfindex = ttfindex++;
	    pt->nextcpindex = 0xfffe;
	    ch = nlgetc(sfd);
	    if ( ch=='x' ) {
		pt->hintmask = chunkalloc(sizeof(HintMask));
		SFDGetHintMask(sfd,pt->hintmask);
	    } else if ( ch!=',' )
		ungetc(ch,sfd);
	    else {
		ch = nlgetc(sfd);
		if ( ch==',' )
		    pt->ttfindex = 0xfffe;
		else {
		    ungetc(ch,sfd);
		    getint(sfd,&val);
		    pt->ttfindex = val;
		    nlgetc(sfd);	/* skip comma */
		    if ( val!=-1 )
			ttfindex = val+1;
		}
		ch = nlgetc(sfd);
		if ( ch=='\r' || ch=='\n' )
		    ungetc(ch,sfd);
		else {
		    ungetc(ch,sfd);
		    getint(sfd,&val);
		    pt->nextcpindex = val;
		    if ( val!=-1 )
			ttfindex = val+1;
		}
	    }
	}
    }
    if ( cur!=NULL )
	SFDCloseCheck(cur,order2);
    if ( lastacceptable && cur->last->prev!=NULL )
	cur->last->prev->acceptableextrema = true;
    getname(sfd,tok);
return( head );
}

static void SFDGetMinimumDistances(FILE *sfd, SplineChar *sc) {
    SplineSet *ss;
    SplinePoint *sp;
    int pt,i, val, err;
    int ch;
    SplinePoint **mapping=NULL;
    MinimumDistance *last, *md, *mdhead=NULL;

    for ( i=0; i<2; ++i ) {
	pt = 0;
	for ( ss = sc->layers[ly_fore].splines; ss!=NULL; ss=ss->next ) {
	    for ( sp=ss->first; ; ) {
		if ( mapping!=NULL ) mapping[pt] = sp;
		pt++;
		if ( sp->next == NULL )
	    break;
		sp = sp->next->to;
		if ( sp==ss->first )
	    break;
	    }
	}
	if ( mapping==NULL )
	    mapping = calloc(pt,sizeof(SplinePoint *));
    }

    last = NULL;
    for ( ch=nlgetc(sfd); ch!=EOF && ch!='\n'; ch=nlgetc(sfd)) {
	err = false;
	while ( isspace(ch) && ch!='\n' ) ch=nlgetc(sfd);
	if ( ch=='\n' )
    break;
	md = chunkalloc(sizeof(MinimumDistance));
	if ( ch=='x' ) md->x = true;
	getint(sfd,&val);
	if ( val<-1 || val>=pt ) {
	    IError( "Minimum Distance specifies bad point (%d) in sfd file\n", val );
	    err = true;
	} else if ( val!=-1 ) {
	    md->sp1 = mapping[val];
	    md->sp1->dontinterpolate = true;
	}
	ch = nlgetc(sfd);
	if ( ch!=',' ) {
	    IError( "Minimum Distance lacks a comma where expected\n" );
	    err = true;
	}
	getint(sfd,&val);
	if ( val<-1 || val>=pt ) {
	    IError( "Minimum Distance specifies bad point (%d) in sfd file\n", val );
	    err = true;
	} else if ( val!=-1 ) {
	    md->sp2 = mapping[val];
	    md->sp2->dontinterpolate = true;
	}
	if ( !err ) {
	    if ( last!=NULL )
		last->next = md;
	    last = md;
	} else
	    chunkfree(md,sizeof(MinimumDistance));
    }
    free(mapping);

    /* Obsolete concept */
    MinimumDistancesFree(mdhead);
}

static HintInstance *SFDReadHintInstances(FILE *sfd, StemInfo *stem) {
    HintInstance *head=NULL, *last=NULL, *cur;
    real begin, end;
    int ch;

    while ( (ch=nlgetc(sfd))==' ' || ch=='\t' );
    if ( ch=='G' && stem != NULL ) {
	stem->ghost = true;
	while ( (ch=nlgetc(sfd))==' ' || ch=='\t' );
    }
    if ( ch!='<' ) {
	ungetc(ch,sfd);
return(NULL);
    }
    while ( getreal(sfd,&begin)==1 && getreal(sfd,&end)) {
	cur = chunkalloc(sizeof(HintInstance));
	cur->begin = begin;
	cur->end = end;
	if ( head == NULL )
	    head = cur;
	else
	    last->next = cur;
	last = cur;
    }
    while ( (ch=nlgetc(sfd))==' ' || ch=='\t' );
    if ( ch!='>' )
	ungetc(ch,sfd);
return( head );
}

static StemInfo *SFDReadHints(FILE *sfd) {
    StemInfo *head=NULL, *last=NULL, *cur;
    real start, width;

    while ( getreal(sfd,&start)==1 && getreal(sfd,&width)) {
	cur = chunkalloc(sizeof(StemInfo));
	cur->start = start;
	cur->width = width;
	cur->where = SFDReadHintInstances(sfd,cur);
	if ( head == NULL )
	    head = cur;
	else
	    last->next = cur;
	last = cur;
    }
return( head );
}

static DStemInfo *SFDReadDHints( SplineFont *sf,FILE *sfd,int old ) {
    DStemInfo *head=NULL, *last=NULL, *cur;
    int i;
    BasePoint bp[4], *bpref[4], left, right, unit;
    double rstartoff, rendoff, lendoff;

    if ( old ) {
        for ( i=0 ; i<4 ; i++ ) bpref[i] = &bp[i];

        while ( getreal( sfd,&bp[0].x ) && getreal( sfd,&bp[0].y ) &&
	        getreal( sfd,&bp[1].x ) && getreal( sfd,&bp[1].y ) &&
	        getreal( sfd,&bp[2].x ) && getreal( sfd,&bp[2].y ) &&
	        getreal( sfd,&bp[3].x ) && getreal( sfd,&bp[3].y )) {

            /* Ensure point coordinates specified in the sfd file do */
            /* form a diagonal line */
            if ( PointsDiagonalable( sf,bpref,&unit )) {
	        cur = chunkalloc( sizeof( DStemInfo ));
	        cur->left = *bpref[0];
	        cur->right = *bpref[1];
                cur->unit = unit;
                /* Generate a temporary hint instance, so that the hint can */
                /* be visible in charview even if subsequent rebuilding instances */
                /* fails (e. g. for composite characters) */
                cur->where = chunkalloc( sizeof( HintInstance ));
                rstartoff = ( cur->right.x - cur->left.x ) * cur->unit.x +
                            ( cur->right.y - cur->left.y ) * cur->unit.y;
                rendoff =   ( bpref[2]->x - cur->left.x ) * cur->unit.x +
                            ( bpref[2]->y - cur->left.y ) * cur->unit.y;
                lendoff =   ( bpref[3]->x - cur->left.x ) * cur->unit.x +
                            ( bpref[3]->y - cur->left.y ) * cur->unit.y;
                cur->where->begin = ( rstartoff > 0 ) ? rstartoff : 0;
                cur->where->end   = ( rendoff > lendoff ) ? lendoff : rendoff;
                MergeDStemInfo( sf,&head,cur );
            }
        }
    } else {
        while ( getreal( sfd,&left.x ) && getreal( sfd,&left.y ) &&
                getreal( sfd,&right.x ) && getreal( sfd,&right.y ) &&
                getreal( sfd,&unit.x ) && getreal( sfd,&unit.y )) {
	    cur = chunkalloc( sizeof( DStemInfo ));
	    cur->left = left;
	    cur->right = right;
            cur->unit = unit;
	    cur->where = SFDReadHintInstances( sfd,NULL );
	    if ( head == NULL )
	        head = cur;
	    else
	        last->next = cur;
	    last = cur;
        }
    }
return( head );
}

static DeviceTable *SFDReadDeviceTable(FILE *sfd,DeviceTable *adjust) {
    int i, junk, first, last, ch, len;

    while ( (ch=nlgetc(sfd))==' ' );
    if ( ch=='{' ) {
	while ( (ch=nlgetc(sfd))==' ' );
	if ( ch=='}' )
return(NULL);
	else
	    ungetc(ch,sfd);
	getint(sfd,&first);
	ch = nlgetc(sfd);		/* Should be '-' */
	getint(sfd,&last);
	len = last-first+1;
	if ( len<=0 ) {
	    IError( "Bad device table, invalid length.\n" );
return(NULL);
	}
	if ( adjust==NULL )
	    adjust = chunkalloc(sizeof(DeviceTable));
	adjust->first_pixel_size = first;
	adjust->last_pixel_size = last;
	adjust->corrections = malloc(len);
	for ( i=0; i<len; ++i ) {
	    while ( (ch=nlgetc(sfd))==' ' );
	    if ( ch!=',' ) ungetc(ch,sfd);
	    getint(sfd,&junk);
	    adjust->corrections[i] = junk;
	}
	while ( (ch=nlgetc(sfd))==' ' );
	if ( ch!='}' ) ungetc(ch,sfd);
    } else
	ungetc(ch,sfd);
return( adjust );
}

static ValDevTab *SFDReadValDevTab(FILE *sfd) {
    int i, j, ch;
    ValDevTab vdt;
    char buf[4];

    memset(&vdt,0,sizeof(vdt));
    buf[3] = '\0';
    while ( (ch=nlgetc(sfd))==' ' );
    if ( ch=='[' ) {
	for ( i=0; i<4; ++i ) {
	    while ( (ch=nlgetc(sfd))==' ' );
	    if ( ch==']' )
	break;
	    buf[0]=ch;
	    for ( j=1; j<3; ++j ) buf[j]=nlgetc(sfd);
	    while ( (ch=nlgetc(sfd))==' ' );
	    if ( ch!='=' ) ungetc(ch,sfd);
	    SFDReadDeviceTable(sfd,
		    strcmp(buf,"ddx")==0 ? &vdt.xadjust :
		    strcmp(buf,"ddy")==0 ? &vdt.yadjust :
		    strcmp(buf,"ddh")==0 ? &vdt.xadv :
		    strcmp(buf,"ddv")==0 ? &vdt.yadv :
			(&vdt.xadjust) + i );
	    while ( (ch=nlgetc(sfd))==' ' );
	    if ( ch!=']' ) ungetc(ch,sfd);
	    else
	break;
	}
	if ( vdt.xadjust.corrections!=NULL || vdt.yadjust.corrections!=NULL ||
		vdt.xadv.corrections!=NULL || vdt.yadv.corrections!=NULL ) {
	    ValDevTab *v = chunkalloc(sizeof(ValDevTab));
	    *v = vdt;
return( v );
	}
    } else
	ungetc(ch,sfd);
return( NULL );
}

static AnchorPoint *SFDReadAnchorPoints(FILE *sfd,SplineChar *sc,AnchorPoint** alist, AnchorPoint *lastap)
{
    AnchorPoint *ap = chunkalloc(sizeof(AnchorPoint));
    AnchorClass *an;
    char *name;
    char tok[200];
    int ch;

    name = SFDReadUTF7Str(sfd);
    if ( name==NULL ) {
	LogError(_("Anchor Point with no class name: %s"), sc->name );
	AnchorPointsFree(ap);
return( lastap );
    }
    for ( an=sc->parent->anchor; an!=NULL && strcmp(an->name,name)!=0; an=an->next );
    free(name);
    ap->anchor = an;
    getreal(sfd,&ap->me.x);
    getreal(sfd,&ap->me.y);
    ap->type = -1;
    if ( getname(sfd,tok)==1 ) {
	if ( strcmp(tok,"mark")==0 )
	    ap->type = at_mark;
	else if ( strcmp(tok,"basechar")==0 )
	    ap->type = at_basechar;
	else if ( strcmp(tok,"baselig")==0 )
	    ap->type = at_baselig;
	else if ( strcmp(tok,"basemark")==0 )
	    ap->type = at_basemark;
	else if ( strcmp(tok,"entry")==0 )
	    ap->type = at_centry;
	else if ( strcmp(tok,"exit")==0 )
	    ap->type = at_cexit;
    }
    getsint(sfd,&ap->lig_index);
    ch = nlgetc(sfd);
    ungetc(ch,sfd);
    if ( ch==' ' ) {
	SFDReadDeviceTable(sfd,&ap->xadjust);
	SFDReadDeviceTable(sfd,&ap->yadjust);
	ch = nlgetc(sfd);
	ungetc(ch,sfd);
	if ( isdigit(ch)) {
	    getsint(sfd,(int16 *) &ap->ttf_pt_index);
	    ap->has_ttf_pt = true;
	}
    }
    if ( ap->anchor==NULL || ap->type==-1 ) {
	LogError(_("Bad Anchor Point: %s"), sc->name );
	AnchorPointsFree(ap);
return( lastap );
    }
    if ( lastap==NULL )
	(*alist) = ap;
    else
	lastap->next = ap;

    return( ap );
}

static RefChar *SFDGetRef(FILE *sfd, int was_enc) {
    RefChar *rf;
    int temp=0, ch;

    rf = RefCharCreate();
    getint(sfd,&rf->orig_pos);
    rf->encoded = was_enc;
    if ( getint(sfd,&temp))
	rf->unicode_enc = temp;
    while ( isspace(ch=nlgetc(sfd)));
    if ( ch=='S' ) rf->selected = true;
    getreal(sfd,&rf->transform[0]);
    getreal(sfd,&rf->transform[1]);
    getreal(sfd,&rf->transform[2]);
    getreal(sfd,&rf->transform[3]);
    getreal(sfd,&rf->transform[4]);
    getreal(sfd,&rf->transform[5]);
    while ( (ch=nlgetc(sfd))==' ');
    ungetc(ch,sfd);
    if ( isdigit(ch) ) {
	getint(sfd,&temp);
	rf->use_my_metrics = temp&1;
	rf->round_translation_to_grid = (temp&2)?1:0;
	rf->point_match = (temp&4)?1:0;
	if ( rf->point_match ) {
	    getsint(sfd,(int16 *) &rf->match_pt_base);
	    getsint(sfd,(int16 *) &rf->match_pt_ref);
	    while ( (ch=nlgetc(sfd))==' ');
	    if ( ch=='O' )
		rf->point_match_out_of_date = true;
	    else
		ungetc(ch,sfd);
	}
    }
return( rf );
}

/* I used to create multiple ligatures by putting ";" between them */
/* that is the component string for "ffi" was "ff i ; f f i" */
/* Now I want to have separate ligature structures for each */
static PST1 *LigaCreateFromOldStyleMultiple(PST1 *liga) {
    char *pt;
    PST1 *new, *last=liga;
    while ( (pt = strrchr(liga->pst.u.lig.components,';'))!=NULL ) {
	new = chunkalloc(sizeof( PST1 ));
	*new = *liga;
	new->pst.u.lig.components = copy(pt+1);
	last->pst.next = (PST *) new;
	last = new;
	*pt = '\0';
    }
return( last );
}

#ifdef FONTFORGE_CONFIG_CVT_OLD_MAC_FEATURES
static struct { int feature, setting; uint32 tag; } formertags[] = {
    { 1, 6, CHR('M','L','O','G') },
    { 1, 8, CHR('M','R','E','B') },
    { 1, 10, CHR('M','D','L','G') },
    { 1, 12, CHR('M','S','L','G') },
    { 1, 14, CHR('M','A','L','G') },
    { 8, 0, CHR('M','S','W','I') },
    { 8, 2, CHR('M','S','W','F') },
    { 8, 4, CHR('M','S','L','I') },
    { 8, 6, CHR('M','S','L','F') },
    { 8, 8, CHR('M','S','N','F') },
    { 22, 1, CHR('M','W','I','D') },
    { 27, 1, CHR('M','U','C','M') },
    { 103, 2, CHR('M','W','I','D') },
    { -1, -1, 0xffffffff },
};

static void CvtOldMacFeature(PST1 *pst) {
    int i;

    if ( pst->macfeature )
return;
    for ( i=0; formertags[i].feature!=-1 ; ++i ) {
	if ( pst->tag == formertags[i].tag ) {
	    pst->macfeature = true;
	    pst->tag = (formertags[i].feature<<16) | formertags[i].setting;
return;
	}
    }
}
#endif

static void SFDSetEncMap(SplineFont *sf,int orig_pos,int enc) {
    EncMap *map = sf->map;

    if ( map==NULL )
return;

    if ( orig_pos>=map->backmax ) {
	int old = map->backmax;
	map->backmax = orig_pos+10;
	map->backmap = realloc(map->backmap,map->backmax*sizeof(int));
	memset(map->backmap+old,-1,(map->backmax-old)*sizeof(int));
    }
    if ( map->backmap[orig_pos] == -1 )		/* backmap will not be unique if multiple encodings come from same glyph */
	map->backmap[orig_pos] = enc;
    if ( enc>=map->encmax ) {
	int old = map->encmax;
	map->encmax = enc+10;
	map->map = realloc(map->map,map->encmax*sizeof(int));
	memset(map->map+old,-1,(map->encmax-old)*sizeof(int));
    }
    if ( enc>=map->enccount )
	map->enccount = enc+1;
    if ( enc!=-1 )
	map->map[enc] = orig_pos;
}

static void SCDefaultInterpolation(SplineChar *sc) {
    SplineSet *cur;
    SplinePoint *sp;
    /* We used not to store the dontinterpolate bit. We used to use the */
    /* presence or absence of instructions as that flag */

    if ( sc->ttf_instrs_len!=0 ) {
	for ( cur=sc->layers[ly_fore].splines; cur!=NULL; cur=cur->next ) {
	    for ( sp=cur->first; ; ) {
		if ( sp->ttfindex!=0xffff && SPInterpolate(sp))
		    sp->dontinterpolate = true;
		if ( sp->next==NULL )
	    break;
		sp=sp->next->to;
		if ( sp==cur->first )
	    break;
	    }
	}
    }
}

static void SFDParseMathValueRecord(FILE *sfd,int16 *value,DeviceTable **devtab) {
    getsint(sfd,value);
    *devtab = SFDReadDeviceTable(sfd,NULL);
}

static struct glyphvariants *SFDParseGlyphComposition(FILE *sfd,
	struct glyphvariants *gv, char *tok) {
    int i;

    if ( gv==NULL )
	gv = chunkalloc(sizeof(struct glyphvariants));
    getint(sfd,&gv->part_cnt);
    gv->parts = calloc(gv->part_cnt,sizeof(struct gv_part));
    for ( i=0; i<gv->part_cnt; ++i ) {
	int temp, ch;
	getname(sfd,tok);
	gv->parts[i].component = copy(tok);
	while ( (ch=nlgetc(sfd))==' ' );
	if ( ch!='%' ) ungetc(ch,sfd);
	getint(sfd,&temp);
	gv->parts[i].is_extender = temp;
	while ( (ch=nlgetc(sfd))==' ' );
	if ( ch!=',' ) ungetc(ch,sfd);
	getint(sfd,&temp);
	gv->parts[i].startConnectorLength=temp;
	while ( (ch=nlgetc(sfd))==' ' );
	if ( ch!=',' ) ungetc(ch,sfd);
	getint(sfd,&temp);
	gv->parts[i].endConnectorLength = temp;
	while ( (ch=nlgetc(sfd))==' ' );
	if ( ch!=',' ) ungetc(ch,sfd);
	getint(sfd,&temp);
	gv->parts[i].fullAdvance = temp;
    }
return( gv );
}

static void SFDParseVertexKern(FILE *sfd, struct mathkernvertex *vertex) {
    int i,ch;

    getint(sfd,&vertex->cnt);
    vertex->mkd = calloc(vertex->cnt,sizeof(struct mathkerndata));
    for ( i=0; i<vertex->cnt; ++i ) {
	SFDParseMathValueRecord(sfd,&vertex->mkd[i].height,&vertex->mkd[i].height_adjusts);
	while ( (ch=nlgetc(sfd))==' ' );
	if ( ch!=EOF && ch!=',' )
	    ungetc(ch,sfd);
	SFDParseMathValueRecord(sfd,&vertex->mkd[i].kern,&vertex->mkd[i].kern_adjusts);
    }
}

static struct gradient *SFDParseGradient(FILE *sfd,char *tok) {
    struct gradient *grad = chunkalloc(sizeof(struct gradient));
    int ch, i;

    getreal(sfd,&grad->start.x);
    while ( isspace(ch=nlgetc(sfd)));
    if ( ch!=';' ) ungetc(ch,sfd);
    getreal(sfd,&grad->start.y);

    getreal(sfd,&grad->stop.x);
    while ( isspace(ch=nlgetc(sfd)));
    if ( ch!=';' ) ungetc(ch,sfd);
    getreal(sfd,&grad->stop.y);

    getreal(sfd,&grad->radius);

    getname(sfd,tok);
    for ( i=0; spreads[i]!=NULL; ++i )
	if ( strmatch(spreads[i],tok)==0 )
    break;
    if ( spreads[i]==NULL ) i=0;
    grad->sm = i;

    getint(sfd,&grad->stop_cnt);
    grad->grad_stops = calloc(grad->stop_cnt,sizeof(struct grad_stops));
    for ( i=0; i<grad->stop_cnt; ++i ) {
	while ( isspace(ch=nlgetc(sfd)));
	if ( ch!='{' ) ungetc(ch,sfd);
	getreal( sfd, &grad->grad_stops[i].offset );
	gethex( sfd, &grad->grad_stops[i].col );
	getreal( sfd, &grad->grad_stops[i].opacity );
	while ( isspace(ch=nlgetc(sfd)));
	if ( ch!='}' ) ungetc(ch,sfd);
    }
return( grad );
}

static struct pattern *SFDParsePattern(FILE *sfd,char *tok) {
    struct pattern *pat = chunkalloc(sizeof(struct pattern));
    int ch;

    getname(sfd,tok);
    pat->pattern = copy(tok);

    getreal(sfd,&pat->width);
    while ( isspace(ch=nlgetc(sfd)));
    if ( ch!=';' ) ungetc(ch,sfd);
    getreal(sfd,&pat->height);

    while ( isspace(ch=nlgetc(sfd)));
    if ( ch!='[' ) ungetc(ch,sfd);
    getreal(sfd,&pat->transform[0]);
    getreal(sfd,&pat->transform[1]);
    getreal(sfd,&pat->transform[2]);
    getreal(sfd,&pat->transform[3]);
    getreal(sfd,&pat->transform[4]);
    getreal(sfd,&pat->transform[5]);
    while ( isspace(ch=nlgetc(sfd)));
    if ( ch!=']' ) ungetc(ch,sfd);
return( pat );
}


static int orig_pos;

static SplineChar *SFDGetChar(FILE *sfd,SplineFont *sf, int had_sf_layer_cnt) {
    SplineChar *sc;
    char tok[2000], ch;
    RefChar *lastr=NULL, *ref;
    AnchorPoint *lastap = NULL;
    int isliga = 0, ispos, issubs=0, ismult=0, islcar=0, ispair=0, temp, i;
    PST *last = NULL;
    uint32 script = 0;
    int current_layer = ly_fore;
    int had_old_dstems = false;
    SplineFont *sli_sf = sf->cidmaster ? sf->cidmaster : sf;
    struct altuni *altuni;
    int oldback = false;

    if ( getname(sfd,tok)!=1 )
return( NULL );
    if ( strcmp(tok,"StartChar:")!=0 )
return( NULL );
    while ( isspace(ch=nlgetc(sfd)));
    ungetc(ch,sfd);
    sc = SFSplineCharCreate(sf);
    if ( ch!='"' ) {
	if ( getname(sfd,tok)!=1 ) {
	    SplineCharFree(sc);
return( NULL );
	}
	sc->name = copy(tok);
    } else {
	sc->name = SFDReadUTF7Str(sfd);
	if ( sc->name==NULL ) {
	    SplineCharFree(sc);
return( NULL );
	}
    }
    sc->vwidth = sf->ascent+sf->descent;
    sc->parent = sf;
    while ( 1 ) {
	if ( getname(sfd,tok)!=1 ) {
	    SplineCharFree(sc);
return( NULL );
	}
	if ( strmatch(tok,"Encoding:")==0 ) {
	    int enc;
	    getint(sfd,&enc);
	    getint(sfd,&sc->unicodeenc);
	    while ( (ch=nlgetc(sfd))==' ' || ch=='\t' );
	    ungetc(ch,sfd);
	    if ( ch!='\n' && ch!='\r' ) {
		getint(sfd,&sc->orig_pos);
		if ( sc->orig_pos==65535 )
		    sc->orig_pos = orig_pos++;
		    /* An old mark meaning: "I don't know" */
		if ( sc->orig_pos<sf->glyphcnt && sf->glyphs[sc->orig_pos]!=NULL )
		    sc->orig_pos = sf->glyphcnt;
		if ( sc->orig_pos>=sf->glyphcnt ) {
		    if ( sc->orig_pos>=sf->glyphmax )
			sf->glyphs = realloc(sf->glyphs,(sf->glyphmax = sc->orig_pos+10)*sizeof(SplineChar *));
		    memset(sf->glyphs+sf->glyphcnt,0,(sc->orig_pos+1-sf->glyphcnt)*sizeof(SplineChar *));
		    sf->glyphcnt = sc->orig_pos+1;
		}
		if ( sc->orig_pos+1 > orig_pos )
		    orig_pos = sc->orig_pos+1;
	    } else if ( sf->cidmaster!=NULL ) {		/* In cid fonts the orig_pos is just the cid */
		sc->orig_pos = enc;
	    } else {
		sc->orig_pos = orig_pos++;
	    }
	    SFDSetEncMap(sf,sc->orig_pos,enc);
	} else if ( strmatch(tok,"AltUni:")==0 ) {
	    int uni;
	    while ( getint(sfd,&uni)==1 ) {
		altuni = chunkalloc(sizeof(struct altuni));
		altuni->unienc = uni;
		altuni->vs = -1;
		altuni->fid = 0;
		altuni->next = sc->altuni;
		sc->altuni = altuni;
	    }
	} else if ( strmatch(tok,"AltUni2:")==0 ) {
	    uint32 uni[3];
	    while ( gethexints(sfd,uni,3) ) {
		altuni = chunkalloc(sizeof(struct altuni));
		altuni->unienc = uni[0];
		altuni->vs = uni[1];
		altuni->fid = uni[2];
		altuni->next = sc->altuni;
		sc->altuni = altuni;
	    }
	} else if ( strmatch(tok,"OldEncoding:")==0 ) {
	    int old_enc;		/* Obsolete info */
	    getint(sfd,&old_enc);
        } else if ( strmatch(tok,"Script:")==0 ) {
	    /* Obsolete. But still used for parsing obsolete ligature/subs tags */
            while ( (ch=nlgetc(sfd))==' ' || ch=='\t' );
            if ( ch=='\n' || ch=='\r' )
                script = 0;
            else {
		ungetc(ch,sfd);
		script = gettag(sfd);
            }
	} else if ( strmatch(tok,"GlifName:")==0 ) {
            while ( isspace(ch=nlgetc(sfd)));
            ungetc(ch,sfd);
            if ( ch!='"' ) {
              if ( getname(sfd,tok)!=1 ) {
                LogError(_("Invalid glif name.\n"));
              }
	      sc->glif_name = copy(tok);
            } else {
	      sc->glif_name = SFDReadUTF7Str(sfd);
	      if ( sc->glif_name==NULL ) {
                LogError(_("Invalid glif name.\n"));
	      }
            }
	} else if ( strmatch(tok,"Width:")==0 ) {
	    getsint(sfd,&sc->width);
	} else if ( strmatch(tok,"VWidth:")==0 ) {
	    getsint(sfd,&sc->vwidth);
	} else if ( strmatch(tok,"GlyphClass:")==0 ) {
	    getint(sfd,&temp);
	    sc->glyph_class = temp;
	} else if ( strmatch(tok,"UnlinkRmOvrlpSave:")==0 ) {
	    getint(sfd,&temp);
	    sc->unlink_rm_ovrlp_save_undo = temp;
	} else if ( strmatch(tok,"InSpiro:")==0 ) {
	    getint(sfd,&temp);
	    sc->inspiro = temp;
	} else if ( strmatch(tok,"LigCaretCntFixed:")==0 ) {
	    getint(sfd,&temp);
	    sc->lig_caret_cnt_fixed = temp;
	} else if ( strmatch(tok,"Flags:")==0 ) {
	    while ( isspace(ch=nlgetc(sfd)) && ch!='\n' && ch!='\r');
	    while ( ch!='\n' && ch!='\r' ) {
		if ( ch=='H' ) sc->changedsincelasthinted=true;
		else if ( ch=='M' ) sc->manualhints = true;
		else if ( ch=='W' ) sc->widthset = true;
		else if ( ch=='O' ) sc->wasopen = true;
		else if ( ch=='I' ) sc->instructions_out_of_date = true;
		ch = nlgetc(sfd);
	    }
	    if ( sf->multilayer || sf->strokedfont || sc->layers[ly_fore].order2 )
		sc->changedsincelasthinted = false;
	} else if ( strmatch(tok,"TeX:")==0 ) {
	    getsint(sfd,&sc->tex_height);
	    getsint(sfd,&sc->tex_depth);
	    while ( isspace(ch=nlgetc(sfd)) && ch!='\n' && ch!='\r');
	    ungetc(ch,sfd);
	    if ( ch!='\n' && ch!='\r' ) {
		int16 old_tex;
		/* Used to store two extra values here */
		getsint(sfd,&old_tex);
		getsint(sfd,&old_tex);
		if ( sc->tex_height==0 && sc->tex_depth==0 )		/* Fixup old bug */
		    sc->tex_height = sc->tex_depth = TEX_UNDEF;
	    }
	} else if ( strmatch(tok,"ItalicCorrection:")==0 ) {
	    SFDParseMathValueRecord(sfd,&sc->italic_correction,&sc->italic_adjusts);
	} else if ( strmatch(tok,"TopAccentHorizontal:")==0 ) {
	    SFDParseMathValueRecord(sfd,&sc->top_accent_horiz,&sc->top_accent_adjusts);
	} else if ( strmatch(tok,"GlyphCompositionVerticalIC:")==0 ) {
	    if ( sc->vert_variants==NULL )
		sc->vert_variants = chunkalloc(sizeof(struct glyphvariants));
	    SFDParseMathValueRecord(sfd,&sc->vert_variants->italic_correction,&sc->vert_variants->italic_adjusts);
	} else if ( strmatch(tok,"GlyphCompositionHorizontalIC:")==0 ) {
	    if ( sc->horiz_variants==NULL )
		sc->horiz_variants = chunkalloc(sizeof(struct glyphvariants));
	    SFDParseMathValueRecord(sfd,&sc->horiz_variants->italic_correction,&sc->horiz_variants->italic_adjusts);
	} else if ( strmatch(tok,"IsExtendedShape:")==0 ) {
	    int temp;
	    getint(sfd,&temp);
	    sc->is_extended_shape = temp;
	} else if ( strmatch(tok,"GlyphVariantsVertical:")==0 ) {
	    if ( sc->vert_variants==NULL )
		sc->vert_variants = chunkalloc(sizeof(struct glyphvariants));
	    geteol(sfd,tok);
	    sc->vert_variants->variants = copy(tok);
	} else if ( strmatch(tok,"GlyphVariantsHorizontal:")==0 ) {
	    if ( sc->horiz_variants==NULL )
		sc->horiz_variants = chunkalloc(sizeof(struct glyphvariants));
	    geteol(sfd,tok);
	    sc->horiz_variants->variants = copy(tok);
	} else if ( strmatch(tok,"GlyphCompositionVertical:")==0 ) {
	    sc->vert_variants = SFDParseGlyphComposition(sfd, sc->vert_variants,tok);
	} else if ( strmatch(tok,"GlyphCompositionHorizontal:")==0 ) {
	    sc->horiz_variants = SFDParseGlyphComposition(sfd, sc->horiz_variants,tok);
	} else if ( strmatch(tok,"TopRightVertex:")==0 ) {
	    if ( sc->mathkern==NULL )
		sc->mathkern = chunkalloc(sizeof(struct mathkern));
	    SFDParseVertexKern(sfd, &sc->mathkern->top_right);
	} else if ( strmatch(tok,"TopLeftVertex:")==0 ) {
	    if ( sc->mathkern==NULL )
		sc->mathkern = chunkalloc(sizeof(struct mathkern));
	    SFDParseVertexKern(sfd, &sc->mathkern->top_left);
	} else if ( strmatch(tok,"BottomRightVertex:")==0 ) {
	    if ( sc->mathkern==NULL )
		sc->mathkern = chunkalloc(sizeof(struct mathkern));
	    SFDParseVertexKern(sfd, &sc->mathkern->bottom_right);
	} else if ( strmatch(tok,"BottomLeftVertex:")==0 ) {
	    if ( sc->mathkern==NULL )
		sc->mathkern = chunkalloc(sizeof(struct mathkern));
	    SFDParseVertexKern(sfd, &sc->mathkern->bottom_left);
#if HANYANG
	} else if ( strmatch(tok,"CompositionUnit:")==0 ) {
	    getsint(sfd,&sc->jamo);
	    getsint(sfd,&sc->varient);
	    sc->compositionunit = true;
#endif
	} else if ( strmatch(tok,"HStem:")==0 ) {
	    sc->hstem = SFDReadHints(sfd);
	    sc->hconflicts = StemListAnyConflicts(sc->hstem);
	} else if ( strmatch(tok,"VStem:")==0 ) {
	    sc->vstem = SFDReadHints(sfd);
	    sc->vconflicts = StemListAnyConflicts(sc->vstem);
	} else if ( strmatch(tok,"DStem:")==0 ) {
	    sc->dstem = SFDReadDHints( sc->parent,sfd,true );
            had_old_dstems = true;
	} else if ( strmatch(tok,"DStem2:")==0 ) {
	    sc->dstem = SFDReadDHints( sc->parent,sfd,false );
	} else if ( strmatch(tok,"CounterMasks:")==0 ) {
	    getsint(sfd,&sc->countermask_cnt);
	    sc->countermasks = calloc(sc->countermask_cnt,sizeof(HintMask));
	    for ( i=0; i<sc->countermask_cnt; ++i ) {
		int ch;
		while ( (ch=nlgetc(sfd))==' ' );
		ungetc(ch,sfd);
		SFDGetHintMask(sfd,&sc->countermasks[i]);
	    }
	} else if ( strmatch(tok,"AnchorPoint:")==0 ) {
	    lastap = SFDReadAnchorPoints(sfd,sc,&sc->anchor,lastap);
	} else if ( strmatch(tok,"Fore")==0 ) {
	    while ( isspace(ch = nlgetc(sfd)));
	    ungetc(ch,sfd);
	    if ( ch!='I' && ch!='R' && ch!='S' && ch!='V' && ch!=' ' && ch!='\n' &&
	         !PeekMatch(sfd, "Pickled") && !PeekMatch(sfd, "EndChar") &&
	         !PeekMatch(sfd, "Fore") && !PeekMatch(sfd, "Back") && !PeekMatch(sfd, "Layer") ) {
		/* Old format, without a SplineSet token */
		sc->layers[ly_fore].splines = SFDGetSplineSet(sfd,sc->layers[ly_fore].order2);
	    }
	    current_layer = ly_fore;
	} else if ( strmatch(tok,"MinimumDistance:")==0 ) {
	    SFDGetMinimumDistances(sfd,sc);
	} else if ( strmatch(tok,"Validated:")==0 ) {
	    getsint(sfd,(int16 *) &sc->layers[current_layer].validation_state);
	} else if ( strmatch(tok,"Back")==0 ) {
	    while ( isspace(ch=nlgetc(sfd)));
	    ungetc(ch,sfd);
	    if ( ch!='I' && ch!='R' && ch!='S' && ch!='V' && ch!=' ' && ch!='\n' &&
	         !PeekMatch(sfd, "Pickled") && !PeekMatch(sfd, "EndChar") &&
	         !PeekMatch(sfd, "Fore") && !PeekMatch(sfd, "Back") && !PeekMatch(sfd, "Layer") ) {
		/* Old format, without a SplineSet token */
		sc->layers[ly_back].splines = SFDGetSplineSet(sfd,sc->layers[ly_back].order2);
		oldback = true;
	    }
	    current_layer = ly_back;
	} else if ( strmatch(tok,"LayerCount:")==0 ) {
	    getint(sfd,&temp);
	    if ( temp>sc->layer_cnt ) {
		sc->layers = realloc(sc->layers,temp*sizeof(Layer));
		memset(sc->layers+sc->layer_cnt,0,(temp-sc->layer_cnt)*sizeof(Layer));
	    }
	    sc->layer_cnt = temp;
	    current_layer = ly_fore;
	} else if ( strmatch(tok,"Layer:")==0 ) {
	    int layer;
	    int dofill, dostroke, fillfirst, linejoin, linecap;
	    uint32 fillcol, strokecol;
	    real fillopacity, strokeopacity, strokewidth, trans[4];
	    DashType dashes[DASH_MAX];
	    int i;
	    getint(sfd,&layer);
	    if ( layer>=sc->layer_cnt ) {
		sc->layers = realloc(sc->layers,(layer+1)*sizeof(Layer));
		memset(sc->layers+sc->layer_cnt,0,(layer+1-sc->layer_cnt)*sizeof(Layer));
	    }
	    if ( sc->parent->multilayer ) {
		getint(sfd,&dofill);
		getint(sfd,&dostroke);
		getint(sfd,&fillfirst);
		gethex(sfd,&fillcol);
		getreal(sfd,&fillopacity);
		gethex(sfd,&strokecol);
		getreal(sfd,&strokeopacity);
		getreal(sfd,&strokewidth);
		getname(sfd,tok);
		for ( i=0; joins[i]!=NULL; ++i )
		    if ( strmatch(joins[i],tok)==0 )
		break;
		if ( joins[i]==NULL ) --i;
		linejoin = i;
		getname(sfd,tok);
		for ( i=0; caps[i]!=NULL; ++i )
		    if ( strmatch(caps[i],tok)==0 )
		break;
		if ( caps[i]==NULL ) --i;
		linecap = i;
		while ( (ch=nlgetc(sfd))==' ' || ch=='[' );
		ungetc(ch,sfd);
		getreal(sfd,&trans[0]);
		getreal(sfd,&trans[1]);
		getreal(sfd,&trans[2]);
		getreal(sfd,&trans[3]);
		while ( (ch=nlgetc(sfd))==' ' || ch==']' );
		if ( ch=='[' ) {
		    for ( i=0;; ++i ) { int temp;
			if ( !getint(sfd,&temp) )
		    break;
			else if ( i<DASH_MAX )
			    dashes[i] = temp;
		    }
		    if ( i<DASH_MAX )
			dashes[i] = 0;
		} else {
		    ungetc(ch,sfd);
		    memset(dashes,0,sizeof(dashes));
		}
		sc->layers[layer].dofill = dofill;
		sc->layers[layer].dostroke = dostroke;
		sc->layers[layer].fillfirst = fillfirst;
		sc->layers[layer].fill_brush.col = fillcol;
		sc->layers[layer].fill_brush.opacity = fillopacity;
		sc->layers[layer].stroke_pen.brush.col = strokecol;
		sc->layers[layer].stroke_pen.brush.opacity = strokeopacity;
		sc->layers[layer].stroke_pen.width = strokewidth;
		sc->layers[layer].stroke_pen.linejoin = linejoin;
		sc->layers[layer].stroke_pen.linecap = linecap;
		memcpy(sc->layers[layer].stroke_pen.dashes,dashes,sizeof(dashes));
		memcpy(sc->layers[layer].stroke_pen.trans,trans,sizeof(trans));
	    }
	    current_layer = layer;
	    lastr = NULL;
	} else if ( strmatch(tok,"FillGradient:")==0 ) {
	    sc->layers[current_layer].fill_brush.gradient = SFDParseGradient(sfd,tok);
	} else if ( strmatch(tok,"FillPattern:")==0 ) {
	    sc->layers[current_layer].fill_brush.pattern = SFDParsePattern(sfd,tok);
	} else if ( strmatch(tok,"StrokeGradient:")==0 ) {
	    sc->layers[current_layer].stroke_pen.brush.gradient = SFDParseGradient(sfd,tok);
	} else if ( strmatch(tok,"StrokePattern:")==0 ) {
	    sc->layers[current_layer].stroke_pen.brush.pattern = SFDParsePattern(sfd,tok);
	} else if ( strmatch(tok,"SplineSet")==0 ) {
	    sc->layers[current_layer].splines = SFDGetSplineSet(sfd,sc->layers[current_layer].order2);
	} else if ( strmatch(tok,"Ref:")==0 || strmatch(tok,"Refer:")==0 ) {
	    /* I should be depending on the version number here, but I made */
	    /*  a mistake and bumped the version too late. So the version is */
	    /*  not an accurate mark, but the presence of a LayerCount keyword*/
	    /*  in the font is an good mark. Before the LayerCount was added */
	    /*  (version 2) only the foreground layer could have references */
	    /*  after that (eventually version 3) any layer could. */
	    if ( oldback || !had_sf_layer_cnt ) current_layer = ly_fore;
	    ref = SFDGetRef(sfd,strmatch(tok,"Ref:")==0);
	    if ( sc->layers[current_layer].refs==NULL )
		sc->layers[current_layer].refs = ref;
	    else
		lastr->next = ref;
	    lastr = ref;
	} else if ( strmatch(tok,"PickledData:")==0 ) {
	    if (current_layer < sc->layer_cnt) {
	      sc->layers[current_layer].python_persistent = SFDUnPickle(sfd, 0);
	      sc->layers[current_layer].python_persistent_has_lists = 0;
	    }
	} else if ( strmatch(tok,"PickledDataWithLists:")==0 ) {
	    if (current_layer < sc->layer_cnt) {
	      sc->layers[current_layer].python_persistent = SFDUnPickle(sfd, 1);
	      sc->layers[current_layer].python_persistent_has_lists = 1;
	    }
	} else if ( strmatch(tok,"OrigType1:")==0 ) {	/* Accept, slurp, ignore contents */
	    SFDGetType1(sfd);
	} else if ( strmatch(tok,"TtfInstrs:")==0 ) {	/* Binary format */
	    SFDGetTtfInstrs(sfd,sc);
	} else if ( strmatch(tok,"TtInstrs:")==0 ) {	/* ASCII format */
	    SFDGetTtInstrs(sfd,sc);
	} else if ( strmatch(tok,"Kerns2:")==0 ||
		strmatch(tok,"VKerns2:")==0 ) {
	    KernPair *kp, *last=NULL;
	    int isv = *tok=='V';
	    int off, index;
	    struct lookup_subtable *sub;

	    while ( fscanf(sfd,"%d %d", &index, &off )==2 ) {
		sub = SFFindLookupSubtableAndFreeName(sf,SFDReadUTF7Str(sfd));
		if ( sub==NULL ) {
		    LogError(_("KernPair with no subtable name.\n"));
	    break;
		}
		kp = chunkalloc(sizeof(KernPair1));
		kp->sc = (SplineChar *) (intpt) index;
		kp->kcid = true;
		kp->off = off;
		kp->subtable = sub;
		kp->next = NULL;
		while ( (ch=nlgetc(sfd))==' ' );
		ungetc(ch,sfd);
		if ( ch=='{' ) {
		    kp->adjust = SFDReadDeviceTable(sfd, NULL);
		}
		if ( last != NULL )
		    last->next = kp;
		else if ( isv )
		    sc->vkerns = kp;
		else
		    sc->kerns = kp;
		last = kp;
	    }
	} else if ( (ispos = (strmatch(tok,"Position:")==0)) ||
		( ispos  = (strmatch(tok,"Position2:")==0)) ||
		( ispair = (strmatch(tok,"PairPos:")==0)) ||
		( ispair = (strmatch(tok,"PairPos2:")==0)) ||
		( islcar = (strmatch(tok,"LCarets:")==0)) ||
		( islcar = (strmatch(tok,"LCarets2:")==0)) ||
		( isliga = (strmatch(tok,"Ligature:")==0)) ||
		( isliga = (strmatch(tok,"Ligature2:")==0)) ||
		( issubs = (strmatch(tok,"Substitution:")==0)) ||
		( issubs = (strmatch(tok,"Substitution2:")==0)) ||
		( ismult = (strmatch(tok,"MultipleSubs:")==0)) ||
		( ismult = (strmatch(tok,"MultipleSubs2:")==0)) ||
		strmatch(tok,"AlternateSubs:")==0 ||
		strmatch(tok,"AlternateSubs2:")==0 ) {
	    PST *pst;
	    int old, type;
	    type = ispos ? pst_position :
			 ispair ? pst_pair :
			 islcar ? pst_lcaret :
			 isliga ? pst_ligature :
			 issubs ? pst_substitution :
			 ismult ? pst_multiple :
			 pst_alternate;
	    if ( strchr(tok,'2')!=NULL ) {
		old = false;
		pst = chunkalloc(sizeof(PST));
		if ( type!=pst_lcaret )
		    pst->subtable = SFFindLookupSubtableAndFreeName(sf,SFDReadUTF7Str(sfd));
	    } else {
		old = true;
		pst = chunkalloc(sizeof(PST1));
		((PST1 *) pst)->tag = CHR('l','i','g','a');
		((PST1 *) pst)->script_lang_index = 0xffff;
		while ( (ch=nlgetc(sfd))==' ' || ch=='\t' );
		if ( isdigit(ch)) {
		    int temp;
		    ungetc(ch,sfd);
		    getint(sfd,&temp);
		    ((PST1 *) pst)->flags = temp;
		    while ( (ch=nlgetc(sfd))==' ' || ch=='\t' );
		} else
		    ((PST1 *) pst)->flags = 0 /*PSTDefaultFlags(type,sc)*/;
		if ( isdigit(ch)) {
		    ungetc(ch,sfd);
		    getusint(sfd,&((PST1 *) pst)->script_lang_index);
		    while ( (ch=nlgetc(sfd))==' ' || ch=='\t' );
		} else
		    ((PST1 *) pst)->script_lang_index = SFFindBiggestScriptLangIndex(sf,
			    script!=0?script:SCScriptFromUnicode(sc),DEFAULT_LANG);
		if ( ch=='\'' ) {
		    ungetc(ch,sfd);
		    ((PST1 *) pst)->tag = gettag(sfd);
		} else if ( ch=='<' ) {
		    getint(sfd,&temp);
		    ((PST1 *) pst)->tag = temp<<16;
		    nlgetc(sfd);	/* comma */
		    getint(sfd,&temp);
		    ((PST1 *) pst)->tag |= temp;
		    nlgetc(sfd);	/* close '>' */
		    ((PST1 *) pst)->macfeature = true;
		} else
		    ungetc(ch,sfd);
		if ( type==pst_lcaret ) {
		/* These are meaningless for lcarets, set them to innocuous values */
		    ((PST1 *) pst)->script_lang_index = SLI_UNKNOWN;
		    ((PST1 *) pst)->tag = CHR(' ',' ',' ',' ');
		} else if ( ((PST1 *) pst)->script_lang_index>=((SplineFont1 *) sli_sf)->sli_cnt && ((PST1 *) pst)->script_lang_index!=SLI_NESTED ) {
		    static int complained=false;
		    if ( !complained )
			IError("'%c%c%c%c' in %s has a script index out of bounds: %d",
				(((PST1 *) pst)->tag>>24), (((PST1 *) pst)->tag>>16)&0xff, (((PST1 *) pst)->tag>>8)&0xff, ((PST1 *) pst)->tag&0xff,
				sc->name, ((PST1 *) pst)->script_lang_index );
		    else
			IError( "'%c%c%c%c' in %s has a script index out of bounds: %d\n",
				(((PST1 *) pst)->tag>>24), (((PST1 *) pst)->tag>>16)&0xff, (((PST1 *) pst)->tag>>8)&0xff, ((PST1 *) pst)->tag&0xff,
				sc->name, ((PST1 *) pst)->script_lang_index );
		    ((PST1 *) pst)->script_lang_index = SFFindBiggestScriptLangIndex(sli_sf,
			    SCScriptFromUnicode(sc),DEFAULT_LANG);
		    complained = true;
		}
	    }
	    if ( (sf->sfd_version<2)!=old ) {
		IError( "Version mixup in PST of sfd file." );
exit(1);
	    }
	    if ( last==NULL )
		sc->possub = pst;
	    else
		last->next = pst;
	    last = pst;
	    pst->type = type;
	    if ( pst->type==pst_position ) {
		fscanf( sfd, " dx=%hd dy=%hd dh=%hd dv=%hd",
			&pst->u.pos.xoff, &pst->u.pos.yoff,
			&pst->u.pos.h_adv_off, &pst->u.pos.v_adv_off);
		pst->u.pos.adjust = SFDReadValDevTab(sfd);
		ch = nlgetc(sfd);		/* Eat new line */
	    } else if ( pst->type==pst_pair ) {
		getname(sfd,tok);
		pst->u.pair.paired = copy(tok);
		pst->u.pair.vr = chunkalloc(sizeof(struct vr [2]));
		fscanf( sfd, " dx=%hd dy=%hd dh=%hd dv=%hd",
			&pst->u.pair.vr[0].xoff, &pst->u.pair.vr[0].yoff,
			&pst->u.pair.vr[0].h_adv_off, &pst->u.pair.vr[0].v_adv_off);
		pst->u.pair.vr[0].adjust = SFDReadValDevTab(sfd);
		fscanf( sfd, " dx=%hd dy=%hd dh=%hd dv=%hd",
			&pst->u.pair.vr[1].xoff, &pst->u.pair.vr[1].yoff,
			&pst->u.pair.vr[1].h_adv_off, &pst->u.pair.vr[1].v_adv_off);
		pst->u.pair.vr[0].adjust = SFDReadValDevTab(sfd);
		ch = nlgetc(sfd);
	    } else if ( pst->type==pst_lcaret ) {
		int i;
		fscanf( sfd, " %d", &pst->u.lcaret.cnt );
		pst->u.lcaret.carets = malloc(pst->u.lcaret.cnt*sizeof(int16));
		for ( i=0; i<pst->u.lcaret.cnt; ++i )
		    fscanf( sfd, " %hd", &pst->u.lcaret.carets[i]);
		geteol(sfd,tok);
	    } else {
		geteol(sfd,tok);
		pst->u.lig.components = copy(tok);	/* it's in the same place for all formats */
		if ( isliga ) {
		    pst->u.lig.lig = sc;
		    if ( old )
			last = (PST *) LigaCreateFromOldStyleMultiple((PST1 *) pst);
		}
	    }
#ifdef FONTFORGE_CONFIG_CVT_OLD_MAC_FEATURES
	    if ( old )
		CvtOldMacFeature((PST1 *) pst);
#endif
	} else if ( strmatch(tok,"Colour:")==0 ) {
	    uint32 temp;
	    gethex(sfd,&temp);
	    sc->color = temp;
	} else if ( strmatch(tok,"Comment:")==0 ) {
	    sc->comment = SFDReadUTF7Str(sfd);
	} else if ( strmatch(tok,"TileMargin:")==0 ) {
	    getreal(sfd,&sc->tile_margin);
	} else if ( strmatch(tok,"TileBounds:")==0 ) {
	    getreal(sfd,&sc->tile_bounds.minx);
	    getreal(sfd,&sc->tile_bounds.miny);
	    getreal(sfd,&sc->tile_bounds.maxx);
	    getreal(sfd,&sc->tile_bounds.maxy);
	} else if ( strmatch(tok,"EndChar")==0 ) {
	    if ( sc->orig_pos<sf->glyphcnt )
		sf->glyphs[sc->orig_pos] = sc;
            /* Recalculating hint active zones may be needed for old .sfd files. */
            /* Do this when we have finished with other glyph components, */
            /* so that splines are already available */
	    if ( had_old_dstems && sc->layers[ly_fore].splines != NULL )
                SCGuessHintInstancesList( sc,ly_fore,NULL,NULL,sc->dstem,false,true );
	    if ( sc->layers[ly_fore].order2 )
		SCDefaultInterpolation(sc);
return( sc );
	} else {
	    geteol(sfd,tok);
	}
    }
}

static void SFDFixupRef(SplineChar *sc,RefChar *ref,int layer) {
    RefChar *rf;
    int ly;

    if ( sc->parent->multilayer ) {
	for ( ly=ly_fore; ly<ref->sc->layer_cnt; ++ly ) {
	    for ( rf = ref->sc->layers[ly].refs; rf!=NULL; rf=rf->next ) {
		if ( rf->sc==sc ) {	/* Huh? */
		    ref->sc->layers[ly].refs = NULL;
	    break;
		}
		if ( rf->layers[0].splines==NULL )
		    SFDFixupRef(ref->sc,rf,layer);
	    }
	}
    } else {
	for ( rf = ref->sc->layers[layer].refs; rf!=NULL; rf=rf->next ) {
	    if ( rf->sc==sc ) {	/* Huh? */
		ref->sc->layers[layer].refs = NULL;
	break;
	    }
	    if ( rf->layers[0].splines==NULL )
		SFDFixupRef(ref->sc,rf,layer);
	}
    }
    SCReinstanciateRefChar(sc,ref,layer);
    SCMakeDependent(sc,ref->sc);
}

/* Look for character duplicates, such as might be generated by having the same */
/*  glyph at two encoding slots */
/* This is an obsolete convention, supported now only in sfd files */
/* I think it is ok if something depends on this character, because the */
/*  code that handles references will automatically unwrap it down to be base */
static SplineChar *SCDuplicate(SplineChar *sc) {
    SplineChar *matched = sc;

    if ( sc==NULL || sc->parent==NULL || sc->parent->cidmaster!=NULL )
return( sc );		/* Can't do this in CID keyed fonts */

    if ( sc->layer_cnt!=2 )
return( sc );

    while ( sc->layers[ly_fore].refs!=NULL &&
	    sc->layers[ly_fore].refs->sc!=NULL &&	/* Can happen if we are called during font loading before references are fixed up */
	    sc->layers[ly_fore].refs->next==NULL &&
	    sc->layers[ly_fore].refs->transform[0]==1 && sc->layers[ly_fore].refs->transform[1]==0 &&
	    sc->layers[ly_fore].refs->transform[2]==0 && sc->layers[ly_fore].refs->transform[3]==1 &&
	    sc->layers[ly_fore].refs->transform[4]==0 && sc->layers[ly_fore].refs->transform[5]==0 ) {
	char *basename = sc->layers[ly_fore].refs->sc->name;
	if ( strcmp(sc->name,basename)!=0 )
    break;
	matched = sc->layers[ly_fore].refs->sc;
	sc = sc->layers[ly_fore].refs->sc;
    }
return( matched );
}


void SFDFixupRefs(SplineFont *sf) {
    int i, isv;
    RefChar *refs, *rnext, *rprev;
    /*int isautorecovery = sf->changed;*/
    KernPair *kp, *prev, *next;
    EncMap *map = sf->map;
    int layer;
    int k,l;
    SplineFont *cidmaster = sf, *ksf;

    k = 1;
    if ( sf->subfontcnt!=0 )
	sf = sf->subfonts[0];

    ff_progress_change_line2(_("Interpreting Glyphs"));
    for (;;) {
	for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL ) {
	    SplineChar *sc = sf->glyphs[i];
	    /* A changed character is one that has just been recovered */
	    /*  unchanged characters will already have been fixed up */
	    /* Er... maybe not. If the character being recovered is refered to */
	    /*  by another character then we need to fix up that other char too*/
	    /*if ( isautorecovery && !sc->changed )*/
	/*continue;*/
	    for ( layer = 0; layer<sc->layer_cnt; ++layer ) {
		rprev = NULL;
		for ( refs = sc->layers[layer].refs; refs!=NULL; refs=rnext ) {
		    rnext = refs->next;
		    if ( refs->encoded ) {		/* Old sfd format */
			if ( refs->orig_pos<map->encmax && map->map[refs->orig_pos]!=-1 )
			    refs->orig_pos = map->map[refs->orig_pos];
			else
			    refs->orig_pos = sf->glyphcnt;
			refs->encoded = false;
		    }
		    if ( refs->orig_pos<sf->glyphcnt && refs->orig_pos>=0 )
			refs->sc = sf->glyphs[refs->orig_pos];
		    if ( refs->sc!=NULL ) {
			refs->unicode_enc = refs->sc->unicodeenc;
			refs->adobe_enc = getAdobeEnc(refs->sc->name);
			rprev = refs;
			if ( refs->use_my_metrics ) {
			    if ( sc->width != refs->sc->width ) {
				LogError(_("Bad sfd file. Glyph %s has width %d even though it should be\n  bound to the width of %s which is %d.\n"),
					sc->name, sc->width, refs->sc->name, refs->sc->width );
				sc->width = refs->sc->width;
			    }
			}
		    } else {
			RefCharFree(refs);
			if ( rprev!=NULL )
			    rprev->next = rnext;
			else
			    sc->layers[layer].refs = rnext;
		    }
		}
	    }
	    /* In old sfd files we used a peculiar idiom to represent a multiply */
	    /*  encoded glyph. Fix it up now. Remove the fake glyph and adjust the*/
	    /*  map */
	    /*if ( isautorecovery && !sc->changed )*/
	/*continue;*/
	    for ( isv=0; isv<2; ++isv ) {
		for ( prev = NULL, kp=isv?sc->vkerns : sc->kerns; kp!=NULL; kp=next ) {
		    int index = (intpt) (kp->sc);

		    next = kp->next;
		    // be impotent if the reference is already to the correct location
                    if ( !kp->kcid ) {	/* It's encoded (old sfds), else orig */
                        if ( index>=map->encmax || map->map[index]==-1 )
                            index = sf->glyphcnt;
                        else
                            index = map->map[index];
                    }
                    kp->kcid = false;
                    ksf = sf;
                    if ( cidmaster!=sf ) {
                        for ( l=0; l<cidmaster->subfontcnt; ++l ) {
                            ksf = cidmaster->subfonts[l];
                            if ( index<ksf->glyphcnt && ksf->glyphs[index]!=NULL )
                                break;
                        }
                    }
                    if ( index>=ksf->glyphcnt || ksf->glyphs[index]==NULL ) {
                        IError( "Bad kerning information in glyph %s\n", sc->name );
                        kp->sc = NULL;
                    } else {
                        kp->sc = ksf->glyphs[index];
                    }

		    if ( kp->sc!=NULL )
			prev = kp;
		    else{
			if ( prev!=NULL )
			    prev->next = next;
			else if ( isv )
			    sc->vkerns = next;
			else
			    sc->kerns = next;
			chunkfree(kp,sizeof(KernPair));
		    }
		}
	    }
	    if ( SCDuplicate(sc)!=sc ) {
		SplineChar *base = SCDuplicate(sc);
		int orig = sc->orig_pos, enc = sf->map->backmap[orig], uni = sc->unicodeenc;
		SplineCharFree(sc);
		sf->glyphs[i]=NULL;
		sf->map->backmap[orig] = -1;
		sf->map->map[enc] = base->orig_pos;
		AltUniAdd(base,uni);
	    }
	}
	for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL ) {
	    SplineChar *sc = sf->glyphs[i];
	    for ( layer=0; layer<sc->layer_cnt; ++layer ) {
		for ( refs = sf->glyphs[i]->layers[layer].refs; refs!=NULL; refs=refs->next ) {
		    SFDFixupRef(sf->glyphs[i],refs,layer);
		}
	    }
	    ff_progress_next();
	}
	if ( sf->cidmaster==NULL )
	    for ( i=sf->glyphcnt-1; i>=0 && sf->glyphs[i]==NULL; --i )
		sf->glyphcnt = i;
	if ( k>=cidmaster->subfontcnt )
    break;
	sf = cidmaster->subfonts[k++];
    }
}

static void SFDGetPrivate(FILE *sfd,SplineFont *sf) {
    int i, cnt, len;
    char name[200];
    char *pt, *end;

    sf->private = calloc(1,sizeof(struct psdict));
    getint(sfd,&cnt);
    sf->private->next = sf->private->cnt = cnt;
    sf->private->values = calloc(cnt,sizeof(char *));
    sf->private->keys = calloc(cnt,sizeof(char *));
    for ( i=0; i<cnt; ++i ) {
	getname(sfd,name);
	sf->private->keys[i] = copy(name);
	getint(sfd,&len);
	nlgetc(sfd);	/* skip space */
	pt = sf->private->values[i] = malloc(len+1);
	for ( end = pt+len; pt<end; ++pt )
	    *pt = nlgetc(sfd);
	*pt='\0';
    }
}

static void SFDGetSubrs(FILE *sfd) {
    /* Obselete, parse it in case there are any old sfds */
    int i, cnt, tot, len;
    struct enc85 dec;

    getint(sfd,&cnt);
    tot = 0;
    for ( i=0; i<cnt; ++i ) {
	getint(sfd,&len);
	tot += len;
    }

    memset(&dec,'\0', sizeof(dec)); dec.pos = -1;
    dec.sfd = sfd;
    for ( i=0; i<tot; ++i )
	Dec85(&dec);
}

static struct ttflangname *SFDGetLangName(FILE *sfd,struct ttflangname *old) {
    struct ttflangname *cur = chunkalloc(sizeof(struct ttflangname)), *prev;
    int i;

    getint(sfd,&cur->lang);
    for ( i=0; i<ttf_namemax; ++i )
	cur->names[i] = SFDReadUTF7Str(sfd);
    if ( old==NULL )
return( cur );
    for ( prev = old; prev->next !=NULL; prev = prev->next );
    prev->next = cur;
return( old );
}

static void SFDGetGasp(FILE *sfd,SplineFont *sf) {
    int i;

    getsint(sfd,(int16 *) &sf->gasp_cnt);
    sf->gasp = malloc(sf->gasp_cnt*sizeof(struct gasp));
    for ( i=0; i<sf->gasp_cnt; ++i ) {
	getsint(sfd,(int16 *) &sf->gasp[i].ppem);
	getsint(sfd,(int16 *) &sf->gasp[i].flags);
    }
    getsint(sfd,(int16 *) &sf->gasp_version);
}

static void SFDGetDesignSize(FILE *sfd,SplineFont *sf) {
    int ch;
    struct otfname *cur;

    getsint(sfd,(int16 *) &sf->design_size);
    while ( (ch=nlgetc(sfd))==' ' );
    ungetc(ch,sfd);
    if ( isdigit(ch)) {
	getsint(sfd,(int16 *) &sf->design_range_bottom);
	while ( (ch=nlgetc(sfd))==' ' );
	if ( ch!='-' )
	    ungetc(ch,sfd);
	getsint(sfd,(int16 *) &sf->design_range_top);
	getsint(sfd,(int16 *) &sf->fontstyle_id);
	for (;;) {
	    while ( (ch=nlgetc(sfd))==' ' );
	    ungetc(ch,sfd);
	    if ( !isdigit(ch))
	break;
	    cur = chunkalloc(sizeof(struct otfname));
	    cur->next = sf->fontstyle_name;
	    sf->fontstyle_name = cur;
	    getsint(sfd,(int16 *) &cur->lang);
	    cur->name = SFDReadUTF7Str(sfd);
	}
    }
}

static void SFDGetOtfFeatName(FILE *sfd,SplineFont *sf) {
    int ch;
    struct otfname *cur;
    struct otffeatname *fn;

    fn = chunkalloc(sizeof(struct otffeatname));
    fn->tag = gettag(sfd);
    for (;;) {
	while ( (ch=nlgetc(sfd))==' ' );
	ungetc(ch,sfd);
	if ( !isdigit(ch))
    break;
	cur = chunkalloc(sizeof(struct otfname));
	cur->next = fn->names;
	fn->names = cur;
	getsint(sfd,(int16 *) &cur->lang);
	cur->name = SFDReadUTF7Str(sfd);
    }
    fn->next = sf->feat_names;
    sf->feat_names = fn;
}

static Encoding *SFDGetEncoding(FILE *sfd, char *tok) {
    Encoding *enc = NULL;
    int encname;

    if ( getint(sfd,&encname) ) {
	if ( encname<(int)(sizeof(charset_names)/sizeof(charset_names[0])-1) )
	    enc = FindOrMakeEncoding(charset_names[encname]);
    } else {
	geteol(sfd,tok);
	enc = FindOrMakeEncoding(tok);
    }
    if ( enc==NULL )
	enc = &custom;
return( enc );
}

static enum uni_interp SFDGetUniInterp(FILE *sfd, char *tok, SplineFont *sf) {
    int uniinterp = ui_none;
    int i;

    geteol(sfd,tok);
    for ( i=0; unicode_interp_names[i]!=NULL; ++i )
	if ( strcmp(tok,unicode_interp_names[i])==0 ) {
	    uniinterp = i;
    break;
	}
    /* These values are now handled by namelists */
    if ( uniinterp == ui_adobe ) {
	sf->for_new_glyphs = NameListByName("AGL with PUA");
	uniinterp = ui_none;
    } else if ( uniinterp == ui_greek ) {
	sf->for_new_glyphs = NameListByName("Greek small caps");
	uniinterp = ui_none;
    } else if ( uniinterp == ui_ams ) {
	sf->for_new_glyphs = NameListByName("AMS Names");
	uniinterp = ui_none;
    }

return( uniinterp );
}

static void SFDGetNameList(FILE *sfd, char *tok, SplineFont *sf) {
    NameList *nl;

    geteol(sfd,tok);
    nl = NameListByName(tok);
    if ( nl==NULL )
	LogError(_("Failed to find NameList: %s"), tok);
    else
	sf->for_new_glyphs = nl;
}


static OTLookup *SFD_ParseNestedLookup(FILE *sfd, SplineFont *sf, int old) {
    uint32 tag;
    int ch, isgpos;
    OTLookup *otl;
    char *name;

    while ( (ch=nlgetc(sfd))==' ' );
    if ( ch=='~' )
return( NULL );
    else if ( old ) {
	if ( ch!='\'' )
return( NULL );

	ungetc(ch,sfd);
	tag = gettag(sfd);
return( (OTLookup *) (intpt) tag );
    } else {
	ungetc(ch,sfd);
	name = SFDReadUTF7Str(sfd);
	if ( name==NULL )
return( NULL );
	for ( isgpos=0; isgpos<2; ++isgpos ) {
	    for ( otl=isgpos ? sf->gpos_lookups : sf->gsub_lookups; otl!=NULL; otl=otl->next ) {
		if ( strcmp(name,otl->lookup_name )==0 )
	goto break2;
	    }
	}
	break2:
	free(name);
return( otl );
    }
}

static void SFDParseChainContext(FILE *sfd,SplineFont *sf,FPST *fpst, char *tok, int old) {
    int ch, i, j, k, temp;
    SplineFont *sli_sf = sf->cidmaster ? sf->cidmaster : sf;

    fpst->type = strnmatch(tok,"ContextPos",10)==0 ? pst_contextpos :
		strnmatch(tok,"ContextSub",10)==0 ? pst_contextsub :
		strnmatch(tok,"ChainPos",8)==0 ? pst_chainpos :
		strnmatch(tok,"ChainSub",8)==0 ? pst_chainsub : pst_reversesub;
    getname(sfd,tok);
    fpst->format = strmatch(tok,"glyph")==0 ? pst_glyphs :
		    strmatch(tok,"class")==0 ? pst_class :
		    strmatch(tok,"coverage")==0 ? pst_coverage : pst_reversecoverage;
    if ( old ) {
	fscanf(sfd, "%hu %hu", &((FPST1 *) fpst)->flags, &((FPST1 *) fpst)->script_lang_index );
	if ( ((FPST1 *) fpst)->script_lang_index>=((SplineFont1 *) sli_sf)->sli_cnt && ((FPST1 *) fpst)->script_lang_index!=SLI_NESTED ) {
	    static int complained=false;
	    if ( ((SplineFont1 *) sli_sf)->sli_cnt==0 )
		IError("'%c%c%c%c' has a script index out of bounds: %d\nYou MUST fix this manually",
			(((FPST1 *) fpst)->tag>>24), (((FPST1 *) fpst)->tag>>16)&0xff, (((FPST1 *) fpst)->tag>>8)&0xff, ((FPST1 *) fpst)->tag&0xff,
			((FPST1 *) fpst)->script_lang_index );
	    else if ( !complained )
		IError("'%c%c%c%c' has a script index out of bounds: %d",
			(((FPST1 *) fpst)->tag>>24), (((FPST1 *) fpst)->tag>>16)&0xff, (((FPST1 *) fpst)->tag>>8)&0xff, ((FPST1 *) fpst)->tag&0xff,
			((FPST1 *) fpst)->script_lang_index );
	    else
		IError("'%c%c%c%c' has a script index out of bounds: %d\n",
			(((FPST1 *) fpst)->tag>>24), (((FPST1 *) fpst)->tag>>16)&0xff, (((FPST1 *) fpst)->tag>>8)&0xff, ((FPST1 *) fpst)->tag&0xff,
			((FPST1 *) fpst)->script_lang_index );
	    if ( ((SplineFont1 *) sli_sf)->sli_cnt!=0 )
		((FPST1 *) fpst)->script_lang_index = ((SplineFont1 *) sli_sf)->sli_cnt-1;
	    complained = true;
	}
	while ( (ch=nlgetc(sfd))==' ' || ch=='\t' );
	if ( ch=='\'' ) {
	    ungetc(ch,sfd);
	    ((FPST1 *) fpst)->tag = gettag(sfd);
	} else
	    ungetc(ch,sfd);
    } else {
	fpst->subtable = SFFindLookupSubtableAndFreeName(sf,SFDReadUTF7Str(sfd));
        if ( !fpst->subtable )
            LogError(_("Missing Subtable definition found in chained context"));
        else
	    fpst->subtable->fpst = fpst;
    }
    fscanf(sfd, "%hu %hu %hu %hu", &fpst->nccnt, &fpst->bccnt, &fpst->fccnt, &fpst->rule_cnt );
    if ( fpst->nccnt!=0 || fpst->bccnt!=0 || fpst->fccnt!=0 ) {
	fpst->nclass = malloc(fpst->nccnt*sizeof(char *));
	fpst->nclassnames = calloc(fpst->nccnt,sizeof(char *));
	if ( fpst->nccnt!=0 ) fpst->nclass[0] = NULL;
	if ( fpst->bccnt!=0 || fpst->fccnt!=0 ) {
	    fpst->bclass = malloc(fpst->bccnt*sizeof(char *));
	    fpst->bclassnames = calloc(fpst->bccnt,sizeof(char *));
	    if (fpst->bccnt!=0 ) fpst->bclass[0] = NULL;
	    fpst->fclass = malloc(fpst->fccnt*sizeof(char *));
	    fpst->fclassnames = calloc(fpst->fccnt,sizeof(char *));
	    if (fpst->fccnt!=0 ) fpst->fclass[0] = NULL;
	}
    }

    for ( j=0; j<3; ++j ) {
	for ( i=1; i<(&fpst->nccnt)[j]; ++i ) {
	    getname(sfd,tok);
	    if ( i==1 && j==0 && strcmp(tok,"Class0:")==0 )
		i=0;
	    getint(sfd,&temp);
	    (&fpst->nclass)[j][i] = malloc(temp+1); (&fpst->nclass)[j][i][temp] = '\0';
	    nlgetc(sfd);	/* skip space */
	    fread((&fpst->nclass)[j][i],1,temp,sfd);
	}
    }

    fpst->rules = calloc(fpst->rule_cnt,sizeof(struct fpst_rule));
    for ( i=0; i<fpst->rule_cnt; ++i ) {
	switch ( fpst->format ) {
	  case pst_glyphs:
	    for ( j=0; j<3; ++j ) {
		getname(sfd,tok);
		getint(sfd,&temp);
		(&fpst->rules[i].u.glyph.names)[j] = malloc(temp+1);
		(&fpst->rules[i].u.glyph.names)[j][temp] = '\0';
		nlgetc(sfd);	/* skip space */
		fread((&fpst->rules[i].u.glyph.names)[j],1,temp,sfd);
	    }
	  break;
	  case pst_class:
	    fscanf( sfd, "%d %d %d", &fpst->rules[i].u.class.ncnt, &fpst->rules[i].u.class.bcnt, &fpst->rules[i].u.class.fcnt );
	    for ( j=0; j<3; ++j ) {
		getname(sfd,tok);
		(&fpst->rules[i].u.class.nclasses)[j] = malloc((&fpst->rules[i].u.class.ncnt)[j]*sizeof(uint16));
		for ( k=0; k<(&fpst->rules[i].u.class.ncnt)[j]; ++k ) {
		    getusint(sfd,&(&fpst->rules[i].u.class.nclasses)[j][k]);
		}
	    }
	  break;
	  case pst_coverage:
	  case pst_reversecoverage:
	    fscanf( sfd, "%d %d %d", &fpst->rules[i].u.coverage.ncnt, &fpst->rules[i].u.coverage.bcnt, &fpst->rules[i].u.coverage.fcnt );
	    for ( j=0; j<3; ++j ) {
		(&fpst->rules[i].u.coverage.ncovers)[j] = malloc((&fpst->rules[i].u.coverage.ncnt)[j]*sizeof(char *));
		for ( k=0; k<(&fpst->rules[i].u.coverage.ncnt)[j]; ++k ) {
		    getname(sfd,tok);
		    getint(sfd,&temp);
		    (&fpst->rules[i].u.coverage.ncovers)[j][k] = malloc(temp+1);
		    (&fpst->rules[i].u.coverage.ncovers)[j][k][temp] = '\0';
		    nlgetc(sfd);	/* skip space */
		    fread((&fpst->rules[i].u.coverage.ncovers)[j][k],1,temp,sfd);
		}
	    }
	  break;
	  default:
	  break;
	}
	switch ( fpst->format ) {
	  case pst_glyphs:
	  case pst_class:
	  case pst_coverage:
	    getint(sfd,&fpst->rules[i].lookup_cnt);
	    fpst->rules[i].lookups = malloc(fpst->rules[i].lookup_cnt*sizeof(struct seqlookup));
	    for ( j=k=0; j<fpst->rules[i].lookup_cnt; ++j ) {
		getname(sfd,tok);
		getint(sfd,&fpst->rules[i].lookups[j].seq);
		fpst->rules[i].lookups[k].lookup = SFD_ParseNestedLookup(sfd,sf,old);
		if ( fpst->rules[i].lookups[k].lookup!=NULL )
		    ++k;
	    }
	    fpst->rules[i].lookup_cnt = k;
	  break;
	  case pst_reversecoverage:
	    getname(sfd,tok);
	    getint(sfd,&temp);
	    fpst->rules[i].u.rcoverage.replacements = malloc(temp+1);
	    fpst->rules[i].u.rcoverage.replacements[temp] = '\0';
	    nlgetc(sfd);	/* skip space */
	    fread(fpst->rules[i].u.rcoverage.replacements,1,temp,sfd);
	  break;
	  default:
	  break;
	}
    }
    getname(sfd,tok);	/* EndFPST, or one of the ClassName tokens (in newer sfds) */
    while ( strcmp(tok,"ClassNames:")==0 || strcmp(tok,"BClassNames:")==0 ||
	    strcmp(tok,"FClassNames:")==0 ) {
	int which = strcmp(tok,"ClassNames:")==0 ? 0 :
		    strcmp(tok,"BClassNames:")==0 ? 1 : 2;
	int cnt = (&fpst->nccnt)[which];
	char **classnames = (&fpst->nclassnames)[which];
	int i;

	for ( i=0; i<cnt; ++i )
	    classnames[i] = SFDReadUTF7Str(sfd);
	getname(sfd,tok);	/* EndFPST, or one of the ClassName tokens (in newer sfds) */
    }

}

static void SFDParseStateMachine(FILE *sfd,SplineFont *sf,ASM *sm, char *tok,int old) {
    int i, temp;

    sm->type = strnmatch(tok,"MacIndic",8)==0 ? asm_indic :
		strnmatch(tok,"MacContext",10)==0 ? asm_context :
		strnmatch(tok,"MacLigature",11)==0 ? asm_lig :
		strnmatch(tok,"MacSimple",9)==0 ? asm_simple :
		strnmatch(tok,"MacKern",7)==0 ? asm_kern : asm_insert;
    if ( old ) {
	getusint(sfd,&((ASM1 *) sm)->feature);
	nlgetc(sfd);		/* Skip comma */
	getusint(sfd,&((ASM1 *) sm)->setting);
    } else {
	sm->subtable = SFFindLookupSubtableAndFreeName(sf,SFDReadUTF7Str(sfd));
	sm->subtable->sm = sm;
    }
    getusint(sfd,&sm->flags);
    getusint(sfd,&sm->class_cnt);
    getusint(sfd,&sm->state_cnt);

    sm->classes = malloc(sm->class_cnt*sizeof(char *));
    sm->classes[0] = sm->classes[1] = sm->classes[2] = sm->classes[3] = NULL;
    for ( i=4; i<sm->class_cnt; ++i ) {
	getname(sfd,tok);
	getint(sfd,&temp);
	sm->classes[i] = malloc(temp+1); sm->classes[i][temp] = '\0';
	nlgetc(sfd);	/* skip space */
	fread(sm->classes[i],1,temp,sfd);
    }

    sm->state = malloc(sm->class_cnt*sm->state_cnt*sizeof(struct asm_state));
    for ( i=0; i<sm->class_cnt*sm->state_cnt; ++i ) {
	getusint(sfd,&sm->state[i].next_state);
	getusint(sfd,&sm->state[i].flags);
	if ( sm->type == asm_context ) {
	    sm->state[i].u.context.mark_lookup = SFD_ParseNestedLookup(sfd,sf,old);
	    sm->state[i].u.context.cur_lookup = SFD_ParseNestedLookup(sfd,sf,old);
	} else if ( sm->type == asm_insert ) {
	    getint(sfd,&temp);
	    if ( temp==0 )
		sm->state[i].u.insert.mark_ins = NULL;
	    else {
		sm->state[i].u.insert.mark_ins = malloc(temp+1); sm->state[i].u.insert.mark_ins[temp] = '\0';
		nlgetc(sfd);	/* skip space */
		fread(sm->state[i].u.insert.mark_ins,1,temp,sfd);
	    }
	    getint(sfd,&temp);
	    if ( temp==0 )
		sm->state[i].u.insert.cur_ins = NULL;
	    else {
		sm->state[i].u.insert.cur_ins = malloc(temp+1); sm->state[i].u.insert.cur_ins[temp] = '\0';
		nlgetc(sfd);	/* skip space */
		fread(sm->state[i].u.insert.cur_ins,1,temp,sfd);
	    }
	} else if ( sm->type == asm_kern ) {
	    int j;
	    getint(sfd,&sm->state[i].u.kern.kcnt);
	    if ( sm->state[i].u.kern.kcnt!=0 )
		sm->state[i].u.kern.kerns = malloc(sm->state[i].u.kern.kcnt*sizeof(int16));
	    for ( j=0; j<sm->state[i].u.kern.kcnt; ++j ) {
		getint(sfd,&temp);
		sm->state[i].u.kern.kerns[j] = temp;
	    }
	}
    }
    getname(sfd,tok);			/* EndASM */
}

static struct macname *SFDParseMacNames(FILE *sfd, char *tok) {
    struct macname *head=NULL, *last=NULL, *cur;
    int enc, lang, len;
    char *pt;
    int ch;

    while ( strcmp(tok,"MacName:")==0 ) {
	cur = chunkalloc(sizeof(struct macname));
	if ( last==NULL )
	    head = cur;
	else
	    last->next = cur;
	last = cur;

	getint(sfd,&enc);
	getint(sfd,&lang);
	getint(sfd,&len);
	cur->enc = enc;
	cur->lang = lang;
	cur->name = pt = malloc(len+1);

	while ( (ch=nlgetc(sfd))==' ');
	if ( ch=='"' )
	    ch = nlgetc(sfd);
	while ( ch!='"' && ch!=EOF && pt<cur->name+len ) {
	    if ( ch=='\\' ) {
		*pt  = (nlgetc(sfd)-'0')<<6;
		*pt |= (nlgetc(sfd)-'0')<<3;
		*pt |= (nlgetc(sfd)-'0');
	    } else
		*pt++ = ch;
	    ch = nlgetc(sfd);
	}
	*pt = '\0';
	getname(sfd,tok);
    }
return( head );
}

MacFeat *SFDParseMacFeatures(FILE *sfd, char *tok) {
    MacFeat *cur, *head=NULL, *last=NULL;
    struct macsetting *slast, *scur;
    int feat, ism, def, set;

    while ( strcmp(tok,"MacFeat:")==0 ) {
	cur = chunkalloc(sizeof(MacFeat));
	if ( last==NULL )
	    head = cur;
	else
	    last->next = cur;
	last = cur;

	getint(sfd,&feat); getint(sfd,&ism); getint(sfd, &def);
	cur->feature = feat; cur->ismutex = ism; cur->default_setting = def;
	getname(sfd,tok);
	cur->featname = SFDParseMacNames(sfd,tok);
	slast = NULL;
	while ( strcmp(tok,"MacSetting:")==0 ) {
	    scur = chunkalloc(sizeof(struct macsetting));
	    if ( slast==NULL )
		cur->settings = scur;
	    else
		slast->next = scur;
	    slast = scur;

	    getint(sfd,&set);
	    scur->setting = set;
	    getname(sfd,tok);
	    scur->setname = SFDParseMacNames(sfd,tok);
	}
    }
return( head );
}

static char *SFDParseMMSubroutine(FILE *sfd) {
    char buffer[400], *sofar=calloc(1,1);
    const char *endtok = "EndMMSubroutine";
    int len = 0, blen, first=true;

    while ( fgets(buffer,sizeof(buffer),sfd)!=NULL ) {
	if ( strncmp(buffer,endtok,strlen(endtok))==0 )
    break;
	if ( first ) {
	    first = false;
	    if ( strcmp(buffer,"\n")==0 )
    continue;
	}
	blen = strlen(buffer);
	sofar = realloc(sofar,len+blen+1);
	strcpy(sofar+len,buffer);
	len += blen;
    }
    if ( len>0 && sofar[len-1]=='\n' )
	sofar[len-1] = '\0';
return( sofar );
}

static void MMInferStuff(MMSet *mm) {
    int i,j;

    if ( mm==NULL )
return;
    if ( mm->apple ) {
	for ( i=0; i<mm->axis_count; ++i ) {
	    for ( j=0; j<mm->axismaps[i].points; ++j ) {
		real val = mm->axismaps[i].blends[j];
		if ( val == -1. )
		    mm->axismaps[i].min = mm->axismaps[i].designs[j];
		else if ( val==0 )
		    mm->axismaps[i].def = mm->axismaps[i].designs[j];
		else if ( val==1 )
		    mm->axismaps[i].max = mm->axismaps[i].designs[j];
	    }
	}
    }
}

static void SFDSizeMap(EncMap *map,int glyphcnt,int enccnt) {
    if ( glyphcnt>map->backmax ) {
	map->backmap = realloc(map->backmap,glyphcnt*sizeof(int));
	memset(map->backmap+map->backmax,-1,(glyphcnt-map->backmax)*sizeof(int));
	map->backmax = glyphcnt;
    }
    if ( enccnt>map->encmax ) {
	map->map = realloc(map->map,enccnt*sizeof(int));
	memset(map->map+map->backmax,-1,(enccnt-map->encmax)*sizeof(int));
	map->encmax = map->enccount = enccnt;
    }
}

static SplineFont *SFD_GetFont(FILE *sfd,SplineFont *cidmaster,char *tok,
	int fromdir, char *dirname, float sfdversion);

static void SFD_DoAltUnis(SplineFont *sf) {
    int i;
    struct altuni *alt;
    SplineChar *sc;

    for ( i=0; i<sf->glyphcnt; ++i ) if ( (sc = sf->glyphs[i])!=NULL ) {
	for ( alt = sc->altuni; alt!=NULL; alt = alt->next ) {
	    if ( alt->vs==-1 && alt->fid==0 ) {
		int enc = EncFromUni(alt->unienc,sf->map->enc);
		if ( enc!=-1 )
		    SFDSetEncMap(sf,sc->orig_pos,enc);
	    }
	}
    }
}

static void SFDParseLookup(FILE *sfd,OTLookup *otl) {
    int ch;
    struct lookup_subtable *sub, *lastsub;
    FeatureScriptLangList *fl, *lastfl;
    struct scriptlanglist *sl, *lastsl;
    int i, lcnt, lmax=0;
    uint32 *langs=NULL;
    char *subname;

    while ( (ch=nlgetc(sfd))==' ' );
    if ( ch=='{' ) {
	lastsub = NULL;
	while ( (subname = SFDReadUTF7Str(sfd))!=NULL ) {
	    while ( (ch=nlgetc(sfd))==' ' );
	    ungetc(ch,sfd);
	    sub = chunkalloc(sizeof(struct lookup_subtable));
	    sub->subtable_name = subname;
	    sub->lookup = otl;
	    switch ( otl->lookup_type ) {
	      case gsub_single:
		while ( (ch=nlgetc(sfd))==' ' );
		if ( ch=='(' ) {
		    sub->suffix = SFDReadUTF7Str(sfd);
		    while ( (ch=nlgetc(sfd))==' ' );
			/* slurp final paren */
		} else
		    ungetc(ch,sfd);
		sub->per_glyph_pst_or_kern = true;
	      break;
	      case gsub_multiple: case gsub_alternate: case gsub_ligature:
	      case gpos_single:
		sub->per_glyph_pst_or_kern = true;
	      break;
	      case gpos_pair:
		if ( (ch=nlgetc(sfd))=='(' ) {
		    ch = nlgetc(sfd);
		    sub->vertical_kerning = (ch=='1');
		    nlgetc(sfd);	/* slurp final paren */
		    ch=nlgetc(sfd);
		}
		if ( ch=='[' ) {
		    getsint(sfd,&sub->separation);
		    nlgetc(sfd);	/* slurp comma */
		    getsint(sfd,&sub->minkern);
		    nlgetc(sfd);	/* slurp comma */
		    ch = nlgetc(sfd);
		    sub->kerning_by_touch = ((ch-'0')&1)?1:0;
		    sub->onlyCloser       = ((ch-'0')&2)?1:0;
		    sub->dontautokern     = ((ch-'0')&4)?1:0;
		    nlgetc(sfd);	/* slurp final bracket */
		} else {
		    ungetc(ch,sfd);
		}
		sub->per_glyph_pst_or_kern = true;
	      break;
	      case gpos_cursive: case gpos_mark2base: case gpos_mark2ligature: case gpos_mark2mark:
		sub->anchor_classes = true;
	      break;
	      default:
	      break;
	    }
	    if ( lastsub==NULL )
		otl->subtables = sub;
	    else
		lastsub->next = sub;
	    lastsub = sub;
	}
	while ( (ch=nlgetc(sfd))==' ' );
	if ( ch=='}' )
	    ch = nlgetc(sfd);
    }
    while ( ch==' ' )
	ch = nlgetc(sfd);
    if ( ch=='[' ) {
	lastfl = NULL;
	for (;;) {
	    while ( (ch=nlgetc(sfd))==' ' );
	    if ( ch==']' )
	break;
	    fl = chunkalloc(sizeof(FeatureScriptLangList));
	    if ( lastfl==NULL )
		otl->features = fl;
	    else
		lastfl->next = fl;
	    lastfl = fl;
	    if ( ch=='<' ) {
		int ft=0,fs=0;
		fscanf(sfd,"%d,%d>", &ft, &fs );
		fl->ismac = true;
		fl->featuretag = (ft<<16) | fs;
	    } else if ( ch=='\'' ) {
		ungetc(ch,sfd);
		fl->featuretag = gettag(sfd);
	    }
	    while ( (ch=nlgetc(sfd))==' ' );
	    if ( ch=='(' ) {
		lastsl = NULL;
		for (;;) {
		    while ( (ch=nlgetc(sfd))==' ' );
		    if ( ch==')' )
		break;
		    sl = chunkalloc(sizeof(struct scriptlanglist));
		    if ( lastsl==NULL )
			fl->scripts = sl;
		    else
			lastsl->next = sl;
		    lastsl = sl;
		    if ( ch=='\'' ) {
			ungetc(ch,sfd);
			sl->script = gettag(sfd);
		    }
		    while ( (ch=nlgetc(sfd))==' ' );
		    if ( ch=='<' ) {
			lcnt = 0;
			for (;;) {
			    while ( (ch=nlgetc(sfd))==' ' );
			    if ( ch=='>' )
			break;
			    if ( ch=='\'' ) {
				ungetc(ch,sfd);
			        if ( lcnt>=lmax )
				    langs = realloc(langs,(lmax+=10)*sizeof(uint32));
				langs[lcnt++] = gettag(sfd);
			    }
			}
			sl->lang_cnt = lcnt;
			if ( lcnt>MAX_LANG )
			    sl->morelangs = malloc((lcnt-MAX_LANG)*sizeof(uint32));
			for ( i=0; i<lcnt; ++i ) {
			    if ( i<MAX_LANG )
				sl->langs[i] = langs[i];
			    else
				sl->morelangs[i-MAX_LANG] = langs[i];
			}
		    }
		}
	    }
	}
    }
    free(langs);
}

static void SFDParseMathItem(FILE *sfd,SplineFont *sf,char *tok) {
    /* The first five characters of a math item's keyword will be "MATH:" */
    /*  the rest will be one of the entries in math_constants_descriptor */
    int i;
    struct MATH *math;

    if ( (math = sf->MATH) == NULL )
	math = sf->MATH = calloc(1,sizeof(struct MATH));
    for ( i=0; math_constants_descriptor[i].script_name!=NULL; ++i ) {
	char *name = math_constants_descriptor[i].script_name;
	int len = strlen( name );
	if ( strncmp(tok+5,name,len)==0 && tok[5+len] == ':' && tok[6+len]=='\0' ) {
	    int16 *pos = (int16 *) (((char *) (math)) + math_constants_descriptor[i].offset );
	    getsint(sfd,pos);
	    if ( math_constants_descriptor[i].devtab_offset != -1 ) {
		DeviceTable **devtab = (DeviceTable **) (((char *) (math)) + math_constants_descriptor[i].devtab_offset );
		*devtab = SFDReadDeviceTable(sfd,*devtab);
    break;
	    }
	}
    }
}

static struct baselangextent *ParseBaseLang(FILE *sfd) {
    struct baselangextent *bl;
    struct baselangextent *cur, *last;
    int ch;

    while ( (ch=nlgetc(sfd))==' ' );
    if ( ch=='{' ) {
	bl = chunkalloc(sizeof(struct baselangextent));
	while ( (ch=nlgetc(sfd))==' ' );
	ungetc(ch,sfd);
	if ( ch=='\'' )
	    bl->lang = gettag(sfd);		/* Lang or Feature tag, or nothing */
	getsint(sfd,&bl->descent);
	getsint(sfd,&bl->ascent);
	last = NULL;
	while ( (ch=nlgetc(sfd))==' ' );
	while ( ch=='{' ) {
	    ungetc(ch,sfd);
	    cur = ParseBaseLang(sfd);
	    if ( last==NULL )
		bl->features = cur;
	    else
		last->next = cur;
	    last = cur;
	    while ( (ch=nlgetc(sfd))==' ' );
	}
	if ( ch!='}' ) ungetc(ch,sfd);
return( bl );
    }
return( NULL );
}

static struct basescript *SFDParseBaseScript(FILE *sfd,struct Base *base) {
    struct basescript *bs;
    int i, ch;
    struct baselangextent *last, *cur;

    if ( base==NULL )
return(NULL);

    bs = chunkalloc(sizeof(struct basescript));

    bs->script = gettag(sfd);
    getint(sfd,&bs->def_baseline);
    if ( base->baseline_cnt!=0 ) {
	bs->baseline_pos = calloc(base->baseline_cnt,sizeof(int16));
	for ( i=0; i<base->baseline_cnt; ++i )
	    getsint(sfd, &bs->baseline_pos[i]);
    }
    while ( (ch=nlgetc(sfd))==' ' );
    last = NULL;
    while ( ch=='{' ) {
	ungetc(ch,sfd);
	cur = ParseBaseLang(sfd);
	if ( last==NULL )
	    bs->langs = cur;
	else
	    last->next = cur;
	last = cur;
	while ( (ch=nlgetc(sfd))==' ' );
    }
return( bs );
}

static struct Base *SFDParseBase(FILE *sfd) {
    struct Base *base = chunkalloc(sizeof(struct Base));
    int i;

    getint(sfd,&base->baseline_cnt);
    if ( base->baseline_cnt!=0 ) {
	base->baseline_tags = malloc(base->baseline_cnt*sizeof(uint32));
	for ( i=0; i<base->baseline_cnt; ++i )
	    base->baseline_tags[i] = gettag(sfd);
    }
return( base );
}

static OTLookup **SFDLookupList(FILE *sfd,SplineFont *sf) {
    int ch;
    OTLookup *space[100], **buf=space, *otl, **ret;
    int lcnt=0, lmax=100;
    char *name;

    for (;;) {
	while ( (ch=nlgetc(sfd))==' ' );
	if ( ch=='\n' || ch==EOF )
    break;
	ungetc(ch,sfd);
	name = SFDReadUTF7Str(sfd);
	otl = SFFindLookup(sf,name);
	free(name);
	if ( otl!=NULL ) {
	    if ( lcnt>lmax ) {
		if ( buf==space ) {
		    buf = malloc((lmax=lcnt+50)*sizeof(OTLookup *));
		    memcpy(buf,space,sizeof(space));
		} else
		    buf = realloc(buf,(lmax+=50)*sizeof(OTLookup *));
	    }
	    buf[lcnt++] = otl;
	}
    }
    if ( lcnt==0 )
return( NULL );

    ret = malloc((lcnt+1)*sizeof(OTLookup *));
    memcpy(ret,buf,lcnt*sizeof(OTLookup *));
    ret[lcnt] = NULL;
return( ret );
}

static void SFDParseJustify(FILE *sfd, SplineFont *sf, char *tok) {
    Justify *last=NULL, *cur;
    struct jstf_lang *jlang, *llast;
    int p = 0,ch;

    while ( strcmp(tok,"Justify:")==0 ) {
	cur = chunkalloc(sizeof(Justify));
	if ( last==NULL )
	    sf->justify = cur;
	else
	    last->next = cur;
	last = cur;
	llast = jlang = NULL;
	cur->script = gettag(sfd);
	while ( getname(sfd,tok)>0 ) {
	    if ( strcmp(tok,"Justify:")==0 || strcmp(tok,"EndJustify")==0 )
	break;
	    if ( strcmp(tok,"JstfExtender:")==0 ) {
		while ( (ch=nlgetc(sfd))==' ' );
		ungetc(ch,sfd);
		geteol(sfd,tok);
		cur->extenders = copy(tok);
	    } else if ( strcmp(tok,"JstfLang:")==0 ) {
		jlang = chunkalloc(sizeof(struct jstf_lang));
		if ( llast==NULL )
		    cur->langs = jlang;
		else
		    llast->next = jlang;
		llast = jlang;
		jlang->lang = gettag(sfd);
		p = -1;
		getint(sfd,&jlang->cnt);
		if ( jlang->cnt!=0 )
		    jlang->prios = calloc(jlang->cnt,sizeof(struct jstf_prio));
	    } else if ( strcmp(tok,"JstfPrio:")==0 ) {
		if ( jlang!=NULL ) {
		    ++p;
		    if ( p>= jlang->cnt ) {
			jlang->prios = realloc(jlang->prios,(p+1)*sizeof(struct jstf_prio));
			memset(jlang->prios+jlang->cnt,0,(p+1-jlang->cnt)*sizeof(struct jstf_prio));
			jlang->cnt = p+1;
		    }
		}
	    } else if ( strcmp(tok,"JstfEnableShrink:" )==0 ) {
		if ( p<0 ) p=0;
		if ( jlang!=NULL && p<jlang->cnt )
		    jlang->prios[p].enableShrink = SFDLookupList(sfd,sf);
	    } else if ( strcmp(tok,"JstfDisableShrink:" )==0 ) {
		if ( p<0 ) p=0;
		if ( jlang!=NULL && p<jlang->cnt )
		    jlang->prios[p].disableShrink = SFDLookupList(sfd,sf);
	    } else if ( strcmp(tok,"JstfMaxShrink:" )==0 ) {
		if ( p<0 ) p=0;
		if ( jlang!=NULL && p<jlang->cnt )
		    jlang->prios[p].maxShrink = SFDLookupList(sfd,sf);
	    } else if ( strcmp(tok,"JstfEnableExtend:" )==0 ) {
		if ( p<0 ) p=0;
		if ( jlang!=NULL && p<jlang->cnt )
		    jlang->prios[p].enableExtend = SFDLookupList(sfd,sf);
	    } else if ( strcmp(tok,"JstfDisableExtend:" )==0 ) {
		if ( p<0 ) p=0;
		if ( jlang!=NULL && p<jlang->cnt )
		    jlang->prios[p].disableExtend = SFDLookupList(sfd,sf);
	    } else if ( strcmp(tok,"JstfMaxExtend:" )==0 ) {
		if ( p<0 ) p=0;
		if ( jlang!=NULL && p<jlang->cnt )
		    jlang->prios[p].maxExtend = SFDLookupList(sfd,sf);
	    } else
		geteol(sfd,tok);
	}
    }
}



void SFD_GetFontMetaDataData_Init( SFD_GetFontMetaDataData* d )
{
    memset( d, 0, sizeof(SFD_GetFontMetaDataData));
}

/**
 *
 * @return true if the function matched the current token. If true
 *         is returned the caller should avoid further processing of 'tok'
 *         a return of false means that the caller might try
 *         to handle the token with another function or drop it.
 */
bool SFD_GetFontMetaData( FILE *sfd,
			  char *tok,
			  SplineFont *sf,
			  SFD_GetFontMetaDataData* d )
{
    int ch;
    int i;
    KernClass* kc = 0;
    int old;
    char val[2000];

    // This allows us to assume we can dereference d
    // at all times
    static SFD_GetFontMetaDataData my_static_d;
    static int my_static_d_is_virgin = 1;
    if( !d )
    {
	if( my_static_d_is_virgin )
	{
	    my_static_d_is_virgin = 0;
	    SFD_GetFontMetaDataData_Init( &my_static_d );
	}
	d = &my_static_d;
    }

    if ( strmatch(tok,"FontName:")==0 )
    {
	geteol(sfd,val);
	sf->fontname = copy(val);
    }
    else if ( strmatch(tok,"FullName:")==0 )
    {
	geteol(sfd,val);
	sf->fullname = copy(val);
    }
    else if ( strmatch(tok,"FamilyName:")==0 )
    {
	geteol(sfd,val);
	sf->familyname = copy(val);
    }
    else if ( strmatch(tok,"DefaultBaseFilename:")==0 )
    {
	geteol(sfd,val);
	sf->defbasefilename = copy(val);
    }
    else if ( strmatch(tok,"Weight:")==0 )
    {
	getprotectedname(sfd,val);
	sf->weight = copy(val);
    }
    else if ( strmatch(tok,"Copyright:")==0 )
    {
	sf->copyright = getquotedeol(sfd);
    }
    else if ( strmatch(tok,"Comments:")==0 )
    {
	char *temp = getquotedeol(sfd);
	sf->comments = latin1_2_utf8_copy(temp);
	free(temp);
    }
    else if ( strmatch(tok,"UComments:")==0 )
    {
	sf->comments = SFDReadUTF7Str(sfd);
    }
    else if ( strmatch(tok,"FontLog:")==0 )
    {
	sf->fontlog = SFDReadUTF7Str(sfd);
    }
    else if ( strmatch(tok,"Version:")==0 )
    {
	geteol(sfd,val);
	sf->version = copy(val);
    }
    else if ( strmatch(tok,"StyleMapFamilyName:")==0 )
    {
    sf->styleMapFamilyName = SFDReadUTF7Str(sfd);
    }
    /* Legacy attribute for StyleMapFamilyName. Deprecated. */
    else if ( strmatch(tok,"OS2FamilyName:")==0 )
    {
    if (sf->styleMapFamilyName == NULL)
        sf->styleMapFamilyName = SFDReadUTF7Str(sfd);
    }
    else if ( strmatch(tok,"FONDName:")==0 )
    {
	geteol(sfd,val);
	sf->fondname = copy(val);
    }
    else if ( strmatch(tok,"ItalicAngle:")==0 )
    {
	getreal(sfd,&sf->italicangle);
    }
    else if ( strmatch(tok,"StrokeWidth:")==0 )
    {
	getreal(sfd,&sf->strokewidth);
    }
    else if ( strmatch(tok,"UnderlinePosition:")==0 )
    {
	getreal(sfd,&sf->upos);
    }
    else if ( strmatch(tok,"UnderlineWidth:")==0 )
    {
	getreal(sfd,&sf->uwidth);
    }
    else if ( strmatch(tok,"ModificationTime:")==0 )
    {
	getlonglong(sfd,&sf->modificationtime);
    }
    else if ( strmatch(tok,"CreationTime:")==0 )
    {
	getlonglong(sfd,&sf->creationtime);
	d->hadtimes = true;
    }
    else if ( strmatch(tok,"PfmFamily:")==0 )
    {
	int temp;
	getint(sfd,&temp);
	sf->pfminfo.pfmfamily = temp;
	sf->pfminfo.pfmset = true;
    }
    else if ( strmatch(tok,"LangName:")==0 )
    {
	sf->names = SFDGetLangName(sfd,sf->names);
    }
    else if ( strmatch(tok,"GaspTable:")==0 )
    {
	SFDGetGasp(sfd,sf);
    }
    else if ( strmatch(tok,"DesignSize:")==0 )
    {
	SFDGetDesignSize(sfd,sf);
    }
    else if ( strmatch(tok,"OtfFeatName:")==0 )
    {
	SFDGetOtfFeatName(sfd,sf);
    }
    else if ( strmatch(tok,"PfmWeight:")==0 || strmatch(tok,"TTFWeight:")==0 )
    {
	getsint(sfd,&sf->pfminfo.weight);
	sf->pfminfo.pfmset = true;
    }
    else if ( strmatch(tok,"TTFWidth:")==0 )
    {
	getsint(sfd,&sf->pfminfo.width);
	sf->pfminfo.pfmset = true;
    }
    else if ( strmatch(tok,"Panose:")==0 )
    {
	int temp,i;
	for ( i=0; i<10; ++i )
	{
	    getint(sfd,&temp);
	    sf->pfminfo.panose[i] = temp;
	}
	sf->pfminfo.panose_set = true;
    }
    else if ( strmatch(tok,"LineGap:")==0 )
    {
	getsint(sfd,&sf->pfminfo.linegap);
	sf->pfminfo.pfmset = true;
    }
    else if ( strmatch(tok,"VLineGap:")==0 )
    {
	getsint(sfd,&sf->pfminfo.vlinegap);
	sf->pfminfo.pfmset = true;
    }
    else if ( strmatch(tok,"HheadAscent:")==0 )
    {
	getsint(sfd,&sf->pfminfo.hhead_ascent);
    }
    else if ( strmatch(tok,"HheadAOffset:")==0 )
    {
	int temp;
	getint(sfd,&temp); sf->pfminfo.hheadascent_add = temp;
    }
    else if ( strmatch(tok,"HheadDescent:")==0 )
    {
	getsint(sfd,&sf->pfminfo.hhead_descent);
    }
    else if ( strmatch(tok,"HheadDOffset:")==0 )
    {
	int temp;
	getint(sfd,&temp); sf->pfminfo.hheaddescent_add = temp;
    }
    else if ( strmatch(tok,"OS2TypoLinegap:")==0 )
    {
	getsint(sfd,&sf->pfminfo.os2_typolinegap);
    }
    else if ( strmatch(tok,"OS2TypoAscent:")==0 )
    {
	getsint(sfd,&sf->pfminfo.os2_typoascent);
    }
    else if ( strmatch(tok,"OS2TypoAOffset:")==0 )
    {
	int temp;
	getint(sfd,&temp); sf->pfminfo.typoascent_add = temp;
    }
    else if ( strmatch(tok,"OS2TypoDescent:")==0 )
    {
	getsint(sfd,&sf->pfminfo.os2_typodescent);
    }
    else if ( strmatch(tok,"OS2TypoDOffset:")==0 )
    {
	int temp;
	getint(sfd,&temp); sf->pfminfo.typodescent_add = temp;
    }
    else if ( strmatch(tok,"OS2WinAscent:")==0 )
    {
	getsint(sfd,&sf->pfminfo.os2_winascent);
    }
    else if ( strmatch(tok,"OS2WinDescent:")==0 )
    {
	getsint(sfd,&sf->pfminfo.os2_windescent);
    }
    else if ( strmatch(tok,"OS2WinAOffset:")==0 )
    {
	int temp;
	getint(sfd,&temp); sf->pfminfo.winascent_add = temp;
    }
    else if ( strmatch(tok,"OS2WinDOffset:")==0 )
    {
	int temp;
	getint(sfd,&temp); sf->pfminfo.windescent_add = temp;
    }
    else if ( strmatch(tok,"HHeadAscent:")==0 )
    {
	// DUPLICATE OF ABOVE
	getsint(sfd,&sf->pfminfo.hhead_ascent);
    }
    else if ( strmatch(tok,"HHeadDescent:")==0 )
    {
	// DUPLICATE OF ABOVE
	getsint(sfd,&sf->pfminfo.hhead_descent);
    }

    else if ( strmatch(tok,"HHeadAOffset:")==0 )
    {
	// DUPLICATE OF ABOVE
	int temp;
	getint(sfd,&temp); sf->pfminfo.hheadascent_add = temp;
    }
    else if ( strmatch(tok,"HHeadDOffset:")==0 )
    {
	// DUPLICATE OF ABOVE
	int temp;
	getint(sfd,&temp); sf->pfminfo.hheaddescent_add = temp;
    }
    else if ( strmatch(tok,"MacStyle:")==0 )
    {
	getsint(sfd,&sf->macstyle);
    }
    else if ( strmatch(tok,"OS2SubXSize:")==0 )
    {
	getsint(sfd,&sf->pfminfo.os2_subxsize);
	sf->pfminfo.subsuper_set = true;
    }
    else if ( strmatch(tok,"OS2SubYSize:")==0 )
    {
	getsint(sfd,&sf->pfminfo.os2_subysize);
    }
    else if ( strmatch(tok,"OS2SubXOff:")==0 )
    {
	getsint(sfd,&sf->pfminfo.os2_subxoff);
    }
    else if ( strmatch(tok,"OS2SubYOff:")==0 )
    {
	getsint(sfd,&sf->pfminfo.os2_subyoff);
    }
    else if ( strmatch(tok,"OS2SupXSize:")==0 )
    {
	getsint(sfd,&sf->pfminfo.os2_supxsize);
    }
    else if ( strmatch(tok,"OS2SupYSize:")==0 )
    {
	getsint(sfd,&sf->pfminfo.os2_supysize);
    }
    else if ( strmatch(tok,"OS2SupXOff:")==0 )
    {
	getsint(sfd,&sf->pfminfo.os2_supxoff);
    }
    else if ( strmatch(tok,"OS2SupYOff:")==0 )
    {
	getsint(sfd,&sf->pfminfo.os2_supyoff);
    }
    else if ( strmatch(tok,"OS2StrikeYSize:")==0 )
    {
	getsint(sfd,&sf->pfminfo.os2_strikeysize);
    }
    else if ( strmatch(tok,"OS2StrikeYPos:")==0 )
    {
	getsint(sfd,&sf->pfminfo.os2_strikeypos);
    }
    else if ( strmatch(tok,"OS2CapHeight:")==0 )
    {
	getsint(sfd,&sf->pfminfo.os2_capheight);
    }
    else if ( strmatch(tok,"OS2XHeight:")==0 )
    {
	getsint(sfd,&sf->pfminfo.os2_xheight);
    }
    else if ( strmatch(tok,"OS2FamilyClass:")==0 )
    {
	getsint(sfd,&sf->pfminfo.os2_family_class);
    }
    else if ( strmatch(tok,"OS2Vendor:")==0 )
    {
	while ( isspace(nlgetc(sfd)));
	sf->pfminfo.os2_vendor[0] = nlgetc(sfd);
	sf->pfminfo.os2_vendor[1] = nlgetc(sfd);
	sf->pfminfo.os2_vendor[2] = nlgetc(sfd);
	sf->pfminfo.os2_vendor[3] = nlgetc(sfd);
	(void) nlgetc(sfd);
    }
    else if ( strmatch(tok,"OS2CodePages:")==0 )
    {
	gethexints(sfd,sf->pfminfo.codepages,2);
	sf->pfminfo.hascodepages = true;
    }
    else if ( strmatch(tok,"OS2UnicodeRanges:")==0 )
    {
	gethexints(sfd,sf->pfminfo.unicoderanges,4);
	sf->pfminfo.hasunicoderanges = true;
    }
    else if ( strmatch(tok,"TopEncoding:")==0 )
    {
	/* Obsolete */
	getint(sfd,&sf->top_enc);
    }
    else if ( strmatch(tok,"Ascent:")==0 )
    {
	getint(sfd,&sf->ascent);
    }
    else if ( strmatch(tok,"Descent:")==0 )
    {
	getint(sfd,&sf->descent);
    }
    else if ( strmatch(tok,"InvalidEm:")==0 )
    {
	getint(sfd,&sf->invalidem);
    }
    else if ( strmatch(tok,"woffMajor:")==0 )
    {
	getint(sfd,&sf->woffMajor);
    }
    else if ( strmatch(tok,"woffMinor:")==0 )
    {
	getint(sfd,&sf->woffMinor);
    }
    else if ( strmatch(tok,"woffMetadata:")==0 )
    {
	sf->woffMetadata = SFDReadUTF7Str(sfd);
    }
    else if ( strmatch(tok,"sfntRevision:")==0 )
    {
	gethex(sfd,(uint32 *)&sf->sfntRevision);
    }
    else if ( strmatch(tok,"LayerCount:")==0 )
    {
	d->had_layer_cnt = true;
	getint(sfd,&sf->layer_cnt);
	if ( sf->layer_cnt>2 ) {
	    sf->layers = realloc(sf->layers,sf->layer_cnt*sizeof(LayerInfo));
	    memset(sf->layers+2,0,(sf->layer_cnt-2)*sizeof(LayerInfo));
	}
    }
    else if ( strmatch(tok,"Layer:")==0 )
    {
        // TODO: Read the U. F. O. path.
	int layer, o2, bk;
	getint(sfd,&layer);
	if ( layer>=sf->layer_cnt ) {
	    sf->layers = realloc(sf->layers,(layer+1)*sizeof(LayerInfo));
	    memset(sf->layers+sf->layer_cnt,0,((layer+1)-sf->layer_cnt)*sizeof(LayerInfo));
	    sf->layer_cnt = layer+1;
	}
	getint(sfd,&o2);
	sf->layers[layer].order2 = o2;
	sf->layers[layer].background = layer==ly_back;
	/* Used briefly, now background is after layer name */
	while ( (ch=nlgetc(sfd))==' ' );
	ungetc(ch,sfd);
	if ( ch!='"' ) {
	    getint(sfd,&bk);
	    sf->layers[layer].background = bk;
	}
	/* end of section for obsolete format */
	sf->layers[layer].name = SFDReadUTF7Str(sfd);
	while ( (ch=nlgetc(sfd))==' ' );
	ungetc(ch,sfd);
	if ( ch!='\n' ) {
	    getint(sfd,&bk);
	    sf->layers[layer].background = bk;
	}
	while ( (ch=nlgetc(sfd))==' ' );
	ungetc(ch,sfd);
	if ( ch!='\n' ) { sf->layers[layer].ufo_path = SFDReadUTF7Str(sfd); }
    }
    else if ( strmatch(tok,"PreferredKerning:")==0 )
    {
	int temp;
	getint(sfd,&temp);
	sf->preferred_kerning = temp;
    }
    else if ( strmatch(tok,"StrokedFont:")==0 )
    {
	int temp;
	getint(sfd,&temp);
	sf->strokedfont = temp;
    }
    else if ( strmatch(tok,"MultiLayer:")==0 )
    {
	int temp;
	getint(sfd,&temp);
	sf->multilayer = temp;
    }
    else if ( strmatch(tok,"NeedsXUIDChange:")==0 )
    {
	int temp;
	getint(sfd,&temp);
	sf->changed_since_xuidchanged = temp;
    }
    else if ( strmatch(tok,"VerticalOrigin:")==0 )
    {
	// this doesn't seem to be written ever.
	int temp;
	getint(sfd,&temp);
	sf->hasvmetrics = true;
    }
    else if ( strmatch(tok,"HasVMetrics:")==0 )
    {
	int temp;
	getint(sfd,&temp);
	sf->hasvmetrics = temp;
    }
    else if ( strmatch(tok,"Justify:")==0 )
    {
	SFDParseJustify(sfd,sf,tok);
    }
    else if ( strmatch(tok,"BaseHoriz:")==0 )
    {
	sf->horiz_base = SFDParseBase(sfd);
	d->last_base = sf->horiz_base;
	d->last_base_script = NULL;
    }
    else if ( strmatch(tok,"BaseVert:")==0 )
    {
	sf->vert_base = SFDParseBase(sfd);
	d->last_base = sf->vert_base;
	d->last_base_script = NULL;
    }
    else if ( strmatch(tok,"BaseScript:")==0 )
    {
	struct basescript *bs = SFDParseBaseScript(sfd,d->last_base);
	if ( d->last_base==NULL )
	{
	    BaseScriptFree(bs);
	    bs = NULL;
	}
	else if ( d->last_base_script!=NULL )
	    d->last_base_script->next = bs;
	else
	    d->last_base->scripts = bs;
	d->last_base_script = bs;
    }
    else if ( strmatch(tok,"StyleMap:")==0 )
    {
    gethex(sfd,(uint32 *)&sf->pfminfo.stylemap);
    }
    /* Legacy attribute for StyleMap. Deprecated. */
    else if ( strmatch(tok,"OS2StyleName:")==0 )
    {
    char* sname = SFDReadUTF7Str(sfd);
    if (sf->pfminfo.stylemap == -1) {
        if (strcmp(sname,"bold italic")==0) sf->pfminfo.stylemap = 0x21;
        else if (strcmp(sname,"bold")==0) sf->pfminfo.stylemap = 0x20;
        else if (strcmp(sname,"italic")==0) sf->pfminfo.stylemap = 0x01;
        else if (strcmp(sname,"regular")==0) sf->pfminfo.stylemap = 0x40;
    }
    free(sname);
    }
    else if ( strmatch(tok,"FSType:")==0 )
    {
	getsint(sfd,&sf->pfminfo.fstype);
    }
    else if ( strmatch(tok,"OS2Version:")==0 )
    {
	getsint(sfd,&sf->os2_version);
    }
    else if ( strmatch(tok,"OS2_WeightWidthSlopeOnly:")==0 )
    {
	int temp;
	getint(sfd,&temp);
	sf->weight_width_slope_only = temp;
    }
    else if ( strmatch(tok,"OS2_UseTypoMetrics:")==0 )
    {
	int temp;
	getint(sfd,&temp);
	sf->use_typo_metrics = temp;
    }
    else if ( strmatch(tok,"UseUniqueID:")==0 )
    {
	int temp;
	getint(sfd,&temp);
	sf->use_uniqueid = temp;
    }
    else if ( strmatch(tok,"UseXUID:")==0 )
    {
	int temp;
	getint(sfd,&temp);
	sf->use_xuid = temp;
    }
    else if ( strmatch(tok,"UniqueID:")==0 )
    {
	getint(sfd,&sf->uniqueid);
    }
    else if ( strmatch(tok,"XUID:")==0 )
    {
	geteol(sfd,tok);
	sf->xuid = copy(tok);
    }
    else if ( strmatch(tok,"Lookup:")==0 )
    {
	OTLookup *otl;
	int temp;
	otl = chunkalloc(sizeof(OTLookup));
	getint(sfd,&temp); otl->lookup_type = temp;
	getint(sfd,&temp); otl->lookup_flags = temp;
	getint(sfd,&temp); otl->store_in_afm = temp;
	otl->lookup_name = SFDReadUTF7Str(sfd);
	if ( otl->lookup_type<gpos_single ) {
	    if ( d->lastsotl==NULL )
		sf->gsub_lookups = otl;
	    else
		d->lastsotl->next = otl;
	    d->lastsotl = otl;
	} else {
	    if ( d->lastpotl==NULL )
		sf->gpos_lookups = otl;
	    else
		d->lastpotl->next = otl;
	    d->lastpotl = otl;
	}
	SFDParseLookup(sfd,otl);
    }
    else if ( strmatch(tok,"MarkAttachClasses:")==0 )
    {
	getint(sfd,&sf->mark_class_cnt);
	sf->mark_classes = malloc(sf->mark_class_cnt*sizeof(char *));
	sf->mark_class_names = malloc(sf->mark_class_cnt*sizeof(char *));
	sf->mark_classes[0] = NULL; sf->mark_class_names[0] = NULL;
	for ( i=1; i<sf->mark_class_cnt; ++i )
	{
	    /* Class 0 is unused */
	    int temp;
	    while ( (temp=nlgetc(sfd))=='\n' || temp=='\r' ); ungetc(temp,sfd);
	    sf->mark_class_names[i] = SFDReadUTF7Str(sfd);
	    getint(sfd,&temp);
	    sf->mark_classes[i] = malloc(temp+1); sf->mark_classes[i][temp] = '\0';
	    nlgetc(sfd);	/* skip space */
	    fread(sf->mark_classes[i],1,temp,sfd);
	}
    }
    else if ( strmatch(tok,"MarkAttachSets:")==0 )
    {
	getint(sfd,&sf->mark_set_cnt);
	sf->mark_sets = malloc(sf->mark_set_cnt*sizeof(char *));
	sf->mark_set_names = malloc(sf->mark_set_cnt*sizeof(char *));
	for ( i=0; i<sf->mark_set_cnt; ++i )
	{
	    /* Set 0 is used */
	    int temp;
	    while ( (temp=nlgetc(sfd))=='\n' || temp=='\r' ); ungetc(temp,sfd);
	    sf->mark_set_names[i] = SFDReadUTF7Str(sfd);
	    getint(sfd,&temp);
	    sf->mark_sets[i] = malloc(temp+1); sf->mark_sets[i][temp] = '\0';
	    nlgetc(sfd);	/* skip space */
	    fread(sf->mark_sets[i],1,temp,sfd);
	}
    }
    else if ( strmatch(tok,"KernClass2:")==0 || strmatch(tok,"VKernClass2:")==0 ||
	      strmatch(tok,"KernClass:")==0 || strmatch(tok,"VKernClass:")==0 ||
	      strmatch(tok,"KernClass3:")==0 || strmatch(tok,"VKernClass3:")==0 )
    {
	int kernclassversion = 0;
	int isv = tok[0]=='V';
	int kcvoffset = (isv ? 10 : 9); //Offset to read kerning class version
	if (isdigit(tok[kcvoffset])) kernclassversion = tok[kcvoffset] - '0';
	int temp, classstart=1;
	int old = (kernclassversion == 0);

	if ( (sf->sfd_version<2)!=old ) {
	    IError( "Version mixup in Kerning Classes of sfd file." );
	    exit(1);
	}
	kc = chunkalloc(old ? sizeof(KernClass1) : sizeof(KernClass));
	getint(sfd,&kc->first_cnt);
	ch=nlgetc(sfd);
	if ( ch=='+' )
	    classstart = 0;
	else
	    ungetc(ch,sfd);
	getint(sfd,&kc->second_cnt);
	if ( old ) {
	    getint(sfd,&temp); ((KernClass1 *) kc)->sli = temp;
	    getint(sfd,&temp); ((KernClass1 *) kc)->flags = temp;
	} else {
	    kc->subtable = SFFindLookupSubtableAndFreeName(sf,SFDReadUTF7Str(sfd));
	    if ( kc->subtable!=NULL && kc->subtable->kc==NULL )
		kc->subtable->kc = kc;
	    else {
		if ( kc->subtable==NULL )
		    LogError(_("Bad SFD file, missing subtable in kernclass defn.\n") );
		else
		    LogError(_("Bad SFD file, two kerning classes assigned to the same subtable: %s\n"), kc->subtable->subtable_name );
		kc->subtable = NULL;
	    }
	}
	kc->firsts = calloc(kc->first_cnt,sizeof(char *));
	kc->seconds = calloc(kc->second_cnt,sizeof(char *));
	kc->offsets = calloc(kc->first_cnt*kc->second_cnt,sizeof(int16));
	kc->adjusts = calloc(kc->first_cnt*kc->second_cnt,sizeof(DeviceTable));
	if (kernclassversion >= 3) {
	  kc->firsts_flags = calloc(kc->first_cnt, sizeof(int));
	  kc->seconds_flags = calloc(kc->second_cnt, sizeof(int));
	  kc->offsets_flags = calloc(kc->first_cnt*kc->second_cnt, sizeof(int));
	  kc->firsts_names = calloc(kc->first_cnt, sizeof(char*));
	  kc->seconds_names = calloc(kc->second_cnt, sizeof(char*));
	}
	kc->firsts[0] = NULL;
	for ( i=classstart; i<kc->first_cnt; ++i ) {
	  if (kernclassversion < 3) {
	    getint(sfd,&temp);
	    kc->firsts[i] = malloc(temp+1); kc->firsts[i][temp] = '\0';
	    nlgetc(sfd);	/* skip space */
	    fread(kc->firsts[i],1,temp,sfd);
	  } else {
	    getint(sfd,&kc->firsts_flags[i]);
	    while ((ch=nlgetc(sfd)) == ' '); ungetc(ch, sfd); if (ch == '\n' || ch == EOF) continue;
	    kc->firsts_names[i] = SFDReadUTF7Str(sfd);
	    while ((ch=nlgetc(sfd)) == ' '); ungetc(ch, sfd); if (ch == '\n' || ch == EOF) continue;
	    kc->firsts[i] = SFDReadUTF7Str(sfd);
            if (kc->firsts[i] == NULL) kc->firsts[i] = copy(""); // In certain places, this must be defined.
	    while ((ch=nlgetc(sfd)) == ' ' || ch == '\n'); ungetc(ch, sfd);
	  }
	}
	kc->seconds[0] = NULL;
	for ( i=1; i<kc->second_cnt; ++i ) {
	  if (kernclassversion < 3) {
	    getint(sfd,&temp);
	    kc->seconds[i] = malloc(temp+1); kc->seconds[i][temp] = '\0';
	    nlgetc(sfd);	/* skip space */
	    fread(kc->seconds[i],1,temp,sfd);
	  } else {
	    getint(sfd,&temp);
	    kc->seconds_flags[i] = temp;
	    while ((ch=nlgetc(sfd)) == ' '); ungetc(ch, sfd); if (ch == '\n' || ch == EOF) continue;
	    kc->seconds_names[i] = SFDReadUTF7Str(sfd);
	    while ((ch=nlgetc(sfd)) == ' '); ungetc(ch, sfd); if (ch == '\n' || ch == EOF) continue;
	    kc->seconds[i] = SFDReadUTF7Str(sfd);
            if (kc->seconds[i] == NULL) kc->seconds[i] = copy(""); // In certain places, this must be defined.
	    while ((ch=nlgetc(sfd)) == ' ' || ch == '\n'); ungetc(ch, sfd);
	  }
	}
	for ( i=0; i<kc->first_cnt*kc->second_cnt; ++i ) {
	  if (kernclassversion >= 3) {
	    getint(sfd,&temp);
	    kc->offsets_flags[i] = temp;
	  }
	    getint(sfd,&temp);
	    kc->offsets[i] = temp;
	    SFDReadDeviceTable(sfd,&kc->adjusts[i]);
	}
	if ( !old && kc->subtable == NULL ) {
	    /* Error. Ignore it. Free it. Whatever */;
	} else if ( !isv ) {
	    if ( d->lastkc==NULL )
		sf->kerns = kc;
	    else
		d->lastkc->next = kc;
	    d->lastkc = kc;
	} else {
	    if ( d->lastvkc==NULL )
		sf->vkerns = kc;
	    else
		d->lastvkc->next = kc;
	    d->lastvkc = kc;
	}
    }
    else if ( strmatch(tok,"ContextPos2:")==0 || strmatch(tok,"ContextSub2:")==0 ||
	      strmatch(tok,"ChainPos2:")==0 || strmatch(tok,"ChainSub2:")==0 ||
	      strmatch(tok,"ReverseChain2:")==0 ||
	      strmatch(tok,"ContextPos:")==0 || strmatch(tok,"ContextSub:")==0 ||
	      strmatch(tok,"ChainPos:")==0 || strmatch(tok,"ChainSub:")==0 ||
	      strmatch(tok,"ReverseChain:")==0 )
    {
	FPST *fpst;
	int old;
	if ( strchr(tok,'2')!=NULL ) {
	    old = false;
	    fpst = chunkalloc(sizeof(FPST));
	} else {
	    old = true;
	    fpst = chunkalloc(sizeof(FPST1));
	}
	if ( (sf->sfd_version<2)!=old ) {
	    IError( "Version mixup in FPST of sfd file." );
	    exit(1);
	}
	if ( d->lastfp==NULL )
	    sf->possub = fpst;
	else
	    d->lastfp->next = fpst;
	d->lastfp = fpst;
	SFDParseChainContext(sfd,sf,fpst,tok,old);
    }
    else if ( strmatch(tok,"Group:")==0 ) {
        struct ff_glyphclasses *grouptmp = calloc(1, sizeof(struct ff_glyphclasses));
        while ((ch=nlgetc(sfd)) == ' '); ungetc(ch, sfd);
        grouptmp->classname = SFDReadUTF7Str(sfd);
        while ((ch=nlgetc(sfd)) == ' '); ungetc(ch, sfd);
        grouptmp->glyphs = SFDReadUTF7Str(sfd);
        while ((ch=nlgetc(sfd)) == ' ' || ch == '\n'); ungetc(ch, sfd);
        if (d->lastgroup != NULL) d->lastgroup->next = grouptmp; else sf->groups = grouptmp;
        d->lastgroup = grouptmp;
    }
    else if ( strmatch(tok,"GroupKern:")==0 ) {
        int temp = 0;
        struct ff_rawoffsets *kerntmp = calloc(1, sizeof(struct ff_rawoffsets));
        while ((ch=nlgetc(sfd)) == ' '); ungetc(ch, sfd);
        kerntmp->left = SFDReadUTF7Str(sfd);
        while ((ch=nlgetc(sfd)) == ' '); ungetc(ch, sfd);
        kerntmp->right = SFDReadUTF7Str(sfd);
        while ((ch=nlgetc(sfd)) == ' '); ungetc(ch, sfd);
        getint(sfd,&temp);
        kerntmp->offset = temp;
        while ((ch=nlgetc(sfd)) == ' ' || ch == '\n'); ungetc(ch, sfd);
        if (d->lastgroupkern != NULL) d->lastgroupkern->next = kerntmp; else sf->groupkerns = kerntmp;
        d->lastgroupkern = kerntmp;
    }
    else if ( strmatch(tok,"GroupVKern:")==0 ) {
        int temp = 0;
        struct ff_rawoffsets *kerntmp = calloc(1, sizeof(struct ff_rawoffsets));
        while ((ch=nlgetc(sfd)) == ' '); ungetc(ch, sfd);
        kerntmp->left = SFDReadUTF7Str(sfd);
        while ((ch=nlgetc(sfd)) == ' '); ungetc(ch, sfd);
        kerntmp->right = SFDReadUTF7Str(sfd);
        while ((ch=nlgetc(sfd)) == ' '); ungetc(ch, sfd);
        getint(sfd,&temp);
        kerntmp->offset = temp;
        while ((ch=nlgetc(sfd)) == ' ' || ch == '\n'); ungetc(ch, sfd);
        if (d->lastgroupvkern != NULL) d->lastgroupvkern->next = kerntmp; else sf->groupvkerns = kerntmp;
        d->lastgroupvkern = kerntmp;
    }
    else if ( strmatch(tok,"MacIndic2:")==0 || strmatch(tok,"MacContext2:")==0 ||
	      strmatch(tok,"MacLigature2:")==0 || strmatch(tok,"MacSimple2:")==0 ||
	      strmatch(tok,"MacKern2:")==0 || strmatch(tok,"MacInsert2:")==0 ||
	      strmatch(tok,"MacIndic:")==0 || strmatch(tok,"MacContext:")==0 ||
	      strmatch(tok,"MacLigature:")==0 || strmatch(tok,"MacSimple:")==0 ||
	      strmatch(tok,"MacKern:")==0 || strmatch(tok,"MacInsert:")==0 )
    {
	ASM *sm;
	if ( strchr(tok,'2')!=NULL ) {
	    old = false;
	    sm = chunkalloc(sizeof(ASM));
	} else {
	    old = true;
	    sm = chunkalloc(sizeof(ASM1));
	}
	if ( (sf->sfd_version<2)!=old ) {
	    IError( "Version mixup in state machine of sfd file." );
	    exit(1);
	}
	if ( d->lastsm==NULL )
	    sf->sm = sm;
	else
	    d->lastsm->next = sm;
	d->lastsm = sm;
	SFDParseStateMachine(sfd,sf,sm,tok,old);
    }
    else if ( strmatch(tok,"MacFeat:")==0 )
    {
	sf->features = SFDParseMacFeatures(sfd,tok);
    }
    else if ( strmatch(tok,"TtfTable:")==0 )
    {
	/* Old, binary format */
	/* still used for maxp and unknown tables */
	SFDGetTtfTable(sfd,sf,d->lastttf);
    }
    else if ( strmatch(tok,"TtTable:")==0 )
    {
	/* text instruction format */
	SFDGetTtTable(sfd,sf,d->lastttf);
    }


    ///////////////////

    else if ( strmatch(tok,"ShortTable:")==0 )
    {
	// only read, not written.
	/* text number format */
	SFDGetShortTable(sfd,sf,d->lastttf);
    }
    else
    {
        //
        // We didn't have a match ourselves.
        //
        return false;
    }
    return true;
}

static SplineFont *SFD_GetFont( FILE *sfd,SplineFont *cidmaster,char *tok,
				int fromdir, char *dirname, float sfdversion )
{
    SplineFont *sf;
    int realcnt, i, eof, mappos=-1, ch;
    struct table_ordering *lastord = NULL;
    struct axismap *lastaxismap = NULL;
    struct named_instance *lastnamedinstance = NULL;
    int pushedbacktok = false;
    Encoding *enc = &custom;
    struct remap *remap = NULL;
    int haddupenc;
    int old_style_order2 = false;
    int had_layer_cnt=false;

    orig_pos = 0;		/* Only used for compatibility with extremely old sfd files */

    sf = SplineFontEmpty();
    sf->sfd_version = sfdversion;
    sf->cidmaster = cidmaster;
    sf->uni_interp = ui_unset;
	SFD_GetFontMetaDataData d;
	SFD_GetFontMetaDataData_Init( &d );
    while ( 1 ) {
	if ( pushedbacktok )
	    pushedbacktok = false;
	else if ( (eof = getname(sfd,tok))!=1 ) {
	    if ( eof==-1 )
    break;
	    geteol(sfd,tok);
    continue;
	}


	bool wasMetadata = SFD_GetFontMetaData( sfd, tok, sf, &d );
	had_layer_cnt = d.had_layer_cnt;
        if( wasMetadata )
        {
            // we have handled the token entirely
            // inside SFD_GetFontMetaData() move to next token.
            continue;
        }


	if ( strmatch(tok,"DisplaySize:")==0 )
	{
	    getint(sfd,&sf->display_size);
	}
	else if ( strmatch(tok,"DisplayLayer:")==0 )
	{
	    getint(sfd,&sf->display_layer);
	}
	else if ( strmatch(tok,"ExtremaBound:")==0 )
	{
	    getint(sfd,&sf->extrema_bound);
	}
	else if ( strmatch(tok,"WidthSeparation:")==0 )
	{
	    getint(sfd,&sf->width_separation);
	}
	else if ( strmatch(tok,"WinInfo:")==0 )
	{
	    int temp1, temp2;
	    getint(sfd,&sf->top_enc);
	    getint(sfd,&temp1);
	    getint(sfd,&temp2);
	    if ( sf->top_enc<=0 ) sf->top_enc=-1;
	    if ( temp1<=0 ) temp1 = 16;
	    if ( temp2<=0 ) temp2 = 4;
	    sf->desired_col_cnt = temp1;
	    sf->desired_row_cnt = temp2;
	}
	else if ( strmatch(tok,"AntiAlias:")==0 )
	{
	    int temp;
	    getint(sfd,&temp);
	    sf->display_antialias = temp;
	}
	else if ( strmatch(tok,"FitToEm:")==0 )
	{
	    int temp;
	    getint(sfd,&temp);
	    sf->display_bbsized = temp;
	}
	else if ( strmatch(tok,"Order2:")==0 )
	{
	    getint(sfd,&old_style_order2);
	    sf->grid.order2 = old_style_order2;
	    sf->layers[ly_back].order2 = old_style_order2;
	    sf->layers[ly_fore].order2 = old_style_order2;
	}
	else if ( strmatch(tok,"GridOrder2:")==0 )
	{
	    int o2;
	    getint(sfd,&o2);
	    sf->grid.order2 = o2;
	}
	else if ( strmatch(tok,"Encoding:")==0 )
	{
	    enc = SFDGetEncoding(sfd,tok);
	    if ( sf->map!=NULL ) sf->map->enc = enc;
	}
	else if ( strmatch(tok,"OldEncoding:")==0 )
	{
	    /* old_encname =*/ (void) SFDGetEncoding(sfd,tok);
	}
	else if ( strmatch(tok,"UnicodeInterp:")==0 )
	{
	    sf->uni_interp = SFDGetUniInterp(sfd,tok,sf);
	}
	else if ( strmatch(tok,"NameList:")==0 )
	{
	    SFDGetNameList(sfd,tok,sf);
	}
	else if ( strmatch(tok,"Compacted:")==0 )
	{
	    int temp;
	    getint(sfd,&temp);
	    sf->compacted = temp;
	}
	else if ( strmatch(tok,"Registry:")==0 )
	{
	    geteol(sfd,tok);
	    sf->cidregistry = copy(tok);
	}


	//////////


	else if ( strmatch(tok,"Ordering:")==0 ) {
	    geteol(sfd,tok);
	    sf->ordering = copy(tok);
	} else if ( strmatch(tok,"Supplement:")==0 ) {
	    getint(sfd,&sf->supplement);
	} else if ( strmatch(tok,"RemapN:")==0 ) {
	    int n;
	    getint(sfd,&n);
	    remap = calloc(n+1,sizeof(struct remap));
	    remap[n].infont = -1;
	    mappos = 0;
	    if ( sf->map!=NULL ) sf->map->remap = remap;
	} else if ( strmatch(tok,"Remap:")==0 ) {
	    uint32 f, l; int p;
	    gethex(sfd,&f);
	    gethex(sfd,&l);
	    getint(sfd,&p);
	    if ( remap!=NULL && remap[mappos].infont!=-1 ) {
		remap[mappos].firstenc = f;
		remap[mappos].lastenc = l;
		remap[mappos].infont = p;
		mappos++;
	    }
	} else if ( strmatch(tok,"CIDVersion:")==0 ) {
	    real temp;
	    getreal(sfd,&temp);
	    sf->cidversion = temp;
	} else if ( strmatch(tok,"Grid")==0 ) {
	    sf->grid.splines = SFDGetSplineSet(sfd,sf->grid.order2);
	} else if ( strmatch(tok,"ScriptLang:")==0 ) {
	    int i,j,k;
	    int imax, jmax, kmax;
	    if ( sf->sfd_version==0 || sf->sfd_version>=2 ) {
		IError( "Script lang lists should not happen in version 2 sfd files." );
                SplineFontFree(sf);
                return NULL;
	    }
	    getint(sfd,&imax);
	    ((SplineFont1 *) sf)->sli_cnt = imax;
	    ((SplineFont1 *) sf)->script_lang = malloc((imax+1)*sizeof(struct script_record *));
	    ((SplineFont1 *) sf)->script_lang[imax] = NULL;
	    for ( i=0; i<imax; ++i ) {
		getint(sfd,&jmax);
		((SplineFont1 *) sf)->script_lang[i] = malloc((jmax+1)*sizeof(struct script_record));
		((SplineFont1 *) sf)->script_lang[i][jmax].script = 0;
		for ( j=0; j<jmax; ++j ) {
		    ((SplineFont1 *) sf)->script_lang[i][j].script = gettag(sfd);
		    getint(sfd,&kmax);
		    ((SplineFont1 *) sf)->script_lang[i][j].langs = malloc((kmax+1)*sizeof(uint32));
		    ((SplineFont1 *) sf)->script_lang[i][j].langs[kmax] = 0;
		    for ( k=0; k<kmax; ++k ) {
			((SplineFont1 *) sf)->script_lang[i][j].langs[k] = gettag(sfd);
		    }
		}
	    }
	} else if ( strmatch(tok,"TeXData:")==0 ) {
	    int temp;
	    getint(sfd,&temp);
	    sf->texdata.type = temp;
	    getint(sfd, &temp);
	    if ( sf->design_size==0 ) {
	    	sf->design_size = (5*temp+(1<<18))>>19;
	    }
	    for ( i=0; i<22; ++i ) {
		int foo;
		getint(sfd,&foo);
		sf->texdata.params[i]=foo;
	    }
	} else if ( strnmatch(tok,"AnchorClass",11)==0 ) {
	    char *name;
	    AnchorClass *lastan = NULL, *an;
	    int old = strchr(tok,'2')==NULL;
	    while ( (name=SFDReadUTF7Str(sfd))!=NULL ) {
		an = chunkalloc(old ? sizeof(AnchorClass1) : sizeof(AnchorClass));
		an->name = name;
		if ( old ) {
		    getname(sfd,tok);
		    if ( tok[0]=='0' && tok[1]=='\0' )
			((AnchorClass1 *) an)->feature_tag = 0;
		    else {
			if ( tok[1]=='\0' ) { tok[1]=' '; tok[2] = 0; }
			if ( tok[2]=='\0' ) { tok[2]=' '; tok[3] = 0; }
			if ( tok[3]=='\0' ) { tok[3]=' '; tok[4] = 0; }
			((AnchorClass1 *) an)->feature_tag = (tok[0]<<24) | (tok[1]<<16) | (tok[2]<<8) | tok[3];
		    }
		    while ( (ch=nlgetc(sfd))==' ' || ch=='\t' );
		    ungetc(ch,sfd);
		    if ( isdigit(ch)) {
			int temp;
			getint(sfd,&temp);
			((AnchorClass1 *) an)->flags = temp;
		    }
		    while ( (ch=nlgetc(sfd))==' ' || ch=='\t' );
		    ungetc(ch,sfd);
		    if ( isdigit(ch)) {
			int temp;
			getint(sfd,&temp);
			((AnchorClass1 *) an)->script_lang_index = temp;
		    } else
			((AnchorClass1 *) an)->script_lang_index = 0xffff;		/* Will be fixed up later */
		    while ( (ch=nlgetc(sfd))==' ' || ch=='\t' );
		    ungetc(ch,sfd);
		    if ( isdigit(ch)) {
			int temp;
			getint(sfd,&temp);
			((AnchorClass1 *) an)->merge_with = temp;
		    } else
			((AnchorClass1 *) an)->merge_with = 0xffff;			/* Will be fixed up later */
		} else {
                    char *subtable_name = SFDReadUTF7Str(sfd);
                    if ( subtable_name!=NULL)                                           /* subtable is optional */
		        an->subtable = SFFindLookupSubtableAndFreeName(sf,subtable_name);
                }
		while ( (ch=nlgetc(sfd))==' ' || ch=='\t' );
		ungetc(ch,sfd);
		if ( isdigit(ch) ) {
		    /* Early versions of SfdFormat 2 had a number here */
		    int temp;
		    getint(sfd,&temp);
		    an->type = temp;
		} else if ( old ) {
		    if ( ((AnchorClass1 *) an)->feature_tag==CHR('c','u','r','s'))
			an->type = act_curs;
		    else if ( ((AnchorClass1 *) an)->feature_tag==CHR('m','k','m','k'))
			an->type = act_mkmk;
		    else
			an->type = act_mark;
		} else {
		    an->type = act_mark;
		    if( an->subtable && an->subtable->lookup )
		    {
			switch ( an->subtable->lookup->lookup_type )
			{
			case gpos_cursive:
			    an->type = act_curs;
			    break;
			case gpos_mark2base:
			    an->type = act_mark;
			    break;
			case gpos_mark2ligature:
			    an->type = act_mklg;
			    break;
			case gpos_mark2mark:
			    an->type = act_mkmk;
			    break;
			default:
			    an->type = act_mark;
			    break;
			}
		    }
		}
		if ( lastan==NULL )
		    sf->anchor = an;
		else
		    lastan->next = an;
		lastan = an;
	    }
	} else if ( strncmp(tok,"MATH:",5)==0 ) {
	    SFDParseMathItem(sfd,sf,tok);
	} else if ( strmatch(tok,"TableOrder:")==0 ) {
	    int temp;
	    struct table_ordering *ord;
	    if ( sfdversion==0 || sfdversion>=2 ) {
		IError("Table ordering specified in version 2 sfd file.\n" );
                SplineFontFree(sf);
                return NULL;
	    }
	    ord = chunkalloc(sizeof(struct table_ordering));
	    ord->table_tag = gettag(sfd);
	    getint(sfd,&temp);
	    ord->ordered_features = malloc((temp+1)*sizeof(uint32));
	    ord->ordered_features[temp] = 0;
	    for ( i=0; i<temp; ++i ) {
		while ( isspace((ch=nlgetc(sfd))) );
		if ( ch=='\'' ) {
		    ungetc(ch,sfd);
		    ord->ordered_features[i] = gettag(sfd);
		} else if ( ch=='<' ) {
		    int f,s;
		    fscanf(sfd,"%d,%d>", &f, &s );
		    ord->ordered_features[i] = (f<<16)|s;
		}
	    }
	    if ( lastord==NULL )
		((SplineFont1 *) sf)->orders = ord;
	    else
		lastord->next = ord;
	    lastord = ord;
	} else if ( strmatch(tok,"BeginPrivate:")==0 ) {
	    SFDGetPrivate(sfd,sf);
	} else if ( strmatch(tok,"BeginSubrs:")==0 ) {	/* leave in so we don't croak on old sfd files */
	    SFDGetSubrs(sfd);
	} else if ( strmatch(tok,"PickledData:")==0 ) {
	    if (sf->python_persistent != NULL) {
#if defined(_NO_PYTHON)
	      free( sf->python_persistent );	/* It's a string of pickled data which we leave as a string */
#else
	      PyFF_FreePythonPersistent(sf->python_persistent);
#endif
	      sf->python_persistent = NULL;
	    }
	    sf->python_persistent = SFDUnPickle(sfd, 0);
	    sf->python_persistent_has_lists = 0;
	} else if ( strmatch(tok,"PickledDataWithLists:")==0 ) {
	    if (sf->python_persistent != NULL) {
#if defined(_NO_PYTHON)
	      free( sf->python_persistent );	/* It's a string of pickled data which we leave as a string */
#else
	      PyFF_FreePythonPersistent(sf->python_persistent);
#endif
	      sf->python_persistent = NULL;
	    }
	    sf->python_persistent = SFDUnPickle(sfd, 1);
	    sf->python_persistent_has_lists = 1;
	} else if ( strmatch(tok,"MMCounts:")==0 ) {
	    MMSet *mm = sf->mm = chunkalloc(sizeof(MMSet));
	    getint(sfd,&mm->instance_count);
	    getint(sfd,&mm->axis_count);
	    ch = nlgetc(sfd);
	    if ( ch!=' ' )
		ungetc(ch,sfd);
	    else { int temp;
		getint(sfd,&temp);
		mm->apple = temp;
		getint(sfd,&mm->named_instance_count);
	    }
	    mm->instances = calloc(mm->instance_count,sizeof(SplineFont *));
	    mm->positions = malloc(mm->instance_count*mm->axis_count*sizeof(real));
	    mm->defweights = malloc(mm->instance_count*sizeof(real));
	    mm->axismaps = calloc(mm->axis_count,sizeof(struct axismap));
	    if ( mm->named_instance_count!=0 )
		mm->named_instances = calloc(mm->named_instance_count,sizeof(struct named_instance));
	} else if ( strmatch(tok,"MMAxis:")==0 ) {
	    MMSet *mm = sf->mm;
	    if ( mm!=NULL ) {
		for ( i=0; i<mm->axis_count; ++i ) {
		    getname(sfd,tok);
		    mm->axes[i] = copy(tok);
		}
	    }
	} else if ( strmatch(tok,"MMPositions:")==0 ) {
	    MMSet *mm = sf->mm;
	    if ( mm!=NULL ) {
		for ( i=0; i<mm->axis_count*mm->instance_count; ++i )
		    getreal(sfd,&mm->positions[i]);
	    }
	} else if ( strmatch(tok,"MMWeights:")==0 ) {
	    MMSet *mm = sf->mm;
	    if ( mm!=NULL ) {
		for ( i=0; i<mm->instance_count; ++i )
		    getreal(sfd,&mm->defweights[i]);
	    }
	} else if ( strmatch(tok,"MMAxisMap:")==0 ) {
	    MMSet *mm = sf->mm;
	    if ( mm!=NULL ) {
		int index, points;
		getint(sfd,&index); getint(sfd,&points);
		mm->axismaps[index].points = points;
		mm->axismaps[index].blends = malloc(points*sizeof(real));
		mm->axismaps[index].designs = malloc(points*sizeof(real));
		for ( i=0; i<points; ++i ) {
		    getreal(sfd,&mm->axismaps[index].blends[i]);
		    while ( (ch=nlgetc(sfd))!=EOF && isspace(ch));
		    ungetc(ch,sfd);
		    if ( (ch=nlgetc(sfd))!='=' )
			ungetc(ch,sfd);
		    else if ( (ch=nlgetc(sfd))!='>' )
			ungetc(ch,sfd);
		    getreal(sfd,&mm->axismaps[index].designs[i]);
		}
		lastaxismap = &mm->axismaps[index];
		lastnamedinstance = NULL;
	    }
	} else if ( strmatch(tok,"MMNamedInstance:")==0 ) {
	    MMSet *mm = sf->mm;
	    if ( mm!=NULL ) {
		int index;
		getint(sfd,&index);
		mm->named_instances[index].coords = malloc(mm->axis_count*sizeof(real));
		for ( i=0; i<mm->axis_count; ++i )
		    getreal(sfd,&mm->named_instances[index].coords[i]);
		lastnamedinstance = &mm->named_instances[index];
		lastaxismap = NULL;
	    }
	} else if ( strmatch(tok,"MacName:")==0 ) {
	    struct macname *names = SFDParseMacNames(sfd,tok);
	    if ( lastaxismap!=NULL )
		lastaxismap->axisnames = names;
	    else if ( lastnamedinstance !=NULL )
		lastnamedinstance->names = names;
	    pushedbacktok = true;
	} else if ( strmatch(tok,"MMCDV:")==0 ) {
	    MMSet *mm = sf->mm;
	    if ( mm!=NULL )
		mm->cdv = SFDParseMMSubroutine(sfd);
	} else if ( strmatch(tok,"MMNDV:")==0 ) {
	    MMSet *mm = sf->mm;
	    if ( mm!=NULL )
		mm->ndv = SFDParseMMSubroutine(sfd);
	} else if ( strmatch(tok,"BeginMMFonts:")==0 ) {
	    int cnt;
	    getint(sfd,&cnt);
	    getint(sfd,&realcnt);
	    ff_progress_change_stages(cnt);
	    ff_progress_change_total(realcnt);
	    MMInferStuff(sf->mm);
    break;
	} else if ( strmatch(tok,"BeginSubFonts:")==0 ) {
	    getint(sfd,&sf->subfontcnt);
	    sf->subfonts = calloc(sf->subfontcnt,sizeof(SplineFont *));
	    getint(sfd,&realcnt);
	    sf->map = EncMap1to1(realcnt);
	    ff_progress_change_stages(2);
	    ff_progress_change_total(realcnt);
    break;
	} else if ( strmatch(tok,"BeginChars:")==0 ) {
	    int charcnt;
	    getint(sfd,&charcnt);
	    if (charcnt<enc->char_cnt) {
		IError("SFD file specifies too few slots for its encoding.\n" );
exit( 1 );
	    }
	    if ( getint(sfd,&realcnt)!=1 || realcnt==-1 )
		realcnt = charcnt;
	    else
		++realcnt;		/* value saved is max glyph, not glyph cnt */
	    ff_progress_change_total(realcnt);
	    sf->glyphcnt = sf->glyphmax = realcnt;
	    sf->glyphs = calloc(realcnt,sizeof(SplineChar *));
	    if ( cidmaster!=NULL ) {
		sf->map = cidmaster->map;
	    } else {
		sf->map = EncMapNew(charcnt,realcnt,enc);
		sf->map->remap = remap;
	    }
	    SFDSizeMap(sf->map,sf->glyphcnt,charcnt);
    break;
#if HANYANG
	} else if ( strmatch(tok,"BeginCompositionRules")==0 ) {
	    sf->rules = SFDReadCompositionRules(sfd);
#endif
	} else {
	    /* If we don't understand it, skip it */
	    geteol(sfd,tok);
	}
    }

    if ( sf->subfontcnt!=0 ) {
	ff_progress_change_stages(2*sf->subfontcnt);
	for ( i=0; i<sf->subfontcnt; ++i ) {
	    if ( i!=0 )
		ff_progress_next_stage();
	    sf->subfonts[i] = SFD_GetFont(sfd,sf,tok,fromdir,dirname,sfdversion);
	}
    } else if ( sf->mm!=NULL ) {
	MMSet *mm = sf->mm;
	ff_progress_change_stages(2*(mm->instance_count+1));
	for ( i=0; i<mm->instance_count; ++i ) {
	    if ( i!=0 )
		ff_progress_next_stage();
	    mm->instances[i] = SFD_GetFont(sfd,NULL,tok,fromdir,dirname,sfdversion);
	    EncMapFree(mm->instances[i]->map); mm->instances[i]->map=NULL;
	    mm->instances[i]->mm = mm;
	}
	ff_progress_next_stage();
	mm->normal = SFD_GetFont(sfd,NULL,tok,fromdir,dirname,sfdversion);
	mm->normal->mm = mm;
	sf->mm = NULL;
	SplineFontFree(sf);
	sf = mm->normal;
	if ( sf->map->enc!=&custom ) {
	    EncMap *map;
	    MMMatchGlyphs(mm);		/* sfd files from before the encoding change can have mismatched orig pos */
	    map = EncMapFromEncoding(sf,sf->map->enc);
	    EncMapFree(sf->map);
	    sf->map = map;
	}
    } else {
	while ( SFDGetChar(sfd,sf,had_layer_cnt)!=NULL ) {
	    ff_progress_next();
	}
	ff_progress_next_stage();
    }
    haddupenc = false;
    while ( getname(sfd,tok)==1 ) {
	if ( strcmp(tok,"EndSplineFont")==0 || strcmp(tok,"EndSubSplineFont")==0 )
    break;
	else if ( strmatch(tok,"DupEnc:")==0 ) {
	    int enc, orig;
	    haddupenc = true;
	    if ( getint(sfd,&enc) && getint(sfd,&orig) && sf->map!=NULL ) {
		SFDSetEncMap(sf,orig,enc);
	    }
	}
    }
    if ( sf->cidmaster==NULL )
	SFDFixupRefs(sf);

    if ( !haddupenc )
	SFD_DoAltUnis(sf);
    else
	AltUniFigure(sf,sf->map,true);
    if ( !d.hadtimes )
	SFTimesFromFile(sf,sfd);
return( sf );
}

void SFTimesFromFile(SplineFont *sf,FILE *file) {
    struct stat b;
    if ( fstat(fileno(file),&b)!=-1 ) {
	sf->modificationtime = b.st_mtime;
	sf->creationtime = b.st_mtime;
    }
}

static double SFDStartsCorrectly(FILE *sfd,char *tok) {
    real dval;
    int ch;

    if ( getname(sfd,tok)!=1 )
return( -1 );
    if ( strcmp(tok,"SplineFontDB:")!=0 )
return( -1 );
    if ( getreal(sfd,&dval)!=1 )
return( -1 );
    /* We don't yet generate version 4 of sfd. It will contain backslash */
    /*  newline in the middle of very long lines. I've put in code to parse */
    /*  this sequence, but I don't yet generate it. I want the parser to */
    /*  perculate through to users before I introduce the new format so there */
    /*  will be fewer complaints when it happens */
    // MIQ: getreal() can give some funky rounding errors it seems
    if ( dval!=2.0 && dval!=3.0
         && !(dval > 3.09 && dval <= 3.11)
         && dval!=4.0 )
    {
        LogError("Bad SFD Version number %.1f", dval );
return( -1 );
    }
    ch = nlgetc(sfd); ungetc(ch,sfd);
    if ( ch!='\r' && ch!='\n' )
return( -1 );

return( dval );
}

static SplineFont *SFD_Read(char *filename,FILE *sfd, int fromdir) {
    SplineFont *sf=NULL;
    char tok[2000];
    double version;

    if ( sfd==NULL ) {
	if ( fromdir ) {
	    snprintf(tok,sizeof(tok),"%s/" FONT_PROPS, filename );
	    sfd = fopen(tok,"r");
	} else
	    sfd = fopen(filename,"r");
    }
    if ( sfd==NULL )
return( NULL );
    locale_t tmplocale; locale_t oldlocale; // Declare temporary locale storage.
    switch_to_c_locale(&tmplocale, &oldlocale); // Switch to the C locale temporarily and cache the old locale.
    ff_progress_change_stages(2);
    if ( (version = SFDStartsCorrectly(sfd,tok))!=-1 )
	sf = SFD_GetFont(sfd,NULL,tok,fromdir,filename,version);
    switch_to_old_locale(&tmplocale, &oldlocale); // Switch to the cached locale.
    if ( sf!=NULL ) {
	sf->filename = copy(filename);
	if ( sf->mm!=NULL ) {
	    int i;
	    for ( i=0; i<sf->mm->instance_count; ++i )
		sf->mm->instances[i]->filename = copy(filename);
	}
    }
    fclose(sfd);
return( sf );
}

SplineFont *SFDRead(char *filename) {
return( SFD_Read(filename,NULL,false));
}
