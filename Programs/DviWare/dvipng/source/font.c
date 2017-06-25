/* font.c */

/************************************************************************

  Part of the dvipng distribution

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program. If not, see
  <http://www.gnu.org/licenses/>.

  Copyright (C) 2002-2015 Jan-Åke Larsson

************************************************************************/

#include "dvipng.h"

void CheckChecksum(uint32_t c1, uint32_t c2, const char* name)
{
  /* Report a warning if both checksums are nonzero, they don't match,
     and the user hasn't turned it off.  */
  if (c1 && c2 && c1 != c2
#ifdef HAVE_LIBKPATHSEA
      && !kpse_tex_hush ("checksum")
#endif
      ) {
     Warning ("checksum mismatch in %s", name) ;
   }
}


static double ActualFactor(uint32_t unmodsize)
/* compute the actual size factor given the approximation */
/* actually factor * 1000 */
{
  double  realsize;     /* the actual magnification factor */
  realsize = (double)unmodsize / 1000.0;
  if (abs((int)(unmodsize - 1095l))<2)
    realsize = 1.095445115; /*stephalf*/
  else if (abs((int)(unmodsize - 1315l))<2)
    realsize = 1.31453414; /*stepihalf*/
  else if (abs((int)(unmodsize - 1577l))<2)
    realsize = 1.57744097; /*stepiihalf*/
  else if (abs((int)(unmodsize - 1893l))<2)
    realsize = 1.89292916; /*stepiiihalf*/
  else if (abs((int)(unmodsize - 2074l))<2)
    realsize = 2.0736;   /*stepiv*/
  else if (abs((int)(unmodsize - 2488l))<2)
    realsize = 2.48832;  /*stepv*/
  else if (abs((int)(unmodsize - 2986l))<2)
    realsize = 2.985984; /*stepvi*/
  /* the remaining magnification steps are represented with sufficient
     accuracy already */
  return(realsize);
}


