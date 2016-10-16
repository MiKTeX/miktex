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
#include "fontforge.h"
#include "edgelist2.h"

#ifdef HAVE_IEEEFP_H
# include <ieeefp.h>		/* Solaris defines isnan in ieeefp rather than math.h */
#endif

/* First thing we do is divide each spline into a set of sub-splines each of */
/*  which is monotonic in both x and y (always increasing or decreasing)     */
/* Then we compare each monotonic spline with every other one and see if they*/
/*  intersect.  If they do, split each up into sub-sub-segments and create an*/
/*  intersection point (note we need to be a little careful if an intersec-  */
/*  tion happens at an end point. We don't need to create a intersection for */
/*  two adjacent splines, there isn't a real intersection... but if a third  */
/*  spline crosses that point (or ends there) then all three (four) splines  */
/*  need to be joined into an intersection point)                            */
/* Nasty things happen if splines are coincident. They will almost never be  */
/*  perfectly coincident and will keep crossing and recrossing as rounding   */
/*  errors suggest one is before the other. Look for coincident splines and  */
/*  treat the places they start and stop being coincident as intersections   */
/*  then when we find needed splines below look for these guys and ignore    */
/*  recrossings of splines which are close together                          */
/* Figure out if each monotonic sub-spline is needed or not		     */
/*  (Note: It was tempting to split the bits up into real splines rather     */
/*   than keeping them as sub-sections of the original. Unfortunately this   */
/*   splitting introduced rounding errors which meant that we got more       */
/*   intersections, which meant that splines could be both needed and un.    */
/*   so I don't do that until later)					     */
/*  if the spline hasn't been tagged yet:				     */
/*   does the spline change greater in x or y?				     */
/*   draw a line parallel to the OTHER axis which hits our spline and doesn't*/
/*    hit any endpoints (or intersections, which are end points too now)     */
/*   count the winding number (as we do this we can mark other splines as    */
/*    needed or not) and figure out if our spline is needed		     */
/*  So run through the list of intersections				     */
/*	At an intersection there should be an even number of needed monos.   */
/*	Use this as the basis of a new splineset, trace it around until	     */
/*	  we get back to the start intersection (should happen)		     */
/*	  (Note: We may need to reverse a monotonic sub-spline or two)	     */
/*	  As we go, mark each monotonic as having been used		     */
/*  Keep doing this until all needed exits from all intersections have been  */
/*	used.								     */
/*  The free up our temporary data structures, merge in any open splinesets  */
/*	free the old closed splinesets					     */

// Frank recommends using the following macro whenever making changes
// to this code and capturing and diffing output in order to track changes
// in errors and reports.
// (The pointers tend to clutter the diff a bit.)
// #define FF_OVERLAP_VERBOSE

static char *glyphname=NULL;

static void SOError(const char *format,...) {
    va_list ap;
    va_start(ap,format);
    if ( glyphname==NULL )
	fprintf(stderr, "Internal Error (overlap): " );
    else
	fprintf(stderr, "Internal Error (overlap) in %s: ", glyphname );
    vfprintf(stderr,format,ap);
    va_end(ap);
}

#ifdef FF_OVERLAP_VERBOSE
#define ValidateMListTs_IF_VERBOSE(input) ValidateMListTs(input);
#else
#define ValidateMListTs_IF_VERBOSE(input) 
#endif

