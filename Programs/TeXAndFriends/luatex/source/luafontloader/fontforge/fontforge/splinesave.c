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
#include <stdio.h>
#include <math.h>
#include "splinefont.h"
#include "psfont.h"
#include <ustring.h>
#include <string.h>
#include <utype.h>

int autohint_before_generate = 1;

/* Let's talk about references. */
/* If we are doing Type1 output, then the obvious way of doing them is seac */
/*  but that's so limitting. It only works for exactly two characters both */
/*  of which are in Adobe's Standard Enc. Only translations allowed. Only */
/*  one reference may be translated and the width of the char must match */
/*  that of the non-translated reference */
/*   The first extension we can make is to allow a single character reference */
/*  by making the other character be a space */
/*   But if we want to do more than that we must use subrs. If we have two */
/*  refs in subrs then we can do translations by preceding the subr calls by */
/*  appropriate rmovetos. Actually the specs say that only one rmoveto should */
/*  precede a path, so that means we can't allow the subroutines to position */
/*  themselves, they must just assume that they are called with the current */
/*  position correct for the first point. But then we need to know where the */
/*  first point should be placed, so we allocate a BasePoint to hold that info*/
/*  and store it into the "keys" array (which the subrs don't use). Similarly */
/*  we need to know where the subr will leave us, so we actually allocate 2 */
/*  BasePoints, one containing the start point, one the end point */
/*   But that's still not good enough, hints are defined in such a way that */
/*  they are not relocateable. So our subrs can't include any hint definitions*/
/*  (or if they do then that subr can't be translated at all). So hints must */
/*  be set outside the subrs, and the subrs can't be for chars that need hint */
/*  substitution. Unless... The subr will never be relocated. */
/*   So we generate two types of reference subrs, one containing no hints, the*/
/*  other containing all the hints, stems and flexes. The first type may be */
/*  translated, the second cannot */
/* Type2 doesn't allow any seacs */
/*  So everything must go in subrs. We have a slightly different problem here:*/
/*  hintmasks need to know exactly how many stem hints there are in the char */
/*  so we can't include any hintmask operators inside a subr (unless we */
/*  guarantee that all invocations of that subr are done with the same number */
/*  of hints in the character). This again means that no char with hint subs- */
/*  titutions may be put in a subr. UNLESS all the other references in a */
/*  refering character contain no hints */

/* That's very complex. And it doesn't do a very good job. */
/* Instead let's take all strings bounded by either moveto or hintmask operators */
/*  store these as potential subroutines. So a glyph becomes a sequence of    */
/*  potential subroutine calls preceded by the glyph header (width, hint decl,*/
/*  counter declarations, etc.) and intersperced by hintmask/moveto operators */
/* Each time we get a potential subr we hash it and see if we've used that    */
/*  string before. If we have then we merge the two. Otherwise it's a new one.*/
/* Then at the end we see what strings get used often enough to go into subrs */
/*  we create the subrs array from that.				      */
/* Then each glyph. We insert the preamble. We check of the potential subroutine */
/*  became a real subroutine. If so we call it, else we insert the data inline*/
/*  Do the same for the next hintmask/moveto and potential subroutine...      */

/* Then, on top of that I tried generating some full glyph subroutines, and   */
/*  to my surprise, it just made things worse.                                */

typedef struct growbuf {
    unsigned char *pt;
    unsigned char *base;
    unsigned char *end;
} GrowBuf;

#define HSH_SIZE	511
/* In type2 charstrings we divide every character into bits where a bit is */
/* bounded by a hintmask/moveto. Each of these is a potential subroutine and */
/* is stored here */
typedef struct glyphinfo {
    struct potentialsubrs {
	uint8 *data;		/* the charstring of the subr */
	int len;		/* the length of the charstring */
	int idx;		/* initially index into psubrs array */
				/*  then index into subrs array or -1 if none */
	int cnt;		/* the usage count */
	int fd;			/* Which sub font is it in */
				/* -1 => used in more than one */
	int next;
	int full_glyph_index;	/* Into the glyphbits array */
				/* for full references */
	BasePoint *startstop;	/* Again for full references */
    } *psubrs;
    int pcnt, pmax;
    int hashed[HSH_SIZE];
    struct glyphbits {
	SplineChar *sc;
	int fd;			/* Which subfont is it in */
	int bcnt;
	struct bits {
	    uint8 *data;
	    int dlen;
	    int psub_index;
	} *bits;
	uint8 wasseac;
    } *gb, *active;
    SplineFont *sf;
    int layer;
    int glyphcnt;
    int subfontcnt;
    int bcnt, bmax;
    struct bits *bits;		/* For current glyph */
    const int *bygid;
    int justbroken;
    int instance_count;
} GlyphInfo;

struct mhlist {
    uint8 mask[HntMax/8];
    int subr;
    struct mhlist *next;
};

struct hintdb {
    uint8 mask[HntMax/8];
    int cnt;				/* number of hints */
    struct mhlist *sublist;
    struct pschars *subrs;
    /*SplineChar *sc;*/
    SplineChar **scs;
    int instance_count;
    unsigned int iscjk: 1;		/* If cjk then don't do stem3 hints */
					/* Will be done with counters instead */
	/* actually, most of the time we can't use stem3s, only if those three*/
	/* stems are always active and there are no other stems !(h/v)hasoverlap*/
    unsigned int noconflicts: 1;
    unsigned int startset: 1;
    unsigned int skiphm: 1;		/* Set when coming back to the start point of a contour. hintmask should be set the first time, not the second */
    unsigned int donefirsthm: 1;
    int cursub;				/* Current subr number */
    DBasePoint current;
    GlyphInfo *gi;
};

static void GIContentsFree(GlyphInfo *gi,SplineChar *dummynotdef) {
    int i,j;

    if ( gi->glyphcnt>0 && gi->gb[0].sc == dummynotdef ) {
	if ( dummynotdef->layers!=NULL ) {
	    SplinePointListsFree(dummynotdef->layers[gi->layer].splines);
	    dummynotdef->layers[gi->layer].splines = NULL;
	}
	StemInfosFree(dummynotdef->hstem);
	StemInfosFree(dummynotdef->vstem);
	dummynotdef->vstem = dummynotdef->hstem = NULL;
	free(dummynotdef->layers);
	dummynotdef->layers = NULL;
    }

    for ( i=0; i<gi->pcnt; ++i ) {
	free(gi->psubrs[i].data);
	free(gi->psubrs[i].startstop);
	gi->psubrs[i].data = NULL;
	gi->psubrs[i].startstop = NULL;
    }
    for ( i=0; i<gi->glyphcnt; ++i ) {
	for ( j=0; j<gi->gb[i].bcnt; ++j )
	    free(gi->gb[i].bits[j].data);
	free(gi->gb[i].bits);
	gi->gb[i].bits = NULL;
	gi->gb[i].bcnt = 0;
    }

    gi->pcnt = 0;
    gi->bcnt = 0;
    gi->justbroken = 0;
}

static void GIFree(GlyphInfo *gi,SplineChar *dummynotdef) {

    GIContentsFree(gi,dummynotdef);

    free(gi->gb);
    free(gi->psubrs);
    free(gi->bits);
}

static void GrowBuffer(GrowBuf *gb) {
    if ( gb->base==NULL ) {
	gb->base = gb->pt = galloc(200);
	gb->end = gb->base + 200;
    } else {
	int len = (gb->end-gb->base) + 400;
	int off = gb->pt-gb->base;
	gb->base = grealloc(gb->base,len);
	gb->end = gb->base + len;
	gb->pt = gb->base+off;
    }
}

static void StartNextSubroutine(GrowBuf *gb,struct hintdb *hdb) {
    GlyphInfo *gi;

    if ( hdb==NULL )
return;
    gi = hdb->gi;
    if ( gi==NULL )
return;
    /* Store everything in the grow buf into the data/dlen of the next bit */
    if ( gi->bcnt==-1 ) gi->bcnt = 0;
    if ( gi->bcnt>=gi->bmax )
	gi->bits = grealloc(gi->bits,(gi->bmax+=20)*sizeof(struct bits));
    gi->bits[gi->bcnt].dlen = gb->pt-gb->base;
    gi->bits[gi->bcnt].data = galloc(gi->bits[gi->bcnt].dlen);
    gi->bits[gi->bcnt].psub_index = -1;
    memcpy(gi->bits[gi->bcnt].data,gb->base,gi->bits[gi->bcnt].dlen);
    gb->pt = gb->base;
    gi->justbroken = false;
}

static int hashfunc(uint8 *data, int len) {
    uint8 *end = data+len;
    unsigned int hash = 0, r;

    while ( data<end ) {
	r = (hash>>30)&3;
	hash <<= 2;
	hash = (hash|r)&0xffffffff;
	hash ^= *data++;
    }
return( hash%HSH_SIZE );
}

static void BreakSubroutine(GrowBuf *gb,struct hintdb *hdb) {
    GlyphInfo *gi;
    struct potentialsubrs *ps;
    int hash;
    int pi;

    if ( hdb==NULL )
return;
    gi = hdb->gi;
    if ( gi==NULL )
return;
    /* The stuff before the first moveto in a glyph (the header that sets */
    /*  the width, sets up the hints, counters, etc.) can't go into a subr */
    if ( gi->bcnt==-1 ) {
	gi->bcnt=0;
	gi->justbroken = true;
return;
    } else if ( gi->justbroken )
return;
    /* Otherwise stuff everything in the growbuffer into a subr */
    hash = hashfunc(gb->base,gb->pt-gb->base);
    ps = NULL;
    for ( pi=gi->hashed[hash]; pi!=-1; pi=gi->psubrs[pi].next ) {
	ps = &gi->psubrs[pi];
	if ( ps->len==gb->pt-gb->base && memcmp(ps->data,gb->base,gb->pt-gb->base)==0 )
    break;
    }
    if ( pi==-1 ) {
	if ( gi->pcnt>=gi->pmax )
	    gi->psubrs = grealloc(gi->psubrs,(gi->pmax+=gi->glyphcnt)*sizeof(struct potentialsubrs));
	ps = &gi->psubrs[gi->pcnt];
	memset(ps,0,sizeof(*ps));	/* set cnt to 0 */
	ps->idx = gi->pcnt++;
	ps->len = gb->pt-gb->base;
	ps->data = galloc(ps->len);
	memcpy(ps->data,gb->base,ps->len);
	ps->next = gi->hashed[hash];
	gi->hashed[hash] = ps->idx;
	ps->fd = gi->active->fd;
	ps->full_glyph_index = -1;
    }
    if ( ps->fd!=gi->active->fd )
	ps->fd = -1;			/* used in multiple cid sub-fonts */
    gi->bits[gi->bcnt].psub_index = ps->idx;
    ++ps->cnt;
    gb->pt = gb->base;
    ++gi->bcnt;
    gi->justbroken = true;
}

