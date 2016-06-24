/* Copyright (C) 2001-2015 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */


/* The PDF backend of Potrace. Stream compression is optionally
	supplied via the functions in flate.c. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "main.h"
#include "backend_pdf.h"
#include "flate.h"
#include "lists.h"
#include "potracelib.h"
#include "auxiliary.h"

typedef int color_t;

/* ---------------------------------------------------------------------- */
/* auxiliary: growing arrays */

struct intarray_s {
  int size;
  int *data;
};
typedef struct intarray_s intarray_t;

static inline void intarray_init(intarray_t *ar) {
  ar->size = 0;
  ar->data = NULL;
}

static inline void intarray_term(intarray_t *ar) {
  free(ar->data);
  ar->size = 0;
  ar->data = NULL;
}

/* Set ar[n]=val. Expects n>=0. Grows array if necessary. Return 0 on
   success and -1 on error with errno set. */
static inline int intarray_set(intarray_t *ar, int n, int val) {
  int *p;
  int s;

  if (n >= ar->size) {
    s = n+1024;
    p = (int *)realloc(ar->data, s * sizeof(int));
    if (!p) {
      return -1;
    }
    ar->data = p;
    ar->size = s;
  }
  ar->data[n] = val;
  return 0;
}

/* ---------------------------------------------------------------------- */
/* some global variables */

static intarray_t xref;
static int nxref = 0;
static intarray_t pages;
static int npages;
static int streamofs;
static size_t outcount;  /* output file position */

/* ---------------------------------------------------------------------- */
/* functions for interfacing with compression backend */

/* xship: callback function that must be initialized before calling
   any other functions of the "ship" family. xship_file must be
   initialized too. */

/* print the token to f, but filtered through a compression
   filter in case filter!=0 */
static int (*xship)(FILE *f, int filter, char *s, int len);
static FILE *xship_file;

/* ship PDF code, filtered */
static int ship(const char *fmt, ...) {
  va_list args;
  static char buf[4096]; /* static string limit is okay here because
			    we only use constant format strings - for
			    the same reason, it is okay to use
			    vsprintf instead of vsnprintf below. */
  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  buf[4095] = 0;
  va_end(args);

  outcount += xship(xship_file, 1, buf, strlen(buf));
  return 0;
}  

/* ship PDF code, unfiltered */
static int shipclear(char *fmt, ...) {
  static char buf[4096];
  va_list args;

  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  buf[4095] = 0;
  va_end(args);

  outcount += xship(xship_file, 0, buf, strlen(buf));
  return 0;
}

/* set all callback functions */
static void pdf_callbacks(FILE *fout) {

  if (info.compress) {
    xship = pdf_xship;
  } else {
    xship = dummy_xship;
  }
  xship_file = fout;
}  

/* ---------------------------------------------------------------------- */
/* PDF path-drawing auxiliary functions */

/* coordinate quantization */
static inline point_t unit(dpoint_t p) {
  point_t q;

  q.x = (long)(floor(p.x*info.unit+.5));
  q.y = (long)(floor(p.y*info.unit+.5));
  return q;
}

static void pdf_coords(dpoint_t p) {
  point_t cur = unit(p);
  ship("%ld %ld ", cur.x, cur.y);
}

static void pdf_moveto(dpoint_t p) {
  pdf_coords(p);
  ship("m\n");
}

static void pdf_lineto(dpoint_t p) {
  pdf_coords(p);
  ship("l\n");
}

static void pdf_curveto(dpoint_t p1, dpoint_t p2, dpoint_t p3) {
  point_t q1, q2, q3;

  q1 = unit(p1);
  q2 = unit(p2);
  q3 = unit(p3);

  ship("%ld %ld %ld %ld %ld %ld c\n", q1.x, q1.y, q2.x, q2.y, q3.x, q3.y);
}

/* this procedure returns a statically allocated string */
static char *pdf_colorstring(const color_t col) {
  double r, g, b;
  static char buf[100];

  r = (col & 0xff0000) >> 16;
  g = (col & 0x00ff00) >> 8;
  b = (col & 0x0000ff) >> 0;

  if (r==0 && g==0 && b==0) {
    return "0 g";
  } else if (r==255 && g==255 && b==255) {
    return "1 g";
  } else if (r == g && g == b) {
    sprintf(buf, "%.3f g", r/255.0);
    return buf;
  } else {
    sprintf(buf, "%.3f %.3f %.3f rg", r/255.0, g/255.0, b/255.0);
    return buf;
  }
}

static color_t pdf_color = -1;

static void pdf_setcolor(const color_t col) {
  if (col == pdf_color) {
    return;
  }
  pdf_color = col;

  ship("%s\n", pdf_colorstring(col));
}

/* explicit encoding, does not use special macros */
static int pdf_path(potrace_curve_t *curve) {
  int i;
  dpoint_t *c;
  int m = curve->n;

  c = curve->c[m-1];
  pdf_moveto(c[2]);

  for (i=0; i<m; i++) {
    c = curve->c[i];
    switch (curve->tag[i]) {
    case POTRACE_CORNER:
      pdf_lineto(c[1]);
      pdf_lineto(c[2]);
      break;
    case POTRACE_CURVETO:
      pdf_curveto(c[0], c[1], c[2]);
      break;
    }
  }
  return 0;
}

/* ---------------------------------------------------------------------- */
/* Backends for various types of output. */

