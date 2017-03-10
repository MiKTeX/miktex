/* Copyright (C) 2001-2017 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

/* mkbitmap.c: a standalone program for converting greymaps to bitmaps
   while optionally applying the following enhancements: highpass
   filter (evening out background gradients), lowpass filter
   (smoothing foreground details), interpolated scaling, inversion. */

#define _XOPEN_SOURCE 500

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include "greymap.h"
#include "bitmap_io.h"
#include "platform.h"

#define SAFE_CALLOC(var, n, typ) \
  if ((var = (typ *)calloc(n, sizeof(typ))) == NULL) goto calloc_error 

/* structure to hold command line options */
struct info_s {
  char *outfile;      /* output file */
  char **infiles;     /* input files */
  int infilecount;    /* how many input files? */
  int invert;         /* invert input? */
  int highpass;       /* use highpass filter? */
  double lambda;      /* highpass filter radius */
  int lowpass;        /* use lowpass filter? */
  double lambda1;     /* lowpass filter radius */
  int scale;          /* scaling factor */
  int linear;         /* linear scaling? */
  int bilevel;        /* convert to bilevel? */
  double level;       /* cutoff grey level */
  const char *outext; /* default output file extension */
};
typedef struct info_s info_t;

static info_t info;

/* apply lowpass filter (an approximate Gaussian blur) to greymap.
   Lambda is the standard deviation of the kernel of the filter (i.e.,
   the approximate filter radius). */
static void lowpass(greymap_t *gm, double lambda) {
  double f, g;
  double c, d;
  double B;
  int x, y;

  if (gm->h == 0 || gm->w == 0) {
    return;
  }
  
  /* calculate filter coefficients from given lambda */
  B = 1+2/(lambda*lambda);
  c = B-sqrt(B*B-1);
  d = 1-c;

  for (y=0; y<gm->h; y++) {
    /* apply low-pass filter to row y */
    /* left-to-right */
    f = g = 0;
    for (x=0; x<gm->w; x++) {
      f = f*c + GM_UGET(gm, x, y)*d;
      g = g*c + f*d;
      GM_UPUT(gm, x, y, g);
    }

    /* right-to-left */
    for (x=gm->w-1; x>=0; x--) {
      f = f*c + GM_UGET(gm, x, y)*d;
      g = g*c + f*d;
      GM_UPUT(gm, x, y, g);
    }

    /* left-to-right mop-up */
    for (x=0; x<gm->w; x++) {
      f = f*c;
      g = g*c + f*d;
      if (f+g < 1/255.0) {
	break;
      }
      GM_UPUT(gm, x, y, GM_UGET(gm, x, y)+g);
    }
  }

  for (x=0; x<gm->w; x++) {
    /* apply low-pass filter to column x */
    /* bottom-to-top */
    f = g = 0;
    for (y=0; y<gm->h; y++) {
      f = f*c + GM_UGET(gm, x, y)*d;
      g = g*c + f*d;
      GM_UPUT(gm, x, y, g);
    }

    /* top-to-bottom */
    for (y=gm->h-1; y>=0; y--) {
      f = f*c + GM_UGET(gm, x, y)*d;
      g = g*c + f*d;
      GM_UPUT(gm, x, y, g);
    }

    /* bottom-to-top mop-up */
    for (y=0; y<gm->h; y++) {
      f = f*c;
      g = g*c + f*d;
      if (f+g < 1/255.0) {
	break;
      }
      GM_UPUT(gm, x, y, GM_UGET(gm, x, y)+g);
    }
  }
}

/* apply highpass filter to greymap. Return 0 on success, 1 on error
   with errno set. */
static int highpass(greymap_t *gm, double lambda) {
  greymap_t *gm1;
  double f;
  int x, y;

  if (gm->h == 0 || gm->w == 0) {
    return 0;
  }

  /* create a copy */
  gm1 = gm_dup(gm);
  if (!gm1) {
    return 1;
  }

  /* apply lowpass filter to the copy */
  lowpass(gm1, lambda);

  /* subtract copy from original */
  for (y=0; y<gm->h; y++) {
    for (x=0; x<gm->w; x++) {
      f = GM_UGET(gm, x, y);
      f -= GM_UGET(gm1, x, y);
      f += 128;    /* normalize! */
      GM_UPUT(gm, x, y, f);
    }
  }
  gm_free(gm1);
  return 0;
}

