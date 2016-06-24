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
#include "baseviews.h"
#include "psfont.h"
#include <ustring.h>
#include <utype.h>
#include <chardata.h>
#include <math.h>
#include <unistd.h>
#include <gfile.h>

static FontViewBase *fv_list=NULL;

extern int onlycopydisplayed;
float joinsnap=0;


/*                             FV Interface                                   */

static FontViewBase *_FontViewBaseCreate(SplineFont *sf) {
    FontViewBase *fv = gcalloc(1,sizeof(FontViewBase));
    int i;

    fv->nextsame = sf->fv;
    fv->active_layer = ly_fore;
    sf->fv = fv;
    if ( sf->mm!=NULL ) {
	sf->mm->normal->fv = fv;
	for ( i = 0; i<sf->mm->instance_count; ++i )
	    sf->mm->instances[i]->fv = fv;
    }
    if ( sf->subfontcnt==0 ) {
	fv->sf = sf;
	if ( fv->nextsame!=NULL ) {
	    fv->map = EncMapCopy(fv->nextsame->map);
	    fv->normal = fv->nextsame->normal==NULL ? NULL : EncMapCopy(fv->nextsame->normal);
	} else if ( sf->compacted ) {
	    fv->normal = sf->map;
	    fv->map = CompactEncMap(EncMapCopy(sf->map),sf);
	} else {
	    fv->map = sf->map;
	    fv->normal = NULL;
	}
    } else {
	fv->cidmaster = sf;
	for ( i=0; i<sf->subfontcnt; ++i )
	    sf->subfonts[i]->fv = fv;
	for ( i=0; i<sf->subfontcnt; ++i )	/* Search for a subfont that contains more than ".notdef" (most significant in .gai fonts) */
	    if ( sf->subfonts[i]->glyphcnt>1 ) {
		fv->sf = sf->subfonts[i];
	break;
	    }
	if ( fv->sf==NULL )
	    fv->sf = sf->subfonts[0];
	sf = fv->sf;
	if ( fv->nextsame==NULL ) EncMapFree(sf->map);
	fv->map = EncMap1to1(sf->glyphcnt);
    }
    fv->selected = gcalloc(fv->map->enccount,sizeof(char));

#ifndef _NO_PYTHON
    PyFF_InitFontHook(fv);
#endif
return( fv );
}

static FontViewBase *FontViewBase_Create(SplineFont *sf,int hide) {
    FontViewBase *fv = _FontViewBaseCreate(sf);
    (void)hide;
return( fv );
}

static FontViewBase *FontViewBase_Append(FontViewBase *fv) {
    /* Normally fontviews get added to the fv list when their windows are */
    /*  created. but we don't create any windows here, so... */
    FontViewBase *test;

    if ( fv_list==NULL ) fv_list = fv;
    else {
	for ( test = fv_list; test->next!=NULL; test=test->next );
	test->next = fv;
    }
return( fv );
}

static void FontViewBase_Free(FontViewBase *fv) {
    int i;
    FontViewBase *prev;

    if (fv->cidmaster)
       EncMapFree(fv->cidmaster->map);

   if ( fv->nextsame==NULL && fv->sf->fv==fv ) {
	EncMapFree(fv->map);
	SplineFontFree(fv->cidmaster?fv->cidmaster:fv->sf);
    } else {
	EncMapFree(fv->map);
	if ( fv->sf->fv==fv ) {
	    if ( fv->cidmaster==NULL )
		fv->sf->fv = fv->nextsame;
	    else {
		fv->cidmaster->fv = fv->nextsame;
		for ( i=0; i<fv->cidmaster->subfontcnt; ++i )
		    fv->cidmaster->subfonts[i]->fv = fv->nextsame;
	    }
	} else {
	    for ( prev = fv->sf->fv; prev->nextsame!=fv; prev=prev->nextsame );
	    prev->nextsame = fv->nextsame;
	}
    }
#ifndef _NO_FFSCRIPT
    DictionaryFree(fv->fontvars);
    free(fv->fontvars);
#endif
    free(fv->selected);
#ifndef _NO_PYTHON
    PyFF_FreeFV(fv);
#endif
    free(fv);
}

static int FontViewBaseWinInfo(FontViewBase *fv, int *cc, int *rc) {
    (void)fv;
    *cc = 16; *rc = 4;
return( -1 );
}

