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
#include "pfaedit.h"
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <locale.h>
#include <utype.h>
#include <ustring.h>
#include <chardata.h>

#ifdef __CygWin
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>
#endif

#include "ttf.h"

char *TTFFoundry=NULL;

/* This file produces a ttf file given a splinefont. */

/* ************************************************************************** */

/* Required tables:
	cmap		encoding
	head		header data
	hhea		horizontal metrics header data
	hmtx		horizontal metrics (widths, lsidebearing)
	maxp		various maxima in the font
	name		various names associated with the font
	post		postscript names and other stuff
Required by windows but not mac
	OS/2		bleah. 
Required for TrueType
	loca		pointers to the glyphs
	glyf		character shapes
Required for OpenType (Postscript)
	CFF 		A complete postscript CFF font here with all its internal tables
Required for bitmaps
	bdat/EBDT	bitmap data
	bloc/EBLC	pointers to bitmaps
	bhed		for apple bitmap only fonts, replaces head
Optional for bitmaps
	EBSC		bitmap scaling table (used in windows "bitmap-only" fonts)
OpenType
	GPOS		(opentype, if kern,anchor data are present)
	GSUB		(opentype, if ligature (other subs) data are present)
	GDEF		(opentype, if anchor data are present)
MATH
	MATH		(MS proposal, if math data present)
additional tables
	cvt		for hinting
	gasp		to control when things should be hinted
	fpgm		for hinting (currently only copied and dumped verbatim)
	prep		for hinting (currently only copied and dumped verbatim)
FontForge
	PfEd		My own table
TeX
	TeX		TeX specific info (stuff that used to live in tfm files)
*/

const char *ttfstandardnames[258] = {
".notdef",
".null",
"nonmarkingreturn",
"space",
"exclam",
"quotedbl",
"numbersign",
"dollar",
"percent",
"ampersand",
"quotesingle",
"parenleft",
"parenright",
"asterisk",
"plus",
"comma",
"hyphen",
"period",
"slash",
"zero",
"one",
"two",
"three",
"four",
"five",
"six",
"seven",
"eight",
"nine",
"colon",
"semicolon",
"less",
"equal",
"greater",
"question",
"at",
"A",
"B",
"C",
"D",
"E",
"F",
"G",
"H",
"I",
"J",
"K",
"L",
"M",
"N",
"O",
"P",
"Q",
"R",
"S",
"T",
"U",
"V",
"W",
"X",
"Y",
"Z",
"bracketleft",
"backslash",
"bracketright",
"asciicircum",
"underscore",
"grave",
"a",
"b",
"c",
"d",
"e",
"f",
"g",
"h",
"i",
"j",
"k",
"l",
"m",
"n",
"o",
"p",
"q",
"r",
"s",
"t",
"u",
"v",
"w",
"x",
"y",
"z",
"braceleft",
"bar",
"braceright",
"asciitilde",
"Adieresis",
"Aring",
"Ccedilla",
"Eacute",
"Ntilde",
"Odieresis",
"Udieresis",
"aacute",
"agrave",
"acircumflex",
"adieresis",
"atilde",
"aring",
"ccedilla",
"eacute",
"egrave",
"ecircumflex",
"edieresis",
"iacute",
"igrave",
"icircumflex",
"idieresis",
"ntilde",
"oacute",
"ograve",
"ocircumflex",
"odieresis",
"otilde",
"uacute",
"ugrave",
"ucircumflex",
"udieresis",
"dagger",
"degree",
"cent",
"sterling",
"section",
"bullet",
"paragraph",
"germandbls",
"registered",
"copyright",
"trademark",
"acute",
"dieresis",
"notequal",
"AE",
"Oslash",
"infinity",
"plusminus",
"lessequal",
"greaterequal",
"yen",
"mu",
"partialdiff",
"summation",
"product",
"pi",
"integral",
"ordfeminine",
"ordmasculine",
"Omega",
"ae",
"oslash",
"questiondown",
"exclamdown",
"logicalnot",
"radical",
"florin",
"approxequal",
"Delta",
"guillemotleft",
"guillemotright",
"ellipsis",
"nonbreakingspace",
"Agrave",
"Atilde",
"Otilde",
"OE",
"oe",
"endash",
"emdash",
"quotedblleft",
"quotedblright",
"quoteleft",
"quoteright",
"divide",
"lozenge",
"ydieresis",
"Ydieresis",
"fraction",
"currency",
"guilsinglleft",
"guilsinglright",
"fi",
"fl",
"daggerdbl",
"periodcentered",
"quotesinglbase",
"quotedblbase",
"perthousand",
"Acircumflex",
"Ecircumflex",
"Aacute",
"Edieresis",
"Egrave",
"Iacute",
"Icircumflex",
"Idieresis",
"Igrave",
"Oacute",
"Ocircumflex",
"apple",
"Ograve",
"Uacute",
"Ucircumflex",
"Ugrave",
"dotlessi",
"circumflex",
"tilde",
"macron",
"breve",
"dotaccent",
"ring",
"cedilla",
"hungarumlaut",
"ogonek",
"caron",
"Lslash",
"lslash",
"Scaron",
"scaron",
"Zcaron",
"zcaron",
"brokenbar",
"Eth",
"eth",
"Yacute",
"yacute",
"Thorn",
"thorn",
"minus",
"multiply",
"onesuperior",
"twosuperior",
"threesuperior",
"onehalf",
"onequarter",
"threequarters",
"franc",
"Gbreve",
"gbreve",
"Idotaccent",
"Scedilla",
"scedilla",
"Cacute",
"cacute",
"Ccaron",
"ccaron",
"dcroat"
};


void putshort(FILE *file,int sval) {
    putc((sval>>8)&0xff,file);
    putc(sval&0xff,file);
}

void putlong(FILE *file,int val) {
    putc((val>>24)&0xff,file);
    putc((val>>16)&0xff,file);
    putc((val>>8)&0xff,file);
    putc(val&0xff,file);
}
#define dumpabsoffset	putlong

static void dumpoffset(FILE *file,int offsize,int val) {
    if ( offsize==1 )
	putc(val,file);
    else if ( offsize==2 )
	putshort(file,val);
    else if ( offsize==3 ) {
	putc((val>>16)&0xff,file);
	putc((val>>8)&0xff,file);
	putc(val&0xff,file);
    } else
	putlong(file,val);
}

void putfixed(FILE *file,real dval) {
    int val;
    int mant;

    val = floor(dval);
    mant = floor(65536.*(dval-val));
    val = (val<<16) | mant;
    putlong(file,val);
}

int ttfcopyfile(FILE *ttf, FILE *other, int pos, char *tab_name) {
    int ch;
    int ret = 1;

    if ( ferror(ttf) || ferror(other)) {
	IError("Disk error of some nature. Perhaps no space on device?\nGenerated font will be unusable" );
    } else if ( pos!=ftell(ttf)) {
	IError("File Offset wrong for ttf table (%s), %d expected %d", tab_name, ftell(ttf), pos );
    }
    rewind(other);
    while (( ch = getc(other))!=EOF )
	putc(ch,ttf);
    if ( ferror(other)) ret = 0;
    if ( fclose(other)) ret = 0;
return( ret );
}

static void FigureFullMetricsEnd(SplineFont *sf,struct glyphinfo *gi, int istt ) {
    /* We can reduce the size of the width array by removing a run at the end */
    /*  of the same width. So start at the end, find the width of the last */
    /*  character we'll output, then run backwards as long as we've got the */
    /*  same width */
    /* (do same thing for vertical metrics too */
    int i, lasti, lastv, lastdefault = istt ? 3 : 1;
    int width, vwidth;

    lasti = lastv = gi->gcnt-1;
    for ( i=gi->gcnt-1; i>lastdefault && gi->bygid[i]==-1; --i );
    if ( i>=lastdefault ) {
	width = sf->glyphs[gi->bygid[i]]->width;
	vwidth = sf->glyphs[gi->bygid[i]]->vwidth;
	lasti = lastv = i;
	for ( i=lasti-1; i>=lastdefault; --i ) {
	    if ( SCWorthOutputting(sf->glyphs[gi->bygid[i]]) ) {
		if ( sf->glyphs[gi->bygid[i]]->width!=width )
	break;
		else
		    lasti = i;
	    }
	}
	gi->lasthwidth = lasti;
	if ( sf->hasvmetrics ) {
	    for ( i=lastv-1; i>=lastdefault; --i ) {
		if ( SCWorthOutputting(sf->glyphs[gi->bygid[i]]) ) {
		    if ( sf->glyphs[gi->bygid[i]]->vwidth!=vwidth )
	    break;
		    else
			lastv = i;
		}
	    }
	    gi->lastvwidth = lastv;
	}
    } else {
	gi->lasthwidth = 0;
	gi->lastvwidth = 0;
    }
}


#define _On_Curve	1
#define _X_Short	2
#define _Y_Short	4
#define _Repeat		8
#define _X_Same		0x10
#define _Y_Same		0x20

int RefDepth(RefChar *ref,int layer) {
    int rd, temp;
    SplineChar *sc = ref->sc;

    if ( sc->layers[layer].refs==NULL || sc->layers[layer].splines!=NULL )
return( 1 );
    rd = 0;
    for ( ref = sc->layers[layer].refs; ref!=NULL; ref=ref->next ) {
	if ( ref->transform[0]>=-2 || ref->transform[0]<=1.999939 ||
		ref->transform[1]>=-2 || ref->transform[1]<=1.999939 ||
		ref->transform[2]>=-2 || ref->transform[2]<=1.999939 ||
		ref->transform[3]>=-2 || ref->transform[3]<=1.999939 ) {
	    temp = RefDepth(ref,layer);
	    if ( temp>rd ) rd = temp;
	}
    }
return( rd+1 );
}

