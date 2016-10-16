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
#include "fontforgevw.h"		/* For Error */


/* ************************************************************************** */
/* **************************** Reading OFM files *************************** */
/* ************************************************************************** */

#define ExtShort(off) (((ext+2*off)[0]<<8)|(ext+2*off)[1])

/* ************************************************************************** */

int SCDrawsSomething(SplineChar *sc) {
    int layer,l;
    RefChar *ref;

    if ( sc==NULL )
return( false );
    for ( layer = 0; layer<sc->layer_cnt; ++layer ) if ( !sc->layers[layer].background ) {
	if ( sc->layers[layer].splines!=NULL)
return( true );
	for ( ref = sc->layers[layer].refs; ref!=NULL; ref=ref->next )
	    for ( l=0; l<ref->layer_cnt; ++l )
		if ( ref->layers[l].splines!=NULL )
return( true );
    }
return( false );
}

int SCWorthOutputting(SplineChar *sc) {
return( sc!=NULL &&
	( SCDrawsSomething(sc) || sc->widthset || sc->anchor!=NULL ||
#if HANYANG
	    sc->compositionunit ||
#endif
	    sc->dependents!=NULL /*||
	    sc->width!=sc->parent->ascent+sc->parent->descent*/ ) );
}

void SFLigatureCleanup(SplineFont *sf) {
    LigList *l, *next;
    struct splinecharlist *scl, *sclnext;
    int j;

    if (sf->internal_temp)
return;

    for ( j=0; j<sf->glyphcnt; ++j ) if ( sf->glyphs[j]!=NULL ) {
	for ( l = sf->glyphs[j]->ligofme; l!=NULL; l = next ) {
	    next = l->next;
	    for ( scl = l->components; scl!=NULL; scl = sclnext ) {
		sclnext = scl->next;
		chunkfree(scl,sizeof(struct splinecharlist));
	    }
	    if ( l->lig->temporary ) {
		free(l->lig->u.lig.components);
		chunkfree(l->lig,sizeof(PST));
	    }
	    free( l );
	}
	sf->glyphs[j]->ligofme = NULL;
    }
}

void SFLigaturePrepare(SplineFont *sf) {
    PST *lig;
    LigList *ll;
    int i,j,k,ch;
    char *pt, *ligstart;
    SplineChar *sc, *tsc;
    struct splinecharlist *head, *last;
    int ccnt, lcnt, lmax=20;
    LigList **all = malloc(lmax*sizeof(LigList *));

    /* First clear out any old stuff */
    for ( j=0; j<sf->glyphcnt; ++j ) if ( sf->glyphs[j]!=NULL )
	sf->glyphs[j]->ligofme = NULL;

    /* Attach all the ligatures to the first character of their components */
    /* Figure out what the components are, and if they all exist */
    /* we're only interested in the lig if all components are worth outputting */
    for ( i=0 ; i<sf->glyphcnt; ++i ) if ( SCWorthOutputting(sf->glyphs[i]) && sf->glyphs[i]->possub!=NULL ) {
	for ( lig = sf->glyphs[i]->possub; lig!=NULL; lig=lig->next ) if ( lig->type==pst_ligature ) {
	    ligstart = lig->u.lig.components;
	    last = head = NULL; sc = NULL;
	    for ( pt = ligstart; *pt!='\0'; ) {
		char *start = pt;
		for ( ; *pt!='\0' && *pt!=' '; ++pt );
		ch = *pt; *pt = '\0';
		tsc = SFGetChar(sf,-1,start);
		*pt = ch;
		if ( tsc!=NULL ) {
		    if ( !SCWorthOutputting(tsc)) {
			sc = NULL;
	    break;
		    }
		    if ( sc==NULL ) {
			sc = tsc;
			ccnt = 1;
		    } else {
			struct splinecharlist *cur = chunkalloc(sizeof(struct splinecharlist));
			if ( head==NULL )
			    head = cur;
			else
			    last->next = cur;
			last = cur;
			cur->sc = tsc;
			cur->next = NULL;
			++ccnt;
		    }
		} else {
		    sc = NULL;
	    break;
		}
		while ( *pt==' ' ) ++pt;
	    }
	    if ( sc!=NULL ) {
		ll = malloc(sizeof(LigList));
		ll->lig = lig;
		ll->next = sc->ligofme;
		ll->first = sc;
		ll->components = head;
		ll->ccnt = ccnt;
		sc->ligofme = ll;
	    } else {
		while ( head!=NULL ) {
		    last = head->next;
		    chunkfree(head,sizeof(*head));
		    head = last;
		}
	    }
	}
    }
    for ( i=0 ; i<sf->glyphcnt; ++i ) if ( (sc=sf->glyphs[i])!=NULL && sc->ligofme!=NULL ) {
	for ( ll=sc->ligofme, lcnt=0; ll!=NULL; ll=ll->next, ++lcnt );
	/* Finally, order the list so that the longest ligatures are first */
	if ( lcnt>1 ) {
	    if ( lcnt>=lmax )
		all = realloc(all,(lmax=lcnt+30)*sizeof(LigList *));
	    for ( ll=sc->ligofme, k=0; ll!=NULL; ll=ll->next, ++k )
		all[k] = ll;
	    for ( k=0; k<lcnt-1; ++k ) for ( j=k+1; j<lcnt; ++j )
		if ( all[k]->ccnt<all[j]->ccnt ) {
		    ll = all[k];
		    all[k] = all[j];
		    all[j] = ll;
		}
	    sc->ligofme = all[0];
	    for ( k=0; k<lcnt-1; ++k )
		all[k]->next = all[k+1];
	    all[k]->next = NULL;
	}
    }
    free( all );
}

