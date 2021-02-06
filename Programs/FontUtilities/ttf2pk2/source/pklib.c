/*
 *   pklib.c
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 *
 *   Trivial declaration change in 2020 by
 *     Karl Berry <tex-live@tug.org>
 */

/*
 *   This code has been derived from the program gsftopk.
 *   Here the original copyright.
 */

/*
 * Copyright (c) 1994 Paul Vojta.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>         /* for size_t */
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "newobj.h"
#include "pklib.h"
#include "errormsg.h"
#include "filesrch.h"

#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif

#define PK_PRE  (char)247
#define PK_ID   89
#define PK_POST (char)245
#define PK_NOP  (char)246

extern int dpi; /* defined in ftlib.c, avoiding -fno-common failure */

FILE *pk_file;


/*
 *   Information from the .tfm file.
 */

int tfm_lengths[12];

#define lh tfm_lengths[1]
#define bc tfm_lengths[2]
#define ec tfm_lengths[3]
#define nw tfm_lengths[4]

long checksum;
long design;
byte width_index[256];
long tfm_widths[256];

/*
 *   Information on the bitmap currently being worked on.
 */

byte   *bitmap;
int    width;
int    skip;
int    height;
int    hoff;
int    voff;
int    bytes_wide;
size_t bm_size;
byte   *bitmap_end;
int    pk_len;

/*
 *   Here's the path searching stuff.  First the typedefs and variables.
 */

static char searchpath[MAXPATHLEN + 1];

#define HUNKSIZE (MAXPATHLEN + 2)

struct spacenode                /* used for storage of directory names */
{
  struct spacenode *next;
  char             *sp_end;     /* end of data for this chunk */
  char             sp[HUNKSIZE];
} firstnode;



static FILE *
search_tfm(char **name)
{
  char *p;
  FILE *f;


  p = TeX_search_tfm(name);
  if (p == NULL)
    return NULL;
  strcpy(searchpath, p);
  f = fopen(searchpath, "rb");
  return f;
}


static long
getlong(FILE *f)
{
  unsigned long value;


  value  = (unsigned long)getc(f) << 24;
  value |= (unsigned long)getc(f) << 16;
  value |= (unsigned long)getc(f) << 8;
  value |= (unsigned long)getc(f);
  return value;
}


char line[82];


static byte masks[] = {0, 1, 3, 7, 017, 037, 077, 0177, 0377};

byte flag;
int pk_dyn_f;
int pk_dyn_g;
int base;          /* cost of this character if pk_dyn_f = 0 */
int deltas[13];    /* cost of increasing pk_dyn_f from i to i+1 */


/*
 *   Add up statistics for putting out the given shift count.
 */

static void
tallyup(int n)
{
  int m;


  if (n > 208)
  {
    ++base;
    n -= 192;
    for (m = 0x100; m != 0 && m < n; m <<= 4)
      base += 2;
    if (m != 0 && (m = (m - n) / 15) < 13)
      deltas[m] += 2;
  }
  else if (n > 13)
    ++deltas[(208 - n) / 15];
  else
    --deltas[n - 1];
}


/*
 *   Routines for storing the shift counts.
 */

static Boolean odd = False;
static byte part;


static void
pk_put_nyb(int n)
{
  if (odd)
  {
    *bitmap_end++ = (part << 4) | n;
    odd = False;
  }
  else
  {
    part = n;
    odd = True;
  }
}


static void
pk_put_long(int n)
{
  if (n >= 16)
  {
    pk_put_nyb(0);
    pk_put_long(n / 16);
  }
  pk_put_nyb(n % 16);
}


static void
pk_put_count(int n)
{
  if (n > pk_dyn_f)
  {
    if (n > pk_dyn_g)
      pk_put_long(n - pk_dyn_g + 15);
    else
    {
      pk_put_nyb(pk_dyn_f + (n - pk_dyn_f + 15) / 16);
      pk_put_nyb((n - pk_dyn_f - 1) % 16);
    }
  }
  else
    pk_put_nyb(n);
}