void SFDummyUpCIDs(struct glyphinfo *gi,SplineFont *sf) {
    int i,j,k,max;
    int *bygid;

    max = 0;
    for ( k=0; k<sf->subfontcnt; ++k )
	if ( sf->subfonts[k]->glyphcnt>max ) max = sf->subfonts[k]->glyphcnt;
    if ( max == 0 )
return;

    sf->glyphs = gcalloc(max,sizeof(SplineChar *));
    sf->glyphcnt = sf->glyphmax = max;
    sf->glyphmin = 0;
    for ( k=0; k<sf->subfontcnt; ++k )
	for ( i=0; i<sf->subfonts[k]->glyphcnt; ++i ) if ( sf->subfonts[k]->glyphs[i]!=NULL )
	    sf->glyphs[i] = sf->subfonts[k]->glyphs[i];

    if ( gi==NULL )
return;

    bygid = galloc((sf->glyphcnt+3)*sizeof(int));
    memset(bygid,0xff, (sf->glyphcnt+3)*sizeof(int));

    j=1;
    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL ) {
	if ( bygid[0]== -1 && strcmp(sf->glyphs[i]->name,".notdef")==0 ) {
	    sf->glyphs[i]->ttf_glyph = 0;
	    bygid[0] = i;
	} else if ( SCWorthOutputting(sf->glyphs[i])) {
	    sf->glyphs[i]->ttf_glyph = j;
	    bygid[j++] = i;
	}
    }
    gi->bygid = bygid;
    gi->gcnt = j;
}

static void AssignNotdefNull(SplineFont *sf,int *bygid, int iscff) {
    int i;

    /* The first three glyphs are magic, glyph 0 is .notdef */
    /*  glyph 1 is .null and glyph 2 is nonmarking return */
    /*  We may generate them automagically */

    bygid[0] = bygid[1] = bygid[2] = -1;
    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL ) {
	if ( bygid[0]== -1 && strcmp(sf->glyphs[i]->name,".notdef")==0 ) {
	    sf->glyphs[i]->ttf_glyph = 0;
	    bygid[0] = i;
	} else if ( !iscff && bygid[1]== -1 &&
		(strcmp(sf->glyphs[i]->name,".null")==0 ||
		 strcmp(sf->glyphs[i]->name,"uni0000")==0 ||
		 (i==1 && strcmp(sf->glyphs[1]->name,"glyph1")==0)) ) {
	    sf->glyphs[i]->ttf_glyph = 1;
	    bygid[1] = i;
	} else if ( !iscff && bygid[2]== -1 &&
		(strcmp(sf->glyphs[i]->name,"nonmarkingreturn")==0 ||
		 strcmp(sf->glyphs[i]->name,"uni000D")==0 ||
		 (i==2 && strcmp(sf->glyphs[2]->name,"glyph2")==0)) ) {
	    sf->glyphs[i]->ttf_glyph = 2;
	    bygid[2] = i;
	}
    }
}

static int AssignTTFGlyph(struct glyphinfo *gi,SplineFont *sf,EncMap *map,int iscff) {
    int *bygid = galloc((sf->glyphcnt+3)*sizeof(int));
    int i,j;

    memset(bygid,0xff, (sf->glyphcnt+3)*sizeof(int));

    AssignNotdefNull(sf,bygid,iscff);

    j = iscff ? 1 : 3;
    for ( i=0; i<map->enccount; ++i ) if ( map->map[i]!=-1 ) {
	SplineChar *sc = sf->glyphs[map->map[i]];
	if ( SCWorthOutputting(sc) && sc->ttf_glyph==-1
#if HANYANG
		&& (!iscff || !sc->compositionunit)
#endif
	) {
	    sc->ttf_glyph = j;
	    bygid[j++] = sc->orig_pos;
	}
    }

    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL ) {
	SplineChar *sc = sf->glyphs[i];
	if ( SCWorthOutputting(sc) && sc->ttf_glyph==-1 
#if HANYANG
		&& (!iscff || !sc->compositionunit)
#endif
	) {
	    sc->ttf_glyph = j;
	    bygid[j++] = i;
	}
    }
    gi->bygid = bygid;
    gi->gcnt = j;
return j;
}

/* Standard names for cff */
extern const char *cffnames[];
extern const int nStdStrings;

static int storesid(struct alltabs *at,char *str) {
    int i;
    FILE *news;
    char *pt;
    long pos;

    if ( str!=NULL ) {			/* NULL is the magic string at end of array */
	for ( i=0; cffnames[i]!=NULL; ++i ) {
	    if ( strcmp(cffnames[i],str)==0 )
return( i );
	}
    }

    pos = ftell(at->sidf)+1;
    if ( pos>=65536 && !at->sidlongoffset ) {
	at->sidlongoffset = true;
	news = tmpfile();
	rewind(at->sidh);
	for ( i=0; i<at->sidcnt; ++i )
	    putlong(news,getushort(at->sidh));
	fclose(at->sidh);
	at->sidh = news;
    }
    if ( at->sidlongoffset )
	putlong(at->sidh,pos);
    else
	putshort(at->sidh,pos);

    if ( str!=NULL ) {
	for ( pt=str; *pt; ++pt )
	    putc(*pt,at->sidf);
    }
return( at->sidcnt++ + nStdStrings );
}

static void dumpint(FILE *cfff,int num) {

    if ( num>=-107 && num<=107 )
	putc(num+139,cfff);
    else if ( num>=108 && num<=1131 ) {
	num -= 108;
	putc((num>>8)+247,cfff);
	putc(num&0xff,cfff);
    } else if ( num>=-1131 && num<=-108 ) {
	num = -num;
	num -= 108;
	putc((num>>8)+251,cfff);
	putc(num&0xff,cfff);
    } else if ( num>=-32768 && num<32768 ) {
	putc(28,cfff);
	putc(num>>8,cfff);
	putc(num&0xff,cfff);
    } else {		/* In dict data we have 4 byte ints, in type2 strings we don't */
	putc(29,cfff);
	putc((num>>24)&0xff,cfff);
	putc((num>>16)&0xff,cfff);
	putc((num>>8)&0xff,cfff);
	putc(num&0xff,cfff);
    }
}

static void dumpdbl(FILE *cfff,double d) {
    if ( d-rint(d)>-.00001 && d-rint(d)<.00001 )
	dumpint(cfff,(int) d);
    else {
	/* The type2 strings have a fixed format, but the dict data does not */
	char buffer[20], *pt;
	int sofar,n,odd;
	sprintf( buffer, "%g", d);
	sofar = 0; odd=true;
	putc(30,cfff);		/* Start a double */
	for ( pt=buffer; *pt; ++pt ) {
	    if ( isdigit(*pt) )
		n = *pt-'0';
	    else if ( *pt=='.' )
		n = 0xa;
	    else if ( *pt=='-' )
		n = 0xe;
	    else if (( *pt=='E' || *pt=='e') && pt[1]=='-' ) {
		n = 0xc;
		++pt;
	    } else if ( *pt=='E' || *pt=='e')
		n = 0xb;
	    else
		n = 0;		/* Should never happen */
	    if ( odd ) {
		sofar = n<<4;
		odd = false;
	    } else {
		putc(sofar|n,cfff);
		sofar=0;
		odd = true;
	    }
	}
	if ( sofar==0 )
	    putc(0xff,cfff);
	else
	    putc(sofar|0xf,cfff);
    }
}

static void dumpoper(FILE *cfff,int oper ) {
    if ( oper!=-1 ) {
	if ( oper>=256 )
	    putc(oper>>8,cfff);
	putc(oper&0xff,cfff);
    }
}

static void dumpdbloper(FILE *cfff,double d, int oper ) {
    dumpdbl(cfff,d);
    dumpoper(cfff,oper);
}

static void dumpintoper(FILE *cfff,int v, int oper ) {
    dumpint(cfff,v);
    dumpoper(cfff,oper);
}

static void dumpsizedint(FILE *cfff,int big,int num, int oper ) {
    if ( big ) {
	putc(29,cfff);
	putc((num>>24)&0xff,cfff);
	putc((num>>16)&0xff,cfff);
	putc((num>>8)&0xff,cfff);
	putc(num&0xff,cfff);
    } else {
	putc(28,cfff);
	putc(num>>8,cfff);
	putc(num&0xff,cfff);
    }
    dumpoper(cfff,oper);
}

static void dumpsid(FILE *cfff,struct alltabs *at,char *str,int oper) {
    if ( str==NULL )
return;
    dumpint(cfff,storesid(at,str));
    dumpoper(cfff,oper);
}

static void DumpStrDouble(char *pt,FILE *cfff,int oper) {
    real d;
    if ( *pt=='[' ) ++pt;		/* For StdHW, StdVW */
    d = strtod(pt,NULL);
    dumpdbloper(cfff,d,oper);
}

static void DumpDblArray(real *arr,int n,FILE *cfff, int oper) {
    int mi,i;

    for ( mi=n-1; mi>=0 && arr[mi]==0; --mi );
    if ( mi<0 )
return;
    dumpdbl(cfff,arr[0]);
    for ( i=1; i<=mi; ++i )
	dumpdbl(cfff,arr[i]-arr[i-1]);
    dumpoper(cfff,oper);
}

static void DumpStrArray(char *pt,FILE *cfff,int oper) {
    real d, last=0;
    char *end;

    while ( *pt==' ' ) ++pt;
    if ( *pt=='\0' )
return;
    if ( *pt=='[' ) ++pt;
    while ( *pt==' ' ) ++pt;
    while ( *pt!=']' && *pt!='\0' ) {
	d = strtod(pt,&end);
	if ( pt==end )		/* User screwed up. Should be a number */
    break;
	dumpdbl(cfff,d-last);
	last = d;
	pt = end;
	while ( *pt==' ' ) ++pt;
    }
    dumpoper(cfff,oper);
}

