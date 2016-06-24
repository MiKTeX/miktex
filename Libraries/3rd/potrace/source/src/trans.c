/* Copyright (C) 2001-2015 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

/* calculations with coordinate transformations and bounding boxes */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <string.h>

#include "trans.h"
#include "bbox.h"

/* rotate the coordinate system counterclockwise by alpha degrees. The
   new bounding box will be the smallest box containing the rotated
   old bounding box */
void trans_rotate(trans_t *r, double alpha) {
  double s, c, x0, x1, y0, y1, o0, o1;
  trans_t t_struct;
  trans_t *t = &t_struct;

  memcpy(t, r, sizeof(trans_t));

  s = sin(alpha/180*M_PI);
  c = cos(alpha/180*M_PI);

  /* apply the transformation matrix to the sides of the bounding box */
  x0 = c * t->bb[0];
  x1 = s * t->bb[0];
  y0 = -s * t->bb[1];
  y1 = c * t->bb[1];

  /* determine new bounding box, and origin of old bb within new bb */
  r->bb[0] = fabs(x0) + fabs(y0);
  r->bb[1] = fabs(x1) + fabs(y1);
  o0 = - min(x0,0) - min(y0,0);
  o1 = - min(x1,0) - min(y1,0);

  r->orig[0] = o0 + c * t->orig[0] - s * t->orig[1];
  r->orig[1] = o1 + s * t->orig[0] + c * t->orig[1];
  r->x[0] = c * t->x[0] - s * t->x[1];
  r->x[1] = s * t->x[0] + c * t->x[1];
  r->y[0] = c * t->y[0] - s * t->y[1];
  r->y[1] = s * t->y[0] + c * t->y[1];
}

/* return the standard cartesian coordinate system for an w x h rectangle. */
void trans_from_rect(trans_t *r, double w, double h) {
  r->bb[0] = w;
  r->bb[1] = h;
  r->orig[0] = 0.0;
  r->orig[1] = 0.0;
  r->x[0] = 1.0;
  r->x[1] = 0.0;
  r->y[0] = 0.0;
  r->y[1] = 1.0;
  r->scalex = 1.0;
  r->scaley = 1.0;
}

/* rescale the coordinate system r by factor sc >= 0. */
void trans_rescale(trans_t *r, double sc) {
  r->bb[0] *= sc;
  r->bb[1] *= sc;
  r->orig[0] *= sc;
  r->orig[1] *= sc;
  r->x[0] *= sc;
  r->x[1] *= sc;
  r->y[0] *= sc;
  r->y[1] *= sc;
  r->scalex *= sc;
  r->scaley *= sc;
}

/* rescale the coordinate system to size w x h */
void trans_scale_to_size(trans_t *r, double w, double h) {
  double xsc = w/r->bb[0];
  double ysc = h/r->bb[1];

  r->bb[0] = w;
  r->bb[1] = h;
  r->orig[0] *= xsc;
  r->orig[1] *= ysc;
  r->x[0] *= xsc;
  r->x[1] *= ysc;
  r->y[0] *= xsc;
  r->y[1] *= ysc;
  r->scalex *= xsc;
  r->scaley *= ysc;
  
  if (w<0) {
    r->orig[0] -= w;
    r->bb[0] = -w;
  }
  if (h<0) {
    r->orig[1] -= h;
    r->bb[1] = -h;
  }
}

/* adjust the bounding box to the actual vector outline */
void trans_tighten(trans_t *r, potrace_path_t *plist) {
  interval_t i;
  dpoint_t dir;
  int j;
  
  /* if pathlist is empty, do nothing */
  if (!plist) {
    return;
  }

  for (j=0; j<2; j++) {
    dir.x = r->x[j];
    dir.y = r->y[j];
    path_limits(plist, dir, &i);
    if (i.min == i.max) {
      /* make the extent non-zero to avoid later division by zero errors */
      i.max = i.min+0.5;
      i.min = i.min-0.5;
    }
    r->bb[j] = i.max - i.min;
    r->orig[j] = -i.min;
  }
}
