/*
 *   ttfaux.c
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000, 2002, 2003 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_BBOX_H

#include "ttf2tfm.h"
#include "newobj.h"
#include "ligkern.h"
#include "ttfenc.h"
#include "tfmaux.h"
#include "errormsg.h"
#include "ttfaux.h"
#include "filesrch.h"
#include "ftlib.h"


char *real_ttfname;

int flags = FT_LOAD_DEFAULT|FT_LOAD_NO_SCALE;

FT_BBox              bbox;


static void
readttf_kern(Font *fnt)
{
  kern *nk;
  ttfinfo *ti, *tj;
  FT_Error error;
  FT_Vector akerning;

  for (ti = fnt->charlist; ti; ti = ti->next)
  {
    kern head;
    kern *tail = &head;
    head.next = NULL;
    for (tj = fnt->charlist; tj; tj = tj->next)
    {
      if ((error = FT_Get_Kerning(face, ti->glyphindex, tj->glyphindex,
                                  FT_KERNING_UNSCALED, &akerning)))
        oops("Cannot get kerning vector (error code = 0x%x).", error);

      if (akerning.x)
      {
        nk = newkern();
        nk->succ = tj->adobename;
        nk->delta = transform(akerning.x * 1000 / fnt->units_per_em, 0,
                              fnt->efactor, fnt->slant);
#if 0
        nk->next = ti->kerns;
        ti->kerns = nk;
#else
        /* We append the new kern to reproduce the old .vpl file */
        tail->next = nk;
        tail = nk;
#endif
      }
    }
    ti->kerns = head.next;
  }
}


