/* set.c */

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
#include <math.h>

#ifndef HAVE_GDIMAGECREATETRUECOLOR
#define gdImageColorAllocateAlpha(i,r,g,b,a) gdImageColorAllocate(i,r,g,b)
#define gdImageColorResolveAlpha(i,r,g,b,a)  gdImageColorResolve(i,r,g,b)
#define gdImageAlpha(i,c)                    0
#define gdAlphaMax                           127
#endif
#ifndef HAVE_GDIMAGEPNGEX
#define gdImagePngEx(i,f,z)                  gdImagePng(i,f)
#endif

/* Persistent color cache. Index is ink thickness,
   0=no ink, 127=total coverage */
static int ColorCache[gdAlphaMax+1];

void CreateImage(pixels x_width,pixels y_width)
{
  if (page_imagep)
    gdImageDestroy(page_imagep);
  if (x_width <= 0) x_width=1;
  if (y_width <= 0) y_width=1;
#ifdef HAVE_GDIMAGECREATETRUECOLOR
  /* GIFs are 256-color */
  if ((option_flags & FORCE_TRUECOLOR
      || page_flags & PAGE_TRUECOLOR)
      && ~option_flags & GIF_OUTPUT
      && ~option_flags & FORCE_PALETTE)
    page_imagep=gdImageCreateTrueColor(x_width,y_width);
  else
#endif
    page_imagep=gdImageCreate(x_width,y_width);
  /* Set bg color. GIFs cannot handle an alpha channel, resort to
     transparent color index, set in WriteImage */
  if (!page_imagep)
    Fatal("cannot allocate GD image for DVI");
  ColorCache[0]
    = gdImageColorAllocateAlpha(page_imagep,
				cstack[0].red,
				cstack[0].green,
				cstack[0].blue,
				(option_flags & BG_TRANSPARENT_ALPHA
				 && ~option_flags & GIF_OUTPUT) ? 127 : 0);
  ColorCache[gdAlphaMax]=-1;
#ifdef HAVE_GDIMAGECREATETRUECOLOR
  /* Alpha blending in libgd is only performed for truecolor images.
     We need it for palette images also. Turn libgd alpha blending off
     and calculate color blending where needed. We turn it back on
     briefly for image inclusion. */
  gdImageAlphaBlending(page_imagep, 0);
  if (option_flags & BG_TRANSPARENT_ALPHA)
    gdImageSaveAlpha(page_imagep, 1);
  if (page_imagep->trueColor)
    /* Truecolor: there is no background color index, fill image instead. */
    gdImageFilledRectangle(page_imagep, 0, 0,
			   x_width-1, y_width-1, ColorCache[0]);
#endif
}


static void ChangeColor(gdImagePtr imagep,int x1,int y1,
			int x2,int y2,int color1,int color2)
/* In the given rectangle, change color1 to color2 */
{
  int x,y;
  for( y=y1; y<=y2; y++) {
    for( x=x1; x<=x2; x++) {
      if (gdImageGetPixel(imagep, x, y)==color1)
	gdImageSetPixel(imagep, x, y, color2);
    }
  }
}

void WriteImage(char *pngname, int pagenum)
{
  char* pos, *freeme=NULL;
  FILE* outfp=NULL;

  /* Set transparent background. Maybe alpha is not available or
     perhaps we are producing GIFs, so test for BG_TRANSPARENT_ALPHA
     too */
  if (option_flags & (BG_TRANSPARENT|BG_TRANSPARENT_ALPHA))
    gdImageColorTransparent(page_imagep,ColorCache[0]);
  /* Transparent border */
  if (borderwidth>0) {
    int Transparent;
    pixels x_width,y_width;

    x_width=gdImageSX(page_imagep);
    y_width=gdImageSY(page_imagep);

    /* Set ANOTHER bg color, transparent this time */
    /* No semi-transparency here, given the motivation for this code
       * (box cursor visibility in Emacs) */
    if (userbordercolor)
      Transparent = gdImageColorAllocate(page_imagep,
					 bordercolor.red,
					 bordercolor.green,
					 bordercolor.blue);
    else
      Transparent = gdImageColorAllocate(page_imagep,
					 gdImageRed(page_imagep,ColorCache[0]),
					 gdImageGreen(page_imagep,ColorCache[0]),
					 gdImageBlue(page_imagep,ColorCache[0]));
    gdImageColorTransparent(page_imagep,Transparent);
    ChangeColor(page_imagep,0,0,x_width-1,borderwidth-1,
		ColorCache[0],Transparent);
    ChangeColor(page_imagep,0,0,borderwidth-1,y_width-1,
		ColorCache[0],Transparent);
    ChangeColor(page_imagep,x_width-borderwidth,0,x_width-1,y_width-1,
		ColorCache[0],Transparent);
    ChangeColor(page_imagep,0,y_width-borderwidth,x_width-1,y_width-1,
		ColorCache[0],Transparent);
  }

  if ((pos=strchr(pngname,'%')) != NULL) {
    if (strchr(++pos,'%'))
      Fatal("too many %%s in output file name");
    if (*pos == 'd'
	|| (*pos=='0' && pos[1]>='1' && pos[1]<='9' && pos[2]=='d')) {
      /* %d -> pagenumber, so add 9 string positions
	 since pagenumber max +-2^31 or +-2*10^9 */
      if ((freeme = malloc(strlen(pngname)+9))==NULL)
				Fatal("cannot allocate memory for output file name");
      sprintf(freeme,pngname,pagenum);
      pngname = freeme;
    } else {
      Fatal("unacceptible format spec in output file name");
    }
  }
#ifdef HAVE_GDIMAGEGIF
  if (option_flags & GIF_OUTPUT && (pos=strrchr(pngname,'.')) != NULL
      && strcmp(pos,".png")==0) {
    *(pos+1)='g';
    *(pos+2)='i';
    *(pos+3)='f';
  }
#endif
  if ((outfp = fopen(pngname,"wb")) == NULL)
      Fatal("cannot open output file %s",pngname);
#ifdef HAVE_GDIMAGEGIF
  if (option_flags & GIF_OUTPUT)
    gdImageGif(page_imagep,outfp);
  else
#endif
    gdImagePngEx(page_imagep,outfp,compression);
  fclose(outfp);
  DEBUG_PRINT(DEBUG_DVI,("\n  WROTE:   \t%s\n",pngname));
  if (freeme)
    free(freeme);
  DestroyImage();
}