static void MoveSubrsToChar(GlyphInfo *gi) {
    struct glyphbits *active;

    if ( gi==NULL )
return;
    active = gi->active;
    active->bcnt = gi->bcnt;
    active->bits = galloc(active->bcnt*sizeof(struct bits));
    memcpy(active->bits,gi->bits,active->bcnt*sizeof(struct bits));
    gi->bcnt = 0;
}

static int NumberHints(SplineChar *scs[MmMax], int instance_count) {
    int i,j, cnt=-1;
    StemInfo *s;

    for ( j=0; j<instance_count; ++j ) {
	for ( s=scs[j]->hstem, i=0; s!=NULL; s=s->next ) {
	    if ( i<HntMax )
		s->hintnumber = i++;
	    else
		s->hintnumber = -1;
	}
	for ( s=scs[j]->vstem; s!=NULL; s=s->next ) {
	    if ( i<HntMax )
		s->hintnumber = i++;
	    else
		s->hintnumber = -1;
	}
	if ( cnt==-1 )
	    cnt = i;
	else if ( cnt!=i )
	    IError("MM font with different hint counts");
    }
return( cnt );
}

void RefCharsFreeRef(RefChar *ref) {
    RefChar *rnext;

    while ( ref!=NULL ) {
	rnext = ref->next;
	/* don't free the splines */
	chunkfree(ref,sizeof(RefChar));
	ref = rnext;
    }
}

static void MarkTranslationRefs(SplineFont *sf,int layer) {
    int i;
    SplineChar *sc;
    RefChar *r;

    for ( i=0; i<sf->glyphcnt; ++i ) if ( (sc = sf->glyphs[i])!=NULL ) {
	for ( r = sc->layers[layer].refs; r!=NULL; r=r->next )
	    r->justtranslated = (r->transform[0]==1 && r->transform[3]==1 &&
		    r->transform[1]==0 && r->transform[2]==0);
    }
}

/* ************************************************************************** */
/* ********************** Type1 PostScript CharStrings ********************** */
/* ************************************************************************** */

static real myround( real pos, int round ) {
    if ( round )
return( rint( pos ));
    else
return( rint( pos*100. )/100. );
}

static void AddNumber(GrowBuf *gb, real pos, int round) {
    int dodiv = 0;
    int val;
    unsigned char *str;

    if ( gb->pt+8>=gb->end )
	GrowBuffer(gb);

    pos = rint(100*pos)/100;

    if ( !round && pos!=floor(pos)) {
	pos *= 100;
	dodiv = true;
    }
    str = gb->pt;
    val = rint(pos);
    if ( pos>=-107 && pos<=107 )
	*str++ = val+139;
    else if ( pos>=108 && pos<=1131 ) {
	val -= 108;
	*str++ = (val>>8)+247;
	*str++ = val&0xff;
    } else if ( pos>=-1131 && pos<=-108 ) {
	val = -val;
	val -= 108;
	*str++ = (val>>8)+251;
	*str++ = val&0xff;
    } else {
	*str++ = '\377';
	*str++ = (val>>24)&0xff;
	*str++ = (val>>16)&0xff;
	*str++ = (val>>8)&0xff;
	*str++ = val&0xff;
    }
    if ( dodiv ) {
	*str++ = 100+139;	/* 100 */
	*str++ = 12;		/* div (byte1) */
	*str++ = 12;		/* div (byte2) */
    }
    gb->pt = str;
}

/* When doing a multiple master font we have multiple instances of the same data */
/*  which must all be added, and then a call made to the appropriate blend routine */
/* This is complicated because all the data may not fit on the stack so we */
/*  may need to make multiple calls */
static void AddData(GrowBuf *gb, real data[MmMax][6], int instances, int num_coords,
	int round) {
    int allsame = true, alls[6];
    int i,j, chunk,min,max,subr;

    for ( j=0; j<num_coords; ++j ) {
	alls[j] = true;
	for ( i=1; i<instances; ++i ) {
	    if ( data[i][j]!=data[0][j] ) {
		alls[j] = false;
		allsame = false;
	    break;
	    }
	}
    }

    if ( allsame ) {		/* No need for blending */
				/*  Probably a normal font, but possible in an mm */
	for ( j=0; j<num_coords; ++j )
	    AddNumber(gb,data[0][j],round);
return;
    }

    chunk = 22/instances;
    if ( chunk == 5 ) chunk = 4;	/* No subroutine for 5 items */
    min = 0;
    while ( min<num_coords ) {
	while ( min<num_coords && alls[min] ) {
	    AddNumber(gb,data[0][min],round);
	    ++min;
	}
	max = min+chunk;
	if ( max>num_coords ) max = num_coords;
	while ( max-1>min && alls[max-1] )
	    --max;
	if ( max-min==5 ) max=min+4;
	if ( min<max ) {
	    for ( j=min; j<max; ++j )
		AddNumber(gb,data[0][j],round);
	    for ( j=min; j<max; ++j )
		for ( i=1; i<instances; ++i )
		    AddNumber(gb,data[i][j]-data[0][j],round);
	    subr = (j-min) + 4;
	    if ( j-min==6 ) subr = 9;
	    AddNumber(gb,subr,round);
	    if ( gb->pt+1>=gb->end )
		GrowBuffer(gb);
	    *gb->pt++ = 10;			/* callsubr */
	    min = j;
	}
    }
}

int CvtPsStem3(GrowBuf *gb, SplineChar *scs[MmMax], int instance_count,
	int ishstem, int round) {
    StemInfo *h1, *h2, *h3;
    StemInfo _h1, _h2, _h3;
    real data[MmMax][6];
    int i;
    real off;

    for ( i=0; i<instance_count; ++i ) {
	if ( (ishstem && scs[i]->hconflicts) || (!ishstem && scs[i]->vconflicts))
return( false );
	h1 = ishstem ? scs[i]->hstem : scs[i]->vstem;
	if ( h1==NULL || (h2 = h1->next)==NULL || (h3=h2->next)==NULL )
return( false );
	if ( h3->next!=NULL )
return( false );
	off = ishstem ? 0 : scs[i]->lsidebearing;
	if ( h1->width<0 ) {
	    _h1 = *h1;
	    _h1.start += _h1.width;
	    _h1.width = -_h1.width;
	    h1 = &_h1;
	}
	if ( h2->width<0 ) {
	    _h2 = *h2;
	    _h2.start += _h2.width;
	    _h2.width = -_h2.width;
	    h2 = &_h2;
	}
	if ( h3->width<0 ) {
	    _h3 = *h3;
	    _h3.start += _h3.width;
	    _h3.width = -_h3.width;
	    h3 = &_h3;
	}

	if ( h1->start>h2->start ) {
	    StemInfo *ht = h1; h1 = h2; h2 = ht;
	}
	if ( h1->start>h3->start ) {
	    StemInfo *ht = h1; h1 = h3; h3 = ht;
	}
	if ( h2->start>h3->start ) {
	    StemInfo *ht = h2; h2 = h3; h3 = ht;
	}
	if ( h1->width != h3->width )
return( false );
	if ( (h2->start+h2->width/2) - (h1->start+h1->width/2) !=
		(h3->start+h3->width/2) - (h2->start+h2->width/2) )
return( false );
	data[i][0] = h1->start-off;
	data[i][1] = h1->width;
	data[i][2] = h2->start-off;
	data[i][3] = h2->width;
	data[i][4] = h3->start-off;
	data[i][5] = h3->width;
    }
    if ( gb==NULL )
return( true );
    AddData(gb,data,instance_count,6,round);
    if ( gb->pt+3>=gb->end )
	GrowBuffer(gb);
    *(gb->pt)++ = 12;
    *(gb->pt)++ = ishstem?2:1;				/* h/v stem3 */
return( true );
}

static int _SCNeedsSubsPts(SplineChar *sc,int layer) {
    RefChar *ref;

    if ( sc->hstem==NULL && sc->vstem==NULL )
return( false );

    if ( sc->layers[layer].splines!=NULL )
return( sc->layers[layer].splines->first->hintmask==NULL );

    for ( ref = sc->layers[layer].refs; ref!=NULL; ref=ref->next )
	if ( ref->layers[0].splines!=NULL )
return( ref->layers[0].splines->first->hintmask==NULL );

return( false );		/* It's empty. that's easy. */
}

static int SCNeedsSubsPts(SplineChar *sc,enum fontformat format,int layer) {
    if ( (format!=ff_mma && format!=ff_mmb) || sc->parent->mm==NULL ) {
	if ( !sc->hconflicts && !sc->vconflicts )
return( false );		/* No conflicts, no swap-over points needed */
return( _SCNeedsSubsPts(sc,layer));
    } else {
	MMSet *mm = sc->parent->mm;
	int i;
	for ( i=0; i<mm->instance_count; ++i ) if ( sc->orig_pos<mm->instances[i]->glyphcnt ) {
	    if ( _SCNeedsSubsPts(mm->instances[i]->glyphs[sc->orig_pos],layer) )
return( true );
	}
return( false );
    }
}

/* Mark those glyphs which can live totally in subrs */
static void SplineFont2FullSubrs1(int flags,GlyphInfo *gi) {
    int i;
    SplineChar *sc;

    for ( i=0; i<gi->glyphcnt; ++i ) if ( (sc=gi->gb[i].sc)!=NULL )
	sc->ttf_glyph = 0x7fff;

}

int SFOneWidth(SplineFont *sf) {
    int width, i;

    width = -2;
    for ( i=0; i<sf->glyphcnt; ++i ) if ( SCWorthOutputting(sf->glyphs[i]) &&
	    (strcmp(sf->glyphs[i]->name,".notdef")!=0 || sf->glyphs[i]->layers[ly_fore].splines!=NULL)) {
	/* Only trust the width of notdef if it's got some content */
	/* (at least as far as fixed pitch determination goes) */
	if ( width==-2 ) width = sf->glyphs[i]->width;
	else if ( width!=sf->glyphs[i]->width ) {
	    width = -1;
    break;
	}
    }
return(width);
}

