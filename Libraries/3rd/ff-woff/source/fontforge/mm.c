/* Copyright (C) 2003-2012 by George Williams */
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
#include "ttf.h"

/******************************************************************************/
/*                                MM Validation                               */
/******************************************************************************/

static int ContourCount(SplineChar *sc) {
    SplineSet *spl;
    int i;

    for ( spl=sc->layers[ly_fore].splines, i=0; spl!=NULL; spl=spl->next, ++i );
return( i );
}

static int ContourPtMatch(SplineChar *sc1, SplineChar *sc2) {
    SplineSet *spl1, *spl2;
    SplinePoint *sp1, *sp2;

    for ( spl1=sc1->layers[ly_fore].splines, spl2=sc2->layers[ly_fore].splines; spl1!=NULL && spl2!=NULL; spl1=spl1->next, spl2=spl2->next ) {
	for ( sp1=spl1->first, sp2 = spl2->first; ; ) {
	    if ( sp1->nonextcp!=sp2->nonextcp || sp1->noprevcp!=sp2->noprevcp )
return( false );
	    if ( sp1->next==NULL || sp2->next==NULL ) {
		if ( sp1->next==NULL && sp2->next==NULL )
	break;
return( false );
	    }
	    sp1 = sp1->next->to; sp2 = sp2->next->to;
	    if ( sp1==spl1->first || sp2==spl2->first ) {
		if ( sp1==spl1->first && sp2==spl2->first )
	break;
return( false );
	    }
	}
    }
return( true );
}

static int ContourDirMatch(SplineChar *sc1, SplineChar *sc2) {
    SplineSet *spl1, *spl2;

    for ( spl1=sc1->layers[ly_fore].splines, spl2=sc2->layers[ly_fore].splines; spl1!=NULL && spl2!=NULL; spl1=spl1->next, spl2=spl2->next ) {
	if ( SplinePointListIsClockwise(spl1)!=SplinePointListIsClockwise(spl2) )
return( false );
    }
return( true );
}

static int ContourHintMaskMatch(SplineChar *sc1, SplineChar *sc2) {
    SplineSet *spl1, *spl2;
    SplinePoint *sp1, *sp2;

    for ( spl1=sc1->layers[ly_fore].splines, spl2=sc2->layers[ly_fore].splines; spl1!=NULL && spl2!=NULL; spl1=spl1->next, spl2=spl2->next ) {
	for ( sp1=spl1->first, sp2 = spl2->first; ; ) {
	    if ( (sp1->hintmask==NULL)!=(sp2->hintmask==NULL) )
return( false );
	    if ( sp1->hintmask!=NULL && memcmp(sp1->hintmask,sp2->hintmask,sizeof(HintMask))!=0 )
return( false );
	    if ( sp1->next==NULL || sp2->next==NULL ) {
		if ( sp1->next==NULL && sp2->next==NULL )
	break;
return( false );
	    }
	    sp1 = sp1->next->to; sp2 = sp2->next->to;
	    if ( sp1==spl1->first || sp2==spl2->first ) {
		if ( sp1==spl1->first && sp2==spl2->first )
	break;
return( false );
	    }
	}
    }
return( true );
}

static int RefMatch(SplineChar *sc1, SplineChar *sc2) {
    RefChar *ref1, *ref2;
    /* I don't require the reference list to be ordered */

    for ( ref1=sc1->layers[ly_fore].refs, ref2=sc2->layers[ly_fore].refs; ref1!=NULL && ref2!=NULL; ref1=ref1->next, ref2=ref2->next )
	ref2->checked = false;
    if ( ref1!=NULL || ref2!=NULL )
return( false );

    for ( ref1=sc1->layers[ly_fore].refs; ref1!=NULL ; ref1=ref1->next ) {
	for ( ref2=sc2->layers[ly_fore].refs; ref2!=NULL ; ref2=ref2->next ) {
	    if ( ref2->sc->orig_pos==ref1->sc->orig_pos && !ref2->checked )
	break;
	}
	if ( ref2==NULL )
return( false );
	ref2->checked = true;
    }

return( true );
}

static int RefTransformsMatch(SplineChar *sc1, SplineChar *sc2) {
    /* Apple only provides a means to change the translation of a reference */
    /*  so if rotation, skewing, scaling, etc. differ then we can't deal with */
    /*  it. */
    RefChar *r1 = sc1->layers[ly_fore].refs;
    RefChar *r2 = sc2->layers[ly_fore].refs;

    while ( r1!=NULL && r2!=NULL ) {
	if ( r1->transform[0]!=r2->transform[0] ||
		r1->transform[1]!=r2->transform[1] ||
		r1->transform[2]!=r2->transform[2] ||
		r1->transform[3]!=r2->transform[3] )
return( false );
	r1 = r1->next;
	r2 = r2->next;
    }
return( true );
}

static int HintsMatch(StemInfo *h1,StemInfo *h2) {
    while ( h1!=NULL && h2!=NULL ) {
	h1 = h1->next;
	h2 = h2->next;
    }
return ( h1==NULL && h2==NULL );
}

static int KernsMatch(SplineChar *sc1, SplineChar *sc2) {
    /* I don't require the kern list to be ordered */
    /* Only interested in kerns that go into afm files (ie. no kernclasses) */
    KernPair *k1, *k2;

    for ( k1=sc1->kerns, k2=sc2->kerns; k1!=NULL && k2!=NULL; k1=k1->next, k2=k2->next )
	k2->kcid = false;
    if ( k1!=NULL || k2!=NULL )
return( false );

    for ( k1=sc1->kerns; k1!=NULL ; k1=k1->next ) {
	for ( k2=sc2->kerns; k2!=NULL ; k2=k2->next ) {
	    if ( k2->sc->orig_pos==k1->sc->orig_pos && !k2->kcid )
	break;
	}
	if ( k2==NULL )
return( false );
	k2->kcid = true;
    }

return( true );
}

