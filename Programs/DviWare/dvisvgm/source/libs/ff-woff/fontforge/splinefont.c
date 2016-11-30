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

/* Use URW 4 letter abbreviations */
const char *knownweights[] = { "Demi", "Bold", "Regu", "Medi", "Book", "Thin",
	"Ligh", "Heav", "Blac", "Ultr", "Nord", "Norm", "Gras", "Stan", "Halb",
	"Fett", "Mage", "Mitt", "Buch", NULL };
const char *realweights[] = { "Demi", "Bold", "Regular", "Medium", "Book", "Thin",
	"Light", "Heavy", "Black", "Ultra", "Nord", "Normal", "Gras", "Standard", "Halbfett",
	"Fett", "Mager", "Mittel", "Buchschrift", NULL};

static const char *modifierlist[] = { "Ital", "Obli", "Kursive", "Cursive", "Slanted",
	"Expa", "Cond", NULL };
static const char *modifierlistfull[] = { "Italic", "Oblique", "Kursive", "Cursive", "Slanted",
    "Expanded", "Condensed", NULL };
static const char **mods[] = { knownweights, modifierlist, NULL };
static const char **fullmods[] = { realweights, modifierlistfull, NULL };

const char *_GetModifiers(const char *fontname, const char *familyname, const char *weight) {
    const char *pt, *fpt;
    static char space[20];
    int i, j;

    /* URW fontnames don't match the familyname */
    /* "NimbusSanL-Regu" vs "Nimbus Sans L" (note "San" vs "Sans") */
    /* so look for a '-' if there is one and use that as the break point... */

    if ( (fpt=strchr(fontname,'-'))!=NULL ) {
	++fpt;
	if ( *fpt=='\0' )
	    fpt = NULL;
    } else if ( familyname!=NULL ) {
	for ( pt = fontname, fpt=familyname; *fpt!='\0' && *pt!='\0'; ) {
	    if ( *fpt == *pt ) {
		++fpt; ++pt;
	    } else if ( *fpt==' ' )
		++fpt;
	    else if ( *pt==' ' )
		++pt;
	    else if ( *fpt=='a' || *fpt=='e' || *fpt=='i' || *fpt=='o' || *fpt=='u' )
		++fpt;	/* allow vowels to be omitted from family when in fontname */
	    else
	break;
	}
	if ( *fpt=='\0' && *pt!='\0' )
	    fpt = pt;
	else
	    fpt = NULL;
    }

    if ( fpt == NULL ) {
	for ( i=0; mods[i]!=NULL; ++i ) for ( j=0; mods[i][j]!=NULL; ++j ) {
	    pt = strstr(fontname,mods[i][j]);
	    if ( pt!=NULL && (fpt==NULL || pt<fpt))
		fpt = pt;
	}
    }
    if ( fpt!=NULL ) {
	for ( i=0; mods[i]!=NULL; ++i ) for ( j=0; mods[i][j]!=NULL; ++j ) {
	    if ( strcmp(fpt,mods[i][j])==0 ) {
		strncpy(space,fullmods[i][j],sizeof(space)-1);
return(space);
	    }
	}
	if ( strcmp(fpt,"BoldItal")==0 )
return( "BoldItalic" );
	else if ( strcmp(fpt,"BoldObli")==0 )
return( "BoldOblique" );

return( fpt );
    }

return( weight==NULL || *weight=='\0' ? "Regular": weight );
}

const char *SFGetModifiers(const SplineFont *sf) {
return( _GetModifiers(sf->fontname,sf->familyname,sf->weight));
}

enum flatness { mt_flat, mt_round, mt_pointy, mt_unknown };

