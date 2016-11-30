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
#include "fontforgevw.h"
#include "encoding.h"
#include <math.h>

#ifdef HAVE_IEEEFP_H
# include <ieeefp.h>		/* Solaris defines isnan in ieeefp rather than math.h */
#endif

#include "sfd1.h" // This has the extended SplineFont type SplineFont1 for old file versions.
#ifdef FF_UTHASH_GLIF_NAMES
# include "glif_name_hash.h"
#endif


/*#define DEBUG 1*/

typedef struct quartic {
    bigreal a,b,c,d,e;
} Quartic;

/* In an attempt to make allocation more efficient I just keep preallocated */
/*  lists of certain common sizes. It doesn't seem to make much difference */
/*  when allocating stuff, but does when freeing. If the extra complexity */
/*  is bad then put:							  */
/*	#define chunkalloc(size)	calloc(1,size)			  */
/*	#define chunkfree(item,size)	free(item)			  */
/*  into splinefont.h after (or instead of) the definition of chunkalloc()*/

#define ALLOC_CHUNK	100		/* Number of small chunks to malloc at a time */
#ifndef FONTFORGE_CONFIG_USE_DOUBLE
# define CHUNK_MAX	100		/* Maximum size (in chunk units) that we are prepared to allocate */
					/* The size of our data structures */
#else
# define CHUNK_MAX	129
#endif
# define CHUNK_UNIT	sizeof(void *)	/*  will vary with the word size of */
					/*  the machine. if pointers are 64 bits*/
					/*  we may need twice as much space as for 32 bits */

#ifdef FLAG
#undef FLAG
#define FLAG 0xbadcafe
#endif

#ifdef CHUNKDEBUG
static int chunkdebug = 0;	/* When this is set we never free anything, insuring that each chunk is unique */
#endif

#if ALLOC_CHUNK>1
struct chunk { struct chunk *next; };
struct chunk2 { struct chunk2 *next; int flag; };
#endif

#if defined(FLAG) && ALLOC_CHUNK>1
void chunktest(void) {
    int i;
    struct chunk2 *c;

    for ( i=2; i<CHUNK_MAX; ++i )
	for ( c=(struct chunk2 *) chunklists[i]; c!=NULL; c=c->next )
	    if ( c->flag!=FLAG ) {
		fprintf( stderr, "Chunk memory list has been corrupted\n" );
		abort();
	    }
}
#endif

void LineListFree(LineList *ll) {
    LineList *next;

    while ( ll!=NULL ) {
	next = ll->next;
	chunkfree(ll,sizeof(LineList));
	ll = next;
    }
}

void LinearApproxFree(LinearApprox *la) {
    LinearApprox *next;

    while ( la!=NULL ) {
	next = la->next;
	LineListFree(la->lines);
	chunkfree(la,sizeof(LinearApprox));
	la = next;
    }
}

void SplineFree(Spline *spline) {
    LinearApproxFree(spline->approx);
    chunkfree(spline,sizeof(Spline));
}

SplinePoint *SplinePointCreate(real x, real y) {
    SplinePoint *sp;
    if ( (sp=chunkalloc(sizeof(SplinePoint)))!=NULL ) {
	sp->me.x = x; sp->me.y = y;
	sp->nextcp = sp->prevcp = sp->me;
	sp->nonextcp = sp->noprevcp = true;
	sp->nextcpdef = sp->prevcpdef = false;
	sp->ttfindex = sp->nextcpindex = 0xfffe;
	sp->name = NULL;
    }
    return( sp );
}

Spline *SplineMake3(SplinePoint *from, SplinePoint *to) {
    Spline *spline = chunkalloc(sizeof(Spline));

    spline->from = from; spline->to = to;
    from->next = to->prev = spline;
    SplineRefigure3(spline);
return( spline );
}

void SplinePointFree(SplinePoint *sp) {
    chunkfree(sp->hintmask,sizeof(HintMask));
	free(sp->name);
    chunkfree(sp,sizeof(SplinePoint));
}

void SplinePointsFree(SplinePointList *spl) {
    Spline *first, *spline, *next;
    int nonext;

    if ( spl==NULL )
      return;
    if ( spl->first!=NULL ) {
	nonext = spl->first->next==NULL; // If there is no spline, we set a flag.
	first = NULL;
        // We start on the first spline if it exists.
	for ( spline = spl->first->next; spline!=NULL && spline!=first; spline = next ) {
	    next = spline->to->next; // Cache the location of the next spline.
	    SplinePointFree(spline->to); // Free the destination point.
	    SplineFree(spline); // Free the spline.
	    if ( first==NULL ) first = spline; // We want to avoid repeating the circuit.
	}
        // If the path is open or has no splines, free the starting point.
	if ( spl->last!=spl->first || nonext )
	    SplinePointFree(spl->first);
    }
}

void SplinePointListFree(SplinePointList *spl) {

    if ( spl==NULL ) return;
    SplinePointsFree(spl);
    free(spl->spiros);
    free(spl->contour_name);
    chunkfree(spl,sizeof(SplinePointList));
}

void SplinePointListsFree(SplinePointList *spl) {
    SplinePointList *next;

    while ( spl!=NULL ) {
	next = spl->next;
	SplinePointListFree(spl);
	spl = next;
    }
}

void SplineSetSpirosClear(SplineSet *spl) {
    free(spl->spiros);
    spl->spiros = NULL;
    spl->spiro_cnt = spl->spiro_max = 0;
}


void RefCharFree(RefChar *ref) {
    int i;

    if ( ref==NULL )
return;
    for ( i=0; i<ref->layer_cnt; ++i ) {
	SplinePointListsFree(ref->layers[i].splines);
	GradientFree(ref->layers[i].fill_brush.gradient);
	GradientFree(ref->layers[i].stroke_pen.brush.gradient);
	PatternFree(ref->layers[i].fill_brush.pattern);
	PatternFree(ref->layers[i].stroke_pen.brush.pattern);
    }
    free(ref->layers);
    chunkfree(ref,sizeof(RefChar));
}

RefChar *RefCharCreate(void) {
    RefChar *ref = chunkalloc(sizeof(RefChar));
    ref->layer_cnt = 1;
    ref->layers = calloc(1,sizeof(struct reflayer));
    ref->layers[0].fill_brush.opacity = ref->layers[0].stroke_pen.brush.opacity = 1.0;
    ref->layers[0].fill_brush.col = ref->layers[0].stroke_pen.brush.col = COLOR_INHERITED;
    ref->layers[0].stroke_pen.width = WIDTH_INHERITED;
    ref->layers[0].stroke_pen.linecap = lc_inherited;
    ref->layers[0].stroke_pen.linejoin = lj_inherited;
    ref->layers[0].dofill = true;
    ref->round_translation_to_grid = true;
return( ref );
}

void RefCharsFree(RefChar *ref) {
    RefChar *rnext;

    while ( ref!=NULL ) {
	rnext = ref->next;
	RefCharFree(ref);
	ref = rnext;
    }
}

typedef struct spline1 {
    Spline1D sp;
    real s0, s1;
    real c0, c1;
} Spline1;

static void FigureSpline1(Spline1 *sp1,bigreal t0, bigreal t1, Spline1D *sp ) {
    bigreal s = (t1-t0);
    if ( sp->a==0 && sp->b==0 ) {
	sp1->sp.d = sp->d + t0*sp->c;
	sp1->sp.c = s*sp->c;
	sp1->sp.b = sp1->sp.a = 0;
    } else {
	sp1->sp.d = sp->d + t0*(sp->c + t0*(sp->b + t0*sp->a));
	sp1->sp.c = s*(sp->c + t0*(2*sp->b + 3*sp->a*t0));
	sp1->sp.b = s*s*(sp->b+3*sp->a*t0);
	sp1->sp.a = s*s*s*sp->a;
    }
    sp1->c0 = sp1->sp.c/3 + sp1->sp.d;
    sp1->c1 = sp1->c0 + (sp1->sp.b+sp1->sp.c)/3;
}

static void SplineFindBounds(const Spline *sp, DBounds *bounds) {
    real t, b2_fourac, v;
    real min, max;
    const Spline1D *sp1;
    int i;

    /* first try the end points */
    for ( i=0; i<2; ++i ) {
	sp1 = &sp->splines[i];
	if ( i==0 ) {
	    if ( sp->to->me.x<bounds->minx ) bounds->minx = sp->to->me.x;
	    if ( sp->to->me.x>bounds->maxx ) bounds->maxx = sp->to->me.x;
	    min = bounds->minx; max = bounds->maxx;
	} else {
	    if ( sp->to->me.y<bounds->miny ) bounds->miny = sp->to->me.y;
	    if ( sp->to->me.y>bounds->maxy ) bounds->maxy = sp->to->me.y;
	    min = bounds->miny; max = bounds->maxy;
	}

	/* then try the extrema of the spline (assuming they are between t=(0,1) */
	/* (I don't bother fixing up for tiny rounding errors here. they don't matter */
	/* But we could call CheckExtremaForSingleBitErrors */
	if ( sp1->a!=0 ) {
	    b2_fourac = 4*sp1->b*sp1->b - 12*sp1->a*sp1->c;
	    if ( b2_fourac>=0 ) {
		b2_fourac = sqrt(b2_fourac);
		t = (-2*sp1->b + b2_fourac) / (6*sp1->a);
		if ( t>0 && t<1 ) {
		    v = ((sp1->a*t+sp1->b)*t+sp1->c)*t + sp1->d;
		    if ( v<min ) min = v;
		    if ( v>max ) max = v;
		}
		t = (-2*sp1->b - b2_fourac) / (6*sp1->a);
		if ( t>0 && t<1 ) {
		    v = ((sp1->a*t+sp1->b)*t+sp1->c)*t + sp1->d;
		    if ( v<min ) min = v;
		    if ( v>max ) max = v;
		}
	    }
	} else if ( sp1->b!=0 ) {
	    t = -sp1->c/(2.0*sp1->b);
	    if ( t>0 && t<1 ) {
		v = (sp1->b*t+sp1->c)*t + sp1->d;
		if ( v<min ) min = v;
		if ( v>max ) max = v;
	    }
	}
	if ( i==0 ) {
	    bounds->minx = min; bounds->maxx = max;
	} else {
	    bounds->miny = min; bounds->maxy = max;
	}
    }
}

static void _SplineSetFindBounds(const SplinePointList *spl, DBounds *bounds) {
    Spline *spline, *first;
    /* Ignore contours consisting of a single point (used for hinting, anchors */
    /*  for mark to base, etc. */

    for ( ; spl!=NULL; spl = spl->next ) if ( spl->first->next!=NULL && spl->first->next->to != spl->first ) {
	first = NULL;
	if ( bounds->minx==0 && bounds->maxx==0 && bounds->miny==0 && bounds->maxy == 0 ) {
	    bounds->minx = bounds->maxx = spl->first->me.x;
	    bounds->miny = bounds->maxy = spl->first->me.y;
	} else {
	    if ( spl->first->me.x<bounds->minx ) bounds->minx = spl->first->me.x;
	    if ( spl->first->me.x>bounds->maxx ) bounds->maxx = spl->first->me.x;
	    if ( spl->first->me.y<bounds->miny ) bounds->miny = spl->first->me.y;
	    if ( spl->first->me.y>bounds->maxy ) bounds->maxy = spl->first->me.y;
	}
	for ( spline = spl->first->next; spline!=NULL && spline!=first; spline=spline->to->next ) {
	    SplineFindBounds(spline,bounds);
	    if ( first==NULL ) first = spline;
	}
    }
}

static void _SplineSetFindClippedBounds(const SplinePointList *spl, DBounds *bounds,DBounds *clipb) {
    Spline *spline, *first;
    /* Ignore contours consisting of a single point (used for hinting, anchors */
    /*  for mark to base, etc. */

    for ( ; spl!=NULL; spl = spl->next ) if ( spl->first->next!=NULL && spl->first->next->to != spl->first ) {
	first = NULL;
	if ( !spl->is_clip_path ) {
	    if ( bounds->minx==0 && bounds->maxx==0 && bounds->miny==0 && bounds->maxy == 0 ) {
		bounds->minx = bounds->maxx = spl->first->me.x;
		bounds->miny = bounds->maxy = spl->first->me.y;
	    } else {
		if ( spl->first->me.x<bounds->minx ) bounds->minx = spl->first->me.x;
		if ( spl->first->me.x>bounds->maxx ) bounds->maxx = spl->first->me.x;
		if ( spl->first->me.y<bounds->miny ) bounds->miny = spl->first->me.y;
		if ( spl->first->me.y>bounds->maxy ) bounds->maxy = spl->first->me.y;
	    }
	    for ( spline = spl->first->next; spline!=NULL && spline!=first; spline=spline->to->next ) {
		SplineFindBounds(spline,bounds);
		if ( first==NULL ) first = spline;
	    }
	} else {
	    if ( clipb->minx==0 && clipb->maxx==0 && clipb->miny==0 && clipb->maxy == 0 ) {
		clipb->minx = clipb->maxx = spl->first->me.x;
		clipb->miny = clipb->maxy = spl->first->me.y;
	    } else {
		if ( spl->first->me.x<clipb->minx ) clipb->minx = spl->first->me.x;
		if ( spl->first->me.x>clipb->maxx ) clipb->maxx = spl->first->me.x;
		if ( spl->first->me.y<clipb->miny ) clipb->miny = spl->first->me.y;
		if ( spl->first->me.y>clipb->maxy ) clipb->maxy = spl->first->me.y;
	    }
	    for ( spline = spl->first->next; spline!=NULL && spline!=first; spline=spline->to->next ) {
		SplineFindBounds(spline,clipb);
		if ( first==NULL ) first = spline;
	    }
	}
    }
}

static void _SplineCharLayerFindBounds(SplineChar *sc,int layer, DBounds *bounds) {
    RefChar *rf;
    real e;
    DBounds b, clipb;

    for ( rf=sc->layers[layer].refs; rf!=NULL; rf = rf->next ) {
	if ( bounds->minx==0 && bounds->maxx==0 && bounds->miny==0 && bounds->maxy == 0 )
	    *bounds = rf->bb;
	else if ( rf->bb.minx!=0 || rf->bb.maxx != 0 || rf->bb.maxy != 0 || rf->bb.miny!=0 ) {
	    if ( rf->bb.minx < bounds->minx ) bounds->minx = rf->bb.minx;
	    if ( rf->bb.miny < bounds->miny ) bounds->miny = rf->bb.miny;
	    if ( rf->bb.maxx > bounds->maxx ) bounds->maxx = rf->bb.maxx;
	    if ( rf->bb.maxy > bounds->maxy ) bounds->maxy = rf->bb.maxy;
	}
    }
    memset(&b,0,sizeof(b));
    memset(&clipb,0,sizeof(clipb));
    _SplineSetFindClippedBounds(sc->layers[layer].splines,&b,&clipb);
    if ( sc->layers[layer].dostroke ) {
	if ( sc->layers[layer].stroke_pen.width!=WIDTH_INHERITED )
	    e = sc->layers[layer].stroke_pen.width*sc->layers[layer].stroke_pen.trans[0];
	else
	    e = sc->layers[layer].stroke_pen.trans[0];
	b.minx -= e; b.maxx += e;
	b.miny -= e; b.maxy += e;
    }
    if ( clipb.minx!=0 || clipb.miny!=0 || clipb.maxx!=0 || clipb.maxy!=0 ) {
	if ( b.minx<clipb.minx ) b.minx = clipb.minx;
	if ( b.miny<clipb.miny ) b.miny = clipb.miny;
	if ( b.maxx>clipb.maxx ) b.maxx = clipb.maxx;
	if ( b.maxy>clipb.maxy ) b.maxy = clipb.maxy;
    }
    if ( bounds->minx==0 && bounds->maxx==0 && bounds->miny==0 && bounds->maxy == 0 )
	*bounds = b;
    else if ( b.minx!=0 || b.maxx != 0 || b.maxy != 0 || b.miny!=0 ) {
	if ( b.minx < bounds->minx ) bounds->minx = b.minx;
	if ( b.miny < bounds->miny ) bounds->miny = b.miny;
	if ( b.maxx > bounds->maxx ) bounds->maxx = b.maxx;
	if ( b.maxy > bounds->maxy ) bounds->maxy = b.maxy;
    }

    if ( sc->parent!=NULL && sc->parent->strokedfont &&
	    (bounds->minx!=bounds->maxx || bounds->miny!=bounds->maxy)) {
	real sw = sc->parent->strokewidth;
	bounds->minx -= sw; bounds->miny -= sw;
	bounds->maxx += sw; bounds->maxy += sw;
    }
}

void SplineCharLayerFindBounds(SplineChar *sc,int layer,DBounds *bounds) {

    if ( sc->parent!=NULL && sc->parent->multilayer ) {
	SplineCharFindBounds(sc,bounds);
return;
    }

    /* a char with no splines (ie. a space) must have an lbearing of 0 */
    bounds->minx = bounds->maxx = 0;
    bounds->miny = bounds->maxy = 0;

    _SplineCharLayerFindBounds(sc,layer,bounds);
}

void SplineCharFindBounds(SplineChar *sc,DBounds *bounds) {
    int i;
    int first,last;

    /* a char with no splines (ie. a space) must have an lbearing of 0 */
    bounds->minx = bounds->maxx = 0;
    bounds->miny = bounds->maxy = 0;

    first = last = ly_fore;
    if ( sc->parent!=NULL && sc->parent->multilayer )
	last = sc->layer_cnt-1;
    for ( i=first; i<=last; ++i )
	_SplineCharLayerFindBounds(sc,i,bounds);
}

void SplineFontLayerFindBounds(SplineFont *sf,int layer,DBounds *bounds) {
    int i, k, first, last;

    if ( sf->multilayer ) {
	SplineFontFindBounds(sf,bounds);
return;
    }

    bounds->minx = bounds->maxx = 0;
    bounds->miny = bounds->maxy = 0;

    for ( i = 0; i<sf->glyphcnt; ++i ) {
	SplineChar *sc = sf->glyphs[i];
	if ( sc!=NULL ) {
	    first = last = ly_fore;
	    if ( sc->parent != NULL && sc->parent->multilayer )
		last = sc->layer_cnt-1;
	    for ( k=first; k<=last; ++k )
		_SplineCharLayerFindBounds(sc,k,bounds);
	}
    }
}

