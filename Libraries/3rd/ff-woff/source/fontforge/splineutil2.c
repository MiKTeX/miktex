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
#include "fontforge.h"
#include <math.h>
#include <time.h>

int new_em_size = 1000;
int new_fonts_are_order2 = false;
int loaded_fonts_same_as_new = false;
int default_fv_row_count = 4;
int default_fv_col_count = 16;
int default_fv_font_size = 48;
int default_fv_antialias=true;
int default_fv_bbsized=false;
int snaptoint=0;

/*#define DEBUG	1*/

#if defined( FONTFORGE_CONFIG_USE_DOUBLE )
# define RE_NearZero	.00000001
# define RE_Factor	(1024.0*1024.0*1024.0*1024.0*1024.0*2.0) /* 52 bits => divide by 2^51 */
#else
# define RE_NearZero	.00001
# define RE_Factor	(1024.0*1024.0*4.0)	/* 23 bits => divide by 2^22 */
#endif

int Within4RoundingErrors(bigreal v1, bigreal v2) {
    bigreal temp=v1*v2;
    bigreal re;

    if ( temp<0 ) /* Ok, if the two values are on different sides of 0 there */
return( false );	/* is no way they can be within a rounding error of each other */
    else if ( temp==0 ) {
	if ( v1==0 )
return( v2<RE_NearZero && v2>-RE_NearZero );
	else
return( v1<RE_NearZero && v1>-RE_NearZero );
    } else if ( v1>0 ) {
	if ( v1>v2 ) {		/* Rounding error from the biggest absolute value */
	    re = v1/ (RE_Factor/4);
return( v1-v2 < re );
	} else {
	    re = v2/ (RE_Factor/4);
return( v2-v1 < re );
	}
    } else {
	if ( v1<v2 ) {
	    re = v1/ (RE_Factor/4);	/* This will be a negative number */
return( v1-v2 > re );
	} else {
	    re = v2/ (RE_Factor/4);
return( v2-v1 > re );
	}
    }
}

int Within16RoundingErrors(bigreal v1, bigreal v2) {
    bigreal temp=v1*v2;
    bigreal re;

    if ( temp<0 ) /* Ok, if the two values are on different sides of 0 there */
return( false );	/* is no way they can be within a rounding error of each other */
    else if ( temp==0 ) {
	if ( v1==0 )
return( v2<RE_NearZero && v2>-RE_NearZero );
	else
return( v1<RE_NearZero && v1>-RE_NearZero );
    } else if ( v1>0 ) {
	if ( v1>v2 ) {		/* Rounding error from the biggest absolute value */
	    re = v1/ (RE_Factor/16);
return( v1-v2 < re );
	} else {
	    re = v2/ (RE_Factor/16);
return( v2-v1 < re );
	}
    } else {
	if ( v1<v2 ) {
	    re = v1/ (RE_Factor/16);	/* This will be a negative number */
return( v1-v2 > re );
	} else {
	    re = v2/ (RE_Factor/16);
return( v2-v1 > re );
	}
    }
}

int RealNear(real a,real b) {
    real d;

#ifdef FONTFORGE_CONFIG_USE_DOUBLE
    if ( a==0 )
return( b>-1e-8 && b<1e-8 );
    if ( b==0 )
return( a>-1e-8 && a<1e-8 );

    d = a/(1024*1024.);
#else		/* For floats */
    if ( a==0 )
return( b>-1e-5 && b<1e-5 );
    if ( b==0 )
return( a>-1e-5 && a<1e-5 );

    d = a/(1024*64.);
#endif
    a-=b;
    if ( d<0 )
return( a>d && a<-d );
    else
return( a>-d && a<d );
}

int RealNearish(real a,real b) {

    a-=b;
return( a<.001 && a>-.001 );
}

int RealApprox(real a,real b) {

    if ( a==0 ) {
	if ( b<.0001 && b>-.0001 )
return( true );
    } else if ( b==0 ) {
	if ( a<.0001 && a>-.0001 )
return( true );
    } else {
	a /= b;
	if ( a>=.95 && a<=1.05 )
return( true );
    }
return( false );
}

int RealWithin(real a,real b,real fudge) {

return( b>=a-fudge && b<=a+fudge );
}

static int MinMaxWithin(Spline *spline) {
    extended dx, dy;
    int which;
    extended t1, t2;
    extended w;
    /* We know that this "spline" is basically one dimensional. As long as its*/
    /*  extrema are between the start and end points on that line then we can */
    /*  treat it as a line. If the extrema are way outside the line segment */
    /*  then it's a line that backtracks on itself */

    if ( (dx = spline->to->me.x - spline->from->me.x)<0 ) dx = -dx;
    if ( (dy = spline->to->me.y - spline->from->me.y)<0 ) dy = -dy;
    which = dx<dy;
    SplineFindExtrema(&spline->splines[which],&t1,&t2);
    if ( t1==-1 )
return( true );
    w = ((spline->splines[which].a*t1 + spline->splines[which].b)*t1
	     + spline->splines[which].c)*t1 + spline->splines[which].d;
    if ( RealNear(w, (&spline->to->me.x)[which]) || RealNear(w, (&spline->from->me.x)[which]) )
	/* Close enough */;
    else if ( (w<(&spline->to->me.x)[which] && w<(&spline->from->me.x)[which]) ||
	    (w>(&spline->to->me.x)[which] && w>(&spline->from->me.x)[which]) )
return( false );		/* Outside */

    w = ((spline->splines[which].a*t2 + spline->splines[which].b)*t2
	     + spline->splines[which].c)*t2 + spline->splines[which].d;
    if ( RealNear(w, (&spline->to->me.x)[which]) || RealNear(w, (&spline->from->me.x)[which]) )
	/* Close enough */;
    else if ( (w<(&spline->to->me.x)[which] && w<(&spline->from->me.x)[which]) ||
	    (w>(&spline->to->me.x)[which] && w>(&spline->from->me.x)[which]) )
return( false );		/* Outside */

return( true );
}

int SplineIsLinear(Spline *spline) {
    bigreal t1,t2, t3,t4;
    int ret;

    if ( spline->knownlinear )
return( true );
    if ( spline->knowncurved )
return( false );

    if ( spline->splines[0].a==0 && spline->splines[0].b==0 &&
	    spline->splines[1].a==0 && spline->splines[1].b==0 )
return( true );

    /* Something is linear if the control points lie on the line between the */
    /*  two base points */

    /* Vertical lines */
    if ( RealNear(spline->from->me.x,spline->to->me.x) ) {
	ret = RealNear(spline->from->me.x,spline->from->nextcp.x) &&
	    RealNear(spline->from->me.x,spline->to->prevcp.x);
	if ( ! ((spline->from->nextcp.y >= spline->from->me.y &&
		  spline->from->nextcp.y <= spline->to->me.y &&
		  spline->to->prevcp.y >= spline->from->me.y &&
		  spline->to->prevcp.y <= spline->to->me.y ) ||
		 (spline->from->nextcp.y <= spline->from->me.y &&
		  spline->from->nextcp.y >= spline->to->me.y &&
		  spline->to->prevcp.y <= spline->from->me.y &&
		  spline->to->prevcp.y >= spline->to->me.y )) )
	    ret = MinMaxWithin(spline);
    /* Horizontal lines */
    } else if ( RealNear(spline->from->me.y,spline->to->me.y) ) {
	ret = RealNear(spline->from->me.y,spline->from->nextcp.y) &&
	    RealNear(spline->from->me.y,spline->to->prevcp.y);
	if ( ! ((spline->from->nextcp.x >= spline->from->me.x &&
		  spline->from->nextcp.x <= spline->to->me.x &&
		  spline->to->prevcp.x >= spline->from->me.x &&
		  spline->to->prevcp.x <= spline->to->me.x) ||
		 (spline->from->nextcp.x <= spline->from->me.x &&
		  spline->from->nextcp.x >= spline->to->me.x &&
		  spline->to->prevcp.x <= spline->from->me.x &&
		  spline->to->prevcp.x >= spline->to->me.x)) )
	    ret = MinMaxWithin(spline);
    } else {
	ret = true;
	t1 = (spline->from->nextcp.y-spline->from->me.y)/(spline->to->me.y-spline->from->me.y);
	t2 = (spline->from->nextcp.x-spline->from->me.x)/(spline->to->me.x-spline->from->me.x);
	t3 = (spline->to->me.y-spline->to->prevcp.y)/(spline->to->me.y-spline->from->me.y);
	t4 = (spline->to->me.x-spline->to->prevcp.x)/(spline->to->me.x-spline->from->me.x);
	ret = (Within16RoundingErrors(t1,t2) || (RealApprox(t1,0) && RealApprox(t2,0))) &&
		(Within16RoundingErrors(t3,t4) || (RealApprox(t3,0) && RealApprox(t4,0)));
	if ( ret ) {
	    if ( t1<0 || t2<0 || t3<0 || t4<0 ||
		    t1>1 || t2>1 || t3>1 || t4>1 )
		ret = MinMaxWithin(spline);
	}
    }
    spline->knowncurved = !ret;
    spline->knownlinear = ret;
    if ( ret ) {
	/* A few places that if the spline is knownlinear then its splines[?] */
	/*  are linear. So give the linear version and not that suggested by */
	/*  the control points */
	spline->splines[0].a = spline->splines[0].b = 0;
	spline->splines[0].d = spline->from->me.x;
	spline->splines[0].c = spline->to->me.x-spline->from->me.x;
	spline->splines[1].a = spline->splines[1].b = 0;
	spline->splines[1].d = spline->from->me.y;
	spline->splines[1].c = spline->to->me.y-spline->from->me.y;
    }
return( ret );
}

