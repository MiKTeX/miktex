/* Copyright (C) 2001-2015 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */


/* the PostScript compression module of Potrace. The basic interface
   is through the *_xship function, which processes a byte array and
   outputs it in compressed or verbatim form, depending on whether
   filter is 1 or 0. To flush the output, simply call with the empty
   string and filter=0. filter=2 is used to output encoded text but
   without the PostScript header to turn on the encoding. Each
   function has variants for shipping a single character, a
   null-terminated string, or a byte array. */

/* different compression algorithms are available. There is
   dummy_xship, which is just the identity, and flate_xship, which
   uses zlib compression. Also, lzw_xship provides LZW compression
   from the file lzw.c/h. a85_xship provides a85-encoding without
   compression. Each function returns the actual number of characters
   written. */

/* note: the functions provided here have global state and are not
   reentrant */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#include "flate.h"
#include "lzw.h"

#define OUTSIZE 1000

static int a85init(FILE *f);
static int a85finish(FILE *f);
static int a85write(FILE *f, char *buf, int n);
static int a85out(FILE *f, int n);
static int a85spool(FILE *f, char c);

/* ---------------------------------------------------------------------- */
/* dummy interface: no encoding */

int dummy_xship(FILE *f, int filter, char *s, int len) {
  fwrite(s, 1, len, f);
  return len;
}

/* ---------------------------------------------------------------------- */
/* flate interface: zlib (=postscript level 3) compression and a85 */

#ifdef HAVE_ZLIB

int pdf_xship(FILE *f, int filter, char *s, int len) {
	static int fstate = 0;
	static z_stream c_stream;
	char outbuf[OUTSIZE];
	int err;
	int n=0;

  if (filter && !fstate) {
    /* switch on filtering */
    c_stream.zalloc = Z_NULL;
    c_stream.zfree = Z_NULL;
    c_stream.opaque = Z_NULL;
    err = deflateInit(&c_stream, 9);
    if (err != Z_OK) {
      fprintf(stderr, "deflateInit: %s (%d)\n", c_stream.msg, err);
      exit(1);
    }
    c_stream.avail_in = 0;
    fstate = 1;
  } else if (!filter && fstate) {
    /* switch off filtering */
    /* flush stream */
    do {
      c_stream.next_out = (Bytef*)outbuf;
      c_stream.avail_out = OUTSIZE;

      err = deflate(&c_stream, Z_FINISH);
      if (err != Z_OK && err != Z_STREAM_END) {
	fprintf(stderr, "deflate: %s (%d)\n", c_stream.msg, err);
	exit(1);
      }
      n += fwrite(outbuf, 1, OUTSIZE-c_stream.avail_out, f);
    } while (err != Z_STREAM_END);

    fstate = 0;
  }
  if (!fstate) {
    fwrite(s, 1, len, f);
    return n+len;
  }
  
  /* do the actual compression */
  c_stream.next_in = (Bytef*) s;
  c_stream.avail_in = len;

  do {
    c_stream.next_out = (Bytef*) outbuf;
    c_stream.avail_out = OUTSIZE;

    err = deflate(&c_stream, Z_NO_FLUSH);
    if (err != Z_OK) {
      fprintf(stderr, "deflate: %s (%d)\n", c_stream.msg, err);
      exit(1);
    }
    n += fwrite(outbuf, 1, OUTSIZE-c_stream.avail_out, f);
  } while (!c_stream.avail_out);
  
  return n;
}

/* ship len bytes from s using zlib compression. */
int flate_xship(FILE *f, int filter, char *s, int len) {
  static int fstate = 0;
  static z_stream c_stream;
  char outbuf[OUTSIZE];
  int err;
  int n=0;

  if (filter && !fstate) {
    /* switch on filtering */
    if (filter == 1) {
      n += fprintf(f, "currentfile /ASCII85Decode filter /FlateDecode filter cvx exec\n");
    }
    c_stream.zalloc = Z_NULL;
    c_stream.zfree = Z_NULL;
    c_stream.opaque = Z_NULL;
    err = deflateInit(&c_stream, 9);
    if (err != Z_OK) {
      fprintf(stderr, "deflateInit: %s (%d)\n", c_stream.msg, err);
      exit(1);
    }
    c_stream.avail_in = 0;
    n += a85init(f);
    fstate = 1;
  } else if (!filter && fstate) {
    /* switch off filtering */
    /* flush stream */
    do {
      c_stream.next_out = (Bytef*)outbuf;
      c_stream.avail_out = OUTSIZE;

      err = deflate(&c_stream, Z_FINISH);
      if (err != Z_OK && err != Z_STREAM_END) {
	fprintf(stderr, "deflate: %s (%d)\n", c_stream.msg, err);
	exit(1);
      }
      n += a85write(f, outbuf, OUTSIZE-c_stream.avail_out);
    } while (err != Z_STREAM_END);

    n += a85finish(f);

    fstate = 0;
  }
  if (!fstate) {
    fwrite(s, 1, len, f);
    return n+len;
  }
  
  /* do the actual compression */
  c_stream.next_in = (Bytef*) s;
  c_stream.avail_in = len;

  do {
    c_stream.next_out = (Bytef*) outbuf;
    c_stream.avail_out = OUTSIZE;

    err = deflate(&c_stream, Z_NO_FLUSH);
    if (err != Z_OK) {
      fprintf(stderr, "deflate: %s (%d)\n", c_stream.msg, err);
      exit(1);
    }
    n += a85write(f, outbuf, OUTSIZE-c_stream.avail_out);
  } while (!c_stream.avail_out);
  
  return n;
}