void SplineFontFindBounds(SplineFont *sf,DBounds *bounds) {
    int i, k, first, last;

    bounds->minx = bounds->maxx = 0;
    bounds->miny = bounds->maxy = 0;

    for ( i = 0; i<sf->glyphcnt; ++i ) {
	SplineChar *sc = sf->glyphs[i];
	if ( sc!=NULL ) {
	    first = last = ly_fore;
	    if ( sf->multilayer )
		last = sc->layer_cnt-1;
	    for ( k=first; k<=last; ++k )
		_SplineCharLayerFindBounds(sc,k,bounds);
	}
    }
}

void CIDLayerFindBounds(SplineFont *cidmaster,int layer,DBounds *bounds) {
    SplineFont *sf;
    int i;
    DBounds b;
    real factor;

    if ( cidmaster->cidmaster )
	cidmaster = cidmaster->cidmaster;
    if ( cidmaster->subfonts==NULL ) {
	SplineFontLayerFindBounds(cidmaster,layer,bounds);
return;
    }

    sf = cidmaster->subfonts[0];
    SplineFontLayerFindBounds(sf,layer,bounds);
    factor = 1000.0/(sf->ascent+sf->descent);
    bounds->maxx *= factor; bounds->minx *= factor; bounds->miny *= factor; bounds->maxy *= factor;
    for ( i=1; i<cidmaster->subfontcnt; ++i ) {
	sf = cidmaster->subfonts[i];
	SplineFontLayerFindBounds(sf,layer,&b);
	factor = 1000.0/(sf->ascent+sf->descent);
	b.maxx *= factor; b.minx *= factor; b.miny *= factor; b.maxy *= factor;
	if ( b.maxx>bounds->maxx ) bounds->maxx = b.maxx;
	if ( b.maxy>bounds->maxy ) bounds->maxy = b.maxy;
	if ( b.miny<bounds->miny ) bounds->miny = b.miny;
	if ( b.minx<bounds->minx ) bounds->minx = b.minx;
    }
}

static void _SplineSetFindTop(SplineSet *ss,BasePoint *top) {
    SplinePoint *sp;

    for ( ; ss!=NULL; ss=ss->next ) {
	for ( sp=ss->first; ; ) {
	    if ( sp->me.y > top->y ) *top = sp->me;
	    if ( sp->next==NULL )
	break;
	    sp = sp->next->to;
	    if ( sp==ss->first )
	break;
	}
    }
}

void SplineSetQuickBounds(SplineSet *ss,DBounds *b) {
    SplinePoint *sp;

    b->minx = b->miny = 1e10;
    b->maxx = b->maxy = -1e10;
    for ( ; ss!=NULL; ss=ss->next ) {
	for ( sp=ss->first; ; ) {
	    if ( sp->me.y < b->miny ) b->miny = sp->me.y;
	    if ( sp->me.x < b->minx ) b->minx = sp->me.x;
	    if ( sp->me.y > b->maxy ) b->maxy = sp->me.y;
	    if ( sp->me.x > b->maxx ) b->maxx = sp->me.x;
	    // Frank added the control points to the calculation since,
	    // according to Adam Twardoch,
	    // the OpenType values that rely upon this function
	    // expect control points to be included.
	    if ( !sp->noprevcp ) {
	      if ( sp->prevcp.y < b->miny ) b->miny = sp->prevcp.y;
	      if ( sp->prevcp.x < b->minx ) b->minx = sp->prevcp.x;
	      if ( sp->prevcp.y > b->maxy ) b->maxy = sp->prevcp.y;
	      if ( sp->prevcp.x > b->maxx ) b->maxx = sp->prevcp.x;
	    }
	    if ( !sp->nonextcp ) {
	      if ( sp->nextcp.y < b->miny ) b->miny = sp->nextcp.y;
	      if ( sp->nextcp.x < b->minx ) b->minx = sp->nextcp.x;
	      if ( sp->nextcp.y > b->maxy ) b->maxy = sp->nextcp.y;
	      if ( sp->nextcp.x > b->maxx ) b->maxx = sp->nextcp.x;
	    }
	    if ( sp->next==NULL )
	break;
	    sp = sp->next->to;
	    if ( sp==ss->first )
	break;
	}
    }
    if ( b->minx>65536 ) b->minx = 0;
    if ( b->miny>65536 ) b->miny = 0;
    if ( b->maxx<-65536 ) b->maxx = 0;
    if ( b->maxy<-65536 ) b->maxy = 0;
}

void SplineCharQuickBounds(SplineChar *sc, DBounds *b) {
    RefChar *ref;
    int i,first, last;
    DBounds temp;
    real e;

    b->minx = b->miny = 1e10;
    b->maxx = b->maxy = -1e10;
    first = last = ly_fore;
    if ( sc->parent!=NULL && sc->parent->multilayer )
	last = sc->layer_cnt-1;
    for ( i=first; i<=last; ++i ) {
	SplineSetQuickBounds(sc->layers[i].splines,&temp);
	if ( sc->layers[i].dostroke && sc->layers[i].splines!=NULL ) {
	    if ( sc->layers[i].stroke_pen.width!=WIDTH_INHERITED )
		e = sc->layers[i].stroke_pen.width*sc->layers[i].stroke_pen.trans[0];
	    else
		e = sc->layers[i].stroke_pen.trans[0];
	    temp.minx -= e; temp.maxx += e;
	    temp.miny -= e; temp.maxy += e;
	}
	if ( temp.minx!=0 || temp.maxx != 0 || temp.maxy != 0 || temp.miny!=0 ) {
	    if ( temp.minx < b->minx ) b->minx = temp.minx;
	    if ( temp.miny < b->miny ) b->miny = temp.miny;
	    if ( temp.maxx > b->maxx ) b->maxx = temp.maxx;
	    if ( temp.maxy > b->maxy ) b->maxy = temp.maxy;
	}
	for ( ref = sc->layers[i].refs; ref!=NULL; ref = ref->next ) {
	    /*SplineSetQuickBounds(ref->layers[0].splines,&temp);*/
	    if ( b->minx==0 && b->maxx==0 && b->miny==0 && b->maxy == 0 )
		*b = ref->bb;
	    else if ( ref->bb.minx!=0 || ref->bb.maxx != 0 || ref->bb.maxy != 0 || ref->bb.miny!=0 ) {
		if ( ref->bb.minx < b->minx ) b->minx = ref->bb.minx;
		if ( ref->bb.miny < b->miny ) b->miny = ref->bb.miny;
		if ( ref->bb.maxx > b->maxx ) b->maxx = ref->bb.maxx;
		if ( ref->bb.maxy > b->maxy ) b->maxy = ref->bb.maxy;
	    }
	}
    }
    if ( sc->parent!=NULL && sc->parent->strokedfont &&
	    (b->minx!=b->maxx || b->miny!=b->maxy)) {
	real sw = sc->parent->strokewidth;
	b->minx -= sw; b->miny -= sw;
	b->maxx += sw; b->maxy += sw;
    }
    if ( b->minx>1e9 )
	memset(b,0,sizeof(*b));
}

void SplineCharLayerQuickBounds(SplineChar *sc,int layer,DBounds *bounds) {
    RefChar *ref;
    DBounds temp;

    if ( sc->parent!=NULL && sc->parent->multilayer ) {
	SplineCharQuickBounds(sc,bounds);
return;
    }

    bounds->minx = bounds->miny = 1e10;
    bounds->maxx = bounds->maxy = -1e10;

    SplineSetQuickBounds(sc->layers[layer].splines,bounds);

    for ( ref = sc->layers[layer].refs; ref!=NULL; ref = ref->next ) {
	SplineSetQuickBounds(ref->layers[0].splines,&temp);
	if ( bounds->minx==0 && bounds->maxx==0 && bounds->miny==0 && bounds->maxy == 0 )
	    *bounds = temp;
	else if ( temp.minx!=0 || temp.maxx != 0 || temp.maxy != 0 || temp.miny!=0 ) {
	    if ( temp.minx < bounds->minx ) bounds->minx = temp.minx;
	    if ( temp.miny < bounds->miny ) bounds->miny = temp.miny;
	    if ( temp.maxx > bounds->maxx ) bounds->maxx = temp.maxx;
	    if ( temp.maxy > bounds->maxy ) bounds->maxy = temp.maxy;
	}
    }
    /* a char with no splines (ie. a space) must have an lbearing of 0 */
    if ( bounds->minx>1e9 )
	memset(bounds,0,sizeof(*bounds));
}

void SplinePointCategorize(SplinePoint *sp) {
    int oldpointtype = sp->pointtype;

    sp->pointtype = pt_corner;
    if ( sp->next==NULL && sp->prev==NULL )
	;
    else if ( (sp->next!=NULL && sp->next->to->me.x==sp->me.x && sp->next->to->me.y==sp->me.y) ||
	    (sp->prev!=NULL && sp->prev->from->me.x==sp->me.x && sp->prev->from->me.y==sp->me.y ))
	;
    else if ( sp->next==NULL ) {
	sp->pointtype = sp->noprevcp ? pt_corner : pt_curve;
    } else if ( sp->prev==NULL ) {
	sp->pointtype = sp->nonextcp ? pt_corner : pt_curve;
    } else if ( sp->nonextcp && sp->noprevcp ) {
	;
    } else {
	BasePoint ndir, ncdir, ncunit, pdir, pcdir, pcunit;
	bigreal nlen, nclen, plen, pclen;
	bigreal cross, bounds;

	ncdir.x = sp->nextcp.x - sp->me.x; ncdir.y = sp->nextcp.y - sp->me.y;
	pcdir.x = sp->prevcp.x - sp->me.x; pcdir.y = sp->prevcp.y - sp->me.y;
	ndir.x = ndir.y = pdir.x = pdir.y = 0;
	if ( sp->next!=NULL ) {
	    ndir.x = sp->next->to->me.x - sp->me.x; ndir.y = sp->next->to->me.y - sp->me.y;
	}
	if ( sp->prev!=NULL ) {
	    pdir.x = sp->prev->from->me.x - sp->me.x; pdir.y = sp->prev->from->me.y - sp->me.y;
	}
	nclen = sqrt(ncdir.x*ncdir.x + ncdir.y*ncdir.y);
	pclen = sqrt(pcdir.x*pcdir.x + pcdir.y*pcdir.y);
	nlen = sqrt(ndir.x*ndir.x + ndir.y*ndir.y);
	plen = sqrt(pdir.x*pdir.x + pdir.y*pdir.y);
	ncunit = ncdir; pcunit = pcdir;
	if ( nclen!=0 ) { ncunit.x /= nclen; ncunit.y /= nclen; }
	if ( pclen!=0 ) { pcunit.x /= pclen; pcunit.y /= pclen; }
	if ( nlen!=0 ) { ndir.x /= nlen; ndir.y /= nlen; }
	if ( plen!=0 ) { pdir.x /= plen; pdir.y /= plen; }

	/* find out which side has the shorter control vector. Cross that vector */
	/*  with the normal of the unit vector on the other side. If the */
	/*  result is less than 1 em-unit then we've got colinear control points */
	/*  (within the resolution of the integer grid) */
	/* Not quite... they could point in the same direction */
        if ( oldpointtype==pt_curve )
            bounds = 4.0;
        else
            bounds = 1.0;
	if ( nclen!=0 && pclen!=0 &&
		((nclen>=pclen && (cross = pcdir.x*ncunit.y - pcdir.y*ncunit.x)<bounds && cross>-bounds ) ||
		 (pclen>nclen && (cross = ncdir.x*pcunit.y - ncdir.y*pcunit.x)<bounds && cross>-bounds )) &&
		 ncdir.x*pcdir.x + ncdir.y*pcdir.y < 0 )
	    sp->pointtype = pt_curve;
	/* Cross product of control point with unit vector normal to line in */
	/*  opposite direction should be less than an em-unit for a tangent */
	else if (( nclen==0 && pclen!=0 && (cross = pcdir.x*ndir.y-pcdir.y*ndir.x)<bounds && cross>-bounds ) ||
		( pclen==0 && nclen!=0 && (cross = ncdir.x*pdir.y-ncdir.y*pdir.x)<bounds && cross>-bounds ))
	    sp->pointtype = pt_tangent;

	/* If a point started out hv, and could still be hv, them make it so */
	/*  but don't make hv points de novo, Alexey doesn't like change */
	/*  (this only works because hv isn't a default setting, so if it's */
	/*   there it was done intentionally) */
	if ( sp->pointtype == pt_curve && oldpointtype == pt_hvcurve &&
		((sp->nextcp.x==sp->me.x && sp->prevcp.x==sp->me.x && sp->nextcp.y!=sp->me.y) ||
		 (sp->nextcp.y==sp->me.y && sp->prevcp.y==sp->me.y && sp->nextcp.x!=sp->me.x)))
	    sp->pointtype = pt_hvcurve;
    }
}

void SPLCategorizePoints(SplinePointList *spl) {
    Spline *spline, *first, *last=NULL;

    for ( ; spl!=NULL; spl = spl->next ) {
	first = NULL;
	for ( spline = spl->first->next; spline!=NULL && spline!=first; spline=spline->to->next ) {
	    SplinePointCategorize(spline->from);
	    last = spline;
	    if ( first==NULL ) first = spline;
	}
	if ( spline==NULL && last!=NULL )
	    SplinePointCategorize(last->to);
    }
}

SplinePointList *SplinePointListCopy1(const SplinePointList *spl) {
    SplinePointList *cur;
    const SplinePoint *pt; SplinePoint *cpt;
    Spline *spline;

    cur = chunkalloc(sizeof(SplinePointList));
    cur->is_clip_path = spl->is_clip_path;
    cur->spiro_cnt = cur->spiro_max = 0;
    cur->spiros = 0;
    if (spl->contour_name != NULL) cur->contour_name = copy(spl->contour_name);
    for ( pt=spl->first; ;  ) {
	cpt = SplinePointCreate( 0, 0 );
	*cpt = *pt;
	if ( pt->hintmask!=NULL ) {
	    cpt->hintmask = chunkalloc(sizeof(HintMask));
	    memcpy(cpt->hintmask,pt->hintmask,sizeof(HintMask));
	}
	if ( pt->name!=NULL ) {
		cpt->name = copy(pt->name);
	}
	cpt->next = cpt->prev = NULL;
	if ( cur->first==NULL ) {
	    cur->first = cur->last = cpt;
	    cur->start_offset = 0;
	} else {
	    spline = chunkalloc(sizeof(Spline));
	    *spline = *pt->prev;
	    spline->from = cur->last;
	    cur->last->next = spline;
	    cpt->prev = spline;
	    spline->to = cpt;
	    spline->approx = NULL;
	    cur->last = cpt;
	}
	if ( pt->next==NULL )
    break;
	pt = pt->next->to;
	if ( pt==spl->first )
    break;
    }
    if ( spl->first->prev!=NULL ) {
	cpt = cur->first;
	spline = chunkalloc(sizeof(Spline));
	*spline = *pt->prev;
	spline->from = cur->last;
	cur->last->next = spline;
	cpt->prev = spline;
	spline->to = cpt;
	spline->approx = NULL;
	cur->last = cpt;
    }
    if ( spl->spiro_cnt!=0 ) {
	cur->spiro_cnt = cur->spiro_max = spl->spiro_cnt;
	cur->spiros = malloc(cur->spiro_cnt*sizeof(spiro_cp));
	memcpy(cur->spiros,spl->spiros,cur->spiro_cnt*sizeof(spiro_cp));
    }
return( cur );
}

SplinePointList *SplinePointListCopy(const SplinePointList *base) {
    SplinePointList *head=NULL, *last=NULL, *cur;

    for ( ; base!=NULL; base = base->next ) {
	cur = SplinePointListCopy1(base);
	if ( head==NULL )
	    head = cur;
	else
	    last->next = cur;
	last = cur;
    }
return( head );
}

void BpTransform(BasePoint *to, BasePoint *from, real transform[6]) {
    BasePoint p;
    p.x = transform[0]*from->x + transform[2]*from->y + transform[4];
    p.y = transform[1]*from->x + transform[3]*from->y + transform[5];
    to->x = rint(1024*p.x)/1024;
    to->y = rint(1024*p.y)/1024;
}

static void TransformPointExtended(SplinePoint *sp, real transform[6], enum transformPointMask tpmask )
{
    /**
     * If we are to transform selected BCP instead of their base splinepoint
     * then lets do that.
     */
    if( tpmask & tpmask_operateOnSelectedBCP
	&& (sp->nextcpselected || sp->prevcpselected ))
    {
	if( sp->nextcpselected )
	{
	    int order2 = sp->next ? sp->next->order2 : 0;
	    BpTransform(&sp->nextcp,&sp->nextcp,transform);
	    SPTouchControl( sp, &sp->nextcp, order2 );
	}
	else if( sp->prevcpselected )
	{
	    int order2 = sp->next ? sp->next->order2 : 0;
	    BpTransform(&sp->prevcp,&sp->prevcp,transform);
	    SPTouchControl( sp, &sp->prevcp, order2 );
	}
    }
    else
    {
	/**
	 * Transform the base splinepoints.
	 */
	BpTransform(&sp->me,&sp->me,transform);

	if ( !sp->nonextcp )
	{
	    BpTransform(&sp->nextcp,&sp->nextcp,transform);
	}
	else
	{
	    sp->nextcp = sp->me;
	}

	if ( !sp->noprevcp )
	{
	    BpTransform(&sp->prevcp,&sp->prevcp,transform);
	}
	else
	{
	    sp->prevcp = sp->me;
	}
    }



    if ( sp->pointtype == pt_hvcurve )
    {
	if(
	    ((sp->nextcp.x==sp->me.x && sp->prevcp.x==sp->me.x && sp->nextcp.y!=sp->me.y) ||
	     (sp->nextcp.y==sp->me.y && sp->prevcp.y==sp->me.y && sp->nextcp.x!=sp->me.x)))
	{
	    /* Do Nothing */;
	}
	else
	{
	    sp->pointtype = pt_curve;
	}
    }
}

static void TransformPoint(SplinePoint *sp, real transform[6])
{
    TransformPointExtended( sp, transform, 0 );
}

static void TransformSpiro(spiro_cp *cp, real transform[6]) {
    bigreal x;

    x = transform[0]*cp->x + transform[2]*cp->y + transform[4];
    cp->y = transform[1]*cp->x + transform[3]*cp->y + transform[5];
    cp->x = x;
}