/* This routine should almost never be called now. It uses a flawed algorithm */
/*  which won't produce the best results. It gets called only when the better */
/*  approach doesn't work (singular matrices, etc.) */
/* Old comment, back when I was confused... */
/* Least squares tells us that:
	| S(xi*ti^3) |	 | S(ti^6) S(ti^5) S(ti^4) S(ti^3) |   | a |
	| S(xi*ti^2) | = | S(ti^5) S(ti^4) S(ti^3) S(ti^2) | * | b |
	| S(xi*ti)   |	 | S(ti^4) S(ti^3) S(ti^2) S(ti)   |   | c |
	| S(xi)	     |   | S(ti^3) S(ti^2) S(ti)   n       |   | d |
 and the definition of a spline tells us:
	| x1         | = |   1        1       1       1    | * (a b c d)
	| x0         | = |   0        0       0       1    | * (a b c d)
So we're a bit over specified. Let's use the last two lines of least squares
and the 2 from the spline defn. So d==x0. Now we've got three unknowns
and only three equations...

For order2 splines we've got
	| S(xi*ti^2) |	 | S(ti^4) S(ti^3) S(ti^2) |   | b |
	| S(xi*ti)   | = | S(ti^3) S(ti^2) S(ti)   | * | c |
	| S(xi)	     |   | S(ti^2) S(ti)   n       |   | d |
 and the definition of a spline tells us:
	| x1         | = |   1       1       1    | * (b c d)
	| x0         | = |   0       0       1    | * (b c d)
=>
    d = x0
    b+c = x1-x0
    S(ti^2)*b + S(ti)*c = S(xi)-n*x0
    S(ti^2)*b + S(ti)*(x1-x0-b) = S(xi)-n*x0
    [ S(ti^2)-S(ti) ]*b = S(xi)-S(ti)*(x1-x0) - n*x0
*/

static int GoodCurve(SplinePoint *sp, int check_prev ) {
    bigreal dx, dy, lenx, leny;

    if ( sp->pointtype!=pt_curve && sp->pointtype!=pt_hvcurve )
return( false );
    if ( check_prev ) {
	dx = sp->me.x - sp->prevcp.x;
	dy = sp->me.y - sp->prevcp.y;
    } else {
	dx = sp->me.x - sp->nextcp.x;
	dy = sp->me.y - sp->nextcp.y;
    }
    /* If the cp is very close to the base point the point might as well be a corner */
    if ( dx<0 ) dx = -dx;
    if ( dy<0 ) dy = -dy;
    if ( dx+dy<1 )
return( false );

    if ( check_prev ) {
	if ( sp->prev==NULL )
return( true );
	lenx = sp->me.x - sp->prev->from->me.x;
	leny = sp->me.y - sp->prev->from->me.y;
    } else {
	if ( sp->next==NULL )
return( true );
	lenx = sp->me.x - sp->next->to->me.x;
	leny = sp->me.y - sp->next->to->me.y;
    }
    if ( lenx<0 ) lenx = -lenx;
    if ( leny<0 ) leny = -leny;
    if ( 50*(dx+dy) < lenx+leny )
return( false );

return( true );
}

/* pf == point from (start point) */
/* Δf == slope from (cp(from) - from) */
/* pt == point to (end point, t==1) */
/* Δt == slope to (cp(to) - to) */

/* A spline from pf to pt with slope vectors rf*Δf, rt*Δt is: */
/* p(t) = pf +  [ 3*rf*Δf ]*t  +  3*[pt-pf+rt*Δt-2*rf*Δf] *t^2 +			*/
/*		[2*pf-2*pt+3*rf*Δf-3*rt*Δt]*t^3 */

/* So I want */
/*   d  Σ (p(t(i))-p(i))^2/ d rf  == 0 */
/*   d  Σ (p(t(i))-p(i))^2/ d rt  == 0 */
/* now... */
/*   d  Σ (p(t(i))-p(i))^2/ d rf  == 0 */
/* => Σ 3*t*Δf*(1-2*t+t^2)*
 *			[pf-pi+ 3*(pt-pf)*t^2 + 2*(pf-pt)*t^3]   +
 *			3*[t - 2*t^2 + t^3]*Δf*rf   +
 *			3*[t^2-t^3]*Δt*rt   */
/* and... */
/*   d  Σ (p(t(i))-p(i))^2/ d rt  == 0 */
/* => Σ 3*t^2*Δt*(1-t)*
 *			[pf-pi+ 3*(pt-pf)*t^2 + 2*(pf-pt)*t^3]   +
 *			3*[t - 2*t^2 + t^3]*Δf*rf   +
 *			3*[t^2-t^3]*Δt*rt   */

/* Now for a long time I looked at that and saw four equations and two unknowns*/
/*  That was I was trying to solve for x and y separately, and that doesn't work. */
/*  There are really just two equations and each sums over both x and y components */

/* Old comment: */
/* I used to do a least squares aproach adding two more to the above set of equations */
/*  which held the slopes constant. But that didn't work very well. So instead*/
/*  Then I tried doing the approximation, and then forcing the control points */
/*  to be in line (witht the original slopes), getting a better approximation */
/*  to "t" for each data point and then calculating an error array, approximating*/
/*  it, and using that to fix up the final result */
/* Then I tried checking various possible cp lengths in the desired directions*/
/*  finding the best one or two, and doing a 2D binary search using that as a */
/*  starting point. */
/* And sometimes a least squares approach will give us the right answer, so   */
/*  try that too. */
/* This still isn't as good as I'd like it... But I haven't been able to */
/*  improve it further yet */

bigreal SplineLength(Spline *spline) {
    /* I ignore the constant term. It's just an unneeded addition */
    bigreal len, t;
    bigreal lastx = 0, lasty = 0;
    bigreal curx, cury;

    len = 0;
    for ( t=1.0/128; t<=1.0001 ; t+=1.0/128 ) {
	curx = ((spline->splines[0].a*t+spline->splines[0].b)*t+spline->splines[0].c)*t;
	cury = ((spline->splines[1].a*t+spline->splines[1].b)*t+spline->splines[1].c)*t;
	len += sqrt( (curx-lastx)*(curx-lastx) + (cury-lasty)*(cury-lasty) );
	lastx = curx; lasty = cury;
    }
return( len );
}

int SPInterpolate(const SplinePoint *sp) {
    /* Using truetype rules, can we interpolate this point? */
return( !sp->dontinterpolate && !sp->nonextcp && !sp->noprevcp &&
	    !sp->roundx && !sp->roundy &&
	    (RealWithin(sp->me.x,(sp->nextcp.x+sp->prevcp.x)/2,.1) &&
	     RealWithin(sp->me.y,(sp->nextcp.y+sp->prevcp.y)/2,.1)) );
}

