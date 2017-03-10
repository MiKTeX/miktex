/* Copyright (C) 2001-2017 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */


#ifndef BACKEND_PGM_H
#define BACKEND_PGM_H

#include <stdio.h>

#include "potracelib.h"
#include "main.h"

int page_pgm(FILE *fout, potrace_path_t *plist, imginfo_t *imginfo);

#endif /* BACKEND_PGM_H */
