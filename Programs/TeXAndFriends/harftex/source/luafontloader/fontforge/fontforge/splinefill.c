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
#include <string.h>
#include <ustring.h>
#include <math.h>
#include "splinefont.h"
#include "edgelist.h"

static void HintsFree(Hints *h) {
    Hints *hnext;
    for ( ; h!=NULL; h = hnext ) {
	hnext = h->next;
	free(h);
    }
}

static void _FreeEdgeList(EdgeList *es) {
    int i;

    /* edges will be NULL if the user tries to make an enormous bitmap */
    /*  if the linear size is bigger than several thousand, we just */
    /*  ignore the request */
    if ( es->edges!=NULL ) {
	for ( i=0; i<es->cnt; ++i ) {
	    Edge *e, *next;
	    for ( e = es->edges[i]; e!=NULL; e = next ) {
		next = e->esnext;
		free(e);
	    }
	    es->edges[i] = NULL;
	}
    }
}

void FreeEdges(EdgeList *es) {
    _FreeEdgeList(es);
    free(es->edges);
    free(es->interesting);
    HintsFree(es->hhints);
    HintsFree(es->vhints);
}

extended IterateSplineSolve(const Spline1D *sp, extended tmin, extended tmax,
	extended sought,double err) {
    extended t, low, high, test;
    Spline1D temp;
    int cnt;

    /* Now the closed form CubicSolver can have rounding errors so if we know */
    /*  the spline to be monotonic, an iterative approach is more accurate */

    temp = *sp;
    temp.d -= sought;

    if ( temp.a==0 && temp.b==0 && temp.c!=0 ) {
	t = -temp.d/(extended) temp.c;
	if ( t<0 || t>1 )
return( -1 );
return( t );
    }

    low = ((temp.a*tmin+temp.b)*tmin+temp.c)*tmin+temp.d;
    high = ((temp.a*tmax+temp.b)*tmax+temp.c)*tmax+temp.d;
    if ( low<err && low>-err )
return(tmin);
    if ( high<err && high>-err )
return(tmax);
    if (( low<0 && high>0 ) ||
	    ( low>0 && high<0 )) {
	
	for ( cnt=0; cnt<1000; ++cnt ) {	/* Avoid impossible error limits */
	    t = (tmax+tmin)/2;
	    test = ((temp.a*t+temp.b)*t+temp.c)*t+temp.d;
	    if ( test>-err && test<err )
return( t );
	    if ( (low<0 && test<0) || (low>0 && test>0) )
		tmin=t;
	    else
		tmax = t;
	}
return( (tmax+tmin)/2 );	
    }
return( -1 );
}

double TOfNextMajor(Edge *e, EdgeList *es, double sought_m ) {
    /* We want to find t so that Mspline(t) = sought_m */
    /*  the curve is monotonic */
    Spline1D *msp = &e->spline->splines[es->major];
    double new_t;

    if ( es->is_overlap ) {

	/* if we've adjusted the height then we won't be able to find it restricting */
	/*  t between [0,1] as we do. So it's a special case. (this is to handle */
	/*  hstem hints) */
	if ( e->max_adjusted && sought_m==e->mmax ) {
	    e->m_cur = sought_m;
return( e->up?1.0:0.0 );
	}

	new_t = IterateSplineSolve(msp,e->t_mmin,e->t_mmax,(sought_m+es->mmin)/es->scale,.001);
	if ( new_t==-1 )
	    IError( "No Solution");
	e->m_cur = (((msp->a*new_t + msp->b)*new_t+msp->c)*new_t + msp->d)*es->scale - es->mmin;
return( new_t );
    } else {
	Spline *sp = e->spline;

	if ( sp->islinear ) {
	    new_t = e->t_cur + (sought_m-e->m_cur)/(es->scale * msp->c);
	    e->m_cur = (msp->c*new_t + msp->d)*es->scale - es->mmin;
return( new_t );
	}
	/* if we have a spline that is nearly horizontal at its max. endpoint */
	/*  then finding A value of t for which y has the right value isn't good */
	/*  enough (at least not when finding intersections) */
	if ( sought_m+1>e->mmax ) {
	    e->m_cur = e->mmax;
return( e->t_mmax );
	}

	/* if we've adjusted the height then we won't be able to find it restricting */
	/*  t between [0,1] as we do. So it's a special case. (this is to handle */
	/*  hstem hints) */
	if ( e->max_adjusted && sought_m==e->mmax ) {
	    e->m_cur = sought_m;
return( e->up?1.0:0.0 );
	}
	new_t = IterateSplineSolve(msp,e->t_mmin,e->t_mmax,(sought_m+es->mmin)/es->scale,.001);
	if ( new_t==-1 )
	    IError( "No Solution");
	e->m_cur = (((msp->a*new_t + msp->b)*new_t+msp->c)*new_t + msp->d)*es->scale - es->mmin;
return( new_t );
    }
}