static void
trim_bitmap(void)
{
  byte *p;
  byte mask;


  /* clear out garbage bits in bitmap */

  if (width % 8 != 0)
  {
    mask = ~masks[8 - width % 8];
    for (p = bitmap + bytes_wide - 1; p < bitmap_end; p += bytes_wide)
      *p &= mask;
  }

  /* Find the bounding box of the bitmap. */

  /* trim top */

  skip = 0;
  mask = 0;

  for (;;)
  {
    if (bitmap >= bitmap_end)   /* if bitmap is empty */
    {
      width = height = hoff = voff = 0;
      return;
    }

    p = bitmap + bytes_wide;
    while (p > bitmap)
      mask |= *--p;
    if (mask)
      break;
    ++skip;
    bitmap += bytes_wide;
  }

  height -= skip;
  voff -= skip;

#ifdef  DEBUG
  if (skip < 2 || skip > 3)
    printf("Character has %d empty rows at top\n", skip);
#endif

  /* trim bottom */

  skip = 0;
  mask = 0;

  for (;;)
  {
    p = bitmap_end - bytes_wide;
    while (p < bitmap_end)
      mask |= *p++;
    if (mask)
      break;
    ++skip;
    bitmap_end -= bytes_wide;
  }

  height -= skip;

#ifdef  DEBUG
  if (skip < 2 || skip > 3)
    printf("Character has %d empty rows at bottom\n", skip);
#endif

  /* trim right */

  skip = 0;
  --width;

  for (;;)
  {
    mask = 0;
    for (p = bitmap + width / 8; p < bitmap_end; p += bytes_wide)
      mask |= *p;
    if (mask & (0x80 >> (width % 8)))
      break;

    --width;
    ++skip;
  }

  ++width;

#ifdef  DEBUG
  if (skip < 2 || skip > 3)
    printf("Character has %d empty columns at right\n", skip);
#endif

  /* trim left */

  skip = 0;

  for (;;)
  {
    mask = 0;
    for (p = bitmap + skip / 8; p < bitmap_end; p += bytes_wide)
      mask |= *p;
    if (mask & (0x80 >> (skip % 8)))
      break;

    ++skip;
  }

  width -= skip;
  hoff -= skip;

#ifdef  DEBUG
  if (skip < 2 || skip > 3)
    printf("Character has %d empty columns at left\n", skip);
#endif

  bitmap += skip / 8;
  skip = skip % 8;
}


/*
 *   Pack the bitmap using the rll method.  (Return false if it's better
 *   to just pack the bits.)
 */

static Boolean
pk_rll_cvt(void)
{
  static int   *counts   = NULL; /* area for saving bit counts */
  static int   maxcounts = 0;    /* size of this area */
  unsigned int ncounts;          /* max to allow this time */
  int          *nextcount;       /* next count value */

  int  *counts_end;              /* pointer to end */
  byte *rowptr;
  byte *p;
  byte mask;
  byte *rowdup;                  /* last row checked for dup */
  byte paint_switch;             /* 0 or 0xff */
  int  bits_left;                /* bits left in row */
  int  cost;
  int  i;


  /* Allocate space for bit counts. */

  ncounts = (width * height + 3) / 4;
  if (ncounts > maxcounts)
  {
    if (counts != NULL)
      free(counts);
    counts = (int *)mymalloc((ncounts + 2) * sizeof (int));
    maxcounts = ncounts;
  }
  counts_end = counts + ncounts;

  /* Form bit counts and collect statistics */

  base = 0;
  memset(deltas, 0, sizeof (deltas));
  rowdup = NULL;                    /* last row checked for duplicates */
  p = rowptr = bitmap;
  mask = 0x80 >> skip;
  flag = 0;
  paint_switch = 0;

  if (*p & mask)
  {
    flag = 8;
    paint_switch = 0xff;
  }

  bits_left = width;
  nextcount = counts;

  while (rowptr < bitmap_end)       /* loop over shift counts */
  {
    int shift_count = bits_left;


    for (;;)
    {
      if (bits_left == 0)
      {
        if ((p = rowptr += bytes_wide) >= bitmap_end)
          break;
        mask = 0x80 >> skip;
        bits_left = width;
        shift_count += width;
      }
      if (((*p ^ paint_switch) & mask) != 0)
        break;
      --bits_left;
      mask >>= 1;
      if (mask == 0)
      {
        ++p;
        while (*p == paint_switch && bits_left >= 8)
        {
          ++p;
          bits_left -= 8;
        }
        mask = 0x80;
      }
    }

    if (nextcount >= counts_end)
      return False;
    shift_count -= bits_left;
    *nextcount++ = shift_count;
    tallyup(shift_count);

    /* check for duplicate rows */
    if (rowptr != rowdup && bits_left != width)
    {
      byte *p1 = rowptr;
      byte *q  = rowptr + bytes_wide;
      int repeat_count;


      while (q < bitmap_end && *p1 == *q)
      {
        ++p1;
        ++q;
      }
      repeat_count = (p1 - rowptr) / bytes_wide;
      if (repeat_count > 0)
      {
        *nextcount++ = -repeat_count;
        if (repeat_count == 1)
          --base;
        else
        {
          ++base;
          tallyup(repeat_count);
        }
        rowptr += repeat_count * bytes_wide;
      }
      rowdup = rowptr;
    }
    paint_switch = ~paint_switch;
  }

#ifdef  DEBUG
  /*
   *      Dump the bitmap
   */

  for (p = bitmap; p < bitmap_end; p += bytes_wide)
  {
    byte *p1 = p;
    int j;


    mask = 0x80 >> skip;
    for (j = 0; j < width; ++j)
    {
      putchar(*p1 & mask ? '@' : '.');
      if ((mask >>= 1) == 0)
      {
        mask = 0x80;
        ++p1;
      }
    }
    putchar('\n');
  }
  putchar('\n');
#endif

  /* Determine the best pk_dyn_f */

  pk_dyn_f = 0;
  cost = base += 2 * (nextcount - counts);

  for (i = 1; i < 14; ++i)
  {
    base += deltas[i - 1];
    if (base < cost)
    {
      pk_dyn_f = i;
      cost = base;
    }
  }

  /* last chance to bail out */

  if (cost * 4 > width * height)
    return False;

  /* Pack the bit counts */

  pk_dyn_g = 208 - 15 * pk_dyn_f;
  flag |= pk_dyn_f << 4;
  bitmap_end = bitmap;
  *nextcount = 0;
  nextcount = counts;

  while (*nextcount != 0)
  {
    if (*nextcount > 0)
      pk_put_count(*nextcount);
    else
      if (*nextcount == -1)
        pk_put_nyb(15);
      else
      {
        pk_put_nyb(14);
        pk_put_count(-*nextcount);
      }
      ++nextcount;
  }

  if (odd)
  {
    pk_put_nyb(0);
    ++cost;
  }

  if (cost != 2 * (bitmap_end - bitmap))
    printf("Cost miscalculation:  expected %d, got %ld\n",
             cost, (long)(2 * (bitmap_end - bitmap)));
  pk_len = bitmap_end - bitmap;
  return True;
}


