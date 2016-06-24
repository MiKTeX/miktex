/* Copyright (C) 2001-2015 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

/* functions to render a progress bar for main.c. We provide a
   standard and a simplified progress bar. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "potracelib.h"
#include "progress_bar.h"

/* ---------------------------------------------------------------------- */
/* vt100 progress bar */

#define COL0 "\033[G"  /* reset cursor to column 0 */

struct vt100_progress_s {
  char name[22];          /* filename for status bar */
  double dnext;           /* threshold value for next tick */
};
typedef struct vt100_progress_s vt100_progress_t;

/* print a progress bar using vt100 control characters. This is a
   callback function that is potentially called often; thus, it has
   been optimized for the typical case, which is when the progress bar
   does not need updating. */
static void vt100_progress(double d, void *data) {
  vt100_progress_t *p = (vt100_progress_t *)data;
  static char b[] = "========================================";
  int tick;    /* number of visible tickmarks, 0..40 */
  int perc;    /* visible percentage, 0..100 */

  /* note: the 0.01 and 0.025 ensure that we always end on 40
     tickmarks and 100%, despite any rounding errors. The 0.995
     ensures that tick always increases when d >= p->dnext. */
  if (d >= p->dnext) {
    tick = (int) floor(d*40+0.01);
    perc = (int) floor(d*100+0.025);
    fprintf(stderr, "%-21s |%-40s| %d%% "COL0"", p->name, b+40-tick, perc);
    fflush(stderr);
    p->dnext = (tick+0.995) / 40.0;
  }
}

/* Initialize progress bar. Return 0 on success or 1 on failure with
   errno set. */
static int init_vt100_progress(potrace_progress_t *prog, const char *filename, int count) {
  vt100_progress_t *p;
  const char *q, *s;
  int len;

  p = (vt100_progress_t *) malloc(sizeof(vt100_progress_t));
  if (!p) {
    return 1;
  }

  /* initialize callback function's data */
  p->dnext = 0;

  if (count != 0) {
    sprintf(p->name, " (p.%d):", count+1);
  } else {
    s = filename;
    if ((q = strrchr(s, '/')) != NULL) {
      s = q+1;
    }
    len = strlen(s);
    strncpy(p->name, s, 21);
    p->name[20] = 0;
    if (len > 20) {
      p->name[17] = '.';
      p->name[18] = '.';
      p->name[19] = '.';
    }
    strcat(p->name, ":");
  }

  /* initialize progress parameters */
  prog->callback = &vt100_progress;
  prog->data = (void *)p;
  prog->min = 0.0;
  prog->max = 1.0;
  prog->epsilon = 0.0;
  
  /* draw first progress bar */
  vt100_progress(0.0, prog->data);
  return 0;
}

/* Finalize the progress bar. */
static void term_vt100_progress(potrace_progress_t *prog) {
  fprintf(stderr, "\n");
  fflush(stderr);
  free(prog->data);
  return;
}

/* progress bar interface structure */
static progress_bar_t progress_bar_vt100_struct = {
  init_vt100_progress,
  term_vt100_progress,
};
progress_bar_t *progress_bar_vt100 = &progress_bar_vt100_struct;

/* ---------------------------------------------------------------------- */
/* another progress bar for dumb terminals */

struct simplified_progress_s {
  int n;                  /* number of ticks displayed so far */
  double dnext;           /* threshold value for next tick */
};
typedef struct simplified_progress_s simplified_progress_t;

/* print a simplified progress bar, not using any special tty control
   codes. Optimized for frequent calling. */
static void simplified_progress(double d, void *data) {
  simplified_progress_t *p = (simplified_progress_t *)data;
  int tick;    /* number of visible tickmarks, 0..40 */

  /* note: the 0.01 and 0.025 ensure that we always end on 40
     tickmarks and 100%, despite any rounding errors. The 0.995
     ensures that tick always increases when d >= p->dnext. */
  if (d >= p->dnext) {
    tick = (int) floor(d*40+0.01);
    while (p->n < tick) {
      fputc('=', stderr);
      p->n++;
    }
    fflush(stderr);
    p->dnext = (tick+0.995) / 40.0;
  }
}

/* Initialize parameters for simplified progress bar. Return 0 on
   success or 1 on error with errno set. */
static int init_simplified_progress(potrace_progress_t *prog, const char *filename, int count) {
  simplified_progress_t *p;
  const char *q, *s;
  int len;
  char buf[22];

  p = (simplified_progress_t *) malloc(sizeof(simplified_progress_t));
  if (!p) {
    return 1;
  }

  /* initialize callback function's data */
  p->n = 0;
  p->dnext = 0;

  if (count != 0) {
    sprintf(buf, " (p.%d):", count+1);
  } else {
    s = filename;
    if ((q = strrchr(s, '/')) != NULL) {
      s = q+1;
    }
    len = strlen(s);
    strncpy(buf, s, 21);
    buf[20] = 0;
    if (len > 20) {
      buf[17] = '.';
      buf[18] = '.';
      buf[19] = '.';
    }
    strcat(buf, ":");
  }

  fprintf(stderr, "%-21s |", buf);

  /* initialize progress parameters */
  prog->callback = &simplified_progress;
  prog->data = (void *)p;
  prog->min = 0.0;
  prog->max = 1.0;
  prog->epsilon = 0.0;
  
  /* draw first progress bar */
  simplified_progress(0.0, prog->data);
  return 0;
}

static void term_simplified_progress(potrace_progress_t *prog) {
  simplified_progress_t *p = (simplified_progress_t *)prog->data;

  simplified_progress(1.0, (void *)p);
  fprintf(stderr, "| 100%%\n");
  fflush(stderr);
  free(p);
  return;
}

/* progress bar interface structure */
static progress_bar_t progress_bar_simplified_struct = {
  init_simplified_progress,
  term_simplified_progress,
};
progress_bar_t *progress_bar_simplified = &progress_bar_simplified_struct;