/* Convert greymap to bitmap by using cutoff threshold c (0=black,
   1=white). On error, return NULL with errno set. */
static potrace_bitmap_t *threshold(greymap_t *gm, double c) {
  int w, h;
  potrace_bitmap_t *bm_out = NULL;
  double c1;
  int x, y;
  double p;

  w = gm->w;
  h = gm->h;

  /* allocate output bitmap */
  bm_out = bm_new(w, h);
  if (!bm_out) {
    return NULL;
  }

  /* thresholding */
  c1 = c * 255;

  for (y=0; y<h; y++) {
    for (x=0; x<w; x++) {
      p = GM_UGET(gm, x, y);
      BM_UPUT(bm_out, x, y, p < c1);
    }
  }
  return bm_out;
}

/* scale greymap by factor s, using linear interpolation. If
   bilevel=0, return a pointer to a greymap_t. If bilevel=1, return a
   pointer to a potrace_bitmap_t and use cutoff threshold c (0=black,
   1=white).  On error, return NULL with errno set. */

static void *interpolate_linear(greymap_t *gm, int s, int bilevel, double c) {
  int p00, p01, p10, p11;
  int i, j, x, y;
  double xx, yy, av;
  double c1 = 0;
  int w, h;
  double p0, p1;
  greymap_t *gm_out = NULL;
  potrace_bitmap_t *bm_out = NULL;

  w = gm->w;
  h = gm->h;

  /* allocate output bitmap/greymap */
  if (bilevel) {
    bm_out = bm_new(w*s, h*s);
    if (!bm_out) {
      return NULL;
    }
    c1 = c * 255;
  } else {
    gm_out = gm_new(w*s, h*s);
    if (!gm_out) {
      return NULL;
    }
  }

  /* interpolate */
  for (i=0; i<w; i++) {
    for (j=0; j<h; j++) {
      p00 = GM_BGET(gm, i, j);
      p01 = GM_BGET(gm, i, j+1);
      p10 = GM_BGET(gm, i+1, j);
      p11 = GM_BGET(gm, i+1, j+1);
      
      if (bilevel) {
	/* treat two special cases which are very common */
	if (p00 < c1 && p01 < c1 && p10 < c1 && p11 < c1) {
	  for (x=0; x<s; x++) {
	    for (y=0; y<s; y++) {
	      BM_UPUT(bm_out, i*s+x, j*s+y, 1);
	    }
	  }
	  continue;
	}
	if (p00 >= c1 && p01 >= c1 && p10 >= c1 && p11 >= c1) {
	  continue;
	}
      }
      
      /* the general case */
      for (x=0; x<s; x++) {
	xx = x/(double)s;
	p0 = p00*(1-xx) + p10*xx;
	p1 = p01*(1-xx) + p11*xx;
	for (y=0; y<s; y++) {
	  yy = y/(double)s;
	  av = p0*(1-yy) + p1*yy;
	  if (bilevel) {
	    BM_UPUT(bm_out, i*s+x, j*s+y, av < c1);
	  } else {
	    GM_UPUT(gm_out, i*s+x, j*s+y, av);
	  }
	}
      }
    }
  }
  if (bilevel) {
    return (void *)bm_out;
  } else {
    return (void *)gm_out;
  }
}

/* same as interpolate_linear, except use cubic interpolation (slower
   and better). */

/* we need this typedef so that the SAFE_CALLOC macro will work */
typedef double double4[4];