static void
pk_bm_cvt(void)
{
  byte *rowptr;
  byte *p;
  int  blib1;          /* bits left in byte */
  int  bits_left;      /* bits left in row */
  byte *q;
  int  blib2;
  byte nextbyte;


  flag = 14 << 4;
  q = bitmap;
  blib2 = 8;
  nextbyte = 0;

  for (rowptr = bitmap; rowptr < bitmap_end; rowptr += bytes_wide)
  {
    p = rowptr;
    blib1 = 8 - skip;
    bits_left = width;

    if (blib2 != 8)
    {
      int n;


      if (blib1 < blib2)
      {
        nextbyte |= *p << (blib2 - blib1);
        n = blib1;
      }
      else
      {
        nextbyte |= *p >> (blib1 - blib2);
        n = blib2;
      }
      blib2 -= n;
      if ((bits_left -= n) < 0)
      {
        blib2 -= bits_left;
        continue;
      }
      if ((blib1 -= n) == 0)
      {
        blib1 = 8;
        ++p;
        if (blib2 > 0)
        {
          nextbyte |= *p >> (8 - blib2);
          blib1 -= blib2;
          bits_left -= blib2;
          if (bits_left < 0)
          {
            blib2 = -bits_left;
            continue;
          }
        }
      }
      *q++ = nextbyte;
    }

    /* fill up whole (destination) bytes */

    while (bits_left >= 8)
    {
      nextbyte = *p++ << (8 - blib1);
      *q++ = nextbyte | (*p >> blib1);
      bits_left -= 8;
    }

    /* now do the remainder */

    nextbyte = *p << (8 - blib1);
    if (bits_left > blib1)
      nextbyte |= p[1] >> blib1;
    blib2 = 8 - bits_left;
  }

  if (blib2 != 8)
    *q++ = nextbyte;

  pk_len = q - bitmap;
}


static void
putshort(int w)
{
  putc(w >> 8, pk_file);
  putc(w, pk_file);
}


static void
putmed(long w)
{
  putc(w >> 16, pk_file);
  putc(w >> 8, pk_file);
  putc(w, pk_file);
}


static void
putlong(long w)
{
  putc(w >> 24, pk_file);
  putc(w >> 16, pk_file);
  putc(w >> 8, pk_file);
  putc(w, pk_file);
}


static char
xgetc(FILE *f)
{
  int c;


  c = getc(f);
  if (c == EOF)
    oops("Premature end of file.");
  return (byte)c;
}


/*
 *   Open and read the tfm file. 
 */

