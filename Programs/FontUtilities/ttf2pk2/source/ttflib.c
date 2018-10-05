/*
 *   ttflib.c
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000 by
 *     Loyer Frederic <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>       /* libc ANSI */
#include <ctype.h>

#include "pklib.h"      /* for the `byte' type */
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_BBOX_H
#include "ttfenc.h"
#include "ttflib.h"
#include "errormsg.h"
#include "newobj.h"
#include "ttf2tfm.h"
#include "ftlib.h"


FT_BBox		bbox;

FT_Bitmap Bit;
unsigned char *Bitp;

int ppem;



static void  
SetRasterArea(int quiet)
{
  int temp1_x, temp1_y, temp2_x, temp2_y;


  temp1_x = bbox.xMin / 64;         /* scaling F16.6 -> int */
  temp1_y = bbox.yMin / 64;
  temp2_x = (bbox.xMax + 63) / 64;
  temp2_y = (bbox.yMax + 63) / 64;

  if (!quiet)
    printf("  off = (%d, %d)", 5 - temp1_x, 5 - temp1_y);

  if (!quiet)
    printf("  bbox = (%d, %d) <->  (%d, %d)\n",
           temp1_x, temp1_y, temp2_x, temp2_y);

  Bit.rows = temp2_y - temp1_y + 10;
  Bit.width = temp2_x - temp1_x + 10;

  Bit.pitch = (Bit.width + 7) / 8;       /* convert to # of bytes */

  /*
   *   We allocate one more row to have valid pointers for comparison
   *   purposes in pklib.c, making `gcc -fbounds-checking' happy.
   */
  if (Bitp)
    free(Bitp);
  Bitp = mymalloc(Bit.rows*Bit.pitch + Bit.pitch);
  Bit.buffer = Bitp;
}

static void
CopyBit(FT_Bitmap *b2,FT_Bitmap *b1)
{
  int y;
  char *p1, *p2;

  p1 = (char *)b1->buffer; /* source */
  p2 = (char *)b2->buffer; /* dest */

  for (y = 0; y < b1->rows; y++)
  {
    memcpy(p2, p1, b1->pitch);
    p1 += b1->pitch;
    p2 += b2->pitch;
  }
}

#if 0

static void
Output(FT_Bitmap Bit)
{
  int x;
  int y;
  int i;

  char *p, b;


  p = Bit.buffer;
  printf("====\n");

  for (y = 0; y < Bit.rows; y++)
  {
    printf("%3d:", y);
    for (x = 0; x < Bit.pitch; x++)
    {
      b = *p++;
      for(i = 0x80; i; i >>= 1)
        printf((b & i) ? "x" : ".");
    }
    printf("\n");
  }
}

#endif /* 0 */


void
TTFopen(char *filename, Font *fnt, int new_dpi, int new_ptsize, Boolean quiet)
{
  dpi = new_dpi;
  ptsize = new_ptsize;
  ppem = (dpi * ptsize + 36) / 72;

  if (!quiet)
    printf("dpi = %d, ptsize = %d, ppem = %d\n\n", dpi, ptsize, ppem);

  FTopen(filename, fnt, False, quiet);
}


static FT_Error
LoadTrueTypeChar(Font *fnt,
                 int idx,
                 Boolean hint,
                 Boolean quiet)
{
  FT_Error error;
  int flags;


  flags = FT_LOAD_DEFAULT;
  if (hint)
    flags |= FT_LOAD_FORCE_AUTOHINT;

  error = FT_Load_Glyph(face, idx, flags);
  if (!error)
  {
    if (fnt->efactor != 1.0 || fnt->slant != 0.0 )
      FT_Outline_Transform(&face->glyph->outline, &matrix1);
    if (fnt->rotate)
    {
      FT_Outline_Transform(&face->glyph->outline, &matrix2);
      error = FT_Outline_Get_BBox(&face->glyph->outline, &bbox); /* we need the non-
                                                                    grid-fitted bbox */
      if (!error)
        FT_Outline_Translate(&face->glyph->outline,
                             face->glyph->metrics.vertBearingY - bbox.xMin,
                             -fnt->y_offset * ppem * 64);
    }
  }
  if (!error)
    error = FT_Outline_Get_BBox(&face->glyph->outline, &bbox);
  if (!error)
  {
    FT_Outline_Get_CBox(&face->glyph->outline, &bbox); /* for the case of BBox != CBox */
    SetRasterArea(quiet);
  }
  return error;
}