void SFKernCleanup(SplineFont *sf,int isv) {
    int i;
    KernPair *kp, *p, *n;
    OTLookup *otl, *otlp, *otln;

    if (sf->internal_temp)
return;

    if ( (!isv && sf->kerns==NULL) || (isv && sf->vkerns==NULL) )	/* can't have gotten messed up */
return;
    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL ) {
	for ( kp = isv ? sf->glyphs[i]->vkerns : sf->glyphs[i]->kerns, p=NULL; kp!=NULL; kp = n ) {
	    n = kp->next;
	    if ( kp->kcid!=0 ) {
		if ( p!=NULL )
		    p->next = n;
		else if ( isv )
		    sf->glyphs[i]->vkerns = n;
		else
		    sf->glyphs[i]->kerns = n;
		chunkfree(kp,sizeof(*kp));
	    } else
		p = kp;
	}
    }
    for ( otl=sf->gpos_lookups, otlp = NULL; otl!=NULL; otl = otln ) {
	otln = otl->next;
	if ( otl->temporary_kern ) {
	    if ( otlp!=NULL )
		otlp->next = otln;
	    else
		sf->gpos_lookups = otln;
	    OTLookupFree(otl);
	} else
	    otlp = otl;
    }
}

static void KCSfree(SplineChar ***scs,int cnt) {
    int i;
    for ( i=1; i<cnt; ++i )
	free( scs[i]);
    free(scs);
}

static SplineChar ***KernClassToSC(SplineFont *sf, char **classnames, int cnt) {
    SplineChar ***scs, *sc;
    int i,j;
    char *pt, *end, ch;

    scs = malloc(cnt*sizeof(SplineChar **));
    for ( i=1; i<cnt; ++i ) {
	for ( pt=classnames[i]-1, j=0; pt!=NULL; pt=strchr(pt+1,' ') )
	    ++j;
	scs[i] = malloc((j+1)*sizeof(SplineChar *));
	for ( pt=classnames[i], j=0; *pt!='\0'; pt=end+1 ) {
	    end = strchr(pt,' ');
	    if ( end==NULL )
		end = pt+strlen(pt);
	    ch = *end;
	    *end = '\0';
	    sc = SFGetChar(sf,-1,pt);
	    if ( sc!=NULL )
		scs[i][j++] = sc;
	    if ( ch=='\0' )
	break;
	    *end = ch;
	}
	scs[i][j] = NULL;
    }
return( scs );
}

static void AddTempKP(SplineChar *first,SplineChar *second,
	int16 offset, struct lookup_subtable *sub,uint16 kcid,int isv) {
    KernPair *kp;

    for ( kp=first->kerns; kp!=NULL; kp=kp->next )
	if ( kp->sc == second )
    break;
    if ( kp==NULL ) {
	kp = chunkalloc(sizeof(KernPair));
	kp->sc = second;
	kp->off = offset;
	kp->subtable = sub;
	kp->kcid = kcid;
	if ( isv ) {
	    kp->next = first->vkerns;
	    first->vkerns = kp;
	} else {
	    kp->next = first->kerns;
	    first->kerns = kp;
	}
    }
}

void SFKernClassTempDecompose(SplineFont *sf,int isv) {
    KernClass *kc, *head= isv ? sf->vkerns : sf->kerns;
    KernPair *kp;
    SplineChar ***first, ***last;
    int i, j, k, l;
    OTLookup *otl;

    /* Make sure the temporary field is cleaned up. Otherwise we may lose kerning data */
    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL ) {
	for ( kp = isv ? sf->glyphs[i]->vkerns : sf->glyphs[i]->kerns; kp!=NULL; kp = kp->next ) {
	    kp->kcid = false;
	}
    }
    for ( kc = head, i=0; kc!=NULL; kc = kc->next )
	kc->kcid = ++i;
    for ( kc = head; kc!=NULL; kc = kc->next ) {

	otl = chunkalloc(sizeof(OTLookup));
	otl->next = sf->gpos_lookups;
	sf->gpos_lookups = otl;
	otl->lookup_type = gpos_pair;
	otl->lookup_flags = kc->subtable->lookup->lookup_flags;
	otl->features = FeatureListCopy(kc->subtable->lookup->features);
	otl->lookup_name = copy(_("<Temporary kerning>"));
	otl->temporary_kern = otl->store_in_afm = true;
	otl->subtables = chunkalloc(sizeof(struct lookup_subtable));
	otl->subtables->lookup = otl;
	otl->subtables->per_glyph_pst_or_kern = true;
	otl->subtables->subtable_name = copy(_("<Temporary kerning>"));

	first = KernClassToSC(sf,kc->firsts,kc->first_cnt);
	last = KernClassToSC(sf,kc->seconds,kc->second_cnt);
	for ( i=1; i<kc->first_cnt; ++i ) for ( j=1; j<kc->second_cnt; ++j ) {
	    if ( kc->offsets[i*kc->second_cnt+j]!=0 ) {
		for ( k=0; first[i][k]!=NULL; ++k )
		    for ( l=0; last[j][l]!=NULL; ++l )
			AddTempKP(first[i][k],last[j][l],
				kc->offsets[i*kc->second_cnt+j],
			        otl->subtables,kc->kcid,isv);
	    }
	}
	KCSfree(first,kc->first_cnt);
	KCSfree(last,kc->second_cnt);
    }
}