void DestroyImage(void)
{
  gdImageDestroy(page_imagep);
  page_imagep=NULL;
}

static int gammatable[]=
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,
   20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,
   40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,
   60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
   80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,
   100,101,102,103,104,105,106,107,108,109,
   110,111,112,113,114,115,116,117,118,119,
   120,121,122,123,124,125,126,127};

void Gamma(double gamma)
{
  int i=0;

  while (i<=gdAlphaMax) {
    gammatable[i]=gdAlphaMax-
      (int)(pow((gdAlphaMax-i)/((double)gdAlphaMax),gamma)*gdAlphaMax);
    DEBUG_PRINT(DEBUG_GLYPH,
		("\n  GAMMA GREYSCALE: %d -> %d ",i,gammatable[i]));
    i++;
  }
}

dviunits SetGlyph(struct char_entry *ptr, int32_t hh,int32_t vv)
/* gdImageChar can only do monochrome glyphs */
{
  int dst_alpha,dst_weight,tot_weight,alpha;
  int x,y,pos=0;
  int bgColor,pixelgrey,pixelcolor;

  hh -= ptr->xOffset/shrinkfactor;
  vv -= ptr->yOffset/shrinkfactor;
  /* Initialize persistent color cache. Perhaps this should be in
     color.c? */
  pixelcolor=gdImageColorResolve(page_imagep,
				 cstack[csp].red,
				 cstack[csp].green,
				 cstack[csp].blue);
  if (ColorCache[gdAlphaMax]!=pixelcolor) {
    for( x=1; x<gdAlphaMax; x++ )
      ColorCache[x]=-1;
    ColorCache[gdAlphaMax]=pixelcolor;
  }
  for( y=0; y<ptr->h; y++) {
    for( x=0; x<ptr->w; x++) {
      if (ptr->data[pos]>0) {
	pixelgrey=gammatable[(int)ptr->data[pos]/2];
	bgColor = gdImageGetPixel(page_imagep, hh + x, vv + y);
	if (ColorCache[0]!=bgColor || ColorCache[pixelgrey]==-1) {
	  DEBUG_PRINT(DEBUG_GLYPH,("\n  GAMMA GREYSCALE: %d -> %d ",
				   ptr->data[pos]/2,pixelgrey));
	  alpha = gdAlphaMax-pixelgrey;
	  dst_alpha = gdImageAlpha(page_imagep,bgColor);
	  dst_weight = (gdAlphaMax - dst_alpha) * alpha / gdAlphaMax;
	  tot_weight = pixelgrey + dst_weight;
	  pixelcolor = gdImageColorResolveAlpha(page_imagep,
	     (cstack[csp].red*pixelgrey
	      + gdImageRed(page_imagep,bgColor)*dst_weight)/tot_weight,
	     (cstack[csp].green*pixelgrey
	      + gdImageGreen(page_imagep,bgColor)*dst_weight)/tot_weight,
	     (cstack[csp].blue*pixelgrey
	      + gdImageBlue(page_imagep,bgColor)*dst_weight)/tot_weight,
	     alpha*dst_alpha/gdAlphaMax);
	  if (ColorCache[0]==bgColor)
	    ColorCache[pixelgrey]=pixelcolor;
	} else
	  pixelcolor=ColorCache[pixelgrey];
	gdImageSetPixel(page_imagep, hh + x, vv + y, pixelcolor);
      }
      pos++;
    }
  }
  return(ptr->tfmw);
}

dviunits SetRule(dviunits a, dviunits b, subpixels hh,subpixels vv)
{
  /*                               This routine will draw a \rule */
  int Color;
  pixels    width=0, height=0;

  if ( a > 0 && b > 0 ) {
    /* Calculate width and height, round up */
    width = (b+dvi->conv*shrinkfactor-1)/dvi->conv/shrinkfactor;
    height = (a+dvi->conv*shrinkfactor-1)/dvi->conv/shrinkfactor;
  }
  if (page_imagep != NULL) {
    if ((height>0) && (width>0)) {
      /* This code produces too dark rules. But what the hell. Grey
       * rules look fuzzy. */
      Color = gdImageColorResolve(page_imagep,
				  cstack[csp].red,
				  cstack[csp].green,
				  cstack[csp].blue);
      /* +1 and -1 are because the Rectangle coords include last pixels */
      gdImageFilledRectangle(page_imagep,hh,vv-height+1,hh+width-1,vv,Color);
      DEBUG_PRINT(DEBUG_DVI,("\n  RULE \t%dx%d at (%d,%d)",
		   width, height, hh, vv));
    }
  } else {
    /* The +1's are because things are cut _at_that_coordinate_. */
    min(x_min,hh);
    min(y_min,vv-height+1);
    max(x_max,hh+width);
    max(y_max,vv+1);
  }
  return(b);
}