static int ArrayCount(char *val) {
    char *end;
    int cnt;

    if ( val==NULL )
return( 0 );
    while ( *val==' ' ) ++val;
    if ( *val=='[' ) ++val;
    cnt=0;
    while ( *val ) {
	strtod(val,&end);
	if ( val==end )
    break;
	++cnt;
	val = end;
    }
return( cnt );
}

int MMValid(MMSet *mm,int complain) {
    int i, j;
    SplineFont *sf;
    static char *arrnames[] = { "BlueValues", "OtherBlues", "FamilyBlues", "FamilyOtherBlues", "StdHW", "StdVW", "StemSnapH", "StemSnapV", NULL };

    if ( mm==NULL )
return( false );

    for ( i=0; i<mm->instance_count; ++i )
	if ( mm->instances[i]->layers[ly_fore].order2 != mm->apple ) {
return( false );
	}

    sf = mm->apple ? mm->normal : mm->instances[0];

    if ( !mm->apple && PSDictHasEntry(sf->private,"ForceBold")!=NULL &&
	    PSDictHasEntry(mm->normal->private,"ForceBoldThreshold")==NULL) {
return( false );
    }

    for ( j=mm->apple ? 0 : 1; j<mm->instance_count; ++j ) {
	if ( sf->glyphcnt!=mm->instances[j]->glyphcnt ) {
return( false );
	} else if ( sf->layers[ly_fore].order2!=mm->instances[j]->layers[ly_fore].order2 ) {
return( false );
	}
	if ( !mm->apple ) {
	    if ( PSDictHasEntry(mm->instances[j]->private,"ForceBold")!=NULL &&
		    PSDictHasEntry(mm->normal->private,"ForceBoldThreshold")==NULL) {
return( false );
	    }
	    for ( i=0; arrnames[i]!=NULL; ++i ) {
		if ( ArrayCount(PSDictHasEntry(mm->instances[j]->private,arrnames[i]))!=
				ArrayCount(PSDictHasEntry(sf->private,arrnames[i])) ) {
return( false );
		}
	    }
	}
    }

    for ( i=0; i<sf->glyphcnt; ++i ) {
	for ( j=mm->apple?0:1; j<mm->instance_count; ++j ) {
	    if ( SCWorthOutputting(sf->glyphs[i])!=SCWorthOutputting(mm->instances[j]->glyphs[i]) ) {
return( false );
	    }
	}
	if ( SCWorthOutputting(sf->glyphs[i]) ) {
	    if ( mm->apple && sf->glyphs[i]->layers[ly_fore].refs!=NULL && sf->glyphs[i]->layers[ly_fore].splines!=NULL ) {
return( false );
	    }
	    for ( j=mm->apple?0:1; j<mm->instance_count; ++j ) {
		if ( mm->apple && mm->instances[j]->glyphs[i]->layers[ly_fore].refs!=NULL &&
			mm->instances[j]->glyphs[i]->layers[ly_fore].splines!=NULL ) {
return( false );
		}
		if ( ContourCount(sf->glyphs[i])!=ContourCount(mm->instances[j]->glyphs[i])) {
return( false );
		} else if ( !mm->apple && !ContourPtMatch(sf->glyphs[i],mm->instances[j]->glyphs[i])) {
return( false );
		} else if ( !ContourDirMatch(sf->glyphs[i],mm->instances[j]->glyphs[i])) {
return( false );
		} else if ( !RefMatch(sf->glyphs[i],mm->instances[j]->glyphs[i])) {
return( false );
		} else if ( mm->apple && !RefTransformsMatch(sf->glyphs[i],mm->instances[j]->glyphs[i])) {
return( false );
		} else if ( !mm->apple && !KernsMatch(sf->glyphs[i],mm->instances[j]->glyphs[i])) {
return( false );
		}
	    }
	    if ( mm->apple && !ContourPtNumMatch(mm,i)) {
return( false );
	    }
	    if ( !mm->apple ) {
		for ( j=1; j<mm->instance_count; ++j ) {
		    if ( !HintsMatch(sf->glyphs[i]->hstem,mm->instances[j]->glyphs[i]->hstem)) {
return( false );
		    } else if ( !HintsMatch(sf->glyphs[i]->vstem,mm->instances[j]->glyphs[i]->vstem)) {
return( false );
		    }
		}
		for ( j=1; j<mm->instance_count; ++j ) {
		    if ( !ContourHintMaskMatch(sf->glyphs[i],mm->instances[j]->glyphs[i])) {
return( false );
		    }
		}
	    }
	}
    }
    if ( mm->apple ) {
	struct ttf_table *cvt;
	for ( cvt = mm->normal->ttf_tables; cvt!=NULL && cvt->tag!=CHR('c','v','t',' '); cvt=cvt->next );
	if ( cvt==NULL ) {
	    for ( j=0; j<mm->instance_count; ++j ) {
		if ( mm->instances[j]->ttf_tables!=NULL ) {
return( false );
		}
	    }
	} else {
	    /* Not all instances are required to have cvts, but any that do */
	    /*  must be the same size */
	    for ( j=0; j<mm->instance_count; ++j ) {
		if ( mm->instances[j]->ttf_tables!=NULL &&
			(mm->instances[j]->ttf_tables->next!=NULL ||
			 mm->instances[j]->ttf_tables->tag!=CHR('c','v','t',' '))) {
return( false );
		}
		if ( mm->instances[j]->ttf_tables!=NULL &&
			mm->instances[j]->ttf_tables->len!=cvt->len ) {
return( false );
		}
	    }
	}
    }
return( true );
}