static void Validate(Monotonic *ms, Intersection *ilist) {
    MList *ml;
    int mcnt;

    while ( ilist!=NULL ) {
        // For each listed intersection, verify that each connected monotonic
        // starts or ends at the intersection (identified by pointer, not geography).
	for ( mcnt=0, ml=ilist->monos; ml!=NULL; ml=ml->next ) {
	    if ( ml->m->isneeded ) ++mcnt;
	    if ( ml->m->start!=ilist && ml->m->end!=ilist )
		SOError( "Intersection (%g,%g) not on a monotonic which should contain it.\n",
			(double) ilist->inter.x, (double) ilist->inter.y );
	}
	if ( mcnt&1 )
	    SOError( "Odd number of needed monotonic sections at intersection. (%g,%g)\n",
		    (double) ilist->inter.x,(double) ilist->inter.y );
	ilist = ilist->next;
    }

    while ( ms!=NULL ) {
        if ( ms->prev == NULL )
          SOError( "Open monotonic loop.\n" );
	else if ( ms->prev->end!=ms->start )
	    SOError( "Mismatched intersection.\n (%g,%g)->(%g,%g) ends at (%g,%g) while (%g,%g)->(%g,%g) starts at (%g,%g)\n",
		(double) ms->prev->s->from->me.x,(double) ms->prev->s->from->me.y,
		(double) ms->prev->s->to->me.x,(double) ms->prev->s->to->me.y,
		(double) (ms->prev->end!=NULL?ms->prev->end->inter.x:-999999), (double) (ms->prev->end!=NULL?ms->prev->end->inter.y:-999999),
		(double) ms->s->from->me.x,(double) ms->s->from->me.y,
		(double) ms->s->to->me.x,(double) ms->s->to->me.y,
		(double) (ms->start!=NULL?ms->start->inter.x:-999999), (double) (ms->start!=NULL?ms->start->inter.y:-999999) );
	ms = ms->linked;
    }
}

static Monotonic *SplineToMonotonic(Spline *s,extended startt,extended endt,
	Monotonic *last,int exclude) {
    Monotonic *m;
    BasePoint start, end;

    if ( startt==0 )
	start = s->from->me;
    else {
	start.x = ((s->splines[0].a*startt+s->splines[0].b)*startt+s->splines[0].c)*startt
		    + s->splines[0].d;
	start.y = ((s->splines[1].a*startt+s->splines[1].b)*startt+s->splines[1].c)*startt
		    + s->splines[1].d;
    }
    if ( endt==1.0 )
	end = s->to->me;
    else {
	end.x = ((s->splines[0].a*endt+s->splines[0].b)*endt+s->splines[0].c)*endt
		    + s->splines[0].d;
	end.y = ((s->splines[1].a*endt+s->splines[1].b)*endt+s->splines[1].c)*endt
		    + s->splines[1].d;
    }
    if ( ( (real) (((start.x+end.x)/2)==start.x || (real) ((start.x+end.x)/2)==end.x) &&
	    (real) (((start.y+end.y)/2)==start.y || (real) ((start.y+end.y)/2)==end.y) ) ||
         (endt <= startt) || Within4RoundingErrors(startt, endt)) {
	/* The distance between the two extrema is so small */
	/*  as to be unobservable. In other words we'd end up with a zero*/
	/*  length spline */
	if ( endt==1.0 && last!=NULL && last->s==s )
	    last->tend = endt;
return( last );
    }

    m = chunkalloc(sizeof(Monotonic));
    m->s = s;
    m->tstart = startt;
    m->tend = endt;
#ifdef FF_RELATIONAL_GEOM
    m->otstart = startt;
    m->otend = endt;
#endif
    m->exclude = exclude;

    if ( end.x>start.x ) {
	m->xup = true;
	m->b.minx = start.x;
	m->b.maxx = end.x;
    } else {
	m->b.minx = end.x;
	m->b.maxx = start.x;
    }
    if ( end.y>start.y ) {
	m->yup = true;
	m->b.miny = start.y;
	m->b.maxy = end.y;
    } else {
	m->b.miny = end.y;
	m->b.maxy = start.y;
    }

    if ( last!=NULL ) {
    // Validate(last, NULL);
	last->next = m;
	last->linked = m;
	m->prev = last;
    // Validate(last, NULL);
    }
return( m );
}


static int BpSame(BasePoint *bp1, BasePoint *bp2) {
    BasePoint mid;

    mid.x = (bp1->x+bp2->x)/2; mid.y = (bp1->y+bp2->y)/2;
    if ( (bp1->x==mid.x || bp2->x==mid.x) &&
	    (bp1->y==mid.y || bp2->y==mid.y))
return( true );

return( false );
}