int CIDOneWidth(SplineFont *_sf) {
    int width, i;
    int k;
    SplineFont *sf;

    if ( _sf->cidmaster!=NULL ) _sf = _sf->cidmaster;
    width = -2;
    k=0;
    do {
	sf = _sf->subfonts==NULL? _sf : _sf->subfonts[k];
	for ( i=0; i<sf->glyphcnt; ++i ) if ( SCWorthOutputting(sf->glyphs[i]) &&
		strcmp(sf->glyphs[i]->name,".null")!=0 &&
		strcmp(sf->glyphs[i]->name,"nonmarkingreturn")!=0 &&
		(strcmp(sf->glyphs[i]->name,".notdef")!=0 || sf->glyphs[i]->layers[ly_fore].splines!=NULL)) {
	    /* Only trust the width of notdef if it's got some content */
	    /* (at least as far as fixed pitch determination goes) */
	    if ( width==-2 ) width = sf->glyphs[i]->width;
	    else if ( width!=sf->glyphs[i]->width ) {
		width = -1;
	break;
	    }
	}
	++k;
    } while ( k<_sf->subfontcnt );
return(width);
}


static void SetupType1Subrs(struct pschars *subrs,GlyphInfo *gi) {
    int scnt, call_size;
    int i;

    scnt = subrs->next;
    call_size = gi->pcnt+scnt<1131 ? 3 : 6;
    for ( i=0; i<gi->pcnt; ++i ) {
	/* A subroutine call takes somewhere between 2 and 6 bytes itself. */
	/*  and we must add a return statement to the end. We don't want to */
	/*  make things bigger */
	if ( gi->psubrs[i].full_glyph_index!=-1 )
	    gi->psubrs[i].idx = scnt++;
	else if ( gi->psubrs[i].cnt*gi->psubrs[i].len>(gi->psubrs[i].cnt*call_size)+gi->psubrs[i].len+1 )
	    gi->psubrs[i].idx = scnt++;
	else
	    gi->psubrs[i].idx = -1;
    }

    subrs->cnt = scnt;
    subrs->next = scnt;
    subrs->lens = grealloc(subrs->lens,scnt*sizeof(int));
    subrs->values = grealloc(subrs->values,scnt*sizeof(unsigned char *));

    for ( i=0; i<gi->pcnt; ++i ) {
	scnt = gi->psubrs[i].idx;
	if ( scnt==-1 || gi->psubrs[i].full_glyph_index != -1 )
    continue;
	subrs->lens[scnt] = gi->psubrs[i].len+1;
	subrs->values[scnt] = galloc(subrs->lens[scnt]);
	memcpy(subrs->values[scnt],gi->psubrs[i].data,gi->psubrs[i].len);
	subrs->values[scnt][gi->psubrs[i].len] = 11;	/* Add a return to end of subr */
    }
}

static void SetupType1Chrs(struct pschars *chrs,struct pschars *subrs,GlyphInfo *gi, int iscid) {
    int i,k,j;

    /* If a glyph lives entirely in a subroutine then we need to create both */
    /*  the subroutine entry, and the char entry which calls the subr. */
    /* The subroutine entry will be everything EXCEPT the glyph header */
    /* the char entry will be the glyph header and a subroutine call */
    /* If the glyph does not go into a subr then everything goes into the char */
    for ( i=0; i<gi->glyphcnt; ++i ) {
	int len=0;
	struct glyphbits *gb = &gi->gb[i];
	if ( gb->sc==NULL )
    continue;
	if ( !iscid )
	    chrs->keys[i] = copy(gb->sc->name);
	for ( k=0; k<2; ++k ) if ( k!=0 || gb->sc->ttf_glyph!=0x7fff ) {
	    uint8 *vals;
	    for ( j=0; j<gb->bcnt; ++j ) {
		if ( k!=0 || j!=0 )
		    len += gb->bits[j].dlen;
		if ( k==1 && gb->sc->ttf_glyph!=0x7fff ) {
		    int si = gi->psubrs[ gb->sc->ttf_glyph ].idx;
		    len += 1 + (si<=107?1:si<=1131?2:5);
	    break;
		}
		if ( gi->psubrs[ gb->bits[j].psub_index ].idx==-1 )
		    len += gi->psubrs[ gb->bits[j].psub_index ].len;
		else {
		    int si = gi->psubrs[ gb->bits[j].psub_index ].idx;
		    len += 1 + (si<=107?1:si<=1131?2:5);
		    /* Space for a subr call & the sub number to call */
		}
	    }
	    if ( k==0 ) {
		int si = gi->psubrs[ gb->sc->ttf_glyph ].idx;
		subrs->lens[si] = len+1;
		vals = subrs->values[si] = galloc(len+2);
	    } else {
		/* Don't need or want and endchar if we are using seac */
		chrs->lens[i] = len + !gb->wasseac;
		vals = chrs->values[i] = galloc(len+2); /* space for endchar and a final NUL (which is really meaningless, but makes me feel better) */
	    }

	    len = 0;
	    for ( j=0; j<gb->bcnt; ++j ) {
		int si;
		if ( k!=0 || j!=0 ) {
		    memcpy(vals+len,gb->bits[j].data,gb->bits[j].dlen);
		    len += gb->bits[j].dlen;
		}
		si = -1;
		if ( k==1 && gb->sc->ttf_glyph!=0x7fff )
		    si = gi->psubrs[ gb->sc->ttf_glyph ].idx;
		else if ( gi->psubrs[ gb->bits[j].psub_index ].idx==-1 ) {
		    memcpy(vals+len,gi->psubrs[ gb->bits[j].psub_index ].data,
			    gi->psubrs[ gb->bits[j].psub_index ].len);
		    len += gi->psubrs[ gb->bits[j].psub_index ].len;
		} else
		    si = gi->psubrs[ gb->bits[j].psub_index ].idx;
		if ( si!=-1 ) {
		    /* space for the number (subroutine index) */
		    if ( si<=107 )
			vals[len++] = si+139;
		    else if ( si>0 && si<=1131 ) {
			si-=108;
			vals[len++] = (si>>8)+247;
			vals[len++] = si&0xff;
		    } else {
			vals[len++] = '\377';
			vals[len++] = (si>>24)&0xff;
			vals[len++] = (si>>16)&0xff;
			vals[len++] = (si>>8)&0xff;
			vals[len++] = si&0xff;
		    }
		    /* space for the subroutine operator */
		    vals[len++] = 10;
		}
		if ( k==1 && gb->sc->ttf_glyph!=0x7fff )
	    break;
	    }
	    if ( k==0 ) {
		vals[len++] = 11;	/* return */
		vals[len] = '\0';
	    } else if ( gb->wasseac ) {
		/* Don't want an endchar */
		vals[len] = '\0';
	    } else {
		vals[len++] = 14;	/* endchar */
		vals[len] = '\0';
	    }
	}
    }
}

/* ************************************************************************** */
/* ********************** Type2 PostScript CharStrings ********************** */
/* ************************************************************************** */

#if 0
static int real_warn = false;
#endif

static real myround2(real pos, int round) {
    if ( round )
return( rint(pos));

return( rint(65536*pos)/65536 );
}

static void AddNumber2(GrowBuf *gb, real pos, int round) {
    int val, factor;
    unsigned char *str;

    if ( gb->pt+5>=gb->end )
	GrowBuffer(gb);

    pos = rint(65536*pos)/65536;
    if ( round )
	pos = rint(pos);

    str = gb->pt;
    if ( pos>32767.99 || pos<-32768 ) {
	/* same logic for big ints and reals */
	if ( pos>0x3fffffff || pos<-0x40000000 ) {
	    LogError( _("Number out of range: %g in type2 output (must be [-65536,65535])\n"),
		    pos );
	    if ( pos>0 ) pos = 0x3fffffff; else pos = -0x40000000;
	}
	for ( factor=2; factor<32768; factor<<=2 )
	    if ( pos/factor<32767.99 && pos/factor>-32768 )
	break;
	AddNumber2(gb,pos/factor,false);
	AddNumber2(gb,factor,false);
	if ( gb->pt+2>=gb->end )
	    GrowBuffer(gb);
	*(gb->pt++) = 0x0c;		/* Multiply operator */
	*(gb->pt++) = 0x18;
    } else if ( pos!=floor(pos )) {
	val = pos*65536;
	*str++ = '\377';
	*str++ = (val>>24)&0xff;
	*str++ = (val>>16)&0xff;
	*str++ = (val>>8)&0xff;
	*str++ = val&0xff;
    } else {
	val = rint(pos);
	if ( pos>=-107 && pos<=107 )
	    *str++ = val+139;
	else if ( pos>=108 && pos<=1131 ) {
	    val -= 108;
	    *str++ = (val>>8)+247;
	    *str++ = val&0xff;
	} else if ( pos>=-1131 && pos<=-108 ) {
	    val = -val;
	    val -= 108;
	    *str++ = (val>>8)+251;
	    *str++ = val&0xff;
	} else {
	    *str++ = 28;
	    *str++ = (val>>8)&0xff;
	    *str++ = val&0xff;
	}
    }
    gb->pt = str;
}

static void AddMask2(GrowBuf *gb,uint8 mask[12],int cnt, int oper) {
    int i;

    if ( gb->pt+1+((cnt+7)>>3)>=gb->end )
	GrowBuffer(gb);
    *gb->pt++ = oper;					/* hintmask,cntrmask */
    for ( i=0; i< ((cnt+7)>>3); ++i )
	*gb->pt++ = mask[i];
}

static void CounterHints2(GrowBuf *gb, SplineChar *sc, int hcnt) {
    int i;

    for ( i=0; i<sc->countermask_cnt; ++i )
	AddMask2(gb,sc->countermasks[i],hcnt,20);	/* cntrmask */
}

static int HintSetup2(GrowBuf *gb,struct hintdb *hdb, SplinePoint *to, int break_subr ) {

    /* We might get a point with a hintmask in a glyph with no conflicts */
    /* (ie. the initial point when we return to it at the end of the splineset*/
    /* in that case hdb->cnt will be 0 and we should ignore it */
    /* components in subroutines depend on not having any hintmasks */
    if ( to->hintmask==NULL || hdb->cnt==0 || hdb->noconflicts || hdb->skiphm )
return( false );

    if ( memcmp(hdb->mask,*to->hintmask,(hdb->cnt+7)/8)==0 )
return( false );

    if ( break_subr )
	BreakSubroutine(gb,hdb);

    AddMask2(gb,*to->hintmask,hdb->cnt,19);		/* hintmask */
    memcpy(hdb->mask,*to->hintmask,sizeof(HintMask));
    hdb->donefirsthm = true;
    if ( break_subr )
	StartNextSubroutine(gb,hdb);
return( true );
}

