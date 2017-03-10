/* Copyright (C) 2001-2017 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */


/* The Postscript backend of Potrace. This can produce "ps" or "eps"
   output, and different kinds of graphical debugging
   output. Postscript compression is optionally supplied via the
   functions in flate.c. */

#define _XOPEN_SOURCE 500

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "potracelib.h"
#include "curve.h"
#include "main.h"
#include "backend_eps.h"
#include "flate.h"
#include "lists.h"
#include "auxiliary.h"

#define SAFE_CALLOC(var, n, typ) \
  if ((var = (typ *)calloc(n, sizeof(typ))) == NULL) goto calloc_error 

typedef int color_t;

#define black  0x000000
#define red    0xff0000
#define green  0x008000
#define blue   0x0000ff

#define TRY(x) if (x) goto try_error

/* ---------------------------------------------------------------------- */
/* functions for interfacing with compression backend */

/* xship: callback function that must be initialized before calling
   any other functions of the "ship" family. xship_file must be
   initialized too. */

/* print the token to f, but filtered through a compression
   filter in case filter!=0 */
static int (*xship)(FILE *f, int filter, const char *s, int len);
static FILE *xship_file;

/* ship postscript code, filtered */
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

  xship(xship_file, 1, buf, strlen(buf));
  return 0;
}  

/* ship a postscript comment, unfiltered */
static int shipcom(const char *fmt, ...) {
  static char buf[4096];
  va_list args;

  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  buf[4095] = 0;
  va_end(args);

  xship(xship_file, 0, buf, strlen(buf));
  return 0;
}

/* set all callback functions */
static void eps_callbacks(FILE *fout) {
  if (info.compress && info.pslevel==2) {
    xship = lzw_xship;
  } else if (info.compress && info.pslevel==3) {
    xship = flate_xship;
  } else {
    xship = dummy_xship;
  }
  xship_file = fout;
}  

/* ---------------------------------------------------------------------- */
/* postscript path-drawing auxiliary functions */

/* coordinate quantization */
static inline point_t unit(dpoint_t p) {
  point_t q;

  q.x = (long)(floor(p.x*info.unit+.5));
  q.y = (long)(floor(p.y*info.unit+.5));
  return q;
}

/* current point */
static point_t cur;

static void eps_coords(dpoint_t p) {
  cur = unit(p);
  ship("%ld %ld ", cur.x, cur.y);
}

static void eps_rcoords(dpoint_t p) {
  point_t q;

  q = unit(p);
  ship("%ld %ld ", q.x-cur.x, q.y-cur.y);
  cur = q;
}

static void eps_moveto(dpoint_t p) {
  eps_coords(p);
  ship("moveto\n");
}

/* move to point + offset */
static void eps_moveto_offs(dpoint_t p, double xoffs, double yoffs) {
  /* note: structs are passed by value, so the following assignment
     does not modify the original struct in the caller */
  p.x += xoffs;
  p.y += yoffs;
  eps_coords(p);
  ship("moveto\n");
}

static void eps_lineto(dpoint_t p) {
  eps_rcoords(p);
  ship("rlineto\n");
}

static void eps_curveto(dpoint_t p1, dpoint_t p2, dpoint_t p3) {
  point_t q1, q2, q3;

  q1 = unit(p1);
  q2 = unit(p2);
  q3 = unit(p3);

  ship("%ld %ld %ld %ld %ld %ld rcurveto\n", q1.x-cur.x, q1.y-cur.y, q2.x-cur.x, q2.y-cur.y, q3.x-cur.x, q3.y-cur.y);
  
  cur = q3;
}

/* this procedure returns a statically allocated string */
static const char *eps_colorstring(const color_t col) {
  double r, g, b;
  static char buf[100];

  r = (col & 0xff0000) >> 16;
  g = (col & 0x00ff00) >> 8;
  b = (col & 0x0000ff) >> 0;

  if (r==0 && g==0 && b==0) {
    return "0 setgray";
  } else if (r==255 && g==255 && b==255) {
    return "1 setgray";
  } else if (r == g && g == b) {
    sprintf(buf, "%.3f setgray", r/255.0);
    return buf;
  } else {
    sprintf(buf, "%.3f %.3f %.3f setrgbcolor", r/255.0, g/255.0, b/255.0);
    return buf;
  }
}

static color_t eps_color = -1;
static double eps_width = -1;

