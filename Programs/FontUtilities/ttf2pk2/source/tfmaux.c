/*
 *   tfmaux.c
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000, 2002 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ttf2tfm.h"
#include "newobj.h"
#include "tfmaux.h"
#include "errormsg.h"


#undef PI
#define PI 3.14159265358979323846264338327


struct sf            /* we need this for subfont ligatures */
{
  long sf_code;
  int position;
};


static long nextd;   /* smallest value that will give a different mincover */


static int lf, lh, nw, nh, nd, ni, nl, nk, ne, np;
static int bc, ec;

static long *header, *charinfo,
            *width, *height, *depth,
            *ligkern, *kerns, *tparam,
            *italic;


static int source[257];     /* utility variables for sorting tfm arrays */
static int unsort[257];


/*
 *   A simple function for sorting sf_array (in inverse order)
 */

static int
compare_sf(const void *a, const void *b)
{
  return (int)(((const struct sf *)b)->sf_code - ((const struct sf *)a)->sf_code);
}


/*
 *   The next routine simply scales something.
 *   Input is in TFM units per em.  Output is in FIXFACTORths of units
 *   per em.  We use 1 em = 1000 TFM units.
 */

static long
scale(long what)
{
  return ((what / 1000) * FIXFACTOR) +
         (((what % 1000) * FIXFACTOR) + 500) / 1000;
}


/*
 *   Next we need a routine to reduce the number of distinct dimensions
 *   in a TFM file.  Given an array what[0]..what[oldn-1], we want to
 *   group its elements into newn clusters, in such a way that the maximum
 *   difference between elements of a cluster is as small as possible.
 *   Furthermore, what[0]=0, and this value must remain in a cluster by
 *   itself.  Data such as `0 4 6 7 9' with newn=3 shows that an iterative
 *   scheme in which 6 is first clustered with 7 will not work.  So we
 *   borrow a neat algorithm from METAFONT to find the true optimum.
 *   Memory location what[oldn] is set to 0x7FFFFFFFL for convenience.
 */


/*
 *   Tells how many clusters result, given max difference d.
 */

static int
mincover(long *what,
         register long d)
{
  register int m;
  register long l;
  register long *p;


  nextd = 0x7FFFFFFFL;
  p = what+1;
  m = 1;

  while (*p < 0x7FFFFFFFL)
  {
    m++;
    l = *p;
    while (*++p <= l + d)
      ;
    if (*p - l < nextd)
      nextd = *p - l;
  }
  return m;
}


static void
remap(long *what,
      int oldn,
      int newn,
      int *source,
      int *unsort)
{
  register int i, j;
  register long d, l;

  what[oldn] = 0x7FFFFFFFL;
  for (i = oldn-1; i > 0; i--)
  {
    d = what[i];
    for (j = i; what[j+1] < d; j++)
    {
      what[j] = what[j+1];
      source[j] = source[j+1];
    }
    what[j] = d;
    source[j] = i;
  }

  i = mincover(what, 0L);
  d = nextd;
  while (mincover(what, d + d) > newn)
    d += d;
  while (mincover(what, d) > newn)
    d = nextd;

  i = 1;
  j = 0;
  while (i < oldn)
  {
    j++;
    l = what[i];
    unsort[source[i]] = j;
    while (what[++i] <= l + d)
    {
      unsort[source[i]] = j;
      if (i - j == oldn - newn)
        d = 0;
    }
    what[j] = (l + what[i-1])/2;
  }
}


static void
write16(int what, FILE *out)
{
  (void)fputc(what >> 8, out);
  (void)fputc(what & 0xFF, out);
}


static void
writearr(register long *p,
         register int n,
         register FILE *out)
{
  while (n)
  {
    write16((short)(*p >> 16), out);
    write16((short)(*p & 65535), out);
    p++;
    n--;
  }
}


static void
writesarr(long *what,
          int len,
          FILE *out)
{
  register long *p;
  int i;


  p = what;
  i = len;
  while (i)
  {
    *p = scale(*p);
    (void)scale(*p);    /* need this kludge for some compilers */
    p++;
    i--;
  }
  writearr(what, len, out);
}