static void TransformPTsInterpolateCPs(BasePoint *fromorig,Spline *spline,
	BasePoint *toorig,real transform[6] ) {
    BasePoint totrans, temp;
    bigreal fraction;

    /* Normally the "from" point will already have been translated, and the "to" */
    /*  point will need to be. But if we have a closed contour then on the */
    /*  last spline both from and to will have been transform. We can detect */
    /*  this because toorig will be different from &spline->to->me */
    if ( spline->to->selected && toorig==&spline->to->me )
	BpTransform(&totrans,&spline->to->me,transform);
    else
	totrans = spline->to->me;

    /* None of the control points will have been transformed yet */
    if ( fromorig->x!=toorig->x ) {
	fraction = (spline->from->nextcp.x-fromorig->x)/( toorig->x-fromorig->x );
	spline->from->nextcp.x = spline->from->me.x + fraction*( totrans.x-spline->from->me.x );
	fraction = (spline->to->prevcp.x-fromorig->x)/( toorig->x-fromorig->x );
	spline->to->prevcp.x = spline->from->me.x + fraction*( totrans.x-spline->from->me.x );
    } else {
	BpTransform(&temp,&spline->from->nextcp,transform);
	spline->from->nextcp.x = temp.x;
	BpTransform(&temp,&spline->to->prevcp,transform);
	spline->to->prevcp.x = temp.x;
    }
    if ( fromorig->y!=toorig->y ) {
	fraction = (spline->from->nextcp.y-fromorig->y)/( toorig->y-fromorig->y );
	spline->from->nextcp.y = spline->from->me.y + fraction*( totrans.y-spline->from->me.y );
	fraction = (spline->to->prevcp.y-fromorig->y)/( toorig->y-fromorig->y );
	spline->to->prevcp.y = spline->from->me.y + fraction*( totrans.y-spline->from->me.y );
    } else {
	BpTransform(&temp,&spline->from->nextcp,transform);
	spline->from->nextcp.y = temp.y;
	BpTransform(&temp,&spline->to->prevcp,transform);
	spline->to->prevcp.y = temp.y;
    }

    if ( spline->to->selected )
	spline->to->me = totrans;
}


SplinePointList *SplinePointListTransformExtended(SplinePointList *base, real transform[6],
						  enum transformPointType tpt, enum transformPointMask tpmask ) {
    Spline *spline, *first;
    SplinePointList *spl;
    SplinePoint *spt, *pfirst;
    int allsel, anysel, alldone=true;
    BasePoint lastpointorig, firstpointorig, orig;

    for ( spl = base; spl!=NULL; spl = spl->next ) {
	pfirst = NULL; first = NULL;
	allsel = true; anysel=false;
	if ( tpt==tpt_OnlySelectedInterpCPs && spl->first->next!=NULL && !spl->first->next->order2 ) {
	    lastpointorig = firstpointorig = spl->first->me;
	    printf("SplinePointListTransformExtended() spl->first->selected %d\n", spl->first->selected );
	    if ( spl->first->selected ) {
		anysel = true;
		BpTransform(&spl->first->me,&spl->first->me,transform);
	    } else
		allsel = false;
	    for ( spline = spl->first->next; spline!=NULL && spline!=first; spline=spline->to->next ) {
		if ( first==NULL ) first = spline;
		orig = spline->to->me;
		if ( spline->from->selected || spline->to->selected )
		{
		    TransformPTsInterpolateCPs( &lastpointorig, spline,
						spl->first==spline->to? &firstpointorig : &spline->to->me,
						transform );
		}
		lastpointorig = orig;
		if ( spline->to->selected ) anysel = true; else allsel = false;
	    }

	} else {
	    for ( spt = spl->first ; spt!=pfirst; spt = spt->next->to ) {
		if ( pfirst==NULL ) pfirst = spt;
		if ( tpt==tpt_AllPoints || spt->selected ) {
		    TransformPointExtended(spt,transform,tpmask);
		    if ( tpt!=tpt_AllPoints ) {
			if ( spt->next!=NULL && spt->next->order2 && !spt->next->to->selected && spt->next->to->ttfindex==0xffff ) {
			    SplinePoint *to = spt->next->to;
			    to->prevcp = spt->nextcp;
			    to->me.x = (to->prevcp.x+to->nextcp.x)/2;
			    to->me.y = (to->prevcp.y+to->nextcp.y)/2;
			}
			if ( spt->prev!=NULL && spt->prev->order2 && !spt->prev->from->selected && spt->prev->from->ttfindex==0xffff ) {
			    SplinePoint *from = spt->prev->from;
			    from->nextcp = spt->prevcp;
			    from->me.x = (from->prevcp.x+from->nextcp.x)/2;
			    from->me.y = (from->prevcp.y+from->nextcp.y)/2;
			}
		    }
		    anysel = true;
		} else
		    allsel = alldone = false;
		if ( spt->next==NULL )
	    break;
	    }
	}
	if ( !anysel )		/* This splineset had no selected points it's unchanged */
    continue;

	/* If we changed all the points, then transform the spiro version too */
	/*  otherwise if we just changed some points, throw away the spiro */
	if ( allsel ) {
	    int i;
	    for ( i=0; i<spl->spiro_cnt-1; ++i )
		TransformSpiro(&spl->spiros[i], transform);
	} else
	    SplineSetSpirosClear(spl);

	/* if we changed all the points then the control points are right */
	/*  otherwise those near the edges may be wonky, fix 'em up */
	/* Figuring out where the edges of the selection are is difficult */
	/*  so let's just tweak all points, it shouldn't matter */
	/* It does matter. Let's tweak all default points */
	if( !(tpmask & tpmask_dontFixControlPoints))
	{
	    if ( tpt!=tpt_AllPoints && !allsel && spl->first->next!=NULL && !spl->first->next->order2 )
	    {
		pfirst = NULL;
		for ( spt = spl->first ; spt!=pfirst; spt = spt->next->to )
		{
		    if ( pfirst==NULL ) pfirst = spt;
		    if ( spt->selected && spt->prev!=NULL && !spt->prev->from->selected &&
			 spt->prev->from->pointtype == pt_tangent )
			SplineCharTangentPrevCP(spt->prev->from);
		    if ( spt->selected && spt->next!=NULL && !spt->next->to->selected &&
			 spt->next->to->pointtype == pt_tangent )
			SplineCharTangentNextCP(spt->next->to);
		    if ( spt->prev!=NULL && spt->prevcpdef && tpt==tpt_OnlySelected )
			SplineCharDefaultPrevCP(spt);
		    if ( spt->next==NULL )
			break;
		    if ( spt->nextcpdef && tpt==tpt_OnlySelected )
			SplineCharDefaultNextCP(spt);
		}
	    }
	}
	first = NULL;
	for ( spline = spl->first->next; spline!=NULL && spline!=first; spline=spline->to->next ) {
	    if ( !alldone ) SplineRefigureFixup(spline); else SplineRefigure(spline);
	    if ( first==NULL ) first = spline;
	}
    }
return( base );
}

SplinePointList *SplinePointListTransform( SplinePointList *base, real transform[6],
					   enum transformPointType tpt )
{
    enum transformPointMask tpmask = 0;
    return SplinePointListTransformExtended( base, transform, tpt, tpmask );
}

HintMask *HintMaskFromTransformedRef(RefChar *ref,BasePoint *trans,
	SplineChar *basesc,HintMask *hm) {
    StemInfo *st, *st2;
    int hst_cnt, bcnt;
    real start, width;
    int i;

    if ( ref->transform[1]!=0 || ref->transform[2]!=0 )
return(NULL);

    memset(hm,0,sizeof(HintMask));
    for ( st = ref->sc->hstem; st!=NULL; st=st->next ) {
	start = st->start*ref->transform[3] + ref->transform[5] + trans->y;
	width = st->width*ref->transform[3];
	for ( st2=basesc->hstem,bcnt=0; st2!=NULL; st2=st2->next, bcnt++ )
	    if ( st2->start == start && st2->width == width )
	break;
	if ( st2!=NULL )
	    (*hm)[bcnt>>3] |= (0x80>>(bcnt&7));
    }
    for ( st2=basesc->hstem,hst_cnt=0; st2!=NULL; st2=st2->next, hst_cnt++ );

    for ( st = ref->sc->vstem; st!=NULL; st=st->next ) {
	start = st->start*ref->transform[0] + ref->transform[4] + trans->x;
	width = st->width*ref->transform[0];
	for ( st2=basesc->vstem,bcnt=hst_cnt; st2!=NULL; st2=st2->next, bcnt++ )
	    if ( st2->start == start && st2->width == width )
	break;
	if ( st2!=NULL )
	    (*hm)[bcnt>>3] |= (0x80>>(bcnt&7));
    }
    for ( i=0; i<HntMax/8; ++i )
	if ( (*hm)[i]!=0 )
return( hm );

return( NULL );
}

static HintMask *HintMaskTransform(HintMask *oldhm,real transform[6],
	SplineChar *basesc,SplineChar *subsc) {
    HintMask *newhm;
    StemInfo *st, *st2;
    int cnt, hst_cnt, bcnt;
    real start, width;

    if ( transform[1]!=0 || transform[2]!=0 )
return( NULL );

    newhm = chunkalloc(sizeof(HintMask));
    for ( st = subsc->hstem,cnt = 0; st!=NULL; st=st->next, cnt++ ) {
	if ( (*oldhm)[cnt>>3]&(0x80>>(cnt&7)) ) {
	    start = st->start*transform[3] + transform[5];
	    width = st->width*transform[3];
	    for ( st2=basesc->hstem,bcnt=0; st2!=NULL; st2=st2->next, bcnt++ )
		if ( st2->start == start && st2->width == width )
	    break;
	    if ( st2!=NULL )
		(*newhm)[bcnt>>3] |= (0x80>>(bcnt&7));
	}
    }
    for ( st2=basesc->hstem,hst_cnt=0; st2!=NULL; st2=st2->next, hst_cnt++ );

    for ( st = subsc->vstem; st!=NULL; st=st->next, cnt++ ) {
	if ( (*oldhm)[cnt>>3]&(0x80>>(cnt&7)) ) {
	    start = st->start*transform[0] + transform[4];
	    width = st->width*transform[0];
	    for ( st2=basesc->vstem,bcnt=hst_cnt; st2!=NULL; st2=st2->next, bcnt++ )
		if ( st2->start == start && st2->width == width )
	    break;
	    if ( st2!=NULL )
		(*newhm)[bcnt>>3] |= (0x80>>(bcnt&7));
	}
    }
return( newhm );
}

SplinePointList *SPLCopyTranslatedHintMasks(SplinePointList *base,
	SplineChar *basesc, SplineChar *subsc, BasePoint *trans ) {
    SplinePointList *spl, *spl2, *head;
    SplinePoint *spt, *spt2, *pfirst;
    real transform[6];
    Spline *s, *first;

    head = SplinePointListCopy(base);

    transform[0] = transform[3] = 1; transform[1] = transform[2] = 0;
    transform[4] = trans->x; transform[5] = trans->y;

    for ( spl = head, spl2=base; spl!=NULL; spl = spl->next, spl2 = spl2->next ) {
	pfirst = NULL;
	for ( spt = spl->first, spt2 = spl2->first ; spt!=pfirst; spt = spt->next->to, spt2 = spt2->next->to ) {
	    if ( pfirst==NULL ) pfirst = spt;
	    TransformPoint(spt,transform);
	    if ( spt2->hintmask ) {
		chunkfree(spt->hintmask,sizeof(HintMask));
		spt->hintmask = HintMaskTransform(spt2->hintmask,transform,basesc,subsc);
	    }
	    if ( spt->next==NULL )
	break;
	}
	first = NULL;
	for ( s = spl->first->next; s!=NULL && s!=first; s=s->to->next ) {
	    SplineRefigure(s);
	    if ( first==NULL ) first = s;
	}
    }
return( head );
}

static SplinePointList *_SPLCopyTransformedHintMasks(SplineChar *subsc,int layer,
	real transform[6], SplineChar *basesc ) {
    SplinePointList *spl, *spl2, *head, *last=NULL, *cur, *base;
    SplinePoint *spt, *spt2, *pfirst;
    Spline *s, *first;
    real trans[6];
    RefChar *rf;

    base = subsc->layers[layer].splines;
    head = SplinePointListCopy(base);
    if ( head!=NULL )
	for ( last = head; last->next!=NULL; last=last->next );

    for ( spl = head, spl2=base; spl!=NULL; spl = spl->next, spl2=spl2->next ) {
	pfirst = NULL;
	for ( spt = spl->first, spt2 = spl2->first ; spt!=pfirst; spt = spt->next->to, spt2 = spt2->next->to ) {
	    if ( pfirst==NULL ) pfirst = spt;
	    TransformPoint(spt,transform);
	    if ( spt2->hintmask ) {
		chunkfree(spt->hintmask,sizeof(HintMask));
		spt->hintmask = HintMaskTransform(spt2->hintmask,transform,basesc,subsc);
	    }
	    if ( spt->next==NULL )
	break;
	}
	first = NULL;
	for ( s = spl->first->next; s!=NULL && s!=first; s=s->to->next ) {
	    SplineRefigure(s);
	    if ( first==NULL ) first = s;
	}
    }
    for ( rf=subsc->layers[layer].refs; rf!=NULL; rf=rf->next ) {
	trans[0] = rf->transform[0]*transform[0] +
		    rf->transform[1]*transform[2];
	trans[1] = rf->transform[0]*transform[1] +
		    rf->transform[1]*transform[3];
	trans[2] = rf->transform[2]*transform[0] +
		    rf->transform[3]*transform[2];
	trans[3] = rf->transform[2]*transform[1] +
		    rf->transform[3]*transform[3];
	trans[4] = rf->transform[4]*transform[0] +
		    rf->transform[5]*transform[2] +
		    transform[4];
	trans[5] = rf->transform[4]*transform[1] +
		    rf->transform[5]*transform[3] +
		    transform[5];
	cur = _SPLCopyTransformedHintMasks(rf->sc,layer,trans,basesc);
	if ( head==NULL )
	    head = cur;
	else
	    last->next = cur;
	if ( cur!=NULL ) {
	    while ( cur->next!=NULL ) cur = cur->next;
	    last = cur;
	}
    }
return( head );
}

SplinePointList *SPLCopyTransformedHintMasks(RefChar *r,
	SplineChar *basesc, BasePoint *trans,int layer ) {
    real transform[6];

    memcpy(transform,r->transform,sizeof(transform));
    transform[4] += trans->x; transform[5] += trans->y;
return( _SPLCopyTransformedHintMasks(r->sc,layer,transform,basesc));
}

void SCMakeDependent(SplineChar *dependent,SplineChar *base) {
    struct splinecharlist *dlist;

    if ( dependent->searcherdummy )
return;

    for ( dlist=base->dependents; dlist!=NULL && dlist->sc!=dependent; dlist = dlist->next);
    if ( dlist==NULL ) {
	dlist = chunkalloc(sizeof(struct splinecharlist));
	dlist->sc = dependent;
	dlist->next = base->dependents;
	base->dependents = dlist;
    }
}

static void LayerToRefLayer(struct reflayer *rl,Layer *layer, real transform[6]) {
    BrushCopy(&rl->fill_brush, &layer->fill_brush,transform);
    PenCopy(&rl->stroke_pen, &layer->stroke_pen,transform);
    rl->dofill = layer->dofill;
    rl->dostroke = layer->dostroke;
    rl->fillfirst = layer->fillfirst;
}

int RefLayerFindBaseLayerIndex(RefChar *rf, int layer) {
	// Note that most of the logic below is copied and lightly modified from SCReinstanciateRefChar.
	SplineChar *rsc = rf->sc;
	int i = 0, j = 0, cnt = 0;
	RefChar *subref;
	for ( i=ly_fore; i<rsc->layer_cnt; ++i ) {
	    if ( rsc->layers[i].splines!=NULL) {
	        if (cnt == layer) return i;
		++cnt;
	    }
	    for ( subref=rsc->layers[i].refs; subref!=NULL; subref=subref->next ) {
		for ( j=0; j<subref->layer_cnt; ++j ) if ( subref->layers[j].splines!=NULL ) {
		    if (cnt == layer) return i;
		    ++cnt;
		}
	    }
	}
	return -1;
}

void RefCharFindBounds(RefChar *rf) {
    int i;
    SplineChar *rsc = rf->sc;
    real extra=0,e;

    memset(&rf->bb,'\0',sizeof(rf->bb));
    rf->top.y = -1e10;
    for ( i=0; i<rf->layer_cnt; ++i ) {
	_SplineSetFindBounds(rf->layers[i].splines,&rf->bb);
	_SplineSetFindTop(rf->layers[i].splines,&rf->top);
	int baselayer = RefLayerFindBaseLayerIndex(rf, i);
	if ( baselayer >= 0 && rsc->layers[baselayer].dostroke ) {
	    if ( rf->layers[i].stroke_pen.width!=WIDTH_INHERITED )
		e = rf->layers[i].stroke_pen.width*rf->layers[i].stroke_pen.trans[0];
	    else
		e = rf->layers[i].stroke_pen.trans[0];
	    if ( e>extra ) extra = e;
	}
    }
    if ( rf->top.y < -65536 ) rf->top.y = rf->top.x = 0;
    rf->bb.minx -= extra; rf->bb.miny -= extra;
    rf->bb.maxx += extra; rf->bb.maxy += extra;
}