static void *interpolate_cubic(greymap_t *gm, int s, int bilevel, double c) {
  int w, h;
  double4 *poly = NULL; /* poly[s][4]: fixed interpolation polynomials */
  double p[4];              /* four current points */
  double4 *window = NULL; /* window[s][4]: current state */
  double t, v;
  int k, l, i, j, x, y;
  double c1 = 0;
  greymap_t *gm_out = NULL;
  potrace_bitmap_t *bm_out = NULL;

  SAFE_CALLOC(poly, s, double4);
  SAFE_CALLOC(window, s, double4);

  w = gm->w;
  h = gm->h;

  /* allocate output bitmap/greymap */
  if (bilevel) {
    bm_out = bm_new(w*s, h*s);
    if (!bm_out) {
      goto calloc_error;
    }
    c1 = c * 255;
  } else {
    gm_out = gm_new(w*s, h*s);
    if (!gm_out) {
      goto calloc_error;
    }
  }

  /* pre-calculate interpolation polynomials */
  for (k=0; k<s; k++) {
    t = k/(double)s;
    poly[k][0] = 0.5 * t * (t-1) * (1-t);
    poly[k][1] = -(t+1) * (t-1) * (1-t) + 0.5 * (t-1) * (t-2) * t;
    poly[k][2] = 0.5 * (t+1) * t * (1-t) - t * (t-2) * t;
    poly[k][3] = 0.5 * t * (t-1) * t;
  }

  /* interpolate */
  for (y=0; y<h; y++) {
    x=0;
    for (i=0; i<4; i++) {
      for (j=0; j<4; j++) {
	p[j] = GM_BGET(gm, x+i-1, y+j-1);
      }
      for (k=0; k<s; k++) {
	window[k][i] = 0.0;
	for (j=0; j<4; j++) {
	  window[k][i] += poly[k][j] * p[j];
	}
      }
    }
    while (1) {
      for (l=0; l<s; l++) {
	for (k=0; k<s; k++) {
	  v = 0.0;
	  for (i=0; i<4; i++) {
	    v += window[k][i] * poly[l][i];
	  }
	  if (bilevel) {
	    BM_UPUT(bm_out, x*s+l, y*s+k, v < c1);
	  } else {
	    GM_UPUT(gm_out, x*s+l, y*s+k, v);
	  }	    
	}
      }
      x++;
      if (x>=w) {
	break;
      }
      for (i=0; i<3; i++) {
	for (k=0; k<s; k++) {
	  window[k][i] = window[k][i+1];
	}
      }
      i=3;
      for (j=0; j<4; j++) {
        p[j] = GM_BGET(gm, x+i-1, y+j-1);
      }
      for (k=0; k<s; k++) {
	window[k][i] = 0.0;
        for (j=0; j<4; j++) {
          window[k][i] += poly[k][j] * p[j];
        }
      }
    }
  }

  free(poly);
  free(window);

  if (bilevel) {
    return (void *)bm_out;
  } else {
    return (void *)gm_out;
  }

 calloc_error:
  free(poly);
  free(window);
  return NULL;
}

/* ---------------------------------------------------------------------- */

/* process a single file, containing one or more images. On error,
   print error message to stderr and exit with code 2. On warning,
   print warning message to stderr. */

static void process_file(FILE *fin, FILE *fout, const char *infile, const char *outfile) {
  int r;
  greymap_t *gm;
  potrace_bitmap_t *bm;
  void *sm;
  int x, y;
  int count;

  for (count=0; ; count++) {
    r = gm_read(fin, &gm);
    switch (r) {
    case -1:  /* system error */
      fprintf(stderr, "" MKBITMAP ": %s: %s\n", infile, strerror(errno));
      exit(2);
    case -2:  /* corrupt file format */
      fprintf(stderr, "" MKBITMAP ": %s: file format error: %s\n", infile, gm_read_error);
      exit(2);
    case -3:  /* empty file */
      if (count>0) {  /* end of file */
	return;
      }
      fprintf(stderr, "" MKBITMAP ": %s: empty file\n", infile);
      exit(2);
    case -4:  /* wrong magic */
      if (count>0) {
	fprintf(stderr, "" MKBITMAP ": %s: warning: junk at end of file\n", infile);
	return;
      }
      fprintf(stderr, "" MKBITMAP ": %s: file format not recognized\n", infile);
      fprintf(stderr, "Possible input file formats are: pnm (pbm, pgm, ppm), bmp.\n");
      exit(2);
    case 1:  /* unexpected end of file */
      fprintf(stderr, "" MKBITMAP ": %s: warning: premature end of file\n", infile);
      break;
    }
    
    if (info.invert) {
      for (y=0; y<gm->h; y++) {
	for (x=0; x<gm->w; x++) {
	  GM_UPUT(gm, x, y, 255-GM_UGET(gm, x, y));
	}
      }
    }
    
    if (info.highpass) {
      r = highpass(gm, info.lambda);
      if (r) {
	fprintf(stderr, "" MKBITMAP ": %s: %s\n", infile, strerror(errno));
	exit(2);
      }
    }

    if (info.lowpass) {
      lowpass(gm, info.lambda1);
    }
    
    if (info.scale == 1 && info.bilevel) {  /* no interpolation necessary */
      sm = threshold(gm, info.level);
      gm_free(gm);
    } else if (info.scale == 1) {
      sm = gm;
    } else if (info.linear) {  /* linear interpolation */
      sm = interpolate_linear(gm, info.scale, info.bilevel, info.level);
      gm_free(gm);
    } else {  /* cubic interpolation */
      sm = interpolate_cubic(gm, info.scale, info.bilevel, info.level);
      gm_free(gm);
    }
    if (!sm) {
      fprintf(stderr, "" MKBITMAP ": %s: %s\n", infile, strerror(errno));
      exit(2);
    }
    
    if (info.bilevel) {
      bm = (potrace_bitmap_t *)sm;
      bm_writepbm(fout, bm);
      bm_free(bm);
    } else {
      gm = (greymap_t *)sm;
      gm_writepgm(fout, gm, NULL, 1, GM_MODE_POSITIVE, 1.0);
      gm_free(gm);
    }
  }
}

