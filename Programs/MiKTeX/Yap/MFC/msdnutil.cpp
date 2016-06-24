/* © 1998 Microsoft Corporation. All rights reserved. */

#include "StdAfx.h"

using namespace std;

#if defined(max)
#  undef max
#  undef min
#endif

// Q79212

void
DrawTransparentBitmap (HDC      hdc,
                       HBITMAP  hBitmap,
                       short    xStart,
                       short    yStart,
                       COLORREF cTransparentColor)
{
   BITMAP     bm;
   COLORREF   cColor;
   HBITMAP    bmAndBack, bmAndObject, bmAndMem, bmSave;
   HBITMAP    bmBackOld, bmObjectOld, bmMemOld, bmSaveOld;
   HDC        hdcMem, hdcBack, hdcObject, hdcTemp, hdcSave;
   POINT      ptSize;

   hdcTemp = CreateCompatibleDC(hdc);
   SelectObject(hdcTemp, hBitmap);   // Select the bitmap

   GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);
   ptSize.x = bm.bmWidth;            // Get width of bitmap
   ptSize.y = bm.bmHeight;           // Get height of bitmap
   DPtoLP(hdcTemp, &ptSize, 1);      // Convert from device
                                     // to logical points

   // Create some DCs to hold temporary data.
   hdcBack   = CreateCompatibleDC(hdc);
   hdcObject = CreateCompatibleDC(hdc);
   hdcMem    = CreateCompatibleDC(hdc);
   hdcSave   = CreateCompatibleDC(hdc);

   // Create a bitmap for each DC. DCs are required for a number of
   // GDI functions.

   // Monochrome DC
   bmAndBack   = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

   // Monochrome DC
   bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

   bmAndMem    = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
   bmSave      = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);

   // Each DC must select a bitmap object to store pixel data.
   bmBackOld   = (HBITMAP) SelectObject(hdcBack, bmAndBack);
   bmObjectOld = (HBITMAP) SelectObject(hdcObject, bmAndObject);
   bmMemOld    = (HBITMAP) SelectObject(hdcMem, bmAndMem);
   bmSaveOld   = (HBITMAP) SelectObject(hdcSave, bmSave);

   // Set proper mapping mode.
   SetMapMode(hdcTemp, GetMapMode(hdc));

   // Save the bitmap sent here, because it will be overwritten.
   BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

   // Set the background color of the source DC to the color.
   // contained in the parts of the bitmap that should be transparent
   cColor = SetBkColor(hdcTemp, cTransparentColor);

   // Create the object mask for the bitmap by performing a BitBlt
   // from the source bitmap to a monochrome bitmap.
   BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0,
          SRCCOPY);

   // Set the background color of the source DC back to the original
   // color.
   SetBkColor(hdcTemp, cColor);

   // Create the inverse of the object mask.
   BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0,
          NOTSRCCOPY);

   // Copy the background of the main DC to the destination.
   BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, xStart, yStart,
          SRCCOPY);

   // Mask out the places where the bitmap will be placed.
   BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);

   // Mask out the transparent colored pixels on the bitmap.
   BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);

   // XOR the bitmap with the background on the destination DC.
   BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT);

   // Copy the destination to the screen.
   BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y, hdcMem, 0, 0,
          SRCCOPY);

   // Place the original bitmap back into the bitmap sent here.
   BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY);

   // Delete the memory bitmaps.
   DeleteObject(SelectObject(hdcBack, bmBackOld));
   DeleteObject(SelectObject(hdcObject, bmObjectOld));
   DeleteObject(SelectObject(hdcMem, bmMemOld));
   DeleteObject(SelectObject(hdcSave, bmSaveOld));

   // Delete the memory DCs.
   DeleteDC(hdcMem);
   DeleteDC(hdcBack);
   DeleteDC(hdcObject);
   DeleteDC(hdcSave);
   DeleteDC(hdcTemp);
}

// Q29240

/* RGBtoHLS() takes a DWORD RGB value, translates it to HLS, and
   stores the results in the global vars H, L, and S. HLStoRGB takes
   the current values of H, L, and S and returns the equivalent value
   in an RGB DWORD. The vars H, L, and S are only written to by:

   1. RGBtoHLS (initialization)
   2. The scroll bar handlers

   A point of reference for the algorithms is Foley and Van Dam,
   "Fundamentals of Interactive Computer Graphics," Pages
   618-19. Their algorithm is in floating point. CHART implements a
   less general (hardwired ranges) integral algorithm.  There are
   potential round-off errors throughout this sample. ((0.5 + x)/y)
   without floating point is phrased ((x + (y/2))/y), yielding a very
   small round-off error. This makes many of the following divisions
   look strange.  */

#define RANGE 240

