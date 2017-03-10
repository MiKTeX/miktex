/* Copyright (C) 2001-2017 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

/* The DXF backend of Potrace. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "main.h"
#include "backend_dxf.h"
#include "potracelib.h"
#include "lists.h"
#include "auxiliary.h"
#include "trans.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ---------------------------------------------------------------------- */
/* auxiliary linear algebra functions */

/* subtract two vectors */
static dpoint_t sub(dpoint_t v, dpoint_t w) {
  dpoint_t r;

  r.x = v.x - w.x;
  r.y = v.y - w.y;
  return r;
}

/* inner product */
static double iprod(dpoint_t v, dpoint_t w) {
  return v.x * w.x + v.y * w.y;
}

/* cross product */
static double xprod(dpoint_t v, dpoint_t w) {
  return v.x * w.y - v.y * w.x;
}

/* calculate the DXF polyline "bulge" value corresponding to the angle
   between two vectors. In case of "infinity" return 0.0. */
static double bulge(dpoint_t v, dpoint_t w) {
  double v2, w2, vw, vxw, nvw;

  v2 = iprod(v, v);
  w2 = iprod(w, w);
  vw = iprod(v, w);
  vxw = xprod(v, w);
  nvw = sqrt(v2 * w2);

  if (vxw == 0.0) {
    return 0.0;
  }    

  return (nvw - vw) / vxw;
}

/* ---------------------------------------------------------------------- */
/* DXF output synthesis */

/* print with group code: the low-level DXF file format */
static int ship(FILE *fout, int gc, const char *fmt, ...) {
  va_list args;
  int r;
  int c;

  r = fprintf(fout, "%3d\n", gc);
  if (r < 0) {
    return r;
  }
  c = r;
  va_start(args, fmt);
  r = vfprintf(fout, fmt, args);
  va_end(args);
  if (r < 0) {
    return r;
  }
  c += r;
  r = fprintf(fout, "\n");
  if (r < 0) {
    return r;
  }
  c += r;
  return c;
}

/* output the start of a polyline */
static void ship_polyline(FILE *fout, const char *layer, int closed) {
  ship(fout, 0, "POLYLINE");
  ship(fout, 8, "%s", layer);
  ship(fout, 66, "%d", 1);
  ship(fout, 70, "%d", closed ? 1 : 0);
}

/* output a vertex */
static void ship_vertex(FILE *fout, const char *layer, dpoint_t v, double bulge) {
  ship(fout, 0, "VERTEX");
  ship(fout, 8, "%s", layer);
  ship(fout, 10, "%f", v.x);
  ship(fout, 20, "%f", v.y);
  ship(fout, 42, "%f", bulge);
}

/* output the end of a polyline */
static void ship_seqend(FILE *fout) {
  ship(fout, 0, "SEQEND");
}

/* output a comment */
static void ship_comment(FILE *fout, const char *comment) {
  ship(fout, 999, "%s", comment);
}

/* output the start of a section */
static void ship_section(FILE *fout, const char *name) {
  ship(fout, 0, "SECTION");
  ship(fout, 2, "%s", name);
}

static void ship_endsec(FILE *fout) {
  ship(fout, 0, "ENDSEC");
}

static void ship_eof(FILE *fout) {
  ship(fout, 0, "EOF");
}

/* ---------------------------------------------------------------------- */
/* Simulated quadratic and bezier curves */

/* Output vertices (with bulges) corresponding to a smooth pair of
   circular arcs from A to B, tangent to AC at A and to CB at
   B. Segments are meant to be concatenated, so don't output the final
   vertex. */