static long *
makebcpl(register long *p,
         register const char *s,
         register int n)
{
  register long t;
  register long sc;


  if (strlen(s) < n)
    n = strlen(s);
  t = ((long)n) << 24;
  sc = 16;

  while (n > 0)
  {
    t |= ((long)(*(unsigned const char *)s++)) << sc;
    sc -= 8;
    if (sc < 0)
    {
      *p++ = t;
      t = 0;
      sc = 24;
    }
    n--;
  }
  if (t)
    *p++ = t;

  return p;
}


static long
checksum(ttfinfo **array)
{
  int i;
  unsigned long s1 = 0, s2 = 0;
  const char *p;
  ttfinfo *ti;


  for (i = 0; i < 256; i++)
    if (NULL != (ti = array[i]))
    {
      s1 = ((s1 << 1) ^ (s1 >> 31)) ^ ti->width; /* cyclic left shift */
      s1 &= 0xFFFFFFFF;         /* in case we're on a 64-bit machine */

      for (p = ti->adobename; *p; p++)
        s2 = (s2 * 3) + *p;
    }

  s1 = (s1 << 1) ^ s2;
  return s1;
}


int
transform(register int x, register int y,
          float ef, float sl)
{
  register double acc;


  acc = ef * x + sl * y;
  return (int)(acc >= 0 ? floor(acc + 0.5) : ceil(acc - 0.5));
}