static int SlopeLess(Edge *e, Edge *p, int other) {
    Spline1D *osp = &e->spline->splines[other];
    Spline1D *psp = &p->spline->splines[other];
    Spline1D *msp = &e->spline->splines[!other];
    Spline1D *qsp = &p->spline->splines[!other];
    real os = (3*osp->a*e->t_cur+2*osp->b)*e->t_cur+osp->c,
	   ps = (3*psp->a*p->t_cur+2*psp->b)*p->t_cur+psp->c;
    real ms = (3*msp->a*e->t_cur+2*msp->b)*e->t_cur+msp->c,
	   qs = (3*qsp->a*p->t_cur+2*qsp->b)*p->t_cur+qsp->c;
    if ( ms<.0001 && ms>-.0001 ) ms = 0;
    if ( qs<.0001 && qs>-.0001 ) qs = 0;
    if ( qs==0 ) {
	if ( p->t_cur==1 ) {
	   qs = (3*qsp->a*.9999+2*qsp->b)*.9999+qsp->c;
	   ps = (3*psp->a*.9999+2*psp->b)*.9999+psp->c;
       } else {
	   qs = (3*qsp->a*(p->t_cur+.0001)+2*qsp->b)*(p->t_cur+.0001)+qsp->c;
	   ps = (3*psp->a*(p->t_cur+.0001)+2*psp->b)*(p->t_cur+.0001)+psp->c;
       }
    }
    if ( ms==0 ) {
	if ( e->t_cur==1 ) {
	    ms = (3*msp->a*.9999+2*msp->b)*.9999+msp->c;
	    os = (3*osp->a*.9999+2*osp->b)*.9999+osp->c;
	} else {
	    ms = (3*msp->a*(e->t_cur+.0001)+2*msp->b)*(e->t_cur+.0001)+msp->c;
	    os = (3*osp->a*(e->t_cur+.0001)+2*osp->b)*(e->t_cur+.0001)+osp->c;
	}
    }
    if ( e->t_cur-e->tmin > e->tmax-e->t_cur ) { os = -os; ms = -ms; }
    if ( p->t_cur-p->tmin > p->tmax-p->t_cur ) { ps = -ps; qs = -qs; }
    if ( ms!=0 && qs!=0 ) { os /= ms; ps /= qs; }
    else if ( ms==0 && qs==0 ) /* Do Nothing */;
    else if ( (ms==0 && os>0) || (qs==0 && ps<0) )		/* Does this make sense? */
return( false );
    else if ( (ms==0 && os<0) || (qs==0 && ps>0) )		/* Does this make sense? */
return( true );

    if ( os==ps || ms==0 || qs==0 )
return( e->o_mmax<p->o_mmax );

return( os<ps );
}