static void FontViewBaseSetTitle(FontViewBase *foo) { (void)foo; }
static void FontViewBaseSetTitles(SplineFont *foo) { (void)foo; }
static void FontViewBaseRefreshAll(SplineFont *foo) { (void)foo; }
static void FontViewBaseReformatOne(FontViewBase *foo) { (void)foo; }
static void FontViewBaseReformatAll(SplineFont *foo) { (void)foo; }
static void FontViewBaseLayerChanged(FontViewBase *foo) {(void)foo;  }
static void FV_ToggleCharChanged(SplineChar *foo) {(void)foo;  }
static FontViewBase *FVAny(void) { return fv_list; }
static int  FontIsActive(SplineFont *sf) {
    FontViewBase *fv;

    for ( fv=fv_list; fv!=NULL; fv=fv->next )
	if ( fv->sf == sf )
return( true );

return( false );
}

static SplineFont *FontOfFilename(const char *filename) {
    char buffer[1025];
    FontViewBase *fv;

    GFileGetAbsoluteName((char *) filename,buffer,sizeof(buffer)); 
    for ( fv=fv_list; fv!=NULL ; fv=fv->next ) {
	if ( fv->sf->filename!=NULL && strcmp(fv->sf->filename,buffer)==0 )
return( fv->sf );
	else if ( fv->sf->origname!=NULL && strcmp(fv->sf->origname,buffer)==0 )
return( fv->sf );
    }
return( NULL );
}

static void FVExtraEncSlots(FontViewBase *fv, int encmax) {
  (void)fv;
  (void)encmax;
}

static void FontViewBase_Close(FontViewBase *fv) {
    if ( fv_list==fv )
	fv_list = fv->next;
    else {
	FontViewBase *n;
	for ( n=fv_list; n->next!=fv; n=n->next );
	n->next = fv->next;
    }
    FontViewFree(fv);
}

static void FVB_ChangeDisplayBitmap(FontViewBase *fv, BDFFont *bdf) {
    fv->active_bitmap = bdf;
}

static void FVB_ShowFilled(FontViewBase *fv) {
    fv->active_bitmap = NULL;
}

static void FVB_ReattachCVs(SplineFont *old, SplineFont *new) {
  (void)old;
  (void)new;
}

static void FVB_DeselectAll(FontViewBase *fv) {
    memset(fv->selected,0,fv->map->encmax);
}

static void FVB_DisplayChar(FontViewBase *fv,int gid) {
  (void)fv;
  (void)gid;
}

static int SFB_CloseAllInstrs(SplineFont *sf) {
  (void)sf;
return( true );
}

struct fv_interface noui_fv = {
    FontViewBase_Create,
    _FontViewBaseCreate,
    FontViewBase_Close,
    FontViewBase_Free,
    FontViewBaseSetTitle,
    FontViewBaseSetTitles,
    FontViewBaseRefreshAll,
    FontViewBaseReformatOne,
    FontViewBaseReformatAll,
    FontViewBaseLayerChanged,
    FV_ToggleCharChanged,
    FontViewBaseWinInfo,
    FontIsActive,
    FVAny,
    FontViewBase_Append,
    FontOfFilename,
    FVExtraEncSlots,
    FVExtraEncSlots,
    FVB_ChangeDisplayBitmap,
    FVB_ShowFilled,
    FVB_ReattachCVs,
    FVB_DeselectAll,
    FVB_DisplayChar,
    FVB_DisplayChar,
    FVB_DisplayChar,
    SFB_CloseAllInstrs
};

struct fv_interface *fv_interface = &noui_fv;


/******************************************************************************/
static int NoGlyphs(struct metricsview *mv) {
  (void)mv;
return( 0 );
}

static SplineChar *Nothing(struct metricsview *mv, int i) {
  (void)mv;
  (void)i; 
return( NULL );
}

static void NoReKern(struct splinefont *sf) {
  (void)sf;
}

static void NoCloseAll(struct splinefont *sf) {
  (void)sf;
}

struct mv_interface noui_mv = {
    NoGlyphs,
    Nothing,
    NoReKern,
    NoCloseAll
};

struct mv_interface *mv_interface = &noui_mv;