Boolean
TTFprocess(Font *fnt,
           long Code,
           byte **bitmap,
           int *width, int *height,
           int *hoff, int *voff,
           Boolean hinting,
           Boolean quiet)
{
  int Num;
  FT_Error error;


  if (!bitmap || !width || !height || !hoff || !voff)
    oops("Invalid parameter in call to TTFprocess()");

  if (Code >= 0x1000000)
    Num = Code & 0xFFFFFF;
  else
  {
    Num = FT_Get_Char_Index(face, Code);

    /* now we try to get a vertical glyph form */
    if (has_gsub)
      Num = Get_Vert(Num);
  }

  if ((error = LoadTrueTypeChar(fnt, Num, hinting, quiet)) == FT_Err_Ok)
  {
    error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
    if (error) {
        warning("Cannot Render to Bitmap");
	return False;
    }

    memset(Bit.buffer, 0, Bit.rows*Bit.pitch);
    CopyBit(&Bit, &face->glyph->bitmap);      

    *bitmap = Bit.buffer;
    *width = Bit.width;
    *height = face->glyph->bitmap.rows;
    *hoff = -face->glyph->bitmap_left;
    *voff = *height + 1 - face->glyph->bitmap_top;
    /* Output(Bit);                    */
    return True;
  }
  else
    return False;
}


/*
 *   We collect first all glyphs addressed via the cmap.  Then we fill the
 *   array up with glyphs not in the cmap.
 *
 *   If PSnames is set to `Only', we get the first 256 glyphs which have
 *   names different from `.notdef', `.null', and `nonmarkingreturn'.
 *
 *   For nicer output, we return the glyph names in an encoding array.
 */

encoding *
TTFget_first_glyphs(Font *fnt, long *array)
{
  unsigned int i, j, Num=0;
  unsigned int index_array[257];     /* we ignore glyph index 0 */
  char buff[128];
  const char *n;
  encoding *e = (encoding *)mymalloc(sizeof (encoding));


  if (!array)
    oops("Invalid parameter in call to TTFget_first_glyphs()");

  for (i = 0; i < 257; i++)
    index_array[i] = 0;

  j = 0;
  if (fnt->PSnames != Only)
  {
    for (i = 0; i <= 0x16FFFF; i++)
    {
      Num = FT_Get_Char_Index(face, i);
      if (Num == 0)
        continue;
      if (Num <= 256)
        index_array[Num] = 1;

      if (fnt->PSnames)
      {
	  (void)FT_Get_Glyph_Name(face,Num,buff,128);
	  n = newstring(buff);
      }
      else
        n = code_to_adobename(i);
      if (strcmp(n, ".notdef") == 0)
        continue;
      if (strcmp(n, ".null") == 0)
        continue;
      if (strcmp(n, "nonmarkingreturn") == 0)
        continue;

      if (j < 256)
      {
        array[j] = i;
        e->vec[j] = n;
      }
      else
        return e;
      j++;
    }

    if (!fnt->PSnames)
    {
      for (i = 1; i < face->num_glyphs; i++)
      {
        if (index_array[i] == 0)
        {
          if (j < 256)
          {
            array[j] = i | 0x1000000;
            e->vec[j] = code_to_adobename(i | 0x1000000);
          }
          else
            return e;
          j++;
        }
      }
    }
  }
  else
  {
    for (i = 0; i < face->num_glyphs; i++)
    {
      (void)FT_Get_Glyph_Name(face,Num,buff,128);
      n = newstring(buff);

      if (strcmp(n, ".notdef") == 0)
        continue;
      if (strcmp(n, ".null") == 0)
        continue;
      if (strcmp(n, "nonmarkingreturn") == 0)
        continue;

      if (j < 256)
      {
        array[j] = i | 0x1000000;
        e->vec[j] = n;
      }
      else
        return e;
      j++;
    }
  }

  return NULL;              /* never reached */
}


/*
 *   This routine fills `array' with the subfont character codes;
 *   additionally, it tests for valid glyph indices.
 */

void
TTFget_subfont(Font *fnt, long *array)
{
  int i, j, Num;


  if (!fnt || !array)
    oops("Invalid parameter in call to TTFget_subfont()");

  for (i = 0; i <= 0xFF; i++)
  {
    j = fnt->sf_code[i];

    if (j < 0)
      array[i] = j;
    else
    {
      Num = FT_Get_Char_Index(face, j);
      if (Num < 0)
        oops("cmap mapping failure.");
      else
        array[i] = j;
    }
  }
}


long
TTFsearch_PS_name(const char *name)
{
  unsigned int i;
  char buff[128];


  for (i = 0; i < face->num_glyphs; i++)
  {
    FT_Get_Glyph_Name(face, i, buff, 128);
    if (strcmp(name, buff) == 0)
      break;
  }

  if (i == face->num_glyphs)
    return -1L;
  else
    return (long)i;
}


/* end */