void FontDef(unsigned char* command, void* parent)
{
  int32_t k;
  uint32_t   c, s, d;
  uint8_t    a, l;
  unsigned char* current;
  struct font_entry *tfontptr; /* temporary font_entry pointer   */
  struct font_num *tfontnump = NULL;  /* temporary font_num pointer   */
  unsigned short i;

  current = command + 1;
  k = UNumRead(current, (int)*command - FNT_DEF1 + 1);
  current += (int)*command - FNT_DEF1 + 1;
  c = UNumRead(current, 4); /* checksum */
  s = UNumRead(current+4, 4); /* space size */
  d = UNumRead(current+8, 4); /* design size */
  a = UNumRead(current+12, 1); /* length for font name */
  l = UNumRead(current+13, 1); /* device length */
  if (((struct font_entry*)parent)->type==FONT_TYPE_VF) {
    DEBUG_PRINT(DEBUG_VF,(" %d %d %d",k,c,s));
    /* Rescale. s is relative to the actual scale /(1<<20) */
    s = (uint32_t)((uint64_t) s * (((struct font_entry*) parent)->s)
		   / (1<<20));
    DEBUG_PRINT(DEBUG_VF,(" (%d) %d",s,d));
    /* Oddly, d differs in the DVI and the VF that my system produces */
    d = (uint32_t)((uint64_t) d * ((struct font_entry*)parent)->d
		   / ((struct font_entry*)parent)->designsize);
    DEBUG_PRINT(DEBUG_VF,(" (%d)",d));
    DEBUG_PRINT(DEBUG_VF,(" %d %d '%.*s'",a,l,a+l,current+14));
#ifdef DEBUG
  } else {
    DEBUG_PRINT(DEBUG_DVI,(" %d %d %d %d %d %d '%.*s'",k,c,s,d,a,l,
		 a+l,current+14));
#endif
  }
  if (a+l > STRSIZE-1)
    Fatal("too long font name for font %ld",k);

  /* Find entry with this font number in use */
  switch (((struct font_entry*)parent)->type) {
  case FONT_TYPE_VF:
    tfontnump = ((struct font_entry*)parent)->vffontnump;
    break;
  case DVI_TYPE:
    tfontnump = ((struct dvi_data*)parent)->fontnump;
  }
  while (tfontnump != NULL && tfontnump->k != k) {
    tfontnump = tfontnump->next;
  }
  /* If found, return if it is correct */
  if (tfontnump!=NULL
      && tfontnump->fontp->s == s
      && tfontnump->fontp->d == d
      && strlen(tfontnump->fontp->n) == a+l
      && strncmp(tfontnump->fontp->n,(char*)current+14,a+l) == 0) {
    DEBUG_PRINT((DEBUG_DVI|DEBUG_VF),("\n  FONT %d:\tMatch found",k));
    return;
  }
  /* If not found, create new */
  if (tfontnump==NULL) {
    if ((tfontnump=malloc(sizeof(struct font_num)))==NULL)
      Fatal("cannot malloc memory for new font number");
    tfontnump->k=k;
    switch (((struct font_entry*)parent)->type) {
    case FONT_TYPE_VF:
      tfontnump->next=((struct font_entry*)parent)->vffontnump;
      ((struct font_entry*)parent)->vffontnump=tfontnump;
      break;
    case DVI_TYPE:
      tfontnump->next=((struct dvi_data*)parent)->fontnump;
      ((struct dvi_data*)parent)->fontnump=tfontnump;
    }
  }

  /* Search font list for possible match */
  tfontptr = hfontptr;
  while (tfontptr != NULL
	 && (tfontptr->s != s
	     || tfontptr->d != d
	     || strlen(tfontptr->n) != a+l
	     || strncmp(tfontptr->n,(char*)current+14,a+l) != 0 ) ) {
    tfontptr = tfontptr->next;
  }
  /* If found, set its number and return */
  if (tfontptr!=NULL) {
    DEBUG_PRINT((DEBUG_DVI|DEBUG_VF),("\n  FONT %d:\tMatch found, number set",k));
    tfontnump->fontp = tfontptr;
    return;
  }

  DEBUG_PRINT((DEBUG_DVI|DEBUG_VF),("\n  FONT %d:\tNew entry created",k));
  /* No fitting font found, create new entry. */
  if ((tfontptr = calloc(1,sizeof(struct font_entry))) == NULL)
    Fatal("cannot malloc space for font_entry");
  tfontptr->next = hfontptr;
  hfontptr = tfontptr;
  tfontnump->fontp = tfontptr;
#if !defined(WIN32) || !defined(ENABLE_WIN32_MMAP)
  tfontptr->fmmap.fd = 0;
#else  /* WIN32 */
  tfontptr->fmmap.hFile = INVALID_HANDLE_VALUE;
#endif
  tfontptr->c = c; /* checksum */
  tfontptr->s = s; /* space size */
  tfontptr->d = d; /* design size */
  tfontptr->a = a; /* length for font name */
  tfontptr->l = l; /* device length */
  strncpy(tfontptr->n,(char*)current+14,a+l); /* full font name */
  tfontptr->n[a+l] = '\0';

  tfontptr->name = NULL;
  for (i = FIRSTFNTCHAR; i <= LASTFNTCHAR; i++) {
    tfontptr->chr[i] = NULL;
  }

  tfontptr->dpi =
    (uint32_t)((ActualFactor((uint32_t)(1000.0*tfontptr->s
				  /(double)tfontptr->d+0.5))
	     * ActualFactor(dvi->mag) * dpi*shrinkfactor) + 0.5);
#ifdef HAVE_FT2
  tfontptr->psfontmap=NULL;
#endif
}

#ifdef HAVE_FT2
static char* kpse_find_t1_or_tt(char* filename)
{
    char* filepath = kpse_find_file(filename, kpse_type1_format, false);
    if ((option_flags & USE_FREETYPE) && filepath==NULL)
      filepath = kpse_find_file(filename, kpse_truetype_format, false);
    return(filepath);
}
#endif

