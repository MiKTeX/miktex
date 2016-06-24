/* Copyright (C) 2001-2015 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */


/* The PGM backend of Potrace. Here we custom-render a set of Bezier
   curves and output the result as a greymap. This is merely a
   convenience, as the same could be achieved by piping the EPS output
   through ghostscript. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>

#include "backend_pgm.h"
#include "potracelib.h"
#include "lists.h"
#include "greymap.h"
#include "render.h"
#include "main.h"
#include "auxiliary.h"
#include "trans.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void pgm_path(potrace_curve_t *curve, trans_t t, render_t *rm) {
  dpoint_t *c, c1[3];
  int i;
  int m = curve->n;
  
  c = curve->c[m-1];
  c1[2] = trans(c[2], t);
  render_moveto(rm, c1[2].x, c1[2].y);
  
  for (i=0; i<m; i++) {
    c = curve->c[i];
    switch (curve->tag[i]) {
    case POTRACE_CORNER:
      c1[1] = trans(c[1], t);
      c1[2] = trans(c[2], t);
      render_lineto(rm, c1[1].x, c1[1].y);
      render_lineto(rm, c1[2].x, c1[2].y);
      break;
    case POTRACE_CURVETO:
      c1[0] = trans(c[0], t);
      c1[1] = trans(c[1], t);
      c1[2] = trans(c[2], t);
      render_curveto(rm, c1[0].x, c1[0].y, c1[1].x, c1[1].y, c1[2].x, c1[2].y);
      break;
    }
  }
}

int page_pgm(FILE *fout, potrace_path_t *plist, imginfo_t *imginfo) {
  potrace_path_t *p;
  greymap_t *gm;
  render_t *rm;
  int w, h;
  trans_t t;
  int mode;
  char *comment = "created by "POTRACE" "VERSION", written by Peter Selinger 2001-2015";

  t.bb[0] = imginfo->trans.bb[0]+imginfo->lmar+imginfo->rmar;
  t.bb[1] = imginfo->trans.bb[1]+imginfo->tmar+imginfo->bmar;
  t.orig[0] = imginfo->trans.orig[0]+imginfo->lmar;
  t.orig[1] = imginfo->trans.orig[1]+imginfo->bmar;
  t.x[0] = imginfo->trans.x[0];
  t.x[1] = imginfo->trans.x[1];
  t.y[0] = imginfo->trans.y[0];
  t.y[1] = imginfo->trans.y[1];

  w = (int)ceil(t.bb[0]);
  h = (int)ceil(t.bb[1]);

  gm = gm_new(w, h);
  if (!gm) {
    return 1;
  }
  rm = render_new(gm);
  if (!rm) {
    return 1;
  }

  gm_clear(gm, 255); /* white */

  list_forall(p, plist) {
    pgm_path(&p->curve, t, rm);
  }

  render_close(rm);

  /* if negative orientation, make sure to invert effect of rendering */
  mode = imginfo->width * imginfo->height < 0 ? GM_MODE_NEGATIVE : GM_MODE_POSITIVE;

  gm_writepgm(fout, rm->gm, comment, 1, mode, info.gamma);

  render_free(rm);
  gm_free(gm);

  return 0;
}

