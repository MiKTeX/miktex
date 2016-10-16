/* -*- coding: utf-8 -*- */
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
# include <utype.h>

#ifdef HAVE_IEEEFP_H
# include <ieeefp.h>		/* Solaris defines isnan in ieeefp rather than math.h */
#endif

int adjustwidth = true;

RefChar *HasUseMyMetrics(SplineChar *sc,int layer) {
    RefChar *r;

    if ( layer==ly_grid ) layer = ly_fore;

    for ( r=sc->layers[layer].refs; r!=NULL; r=r->next )
	if ( r->use_my_metrics )
return( r );

return( NULL );
}

/* if they changed the width, then change the width on all bitmap chars of */
/*  ours, and if we are a letter, then change the width on all chars linked */
/*  to us which had the same width that we used to have (so if we change the */
/*  width of A, we'll also change that of À and Ä and ... */
void SCSynchronizeWidth(SplineChar *sc,real newwidth, real oldwidth) {
    struct splinecharlist *dlist;
    RefChar *r = HasUseMyMetrics(sc,ly_fore);
    int isprobablybase;

    sc->widthset = true;
    if( r!=NULL ) {
	if ( oldwidth==r->sc->width ) {
	    sc->width = r->sc->width;
return;
	}
	newwidth = r->sc->width;
    }
    if ( newwidth==oldwidth )
return;
    sc->width = newwidth;
    if ( !adjustwidth )
return;

    isprobablybase = true;
    if ( sc->unicodeenc==-1 || sc->unicodeenc>=0x10000 ||
	    !isalpha(sc->unicodeenc) || iscombining(sc->unicodeenc))
	isprobablybase = false;

    for ( dlist=sc->dependents; dlist!=NULL; dlist=dlist->next ) {
	RefChar *metrics = HasUseMyMetrics(dlist->sc,ly_fore);
	if ( metrics!=NULL && metrics->sc!=sc )
    continue;
	else if ( metrics==NULL && !isprobablybase )
    continue;
	if ( dlist->sc->width==oldwidth &&
		(metrics!=NULL)) {
	    SCSynchronizeWidth(dlist->sc,newwidth,oldwidth);
	    if ( !dlist->sc->changed ) {
		dlist->sc->changed = true;
	    }
	}
    }
}

static int _SCRefNumberPoints2(SplineSet **_rss,SplineChar *sc,int pnum,int layer) {
    SplineSet *ss, *rss = *_rss;
    SplinePoint *sp, *rsp;
    RefChar *r;
    int starts_with_cp, startcnt;

    for ( ss=sc->layers[layer].splines; ss!=NULL; ss=ss->next, rss=rss->next ) {
	if ( rss==NULL )		/* Can't happen */
    break;
	starts_with_cp = !ss->first->noprevcp &&
		((ss->first->ttfindex == pnum+1 && ss->first->prev!=NULL &&
		    ss->first->prev->from->nextcpindex==pnum ) ||
		 ((ss->first->ttfindex==0xffff || SPInterpolate( ss->first ))));
	startcnt = pnum;
	if ( starts_with_cp ) ++pnum;
	for ( sp = ss->first, rsp=rss->first; ; ) {
	    if ( sp->ttfindex==0xffff || SPInterpolate( sp ))
		rsp->ttfindex = 0xffff;
	    else
		rsp->ttfindex = pnum++;
	    if ( sp->next==NULL )
	break;
	    if ( sp->next!=NULL && sp->next->to == ss->first ) {
		if ( sp->nonextcp )
		    rsp->nextcpindex = 0xffff;
		else if ( starts_with_cp )
		    rsp->nextcpindex = startcnt;
		else
		    rsp->nextcpindex = pnum++;
	break;
	    }
	    if ( sp->nonextcp )
		rsp->nextcpindex = 0xffff;
	    else
		rsp->nextcpindex = pnum++;
	    if ( sp->next==NULL )
	break;
	    sp = sp->next->to;
	    rsp = rsp->next->to;
	}
    }

    *_rss = rss;
    for ( r = sc->layers[layer].refs; r!=NULL; r=r->next )
	pnum = _SCRefNumberPoints2(_rss,r->sc,pnum,layer);
return( pnum );
}

static int SCRefNumberPoints2(RefChar *ref,int pnum,int layer) {
    SplineSet *rss;

    rss = ref->layers[0].splines;
return( _SCRefNumberPoints2(&rss,ref->sc,pnum,layer));
}

int SSTtfNumberPoints(SplineSet *ss) {
    int pnum=0;
    SplinePoint *sp;
    int starts_with_cp;

    for ( ; ss!=NULL; ss=ss->next ) {
	starts_with_cp = !ss->first->noprevcp &&
		((ss->first->ttfindex == pnum+1 && ss->first->prev!=NULL &&
		  ss->first->prev->from->nextcpindex==pnum ) ||
		 SPInterpolate( ss->first ));
	if ( starts_with_cp && ss->first->prev!=NULL )
	    ss->first->prev->from->nextcpindex = pnum++;
	for ( sp=ss->first; ; ) {
	    if ( SPInterpolate(sp) )
		sp->ttfindex = 0xffff;
	    else
		sp->ttfindex = pnum++;
	    if ( sp->nonextcp && sp->nextcpindex!=pnum )
		sp->nextcpindex = 0xffff;
	    else if ( !starts_with_cp || (sp->next!=NULL && sp->next->to!=ss->first) )
		sp->nextcpindex = pnum++;
	    if ( sp->next==NULL )
	break;
	    sp = sp->next->to;
	    if ( sp==ss->first )
	break;
	}
    }
return( pnum );
}

