/*
 *   ftlib.c
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000, 2002, 2003 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 *
 *   Copyright (C) 2012, 2013 by
 *     Peter Breitenlohner <tex-live@tug.org>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H
#include FT_OPENTYPE_VALIDATE_H

#include "ttf2tfm.h"
#include "ttfenc.h"
#include "errormsg.h"
#include "newobj.h"
#include "ftlib.h"

#define Macintosh_platform 1
#define Macintosh_encoding 0

#define Microsoft_platform 3
#define Microsoft_Symbol_encoding 0
#define Microsoft_Unicode_encoding 1

FT_Library  engine;
FT_Face     face;

FT_Matrix   matrix1, matrix2;

int dpi, ptsize;

Boolean has_gsub;

TT_OS2 *os2;
TT_Postscript  *postscript;

FT_Byte *GSUB_table;
FT_ULong GSUB_length;
FT_ULong GSUB_ptr;

static inline void
need (FT_ULong needed)
{
  if (GSUB_ptr + needed > GSUB_length)
    oops("GSUB: Unexpected end of table.");
}

static inline FT_Byte
get_Byte(void)
{
  return GSUB_table[GSUB_ptr++];
}

static inline signed char
get_SByte(void)
{
  return (signed char) GSUB_table[GSUB_ptr++];
}

static FT_UShort
get_UShort(void)
{
  unsigned int cc;
  cc = get_Byte() << 8;
  cc |= get_Byte();
  return (FT_UShort) cc;
}

static FT_Short
get_Short(void)
{
  int cc;
  cc = get_SByte() << 8;
  cc |= get_Byte();
  return (FT_Short) cc;
}

static FT_ULong
get_ULong(void)
{
  unsigned int cc;
  cc = get_Byte() << 24;
  cc |= get_Byte() << 16;
  cc |= get_Byte() << 8;
  cc |= get_Byte();
  return (FT_ULong) cc;
}

#if 0 /* unused */
static FT_Long
get_Long(void)
{
  int cc;
  cc = get_SByte() << 24;
  cc |= get_Byte() << 16;
  cc |= get_Byte() << 8;
  cc |= get_Byte();
  return (FT_Long) cc;
}
#endif

typedef struct {
  FT_UShort start;
  FT_UShort end;
  FT_UShort start_Index;
} Range_Record;

typedef struct {
  FT_ULong ptr;
  FT_UShort format;
  FT_UShort count;
  union {
    FT_UShort *glyphs;      /* format = 1 */
    Range_Record *ranges;   /* format = 2 */
  } data;
} Coverage_Item;

static inline void
fetch_Glyphs (FT_UShort count, FT_UShort *glyphs)
{
  int i;

  need(2 * count);
  for (i = 0; i < count; i++)
    glyphs[i]  = get_UShort();
  for (i = 1; i < count; i++)
    if (glyphs[i] <= glyphs[i - 1])
      oops("Glyph[%d]=%d and Glyph[%d]=%d: out of order.", i - 1, glyphs[i - 1], i, glyphs[i]);
}

static inline FT_UShort
fetch_Ranges (FT_UShort count, Range_Record *ranges)
{
  int i;
  FT_UShort ndx = 0;

      need(6 * count);
      for (i = 0; i < count; i++)
      {
        ranges[i].start = get_UShort();
        ranges[i].end = get_UShort();
        if (ranges[i].start > ranges[i].end)
          oops("Range[%d]: bad range=%d..%d.", i, ranges[i].start, ranges[i].end);
        ranges[i].start_Index = get_UShort();
        if (ranges[i].start_Index != ndx)
          oops("Range[%d]: bad start_Index=%d (should be %d).", i, ranges[i].start_Index, ndx);
        ndx += (ranges[i].end - ranges[i].start) + 1;
      }
      for (i = 1; i < count; i++)
        if (ranges[i].start <= ranges[i-1].end)
          oops("Range[%d]=%d..%d and Range[%d]=%d..%d: out of order.",
               i - 1, ranges[i - 1].start, ranges[i - 1].end, i, ranges[i].start, ranges[i].end);
      return ndx;
}