void
TFMopen(char **filename)
{
  FILE *tfm_file;
  int  i;
  int  cc;


  tfm_file = search_tfm(filename);
  if (tfm_file == NULL)
    oops("Cannot find tfm file.");

  for (i = 0; i < 12; i++)
  {
    int j;

    
    j = (int)((byte)getc(tfm_file)) << 8;
    tfm_lengths[i] = j | (int)((byte)xgetc(tfm_file));
  }

  checksum = getlong(tfm_file);
  design = getlong(tfm_file);
  fseek(tfm_file, 4 * (lh + 6), 0);

  for (cc = bc; cc <= ec; ++cc)
  {
    width_index[cc] = (byte)xgetc(tfm_file);

    (void)xgetc(tfm_file);
    (void)xgetc(tfm_file);
    (void)xgetc(tfm_file);
  }

  for (i = 0; i < nw; ++i)
    tfm_widths[i] = getlong(tfm_file);

  fclose(tfm_file);
}


/*
 *   Create pk file and write preamble.
 */

void
PKopen(char *filename,
       char *ident,
       int resolution)
{
  int ppp;
  int i;


  dpi = resolution;

  if ((pk_file = fopen(filename, "wb")) == NULL)
  {
    perror(filename);
    exit(1);
  }

  putc(PK_PRE, pk_file);
  putc(PK_ID, pk_file);

  i = strlen(ident);

  putc(i, pk_file);
  fwrite(ident, 1, i, pk_file);
  putlong(design);
  putlong(checksum);
  ppp = dpi / 72.27 * 65536.0 + 0.5;
  putlong(ppp);                         /* hppp */
  putlong(ppp);                         /* vppp */
}


void
PKputglyph(int cc,
           int llx, int lly, int urx, int ury, 
           int w, int h,
           byte *b)
{
  float char_width;

  long dm;
  long tfm_wid;


  bitmap = b;
  width = w;
  height = h;

  hoff = -llx;
  voff = ury - 2; /* Don't ask me why `-2' */
                  /*   Fred                */

  if (width != urx - llx || height != ury - lly)
    oops("Dimensions do not match:  (%d - %d) (%d - %d) <=> %d %d",
           llx, lly, urx, ury, width, height);

  bytes_wide = (width + 7) / 8;
  bm_size = bytes_wide * height;
  bitmap_end = bitmap + bm_size;

  trim_bitmap();

  if (height == 0 || !pk_rll_cvt())
    pk_bm_cvt();

  if (!width_index[cc])
    return;

  tfm_wid = tfm_widths[width_index[cc]];
  char_width = tfm_wid / 1048576.0 * design / 1048576.0 * dpi / 72.27;
  dm = (long)(char_width + 0.5) - (char_width < -0.5);

  if (pk_len + 8 < 4 * 256 && tfm_wid < (1<<24) &&
      dm >= 0 && dm < 256 && width < 256 && height < 256 &&
      hoff >= -128 && hoff < 128 && voff >= -128 && voff < 128)
  {
    putc(flag | ((pk_len + 8) >> 8), pk_file);
    putc(pk_len + 8, pk_file);
    putc(cc, pk_file);
    putmed(tfm_wid);
    putc(dm, pk_file);
    putc(width, pk_file);
    putc(height, pk_file);
    putc(hoff, pk_file);
    putc(voff, pk_file);
  }
  else if (pk_len + 13 < 3 * 65536L && tfm_wid < (1<<24) &&
           dm >= 0 && dm < 65536L && width < 65536L && height < 65536L &&
           hoff >= -65536L && hoff < 65536L &&
           voff >= -65536L && voff < 65536L)
  {
    putc(flag | 4 | ((pk_len + 13) >> 16), pk_file);
    putshort(pk_len + 13);
    putc(cc, pk_file);
    putmed(tfm_wid);
    putshort(dm);
    putshort(width);
    putshort(height);
    putshort(hoff);
    putshort(voff);
  }
  else
  {
    putc(flag | 7, pk_file);
    putlong(pk_len + 28);
    putlong(cc);
    putlong(tfm_wid);
    putlong((long)(char_width * 65536.0 + 0.5) - (char_width < -0.5));
    putlong(0);
    putlong(width);
    putlong(height);
    putlong(hoff);
    putlong(voff);
  }
  fwrite(bitmap, 1, pk_len, pk_file);
}


void
PKclose(void)
{
  putc(PK_POST, pk_file);
  while (ftell(pk_file) % 4 != 0)
    putc(PK_NOP, pk_file);

  fclose(pk_file);
}


/* end */