int SpIsExtremum(SplinePoint *sp) {
    BasePoint *ncp, *pcp;
    BasePoint *nncp, *ppcp;
    if ( sp->next==NULL || sp->prev==NULL )
return( true );
    nncp = &sp->next->to->me;
    if ( !sp->nonextcp ) {
	ncp = &sp->nextcp;
	if ( !sp->next->to->noprevcp )
	    nncp = &sp->next->to->prevcp;
    } else if ( !sp->next->to->noprevcp )
	ncp = &sp->next->to->prevcp;
    else
	ncp = nncp;
    ppcp = &sp->prev->from->me;
    if ( !sp->noprevcp ) {
	pcp = &sp->prevcp;
	if ( !sp->prev->from->nonextcp )
	    ppcp = &sp->prev->from->nextcp;
    } else if ( !sp->prev->from->nonextcp )
	pcp = &sp->prev->from->nextcp;
    else
	pcp = ppcp;
    if ((( ncp->x<sp->me.x || (ncp->x==sp->me.x && nncp->x<sp->me.x)) &&
		(pcp->x<sp->me.x || (pcp->x==sp->me.x && ppcp->x<sp->me.x))) ||
	    ((ncp->x>sp->me.x || (ncp->x==sp->me.x && nncp->x>sp->me.x)) &&
		(pcp->x>sp->me.x || (pcp->x==sp->me.x && ppcp->x>sp->me.x))) ||
	(( ncp->y<sp->me.y || (ncp->y==sp->me.y && nncp->y<sp->me.y)) &&
		(pcp->y<sp->me.y || (pcp->y==sp->me.y && ppcp->y<sp->me.y))) ||
	    ((ncp->y>sp->me.y || (ncp->y==sp->me.y && nncp->y>sp->me.y)) &&
		(pcp->y>sp->me.y || (pcp->y==sp->me.y && ppcp->y>sp->me.y))))
return( true );

    /* These aren't true points of extrema, but they probably should be treated */
    /*  as if they were */
    if ( !sp->nonextcp && !sp->noprevcp &&
	    ((sp->me.x==sp->nextcp.x && sp->me.x==sp->prevcp.x) ||
	     (sp->me.y==sp->nextcp.y && sp->me.y==sp->prevcp.y)) )
return( true );

return( false );
}

/* An extremum is very close to the end-point. So close that we don't want */
/*  to add a new point. Instead try moving the control points around */
/*  Options: */
/*    o  if the control point is very close to the base point then remove it */
/*    o  if the slope at the endpoint is in the opposite direction from */
/*           what we expect, then subtract off the components we don't like */
/*    o  make the slope at the end point horizontal/vertical */
static int ForceEndPointExtrema(Spline *s,int isto) {
    SplinePoint *end;
    BasePoint *cp, to, unitslope, othercpunit, myslope;
    bigreal xdiff, ydiff, mylen, cplen, mydot, cpdot, len;
    /* To get here we know that the extremum is extremely close to the end */
    /*  point, and adjusting the slope at the end-point may be all we need */
    /*  to do. We won't need to adjust it by much, because it is so close. */

    if ( isto ) {
	end = s->to; cp = &end->prevcp;
	othercpunit.x = s->from->nextcp.x - s->from->me.x;
	othercpunit.y = s->from->nextcp.y - s->from->me.y;
    } else {
	end = s->from; cp = &end->nextcp;
	othercpunit.x = s->to->prevcp.x-s->to->me.x;
	othercpunit.y = s->to->prevcp.y-s->to->me.y;
    }
    cplen = othercpunit.x*othercpunit.x + othercpunit.y*othercpunit.y;
    cplen = sqrt(cplen);
    myslope.x = cp->x - end->me.x;
    myslope.y = cp->y - end->me.y;
    mylen = sqrt(myslope.x*myslope.x + myslope.y*myslope.y);

    unitslope.x = s->to->me.x - s->from->me.x;
    unitslope.y = s->to->me.y - s->from->me.y;
    len = unitslope.x*unitslope.x + unitslope.y*unitslope.y;
    if ( len==0 )
return( -1 );
    len = sqrt(len);
    if ( mylen<30*len && mylen<cplen && mylen<1 ) {
	if ( isto ) {
	    s->to->noprevcp = true;
	    s->to->prevcp = s->to->me;
	} else {
	    s->from->nonextcp = true;
	    s->from->nextcp = s->from->me;
	}
	end->pointtype = pt_corner;
	SplineRefigure(s);
return( true );	/* We changed the slope */
    }
    unitslope.x /= len; unitslope.y /= len;

    mydot = myslope.x*unitslope.y - myslope.y*unitslope.x;
    cpdot = othercpunit.x*unitslope.y - othercpunit.y*unitslope.y;
    if ( mydot*cpdot<0 && mylen<cplen ) {
	/* The two control points are in opposite directions with respect to */
	/*  the main spline, and ours isn't very big, so make it point along */
	/*  the spline */
	end->pointtype = pt_corner;
	if ( isto ) {
	    s->to->prevcp.x = s->to->me.x - mydot*unitslope.x;
	    s->to->prevcp.y = s->to->me.y - mydot*unitslope.y;
	} else {
	    s->from->nextcp.x = s->from->me.x + mydot*unitslope.x;
	    s->from->nextcp.y = s->from->me.y + mydot*unitslope.y;
	}
	SplineRefigure(s);
return( true );	/* We changed the slope */
    }

    if ( (xdiff = cp->x - end->me.x)<0 ) xdiff = -xdiff;
    if ( (ydiff = cp->y - end->me.y)<0 ) ydiff = -ydiff;

    to = *cp;
    if ( xdiff<ydiff/10.0 && xdiff>0 ) {
	to.x = end->me.x;
	end->pointtype = pt_corner;
	SPAdjustControl(end,cp,&to,s->order2);
return( true );	/* We changed the slope */
    } else if ( ydiff<xdiff/10 && ydiff>0 ) {
	to.y = end->me.y;
	end->pointtype = pt_corner;
	SPAdjustControl(end,cp,&to,s->order2);
return( true );	/* We changed the slope */
    }

return( -1 );		/* Didn't do anything */
}

int Spline1DCantExtremeX(const Spline *s) {
    /* Sometimes we get rounding errors when converting from control points */
    /*  to spline coordinates. These rounding errors can give us false */
    /*  extrema. So do a sanity check to make sure it is possible to get */
    /*  any extrema before actually looking for them */

    if ( s->from->me.x>=s->from->nextcp.x &&
	    s->from->nextcp.x>=s->to->prevcp.x &&
	    s->to->prevcp.x>=s->to->me.x )
return( true );
    if ( s->from->me.x<=s->from->nextcp.x &&
	    s->from->nextcp.x<=s->to->prevcp.x &&
	    s->to->prevcp.x<=s->to->me.x )
return( true );

return( false );
}

int Spline1DCantExtremeY(const Spline *s) {
    /* Sometimes we get rounding errors when converting from control points */
    /*  to spline coordinates. These rounding errors can give us false */
    /*  extrema. So do a sanity check to make sure it is possible to get */
    /*  any extrema before actually looking for them */

    if ( s->from->me.y>=s->from->nextcp.y &&
	    s->from->nextcp.y>=s->to->prevcp.y &&
	    s->to->prevcp.y>=s->to->me.y )
return( true );
    if ( s->from->me.y<=s->from->nextcp.y &&
	    s->from->nextcp.y<=s->to->prevcp.y &&
	    s->to->prevcp.y<=s->to->me.y )
return( true );

return( false );
}

