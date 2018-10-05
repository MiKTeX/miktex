/*
 *   ttf2tfm.h
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */

#ifndef TTF2TFM_H
#define TTF2TFM_H

#include <stdio.h>


enum Boolean_
{
  False = 0,
  True = 1
};
typedef enum Boolean_ Boolean;

enum PSstate_
{
  No = 0,
  Yes = 1,
  Only = 2
};
typedef enum PSstate_ PSstate;


struct _encoding
{
  const char *name;
  const char *vec[256];
};
typedef struct _encoding encoding;


/*
 *   This is what we store character data in.
 */

struct _ttfinfo;
typedef struct _ttfinfo ttfinfo;
struct _lig;
typedef struct _lig lig;
struct _kern;
typedef struct _kern kern;
struct _ttfptr;
typedef struct _ttfptr ttfptr;
struct _pcc;
typedef struct _pcc pcc;
struct _stringlist;
typedef struct _stringlist stringlist;
struct _sflist;
typedef struct _sflist sflist;


struct _ttfinfo
{
  ttfinfo *next;
  long charcode;                /* the TTF character code (or glyph index */
                                /* if bit 17 is set)                      */
  unsigned short glyphindex;    /* the TTF glyph number */
  short incode;                 /* the code position in the raw TeX font */
  short outcode;                /* the code position in the virtual font */
  const char *adobename;

  short width;
  short llx, lly, urx, ury;

  lig *ligs;
  kern *kerns;
  ttfptr *kern_equivs;
  Boolean constructed;
  pcc *pccs;                    /* we use the composite feature for */
                                /* `germandbls' <--> `SS' only      */
  unsigned char wptr, hptr, dptr, iptr;

  short fntnum;
};


struct _lig
{
  lig *next;
  const char *succ, *sub;
  short op, boundleft;
};


struct _kern
{
  kern *next;
  const char *succ;
  short delta;
};


struct _ttfptr
{
  ttfptr *next;
  ttfinfo *ch;
};


struct _pcc
{
  pcc *next;
  const char *partname;
  short xoffset, yoffset;
};


struct _stringlist
{
  stringlist* next;
  char *old_name;
  char *new_name;
  Boolean single_replacement;
};


struct _sflist
{
  char *name;
  long cksum;
};


struct _Font
{
  char *ttfname;

  /*
   *   Full path and extension of the tfm file
   */

  char *tfm_path;
  char *tfm_ext;

  /*
   *   The final tfm name is composed of the following three parts.
   */

  char *outname;            /* only namestem without extension */
  char *subfont_name;       /* NULL if not used */
  char *outname_postfix;    /* NULL if not used */
  char *fullname;           /* outname + subfont_name + outname_postfix */

  FILE *vplout;
  FILE *tfmout;

  /*
   *   The input encoding maps from the TrueType font to the raw TeX font.
   */

  char *inencname;                  /* name of input encoding file */
  encoding *inencoding;             /* the input encoding vector */
  ttfinfo *inencptrs[256];          /* the input mapping table.  Will be
                                       filled initially with the first
                                       256 characters in the selected
                                       cmap of the TrueType font */
  stringlist *replacements;         /* replacements for glyph names given
                                       with the -r option on the command
                                       line */
  char *replacementname;            /* name of replacement file */

  /*
   *   The output encoding maps from the raw TeX font to the virtual font.
   */

  char *outencname;                 /* name of output encoding file */
  encoding *outencoding;            /* the output encoding vector */
  ttfinfo *outencptrs[256];         /* the output mapping table */

  short nextout[256];               /* for characters encoded multiple times
                                       in output */

  Boolean sawligkern;               /* there were LIGKERN lines in the
                                       output encoding file */
  Boolean subfont_ligs;             /* ligatures 1st byte/2nd byte in
                                       subfonts wanted */
  char *ligname;                    /* ligatures 1st byte/2nd byte in
                                       subfonts with ligaturename */
  Boolean write_enc;                /* generate encoding vectors
                                       containing glyph indices wanted */

  ttfinfo *charlist;                /* a linked list of all valid chars */

  ttfinfo *uppercase[256];          /* needed for small caps fonts */
  ttfinfo *lowercase[256];          /* ditto */

  short boundarychar;               /* the boundary character */
  const char *codingscheme;         /* coding scheme for TeX */
  char *titlebuf;

  long cksum;

  short subfont_num;
  sflist *subfont_list;

  /*
   *   The name of the subfont definition file.
   */

  char *sfdname;
  long sf_code[256];

  /*
   *   We get the following three values from the TTF's postscript table.
   */

  short units_per_em;
  float italicangle;
  char fixedpitch;

  short xheight;                    /* xheight for TeX */
  short fontspace;                  /* font space for TeX */

  /*
   *   These values can be specified on the command line.
   */

  unsigned short pid;               /* the TTF platform ID */
  unsigned short eid;               /* the TTF encoding ID */
  float efactor;                    /* to extend the glyphs horizontally */
  float slant;                      /* to slant the font */
  unsigned long fontindex;          /* font number in TTC */
  float capheight;                  /* the height of small caps glyphs */
  PSstate PSnames;                  /* we use the PS names in the TTF */
  Boolean rotate;                   /* we rotate the glyphs by 90 degrees */
  float y_offset;                   /* y offset for rotated glyphs */

  /*
   *   The command line parameter strings needed for ttf2pk.
   */

  char *pidparam;
  char *eidparam;
  char *efactorparam;
  char *slantparam;
  char *fontindexparam;
  char *y_offsetparam;
};
typedef struct _Font Font;

#endif /* TEX2TFM_H */


/* end */
