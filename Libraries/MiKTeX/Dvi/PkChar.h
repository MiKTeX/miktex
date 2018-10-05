/* PkChar.h:                                            -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX DVI Library.

   The MiKTeX DVI Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2, or (at your option) any later version.

   The MiKTeX DVI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the MiKTeX DVI Library; if not, write to the
   Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
   USA.  */

#pragma once

class InputStream;
class DviFont;

class PkChar : public DviChar
{
public:
  PkChar(DviFont * pfont = nullptr);

public:
  virtual ~PkChar();

public:
  void Read(InputStream & inputstream, int flag);

public:
  int GetWidthShr(int shrinkFactor)
  {
    return shrinkFactor == 1 ? rasterWidth : WidthShrink(shrinkFactor, rasterWidth) + 1;
  }

public:
  int GetHeightShr(int shrinkFactor)
  {
    return shrinkFactor == 1 ? rasterHeight : WidthShrink(shrinkFactor, rasterHeight) + 1;
  }

public:
  int GetWidthUns()
  {
    return rasterWidth;
  }

public:
  int GetHeightUns()
  {
    return rasterHeight;
  }

public:
  int GetLeftShr(int shrinkFactor, int x)
  {
    return PixelShrink(shrinkFactor, x) - cxOffset / shrinkFactor;
  }

public:
  int GetTopShr(int shrinkFactor, int y)
  {
    return PixelShrink(shrinkFactor, y) - cyOffset / shrinkFactor;
  }

public:
  int GetLeftUns(int x)
  {
    return x - cxOffset;
  }

public:
  int GetTopUns(int y)
  {
    return y - cyOffset;
  }

public:
  const void *
    GetBitmap(int shrinkFactor);

public:
  void
    Print();

  // 16-bit raster word, big-endian
private:
  typedef short int RASTERWORD;

private:
  int GetLower3();

private:
  bool IsShort();

private:
  bool IsExtendedShort();

private:
  bool IsLong();

private:
  unsigned long CountBits(const RASTERWORD * pRasterWord, int xStart, int rasterWordsPerLine, int w, int h);

private:
  void Unpack();

private:
  void * Shrink(int shrinkFactor);

private:
  inline int PixelShrink(int shrinkFactor, int pxl);

private:
  inline int WidthShrink(int shrinkFactor, int pxl);

private:
  typedef unordered_map<int, void *> MAPINTTORASTER;

private:
  MAPINTTORASTER bitmaps;

  // flag byte  
private:
  int flag;

  // length (in bytes) of packed raster data
private:
  int packetSize;

  // width (in pixels) of minimum bounding box
private:
  int rasterWidth = 0;

  // height (in pixels) of minimum bounding box
private:
  int rasterHeight = 0;

  // horizontal offset
private:
  int cxOffset = 0;

  // vertical offset
private:
  int cyOffset = 0;

  // the packed raster data
private:
  unsigned char * pPackedRaster = nullptr;

  // the unpacked raster data
private:
  RASTERWORD * pUnpackedRaster = nullptr;

  // rwords per line
private:
  int numberOfRasterWords;

  // contains the powers of two
private:
  static const int powerOfTwo[32];

  // contains various rows of black
private:
  static const int gpower[33]; // FIXME

private:
  class Unpacker
  {
  public:
    Unpacker(const unsigned char * p, int dynf) :
      pRaster(p),
      dynf(dynf)
    {
    }

  public:
    void ResetBitWeight()
    {
      bitWeight = 0;
    }

  public:
    int GetPackedNumber();

    // dynamic packing variable
  private:
    int dynf;

  private:
    int GetByte()
    {
      return *pRaster++;
    }

    // the byte we are currently decimating
  private:
    int currentByte;

    // weight of the current bit
  private:
    int bitWeight = 0;

    // weight of the current position
  public:
    int rasterWordHeight; // FIXME

  public:
    int repeatCount; // FIXME

  private:
    const unsigned char * pRaster = nullptr;

  private:
    int GetNybble();

  public:
    bool GetBit();
  };

private:
  unique_ptr<TraceStream> trace_error;

private:
  unique_ptr<TraceStream> trace_pkchar;
};