static void eps_setcolor(const color_t col) {
  if (col == eps_color) {
    return;
  }
  eps_color = col;

  ship("%s\n", eps_colorstring(col));
}

static void eps_linewidth(double w) {
  if (w == eps_width) {
    return;
  }
  eps_width = w;
  ship("%f setlinewidth\n", w * info.unit);
}

/* ---------------------------------------------------------------------- */
/* functions for converting a path to postscript code */

/* ---------------------------------------------------------------------- */
/* ASCII encoding */

/* explicit encoding, does not use special macros */
static int eps_path_long(privcurve_t *curve) {
  int i;
  dpoint_t *c;
  int m = curve->n;

  c = curve->c[m-1];
  eps_moveto(c[2]);

  for (i=0; i<m; i++) {
    c = curve->c[i];
    switch (curve->tag[i]) {
    case POTRACE_CORNER:
      eps_lineto(c[1]);
      eps_lineto(c[2]);
      break;
    case POTRACE_CURVETO:
      eps_curveto(c[0], c[1], c[2]);
      break;
    }
  }
  return 0;
}

/* size-optimized encoding relies on special macros */
static int eps_path_short(privcurve_t *curve) {
  int i, i1;
  long int *bq = NULL;  /* bq[m] */
  long int *aq = NULL;  /* aq[m] */
  point_t *v = NULL;    /* v[m] */
  dpoint_t *q = NULL;   /* q[m] */
  double M;
  int m = curve->n;

  SAFE_CALLOC(bq, m, long int);
  SAFE_CALLOC(aq, m, long int);
  SAFE_CALLOC(v, m, point_t);
  SAFE_CALLOC(q, m, dpoint_t);

  /* quantize vertices */
  for (i=0; i<m; i++) {
    v[i] = unit(curve->vertex[i]);
  }

  /* quantize beta */
  for (i=0; i<m; i++) {
    i1 = mod(i+1,m);
    M = max(10, max(abs(v[i1].x-v[i].x), abs(v[i1].y-v[i].y)));
    bq[i] = (int)(M * curve->beta[i] + 0.5);
    if (curve->beta[i] != 0.5) {
      q[i1] = interval(bq[i]/M, dpoint(v[i]), dpoint(v[i1]));
    } else {
      q[i1] = interval(0.5, dpoint(v[i]), dpoint(v[i1]));
    }
  }

  /* quantize alpha */
  for (i=0; i<m; i++) {
    i1 = mod(i+1,m);
    M = max(10, max(max(abs(q[i].x-v[i].x), abs(q[i].y-v[i].y)),
		    max(abs(v[i].x-q[i1].x), abs(v[i].y-q[i1].y))));
    if (curve->tag[i] == POTRACE_CURVETO) {
      aq[i] = (int)(M * curve->alpha[i] + 0.5);
      if (aq[i] > M) {
	aq[i]--;
      }
    }
  }

  /* generate output */
  ship("%ld %ld ", v[m-1].x, v[m-1].y);
  ship("%ld %ld ", v[0].x - v[m-1].x, v[0].y - v[m-1].y);
  if (curve->beta[m-1] == 0.5) {
    ship("i\n");
  } else {
    ship("%ld I\n", bq[m-1]);
  }
  for (i=0; i<m; i++) {
    if (i<m-1) {
      ship("%ld %ld ", v[i+1].x - v[i].x, v[i+1].y - v[i].y);
      if (curve->beta[i] != 0.5) {
	ship("%ld ", bq[i]);
      }
    }
    if (curve->tag[i] == POTRACE_CURVETO) {
      ship(curve->beta[i] == 0.5 ? "%ld c\n" : "%ld C\n", aq[i]);
    } else {
      ship(curve->beta[i] == 0.5 ? "v\n" : "V\n");
    }
  }  

  free(bq);
  free(aq);
  free(v);
  free(q);
  return 0;

 calloc_error:
  free(bq);
  free(aq);
  free(v);
  free(q);
  return 1;
}

static int eps_path(privcurve_t *curve) {
  if (info.longcoding==0 && curve->alphacurve) {
    return eps_path_short(curve);
  } else {
    return eps_path_long(curve);
  }
}

/* ---------------------------------------------------------------------- */
/* functions for rendering various internal data structures, used to
   generate debugging output */

