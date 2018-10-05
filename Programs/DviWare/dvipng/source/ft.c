/* ft.c */

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

  Copyright (C) 2002-2009 Jan-Åke Larsson

************************************************************************/

#include "dvipng.h"

void LoadFT(int32_t c, struct char_entry * ptr)
{
  FT_Bitmap  bitmap;
  FT_UInt    glyph_i;
  int i,j,k;
  unsigned char* bit;
  static bool hintwarning=false;

  DEBUG_PRINT(DEBUG_FT,("\n  LOAD FT CHAR\t%d (%d)",c,ptr->tfmw));
  if (currentfont->psfontmap!=NULL
      && currentfont->psfontmap->encoding != NULL) {
    DEBUG_PRINT(DEBUG_FT,(" %s",currentfont->psfontmap->encoding->charname[c]));
    glyph_i = FT_Get_Name_Index(currentfont->face,
				currentfont->psfontmap->encoding->charname[c]);
  } else if (currentfont->psfontmap!=NULL
	     && currentfont->psfontmap->subfont != NULL) {
    glyph_i = FT_Get_Char_Index( currentfont->face, 
				 currentfont->psfontmap->subfont->charindex[c] );
    DEBUG_PRINT(DEBUG_FT,(" 0x%X",currentfont->psfontmap->subfont->charindex[c]));
  } else
    glyph_i = FT_Get_Char_Index( currentfont->face, c );
  if (FT_Load_Glyph( currentfont->face, glyph_i,
		     FT_LOAD_RENDER | FT_LOAD_TARGET_LIGHT )) {
    /* On some configurations (with FreeType <= 2.1.7) the above
       fails, while the below works */
    if (!hintwarning) {
      hintwarning=true;
      Warning("the used FreeType does not have target_light hinting");
    }
    if (FT_Load_Glyph( currentfont->face, glyph_i,
		       FT_LOAD_RENDER | FT_LOAD_NO_HINTING ))
      Fatal("cannot load FT char %d",c);
  }
  ptr->xOffset = -currentfont->face->glyph->bitmap_left*shrinkfactor;
  ptr->yOffset = (currentfont->face->glyph->bitmap_top-1)*shrinkfactor;
  bitmap=currentfont->face->glyph->bitmap;
  DEBUG_PRINT(DEBUG_FT,(" (%dx%d)",bitmap.width,bitmap.rows));
    
  if ((ptr->data = calloc(bitmap.width*bitmap.rows,sizeof(char))) == NULL)
    Fatal("unable to malloc image space for char %c", (char)c);
  ptr->w = bitmap.width;
  ptr->h = bitmap.rows;

#define GREYLEVELS 16
  DEBUG_PRINT(DEBUG_GLYPH,("\nDRAW GLYPH %d\n", (int)c));
  bit=ptr->data;
  for(i=0;i<bitmap.rows;i++) {
    for(j=0;j<bitmap.width;j++) {
      k=bitmap.buffer[i*bitmap.pitch+j]/(256/GREYLEVELS)*17;
      /* k=(bitmap.buffer[i*bitmap.pitch+j]+1)/16; */
      /* k= k>0 ? k*16-1 : 0; */
      DEBUG_PRINT(DEBUG_GLYPH,("%3u ",k));
      bit[i*bitmap.width+j]=k;
    }
    DEBUG_PRINT(DEBUG_GLYPH,("|\n"));
  }
}

bool InitFT(struct font_entry * tfontp)
{
  int error;

  if (libfreetype==NULL) {
    if (FT_Init_FreeType( &libfreetype )) {
      Warning("an error occured during freetype initialisation, disabling it"); 
      option_flags &= ~USE_FREETYPE;
      return(false);
    } 
# ifdef DEBUG
    else {
      FT_Int      amajor, aminor, apatch;

      DEBUG_PRINT(DEBUG_FT,("\n  COMPILED WITH FREETYPE %d.%d.%d",
		 FREETYPE_MAJOR, FREETYPE_MINOR, FREETYPE_PATCH));
#  ifdef HAVE_FT_LIBRARY_VERSION	    
      FT_Library_Version( libfreetype, &amajor, &aminor, &apatch );
      DEBUG_PRINT(DEBUG_FT,("\n  USING LIBFT %d.%d.%d",
			    amajor, aminor, apatch));
#  endif
    }
# endif
  }
  DEBUG_PRINT((DEBUG_DVI|DEBUG_FT),("\n  OPEN FT FONT:\t'%s'", tfontp->name));
  error = FT_New_Face( libfreetype, tfontp->name, 0, &tfontp->face );
  if (error == FT_Err_Unknown_File_Format) {
    Warning("font file %s has unknown format", tfontp->name);
    return(false);
  } else if (error) { 
    Warning("font file %s could not be opened", tfontp->name);
    return(false);
  } 
  Message(BE_VERBOSE,"<%s>", tfontp->name);
  if (tfontp->psfontmap != NULL && tfontp->psfontmap->subfont != NULL)
    error=FT_Select_Charmap(tfontp->face, tfontp->psfontmap->subfont->encoding);
  else if (tfontp->psfontmap == NULL || tfontp->psfontmap->encoding == NULL)
#ifndef FT_ENCODING_ADOBE_CUSTOM
# define FT_ENCODING_ADOBE_CUSTOM ft_encoding_adobe_custom
# define FT_ENCODING_ADOBE_STANDARD ft_encoding_adobe_standard
#endif
    error=FT_Select_Charmap(tfontp->face, FT_ENCODING_ADOBE_CUSTOM);
  if (error) {
    Warning("unable to set font encoding for %s", tfontp->name);
    if(FT_Select_Charmap( tfontp->face, FT_ENCODING_ADOBE_STANDARD )) {
      Warning("unable to set fallback font encoding for %s", tfontp->name);
      return(false);
    }
  }
  if (FT_Set_Char_Size( tfontp->face, /* handle to face object           */
			0,            /* char_width in 1/64th of points  */
			((int64_t)tfontp->d*64*7200)/7227/65536,
			/* char_height in 1/64th of _big_points, 
			   not TeX points */
			tfontp->dpi/shrinkfactor,   /* horizontal resolution */
			tfontp->dpi/shrinkfactor )) /* vertical resolution   */ {
    Warning("unable to set font size for %s", tfontp->name);
    return(false);
  }
  if (tfontp->psfontmap!=NULL)
    FT_Set_Transform(tfontp->face, tfontp->psfontmap->ft_transformp, NULL);
  tfontp->type = FONT_TYPE_FT;
  return(true);
}


static void UnLoadFT(struct char_entry *ptr)
{
  if (ptr->data!=NULL)
    free(ptr->data);
  ptr->data=NULL;
}


void DoneFT(struct font_entry *tfontp)
{
  int c=0;

  int error = FT_Done_Face( tfontp->face );
  if (error)
    Warning("font file %s could not be closed", tfontp->name);
  while(c<NFNTCHARS) {
    if (tfontp->chr[c]!=NULL) {
      UnLoadFT((struct char_entry*)tfontp->chr[c]);
      free(tfontp->chr[c]);
      tfontp->chr[c]=NULL;
    }
    c++;
  }
  if (tfontp->name!=NULL)
    free(tfontp->name);
  tfontp->name=NULL;
}