void SCReinstanciateRefChar(SplineChar *sc,RefChar *rf,int layer) {
    SplinePointList *new, *last;
    RefChar *refs;
    int i,j;
    SplineChar *rsc = rf->sc;
    real extra=0,e;

    for ( i=0; i<rf->layer_cnt; ++i ) {
	SplinePointListsFree(rf->layers[i].splines);
	GradientFree(rf->layers[i].fill_brush.gradient);
	PatternFree(rf->layers[i].fill_brush.pattern);
	GradientFree(rf->layers[i].stroke_pen.brush.gradient);
	PatternFree(rf->layers[i].stroke_pen.brush.pattern);
    }
    free( rf->layers );
    rf->layers = NULL;
    rf->layer_cnt = 0;
    if ( rsc==NULL )
return;
    /* Can be called before sc->parent is set, but only when reading a ttf */
    /*  file which won't be multilayer */
    if ( sc->parent!=NULL && sc->parent->multilayer ) {
	int cnt = 0;
	RefChar *subref;
	for ( i=ly_fore; i<rsc->layer_cnt; ++i ) {
	    if ( rsc->layers[i].splines!=NULL)
		++cnt;
	    for ( subref=rsc->layers[i].refs; subref!=NULL; subref=subref->next )
		cnt += subref->layer_cnt;
	}

	rf->layer_cnt = cnt;
	rf->layers = calloc(cnt,sizeof(struct reflayer));
	cnt = 0;
	for ( i=ly_fore; i<rsc->layer_cnt; ++i ) {
	    if ( rsc->layers[i].splines!=NULL ) {
		rf->layers[cnt].splines =
			SplinePointListTransform(
			 SplinePointListCopy(rsc->layers[i].splines),rf->transform,tpt_AllPoints);
		LayerToRefLayer(&rf->layers[cnt],&rsc->layers[i],rf->transform);
		++cnt;
	    }
	    for ( subref=rsc->layers[i].refs; subref!=NULL; subref=subref->next ) {
		for ( j=0; j<subref->layer_cnt; ++j ) if ( subref->layers[j].splines!=NULL ) {
		    rf->layers[cnt] = subref->layers[j];
		    rf->layers[cnt].splines =
			    SplinePointListTransform(
			     SplinePointListCopy(subref->layers[j].splines),rf->transform,tpt_AllPoints);
		    ++cnt;
		}
	    }
	}

	memset(&rf->bb,'\0',sizeof(rf->bb));
	rf->top.y = -1e10;
	for ( i=0; i<rf->layer_cnt; ++i ) {
	    _SplineSetFindBounds(rf->layers[i].splines,&rf->bb);
	    _SplineSetFindTop(rf->layers[i].splines,&rf->top);
	    int baselayer = RefLayerFindBaseLayerIndex(rf, i);
	    if ( baselayer >= 0 && rsc->layers[baselayer].dostroke ) {
		if ( rf->layers[i].stroke_pen.width!=WIDTH_INHERITED )
		    e = rf->layers[i].stroke_pen.width*rf->layers[i].stroke_pen.trans[0];
		else
		    e = rf->layers[i].stroke_pen.trans[0];
		if ( e>extra ) extra = e;
	    }
	}
	if ( rf->top.y < -65536 ) rf->top.y = rf->top.x = 0;
	rf->bb.minx -= extra; rf->bb.miny -= extra;
	rf->bb.maxx += extra; rf->bb.maxy += extra;
    } else {
	if ( rf->layer_cnt>0 ) {
	    SplinePointListsFree(rf->layers[0].splines);
	    rf->layers[0].splines = NULL;
	}
	rf->layers = calloc(1,sizeof(struct reflayer));
	rf->layer_cnt = 1;
	rf->layers[0].dofill = true;
	new = SplinePointListTransform(SplinePointListCopy(rf->sc->layers[layer].splines),rf->transform,tpt_AllPoints);
	rf->layers[0].splines = new;
	last = NULL;
	if ( new!=NULL )
	    for ( last = new; last->next!=NULL; last = last->next );
	for ( refs = rf->sc->layers[layer].refs; refs!=NULL; refs = refs->next ) {
	    new = SplinePointListTransform(SplinePointListCopy(refs->layers[0].splines),rf->transform,tpt_AllPoints);
	    if ( last!=NULL )
		last->next = new;
	    else
		rf->layers[0].splines = new;
	    if ( new!=NULL )
		for ( last = new; last->next!=NULL; last = last->next );
	}
    }
    RefCharFindBounds(rf);
}

/* This returns all real solutions, even those out of bounds */
/* I use -999999 as an error flag, since we're really only interested in */
/*  solns near 0 and 1 that should be ok. -1 is perhaps a little too close */
/* Sigh. When solutions are near 0, the rounding errors are appalling. */
int _CubicSolve(const Spline1D *sp,bigreal sought, extended ts[3]) {
    extended d, xN, yN, delta2, temp, delta, h, t2, t3, theta;
    extended sa=sp->a, sb=sp->b, sc=sp->c, sd=sp->d-sought;
    int i=0;

    ts[0] = ts[1] = ts[2] = -999999;
    if ( sd==0 && sa!=0 ) {
	/* one of the roots is 0, the other two are the soln of a quadratic */
	ts[0] = 0;
	if ( sc==0 ) {
	    ts[1] = -sb/(extended) sa;	/* two zero roots */
	} else {
	    temp = sb*(extended) sb-4*(extended) sa*sc;
	    if ( RealNear(temp,0))
		ts[1] = -sb/(2*(extended) sa);
	    else if ( temp>=0 ) {
		temp = sqrt(temp);
		ts[1] = (-sb+temp)/(2*(extended) sa);
		ts[2] = (-sb-temp)/(2*(extended) sa);
	    }
	}
    } else if ( sa!=0 ) {
    /* http://www.m-a.org.uk/eb/mg/mg077ch.pdf */
    /* this nifty solution to the cubic neatly avoids complex arithmatic */
	xN = -sb/(3*(extended) sa);
	yN = ((sa*xN + sb)*xN+sc)*xN + sd;

	delta2 = (sb*(extended) sb-3*(extended) sa*sc)/(9*(extended) sa*sa);
	/*if ( RealWithin(delta2,0,.00000001) ) delta2 = 0;*/

	/* the descriminant is yN^2-h^2, but delta might be <0 so avoid using h */
	d = yN*yN - 4*sa*sa*delta2*delta2*delta2;
	if ( ((yN>.01 || yN<-.01) && RealNear(d/yN,0)) || ((yN<=.01 && yN>=-.01) && RealNear(d,0)) )
	    d = 0;
	if ( d>0 ) {
	    temp = sqrt(d);
	    t2 = (-yN-temp)/(2*sa);
	    t2 = (t2==0) ? 0 : (t2<0) ? -pow(-t2,1./3.) : pow(t2,1./3.);
	    t3 = (-yN+temp)/(2*sa);
	    t3 = t3==0 ? 0 : (t3<0) ? -pow(-t3,1./3.) : pow(t3,1./3.);
	    ts[0] = xN + t2 + t3;
	} else if ( d<0 ) {
	    if ( delta2>=0 ) {
		delta = sqrt(delta2);
		h = 2*sa*delta2*delta;
		temp = -yN/h;
		if ( temp>=-1.0001 && temp<=1.0001 ) {
		    if ( temp<-1 ) temp = -1; else if ( temp>1 ) temp = 1;
		    theta = acos(temp)/3;
		    ts[i++] = xN+2*delta*cos(theta);
		    ts[i++] = xN+2*delta*cos(2.0943951+theta);	/* 2*pi/3 */
		    ts[i++] = xN+2*delta*cos(4.1887902+theta);	/* 4*pi/3 */
		}
	    }
	} else if ( /* d==0 && */ delta2!=0 ) {
	    delta = yN/(2*sa);
	    delta = delta==0 ? 0 : delta>0 ? pow(delta,1./3.) : -pow(-delta,1./3.);
	    ts[i++] = xN + delta;	/* this root twice, but that's irrelevant to me */
	    ts[i++] = xN - 2*delta;
	} else if ( /* d==0 && */ delta2==0 ) {
	    if ( xN>=-0.0001 && xN<=1.0001 ) ts[0] = xN;
	}
    } else if ( sb!=0 ) {
	extended d = sc*(extended) sc-4*(extended) sb*sd;
	if ( d<0 && RealNear(d,0)) d=0;
	if ( d<0 )
return(false);		/* All roots imaginary */
	d = sqrt(d);
	ts[0] = (-sc-d)/(2*(extended) sb);
	ts[1] = (-sc+d)/(2*(extended) sb);
    } else if ( sc!=0 ) {
	ts[0] = -sd/(extended) sc;
    } else {
	/* If it's a point then either everything is a solution, or nothing */
    }
return( ts[0]!=-999999 );
}

int CubicSolve(const Spline1D *sp,bigreal sought, extended ts[3]) {
    extended t;
    extended ts2[3];
    int i,j;
    /* This routine gives us all solutions between [0,1] with -1 as an error flag */
    /* http://mathforum.org/dr.math/faq/faq.cubic.equations.html */

    ts[0] = ts[1] = ts[2] = -1;
    if ( !_CubicSolve(sp,sought,ts2)) {
return( false );
    }

    for ( i=j=0; i<3; ++i ) {
	if ( ts2[i]>-.0001 && ts2[i]<1.0001 ) {
	    if ( ts2[i]<0 ) ts[j++] = 0;
	    else if ( ts2[i]>1 ) ts[j++] = 1;
	    else
		ts[j++] = ts2[i];
	}
    }
    if ( j==0 )
return( false );

    if ( ts[0]>ts[2] && ts[2]!=-1 ) {
	t = ts[0]; ts[0] = ts[2]; ts[2] = t;
    }
    if ( ts[0]>ts[1] && ts[1]!=-1 ) {
	t = ts[0]; ts[0] = ts[1]; ts[1] = t;
    }
    if ( ts[1]>ts[2] && ts[2]!=-1 ) {
	t = ts[1]; ts[1] = ts[2]; ts[2] = t;
    }
return( true );
}

/* An IEEE double has 52 bits of precision. So one unit of rounding error will be */
/*  the number divided by 2^51 */
# define D_RE_Factor	(1024.0*1024.0*1024.0*1024.0*1024.0*2.0)
/* But that's not going to work near 0, so, since the t values we care about */
/*  are [0,1], let's use 1.0/D_RE_Factor */

extended IterateSplineSolve(const Spline1D *sp, extended tmin, extended tmax,
	extended sought) {
    extended t, low, high, test;
    Spline1D temp;
    /* Now the closed form CubicSolver can have rounding errors so if we know */
    /*  the spline to be monotonic, an iterative approach is more accurate */

    if ( tmin>tmax ) {
	t=tmin; tmin=tmax; tmax=t;
    }

    temp = *sp;
    temp.d -= sought;

    if ( temp.a==0 && temp.b==0 && temp.c!=0 ) {
	t = -temp.d/(extended) temp.c;
	if ( t<tmin || t>tmax )
return( -1 );
return( t );
    }

    low = ((temp.a*tmin+temp.b)*tmin+temp.c)*tmin+temp.d;
    high = ((temp.a*tmax+temp.b)*tmax+temp.c)*tmax+temp.d;
    if ( low==0 )
return(tmin);
    if ( high==0 )
return(tmax);
    if (( low<0 && high>0 ) ||
	    ( low>0 && high<0 )) {

	for (;;) {
	    t = (tmax+tmin)/2;
	    if ( t==tmax || t==tmin )
return( t );
	    test = ((temp.a*t+temp.b)*t+temp.c)*t+temp.d;
	    if ( test==0 )	/* someone complained that this test relied on exact arithmetic. In fact this test will almost never be hit, the real exit test is the line above, when tmin/tmax are so close that there is no space between them in the floating representation */
return( t );
	    if ( (low<0 && test<0) || (low>0 && test>0) )
		tmin=t;
	    else
		tmax = t;
	}
    } else if ( low<.0001 && low>-.0001 )
return( tmin );			/* Rounding errors */
    else if ( high<.0001 && high>-.0001 )
return( tmax );

return( -1 );
}

extended IterateSplineSolveFixup(const Spline1D *sp, extended tmin, extended tmax,
	extended sought) {
    // Search between tmin and tmax for a t-value at which the spline outputs sought.
    extended t;
    bigreal factor;
    extended val, valp, valm;

    if ( tmin>tmax ) {
	t=tmin; tmin=tmax; tmax=t;
    }

    t = IterateSplineSolve(sp,tmin,tmax,sought);

    if ( t==-1 )
return( -1 );

    if ((val = (((sp->a*t+sp->b)*t+sp->c)*t+sp->d) - sought)<0 )
	val=-val;
    if ( val!=0 ) {
	for ( factor=1024.0*1024.0*1024.0*1024.0*1024.0; factor>.5; factor/=2.0 ) {
	    extended tp = t + (factor*t)/D_RE_Factor;
	    extended tm = t - (factor*t)/D_RE_Factor;
	    if ( tp>tmax ) tp=tmax;
	    if ( tm<tmin ) tm=tmin;
	    if ( (valp = (((sp->a*tp+sp->b)*tp+sp->c)*tp+sp->d) - sought)<0 )
		valp = -valp;
	    if ( (valm = (((sp->a*tm+sp->b)*tm+sp->c)*tm+sp->d) - sought)<0 )
		valm = -valm;
	    if ( valp<val && valp<valm ) {
		t = tp;
		val = valp;
	    } else if ( valm<val ) {
		t = tm;
		val = valm;
	    }
	}
    }
    if ( t==0 && !Within16RoundingErrors(sought,sought+val))
return( -1 );
    /* if t!=0 then we we get the chance of far worse rounding errors */
    else if ( t==tmax || t==tmin ) {
	if ( Within16RoundingErrors(sought,sought+val) ||
		Within16RoundingErrors(sp->a,sp->a+val) ||
		Within16RoundingErrors(sp->b,sp->b+val) ||
		Within16RoundingErrors(sp->c,sp->c+val) ||
		Within16RoundingErrors(sp->c,sp->c+val) ||
		Within16RoundingErrors(sp->d,sp->d+val))
return( t );
	else
return( -1 );
    }

    if ( t>=tmin && t<=tmax )
return( t );

    /* I don't think this can happen... */
return( -1 );
}

double CheckExtremaForSingleBitErrors(const Spline1D *sp, double t, double othert) {
    double u1, um1;
    double slope, slope1, slopem1;
    int err;
    double diff, factor;

    if ( t<0 || t>1 )
return( t );

    factor = t*0x40000/D_RE_Factor;
    if ( (diff = t-othert)<0 ) diff= -diff;
    if ( factor>diff/4 && diff!=0 )		/* This little check is to insure we don't skip beyond the well of this extremum into the next */
	factor = diff/4;

    slope = (3*(double) sp->a*t+2*sp->b)*t+sp->c;
    if ( slope<0 ) slope = -slope;

    for ( err = 0x40000; err!=0; err>>=1 ) {
	u1 = t+factor;
	slope1 = (3*(double) sp->a*u1+2*sp->b)*u1+sp->c;
	if ( slope1<0 ) slope1 = -slope1;

	um1 = t-factor;
	slopem1 = (3*(double) sp->a*um1+2*sp->b)*um1+sp->c;
	if ( slopem1<0 ) slopem1 = -slopem1;

	if ( slope1<slope && slope1<=slopem1 && u1<=1.0 ) {
	    t = u1;
	} else if ( slopem1<slope && slopem1<=slope1 && um1>=0.0 ) {
	    t = um1;
	}
	factor /= 2.0;
    }
    /* that seems as good as it gets */

return( t );
}

void SplineFindExtrema(const Spline1D *sp, extended *_t1, extended *_t2 ) {
    extended t1= -1, t2= -1;
    extended b2_fourac;

    /* Find the extreme points on the curve */
    /*  Set to -1 if there are none or if they are outside the range [0,1] */
    /*  Order them so that t1<t2 */
    /*  If only one valid extremum it will be t1 */
    /*  (Does not check the end points unless they have derivative==0) */
    /*  (Does not check to see if d/dt==0 points are inflection points (rather than extrema) */
    if ( sp->a!=0 ) {
	/* cubic, possibly 2 extrema (possibly none) */
	b2_fourac = 4*(extended) sp->b*sp->b - 12*(extended) sp->a*sp->c;
	if ( b2_fourac>=0 ) {
	    b2_fourac = sqrt(b2_fourac);
	    t1 = (-2*sp->b - b2_fourac) / (6*sp->a);
	    t2 = (-2*sp->b + b2_fourac) / (6*sp->a);
	    t1 = CheckExtremaForSingleBitErrors(sp,t1,t2);
	    t2 = CheckExtremaForSingleBitErrors(sp,t2,t1);
	    if ( t1>t2 ) { extended temp = t1; t1 = t2; t2 = temp; }
	    else if ( t1==t2 ) t2 = -1;
	    if ( RealNear(t1,0)) t1=0; else if ( RealNear(t1,1)) t1=1;
	    if ( RealNear(t2,0)) t2=0; else if ( RealNear(t2,1)) t2=1;
	    if ( t2<=0 || t2>=1 ) t2 = -1;
	    if ( t1<=0 || t1>=1 ) { t1 = t2; t2 = -1; }
	}
    } else if ( sp->b!=0 ) {
	/* Quadratic, at most one extremum */
	t1 = -sp->c/(2.0*(extended) sp->b);
	if ( t1<=0 || t1>=1 ) t1 = -1;
    } else /*if ( sp->c!=0 )*/ {
	/* linear, no extrema */
    }
    *_t1 = t1; *_t2 = t2;
}

bigreal SplineCurvature(Spline *s, bigreal t) {
	/* Kappa = (x'y'' - y'x'') / (x'^2 + y'^2)^(3/2) */
    bigreal dxdt, dydt, d2xdt2, d2ydt2, denom, numer;

    if ( s==NULL )
return( CURVATURE_ERROR );

    dxdt = (3*s->splines[0].a*t+2*s->splines[0].b)*t+s->splines[0].c;
    dydt = (3*s->splines[1].a*t+2*s->splines[1].b)*t+s->splines[1].c;
    d2xdt2 = 6*s->splines[0].a*t + 2*s->splines[0].b;
    d2ydt2 = 6*s->splines[1].a*t + 2*s->splines[1].b;
    denom = pow( dxdt*dxdt + dydt*dydt, 3.0/2.0 );
    numer = dxdt*d2ydt2 - dydt*d2xdt2;

    if ( numer==0 )
return( 0 );
    if ( denom==0 )
return( CURVATURE_ERROR );

return( numer/denom );
}

