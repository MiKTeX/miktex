//========================================================================
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2014 Rodrigo Rivas Costa <rodrigorivascosta@gmail.com>
// Copyright (C) 2014 Adrian Johnson <ajohnson@redneon.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <cstdint>
#include <cairo.h>
#include "goo/gmem.h"
#include "goo/GooString.h"

#ifdef CAIRO_HAS_WIN32_SURFACE

#    include <cairo-win32.h>

void win32SetupPrinter(GooString *printer, GooString *printOpt, bool duplex, bool setupdlg);
void win32ShowPrintDialog(bool *expand, bool *noShrink, bool *noCenter, bool *usePDFPageSize, bool *allPages, int *firstPage, int *lastPage, int maxPages);
cairo_surface_t *win32BeginDocument(GooString *inputFileName, GooString *outputFileName);
void win32BeginPage(double *w, double *h, bool changePageSize, bool useFullPage);
void win32EndPage(GooString *imageFileName);
void win32EndDocument();

#endif // CAIRO_HAS_WIN32_SURFACE