/* ---------------------------------------------------------------------- */
/* some info functions and option processing */

static int license(FILE *f) {
  fprintf(f, 
  "This program is free software; you can redistribute it and/or modify\n"
  "it under the terms of the GNU General Public License as published by\n"
  "the Free Software Foundation; either version 2 of the License, or\n"
  "(at your option) any later version.\n"
  "\n"
  "This program is distributed in the hope that it will be useful,\n"
  "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
  "GNU General Public License for more details.\n"
  "\n"
  "You should have received a copy of the GNU General Public License\n"
  "along with this program; if not, write to the Free Software Foundation\n"
  "Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n"
	  );
  return 0;
}

static int usage(FILE *f) {
  fprintf(f, "Usage: " MKBITMAP " [options] [file...]\n");
  fprintf(f, "Options:\n");
  fprintf(f, " -h, --help           - print this help message and exit\n");
  fprintf(f, " -v, --version        - print version info and exit\n");
  fprintf(f, " -l, --license        - print license info and exit\n");
  fprintf(f, " -o, --output <file>  - output to file\n");
  fprintf(f, " -x, --nodefaults     - turn off default options\n");
  fprintf(f, "Inversion:\n");
  fprintf(f, " -i, --invert         - invert the input (undo 'blackboard' effect)\n");
  fprintf(f, "Highpass filtering:\n");
  fprintf(f, " -f, --filter <n>     - apply highpass filter with radius n (default 4)\n");
  fprintf(f, " -n, --nofilter       - no highpass filtering\n");
  fprintf(f, " -b, --blur <n>       - apply lowpass filter with radius n (default: none)\n");
  fprintf(f, "Scaling:\n");
  fprintf(f, " -s, --scale <n>      - scale by integer factor n (default 2)\n");
  fprintf(f, " -1, --linear         - use linear interpolation\n");
  fprintf(f, " -3, --cubic          - use cubic interpolation (default)\n");
  fprintf(f, "Thresholding:\n");
  fprintf(f, " -t, --threshold <n>  - set threshold for bilevel conversion (default 0.45)\n");
  fprintf(f, " -g, --grey           - no bilevel conversion, output a greymap\n");

  fprintf(f, "\n");
  fprintf(f, "Possible input file formats are: pnm (pbm, pgm, ppm), bmp.\n");
  fprintf(f, "The default options are: -f 4 -s 2 -3 -t 0.45\n");
  
  return 0;
}

static struct option longopts[] = {
  {"help",          0, 0, 'h'},
  {"version",       0, 0, 'v'},
  {"license",       0, 0, 'l'},
  {"output",        1, 0, 'o'},
  {"reset",         0, 0, 'x'},
  {"invert",        0, 0, 'i'},
  {"filter",        1, 0, 'f'},
  {"nofilter",      0, 0, 'n'},
  {"blur",          1, 0, 'b'},
  {"scale",         1, 0, 's'},
  {"linear",        0, 0, '1'},
  {"cubic",         0, 0, '3'},
  {"grey",          0, 0, 'g'},
  {"threshold",     1, 0, 't'},
  {0, 0, 0, 0}
};