int
buildtfm(Font *fnt)
{
  register int i, j;
  register ttfinfo *ti;
  int byte1, old_byte1, byte2;
  double Slant;
  char buffer[256];
  struct sf sf_array[256];


  if (fnt->subfont_ligs)
  {
    if (fnt->ligname)
    {
      for (i = 0; i < 256; i++)
      {
        sf_array[i].sf_code = fnt->sf_code[i];
        sf_array[i].position = (fnt->sf_code[i] == -1 ? -1 : i);
      }
    }
    else
    {
      for (i = 0; i < 256; i++)
      {
        ti = fnt->inencptrs[i];
        if (ti)
        {
          sf_array[i].sf_code = ti->charcode;
          sf_array[i].position = i;
        }
        else
        {
          sf_array[i].sf_code = -1;
          sf_array[i].position = -1;
        }
      }
    }
    /* we sort the subfont character codes before we build a ligkern list */
    qsort(sf_array, 256, sizeof (struct sf), compare_sf);

    /* we need to create dummy characters for the ligatures in case the
       character slots of the affected codes are unused */
    i = 0;
    while (i < 256 && sf_array[i].sf_code > -1)
    {
      byte1 = sf_array[i].sf_code >> 8;
      byte2 = sf_array[i].sf_code & 0xFF;
      if (!fnt->inencptrs[byte1])
      {
        ti = newchar(fnt);
        ti->llx = ti->lly = 0;
        ti->urx = ti->ury = 0;
        ti->width = 0;
        fnt->inencptrs[byte1] = ti;
        ti->incode = byte1;
        ti->adobename = ".dummy";
      }
      if (!fnt->inencptrs[byte2])
      {
        ti = newchar(fnt);
        ti->llx = ti->lly = 0;
        ti->urx = ti->ury = 0;
        ti->width = 0;
        fnt->inencptrs[byte2] = ti;
        ti->incode = byte2;
        ti->adobename = ".dummy";
      }
      i++;
    }
  }

  for (i = 0; i <= 0xFF && fnt->inencptrs[i] == NULL; i++)
    ;
  bc = i;
  for (i = 0xFF; i >= 0 && fnt->inencptrs[i] == NULL; i--)
    ;
  ec = i;

  if (ec < bc)
  {
    if (fnt->sfdname)
      return 0;
    else
      oops("No TTF characters.");
  }

  header = (long *)mycalloc(40000L, sizeof(char));
  fnt->cksum = checksum(fnt->inencptrs);
  header[0] = fnt->cksum;
  header[1] = 0xA00000;                     /* 10pt design size */

  (void)makebcpl(header + 2, fnt->codingscheme, 39);
  (void)makebcpl(header + 12, fnt->fullname, 19);

  /* 4 bytes are left free for the unused FACE value */

  buffer[0] = '\0';
  strncat(buffer, "Created by `", 12);
  strncat(buffer, fnt->titlebuf, 255 - 12 - 1);
  strncat(buffer, "'", 1);
  charinfo = makebcpl(header + 18, buffer, 255);
  lh = charinfo - header;

  width = charinfo + (ec - bc + 1);
  width[0] = 0;
  nw = 1;

  for (i = bc; i <= ec; i++)
    if (NULL != (ti = fnt->inencptrs[i]))
    {
      width[nw] = ti->width;
      for (j = 1; width[j] != ti->width; j++)
        ;
      ti->wptr = j;
      if (j == nw)
        nw++;
    }
  if (nw > 256)
    oops("256 chars with different widths.");

  depth = width + nw;
  depth[0] = 0;
  nd = 1;

  for (i = bc; i <= ec; i++)
    if (NULL != (ti = fnt->inencptrs[i]))
    {
      depth[nd] = -ti->lly;
      for (j = 0; depth[j] != -ti->lly; j++)
        ;
      ti->dptr = j;
      if (j == nd)
        nd++;
    }

  if (nd > 16)
  {
    remap(depth, nd, 16, source, unsort);
    nd = 16;
    for (i = bc; i <= ec; i++)
      if (NULL != (ti = fnt->inencptrs[i]))
        ti->dptr = unsort[ti->dptr];
  }

  height = depth + nd;
  height[0] = 0;
  nh = 1;

  for (i = bc; i <= ec; i++)
    if (NULL != (ti = fnt->inencptrs[i]))
    {
      height[nh] = ti->ury;
      for (j = 0; height[j] != ti->ury; j++)
        ;
      ti->hptr = j;
      if (j == nh)
        nh++;
    }

  if (nh > 16)
  {
    remap(height, nh, 16, source, unsort);
    nh = 16;
    for (i = bc; i <= ec; i++)
      if (NULL != (ti = fnt->inencptrs[i]))
        ti->hptr = unsort[ti->hptr];
  }

  italic = height + nh;
  italic[0] = 0;
  ni = 1;

  for (i = bc; i <= ec; i++)
    if (NULL != (ti = fnt->inencptrs[i]))
    {
      italic[ni] = ti->urx - ti->width;
      if (italic[ni] < 0)
        italic[ni] = 0;
      for (j = 0; italic[j] != italic[ni]; j++)
        ;
      ti->iptr = j;
      if (j == ni)
        ni++;
    }

  if (ni > 64)
  {
    remap(italic, ni, 64, source, unsort);
    ni = 64;
    for (i = bc; i <= ec; i++)
      if (NULL != (ti = fnt->inencptrs[i]))
        ti->iptr = unsort[ti->iptr];
  }

  for (i = bc; i <= ec; i++)
    if (NULL != (ti = fnt->inencptrs[i]))
      charinfo[i - bc] = ((long)(ti->wptr) << 24) +
                         ((long)(ti->hptr) << 20) +
                         ((long)(ti->dptr) << 16) +
                         ((long)(ti->iptr) << 10);
    else
      charinfo[i - bc] = 0;

  ligkern = italic + ni;
  nl = 0;

  if (fnt->subfont_ligs)
  {
    /* Now we build the ligature list.  The ligature consisting of character
       code byte1 + byte2 should yield the actual character.  The fonts of
       the HLaTeX package for Korean use this mechanism. */

    old_byte1 = -1;
    while (nl < 256 && sf_array[nl].sf_code > -1)
    {
      byte1 = sf_array[nl].sf_code >> 8;
      byte2 = sf_array[nl].sf_code & 0xFF;
      if (byte1 != old_byte1)
      {
        charinfo[byte1 - bc] += 0x100L +    /* set the lig tag */
                                nl;         /* set the position in array */
        if (old_byte1 > -1)
          ligkern[nl - 1] |= 0x80000000L;   /* set the STOP byte in previous
                                               ligkern command */
      }

      ligkern[nl] = ((long)byte2 << 16) +
                    (long)sf_array[nl].position;
      old_byte1 = byte1;
      nl++;
    }
    if (nl > 0)
      ligkern[nl - 1] |= 0x80000000L;
  }

  kerns = ligkern + nl;
  nk = 0;               /* kerns are omitted from raw TeX font */

  Slant = fnt->slant - fnt->efactor * tan(fnt->italicangle * (PI / 180.0));

  tparam = kerns + nk;
  tparam[0] = (long)(FIXFACTOR * Slant + 0.5);
  tparam[1] = scale((long)fnt->fontspace);
  tparam[2] = (fnt->fixedpitch ? 0 : scale((long)(300 * fnt->efactor + 0.5)));
  tparam[3] = (fnt->fixedpitch ? 0 : scale((long)(100 * fnt->efactor + 0.5)));
  tparam[4] = scale((long)fnt->xheight);
  tparam[5] = scale((long)(1000 * fnt->efactor + 0.5));
  np = 6;

  return 1;
}


