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
#define _DEFINE_SEARCHVIEW_
#include "fontforgevw.h"
#include <math.h>
#include <ustring.h>
#include <utype.h>

extern char *coord_sep;

int onlycopydisplayed = 0;
int copymetadata = 0;
int copyttfinstr = 0;

/* ********************************* Undoes ********************************* */

int maxundoes = 0;		/* -1 is infinite */

static void UHintListFree(void *hints) {
    StemInfo *h, *t, *p;

    if ( hints==NULL )
return;
    if ( ((StemInfo *) hints)->hinttype==ht_d )
	DStemInfosFree(hints);
    else {
	h = t = hints;
	p = NULL;
	while ( t!=NULL && t->hinttype!=ht_d ) {
	    p = t;
	    t = t->next;
	}
	p->next = NULL;
	StemInfosFree(h);
	DStemInfosFree((DStemInfo *) t);
    }
}

void UndoesFree(Undoes *undo) {
    Undoes *unext;

    while ( undo!=NULL ) {
	unext = undo->next;
	switch ( undo->undotype ) {
	  case ut_noop:
	  case ut_width: case ut_vwidth: case ut_lbearing: case ut_rbearing:
	    /* Nothing else to free */;
	  break;
	  case ut_state: case ut_tstate: case ut_statehint: case ut_statename:
	  case ut_hints: case ut_anchors: case ut_statelookup:
	    SplinePointListsFree(undo->u.state.splines);
	    RefCharsFree(undo->u.state.refs);
	    UHintListFree(undo->u.state.hints);
	    free(undo->u.state.instrs);
	    ImageListsFree(undo->u.state.images);
	    if ( undo->undotype==ut_statename ) {
		free( undo->u.state.charname );
		free( undo->u.state.comment );
		PSTFree( undo->u.state.possub );
	    }
	    AnchorPointsFree(undo->u.state.anchor);
	  break;
	  case ut_bitmap:
	    free(undo->u.bmpstate.bitmap);
	  break;
	  case ut_multiple: case ut_layers:
	    UndoesFree( undo->u.multiple.mult );
	  break;
	  case ut_composit:
	    UndoesFree(undo->u.composit.state);
	    UndoesFree(undo->u.composit.bitmaps);
	  break;
	  default:
	    IError( "Unknown undo type in UndoesFree: %d", undo->undotype );
	  break;
	}
	chunkfree(undo,sizeof(Undoes));
	undo = unext;
    }
}

int getAdobeEnc(char *name) {
    extern char *AdobeStandardEncoding[256];
    int i;

    for ( i=0; i<256; ++i )
	if ( strcmp(name,AdobeStandardEncoding[i])==0 )
    break;
    if ( i==256 ) i = -1;
return( i );
}

int SCWasEmpty(SplineChar *sc, int skip_this_layer) {
    int i;

    for ( i=ly_fore; i<sc->layer_cnt; ++i ) if ( i!=skip_this_layer && !sc->layers[i].background ) {
	if ( sc->layers[i].refs!=NULL )
return( false );
	else if ( sc->layers[i].splines!=NULL ) {
	    SplineSet *ss;
	    for ( ss = sc->layers[i].splines; ss!=NULL; ss=ss->next ) {
		if ( ss->first->prev!=NULL )
return( false );			/* Closed contour */
	    }
	}
    }
return( true );
}