static void FontFind(struct font_entry * tfontptr)
{
  kpse_glyph_file_type font_ret;

  /* tfontptr->dpi = kpse_magstep_fix (tfontptr->dpi, resolution, NULL); */
  DEBUG_PRINT(DEBUG_DVI,("\n  FIND FONT:\t%s %d",tfontptr->n,tfontptr->dpi));

  tfontptr->name = kpse_find_vf (tfontptr->n);
  if (tfontptr->name!=NULL)
    InitVF(tfontptr);
#ifdef HAVE_FT2
  else if (option_flags & USE_FREETYPE) {
    tfontptr->psfontmap = FindPSFontMap(tfontptr->n);
    if (tfontptr->psfontmap!=NULL)
      tfontptr->name=kpse_find_t1_or_tt(tfontptr->psfontmap->psfile);
    else
      tfontptr->name=kpse_find_t1_or_tt(tfontptr->n);
    if (tfontptr->name!=NULL) {
      char* tfmname=kpse_find_file(tfontptr->n, kpse_tfm_format, false);
      if (tfmname!=NULL) {
        if (!ReadTFM(tfontptr,tfmname)) {
          Warning("unable to read tfm file %s", tfmname);
          free(tfontptr->name);
          tfontptr->name=NULL;
        } else if ((option_flags & USE_FREETYPE)==0 || !InitFT(tfontptr)) {
          /* if Freetype loading fails for some reason, fall back to PK font */
          free(tfontptr->name);
          tfontptr->name=NULL;
        }
        free(tfmname);
      }
    }
  }
#endif /* HAVE_FT2 */
  if (tfontptr->name==NULL) {
    tfontptr->name=kpse_find_pk (tfontptr->n, tfontptr->dpi, &font_ret);
    if (tfontptr->name!=NULL) {
      if (!FILESTRCASEEQ (tfontptr->n, font_ret.name)) {
        page_flags |= PAGE_GAVE_WARN;
        Warning("font %s not found, using %s at %d dpi instead",
                tfontptr->n, font_ret.name, font_ret.dpi);
        tfontptr->c = 0; /* no checksum warning */
      } else if (!kpse_bitmap_tolerance ((double)font_ret.dpi,
           (double) tfontptr->dpi)) {
        page_flags |= PAGE_GAVE_WARN;
        Warning("font %s at %d dpi not found, using %d dpi instead",
                tfontptr->n, tfontptr->dpi, font_ret.dpi);
      }
      InitPK(tfontptr);
    } else {
      page_flags |= PAGE_GAVE_WARN;
      Warning("font %s at %d dpi not found, characters will be left blank",
        tfontptr->n, tfontptr->dpi);
#if !defined(WIN32) || !defined(ENABLE_WIN32_MMAP)
      tfontptr->fmmap.fd = 0;
#else  /* WIN32 */
      tfontptr->fmmap.hFile = INVALID_HANDLE_VALUE;
#endif
      tfontptr->magnification = 0;
      tfontptr->designsize = 0;
    }
  }
}


static void DoneFont(struct font_entry *tfontp)
{
  switch (tfontp->type) {
  case FONT_TYPE_PK:
    DonePK(tfontp);
    break;
  case FONT_TYPE_VF:
    DoneVF(tfontp);
    break;
#ifdef HAVE_FT2
  case FONT_TYPE_FT:
    DoneFT(tfontp);
    break;
#endif
  }
}


void FreeFontNumP(struct font_num *hfontnump)
{
  struct font_num *tmp;
  while(hfontnump!=NULL) {
    tmp=hfontnump->next;
    free(hfontnump);
    hfontnump=tmp;
  }
}

void ClearFonts(void)
{
  struct font_entry *tmp;

  while(hfontptr!=NULL) {
    tmp=hfontptr->next;
    DoneFont(hfontptr);
    if (hfontptr->name != NULL)
      free(hfontptr->name);
    free(hfontptr);
    hfontptr=tmp;
  }
  if (dvi!=NULL)
    FreeFontNumP(dvi->fontnump);
}

/*-->SetFntNum*/
/**********************************************************************/
/****************************  SetFntNum  *****************************/
/**********************************************************************/
void SetFntNum(int32_t k, void* parent /* dvi/vf */)
/*  this routine is used to specify the font to be used in printing future
    characters */
{
  struct font_num *tfontnump=NULL;  /* temporary font_num pointer   */

  switch (((struct font_entry*)parent)->type) {
  case FONT_TYPE_VF:
    tfontnump = ((struct font_entry*)parent)->vffontnump;
    break;
  case DVI_TYPE:
    tfontnump = ((struct dvi_data*)parent)->fontnump;
  }
  while (tfontnump != NULL && tfontnump->k != k)
    tfontnump = tfontnump->next;
  if (tfontnump == NULL)
    Fatal("font %d undefined", k);

  currentfont = tfontnump->fontp;
  if (currentfont->name==NULL)
    FontFind(currentfont);
}