static void dumpcffheader(SplineFont *sf,FILE *cfff) {
    putc('\1',cfff);		/* Major version: 1 */
    putc('\0',cfff);		/* Minor version: 0 */
    putc('\4',cfff);		/* Header size in bytes */
    putc('\4',cfff);		/* Absolute Offset size. */
	/* I don't think there are any absolute offsets that aren't encoded */
	/*  in a dict as numbers (ie. inherently variable sized items) */
}

static void dumpcffnames(SplineFont *sf,FILE *cfff) {
    char *pt;

    putshort(cfff,1);		/* One font name */
    putc('\1',cfff);		/* Offset size */
    putc('\1',cfff);		/* Offset to first name */
    putc('\1'+strlen(sf->fontname),cfff);
    for ( pt=sf->fontname; *pt; ++pt )
	putc(*pt,cfff);
}

static void dumpcffcharset(SplineFont *sf,struct alltabs *at) {
    int i;

    at->gn_sid = gcalloc(at->gi.gcnt,sizeof(uint32));
    putc(0,at->charset);
    /* I always use a format 0 charset. ie. an array of SIDs in random order */

    /* First element must be ".notdef" and is omitted */

    for ( i=1; i<at->gi.gcnt; ++i )
	if ( at->gi.bygid[i]!=-1 && SCWorthOutputting(sf->glyphs[at->gi.bygid[i]])) {
	    at->gn_sid[i] = storesid(at,sf->glyphs[at->gi.bygid[i]]->name);
	    putshort(at->charset,at->gn_sid[i]);
	}
}

static void dumpcffcidset(SplineFont *sf,struct alltabs *at) {
    int gid, start;

    putc(2,at->charset);

    start = -1;			/* Glyph 0 always maps to CID 0, and is omitted */
    for ( gid = 1; gid<at->gi.gcnt; ++gid ) {
	if ( start==-1 )
	    start = gid;
	else if ( at->gi.bygid[gid]-at->gi.bygid[start]!=gid-start ) {
	    putshort(at->charset,at->gi.bygid[start]);
	    putshort(at->charset,at->gi.bygid[gid-1]-at->gi.bygid[start]);
	    start = gid;
	}
    }
    if ( start!=-1 ) {
	putshort(at->charset,at->gi.bygid[start]);
	putshort(at->charset,at->gi.bygid[gid-1]-at->gi.bygid[start]);
    }
}

static void dumpcfffdselect(SplineFont *sf,struct alltabs *at) {
    int cid, k, lastfd, cnt;
    int gid;

    putc(3,at->fdselect);
    putshort(at->fdselect,0);		/* number of ranges, fill in later */

    for ( k=0; k<sf->subfontcnt; ++k )
	if ( SCWorthOutputting(sf->subfonts[k]->glyphs[0]))
    break;
    if ( k==sf->subfontcnt ) --k;	/* If CID 0 not defined, put it in last font */
    putshort(at->fdselect,0);
    putc(k,at->fdselect);
    lastfd = k;
    cnt = 1;
    for ( gid = 1; gid<at->gi.gcnt; ++gid ) {
	cid = at->gi.bygid[gid];
	for ( k=0; k<sf->subfontcnt; ++k ) {
	    if ( cid<sf->subfonts[k]->glyphcnt &&
		SCWorthOutputting(sf->subfonts[k]->glyphs[cid]) )
	break;
	}
	if ( k==sf->subfontcnt )
	    /* Doesn't map to a glyph, irrelevant */;
	else {
	    if ( k!=lastfd ) {
		putshort(at->fdselect,gid);
		putc(k,at->fdselect);
		lastfd = k;
		++cnt;
	    }
	}
    }
    putshort(at->fdselect,gid);
    fseek(at->fdselect,1,SEEK_SET);
    putshort(at->fdselect,cnt);
    fseek(at->fdselect,0,SEEK_END);
}

static void dumpcffencoding(SplineFont *sf,struct alltabs *at) {
    int i, cnt, anydups;
    uint32 start_pos = ftell(at->encoding);
    SplineChar *sc;
    EncMap *map = at->map;

    putc(0,at->encoding);
    /* I always use a format 0 encoding. ie. an array of glyph indexes */
    putc(0xff,at->encoding);		/* fixup later */

    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL )
	sf->glyphs[i]->ticked = false;

    cnt = 0;
    anydups = 0;
    for ( i=0; i<256 && i<map->enccount; ++i ) if ( map->map[i]!=-1 && (sc=sf->glyphs[map->map[i]])!=NULL ) {
	if ( sc->ttf_glyph>255 )
    continue;
	if ( sc->ticked ) {
	    ++anydups;
	} else if ( sc->ttf_glyph>0 ) {
	    if ( cnt>=255 )
    break;
	    putc(i,at->encoding);
	    ++cnt;
	    sc->ticked = true;
	}
    }
    if ( anydups ) {
	fseek(at->encoding,start_pos,SEEK_SET);
	putc(0x80,at->encoding);
	putc(cnt,at->encoding);
	fseek(at->encoding,0,SEEK_END);
	putc(anydups,at->encoding);

	for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL )
	    sf->glyphs[i]->ticked = false;
	for ( i=0; i<256 && i<map->enccount; ++i ) if ( map->map[i]!=-1 && (sc=sf->glyphs[map->map[i]])!=NULL ) {
	    if ( sc->ttf_glyph>255 )
    continue;
	    if ( sc->ticked ) {
		putc(i,at->encoding);
		putshort(at->encoding,at->gn_sid[sc->ttf_glyph]);
	    }
	    sc->ticked = true;
	}
    } else {
	fseek(at->encoding,start_pos+1,SEEK_SET);
	putc(cnt,at->encoding);
	fseek(at->encoding,0,SEEK_END);
    }
    free( at->gn_sid );
    at->gn_sid = NULL;
}

static void _dumpcffstrings(FILE *file, struct pschars *strs) {
    int i, len, offsize;

    /* First figure out the offset size */
    len = 1;
    for ( i=0; i<strs->next; ++i )
	len += strs->lens[i];

    /* Then output the index size and offsets */
    putshort( file, strs->next );
    if ( strs->next!=0 ) {
	/* presumably offsets are unsigned. But the docs don't state this in the obvious place */
	offsize = len<=255?1:len<=65535?2:len<=0xffffff?3:4;
	putc(offsize,file);
	len = 1;
	for ( i=0; i<strs->next; ++i ) {
	    dumpoffset(file,offsize,len);
	    len += strs->lens[i];
	}
	dumpoffset(file,offsize,len);

	/* last of all the strings */
	for ( i=0; i<strs->next; ++i ) {
	    uint8 *pt = strs->values[i], *end = pt+strs->lens[i];
	    while ( pt<end )
		putc( *pt++, file );
	}
    }
}

static FILE *dumpcffstrings(struct pschars *strs) {
    FILE *file = tmpfile();
    _dumpcffstrings(file,strs);
    PSCharsFree(strs);
return( file );
}

int SFFigureDefWidth(SplineFont *sf, int *_nomwid) {
    uint16 *widths; uint32 *cumwid;
    int nomwid, defwid, i, sameval=(int) 0x80000000, maxw=0, allsame=true;
    int cnt,j;

    for ( i=0; i<sf->glyphcnt; ++i )
	if ( SCWorthOutputting(sf->glyphs[i]) ) {
	    if ( maxw<sf->glyphs[i]->width ) maxw = sf->glyphs[i]->width;
	    if ( sameval == 0x8000000 )
		sameval = sf->glyphs[i]->width;
	    else if ( sameval!=sf->glyphs[i]->width )
		allsame = false;
	}
    if ( allsame ) {
	nomwid = defwid = sameval;
    } else {
	++maxw;
	if ( maxw>65535 ) maxw = 3*(sf->ascent+sf->descent);
	widths = gcalloc(maxw,sizeof(uint16));
	cumwid = gcalloc(maxw,sizeof(uint32));
	defwid = 0; cnt=0;
	for ( i=0; i<sf->glyphcnt; ++i )
	    if ( SCWorthOutputting(sf->glyphs[i]) &&
		    sf->glyphs[i]->width>=0 &&
		    sf->glyphs[i]->width<maxw )
		if ( ++widths[sf->glyphs[i]->width] > cnt ) {
		    defwid = sf->glyphs[i]->width;
		    cnt = widths[defwid];
		}
	widths[defwid] = 0;
	for ( i=0; i<maxw; ++i )
		for ( j=-107; j<=107; ++j )
		    if ( i+j>=0 && i+j<maxw )
			cumwid[i] += widths[i+j];
	cnt = 0; nomwid = 0;
	for ( i=0; i<maxw; ++i )
	    if ( cnt<cumwid[i] ) {
		cnt = cumwid[i];
		nomwid = i;
	    }
	free(widths); free(cumwid);
    }
    if ( _nomwid!=NULL )
	*_nomwid = nomwid;
return( defwid );
}

static void ATFigureDefWidth(SplineFont *sf, struct alltabs *at, int subfont) {
    int nomwid, defwid;

    defwid = SFFigureDefWidth(sf,&nomwid);
    if ( subfont==-1 )
	at->defwid = defwid;
    else
	at->fds[subfont].defwid = defwid;
    if ( subfont==-1 )
	at->nomwid = nomwid;
    else
	at->fds[subfont].nomwid = nomwid;
}