Spline *SplineAddExtrema(Spline *s,int always,real lenbound, real offsetbound,
	DBounds *b) {
    /* First find the extrema, if any */
    bigreal t[4], min;
    uint8 rmfrom[4], rmto[4];
    int p, i,j, p_s, mini, restart, forced;
    SplinePoint *sp;
    real len;

    if ( !always ) {
	real xlen, ylen;
	xlen = (s->from->me.x-s->to->me.x);
	ylen = (s->from->me.y-s->to->me.y);
	len = xlen*xlen + ylen*ylen;
	lenbound *= lenbound;
	if ( len < lenbound ) {
	    len = SplineLength(s);
	    len *= len;
	}
    }

    memset(rmfrom,0,sizeof(rmfrom));
    memset(rmto,0,sizeof(rmto));

    for (;;) {
	if ( s->knownlinear )
return(s);
	p = 0;
	if ( Spline1DCantExtremeX(s) ) {
	    /* If the control points are at the end-points then this (1D) spline is */
	    /*  basically a line. But rounding errors can give us very faint extrema */
	    /*  if we look for them */
	} else if ( s->splines[0].a!=0 ) {
	    bigreal d = 4*s->splines[0].b*s->splines[0].b-4*3*s->splines[0].a*s->splines[0].c;
	    if ( d>0 ) {
		extended t1, t2;
		d = sqrt(d);
		t1 = (-2*s->splines[0].b+d)/(2*3*s->splines[0].a);
		t2 = (-2*s->splines[0].b-d)/(2*3*s->splines[0].a);
		t[p++] = CheckExtremaForSingleBitErrors(&s->splines[0],t1,t2);
		t[p++] = CheckExtremaForSingleBitErrors(&s->splines[0],t2,t1);
	    }
	} else if ( s->splines[0].b!=0 )
	    t[p++] = -s->splines[0].c/(2*s->splines[0].b);
	if ( !always ) {
	    /* Generally we are only interested in extrema on long splines, or */
	    /* extrema which are extrema for the entire contour, not just this */
	    /* spline */
	    /* Also extrema which are very close to one of the end-points can */
	    /*  be ignored. */
	    /* No they can't. But we need to remove the original point in this*/
	    /*  case */
	    for ( i=0; i<p; ++i ) {
		real x = ((s->splines[0].a*t[i]+s->splines[0].b)*t[i]+s->splines[0].c)*t[i]+s->splines[0].d;
		real y = ((s->splines[1].a*t[i]+s->splines[1].b)*t[i]+s->splines[1].c)*t[i]+s->splines[1].d;
		int close_from = ( x-s->from->me.x<offsetbound && x-s->from->me.x>-offsetbound) &&
				( y-s->from->me.y<10*offsetbound && y-s->from->me.y>-10*offsetbound );
		int close_to = ( x-s->to->me.x<offsetbound && x-s->to->me.x>-offsetbound) &&
				( y-s->to->me.y<10*offsetbound && y-s->to->me.y>-10*offsetbound );
		int remove_from = close_from  && GoodCurve(s->from,true) && !SpIsExtremum(s->from);
		int remove_to = close_to  && GoodCurve(s->to,false) && !SpIsExtremum(s->to);
		if (( x>b->minx && x<b->maxx  && len<lenbound ) ||
			(close_from && !remove_from) || (close_to && !remove_to) ) {
		    --p;
		    for ( j=i; j<p; ++j )
			t[j] = t[j+1];
		    --i;
		} else {
		    rmfrom[i] = remove_from;
		    rmto[i] = remove_to;
		}
	    }
	}

	p_s = p;
	if ( Spline1DCantExtremeY(s) ) {
	    /* If the control points are at the end-points then this (1D) spline is */
	    /*  basically a line. But rounding errors can give us very faint extrema */
	    /*  if we look for them */
	} else if ( s->splines[1].a!=0 ) {
	    bigreal d = 4*s->splines[1].b*s->splines[1].b-4*3*s->splines[1].a*s->splines[1].c;
	    if ( d>0 ) {
		extended t1,t2;
		d = sqrt(d);
		t1 = (-2*s->splines[1].b+d)/(2*3*s->splines[1].a);
		t2 = (-2*s->splines[1].b-d)/(2*3*s->splines[1].a);
		t[p++] = CheckExtremaForSingleBitErrors(&s->splines[1],t1,t2);
		t[p++] = CheckExtremaForSingleBitErrors(&s->splines[1],t2,t1);
	    }
	} else if ( s->splines[1].b!=0 )
	    t[p++] = -s->splines[1].c/(2*s->splines[1].b);
	if ( !always ) {
	    for ( i=p_s; i<p; ++i ) {
		real x = ((s->splines[0].a*t[i]+s->splines[0].b)*t[i]+s->splines[0].c)*t[i]+s->splines[0].d;
		real y = ((s->splines[1].a*t[i]+s->splines[1].b)*t[i]+s->splines[1].c)*t[i]+s->splines[1].d;
		int close_from =( y-s->from->me.y<offsetbound && y-s->from->me.y>-offsetbound ) &&
			( x-s->from->me.x<offsetbound && x-s->from->me.x>-offsetbound);
		int close_to = ( y-s->to->me.y<offsetbound && y-s->to->me.y>-offsetbound ) &&
			( x-s->to->me.x<offsetbound && x-s->to->me.x>-offsetbound);
		int remove_from = close_from  && GoodCurve(s->from,true) && !SpIsExtremum(s->from);
		int remove_to = close_to  && GoodCurve(s->to,false) && !SpIsExtremum(s->to);
		if (( y>b->miny && y<b->maxy && len<lenbound ) ||
			(close_from && !remove_from) || (close_to && !remove_to) ) {
		    --p;
		    for ( j=i; j<p; ++j )
			t[j] = t[j+1];
		    --i;
		} else {
		    rmfrom[i] = remove_from;
		    rmto[i] = remove_to;
		}
	    }
	}

	/* Throw out any t values which are not between 0 and 1 */
	/*  (we do a little fudging near the endpoints so we don't get confused */
	/*   by rounding errors) */
	restart = false;
	for ( i=0; i<p; ++i ) {
	    if ( t[i]>0 && t[i]<.05 ) {
		BasePoint test;
		/* Expand stroke gets very confused on zero-length splines so */
		/*  don't let that happen */
		test.x = ((s->splines[0].a*t[i]+s->splines[0].b)*t[i]+s->splines[0].c)*t[i]+s->splines[0].d - s->from->me.x;
		test.y = ((s->splines[1].a*t[i]+s->splines[1].b)*t[i]+s->splines[1].c)*t[i]+s->splines[1].d - s->from->me.y;
		if (( test.x*test.x + test.y*test.y<1e-7 ) && ( test.x*test.x + test.y*test.y>0.0 )) {
		    if ( (forced = ForceEndPointExtrema(s,0))>=0 ) {
			if ( forced && s->from->prev!=NULL )
			    SplineAddExtrema(s->from->prev,always,lenbound,offsetbound,b);
			restart = true;
	break;
		    }
		}
	    }
	    if ( t[i]<1 && t[i]>.95 ) {
		BasePoint test;
		test.x = ((s->splines[0].a*t[i]+s->splines[0].b)*t[i]+s->splines[0].c)*t[i]+s->splines[0].d - s->to->me.x;
		test.y = ((s->splines[1].a*t[i]+s->splines[1].b)*t[i]+s->splines[1].c)*t[i]+s->splines[1].d - s->to->me.y;
		if (( test.x*test.x + test.y*test.y < 1e-7 ) && ( test.x*test.x + test.y*test.y>0.0 )) {
		    if ( ForceEndPointExtrema(s,1)>=0 ) {
			/* don't need to fix up next, because splinesetaddextrema will do that soon */
			restart = true;
	break;
		    }
		}
	    }

	    if ( t[i]<=0 || t[i]>=1.0 ) {
		--p;
		for ( j=i; j<p; ++j ) {
		    t[j] = t[j+1];
		    rmfrom[j] = rmfrom[j+1];
		    rmto[j] = rmto[j+1];
		}
		--i;
	    }
	}
	if ( restart )
    continue;

	if ( p==0 )
return(s);

	/* Find the smallest of all the interesting points */
	min = t[0]; mini = 0;
	for ( i=1; i<p; ++i ) {
	    if ( t[i]<min ) {
		min=t[i];
		mini = i;
	    }
	}
	sp = SplineBisect(s,min);
/* On the mac we get rounding errors in the bisect routine */
	{ bigreal dx, dy;
	    if ( (dx = sp->me.x - sp->prevcp.x)<0 ) dx=-dx;
	    if ( (dy = sp->me.y - sp->prevcp.y)<0 ) dy=-dy;
	    if ( dx!=0 && dy!=0 ) {
		if ( dx<dy )
		    sp->prevcp.x = sp->me.x;
		else
		    sp->prevcp.y = sp->me.y;
	    }
	    if ( (dx = sp->me.x - sp->nextcp.x)<0 ) dx=-dx;
	    if ( (dy = sp->me.y - sp->nextcp.y)<0 ) dy=-dy;
	    if ( dx!=0 && dy!=0 ) {
		if ( dx<dy )
		    sp->nextcp.x = sp->me.x;
		else
		    sp->nextcp.y = sp->me.y;
	    }
	}

	if ( rmfrom[mini] ) sp->prev->from->ticked = true;
	if ( rmto[mini] ) sp->next->to->ticked = true;
	s = sp->next;
	if ( p==1 )
return( s );
	/* Don't try to use any other computed t values, it is easier to */
	/*  recompute them than to try and figure out what they map to on the */
	/*  new spline */
    }
}