static bigreal SPLMaxHeight(SplineSet *spl, enum flatness *isflat) {
    enum flatness f = mt_unknown;
    bigreal max = -1.0e23;
    Spline *s, *first;
    extended ts[2];
    int i;

    for ( ; spl!=NULL; spl=spl->next ) {
	first = NULL;
	for ( s = spl->first->next; s!=first && s!=NULL; s=s->to->next ) {
	    if ( first==NULL ) first = s;
	    if ( s->from->me.y >= max ||
		    s->to->me.y >= max ||
		    s->from->nextcp.y > max ||
		    s->to->prevcp.y > max ) {
		if ( !s->knownlinear ) {
		    if ( s->from->me.y > max ) {
			f = mt_round;
			max = s->from->me.y;
		    }
		    if ( s->to->me.y > max ) {
			f = mt_round;
			max = s->to->me.y;
		    }
		    SplineFindExtrema(&s->splines[1],&ts[0],&ts[1]);
		    for ( i=0; i<2; ++i ) if ( ts[i]!=-1 ) {
			bigreal y = ((s->splines[1].a*ts[i]+s->splines[1].b)*ts[i]+s->splines[1].c)*ts[i]+s->splines[1].d;
			if ( y>max ) {
			    f = mt_round;
			    max = y;
			}
		    }
		} else if ( s->from->me.y == s->to->me.y ) {
		    if ( s->from->me.y >= max ) {
			max = s->from->me.y;
			f = mt_flat;
		    }
		} else {
		    if ( s->from->me.y > max ) {
			f = mt_pointy;
			max = s->from->me.y;
		    }
		    if ( s->to->me.y > max ) {
			f = mt_pointy;
			max = s->to->me.y;
		    }
		}
	    }
	}
    }
    *isflat = f;
return( max );
}

static bigreal SCMaxHeight(SplineChar *sc, int layer, enum flatness *isflat) {
    /* Find the max height of this layer of the glyph. Also find whether that */
    /* max is flat (as in "z", curved as in "o" or pointy as in "A") */
    enum flatness f = mt_unknown, curf;
    bigreal max = -1.0e23, test;
    RefChar *r;

    max = SPLMaxHeight(sc->layers[layer].splines,&curf);
    f = curf;
    for ( r = sc->layers[layer].refs; r!=NULL; r=r->next ) {
	test = SPLMaxHeight(r->layers[0].splines,&curf);
	if ( test>max || (test==max && curf==mt_flat)) {
	    max = test;
	    f = curf;
	}
    }
    *isflat = f;
return( max );
}

static bigreal SPLMinHeight(SplineSet *spl, enum flatness *isflat) {
    enum flatness f = mt_unknown;
    bigreal min = 1.0e23;
    Spline *s, *first;
    extended ts[2];
    int i;

    for ( ; spl!=NULL; spl=spl->next ) {
	first = NULL;
	for ( s = spl->first->next; s!=first && s!=NULL; s=s->to->next ) {
	    if ( first==NULL ) first = s;
	    if ( s->from->me.y <= min ||
		    s->to->me.y <= min ||
		    s->from->nextcp.y < min ||
		    s->to->prevcp.y < min ) {
		if ( !s->knownlinear ) {
		    if ( s->from->me.y < min ) {
			f = mt_round;
			min = s->from->me.y;
		    }
		    if ( s->to->me.y < min ) {
			f = mt_round;
			min = s->to->me.y;
		    }
		    SplineFindExtrema(&s->splines[1],&ts[0],&ts[1]);
		    for ( i=0; i<2; ++i ) if ( ts[i]!=-1 ) {
			bigreal y = ((s->splines[1].a*ts[i]+s->splines[1].b)*ts[i]+s->splines[1].c)*ts[i]+s->splines[1].d;
			if ( y<min ) {
			    f = mt_round;
			    min = y;
			}
		    }
		} else if ( s->from->me.y == s->to->me.y ) {
		    if ( s->from->me.y <= min ) {
			min = s->from->me.y;
			f = mt_flat;
		    }
		} else {
		    if ( s->from->me.y < min ) {
			f = mt_pointy;
			min = s->from->me.y;
		    }
		    if ( s->to->me.y < min ) {
			f = mt_pointy;
			min = s->to->me.y;
		    }
		}
	    }
	}
    }
    *isflat = f;
return( min );
}