static void moveto2(GrowBuf *gb,struct hintdb *hdb,SplinePoint *to, int round) {
    BasePoint temp, *tom;

    if ( gb->pt+18 >= gb->end )
	GrowBuffer(gb);

    BreakSubroutine(gb,hdb);
    HintSetup2(gb,hdb,to,false);
    tom = &to->me;
    if ( round ) {
	temp.x = rint(tom->x);
	temp.y = rint(tom->y);
	tom = &temp;
    }
#if 0
    if ( hdb->current.x==tom->x && hdb->current.y==tom->y ) {
	/* we're already here */
	/* Yes, but a move is required anyway at char start */
    } else
#endif
    if ( hdb->current.x==tom->x ) {
	AddNumber2(gb,tom->y-hdb->current.y,round);
	*(gb->pt)++ = 4;		/* v move to */
    } else if ( hdb->current.y==tom->y ) {
	AddNumber2(gb,tom->x-hdb->current.x,round);
	*(gb->pt)++ = 22;		/* h move to */
    } else {
	AddNumber2(gb,tom->x-hdb->current.x,round);
	AddNumber2(gb,tom->y-hdb->current.y,round);
	*(gb->pt)++ = 21;		/* r move to */
    }
    hdb->current.x = rint(32768*tom->x)/32768;
    hdb->current.y = rint(32768*tom->y)/32768;
    StartNextSubroutine(gb,hdb);
}

static Spline *lineto2(GrowBuf *gb,struct hintdb *hdb,Spline *spline, Spline *done, int round) {
    int cnt, hv, hvcnt;
    Spline *test, *lastgood, *lasthvgood;
    BasePoint temp1, temp2, *tom, *fromm;
    int donehm;

    lastgood = NULL;
    for ( test=spline, cnt=0; test->knownlinear && cnt<15; ) {
	++cnt;
	lastgood = test;
	test = test->to->next;
	/* it will be smaller to use a closepath operator so ignore the */
	/*  ultimate spline */
	if ( test==done || test==NULL || test->to->next==done )
    break;
    }

    HintSetup2(gb,hdb,spline->to,true);

    hv = -1; hvcnt=1; lasthvgood = NULL;
    if ( spline->from->me.x==spline->to->me.x )
	hv = 1;		/* Vertical */
    else if ( spline->from->me.y==spline->to->me.y )
	hv = 0;		/* Horizontal */
    donehm = true;
    if ( hv!=-1 ) {
	lasthvgood = spline; hvcnt = 1;
	if ( cnt!=1 ) {
	    for ( test=spline->to->next; test!=NULL ; test = test->to->next ) {
		fromm = &test->from->me;
		if ( round ) {
		    temp2.x = rint(fromm->x);
		    temp2.y = rint(fromm->y);
		    fromm = &temp2;
		}
		tom = &test->to->me;
		if ( round ) {
		    temp1.x = rint(tom->x);
		    temp1.y = rint(tom->y);
		    tom = &temp1;
		}
		if ( hv==1 && tom->y==fromm->y )
		    hv = 0;
		else if ( hv==0 && tom->x==fromm->x )
		    hv = 1;
		else
	    break;
		lasthvgood = test;
		++hvcnt;
		if ( test==lastgood )
	    break;
	    }
	}
	donehm = true;
	if ( hvcnt==cnt || hvcnt>=2 ) {
	    /* It's more efficient to do some h/v linetos */
	    for ( test=spline; ; test = test->to->next ) {
		if ( !donehm && test->to->hintmask!=NULL )
	    break;
		donehm = false;
		fromm = &test->from->me;
		if ( round ) {
		    temp2.x = rint(fromm->x);
		    temp2.y = rint(fromm->y);
		    fromm = &temp2;
		}
		tom = &test->to->me;
		if ( round ) {
		    temp1.x = rint(tom->x);
		    temp1.y = rint(tom->y);
		    tom = &temp1;
		}
		if ( fromm->x==tom->x )
		    AddNumber2(gb,tom->y-fromm->y,round);
		else
		    AddNumber2(gb,tom->x-fromm->x,round);
		hdb->current.x = rint(32768*tom->x)/32768;
		hdb->current.y = rint(32768*tom->y)/32768;
		if ( test==lasthvgood ) {
		    test = test->to->next;
	    break;
		}
	    }
	    if ( gb->pt+1 >= gb->end )
		GrowBuffer(gb);
	    *(gb->pt)++ = spline->from->me.x==spline->to->me.x? 7 : 6;
return( test );
	}
    }

    for ( test=spline; test!=NULL; test = test->to->next ) {
	if ( !donehm && test->to->hintmask!=NULL )
    break;
	donehm = false;
	fromm = &test->from->me;
	if ( round ) {
	    temp2.x = rint(fromm->x);
	    temp2.y = rint(fromm->y);
	    fromm = &temp2;
	}
	tom = &test->to->me;
	if ( round ) {
	    temp1.x = rint(tom->x);
	    temp1.y = rint(tom->y);
	    tom = &temp1;
	}
	AddNumber2(gb,tom->x-fromm->x,round);
	AddNumber2(gb,tom->y-fromm->y,round);
	hdb->current.x = rint(32768*tom->x)/32768;
	hdb->current.y = rint(32768*tom->y)/32768;
	if ( test==lastgood ) {
	    test = test->to->next;
    break;
	}
    }
    if ( gb->pt+1 >= gb->end )
	GrowBuffer(gb);
    *(gb->pt)++ = 5;		/* r line to */
return( test );
}

static Spline *curveto2(GrowBuf *gb,struct hintdb *hdb,Spline *spline, Spline *done, int round) {
    int cnt=0, hv;
    Spline *first;
    DBasePoint start;
    int donehm;

    HintSetup2(gb,hdb,spline->to,true);

    hv = -1;
    if ( hdb->current.x==myround2(spline->from->nextcp.x,round) &&
	    myround2(spline->to->prevcp.y,round)==myround2(spline->to->me.y,round) )
	hv = 1;
    else if ( hdb->current.y==myround2(spline->from->nextcp.y,round) &&
	    myround2(spline->to->prevcp.x,round)==myround2(spline->to->me.x,round) )
	hv = 0;
    donehm = true;
    if ( hv!=-1 ) {
	first = spline; start = hdb->current;
	while (
		(hv==1 && hdb->current.x==myround2(spline->from->nextcp.x,round) &&
			myround2(spline->to->prevcp.y,round)==myround2(spline->to->me.y,round) ) ||
		(hv==0 && hdb->current.y==myround2(spline->from->nextcp.y,round) &&
			myround2(spline->to->prevcp.x,round)==myround2(spline->to->me.x,round) ) ) {
	    if ( !donehm && spline->to->hintmask!=NULL )
	break;
	    donehm = false;
	    if ( hv==1 ) {
		AddNumber2(gb,myround2(spline->from->nextcp.y,round)-hdb->current.y,round);
		AddNumber2(gb,myround2(spline->to->prevcp.x,round)-myround2(spline->from->nextcp.x,round),round);
		AddNumber2(gb,myround2(spline->to->prevcp.y,round)-myround2(spline->from->nextcp.y,round),round);
		AddNumber2(gb,myround2(spline->to->me.x,round)-myround2(spline->to->prevcp.x,round),round);
		hv = 0;
	    } else {
		AddNumber2(gb,myround2(spline->from->nextcp.x,round)-hdb->current.x,round);
		AddNumber2(gb,myround2(spline->to->prevcp.x,round)-myround2(spline->from->nextcp.x,round),round);
		AddNumber2(gb,myround2(spline->to->prevcp.y,round)-myround2(spline->from->nextcp.y,round),round);
		AddNumber2(gb,myround2(spline->to->me.y,round)-myround2(spline->to->prevcp.y,round),round);
		hv = 1;
	    }
	    hdb->current.x = myround2(spline->to->me.x,round);
	    hdb->current.y = myround2(spline->to->me.y,round);
	    ++cnt;
	    spline = spline->to->next;
	    if ( spline==done || spline==NULL || cnt>9 || spline->knownlinear )
	break;
	}
	if ( gb->pt+1 >= gb->end )
	    GrowBuffer(gb);
	*(gb->pt)++ = ( start.x==myround2(first->from->nextcp.x,round) && myround2(first->to->prevcp.y,round)==myround2(first->to->me.y,round) )?
		30:31;		/* vhcurveto:hvcurveto */
return( spline );
    }
    while ( cnt<6 ) {
	if ( !donehm && spline->to->hintmask!=NULL )
    break;
	donehm = false;
	hv = -1;
	if ( hdb->current.x==myround2(spline->from->nextcp.x,round) &&
		myround2(spline->to->prevcp.y,round)==myround2(spline->to->me.y,round) &&
		spline->to->next!=NULL &&
		myround2(spline->to->me.y,round)==myround2(spline->to->nextcp.y,round) &&
		myround2(spline->to->next->to->prevcp.x,round)==myround2(spline->to->next->to->me.x,round) )
    break;
	else if ( hdb->current.y==myround2(spline->from->nextcp.y,round) &&
		myround2(spline->to->prevcp.x,round)==myround2(spline->to->me.x,round) &&
		spline->to->next!=NULL &&
		myround2(spline->to->me.x,round)==myround2(spline->to->nextcp.x,round) &&
		myround2(spline->to->next->to->prevcp.y,round)==myround2(spline->to->next->to->me.y,round) )
    break;
	AddNumber2(gb,myround2(spline->from->nextcp.x,round)-hdb->current.x,round);
	AddNumber2(gb,myround2(spline->from->nextcp.y,round)-hdb->current.y,round);
	AddNumber2(gb,myround2(spline->to->prevcp.x,round)-myround2(spline->from->nextcp.x,round),round);
	AddNumber2(gb,myround2(spline->to->prevcp.y,round)-myround2(spline->from->nextcp.y,round),round);
	AddNumber2(gb,myround2(spline->to->me.x,round)-myround2(spline->to->prevcp.x,round),round);
	AddNumber2(gb,myround2(spline->to->me.y,round)-myround2(spline->to->prevcp.y,round),round);
	hdb->current.x = myround2(spline->to->me.x,round);
	hdb->current.y = myround2(spline->to->me.y,round);
	++cnt;
	spline = spline->to->next;
	if ( spline==done || spline==NULL || spline->knownlinear )
    break;
    }
    if ( gb->pt+1 >= gb->end )
	GrowBuffer(gb);
    *(gb->pt)++ = 8;		/* rrcurveto */
return( spline );
}

