/* Copyright (C) 2001-2015 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

/* figure out the true bounding box of a vector image */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <stdlib.h>

#include "bbox.h"
#include "potracelib.h"
#include "lists.h"

/* ---------------------------------------------------------------------- */
/* intervals */

/* initialize the interval to [min, max] */
static void interval(interval_t *i, double min, double max) {
  i->min = min;
  i->max = max;
}

/* initialize the interval to [x, x] */
static inline void singleton(interval_t *i, double x) {
  interval(i, x, x);
}

/* extend the interval to include the number x */
static inline void extend(interval_t *i, double x) {
  if (x < i->min) {
    i->min = x;
  } else if (x > i->max) {
    i->max = x;
  }
}

static inline int in_interval(interval_t *i, double x) {
  return i->min <= x && x <= i->max;
}

/* ---------------------------------------------------------------------- */
/* inner product */

typedef potrace_dpoint_t dpoint_t;

static double iprod(dpoint_t a, dpoint_t b) {
  return a.x * b.x + a.y * b.y;
}

/* ---------------------------------------------------------------------- */
/* linear Bezier segments */

/* return a point on a 1-dimensional Bezier segment */
static inline double bezier(double t, double x0, double x1, double x2, double x3) {
  double s = 1-t;
  return s*s*s*x0 + 3*(s*s*t)*x1 + 3*(t*t*s)*x2 + t*t*t*x3;
}

/* Extend the interval i to include the minimum and maximum of a
   1-dimensional Bezier segment given by control points x0..x3. For
   efficiency, x0 in i is assumed as a precondition. */
static void bezier_limits(double x0, double x1, double x2, double x3, interval_t *i) {
  double a, b, c, d, r;
  double t, x;

  /* the min and max of a cubic curve segment are attained at one of
     at most 4 critical points: the 2 endpoints and at most 2 local
     extrema. We don't check the first endpoint, because all our
     curves are cyclic so it's more efficient not to check endpoints
     twice. */

  /* endpoint */
  extend(i, x3);

  /* optimization: don't bother calculating extrema if all control
     points are already in i */
  if (in_interval(i, x1) && in_interval(i, x2)) {
    return;
  }

  /* solve for extrema. at^2 + bt + c = 0 */
  a = -3*x0 + 9*x1 - 9*x2 + 3*x3;
  b = 6*x0 - 12*x1 + 6*x2;
  c = -3*x0 + 3*x1;
  d = b*b - 4*a*c;
  if (d > 0) {
    r = sqrt(d);
    t = (-b-r)/(2*a);
    if (t > 0 && t < 1) {
      x = bezier(t, x0, x1, x2, x3);
      extend(i, x);
    }
    t = (-b+r)/(2*a);
    if (t > 0 && t < 1) {
      x = bezier(t, x0, x1, x2, x3);
      extend(i, x);
    }
  }
  return;
}

/* ---------------------------------------------------------------------- */
/* Potrace segments, curves, and pathlists */

/* extend the interval i to include the inner product <v | dir> for
   all points v on the segment. Assume precondition a in i. */
static inline void segment_limits(int tag, dpoint_t a, dpoint_t c[3], dpoint_t dir, interval_t *i) {
  switch (tag) {
  case POTRACE_CORNER:
    extend(i, iprod(c[1], dir));
    extend(i, iprod(c[2], dir));
    break;
  case POTRACE_CURVETO:
    bezier_limits(iprod(a, dir), iprod(c[0], dir), iprod(c[1], dir), iprod(c[2], dir), i);
    break;
  }
}

/* extend the interval i to include <v | dir> for all points v on the
   curve. */
static void curve_limits(potrace_curve_t *curve, dpoint_t dir, interval_t *i) {
  int k;
  int n = curve->n;

  segment_limits(curve->tag[0], curve->c[n-1][2], curve->c[0], dir, i);
  for (k=1; k<n; k++) {
    segment_limits(curve->tag[k], curve->c[k-1][2], curve->c[k], dir, i);
  }
}

/* compute the interval i to be the smallest interval including all <v
   | dir> for points in the pathlist. If the pathlist is empty, return
   the singleton interval [0,0]. */
void path_limits(potrace_path_t *path, dpoint_t dir, interval_t *i) {
  potrace_path_t *p;

  /* empty image? */
  if (path == NULL) {
    interval(i, 0, 0);
    return;
  }

  /* initialize interval to a point on the first curve */
  singleton(i, iprod(path->curve.c[0][2], dir));

  /* iterate */
  list_forall(p, path) {
    curve_limits(&p->curve, dir, i);
  }
}
