/*
 *   ttflib.h
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */

#ifndef TTFLIB_H
#define TTFLIB_H

#include "pklib.h"      /* for the `byte' type */

void TTFopen(char *filename, Font *fnt, int new_dpi, int new_ptsize, 
             Boolean quiet);

Boolean TTFprocess(Font *fnt, long Code, byte **bitmap,
                   int *width, int *height, int *hoff, int *voff,
                   Boolean hinting, Boolean quiet);

encoding *TTFget_first_glyphs(Font *fnt, long *array);
void TTFget_subfont(Font *fnt, long *array);

long TTFsearch_PS_name(const char *name);

#endif /* TTFLIB_H */


/* end */