int Spline2DFindExtrema(const Spline *sp, extended extrema[4] ) {
    int i,j;
    BasePoint last, cur, mid;

    /* If the control points are at the end-points then this (1D) spline is */
    /*  basically a line. But rounding errors can give us very faint extrema */
    /*  if we look for them */
    if ( !Spline1DCantExtremeX(sp) )
	SplineFindExtrema(&sp->splines[0],&extrema[0],&extrema[1]);
    else
	extrema[0] = extrema[1] = -1;
    if ( !Spline1DCantExtremeY(sp) )
	SplineFindExtrema(&sp->splines[1],&extrema[2],&extrema[3]);
    else
	extrema[2] = extrema[3] = -1;

    for ( i=0; i<3; ++i ) for ( j=i+1; j<4; ++j ) {
	if ( (extrema[i]==-1 && extrema[j]!=-1) || (extrema[i]>extrema[j] && extrema[j]!=-1) ) {
	    extended temp = extrema[i];
	    extrema[i] = extrema[j];
	    extrema[j] = temp;
	}
    }
    for ( i=j=0; i<3 && extrema[i]!=-1; ++i ) {
	if ( extrema[i]==extrema[i+1] ) {
	    for ( j=i+1; j<3; ++j )
		extrema[j] = extrema[j+1];
	    extrema[3] = -1;
	}
    }

    /* Extrema which are too close together are not interesting */
    last = sp->from->me;
    for ( i=0; i<4 && extrema[i]!=-1; ++i ) {
	cur.x = ((sp->splines[0].a*extrema[i]+sp->splines[0].b)*extrema[i]+
		sp->splines[0].c)*extrema[i]+sp->splines[0].d;
	cur.y = ((sp->splines[1].a*extrema[i]+sp->splines[1].b)*extrema[i]+
		sp->splines[1].c)*extrema[i]+sp->splines[1].d;
	mid.x = (last.x+cur.x)/2; mid.y = (last.y+cur.y)/2;
	if ( (mid.x==last.x || mid.x==cur.x) &&
		(mid.y==last.y || mid.y==cur.y)) {
	    for ( j=i; j<3; ++j )
		extrema[j] = extrema[j+1];
	    extrema[3] = -1;
	    --i;
	} else
	    last = cur;
    }
    if ( extrema[0]!=-1 ) {
	mid.x = (last.x+sp->to->me.x)/2; mid.y = (last.y+sp->to->me.y)/2;
	if ( (mid.x==last.x || mid.x==cur.x) &&
		(mid.y==last.y || mid.y==cur.y))
	    extrema[i-1] = -1;
    }
    for ( i=0; i<4 && extrema[i]!=-1; ++i );
    if ( i!=0 ) {
	cur = sp->to->me;
	mid.x = (last.x+cur.x)/2; mid.y = (last.y+cur.y)/2;
	if ( (mid.x==last.x || mid.x==cur.x) &&
		(mid.y==last.y || mid.y==cur.y))
	    extrema[--i] = -1;
    }

return( i );
}

int Spline2DFindPointsOfInflection(const Spline *sp, extended poi[2] ) {
    int cnt=0;
    extended a, b, c, b2_fourac, t;
    /* A POI happens when d2 y/dx2 is zero. This is not the same as d2y/dt2 / d2x/dt2 */
    /* d2 y/dx^2 = d/dt ( dy/dt / dx/dt ) / dx/dt */
    /*		 = ( (dx/dt) * d2 y/dt2 - ((dy/dt) * d2 x/dt2) )/ (dx/dt)^3 */
    /* (3ax*t^2+2bx*t+cx) * (6ay*t+2by) - (3ay*t^2+2by*t+cy) * (6ax*t+2bx) == 0 */
    /* (3ax*t^2+2bx*t+cx) * (3ay*t+by) - (3ay*t^2+2by*t+cy) * (3ax*t+bx) == 0 */
    /*   9*ax*ay*t^3 + (3ax*by+6bx*ay)*t^2 + (2bx*by+3cx*ay)*t + cx*by	   */
    /* -(9*ax*ay*t^3 + (3ay*bx+6by*ax)*t^2 + (2by*bx+3cy*ax)*t + cy*bx)==0 */
    /* 3*(ax*by-ay*bx)*t^2 + 3*(cx*ay-cy*ax)*t+ (cx*by-cy*bx) == 0	   */

    a = 3*((extended) sp->splines[1].a*sp->splines[0].b-(extended) sp->splines[0].a*sp->splines[1].b);
    b = 3*((extended) sp->splines[0].c*sp->splines[1].a - (extended) sp->splines[1].c*sp->splines[0].a);
    c = (extended) sp->splines[0].c*sp->splines[1].b-(extended) sp->splines[1].c*sp->splines[0].b;
    if ( !RealNear(a,0) ) {
	b2_fourac = b*b - 4*a*c;
	poi[0] = poi[1] = -1;
	if ( b2_fourac<0 )
return( 0 );
	b2_fourac = sqrt( b2_fourac );
	t = (-b+b2_fourac)/(2*a);
	if ( t>=0 && t<=1.0 )
	    poi[cnt++] = t;
	t = (-b-b2_fourac)/(2*a);
	if ( t>=0 && t<=1.0 ) {
	    if ( cnt==1 && poi[0]>t ) {
		poi[1] = poi[0];
		poi[0] = t;
		++cnt;
	    } else
		poi[cnt++] = t;
	}
    } else if ( !RealNear(b,0) ) {
	t = -c/b;
	if ( t>=0 && t<=1.0 )
	    poi[cnt++] = t;
    }
    if ( cnt<2 )
	poi[cnt] = -1;

return( cnt );
}

/* Ok, if the above routine finds an extremum that less than 1 unit */
/*  from an endpoint or another extremum, then many things are */
/*  just going to skip over it, and other things will be confused by this */
/*  so just remove it. It should be so close the difference won't matter */
void SplineRemoveExtremaTooClose(Spline1D *sp, extended *_t1, extended *_t2 ) {
    extended last, test;
    extended t1= *_t1, t2 = *_t2;

    if ( t1>t2 && t2!=-1 ) {
	t1 = t2;
	t2 = *_t1;
    }
    last = sp->d;
    if ( t1!=-1 ) {
	test = ((sp->a*t1+sp->b)*t1+sp->c)*t1+sp->d;
	if ( (test-last)*(test-last)<1 )
	    t1 = -1;
	else
	    last = test;
    }
    if ( t2!=-1 ) {
	test = ((sp->a*t2+sp->b)*t2+sp->c)*t2+sp->d;
	if ( (test-last)*(test-last)<1 )
	    t2 = -1;
	else
	    last = test;
    }
    test = sp->a+sp->b+sp->c+sp->d;
    if ( (test-last)*(test-last)<1 ) {
	if ( t2!=-1 )
	    t2 = -1;
	else if ( t1!=-1 )
	    t1 = -1;
	else {
	    /* Well we should just remove the whole spline? */
	    ;
	}
    }
    *_t1 = t1; *_t2 = t2;
}

int IntersectLines(BasePoint *inter,
	BasePoint *line1_1, BasePoint *line1_2,
	BasePoint *line2_1, BasePoint *line2_2) {
    // A lot of functions call this with the same address as an input and the output.
    // In order to avoid unexpected behavior, we delay writing to the output until the end.
    bigreal s1, s2;
    BasePoint _output;
    BasePoint * output = &_output;
    if ( line1_1->x == line1_2->x ) {
        // Line 1 is vertical.
	output->x = line1_1->x;
	if ( line2_1->x == line2_2->x ) {
            // Line 2 is vertical.
	    if ( line2_1->x!=line1_1->x )
              return( false );		/* Parallel vertical lines */
	    output->y = (line1_1->y+line2_1->y)/2;
	} else {
	    output->y = line2_1->y + (output->x-line2_1->x) * (line2_2->y - line2_1->y)/(line2_2->x - line2_1->x);
        }
        *inter = *output;
        return( true );
    } else if ( line2_1->x == line2_2->x ) {
        // Line 2 is vertical, but we know that line 1 is not.
	output->x = line2_1->x;
	output->y = line1_1->y + (output->x-line1_1->x) * (line1_2->y - line1_1->y)/(line1_2->x - line1_1->x);
        *inter = *output;
        return( true );
    } else {
        // Both lines are oblique.
	s1 = (line1_2->y - line1_1->y)/(line1_2->x - line1_1->x);
	s2 = (line2_2->y - line2_1->y)/(line2_2->x - line2_1->x);
	if ( RealNear(s1,s2)) {
	    if ( !RealNear(line1_1->y + (line2_1->x-line1_1->x) * s1,line2_1->y))
              return( false );
	    output->x = (line1_2->x+line2_2->x)/2;
	    output->y = (line1_2->y+line2_2->y)/2;
	} else {
	    output->x = (s1*line1_1->x - s2*line2_1->x - line1_1->y + line2_1->y)/(s1-s2);
	    output->y = line1_1->y + (output->x-line1_1->x) * s1;
	}
        *inter = *output;
        return( true );
    }
}

int IntersectLinesClip(BasePoint *inter,
	BasePoint *line1_1, BasePoint *line1_2,
	BasePoint *line2_1, BasePoint *line2_2) {
    BasePoint old = *inter, unit;
    bigreal len, val;

    if ( !IntersectLines(inter,line1_1,line1_2,line2_1,line2_2))
return( false );
    else {
	unit.x = line2_2->x-line1_2->x;
	unit.y = line2_2->y-line1_2->y;
	len = sqrt(unit.x*unit.x + unit.y*unit.y);
	if ( len==0 )
return( false );
	else {
	    unit.x /= len; unit.y /= len;
	    val = unit.x*(inter->x-line1_2->x) + unit.y*(inter->y-line1_2->y);
	    if ( val<=0 || val>=len ) {
		*inter = old;
return( false );
	    }
	}
    }
return( true );
}

static int AddPoint(extended x,extended y,extended t,extended s,BasePoint *pts,
	extended t1s[3],extended t2s[3], int soln) {
    int i;

    for ( i=0; i<soln; ++i )
	if ( x==pts[i].x && y==pts[i].y )
return( soln );
    if ( soln>=9 )
	IError( "Too many solutions!\n" );
    t1s[soln] = t;
    t2s[soln] = s;
    pts[soln].x = x;
    pts[soln].y = y;
return( soln+1 );
}

static void IterateSolve(const Spline1D *sp,extended ts[3]) {
    /* The closed form solution has too many rounding errors for my taste... */
    int i,j;

    ts[0] = ts[1] = ts[2] = -1;

    if ( sp->a!=0 ) {
	extended e[4];
	e[0] = 0; e[1] = e[2] = e[3] = 1.0;
	SplineFindExtrema(sp,&e[1],&e[2]);
	if ( e[1]==-1 ) e[1] = 1;
	if ( e[2]==-1 ) e[2] = 1;
	for ( i=j=0; i<3; ++i ) {
	    ts[j] = IterateSplineSolve(sp,e[i],e[i+1],0);
	    if ( ts[j]!=-1 ) ++j;
	    if ( e[i+1]==1.0 )
	break;
	}
    } else if ( sp->b!=0 ) {
	extended b2_4ac = sp->c*(extended) sp->c - 4*sp->b*(extended) sp->d;
	if ( b2_4ac>=0 ) {
	    b2_4ac = sqrt(b2_4ac);
	    ts[0] = (-sp->c-b2_4ac)/(2*sp->b);
	    ts[1] = (-sp->c+b2_4ac)/(2*sp->b);
	    if ( ts[0]>ts[1] ) { bigreal t = ts[0]; ts[0] = ts[1]; ts[1] = t; }
	}
    } else if ( sp->c!=0 ) {
	ts[0] = -sp->d/(extended) sp->c;
    } else {
	/* No solutions, or all solutions */
	;
    }

    for ( i=j=0; i<3; ++i )
	if ( ts[i]>=0 && ts[i]<=1 )
	    ts[j++] = ts[i];
    for ( i=0; i<j-1; ++i )
	if ( ts[i]+.0000001>ts[i+1]) {
	    ts[i] = (ts[i]+ts[i+1])/2;
	    --j;
	    for ( ++i; i<j; ++i )
		ts[i] = ts[i+1];
	}
    if ( j!=0 ) {
	if ( ts[0]!=0 ) {
	    extended d0 = sp->d;
	    extended dt = ((sp->a*ts[0]+sp->b)*ts[0]+sp->c)*ts[0]+sp->d;
	    if ( d0<0 ) d0=-d0;
	    if ( dt<0 ) dt=-dt;
	    if ( d0<dt )
		ts[0] = 0;
	}
	if ( ts[j-1]!=1.0 ) {
	    extended d1 = sp->a+(extended) sp->b+sp->c+sp->d;
	    extended dt = ((sp->a*ts[j-1]+sp->b)*ts[j-1]+sp->c)*ts[j-1]+sp->d;
	    if ( d1<0 ) d1=-d1;
	    if ( dt<0 ) dt=-dt;
	    if ( d1<dt )
		ts[j-1] = 1;
	}
    }
    for ( ; j<3; ++j )
	ts[j] = -1;
}

static extended ISolveWithin(const Spline *spline,int major,
	extended val,extended tlow, extended thigh) {
    Spline1D temp;
    extended ts[3];
    const Spline1D *sp = &spline->splines[major];
    int i;

    /* Calculation for t=1 can yield rounding errors. Insist on the endpoints */
    /*  (the Spline1D is not a perfectly accurate description of the spline,  */
    /*   but the control points are right -- at least that's my defn.) */
    if ( tlow==0 && val==(&spline->from->me.x)[major] )
return( 0 );
    if ( thigh==1.0 && val==(&spline->to->me.x)[major] )
return( 1.0 );

    temp = *sp;
    temp.d -= val;
    IterateSolve(&temp,ts);
    if ( tlow<thigh ) {
	for ( i=0; i<3; ++i )
	    if ( ts[i]>=tlow && ts[i]<=thigh )
return( ts[i] );
	for ( i=0; i<3; ++i ) {
	    if ( ts[i]>=tlow-1./1024. && ts[i]<=tlow )
return( tlow );
	    if ( ts[i]>=thigh && ts[i]<=thigh+1./1024 )
return( thigh );
	}
    } else {
	for ( i=0; i<3; ++i )
	    if ( ts[i]>=thigh && ts[i]<=tlow )
return( ts[i] );
	for ( i=0; i<3; ++i ) {
	    if ( ts[i]>=thigh-1./1024. && ts[i]<=thigh )
return( thigh );
	    if ( ts[i]>=tlow && ts[i]<=tlow+1./1024 )
return( tlow );
	}
    }
return( -1 );
}

static int ICAddInter(int cnt,BasePoint *foundpos,extended *foundt1,extended *foundt2,
	const Spline *s1,const Spline *s2,extended t1,extended t2, int maxcnt) {

    if ( cnt>=maxcnt )
return( cnt );

    foundt1[cnt] = t1;
    foundt2[cnt] = t2;
    foundpos[cnt].x = ((s1->splines[0].a*t1+s1->splines[0].b)*t1+
			s1->splines[0].c)*t1+s1->splines[0].d;
    foundpos[cnt].y = ((s1->splines[1].a*t1+s1->splines[1].b)*t1+
			s1->splines[1].c)*t1+s1->splines[1].d;
return( cnt+1 );
}

static int ICBinarySearch(int cnt,BasePoint *foundpos,extended *foundt1,extended *foundt2,
	int other,
	const Spline *s1,const Spline *s2,extended t1low,extended t1high,extended t2low,extended t2high,
	int maxcnt) {
    int major;
    extended t1, t2;
    extended o1o, o2o, o1n, o2n, m;

    major = !other;
    o1o = ((s1->splines[other].a*t1low+s1->splines[other].b)*t1low+
		    s1->splines[other].c)*t1low+s1->splines[other].d;
    o2o = ((s2->splines[other].a*t2low+s2->splines[other].b)*t2low+
		    s2->splines[other].c)*t2low+s2->splines[other].d;
    for (;;) {
	t1 = (t1low+t1high)/2;
	m = ((s1->splines[major].a*t1+s1->splines[major].b)*t1+
			s1->splines[major].c)*t1+s1->splines[major].d;
	t2 = ISolveWithin(s2,major,m,t2low,t2high);
	if ( t2==-1 )
return( cnt );

	o1n = ((s1->splines[other].a*t1+s1->splines[other].b)*t1+
			s1->splines[other].c)*t1+s1->splines[other].d;
	o2n = ((s2->splines[other].a*t2+s2->splines[other].b)*t2+
			s2->splines[other].c)*t2+s2->splines[other].d;
	if (( o1n-o2n<.001 && o1n-o2n>-.001) ||
		(t1-t1low<.0001 && t1-t1low>-.0001))
return( ICAddInter(cnt,foundpos,foundt1,foundt2,s1,s2,t1,t2,maxcnt));
	if ( (o1o>o2o && o1n<o2n) || (o1o<o2o && o1n>o2n)) {
	    t1high = t1;
	    t2high = t2;
	} else {
	    t1low = t1;
	    t2low = t2;
	}
    }
}

static int CubicsIntersect(const Spline *s1,extended lowt1,extended hight1,BasePoint *min1,BasePoint *max1,
			    const Spline *s2,extended lowt2,extended hight2,BasePoint *min2,BasePoint *max2,
			    BasePoint *foundpos,extended *foundt1,extended *foundt2,
			    int maxcnt) {
    int major, other;
    BasePoint max, min;
    extended t1max, t1min, t2max, t2min, t1, t2, t1diff, oldt2;
    extended o1o, o2o, o1n, o2n, m;
    int cnt=0;

    if ( (min.x = min1->x)<min2->x ) min.x = min2->x;
    if ( (min.y = min1->y)<min2->y ) min.y = min2->y;
    if ( (max.x = max1->x)>max2->x ) max.x = max2->x;
    if ( (max.y = max1->y)>max2->y ) max.y = max2->y;
    if ( max.x<min.x || max.y<min.y )
return( 0 );
    if ( max.x-min.x > max.y-min.y )
	major = 0;
    else
	major = 1;
    other = 1-major;

    t1max = ISolveWithin(s1,major,(&max.x)[major],lowt1,hight1);
    t1min = ISolveWithin(s1,major,(&min.x)[major],lowt1,hight1);
    t2max = ISolveWithin(s2,major,(&max.x)[major],lowt2,hight2);
    t2min = ISolveWithin(s2,major,(&min.x)[major],lowt2,hight2);
    if ( t1max==-1 || t1min==-1 || t2max==-1 || t2min==-1 )
return( 0 );
    t1diff = (t1max-t1min)/64.0;
    if (RealNear(t1diff,0))
return( 0 );

    t1 = t1min; t2 = t2min;
    o1o = t1==0   ? (&s1->from->me.x)[other] :
	  t1==1.0 ? (&s1->to->me.x)[other] :
	    ((s1->splines[other].a*t1+s1->splines[other].b)*t1+
		    s1->splines[other].c)*t1+s1->splines[other].d;
    o2o = t2==0   ? (&s2->from->me.x)[other] :
	  t2==1.0 ? (&s2->to->me.x)[other] :
	    ((s2->splines[other].a*t2+s2->splines[other].b)*t2+
		    s2->splines[other].c)*t2+s2->splines[other].d;
    if ( o1o==o2o )
	cnt = ICAddInter(cnt,foundpos,foundt1,foundt2,s1,s2,t1,t2,maxcnt);
    for (;;) {
	if ( cnt>=maxcnt )
    break;
	t1 += t1diff;
	if (( t1max>t1min && t1>t1max ) || (t1max<t1min && t1<t1max) || cnt>3 )
    break;
	m =   t1==0   ? (&s1->from->me.x)[major] :
	      t1==1.0 ? (&s1->to->me.x)[major] :
		((s1->splines[major].a*t1+s1->splines[major].b)*t1+
			s1->splines[major].c)*t1+s1->splines[major].d;
	oldt2 = t2;
	t2 = ISolveWithin(s2,major,m,lowt2,hight2);
	if ( t2==-1 )
    continue;

	o1n = t1==0   ? (&s1->from->me.x)[other] :
	      t1==1.0 ? (&s1->to->me.x)[other] :
		((s1->splines[other].a*t1+s1->splines[other].b)*t1+
			s1->splines[other].c)*t1+s1->splines[other].d;
	o2n = t2==0   ? (&s2->from->me.x)[other] :
	      t2==1.0 ? (&s2->to->me.x)[other] :
		((s2->splines[other].a*t2+s2->splines[other].b)*t2+
			s2->splines[other].c)*t2+s2->splines[other].d;
	if ( o1n==o2n )
	    cnt = ICAddInter(cnt,foundpos,foundt1,foundt2,s1,s2,t1,t2,maxcnt);
	if ( (o1o>o2o && o1n<o2n) || (o1o<o2o && o1n>o2n))
	    cnt = ICBinarySearch(cnt,foundpos,foundt1,foundt2,other,
		    s1,s2,t1-t1diff,t1,oldt2,t2,maxcnt);
	o1o = o1n; o2o = o2n;
    }
return( cnt );
}