void
writetfm(Font *fnt)
{
  FILE *out;
  char *tfm_name;
  int len = 0;


  if (fnt->tfm_path)
    len += strlen(fnt->tfm_path);
  len += strlen(fnt->fullname);
  len += strlen(fnt->tfm_ext);
  len++;

  tfm_name = (char *)mymalloc(len);
  tfm_name[0] = '\0';
  if (fnt->tfm_path)
    strcat(tfm_name, fnt->tfm_path);
  strcat(tfm_name, fnt->fullname);
  strcat(tfm_name, fnt->tfm_ext);

  if ((out = fopen(tfm_name, "wb")) == NULL)
    oops("Cannot open tfm file `%s'.", tfm_name);

  free(tfm_name);


  lf = 6 + lh + (ec - bc + 1) + nw + nh + nd + ni + nl + nk + ne + np;

  write16(lf, out);
  write16(lh, out);
  write16(bc, out);
  write16(ec, out);
  write16(nw, out);
  write16(nh, out);
  write16(nd, out);
  write16(ni, out);
  write16(nl, out);
  write16(nk, out);
  write16(ne, out);
  write16(np, out);
  writearr(header, lh, out);
  writearr(charinfo, ec - bc + 1, out);
  writesarr(width, nw, out);
  writesarr(height, nh, out);
  writesarr(depth, nd, out);
  writesarr(italic, ni, out);
  writearr(ligkern, nl, out);
  writesarr(kerns, nk, out);
  writearr(tparam, np, out);

  free(header);
  fclose(out);
}


void
writeenc(Font *fnt)
{
  FILE *out;
  char *enc_name;
  int i, len = 0;


  if (fnt->tfm_path)
    len += strlen(fnt->tfm_path);
  len += strlen(fnt->fullname);
  len += 5;                         /* ".enc" + 1 */

  enc_name = (char *)mymalloc(len);
  enc_name[0] = '\0';
  if (fnt->tfm_path)
    strcat(enc_name, fnt->tfm_path);
  strcat(enc_name, fnt->fullname);
  strcat(enc_name, ".enc");

  if ((out = fopen(enc_name, "wb")) == NULL)
    oops("Cannot open enc file `%s'.", enc_name);

  free(enc_name);

  fprintf(out, "/%sEncoding [", fnt->fullname);
  for (i = 0; i < 256; i++)
  {
    if (i % 4 == 0)
      fputc('\n', out);
    if (fnt->inencptrs[i] == NULL)
      fputs("/.notdef ", out);
    else if (strcmp(fnt->inencptrs[i]->adobename, ".notdef") == 0 ||
             strcmp(fnt->inencptrs[i]->adobename, ".null") == 0 ||
             strcmp(fnt->inencptrs[i]->adobename, ".nonmarkingreturn") == 0 ||
             strcmp(fnt->inencptrs[i]->adobename, ".dummy") == 0)
      fputs("/.notdef ", out);
    else
      fprintf(out, "/index0x%04X ", fnt->inencptrs[i]->glyphindex);
  }
  fputs("\n] def\n", out);

  fclose(out);
}


/* end */