static void dumpcffprivate(SplineFont *sf,struct alltabs *at,int subfont,
	int subrcnt) {
    char *pt;
    FILE *private = subfont==-1?at->private:at->fds[subfont].private;
    int mi,i;
    real bluevalues[14], otherblues[10];
    real snapcnt[12];
    real stemsnaph[12], stemsnapv[12];
    real stdhw[1], stdvw[1];
    int hasblue=0, hash=0, hasv=0, bs;
    int nomwid, defwid;
    EncMap *map = at->map;
    double bluescale;

    /* The private dict is not in an index, so no index header. Just the data */

    if ( subfont==-1 )
	defwid = at->defwid;
    else
	defwid = at->fds[subfont].defwid;
    dumpintoper(private,defwid,20);		/* Default Width */
    if ( subfont==-1 )
	nomwid = at->nomwid;
    else
	nomwid = at->fds[subfont].nomwid;
    dumpintoper(private,nomwid,21);		/* Nominative Width */

    bs = SplineFontIsFlexible(sf,at->gi.layer,at->gi.flags);
    hasblue = PSDictHasEntry(sf->private,"BlueValues")!=NULL;
    hash = PSDictHasEntry(sf->private,"StdHW")!=NULL;
    hasv = PSDictHasEntry(sf->private,"StdVW")!=NULL;
    ff_progress_change_stages(2+autohint_before_generate+!hasblue);

    otherblues[0] = otherblues[1] = bluevalues[0] = bluevalues[1] = 0;
    if ( !hasblue ) {
	FindBlues(sf,at->gi.layer,bluevalues,otherblues);
	ff_progress_next_stage();
    }

    stdhw[0] = stdvw[0] = 0;
    if ( !hash ) {
	FindHStems(sf,stemsnaph,snapcnt);
	mi = -1;
	for ( i=0; stemsnaph[i]!=0 && i<12; ++i )
	    if ( mi==-1 ) mi = i;
	    else if ( snapcnt[i]>snapcnt[mi] ) mi = i;
	if ( mi!=-1 ) stdhw[0] = stemsnaph[mi];
    }

    if ( !hasv ) {
	FindVStems(sf,stemsnapv,snapcnt);
	mi = -1;
	for ( i=0; stemsnapv[i]!=0 && i<12; ++i )
	    if ( mi==-1 ) mi = i;
	    else if ( snapcnt[i]>snapcnt[mi] ) mi = i;
	if ( mi!=-1 ) stdvw[0] = stemsnapv[mi];
    }
    ff_progress_change_line1(_("Saving OpenType Font"));

    if ( hasblue )
	DumpStrArray(PSDictHasEntry(sf->private,"BlueValues"),private,6);
    else
	DumpDblArray(bluevalues,sizeof(bluevalues)/sizeof(bluevalues[0]),private,6);
    if ( (pt=PSDictHasEntry(sf->private,"OtherBlues"))!=NULL )
	DumpStrArray(pt,private,7);
    else if ( !hasblue )
	DumpDblArray(otherblues,sizeof(otherblues)/sizeof(otherblues[0]),private,7);
    if ( (pt=PSDictHasEntry(sf->private,"FamilyBlues"))!=NULL )
	DumpStrArray(pt,private,8);
    bluescale = BlueScaleFigure(sf->private,bluevalues,otherblues);
    if ( (pt=PSDictHasEntry(sf->private,"FamilyOtherBlues"))!=NULL )
	DumpStrArray(pt,private,9);
    if ( (pt=PSDictHasEntry(sf->private,"BlueScale"))!=NULL )
	DumpStrDouble(pt,private,(12<<8)+9);
    else if ( bluescale!=-1 )
	dumpdbloper(private,bluescale,(12<<8)+9);
    if ( (pt=PSDictHasEntry(sf->private,"BlueShift"))!=NULL )
	DumpStrDouble(pt,private,(12<<8)+10);
    else
	dumpintoper(private,bs,(12<<8)+10);
    if ( (pt=PSDictHasEntry(sf->private,"BlueFuzz"))!=NULL )
	DumpStrDouble(pt,private,(12<<8)+11);
    if ( hash ) {
	DumpStrDouble(PSDictHasEntry(sf->private,"StdHW"),private,10);
	if ( (pt=PSDictHasEntry(sf->private,"StemSnapH"))!=NULL )
	    DumpStrArray(pt,private,(12<<8)|12);
    } else {
	if ( stdhw[0]!=0 )
	    dumpdbloper(private,stdhw[0],10);
	DumpDblArray(stemsnaph,sizeof(stemsnaph)/sizeof(stemsnaph[0]),private,(12<<8)|12);
    }
    if ( hasv ) {
	DumpStrDouble(PSDictHasEntry(sf->private,"StdVW"),private,11);
	if ( (pt=PSDictHasEntry(sf->private,"StemSnapV"))!=NULL )
	    DumpStrArray(pt,private,(12<<8)|13);
    } else {
	if ( stdvw[0]!=0 )
	    dumpdbloper(private,stdvw[0],11);
	DumpDblArray(stemsnapv,sizeof(stemsnapv)/sizeof(stemsnapv[0]),private,(12<<8)|13);
    }
    if ( (pt=PSDictHasEntry(sf->private,"ForceBold"))!=NULL ) {
	dumpintoper(private,*pt=='t'||*pt=='T',(12<<8)|14);
    } else if ( sf->weight!=NULL &&
	    (strstrmatch(sf->weight,"Bold")!=NULL ||
	     strstrmatch(sf->weight,"Demi")!=NULL ||
	     strstrmatch(sf->weight,"Fett")!=NULL ||
	     strstrmatch(sf->weight,"Gras")!=NULL ||
	     strstrmatch(sf->weight,"Heavy")!=NULL ||
	     strstrmatch(sf->weight,"Black")!=NULL))
	dumpintoper(private,1,(12<<8)|14);
    if ( (pt=PSDictHasEntry(sf->private,"LanguageGroup"))!=NULL )
	DumpStrDouble(pt,private,(12<<8)+17);
    else if ( map->enc->is_japanese ||
	      map->enc->is_korean ||
	      map->enc->is_tradchinese ||
	      map->enc->is_simplechinese )
	dumpintoper(private,1,(12<<8)|17);
    if ( (pt=PSDictHasEntry(sf->private,"ExpansionFactor"))!=NULL )
	DumpStrDouble(pt,private,(12<<8)+18);
    if ( subrcnt!=0 )
	dumpsizedint(private,false,ftell(private)+3+1,19);	/* Subrs */

    if ( subfont==-1 )
	at->privatelen = ftell(private);
    else
	at->fds[subfont].privatelen = ftell(private);
}

/* When we exit this the topdict is not complete, we still need to fill in */
/*  values for charset,encoding,charstrings and private. Then we need to go */
/*  back and fill in the table length (at lenpos) */
static void dumpcfftopdict(SplineFont *sf,struct alltabs *at) {
    char *pt, *end;
    FILE *cfff = at->cfff;
    DBounds b;

    putshort(cfff,1);		/* One top dict */
    putc('\2',cfff);		/* Offset size */
    putshort(cfff,1);		/* Offset to topdict */
    at->lenpos = ftell(cfff);
    putshort(cfff,0);		/* placeholder for final position (final offset in index points beyond last element) */
    dumpsid(cfff,at,sf->version,0);
    dumpsid(cfff,at,sf->copyright,1);
    dumpsid(cfff,at,sf->fullname?sf->fullname:sf->fontname,2);
    dumpsid(cfff,at,sf->familyname,3);
    dumpsid(cfff,at,sf->weight,4);
    if ( at->gi.fixed_width!=-1 ) dumpintoper(cfff,1,(12<<8)|1);
    if ( sf->italicangle!=0 ) dumpdbloper(cfff,sf->italicangle,(12<<8)|2);
    if ( sf->upos!=-100 ) dumpdbloper(cfff,sf->upos,(12<<8)|3);
    if ( sf->uwidth!=50 ) dumpdbloper(cfff,sf->uwidth,(12<<8)|4);
    if ( sf->strokedfont ) {
	dumpintoper(cfff,2,(12<<8)|5);
	dumpdbloper(cfff,sf->strokewidth,(12<<8)|8);
    }
    /* We'll never set CharstringType */
    if ( sf->ascent+sf->descent!=1000 ) {
	dumpdbl(cfff,1.0/(sf->ascent+sf->descent));
	dumpint(cfff,0);
	dumpint(cfff,0);
	dumpdbl(cfff,1.0/(sf->ascent+sf->descent));
	dumpint(cfff,0);
	dumpintoper(cfff,0,(12<<8)|7);
    }
    if ( sf->uniqueid!=-1 && sf->use_uniqueid )
	dumpintoper(cfff, sf->uniqueid?sf->uniqueid:4000000 + (rand()&0x3ffff), 13 );
    SplineFontLayerFindBounds(sf,at->gi.layer,&b);
    at->gi.xmin = b.minx;
    at->gi.ymin = b.miny;
    at->gi.xmax = b.maxx;
    at->gi.ymax = b.maxy;
    dumpdbl(cfff,floor(b.minx));
    dumpdbl(cfff,floor(b.miny));
    dumpdbl(cfff,ceil(b.maxx));
    dumpdbloper(cfff,ceil(b.maxy),5);
    /* We'll never set StrokeWidth */
    if ( sf->xuid!=NULL && sf->use_xuid ) {
	pt = sf->xuid; if ( *pt=='[' ) ++pt;
	while ( *pt && *pt!=']' ) {
	    dumpint(cfff,strtol(pt,&end,10));
	    if ( pt==end )	/* garbage in XUID */
	break;
	    for ( pt = end; *pt==' '; ++pt );
	}
	putc(14,cfff);
	if ( sf->changed_since_xuidchanged )
	    SFIncrementXUID(sf);
    }
    /* Offset to charset (oper=15) needed here */
    /* Offset to encoding (oper=16) needed here (not for CID )*/
    /* Offset to charstrings (oper=17) needed here */
    /* Length of, and Offset to private (oper=18) needed here (not for CID )*/
}