static int SSRmNullSplines(SplineSet *spl) {
    Spline *s, *first, *next;

    first = NULL;
    for ( s=spl->first->next ; s!=first; s=next ) {
	next = s->to->next;
	if ( ((s->splines[0].a>-.01 && s->splines[0].a<.01 &&
		s->splines[0].b>-.01 && s->splines[0].b<.01 &&
		s->splines[1].a>-.01 && s->splines[1].a<.01 &&
		s->splines[1].b>-.01 && s->splines[1].b<.01) ||
		/* That describes a null spline (a line between the same end-point) */
	       RealNear((s->from->nextcp.x-s->from->me.x)*(s->to->me.y-s->to->prevcp.y)-
			(s->from->nextcp.y-s->from->me.y)*(s->to->me.x-s->to->prevcp.x),0)) &&
		/* And the above describes a point with a spline between it */
		/*  and itself where the spline covers no area (the two cps */
		/*  point in the same direction) */
		BpSame(&s->from->me,&s->to->me)) {
	    if ( next==s )
return( true );
	    if ( next->from->selected ) s->from->selected = true;
	    s->from->next = next;
	    s->from->nextcp = next->from->nextcp;
	    s->from->nonextcp = next->from->nonextcp;
	    s->from->nextcpdef = next->from->nextcpdef;
	    SplinePointFree(next->from);
	    if ( spl->first==next->from )
		spl->last = spl->first = s->from;
	    next->from = s->from;
	    SplineFree(s);
	} else {
	    if ( first==NULL )
		first = s;
	}
    }
return( false );
}

static Monotonic *SSToMContour(SplineSet *spl, Monotonic *start,
	Monotonic **end, enum overlap_type ot) {
    extended ts[4];
    Spline *first, *s;
    Monotonic *head=NULL, *last=NULL;
    int cnt, i, selected = false;
    extended lastt;

    if ( spl->first->prev==NULL )
return( start );		/* Open contours have no interior, ignore 'em */
    if ( spl->first->prev->from==spl->first &&
	    spl->first->noprevcp && spl->first->nonextcp )
return( start );		/* Let's just remove single points */

	if ( ot==over_rmselected || ot==over_intersel || ot==over_fisel ) {
	    if ( !selected )
	    	return( start );
	}

    /* We blow up on zero length splines. And a zero length contour is nasty */
    if ( SSRmNullSplines(spl))
return( start );

    first = NULL;
    for ( s=spl->first->next; s!=first; s=s->to->next ) {
	if ( first==NULL ) first = s;
	cnt = Spline2DFindExtrema(s,ts);
	lastt = 0;
	for ( i=0; i<cnt; ++i ) {
	    last = SplineToMonotonic(s,lastt,ts[i],last,selected);
	    if ( head==NULL ) head = last;
	    lastt=ts[i];
	}
	if ( lastt!=1.0 ) {
	    last = SplineToMonotonic(s,lastt,1.0,last,selected);
	    if ( head==NULL ) head = last;
	}
    }
    head->prev = last;
    last->next = head;
    if ( start==NULL )
	start = head;
    else
	(*end)->linked = head;
    *end = last;
    Validate(start, NULL);
return( start );
}

Monotonic *SSsToMContours(SplineSet *spl, enum overlap_type ot) {
    Monotonic *head=NULL, *last = NULL;

    while ( spl!=NULL ) {
	if ( spl->first->prev!=NULL )
	    head = SSToMContour(spl,head,&last,ot);
	spl = spl->next;
    }
return( head );
}

/* An IEEE double has 52 bits of precision. So one unit of rounding error will be */
/*  the number divided by 2^51 */
# define BR_RE_Factor	(1024.0*1024.0*1024.0*1024.0*1024.0*2.0)
/* But that's not going to work near 0, so, since the t values we care about */
/*  are [0,1], let's use 1.0/D_RE_Factor */

#ifdef FF_OVERLAP_VERBOSE
#define FF_DUMP_MONOTONIC_IF_VERBOSE(m) DumpMonotonic(m);
#else
#define FF_DUMP_MONOTONIC_IF_VERBOSE(m) 
#endif

static int mcmp(const void *_p1, const void *_p2) {
    const Monotonic * const *mpt1 = _p1, * const *mpt2 = _p2;
    if ( (*mpt1)->other>(*mpt2)->other )
return( 1 );
    else if ( (*mpt1)->other<(*mpt2)->other )
return( -1 );

return( 0 );
}

int CheckMonotonicClosed(struct monotonic *ms) {
  struct monotonic * current;
  if (ms == NULL) return 0;
  current = ms->next;
  while (current != ms && current != NULL) {
    current = current->next;    
  }
  if (current == NULL) return 0;
  return 1;
}

