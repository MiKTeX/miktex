/* DviPage.cpp:

   Copyright (C) 1996-2020 Christian Schenk

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

#include "config.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Paths>

#include "internal.h"

#define SMALLBITMAPS 0

#if SMALLBITMAPS
const int MaxVerticalWhite = 0;
const int MaxHorizontalWhite = 0;
#else
const int MaxVerticalWhite = 2;
const int MaxHorizontalWhite = 32;
#endif

size_t DviPageImpl::totalSize = 0;

#if defined(max)
#undef max
#undef min
#endif

inline bool operator< (DviItem& item1, DviItem& item2)
{
  return item1.GetTopShr(1) < item2.GetTopShr(1);
}

inline bool operator>= (DviItem& item1, DviItem& item2)
{
  return !(item1 < item2);
}

class ItemTopDown
{
public:
  bool operator() (DviItem& i1, DviItem& i2) const
  {
    return i1 < i2;
  }
};

class ItemLeftRight
{
public:
  bool operator() (DviItem* i1, DviItem* i2) const
  {
    return i1->GetLeftShr(1) < i2->GetLeftShr(1);
  }
};

// copied from tex.web
string ToRoman(unsigned n)
{
  string result;
  const char* NUMERALS = "m2d5c2l5x2v5i";
  unsigned j = 0;
  unsigned v = 1000;
  for (;;)
  {
    while (n >= v)
    {
      result += NUMERALS[j];
      n -= v;
    }
    if (n == 0)
    {
      return result;
    }
    unsigned k = j + 2;
    unsigned u = v / (NUMERALS[k - 1] - '0');
    if (NUMERALS[k - 1] == '2')
    {
      k += 2;
      u /= (NUMERALS[k - 1] - '0');
    }
    if (n + u >= v)
    {
      result += NUMERALS[k];
      n += u;
    }
    else
    {
      j += 2;
      v /= (NUMERALS[j - 1] - '0');
    }
  }
}

DviPageImpl::DviPageImpl(DviImpl* dviImpl, int pageIdx, DviPageMode pageMode, long readPosition, int c0, int c1, int c2, int c3, int c4, int c5, int c6, int c7, int c8, int c9) :
  dviImpl(dviImpl),
  pageIdx(pageIdx),
  pageMode(pageMode),
  readPosition(readPosition),
  tracePage(TraceStream::Open(MIKTEX_TRACE_DVIPAGE, dviImpl->GetTraceCallback())),
  traceBitmap(TraceStream::Open(MIKTEX_TRACE_DVIBITMAP, dviImpl->GetTraceCallback()))
{
  if (pageMode != DviPageMode::Dvips)
  {
    dviItems.reserve(500);
  }

  counts[0] = c0;
  counts[1] = c1;
  counts[2] = c2;
  counts[3] = c3;
  counts[4] = c4;
  counts[5] = c5;
  counts[6] = c6;
  counts[7] = c7;
  counts[8] = c8;
  counts[9] = c9;

  int j;

  for (j = 9; j >= 0 && counts[j] == 0; --j)
  {
    ;
  }

  for (int k = 0; k <= j; ++k)
  {
    if (k > 0)
    {
      pageName += '-';
    }
    if (counts[k] > 0)
    {
      pageName += std::to_string(counts[k]);
    }
    else
    {
      pageName += ToRoman(-counts[k]);
    }
  }

  tracePage->WriteLine("libdvi", fmt::format(T_("created page object '{0}'"), pageName));

  lastVisited = time(nullptr) + 60 * 60;
}

DviPageImpl::~DviPageImpl()
{
  try
  {
    Lock();
    AutoUnlockPage autoUnlock(this);
    FreeContents();
    autoClean = false;
    if (tracePage != nullptr)
    {
      tracePage->Close();
      tracePage = nullptr;
    }
    if (traceBitmap != nullptr)
    {
      traceBitmap->Close();
      traceBitmap = nullptr;
    }
  }
  catch (const exception&)
  {
  }
}

void DviPageImpl::AddItem(const DviItem& item)
{
  MIKTEX_ASSERT(IsLocked());
  dviItems.push_back(item);
  frozen = false;
}

void DviPageImpl::AddSpecial(DviSpecial* special)
{
  MIKTEX_ASSERT(IsLocked());
  dviSpecials.reserve(100);
  dviSpecials.push_back(special);
}

void DviPageImpl::AddRule(DviRuleImpl* rule)
{
  MIKTEX_ASSERT(IsLocked());
  if (pageMode != DviPageMode::Dvips)
  {
    dviRules.reserve(100);
    dviRules.push_back(rule);
  }
}

int DviPageImpl::GetNumberOfDviBitmaps(int shrinkFactor)
{
  MIKTEX_ASSERT(IsLocked());
  MAPNUMTOBOOL::const_iterator it = haveShrinkedRaster.find(shrinkFactor);
  if (it == haveShrinkedRaster.end() || !it->second)
  {
    MakeShrinkedRaster(shrinkFactor);
  }
  return static_cast<int>(shrinkedDviBitmaps[shrinkFactor].size());
}

int DviPageImpl::GetNumberOfDibChunks(int shrinkFactor)
{
  MIKTEX_ASSERT(IsLocked());
  MAPNUMTOBOOL::const_iterator it = haveShrinkedRaster.find(shrinkFactor);
  if (it == haveShrinkedRaster.end() || !it->second)
  {
    MakeShrinkedRaster(shrinkFactor);
  }
  return static_cast<int>(shrinkedDibChunks[shrinkFactor].size());
}

void DviPageImpl::MakeShrinkedRaster(int shrinkFactor)
{
  if (pageMode == DviPageMode::Dvips)
  {
    // make DIB chunks
    MakeDibChunks(shrinkFactor);
  }
  else
  {
    if (!dviItems.empty())
    {
      // sort items, color items
      Freeze(true);

      // make DVI bitmaps
      MakeDviBitmaps(shrinkFactor);
    }
  }
  haveShrinkedRaster[shrinkFactor] = true;
}

void DviPageImpl::MakeDviBitmaps(int shrinkFactor)
{
  MIKTEX_ASSERT(!dviItems.empty());
  MIKTEX_ASSERT(frozen);

  vector<DviItem>::iterator it = dviItems.begin();

  // initialize band
  vector<DviItem*> dviItemPointers;
  dviItemPointers.reserve(200);
  dviItemPointers.push_back(&*it);
  int bandBottom = it->GetBottomShr(shrinkFactor);

  // divide vertically
  for (; it != dviItems.end(); ++it)
  {
    int itemTop = it->GetTopShr(shrinkFactor);
    int itemBottom = it->GetBottomShr(shrinkFactor);

    if (itemTop > bandBottom + MaxVerticalWhite)
    {
      ProcessBand(shrinkFactor, dviItemPointers);
      bandBottom = itemBottom;
    }
    else
    {
      bandBottom = std::max(bandBottom, itemBottom);
    }

    dviItemPointers.push_back(&*it);
  }

  // process last band
  ProcessBand(shrinkFactor, dviItemPointers);
}

void DviPageImpl::ProcessBand(int shrinkFactor, vector<DviItem*>& dviItemPointers)
{
  MIKTEX_ASSERT(dviItemPointers.size() > 0);

  // sort band from left to right
  sort(dviItemPointers.begin(), dviItemPointers.end(), ItemLeftRight());

  vector<DviItem*>::iterator itItemPtr = dviItemPointers.begin();
  vector<DviItem*>::iterator itItemPtrMark = itItemPtr;

  int x1 = (*itItemPtr)->GetLeftShr(shrinkFactor);
  int x2 = (*itItemPtr)->GetLeftShr(shrinkFactor) + (*itItemPtr)->GetWidthShr(shrinkFactor) - 1;

  // initialize bitmap
  DviBitmap currentBitmap;
  currentBitmap.x = x1;
  currentBitmap.y = (*itItemPtr)->GetTopShr(shrinkFactor);
  currentBitmap.width = x2 - x1 + 1;
  currentBitmap.height = (*itItemPtr)->GetHeightShr(shrinkFactor);
  currentBitmap.pixels = nullptr;
  currentBitmap.monochrome = true;
  currentBitmap.foregroundColor = (*itItemPtr)->rgbForeground;
  currentBitmap.backgroundColor = (*itItemPtr)->rgbBackground;

  int bitmapBottom = currentBitmap.y + currentBitmap.height - 1;

  // divide horizontally
  for (; itItemPtr != dviItemPointers.end(); ++itItemPtr)
  {
    DviItem& item = **itItemPtr;

    int itemTop = item.GetTopShr(shrinkFactor);
    int item_left = item.GetLeftShr(shrinkFactor);
    int item_width = item.GetWidthShr(shrinkFactor);
    int itemBottom = item.GetBottomShr(shrinkFactor);

    if (x2 + MaxHorizontalWhite <= item_left
      || item.rgbForeground != currentBitmap.foregroundColor
      || item.rgbBackground != currentBitmap.backgroundColor)
    {
      // add the current bitmap
      if (currentBitmap.width > 0 && currentBitmap.height > 0)
      {
        MakeDviBitmap(shrinkFactor, currentBitmap, itItemPtrMark, itItemPtr);
      }

      itItemPtrMark = itItemPtr;

      x1 = item_left;
      x2 = item_left + item_width - 1;

      currentBitmap.x = x1;
      currentBitmap.y = itemTop;
      currentBitmap.width = x2 - x1 + 1;
      currentBitmap.height = item.GetHeightShr(shrinkFactor);
      currentBitmap.pixels = nullptr;
      currentBitmap.foregroundColor = item.rgbForeground;
      currentBitmap.backgroundColor = item.rgbBackground;

      bitmapBottom = currentBitmap.y + currentBitmap.height - 1;
    }
    else
    {
      x1 = std::min(x1, item_left);
      x2 = std::max(x2, item_left + item_width - 1);
      currentBitmap.x = std::min(currentBitmap.x, x1);
      currentBitmap.y = std::min(currentBitmap.y, itemTop);
      currentBitmap.width = std::max(currentBitmap.width, x2 - x1 + 1);
      if (bitmapBottom < itemBottom)
      {
        bitmapBottom = itemBottom;
      }
      currentBitmap.height = bitmapBottom - currentBitmap.y + 1;
    }
  }

  // add the current bitmap
  MakeDviBitmap(shrinkFactor, currentBitmap, itItemPtrMark, dviItemPointers.end());

  // clear the band, since we are ready
  dviItemPointers.clear();
}

void DviPageImpl::MakeDviBitmap(int shrinkFactor, DviBitmap& bitmap, vector<DviItem*>::iterator itItemPtrBegin, vector<DviItem*>::iterator itItemPtrEnd)
{
  MIKTEX_ASSERT(bitmap.pixels == nullptr);

  vector<DviBitmap>& bitmaps = shrinkedDviBitmaps[shrinkFactor];

  bitmaps.reserve(1000 / shrinkFactor);

  traceBitmap->WriteLine("libdvi", fmt::format(T_("bitmap {0}; bounding box: {1},{2},{3},{4}"), bitmaps.size(), bitmap.x, bitmap.y, bitmap.width, bitmap.height));

  int bytesPerLine = dviImpl->GetBytesPerLine(shrinkFactor, bitmap.width);
  MIKTEX_ASSERT(bytesPerLine > 0);
  bitmap.bytesPerLine = bytesPerLine;

  int rasterSize = (bytesPerLine * bitmap.height);
  bitmap.pixels = malloc(rasterSize);
  if (bitmap.pixels == nullptr)
  {
    OUT_OF_MEMORY("malloc");
  }
  size += rasterSize;
  totalSize += rasterSize;
  memset(const_cast<void*>(bitmap.pixels), 0, rasterSize);

  int bitsPerPixel = dviImpl->GetBitsPerPixel(shrinkFactor);
  int pixelsPerByte = dviImpl->GetPixelsPerByte(shrinkFactor);

  for (; itItemPtrBegin != itItemPtrEnd; ++itItemPtrBegin)
  {
    DviItem& item = **itItemPtrBegin;

    int itemLeft = item.GetLeftShr(shrinkFactor);
    int itemTop = item.GetTopShr(shrinkFactor);
    int itemHeight = item.GetHeightShr(shrinkFactor);
    int itemBottom = item.GetBottomShr(shrinkFactor);
    UNUSED(itemBottom);

    int itemWidth = item.GetWidthShr(shrinkFactor);
    unsigned long itemSize = dviImpl->GetBytesPerLine(shrinkFactor, itemWidth);

    int bitOffset = (itemLeft - bitmap.x) % pixelsPerByte;
    int bitShift = (bitOffset * bitsPerPixel) % 8;

    MIKTEX_ASSERT(itemTop >= bitmap.y);
    MIKTEX_ASSERT(itemLeft >= bitmap.x);
    MIKTEX_ASSERT(itemBottom <= (bitmap.y + (bitmap.height - 1)));
    MIKTEX_ASSERT(item.GetRightShr(shrinkFactor) <= bitmap.x + bitmap.width - 1);

    BYTE* raster = const_cast<BYTE*>(reinterpret_cast<const BYTE*>(bitmap.pixels));

    const BYTE* rasterChar = reinterpret_cast<const BYTE*>(item.pkChar->GetBitmap(shrinkFactor));

    int column = itemLeft - bitmap.x;

    for (int j = 0; j < static_cast<int>(itemSize); ++j)
    {
      int lineNum = (bitmap.height - (itemTop - bitmap.y) - 1);
      int rasterIdx = ((lineNum * bytesPerLine) + (column / pixelsPerByte) + j);
      for (int i = 0; i < itemHeight; ++i, --lineNum, rasterIdx -= bytesPerLine)
      {
        int idxRasterChar = i * itemSize + j;

        BYTE byte = rasterChar[idxRasterChar];
        BYTE mask = static_cast<BYTE>(byte >> bitShift);

        if (mask != 0)
        {
          MIKTEX_ASSERT(rasterIdx >= 0 && rasterIdx < rasterSize);
          raster[rasterIdx] |= mask;
        }

        if (bitOffset != 0 && (pixelsPerByte * j + pixelsPerByte - bitOffset < itemWidth))
        {
          unsigned mask = (byte << (8 - bitShift)) & 0xff;
          if (mask)
          {
            MIKTEX_ASSERT(rasterIdx + 1 < rasterSize);
            raster[rasterIdx + 1] |= mask;
          }
        }
      }
    }
  }

  bitmaps.push_back(bitmap);
}

void DviPageImpl::DestroyDviBitmaps()
{
  for (MAPNUMTOBITMAPVEC::iterator it = shrinkedDviBitmaps.begin(); it != shrinkedDviBitmaps.end(); ++it)
  {
    vector<DviBitmap>& bitmaps = it->second;
    for (size_t j = 0; j < bitmaps.size(); ++j)
    {
      if (bitmaps[j].pixels != nullptr)
      {
        size -= (bitmaps[j].bytesPerLine * bitmaps[j].height);
        totalSize -= (bitmaps[j].bytesPerLine * bitmaps[j].height);
        free(const_cast<void*>(bitmaps[j].pixels));
        bitmaps[j].pixels = nullptr;
      }
    }
    bitmaps.clear();
  }
  shrinkedDviBitmaps.clear();
}

void DviPageImpl::DestroyDibChunks()
{
  for (MAPNUMTODIBCHUNKVEC::iterator it = shrinkedDibChunks.begin(); it != shrinkedDibChunks.end(); ++it)
  {
    vector<shared_ptr<DibChunk>>& dibChunks = it->second;
    for (size_t j = 0; j < dibChunks.size(); ++j)
    {
      size -= dibChunks[j]->GetSize();
      totalSize -= dibChunks[j]->GetSize();
    }
  }
  shrinkedDibChunks.clear();
}

void DviPageImpl::FreeContents(bool keepSpecials, bool keepItems)
{
  MIKTEX_ASSERT(IsLocked());
  if (!keepSpecials)
  {
    for (size_t idx = 0; idx < dviSpecials.size(); ++idx)
    {
      DviSpecial* special = dviSpecials[idx];
      dviSpecials[idx] = nullptr;
      delete dynamic_cast<SpecialRoot*>(special);
    }
    dviSpecials.clear();
  }
  if (!keepItems)
  {
    dviItems.clear();
    for (size_t idx = 0; idx < dviRules.size(); ++idx)
    {
      DviRuleImpl* rule = dviRules[idx];
      dviRules[idx] = nullptr;
      delete rule;
    }
    dviRules.clear();
  }
  haveShrinkedRaster.clear();
  DestroyDviBitmaps();
  DestroyDibChunks();
  haveGraphicsInclusions.clear();
  graphicsInclusions.clear();
  frozen = false;
}

const char* DviPageImpl::GetName()
{
  return pageName.c_str();
}

void DviPageImpl::CheckRules()
{
  for (size_t idx = 0; idx < dviRules.size(); ++idx)
  {
    DviRuleImpl& rule = *dviRules[idx];
    rule.ClearFlag(DviRuleImpl::flblackboard);
    int ruleTop = rule.GetTopUns();
    int ruleBottom = rule.GetBottomUns();
    int ruleLeft = rule.GetLeftUns();
    int ruleRight = rule.GetRightUns();
    size_t nItems = dviItems.size();
    for (size_t j = 0; j < nItems; ++j)
    {
      DviItem& item = dviItems[j];
      int itemTop = item.GetTopUns();
      if (itemTop > ruleBottom)
      {
        break;
      }
      if (itemTop >= ruleTop
        && item.GetBottomUns() <= ruleBottom
        && item.GetLeftUns() >= ruleLeft
        && item.GetRightUns() <= ruleRight)
      {
        rule.SetFlag(DviRuleImpl::flblackboard);
        item.rgbBackground = rule.GetBackgroundColor();
      }
    }
  }
}

void DviPageImpl::Freeze(bool force)
{
  MIKTEX_ASSERT(IsLocked());
  if (!frozen || force)
  {
    sort(dviItems.begin(), dviItems.end(), ItemTopDown());
#if defined(MIKTEX_DEBUG)
    for (size_t idx = 1; idx < dviItems.size(); ++idx)
    {
      MIKTEX_ASSERT(dviItems[idx] >= dviItems[idx - 1]);
    }
#endif
    CheckRules();
    frozen = true;
  }
}

const DviBitmap& DviPageImpl::GetDviBitmap(int shrinkFactor, int idx)
{
  MIKTEX_ASSERT(IsLocked());
  MIKTEX_ASSERT(IsFrozen());
  MIKTEX_ASSERT(idx >= 0 && (static_cast<unsigned>(idx) < shrinkedDviBitmaps[shrinkFactor].size()));
  lastVisited = time(nullptr);
  return shrinkedDviBitmaps[shrinkFactor][idx];
}

shared_ptr<DibChunk> DviPageImpl::GetDibChunk(int shrinkFactor, int idx)
{
  MIKTEX_ASSERT(IsLocked());
  MIKTEX_ASSERT(IsFrozen());
  MIKTEX_ASSERT(idx >= 0 && (static_cast<unsigned>(idx) < shrinkedDibChunks[shrinkFactor].size()));
  lastVisited = time(nullptr);
  return shrinkedDibChunks[shrinkFactor][idx];
}

DviImpl* DviPageImpl::GetDviObject()
{
  return dviImpl;
}

TraceCallback* DviPageImpl::GetTraceCallback() const
{
  return dviImpl->GetTraceCallback();
}

void DviPageImpl::Error(const string& line)
{
  dviImpl->DviError(line);
}

int DviPageImpl::GetReg(int idx)
{
  MIKTEX_ASSERT(idx >= 0 && idx < 10);
  return counts[idx];
}

DviRule* DviPageImpl::GetRule(int idx)
{
  MIKTEX_ASSERT(IsLocked());
  int size = static_cast<int>(dviRules.size());
  if (idx >= size)
  {
    if (idx == size)
    {
      return nullptr;
    }
    else
    {
      MIKTEX_UNEXPECTED();
    }
  }
  else
  {
    return dviRules[idx];
  }
}

DviSpecial* DviPageImpl::GetSpecial(int idx)
{
  MIKTEX_ASSERT(IsLocked());
  int size = static_cast<int>(dviSpecials.size());
  if (idx >= size)
  {
    if (idx == size)
    {
      return nullptr;
    }
    else
    {
      MIKTEX_UNEXPECTED();
    }
  }
  else
  {
    return dviSpecials[idx];
  }
}

unsigned long DviPageImpl::GetBackgroundColor()
{
  if ((backgroundColor & 0x80000000) != 0)
  {
    return 0x00ffffff; // TODO
  }
  else
  {
    return backgroundColor;
  }
}

void DviPageImpl::Lock()
{
  pageMutex.lock();
  MIKTEX_ASSERT(nLocks >= 0);
  MIKTEX_ASSERT(nLocks < 1000);
  nLocks += 1;
}

void DviPageImpl::Unlock()
{
  MIKTEX_ASSERT(nLocks > 0);
  nLocks -= 1;
  try
  {
    if (nLocks == 0 && autoClean)
    {
      tracePage->WriteLine("libdvi", fmt::format(T_("auto-cleaning page '{0}'"), pageName));
      FreeContents(false, false);
    }
  }
  catch (const exception&)
  {
    pageMutex.unlock();
    throw;
  }
  pageMutex.unlock();
}

HypertexSpecial* DviPageImpl::GetNextHyperref(int& idx)
{
  HypertexSpecial* hyperSpecial;
  while ((hyperSpecial = GetNextSpecial<HypertexSpecial>(idx)) != 0)
  {
    if (!hyperSpecial->IsName())
    {
      return hyperSpecial;
    }
  }
  return nullptr;
}

void DviPageImpl::MakeDibChunks(int shrinkFactor)
{
  unique_ptr<Process> pDvips;
  unique_ptr<Process> pGhostscript;
  try
  {
    pDvips = StartDvips();
    thread dvipsTranscriptReader(&DviPageImpl::DvipsTranscriptReader, this);
    pGhostscript = StartGhostscript(shrinkFactor);
    thread ghostscriptTranscriptReader(&DviPageImpl::GhostscriptTranscriptReader, this);
    unique_ptr<DibChunker> pChunker(DibChunker::Create());
    const size_t CHUNK_SIZE = 1024 * 64;
    MIKTEX_ASSERT(IsLocked());
    dibShrinkFactor = shrinkFactor;
    while (pChunker->Process(DibChunker::Default, CHUNK_SIZE, this))
    {
    }
    dvipsTranscriptReader.join();
    ghostscriptTranscriptReader.join();
  }
  catch (const exception&)
  {
    dvipsErr.Close();
    if (dvipsOut.GetFile() != nullptr)
    {
      dvipsOut.Close();
    }
    gsOut.Close();
    gsErr.Close();
    throw;
  }
  dvipsErr.Close();
  if (dvipsOut.GetFile() != nullptr)
  {
    dvipsOut.Close();
  }
  gsOut.Close();
  gsErr.Close();
  tracePage->WriteLine("libdvi", T_("Dvips transcript:"));
  tracePage->WriteLine("libdvi", dvipsTranscript.c_str());
  tracePage->WriteLine("libdvi", T_("Ghostscript transcript:"));
  tracePage->WriteLine("libdvi", gsTranscript.c_str());
  MIKTEX_ASSERT(pDvips != nullptr);
  if (pDvips->get_ExitCode() != 0)
  {
    MIKTEX_FATAL_ERROR(T_("The page could not be rendered."));
  }
  MIKTEX_ASSERT(pGhostscript != nullptr);
  if (pGhostscript->get_ExitCode() != 0)
  {
    MIKTEX_FATAL_ERROR(T_("The page could not be rendered."));
  }
}

void DviPageImpl::DvipsTranscriptReader()
{
  try
  {
#define CHUNK_SIZE 64
    char buf[CHUNK_SIZE];
    dvipsTranscript = "";
    try
    {
      size_t n;
      while ((n = dvipsErr.Read(buf, CHUNK_SIZE)) > 0)
      {
        dvipsTranscript.append(buf, n);
      }
    }
    catch (const BrokenPipeException&)
    {
    }
  }
  catch (const MiKTeXException&)
  {
  }
  catch (const exception&)
  {
  }
}

void DviPageImpl::GhostscriptTranscriptReader()
{
  try
  {
#define CHUNK_SIZE 64
    char buf[CHUNK_SIZE];
    gsTranscript = "";
    try
    {
      size_t n;
      while ((n = gsErr.Read(buf, CHUNK_SIZE)) > 0)
      {
        gsTranscript.append(buf, n);
      }
    }
    catch (const BrokenPipeException&)
    {
    }
  }
  catch (const MiKTeXException&)
  {
  }
  catch (const exception&)
  {
  }
}

void DviPageImpl::OnNewChunk(shared_ptr<DibChunk> chunk)
{
  vector<shared_ptr<DibChunk>>& dibChunks = shrinkedDibChunks[dibShrinkFactor];

  const BITMAPINFO* bitmapInfo = chunk->GetBitmapInfo();

  traceBitmap->WriteLine("libdvi", fmt::format(T_("new DIB chunk {0}; bounding box: {1},{2},{3},{4}"), dibChunks.size(), chunk->GetX(), chunk->GetY(), bitmapInfo->bmiHeader.biWidth, bitmapInfo->bmiHeader.biHeight));

  size += chunk->GetSize();
  totalSize += chunk->GetSize();

  dibChunks.push_back(chunk);
}

size_t DviPageImpl::Read(void* data, size_t size)
{
  return gsOut.Read(data, size);
}

unique_ptr<Process> DviPageImpl::StartDvips()
{
  // locate dvips.exe
  PathName dvipsPath;
  if (!session->FindFile(MIKTEX_DVIPS_EXE, FileType::EXE, dvipsPath))
  {
    MIKTEX_UNEXPECTED();
  }

  // make Dvips command line
  vector<string> arguments{ dvipsPath.GetFileNameWithoutExtension().ToString() };
  arguments.push_back("-D" + std::to_string(dviImpl->GetResolution()));
  string metafontMode = dviImpl->GetMetafontMode();
  if (!metafontMode.empty())
  {
    arguments.push_back("-mode");
    arguments.push_back(metafontMode);
  }
  arguments.push_back("-f"s + "1");
  arguments.push_back("-p=" + std::to_string(pageIdx + 1));
  arguments.push_back("-l" + std::to_string(pageIdx + 1));
  if (!dviImpl->HavePaperSizeSpecial())
  {
    PaperSizeInfo paperSizeInfo = dviImpl->GetPaperSizeInfo();
    int width = paperSizeInfo.width;
    int height = paperSizeInfo.height;
    if (dviImpl->Landscape())
    {
      swap(width, height);
    }
    arguments.push_back("-T" + std::to_string(width) + "bp" + ',' + std::to_string(height) + "bp");
  }
  arguments.push_back("-MiKTeX:nolandscape");
  if (session->GetConfigValue("Dvips", "Pedantic", ConfigValue(false)).GetBool())
  {
    arguments.push_back("-MiKTeX:pedantic");
  }
  arguments.push_back("-MiKTeX:allowallpaths");
  arguments.push_back("-h"s + "gs_permitfilereading.pro");
  arguments.push_back(dviImpl->GetDviFileName().ToString());

  PathName dir(dviImpl->GetDviFileName());
  dir.MakeFullyQualified();
  dir.RemoveFileSpec();

  ProcessStartInfo processStartInfo;

  processStartInfo.Arguments = arguments;
  processStartInfo.FileName = dvipsPath.ToString();
  processStartInfo.RedirectStandardError = true;
  processStartInfo.RedirectStandardOutput = true;
  processStartInfo.WorkingDirectory = dir.ToString();

  unique_ptr<Process> pDvips(Process::Start(processStartInfo));

  dvipsOut.Attach(pDvips->get_StandardOutput());
  dvipsErr.Attach(pDvips->get_StandardError());

  return pDvips;
}

unique_ptr<Process> DviPageImpl::StartGhostscript(int shrinkFactor)
{
  PathName gsPath = session->GetGhostscript(nullptr);

  // make Ghostscript command line
  vector<string> arguments{ gsPath.GetFileNameWithoutExtension().ToString() };
  string res = std::to_string(static_cast<double>(dviImpl->GetResolution()) / shrinkFactor);
  arguments.push_back("-r" + res + 'x' + res);
  PaperSizeInfo paperSizeInfo = dviImpl->GetPaperSizeInfo();
  int width = paperSizeInfo.width;
  int height = paperSizeInfo.height;
  if (dviImpl->Landscape())
  {
    swap(width, height);
  }
  width = static_cast<int>(((dviImpl->GetResolution() * width) / 72.0) / shrinkFactor);
  height = static_cast<int>(((dviImpl->GetResolution() * height) / 72.0) / shrinkFactor);
  arguments.push_back("-g" + std::to_string(width) + 'x' + std::to_string(height));
  arguments.push_back("-sDEVICE="s + "bmp16m");
  arguments.push_back("-q");
  arguments.push_back("-dBATCH");
  arguments.push_back("-dNOPAUSE");
  arguments.push_back("-dDELAYSAFER");
  arguments.push_back("-sstdout="s + "%stderr");
  arguments.push_back("-dTextAlphaBits="s + "4");
  arguments.push_back("-dGraphicsAlphaBits="s + "4");
  arguments.push_back("-dDOINTERPOLATE");
  arguments.push_back("-sOutputFile="s + "-");
  arguments.push_back("-");

  ProcessStartInfo processStartInfo;

  processStartInfo.Arguments = arguments;
  processStartInfo.FileName = gsPath.ToString();
  processStartInfo.StandardInput = dvipsOut.GetFile();
  processStartInfo.RedirectStandardError = true;
  processStartInfo.RedirectStandardOutput = true;
  processStartInfo.WorkingDirectory = dviImpl->GetDviFileName().MakeFullyQualified().RemoveFileSpec().ToString();

  unique_ptr<Process> pGhostscript(Process::Start(processStartInfo));

  // close unused file stream (this prevents a hang situation)
  dvipsOut.Close();

  gsOut.Attach(pGhostscript->get_StandardOutput());
  gsErr.Attach(pGhostscript->get_StandardError());

  return pGhostscript;
}

int DviPageImpl::GetNumberOfGraphicsInclusions(int shrinkFactor)
{
  MIKTEX_ASSERT(IsLocked());
  MAPNUMTOBOOL::const_iterator it = haveGraphicsInclusions.find(shrinkFactor);
  if (it == haveGraphicsInclusions.end() || !it->second)
  {
    haveGraphicsInclusions[shrinkFactor] = true;
    if (pageMode != DviPageMode::Dvips)
    {
      DoPostScriptSpecials(shrinkFactor);
      DoGraphicsSpecials(shrinkFactor);
    }
  }
  return static_cast<int>(graphicsInclusions[shrinkFactor].size());
}

void DviPageImpl::DoPostScriptSpecials(int shrinkFactor)
{
  Ghostscript gs(dviImpl->GetTraceCallback());

  for (size_t idx = 0; idx < dviSpecials.size(); ++idx)
  {
    DviSpecial* special = dviSpecials[idx];
    if (special->GetType() == DviSpecialType::Psdef)
    {
      gs.AddDefinition(reinterpret_cast<PsdefSpecial*>(special));
    }
    else if (special->GetType() == DviSpecialType::Psfile)
    {
      if (!gs.IsOpen())
      {
        gs.Open(dviImpl, shrinkFactor);
      }
      if (gs.IsPageEmpty())
      {
        gs.BeginPage();
      }
      gs.DoSpecial(reinterpret_cast<PsfileSpecial*>(special));
    }
    else if (special->GetType() == DviSpecialType::Ps)
    {
      if (!gs.IsOpen())
      {
        gs.Open(dviImpl, shrinkFactor);
      }
      if (gs.IsPageEmpty())
      {
        gs.BeginPage();
      }
      gs.DoSpecial(reinterpret_cast<DvipsSpecial*>(special));
    }
  }

  if (!gs.IsPageEmpty())
  {
    gs.EndPage();
  }

  if (gs.IsOpen())
  {
    gs.Close();
  }

  shared_ptr<GraphicsInclusion> pGrinc;
  vector<shared_ptr<GraphicsInclusion> >& vec = graphicsInclusions[shrinkFactor];
  for (int idx = 0; (pGrinc = gs.GetGraphicsInclusion(idx)) != nullptr; ++idx)
  {
    vec.push_back(pGrinc);
  }
}

void DviPageImpl::DoGraphicsSpecials(int shrinkFactor)
{
  vector<shared_ptr<GraphicsInclusion> >& vec = graphicsInclusions[shrinkFactor];
  for (size_t idx = 0; idx < dviSpecials.size(); ++idx)
  {
    DviSpecial* special = dviSpecials[idx];
    if (special->GetType() == DviSpecialType::IncludeGraphics)
    {
      GraphicsSpecial* pGraphicsSpecial = reinterpret_cast<GraphicsSpecial*>(special);
      PathName fileName;
      if (!dviImpl->FindGraphicsFile(pGraphicsSpecial->GetFileName(), fileName))
      {
        MIKTEX_FATAL_ERROR_2(T_("The graphics file could not be found."), "path", pGraphicsSpecial->GetFileName());
      }
      ImageType imageType(ImageType::None);
      if (fileName.HasExtension(".bmp"))
      {
        imageType = ImageType::DIB;
      }
      else if (fileName.HasExtension(".emf") || fileName.HasExtension(".wmf"))
      {
        imageType = ImageType::EMF;
      }
      else
      {
        PathName tempFileName;
        if (!dviImpl->TryGetTempFile(fileName.GetData(), tempFileName))
        {
          if (!session->ConvertToBitmapFile(fileName, tempFileName, nullptr))
          {
            MIKTEX_FATAL_ERROR_2(T_("Could not convert to bitmap file."), "path", fileName.GetData());
          }
          dviImpl->RememberTempFile(fileName.GetData(), tempFileName);
        }
        imageType = ImageType::DIB;
        fileName = tempFileName;
      }
      int x = PixelShrink(shrinkFactor, pGraphicsSpecial->GetX());
      int y = PixelShrink(shrinkFactor, pGraphicsSpecial->GetY());
      int cx = pGraphicsSpecial->GetWidth();
      if (cx >= 0)
      {
        cx = PixelShrink(shrinkFactor, cx);
      }
      int cy = pGraphicsSpecial->GetHeight();
      if (cy >= 0)
      {
        cy = PixelShrink(shrinkFactor, cy);
      }
      vec.push_back(make_shared<GraphicsInclusionImpl>(imageType, fileName, false, x, y, cx, cy));
    }
  }
}

shared_ptr<GraphicsInclusion> DviPageImpl::GetGraphicsInclusion(int shrinkFactor, int idx)
{
  MIKTEX_ASSERT(IsLocked());
  MIKTEX_ASSERT(IsFrozen());
  MIKTEX_ASSERT(idx >= 0 && (static_cast<unsigned>(idx) < graphicsInclusions[shrinkFactor].size()));
  lastVisited = time(nullptr);
  return graphicsInclusions[shrinkFactor][idx];
}
