/* DviDraw.h:                                           -*- C++ -*-

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

#pragma once

class DviDoc;

struct foreback
{
  unsigned numcolors;
  COLORREF fore;
  COLORREF back;
};

inline bool operator< (const foreback & fb1, const foreback & fb2)
{
  return fb1.numcolors < fb2.numcolors
    ? true
    : (fb1.numcolors > fb2.numcolors
      ? false
      : (fb1.fore < fb2.fore
        ? true
        : (fb1.fore > fb2.fore
          ? false
          : fb1.back < fb2.back)));
}

class DviDraw
{
protected:
  DviDraw();

protected:
  virtual ~DviDraw();

protected:
  void DrawDviBitmaps(CDC * pDC, DviDoc * pDoc, DviPage * pPage);

protected:
  void DrawDibChunks(CDC * pDC, DviDoc * pDoc, DviPage * pPage);

protected:
  void DrawRules(CDC * pDC, bool blackBoards, DviDoc * pDoc, DviPage * pPage);

protected:
  void InitializeDviBitmapPalettes();

private:
  HPALETTE CreateDviBitmapPalette(COLORREF foreColor, COLORREF backColor, size_t nColors);

private:
  BITMAPINFO * MakeBitmapInfo(size_t width, size_t height, size_t dpi, size_t bytesPerLine, DviDoc * pDoc);

protected:
  map<foreback, HPALETTE> foregroundPalettes;

protected:
  double gamma;

protected:
  enum {
    DVIVIEW_DISPLAY = 0,
    DVIVIEW_PRINTER = 1
  };

private:
  LPBITMAPINFO bitmapInfoTable[2];
};