static int dumpcffdict(SplineFont *sf,struct alltabs *at) {
    FILE *fdarray = at->fdarray;
    int pstart;
    /* according to the PSRef Man v3, only fontname, fontmatrix and private */
    /*  appear in this dictionary */

    dumpsid(fdarray,at,sf->fontname,(12<<8)|38);
    if ( sf->ascent+sf->descent!=1000 ) {
	dumpdbl(fdarray,1.0/(sf->ascent+sf->descent));
	dumpint(fdarray,0);
	dumpint(fdarray,0);
	dumpdbl(fdarray,1.0/(sf->ascent+sf->descent));
	dumpint(fdarray,0);
	dumpintoper(fdarray,0,(12<<8)|7);
    }
    pstart = ftell(fdarray);
    dumpsizedint(fdarray,false,0,-1);	/* private length */
    dumpsizedint(fdarray,true,0,18);	/* private offset */
return( pstart );
}

static void dumpcffdictindex(SplineFont *sf,struct alltabs *at) {
    int i;
    int pos;

    putshort(at->fdarray,sf->subfontcnt);
    putc('\2',at->fdarray);		/* DICTs aren't very big, and there are at most 255 */
    putshort(at->fdarray,1);		/* Offset to first dict */
    for ( i=0; i<sf->subfontcnt; ++i )
	putshort(at->fdarray,0);	/* Dump offset placeholders (note there's one extra to mark the end) */
    pos = ftell(at->fdarray)-1;
    for ( i=0; i<sf->subfontcnt; ++i ) {
	at->fds[i].fillindictmark = dumpcffdict(sf->subfonts[i],at);
	at->fds[i].eodictmark = ftell(at->fdarray);
	if ( at->fds[i].eodictmark>65536 )
	    IError("The DICT INDEX got too big, result won't work");
    }
    fseek(at->fdarray,2*sizeof(short)+sizeof(char),SEEK_SET);
    for ( i=0; i<sf->subfontcnt; ++i )
	putshort(at->fdarray,at->fds[i].eodictmark-pos);
    fseek(at->fdarray,0,SEEK_END);
}

static void dumpcffcidtopdict(SplineFont *sf,struct alltabs *at) {
    char *pt, *end;
    FILE *cfff = at->cfff;
    DBounds b;
    int cidcnt=0, k;

    for ( k=0; k<sf->subfontcnt; ++k )
	if ( sf->subfonts[k]->glyphcnt>cidcnt ) cidcnt = sf->subfonts[k]->glyphcnt;

    putshort(cfff,1);		/* One top dict */
    putc('\2',cfff);		/* Offset size */
    putshort(cfff,1);		/* Offset to topdict */
    at->lenpos = ftell(cfff);
    putshort(cfff,0);		/* placeholder for final position */
    dumpsid(cfff,at,sf->cidregistry,-1);
    dumpsid(cfff,at,sf->ordering,-1);
    dumpintoper(cfff,sf->supplement,(12<<8)|30);		/* ROS operator must be first */
    dumpdbloper(cfff,sf->cidversion,(12<<8)|31);
    dumpintoper(cfff,cidcnt,(12<<8)|34);
    if ( sf->use_uniqueid )
	dumpintoper(cfff, sf->uniqueid?sf->uniqueid:4000000 + (rand()&0x3ffff), (12<<8)|35 );

    dumpsid(cfff,at,sf->copyright,1);
    dumpsid(cfff,at,sf->fullname?sf->fullname:sf->fontname,2);
    dumpsid(cfff,at,sf->familyname,3);
    dumpsid(cfff,at,sf->weight,4);
    /* FontMatrix  (identity here, real ones in sub fonts)*/
    /* Actually there is no fontmatrix in the adobe cid font I'm looking at */
    /*  which means it should default to [.001...] but it doesn't so the */
    /*  docs aren't completely accurate */
    /* I now see I've no idea what the FontMatrix means in a CID keyed font */
    /*  it seems to be ignored everywhere */
#if 0
    dumpdbl(cfff,1.0);
    dumpint(cfff,0);
    dumpint(cfff,0);
    dumpdbl(cfff,1.0);
    dumpint(cfff,0);
    dumpintoper(cfff,0,(12<<8)|7);
#endif

    CIDLayerFindBounds(sf,at->gi.layer,&b);
    at->gi.xmin = b.minx;
    at->gi.ymin = b.miny;
    at->gi.xmax = b.maxx;
    at->gi.ymax = b.maxy;
    dumpdbl(cfff,floor(b.minx));
    dumpdbl(cfff,floor(b.miny));
    dumpdbl(cfff,ceil(b.maxx));
    dumpdbloper(cfff,ceil(b.maxy),5);
    /* We'll never set StrokeWidth */
    if ( sf->xuid!=NULL && sf->use_xuid ) {
	pt = sf->xuid; if ( *pt=='[' ) ++pt;
	while ( *pt && *pt!=']' ) {
	    dumpint(cfff,strtol(pt,&end,10));
	    for ( pt = end; *pt==' '; ++pt );
	}
	putc(14,cfff);
	if ( sf->changed_since_xuidchanged )
	    SFIncrementXUID(sf);
    }
#if 0
    /* Acrobat doesn't seem to care about a private dict here. Ghostscript */
    /*  dies.  Tech Note: 5176.CFF.PDF, top of page 23 says:		   */
    /*		A Private DICT is required, but may be specified as having */
    /*		a length of 0 if there are no non-default values to be stored*/
    /* No indication >where< it is required. I assumed everywhere. Perhaps */
    /*  just in basefonts? 						   */
    dumpint(cfff,0);			/* Docs say a private dict is required and they don't specifically omit CID top dicts */
    dumpintoper(cfff,0,18);		/* But they do say it can be zero */
#endif
    /* Offset to charset (oper=15) needed here */
    /* Offset to charstrings (oper=17) needed here */
    /* Offset to FDArray (oper=12,36) needed here */
    /* Offset to FDSelect (oper=12,37) needed here */
}

static int isStdEncoding(SplineFont *sf,EncMap *map) {
    int i;

    for ( i=0; i<256 && i<map->enccount; ++i ) if ( map->map[i]!=-1 && sf->glyphs[map->map[i]]!=NULL )
	if ( sf->glyphs[map->map[i]]->unicodeenc!=-1 )
	    if ( sf->glyphs[map->map[i]]->unicodeenc!=unicode_from_adobestd[i] )
return( 0 );

return( 1 );
}

static void finishup(SplineFont *sf,struct alltabs *at) {
    int strlen, shlen, glen,enclen,csetlen,cstrlen,prvlen;
    int base, eotop, strhead;
    int output_enc = ( at->format==ff_cff && !isStdEncoding(sf,at->map));

    storesid(at,NULL);		/* end the strings index */
    strlen = ftell(at->sidf) + (shlen = ftell(at->sidh));
    glen = sizeof(short);	/* Single entry: 0, no globals */
    enclen = ftell(at->encoding);
    csetlen = ftell(at->charset);
    cstrlen = ftell(at->charstrings);
    prvlen = ftell(at->private);
    base = ftell(at->cfff);
    if ( base+6*3+strlen+glen+enclen+csetlen+cstrlen+prvlen > 32767 ) {
	at->cfflongoffset = true;
	base += 5*5+4;
    } else
	base += 5*3+4;
    strhead = 2+(at->sidcnt>1);
    base += strhead;

    dumpsizedint(at->cfff,at->cfflongoffset,base+strlen+glen,15);		/* Charset */
    if ( output_enc )					/* encoding offset */
	dumpsizedint(at->cfff,at->cfflongoffset,base+strlen+glen+csetlen,16);	/* encoding offset */
    else {
	dumpsizedint(at->cfff,at->cfflongoffset,0,16);
	enclen = 0;
    }
    dumpsizedint(at->cfff,at->cfflongoffset,base+strlen+glen+csetlen+enclen,17);/* charstrings */
    dumpsizedint(at->cfff,at->cfflongoffset,at->privatelen,-1);
    dumpsizedint(at->cfff,at->cfflongoffset,base+strlen+glen+csetlen+enclen+cstrlen,18); /* private size */
    eotop = base-strhead-at->lenpos-1;
    if ( at->cfflongoffset ) {
	fseek(at->cfff,3,SEEK_SET);
	putc(4,at->cfff);
    }
    fseek(at->cfff,at->lenpos,SEEK_SET);
    putshort(at->cfff,eotop);
    fseek(at->cfff,0,SEEK_END);

    /* String Index */
    putshort(at->cfff,at->sidcnt-1);
    if ( at->sidcnt!=1 ) {		/* Everybody gets an added NULL */
	putc(at->sidlongoffset?4:2,at->cfff);
	if ( !ttfcopyfile(at->cfff,at->sidh,base,"CFF-StringBase")) at->error = true;
	if ( !ttfcopyfile(at->cfff,at->sidf,base+shlen,"CFF-StringData")) at->error = true;
    }

    /* Global Subrs */
    putshort(at->cfff,0);

    /* Charset */
    if ( !ttfcopyfile(at->cfff,at->charset,base+strlen+glen,"CFF-Charset")) at->error = true;

    /* Encoding */
    if ( !ttfcopyfile(at->cfff,at->encoding,base+strlen+glen+csetlen,"CFF-Encoding")) at->error = true;

    /* Char Strings */
    if ( !ttfcopyfile(at->cfff,at->charstrings,base+strlen+glen+csetlen+enclen,"CFF-CharStrings")) at->error = true;

    /* Private & Subrs */
    if ( !ttfcopyfile(at->cfff,at->private,base+strlen+glen+csetlen+enclen+cstrlen,"CFF-Private")) at->error = true;
}