static inline FT_UShort
fetch_Coverage (Coverage_Item *coverage)
{
  FT_UShort res = 0;
  GSUB_ptr = coverage->ptr;
  need(4);
  coverage->format = get_UShort();
  coverage->count = res = get_UShort();
  switch (coverage->format)
  {
    case 1:
      coverage->data.glyphs = mymalloc (res * sizeof(FT_UShort));
#ifdef  DEBUG
      warning("Coverage Format=1 with %d Glyphs at 0x%04x.", res, coverage->ptr);
#endif
      fetch_Glyphs (res, coverage->data.glyphs);
      break;
    case 2:
      coverage->data.ranges = mymalloc (res * sizeof(Range_Record));
#ifdef  DEBUG
      warning("Coverage Format=1 with %d Ranges at 0x%04x.", res, coverage->ptr);
#endif
      res = fetch_Ranges (res, coverage->data.ranges);
      break;
    default:
      oops("Coverage at 0x%04x: bad Format=%d.", coverage->ptr, coverage->format);
  }
  return res;
}

/*
 *   We only accept Single Substitutions (and Extensions)
 */

typedef struct {
  FT_UShort count;
  FT_UShort *glyphs;
} Glyph_Array;

typedef struct {
  FT_ULong ptr;
  FT_UShort format;
  Coverage_Item *coverage;
  union {
    FT_Short *delta;        /* format = 1 */
    Glyph_Array *array;     /* format = 2 */
  } data;
} Single_Item;

typedef struct _Subst_Item {
  Single_Item *item;
  struct _Subst_Item *next;
} Subst_Item;

Subst_Item *Subst_list, *Subst_last;

static inline void
add_Single (Single_Item *item)
{
  Subst_Item *entry = mycalloc (1, sizeof(Subst_Item));
  entry->item = item;
  if (Subst_last)
    Subst_last->next = entry;
  else
    Subst_list = entry;
  Subst_last = entry;
  has_gsub = True;
}

static inline void
fetch_Single (Single_Item *item)
{
  int i;
  FT_UShort val;

  item->coverage = mycalloc (1, sizeof(Coverage_Item));
  GSUB_ptr = item->ptr;
  need(6);
  item->format = get_UShort();
  item->coverage->ptr = item->ptr + get_UShort();
  switch (item->format)
  {
    case 1:
      val = get_Short();
      item->data.delta = mycalloc (1, sizeof(FT_UShort));
      *item->data.delta = val;
#ifdef  DEBUG
      warning("Single Substitution Format=1 with Delta=%d at 0x%04x.", val, item->ptr);
#endif
      fetch_Coverage (item->coverage);
      break;
    case 2:
      val = get_UShort();
      need(2 * val);
      item->data.array = mycalloc (1, sizeof(Glyph_Array));
      item->data.array->count = val;
      item->data.array->glyphs = mycalloc (val, sizeof(FT_UShort));
      for (i = 0; i < val; i++)
        item->data.array->glyphs[i] = get_UShort();
#ifdef  DEBUG
      warning("Single Substitution Format=2 with %d Glyphs at 0x%04x.", val, item->ptr);
#endif
      val = fetch_Coverage (item->coverage);
      if (val != item->data.array->count)
        oops("Coverage at 0x%04x: covers %d glyphs (should be %d).", item->ptr, val, item->data.array->count);
      break;
    default:
      oops("Single Substitution at 0x%04x: bad Format=%d.", item->ptr, item->format);
  }
  
  add_Single (item);
}

typedef struct {
  FT_UShort count;
  Single_Item *list;
} Lookup_Item;

typedef struct {
  FT_ULong ptr;
  Lookup_Item *entry;
} Lookup_Record;

Lookup_Record *Lookup_List;
FT_UShort Lookup_Count;