static int Closer(const Spline *s1,const Spline *s2,extended t1,extended t2,extended t1p,extended t2p) {
    bigreal x1 = ((s1->splines[0].a*t1+s1->splines[0].b)*t1+s1->splines[0].c)*t1+s1->splines[0].d;
    bigreal y1 = ((s1->splines[1].a*t1+s1->splines[1].b)*t1+s1->splines[1].c)*t1+s1->splines[1].d;
    bigreal x2 = ((s2->splines[0].a*t2+s2->splines[0].b)*t2+s2->splines[0].c)*t2+s2->splines[0].d;
    bigreal y2 = ((s2->splines[1].a*t2+s2->splines[1].b)*t2+s2->splines[1].c)*t2+s2->splines[1].d;
    bigreal diff = (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2);
    bigreal x1p = ((s1->splines[0].a*t1p+s1->splines[0].b)*t1p+s1->splines[0].c)*t1p+s1->splines[0].d;
    bigreal y1p = ((s1->splines[1].a*t1p+s1->splines[1].b)*t1p+s1->splines[1].c)*t1p+s1->splines[1].d;
    bigreal x2p = ((s2->splines[0].a*t2p+s2->splines[0].b)*t2p+s2->splines[0].c)*t2p+s2->splines[0].d;
    bigreal y2p = ((s2->splines[1].a*t2p+s2->splines[1].b)*t2p+s2->splines[1].c)*t2p+s2->splines[1].d;
    bigreal diffp = (x1p-x2p)*(x1p-x2p) + (y1p-y2p)*(y1p-y2p);

    if ( diff<diffp )
return( false );

return( true );
}

/* returns 0=>no intersection, 1=>at least one, location in pts, t1s, t2s */
/*  -1 => We couldn't figure it out in a closed form, have to do a numerical */
/*  approximation */
int SplinesIntersect(const Spline *s1, const Spline *s2, BasePoint pts[9],
	extended t1s[10], extended t2s[10]) {	/* One extra for a trailing -1 */
    BasePoint min1, max1, min2, max2;
    int soln = 0;
    extended x,y,t, ac0, ac1;
    int i,j,found;
    Spline1D spline;
    extended tempts[4];	/* 3 solns for cubics, 4 for quartics */
    extended extrema1[6], extrema2[6];
    int ecnt1, ecnt2;

    t1s[0] = t1s[1] = t1s[2] = t1s[3] = -1;
    t2s[0] = t2s[1] = t2s[2] = t2s[3] = -1;

    if ( s1==s2 && !s1->knownlinear && !s1->isquadratic )
	/* Special case see if it doubles back on itself anywhere */;
    else if ( s1==s2 )
return( 0 );			/* Linear and quadratics can't double back, can't self-intersect */
    else if ( s1->splines[0].a == s2->splines[0].a &&
	    s1->splines[0].b == s2->splines[0].b &&
	    s1->splines[0].c == s2->splines[0].c &&
	    s1->splines[0].d == s2->splines[0].d &&
	    s1->splines[1].a == s2->splines[1].a &&
	    s1->splines[1].b == s2->splines[1].b &&
	    s1->splines[1].c == s2->splines[1].c &&
	    s1->splines[1].d == s2->splines[1].d )
return( -1 );			/* Same spline. Intersects everywhere */

    /* Ignore splines which are just a point */
    if ( s1->knownlinear && s1->splines[0].c==0 && s1->splines[1].c==0 )
return( 0 );
    if ( s2->knownlinear && s2->splines[0].c==0 && s2->splines[1].c==0 )
return( 0 );

    if ( s1->knownlinear )
	/* Do Nothing */;
    else if ( s2->knownlinear || (!s1->isquadratic && s2->isquadratic)) {
	const Spline *stemp = s1;
	extended *ts = t1s;
	t1s = t2s; t2s = ts;
	s1 = s2; s2 = stemp;
    }

    min1 = s1->from->me; max1 = min1;
    min2 = s2->from->me; max2 = min2;
    if ( s1->from->nextcp.x>max1.x ) max1.x = s1->from->nextcp.x;
    else if ( s1->from->nextcp.x<min1.x ) min1.x = s1->from->nextcp.x;
    if ( s1->from->nextcp.y>max1.y ) max1.y = s1->from->nextcp.y;
    else if ( s1->from->nextcp.y<min1.y ) min1.y = s1->from->nextcp.y;
    if ( s1->to->prevcp.x>max1.x ) max1.x = s1->to->prevcp.x;
    else if ( s1->to->prevcp.x<min1.x ) min1.x = s1->to->prevcp.x;
    if ( s1->to->prevcp.y>max1.y ) max1.y = s1->to->prevcp.y;
    else if ( s1->to->prevcp.y<min1.y ) min1.y = s1->to->prevcp.y;
    if ( s1->to->me.x>max1.x ) max1.x = s1->to->me.x;
    else if ( s1->to->me.x<min1.x ) min1.x = s1->to->me.x;
    if ( s1->to->me.y>max1.y ) max1.y = s1->to->me.y;
    else if ( s1->to->me.y<min1.y ) min1.y = s1->to->me.y;

    if ( s2->from->nextcp.x>max2.x ) max2.x = s2->from->nextcp.x;
    else if ( s2->from->nextcp.x<min2.x ) min2.x = s2->from->nextcp.x;
    if ( s2->from->nextcp.y>max2.y ) max2.y = s2->from->nextcp.y;
    else if ( s2->from->nextcp.y<min2.y ) min2.y = s2->from->nextcp.y;
    if ( s2->to->prevcp.x>max2.x ) max2.x = s2->to->prevcp.x;
    else if ( s2->to->prevcp.x<min2.x ) min2.x = s2->to->prevcp.x;
    if ( s2->to->prevcp.y>max2.y ) max2.y = s2->to->prevcp.y;
    else if ( s2->to->prevcp.y<min2.y ) min2.y = s2->to->prevcp.y;
    if ( s2->to->me.x>max2.x ) max2.x = s2->to->me.x;
    else if ( s2->to->me.x<min2.x ) min2.x = s2->to->me.x;
    if ( s2->to->me.y>max2.y ) max2.y = s2->to->me.y;
    else if ( s2->to->me.y<min2.y ) min2.y = s2->to->me.y;
    if ( min1.x>max2.x || min2.x>max1.x || min1.y>max2.y || min2.y>max1.y )
return( false );		/* no intersection of bounding boxes */

    if ( s1->knownlinear ) {
	spline.d = s1->splines[1].c*((bigreal) s2->splines[0].d-(bigreal) s1->splines[0].d)-
		s1->splines[0].c*((bigreal) s2->splines[1].d-(bigreal) s1->splines[1].d);
	spline.c = s1->splines[1].c*(bigreal) s2->splines[0].c - s1->splines[0].c*(bigreal) s2->splines[1].c;
	spline.b = s1->splines[1].c*(bigreal) s2->splines[0].b - s1->splines[0].c*(bigreal) s2->splines[1].b;
	spline.a = s1->splines[1].c*(bigreal) s2->splines[0].a - s1->splines[0].c*(bigreal) s2->splines[1].a;
	IterateSolve(&spline,tempts);
	if ( tempts[0]==-1 )
return( false );
	for ( i = 0; i<3 && tempts[i]!=-1; ++i ) {
	    x = ((s2->splines[0].a*tempts[i]+s2->splines[0].b)*tempts[i]+
		    s2->splines[0].c)*tempts[i]+s2->splines[0].d;
	    y = ((s2->splines[1].a*tempts[i]+s2->splines[1].b)*tempts[i]+
		    s2->splines[1].c)*tempts[i]+s2->splines[1].d;
	    if ( s1->splines[0].c==0 )
		x = s1->splines[0].d;
	    if ( s1->splines[1].c==0 )
		y = s1->splines[1].d;
	    if ( (ac0 = s1->splines[0].c)<0 ) ac0 = -ac0;
	    if ( (ac1 = s1->splines[1].c)<0 ) ac1 = -ac1;
	    if ( ac0>ac1 )
		t = (x-s1->splines[0].d)/s1->splines[0].c;
	    else
		t = (y-s1->splines[1].d)/s1->splines[1].c;
	    if ( tempts[i]>.99996 && Closer(s1,s2,t,tempts[i],t,1)) {
		tempts[i] = 1;
		x = s2->to->me.x; y = s2->to->me.y;
	    } else if ( tempts[i]<.00001 && Closer(s1,s2,t,tempts[i],t,0)) {
		tempts[i] = 0;
		x = s2->from->me.x; y = s2->from->me.y;
	    }
	    /* I know we just did this, but we might have changed x,y so redo */
	    if ( ac0>ac1 )
		t = (x-s1->splines[0].d)/s1->splines[0].c;
	    else
		t = (y-s1->splines[1].d)/s1->splines[1].c;
	    if ( t>.99996 && t<1.001 && Closer(s1,s2,t,tempts[i],1,tempts[i])) {
		t = 1;
		x = s1->to->me.x; y = s1->to->me.y;
	    } else if ( t<.00001 && t>-.001 && Closer(s1,s2,t,tempts[i],0,tempts[i])) {
		t = 0;
		x = s1->from->me.x; y = s1->from->me.y;
	    }
	    if ( t<-.001 || t>1.001 || x<min1.x-.01 || y<min1.y-.01 || x>max1.x+.01 || y>max1.y+.01 )
	continue;
	    if ( t<=0 ) {t=0; x=s1->from->me.x; y = s1->from->me.y; }
	    else if ( t>=1 ) { t=1; x=s1->to->me.x; y = s1->to->me.y; }
	    if ( s1->from->me.x==s1->to->me.x )		/* Avoid rounding errors */
		x = s1->from->me.x;			/* on hor/vert lines */
	    else if ( s1->from->me.y==s1->to->me.y )
		y = s1->from->me.y;
	    if ( s2->knownlinear ) {
		if ( s2->from->me.x==s2->to->me.x )
		    x = s2->from->me.x;
		else if ( s2->from->me.y==s2->to->me.y )
		    y = s2->from->me.y;
	    }
	    soln = AddPoint(x,y,t,tempts[i],pts,t1s,t2s,soln);
	}
return( soln!=0 );
    }
    /* if one of the splines is quadratic then we can get an expression */
    /*  relating c*t+d to poly(s^3), and substituting this back we get */
    /*  a poly of degree 6 in s which could be solved iteratively */
    /* however mixed quadratics and cubics are unlikely */

    /* but if both splines are degree 3, the t is expressed as the sqrt of */
    /*  a third degree poly, which must be substituted into a cubic, and */
    /*  then squared to get rid of the sqrts leaving us with an ?18? degree */
    /*  poly. Ick. */

    /* So let's do it the hard way... we break the splines into little bits */
    /*  where they are monotonic in both dimensions, then check these for */
    /*  possible intersections */
    extrema1[0] = extrema2[0] = 0;
    ecnt1 = Spline2DFindExtrema(s1,extrema1+1);
    ecnt2 = Spline2DFindExtrema(s2,extrema2+1);
    extrema1[++ecnt1] = 1.0;
    extrema2[++ecnt2] = 1.0;
    found=0;
    for ( i=0; i<ecnt1; ++i ) {
	min1.x = ((s1->splines[0].a*extrema1[i]+s1->splines[0].b)*extrema1[i]+
		s1->splines[0].c)*extrema1[i]+s1->splines[0].d;
	min1.y = ((s1->splines[1].a*extrema1[i]+s1->splines[1].b)*extrema1[i]+
		s1->splines[1].c)*extrema1[i]+s1->splines[1].d;
	max1.x = ((s1->splines[0].a*extrema1[i+1]+s1->splines[0].b)*extrema1[i+1]+
		s1->splines[0].c)*extrema1[i+1]+s1->splines[0].d;
	max1.y = ((s1->splines[1].a*extrema1[i+1]+s1->splines[1].b)*extrema1[i+1]+
		s1->splines[1].c)*extrema1[i+1]+s1->splines[1].d;
	if ( max1.x<min1.x ) { extended temp = max1.x; max1.x = min1.x; min1.x = temp; }
	if ( max1.y<min1.y ) { extended temp = max1.y; max1.y = min1.y; min1.y = temp; }
	for ( j=(s1==s2)?i+1:0; j<ecnt2; ++j ) {
	    min2.x = ((s2->splines[0].a*extrema2[j]+s2->splines[0].b)*extrema2[j]+
		    s2->splines[0].c)*extrema2[j]+s2->splines[0].d;
	    min2.y = ((s2->splines[1].a*extrema2[j]+s2->splines[1].b)*extrema2[j]+
		    s2->splines[1].c)*extrema2[j]+s2->splines[1].d;
	    max2.x = ((s2->splines[0].a*extrema2[j+1]+s2->splines[0].b)*extrema2[j+1]+
		    s2->splines[0].c)*extrema2[j+1]+s2->splines[0].d;
	    max2.y = ((s2->splines[1].a*extrema2[j+1]+s2->splines[1].b)*extrema2[j+1]+
		    s2->splines[1].c)*extrema2[j+1]+s2->splines[1].d;
	    if ( max2.x<min2.x ) { extended temp = max2.x; max2.x = min2.x; min2.x = temp; }
	    if ( max2.y<min2.y ) { extended temp = max2.y; max2.y = min2.y; min2.y = temp; }
	    if ( min1.x>max2.x || min2.x>max1.x || min1.y>max2.y || min2.y>max1.y )
		/* No possible intersection */;
	    else if ( s1!=s2 )
		found += CubicsIntersect(s1,extrema1[i],extrema1[i+1],&min1,&max1,
				    s2,extrema2[j],extrema2[j+1],&min2,&max2,
				    &pts[found],&t1s[found],&t2s[found],9-found);
	    else {
		int k,l;
		int cnt = CubicsIntersect(s1,extrema1[i],extrema1[i+1],&min1,&max1,
				    s2,extrema2[j],extrema2[j+1],&min2,&max2,
				    &pts[found],&t1s[found],&t2s[found],9-found);
		for ( k=0; k<cnt; ++k ) {
		    if ( RealNear(t1s[found+k],t2s[found+k]) ) {
			for ( l=k+1; l<cnt; ++l ) {
			    pts[found+l-1] = pts[found+l];
			    t1s[found+l-1] = t1s[found+l];
			    t2s[found+l-1] = t2s[found+l];
			}
			--cnt; --k;
		    }
		}
		found += cnt;
	    }
	    if ( found>=8 ) {
		/* If the splines are colinear then we might get an unbounded */
		/*  number of intersections */
	break;
	    }
	}
    }
    t1s[found] = t2s[found] = -1;
return( found!=0 );
}

void StemInfoFree(StemInfo *h) {
    HintInstance *hi, *n;

    for ( hi=h->where; hi!=NULL; hi=n ) {
	n = hi->next;
	chunkfree(hi,sizeof(HintInstance));
    }
    chunkfree(h,sizeof(StemInfo));
}

void StemInfosFree(StemInfo *h) {
    StemInfo *hnext;
    HintInstance *hi, *n;

    for ( ; h!=NULL; h = hnext ) {
	for ( hi=h->where; hi!=NULL; hi=n ) {
	    n = hi->next;
	    chunkfree(hi,sizeof(HintInstance));
	}
	hnext = h->next;
	chunkfree(h,sizeof(StemInfo));
    }
}

void DStemInfoFree(DStemInfo *h) {
    HintInstance *hi, *n;

    for ( hi=h->where; hi!=NULL; hi=n ) {
	n = hi->next;
	chunkfree(hi,sizeof(HintInstance));
    }
    chunkfree(h,sizeof(DStemInfo));
}

void DStemInfosFree(DStemInfo *h) {
    DStemInfo *hnext;
    HintInstance *hi, *n;

    for ( ; h!=NULL; h = hnext ) {
	for ( hi=h->where; hi!=NULL; hi=n ) {
	    n = hi->next;
	    chunkfree(hi,sizeof(HintInstance));
	}
	hnext = h->next;
	chunkfree(h,sizeof(DStemInfo));
    }
}

void KernPairsFree(KernPair *kp) {
    KernPair *knext;
    for ( ; kp!=NULL; kp = knext ) {
	knext = kp->next;
	if ( kp->adjust!=NULL ) {
	    free(kp->adjust->corrections);
	    chunkfree(kp->adjust,sizeof(DeviceTable));
	}
	chunkfree(kp,sizeof(KernPair));
    }
}

void AnchorPointsFree(AnchorPoint *ap) {
    AnchorPoint *anext;
    for ( ; ap!=NULL; ap = anext ) {
	anext = ap->next;
	free(ap->xadjust.corrections);
	free(ap->yadjust.corrections);
	chunkfree(ap,sizeof(AnchorPoint));
    }
}