static void pseudo_quad(FILE *fout, const char *layer, dpoint_t A, dpoint_t C, dpoint_t B) {
  dpoint_t v, w;
  double v2, w2, vw, vxw, nvw;
  double a, b, c, y;
  dpoint_t G;
  double bulge1, bulge2;

  v = sub(A, C);
  w = sub(B, C);

  v2 = iprod(v, v);
  w2 = iprod(w, w);
  vw = iprod(v, w);
  vxw = xprod(v, w);
  nvw = sqrt(v2 * w2);

  a = v2 + 2*vw + w2;
  b = v2 + 2*nvw + w2;
  c = 4*nvw;
  if (vxw == 0 || a == 0) {
    goto degenerate;
  }
  /* assert: a,b,c >= 0, b*b - a*c >= 0, and 0 <= b - sqrt(b*b - a*c) <= a */
  y = (b - sqrt(b*b - a*c)) / a;
  G = interval(y, C, interval(0.5, A, B));

  bulge1 = bulge(sub(A,G), v);
  bulge2 = bulge(w, sub(B,G));

  ship_vertex(fout, layer, A, -bulge1);
  ship_vertex(fout, layer, G, -bulge2);
  return;

 degenerate:
  ship_vertex(fout, layer, A, 0);

  return;
}

/* produce a smooth from A to D, tangent to AB at A and to CD at D.
   This is similar to a Bezier curve, except that our curve will be
   made up of up to 4 circular arcs. This is particularly intended for
   the case when AD and BC are parallel. Like arcs(), don't output the
   final vertex. */
static void pseudo_bezier(FILE *fout, const char *layer, dpoint_t A, dpoint_t B, dpoint_t C, dpoint_t D) {
  dpoint_t E = interval(0.75, A, B);
  dpoint_t G = interval(0.75, D, C);
  dpoint_t F = interval(0.5, E, G);

  pseudo_quad(fout, layer, A, E, F);
  pseudo_quad(fout, layer, F, G, D);
  return;
}

/* ---------------------------------------------------------------------- */
/* functions for converting a path to a DXF polyline */

/* do one path. */
static int dxf_path(FILE *fout, const char *layer, potrace_curve_t *curve, trans_t t) {
  int i;
  dpoint_t *c, *c1;
  int n = curve->n;

  ship_polyline(fout, layer, 1);

  for (i=0; i<n; i++) {
    c = curve->c[i];
    c1 = curve->c[mod(i-1,n)];
    switch (curve->tag[i]) {
    case POTRACE_CORNER:
      ship_vertex(fout, layer, trans(c1[2], t), 0);
      ship_vertex(fout, layer, trans(c[1], t), 0);
      break;
    case POTRACE_CURVETO:
      pseudo_bezier(fout, layer, trans(c1[2], t), trans(c[0], t), trans(c[1], t), trans(c[2], t));
      break;
    }
  }
  ship_seqend(fout);
  return 0;
}

/* ---------------------------------------------------------------------- */
/* Backend. */

/* public interface for DXF */
int page_dxf(FILE *fout, potrace_path_t *plist, imginfo_t *imginfo) {
  potrace_path_t *p;
  trans_t t;
  const char *layer = "0";

  /* set up the coordinate transform (rotation) */
  t.bb[0] = imginfo->trans.bb[0]+imginfo->lmar+imginfo->rmar;
  t.bb[1] = imginfo->trans.bb[1]+imginfo->tmar+imginfo->bmar;
  t.orig[0] = imginfo->trans.orig[0]+imginfo->lmar;
  t.orig[1] = imginfo->trans.orig[1]+imginfo->bmar;
  t.x[0] = imginfo->trans.x[0];
  t.x[1] = imginfo->trans.x[1];
  t.y[0] = imginfo->trans.y[0];
  t.y[1] = imginfo->trans.y[1];

  ship_comment(fout, "DXF data, created by " POTRACE " " VERSION ", written by Peter Selinger 2001-2017");

  /* header section */
  ship_section(fout, "HEADER");

  /* variables */
  ship(fout, 9, "$ACADVER");
  ship(fout, 1, "AC1006");
  ship(fout, 9, "$EXTMIN");
  ship(fout, 10, "%f", 0.0);
  ship(fout, 20, "%f", 0.0);
  ship(fout, 30, "%f", 0.0);
  ship(fout, 9, "$EXTMAX");
  ship(fout, 10, "%f", t.bb[0]);
  ship(fout, 20, "%f", t.bb[1]);
  ship(fout, 30, "%f", 0.0);

  ship_endsec(fout);

  /* entities section */
  ship_section(fout, "ENTITIES");

  /* write paths */
  list_forall (p, plist) {
    dxf_path(fout, layer, &p->curve, t);
  }

  ship_endsec(fout);
  ship_eof(fout);

  fflush(fout);
  return 0;
}