static void flexto2(GrowBuf *gb,struct hintdb *hdb,Spline *pspline,int round) {
    BasePoint *c0, *c1, *mid, *end, *nc0, *nc1;
    Spline *nspline;

    c0 = &pspline->from->nextcp;
    c1 = &pspline->to->prevcp;
    mid = &pspline->to->me;
    nspline = pspline->to->next;
    nc0 = &nspline->from->nextcp;
    nc1 = &nspline->to->prevcp;
    end = &nspline->to->me;

    HintSetup2(gb,hdb,nspline->to,true);

    if ( myround2(c0->y,round)==hdb->current.y && myround2(nc1->y,round)==hdb->current.y &&
	    myround2(end->y,round)==hdb->current.y &&
	    myround2(c1->y,round)==myround2(mid->y,round) && myround2(nc0->y,round)==myround2(mid->y,round) ) {
	if ( gb->pt+7*6+2 >= gb->end )
	    GrowBuffer(gb);
	AddNumber2(gb,myround2(c0->x,round)-hdb->current.x,round);
	AddNumber2(gb,myround2(c1->x,round)-myround2(c0->x,round),round);
	AddNumber2(gb,myround2(c1->y,round)-myround2(c0->y,round),round);
	AddNumber2(gb,myround2(mid->x,round)-myround2(c1->x,round),round);
	AddNumber2(gb,myround2(nc0->x,round)-myround2(mid->x,round),round);
	AddNumber2(gb,myround2(nc1->x,round)-myround2(nc0->x,round),round);
	AddNumber2(gb,myround2(end->x,round)-myround2(nc1->x,round),round);
	*gb->pt++ = 12; *gb->pt++ = 34;		/* hflex */
    } else {
	if ( gb->pt+11*6+2 >= gb->end )
	    GrowBuffer(gb);
	AddNumber2(gb,myround2(c0->x,round)-hdb->current.x,round);
	AddNumber2(gb,myround2(c0->y,round)-hdb->current.y,round);
	AddNumber2(gb,myround2(c1->x,round)-myround2(c0->x,round),round);
	AddNumber2(gb,myround2(c1->y,round)-myround2(c0->y,round),round);
	AddNumber2(gb,myround2(mid->x,round)-myround2(c1->x,round),round);
	AddNumber2(gb,myround2(mid->y,round)-myround2(c1->y,round),round);
	AddNumber2(gb,myround2(nc0->x,round)-myround2(mid->x,round),round);
	AddNumber2(gb,myround2(nc0->y,round)-myround2(mid->y,round),round);
	AddNumber2(gb,myround2(nc1->x,round)-myround2(nc0->x,round),round);
	AddNumber2(gb,myround2(nc1->y,round)-myround2(nc0->y,round),round);
	if ( hdb->current.y==myround2(end->y,round) )
	    AddNumber2(gb,myround2(end->x,round)-myround2(nc1->x,round),round);
	else
	    AddNumber2(gb,myround2(end->y,round)-myround2(nc1->y,round),round);
	*gb->pt++ = 12; *gb->pt++ = 37;		/* flex1 */
    }

    hdb->current.x = rint(32768*end->x)/32768;
    hdb->current.y = rint(32768*end->y)/32768;
}

static void CvtPsSplineSet2(GrowBuf *gb, SplinePointList *spl,
	struct hintdb *hdb, int is_order2,int round ) {
    Spline *spline, *first;
    SplinePointList temp, *freeme = NULL;
    int unhinted = true;;

    if ( is_order2 )
	freeme = spl = SplineSetsPSApprox(spl);

    for ( ; spl!=NULL; spl = spl->next ) {
	first = NULL;
	SplineSetReverse(spl);
	/* PostScript and TrueType store their splines in in reverse */
	/*  orientations. Annoying. Oh well. I shall adopt TrueType and */
	/*  If I reverse the PS splinesets after reading them in, and then */
	/*  again when saving them out, all should be well */
	if ( spl->first->flexy || spl->first->flexx ) {
	    /* can't handle a flex (mid) point as the first point. rotate the */
	    /* list by one, this is possible because only closed paths have */
	    /* points marked as flex, and because we can't have two flex mid- */
	    /* points in a row */
	    if ( spl->first->hintmask==NULL || spl->first->next->to->hintmask!=NULL ) {
		/* But we can't rotate it if we expect it to provide us with */
		/*  a hintmask.                 			     */
		temp = *spl;
		temp.first = temp.last = spl->first->next->to;
		spl = &temp;
	    }
	    if ( spl->first->flexy || spl->first->flexx ) {
		/* If we couldn't rotate, or if we rotated to something that */
		/*  also is flexible, then just turn off flex. That's safe   */
		spl->first->flexx = spl->first->flexy = false;
	    }
	}
	if ( unhinted && hdb->cnt>0 && spl->first->hintmask!=NULL ) {
	    hdb->mask[0] = ~(*spl->first->hintmask)[0];	/* Make it different */
	    unhinted = false;
	}
	moveto2(gb,hdb,spl->first,round);
	for ( spline = spl->first->next; spline!=NULL && spline!=first; ) {
	    if ( first==NULL ) first = spline;
	    else if ( first->from==spline->to )
		hdb->skiphm = true;
	    if ( spline->to->flexx || spline->to->flexy ) {
		flexto2(gb,hdb,spline,round);	/* does two adjacent splines */
		spline = spline->to->next->to->next;
	    } else if ( spline->knownlinear && spline->to == spl->first )
		/* In Type2 we don't even need a closepath to finish this off */
		/*  (which is good, because there isn't a close path) */
	break;
	    else if ( spline->knownlinear )
		spline = lineto2(gb,hdb,spline,first,round);
	    else
		spline = curveto2(gb,hdb,spline,first,round);
	}
	hdb->skiphm = false;
	/* No closepath oper in type2 fonts, it's implied */
	SplineSetReverse(spl);
	/* Of course, I have to Reverse again to get back to my convention after*/
	/*  saving */
    }
    SplinePointListsFree(freeme);
}

static void DumpHints(GrowBuf *gb,StemInfo *h,int oper,int midoper,int round) {
    real last = 0, cur;
    int cnt;

    if ( h==NULL )
return;
    cnt = 0;
    while ( h!=NULL && h->hintnumber!=-1 ) {
	/* Type2 hints do not support negative widths except in the case of */
	/*  ghost (now called edge) hints */
	if ( cnt>24-1 ) {	/* stack max = 48 numbers, => 24 hints, leave a bit of slop for the width */
	    if ( gb->pt+1>=gb->end )
		GrowBuffer(gb);
	    *gb->pt++ = midoper;
	    cnt = 0;
	}
	cur = myround2(h->start,round) + myround2(h->width,round);
	if ( h->width<0 ) {
	    AddNumber2(gb,cur-last,round);
	    AddNumber2(gb,-myround2(h->width,round),round);
	    cur -= myround2(h->width,round);
	} else if ( h->ghost ) {
	    if ( h->width==20 ) {
		AddNumber2(gb,myround2(h->start,round)-last+20,round);
		AddNumber2(gb,-20,round);
		cur = myround2(h->start,round);
	    } else {
		AddNumber2(gb,myround2(h->start+21,round)-last,round);
		AddNumber2(gb,-21,round);
		cur = myround2(h->start+21,round)-21;
	    }
	} else {
	    AddNumber2(gb,myround2(h->start,round)-last,round);
	    AddNumber2(gb,myround2(h->width,round),round);
	}
	last = cur;
	h = h->next;
	++cnt;
    }
    if ( oper!=-1 ) {
	if ( gb->pt+1>=gb->end )
	    GrowBuffer(gb);
	*gb->pt++ = oper;
    }
}

static void DumpRefsHints(GrowBuf *gb, struct hintdb *hdb,RefChar *cur,StemInfo *h,StemInfo *v,
	BasePoint *trans, int round,int layer) {
    uint8 masks[12];
    int cnt, sets=0;
    StemInfo *rs;

    /* trans has already been rounded (whole char is translated by an integral amount) */

    /* If we have a subroutine containing conflicts, then its hints will match*/
    /*  ours exactly, and we can use its hintmasks directly */
    if (( cur->sc->hconflicts || cur->sc->vconflicts ) &&
	    cur->sc->layers[layer].splines!=NULL &&
	    cur->sc->layers[layer].splines->first->hintmask!=NULL ) {
	AddMask2(gb,*cur->sc->layers[layer].splines->first->hintmask,hdb->cnt,19);		/* hintmask */
	hdb->donefirsthm = true;
	memcpy(hdb->mask,*cur->sc->layers[layer].splines->first->hintmask,sizeof(HintMask));
return;
    }

    if ( h==NULL && v==NULL )
	IError("hintmask invoked when there are no hints");
    memset(masks,'\0',sizeof(masks));
    cnt = 0;
    while ( h!=NULL && h->hintnumber>=0 ) {
	/* Horizontal stems are defined by vertical bounds */
	real pos = (round ? rint(h->start) : h->start) - trans->y;
	for ( rs = cur->sc->hstem; rs!=NULL; rs=rs->next ) {
	    real rpos = round ? rint(rs->start) : rs->start;
	    if ( rpos==pos && (round ? (rint(rs->width)==rint(h->width)) : (rs->width==h->width)) ) {
		masks[h->hintnumber>>3] |= 0x80>>(h->hintnumber&7);
		++sets;
	break;
	    } else if ( rpos>pos )
	break;
	}
	h = h->next; ++cnt;
    }
    while ( v!=NULL && v->hintnumber>=0 ) {
	real pos = (round ? rint(v->start) : v->start) - trans->x;
	for ( rs = cur->sc->vstem; rs!=NULL; rs=rs->next ) {
	    real rpos = round ? rint(rs->start) : rs->start;
	    if ( rpos==pos && (round ? (rint(rs->width)==rint(v->width)) : (rs->width==v->width)) ) {
		masks[v->hintnumber>>3] |= 0x80>>(v->hintnumber&7);
		++sets;
	break;
	    } else if ( rpos>pos )
	break;
	}
	v = v->next; ++cnt;
    }
    BreakSubroutine(gb,hdb);
    hdb->donefirsthm = true;
    /* if ( sets!=0 ) */	/* First ref will need a hintmask even if it has no hints (if there are conflicts) */
	AddMask2(gb,masks,cnt,19);		/* hintmask */
}