static inline void
fetch_Lookup (int i)
{
  FT_ULong Lookup_ptr = Lookup_List[i].ptr;

  if (Lookup_ptr && !Lookup_List[i].entry)
  {
    Lookup_Item *entry = Lookup_List[i].entry = mycalloc (1, sizeof(Lookup_Item));
    Boolean is_ext = False;
    FT_UShort val;
    int j;

#ifdef  DEBUG
    warning("Lookup[%d] at 0x%04x.", i, Lookup_ptr);
#endif
    GSUB_ptr = Lookup_ptr;

    /*
     *   Read Lookup Item
     */
    need(6);
    val = get_UShort();
    if (val == 7)
      is_ext = True;
    else if (val != 1)
      oops("Lookup[%d] at 0x%04x: bad Type=%d.", i, Lookup_ptr, val);
    get_UShort(); /* Ignore Lookup Flag */
    entry->count = get_UShort();
    entry->list = mycalloc (entry->count, sizeof(Single_Item));

    need(2 * entry->count);
    for (j = 0; j < entry->count; j++)
      entry->list[j].ptr = Lookup_ptr + get_UShort();

    if (is_ext)
      for (j = 0; j < entry->count; j++)
      {
        GSUB_ptr = entry->list[j].ptr;
        need(8);
        val = get_UShort();
        if (val != 1)
          oops("Lookup[%d] Extension[%d] at 0x%04x: bad Format=%d.", i, j, entry->list[j].ptr, val);
        val = get_UShort();
        if (val != 1)
          oops("Lookup[%d] Extension[%d] at 0x%04x: bad Type=%d.", i, j, entry->list[j].ptr, val);
        entry->list[j].ptr += get_ULong();
      }

    for (j = 0; j < entry->count; j++)
      fetch_Single (&entry->list[j]);
  }
}

typedef struct {
  FT_UShort count;
  FT_UShort *list;
} Feature_Item;

typedef struct {
  FT_Tag tag;
  FT_ULong ptr;
  Feature_Item *entry;
} Feature_Record;

Feature_Record *Feature_List;
FT_UShort Feature_Count;

/*
 *   We check for the `vert' or `vrt2' feature in Chinese, Japanese, and Korean
 */
#define FEATURE_vert FT_MAKE_TAG('v', 'e', 'r', 't')
#define FEATURE_vrt2 FT_MAKE_TAG('v', 'r', 't', '2')

#define print_Tag(t) t >> 24, (t >> 16) & 0xff, (t >> 8) & 0xff, t & 0xff

static inline void
fetch_Feature (int i)
{
  FT_ULong Feature_ptr = Feature_List[i].ptr;

  if (Feature_ptr && !Feature_List[i].entry)
  {
    Feature_Item *entry = Feature_List[i].entry = mycalloc (1, sizeof(Feature_Item));  
    FT_UShort val;
    int j;

#ifdef  DEBUG
    warning("Feature[%d] '%c%c%c%c' at 0x%04x.", i, print_Tag(Feature_List[i].tag), Feature_ptr);
#endif
    GSUB_ptr = Feature_ptr;

    /*
     *   Read Feature Item
     */
    need(4);
    val = get_UShort();
    if (val)
      oops("Feature[%d] at 0x%04x: bad FeaturParams 0x%04x.", Feature_ptr, i, val);
    entry->count = get_UShort();
    entry->list = mycalloc (entry->count, sizeof(FT_UShort));

    need(2*entry->count);
    for (j = 0; j < entry->count; j++) {
      entry->list[j] = get_UShort();
      if (entry->list[j] >= Lookup_Count)
        oops("Feature[%d] at 0x%04x: bad Lookup Index[%d]=%d.", i, Feature_ptr, j, entry->list[j]);
    }

    for (j = 0; j < entry->count; j++)
      fetch_Lookup (entry->list[j]);
  }
}

#define SCRIPT_kana FT_MAKE_TAG('k', 'a', 'n', 'a')
#define SCRIPT_hani FT_MAKE_TAG('h', 'a', 'n', 'i')
#define SCRIPT_hang FT_MAKE_TAG('h', 'a', 'n', 'g')

#define LANGUAGE_JAN FT_MAKE_TAG('J', 'A', 'N', ' ')
#define LANGUAGE_CHN FT_MAKE_TAG('C', 'H', 'N', ' ')
#define LANGUAGE_KOR FT_MAKE_TAG('K', 'O', 'R', ' ')

struct _Script_Lang
{
  FT_Tag script;
  FT_Tag language;
};

static const struct _Script_Lang Script_Lang[] = {
  { SCRIPT_kana, LANGUAGE_JAN },
  { SCRIPT_hani, LANGUAGE_CHN },
  { SCRIPT_hang, LANGUAGE_KOR }
};
#define num_Script_Lang (sizeof(Script_Lang) / sizeof(Script_Lang[0]))

typedef struct {
  FT_UShort req;
  FT_UShort count;
  FT_UShort *list;
} LangSys_Item;

typedef struct {
  FT_Tag tag;
  FT_ULong ptr;
  LangSys_Item *entry;
} LangSys_Record;