static void finishupcid(SplineFont *sf,struct alltabs *at) {
    int strlen, shlen, glen,csetlen,cstrlen,fdsellen,fdarrlen,prvlen;
    int base, eotop, strhead;
    int i;

    storesid(at,NULL);		/* end the strings index */
    strlen = ftell(at->sidf) + (shlen = ftell(at->sidh));
    glen = ftell(at->globalsubrs);
    /* No encodings */
    csetlen = ftell(at->charset);
    fdsellen = ftell(at->fdselect);
    cstrlen = ftell(at->charstrings);
    fdarrlen = ftell(at->fdarray);
    base = ftell(at->cfff);

    at->cfflongoffset = true;
    base += 5*4+4+2;		/* two of the opers below are two byte opers */
    strhead = 2+(at->sidcnt>1);
    base += strhead;

    prvlen = 0;
    for ( i=0; i<sf->subfontcnt; ++i ) {
	fseek(at->fdarray,at->fds[i].fillindictmark,SEEK_SET);
	dumpsizedint(at->fdarray,false,at->fds[i].privatelen,-1);	/* Private len */
	dumpsizedint(at->fdarray,true,base+strlen+glen+csetlen+fdsellen+cstrlen+fdarrlen+prvlen,18);	/* Private offset */
	prvlen += ftell(at->fds[i].private);	/* private & subrs */
    }

    dumpsizedint(at->cfff,at->cfflongoffset,base+strlen+glen,15);	/* charset */
    dumpsizedint(at->cfff,at->cfflongoffset,base+strlen+glen+csetlen,(12<<8)|37);	/* fdselect */
    dumpsizedint(at->cfff,at->cfflongoffset,base+strlen+glen+csetlen+fdsellen,17);	/* charstrings */
    dumpsizedint(at->cfff,at->cfflongoffset,base+strlen+glen+csetlen+fdsellen+cstrlen,(12<<8)|36);	/* fdarray */
    eotop = base-strhead-at->lenpos-1;
    fseek(at->cfff,at->lenpos,SEEK_SET);
    putshort(at->cfff,eotop);
    fseek(at->cfff,0,SEEK_END);

    /* String Index */
    putshort(at->cfff,at->sidcnt-1);
    if ( at->sidcnt!=1 ) {		/* Everybody gets an added NULL */
	putc(at->sidlongoffset?4:2,at->cfff);
	if ( !ttfcopyfile(at->cfff,at->sidh,base,"CFF-StringBase")) at->error = true;
	if ( !ttfcopyfile(at->cfff,at->sidf,base+shlen,"CFF-StringData")) at->error = true;
    }

    /* Global Subrs */
    if ( !ttfcopyfile(at->cfff,at->globalsubrs,base+strlen,"CFF-GlobalSubrs")) at->error = true;

    /* Charset */
    if ( !ttfcopyfile(at->cfff,at->charset,base+strlen+glen,"CFF-Charset")) at->error = true;

    /* FDSelect */
    if ( !ttfcopyfile(at->cfff,at->fdselect,base+strlen+glen+csetlen,"CFF-FDSelect")) at->error = true;

    /* Char Strings */
    if ( !ttfcopyfile(at->cfff,at->charstrings,base+strlen+glen+csetlen+fdsellen,"CFF-CharStrings")) at->error = true;

    /* FDArray (DICT Index) */
    if ( !ttfcopyfile(at->cfff,at->fdarray,base+strlen+glen+csetlen+fdsellen+cstrlen,"CFF-FDArray")) at->error = true;

    /* Private & Subrs */
    prvlen = 0;
    for ( i=0; i<sf->subfontcnt; ++i ) {
	int temp = ftell(at->fds[i].private);
	if ( !ttfcopyfile(at->cfff,at->fds[i].private,
		base+strlen+glen+csetlen+fdsellen+cstrlen+fdarrlen+prvlen,"CFF-PrivateSubrs")) at->error = true;
	prvlen += temp;
    }

    free(at->fds);
}

static int dumpcffhmtx(struct alltabs *at,SplineFont *sf,int bitmaps) {
    DBounds b;
    SplineChar *sc;
    int i,cnt;
    int dovmetrics = sf->hasvmetrics;
    int width = at->gi.fixed_width;

    at->gi.hmtx = tmpfile();
    if ( dovmetrics )
	at->gi.vmtx = tmpfile();
    FigureFullMetricsEnd(sf,&at->gi,bitmaps);	/* Bitmap fonts use ttf convention of 3 magic glyphs */
    if ( at->gi.bygid[0]!=-1 && (sf->glyphs[at->gi.bygid[0]]->width==width || width==-1 )) {
	putshort(at->gi.hmtx,sf->glyphs[at->gi.bygid[0]]->width);
	SplineCharLayerFindBounds(sf->glyphs[at->gi.bygid[0]],at->gi.layer,&b);
	putshort(at->gi.hmtx,b.minx);
	if ( dovmetrics ) {
	    putshort(at->gi.vmtx,sf->glyphs[at->gi.bygid[0]]->vwidth);
	    putshort(at->gi.vmtx,/*sf->vertical_origin-*/b.miny);
	}
    } else {
	putshort(at->gi.hmtx,width==-1?(sf->ascent+sf->descent)/2:width);
	putshort(at->gi.hmtx,0);
	if ( dovmetrics ) {
	    putshort(at->gi.vmtx,sf->ascent+sf->descent);
	    putshort(at->gi.vmtx,0);
	}
    }
    cnt = 1;
    if ( bitmaps ) {
	if ( width==-1 ) width = (sf->ascent+sf->descent)/3;
	putshort(at->gi.hmtx,width);
	putshort(at->gi.hmtx,0);
	if ( dovmetrics ) {
	    putshort(at->gi.vmtx,sf->ascent+sf->descent);
	    putshort(at->gi.vmtx,0);
	}
	putshort(at->gi.hmtx,width);
	putshort(at->gi.hmtx,0);
	if ( dovmetrics ) {
	    putshort(at->gi.vmtx,sf->ascent+sf->descent);
	    putshort(at->gi.vmtx,0);
	}
	cnt = 3;
    }

    for ( i=cnt; i<at->gi.gcnt; ++i ) if ( at->gi.bygid[i]!=-1 ) {
	sc = sf->glyphs[at->gi.bygid[i]];
	if ( SCWorthOutputting(sc) ) {
	    if ( i<=at->gi.lasthwidth )
		putshort(at->gi.hmtx,sc->width);
	    SplineCharLayerFindBounds(sc,at->gi.layer,&b);
	    putshort(at->gi.hmtx,b.minx);
	    if ( dovmetrics ) {
		if ( i<=at->gi.lastvwidth )
		    putshort(at->gi.vmtx,sc->vwidth);
		putshort(at->gi.vmtx,/*sf->vertical_origin-*/b.maxy);
	    }
	    ++cnt;
	    if ( i==at->gi.lasthwidth )
		at->gi.hfullcnt = cnt;
	    if ( i==at->gi.lastvwidth )
		at->gi.vfullcnt = cnt;
	}
    }
    at->gi.hmtxlen = ftell(at->gi.hmtx);
    if ( at->gi.hmtxlen&2 ) putshort(at->gi.hmtx,0);
    if ( dovmetrics ) {
	at->gi.vmtxlen = ftell(at->gi.vmtx);
	if ( at->gi.vmtxlen&2 ) putshort(at->gi.vmtx,0);
    }

    at->gi.maxp->numGlyphs = cnt;
return( true );
}

static void dumpcffcidhmtx(struct alltabs *at,SplineFont *_sf) {
    DBounds b;
    SplineChar *sc;
    int cid,i,cnt=0,max;
    SplineFont *sf = NULL;
    int dovmetrics = _sf->hasvmetrics;

    at->gi.hmtx = tmpfile();
    if ( dovmetrics )
	at->gi.vmtx = tmpfile();
    FigureFullMetricsEnd(_sf,&at->gi,false);

    max = 0;
    for ( i=0; i<_sf->subfontcnt; ++i )
	if ( max<_sf->subfonts[i]->glyphcnt )
	    max = _sf->subfonts[i]->glyphcnt;
    for ( cid = 0; cid<max; ++cid ) {
	for ( i=0; i<_sf->subfontcnt; ++i ) {
	    sf = _sf->subfonts[i];
	    if ( cid<sf->glyphcnt && SCWorthOutputting(sf->glyphs[cid]))
	break;
	}
	if ( i!=_sf->subfontcnt ) {
	    sc = sf->glyphs[cid];
	    if ( sc->ttf_glyph<=at->gi.lasthwidth )
		putshort(at->gi.hmtx,sc->width);
	    SplineCharLayerFindBounds(sc,at->gi.layer,&b);
	    putshort(at->gi.hmtx,b.minx);
	    if ( dovmetrics ) {
		if ( sc->ttf_glyph<=at->gi.lastvwidth )
		    putshort(at->gi.vmtx,sc->vwidth);
		putshort(at->gi.vmtx,/*sf->vertical_origin-*/b.maxy);
	    }
	    ++cnt;
	    if ( sc->ttf_glyph==at->gi.lasthwidth )
		at->gi.hfullcnt = cnt;
	    if ( sc->ttf_glyph==at->gi.lastvwidth )
		at->gi.vfullcnt = cnt;
	} else if ( cid==0 ) {
	    /* Create a dummy entry for .notdef */
	    sf = _sf->subfonts[0];
	    putshort(at->gi.hmtx,sf->ascent+sf->descent);
	    putshort(at->gi.hmtx,0);
	    ++cnt;
	    if ( dovmetrics ) {
		putshort(at->gi.vmtx,sf->ascent+sf->descent);
		putshort(at->gi.vmtx,0);
	    }
	}
    }
    at->gi.hmtxlen = ftell(at->gi.hmtx);
    if ( at->gi.hmtxlen&2 ) putshort(at->gi.hmtx,0);
    if ( dovmetrics ) {
	at->gi.vmtxlen = ftell(at->gi.vmtx);
	if ( at->gi.vmtxlen&2 ) putshort(at->gi.vmtx,0);
    }

    at->gi.maxp->numGlyphs = cnt;
}