SplineFont *SplineFontEmpty(void) {
    extern int default_fv_row_count, default_fv_col_count;
    time_t now;
    SplineFont *sf;

    sf = calloc(1,sizeof(SplineFont));
    sf->pfminfo.fstype = -1;
    sf->pfminfo.stylemap = -1;
    sf->top_enc = -1;
    sf->map = NULL;
    sf->macstyle = -1;
    sf->desired_row_cnt = default_fv_row_count; sf->desired_col_cnt = default_fv_col_count;
    sf->display_antialias = default_fv_antialias;
    sf->display_bbsized = default_fv_bbsized;
    sf->display_size = -default_fv_font_size;
    sf->display_layer = ly_fore;
    sf->sfntRevision = sfntRevisionUnset;
    sf->woffMajor = woffUnset;
    sf->woffMinor = woffUnset;
    sf->pfminfo.winascent_add = sf->pfminfo.windescent_add = true;
    sf->pfminfo.hheadascent_add = sf->pfminfo.hheaddescent_add = true;
    sf->pfminfo.typoascent_add = sf->pfminfo.typodescent_add = true;
    if ( TTFFoundry!=NULL )
	strncpy(sf->pfminfo.os2_vendor,TTFFoundry,4);
    else
	memcpy(sf->pfminfo.os2_vendor,"PfEd",4);
    sf->for_new_glyphs = DefaultNameListForNewFonts();
    time(&now);
    sf->creationtime = sf->modificationtime = now;

    sf->layer_cnt = 2;
    sf->layers = calloc(2,sizeof(LayerInfo));
    sf->layers[ly_back].name = copy(_("Back"));
    sf->layers[ly_back].background = true;
    sf->layers[ly_fore].name = copy(_("Fore"));
    sf->layers[ly_fore].background = false;
    sf->grid.background = true;

return( sf );
}

static void SFChangeXUID(SplineFont *sf, int random) {
    char *pt, *new, *npt;
    int val;

    if ( sf->xuid==NULL )
return;
    pt = strrchr(sf->xuid,' ');
    if ( pt==NULL )
	pt = strchr(sf->xuid,'[');
    if ( pt==NULL )
	pt = sf->xuid;
    else
	++pt;
    if ( random )
	val = rand()&0xffffff;
    else {
	val = strtol(pt,NULL,10);
	val = (val+1)&0xffffff;
    }

    new = malloc(pt-sf->xuid+12);
    strncpy(new,sf->xuid,pt-sf->xuid);
    npt = new + (pt-sf->xuid);
    if ( npt==new ) *npt++ = '[';
    sprintf(npt, "%d]", val );
    free(sf->xuid); sf->xuid = new;
    sf->changed = true;
    sf->changed_since_xuidchanged = false;
}

void SFIncrementXUID(SplineFont *sf) {
    SFChangeXUID(sf,false);
}

void SplineCharTangentNextCP(SplinePoint *sp) {
    bigreal len;
    BasePoint *bp, unit;
    extern int snaptoint;

    if ( sp->prev==NULL )
return;
    bp = &sp->prev->from->me;

    unit.y = sp->me.y-bp->y; unit.x = sp->me.x-bp->x;
    len = sqrt( unit.x*unit.x + unit.y*unit.y );
    if ( len!=0 ) {
	unit.x /= len;
	unit.y /= len;
    }
    len = sqrt((sp->nextcp.y-sp->me.y)*(sp->nextcp.y-sp->me.y) + (sp->nextcp.x-sp->me.x)*(sp->nextcp.x-sp->me.x));
    sp->nextcp.x = sp->me.x + len*unit.x;
    sp->nextcp.y = sp->me.y + len*unit.y;
    if ( snaptoint ) {
	sp->nextcp.x = rint(sp->nextcp.x);
	sp->nextcp.y = rint(sp->nextcp.y);
    } else {
	sp->nextcp.x = rint(sp->nextcp.x*1024)/1024;
	sp->nextcp.y = rint(sp->nextcp.y*1024)/1024;
    }
    if ( sp->next!=NULL && sp->next->order2 )
	sp->next->to->prevcp = sp->nextcp;
}

void SplineCharTangentPrevCP(SplinePoint *sp) {
    bigreal len;
    BasePoint *bp, unit;
    extern int snaptoint;

    if ( sp->next==NULL )
return;
    bp = &sp->next->to->me;

    unit.y = sp->me.y-bp->y; unit.x = sp->me.x-bp->x;
    len = sqrt( unit.x*unit.x + unit.y*unit.y );
    if ( len!=0 ) {
	unit.x /= len;
	unit.y /= len;
    }
    len = sqrt((sp->prevcp.y-sp->me.y)*(sp->prevcp.y-sp->me.y) + (sp->prevcp.x-sp->me.x)*(sp->prevcp.x-sp->me.x));
    sp->prevcp.x = sp->me.x + len*unit.x;
    sp->prevcp.y = sp->me.y + len*unit.y;
    if ( snaptoint ) {
	sp->prevcp.x = rint(sp->prevcp.x);
	sp->prevcp.y = rint(sp->prevcp.y);
    } else {
	sp->prevcp.x = rint(sp->prevcp.x*1024)/1024;
	sp->prevcp.y = rint(sp->prevcp.y*1024)/1024;
    }
    if ( sp->prev!=NULL && sp->prev->order2 )
	sp->prev->from->nextcp = sp->prevcp;
}

void BP_HVForce(BasePoint *vector) {
    /* Force vector to be horizontal/vertical */
    bigreal dx, dy, len;

    if ( (dx= vector->x)<0 ) dx = -dx;
    if ( (dy= vector->y)<0 ) dy = -dy;
    if ( dx==0 || dy==0 )
return;
    len = sqrt(dx*dx + dy*dy);
    if ( dx>dy ) {
	vector->x = vector->x<0 ? -len : len;
	vector->y = 0;
    } else {
	vector->y = vector->y<0 ? -len : len;
	vector->x = 0;
    }
}
    
