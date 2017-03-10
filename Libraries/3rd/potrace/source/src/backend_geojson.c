/* Copyright (C) 2001-2017 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

/* The GeoJSON backend of Potrace. */
/* Written 2012 by Christoph Hormann <chris_hormann@gmx.de> */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "potracelib.h"
#include "curve.h"
#include "main.h"
#include "backend_geojson.h"
#include "lists.h"
#include "auxiliary.h"

/* ---------------------------------------------------------------------- */
/* auxiliary */

/* return a point on a 1-dimensional Bezier segment */
static inline double bezier(double t, double x0, double x1, double x2, double x3) {
  double s = 1-t;
  return s*s*s*x0 + 3*(s*s*t)*x1 + 3*(t*t*s)*x2 + t*t*t*x3;
}

static const char *format = "%f";

/* Convert a floating-point number to a string, using a pre-determined
   format. The format must be previously set with set_format().
   Returns one of a small number of statically allocated strings. */
static char *round_to_unit(double x) {
  static int n = 0;
  static char buf[4][100];

  n++;
  if (n >= 4) {
    n = 0;
  }
  sprintf(buf[n], format, x);
  return buf[n];
}

/* Select a print format for floating point numbers, appropriate for
   the given scaling and info.unit. Note: the format must be so that
   the resulting number fits into a buffer of size 100. */
static void set_format(trans_t tr) {
  double s;
  int d;
  static char buf[10];

  s = min(fabs(tr.scalex), fabs(tr.scaley));
  if (info.unit != 0.0 && s != 0.0) {
    d = (int)ceil(log(info.unit/s) / log(10));
  } else {
    d = 0;
  }
  if (d <= 0) {
    format = "%.0f";
  } else if (d <= 20) {
    sprintf(buf, "%%.%df", d);
    format = buf;
  } else {
    format = "%e";
  }  
}

/* ---------------------------------------------------------------------- */
/* path-drawing auxiliary functions */

static dpoint_t cur;

static void geojson_moveto(FILE *fout, dpoint_t p, trans_t tr) {
  dpoint_t q;

  q = trans(p, tr);

  fprintf(fout, "[%s, %s]", round_to_unit(q.x), round_to_unit(q.y));

  cur = q;
}

static void geojson_lineto(FILE *fout, dpoint_t p, trans_t tr) {
  dpoint_t q;

  q = trans(p, tr);

  fprintf(fout, ", [%s, %s]", round_to_unit(q.x), round_to_unit(q.y));

  cur = q;
}

static void geojson_curveto(FILE *fout, dpoint_t p1, dpoint_t p2, dpoint_t p3, trans_t tr) {
  dpoint_t q1;
  dpoint_t q2;
  dpoint_t q3;
  double step, t;
  int i;
  double x, y;

  q1 = trans(p1, tr);
  q2 = trans(p2, tr);
  q3 = trans(p3, tr);

  step = 1.0 / 8.0;

  for (i=0, t=step; i<8; i++, t+=step) {
    x = bezier(t, cur.x, q1.x, q2.x, q3.x);
    y = bezier(t, cur.y, q1.y, q2.y, q3.y);

    fprintf(fout, ", [%s, %s]", round_to_unit(x), round_to_unit(y));
  }

  cur = q3;
}

/* ---------------------------------------------------------------------- */
/* functions for converting a path to an SVG path element */

static int geojson_path(FILE *fout, potrace_curve_t *curve, trans_t tr) {
  int i;
  dpoint_t *c;
  int m = curve->n;

  fprintf(fout, "      [");

  c = curve->c[m-1];
  geojson_moveto(fout, c[2], tr);

  for (i=0; i<m; i++) {
    c = curve->c[i];
    switch (curve->tag[i]) {
    case POTRACE_CORNER:
      geojson_lineto(fout, c[1], tr);
      geojson_lineto(fout, c[2], tr);
      break;
    case POTRACE_CURVETO:
      geojson_curveto(fout, c[0], c[1], c[2], tr);
      break;
    }
  }

  fprintf(fout, " ]");

  return 0;
}


static void write_polygons(FILE *fout, potrace_path_t *tree, trans_t tr, int first) {
  potrace_path_t *p, *q;

  for (p=tree; p; p=p->sibling) {

    if (!first) fprintf(fout, ",\n");

    fprintf(fout, "{ \"type\": \"Feature\",\n");
    fprintf(fout, "  \"properties\": { },\n");
    fprintf(fout, "  \"geometry\": {\n");
    fprintf(fout, "    \"type\": \"Polygon\",\n");
    fprintf(fout, "    \"coordinates\": [\n");

    geojson_path(fout, &p->curve, tr);

    for (q=p->childlist; q; q=q->sibling) {
      fprintf(fout, ",\n");
      geojson_path(fout, &q->curve, tr);
    }

    fprintf(fout, "    ]\n");
    fprintf(fout, "  }\n");
    fprintf(fout, "}");

    for (q=p->childlist; q; q=q->sibling) {
      write_polygons(fout, q->childlist, tr, 0);
    }

    first = 0;
  }
}

/* ---------------------------------------------------------------------- */
/* Backend. */

/* public interface for GeoJSON */
int page_geojson(FILE *fout, potrace_path_t *plist, imginfo_t *imginfo) {

  trans_t tr;

  /* set up the coordinate transform (rotation) */
  tr.bb[0] = imginfo->trans.bb[0]+imginfo->lmar+imginfo->rmar;
  tr.bb[1] = imginfo->trans.bb[1]+imginfo->tmar+imginfo->bmar;
  tr.orig[0] = imginfo->trans.orig[0]+imginfo->lmar;
  tr.orig[1] = imginfo->trans.orig[1]+imginfo->bmar;
  tr.x[0] = imginfo->trans.x[0];
  tr.x[1] = imginfo->trans.x[1];
  tr.y[0] = imginfo->trans.y[0];
  tr.y[1] = imginfo->trans.y[1];

  /* set the print format for floating point numbers */
  set_format(imginfo->trans);

  /* header */
  fprintf(fout, "{\n");
  fprintf(fout, "\"type\": \"FeatureCollection\",\n");
  fprintf(fout, "\"features\": [\n");

  write_polygons(fout, plist, tr, 1);

  /* write footer */
  fprintf(fout, "\n]\n");
  fprintf(fout, "}\n");

  fflush(fout);

  return 0;
}