static int SSPsNumberPoints(SplineChar *sc, SplineSet *splines,int pnum) {
    SplineSet *ss;
    SplinePoint *sp;

    for ( ss = splines; ss!=NULL; ss=ss->next ) {
	for ( sp=ss->first; ; ) {
	    sp->ttfindex = pnum++;
	    sp->nextcpindex = 0xffff;
	    if ( sc->numberpointsbackards ) {
		if ( sp->prev==NULL )
	break;
		if ( !sp->noprevcp || !sp->prev->from->nonextcp )
		    pnum += 2;
		sp = sp->prev->from;
	    } else {
		if ( sp->next==NULL )
	break;
		if ( !sp->nonextcp || !sp->next->to->noprevcp )
		    pnum += 2;
		sp = sp->next->to;
	    }
	    if ( sp==ss->first )
	break;
	}
    }
return( pnum );
}

int SCNumberPoints(SplineChar *sc,int layer) {
    int pnum=0;
    SplineSet *ss;
    SplinePoint *sp;
    RefChar *ref;

    if ( layer<0 || layer>=sc->layer_cnt )
        return( pnum );

    if ( sc->layers[layer].order2 ) {		/* TrueType and its complexities. I ignore svg here */
	if ( sc->layers[layer].refs!=NULL ) {
	    /* if there are references there can't be splines. So if we've got*/
	    /*  splines mark all point numbers on them as meaningless */
	    for ( ss = sc->layers[layer].splines; ss!=NULL; ss=ss->next ) {
		for ( sp=ss->first; ; ) {
		    sp->ttfindex = 0xfffe;
		    if ( !sp->nonextcp )
			sp->nextcpindex = 0xfffe;
		    if ( sp->next==NULL )
		break;
		    sp = sp->next->to;
		    if ( sp==ss->first )
		break;
		}
	    }
	    for ( ref = sc->layers[layer].refs; ref!=NULL; ref=ref->next )
		pnum = SCRefNumberPoints2(ref,pnum,layer);
	} else {
	    pnum = SSTtfNumberPoints(sc->layers[layer].splines);
	}
    } else {		/* cubic (PostScript/SVG) splines */
	int first, last;
	if ( sc->parent->multilayer ) {
	    first = ly_fore;
	    last = sc->layer_cnt-1;
	} else
	    first = last = layer;
	for ( layer=first; layer<=last; ++layer ) {
	    for ( ref = sc->layers[layer].refs; ref!=NULL; ref=ref->next )
		pnum = SSPsNumberPoints(sc,ref->layers[0].splines,pnum);
	    pnum = SSPsNumberPoints(sc,sc->layers[layer].splines,pnum);
	}
    }
return( pnum );
}

int SCPointsNumberedProperly(SplineChar *sc,int layer) {
    int pnum=0, skipit;
    SplineSet *ss;
    SplinePoint *sp;
    int starts_with_cp;
    int start_pnum;

    if ( sc->layers[layer].splines!=NULL &&
	    sc->layers[layer].refs!=NULL )
return( false );	/* TrueType can't represent this, so always remove instructions. They can't be meaningful */

    for ( ss = sc->layers[layer].splines; ss!=NULL; ss=ss->next ) {
	starts_with_cp = (ss->first->ttfindex == pnum+1 || ss->first->ttfindex==0xffff) &&
		!ss->first->noprevcp;
	start_pnum = pnum;
	if ( starts_with_cp ) ++pnum;
	for ( sp=ss->first; ; ) {
	    skipit = SPInterpolate(sp);
	    if ( sp->nonextcp || sp->noprevcp ) skipit = false;
	    if ( sp->ttfindex==0xffff && skipit )
		/* Doesn't count */;
	    else if ( sp->ttfindex!=pnum )
return( false );
	    else
		++pnum;
	    if ( sp->nonextcp && sp->nextcpindex==0xffff )
		/* Doesn't count */;
	    else if ( sp->nextcpindex==pnum )
		++pnum;
	    else if ( sp->nextcpindex==start_pnum && starts_with_cp &&
		    (sp->next!=NULL && sp->next->to==ss->first) )
		/* Ok */;
	    else
return( false );
	    if ( sp->next==NULL )
	break;
	    sp = sp->next->to;
	    if ( sp==ss->first )
	break;
	}
	/* if ( starts_with_cp ) --pnum; */
    }
return( true );
}

void AltUniAdd(SplineChar *sc,int uni) {
    struct altuni *altuni;

    if ( sc!=NULL && uni!=-1 && uni!=sc->unicodeenc ) {
	for ( altuni = sc->altuni; altuni!=NULL && (altuni->unienc!=uni ||
						    altuni->vs!=-1 ||
			                            altuni->fid); altuni=altuni->next );
	if ( altuni==NULL ) {
	    altuni = chunkalloc(sizeof(struct altuni));
	    altuni->next = sc->altuni;
	    sc->altuni = altuni;
	    altuni->unienc = uni;
	    altuni->vs = -1;
	    altuni->fid = 0;
	}
    }
}

void AltUniAdd_DontCheckDups(SplineChar *sc,int uni) {
    struct altuni *altuni;

    if ( sc!=NULL && uni!=-1 && uni!=sc->unicodeenc ) {
	altuni = chunkalloc(sizeof(struct altuni));
	altuni->next = sc->altuni;
	sc->altuni = altuni;
	altuni->unienc = uni;
	altuni->vs = -1;
	altuni->fid = 0;
    }
}