static void DummyHintmask(GrowBuf *gb,struct hintdb *hdb) {
    HintMask hm;

    memset(hm,0,sizeof(hm));
    if ( hdb->cnt!=0 ) {
	BreakSubroutine(gb,hdb);
	hdb->donefirsthm = true;
	AddMask2(gb,hm,hdb->cnt,19);		/* hintmask */
    }
}

static void SetTransformedHintMask(GrowBuf *gb,struct hintdb *hdb,
	SplineChar *sc, RefChar *ref, BasePoint *trans, int round) {
    HintMask hm;

    if ( HintMaskFromTransformedRef(ref,trans,sc,&hm)!=NULL ) {
	BreakSubroutine(gb,hdb);
	hdb->donefirsthm = true;
	AddMask2(gb,hm,hdb->cnt,19);		/* hintmask */
    } else if ( !hdb->donefirsthm )
	DummyHintmask(gb,hdb);
}

static void ExpandRef2(GrowBuf *gb, SplineChar *sc, struct hintdb *hdb,
	RefChar *r, BasePoint *trans,
	struct pschars *subrs, int round,int layer) {
    BasePoint *bpt;
    BasePoint temp, rtrans;
    GlyphInfo *gi;
    /* The only refs I deal with here have no hint conflicts within them */
    
    rtrans.x = r->transform[4]+trans->x;
    rtrans.y = r->transform[5]+trans->y;
    if ( round ) {
	rtrans.x = rint(rtrans.x);
	rtrans.y = rint(rtrans.y);
    }

    BreakSubroutine(gb,hdb);
    if ( hdb->cnt>0 && !hdb->noconflicts )
	DumpRefsHints(gb,hdb,r,sc->hstem,sc->vstem,&rtrans,round,layer);

    /* Translate from end of last character to where this one should */
    /*  start (we must have one moveto operator to start off, none */
    /*  in the subr) */
    bpt = hdb->gi->psubrs[r->sc->lsidebearing].startstop;
    temp.x = bpt[0].x+rtrans.x;
    temp.y = bpt[0].y+rtrans.y;
    if ( hdb->current.x!=temp.x )
	AddNumber2(gb,temp.x-hdb->current.x,round);
    if ( hdb->current.y!=temp.y || hdb->current.x==temp.x )
	AddNumber2(gb,temp.y-hdb->current.y,round);
    if ( gb->pt+1>=gb->end )
	GrowBuffer(gb);
    *gb->pt++ = hdb->current.x==temp.x?4:	/* vmoveto */
		hdb->current.y==temp.y?22:	/* hmoveto */
		21;				/* rmoveto */
    if ( r->sc->lsidebearing==0x7fff )
	IError("Attempt to reference an unreferenceable glyph %s", r->sc->name );

    gi = hdb->gi;
    StartNextSubroutine(gb,hdb);
    gi->bits[gi->bcnt].psub_index = r->sc->lsidebearing;
    ++gi->bcnt;
    gi->justbroken = true;
    hdb->current.x = bpt[1].x+rtrans.x;
    hdb->current.y = bpt[1].y+rtrans.y;
}

static void RSC2PS2(GrowBuf *gb, SplineChar *base,SplineChar *rsc,
	struct hintdb *hdb, BasePoint *trans, struct pschars *subrs,
	int flags, int layer ) {
    BasePoint subtrans;
    int stationary = trans->x==0 && trans->y==0;
    RefChar *r, *unsafe=NULL;
    int unsafecnt=0, allwithouthints=true;
    int round = (flags&ps_flag_round)? true : false;
    StemInfo *oldh = NULL, *oldv = NULL;
    int hc = 0, vc = 0;
    SplineSet *freeme, *temp;
    int wasntconflicted = hdb->noconflicts;

    if ( flags&ps_flag_nohints ) {
	oldh = rsc->hstem; oldv = rsc->vstem;
	hc = rsc->hconflicts; vc = rsc->vconflicts;
	rsc->hstem = NULL; rsc->vstem = NULL;
	rsc->hconflicts = false; rsc->vconflicts = false;
    } else {
	for ( r=rsc->layers[layer].refs; r!=NULL; r=r->next ) {
	    if ( !r->justtranslated )
	continue;
	    if ( r->sc->hconflicts || r->sc->vconflicts ) {
		++unsafecnt;
		unsafe = r;
	    } else if ( r->sc->hstem!=NULL || r->sc->vstem!=NULL )
		allwithouthints = false;
	}
	if ( !stationary )
	    allwithouthints = false;
	if ( allwithouthints && unsafe!=NULL && hdb->cnt!=NumberHints(&unsafe->sc,1)) 
	    allwithouthints = false;		/* There are other hints elsewhere in the base glyph */
    }

    if ( unsafe && allwithouthints ) {
	if ( unsafe->sc->lsidebearing!=0x7fff ) {
	    ExpandRef2(gb,base,hdb,unsafe,trans,subrs,round,layer);
	} else if ( unsafe->transform[4]==0 && unsafe->transform[5]==0 )
	    RSC2PS2(gb,base,unsafe->sc,hdb,trans,subrs,flags,layer);
	else
	    unsafe = NULL;
    } else
	unsafe = NULL;

    /* What is the hintmask state here? It should not matter */
    freeme = NULL; temp = rsc->layers[layer].splines;
    if ( base!=rsc )
	temp = freeme = SPLCopyTranslatedHintMasks(temp,base,rsc,trans);
    CvtPsSplineSet2(gb,temp,hdb,rsc->layers[layer].order2,round);
    SplinePointListsFree(freeme);

    for ( r = rsc->layers[layer].refs; r!=NULL; r = r->next ) if ( r!=unsafe ) {
	if ( !r->justtranslated ) {
	    if ( !r->sc->hconflicts && !r->sc->vconflicts && !hdb->noconflicts &&
		    r->transform[1]==0 && r->transform[2]==0 &&
		    r->transform[0]>0 && r->transform[3]>0 )
		SetTransformedHintMask(gb,hdb,base,r,trans,round);
	    if ( !hdb->donefirsthm )
		DummyHintmask(gb,hdb);
	    temp = SPLCopyTransformedHintMasks(r,base,trans,layer);
	    CvtPsSplineSet2(gb,temp,hdb,rsc->layers[layer].order2,round);
	    SplinePointListsFree(temp);
	} else if ( r->sc->lsidebearing!=0x7fff &&
		((flags&ps_flag_nohints) ||
		 (!r->sc->hconflicts && !r->sc->vconflicts)) ) {
	    ExpandRef2(gb,base,hdb,r,trans,subrs,round,layer);
	} else {
	    subtrans.x = trans->x + r->transform[4];
	    subtrans.y = trans->y + r->transform[5];
	    if ( !hdb->noconflicts && !r->sc->hconflicts && !r->sc->vconflicts) {
		SetTransformedHintMask(gb,hdb,base,r,trans,round);
		hdb->noconflicts = true;
	    }
	    RSC2PS2(gb,base,r->sc,hdb,&subtrans,subrs,flags,layer);
	    hdb->noconflicts = wasntconflicted;
	}
    }

    if ( flags&ps_flag_nohints ) {
	rsc->hstem = oldh; rsc->vstem = oldv;
	rsc->hconflicts = hc; rsc->vconflicts = vc;
    }
}

static unsigned char *SplineChar2PS2(SplineChar *sc,int *len, int nomwid,
	int defwid, struct pschars *subrs, int flags,
	GlyphInfo *gi) {
    GrowBuf gb;
    unsigned char *ret;
    struct hintdb hdb;
    StemInfo *oldh = NULL, *oldv = NULL;
    int hc = 0, vc = 0;
    SplineChar *scs[MmMax];
    int round = (flags&ps_flag_round)? true : false;
    HintMask *hm = NULL;
    BasePoint trans;

    if ( !(flags&ps_flag_nohints) && SCNeedsSubsPts(sc,ff_otf,gi->layer))
	SCFigureHintMasks(sc,gi->layer);

    if ( flags&ps_flag_nohints ) {
	oldh = sc->hstem; oldv = sc->vstem;
	hc = sc->hconflicts; vc = sc->vconflicts;
	sc->hstem = NULL; sc->vstem = NULL;
	sc->hconflicts = false; sc->vconflicts = false;
    } else if ( sc->layers[gi->layer].splines!=NULL && !sc->vconflicts &&
	    !sc->hconflicts ) {
	hm = sc->layers[gi->layer].splines->first->hintmask;
	sc->layers[gi->layer].splines->first->hintmask = NULL;
    }

    memset(&gb,'\0',sizeof(gb));

    GrowBuffer(&gb);

    /* store the width on the stack */
    if ( sc->width==defwid )
	/* Don't need to do anything for the width */;
    else
	AddNumber2(&gb,sc->width-nomwid,round);

    memset(&trans,'\0',sizeof(trans));
    memset(&hdb,'\0',sizeof(hdb));
    hdb.scs = scs;
    hdb.gi = gi;
    if ( gi!=NULL )
	gi->bcnt = -1;
    scs[0] = sc;
    hdb.noconflicts = !sc->hconflicts && !sc->vconflicts;
    hdb.cnt = NumberHints(hdb.scs,1);
    DumpHints(&gb,sc->hstem,sc->hconflicts || sc->vconflicts?18:1,
			    sc->hconflicts || sc->vconflicts?18:1,round);
    DumpHints(&gb,sc->vstem,sc->hconflicts || sc->vconflicts?-1:3,
			    sc->hconflicts || sc->vconflicts?23:3,round);
    CounterHints2(&gb, sc, hdb.cnt );
    RSC2PS2(&gb,sc,sc,&hdb,&trans,subrs,flags,gi->layer);

    if ( gi->bcnt==-1 ) {	/* If it's whitespace */
	gi->bcnt = 0;
	StartNextSubroutine(&gb,&hdb);
    }
    BreakSubroutine(&gb,&hdb);
    MoveSubrsToChar(gi);
    ret = NULL;

    free(gb.base);
    if ( flags&ps_flag_nohints ) {
	sc->hstem = oldh; sc->vstem = oldv;
	sc->hconflicts = hc; sc->vconflicts = vc;
    } else if ( hm!=NULL )
	sc->layers[gi->layer].splines->first->hintmask = hm;
return( ret );
}

static SplinePoint *FFLineTo(SplinePoint *last, int x, int y) {
    SplinePoint *sp = SplinePointCreate(x,y);
    SplineMake3(last,sp);
return( sp );
}