void ValDevFree(ValDevTab *adjust) {
    if ( adjust==NULL )
return;
    free( adjust->xadjust.corrections );
    free( adjust->yadjust.corrections );
    free( adjust->xadv.corrections );
    free( adjust->yadv.corrections );
    chunkfree(adjust,sizeof(ValDevTab));
}

void DeviceTableFree(DeviceTable *dt) {

    if ( dt==NULL )
return;

    free(dt->corrections);
    chunkfree(dt,sizeof(DeviceTable));
}

void PSTFree(PST *pst) {
    PST *pnext;
    for ( ; pst!=NULL; pst = pnext ) {
	pnext = pst->next;
	if ( pst->type==pst_lcaret )
	    free(pst->u.lcaret.carets);
	else if ( pst->type==pst_pair ) {
	    free(pst->u.pair.paired);
	    ValDevFree(pst->u.pair.vr[0].adjust);
	    ValDevFree(pst->u.pair.vr[1].adjust);
	    chunkfree(pst->u.pair.vr,sizeof(struct vr [2]));
	} else if ( pst->type!=pst_position ) {
	    free(pst->u.subs.variant);
	} else if ( pst->type==pst_position ) {
	    ValDevFree(pst->u.pos.adjust);
	}
	chunkfree(pst,sizeof(PST));
    }
}

void FPSTRuleContentsFree(struct fpst_rule *r, enum fpossub_format format) {
    int j;

    switch ( format ) {
    case pst_glyphs:
    	free(r->u.glyph.names);
    	free(r->u.glyph.back);
    	free(r->u.glyph.fore);
    	break;
    case pst_class:
    	free(r->u.class.nclasses);
    	free(r->u.class.bclasses);
    	free(r->u.class.fclasses);
    	break;
    case pst_reversecoverage:
    	free(r->u.rcoverage.replacements);
    case pst_coverage:
    	for ( j=0 ; j<r->u.coverage.ncnt ; ++j )
    		free(r->u.coverage.ncovers[j]);
    	free(r->u.coverage.ncovers);
    	for ( j=0 ; j<r->u.coverage.bcnt ; ++j )
    		free(r->u.coverage.bcovers[j]);
    	free(r->u.coverage.bcovers);
    	for ( j=0 ; j<r->u.coverage.fcnt ; ++j )
    		free(r->u.coverage.fcovers[j]);
    	free(r->u.coverage.fcovers);
    	break;
    default:;
    }
    free(r->lookups);
}

void FPSTClassesFree(FPST *fpst) {
    int i;

    for ( i=0; i<fpst->nccnt; ++i ) {
	free(fpst->nclass[i]);
	free(fpst->nclassnames[i]);
    }
    for ( i=0; i<fpst->bccnt; ++i ) {
	free(fpst->bclass[i]);
	free(fpst->bclassnames[i]);
    }
    for ( i=0; i<fpst->fccnt; ++i ) {
	free(fpst->fclass[i]);
	free(fpst->fclassnames[i]);
    }
    free(fpst->nclass); free(fpst->bclass); free(fpst->fclass);
    free(fpst->nclassnames); free(fpst->bclassnames); free(fpst->fclassnames);

    fpst->nccnt = fpst->bccnt = fpst->fccnt = 0;
    fpst->nclass = fpst->bclass = fpst->fclass = NULL;
    fpst->nclassnames = fpst->bclassnames = fpst->fclassnames = NULL;
}

void FPSTFree(FPST *fpst) {
    FPST *next;
    int i;

    while ( fpst!=NULL ) {
	next = fpst->next;
	FPSTClassesFree(fpst);
	for ( i=0; i<fpst->rule_cnt; ++i ) {
	    FPSTRuleContentsFree( &fpst->rules[i],fpst->format );
	}
	free(fpst->rules);
	chunkfree(fpst,sizeof(FPST));
	fpst = next;
    }
}

void MinimumDistancesFree(MinimumDistance *md) {
    MinimumDistance *next;

    while ( md!=NULL ) {
	next = md->next;
	chunkfree(md,sizeof(MinimumDistance));
	md = next;
    }
}

void TTFLangNamesFree(struct ttflangname *l) {
    struct ttflangname *next;
    int i;

    while ( l!=NULL ) {
	next = l->next;
	for ( i=0; i<ttf_namemax; ++i )
	    free(l->names[i]);
	chunkfree(l,sizeof(*l));
	l = next;
    }
}

void AltUniFree(struct altuni *altuni) {
    struct altuni *next;

    while ( altuni ) {
	next = altuni->next;
	chunkfree(altuni,sizeof(struct altuni));
	altuni = next;
    }
}

void LayerDefault(Layer *layer) {
    memset(layer,0,sizeof(Layer));
    layer->fill_brush.opacity = layer->stroke_pen.brush.opacity = 1.0;
    layer->fill_brush.col = layer->stroke_pen.brush.col = COLOR_INHERITED;
    layer->stroke_pen.width = 10;
    layer->stroke_pen.linecap = lc_round;
    layer->stroke_pen.linejoin = lj_round;
    layer->dofill = true;
    layer->fillfirst = true;
    layer->stroke_pen.trans[0] = layer->stroke_pen.trans[3] = 1.0;
    layer->stroke_pen.trans[1] = layer->stroke_pen.trans[2] = 0.0;
    /* Dashes default to an unbroken line */
}

SplineChar *SplineCharCreate(int layer_cnt) {
    SplineChar *sc = chunkalloc(sizeof(SplineChar));
    int i;

    sc->color = COLOR_DEFAULT;
    sc->orig_pos = 0xffff;
    sc->unicodeenc = -1;
    sc->layer_cnt = layer_cnt;
    sc->layers = calloc(layer_cnt,sizeof(Layer));
    for ( i=0; i<layer_cnt; ++i )
	LayerDefault(&sc->layers[i]);
    sc->tex_height = sc->tex_depth = sc->italic_correction = sc->top_accent_horiz =
	    TEX_UNDEF;
return( sc );
}

SplineChar *SFSplineCharCreate(SplineFont *sf) {
    SplineChar *sc = SplineCharCreate(sf==NULL?2:sf->layer_cnt);
    int i;

    if ( sf==NULL ) {
	sc->layers[ly_back].background = true;
	sc->layers[ly_fore].background = false;
    } else {
	for ( i=0; i<sf->layer_cnt; ++i ) {
	    sc->layers[i].background = sf->layers[i].background;
	    sc->layers[i].order2     = sf->layers[i].order2;
	}
	sc->parent = sf;
    }
return( sc );
}

void GlyphVariantsFree(struct glyphvariants *gv) {
    int i;

    if ( gv==NULL )
return;
    free(gv->variants);
    DeviceTableFree(gv->italic_adjusts);
    for ( i=0; i<gv->part_cnt; ++i )
	free( gv->parts[i].component );
    free(gv->parts);
    chunkfree(gv,sizeof(*gv));
}

void MathKernVContentsFree(struct mathkernvertex *mk) {
    int i;
    for ( i=0; i<mk->cnt; ++i ) {
	DeviceTableFree(mk->mkd[i].height_adjusts);
	DeviceTableFree(mk->mkd[i].kern_adjusts);
    }
    free(mk->mkd);
}

void MathKernFree(struct mathkern *mk) {
    int i;

    if ( mk==NULL )
return;
    for ( i=0; i<4; ++i )
	MathKernVContentsFree( &(&mk->top_right)[i] );
    chunkfree(mk,sizeof(*mk));
}

void SplineCharListsFree(struct splinecharlist *dlist) {
    struct splinecharlist *dnext;
    for ( ; dlist!=NULL; dlist = dnext ) {
	dnext = dlist->next;
	chunkfree(dlist,sizeof(struct splinecharlist));
    }
}

struct pattern *PatternCopy(struct pattern *old, real transform[6]) {
    struct pattern *pat;

    if ( old==NULL )
return( NULL );

    pat = chunkalloc(sizeof(struct pattern));

    *pat = *old;
    pat->pattern = copy( old->pattern );
    if ( transform!=NULL )
	MatMultiply(pat->transform,transform,pat->transform);
return( pat );
}

void PatternFree(struct pattern *pat) {
    if ( pat==NULL )
return;
    free(pat->pattern);
    chunkfree(pat,sizeof(struct pattern));
}

struct gradient *GradientCopy(struct gradient *old,real transform[6]) {
    struct gradient *grad;

    if ( old==NULL )
return( NULL );

    grad = chunkalloc(sizeof(struct gradient));

    *grad = *old;
    grad->grad_stops = malloc(old->stop_cnt*sizeof(struct grad_stops));
    memcpy(grad->grad_stops,old->grad_stops,old->stop_cnt*sizeof(struct grad_stops));
    if ( transform!=NULL ) {
	BpTransform(&grad->start,&grad->start,transform);
	BpTransform(&grad->stop,&grad->stop,transform);
    }
return( grad );
}

void GradientFree(struct gradient *grad) {
    if ( grad==NULL )
return;
    free(grad->grad_stops);
    chunkfree(grad,sizeof(struct gradient));
}

void BrushCopy(struct brush *into, struct brush *from, real transform[6]) {
    *into = *from;
    into->gradient = GradientCopy(from->gradient,transform);
    into->pattern = PatternCopy(from->pattern,transform);
}

void PenCopy(struct pen *into, struct pen *from,real transform[6]) {
    *into = *from;
    into->brush.gradient = GradientCopy(from->brush.gradient,transform);
    into->brush.pattern = PatternCopy(from->brush.pattern,transform);
}

void LayerFreeContents(SplineChar *sc,int layer) {
    SplinePointListsFree(sc->layers[layer].splines);
    GradientFree(sc->layers[layer].fill_brush.gradient);
    PatternFree(sc->layers[layer].fill_brush.pattern);
    GradientFree(sc->layers[layer].stroke_pen.brush.gradient);
    PatternFree(sc->layers[layer].stroke_pen.brush.pattern);
    RefCharsFree(sc->layers[layer].refs);
    /* image garbage collection????!!!! */
}

void SplineCharFreeContents(SplineChar *sc) {
    int i;

    if ( sc==NULL )
return;
    if (sc->name != NULL) free(sc->name);
    if (sc->comment != NULL) free(sc->comment);
    for ( i=0; i<sc->layer_cnt; ++i ) {
#if defined(_NO_PYTHON)
        if (sc->layers[i].python_persistent != NULL) free( sc->layers[i].python_persistent );	/* It's a string of pickled data which we leave as a string */
#else
        PyFF_FreeSCLayer(sc, i);
#endif
	LayerFreeContents(sc,i);
    }
    StemInfosFree(sc->hstem);
    StemInfosFree(sc->vstem);
    DStemInfosFree(sc->dstem);
    MinimumDistancesFree(sc->md);
    KernPairsFree(sc->kerns);
    KernPairsFree(sc->vkerns);
    AnchorPointsFree(sc->anchor);
    SplineCharListsFree(sc->dependents);
    PSTFree(sc->possub);
    if (sc->ttf_instrs != NULL) free(sc->ttf_instrs);
    if (sc->countermasks != NULL) free(sc->countermasks);
    if (sc->layers != NULL) free(sc->layers);
    AltUniFree(sc->altuni);
    GlyphVariantsFree(sc->horiz_variants);
    GlyphVariantsFree(sc->vert_variants);
    DeviceTableFree(sc->italic_adjusts);
    DeviceTableFree(sc->top_accent_adjusts);
    MathKernFree(sc->mathkern);
    if (sc->glif_name != NULL) { free(sc->glif_name); sc->glif_name = NULL; }
}

void SplineCharFree(SplineChar *sc) {

    if ( sc==NULL )
return;
    SplineCharFreeContents(sc);
    chunkfree(sc,sizeof(SplineChar));
}

void AnchorClassesFree(AnchorClass *an) {
    AnchorClass *anext;
    for ( ; an!=NULL; an = anext ) {
	anext = an->next;
	free(an->name);
	chunkfree(an,sizeof(AnchorClass));
    }
}

void TtfTablesFree(struct ttf_table *tab) {
    struct ttf_table *next;

    for ( ; tab!=NULL; tab = next ) {
	next = tab->next;
	free(tab->data);
	chunkfree(tab,sizeof(struct ttf_table));
    }
}

void ScriptLangListFree(struct scriptlanglist *sl) {
    struct scriptlanglist *next;

    while ( sl!=NULL ) {
	next = sl->next;
	free(sl->morelangs);
	chunkfree(sl,sizeof(*sl));
	sl = next;
    }
}

void FeatureScriptLangListFree(FeatureScriptLangList *fl) {
    FeatureScriptLangList *next;

    while ( fl!=NULL ) {
	next = fl->next;
	ScriptLangListFree(fl->scripts);
	chunkfree(fl,sizeof(*fl));
	fl = next;
    }
}

void OTLookupFree(OTLookup *lookup) {
    struct lookup_subtable *st, *stnext;

    free(lookup->lookup_name);
    FeatureScriptLangListFree(lookup->features);
    for ( st=lookup->subtables; st!=NULL; st=stnext ) {
	stnext = st->next;
	free(st->subtable_name);
	free(st->suffix);
	chunkfree(st,sizeof(struct lookup_subtable));
    }
    chunkfree( lookup,sizeof(OTLookup) );
}

void OTLookupListFree(OTLookup *lookup ) {
    OTLookup *next;

    for ( ; lookup!=NULL; lookup = next ) {
	next = lookup->next;
	OTLookupFree(lookup);
    }
}

void KernClassFreeContents(KernClass *kc) {
    int i;
    for ( i=1; i<kc->first_cnt; ++i )
	free(kc->firsts[i]);
    for ( i=1; i<kc->second_cnt; ++i )
	free(kc->seconds[i]);
    free(kc->firsts);
    free(kc->seconds);
    free(kc->offsets);
    for ( i=kc->first_cnt*kc->second_cnt-1; i>=0 ; --i )
	free(kc->adjusts[i].corrections);
    free(kc->adjusts);
    if (kc->firsts_flags) free(kc->firsts_flags);
    if (kc->seconds_flags) free(kc->seconds_flags);
    if (kc->offsets_flags) free(kc->offsets_flags);
    if (kc->firsts_names) {
      for ( i=kc->first_cnt-1; i>=0 ; --i )
	free(kc->firsts_names[i]);
      free(kc->firsts_names);
    }
    if (kc->seconds_names) {
      for ( i=kc->second_cnt-1; i>=0 ; --i )
	free(kc->seconds_names[i]);
      free(kc->seconds_names);
    }
}

void KernClassClearSpecialContents(KernClass *kc) {
    // This frees and zeros special data not handled by the FontForge GUI,
    // most of which comes from U. F. O..
    int i;
    if (kc->firsts_flags) { free(kc->firsts_flags); kc->firsts_flags = NULL; }
    if (kc->seconds_flags) { free(kc->seconds_flags); kc->seconds_flags = NULL; }
    if (kc->offsets_flags) { free(kc->offsets_flags); kc->offsets_flags = NULL; }
    if (kc->firsts_names) {
      for ( i=kc->first_cnt-1; i>=0 ; --i )
	free(kc->firsts_names[i]);
      free(kc->firsts_names);
      kc->firsts_names = NULL;
    }
    if (kc->seconds_names) {
      for ( i=kc->second_cnt-1; i>=0 ; --i )
	free(kc->seconds_names[i]);
      free(kc->seconds_names);
      kc->seconds_names = NULL;
    }
}

void KernClassListFree(KernClass *kc) {
    KernClass *n;

    while ( kc ) {
	KernClassFreeContents(kc);
	n = kc->next;
	chunkfree(kc,sizeof(KernClass));
	kc = n;
    }
}

void KernClassListClearSpecialContents(KernClass *kc) {
    KernClass *n;

    while ( kc ) {
	KernClassClearSpecialContents(kc);
	n = kc->next;
	kc = n;
    }
}

void MacNameListFree(struct macname *mn) {
    struct macname *next;

    while ( mn!=NULL ) {
	next = mn->next;
	free(mn->name);
	chunkfree(mn,sizeof(struct macname));
	mn = next;
    }
}

void MacSettingListFree(struct macsetting *ms) {
    struct macsetting *next;

    while ( ms!=NULL ) {
	next = ms->next;
	MacNameListFree(ms->setname);
	chunkfree(ms,sizeof(struct macsetting));
	ms = next;
    }
}

void MacFeatListFree(MacFeat *mf) {
    MacFeat *next;

    while ( mf!=NULL ) {
	next = mf->next;
	MacNameListFree(mf->featname);
	MacSettingListFree(mf->settings);
	chunkfree(mf,sizeof(MacFeat));
	mf = next;
    }
}

void ASMFree(ASM *sm) {
    ASM *next;
    int i;

    while ( sm!=NULL ) {
	next = sm->next;
	if ( sm->type==asm_insert ) {
	    for ( i=0; i<sm->class_cnt*sm->state_cnt; ++i ) {
		free( sm->state[i].u.insert.mark_ins );
		free( sm->state[i].u.insert.cur_ins );
	    }
	} else if ( sm->type==asm_kern ) {
	    for ( i=0; i<sm->class_cnt*sm->state_cnt; ++i ) {
		free( sm->state[i].u.kern.kerns );
	    }
	}
	for ( i=4; i<sm->class_cnt; ++i )
	    free(sm->classes[i]);
	free(sm->state);
	free(sm->classes);
	chunkfree(sm,sizeof(ASM));
	sm = next;
    }
}

void OtfNameListFree(struct otfname *on) {
    struct otfname *on_next;

    for ( ; on!=NULL; on = on_next ) {
	on_next = on->next;
	free(on->name);
	chunkfree(on,sizeof(*on));
    }
}

void OtfFeatNameListFree(struct otffeatname *fn) {
    struct otffeatname *fn_next;

    for ( ; fn!=NULL; fn = fn_next ) {
	fn_next = fn->next;
	OtfNameListFree(fn->names);
	chunkfree(fn,sizeof(*fn));
    }
}

EncMap *EncMapNew(int enccount,int backmax,Encoding *enc) {
/* NOTE: 'enccount' and 'backmax' can sometimes be different map sizes */
    EncMap *map;

    /* Ensure all memory available, otherwise cleanup and exit as NULL */
    if ( (map=chunkalloc(sizeof(EncMap)))!=NULL ) {
	if ( (map->map=malloc(enccount*sizeof(int32)))!=NULL ) {
	    if ( (map->backmap=malloc(backmax*sizeof(int32)))!=NULL ) {
		map->enccount = map->encmax = enccount;
		map->backmax = backmax;
		memset(map->map,-1,enccount*sizeof(int32));
		memset(map->backmap,-1,backmax*sizeof(int32));
		map->enc = enc;
		return( map );
	    }
	    free(map->map);
	}
	free(map);
    }
    return( NULL );
}