/* output jaggie curve in grey */
static void eps_jaggy(potrace_path_t *plist) {
  potrace_path_t *p;
  int i;

  ship(".9 setgray\n");
  list_forall (p, plist) {
    point_t *pt = p->priv->pt;
    point_t cur, prev;

    if (p->sign == '+') {
      cur = prev = pt[p->priv->len-1];
      eps_moveto(dpoint(cur));
      for (i=0; i<p->priv->len; i++) {
	if (pt[i].x != cur.x && pt[i].y != cur.y) {
	  cur = prev;
	  eps_lineto(dpoint(cur));
	}
	prev = pt[i];
      }
      eps_lineto(dpoint(pt[p->priv->len-1]));
    } else {
      cur = prev = pt[0];
      eps_moveto(dpoint(cur));
      for (i=p->priv->len-1; i>=0; i--) {
	if (pt[i].x != cur.x && pt[i].y != cur.y) {
          cur = prev;
          eps_lineto(dpoint(cur));
        }
        prev = pt[i];
      }
      eps_lineto(dpoint(pt[0]));
    }
    if (p->next == NULL || p->next->sign == '+') {
      ship("fill\n");
    }
  }
}

/* output polygon */
static void eps_polygon(privcurve_t *curve, const color_t col) {
  int i;
  int m = curve->n;

  eps_linewidth(.02);
  eps_setcolor(col);
  eps_moveto(curve->vertex[m-1]);
  for (i=0; i<m; i++) {
    eps_lineto(curve->vertex[i]);
  }
  ship("stroke\n");
}

/* output lines L and parameter alpha */
static void eps_L(privcurve_t *curve, const color_t col) {
  int i, i1;
  double gamma;
  dpoint_t p1, p4, p1l, p4l;
  int m = curve->n;
 
  for (i=0; i<m; i++) {
    i1 = mod(i+1, m);
    gamma = curve->alpha0[i1] * 0.75;
    
    p1 = curve->c[i][2];
    p4 = curve->c[i1][2];
    p1l = interval(gamma, p1, curve->vertex[i1]);
    p4l = interval(gamma, p4, curve->vertex[i1]);
    eps_linewidth(.02);
    eps_setcolor(col);
    eps_moveto(p1l);
    eps_lineto(p4l);
    ship("stroke\n");
    eps_moveto_offs(curve->vertex[i1], -.4, -.4);
    ship("times (%.2f) show\n", curve->alpha0[i1]);
  }
}

/* ---------------------------------------------------------------------- */
/* postscript macros */

/* special macros for size-optimized rendering of Bezier curves */
static const char *optimacros =
  "/D{bind def}def\n"
  "/R{roll}D\n"
  "/K{copy}D\n"
  "/P{pop}D\n"
  "/p{3 2 R add 3 1 R add exch}D\n"
  "/t{dup 4 3 R mul 3 1 R mul}D\n"
  "/a{dup 1 sub neg 4 1 R t 5 2 R t p}D\n"
  "/m{2 K le{exch}if P}D\n"
  "/n{abs exch abs m}D\n"
  "/d{-1 t p n}D\n"
  "/s{[4 2 R] cvx def}D\n"
  "/g{7 K P 4 K P P d 5 1 R d 10 m m div 5 K 12 8 R 5 4 R a 9 4 R 3 2 R a 6 4 R curveto}D\n"
  "/e{4 2 R lineto lineto P P}D\n"
  "/q{3 K P n 10 m div}D\n"
  "/f{x y 7 4 R 5 1 R 4 K p /y s 7 2 R 2 K 9 7 R 7 6 R t p 2 K /x s}D\n"
  "/C{4 1 R q f 7 6 R g}D\n"
  "/V{q f e}D\n"
  "/c{3 1 R .5 f 7 6 R g}D\n"
  "/v{.5 f e}D\n"
  "/j{5 K P p /y s 3 K t 7 5 R p /x s x moveto P}D\n"
  "/i{.5 j}D\n"
  "/I{dup 6 1 R q j 3 2 R}D\n"
  "/z{closepath}D\n"
  "/b{%s z fill}D\n"
  "/w{%s z fill}D\n";

