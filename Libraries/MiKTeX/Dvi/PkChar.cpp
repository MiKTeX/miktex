/* PkChar.cpp:

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

#include "StdAfx.h"

#include "internal.h"

#define twopwr(n) (((int) 1) << (n))

const unsigned long bitsPerRasterWord = 16;

#if defined(max)
#undef max
#undef min
#endif

const int PkChar::powerOfTwo[32] =
{
  twopwr(0), twopwr(1), twopwr(2), twopwr(3), twopwr(4), twopwr(5), twopwr(6), twopwr(7),
  twopwr(8), twopwr(9), twopwr(10), twopwr(11), twopwr(12), twopwr(13), twopwr(14), twopwr(15),
  twopwr(16), twopwr(17), twopwr(18), twopwr(19), twopwr(20), twopwr(21), twopwr(22), twopwr(23),
  twopwr(24), twopwr(25), twopwr(26), twopwr(27), twopwr(28), twopwr(29), twopwr(30), twopwr(31)
};

const int PkChar::gpower[33] =
{
  twopwr(0) - 1, twopwr(1) - 1, twopwr(2) - 1, twopwr(3) - 1, twopwr(4) - 1, twopwr(5) - 1, twopwr(6) - 1, twopwr(7) - 1,
  twopwr(8) - 1, twopwr(9) - 1, twopwr(10) - 1, twopwr(11) - 1, twopwr(12) - 1, twopwr(13) - 1, twopwr(14) - 1, twopwr(15) - 1,
  twopwr(16) - 1, twopwr(17) - 1, twopwr(18) - 1, twopwr(19) - 1, twopwr(20) - 1, twopwr(21) - 1, twopwr(22) - 1, twopwr(23) - 1,
  twopwr(24) - 1, twopwr(25) - 1, twopwr(26) - 1, twopwr(27) - 1, twopwr(28) - 1, twopwr(29) - 1, twopwr(30) - 1, 2147483647,
  -1
};

int PkChar::GetLower3()
{
  return flag & 7;
}

PkChar::PkChar(DviFont * pFont) :
  DviChar(pFont),
  trace_error(TraceStream::Open(MIKTEX_TRACE_ERROR)),
  trace_pkchar(TraceStream::Open(MIKTEX_TRACE_DVIPKCHAR))
{
}

PkChar::~PkChar()
{
  try
  {
    if (pPackedRaster != nullptr)
    {
      delete[] pPackedRaster;
      pPackedRaster = nullptr;
    }
    if (pUnpackedRaster != nullptr)
    {
      delete[] pUnpackedRaster;
      pUnpackedRaster = nullptr;
    }
    for (MAPINTTORASTER::iterator it = bitmaps.begin(); it != bitmaps.end(); ++it)
    {
      free(it->second);
      it->second = nullptr;
    }
    if (trace_error != nullptr)
    {
      trace_error->Close();
      trace_error = nullptr;
    }
    if (trace_pkchar != nullptr)
    {
      trace_pkchar->Close();
      trace_pkchar = nullptr;
    }
  }
  catch (const exception &)
  {
  }
}

bool PkChar::IsShort()
{
  return GetLower3() < 4 ? true : false;
}

bool PkChar::IsExtendedShort()
{
  return GetLower3() < 7 && !IsShort() ? true : false;
}

bool PkChar::IsLong()
{
  return GetLower3() == 7 ? true : false;
}

void PkChar::Read(InputStream & inputstream, int flag)
{
  this->flag = flag;

  if (IsShort())
  {
    // read character preamble (short form)
    packetSize = ((static_cast<int>(GetLower3()) % 4) << 8 | inputstream.ReadByte()) - 8;
    charCode = inputstream.ReadByte();
    tfm = inputstream.ReadTrio();
    cx = inputstream.ReadByte();
    rasterWidth = inputstream.ReadByte();
    rasterHeight = inputstream.ReadByte();
    cxOffset = inputstream.ReadSignedByte();
    cyOffset = inputstream.ReadSignedByte();
  }
  else if (IsExtendedShort())
  {
    // read character preamble (extended short form)
    packetSize = ((static_cast<int>(GetLower3()) % 4) << 16 | inputstream.ReadPair()) - 13;
    charCode = inputstream.ReadByte();
    tfm = inputstream.ReadTrio();
    cx = inputstream.ReadPair();
    rasterWidth = inputstream.ReadPair();
    rasterHeight = inputstream.ReadPair();
    cxOffset = inputstream.ReadSignedPair();
    cyOffset = inputstream.ReadSignedPair();
  }
  else
  {
    // read character preamble (long form)
    packetSize = inputstream.ReadSignedQuad() - 28;
    charCode = inputstream.ReadSignedQuad();
    tfm = inputstream.ReadSignedQuad();
    cx = inputstream.ReadSignedQuad();
    cx = (cx + 0x10000) >> 16;
    cy = inputstream.ReadSignedQuad();
    cy = (cy + 0x10000) >> 16;
    rasterWidth = inputstream.ReadSignedQuad();
    rasterHeight = inputstream.ReadSignedQuad();
    cxOffset = inputstream.ReadSignedQuad();
    cyOffset = inputstream.ReadSignedQuad();
  }

  tfm = ScaleFix(tfm, pDviFont->GetScaledAt());

  if (packetSize == 0)
  {
#if 0
    trace_error->WriteFormattedLine("libdvi", T_("%d: no glyph!"), charCode);
#endif
  }
  else
  {
    trace_pkchar->WriteFormattedLine("libdvi", T_("going to read character %d"), charCode);
    pPackedRaster = new BYTE[packetSize];
    inputstream.Read(pPackedRaster, packetSize);
  }
}

int PkChar::Unpacker::GetNybble()
{
  if (bitWeight == 0)
  {
    currentByte = GetByte();
    bitWeight = 16;
  }

  int temp = currentByte / bitWeight;
  currentByte -= temp * bitWeight;
  bitWeight /= 16;

  return temp;
}

bool PkChar::Unpacker::GetBit()
{
  bitWeight /= 2;

  if (bitWeight == 0)
  {
    currentByte = GetByte();
    bitWeight = 128;
  }

  bool temp = (currentByte >= bitWeight ? true : false);

  if (temp)
  {
    currentByte -= bitWeight;
  }

  return temp;
}

int PkChar::Unpacker::GetPackedNumber()
{
  int i = GetNybble();
  int j;
  if (i == 0)
  {
    do
    {
      j = GetNybble();
      ++i;
    } while (j == 0);
    while (i > 0)
    {
      j = j * 16 + GetNybble();
      --i;
    }
    return j - 15 + (13 - dynf) * 16 + dynf;
  }
  else if (i <= dynf)
  {
    return i;
  }
  else if (i < 14)
  {
    return (i - dynf - 1) * 16 + GetNybble() + dynf + 1;
  }
  else
  {
    if (i == 14)
    {
      repeatCount = GetPackedNumber();
    }
    else
    {
      repeatCount = 1;
    }
    return GetPackedNumber();
  }
}

void PkChar::Unpack()
{
  if (pUnpackedRaster != nullptr)
  {
    return;
  }
  int dynf = flag >> 4;
  Unpacker unp(pPackedRaster, dynf);
  numberOfRasterWords = (rasterWidth + bitsPerRasterWord - 1) / bitsPerRasterWord;
  if (rasterWidth == 0 || rasterHeight == 0)
  {
#if 0
    MIKTEX_UNEXPECTED();
#endif
    return;
  }
  pUnpackedRaster = new RASTERWORD[rasterHeight * numberOfRasterWords];
  RASTERWORD rword = 0;
  RASTERWORD * pUnpackedRaster = this->pUnpackedRaster;
  bool turnOn = (flag & 8 ? true : false);
  if (dynf == 14)
  {
    // get raster by bits
    unp.ResetBitWeight();
    for (int i = 1; i <= rasterHeight; ++i)
    {
      rword = 0;
      unp.rasterWordHeight = bitsPerRasterWord - 1;
      for (int j = 1; j <= rasterWidth; ++j)
      {
        if (unp.GetBit())
        {
          rword = static_cast<RASTERWORD>(rword + powerOfTwo[unp.rasterWordHeight]);
        }
        --unp.rasterWordHeight;
        if (unp.rasterWordHeight == -1)
        {
          *pUnpackedRaster++ = rword;
          rword = 0;
          unp.rasterWordHeight = bitsPerRasterWord - 1;
        }
      }
      if (unp.rasterWordHeight < bitsPerRasterWord - 1)
      {
        *pUnpackedRaster++ = rword;
      }
    }
  }
  else
  {
    // create normally packed raster
    int rows_left = rasterHeight;     // how many rows left?
    int h_bit = rasterWidth;          // what is our horizontal position?
    unp.repeatCount = 0;
    unp.rasterWordHeight = bitsPerRasterWord;
    rword = 0;
    while (rows_left > 0)
    {
      int count;
      count = unp.GetPackedNumber(); // how many bits of current color left?
      while (count > 0)
      {
        if ((count < unp.rasterWordHeight) && (count < h_bit))
        {
          if (turnOn)
          {
            rword = static_cast<RASTERWORD>(rword + (gpower[unp.rasterWordHeight] - gpower[unp.rasterWordHeight - count]));
          }
          h_bit -= count;
          unp.rasterWordHeight -= count;
          count = 0;
        }
        else if ((count >= h_bit) && (h_bit <= unp.rasterWordHeight))
        {
          if (turnOn)
          {
            rword = static_cast<RASTERWORD>(rword + (gpower[unp.rasterWordHeight] - gpower[unp.rasterWordHeight - h_bit]));
          }
          *pUnpackedRaster++ = rword;
          // send row
          for (int i = 1; i <= unp.repeatCount; ++i)
          {
            for (int j = 1; j <= numberOfRasterWords; ++j, ++pUnpackedRaster)
            {
              *pUnpackedRaster = pUnpackedRaster[-numberOfRasterWords];
            }
          }
          rows_left = rows_left - unp.repeatCount - 1;
          unp.repeatCount = 0;
          rword = 0;
          unp.rasterWordHeight = bitsPerRasterWord;
          count -= h_bit;
          h_bit = rasterWidth;
        }
        else
        {
          if (turnOn)
          {
            rword = static_cast<RASTERWORD>(rword + gpower[unp.rasterWordHeight]);
          }
          *pUnpackedRaster++ = rword;
          rword = 0;
          count -= unp.rasterWordHeight;
          h_bit -= unp.rasterWordHeight;
          unp.rasterWordHeight = bitsPerRasterWord;
        }
      }
      turnOn = (turnOn ? false : true);
    }
    if ((rows_left != 0) || (h_bit != rasterWidth))
    {
      MIKTEX_UNEXPECTED();
    }
  }
}

const unsigned long maxBitFieldLength = bitsPerRasterWord;

namespace {
  const unsigned char bitcounts[] =
  {
#include "bitcounts.h"
  };
}

unsigned long PkChar::CountBits(const RASTERWORD * pRasterWord, int xStart, int rasterWordsPerLine, int w, int h)
{
  unsigned long result = 0;

  unsigned long rightShift = bitsPerRasterWord - (xStart % bitsPerRasterWord);
  pRasterWord += xStart / bitsPerRasterWord;

  while (w > 0)
  {
    unsigned long bitFieldLength = std::min(rightShift, static_cast<unsigned long>(w));
    bitFieldLength = std::min(bitFieldLength, maxBitFieldLength);
    rightShift -= bitFieldLength;
    const RASTERWORD * pRasterWord2 = pRasterWord;
    for (int i = 0; i < h; ++i, pRasterWord2 += rasterWordsPerLine)
    {
      RASTERWORD rw = *pRasterWord2;
      rw >>= rightShift;
      rw &= gpower[bitFieldLength];
      result += bitcounts[rw];
    }
    if (rightShift == 0)
    {
      rightShift = bitsPerRasterWord;
      pRasterWord++;
    }
    w -= bitFieldLength;
  }

  return result;
}

void PkChar::Print()
{
}

inline unsigned long color(unsigned long n, unsigned long bitsPerPixel, unsigned long shrinkFactor)
{
  return Round(static_cast<double>(n * (twopwr(bitsPerPixel) - 1)) / static_cast<double>(shrinkFactor * shrinkFactor));
}

void * PkChar::Shrink(int shrinkFactor)
{
#define BE_FAST
#ifdef BE_FAST
  if (shrinkFactor == 1)
  {
    unsigned long cbLine = ((rasterWidth + 31) / 32) * 4;
    unsigned long rasterWordsPerLine = (rasterWidth + bitsPerRasterWord - 1) / bitsPerRasterWord;

    unsigned char * pShrinkedRaster = reinterpret_cast<unsigned char*>(malloc(rasterHeight * cbLine));
    memset(pShrinkedRaster, 0, rasterHeight * cbLine);

    int shrinkedRasterHeight = 0;

    for (int row = 0; row < rasterHeight; ++shrinkedRasterHeight, ++row)
    {
      BYTE * pbyte = &pShrinkedRaster[shrinkedRasterHeight * cbLine];
      unsigned long idxBit = 7;
      for (int col = 0; col < rasterWidth; ++col)
      {
        RASTERWORD rw =
          pUnpackedRaster[(rasterWordsPerLine * row
            + col / bitsPerRasterWord)];
        unsigned long n =
          ((rw & powerOfTwo[bitsPerRasterWord
            - col % bitsPerRasterWord
            - 1])
            ? 1
            : 0);
        if (idxBit == 0)
        {
          *pbyte |= static_cast<BYTE>(n);
          ++pbyte;
          idxBit = 7;
        }
        else
        {
          int shift = idxBit;
          *pbyte |= static_cast<BYTE>((n << shift) & 0xff);
          --idxBit;
        }

      }
    }

    return pShrinkedRaster;
  }
#endif // BE_FAST

  int widthShr = GetWidthShr(shrinkFactor);
  int heightShr = GetHeightShr(shrinkFactor);

  unsigned long bitsPerPixel = pDviFont->GetDviObject()->GetBitsPerPixel(shrinkFactor);
  unsigned long lineSizeShr = ((widthShr * bitsPerPixel + 31) / 32) * 4;

  unsigned long rasterWordsPerLine = (rasterWidth + bitsPerRasterWord - 1) / bitsPerRasterWord;

  unsigned char * pShrinkedRaster = reinterpret_cast<unsigned char*>(malloc(heightShr * lineSizeShr));
  memset(pShrinkedRaster, 0, heightShr * lineSizeShr);

  int shrinkedRasterHeight = 0;

  int sampleHeight = (cyOffset + 1) - ((cyOffset + 1) / shrinkFactor) * shrinkFactor;

  if (sampleHeight <= 0)
  {
    sampleHeight += shrinkFactor;
  }

  for (int row = 0; row < rasterHeight; shrinkedRasterHeight += 1)
  {
    BYTE * pbyte = &pShrinkedRaster[shrinkedRasterHeight * lineSizeShr];
    unsigned long idxBit = 7;

    int sampleWidth = ((cxOffset + 1) - ((cxOffset + 1) / shrinkFactor) * shrinkFactor);
    if (sampleWidth <= 0)
    {
      sampleWidth += shrinkFactor;
    }

    for (int col = 0; col < rasterWidth; )
    {
      unsigned long n = color(CountBits(pUnpackedRaster + (rasterWordsPerLine * row), col, rasterWordsPerLine, std::min(sampleWidth, rasterWidth - col), std::min(sampleHeight, rasterHeight - row)), bitsPerPixel, shrinkFactor);

      if (idxBit == bitsPerPixel - 1)
      {
        *pbyte |= static_cast<BYTE>(n);
        pbyte++;
        idxBit = 7;
      }
      else
      {
        int shift = idxBit - bitsPerPixel + 1;
        *pbyte |= static_cast<BYTE>((n << shift) & 0xff);
        idxBit -= bitsPerPixel;
      }

      col += sampleWidth;
      sampleWidth = shrinkFactor;
    }

    row += sampleHeight;
    sampleHeight = shrinkFactor;
  }

  return pShrinkedRaster;
}

const void * PkChar::GetBitmap(int shrinkFactor)
{
  MAPINTTORASTER::const_iterator it = bitmaps.find(shrinkFactor);
  if (it != bitmaps.end())
  {
    return it->second;
  }
  Unpack();
  void * p = Shrink(shrinkFactor);
  bitmaps[shrinkFactor] = p;
  return p;
}