static const char *shortopts = "hvlo:xif:nb:s:13gt:";

/* process options. On error, print error message to stderr and exit
   with code 1 */
static void dopts(int ac, char *av[]) {
  int c;
  char *p;
  
  /* set defaults for command line parameters */
  info.outfile = NULL;    /* output file */
  info.infiles = NULL;    /* input files */
  info.infilecount = 0;   /* how many input files? */
  info.invert = 0;        /* invert input? */
  info.highpass = 1;      /* use highpass filter? */
  info.lambda = 4;        /* highpass filter radius */
  info.lowpass = 0;       /* use lowpass filter? */
  info.lambda1 = 0;       /* lowpass filter radius */
  info.scale = 2;         /* scaling factor */
  info.linear = 0;        /* linear scaling? */
  info.bilevel = 1;       /* convert to bilevel? */
  info.level = 0.45;      /* cutoff grey level */
  info.outext = ".pbm";   /* output file extension */

  while ((c = getopt_long(ac, av, shortopts, longopts, NULL)) != -1) {
    switch (c) {
    case 'h':
      fprintf(stdout, "" MKBITMAP " " VERSION ". Transforms images into bitmaps with scaling and filtering.\n\n");
      usage(stdout);
      exit(0);
      break;
    case 'v':
      fprintf(stdout, "" MKBITMAP " " VERSION ". Copyright (C) 2001-2017 Peter Selinger.\n");
      exit(0);
      break;
    case 'l':
      fprintf(stdout, "" MKBITMAP " " VERSION ". Copyright (C) 2001-2017 Peter Selinger.\n\n");
      license(stdout);
      exit(0);
      break;
    case 'o':
      free(info.outfile);
      info.outfile = strdup(optarg);
      if (!info.outfile) {
        fprintf(stderr, "" MKBITMAP ": %s\n", strerror(errno));
        exit(2);
      }
      break;
    case 'x':
      info.invert = 0;
      info.highpass = 0;
      info.scale = 1;
      info.bilevel = 0;
      info.outext = ".pgm";
      break;
    case 'i':
      info.invert = 1;
      break;
    case 'f':
      info.highpass = 1;
      info.lambda = strtod(optarg, &p);
      if (*p || info.lambda<0) {
	fprintf(stderr, "" MKBITMAP ": invalid filter radius -- %s\n", optarg);
        exit(1);
      }
      break;
    case 'n':
      info.highpass = 0;
      break;
    case 'b':
      info.lowpass = 1;
      info.lambda1 = strtod(optarg, &p);
      if (*p || info.lambda1<0) {
	fprintf(stderr, "" MKBITMAP ": invalid filter radius -- %s\n", optarg);
        exit(1);
      }
      break;
    case 's':
      info.scale = strtol(optarg, &p, 0);
      if (*p || info.scale<=0) {
	fprintf(stderr, "" MKBITMAP ": invalid scaling factor -- %s\n", optarg);
        exit(1);
      }
      break;
    case '1':
      info.linear = 1;
      break;
    case '3':
      info.linear = 0;
      break;
    case 'g':
      info.bilevel = 0;
      info.outext = ".pgm";
      break;
    case 't':
      info.bilevel = 1;
      info.outext = ".pbm";
      info.level = strtod(optarg, &p);
      if (*p || info.level<0) {
	fprintf(stderr, "" MKBITMAP ": invalid threshold -- %s\n", optarg);
        exit(1);
      }
      break;
    case '?':
      fprintf(stderr, "Try --help for more info\n");
      exit(1);
      break;
    default:
      fprintf(stderr, "" MKBITMAP ": Unimplemented option -- %c\n", c);
      exit(1);
    }
  }
  info.infiles = &av[optind];
  info.infilecount = ac-optind;
  return;
}

/* ---------------------------------------------------------------------- */
/* auxiliary functions for file handling */

