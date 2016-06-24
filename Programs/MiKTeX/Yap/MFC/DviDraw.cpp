/* DviDraw.cpp:

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of Yap.

   Yap is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Yap is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Yap; if not, write to the Free Software Foundation, 59
   Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include "StdAfx.h"

#include "yap.h"

#include "DviDoc.h"
#include "DviDraw.h"

DviDraw::DviDraw() :
  gamma(g_pYapConfig->gamma)
{
  bitmapInfoTable[DVIVIEW_DISPLAY] = 0;
  bitmapInfoTable[DVIVIEW_PRINTER] = 0;

  try
  {
    // initialize display glyph BITMAPINFO
    void * p = malloc(sizeof(BITMAPINFO) + 16 * (sizeof(WORD)));
    if (p == nullptr)
    {
      OUT_OF_MEMORY("malloc");
    }
    LPBITMAPINFO pBitmapInfo = reinterpret_cast<LPBITMAPINFO>(p);
    bitmapInfoTable[DVIVIEW_DISPLAY] = pBitmapInfo;
    pBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pBitmapInfo->bmiHeader.biPlanes = 1;
    pBitmapInfo->bmiHeader.biBitCount = 4;
    pBitmapInfo->bmiHeader.biCompression = BI_RGB;
    pBitmapInfo->bmiHeader.biClrUsed = 0;
    pBitmapInfo->bmiHeader.biClrImportant = 0;
    LPWORD pCol = reinterpret_cast<LPWORD>(pBitmapInfo->bmiColors);
    for (WORD i = 0; i < 16; ++i)
    {
      pCol[i] = i;
    }

    // initialize printer glyph BITMAPINFO
    p = malloc(sizeof(BITMAPINFO) + 2 * (sizeof(DWORD)));
    if (p == nullptr)
    {
      OUT_OF_MEMORY("malloc");
    }
    pBitmapInfo = reinterpret_cast<LPBITMAPINFO>(p);
    bitmapInfoTable[DVIVIEW_PRINTER] = pBitmapInfo;
    pBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pBitmapInfo->bmiHeader.biPlanes = 1;
    pBitmapInfo->bmiHeader.biBitCount = 1;
    pBitmapInfo->bmiHeader.biCompression = BI_RGB;
    pBitmapInfo->bmiHeader.biClrUsed = 0;
    pBitmapInfo->bmiHeader.biClrImportant = 0;

    InitializeDviBitmapPalettes();
  }
  catch (const exception &)
  {
    if (bitmapInfoTable[DVIVIEW_DISPLAY] != nullptr)
    {
      free(bitmapInfoTable[DVIVIEW_DISPLAY]);
    }
    if (bitmapInfoTable[DVIVIEW_PRINTER] != nullptr)
    {
      free(bitmapInfoTable[DVIVIEW_PRINTER]);
    }
    throw;
  }
}

DviDraw::~DviDraw()
{
  try
  {
    MIKTEX_ASSERT(bitmapInfoTable[DVIVIEW_DISPLAY] != nullptr);
    free(bitmapInfoTable[DVIVIEW_DISPLAY]);
    bitmapInfoTable[DVIVIEW_DISPLAY] = nullptr;
    MIKTEX_ASSERT(bitmapInfoTable[DVIVIEW_PRINTER]);
    free(bitmapInfoTable[DVIVIEW_PRINTER]);
    bitmapInfoTable[DVIVIEW_PRINTER] = nullptr;
    map<foreback, HPALETTE>::iterator it;
    for (it = foregroundPalettes.begin(); it != foregroundPalettes.end(); ++it)
    {
      if (!DeleteObject((*it).second))
      {
        TraceError(T_("DeleteObject() failed for some reason"));
      }
    }
  }
  catch (const exception &)
  {
  }
}

#define USE_BITBLT 1
#define USE_STRETCHDIBITS 0

void DviDraw::DrawDviBitmaps(CDC * pDC, DviDoc * pDoc, DviPage * pPage)
{
  ASSERT_VALID(pDC);
  ASSERT_VALID(pDoc);
  MIKTEX_ASSERT(USE_BITBLT + USE_STRETCHDIBITS == 1);
  MIKTEX_ASSERT(pPage != nullptr);

#if USE_STRETCHDIBITS
  if ((pDC->GetDeviceCaps(RASTERCAPS) & RC_STRETCHDIB) == 0)
  {
    FATAL_MIKTEX_ERROR("DviDraw::DrawDviBitmaps", T_("Device does not support StretchDIBits()."), 0);
  }
#endif

#if USE_BITBLT
  // create memory device context
  HDC hdcMem = CreateCompatibleDC(pDC->GetSafeHdc());
  if (hdcMem == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
#endif

  foreback fb;
  fb.fore = RGB(0, 0, 0);
  fb.back = RGB(255, 255, 255);
  fb.numcolors = (pDoc->GetShrinkFactor() == 1 ? 2 : 16);

  if (pDoc->GetShrinkFactor() > 1)
  {
    // select standard b/w palette
    HPALETTE hPalOld = SelectPalette(pDC->GetSafeHdc(), foregroundPalettes[fb], TRUE);
    if (hPalOld == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    if (pDC->RealizePalette() == GDI_ERROR)
    {
      MIKTEX_UNEXPECTED();
    }
  }

  // process all bitmaps
  size_t nBitmaps = pPage->GetNumberOfDviBitmaps(pDoc->GetShrinkFactor());
  for (size_t idx = 0; idx < nBitmaps; ++idx)
  {
    const DviBitmap & dvibm = pPage->GetDviBitmap(pDoc->GetShrinkFactor(), static_cast<int>(idx));

    CRect rectBitmap(dvibm.x, dvibm.y, dvibm.x + dvibm.width - 1, dvibm.y + dvibm.height - 1);

    // check to see whether the bitmap is visible
    if (!pDoc->IsPrintContext() && !pDC->RectVisible(rectBitmap))
    {
      continue;
    }

    // select palette for current bitmap
    if (pDoc->GetShrinkFactor() > 1 && (dvibm.foregroundColor != fb.fore || dvibm.backgroundColor != fb.back))
    {
      fb.fore = dvibm.foregroundColor;
      fb.back = dvibm.backgroundColor;
      HPALETTE hPal;
      if (foregroundPalettes.find(fb) == foregroundPalettes.end())
      {
        hPal = CreateDviBitmapPalette(fb.fore, fb.back, pDoc->GetShrinkFactor() == 1 ? 2 : 16);
        foregroundPalettes[fb] = hPal;
      }
      else
      {
        hPal = foregroundPalettes[fb];
      }
      HPALETTE hPalOld = SelectPalette(pDC->GetSafeHdc(), hPal, TRUE);
      if (hPalOld == nullptr)
      {
        MIKTEX_UNEXPECTED();
      }
      if (pDC->RealizePalette() == GDI_ERROR)
      {
        MIKTEX_UNEXPECTED();
      }
    }

#if defined(MIKTEX_DEBUG)
    if (!pDoc->IsPrintContext() && g_pYapConfig->showBoundingBoxes)
    {
      pDC->Rectangle(rectBitmap);
    }
#endif

    BITMAPINFO * pBitmapInfo;

    // make the bitmap info
    pBitmapInfo =
      MakeBitmapInfo(dvibm.width, dvibm.height, (pDoc->IsPrintContext()
        ? pDoc->GetPrinterResolution()
        : (pDoc->GetDisplayResolution()
          / pDoc->GetDisplayShrinkFactor())), dvibm.bytesPerLine, pDoc);

    // set printer colors
    if (pDoc->GetShrinkFactor() == 1)
    {
      pBitmapInfo->bmiColors[0].rgbRed = GetRValue(fb.back);
      pBitmapInfo->bmiColors[0].rgbGreen = GetGValue(fb.back);
      pBitmapInfo->bmiColors[0].rgbBlue = GetBValue(fb.back);
      pBitmapInfo->bmiColors[0].rgbReserved = 0;
      pBitmapInfo->bmiColors[1].rgbRed = GetRValue(fb.fore);
      pBitmapInfo->bmiColors[1].rgbGreen = GetGValue(fb.fore);
      pBitmapInfo->bmiColors[1].rgbBlue = GetBValue(fb.fore);
      pBitmapInfo->bmiColors[1].rgbReserved = 0;
    }

#if USE_STRETCHDIBITS
    int n =
      StretchDIBits(pDC->GetSafeHdc(), dvibm.x, dvibm.y, dvibm.width, dvibm.height, 0, 0, dvibm.width, dvibm.height, reinterpret_cast<const void*>(dvibm.pPixels), pBitmapInfo, (pDoc->GetShrinkFactor() == 1
        ? DIB_RGB_COLORS
        : DIB_PAL_COLORS), SRCCOPY);
    if (n != dvibm.height)
    {
      if (n == GDI_ERROR)
      {
        FATAL_WINDOWS_ERROR(T_("StretchDIBits"), 0);
      }
      else
      {
        MIKTEX_UNEXPECTED();
      }
    }
#endif

#if USE_BITBLT
    HBITMAP hBitmap = CreateDIBitmap(pDC->GetSafeHdc(), &pBitmapInfo->bmiHeader, CBM_INIT, reinterpret_cast<const void*>(dvibm.pPixels), pBitmapInfo, pDoc->GetShrinkFactor() == 1 ? DIB_RGB_COLORS : DIB_PAL_COLORS);
    if (hBitmap == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(hdcMem, hBitmap));
    if (hOldBitmap == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    SetMapMode(hdcMem, GetMapMode(pDC->GetSafeHdc()));
    if (!BitBlt(pDC->GetSafeHdc(), dvibm.x, dvibm.y, dvibm.width, dvibm.height, hdcMem, 0, 0, SRCCOPY))
    {
      MIKTEX_UNEXPECTED();
    }
    // FIXME: use AutoResource
    if (!SelectObject(hdcMem, hOldBitmap))
    {
      TraceError(T_("SelectObject() failed for some reason"));
    }
    if (!DeleteObject(hBitmap))
    {
      TraceError(T_("DeleteObject() failed for some reason"));
    }
#endif
  }

#if USE_BITBLT
  // FIXME: use AutoResource
  if (!DeleteDC(hdcMem))
  {
    TraceError(T_("DeleteDC() failed for some reason"));
  }
#endif
}

void DviDraw::DrawDibChunks(CDC * pDC, DviDoc * pDoc, DviPage * pPage)
{
  ASSERT_VALID(pDC);
  ASSERT_VALID(pDoc);
  MIKTEX_ASSERT(USE_BITBLT + USE_STRETCHDIBITS == 1);
  MIKTEX_ASSERT(pPage != nullptr);

#if USE_STRETCHDIBITS
  if ((pDC->GetDeviceCaps(RASTERCAPS) & RC_STRETCHDIB) == 0)
  {
    FATAL_MIKTEX_ERROR("DviDraw::DrawDviBitmaps", T_("Device does not support StretchDIBits()."), 0);
  }
#endif

#if USE_BITBLT
  // create memory device context
  HDC hdcMem = CreateCompatibleDC(pDC->GetSafeHdc());
  if (hdcMem == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
#endif

  // process all DIB chunks
  size_t nChunks = pPage->GetNumberOfDibChunks(pDoc->GetShrinkFactor());
  for (size_t idx = 0; idx < nChunks; ++idx)
  {
    const DibChunk & chunk = *pPage->GetDibChunk(pDoc->GetShrinkFactor(), static_cast<int>(idx));

    const BITMAPINFO * pBitmapInfo = chunk.GetBitmapInfo();

    int x = chunk.GetX();
    int y = chunk.GetY();
    int x2 = x + pBitmapInfo->bmiHeader.biWidth - 1;
    int y2 = y + pBitmapInfo->bmiHeader.biHeight - 1;

    CRect rectBitmap(x, y, x2, y2);

    // check to see whether the bitmap is visible
    if (!pDoc->IsPrintContext() && !pDC->RectVisible(rectBitmap))
    {
      continue;
    }

#if defined(MIKTEX_DEBUG)
    if (!pDoc->IsPrintContext() && g_pYapConfig->showBoundingBoxes)
    {
      pDC->Rectangle(rectBitmap);
    }
#endif

#if USE_STRETCHDIBITS
    int n = StretchDIBits(pDC->GetSafeHdc(), x, y, pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight, 0, 0, pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight, chunk.GetBits(), pBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
    if (n != pBitmapInfo->bmiHeader.biHeight)
    {
      MIKTEX_UNEXPECTED();
    }
#endif

#if USE_BITBLT
    HBITMAP hBitmap = CreateDIBitmap(pDC->GetSafeHdc(), &pBitmapInfo->bmiHeader, CBM_INIT, chunk.GetBits(), pBitmapInfo, DIB_RGB_COLORS);
    if (hBitmap == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(hdcMem, hBitmap));
    if (hOldBitmap == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    SetMapMode(hdcMem, GetMapMode(pDC->GetSafeHdc()));
    if (!BitBlt(pDC->GetSafeHdc(), x, y, pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight, hdcMem, 0, 0, SRCCOPY))
    {
      MIKTEX_UNEXPECTED();
    }
    // FIXME: use AutoResource
    if (!SelectObject(hdcMem, hOldBitmap))
    {
      TraceError(T_("SelectObject() failed for some reason"));
    }
    if (!DeleteObject(hBitmap))
    {
      TraceError(T_("DeleteObject() failed for some reason"));
    }
#endif
  }

#if USE_BITBLT
  // FIXME: use AutoResource
  if (!DeleteDC(hdcMem))
  {
    TraceError(T_("DeleteDC() failed for some reason"));
  }
#endif
}

LPBITMAPINFO DviDraw::MakeBitmapInfo(size_t width, size_t height, size_t dpi, size_t bytesPerLine, DviDoc * pDoc)
{
  int mode = pDoc->GetShrinkFactor() == 1 ? DVIVIEW_PRINTER : DVIVIEW_DISPLAY;
  LPBITMAPINFO pBitmapInfo = reinterpret_cast<LPBITMAPINFO>(bitmapInfoTable[mode]);
  pBitmapInfo->bmiHeader.biWidth = static_cast<LONG>(width);
  pBitmapInfo->bmiHeader.biHeight = static_cast<LONG>(height);
  MIKTEX_ASSERT(bytesPerLine % 4 == 0);
  pBitmapInfo->bmiHeader.biSizeImage = static_cast<DWORD>(bytesPerLine * height);
  pBitmapInfo->bmiHeader.biXPelsPerMeter = static_cast<LONG>((dpi * 10000) / 254);
  pBitmapInfo->bmiHeader.biYPelsPerMeter = static_cast<LONG>((dpi * 10000) / 254);
  return pBitmapInfo;
}

void DviDraw::InitializeDviBitmapPalettes()
{
  map<foreback, HPALETTE>::iterator it;
  for (it = foregroundPalettes.begin(); it != foregroundPalettes.end(); ++it)
  {
    if (!DeleteObject((*it).second))
    {
      MIKTEX_UNEXPECTED();
    }
    (*it).second = 0;
  }
  foregroundPalettes.clear();
  foreback fb;
  fb.fore = RGB(0, 0, 0);
  fb.back = RGB(255, 255, 255);
  fb.numcolors = 16;
  foregroundPalettes[fb] = CreateDviBitmapPalette(fb.fore, fb.back, fb.numcolors);
  fb.numcolors = 2;
  foregroundPalettes[fb] = CreateDviBitmapPalette(fb.fore, fb.back, fb.numcolors);
}

HPALETTE DviDraw::CreateDviBitmapPalette(COLORREF foreColor, COLORREF backColor, size_t nColors)
{
  MIKTEX_ASSERT(nColors >= 2);
  size_t size = sizeof(LOGPALETTE) + (nColors - 1) * sizeof(PALETTEENTRY);
  LPLOGPALETTE pLogPal = reinterpret_cast<LPLOGPALETTE>(new BYTE[size]);
  try
  {
    pLogPal->palVersion = 0x300;
    pLogPal->palNumEntries = static_cast<WORD>(nColors);
    pLogPal->palPalEntry[0].peRed = GetRValue(backColor);
    pLogPal->palPalEntry[0].peGreen = GetGValue(backColor);
    pLogPal->palPalEntry[0].peBlue = GetBValue(backColor);
    pLogPal->palPalEntry[0].peFlags = 0;
    pLogPal->palPalEntry[nColors - 1].peRed = GetRValue(foreColor);
    pLogPal->palPalEntry[nColors - 1].peGreen = GetGValue(foreColor);
    pLogPal->palPalEntry[nColors - 1].peBlue = GetBValue(foreColor);
    pLogPal->palPalEntry[nColors - 1].peFlags = 0;
    for (size_t idx = 1; idx < nColors - 1; ++idx)
    {
      double frac = pow(static_cast<double>(idx) / (nColors - 1), 1 / gamma);
      double r = GetRValue(backColor);
      r -= frac * (GetRValue(backColor) - GetRValue(foreColor));
      double g = GetGValue(backColor);
      g -= frac * (GetGValue(backColor) - GetGValue(foreColor));
      double b = GetBValue(backColor);
      b -= frac * (GetBValue(backColor) - GetBValue(foreColor));
      pLogPal->palPalEntry[idx].peRed = static_cast<BYTE>(r + 0.5);
      pLogPal->palPalEntry[idx].peGreen = static_cast<BYTE>(g + 0.5);
      pLogPal->palPalEntry[idx].peBlue = static_cast<BYTE>(b + 0.5);
      pLogPal->palPalEntry[idx].peFlags = 0;
    }
    HPALETTE ret = CreatePalette(pLogPal);
    if (ret == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    delete[] reinterpret_cast<BYTE*>(pLogPal);
    return ret;
  }
  catch (const exception &)
  {
    if (pLogPal != nullptr)
    {
      delete[] reinterpret_cast<BYTE*>(pLogPal);
    }
    throw;
  }
}

#define USE_FILLSOLIDRECT 0     // doesn't seem to work under Windows 95
#define USE_FILLRECT 1

void DviDraw::DrawRules(CDC * pDC, bool blackBoards, DviDoc * pDoc, DviPage * pPage)
{
  UNUSED_ALWAYS(pDoc);

  MIKTEX_ASSERT(pPage != nullptr);

  DviRule * pRule;
  int idx = 0;
  while ((pRule = pPage->GetRule(idx++)) != nullptr)
  {
    if (pRule->IsBlackboard() && !blackBoards)
    {
      continue;
    }
    if (!pRule->IsBlackboard() && blackBoards)
    {
      continue;
    }
    CRect rectRule(pRule->GetLeft(pDoc->GetShrinkFactor()), pRule->GetTop(pDoc->GetShrinkFactor()), pRule->GetRight(pDoc->GetShrinkFactor()) + 1, pRule->GetBottom(pDoc->GetShrinkFactor()) + 1);
#if USE_FILLSOLIDRECT
    pDC->FillSolidRect(&rectRule, pRule->GetBackgroundColor());
#endif
#if USE_FILLRECT
    CBrush brush;
    if (!brush.CreateSolidBrush(pRule->GetBackgroundColor()))
    {
      MIKTEX_UNEXPECTED();
    }
    CBrush * pOldBrush = reinterpret_cast<CBrush*>(pDC->SelectObject(&brush));
    if (pOldBrush == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    AutoSelectObject autoSelectOldBrush(pDC, pOldBrush);
    pDC->FillRect(&rectRule, &brush);
#endif
  }
}