#define  HLSMAX   RANGE /* H,L, and S vary over 0-HLSMAX */
#define  RGBMAX   255   /* R,G, and B vary over 0-RGBMAX */
                        /* HLSMAX BEST IF DIVISIBLE BY 6 */
                        /* RGBMAX, HLSMAX must each fit in a byte. */

/* Hue is undefined if Saturation is 0 (grey-scale) */
/* This value determines where the Hue scrollbar is */
/* initially set for achromatic colors */
#define UNDEFINED (HLSMAX*2/3)

void
RGBtoHLS (DWORD         lRGBColor,
          WORD &        _H,
          WORD &        _L,
          WORD &        _S)
{
  SHORT H;
  SHORT L;
  SHORT S;

  WORD R, G, B;                 // input RGB values
  BYTE cMax, cMin;              // max and min RGB values
  WORD  Rdelta, Gdelta, Bdelta; // intermediate value: % of spread from max

  /* get R, G, and B out of DWORD */
  R = GetRValue(lRGBColor);
  G = GetGValue(lRGBColor);
  B = GetBValue(lRGBColor);
  
  /* calculate lightness */
  cMax = static_cast<BYTE>(std::max( std::max(R,G), B));
  cMin = static_cast<BYTE>(std::min(  std::min(R,G), B));
  L = ( ((cMax+cMin)*HLSMAX) + RGBMAX )/(2*RGBMAX);

  if (cMax == cMin)
    {           /* r=g=b --> achromatic case */
      S = 0;                     /* saturation */
      H = UNDEFINED;             /* hue */
    }
  else
    {                        /* chromatic case */
      /* saturation */
      if (L <= (HLSMAX/2))
        S = ( ((cMax-cMin)*HLSMAX) + ((cMax+cMin)/2) ) / (cMax+cMin);
      else
        S = (( ((cMax-cMin)*HLSMAX) + ((2*RGBMAX-cMax-cMin)/2) )
             / (2*RGBMAX-cMax-cMin));
      
      /* hue */
      Rdelta = ( ((cMax-R)*(HLSMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin);
      Gdelta = ( ((cMax-G)*(HLSMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin);
      Bdelta = ( ((cMax-B)*(HLSMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin);
      
      if (R == cMax)
        H = Bdelta - Gdelta;
      else if (G == cMax)
        H = (HLSMAX/3) + Rdelta - Bdelta;
      else /* B == cMax */
        H = ((2*HLSMAX)/3) + Gdelta - Rdelta;
      
      if (H < 0)
        H += HLSMAX;
      if (H > HLSMAX)
        H -= HLSMAX;
    }

  _H = H;
  _L = L;
  _S = S;
}

/* utility routine for HLStoRGB */
WORD
HueToRGB (WORD n1,
          WORD n2,
          WORD hue)
{
  /* range check: note values passed add/subtract thirds of range */
  if (hue < 0)
    hue += HLSMAX;

  if (hue > HLSMAX)
    hue -= HLSMAX;
  
  /* return r,g, or b value from this tridrant */
  if (hue < (HLSMAX/6))
    return ( n1 + (((n2-n1)*hue+(HLSMAX/12))/(HLSMAX/6)) );
  if (hue < (HLSMAX/2))
    return ( n2 );
  if (hue < ((HLSMAX*2)/3))
    return ( n1 + (((n2-n1)*(((HLSMAX*2)/3)-hue)+(HLSMAX/12))/(HLSMAX/6)) ); 
  else
    return ( n1 );
} 

DWORD
HLStoRGB (WORD hue,
          WORD lum,
          WORD sat)
{
  WORD R, G, B;                 // RGB component values
  WORD Magic1, Magic2;          // calculated magic numbers (really!)

  if (sat == 0)
    {                           /* achromatic case */
      R=G=B=(lum*RGBMAX)/HLSMAX;
      if (hue != UNDEFINED)
        {
          /* ERROR */
        }
    }
   else
     {                          /* chromatic case */
       /* set up magic numbers */
       if (lum <= (HLSMAX/2))
         Magic2 = (lum*(HLSMAX + sat) + (HLSMAX/2))/HLSMAX;
       else
         Magic2 = lum + sat - ((lum*sat) + (HLSMAX/2))/HLSMAX;
       Magic1 = 2*lum-Magic2;

       /* get RGB, change units from HLSMAX to RGBMAX */
       R = (HueToRGB(Magic1,Magic2,hue+(HLSMAX/3))*RGBMAX + (HLSMAX/2))/HLSMAX; 
       G = (HueToRGB(Magic1,Magic2,hue)*RGBMAX + (HLSMAX/2)) / HLSMAX;
       B = (HueToRGB(Magic1,Magic2,hue-(HLSMAX/3))*RGBMAX + (HLSMAX/2))/HLSMAX;
     }
  return (RGB(R,G,B));
} 
