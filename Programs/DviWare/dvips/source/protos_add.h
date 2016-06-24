#ifndef _PROTOS_ADD_H
#define _PROTOS_ADD_H

#include "protos.h"

/****** additional global variables ******
 *
 *   We can declare 'dvistack' and 'name' only in some files, because
 *   otherwise they would conflict with equally named static variables.
 */

/* global variables from dopage.c */
extern struct dvistack {
  integer hh, vv;
  integer h, v, w, x, y, z, dir;
} stack[];

/* global variables from loadfont.c */
extern char name[];

#endif