#define NICE_PROPORTION	.39
void SplineCharDefaultNextCP(SplinePoint *base) {
    SplinePoint *prev=NULL, *next;
    bigreal len, plen, ulen;
    BasePoint unit;
    extern int snaptoint;

    if ( base->next==NULL )
return;
    if ( base->next->order2 ) {
	SplineRefigureFixup(base->next);
return;
    }
    if ( !base->nextcpdef ) {
	if ( base->pointtype==pt_tangent )
	    SplineCharTangentNextCP(base);
return;
    }
    next = base->next->to;
    if ( base->prev!=NULL )
	prev = base->prev->from;

    len = NICE_PROPORTION * sqrt((base->me.x-next->me.x)*(base->me.x-next->me.x) +
	    (base->me.y-next->me.y)*(base->me.y-next->me.y));
    unit.x = next->me.x - base->me.x;
    unit.y = next->me.y - base->me.y;
    ulen = sqrt(unit.x*unit.x + unit.y*unit.y);
    if ( ulen!=0 )
	unit.x /= ulen, unit.y /= ulen;
    base->nonextcp = false;

    if ( base->pointtype == pt_curve || base->pointtype == pt_hvcurve ) {
	if ( prev!=NULL && (base->prevcpdef || base->noprevcp)) {
	    unit.x = next->me.x - prev->me.x;
	    unit.y = next->me.y - prev->me.y;
	    ulen = sqrt(unit.x*unit.x + unit.y*unit.y);
	    if ( ulen!=0 )
		unit.x /= ulen, unit.y /= ulen;
	    if ( base->pointtype == pt_hvcurve )
		BP_HVForce(&unit);
	    plen = sqrt((base->prevcp.x-base->me.x)*(base->prevcp.x-base->me.x) +
		    (base->prevcp.y-base->me.y)*(base->prevcp.y-base->me.y));
	    base->prevcp.x = base->me.x - plen*unit.x;
	    base->prevcp.y = base->me.y - plen*unit.y;
	    if ( snaptoint ) {
		base->prevcp.x = rint(base->prevcp.x);
		base->prevcp.y = rint(base->prevcp.y);
	    }
	    SplineRefigureFixup(base->prev);
	} else if ( prev!=NULL ) {
	    /* The prev control point is fixed. So we've got to use the same */
	    /*  angle it uses */
	    unit.x = base->me.x-base->prevcp.x;
	    unit.y = base->me.y-base->prevcp.y;
	    ulen = sqrt(unit.x*unit.x + unit.y*unit.y);
	    if ( ulen!=0 )
		unit.x /= ulen, unit.y /= ulen;
	} else {
	    base->prevcp = base->me;
	    base->noprevcp = true;
	    base->prevcpdef = true;
	}
	if ( base->pointtype == pt_hvcurve )
	    BP_HVForce(&unit);
    } else if ( base->pointtype == pt_corner ) {
	if ( next->pointtype != pt_curve && next->pointtype != pt_hvcurve ) {
	    base->nonextcp = true;
	}
    } else /* tangent */ {
	if ( next->pointtype != pt_curve ) {
	    base->nonextcp = true;
	} else {
	    if ( prev!=NULL ) {
		if ( !base->noprevcp ) {
		    plen = sqrt((base->prevcp.x-base->me.x)*(base->prevcp.x-base->me.x) +
			    (base->prevcp.y-base->me.y)*(base->prevcp.y-base->me.y));
		    base->prevcp.x = base->me.x - plen*unit.x;
		    base->prevcp.y = base->me.y - plen*unit.y;
		    SplineRefigureFixup(base->prev);
		}
		unit.x = base->me.x-prev->me.x;
		unit.y = base->me.y-prev->me.y;
		ulen = sqrt(unit.x*unit.x + unit.y*unit.y);
		if ( ulen!=0 )
		    unit.x /= ulen, unit.y /= ulen;
	    }
	}
    }
    if ( base->nonextcp )
	base->nextcp = base->me;
    else {
	base->nextcp.x = base->me.x + len*unit.x;
	base->nextcp.y = base->me.y + len*unit.y;
	if ( snaptoint ) {
	    base->nextcp.x = rint(base->nextcp.x);
	    base->nextcp.y = rint(base->nextcp.y);
	} else {
	    base->nextcp.x = rint(base->nextcp.x*1024)/1024;
	    base->nextcp.y = rint(base->nextcp.y*1024)/1024;
	}
	if ( base->next != NULL )
	    SplineRefigureFixup(base->next);
    }
}

void SplineCharDefaultPrevCP(SplinePoint *base) {
    SplinePoint *next=NULL, *prev;
    bigreal len, nlen, ulen;
    BasePoint unit;
    extern int snaptoint;

    if ( base->prev==NULL )
return;
    if ( base->prev->order2 ) {
	SplineRefigureFixup(base->prev);
return;
    }
    if ( !base->prevcpdef ) {
	if ( base->pointtype==pt_tangent )
	    SplineCharTangentPrevCP(base);
return;
    }
    prev = base->prev->from;
    if ( base->next!=NULL )
	next = base->next->to;

    len = NICE_PROPORTION * sqrt((base->me.x-prev->me.x)*(base->me.x-prev->me.x) +
	    (base->me.y-prev->me.y)*(base->me.y-prev->me.y));
    unit.x = prev->me.x - base->me.x;
    unit.y = prev->me.y - base->me.y;
    ulen = sqrt(unit.x*unit.x + unit.y*unit.y);
    if ( ulen!=0 )
	unit.x /= ulen, unit.y /= ulen;
    base->noprevcp = false;

    if ( base->pointtype == pt_curve || base->pointtype == pt_hvcurve ) {
	if ( next!=NULL && (base->nextcpdef || base->nonextcp)) {
	    unit.x = prev->me.x - next->me.x;
	    unit.y = prev->me.y - next->me.y;
	    ulen = sqrt(unit.x*unit.x + unit.y*unit.y);
	    if ( ulen!=0 ) 
		unit.x /= ulen, unit.y /= ulen;
	    if ( base->pointtype == pt_hvcurve )
		BP_HVForce(&unit);
	    nlen = sqrt((base->nextcp.x-base->me.x)*(base->nextcp.x-base->me.x) +
		    (base->nextcp.y-base->me.y)*(base->nextcp.y-base->me.y));
	    base->nextcp.x = base->me.x - nlen*unit.x;
	    base->nextcp.y = base->me.y - nlen*unit.y;
	    if ( snaptoint ) {
		base->nextcp.x = rint(base->nextcp.x);
		base->nextcp.y = rint(base->nextcp.y);
	    }
	    SplineRefigureFixup(base->next);
	} else if ( next!=NULL ) {
	    /* The next control point is fixed. So we got to use the same */
	    /*  angle it uses */
	    unit.x = base->me.x-base->nextcp.x;
	    unit.y = base->me.y-base->nextcp.y;
	    ulen = sqrt(unit.x*unit.x + unit.y*unit.y);
	    if ( ulen!=0 )
		unit.x /= ulen, unit.y /= ulen;
	} else {
	    base->nextcp = base->me;
	    base->nonextcp = true;
	    base->nextcpdef = true;
	}
	if ( base->pointtype == pt_hvcurve )
	    BP_HVForce(&unit);
    } else if ( base->pointtype == pt_corner ) {
	if ( prev->pointtype != pt_curve && prev->pointtype != pt_hvcurve ) {
	    base->noprevcp = true;
	}
    } else /* tangent */ {
	if ( prev->pointtype != pt_curve ) {
	    base->noprevcp = true;
	} else {
	    if ( next!=NULL ) {
		if ( !base->nonextcp ) {
		    nlen = sqrt((base->nextcp.x-base->me.x)*(base->nextcp.x-base->me.x) +
			    (base->nextcp.y-base->me.y)*(base->nextcp.y-base->me.y));
		    base->nextcp.x = base->me.x - nlen*unit.x;
		    base->nextcp.y = base->me.y - nlen*unit.y;
		    SplineRefigureFixup(base->next);
		}
		unit.x = base->me.x-next->me.x;
		unit.y = base->me.y-next->me.y;
		ulen = sqrt(unit.x*unit.x + unit.y*unit.y);
		if ( ulen!=0 )
		    unit.x /= ulen, unit.y /= ulen;
	    }
	}
    }
    if ( base->noprevcp )
	base->prevcp = base->me;
    else {
	base->prevcp.x = base->me.x + len*unit.x;
	base->prevcp.y = base->me.y + len*unit.y;
	if ( snaptoint ) {
	    base->prevcp.x = rint(base->prevcp.x);
	    base->prevcp.y = rint(base->prevcp.y);
	} else {
	    base->prevcp.x = rint(base->prevcp.x*1024)/1024;
	    base->prevcp.y = rint(base->prevcp.y*1024)/1024;
	}
	if ( base->prev!=NULL )
	    SplineRefigureFixup(base->prev);
    }
}

void SPTouchControl(SplinePoint *sp,BasePoint *which, int order2)
{
    BasePoint to = *which;
    SPAdjustControl( sp, which, &to, order2 );
}