/* open a file for reading. Return stdin if filename is NULL or "-" */ 
static FILE *my_fopen_read(const char *filename) {
  if (filename == NULL || strcmp(filename, "-") == 0) {
    return stdin;
  }
  return fopen(filename, "rb");
}

/* open a file for writing. Return stdout if filename is NULL or "-" */ 
static FILE *my_fopen_write(const char *filename) {
  if (filename == NULL || strcmp(filename, "-") == 0) {
    return stdout;
  }
  return fopen(filename, "wb");
}

/* close a file, but do nothing is filename is NULL or "-" */
static void my_fclose(FILE *f, const char *filename) {
  if (filename == NULL || strcmp(filename, "-") == 0) {
    return;
  }
  fclose(f);
}

/* make output filename from input filename. Return an allocated value. */
static char *make_outfilename(const char *infile, const char *ext) {
  char *outfile;
  char *p;

  if (strcmp(infile, "-") == 0) {
    return strdup("-");
  }

  outfile = (char *) malloc(strlen(infile)+strlen(ext)+5);
  if (!outfile) {
    return NULL;
  }
  strcpy(outfile, infile);
  p = strrchr(outfile, '.');
  if (p) {
    *p = 0;
  }
  strcat(outfile, ext);

  /* check that input and output filenames are different */
  if (strcmp(infile, outfile) == 0) {
    strcpy(outfile, infile);
    strcat(outfile, "-out");
  }

  return outfile;
}

/* ---------------------------------------------------------------------- */
/* Main function */

int main(int ac, char *av[]) {
  FILE *fin, *fout;
  int i;
  char *outfile;

  /* platform-specific initializations, e.g., set file i/o to binary */
  platform_init();

  /* process options */
  dopts(ac, av);

  /* there are several ways to call us:
     mkbitmap                    -- stdin to stdout
     mkbitmap -o outfile         -- stdin to outfile
     mkbitmap file...            -- encode each file and generate outfile names
     mkbitmap file... -o outfile -- concatenate files and write to outfile
  */

  if (info.infilecount == 0 && info.outfile == NULL) {  /* stdin to stdout */

    process_file(stdin, stdout, "stdin", "stdout");
    return 0;

  } else if (info.infilecount == 0) {                  /* stdin to outfile */

    fout = my_fopen_write(info.outfile);
    if (!fout) {
      fprintf(stderr, "" MKBITMAP ": %s: %s\n", info.outfile, strerror(errno));
      exit(2);
    }
    process_file(stdin, fout, "stdin", info.outfile);
    my_fclose(fout, info.outfile);
    free(info.outfile);
    return 0;

  } else if (info.outfile == NULL) {       /* infiles -> multiple outfiles */

    for (i=0; i<info.infilecount; i++) {
      outfile = make_outfilename(info.infiles[i], info.outext);
      if (!outfile) {
	fprintf(stderr, "" MKBITMAP ": %s\n", strerror(errno));
        exit(2);
      }
      fin = my_fopen_read(info.infiles[i]);
      if (!fin) {
	fprintf(stderr, "" MKBITMAP ": %s: %s\n", info.infiles[i], strerror(errno));
	exit(2);
      }
      fout = my_fopen_write(outfile);
      if (!fout) {
	fprintf(stderr, "" MKBITMAP ": %s: %s\n", outfile, strerror(errno));
	exit(2);
      }
      process_file(fin, fout, info.infiles[i], outfile);
      my_fclose(fin, info.infiles[i]);
      my_fclose(fout, outfile);
      free(outfile);
    }
    return 0;

  } else {                                    /* infiles to single outfile */

    fout = my_fopen_write(info.outfile);
    if (!fout) {
      fprintf(stderr, "" MKBITMAP ": %s: %s\n", info.outfile, strerror(errno));
      exit(2);
    }
    for (i=0; i<info.infilecount; i++) {
      fin = my_fopen_read(info.infiles[i]);
      if (!fin) {
	fprintf(stderr, "" MKBITMAP ": %s: %s\n", info.infiles[i], strerror(errno));
	exit(2);
      }
      process_file(fin, fout, info.infiles[i], info.outfile);
      my_fclose(fin, info.infiles[i]);
    }
    my_fclose(fout, info.outfile);
    free(info.outfile);
    return 0;

  }      

  /* not reached */
}