typedef struct {
  FT_ULong ptr;
  LangSys_Item *entry;
  FT_UShort count;
  LangSys_Record *list;
} Script_Item;

typedef struct {
  FT_Tag tag, ltag;
  FT_ULong ptr;
  Script_Item *entry;
} Script_Record;

Script_Record *Script_List;
FT_UShort Script_Count;

static inline void
fetch_LangSys (FT_ULong ptr, LangSys_Item **entry)
{
  LangSys_Item * item;
  FT_UShort val;
  int i;

  *entry = item = mycalloc (1, sizeof(LangSys_Item));
  GSUB_ptr = ptr;

  /*
   *   Read LangSys Item
   */
  need(6);
  val = get_UShort();
  if (val)
    oops("LangSys at 0x%04x: bad LookupOrder 0x%04x.", ptr, val);
  item->req = get_UShort();
  if (item->req != 0xffff && item->req >= Feature_Count)
    oops("LangSys at 0x%04x: bad required Feature Index=%d.", ptr, item->req);
  item->count = get_UShort();
  item->list = mycalloc (item->count, sizeof(FT_UShort));

  need(2 * item->count);
  for (i = 0; i < item->count; i++) {
    item->list[i] = get_UShort();
    if (item->list[i] >= Feature_Count)
      oops("LangSys at 0x%04x: bad Feature Index[%d]=%d.", ptr, i, item->list[i]);
  }

  if (item->req != 0xffff)
    fetch_Feature (item->req);
  for (i = 0; i < item->count; i++)
    fetch_Feature (item->list[i]);
}

static inline void
fetch_Script (int i)
{
  FT_ULong Script_ptr = Script_List[i].ptr;

  if (Script_ptr)
  {
    Script_Item *entry = Script_List[i].entry = mycalloc (1, sizeof(Script_Item));
    int j;

#ifdef  DEBUG
    warning("Script[%d] '%c%c%c%c' at 0x%04x.", i, print_Tag(Script_List[i].tag), Script_ptr);
#endif
    GSUB_ptr = Script_ptr;

    /*
     *   Read Script Item
     */
    need(4);
    if ((entry->ptr = get_UShort()))
      entry->ptr += Script_ptr;
    entry->count = get_UShort();
    entry->list = mycalloc (entry->count, sizeof(LangSys_Record));

    need(6 * entry->count);
    for (j = 0; j < entry->count; j++) {
      FT_Tag tag = Script_List[i].tag;
      FT_ULong ptr = Script_ptr + get_UShort();

      entry->list[j].tag = tag;
      if (tag == Script_List[i].ltag)
        entry->list[j].ptr = ptr;
    }

    /*
     *   Read Default LangSys entriy
     */
    if (entry->ptr) {
#ifdef  DEBUG
      warning("Script[%d] Default LangSys at 0x%04x.", i, entry->ptr);
#endif
      fetch_LangSys (entry->ptr, &entry->entry);
    }

    /*
     *   Read language LangSys entries
     */
    for (j = 0; j < entry->count; j++)
      if (entry->list[j].ptr) {
#ifdef  DEBUG
        warning("Script[%d] LangSys[%d] '%c%c%c%c' at 0x%04x.", i, j,
                print_Tag(entry->list[j].tag), entry->list[j].ptr); 
#endif
        fetch_LangSys (entry->list[j].ptr, &entry->list[j].entry);
      }
  }
}

#define TTAG_GSUB FT_MAKE_TAG('G', 'S', 'U', 'B')