static void AddEdge(EdgeList *es, Spline *sp, real tmin, real tmax ) {
    Edge *e, *pr;
    real m1, m2;
    int mpos;
    Hints *hint;
    Spline1D *msp = &sp->splines[es->major], *osp = &sp->splines[es->other];

    e = gcalloc(1,sizeof(Edge));
    e->spline = sp;

    m1 = ( ((msp->a*tmin+msp->b)*tmin+msp->c)*tmin + msp->d ) * es->scale;
    m2 = ( ((msp->a*tmax+msp->b)*tmax+msp->c)*tmax + msp->d ) * es->scale;
    if ( m1>m2 ) {
	e->mmin = m2;
	e->t_mmin = tmax;
	e->mmax = m1;
	e->t_mmax = tmin;
	e->up = false;
    } else {
	e->mmax = m2;
	e->t_mmax = tmax;
	e->mmin = m1;
	e->t_mmin = tmin;
	e->up = true;
    }
    if ( RealNear(e->mmin,es->mmin)) e->mmin = es->mmin;
    e->o_mmin = ( ((osp->a*e->t_mmin+osp->b)*e->t_mmin+osp->c)*e->t_mmin + osp->d ) * es->scale;
    e->o_mmax = ( ((osp->a*e->t_mmax+osp->b)*e->t_mmax+osp->c)*e->t_mmax + osp->d ) * es->scale;
    e->mmin -= es->mmin; e->mmax -= es->mmin;
    e->t_cur = e->t_mmin;
    e->o_cur = e->o_mmin;
    e->m_cur = e->mmin;
    e->last_opos = e->last_mpos = -2;
    e->tmin = tmin; e->tmax = tmax;

    if ( e->mmin<0 || e->mmin>=e->mmax ) {
	/*IError("Probably not serious, but we've got a zero length spline in AddEdge in %s",es->sc==NULL?<nameless>:es->sc->name);*/
	free(e);
return;
    }

    if ( es->sc!=NULL ) for ( hint=es->hhints; hint!=NULL; hint=hint->next ) {
	if ( hint->adjustb ) {
	    if ( e->m_cur>hint->b1 && e->m_cur<hint->b2 ) {
		e->m_cur = e->mmin = hint->ab;
		e->min_adjusted = true;
	    } else if ( e->mmax>hint->b1 && e->mmax<hint->b2 ) {
		e->mmax = hint->ab;
		e->max_adjusted = true;
	    }
	} else if ( hint->adjuste ) {
	    if ( e->m_cur>hint->e1 && e->m_cur<hint->e2 ) {
		e->m_cur = e->mmin = hint->ae;
		e->min_adjusted = true;
	    } else if ( e->mmax>hint->e1 && e->mmax<hint->e2 ) {
		e->mmax = hint->ae;
		e->max_adjusted = true;
	    }
	}
    }

    mpos = (int) ceil(e->m_cur);
    if ( mpos>e->mmax || mpos>=es->cnt ) {
	free(e);
return;
    }

    if ( e->m_cur!=ceil(e->m_cur) ) {
	/* bring the new edge up to its first scan line */
	e->t_cur = TOfNextMajor(e,es,ceil(e->m_cur));
	e->o_cur = ( ((osp->a*e->t_cur+osp->b)*e->t_cur+osp->c)*e->t_cur + osp->d ) * es->scale;
    }

    e->before = es->last;
    if ( es->last!=NULL )
	es->last->after = e;
    if ( es->last==NULL )
	es->splinesetfirst = e;
    es->last = e;

    if ( es->edges[mpos]==NULL || e->o_cur<es->edges[mpos]->o_cur ||
	    (e->o_cur==es->edges[mpos]->o_cur && SlopeLess(e,es->edges[mpos],es->other))) {
	e->esnext = es->edges[mpos];
	es->edges[mpos] = e;
    } else {
	for ( pr=es->edges[mpos]; pr->esnext!=NULL && pr->esnext->o_cur<e->o_cur ;
		pr = pr->esnext );
	/* When two splines share a vertex which is a local minimum, then */
	/*  o_cur will be equal for both (to the vertex's o value) and so */
	/*  the above code randomly picked one to go first. That screws up */
	/*  the overlap code, which wants them properly ordered from the */
	/*  start. so look at the end point, nope the end point isn't always */
	/*  meaningful, look at the slope... */
	if ( pr->esnext!=NULL && pr->esnext->o_cur==e->o_cur &&
		SlopeLess(e,pr->esnext,es->other)) {
	    pr = pr->esnext;
	}
	e->esnext = pr->esnext;
	pr->esnext = e;
    }
    if ( es->interesting ) {
	/* Mark the other end of the spline as interesting */
	es->interesting[(int) ceil(e->mmax)]=1;
    }
}