static void Type2NotDefSplines(SplineFont *sf,SplineChar *sc,int layer) {
    /* I'd always assumed that Type2 notdefs would look like type1 notdefs */
    /*  but they don't, they look like truetype notdefs. And Ralf Stubner */
    /*  points out that the spec says they should. So make a box here */
    int stem, ymax;
    SplineSet *inner, *ss;
    StemInfo *h, *hints;

    stem = (sf->ascent+sf->descent)/20;
    ymax = 2*sf->ascent/3;

    ss = chunkalloc(sizeof(SplineSet));
    ss->first = ss->last = SplinePointCreate(stem,0);
    ss->last = FFLineTo(ss->last,stem,ymax);
    ss->last = FFLineTo(ss->last,sc->width-stem,ymax);
    ss->last = FFLineTo(ss->last,sc->width-stem,0);
    SplineMake3(ss->last,ss->first);
    ss->last = ss->first;

    ss->next = inner = chunkalloc(sizeof(SplineSet));
    inner->first = inner->last = SplinePointCreate(2*stem,stem);
    inner->last = FFLineTo(inner->last,sc->width-2*stem,stem);
    inner->last = FFLineTo(inner->last,sc->width-2*stem,ymax-stem);
    inner->last = FFLineTo(inner->last,2*stem,ymax-stem);
    SplineMake3(inner->last,inner->first);
    inner->last = inner->first;

    sc->layers[layer].splines = ss;

    hints = chunkalloc(sizeof(StemInfo));
    hints->start = stem;
    hints->width = stem;
    hints->next = h = chunkalloc(sizeof(StemInfo));
    h->start = sc->width-2*stem;
    h->width = stem;
    sc->vstem = hints;

    hints = chunkalloc(sizeof(StemInfo));
    hints->start = 0;
    hints->width = stem;
    hints->next = h = chunkalloc(sizeof(StemInfo));
    h->start = ymax-stem;
    h->width = stem;
    sc->hstem = hints;
}

/* Mark those glyphs which can live totally in subrs */
static void SplineFont2FullSubrs2(int flags,GlyphInfo *gi) {
    int i;
    SplineChar *sc;

    for ( i=0; i<gi->glyphcnt; ++i ) if ( (sc=gi->gb[i].sc)!=NULL )
	sc->lsidebearing = 0x7fff;

}

struct pschars *SplineFont2ChrsSubrs2(SplineFont *sf, int nomwid, int defwid,
	const int *bygid, int cnt, int flags, struct pschars **_subrs, int layer) {
    struct pschars *subrs, *chrs;
    int i,j,k,scnt;
    SplineChar *sc;
    GlyphInfo gi;
    SplineChar dummynotdef;

    memset(&gi,0,sizeof(gi));
    memset(&gi.hashed,-1,sizeof(gi.hashed));
    gi.instance_count = 1;
    gi.sf = sf;
    gi.layer = layer;
    gi.glyphcnt = cnt;
    gi.bygid = bygid;
    gi.gb = gcalloc(cnt,sizeof(struct glyphbits));
    gi.pmax = 3*cnt;
    gi.psubrs = galloc(gi.pmax*sizeof(struct potentialsubrs));
    for ( i=0; i<cnt; ++i ) {
	int gid = bygid[i];
	if ( i==0 && gid==-1 ) {
	    sc = &dummynotdef;
	    memset(sc,0,sizeof(dummynotdef));
	    dummynotdef.name = ".notdef";
	    dummynotdef.parent = sf;
	    dummynotdef.layer_cnt = sf->layer_cnt;
	    dummynotdef.layers = gcalloc(sf->layer_cnt,sizeof(Layer));
	    dummynotdef.width = SFOneWidth(sf);
	    if ( dummynotdef.width==-1 )
		dummynotdef.width = (sf->ascent+sf->descent)/2;
	    Type2NotDefSplines(sf,&dummynotdef,layer);
	} else if ( gid!=-1 )
	    sc = sf->glyphs[gid];
	else
    continue;
	gi.gb[i].sc = sc;
	sc->lsidebearing = 0x7fff;
    }
    MarkTranslationRefs(sf,layer);
    SplineFont2FullSubrs2(flags,&gi);

    for ( i=0; i<cnt; ++i ) {
	if ( (sc = gi.gb[i].sc)==NULL )
    continue;
	gi.active = &gi.gb[i];
	SplineChar2PS2(sc,NULL,nomwid,defwid,NULL,flags,&gi);
	ff_progress_next();
    }

    for ( i=scnt=0; i<gi.pcnt; ++i ) {
	/* A subroutine call takes somewhere between 2 and 4 bytes itself. */
	/*  and we must add a return statement to the end. We don't want to */
	/*  make things bigger */
	/* if we have more than 65535 subrs a subr call can take 9 bytes */
	if ( gi.psubrs[i].full_glyph_index!=-1 )
	    gi.psubrs[i].idx = scnt++;
	else if ( gi.psubrs[i].cnt*gi.psubrs[i].len>(gi.psubrs[i].cnt*4)+gi.psubrs[i].len+1 )
	    gi.psubrs[i].idx = scnt++;
	else
	    gi.psubrs[i].idx = -1;
    }
    subrs = gcalloc(1,sizeof(struct pschars));
    subrs->cnt = scnt;
    subrs->next = scnt;
    subrs->lens = galloc(scnt*sizeof(int));
    subrs->values = galloc(scnt*sizeof(unsigned char *));
    subrs->bias = scnt<1240 ? 107 :
		  scnt<33900 ? 1131 : 32768;
    for ( i=0; i<gi.pcnt; ++i ) {
	if ( gi.psubrs[i].idx != -1 ) {
	    scnt = gi.psubrs[i].idx;
	    subrs->lens[scnt] = gi.psubrs[i].len+1;
	    subrs->values[scnt] = galloc(subrs->lens[scnt]);
	    memcpy(subrs->values[scnt],gi.psubrs[i].data,gi.psubrs[i].len);
	    subrs->values[scnt][gi.psubrs[i].len] = 11;	/* Add a return to end of subr */
	}
    }

    chrs = gcalloc(1,sizeof(struct pschars));
    chrs->cnt = cnt;
    chrs->next = cnt;
    chrs->lens = galloc(cnt*sizeof(int));
    chrs->values = galloc(cnt*sizeof(unsigned char *));
    chrs->keys = galloc(cnt*sizeof(char *));
    for ( i=0; i<cnt; ++i ) {
	int len=0;
	uint8 *vals;
	struct glyphbits *gb = &gi.gb[i];
	if ( gb->sc==NULL )
    continue;
	chrs->keys[i] = copy(gb->sc->name);
	for ( k=0; k<2; ++k ) if ( k!=0 || gb->sc->lsidebearing!=0x7fff ) {
	    for ( j=0; j<gb->bcnt; ++j ) {
		if ( k!=0 || j!=0 )
		    len += gb->bits[j].dlen;
		if ( k==1 && gb->sc->lsidebearing!=0x7fff ) {
		    int si = gi.psubrs[ gb->sc->lsidebearing ].idx;
		    len += 1 + (si<=107 && si>=-107?1:si<=1131 && si>=-1131?2:si>=-32768 && si<32767?3:8);
	    break;
		}
		if ( gi.psubrs[ gb->bits[j].psub_index ].idx==-1 )
		    len += gi.psubrs[ gb->bits[j].psub_index ].len;
		else {
		    int si = gi.psubrs[ gb->bits[j].psub_index ].idx - subrs->bias;
		    /* space for the number (subroutine index) */
		    if ( si>=-107 && si<=107 )
			++len;
		    else if ( si>=-1131 && si<=1131 )
			len += 2;
		    else if ( si>=-32768 && si<=32767 )
			len += 3;
		    else
			len += 8;
		    /* space for the subroutine operator */
		    ++len;
		}
	    }
	    if ( k==0 ) {
		int si = gi.psubrs[ gb->sc->lsidebearing ].idx;
		subrs->lens[si] = len+1;
		vals = subrs->values[si] = galloc(len+2);
	    } else {
		chrs->lens[i] = len+1;
		vals = chrs->values[i] = galloc(len+2); /* space for endchar and a final NUL (which is really meaningless, but makes me feel better) */
	    }

	    len = 0;
	    for ( j=0; j<gb->bcnt; ++j ) {
		int si;
		if ( k!=0 || j!=0 ) {
		    memcpy(vals+len,gb->bits[j].data,gb->bits[j].dlen);
		    len += gb->bits[j].dlen;
		}
		si = 0x80000000;
		if ( k==1 && gb->sc->lsidebearing!=0x7fff )
		    si = gi.psubrs[ gb->sc->lsidebearing ].idx - subrs->bias;
		else if ( gi.psubrs[ gb->bits[j].psub_index ].idx==-1 ) {
		    memcpy(vals+len,gi.psubrs[ gb->bits[j].psub_index ].data,
			    gi.psubrs[ gb->bits[j].psub_index ].len);
		    len += gi.psubrs[ gb->bits[j].psub_index ].len;
		} else
		    si = gi.psubrs[ gb->bits[j].psub_index ].idx - subrs->bias;
		if ( si!=0x80000000 ) {
		    /* space for the number (subroutine index) */
		    if ( si>=-107 && si<=107 )
			vals[len++] = si+139;
		    else if ( si>0 && si<=1131 ) {
			si-=108;
			vals[len++] = (si>>8)+247;
			vals[len++] = si&0xff;
		    } else if ( si>=-1131 && si<0 ) {
			si=(-si)-108;
			vals[len++] = (si>>8)+251;
			vals[len++] = si&0xff;
		    } else if ( si>=-32768 && si<=32767 ) {
			vals[len++] = 28;
			vals[len++] = (si>>8)&0xff;
			vals[len++] = si&0xff;
		    } else {
			/* store as fixed point, then multiply by 64. Takes 8 bytes */
			si *= (65536/64);
			vals[len++] = '\377';
			vals[len++] = (si>>24)&0xff;
			vals[len++] = (si>>16)&0xff;
			vals[len++] = (si>>8)&0xff;
			vals[len++] = si&0xff;
			vals[len++] = 64 + 139;
			vals[len++] = 0xc; vals[len++] = 0x18;	/* Multiply */
		    }
    
		    /* space for the subroutine operator */
		    vals[len++] = 10;
		}
		if ( k==1 && gb->sc->lsidebearing!=0x7fff )
	    break;
	    }
	    if ( k==0 ) {
		vals[len++] = 11;	/* return */
		vals[len] = '\0';
	    } else {
		vals[len++] = 14;	/* endchar */
		vals[len] = '\0';
	    }
	}
    }
    
    GIFree(&gi,&dummynotdef);
    *_subrs = subrs;
return( chrs );
}