void SPAdjustControl(SplinePoint *sp,BasePoint *cp, BasePoint *to,int order2) {
    BasePoint *othercp = cp==&sp->nextcp?&sp->prevcp:&sp->nextcp;
    int refig = false, otherchanged = false;

    if ( sp->ttfindex==0xffff && order2 ) {
	/* If the point itself is implied, then it's the control points that */
	/*  are fixed. Moving a CP should move the implied point so that it */
	/*  continues to be in the right place */
	sp->me.x = (to->x+othercp->x)/2;
	sp->me.y = (to->y+othercp->y)/2;
	*cp = *to;
	refig = true;
    } else if ( sp->pointtype==pt_corner ) {
	*cp = *to;
    } else if ( sp->pointtype==pt_curve || sp->pointtype==pt_hvcurve ) {
	if ( sp->pointtype==pt_hvcurve ) {
	    BasePoint diff;
	    diff.x = to->x - sp->me.x;
	    diff.y = to->y - sp->me.y;
	    BP_HVForce(&diff);
	    cp->x = sp->me.x + diff.x;
	    cp->y = sp->me.y + diff.y;
	} else {
	    *cp = *to;
	}
	if (( cp->x!=sp->me.x || cp->y!=sp->me.y ) &&
		(!order2 ||
		 (cp==&sp->nextcp && sp->next!=NULL && sp->next->to->ttfindex==0xffff) ||
		 (cp==&sp->prevcp && sp->prev!=NULL && sp->prev->from->ttfindex==0xffff)) ) {
	    bigreal len1, len2;
	    len1 = sqrt((cp->x-sp->me.x)*(cp->x-sp->me.x) +
			(cp->y-sp->me.y)*(cp->y-sp->me.y));
	    len2 = sqrt((othercp->x-sp->me.x)*(othercp->x-sp->me.x) +
			(othercp->y-sp->me.y)*(othercp->y-sp->me.y));
	    len2 /= len1;
	    othercp->x = len2 * (sp->me.x-cp->x) + sp->me.x;
	    othercp->y = len2 * (sp->me.y-cp->y) + sp->me.y;
	    otherchanged = true;
	    if ( sp->next!=NULL && othercp==&sp->nextcp ) {
		if ( order2 ) sp->next->to->prevcp = *othercp;
		SplineRefigure(sp->next);
	    } else if ( sp->prev!=NULL && othercp==&sp->prevcp ) {
		if ( order2 ) sp->prev->from->nextcp = *othercp;
		SplineRefigure(sp->prev);
	    }
	}
	if ( cp==&sp->nextcp ) sp->prevcpdef = false;
	else sp->nextcpdef = false;
    } else {
	BasePoint *bp;
	if ( cp==&sp->prevcp && sp->next!=NULL )
	    bp = &sp->next->to->me;
	else if ( cp==&sp->nextcp && sp->prev!=NULL )
	    bp = &sp->prev->from->me;
	else
	    bp = NULL;
	if ( bp!=NULL ) {
	    real angle = atan2(bp->y-sp->me.y,bp->x-sp->me.x);
	    real len = sqrt((bp->x-sp->me.x)*(bp->x-sp->me.x) + (bp->y-sp->me.y)*(bp->y-sp->me.y));
	    real dotprod =
		    ((to->x-sp->me.x)*(bp->x-sp->me.x) +
		     (to->y-sp->me.y)*(bp->y-sp->me.y));
	    if ( len!=0 ) {
		dotprod /= len;
		if ( dotprod>0 ) dotprod = 0;
		cp->x = sp->me.x + dotprod*cos(angle);
		cp->y = sp->me.y + dotprod*sin(angle);
	    }
	}
    }

    if ( order2 ) {
	if ( (cp==&sp->nextcp || otherchanged) && sp->next!=NULL ) {
	    SplinePoint *osp = sp->next->to;
	    if ( osp->ttfindex==0xffff ) {
		osp->prevcp = sp->nextcp;
		osp->me.x = (osp->prevcp.x+osp->nextcp.x)/2;
		osp->me.y = (osp->prevcp.y+osp->nextcp.y)/2;
		SplineRefigure(osp->next);
	    }
	}
	if ( (cp==&sp->prevcp || otherchanged) && sp->prev!=NULL ) {
	    SplinePoint *osp = sp->prev->from;
	    if ( osp->ttfindex==0xffff ) {
		osp->nextcp = sp->prevcp;
		osp->me.x = (osp->prevcp.x+osp->nextcp.x)/2;
		osp->me.y = (osp->prevcp.y+osp->nextcp.y)/2;
		SplineRefigure(osp->prev);
	    }
	}
    }

    if ( cp->x==sp->me.x && cp->y==sp->me.y ) {
	if ( cp==&sp->nextcp ) sp->nonextcp = true;
	else sp->noprevcp = true;
    }  else {
	if ( cp==&sp->nextcp ) sp->nonextcp = false;
	else sp->noprevcp = false;
    }
    if ( cp==&sp->nextcp ) sp->nextcpdef = false;
    else sp->prevcpdef = false;

    if ( sp->next!=NULL && cp==&sp->nextcp ) {
	if ( order2 && !sp->nonextcp ) {
	    sp->next->to->prevcp = *cp;
	    sp->next->to->noprevcp = false;
	}
	SplineRefigureFixup(sp->next);
    }
    if ( sp->prev!=NULL && cp==&sp->prevcp ) {
	if ( order2 && !sp->noprevcp ) {
	    sp->prev->from->nextcp = *cp;
	    sp->prev->from->nonextcp = false;
	}
	SplineRefigureFixup(sp->prev);
    }
    if ( refig ) {
	SplineRefigure(sp->prev);
	SplineRefigure(sp->next);
    }
}

SplineSet *SplineSetReverse(SplineSet *spl) {
    Spline *spline, *first, *next;
    BasePoint tp;
    SplinePoint *temp;
    int flag;
    int i;
    /* reverse the splineset so that what was the start point becomes the end */
    /*  and vice versa. This entails reversing every individual spline, and */
    /*  each point */

    first = NULL;
    spline = spl->first->next;
    if ( spline==NULL )
return( spl );			/* Only one point, reversal is meaningless */

    tp = spline->from->nextcp;
    spline->from->nextcp = spline->from->prevcp;
    spline->from->prevcp = tp;
    flag = spline->from->nonextcp;
    spline->from->nonextcp = spline->from->noprevcp;
    spline->from->noprevcp = flag;
    flag = spline->from->nextcpdef;
    spline->from->nextcpdef = spline->from->prevcpdef;
    spline->from->prevcpdef = flag;

    for ( ; spline!=NULL && spline!=first; spline=next ) {
	next = spline->to->next;

	if ( spline->to!=spl->first ) {		/* On a closed spline don't want to reverse the first point twice */
	    tp = spline->to->nextcp;
	    spline->to->nextcp = spline->to->prevcp;
	    spline->to->prevcp = tp;
	    flag = spline->to->nonextcp;
	    spline->to->nonextcp = spline->to->noprevcp;
	    spline->to->noprevcp = flag;
	    flag = spline->to->nextcpdef;
	    spline->to->nextcpdef = spline->to->prevcpdef;
	    spline->to->prevcpdef = flag;
	}

	temp = spline->to;
	spline->to = spline->from;
	spline->from = temp;
	spline->from->next = spline;
	spline->to->prev = spline;
	SplineRefigure(spline);
	if ( first==NULL ) first = spline;
    }

    if ( spl->first!=spl->last ) {
	temp = spl->first;
	spl->first = spl->last;
	spl->start_offset = 0;
	spl->last = temp;
	spl->first->prev = NULL;
	spl->last->next = NULL;
    }

    if ( spl->spiro_cnt>2 ) {
	for ( i=(spl->spiro_cnt-1)/2-1; i>=0; --i ) {
	    spiro_cp temp_cp = spl->spiros[i];
	    spl->spiros[i] = spl->spiros[spl->spiro_cnt-2-i];
	    spl->spiros[spl->spiro_cnt-2-i] = temp_cp;
	}
	if ( (spl->spiros[spl->spiro_cnt-2].ty&0x7f)==SPIRO_OPEN_CONTOUR ) {
	    spl->spiros[spl->spiro_cnt-2].ty = (spl->spiros[0].ty&0x7f) | (spl->spiros[spl->spiro_cnt-2].ty&0x80);
	    spl->spiros[0].ty = SPIRO_OPEN_CONTOUR | (spl->spiros[0].ty&0x80);
	}
	for ( i=spl->spiro_cnt-2; i>=0; --i ) {
	    if ( (spl->spiros[i].ty&0x7f) == SPIRO_LEFT )
		spl->spiros[i].ty = SPIRO_RIGHT | (spl->spiros[i].ty&0x80);
	    else if ( (spl->spiros[i].ty&0x7f) == SPIRO_RIGHT )
		spl->spiros[i].ty = SPIRO_LEFT | (spl->spiros[i].ty&0x80);
	}
    }
return( spl );
}