static void AddMajorEdge(EdgeList *es, Spline *sp) {
    Edge *e, *pr;
    real m1;
    Spline1D *msp = &sp->splines[es->major], *osp = &sp->splines[es->other];

    e = gcalloc(1,sizeof(Edge));
    e->spline = sp;

    e->mmin = e->mmax = m1 = msp->d * es->scale - es->mmin;
    e->t_mmin = 0;
    e->t_mmax = 1;
    e->up = false;
    e->o_mmin = osp->d * es->scale;
    e->o_mmax = ( osp->a + osp->b + osp->c + osp->d ) * es->scale;
    if ( e->o_mmin == e->o_mmax ) {	/* Just a point? */
	free(e);
return;
    }
    if ( e->mmin<0 )
	IError("Grg!");

    if ( ceil(e->m_cur)>e->mmax ) {
	free(e);
return;
    }

    if ( es->majors==NULL || es->majors->mmin>=m1 ) {
	e->esnext = es->majors;
	es->majors = e;
    } else {
	for ( pr=es->majors; pr->esnext!=NULL && pr->esnext->mmin<m1; pr = pr->esnext );
	e->esnext = pr->esnext;
	pr->esnext = e;
    }
}

static void AddSpline(EdgeList *es, Spline *sp ) {
    real t1=2, t2=2, t;
    real b2_fourac;
    real fm, tm;
    Spline1D *msp = &sp->splines[es->major], *osp = &sp->splines[es->other];

    /* Find the points of extrema on the curve discribing y behavior */
    if ( !RealNear(msp->a,0) ) {
	/* cubic, possibly 2 extrema (possibly none) */
	b2_fourac = 4*msp->b*msp->b - 12*msp->a*msp->c;
	if ( b2_fourac>=0 ) {
	    b2_fourac = sqrt(b2_fourac);
	    t1 = CheckExtremaForSingleBitErrors(msp,(-2*msp->b - b2_fourac) / (6*msp->a));
	    t2 = CheckExtremaForSingleBitErrors(msp,(-2*msp->b + b2_fourac) / (6*msp->a));
	    if ( t1>t2 ) { real temp = t1; t1 = t2; t2 = temp; }
	    else if ( t1==t2 ) t2 = 2.0;

	    /* check for curves which have such a small slope they might */
	    /*  as well be horizontal */
	    fm = es->major==1?sp->from->me.y:sp->from->me.x;
	    tm = es->major==1?sp->to->me.y:sp->to->me.x;
	    if ( fm==tm ) {
		real m1, m2, d1, d2;
		m1 = m2 = fm;
		if ( t1>0 && t1<1 )
		    m1 = ((msp->a*t1+msp->b)*t1+msp->c)*t1 + msp->d;
		if ( t2>0 && t2<1 )
		    m2 = ((msp->a*t2+msp->b)*t2+msp->c)*t2 + msp->d;
		d1 = (m1-fm)*es->scale;
		d2 = (m2-fm)*es->scale;
		if ( d1>-.5 && d1<.5 && d2>-.5 && d2<.5 ) {
		    sp->ishorvert = true;
		    if ( es->genmajoredges )
			AddMajorEdge(es,sp);
return;		/* Pretend it's horizontal, ignore it */
		}
	    }
	}
    } else if ( !RealNear(msp->b,0) ) {
	/* Quadratic, at most one extremum */
	t1 = -msp->c/(2.0*msp->b);
    } else if ( !RealNear(msp->c,0) ) {
	/* linear, no points of extrema */
    } else {
	sp->ishorvert = true;
	if ( es->genmajoredges )
	    AddMajorEdge(es,sp);
return;		/* Horizontal line, ignore it */
    }

    if ( RealNear(t1,0)) t1=0;
    if ( RealNear(t1,1)) t1=1;
    if ( RealNear(t2,0)) t2=0;
    if ( RealNear(t2,1)) t2=1;
    if ( RealNear(t1,t2)) t2=2;
    t=0;
    if ( t1>0 && t1<1 ) {
	AddEdge(es,sp,0,t1);
	t = t1;
    }
    if ( t2>0 && t2<1 ) {
	AddEdge(es,sp,t,t2);
	t = t2;
    }
    AddEdge(es,sp,t,1.0);
    if ( es->interesting ) {
	/* Also store up points of extrema in X as interesting (we got the endpoints, just internals now)*/
	extended ot1, ot2;
	int mpos;
	SplineFindExtrema(osp,&ot1,&ot2);
	if ( ot1>0 && ot1<1 ) {
	    mpos = (int) ceil( ( ((msp->a*ot1+msp->b)*ot1+msp->c)*ot1+msp->d )*es->scale-es->mmin );
	    es->interesting[mpos] = 1;
	}
	if ( ot2>0 && ot2<1 ) {
	    mpos = (int) ceil( ( ((msp->a*ot2+msp->b)*ot2+msp->c)*ot2+msp->d )*es->scale-es->mmin );
	    es->interesting[mpos] = 1;
	}
    }
}

