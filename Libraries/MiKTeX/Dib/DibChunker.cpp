/* DibChunker.cpp:

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

#include "StdAfx.h"

#include "internal.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Graphics;
using namespace MiKTeX::Trace;
using namespace std;

const COLORREF RGB_WHITE = RGB(255, 255, 255);
const COLORREF RGB_BLACK = RGB(0, 0, 0);

const RGBQUAD RGBQUAD_WHITE = { 255, 255, 255, 0 };
const RGBQUAD RGBQUAD_BLACK = { 0, 0, 0, 0 };

const RGBQUAD whiteAndBlack[2] = { RGBQUAD_WHITE, RGBQUAD_BLACK };

DibChunk::~DibChunk()
{
}

unique_ptr<DibChunker> DibChunker::Create()
{
  return make_unique<DibChunkerImpl>();
}

DibChunker::~DibChunker()
{
}

DibChunkerImpl::DibChunkerImpl()
  : trace_dib(TraceStream::Open(MIKTEX_TRACE_DIB))
{
}

DibChunkerImpl::~DibChunkerImpl()
{
  try
  {
    if (pColors != nullptr)
    {
      delete[] pColors;
      pColors = nullptr;
    }
    if (pScanLine != nullptr)
    {
      delete[] pScanLine;
      pScanLine = nullptr;
    }
    if (pBits != nullptr)
    {
      delete[] pBits;
      pBits = nullptr;
    }
    if (trace_dib != nullptr)
    {
      trace_dib->Close();
      trace_dib = nullptr;
    }
  }
  catch (const exception &)
  {
  }
}

bool DibChunkerImpl::Read(void * pData, size_t size, bool allowEof)
{
  while (size > 0)
  {
    size_t n = pCallback->Read(pData, size);
    size -= n;
    this->numBytesRead += n;
    pData = reinterpret_cast<unsigned char*>(pData) + n;
    if (n == 0)
    {
      if (allowEof)
      {
        return false;
      }
      MIKTEX_UNEXPECTED();
    }
  }
  return true;
}

bool DibChunkerImpl::ReadBitmapFileHeader()
{
  if (!Read(&bitmapFileHeader.bfType, sizeof(bitmapFileHeader.bfType), true))
  {
    return false;
  }
  if (bitmapFileHeader.bfType != 0x4d42)
  {
    MIKTEX_UNEXPECTED();
  }
  Read(&bitmapFileHeader.bfSize, (sizeof(BITMAPFILEHEADER) - offsetof(BITMAPFILEHEADER, bfSize)));
  return true;
}

unsigned long CalcNumColors(unsigned long bitCount)
{
  if (bitCount == 24)
  {
    return 0;
  }
  else
  {
    return 1 << bitCount;
  }
}

void DibChunkerImpl::ReadBitmapInfo()
{
  // get known part of bitmap info header
  Read(&bitmapInfoHeader, sizeof(bitmapInfoHeader));
  if (bitmapInfoHeader.biSize < sizeof(bitmapInfoHeader))
  {
    MIKTEX_FATAL_ERROR(T_("Unsupported bitmap format."));
  }
  if (!(bitmapInfoHeader.biBitCount == 1
    || bitmapInfoHeader.biBitCount == 4
    || bitmapInfoHeader.biBitCount == 8
    || bitmapInfoHeader.biBitCount == 24))
  {
    MIKTEX_FATAL_ERROR(T_("Unsupported bitmap format."));
  }
  if (bitmapInfoHeader.biCompression != BI_RGB)
  {
    MIKTEX_FATAL_ERROR(T_("Unsupported bitmap format."));
  }

  // skip unknown part of bitmap info header
  if (bitmapInfoHeader.biSize > sizeof(bitmapInfoHeader))
  {
    unsigned long n = bitmapInfoHeader.biSize - sizeof(bitmapInfoHeader);
    void * p = malloc(n);
    if (p == nullptr)
    {
      OUT_OF_MEMORY("malloc");
    }
    AutoMemoryPointer autoFree(p);
    Read(p, n);
  }

  // get colors
  numColors = bitmapInfoHeader.biClrUsed == 0 ? CalcNumColors(bitmapInfoHeader.biBitCount) : bitmapInfoHeader.biClrUsed;
  if (numColors > 0)
  {
    pColors = new RGBQUAD[numColors];
    Read(pColors, sizeof(RGBQUAD) * numColors);
  }

  // seek for bitmap bits
  if (bitmapFileHeader.bfOffBits < numBytesRead)
  {
    MIKTEX_UNEXPECTED();
  }
  if (bitmapFileHeader.bfOffBits > numBytesRead)
  {
    unsigned long n = bitmapFileHeader.bfOffBits - numBytesRead;
    void * p = malloc(n);
    if (p == nullptr)
    {
      OUT_OF_MEMORY("malloc");
    }
    AutoMemoryPointer autoFree(p);
    Read(p, n);
  }
  MIKTEX_ASSERT(bitmapFileHeader.bfOffBits == numBytesRead);
  trace_dib->WriteFormattedLine("libdib", T_("chunking bitmap %ldx%ld, %u colors"), bitmapInfoHeader.biWidth, bitmapInfoHeader.biHeight, numColors);
}

bool DibChunkerImpl::Crop1(unsigned long & left, unsigned long & right)
{
  left = UINT_MAX;
  right = UINT_MAX;

  // find white in color table
  unsigned long whiteIdx = 0;
  if (GetColor(0) != RGB_WHITE)
  {
    whiteIdx = 1;
    if (GetColor(1) != RGB_WHITE)
    {
      // white not found
      return false;
    }
  }

  // white byte
  unsigned char white = static_cast<unsigned char>(whiteIdx == 0 ? 0 : 255);

  // number of bytes to check
  unsigned long n = (bitmapInfoHeader.biWidth + 7) / 8;

  // crop
  for (unsigned long idx = 0; idx < n; idx += 1)
  {
    if (pScanLine[idx] != white)
    {
      if (idx < left)
      {
        left = idx;
      }
      right = idx;
    }
  }

  return true;
}

bool DibChunkerImpl::Crop4(unsigned long & left, unsigned long & right)
{
  left = UINT_MAX;
  right = UINT_MAX;

  // number of bytes to check
  unsigned long n = (bitmapInfoHeader.biWidth + 1) / 2;

  bool isBlackAndWhite = true;

  // crop
  for (unsigned long idx = 0; idx < n; idx += 1)
  {
    unsigned char byte = pScanLine[idx];
    unsigned high = (byte >> 4) & 15;
    unsigned low = byte & 15;
    if (!(GetColor(high) == RGB_WHITE && GetColor(low) == RGB_WHITE))
    {
      if (idx < left)
      {
        left = idx;
      }
      right = idx;
      if (!(GetColor(high) == RGB_BLACK && GetColor(low) == RGB_BLACK))
      {
        isBlackAndWhite = false;
      }
    }
  }

  return isBlackAndWhite;
}

bool DibChunkerImpl::Crop8(unsigned long & left, unsigned long & right)
{
  left = UINT_MAX;
  right = UINT_MAX;

  // number of bytes to check
  unsigned long n = bitmapInfoHeader.biWidth;

  bool isBlackAndWhite = true;

  // crop
  for (unsigned long idx = 0; idx < n; idx += 1)
  {
    if (GetColor(pScanLine[idx]) != RGB_WHITE)
    {
      if (idx < left)
      {
        left = idx;
      }
      right = idx;
      if (GetColor(pScanLine[idx]) != RGB_BLACK)
      {
        isBlackAndWhite = false;
      }
    }
  }

  return isBlackAndWhite;
}

bool DibChunkerImpl::Crop24(unsigned long & left, unsigned long & right)
{
  left = UINT_MAX;
  right = UINT_MAX;

  // number of bytes to check
  unsigned long n = bitmapInfoHeader.biWidth * 3;

  bool isBlackAndWhite = true;

  // crop
  for (unsigned long idx = 0; idx < n; idx += 3)
  {
    COLORREF clr = RGB(pScanLine[idx + 2], pScanLine[idx + 1], pScanLine[idx]);
    if (clr != RGB_WHITE)
    {
      if (idx < left)
      {
        left = idx;
      }
      right = idx + 2;
      if (clr != RGB_BLACK)
      {
        isBlackAndWhite = false;
      }
    }
  }

  return isBlackAndWhite;
}

bool DibChunkerImpl::ReadScanLine(unsigned long & left, unsigned long & right)
{
  unsigned long n = BytesPerLine();

  if (pScanLine == nullptr)
  {
    pScanLine = new unsigned char[n];
  }

  Read(pScanLine, n);

  bool isBlackAndWhite;

  if ((processingFlags & Crop) != 0)
  {
    // analyse the scan line
    switch (bitmapInfoHeader.biBitCount)
    {
    case 1:
      isBlackAndWhite = true;
      Crop1(left, right);
      break;
    case 4:
      isBlackAndWhite = Crop4(left, right);
      break;
    case 8:
      isBlackAndWhite = Crop8(left, right);
      break;
    case 24:
      isBlackAndWhite = Crop24(left, right);
      break;
    default:
      MIKTEX_UNEXPECTED();
    }
  }
  else
  {
    isBlackAndWhite = false;
    left = 0;
    // analyse the scan line
    switch (bitmapInfoHeader.biBitCount)
    {
    case 1:
      right = (bitmapInfoHeader.biWidth + 7) / 8 - 1;
      break;
    case 4:
      right = (bitmapInfoHeader.biWidth + 1) / 2 - 1;
      break;
    case 8:
      right = bitmapInfoHeader.biWidth;
      break;
    case 24:
      right = bitmapInfoHeader.biWidth * 3 - 1;
      break;
    default:
      MIKTEX_UNEXPECTED();
    }
  }

  return isBlackAndWhite;
}

void DibChunkerImpl::AddScanLine()
{
  unsigned long n = BytesPerLine();
  memcpy(&pBits[n * numScanLines], pScanLine, n);
  numScanLines += 1;
}

void DibChunkerImpl::BeginChunk()
{
  inChunk = true;
  numScanLines = 0;
  blankLines = 0;
  leftPos = BytesPerLine();
  rightPos = 0;
  isBlackAndWhite = true;
  yPosChunk = yPos;
}

void DibChunkerImpl::Monochromize24(const unsigned char * pSrc, unsigned char * pDst, unsigned long width)
{
  unsigned char byte = 0;
  unsigned long idx;
  for (idx = 0; idx < width; ++idx)
  {
    unsigned n = (idx % 8);
    COLORREF clr = RGB(pSrc[idx * 3 + 2], pSrc[idx * 3 + 1], pSrc[idx * 3 + 0]);
    if (clr != RGB_WHITE)
    {
      byte |= (1 << (7 - n));
    }
    if (n == 7)
    {
      *pDst++ = byte;
      byte = 0;
    }
  }
  if ((idx % 8) > 0)
  {
    *pDst++ = byte;
  }
}

void DibChunkerImpl::EndChunk()
{
  // cut off white lines 
  numScanLines -= blankLines;

  long bytesInChunk = (rightPos - leftPos + 1);
  int x;
  long biWidth;
  long bytesPerLine;
  bool monochromize24 = false;
  switch (bitmapInfoHeader.biBitCount)
  {
  case 1:
    x = leftPos * 8;
    biWidth = bytesInChunk * 8;
    bytesPerLine = BytesPerLine(biWidth, 1);
    break;
  case 4:
    x = leftPos * 2;
    biWidth = bytesInChunk * 2;
    bytesPerLine = BytesPerLine(biWidth, 4);
    break;
  case 8:
    x = leftPos;
    biWidth = bytesInChunk;
    bytesPerLine = BytesPerLine(biWidth, 8);
    break;
  case 24:
    MIKTEX_ASSERT(leftPos % 3 == 0);
    x = leftPos / 3;
    MIKTEX_ASSERT(bytesInChunk % 3 == 0);
    biWidth = bytesInChunk / 3;
#if 1
    if (isBlackAndWhite)
    {
      monochromize24 = true;
      bytesPerLine = BytesPerLine(biWidth, 1);
    }
    else
#endif
    {
      bytesPerLine = BytesPerLine(biWidth, 24);
    }
    break;
  default:
    MIKTEX_UNEXPECTED();
  }

  shared_ptr<DibChunkImpl> pChunk = make_shared<DibChunkImpl>(bytesPerLine, numScanLines);

  pChunk->SetX(x);

  // make chunked bitmap info header
  BITMAPINFOHEADER bitmapinfoheader;
  bitmapinfoheader = this->bitmapInfoHeader;
  bitmapinfoheader.biHeight = numScanLines;
  bitmapinfoheader.biWidth = biWidth;
  bitmapinfoheader.biSizeImage = 0;
  const RGBQUAD * pColors = nullptr;
  unsigned numColors = 0;
  if (monochromize24)
  {
    bitmapinfoheader.biBitCount = 1;
    numColors = 2;
    pColors = &whiteAndBlack[0];
  }
  else
  {
    numColors = this->numColors;
    pColors = this->pColors;
  }
  // FIXME: okay?
  int y = yPosChunk;
  y += bitmapinfoheader.biHeight - 1;
  y = this->bitmapInfoHeader.biHeight - y;
  pChunk->SetY(y);
  //
  unsigned char * pBits = reinterpret_cast<unsigned char*>(pChunk->GetBits2());
  for (unsigned long i = 0; i < numScanLines; ++i)
  {
    const unsigned char * pSrc = this->pBits + i * BytesPerLine() + leftPos;
    unsigned char * pDest = pBits + i * bytesPerLine;
    memset(pDest, 0, bytesPerLine);
    if (monochromize24)
    {
      Monochromize24(pSrc, pDest, biWidth);
    }
    else
    {
      memcpy(pDest, pSrc, bytesInChunk);
    }
  }
  trace_dib->WriteFormattedLine("libdib", T_("shipping chunk: x=%ld, y=%ld, w=%ld, h=%ld, monochromized=%s"), pChunk->GetX(), pChunk->GetY(), bitmapinfoheader.biWidth, bitmapinfoheader.biHeight, (monochromize24 ? "true" : "false"));
  inChunk = false;
  pChunk->SetBitmapInfo(bitmapinfoheader, numColors, pColors);
  pCallback->OnNewChunk(pChunk);
}

bool DibChunkerImpl::Process(unsigned long flags, unsigned long chunkSize, IDibChunkerCallback * pCallback)
{
  this->processingFlags = flags;
  this->pCallback = pCallback;
  numBytesRead = 0;
  if (!ReadBitmapFileHeader())
  {
    return false;
  }
  ReadBitmapInfo();
  if (BytesPerLine() > chunkSize)
  {
    MIKTEX_UNEXPECTED();
  }
  pBits = new unsigned char[chunkSize];
  try
  {
    inChunk = false;
    blankLines = 0;
    for (yPos = 0; yPos < bitmapInfoHeader.biHeight; yPos += 1)
    {
      unsigned long left;
      unsigned long right;
      if (!ReadScanLine(left, right))
      {
        isBlackAndWhite = false;
      }
      if (left == UINT_MAX)
      {                   // white line
        if (!inChunk)
        {
          continue;
        }
        if (inChunk && (flags & RemoveBlankLines))
        {
          EndChunk();
          continue;
        }
        blankLines += 1;
      }
      else
      {
        blankLines = 0;
      }
      if (!inChunk)
      {
        BeginChunk();
      }
      if ((numScanLines + 1) * BytesPerLine() > chunkSize)
      {
        if (!(flags & CreateChunks))
        {
          MIKTEX_UNEXPECTED();
        }
        EndChunk();
        BeginChunk();
      }
      AddScanLine();
      if (left < this->leftPos)
      {
        this->leftPos = left;
      }
      if (right != UINT_MAX && right > this->rightPos)
      {
        this->rightPos = right;
      }
    }
    if (inChunk)
    {
      EndChunk();
    }
    delete[] pBits;
    pBits = nullptr;
    return true;
  }
  catch (const exception &)
  {
    delete[] pBits;
    pBits = nullptr;
    throw;
  }
}