static inline void
fetch_GSUB (void)
{
  FT_Error error;
  int i, j;

  FT_ULong Script_List_ptr, Feature_List_ptr, Lookup_List_ptr;

  /*
   *   Fetch GSUB table
   */
  if ((error = FT_Load_Sfnt_Table(face, TTAG_GSUB, 0, NULL, &GSUB_length)))
  {
#ifdef  DEBUG
    warning("No GSUB data available for vertical glyph presentation forms.");
#endif
    return;
  }
  GSUB_table = mymalloc(GSUB_length);
  if ((error = FT_Load_Sfnt_Table(face, TTAG_GSUB, 0, GSUB_table, &GSUB_length)))
  {
    warning("Cannot load GSUB table (error code = 0x%x).", error);
    return;
  }

  /*
   *   Read GSUB header
   */
  need(10);
  if (get_ULong() != 0x00010000)
    oops("GSUB: Bad version.");
  Script_List_ptr = get_UShort();
  Feature_List_ptr = get_UShort();
  Lookup_List_ptr = get_UShort();

  /*
   *   Read Script_List
   */
  GSUB_ptr = Script_List_ptr;
  need(2);
  Script_Count = get_UShort();
#ifdef  DEBUG
  warning("GSUB: Script List with %d entries at 0x%04x.", Script_Count, Script_List_ptr);
#endif
  Script_List = mycalloc (Script_Count, sizeof(Script_Record));

  need (6 * Script_Count);
  for (i = 0; i < Script_Count; i++)
  {
    FT_Tag tag = get_ULong();
    FT_ULong ptr = Script_List_ptr + get_UShort();
    for (j = 0; j < num_Script_Lang; j++)
      if (tag == Script_Lang[j].script)
      {
        Script_List[i].tag = tag;
        Script_List[i].ltag = Script_Lang[j].language;
        Script_List[i].ptr = ptr;
      }
  }

  /*
   *   Read Feature_List
   */
  GSUB_ptr = Feature_List_ptr;
  need(2);
  Feature_Count = get_UShort();
#ifdef  DEBUG
  warning("GSUB: Feature List with %d entries at 0x%04x.", Feature_Count, Feature_List_ptr);
#endif
  Feature_List = mycalloc (Feature_Count, sizeof(Feature_Record));

  need (6 * Feature_Count);
  for (i = 0; i < Feature_Count; i++)
  {
    FT_Tag tag = get_ULong();
    FT_ULong ptr = Feature_List_ptr + get_UShort();
    if (tag == FEATURE_vert || tag == FEATURE_vrt2)
    {
      Feature_List[i].tag = tag;
      Feature_List[i].ptr = ptr;
    }
  }

  /*
   *   Read Lookup_List
   */
  GSUB_ptr = Lookup_List_ptr;
  need(2);
  Lookup_Count = get_UShort();
#ifdef  DEBUG
  warning("GSUB: Lookup List with %d entries at 0x%04x.", Lookup_Count, Lookup_List_ptr);
#endif
  Lookup_List = mycalloc (Lookup_Count, sizeof(Lookup_Record));

  need (2 * Lookup_Count);
  for (i = 0; i < Lookup_Count; i++)
    Lookup_List[i].ptr = Lookup_List_ptr + get_UShort();

  /*
   *   Read Script_List entries
   */
  for (i = 0; i < Script_Count && !has_gsub; i++)
    fetch_Script (i);
}