static bigreal SCMinHeight(SplineChar *sc, int layer, enum flatness *isflat) {
    /* Find the min height of this layer of the glyph. Also find whether that */
    /* min is flat (as in "z", curved as in "o" or pointy as in "A") */
    enum flatness f = mt_unknown, curf;
    bigreal min = 1.0e23, test;
    RefChar *r;

    min = SPLMinHeight(sc->layers[layer].splines,&curf);
    f = curf;
    for ( r = sc->layers[layer].refs; r!=NULL; r=r->next ) {
	test = SPLMinHeight(r->layers[0].splines,&curf);
	if ( test<min || (test==min && curf==mt_flat)) {
	    min = test;
	    f = curf;
	}
    }
    *isflat = f;
return( min );
}

#define RANGE	0x40ffffff

struct dimcnt { bigreal pos; int cnt; };

static int dclist_insert( struct dimcnt *arr, int cnt, bigreal val ) {
    int i;

    for ( i=0; i<cnt; ++i ) {
	if ( arr[i].pos == val ) {
	    ++arr[i].cnt;
return( cnt );
	}
    }
    arr[i].pos = val;
    arr[i].cnt = 1;
return( i+1 );
}

static bigreal SFStandardHeight(SplineFont *sf, int layer, int do_max, unichar_t *list) {
    struct dimcnt flats[200], curves[200];
    bigreal test;
    enum flatness curf;
    int fcnt=0, ccnt=0, cnt, tot, i, useit;
    unichar_t ch, top;
    bigreal result, bestheight, bestdiff, diff, val;
    char *blues, *end;

    while ( *list ) {
	ch = top = *list;
	if ( list[1]==RANGE && list[2]!=0 ) {
	    list += 2;
	    top = *list;
	}
	for ( ; ch<=top; ++ch ) {
	    SplineChar *sc = SFGetChar(sf,ch,NULL);
	    if ( sc!=NULL ) {
		if ( do_max )
		    test = SCMaxHeight(sc, layer, &curf );
		else
		    test = SCMinHeight(sc, layer, &curf );
		if ( curf==mt_flat )
		    fcnt = dclist_insert(flats, fcnt, test);
		else if ( curf!=mt_unknown )
		    ccnt = dclist_insert(curves, ccnt, test);
	    }
	}
	++list;
    }

    /* All flat surfaces at tops of glyphs are at the same level */
    if ( fcnt==1 )
	result = flats[0].pos;
    else if ( fcnt>1 ) {
	cnt = 0;
	for ( i=0; i<fcnt; ++i ) {
	    if ( flats[i].cnt>cnt )
		cnt = flats[i].cnt;
	}
	test = 0;
	tot = 0;
	/* find the mode. If multiple values have the same high count, average them */
	for ( i=0; i<fcnt; ++i ) {
	    if ( flats[i].cnt==cnt ) {
		test += flats[i].pos;
		++tot;
	    }
	}
	result = test/tot;
    } else if ( ccnt==0 )
return( do_max ? -1e23 : 1e23 );		/* We didn't find any glyphs */
    else {
	/* Italic fonts will often have no flat surfaces for x-height just wavies */
	test = 0;
	tot = 0;
	/* find the mean */
	for ( i=0; i<ccnt; ++i ) {
	    test += curves[i].pos;
	    ++tot;
	}
	result = test/tot;
    }

    /* Do we have a BlueValues entry? */
    /* If so, snap height to the closest alignment zone (bottom of the zone) */
    if ( sf->private!=NULL && (blues = PSDictHasEntry(sf->private,do_max ? "BlueValues" : "OtherBlues"))!=NULL ) {
	while ( *blues==' ' || *blues=='[' ) ++blues;
	/* Must get at least this close, else we'll just use what we found */
	bestheight = result; bestdiff = (sf->ascent+sf->descent)/100.0;
	useit = true;
	while ( *blues!='\0' && *blues!=']' ) {
	    val = strtod(blues,&end);
	    if ( blues==end )
	break;
	    blues = end;
	    while ( *blues==' ' ) ++blues;
	    if ( useit ) {
		if ( (diff = val-result)<0 ) diff = -diff;
		if ( diff<bestdiff ) {
		    bestheight = val;
		    bestdiff = diff;
		}
	    }
	    useit = !useit;	/* Only interested in every other BV entry */
	}
	result = bestheight;
    }
return( result );
}