EncMap *EncMap1to1(int enccount) {
/* Used for CID fonts where CID is same as orig_pos */
/* NOTE: map-enc point to a global variable custom. */
/* TODO: avoid global custom and use passed pointer */
    EncMap *map;
    int i;

    if ( (map=EncMapNew(enccount,enccount,&custom))!=NULL ) {
	for ( i=0; i<enccount; ++i )
	    map->map[i] = map->backmap[i] = i;
    }
    return( map );
}

void EncMapFree(EncMap *map) {
    if ( map==NULL )
return;

    if ( map->enc->is_temporary )
	EncodingFree(map->enc);
    free(map->map);
    free(map->backmap);
    free(map->remap);
    chunkfree(map,sizeof(EncMap));
}

void MarkClassFree(int cnt,char **classes,char **names) {
    int i;

    for ( i=1; i<cnt; ++i ) {
	free( classes[i] );
	free( names[i] );
    }
    free( classes );
    free( names );
}

void MarkSetFree(int cnt,char **classes,char **names) {
    int i;

    for ( i=0; i<cnt; ++i ) {
	free( classes[i] );
	free( names[i] );
    }
    free( classes );
    free( names );
}

struct baselangextent *BaseLangCopy(struct baselangextent *extent) {
    struct baselangextent *head, *last, *cur;

    last = head = NULL;
    for ( ; extent!=NULL; extent = extent->next ) {
	cur = chunkalloc(sizeof(struct baselangextent));
	*cur = *extent;
	cur->features = BaseLangCopy(cur->features);
	if ( head==NULL )
	    head = cur;
	else
	    last->next = cur;
	last = cur;
    }
return( head );
}

void BaseLangFree(struct baselangextent *extent) {
    struct baselangextent *next;

    while ( extent!=NULL ) {
	next = extent->next;
	BaseLangFree(extent->features);
	chunkfree(extent,sizeof(struct baselangextent));
	extent = next;
    }
}

void BaseScriptFree(struct basescript *bs) {
    struct basescript *next;

    while ( bs!=NULL ) {
	next = bs->next;
	if ( bs->baseline_pos )
	    free(bs->baseline_pos);
	BaseLangFree(bs->langs);
	chunkfree(bs,sizeof(struct basescript));
	bs = next;
    }
}

void BaseFree(struct Base *base) {
    if ( base==NULL )
return;

    free(base->baseline_tags);
    BaseScriptFree(base->scripts);
    chunkfree(base,sizeof(struct Base));
}

void JstfLangFree(struct jstf_lang *jl) {
    struct jstf_lang *next;
    int i;

    while ( jl!=NULL ) {
	next = jl->next;
	for ( i=0; i<jl->cnt; ++i ) {
	    struct jstf_prio *jp = &jl->prios[i];
	    free(jp->enableShrink);
	    free(jp->disableShrink);
	    free(jp->maxShrink);
	    free(jp->enableExtend);
	    free(jp->disableExtend);
	    free(jp->maxExtend);
	}
	free(jl->prios);
	chunkfree(jl,sizeof(*jl));
	jl = next;
    }
}

void JustifyFree(Justify *just) {
    Justify *next;

    while ( just!=NULL ) {
	next = just->next;
	free(just->extenders);
	JstfLangFree(just->langs);
	chunkfree(just,sizeof(*just));
	just = next;
    }
}

void SplineFontFree(SplineFont *sf) {
    int i;

    if ( sf==NULL )
return;
    if ( sf->mm!=NULL ) {
	MMSetFree(sf->mm);
return;
    }
    if ( sf->sfd_version>0 && sf->sfd_version<2 ) {
      // Free special data.
      SplineFont1* oldsf = (SplineFont1*)sf;
      // First the script language lists.
      if (oldsf->script_lang != NULL) {
        int scripti;
        for (scripti = 0; oldsf->script_lang[scripti] != NULL; scripti ++) {
          int scriptj;
          for (scriptj = 0; oldsf->script_lang[scripti][scriptj].script != 0; scriptj ++) {
            if (oldsf->script_lang[scripti][scriptj].langs != NULL) free(oldsf->script_lang[scripti][scriptj].langs);
          }
          free(oldsf->script_lang[scripti]); oldsf->script_lang[scripti] = NULL;
        }
        free(oldsf->script_lang); oldsf->script_lang = NULL;
      }
      // Then the table orderings.
      {
        struct table_ordering *ord = oldsf->orders;
        while (ord != NULL) {
          struct table_ordering *ordtofree = ord;
          if (ord->ordered_features != NULL) free(ord->ordered_features);
          ord = ord->next;
          chunkfree(ordtofree, sizeof(struct table_ordering));
        }
        oldsf->orders = NULL;
      }
    }
    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL )
	SplineCharFree(sf->glyphs[i]);
    free(sf->glyphs);
    free(sf->fontname);
    free(sf->fullname);
    free(sf->familyname);
    free(sf->weight);
    free(sf->copyright);
    free(sf->comments);
    free(sf->filename);
    free(sf->origname);
    free(sf->autosavename);
    free(sf->version);
    free(sf->xuid);
    free(sf->cidregistry);
    free(sf->ordering);
    if (sf->map != 0)
   	 EncMapFree(sf->map);
    if (sf->MATH)
   	 free(sf->MATH);
    if ( sf->styleMapFamilyName && sf->styleMapFamilyName[0]!='\0' ) { free(sf->styleMapFamilyName); sf->styleMapFamilyName = NULL; }
    MacFeatListFree(sf->features);
    /* We don't free the EncMap. That field is only a temporary pointer. Let the FontViewBase free it, that's where it really lives */
    // TODO: But that doesn't always get freed. The statement below causes double-frees, so we need to come up with better conditions.
    #if 0
    if (sf->cidmaster == NULL || sf->cidmaster == sf)
      if (sf->map != NULL) { free(sf->map); sf->map = NULL; }
    #endif // 0
    SplinePointListsFree(sf->grid.splines);
    AnchorClassesFree(sf->anchor);
    TtfTablesFree(sf->ttf_tables);
    TtfTablesFree(sf->ttf_tab_saved);
    PSDictFree(sf->private);
    TTFLangNamesFree(sf->names);
    for ( i=0; i<sf->subfontcnt; ++i )
	SplineFontFree(sf->subfonts[i]);
    free(sf->subfonts);
    GlyphHashFree(sf);
    OTLookupListFree(sf->gpos_lookups);
    OTLookupListFree(sf->gsub_lookups);
    KernClassListFree(sf->kerns);
    KernClassListFree(sf->vkerns);
    FPSTFree(sf->possub);
    ASMFree(sf->sm);
    OtfNameListFree(sf->fontstyle_name);
    OtfFeatNameListFree(sf->feat_names);
    MarkClassFree(sf->mark_class_cnt,sf->mark_classes,sf->mark_class_names);
    MarkSetFree(sf->mark_set_cnt,sf->mark_sets,sf->mark_set_names);
    GlyphGroupsFree(sf->groups);
    GlyphGroupKernsFree(sf->groupkerns);
    GlyphGroupKernsFree(sf->groupvkerns);
    free( sf->gasp );
#if defined(_NO_PYTHON)
    free( sf->python_persistent );	/* It's a string of pickled data which we leave as a string */
#else
    PyFF_FreeSF(sf);
#endif
    BaseFree(sf->horiz_base);
    BaseFree(sf->vert_base);
    JustifyFree(sf->justify);
    if (sf->layers != NULL) {
      int layer;
      for (layer = 0; layer < sf->layer_cnt; layer ++) {
        if (sf->layers[layer].name != NULL) {
          free(sf->layers[layer].name);
          sf->layers[layer].name = NULL;
        }
        if (sf->layers[layer].ufo_path != NULL) {
          free(sf->layers[layer].ufo_path);
          sf->layers[layer].ufo_path = NULL;
        }
      }
      free(sf->layers); sf->layers = NULL;
    }
    free(sf);
}

void SplineFontClearSpecial(SplineFont *sf) {
    int i;

    if ( sf==NULL )
return;
    if ( sf->mm!=NULL ) {
	MMSetClearSpecial(sf->mm);
return;
    }
    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL ) {
	struct splinechar *sc = sf->glyphs[i];
	if (sc->glif_name != NULL) { free(sc->glif_name); sc->glif_name = NULL; }
    }
    for ( i=0; i<sf->subfontcnt; ++i )
	SplineFontClearSpecial(sf->subfonts[i]);
    KernClassListClearSpecialContents(sf->kerns);
    KernClassListClearSpecialContents(sf->vkerns);
    if (sf->groups) { GlyphGroupsFree(sf->groups); sf->groups = NULL; }
    if (sf->groupkerns) { GlyphGroupKernsFree(sf->groupkerns); sf->groupkerns = NULL; }
    if (sf->groupvkerns) { GlyphGroupKernsFree(sf->groupvkerns); sf->groupvkerns = NULL; }
    if (sf->python_persistent) {
#if defined(_NO_PYTHON)
      free( sf->python_persistent );	/* It's a string of pickled data which we leave as a string */
#else
      PyFF_FreeSF(sf);
#endif
      sf->python_persistent = NULL;
    }
    if (sf->layers != NULL) {
      int layer;
      for (layer = 0; layer < sf->layer_cnt; layer ++) {
        if (sf->layers[layer].ufo_path != NULL) {
          free(sf->layers[layer].ufo_path);
          sf->layers[layer].ufo_path = NULL;
        }
      }
    }
}

#if 0
// These are in splinefont.h.
#define GROUP_NAME_KERNING_UFO 1
#define GROUP_NAME_KERNING_FEATURE 2
#define GROUP_NAME_VERTICAL 4 // Otherwise horizontal.
#define GROUP_NAME_RIGHT 8 // Otherwise left (or above).
#endif // 0


void GlyphGroupFree(struct ff_glyphclasses* group) {
  if (group->classname != NULL) free(group->classname);
  if (group->glyphs != NULL) free(group->glyphs);
  free(group);
}

void GlyphGroupsFree(struct ff_glyphclasses* root) {
  struct ff_glyphclasses* current = root;
  struct ff_glyphclasses* next;
  while (current != NULL) {
    next = current->next;
    GlyphGroupFree(current);
    current = next;
  }
}

void GlyphGroupKernFree(struct ff_rawoffsets* groupkern) {
  if (groupkern->left != NULL) free(groupkern->left);
  if (groupkern->right != NULL) free(groupkern->right);
  free(groupkern);
}

void GlyphGroupKernsFree(struct ff_rawoffsets* root) {
  struct ff_rawoffsets* current = root;
  struct ff_rawoffsets* next;
  while (current != NULL) {
    next = current->next;
    GlyphGroupKernFree(current);
    current = next;
  }
}


#ifdef FF_UTHASH_GLIF_NAMES
int HashKerningClassNamesFlex(SplineFont *sf, struct glif_name_index * class_name_hash, int capitalize) {
    struct kernclass *current_kernclass;
    int isv;
    int isr;
    int i;
    int absolute_index = 0; // This gives us a unique index for each kerning class.
    // First we catch the existing names.
    absolute_index = 0;
    for (isv = 0; isv < 2; isv++)
    for (current_kernclass = (isv ? sf->vkerns : sf->kerns); current_kernclass != NULL; current_kernclass = current_kernclass->next)
    for (isr = 0; isr < 2; isr++) if ( (isr ? current_kernclass->seconds_names : current_kernclass->firsts_names) != NULL ) {
    for ( i=0; i < (isr ? current_kernclass->second_cnt : current_kernclass->first_cnt); ++i )
    if ( (isr ? current_kernclass->seconds_names[i] : current_kernclass->firsts_names[i]) != NULL ) {
        // Add it to the hash table with its index.
	if (capitalize) {
          char * cap_name = upper_case(isr ? current_kernclass->seconds_names[i] : current_kernclass->firsts_names[i]);
          glif_name_track_new(class_name_hash, absolute_index + i, cap_name);
          free(cap_name); cap_name = NULL;
	} else {
          glif_name_track_new(class_name_hash, absolute_index + i, (isr ? current_kernclass->seconds_names[i] : current_kernclass->firsts_names[i]));
        }
    }
    absolute_index +=i;
    }
    return absolute_index;
}
int HashKerningClassNames(SplineFont *sf, struct glif_name_index * class_name_hash) {
  return HashKerningClassNamesFlex(sf, class_name_hash, 0);
}
int HashKerningClassNamesCaps(SplineFont *sf, struct glif_name_index * class_name_hash) {
  return HashKerningClassNamesFlex(sf, class_name_hash, 1);
}
#endif

void MMSetFreeContents(MMSet *mm) {
    int i;

    free(mm->instances);

    free(mm->positions);
    free(mm->defweights);

    for ( i=0; i<mm->axis_count; ++i ) {
	free(mm->axes[i]);
	free(mm->axismaps[i].blends);
	free(mm->axismaps[i].designs);
	MacNameListFree(mm->axismaps[i].axisnames);
    }
    free(mm->axismaps);
    free(mm->cdv);
    free(mm->ndv);
    for ( i=0; i<mm->named_instance_count; ++i ) {
	free(mm->named_instances[i].coords);
	MacNameListFree(mm->named_instances[i].names);
    }
    free(mm->named_instances);
}

void MMSetFree(MMSet *mm) {
    int i;

    for ( i=0; i<mm->instance_count; ++i ) {
	mm->instances[i]->mm = NULL;
	mm->instances[i]->map = NULL;
	SplineFontFree(mm->instances[i]);
    }
    mm->normal->mm = NULL;
    SplineFontFree(mm->normal);		/* EncMap gets freed here */
    MMSetFreeContents(mm);

    chunkfree(mm,sizeof(*mm));
}

void MMSetClearSpecial(MMSet *mm) {
    int i;

    for ( i=0; i<mm->instance_count; ++i ) {
	SplineFontClearSpecial(mm->instances[i]);
    }
    SplineFontClearSpecial(mm->normal);
}

SplinePoint *SplineBisect(Spline *spline, extended t) {
    Spline1 xstart, xend;
    Spline1 ystart, yend;
    Spline *spline1, *spline2;
    SplinePoint *mid;
    SplinePoint *old0, *old1;
    Spline1D *xsp = &spline->splines[0], *ysp = &spline->splines[1];
    int order2 = spline->order2;

#ifdef DEBUG
    if ( t<=1e-3 || t>=1-1e-3 )
	IError("Bisection to create a zero length spline");
#endif
    xstart.s0 = xsp->d; ystart.s0 = ysp->d;
    xend.s1 = (extended) xsp->a+xsp->b+xsp->c+xsp->d;
    yend.s1 = (extended) ysp->a+ysp->b+ysp->c+ysp->d;
    xstart.s1 = xend.s0 = ((xsp->a*t+xsp->b)*t+xsp->c)*t + xsp->d;
    ystart.s1 = yend.s0 = ((ysp->a*t+ysp->b)*t+ysp->c)*t + ysp->d;
    FigureSpline1(&xstart,0,t,xsp);
    FigureSpline1(&xend,t,1,xsp);
    FigureSpline1(&ystart,0,t,ysp);
    FigureSpline1(&yend,t,1,ysp);

    mid = chunkalloc(sizeof(SplinePoint));
    mid->me.x = xstart.s1;	mid->me.y = ystart.s1;
    if ( order2 ) {
	mid->nextcp.x = xend.sp.d + xend.sp.c/2;
	mid->nextcp.y = yend.sp.d + yend.sp.c/2;
	mid->prevcp.x = xstart.sp.d + xstart.sp.c/2;
	mid->prevcp.y = ystart.sp.d + ystart.sp.c/2;
    } else {
	mid->nextcp.x = xend.c0;	mid->nextcp.y = yend.c0;
	mid->prevcp.x = xstart.c1;	mid->prevcp.y = ystart.c1;
    }
    if ( mid->me.x==mid->nextcp.x && mid->me.y==mid->nextcp.y )
	mid->nonextcp = true;
    if ( mid->me.x==mid->prevcp.x && mid->me.y==mid->prevcp.y )
	mid->noprevcp = true;

    old0 = spline->from; old1 = spline->to;
    if ( order2 ) {
	old0->nextcp = mid->prevcp;
	old1->prevcp = mid->nextcp;
    } else {
	old0->nextcp.x = xstart.c0;	old0->nextcp.y = ystart.c0;
	old1->prevcp.x = xend.c1;	old1->prevcp.y = yend.c1;
    }
    old0->nonextcp = (old0->nextcp.x==old0->me.x && old0->nextcp.y==old0->me.y);
    old1->noprevcp = (old1->prevcp.x==old1->me.x && old1->prevcp.y==old1->me.y);
    old0->nextcpdef = false;
    old1->prevcpdef = false;
    SplineFree(spline);

    spline1 = chunkalloc(sizeof(Spline));
    spline1->splines[0] = xstart.sp;	spline1->splines[1] = ystart.sp;
    spline1->from = old0;
    spline1->to = mid;
    spline1->order2 = order2;
    old0->next = spline1;
    mid->prev = spline1;
    if ( SplineIsLinear(spline1)) {
	spline1->islinear = spline1->from->nonextcp = spline1->to->noprevcp = true;
	spline1->from->nextcp = spline1->from->me;
	spline1->to->prevcp = spline1->to->me;
    }
    SplineRefigure(spline1);

    spline2 = chunkalloc(sizeof(Spline));
    spline2->splines[0] = xend.sp;	spline2->splines[1] = xend.sp;
    spline2->from = mid;
    spline2->to = old1;
    spline2->order2 = order2;
    mid->next = spline2;
    old1->prev = spline2;
    if ( SplineIsLinear(spline2)) {
	spline2->islinear = spline2->from->nonextcp = spline2->to->noprevcp = true;
	spline2->from->nextcp = spline2->from->me;
	spline2->to->prevcp = spline2->to->me;
    }
    SplineRefigure(spline2);
return( mid );
}

void GrowBuffer(GrowBuf *gb) {
    if ( gb->base==NULL ) {
	gb->base = gb->pt = malloc(200);
	gb->end = gb->base + 200;
    } else {
	int len = (gb->end-gb->base) + 400;
	int off = gb->pt-gb->base;
	gb->base = realloc(gb->base,len);
	gb->end = gb->base + len;
	gb->pt = gb->base+off;
    }
}