/* special macros for debug output */
static const char *debugmacros =
  "/unit { %f } def\n"
  "/box { newpath 0 0 moveto 0 1 lineto 1 1 lineto 1 0 lineto closepath } def\n"
  "/circ { newpath 0 0 1 0 360 arc closepath } def\n"
  "/dot { gsave .15 mul dup scale circ fill grestore } def\n"
  "/sq { gsave unit unit scale -.5 -.5 translate box .02 setlinewidth stroke grestore } def\n"
  "/sq1 { gsave translate sq unit .6 mul dot grestore } def\n"
  "/dot2 { gsave translate unit dot grestore } def\n"
  "/usq { gsave unit unit scale -.5 -.5 rmoveto 0 1 rlineto 1 0 rlineto 0 -1 rlineto closepath .02 setlinewidth stroke grestore } def\n"
  "/dot1 { gsave translate unit .3 mul dup scale circ fill grestore } def\n"
  "/times { /Times-Roman findfont unit .3 mul scalefont setfont } def\n"
  "/times1 { /Times-Roman findfont unit 10 mul scalefont setfont 0 0 0 setrgbcolor } def\n"
  "/times2 { /Times-Roman findfont unit 2 mul scalefont setfont 0 0 0 setrgbcolor } def\n";

/* ---------------------------------------------------------------------- */
/* Backends for various types of output. */

/* Normal output: black on transparent */
static int render0(potrace_path_t *plist) {
  potrace_path_t *p;

  if (info.longcoding) {
    eps_setcolor(info.color);
    list_forall (p, plist) {
      TRY(eps_path(p->priv->fcurve));
      ship("closepath\n");
      if (p->next == NULL || p->next->sign == '+') {
	ship("fill\n");
      }
    }
  } else {
    list_forall (p, plist) {
      TRY(eps_path(p->priv->fcurve));
      if (p->next == NULL || p->next->sign == '+') {
	ship("b\n");
      } else {
	ship("z\n");
      }
    }
  }
  return 0;

 try_error:
  return 1;
}

/* Opaque output: alternating black and white */
static int render0_opaque(potrace_path_t *plist) {
  potrace_path_t *p;
  
  if (info.longcoding) {
    list_forall (p, plist) {
      TRY(eps_path(p->priv->fcurve));
      ship("closepath\n");
      eps_setcolor(p->sign=='+' ? info.color : info.fillcolor);
      ship("fill\n");
    }
  } else {
    list_forall (p, plist) {
      TRY(eps_path(p->priv->fcurve));
      ship(p->sign=='+' ? "b\n" : "w\n");
    }
  }
  return 0;

 try_error:
  return 1;
}

/* Debug output type 1 (show optimal polygon) */
static int render1(potrace_path_t *plist) {
  potrace_path_t *p;
  int i;

  eps_jaggy(plist);

  list_forall (p, plist) {

    point_t *pt = p->priv->pt;
    int n = p->priv->len;
    int m = p->priv->m;
    int *po = p->priv->po;

    eps_linewidth(.02);
    eps_setcolor(black);
    /* output jaggie curve in boxed style */
    for (i=1; i<n; i++) {
      eps_moveto(dpoint(pt[i-1]));
      eps_lineto(dpoint(pt[i]));
      ship("stroke\n");
      eps_coords(dpoint(pt[i]));
      ship("sq1\n");
    }
    eps_moveto(dpoint(pt[n-1]));
    eps_lineto(dpoint(pt[0]));
    ship("stroke\n");
    eps_coords(dpoint(pt[0]));
    ship("sq1\n");

    /* output the uncorrected polygon */
    eps_linewidth(.1);
    eps_setcolor(blue);
    eps_moveto(dpoint(pt[po[0]]));
    for (i=1; i<m; i++) {
      eps_lineto(dpoint(pt[po[i]]));
    }
    eps_lineto(dpoint(pt[po[0]]));
    ship("stroke\n");
    for (i=0; i<m; i++) {
      eps_coords(dpoint(pt[po[i]]));
      ship("dot2\n");
    }
  }
  return 0;
}

