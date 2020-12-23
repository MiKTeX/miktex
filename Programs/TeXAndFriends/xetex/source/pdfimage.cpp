/****************************************************************************\
 Part of the XeTeX typesetting system
 Copyright (c) 1994-2008 by SIL International
 Copyright (c) 2009-2018 by Jonathan Kew

 SIL Author(s): Jonathan Kew

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the copyright holders
shall not be used in advertising or otherwise to promote the sale,
use or other dealings in this Software without prior written
authorization from the copyright holders.
\****************************************************************************/

#if defined(MIKTEX)
#include "miktex-first.h"
#endif
#include <w2c/config.h>

#include "XeTeX_ext.h"

#include "pdfimage.h"

/* 
 * From TeX Live 2021, we use pplib by Pawe\l Jackowski instead of
 * libpoppler
 */
extern "C" {
#include "ppapi.h"
}

#include "XeTeX_ext.h"

/* use our own fmin function because it seems to be missing on certain platforms */
inline double
my_fmin(double x, double y)
{
	return (x < y) ? x : y;
}

int
pdf_get_rect(char* filename, int page_num, int pdf_box, realrect* box)
	/* return the box converted to TeX points */
{
	ppdoc*	doc = ppdoc_load(filename);

	if (!doc) {
		return -1;
	}

	int	pages = ppdoc_page_count(doc);

	if (page_num > pages)
		page_num = pages;
	if (page_num < 0)
		page_num = pages + 1 + page_num;
	if (page_num < 1)
		page_num = 1;

	ppref*	page = ppdoc_page(doc, page_num);
	ppdict*	pdict = page->object.dict;

	pprect	Rect;
	pprect*	r;

	switch (pdf_box) {
		default:
		case pdfbox_crop:
			r = ppdict_get_box(pdict, "CropBox", &Rect);
			break;
		case pdfbox_media:
			r = ppdict_get_box(pdict, "MediaBox", &Rect);
			break;
		case pdfbox_bleed:
			r = ppdict_get_box(pdict, "BleedBox", &Rect);
			break;
		case pdfbox_trim:
			r = ppdict_get_box(pdict, "TrimBox", &Rect);
			break;
		case pdfbox_art:
			r = ppdict_get_box(pdict, "ArtBox", &Rect);
			break;
	}

/*
 *  In pplib, r can be NULL. If r == NULL, we try "CropBox",
 *  "MediaBox",  "BleedBox", "TrimBox", "ArtBox" in this order.
 */
	if (!r) {
		r = ppdict_get_box(pdict, "CropBox", &Rect);
	}
	if (!r) {
		r = ppdict_get_box(pdict, "MediaBox", &Rect);
	}
	if (!r) {
		r = ppdict_get_box(pdict, "BleedBox", &Rect);
	}
	if (!r) {
		r = ppdict_get_box(pdict, "TrimBox", &Rect);
	}
	if (!r) {
		r = ppdict_get_box(pdict, "ArtBox", &Rect);
	}

/*
 * If r == NULL, return error.
 */
	if (!r) {
		return -1;
	}

	ppint RotAngle = 0;
	(void)ppdict_get_int(pdict, "Rotate", &RotAngle);
	RotAngle = RotAngle % 360;
	if (RotAngle < 0)
		RotAngle += 360;
	if (RotAngle == 90 || RotAngle == 270) {
		box->wd = 72.27 / 72 * fabs(r->ry - r->ly);
		box->ht = 72.27 / 72 * fabs(r->rx - r->lx);
	} else {
		box->wd = 72.27 / 72 * fabs(r->rx - r->lx);
		box->ht = 72.27 / 72 * fabs(r->ry - r->ly);
	}
	box->x  = 72.27 / 72 * my_fmin(r->lx, r->rx);
	box->y  = 72.27 / 72 * my_fmin(r->ly, r->ry);

	ppdoc_free(doc);

	return 0;
}

int
pdf_count_pages(char* filename)
{
	int	pages = 0;
	ppdoc*	doc = ppdoc_load(filename);

	if (!doc) {
		return 0;
	}

	pages = ppdoc_page_count(doc);
	if (doc) ppdoc_free(doc);

	return pages;
}