#include "edgelist.h"

int SplinePointListIsClockwise(const SplineSet *spl) {
    EIList el;
    EI *active=NULL, *apt, *pr, *e;
    int i, winding,change,waschange, cnt;
    SplineChar dummy;
    SplineSet *next;
    Layer layers[2];
    int cw_cnt=0, ccw_cnt=0;

    memset(&el,'\0',sizeof(el));
    memset(&dummy,'\0',sizeof(dummy));
    memset(layers,0,sizeof(layers));
    el.layer = ly_fore;
    dummy.layers = layers;
    dummy.layer_cnt = 2;
    dummy.layers[ly_fore].splines = (SplineSet *) spl;
    dummy.name = "Clockwise Test";
    next = spl->next; ((SplineSet *) spl)->next = NULL;
    ELFindEdges(&dummy,&el);
    if ( el.coordmax[1]-el.coordmin[1] > 1.e6 ) {
	LogError( _("Warning: Unreasonably big splines. They will be ignored.\n") );
	((SplineSet *) spl)->next = next;
return( -1 );
    }
    el.major = 1;
    ELOrder(&el,el.major);

    waschange = false;
    for ( i=0; i<el.cnt ; ++i ) {
	active = EIActiveEdgesRefigure(&el,active,i,1,&change);
	for ( apt=active, cnt=0; apt!=NULL; apt = apt->aenext , ++cnt );
	if ( el.ordered[i]!=NULL || el.ends[i] || cnt&1 ||
		waschange || change ||
		(i!=el.cnt-1 && (el.ends[i+1] || el.ordered[i+1])) ) {
	    waschange = change;
    continue;			/* Just too hard to get the edges sorted when we are at a start vertex */
	}
	waschange = change;
	for ( apt=active; apt!=NULL; apt = e) {
	    if ( EISkipExtremum(apt,i+el.low,1)) {
		e = apt->aenext->aenext;
	continue;
	    }
	    if ( apt->up )
		++cw_cnt;
	    else
		++ccw_cnt;
	    if ( cw_cnt!=0 && ccw_cnt!=0 ) {
		((SplineSet *) spl)->next = next;
return( -1 );
	    }
	    winding = apt->up?1:-1;
	    for ( pr=apt, e=apt->aenext; e!=NULL && winding!=0; pr=e, e=e->aenext ) {
		if ( EISkipExtremum(e,i+el.low,1)) {
		    e = e->aenext;
	    continue;
		}
		if ( pr->up!=e->up ) {
		    if ( (winding<=0 && !e->up) || (winding>0 && e->up )) {
/* return( -1 );*/	/* This is an erroneous condition... but I don't think*/
			/*  it can actually happen with a single contour. I */
			/*  think it is more likely this means a rounding error*/
			/*  and a problem in my algorithm */
			fprintf( stderr, "SplinePointListIsClockwise: Found error\n" );
		    }
		    winding += (e->up?1:-1);
		} else if ( EISameLine(pr,e,i+el.low,1) )
		    /* This just continues the line and doesn't change count */;
		else {
		    if ( (winding<=0 && !e->up) || (winding>0 && e->up )) {
			fprintf( stderr, "SplinePointListIsClockwise: Found error\n" );
/*return( -1 );*/
		    }
		    winding += (e->up?1:-1);
		}
	    }
	}
    }
    free(el.ordered);
    free(el.ends);
    ElFreeEI(&el);
    ((SplineSet *) spl)->next = next;

    if ( cw_cnt!=0 )
return( true );
    else if ( ccw_cnt!=0 )
return( false );

return( -1 );
}

/* Since this function now deals with 4 arbitrarily selected points, */
/* it has to try to combine them by different ways in order to see */
/* if they actually can specify a diagonal stem. The reordered points */
/* are placed back to array passed to the function.*/
int PointsDiagonalable( SplineFont *sf,BasePoint **bp,BasePoint *unit ) {
    BasePoint *line1[2], *line2[2], *temp, *base;
    BasePoint unit1, unit2;
    int i, j, k;
    bigreal dist_error_diag, len1, len2, width, dot;
    bigreal off1, off2;

    for ( i=0; i<4; i++ ) {
        if ( bp[i] == NULL )
return( false );
    }

    dist_error_diag = 0.0065 * ( sf->ascent + sf->descent );
    /* Assume that the first point passed to the function is the starting */
    /* point of the first of two vectors. Then try all possible combinations */
    /* (there are 3), ensure the vectors are consistantly ordered, and */
    /* check if they are parallel.*/
    base = bp[0];
    for ( i=1; i<4; i++ ) {
        line1[0] = base; line1[1] = bp[i];
        
        k=0;
        for ( j=1; j<4; j++ ) {
            if ( j != i )
                line2[k++] = bp[j];
        }
        unit1.x = line1[1]->x - line1[0]->x;
        unit1.y = line1[1]->y - line1[0]->y;
        unit2.x = line2[1]->x - line2[0]->x;
        unit2.y = line2[1]->y - line2[0]->y;
        /* No horizontal, vertical edges */
        if ( unit1.x == 0 || unit1.y == 0 || unit2.x == 0 || unit2.y == 0 )
    continue;
        len1 = sqrt( pow( unit1.x,2 ) + pow( unit1.y,2 ));
        len2 = sqrt( pow( unit2.x,2 ) + pow( unit2.y,2 ));
        unit1.x /= len1; unit1.y /= len1;
        unit2.x /= len2; unit2.y /= len2;
        dot = unit1.x * unit2.y - unit1.y * unit2.x;
        /* Units parallel */
        if ( dot <= -.05 || dot >= .05 )
    continue;
        /* Ensure vectors point by such a way that the angle is between 90 and 270 degrees */
        if ( unit1.x <  0 ) {
            temp = line1[0]; line1[0] = line1[1]; line1[1] = temp;
            unit1.x = -unit1.x; unit1.y = -unit1.y;
        }
        if ( unit2.x <  0 ) {
            temp = line2[0]; line2[0] = line2[1]; line2[1] = temp;
            unit2.x = -unit2.x; unit2.y = -unit2.y;
        }
        off1 =  ( line1[1]->x - line1[0]->x ) * unit2.y -
                ( line1[1]->y - line1[0]->y ) * unit2.x;
        off2 =  ( line2[1]->x - line2[0]->x ) * unit1.y -
                ( line2[1]->y - line2[0]->y ) * unit1.x;
        if ( len1 > len2 && fabs( off2 ) < 2*dist_error_diag ) *unit = unit1;
        else if ( fabs( off1 ) < 2*dist_error_diag ) *unit = unit2;
        else
    continue;
        width = ( line2[0]->x - line1[0]->x ) * unit->y -
                ( line2[0]->y - line1[0]->y ) * unit->x;
        /* Make sure this is a real line, rather than just two */
        /* short spline segments which occasionally have happened to be */
        /* parallel. This is necessary to correctly handle things which may */
        /* be "diagonalable" in 2 different directions (like slash in some */
        /* designs). */
        if ( fabs( width ) > len1 || fabs( width ) > len2 )
    continue;
	/* Make sure line2 is further right than line1 */
        if ( width < 0 ) {
	    temp = line1[0]; line1[0] = line2[0]; line2[0] = temp;
	    temp = line1[1]; line1[1] = line2[1]; line2[1] = temp;
        }
        bp[0] = line1[0];
        bp[1] = line2[0];
        bp[2] = line1[1];
        bp[3] = line2[1];
return( true );
    }
return( false );
}