/* Debug output type 2 (show corrected polygon and edge detection) */
static int render2(potrace_path_t *plist) {
  potrace_path_t *p;
  int i;

  /* output original bitmap in grey */
  eps_jaggy(plist);
  
  list_forall (p, plist) {
    /* output polygon with corrected edges, lines L, and parameter alpha */
    eps_polygon(&p->priv->curve, black);
    eps_L(&p->priv->curve, black);
    
    /* output the vertex unit squares */
    for (i=0; i<p->priv->curve.n; i++) {
      eps_moveto(p->priv->curve.vertex[i]);
      ship("usq\n");
    }

    /* output the path */
    eps_linewidth(.1);
    eps_setcolor(blue);
    TRY(eps_path(&p->priv->curve));
    ship("closepath\n");
    ship("stroke\n");

    if (info.param->opticurve && info.debug == 3) {

      /* output opticurve */
      eps_linewidth(.05);
      eps_setcolor(red);
      TRY(eps_path(&p->priv->ocurve));
      ship("closepath\n");
      ship("stroke\n");
      
      /* output dots */
      for (i=0; i<p->priv->ocurve.n; i++) {
	eps_coords(p->priv->ocurve.c[i][2]);
	ship("dot1\n");
      }
    }
  }
  return 0;

 try_error:
  return 1;
}

/* Free-style debug output */
static int render_debug(potrace_path_t *plist) {
  potrace_path_t *p;
  int count;
  int i;

  /* output original bitmap in grey */
  eps_jaggy(plist);

  count = -1;
  list_forall (p, plist) {
    count++;

    /* output path numbers */
    eps_moveto_offs(p->priv->curve.vertex[0], 0, 5);
    ship("times1 (%d) show\n", count);

    /* output polygon with corrected edges, lines L, and parameter alpha */
    eps_polygon(&p->priv->curve, black);
    eps_L(&p->priv->curve, black);

    /* output the vertex unit squares */
    for (i=0; i<p->priv->curve.n; i++) {
      eps_moveto(p->priv->curve.vertex[i]);
      ship("usq\n");
    }

    /* output the vertex numbers */
    for (i=0; i<p->priv->curve.n; i++) {
      eps_moveto_offs(p->priv->curve.vertex[i], +1, +1);
      ship("times2 (%d) show\n", i);
    }
    
    /* output the path */
    eps_linewidth(.1);
    eps_setcolor(blue);
    TRY(eps_path(&p->priv->curve));
    ship("closepath\n");
    ship("stroke\n");
    
    if (info.param->opticurve) {

      /* output the opti-verteces polygon */
      eps_polygon(&p->priv->ocurve, green);
      
      /* output opticurve */
      eps_linewidth(.05);
      eps_setcolor(red);
      TRY(eps_path(&p->priv->ocurve));
      ship("closepath\n");
      ship("stroke\n");
      
      /* output dots */
      for (i=0; i<p->priv->ocurve.n; i++) {
	eps_coords(p->priv->ocurve.c[i][2]);
	ship("dot1\n");
      }

      /* output beta parameters */
      for (i=0; i<p->priv->ocurve.n; i++) {
	eps_moveto_offs(p->priv->ocurve.c[i][2], +.4, -.4);
	ship("times (%.2f) show\n", p->priv->ocurve.beta[i]);
      }
    }
  }
  return 0;

 try_error:
  return 1;
}

/* select the appropriate rendering function from above */
static int eps_render(potrace_path_t *plist) {
  int r;
  
  switch (info.debug) {
  case 0:
    if (info.opaque) {
      r = render0_opaque(plist);
    } else {
      r = render0(plist);
    }
    break;
  case 1:
    r = render1(plist);
    break;
  case 2: case 3:
    r = render2(plist);
    break;
  default:
    r = render_debug(plist);
    break;
  }
  return r;
}  

/* ---------------------------------------------------------------------- */
/* EPS header and footer */