/* Normal output: black on transparent */
static int render0(potrace_path_t *plist) {
  potrace_path_t *p;

  pdf_setcolor(info.color);
  list_forall (p, plist) {
    pdf_path(&p->curve);
    ship("h\n");
    if (p->next == NULL || p->next->sign == '+') {
      ship("f\n");
    }
  }
  return 0;
}

/* Opaque output: alternating black and white */
static int render0_opaque(potrace_path_t *plist) {
  potrace_path_t *p;
  
  list_forall (p, plist) {
    pdf_path(&p->curve);
    ship("h\n");
    pdf_setcolor(p->sign=='+' ? info.color : info.fillcolor);
    ship("f\n");
  }
  return 0;
}

/* select the appropriate rendering function from above */
static int pdf_render(potrace_path_t *plist)
{
  if (info.opaque) {
    return render0_opaque(plist);
  }
  return render0(plist);
}  

/* ---------------------------------------------------------------------- */
/* PDF header and footer */

int init_pdf(FILE *fout)
{
        intarray_init(&xref);
	intarray_init(&pages);
	nxref = 0;
	npages = 0;

	/* set callback functions for shipping routines */
	pdf_callbacks(fout);
	outcount = 0;

	shipclear("%%PDF-1.3\n");

	intarray_set(&xref, nxref++, outcount);
	shipclear("1 0 obj\n<</Type/Catalog/Pages 3 0 R>>\nendobj\n");

	intarray_set(&xref, nxref++, outcount);
	shipclear("2 0 obj\n"
		"<</Creator"
		"("POTRACE" "VERSION", written by Peter Selinger 2001-2015)>>\n"
		"endobj\n");

	/* delay obj #3 (pages) until end */
	nxref++;

	fflush(fout);
	return 0;
}

int term_pdf(FILE *fout)
{
	int startxref;
	int i;

	pdf_callbacks(fout);

	intarray_set(&xref, 2, outcount);
	shipclear("3 0 obj\n"
		"<</Type/Pages/Count %d/Kids[\n", npages);
	for (i = 0; i < npages; i++)
		shipclear("%d 0 R\n", pages.data[i]);
	shipclear("]>>\nendobj\n");

	startxref = outcount;

	shipclear("xref\n0 %d\n", nxref + 1);
	shipclear("0000000000 65535 f \n");
	for (i = 0; i < nxref; i++)
		shipclear("%0.10d 00000 n \n", xref.data[i]);

	shipclear("trailer\n<</Size %d/Root 1 0 R/Info 2 0 R>>\n", nxref + 1);
	shipclear("startxref\n%d\n%%%%EOF\n", startxref);

	fflush(fout);
	intarray_term(&xref);
	intarray_term(&pages);
	return 0;
}

/* if largebbox is set, set bounding box to pagesize. */
static void pdf_pageinit(imginfo_t *imginfo, int largebbox)
{
	double origx = imginfo->trans.orig[0] + imginfo->lmar;
	double origy = imginfo->trans.orig[1] + imginfo->bmar;
	double dxx = imginfo->trans.x[0] / info.unit;
	double dxy = imginfo->trans.x[1] / info.unit;
	double dyx = imginfo->trans.y[0] / info.unit;
	double dyy = imginfo->trans.y[1] / info.unit;

	double pagew = imginfo->trans.bb[0]+imginfo->lmar+imginfo->rmar;
	double pageh = imginfo->trans.bb[1]+imginfo->tmar+imginfo->bmar;

	pdf_color = -1;

	intarray_set(&xref, nxref++, outcount);
	shipclear("%d 0 obj\n", nxref);
	shipclear("<</Type/Page/Parent 3 0 R/Resources<</ProcSet[/PDF]>>");
	if (largebbox) {
	  shipclear("/MediaBox[0 0 %d %d]", info.paperwidth, info.paperheight);
	} else {
	  shipclear("/MediaBox[0 0 %f %f]", pagew, pageh);
	}
	shipclear("/Contents %d 0 R>>\n", nxref + 1);
	shipclear("endobj\n");

	intarray_set(&pages, npages++, nxref);

	intarray_set(&xref, nxref++, outcount);
	shipclear("%d 0 obj\n", nxref);
	if (info.compress)
		shipclear("<</Filter/FlateDecode/Length %d 0 R>>\n", nxref + 1);
	else
		shipclear("<</Length %d 0 R>>\n", nxref + 1);
	shipclear("stream\n");

	streamofs = outcount;

	ship("%f %f %f %f %f %f cm\n", dxx, dxy, dyx, dyy, origx, origy);
}

static void pdf_pageterm(void)
{
	int streamlen;

	shipclear("");

	streamlen = outcount - streamofs;
	shipclear("endstream\nendobj\n");
	
	intarray_set(&xref, nxref++, outcount);
	shipclear("%d 0 obj\n%d\nendobj\n", nxref, streamlen);
}

int page_pdf(FILE *fout, potrace_path_t *plist, imginfo_t *imginfo)
{
  int r;

  pdf_callbacks(fout);

  pdf_pageinit(imginfo, 0);

  r = pdf_render(plist);
  if (r) {
    return r;
  }

  pdf_pageterm();

  fflush(fout);

  return 0;
}

int page_pdfpage(FILE *fout, potrace_path_t *plist, imginfo_t *imginfo)
{
  int r;

  pdf_callbacks(fout);

  pdf_pageinit(imginfo, 1);

  r = pdf_render(plist);
  if (r) {
    return r;
  }

  pdf_pageterm();

  fflush(fout);

  return 0;
}