static unichar_t capheight_str[] = { 'A', RANGE, 'Z',
    0x391, RANGE, 0x3a9,
    0x402, 0x404, 0x405, 0x406, 0x408, RANGE, 0x40b, 0x40f, RANGE, 0x418, 0x41a, 0x42f,
    0 };
static unichar_t xheight_str[] = { 'a', 'c', 'e', 'g', 'm', 'n', 'o', 'p', 'q', 'r', 's', 'u', 'v', 'w', 'x', 'y', 'z', 0x131,
    0x3b3, 0x3b9, 0x3ba, 0x3bc, 0x3bd, 0x3c0, 0x3c3, 0x3c4, 0x3c5, 0x3c7, 0x3c8, 0x3c9,
    0x432, 0x433, 0x438, 0x43a, RANGE, 0x43f, 0x442, 0x443, 0x445, 0x44c,0x44f, 0x459, 0x45a,
    0 };

bigreal SFCapHeight(SplineFont *sf, int layer, int return_error) {
    bigreal result = SFStandardHeight(sf,layer,true,capheight_str);

    if ( result==-1e23 && !return_error )
	result = (8*sf->ascent)/10;
return( result );
}

bigreal SFXHeight(SplineFont *sf, int layer, int return_error) {
    bigreal result = SFStandardHeight(sf,layer,true,xheight_str);

    if ( result==-1e23 && !return_error )
	result = (6*sf->ascent)/10;
return( result );
}

void SPLFirstVisitSplines( SplinePoint* splfirst, SPLFirstVisitSplinesVisitor f, void* udata )
{
    Spline *spline=0;
    Spline *first=0;
    Spline *next=0;

    if ( splfirst!=NULL )
    {
	first = NULL;
	for ( spline = splfirst->next; spline!=NULL && spline!=first; spline = next )
	{
	    next = spline->to->next;

	    // callback
	    f( splfirst, spline, udata );

	    if ( first==NULL )
	    {
		first = spline;
	    }
	}
    }
}

typedef struct SPLFirstVisitorFoundSoughtXYDataS
{
    int use_x;
    int use_y;
    real x;
    real y;

    // outputs
    int found;
    Spline* spline;
    SplinePoint* sp;

} SPLFirstVisitorFoundSoughtXYData;

static void SPLFirstVisitorFoundSoughtXY(SplinePoint* splfirst, Spline* spline, void* udata )
{
    SPLFirstVisitorFoundSoughtXYData* d = (SPLFirstVisitorFoundSoughtXYData*)udata;
    int found = 0;

    if( d->found )
	return;

    // printf("SPLFirstVisitorFoundSoughtXY() %f %f %f\n", d->x, spline->from->me.x, spline->to->me.x );
    if( d->use_x )
    {
	if( spline->from->me.x == d->x )
	{
	    found = 1;
	    d->spline = spline;
	    d->sp = spline->from;
	}

	if( spline->to->me.x == d->x )
	{
	    found = 1;
	    d->spline = spline;
	    d->sp = spline->to;
	}
    }
    if( d->use_x && found && d->use_y )
    {
	if( d->sp->me.y != d->y )
	{
	    found = 0;
	}
    }
    else if( d->use_y )
    {
	if( spline->from->me.y == d->y )
	{
	    found = 1;
	    d->spline = spline;
	    d->sp = spline->from;
	}

	if( spline->to->me.y == d->y )
	{
	    found = 1;
	    d->spline = spline;
	    d->sp = spline->to;
	}
    }

    if( found )
    {
	d->found = found;
	d->spline = spline;
    }
    else
    {
	d->sp = 0;
    }
}

SplinePoint* SplinePointListContainsPointAtX( SplinePointList* container, real x )
{
    SplinePointList *spl;
    for ( spl = container; spl!=NULL; spl = spl->next )
    {
	SPLFirstVisitorFoundSoughtXYData d;
	d.use_x  = 1;
	d.use_y  = 0;
	d.x      = x;
	d.y      = 0;
	d.found  = 0;
	SPLFirstVisitSplines( spl->first, SPLFirstVisitorFoundSoughtXY, &d );
	if( d.found )
	    return d.sp;
    }
    return 0;
}
