/* Copyright (C) 2001-2015 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

#ifndef BBOX_H
#define BBOX_H

#include "potracelib.h"

/* an interval [min, max] */
struct interval_s {
  double min, max;
};
typedef struct interval_s interval_t;

void path_limits(potrace_path_t *path, potrace_dpoint_t dir, interval_t *i);

#endif /* BBOX_H */
