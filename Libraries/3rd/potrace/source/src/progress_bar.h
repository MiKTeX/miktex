/* Copyright (C) 2001-2017 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

/* functions to render a progress bar for main.c. We provide a
   standard and a simplified progress bar. */

#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

#include "potracelib.h"

/* structure to hold a progress bar interface */
struct progress_bar_s {
  int (*init)(potrace_progress_t *prog, const char *filename, int count);
  void (*term)(potrace_progress_t *prog);
};
typedef struct progress_bar_s progress_bar_t;

extern progress_bar_t *progress_bar_vt100;
extern progress_bar_t *progress_bar_simplified;



#endif /* PROGRESS_BAR_H */