static int dumptype2glyphs(SplineFont *sf,struct alltabs *at) {
    int i;
    struct pschars *subrs, *chrs;

    at->cfff = tmpfile();
    at->sidf = tmpfile();
    at->sidh = tmpfile();
    at->charset = tmpfile();
    at->encoding = tmpfile();
    at->private = tmpfile();

    dumpcffheader(sf,at->cfff);
    dumpcffnames(sf,at->cfff);
    dumpcffcharset(sf,at);
    ff_progress_change_stages(2+at->gi.strikecnt);

    ATFigureDefWidth(sf,at,-1);
    if ((chrs =SplineFont2ChrsSubrs2(sf,at->nomwid,at->defwid,at->gi.bygid,at->gi.gcnt,at->gi.flags,&subrs,at->gi.layer))==NULL )
return( false );
    dumpcffprivate(sf,at,-1,subrs->next);
    if ( subrs->next!=0 )
	_dumpcffstrings(at->private,subrs);
    ff_progress_next_stage();
    at->charstrings = dumpcffstrings(chrs);
    PSCharsFree(subrs);
    if ( at->charstrings == NULL )
return( false );
    if ( at->format==ff_cff && !isStdEncoding(sf,at->map))
	dumpcffencoding(sf,at);		/* Do this after we've assigned glyph ids */
    dumpcfftopdict(sf,at);
    finishup(sf,at);

    at->cfflen = ftell(at->cfff);
    if ( at->cfflen&3 ) {
	for ( i=4-(at->cfflen&3); i>0; --i )
	    putc('\0',at->cfff);
    }

    if ( at->format!=ff_cff )
	dumpcffhmtx(at,sf,false);
    free(at->gn_sid); at->gn_sid=NULL;
return( true );
}

static int dumpcidglyphs(SplineFont *sf,struct alltabs *at) {
    int i;
    struct pschars *glbls = NULL, *chrs;

    at->cfff = tmpfile();
    at->sidf = tmpfile();
    at->sidh = tmpfile();
    at->charset = tmpfile();
    at->fdselect = tmpfile();
    at->fdarray = tmpfile();
    at->globalsubrs = tmpfile();

    at->fds = gcalloc(sf->subfontcnt,sizeof(struct fd2data));
    for ( i=0; i<sf->subfontcnt; ++i ) {
	at->fds[i].private = tmpfile();
	ATFigureDefWidth(sf->subfonts[i],at,i);
    }
    if ( (chrs = CID2ChrsSubrs2(sf,at->fds,at->gi.flags,&glbls,at->gi.layer))==NULL )
return( false );
    for ( i=0; i<sf->subfontcnt; ++i ) {
	dumpcffprivate(sf->subfonts[i],at,i,at->fds[i].subrs->next);
	if ( at->fds[i].subrs->next!=0 )
	    _dumpcffstrings(at->fds[i].private,at->fds[i].subrs);
	PSCharsFree(at->fds[i].subrs);
    }
    _dumpcffstrings(at->globalsubrs,glbls);
    PSCharsFree(glbls);

    dumpcffheader(sf,at->cfff);
    dumpcffnames(sf,at->cfff);
    dumpcffcidset(sf,at);
    dumpcfffdselect(sf,at);
    dumpcffdictindex(sf,at);
    if ( (at->charstrings = dumpcffstrings(chrs))==NULL )
return( false );
    dumpcffcidtopdict(sf,at);
    finishupcid(sf,at);

    at->cfflen = ftell(at->cfff);
    if ( at->cfflen&3 ) {
	for ( i=4-(at->cfflen&3); i>0; --i )
	    putc('\0',at->cfff);
    }

    if ( at->format!=ff_cffcid )
	dumpcffcidhmtx(at,sf);
return( true );
}



static void OS2WeightCheck(struct pfminfo *pfminfo,char *weight) {
    if ( weight==NULL ) {
	/* default it */
    } else if ( strstrmatch(weight,"medi")!=NULL ) {
	pfminfo->weight = 500;
	pfminfo->panose[2] = 6;
    } else if ( strstrmatch(weight,"demi")!=NULL ||
		strstrmatch(weight,"halb")!=NULL ||
		(strstrmatch(weight,"semi")!=NULL &&
		    strstrmatch(weight,"bold")!=NULL) ) {
	pfminfo->weight = 600;
	pfminfo->panose[2] = 7;
    } else if ( strstrmatch(weight,"bold")!=NULL ||
		strstrmatch(weight,"fett")!=NULL ||
		strstrmatch(weight,"gras")!=NULL ) {
	pfminfo->weight = 700;
	pfminfo->panose[2] = 8;
    } else if ( strstrmatch(weight,"heavy")!=NULL ) {
	pfminfo->weight = 800;
	pfminfo->panose[2] = 9;
    } else if ( strstrmatch(weight,"black")!=NULL ) {
	pfminfo->weight = 900;
	pfminfo->panose[2] = 10;
    } else if ( strstrmatch(weight,"nord")!=NULL ) {
	pfminfo->weight = 950;
	pfminfo->panose[2] = 11;
    } else if ( strstrmatch(weight,"thin")!=NULL ) {
	pfminfo->weight = 100;
	pfminfo->panose[2] = 2;
    } else if ( strstrmatch(weight,"extra")!=NULL ||
	    strstrmatch(weight,"light")!=NULL ) {
	pfminfo->weight = 200;
	pfminfo->panose[2] = 3;
    } else if ( strstrmatch(weight,"light")!=NULL ) {
	pfminfo->weight = 300;
	pfminfo->panose[2] = 4;
    }
}

void SFDefaultOS2Simple(struct pfminfo *pfminfo,SplineFont *sf) {
    pfminfo->pfmfamily = 0x11;
    pfminfo->panose[0] = 2;
    pfminfo->weight = 400;
    pfminfo->panose[2] = 5;
    pfminfo->width = 5;
    pfminfo->panose[3] = 3;
    pfminfo->winascent_add = pfminfo->windescent_add = true;
    pfminfo->hheadascent_add = pfminfo->hheaddescent_add = true;
    pfminfo->typoascent_add = pfminfo->typodescent_add = true;
    pfminfo->os2_winascent = pfminfo->os2_windescent = 0;

    if ( sf->subfonts!=NULL ) sf = sf->subfonts[0];
    pfminfo->linegap = pfminfo->vlinegap = pfminfo->os2_typolinegap = 
	    rint(.09*(sf->ascent+sf->descent));
}

void SFDefaultOS2SubSuper(struct pfminfo *pfminfo,int emsize,double italic_angle) {
    double s = sin(italic_angle*3.1415926535897932/180.0);
    pfminfo->os2_supysize = pfminfo->os2_subysize = .7*emsize;
    pfminfo->os2_supxsize = pfminfo->os2_subxsize = .65*emsize;
    pfminfo->os2_subyoff = .14*emsize;
    pfminfo->os2_supyoff = .48*emsize;
    pfminfo->os2_supxoff =  s*pfminfo->os2_supyoff;
    pfminfo->os2_subxoff = -s*pfminfo->os2_subyoff;
    pfminfo->os2_strikeysize = 102*emsize/2048;
    pfminfo->os2_strikeypos = 530*emsize/2048;
}

