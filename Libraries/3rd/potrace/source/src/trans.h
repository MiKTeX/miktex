/* Copyright (C) 2001-2017 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */


#ifndef TRANS_H
#define TRANS_H

#include "auxiliary.h"

/* structure to hold a coordinate transformation */
struct trans_s {
  double bb[2];    /* dimensions of bounding box */
  double orig[2];  /* origin relative to bounding box */
  double x[2];     /* basis vector for the "x" direction */
  double y[2];     /* basis vector for the "y" direction */
  double scalex, scaley;  /* redundant info for some backends' benefit */
};
typedef struct trans_s trans_t;

/* apply a coordinate transformation to a point */
static inline dpoint_t trans(dpoint_t p, trans_t t) {
  dpoint_t res;

  res.x = t.orig[0] + p.x * t.x[0] + p.y * t.y[0];
  res.y = t.orig[1] + p.x * t.x[1] + p.y * t.y[1];
  return res;
}

void trans_rotate(trans_t *r, double alpha);
void trans_from_rect(trans_t *r, double w, double h);
void trans_rescale(trans_t *r, double sc);
void trans_scale_to_size(trans_t *r, double w, double h);
void trans_tighten(trans_t *r, potrace_path_t *plist);

#endif /* TRANS_H */
