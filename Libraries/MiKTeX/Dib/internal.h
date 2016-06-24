/* internal.h:                                          -*- C++ -*-

   Copyright (C) 2002-2016 Christian Schenk

   This file is part of the MiKTeX DibChunker Library.

   The MiKTeX DibChunker Library is free software; you can
   redistribute it and/or modify it under the terms of the GNU General
   Public License as published by the Free Software Foundation; either
   version 2, or (at your option) any later version.

   The MiKTeX DibChunker Library is distributed in the hope that it
   will be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX DibChunker Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#define MIKTEXDIBEXPORT __declspec(dllexport)

#define CA71604DC6CF21439A421D76D5C2E5D7
#include "miktex/Graphics/DibChunker.h"

#define OUT_OF_MEMORY(function) MIKTEX_INTERNAL_ERROR()

#define T_(x) MIKTEXTEXT(x)

#define INTERNALFUNC(type) type

class DibChunkImpl : public MiKTeX::Graphics::DibChunk
{
public:
  DibChunkImpl(unsigned bytesPerLine, unsigned numScanLines) :
    size(bytesPerLine * numScanLines)
  {
    pBits = malloc(size);
    if (pBits == nullptr)
    {
      OUT_OF_MEMORY("malloc");
    }
  }

public:
  virtual MIKTEXTHISCALL ~DibChunkImpl()
  {
    if (pBitmapInfo != nullptr)
    {
      free(pBitmapInfo);
      pBitmapInfo = nullptr;
    }
    if (pBits != nullptr)
    {
      free(pBits);
      pBits = nullptr;
    }
  }

public:
  virtual int MIKTEXTHISCALL GetX() const
  {
    return x;
  }

public:
  virtual int MIKTEXTHISCALL GetY() const
  {
    return y;
  }

public:
  virtual BITMAPINFOHEADER MIKTEXTHISCALL GetBitmapInfoHeader() const
  {
    return pBitmapInfo->bmiHeader;
  }

public:
  virtual const RGBQUAD * MIKTEXTHISCALL GetColors() const
  {
    return &pBitmapInfo->bmiColors[0];
  }

public:
  virtual const void * MIKTEXTHISCALL GetBits() const
  {
    return pBits;
  }

public:
  virtual const BITMAPINFO * MIKTEXTHISCALL GetBitmapInfo() const
  {
    return pBitmapInfo;
  }

public:
  virtual size_t MIKTEXTHISCALL GetSize() const
  {
    return size;
  }

public:
  void SetX(int x)
  {
    this->x = x;
  }

public:
  void SetY(int y)
  {
    this->y = y;
  }

public:
  void SetBitmapInfo(const BITMAPINFOHEADER & bitmapInfoHeader, unsigned numColors, const RGBQUAD * pColors)
  {
    if (pBitmapInfo != nullptr)
    {
      free(pBitmapInfo);
      pBitmapInfo = nullptr;
    }
    pBitmapInfo = reinterpret_cast<BITMAPINFO*>(malloc(bitmapInfoHeader.biSize + numColors * sizeof(RGBQUAD)));
    if (pBitmapInfo == nullptr)
    {
      OUT_OF_MEMORY("malloc");
    }
    memcpy(&pBitmapInfo->bmiHeader, &bitmapInfoHeader, bitmapInfoHeader.biSize);
    if (numColors > 0)
    {
      memcpy(pBitmapInfo->bmiColors, pColors, numColors * sizeof(RGBQUAD));
    }
  }

public:
  void * GetBits2()
  {
    return pBits;
  }

private:
  int x;

private:
  int y;

private:
  BITMAPINFO * pBitmapInfo = nullptr;

private:
  void * pBits;

private:
  size_t size;
};

class DibChunkerImpl : public MiKTeX::Graphics::DibChunker
{
public:
  DibChunkerImpl();

public:
  virtual MIKTEXTHISCALL ~DibChunkerImpl();

public:
  virtual bool MIKTEXTHISCALL Process(unsigned long flags, unsigned long chunkSize, MiKTeX::Graphics::IDibChunkerCallback * pCallback);

private:
  bool Read(void * pData, size_t n, bool allowEof = false);

private:
  bool ReadBitmapFileHeader();

private:
  void ReadBitmapInfo();

private:
  bool ReadScanLine(unsigned long & left, unsigned long & right);

private:
  void BeginChunk();

private:
  void AddScanLine();

private:
  void EndChunk();

private:
  unsigned long BytesPerLine(long width, long bitCount) const
  {
    return (((width * bitCount) + 31) & ~31) >> 3;
  }

private:
  unsigned long BytesPerLine(long width) const
  {
    return BytesPerLine(width, bitmapInfoHeader.biBitCount);
  }

private:
  unsigned long BytesPerLine() const
  {
    return BytesPerLine(bitmapInfoHeader.biWidth);
  }

private:
  COLORREF GetColor(unsigned long idx) const
  {
    MIKTEX_ASSERT(pColors != nullptr);
    MIKTEX_ASSERT(idx < numColors);
    RGBQUAD q = pColors[idx];
    return RGB(q.rgbRed, q.rgbGreen, q.rgbBlue);
  }

private:
  bool Crop1(unsigned long & left, unsigned long & right);

private:
  bool Crop4(unsigned long & left, unsigned long & right);

private:
  bool Crop8(unsigned long & left, unsigned long & right);

private:
  bool Crop24(unsigned long & left, unsigned long & right);

private:
  void Monochromize24(const unsigned char * pSrc, unsigned char * pDst, unsigned long width);

private:
  MiKTeX::Graphics::IDibChunkerCallback * pCallback;

private:
  RGBQUAD * pColors = nullptr;

private:
  size_t numBytesRead;

private:
  unsigned char * pScanLine = nullptr;

private:
  unsigned char * pBits = nullptr;

private:
  unsigned numScanLines;

private:
  int yPos;

private:
  bool isBlackAndWhite;

private:
  unsigned long leftPos;

private:
  unsigned long rightPos;

private:
  bool inChunk;

private:
  int yPosChunk;

private:
  unsigned long numColors;

private:
  unsigned long blankLines;

private:
  BITMAPFILEHEADER bitmapFileHeader;

private:
  BITMAPINFOHEADER bitmapInfoHeader;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_dib;

private:
  int processingFlags = Default;
};