void SFDefaultOS2Info(struct pfminfo *pfminfo,SplineFont *sf,char *fontname) {
    int samewid= -1;
    char *weight = sf->cidmaster==NULL ? sf->weight : sf->cidmaster->weight;

    if ( sf->pfminfo.pfmset ) {
	if ( pfminfo!=&sf->pfminfo )
	    *pfminfo = sf->pfminfo;
	if ( !pfminfo->panose_set ) {
	    struct pfminfo info;
	    memset(&info,0,sizeof(info));
	    sf->pfminfo.pfmset = false;
	    SFDefaultOS2Info(&info,sf,fontname);
	    sf->pfminfo.pfmset = true;
	    memcpy(pfminfo->panose,info.panose,sizeof(info.panose));
	}
    } else {
	struct pfminfo hold;
	if ( pfminfo->hheadset || pfminfo->vheadset )
	    hold = *pfminfo;
	else
	    hold.hheadset = hold.vheadset = false;
	memset(pfminfo,'\0',sizeof(*pfminfo));
	SFDefaultOS2Simple(pfminfo,sf);
	samewid = CIDOneWidth(sf);

	pfminfo->pfmfamily = 0x10;
	if ( samewid>0 ) {
	    pfminfo->pfmfamily = 0x30;
	    /* pfminfo->panose[3] = 9; */ /* This is done later */
	} else if ( strstrmatch(fontname,"sans")!=NULL )
	    pfminfo->pfmfamily = 0x20;
	else if ( strstrmatch(fontname,"script")!=NULL ) {
	    pfminfo->pfmfamily = 0x40;
	    pfminfo->panose[0] = 3;
	}
	if ( samewid==-1 )
	    pfminfo->pfmfamily |= 0x1;	/* Else it assumes monospace */

/* urw uses 4 character abreviations */
	if ( weight!=NULL )
	    OS2WeightCheck(pfminfo,weight);
	OS2WeightCheck(pfminfo,fontname);

	if ( strstrmatch(fontname,"ultra")!=NULL &&
		strstrmatch(fontname,"condensed")!=NULL ) {
	    pfminfo->width = 1;
	    pfminfo->panose[3] = 8;
	} else if ( strstrmatch(fontname,"extra")!=NULL &&
		strstrmatch(fontname,"condensed")!=NULL ) {
	    pfminfo->width = 2;
	    pfminfo->panose[3] = 8;
	} else if ( strstrmatch(fontname,"semi")!=NULL &&
		strstrmatch(fontname,"condensed")!=NULL ) {
	    pfminfo->width = 4;
	    pfminfo->panose[3] = 6;
	} else if ( strstrmatch(fontname,"condensed")!=NULL ||
		strstrmatch(fontname,"narrow")!=NULL ) {
	    pfminfo->width = 3;
	    pfminfo->panose[3] = 6;
	} else if ( strstrmatch(fontname,"ultra")!=NULL &&
		strstrmatch(fontname,"expanded")!=NULL ) {
	    pfminfo->width = 9;
	    pfminfo->panose[3] = 7;
	} else if ( strstrmatch(fontname,"extra")!=NULL &&
		strstrmatch(fontname,"expanded")!=NULL ) {
	    pfminfo->width = 8;
	    pfminfo->panose[3] = 7;
	} else if ( strstrmatch(fontname,"semi")!=NULL &&
		strstrmatch(fontname,"expanded")!=NULL ) {
	    pfminfo->width = 6;
	    pfminfo->panose[3] = 5;
	} else if ( strstrmatch(fontname,"expanded")!=NULL ) {
	    pfminfo->width = 7;
	    pfminfo->panose[3] = 5;
	}
	if ( samewid>0 )
	    pfminfo->panose[3] = 9;
	if ( hold.hheadset ) {
	    pfminfo->hheadset = true;
	    pfminfo->hheadascent_add = hold.hheadascent_add;
	    pfminfo->hheaddescent_add = hold.hheaddescent_add;
	    pfminfo->hhead_ascent = hold.hhead_ascent;
	    pfminfo->hhead_descent = hold.hhead_descent;
	    pfminfo->linegap = hold.linegap;
	}
	if ( hold.vheadset ) {
	    pfminfo->vheadset = true;
	    pfminfo->vlinegap = hold.vlinegap;
	}
    }
    if ( !pfminfo->subsuper_set )
	SFDefaultOS2SubSuper(pfminfo,sf->ascent+sf->descent,sf->italicangle);
}

char *utf8_verify_copy(const char *str) {
    /* When given a postscript string it SHOULD be in ASCII. But it will often*/
    /* contain a copyright symbol (sometimes in latin1, sometimes in macroman)*/
    /* unfortunately both encodings use 0xa9 for copyright so we can't distinguish */
    /* guess that it's latin1 (or that copyright is the only odd char which */
    /* means a latin1 conversion will work for macs too). */

    if ( str==NULL )
return( NULL );

    if ( utf8_valid(str))
return( copy(str));		/* Either in ASCII (good) or appears to be utf8*/
return( latin1_2_utf8_copy(str));
}

int SFHasInstructions(SplineFont *sf) {
    int i;

    if ( sf->subfontcnt!=0 )
return( false );		/* Truetype doesn't support cid keyed fonts */

    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL ) {
	if ( strcmp(sf->glyphs[i]->name,".notdef")==0 )
    continue;		/* ff produces fonts with instructions in .notdef & not elsewhere. Ignore these */
	if ( sf->glyphs[i]->ttf_instrs!=NULL )
return( true );
    }
return( false );
}

static int dumpcff(struct alltabs *at,SplineFont *sf,enum fontformat format,
	FILE *cff) {
    int ret;

    if ( format==ff_cff ) {
	AssignTTFGlyph(&at->gi,sf,at->map,true);
	ret = dumptype2glyphs(sf,at);
    } else {
	SFDummyUpCIDs(&at->gi,sf);	/* life is easier if we ignore the seperate fonts of a cid keyed fonts and treat it as flat */
	ret = dumpcidglyphs(sf,at);
	free(sf->glyphs); sf->glyphs = NULL;
	sf->glyphcnt = sf->glyphmax = sf->glyphmin = 0;
    }
    free( at->gi.bygid );

    if ( !ret )
	at->error = true;
    else if ( at->gi.flags & ps_flag_nocffsugar ) {
	if ( !ttfcopyfile(cff,at->cfff,0,"CFF"))
	    at->error = true;
    } else {
	long len;
	char buffer[80];
	fprintf(cff,"%%!PS-Adobe-3.0 Resource-FontSet\n");
	fprintf(cff,"%%%%DocumentNeedResources:ProcSet (FontSetInit)\n");
	fprintf(cff,"%%%%Title: (FontSet/%s)\n", sf->fontname);
	fprintf(cff,"%%%%EndComments\n" );
	fprintf(cff,"%%%%IncludeResource: ProcSet(FontSetInit)\n" );
	fprintf(cff,"%%%%BeginResource: FontSet(%s)\n", sf->fontname );
	fprintf(cff,"/FontSetInit /ProcSet findresource begin\n" );
	fseek(at->cfff,0,SEEK_END);
	len = ftell(at->cfff);
	rewind(at->cfff);
	sprintf( buffer, "/%s %ld StartData\n", sf->fontname, len );
	fprintf(cff,"%%%%BeginData: %ld Binary Bytes\n", (long) (len+strlen(buffer)) );
	fputs(buffer,cff);
	if ( !ttfcopyfile(cff,at->cfff,ftell(cff),"CFF"))
	    at->error = true;
	fprintf(cff,"\n%%%%EndData\n" );
	fprintf(cff,"%%%%EndResource\n" );
	fprintf(cff,"%%%%EOF\n" );
    }
return( !at->error );
}

int _WriteTTFFont(FILE *ttf,SplineFont *sf,enum fontformat format,
	int32 *bsizes, enum bitmapformat bf,int flags,EncMap *map, int layer) {
    struct alltabs at;
    char *oldloc;
    int i, anyglyphs;

    oldloc = setlocale(LC_NUMERIC,"C");		/* TrueType probably doesn't need this, but OpenType does for floats in dictionaries */
    if ( format==ff_otfcid || format== ff_cffcid ) {
	if ( sf->cidmaster ) sf = sf->cidmaster;
    } else {
	if ( sf->subfontcnt!=0 ) sf = sf->subfonts[0];
    }

    if ( sf->subfontcnt==0 ) {
	anyglyphs = false;
	for ( i=sf->glyphcnt-1; i>0 ; --i ) {
	    if ( SCWorthOutputting(sf->glyphs[i])) {
		anyglyphs = true;
		if ( sf->glyphs[i]->unicodeenc!=-1 )
	break;
	    }
	}
	if ( !anyglyphs && !sf->internal_temp ) {
	    ff_post_error(_("No Encoded Glyphs"),_("Warning: Font contained no glyphs"));
	}
	if ( format!=ff_ttfsym && !(flags&ttf_flag_symbol) && !sf->internal_temp ) {
	    if ( i==0 && anyglyphs ) {
		if ( map->enccount<=256 ) {
		    char *buts[3];
		    buts[0] = _("_Yes"); buts[1] = _("_No"); buts[2] = NULL;
		    if ( ff_ask(_("No Encoded Glyphs"),(const char **) buts,0,1,_("This font contains no glyphs with unicode encodings.\nWould you like to use a \"Symbol\" encoding instead of Unicode?"))==0 )
			flags |= ttf_flag_symbol;
		} else
		    ff_post_error(_("No Encoded Glyphs"),_("This font contains no glyphs with unicode encodings.\nYou will probably not be able to use the output."));
	    }
	}
    }

    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL )
	sf->glyphs[i]->ttf_glyph = -1;

    memset(&at,'\0',sizeof(struct alltabs));
    at.gi.flags = flags;
    at.gi.layer = layer;
    at.gi.is_ttf = false;
    at.applemode = 0;
    at.opentypemode = 0;
    at.msbitmaps = false;
    at.applebitmaps = false;
    at.gi.onlybitmaps = false;
    at.gi.bsizes = bsizes;
    at.gi.fixed_width = CIDOneWidth(sf);
    at.isotf = false;
    at.format = format;
    at.next_strid = 256;
    at.sf = sf;
    at.map = map;

    if ( format==ff_cff || format==ff_cffcid ) {
	dumpcff(&at,sf,format,ttf);
    } else {
      /* TODO: remove top function layers here */
    }
    setlocale(LC_NUMERIC,oldloc);
    if ( at.error || ferror(ttf))
return( 0 );

#ifdef __CygWin
    /* Modern versions of windows want the execute bit set on a ttf file */
    /* I've no idea what this corresponds to in windows, nor any idea on */
    /*  how to set it from the windows UI, but this seems to work */
    {
	struct stat buf;
	fstat(fileno(ttf),&buf);
	fchmod(fileno(ttf),S_IXUSR | buf.st_mode );
    }
#endif

return( 1 );
}

int WriteTTFFont(char *fontname,SplineFont *sf,enum fontformat format,
	int32 *bsizes, enum bitmapformat bf,int flags,EncMap *map, int layer) {
    FILE *ttf;
    int ret;

    if ( strstr(fontname,"://")!=NULL ) {
	if (( ttf = tmpfile())==NULL )
return( 0 );
    } else {
	if (( ttf=fopen(fontname,"wb+"))==NULL )
return( 0 );
    }
    ret = _WriteTTFFont(ttf,sf,format,bsizes,bf,flags,map,layer);
    if ( fclose(ttf)==-1 )
return( 0 );
return( ret );
}