void
FTopen (char *filename, Font *fnt, Boolean do_tfm, Boolean quiet)
{
  FT_Error error;
  int i;
  unsigned short num_cmap, cmap_plat=0, cmap_enc=0;

  if ((error = FT_Init_FreeType(&engine)))
    oops("Cannot initialize FreeType engine (error code = 0x%x).", error);

  /*
   *   Load face.
   */
  if ((error = FT_New_Face(engine, filename, 0, &face)))
    oops("Cannot open `%s'.", filename);

  if (face->num_faces == 1)
  {
    if (fnt->fontindex != 0)
    {
      warning("This isn't a TrueType collection.\n"
              "Parameter `%s' is ignored.", do_tfm ? "-f" : "Fontindex");
      fnt->fontindex = 0;
    }
    fnt->fontindexparam = NULL;
  }
  else
  {
    if (fnt->fontindex != 0)
    {
      /*
       *   Now we try to open the proper font in a collection.
       */
      FT_Done_Face(face);
      if ((error = FT_New_Face(engine, filename,  (FT_Long)fnt->fontindex, &face)))
        oops("Cannot open font %lu in TrueType Collection `%s'.",
             fnt->fontindex, filename);
    }
  }

  if (do_tfm)
  {
    /*
     *   Get the OS/2 table.
     */
    if ((os2 = FT_Get_Sfnt_Table(face, ft_sfnt_os2)) == NULL)
      oops("Cannot find OS/2 table for `%s'.", filename);

    /*
     *   Get the Postscript table.
     */
    if ((postscript = FT_Get_Sfnt_Table(face, ft_sfnt_post)) == NULL)
      oops("Cannot find Postscript table for `%s'.", filename);
  }

  if ((error = FT_Set_Char_Size(face, ptsize * 64, ptsize * 64, dpi, dpi)))
    oops("Cannot set character size (error code = 0x%x).", error);

  matrix1.xx = (FT_Fixed)(floor(fnt->efactor * 1024) * (1<<16)/1024);
  matrix1.xy = (FT_Fixed)(floor(fnt->slant * 1024) * (1<<16)/1024);
  matrix1.yx = (FT_Fixed)0;
  matrix1.yy = (FT_Fixed)(1<<16);

  if (fnt->rotate)
  {
    matrix2.xx = 0;
    matrix2.yx = 1L << 16;
    matrix2.xy = -matrix2.yx;
    matrix2.yy = matrix2.xx;
  }

  if (do_tfm)
  {
    fnt->units_per_em = face->units_per_EM;
    fnt->fixedpitch = postscript->isFixedPitch;
    fnt->italicangle = postscript->italicAngle / 65536.0;
    fnt->xheight = os2->sxHeight * 1000 / fnt->units_per_em;
  }

  if (fnt->PSnames != Only)
  {
    num_cmap = face->num_charmaps;
    for (i = 0; i < num_cmap; i++)
    {
      cmap_plat=face->charmaps[i]->platform_id;
      cmap_enc=face->charmaps[i]->encoding_id;
      if (cmap_plat == fnt->pid && cmap_enc == fnt->eid)
        break;
    }
    if (i == num_cmap)
    {
      fprintf(stderr, "%s: ERROR: Invalid platform and/or encoding ID.\n",
              progname);
      if (num_cmap == 1)
        fprintf(stderr, "  The only valid PID/EID pair is");
      else
        fprintf(stderr, "  Valid PID/EID pairs are:\n");
      for (i = 0; i < num_cmap; i++)
      {
        cmap_plat=face->charmaps[i]->platform_id;
        cmap_enc=face->charmaps[i]->encoding_id;
        fprintf(stderr, "    (%i,%i)\n", cmap_plat, cmap_enc);
      }
      fprintf(stderr, "\n");
      exit(1);
    }
  
    if ((error = FT_Set_Charmap(face, face->charmaps[i])))
      oops("Cannot load cmap (error code = 0x%x).", error);
  }

  if (fnt->PSnames)
  {
    if (!FT_HAS_GLYPH_NAMES(face))
      oops("This font does not support PS names.");
  }
  else if (cmap_plat == Microsoft_platform &&
           cmap_enc == Microsoft_Unicode_encoding)
    set_encoding_scheme(encUnicode, fnt);
  else if (cmap_plat == Macintosh_platform &&
           cmap_enc == Macintosh_encoding)
    set_encoding_scheme(encMac, fnt);
  else
    set_encoding_scheme(encFontSpecific, fnt);

  if (fnt->rotate)
    fetch_GSUB();
}

static inline int
find_ndx (int Num, Coverage_Item *coverage)
{
  int i;

  switch (coverage->format)
  {
    case 1:
      for (i = 0; i < coverage->count; i++)
      {
        FT_UShort glyph = coverage->data.glyphs[i];
        if (Num < glyph)
          break;
        else if (Num == glyph)
          return i;
      }
      break;
    case 2:
      for (i = 0; i < coverage->count; i++)
      {
        Range_Record *range = &coverage->data.ranges[i];

        if (Num < range->start)
          break;
        else if (Num <= range->end)
          return (Num - range->start) + range->start_Index;
      }
      break;
    default:
      oops("Internal error: Invalid Coverage Format=%d.", coverage->format);
  }
  return -1;
}

int
Get_Vert (int Num)
{
  Subst_Item *entry;

#ifdef  DEBUG
  warning("GSUB: Looking for vertical form of glyph %d.", Num);
#endif
  for (entry = Subst_list; entry; entry = entry->next)
  {
    Single_Item *item = entry->item;
    int ndx = find_ndx (Num, item->coverage);

    if (ndx >= 0)
    {
#ifdef  DEBUG
      warning("Covered at position %d.", ndx);
#endif
      switch (item->format)
      {
        case 1:
          Num += *item->data.delta;
          break;
        case 2:
          Num = item->data.array->glyphs[ndx];
          break;
        default:
          oops("Internal error: Invalid Single Format=%d.", item->format);
      }
#ifdef  DEBUG
      warning("Substituted by glyph %d.", Num);
#endif
    }
  }
  return Num;
}