void FindEdgesSplineSet(SplinePointList *spl, EdgeList *es, int ignore_clip) {
    Spline *spline, *first;

    for ( ; spl!=NULL; spl = spl->next ) {
	if ( spl->first->prev!=NULL && spl->first->prev->from!=spl->first &&
		(!ignore_clip || (ignore_clip==1 && !spl->is_clip_path) || (ignore_clip==2 && spl->is_clip_path))) {
	    first = NULL;
	    es->last = es->splinesetfirst = NULL;
	    /* Set so there is no previous point!!! */
	    for ( spline = spl->first->next; spline!=NULL && spline!=first; spline=spline->to->next ) {
		AddSpline(es,spline);
		if ( first==NULL ) first = spline;
	    }
	    if ( es->last!=NULL ) {
		es->splinesetfirst->before = es->last;
		es->last->after = es->splinesetfirst;
	    }
	}
    }
}

Edge *ActiveEdgesInsertNew(EdgeList *es, Edge *active,int i) {
    Edge *apt, *pr, *npt;

    for ( pr=NULL, apt=active, npt=es->edges[(int) i]; apt!=NULL && npt!=NULL; ) {
	if ( npt->o_cur<apt->o_cur ) {
	    npt->aenext = apt;
	    if ( pr==NULL )
		active = npt;
	    else
		pr->aenext = npt;
	    pr = npt;
	    npt = npt->esnext;
	} else {
	    pr = apt;
	    apt = apt->aenext;
	}
    }
    while ( npt!=NULL ) {
	npt->aenext = NULL;
	if ( pr==NULL )
	    active = npt;
	else
	    pr->aenext = npt;
	pr = npt;
	npt = npt->esnext;
    }
return( active );
}

Edge *ActiveEdgesRefigure(EdgeList *es, Edge *active,real i) {
    Edge *apt, *pr;
    int any;

    /* first remove any entry which doesn't intersect the new scan line */
    /*  (ie. stopped on last line) */
    for ( pr=NULL, apt=active; apt!=NULL; apt = apt->aenext ) {
	if ( apt->mmax<i ) {
	    if ( pr==NULL )
		active = apt->aenext;
	    else
		pr->aenext = apt->aenext;
	} else
	    pr = apt;
    }
    /* then move the active list to the next line */
    for ( apt=active; apt!=NULL; apt = apt->aenext ) {
	Spline1D *osp = &apt->spline->splines[es->other];
	apt->t_cur = TOfNextMajor(apt,es,i);
	apt->o_cur = ( ((osp->a*apt->t_cur+osp->b)*apt->t_cur+osp->c)*apt->t_cur + osp->d ) * es->scale;
    }
    /* reorder list */
    if ( active!=NULL ) {
	any = true;
	while ( any ) {
	    any = false;
	    for ( pr=NULL, apt=active; apt->aenext!=NULL; ) {
		if ( apt->o_cur <= apt->aenext->o_cur ) {
		    /* still ordered */;
		    pr = apt;
		    apt = apt->aenext;
		} else if ( pr==NULL ) {
		    active = apt->aenext;
		    apt->aenext = apt->aenext->aenext;
		    active->aenext = apt;
		    /* don't need to set any, since this reorder can't disorder the list */
		    pr = active;
		} else {
		    pr->aenext = apt->aenext;
		    apt->aenext = apt->aenext->aenext;
		    pr->aenext->aenext = apt;
		    any = true;
		    pr = pr->aenext;
		}
	    }
	}
    }
    /* Insert new nodes */
    active = ActiveEdgesInsertNew(es,active,i);
return( active );
}