static int eps_init(imginfo_t *imginfo) {
  double origx = imginfo->trans.orig[0] + imginfo->lmar;
  double origy = imginfo->trans.orig[1] + imginfo->bmar;
  double scalex = imginfo->trans.scalex / info.unit;
  double scaley = imginfo->trans.scaley / info.unit;
  char *c0, *c1;

  shipcom("%%!PS-Adobe-3.0 EPSF-3.0\n");
  shipcom("%%%%Creator: " POTRACE " " VERSION ", written by Peter Selinger 2001-2017\n");
  shipcom("%%%%LanguageLevel: %d\n", info.pslevel);
  shipcom("%%%%BoundingBox: 0 0 %.0f %.0f\n", 
	  ceil(imginfo->trans.bb[0]+imginfo->lmar+imginfo->rmar),
	  ceil(imginfo->trans.bb[1]+imginfo->tmar+imginfo->bmar));
  shipcom("%%%%HiResBoundingBox: 0 0 %f %f\n", 
	  imginfo->trans.bb[0]+imginfo->lmar+imginfo->rmar,
	  imginfo->trans.bb[1]+imginfo->tmar+imginfo->bmar);
  shipcom("%%%%Pages: 1\n");
  shipcom("%%%%EndComments\n");
  
  shipcom("%%%%Page: 1 1\n");
  ship("save\n");
  if (!info.longcoding) {
    c0 = strdup(eps_colorstring(info.color));
    c1 = strdup(eps_colorstring(info.fillcolor));
    if (!c0 || !c1) {
      free(c0);
      free(c1);
      return 1;
    }
    ship(optimacros, c0, c1);
    free(c0);
    free(c1);
  }
  if (info.debug) {
    ship(debugmacros, info.unit);
  }
  if (origx != 0 || origy != 0) {
    ship("%f %f translate\n", origx, origy);
  }
  if (info.angle != 0) {
    ship("%.2f rotate\n", info.angle);
  }
  ship("%f %f scale\n", scalex, scaley);

  return 0;
}

static void eps_term(void) {
  ship("restore\n");
  shipcom("%%%%EOF\n");
}

/* public interface for EPS */
int page_eps(FILE *fout, potrace_path_t *plist, imginfo_t *imginfo) {
  eps_callbacks(fout);

  TRY(eps_init(imginfo));
  TRY(eps_render(plist));
  eps_term();
  return 0;

 try_error:
  return 1;
}

/* ---------------------------------------------------------------------- */
/* PostScript header and footer */

static int eps_pagenumber;

int init_ps(FILE *fout) {
  char *c0, *c1;

  /* set callback functions for shipping routines */
  eps_callbacks(fout);

  shipcom("%%!PS-Adobe-3.0\n");
  shipcom("%%%%Creator: " POTRACE " " VERSION ", written by Peter Selinger 2001-2017\n");
  shipcom("%%%%LanguageLevel: %d\n", info.pslevel);
  shipcom("%%%%BoundingBox: 0 0 %d %d\n", info.paperwidth, info.paperheight);
  shipcom("%%%%Pages: (atend)\n");
  shipcom("%%%%EndComments\n");
  if (!info.longcoding || info.debug) {
    shipcom("%%%%BeginSetup\n");
    if (!info.longcoding) {
      c0 = strdup(eps_colorstring(info.color));
      c1 = strdup(eps_colorstring(info.fillcolor));
      if (!c0 || !c1) {
	free(c0);
	free(c1);
	return 1;
      }
      ship(optimacros, c0, c1);
      free(c0);
      free(c1);
    }
    if (info.debug) {
      ship(debugmacros, info.unit);
    }
    shipcom("%%%%EndSetup\n");
  }
  eps_pagenumber = 0;
  fflush(fout);
  return 0;
}

int term_ps(FILE *fout) {
  eps_callbacks(fout);

  shipcom("%%%%Trailer\n");
  shipcom("%%%%Pages: %d\n", eps_pagenumber);
  shipcom("%%%%EOF\n");
  fflush(fout);

  return 0;
}

static void eps_pageinit_ps(imginfo_t *imginfo) {
  double origx = imginfo->trans.orig[0] + imginfo->lmar;
  double origy = imginfo->trans.orig[1] + imginfo->bmar;
  double scalex = imginfo->trans.scalex / info.unit;
  double scaley = imginfo->trans.scaley / info.unit;

  eps_pagenumber++;
  eps_color = -1;
  eps_width = -1;

  shipcom("%%%%Page: %d %d\n", eps_pagenumber, eps_pagenumber);
  ship("save\n");
  if (origx != 0 || origy != 0) {
    ship("%f %f translate\n", origx, origy);
  }
  if (info.angle != 0) {
    ship("%.2f rotate\n", info.angle);
  }
  ship("%f %f scale\n", scalex, scaley);
}

static void eps_pageterm_ps(void) {
  ship("restore\n");
  ship("showpage\n");
}

int page_ps(FILE *fout, potrace_path_t *plist, imginfo_t *imginfo) {
  int r;

  eps_callbacks(fout);

  eps_pageinit_ps(imginfo);

  r = eps_render(plist);
  if (r) {
    return r;
  }

  eps_pageterm_ps();

  shipcom("");

  fflush(fout);

  return 0;
}