void
readttf(Font *fnt, Boolean quiet, Boolean only_range)
{
  FT_Error error;
  ttfinfo *ti, *Ti;
  long Num, index;
  unsigned int i, j;
  long k, max_k;
  int index_array[257];

  static Boolean initialized = False;


  /*
   *   We allocate a placeholder boundary and the `.notdef' character.
   */

  if (!only_range)
  {
    ti = newchar(fnt);
    ti->charcode = -1;
    ti->adobename = ".notdef";

    ti = newchar(fnt);
    ti->charcode = -1;
    ti->adobename = "||"; /* boundary character name */
  }

  /*
   *   Initialize FreeType engine.
   */

  if (!initialized)
  {
    /*
     *   We use a dummy glyph size of 10pt.
     */
    dpi = 92;
    ptsize = 10 * 64;

    real_ttfname = TeX_search_ttf_file(&(fnt->ttfname));
    if (!real_ttfname)
      oops("Cannot find `%s'.", fnt->ttfname);

    FTopen(real_ttfname, fnt, True, True);

    initialized = True;
  }

  if (!quiet)
  {
    if (only_range)
      printf("\n\n%s:\n", fnt->fullname);
    printf("\n");
    printf("Glyph  Code   Glyph Name                ");
    printf("Width    llx   lly      urx   ury\n");
    printf("----------------------------------------");
    printf("---------------------------------\n");
  }

  /*
   *   We load only glyphs with a valid cmap entry.  Nevertheless, for
   *   the default mapping, we use the first 256 glyphs addressed by
   *   ascending code points, followed by glyphs not in the cmap.
   *
   *   If we compute a range, we take the character codes given in
   *   the fnt->sf_code array.
   *
   *   If the -N flag is set, no cmap is used at all.  Instead, the
   *   first 256 glyphs (with a valid PS name) are used for the default
   *   mapping.
   */

  if (!only_range)
    for (i = 0; i < 257; i++)
      index_array[i] = 0;
  else
    for (i = 0; i < 256; i++)
      fnt->inencptrs[i] = 0;

  j = 0;
  if (fnt->PSnames == Only)
    max_k = face->num_glyphs - 1;
  else
    max_k = only_range ? 0xFF : 0x16FFFF;

  for (k = 0; k <= max_k; k++)
  {
    char buff[128];
    const char *an;


    if (fnt->PSnames != Only)
    {
      if (only_range)
      {
        index = fnt->sf_code[k];
        if (index < 0)
          continue;
        j = k;
      }
      else
        index = k;

      Num = FT_Get_Char_Index(face, index);

      /* now we try to get a vertical glyph form */
      if (has_gsub)
        Num = Get_Vert(Num);

      if (Num < 0)
        oops("Failure on cmap mapping from %s.", fnt->ttfname);
      if (Num == 0)
        continue;
      if (!only_range)
        if (Num <= 256)
          index_array[Num] = 1;
    }
    else
    {
      Num = k;
      index = 0;
    }

    error = FT_Load_Glyph(face, Num, flags);
    if (!error)
    {
      if (fnt->efactor != 1.0 || fnt->slant != 0.0 )
        FT_Outline_Transform(&face->glyph->outline, &matrix1);
      if (fnt->rotate)
        FT_Outline_Transform(&face->glyph->outline, &matrix2);
      error = FT_Outline_Get_BBox(&face->glyph->outline, &bbox); /* we need the non-
                                                                    grid-fitted bbox */
    }
    if (!error)
    {
      if (fnt->PSnames)
      {
        (void)FT_Get_Glyph_Name(face, Num, buff, 128);
        an = newstring(buff);
      }
      else
        an = code_to_adobename(index);

      /* ignore characters not usable for typesetting with TeX */

      if (strcmp(an, ".notdef") == 0)
        continue;
      if (strcmp(an, ".null") == 0)
        continue;
      if (strcmp(an, "nonmarkingreturn") == 0)
        continue;

      ti = newchar(fnt);
      ti->charcode = index;
      ti->glyphindex = Num;
      ti->adobename = an;
      ti->llx = bbox.xMin * 1000 / fnt->units_per_em;
      ti->lly = bbox.yMin * 1000 / fnt->units_per_em;
      ti->urx = bbox.xMax * 1000 / fnt->units_per_em;
      ti->ury = bbox.yMax * 1000 / fnt->units_per_em;

      ti->fntnum = fnt->subfont_num;

      /*
       *   We must now shift the rotated character both horizontally
       *   and vertically.  The vertical amount is 25% by default.
       */

      if (fnt->rotate)
      {
        ti->llx += (face->glyph->metrics.vertBearingY - bbox.xMin) *
                     1000 / fnt->units_per_em;
        ti->lly -= 1000 * fnt->y_offset;
        ti->urx += (face->glyph->metrics.vertBearingY - bbox.xMin) *
                     1000 / fnt->units_per_em;
        ti->ury -= 1000 * fnt->y_offset;
      }

      /*
       *   We need to avoid negative heights or depths.  They break accents
       *   in math mode, among other things.
       */

      if (ti->lly > 0)
        ti->lly = 0;
      if (ti->ury < 0)
        ti->ury = 0;
      if (fnt->rotate)
        ti->width = face->glyph->metrics.vertAdvance * 1000 / fnt->units_per_em;
      else
        ti->width = transform(face->glyph->metrics.horiAdvance * 1000 / fnt->units_per_em,
                              0, fnt->efactor, fnt->slant);

      if (!quiet)
        printf("%5ld  %05lx  %-25s %5d  % 5d,% 5d -- % 5d,% 5d\n",
               Num, index, ti->adobename,
               ti->width,
               ti->llx, ti->lly, ti->urx, ti->ury);

      if (j < 256)
      {
        fnt->inencptrs[j] = ti;
        ti->incode = j;
      }
      j++;
    }
  }

  /*
   *   Now we load glyphs without a cmap entry, provided some slots are
   *   still free -- we skip this if we have to compute a range or use
   *   PS names.
   */

  if (!only_range && !fnt->PSnames)
  {
    for (i = 1; i <= face->num_glyphs; i++)
    {
      const char *an;


      if (index_array[i] == 0)
      {
        error = FT_Load_Glyph(face, i, flags);
        if (!error)
          error = FT_Outline_Get_BBox(&face->glyph->outline, &bbox);
        if (!error)
        {
          an = code_to_adobename(i | 0x1000000);

          ti = newchar(fnt);
          ti->charcode = i | 0x1000000;
          ti->glyphindex = i;
          ti->adobename = an;
          ti->llx = bbox.xMin * 1000 / fnt->units_per_em;
          ti->lly = bbox.yMin * 1000 / fnt->units_per_em;
          ti->urx = bbox.xMax * 1000 / fnt->units_per_em;
          ti->ury = bbox.yMax * 1000 / fnt->units_per_em;

          if (ti->lly > 0)
            ti->lly = 0;
          if (ti->ury < 0)
            ti->ury = 0;
          ti->width = transform(face->glyph->metrics.horiAdvance*1000 / fnt->units_per_em,
                                0, fnt->efactor, fnt->slant);

          if (!quiet)
            printf("%5d         %-25s %5d  % 5d,% 5d -- % 5d,% 5d\n",
                   i, ti->adobename,
                   ti->width,
                   ti->llx, ti->lly, ti->urx, ti->ury);

          if (j < 256)
          {
            fnt->inencptrs[j] = ti;
            ti->incode = j;
          }
          else
            break;
          j++;
        }
      }
    }
  }

  /* Finally, we construct a `Germandbls' glyph if necessary */

  if (!only_range)
  {
    if (NULL == findadobe("Germandbls", fnt->charlist) &&
        NULL != (Ti = findadobe("S", fnt->charlist)))
    {
      pcc *np, *nq;


      ti = newchar(fnt);
      ti->charcode = face->num_glyphs | 0x1000000;
      ti->glyphindex = face->num_glyphs;
      ti->adobename = "Germandbls";
      ti->width = Ti->width << 1;
      ti->llx = Ti->llx;
      ti->lly = Ti->lly;
      ti->urx = Ti->width + Ti->urx;
      ti->ury = Ti->ury;
      ti->kerns = Ti->kerns;

      np = newpcc();
      np->partname = "S";
      nq = newpcc();
      nq->partname = "S";
      nq->xoffset = Ti->width;
      np->next = nq;
      ti->pccs = np;
      ti->constructed = True;

      if (!quiet)
        printf("*            %-25s %5d  % 5d,% 5d -- % 5d,% 5d\n",
               ti->adobename,
               ti->width,
               ti->llx, ti->lly, ti->urx, ti->ury);
    }
  }

  /* kerning between subfonts isn't available */
  if (!only_range)
    readttf_kern(fnt);
}


/* end */