#else  /* HAVE_ZLIB */

int pdf_xship(FILE *f, int filter, char *s, int len) {
  return dummy_xship(f, filter, s, len);
}

int flate_xship(FILE *f, int filter, char *s, int len) {
  return dummy_xship(f, filter, s, len);
}

#endif /* HAVE_ZLIB */

/* ---------------------------------------------------------------------- */
/* lzw interface: LZW (=postscript level 2) compression with a85.
   This relies on lzw.c/h to do the actual compression. */

/* use Postscript level 2 compression. Ship len bytes from str. */
int lzw_xship(FILE *f, int filter, char *str, int len) {
  static int fstate = 0;
  static lzw_stream_t *s = NULL;
  char outbuf[OUTSIZE];
  int err;
  int n=0;

  if (filter && !fstate) {
    /* switch on filtering */
    if (filter == 1) {
      n += fprintf(f, "currentfile /ASCII85Decode filter /LZWDecode filter cvx exec\n");
    }
    s = lzw_init();
    if (s == NULL) {
      fprintf(stderr, "lzw_init: %s\n", strerror(errno));
      exit(1);
    }
    n += a85init(f);
    fstate = 1;
  } else if (!filter && fstate) {
    /* switch off filtering */
    /* flush stream */
    s->next_in = 0;
    s->avail_in = 0;
    do {
      s->next_out = outbuf;
      s->avail_out = OUTSIZE;

      err = lzw_compress(s, LZW_EOD);
      if (err) {
	fprintf(stderr, "lzw_compress: %s\n", strerror(errno));
	exit(1);
      }
      n += a85write(f, outbuf, OUTSIZE - s->avail_out);
    } while (s->avail_out == 0);

    n += a85finish(f);

    lzw_free(s);
    s = NULL;

    fstate = 0;
  }
  if (!fstate) {
    fwrite(str, 1, len, f);
    return n+len;
  }
  
  /* do the actual compression */
  s->next_in = str;
  s->avail_in = len;

  do {
    s->next_out = outbuf;
    s->avail_out = OUTSIZE;

    err = lzw_compress(s, LZW_NORMAL);
    if (err) {
      fprintf(stderr, "lzw_compress: %s\n", strerror(errno));
      exit(1);
    }
    n += a85write(f, outbuf, OUTSIZE - s->avail_out);
  } while (s->avail_out == 0);
  
  return n;
}

/* ---------------------------------------------------------------------- */
/* a85 interface: a85 encoding without compression */

/* ship len bytes from s using a85 encoding only. */
int a85_xship(FILE *f, int filter, char *s, int len) {
  static int fstate = 0;
  int n=0;

  if (filter && !fstate) {
    /* switch on filtering */
    if (filter == 1) {
      n += fprintf(f, "currentfile /ASCII85Decode filter cvx exec\n");
    }
    n += a85init(f);
    fstate = 1;
  } else if (!filter && fstate) {
    /* switch off filtering */
    /* flush stream */
    n += a85finish(f);
    fstate = 0;
  }
  if (!fstate) {
    fwrite(s, 1, len, f);
    return n+len;
  }
  
  n += a85write(f, s, len);
  
  return n;
}

/* ---------------------------------------------------------------------- */
/* low-level a85 backend */

static unsigned long a85buf[4];
static int a85n;
static int a85col;

static int a85init(FILE *f) {
  a85n = 0;
  a85col = 0;
  return 0;
}

static int a85finish(FILE *f) {
  int r=0;

  if (a85n) {
    r+=a85out(f, a85n);
  }
  fputs("~>\n", f);
  return r+2;
}

static int a85write(FILE *f, char *buf, int n) {
  int i;
  int r=0;

  for (i=0; i<n; i++) {
    a85buf[a85n] = (unsigned char)buf[i];
    a85n++;
    
    if (a85n == 4) {
      r+=a85out(f, 4);
      a85n = 0;
    }
  }
  return r;
}

static int a85out(FILE *f, int n) {
  char out[5];
  unsigned long s;
  int r=0;
  int i;

  for (i=n; i<4; i++) {
    a85buf[i] = 0;
  }

  s = (a85buf[0]<<24) + (a85buf[1]<<16) + (a85buf[2]<<8) + (a85buf[3]<<0);

  if (!s) {
    r+=a85spool(f, 'z');
  } else {
    for (i=4; i>=0; i--) {
      out[i] = s % 85;
      s /= 85;
    }
    for (i=0; i<n+1; i++) {
      r+=a85spool(f, out[i]+33);
    }
  } 
  return r;
}

static int a85spool(FILE *f, char c) {
  fputc(c, f);

  a85col++;
  if (a85col>70) {
    fputc('\n', f);
    a85col=0;
    return 2;
  }
  
  return 1;
}