struct pschars *CID2ChrsSubrs2(SplineFont *cidmaster,struct fd2data *fds,
	int flags, struct pschars **_glbls, int layer) {
    struct pschars *chrs, *glbls;
    int i, j, cnt, cid, max, fd;
    int *scnts;
    SplineChar *sc;
    SplineFont *sf = NULL;
    /* In a cid-keyed font, cid 0 is defined to be .notdef so there are no */
    /*  special worries. If it is defined we use it. If it is not defined */
    /*  we add it. */
    GlyphInfo gi;
    SplineChar dummynotdef;

    max = 0;
    for ( i=0; i<cidmaster->subfontcnt; ++i ) {
	if ( max<cidmaster->subfonts[i]->glyphcnt )
	    max = cidmaster->subfonts[i]->glyphcnt;
	MarkTranslationRefs(cidmaster->subfonts[i],layer);
    }
    cnt = 1;			/* for .notdef */
    for ( cid = 1; cid<max; ++cid ) {
	for ( i=0; i<cidmaster->subfontcnt; ++i ) {
	    sf = cidmaster->subfonts[i];
	    if ( cid<sf->glyphcnt && (sc=sf->glyphs[cid])!=NULL ) {
		sc->ttf_glyph = -1;
		sc->lsidebearing = 0x7fff;
		if ( SCWorthOutputting(sc))
		    ++cnt;
	break;
	    }
	}
    }

    memset(&gi,0,sizeof(gi));
    memset(&gi.hashed,-1,sizeof(gi.hashed));
    gi.instance_count = 1;
    gi.sf = sf;
    gi.glyphcnt = cnt;
    gi.bygid = NULL;
    gi.gb = gcalloc(cnt,sizeof(struct glyphbits));
    gi.pmax = 3*cnt;
    gi.psubrs = galloc(gi.pmax*sizeof(struct potentialsubrs));
    gi.layer = layer;

    for ( cid = cnt = 0; cid<max; ++cid ) {
	sf = NULL;
	for ( i=0; i<cidmaster->subfontcnt; ++i ) {
	    sf = cidmaster->subfonts[i];
	    if ( cid<sf->glyphcnt && SCWorthOutputting(sf->glyphs[cid]) )
	break;
	}
	if ( cid!=0 && i==cidmaster->subfontcnt ) {
	    sc=NULL;
	} else if ( i==cidmaster->subfontcnt ) {
	    /* They didn't define CID 0 */
	    sc = &dummynotdef;
	    /* Place it in the final subfont (which is what sf points to) */
	    memset(sc,0,sizeof(dummynotdef));
	    dummynotdef.name = ".notdef";
	    dummynotdef.parent = sf;
	    dummynotdef.layer_cnt = layer+1;
	    dummynotdef.layers = gcalloc(layer+1,sizeof(Layer));
	    dummynotdef.width = SFOneWidth(sf);
	    if ( dummynotdef.width==-1 )
		dummynotdef.width = (sf->ascent+sf->descent);
	    Type2NotDefSplines(sf,&dummynotdef,layer);
	    gi.gb[cnt].sc = sc;
	    gi.gb[cnt].fd = i = cidmaster->subfontcnt-1;
#if 0 && HANYANG			/* Too much stuff knows the glyph cnt, can't refigure it here at the end */
	} else if ( sf->glyphs[cid]->compositionunit ) {
	    sc=NULL;	/* don't output it, should be in a subroutine */;
#endif
	} else {
	    gi.gb[cnt].sc = sc = sf->glyphs[cid];
	    gi.gb[cnt].fd = i;
	}
	if ( sc!=NULL ) {
	    sc->lsidebearing = 0x7fff;
	    gi.active = &gi.gb[cnt];
	    sc->ttf_glyph = cnt++;
	    SplineChar2PS2(sc,NULL,fds[i].nomwid,fds[i].defwid,NULL,flags,&gi);
	}
	ff_progress_next();
    }

    scnts = gcalloc( cidmaster->subfontcnt+1,sizeof(int));
    for ( i=0; i<gi.pcnt; ++i ) {
	gi.psubrs[i].idx = -1;
	if ( gi.psubrs[i].cnt*gi.psubrs[i].len>(gi.psubrs[i].cnt*4)+gi.psubrs[i].len+1 )
	    gi.psubrs[i].idx = scnts[gi.psubrs[i].fd+1]++;
    }

    glbls = gcalloc(1,sizeof(struct pschars));
    glbls->cnt = scnts[0];
    glbls->next = scnts[0];
    glbls->lens = galloc(scnts[0]*sizeof(int));
    glbls->values = galloc(scnts[0]*sizeof(unsigned char *));
    glbls->bias = scnts[0]<1240 ? 107 :
		  scnts[0]<33900 ? 1131 : 32768;
    for ( fd=0; fd<cidmaster->subfontcnt; ++fd ) {
	fds[fd].subrs = gcalloc(1,sizeof(struct pschars));
	fds[fd].subrs->cnt = scnts[fd+1];
	fds[fd].subrs->next = scnts[fd+1];
	fds[fd].subrs->lens = galloc(scnts[fd+1]*sizeof(int));
	fds[fd].subrs->values = galloc(scnts[fd+1]*sizeof(unsigned char *));
	fds[fd].subrs->bias = scnts[fd+1]<1240 ? 107 :
			      scnts[fd+1]<33900 ? 1131 : 32768;
    }
    free( scnts);

    for ( i=0; i<gi.pcnt; ++i ) {
	if ( gi.psubrs[i].idx != -1 ) {
	    struct pschars *subrs = gi.psubrs[i].fd==-1 ? glbls : fds[gi.psubrs[i].fd].subrs;
	    int scnt = gi.psubrs[i].idx;
	    subrs->lens[scnt] = gi.psubrs[i].len+1;
	    subrs->values[scnt] = galloc(subrs->lens[scnt]);
	    memcpy(subrs->values[scnt],gi.psubrs[i].data,gi.psubrs[i].len);
	    subrs->values[scnt][gi.psubrs[i].len] = 11;	/* Add a return to end of subr */
	}
    }


    chrs = gcalloc(1,sizeof(struct pschars));
    chrs->cnt = cnt;
    chrs->next = cnt;
    chrs->lens = galloc(cnt*sizeof(int));
    chrs->values = galloc(cnt*sizeof(unsigned char *));
    chrs->keys = galloc(cnt*sizeof(char *));
    for ( i=0; i<cnt; ++i ) {
	int len=0;
	struct glyphbits *gb = &gi.gb[i];
	chrs->keys[i] = copy(gb->sc->name);
	for ( j=0; j<gb->bcnt; ++j ) {
	    len += gb->bits[j].dlen;
	    if ( gi.psubrs[ gb->bits[j].psub_index ].idx==-1 )
		len += gi.psubrs[ gb->bits[j].psub_index ].len;
	    else {
		struct pschars *subrs = gi.psubrs[gb->bits[j].psub_index].fd==-1 ? glbls : fds[gi.psubrs[gb->bits[j].psub_index].fd].subrs;
		int si = gi.psubrs[ gb->bits[j].psub_index ].idx - subrs->bias;
		/* space for the number (subroutine index) */
		if ( si>=-107 && si<=107 )
		    ++len;
		else if ( si>=-1131 && si<=1131 )
		    len += 2;
		else if ( si>=-32768 && si<=32767 )
		    len += 3;
		else
		    len += 8;
		/* space for the subroutine operator */
		++len;
	    }
	}
	chrs->lens[i] = len+1;
	chrs->values[i] = galloc(len+2); /* space for endchar and a final NUL (which is really meaningless, but makes me feel better) */

	len = 0;
	for ( j=0; j<gb->bcnt; ++j ) {
	    memcpy(chrs->values[i]+len,gb->bits[j].data,gb->bits[j].dlen);
	    len += gb->bits[j].dlen;
	    if ( gi.psubrs[ gb->bits[j].psub_index ].idx==-1 ) {
		memcpy(chrs->values[i]+len,gi.psubrs[ gb->bits[j].psub_index ].data,
			gi.psubrs[ gb->bits[j].psub_index ].len);
		len += gi.psubrs[ gb->bits[j].psub_index ].len;
	    } else {
		struct pschars *subrs = gi.psubrs[gb->bits[j].psub_index].fd==-1 ? glbls : fds[gi.psubrs[gb->bits[j].psub_index].fd].subrs;
		int si = gi.psubrs[ gb->bits[j].psub_index ].idx - subrs->bias;
		/* space for the number (subroutine index) */
		if ( si>=-107 && si<=107 )
		    chrs->values[i][len++] = si+139;
		else if ( si>0 && si<=1131 ) {
		    si-=108;
		    chrs->values[i][len++] = (si>>8)+247;
		    chrs->values[i][len++] = si&0xff;
		} else if ( si>=-1131 && si<0 ) {
		    si=(-si)-108;
		    chrs->values[i][len++] = (si>>8)+251;
		    chrs->values[i][len++] = si&0xff;
		} else if ( si>=-32768 && si<=32767 ) {
		    chrs->values[i][len++] = 28;
		    chrs->values[i][len++] = (si>>8)&0xff;
		    chrs->values[i][len++] = si&0xff;
		} else {
		    /* store as fixed point, then multiply by 64. Takes 8 bytes */
		    si *= (65536/64);
		    chrs->values[i][len++] = '\377';
		    chrs->values[i][len++] = (si>>24)&0xff;
		    chrs->values[i][len++] = (si>>16)&0xff;
		    chrs->values[i][len++] = (si>>8)&0xff;
		    chrs->values[i][len++] = si&0xff;
		    chrs->values[i][len++] = 64 + 139;
		    chrs->values[i][len++] = 0xc; chrs->values[i][len++] = 0x18;	/* Multiply */
		}
		/* space for the subroutine operator */
		if ( gi.psubrs[ gb->bits[j].psub_index ].fd==-1 ) {
		    chrs->values[i][len++] = 29;
		} else
		    chrs->values[i][len++] = 10;
	    }
	}
	chrs->values[i][len++] = 14;	/* endchar */
	chrs->values[i][len] = '\0';
    }
    GIFree(&gi,&dummynotdef);
    *_glbls = glbls;
return( chrs );
}

