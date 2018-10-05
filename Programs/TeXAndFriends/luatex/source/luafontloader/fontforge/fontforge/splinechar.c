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
#include <math.h>
#include <locale.h>
# include <ustring.h>
# include <utype.h>
#ifdef HAVE_IEEEFP_H
# include <ieeefp.h>		/* Solaris defines isnan in ieeefp rather than math.h */
#endif
#include "ttf.h"

int adjustwidth = true;
int adjustlbearing = true;
int allow_utf8_glyphnames = false;
int clear_tt_instructions_when_needed = true;

void SCClearRounds(SplineChar *sc,int layer) {
    SplineSet *ss;
    SplinePoint *sp;

    for ( ss=sc->layers[layer].splines; ss!=NULL; ss=ss->next ) {
	for ( sp=ss->first; ; ) {
	    sp->roundx = sp->roundy = false;
	    if ( sp->next==NULL )
	break;
	    sp = sp->next->to;
	    if ( sp==ss->first )
	break;
	}
    }
}


void SCClearLayer(SplineChar *sc,int layer) {
    RefChar *refs, *next;

    SplinePointListsFree(sc->layers[layer].splines);
    sc->layers[layer].splines = NULL;
    for ( refs=sc->layers[layer].refs; refs!=NULL; refs = next ) {
	next = refs->next;
	SCRemoveDependent(sc,refs,layer);
    }
    sc->layers[layer].refs = NULL;
    ImageListsFree(sc->layers[layer].images);
    sc->layers[layer].images = NULL;
}

void SCClearContents(SplineChar *sc,int layer) {
    int ly_first, ly_last;

    if ( sc==NULL )
return;
    if ( sc->parent!=NULL && sc->parent->multilayer ) {
	ly_first = ly_fore;
	ly_last = sc->layer_cnt-1;
    } else
	ly_first = ly_last = layer;
    for ( layer = ly_first; layer<=ly_last; ++layer )
	SCClearLayer(sc,layer);
    --layer;

    if ( sc->parent!=NULL &&
	    (sc->parent->multilayer ||
		(!sc->parent->layers[layer].background && SCWasEmpty(sc,layer)))) {
	sc->widthset = false;
	if ( sc->parent!=NULL && sc->width!=0 )
	    sc->width = sc->parent->ascent+sc->parent->descent;
	AnchorPointsFree(sc->anchor);
	sc->anchor = NULL;
	StemInfosFree(sc->hstem); sc->hstem = NULL;
	StemInfosFree(sc->vstem); sc->vstem = NULL;
	DStemInfosFree(sc->dstem); sc->dstem = NULL;
	MinimumDistancesFree(sc->md); sc->md = NULL;
	free(sc->ttf_instrs);
	sc->ttf_instrs = NULL;
	sc->ttf_instrs_len = 0;
	SCOutOfDateBackground(sc);
    }
}


void SplinePointRound(SplinePoint *sp,real factor) {

    sp->nextcp.x = rint(sp->nextcp.x*factor)/factor;
    sp->nextcp.y = rint(sp->nextcp.y*factor)/factor;
    if ( sp->next!=NULL && sp->next->order2 )
	sp->next->to->prevcp = sp->nextcp;
    sp->prevcp.x = rint(sp->prevcp.x*factor)/factor;
    sp->prevcp.y = rint(sp->prevcp.y*factor)/factor;
    if ( sp->prev!=NULL && sp->prev->order2 )
	sp->prev->from->nextcp = sp->prevcp;
    if ( sp->prev!=NULL && sp->next!=NULL && sp->next->order2 &&
	    sp->ttfindex == 0xffff ) {
	sp->me.x = (sp->nextcp.x + sp->prevcp.x)/2;
	sp->me.y = (sp->nextcp.y + sp->prevcp.y)/2;
    } else {
	sp->me.x = rint(sp->me.x*factor)/factor;
	sp->me.y = rint(sp->me.y*factor)/factor;
    }
}

void SplineSetsRound2Int(SplineSet *spl,real factor, int inspiro, int onlysel) {
    SplinePoint *sp;

    for ( ; spl!=NULL; spl=spl->next ) {
	if ( inspiro ) {
	} else {
	    for ( sp=spl->first; ; ) {
		if ( sp->selected || !onlysel )
		    SplinePointRound(sp,factor);
		if ( sp->prev!=NULL )
		    SplineRefigure(sp->prev);
		if ( sp->next==NULL )
	    break;
		sp = sp->next->to;
		if ( sp==spl->first )
	    break;
	    }
	    if ( spl->first->prev!=NULL )
		SplineRefigure(spl->first->prev);
	}
    }
}

void SCOrderAP(SplineChar *sc) {
    int lc=0, cnt=0, out=false, i,j;
    AnchorPoint *ap, **array;
    /* Order so that first ligature index comes first */

    for ( ap=sc->anchor; ap!=NULL; ap=ap->next ) {
	if ( ap->lig_index<lc ) out = true;
	if ( ap->lig_index>lc ) lc = ap->lig_index;
	++cnt;
    }
    if ( !out )
return;

    array = galloc(cnt*sizeof(AnchorPoint *));
    for ( i=0, ap=sc->anchor; ap!=NULL; ++i, ap=ap->next )
	array[i] = ap;
    for ( i=0; i<cnt-1; ++i ) {
	for ( j=i+1; j<cnt; ++j ) {
	    if ( array[i]->lig_index>array[j]->lig_index ) {
		ap = array[i];
		array[i] = array[j];
		array[j] = ap;
	    }
	}
    }
    sc->anchor = array[0];
    for ( i=0; i<cnt-1; ++i )
	array[i]->next = array[i+1];
    array[cnt-1]->next = NULL;
    free( array );
}


static void SCUpdateNothing(SplineChar *sc) {
}

static void SCHintsChng(SplineChar *sc) {
    sc->changedsincelasthinted = false;
    if ( !sc->changed ) {
	sc->changed = true;
	sc->parent->changed = true;
    }
}

static void _SCChngNoUpdate(SplineChar *sc,int layer,int changed) {
}

static void SCChngNoUpdate(SplineChar *sc,int layer) {
    _SCChngNoUpdate(sc,layer,true);
}

static void SCB_MoreLayers(SplineChar *sc,Layer *old) {
}

static struct sc_interface noui_sc = {
    SCUpdateNothing,
    SCUpdateNothing,
    SCUpdateNothing,
    SCHintsChng,
    SCChngNoUpdate,
    _SCChngNoUpdate,
    SCUpdateNothing,
    SCUpdateNothing,
    SCB_MoreLayers
};

struct sc_interface *sc_interface = &noui_sc;

void FF_SetSCInterface(struct sc_interface *sci) {
    sc_interface = sci;
}

static void CVChngNoUpdate(CharViewBase *cv) {
}

static void _CVChngNoUpdate(CharViewBase *cv,int changed) {
}

static void CVGlphRenameFixup(SplineFont *sf,char *oldname, char *newname) {
}

static void CV__LayerPaletteCheck(SplineFont *sf) {
}

static struct cv_interface noui_cv = {
    CVChngNoUpdate,
    _CVChngNoUpdate,
    CVGlphRenameFixup,
    CV__LayerPaletteCheck
};

struct cv_interface *cv_interface = &noui_cv;

void FF_SetCVInterface(struct cv_interface *cvi) {
    cv_interface = cvi;
}