int MonotonicFindAt(Monotonic *ms,int which, extended test, Monotonic **space ) {
    /* Find all monotonic sections which intersect the line (x,y)[which] == test */
    /*  find the value of the other coord on that line */
    /*  Order them (by the other coord) */
    /*  then run along that line figuring out which monotonics are needed */
    extended t;
    Monotonic *m, *mm;
    int i, j, k, cnt;
    int nw = !which;

    for ( m=ms, i=0; m!=NULL; m=m->linked ) {
        if (CheckMonotonicClosed(m) == 0) continue; // Open monotonics break things.
	if (( which==0 && test >= m->b.minx && test <= m->b.maxx ) ||
		( which==1 && test >= m->b.miny && test <= m->b.maxy )) {
	    /* Lines parallel to the direction we are testing just get in the */
	    /*  way and don't add any useful info */
	    if ( m->s->knownlinear &&
		    (( which==1 && m->s->from->me.y==m->s->to->me.y ) ||
			(which==0 && m->s->from->me.x==m->s->to->me.x)))
    continue;
	    t = IterateSplineSolveFixup(&m->s->splines[which],m->tstart,m->tend,test);
	    if ( t==-1 ) {
		if ( which==0 ) {
		    if (( test-m->b.minx > m->b.maxx-test && m->xup ) ||
			    ( test-m->b.minx < m->b.maxx-test && !m->xup ))
			t = m->tstart;
		    else
			t = m->tend;
		} else {
		    if (( test-m->b.miny > m->b.maxy-test && m->yup ) ||
			    ( test-m->b.miny < m->b.maxy-test && !m->yup ))
			t = m->tstart;
		    else
			t = m->tend;
		}
	    }
	    m->t = t;
	    if ( t==m->tend ) t -= (m->tend-m->tstart)/100;
	    else if ( t==m->tstart ) t += (m->tend-m->tstart)/100;
	    m->other = ((m->s->splines[nw].a*t+m->s->splines[nw].b)*t+
		    m->s->splines[nw].c)*t+m->s->splines[nw].d;
	    space[i++] = m;
	}
    }
    cnt = i;

    /* Things get a little tricky at end-points */
    for ( i=0; i<cnt; ++i ) {
	m = space[i];
	if ( m->t==m->tend ) {
	    /* Ignore horizontal/vertical lines (as appropriate) */
	    for ( mm=m->next; mm!=m && mm !=NULL; mm=mm->next ) {
		if ( !mm->s->knownlinear )
	    break;
		if (( which==1 && mm->s->from->me.y!=m->s->to->me.y ) ||
			(which==0 && mm->s->from->me.x!=m->s->to->me.x))
	    break;
	    }
	} else if ( m->t==m->tstart ) {
	    for ( mm=m->prev; mm!=m && mm !=NULL; mm=mm->prev ) {
		if ( !mm->s->knownlinear )
	    break;
		if (( which==1 && mm->s->from->me.y!=m->s->to->me.y ) ||
			(which==0 && mm->s->from->me.x!=m->s->to->me.x))
	    break;
	    }
	} else
    break;
	/* If the next monotonic continues in the same direction, and we found*/
	/*  it too, then don't count both. They represent the same intersect */
	/* If they are in oposite directions then they cancel each other out */
	/*  and that is correct */
	if ( mm!=m &&	/* Should always be true */
		(&mm->xup)[which]==(&m->xup)[which] ) {
	    for ( j=cnt-1; j>=0; --j )
		if ( space[j]==mm )
	    break;
	    if ( j!=-1 ) {
		/* remove mm */
		for ( k=j+1; k<cnt; ++k )
		    space[k-1] = space[k];
		--cnt;
		if ( i>j ) --i;
	    }
	}
    }

    space[cnt] = NULL; space[cnt+1] = NULL;
    qsort(space,cnt,sizeof(Monotonic *),mcmp);
return(cnt);
}

void FreeMonotonics(Monotonic *m) {
    Monotonic *next;

    while ( m!=NULL ) {
	next = m->linked;
	chunkfree(m,sizeof(*m));
	m = next;
    }
}
