/* internal.h: internal DVI definitions                 -*- C++ -*-

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

#define MIKTEXDVIEXPORT MIKTEXDLLEXPORT

#define A86DAFD3F286A340B6BD420E71BCD906
#include "miktex/DVI/Dvi.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::DVI;
using namespace MiKTeX::Graphics;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace std;

class DviFont;
class DviImpl;

#define DEFAULT_PAGE_MODE DviPageMode::Pk

#include "Dib.h"

#if !defined(UNUSED)
#  if !defined(NDEBUG)
#    define UNUSED(x)
#  else
#    define UNUSED(x) static_cast<void>(x)
#  endif
#endif

#if !defined(UNUSED_ALWAYS)
#  define UNUSED_ALWAYS(x) static_cast<void>(x)
#endif

#define OUT_OF_MEMORY(function) MIKTEX_INTERNAL_ERROR()

#define FATAL_DVI_ERROR(message) throw DviException(string(), message, MiKTeXException::KVMAP(), MIKTEX_SOURCE_LOCATION())

#define FATAL_DVI_ERROR_2(message, ...) \
  throw DviException(string(), message, MiKTeXException::KVMAP(__VA_ARGS__), MIKTEX_SOURCE_LOCATION())

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).Get()

#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()

#define STATICFUNC(type) static type
#define INTERNALFUNC(type) type

#define BEGIN_CRITICAL_SECTION(mutex)                   \
  {                                                     \
    std::lock_guard<std::recursive_mutex>               \
      miktex_h__criticalSectionMonitor_ (mutex);

#define END_CRITICAL_SECTION()                          \
  }


struct DviPoint
{
public:
  DviPoint(int x = 0, int y = 0) :
    x(x),
    y(y)
  {
  }
public:
  int x;
public:
  int y;
};

enum class ImageType
{
  None,
  DIB,
  EMF
};

class GraphicsInclusionImpl : public GraphicsInclusion
{
public:
  GraphicsInclusionImpl()
  {
  }

public:
  GraphicsInclusionImpl(ImageType imageType, const PathName & fileName, bool temporary, int x, int y, int cx, int cy) :
    imageType(imageType),
    fileName(fileName),
    temporary(temporary),
    x(x),
    y(y),
    cx(cx),
    cy(cy)
  {
    switch (imageType)
    {
    case ImageType::DIB:
      pDib = make_unique<Dib>(fileName);
      break;
    case ImageType::EMF:
      hEmf = LoadEnhMetaFile(fileName);
      break;
    default:
      MIKTEX_FATAL_ERROR_2(T_("Unsupported image type."), "fileName", fileName.ToString());
    }
  }

public:
  virtual MIKTEXTHISCALL ~GraphicsInclusionImpl()
  {
    try
    {
      pDib = nullptr;
      if (hEmf != nullptr)
      {
        if (!DeleteEnhMetaFile(hEmf))
        {
          MIKTEX_FATAL_WINDOWS_ERROR("DeleteEnhMetaFile");
        }
        hEmf = nullptr;
      }
      if (temporary && fileName[0] != 0)
      {
        File::Delete(fileName, { FileDeleteOption::TryHard, FileDeleteOption::UpdateFndb });
      }
    }
    catch (const exception &)
    {
    }
  }

public:
  virtual void MIKTEXTHISCALL Render(HDC hdc)
  {
    switch (imageType)
    {
    case ImageType::DIB:
      pDib->Render(hdc, x, y, cx, cy);
      break;
    case ImageType::EMF:
    {
      RECT rect;
      rect.left = x;
      rect.top = y;
      rect.right = x + cx;
      rect.bottom = y + cy;
      if (!PlayEnhMetaFile(hdc, hEmf, &rect))
      {
        MIKTEX_UNEXPECTED();
      }
    }
    default:
      MIKTEX_UNEXPECTED();
    }
  }

private:
  HENHMETAFILE LoadEnhMetaFile(const PathName & fileName);

private:
  ImageType imageType = ImageType::None;

private:
  unique_ptr<Dib> pDib;

private:
  HENHMETAFILE hEmf = nullptr;

private:
  PathName fileName;

private:
  bool temporary = false;

private:
  int x = -1;

private:
  int y = -1;

private:
  int cx = -1;

private:
  int cy = -1;
};

class StdoutReader : public IRunProcessCallback
{
public:
  virtual bool MIKTEXTHISCALL OnProcessOutput(const void * pOutput, size_t n)
  {
    output.append(reinterpret_cast<const char *>(pOutput), n);
    return true;
  }

private:
  string output;
};

typedef unordered_map<int, DviFont *> FontMap;

typedef unordered_map<int, bool> MAPNUMTOBOOL;

typedef unordered_map<int, DviPoint> MAPNUMTOPOINT;

typedef unordered_map<int, vector<DviBitmap> > MAPNUMTOBITMAPVEC;

typedef unordered_map<int, vector<shared_ptr<DibChunk> > > MAPNUMTODIBCHUNKVEC;

typedef unordered_map<string, unique_ptr<TemporaryFile> > TempFileCollection;

typedef unordered_map<int, vector<shared_ptr<GraphicsInclusion> > > MAPNUMTOGRINCVEC;

#include "Dib.h"
#include "DviChar.h"
#include "DviFont.h"
#include "Ghostscript.h"
#include "PkChar.h"
#include "PkFont.h"
#include "PostScript.h"
#include "Tfm.h"
#include "VFont.h"
#include "VfChar.h"

int ScaleFix(int tfm, int z);

void ExpandBoundingBox(int llx, int lly, int urx, int ury);

int CalculateWidth(float width, const char * unit, int resolution);

struct CmykColor
{
  double cyan;
  double magenta;
  double yellow;
  double black;
  operator unsigned long();
};

struct RgbColor
{
  double red;
  double blue;
  double green;
  operator unsigned long();
};

const unsigned long rgbDefaultColor = 0; // black

struct HsbColor
{
  double hue;
  double saturation;
  double brightness;
  operator unsigned long();
};

template<class VALTYPE> class AutoRestore
{
public:
  AutoRestore(VALTYPE & val) :
    oldVal(val),
    pVal(&val)
  {
  }

public:
  ~AutoRestore()
  {
    *pVal = oldVal;
  }

private:
  VALTYPE oldVal;

private:
  VALTYPE * pVal;
};

struct DviItem
{
public:
  int x;

public:
  int y;

public:
  unsigned long rgbForeground;

public:
  unsigned long rgbBackground;

public:
  PkChar * pPkChar;

public:
  inline int GetWidthShr(int shrinkFactor);

public:
  inline int GetWidthUns();

public:
  inline int GetHeightShr(int shrinkFactor);

public:
  inline int GetHeightUns();

public:
  inline int GetLeftShr(int shrinkFactor);

public:
  inline int GetLeftUns();

public:
  int GetRightShr(int shrinkFactor)
  {
    return GetLeftShr(shrinkFactor) + GetWidthShr(shrinkFactor) - 1;
  }

public:
  int GetRightUns()
  {
    return GetLeftUns() + GetWidthUns() - 1;
  }

public:
  inline int GetTopShr(int shrinkFactor);

public:
  inline int GetTopUns();

public:
  int GetBottomShr(int shrinkFactor)
  {
    return GetTopShr(shrinkFactor) + GetHeightShr(shrinkFactor) - 1;
  }

public:
  int GetBottomUns()
  {
    return GetTopUns() + GetHeightUns() - 1;
  }
};

class InputStream
{
public:
  InputStream(const char * lpszFileName);

public:
  InputStream(const BYTE * pBytes, size_t nBytes);

public:
  ~InputStream();

public:
  bool Read(void * pBytes, size_t nBytes, bool allowEof = false);

public:
  short ReadByte()
  {
    Read(buffer, 1);
    return static_cast<short>(buffer[0] & 0xff);
  }

public:
  bool TryToReadByte(int & by);

public:
  short ReadSignedByte()
  {
    Read(buffer, 1);
    return buffer[0];
  }

public:
  long ReadPair()
  {
    Read(buffer, 2);
    return MakePair(buffer);
  }

public:
  long ReadSignedPair()
  {
    Read(buffer, 2);
    return MakeSignedPair(buffer);
  }

public:
  long ReadTrio()
  {
    Read(buffer, 3);
    return MakeTrio(buffer);
  }

public:
  long ReadSignedTrio()
  {
    Read(buffer, 3);
    return MakeSignedTrio(buffer);
  }

public:
  long ReadSignedQuad()
  {
    Read(buffer, 4);
    return MakeSignedQuad(buffer);
  }

public:
  void SkipBytes(long count);

public:
  void SetReadPosition(long offset, SeekOrigin origin);

public:
  long GetReadPosition();

public:
  bool IsEndOfStream()
  {
    if (pBytes != nullptr)
    {
      return idx == nBytes;
    }
    else
    {
      MIKTEX_UNEXPECTED();
    }
  }

protected:
  short MakeByte(const char * ptr)
  {
    return static_cast<short>(*ptr & 0xff);
  }

protected:
  short MakeSignedByte(const char * ptr)
  {
    return *ptr;
  }

protected:
  long MakePair(const char * ptr)
  {
    return (MakeByte(ptr) << 8 | MakeByte(ptr + 1)) & 0xffff;
  }

protected:
  long MakeSignedPair(const char * ptr)
  {
    return MakeSignedByte(ptr) << 8 | MakeByte(ptr + 1);
  }

protected:
  long MakeTrio(const char * ptr)
  {
    return ((static_cast<long>(MakeByte(ptr)) << 16) | MakePair(ptr + 1)) & 0x00ffffff;
  }

protected:
  long MakeSignedTrio(const char * ptr)
  {
    return (static_cast<long>(MakeSignedByte(ptr)) << 16) | MakePair(ptr + 1);
  }

protected:
  long MakeSignedQuad(const char * ptr)
  {
    return MakeSignedPair(ptr) << 16 | MakePair(ptr + 2);
  }

protected:
  char buffer[4];

protected:
  PathName fileName;

protected:
  FileStream stream;

protected:
  char * pBytes = nullptr;

protected:
  size_t nBytes = 0;

protected:
  size_t idx = 0;
};

class DviRuleImpl : public DviRule
{
public:
  virtual int MIKTEXTHISCALL GetLeft(int shrinkFactor);

public:
  virtual int MIKTEXTHISCALL GetRight(int shrinkFactor);

public:
  virtual int MIKTEXTHISCALL GetTop(int shrinkFactor);

public:
  virtual int MIKTEXTHISCALL GetBottom(int shrinkFactor);

public:
  virtual unsigned long MIKTEXTHISCALL GetBackgroundColor();

public:
  virtual unsigned long MIKTEXTHISCALL GetForegroundColor();

public:
  virtual bool MIKTEXTHISCALL IsBlackboard();

public:
  DviRuleImpl(DviImpl * pDviImpl, int x, int y, int width, int height, unsigned long rgb);

public:
  enum Flag { flblackboard = 1 };

public:
  void SetFlag(Flag fl)
  {
    flags |= static_cast<short>(fl);
  }

public:
  void ClearFlag(Flag fl)
  {
    flags &= ~static_cast<short>(fl);
  }

public:
  bool GetFlag(Flag fl)
  {
    return static_cast<short>(fl) & flags ? true : false;
  }

public:
  int GetLeftUns()
  {
    return x;
  }

public:
  int GetRightUns()
  {
    return x + width - 1;
  }

public:
  int GetTopUns()
  {
    return y - height + 1;
  }

public:
  int GetBottomUns()
  {
    return y;
  }

private:
  short flags = 0; // FIXME

private:
  int x;

private:
  int y;

private:
  int width;

private:
  int height;

private:
  unsigned long rgb;

private:
  DviImpl * pDviImpl = nullptr;
};

class DviPageImpl : public DviPage, public IDibChunkerCallback
{
public:
  virtual const DviBitmap & MIKTEXTHISCALL GetDviBitmap(int shrinkFactor, int idx);

public:
  virtual int MIKTEXTHISCALL GetNumberOfDviBitmaps(int shrinkFactor);

public:
  virtual DviSpecial * MIKTEXTHISCALL GetSpecial(int idx);

public:
  virtual DviRule * MIKTEXTHISCALL GetRule(int idx);

public:
  virtual int MIKTEXTHISCALL GetReg(int idx);

public:
  virtual const char * MIKTEXTHISCALL GetName();

public:
  virtual unsigned long MIKTEXTHISCALL GetBackgroundColor();

public:
  virtual void MIKTEXTHISCALL Lock();

public:
  virtual void MIKTEXTHISCALL Unlock();

public:
  virtual HypertexSpecial * MIKTEXTHISCALL GetNextHyperref(int & idx);

public:
  virtual shared_ptr<DibChunk> MIKTEXTHISCALL GetDibChunk(int shrinkFactor, int idx);

public:
  virtual int MIKTEXTHISCALL GetNumberOfDibChunks(int shrinkFactor);

public:
  virtual DviPageMode MIKTEXTHISCALL GetDviPageMode()
  {
    return pageMode;
  }

public:
  virtual int MIKTEXTHISCALL GetNumberOfGraphicsInclusions(int shrinkFactor);

public:
  virtual shared_ptr<GraphicsInclusion> MIKTEXTHISCALL GetGraphicsInclusion(int shrinkFactor, int idx);

public:
  DviImpl * GetDviObject();

public:
  void Freeze(bool force = false);

public:
  void FreeContents(bool keepSpecials = false, bool keepItems = false);

public:
  bool IsFrozen()
  {
    return frozen;
  }

public:
  bool IsLocked()
  {
    return nLocks > 0;
  }

public:
  size_t GetSize()
  {
    return size;
  }

public:
  static size_t GetTotalSize()
  {
    return totalSize;
  }

public:
  time_t GetTimeLastVisit()
  {
    return lastVisited;
  }

public:
  template<class T> T * GetNextSpecial(int & idx)
  {
    MIKTEX_ASSERT(IsLocked());
    MIKTEX_ASSERT(idx >= -1 && idx < static_cast<int>(dviSpecials.size()));
    for (int j = idx + 1; j < static_cast<int>(dviSpecials.size()); ++j)
    {
      T * pSpecial = dynamic_cast<T *>(dviSpecials[j]);
      if (pSpecial != nullptr)
      {
        idx = j;
        return pSpecial;
      }
    }
    return nullptr;
  }

private:
  DviPageImpl(DviImpl * pDvi, int pageIdx, DviPageMode pageMode, long readPosition, int c0 = 0, int c1 = 0, int c2 = 0, int c3 = 0, int c4 = 0, int c5 = 0, int c6 = 0, int c7 = 0, int c8 = 0, int c9 = 0);

private:
  void SetAutoClean(bool enabled = true)
  {
    autoClean = enabled;
  }

private:
  virtual ~DviPageImpl();

private:
  void AddItem(const DviItem & item);

private:
  void AddSpecial(DviSpecial * pSpecial);

private:
  void AddRule(DviRuleImpl * pRule);

private:
  void MakeShrinkedRaster(int shrinkFactor);

private:
  void MakeDviBitmaps(int shrinkFactor);

private:
  void ProcessBand(int shrinkFactor, vector<DviItem *> & vecDviItemPtr);

private:
  void MakeDviBitmap(int shrinkFactor, DviBitmap & bitmap, vector<DviItem *>::iterator ititemptrBegin, vector<DviItem *>::iterator ititemptrEnd);

private:
  void CheckRules();

private:
  void DestroyDviBitmaps();

private:
  void DestroyDibChunks();

private:
  void MakeDibChunks(int shrinkFactor);

private:
  unique_ptr<Process> StartDvips();

private:
  void DvipsTranscriptReader();

private:
  void GhostscriptTranscriptReader();

public:
  virtual size_t MIKTEXTHISCALL Read(void * pBuf, size_t size);

public:
  virtual void MIKTEXTHISCALL OnNewChunk(shared_ptr<DibChunk> pChunk);

private:
  unique_ptr<Process> StartGhostscript(int shrinkFactor);

private:
  int GetReadPosition()
  {
    return readPosition;
  }

private:
  inline int PixelShrink(int shrinkFactor, int pxl); // FIXME

private:
  inline int WidthShrink(int shrinkFactor, int pxl); // FIXME

private:
  void DoPostScriptSpecials(int shrinkFactor);

private:
  void DoGraphicsSpecials(int shrinkFactor);

private:
  shared_ptr<Session> session = Session::Get();

private:
  unique_ptr<TraceStream> tracePage;

private:
  unique_ptr<TraceStream> traceBitmap;

private:
  unsigned long backgroundColor = 0x80000000;

  // position within DVI file
private:
  long readPosition;

  // page numbers
private:
  int counts[10];

  // page index
private:
  int pageIdx;

  // item vector
private:
  vector<DviItem> dviItems;

  // special vector
private:
  vector<DviSpecial *> dviSpecials;

  // rule vector
private:
  vector<DviRuleImpl *> dviRules;

private:
  MAPNUMTOBOOL haveShrinkedRaster;

private:
  MAPNUMTOBOOL haveGraphicsInclusions;

private:
  MAPNUMTOGRINCVEC graphicsInclusions;

private:
  time_t lastVisited;

private:
  string pageName;

private:
  bool frozen = false;

private:
  MAPNUMTOBITMAPVEC shrinkedDviBitmaps;

private:
  MAPNUMTODIBCHUNKVEC shrinkedDibChunks;

private:
  int dibShrinkFactor;

private:
  string dvipsTranscript;

private:
  string gsTranscript;

private:
  DviImpl * pDviImpl;

private:
  mutex pageMutex;

private:
  size_t size = 0;

private:
  bool autoClean = false;

private:
  atomic_long nLocks = 0;

private:
  DviPageMode pageMode;

private:
  FileStream dvipsOut;

private:
  FileStream dvipsErr;

private:
  FileStream gsOut;

private:
  FileStream gsErr;

private:
  static size_t totalSize;

private:
  friend DviImpl; // FIXME
};

struct DviInfo
{
  string comment;
  long nPages;
  time_t lastWriteTime;
};

class DviImpl : public Dvi
{
public:
  virtual int MIKTEXTHISCALL GetNumberOfPages();

public:
  virtual bool MIKTEXTHISCALL GetSource(const DviPosition & pos, PathName & fileName, int * pLineNum);

public:
  virtual bool MIKTEXTHISCALL FindSource(const char * lpszFileName, int line, DviPosition & position);

public:
  virtual DviPage * MIKTEXTHISCALL GetPage(int pageidx);

public:
  virtual int MIKTEXTHISCALL GetMinPageNumber();

public:
  virtual int MIKTEXTHISCALL GetMaxPageNumber();

public:
  virtual int MIKTEXTHISCALL GetMagnification();

public:
  virtual int MIKTEXTHISCALL GetMaxH();

public:
  virtual int MIKTEXTHISCALL GetMaxV();

public:
  virtual PageStatus MIKTEXTHISCALL GetPageStatus(int pageidx);

public:
  virtual DviPage * MIKTEXTHISCALL GetLoadedPage(int pageno);

public:
  virtual bool MIKTEXTHISCALL FindHyperLabel(const char * lpszLabel, DviPosition & position);

public:
  virtual string MIKTEXTHISCALL GetStatusText();

public:
  virtual bool MIKTEXTHISCALL MakeFonts();

public:
  virtual vector<DviFontInfo>
    MIKTEXTHISCALL GetFontTable();

public:
  virtual PaperSizeInfo MIKTEXTHISCALL GetPaperSizeInfo()
  {
    return paperSizeInfo;
  }

public:
  virtual bool MIKTEXTHISCALL Landscape()
  {
    return landscape;
  }

public:
  virtual void MIKTEXTHISCALL Lock();

public:
  virtual void MIKTEXTHISCALL Unlock();

public:
  virtual void MIKTEXTHISCALL Scan();

private:
  DviImpl(const char * lpszFileName, const char * lpszMetafontMode, int resolution, int shrinkFactor, DviAccess access, DviPageMode pageMode, const PaperSizeInfo & paperSizeInfo, bool landscape);

private:
  virtual MIKTEXTHISCALL ~DviImpl();

public:
  virtual void MIKTEXTHISCALL Dispose();

private:
  friend class Dvi; // FIXME

private:
  void CheckCondition();

public:
  void Progress(DviNotification nf, const char * lpszFormat, ...);

public:
  int PixelShrink(int shrinkFactor, int pxl)
  {
    return pxl / shrinkFactor;
  }

public:
  int WidthShrink(int shrinkFactor, int pxl)
  {
    return PixelShrink(shrinkFactor, pxl + shrinkFactor - 1);
  }

public:
  int GetBitsPerPixel(int shrinkFactor)
  {
    return shrinkFactor == 1 ? 1 : 4;
  }

public:
  int GetBytesPerLine(int shrinkFactor, int width)
  {
    return (((width * GetBitsPerPixel(shrinkFactor)) + 31) / 32) * 4;
  }

public:
  int GetPixelsPerByte(int shrinkFactor)
  {
    return 8 / GetBitsPerPixel(shrinkFactor);
  }

public:
  int GetResolution()
  {
    return resolution;
  }

public:
  bool HaveLandscapeSpecial()
  {
    return haveLandscapeSpecial;
  }

public:
  bool HavePaperSizeSpecial()
  {
    return havePaperSizeSpecial;
  }

public:
  string
    GetMetafontMode()
  {
    return metafontMode;
  }

public:
  PathName GetDviFileName()
  {
    return dviFileName;
  }

public:
  MAPNUMTOPOINT & GetPoints()
  {
    return pointTable;
  }

public:
  unsigned GetLineWidth()
  {
    return lineWidth;
  }

public:
  unsigned long GetCurrentColor()
  {
    return currentColor;
  }

public:
  DviPageMode GetPageMode()
  {
    return pageMode;
  }

public:
  void RememberTempFile(const string & key, const PathName & path)
  {
    tempFiles[key] = TemporaryFile::Create(path);
  }

public:
  bool TryGetTempFile(const string & key, PathName & path)
  {
    TempFileCollection::const_iterator it = tempFiles.find(key);
    if (it != tempFiles.end())
    {
      path = it->second->GetPathName();
      return true;
    }
    else
    {
      return false;
    }
  }

public:
  bool FindGraphicsFile(const char * lpszFileName, PathName & result);

private:
  bool InterpretSpecial(DviPageImpl * pPage, int x, int y, InputStream & inputstream, DWORD p, DviSpecial * & pSpecial);

private:
  bool SetCurrentColor(const char * lpszColor);

private:
  bool ParseColorSpec(const char * lpsz, unsigned long & rgb);

private:
  void PushColor(unsigned long rgb);

private:
  void PopColor();

private:
  void ResetCurrentColor();

private:
  int FirstParam(InputStream & inputstream, int opcode); // FIXME

private:
  int PixelRound(int du); // FIXME

private:
  void DefineFont(InputStream & inputstream, int fontnum);

private:
  void DoPage(int pageidx);

private:
  bool DoNextCommand(InputStream & inputstream, DviPageImpl & page);

private:
  void SpecialCases(InputStream & inputstream, int opcode, int p, DviPageImpl & page); // FIXME

private:
  int RulePixels(int x); // FIXME

private:
  void FreeContents(bool keepFonts = false);

private:
  void PushState();

private:
  void PopState();

private:
  void GetFontTable(const FontMap & mapnumtofontptr, vector<DviFontInfo> & vec, int recursion);

private:
  bool MakeFonts(const FontMap & mapnumtofontptr, int recursion);

private:
  double GetConv()
  {
    return conv;
  }

private:
  DviSpecial * ProcessHtmlSpecial(DviPageImpl * ppage, int x, int y, const char * lpszSpecial);

private:
  float PatternToShadeLevel(const char * lpszTexture);

private:
  void GarbageCollector();

private:
  void PageLoader();

private:
  shared_ptr<Session> session = Session::Get();

private:
  HANDLE hByeByteEvent;

private:
  HANDLE hNewPageEvent;

private:
  HANDLE hScannedEvent;

private:
  int currentPageIdx = -1;

private:
  int direction = 1;

private:
  DviPageMode pageMode;

private:
  DviAccess dviAccess;

private:
  PaperSizeInfo paperSizeInfo;

private:
  bool havePaperSizeSpecial = false;

private:
  bool haveLandscapeSpecial = false;

private:
  bool landscape;

private:
  thread garbageCollectorThread;

private:
  thread pageLoaderThread;

  // resolution in dots per inch
private:
  int resolution;

  // converts DVI units to pixels
private:
  double conv;

  // converts TFM units to DVI units
private:
  double tfmConv;

  // current font object
private:
  DviFont * pCurrentFont;

  // current char object
private:
  class DviChar * pCurrentChar;

  // current VfChar object
private:
  class VfChar * pCurrentVfChar;

  // stated conversion ratio
private:
  int numerator, denominator;

  // magnification factor times 1000
private:
  int mag;

  // the value of abs(v) should probably not exceed this
private:
  int maxV;

  // the value of abs(h) should probably not exceed this
private:
  int maxH;

private:
  string metafontMode;

private:
  vector<class DviPageImpl *> pages;

private:
  FontMap * pFontMap;

private:
  int currentFontNumber;

private:
  int recursion = 0;

private:
  int minPageNumber;

private:
  int maxPageNumber;

private:
  DviInfo dviInfo;

private:
  PathName dviFileName;

private:
  PathName fqDviFileName;

private:
  int defaultShrinkFactor;

private:
  struct DviState
  {
  public:
    int h = 0, v = 0, w = 0, x = 0, y = 0, z = 0, hh = 0, vv = 0;
  };

  // current state
private:
  DviState currentState;

private:
  stack<DviState> stateStack;

private:
  stack<unsigned long> colorStack;

private:
  unsigned long currentColor;

  // last time the Dvi file was checked
private:
  clock_t lastChecked = 0;

private:
  bool hasDviFileChanged = false;

private:
  MAPNUMTOPOINT pointTable;

private:
  unsigned lineWidth = 0;

private:
  string progressStatus;

private:
  IDviCallback * pCallback = nullptr;

private:
  mutex statusTextMutex;

private:
  recursive_mutex dviMutex;

private:
  MiKTeXException miktexException;

private:
  exception stdException;

private:
  bool fatalMiKTeXError = false;

private:
  bool fatalError = false;

private:
  unique_ptr<TraceStream> trace_color;

private:
  unique_ptr<TraceStream> trace_dvifile;

private:
  unique_ptr<TraceStream> trace_dvipage;

private:
  unique_ptr<TraceStream> trace_error;

private:
  unique_ptr<TraceStream> trace_gc;

private:
  unique_ptr<TraceStream> trace_hypertex;

private:
  unique_ptr<TraceStream> trace_search;

private:
  TempFileCollection tempFiles;
};

class MIKTEXNOVTABLE SpecialRoot
{
public:
  SpecialRoot() :
    trace_error(TraceStream::Open(MIKTEX_TRACE_ERROR))
  {
  }

public:
  virtual ~SpecialRoot()
  {
    try
    {
      if (trace_error != nullptr)
      {
        trace_error->Close();
        trace_error = nullptr;
      }
    }
    catch (const exception &)
    {
    }
  }

protected:
  DviPageImpl * GetPage()
  {
    return pDviPageImpl;
  }

protected:
  DviPageImpl * pDviPageImpl = nullptr;

protected:
  int x, y;

protected:
  string specialString;

protected:
  DviSpecialType specialType = DviSpecialType::Unknown;

protected:
  unique_ptr<TraceStream> trace_error;
};

template<class T> class DviSpecialObject : public T
{
public:
  virtual ~DviSpecialObject()
  {
  }

public:
  virtual int MIKTEXTHISCALL GetX()
  {
    return x;
  }

public:
  virtual int MIKTEXTHISCALL GetY()
  {
    return y;
  }

public:
  virtual const char * MIKTEXTHISCALL GetXXX()
  {
    return specialString.c_str();
  }

public:
  virtual DviSpecialType MIKTEXTHISCALL GetType()
  {
    return specialType;
  }

public:
  DviSpecialObject(DviPageImpl * ppage, int x, int y, const char * lpszSpecial)
  {
    pDviPageImpl = ppage;
    this->x = x;
    this->y = y;
    if (lpszSpecial != nullptr)
    {
      specialString = lpszSpecial;
    }
    specialType = Parse();
  }
};

class MIKTEXNOVTABLE SolidLineSpecialImpl :
  public SolidLineSpecial,
  public SpecialRoot
{
public:
  unsigned long MIKTEXTHISCALL GetColor()
  {
    return color;
  }

public:
  unsigned MIKTEXTHISCALL GetWidth()
  {
    return width;
  }

public:
  int MIKTEXTHISCALL GetStartX()
  {
    return xStart;
  }

public:
  int MIKTEXTHISCALL GetStartY()
  {
    return yStart;
  }

public:
  int MIKTEXTHISCALL GetEndX()
  {
    return xEnd;
  }

public:
  int MIKTEXTHISCALL GetEndY()
  {
    return yEnd;
  }

protected:
  DviSpecialType Parse();

protected:
  unsigned long color;

protected:
  unsigned width;

protected:
  int xStart;

protected:
  int yStart;

protected:
  int xEnd;

protected:
  int yEnd;
};

class MIKTEXNOVTABLE SourceSpecialImpl :
  public SourceSpecial,
  public SpecialRoot
{
public:
  const char * MIKTEXTHISCALL GetFileName()
  {
    return fileName.c_str();
  }

public:
  int MIKTEXTHISCALL GetLineNum()
  {
    return lineNum;
  }

public:
  int MIKTEXTHISCALL GetId()
  {
    return id;
  }

protected:
  DviSpecialType Parse();

protected:
  int id;

protected:
  string fileName;

protected:
  int lineNum;
};

struct TpicContext
{
public:
  TpicContext()
  {
    Reset();
  }

public:
  void Reset()
  {
    tpicPath.clear();
    shade = 0.5;
    penSize = 5;
  }

public:
  TpicSpecial::path tpicPath;

public:
  float shade;

public:
  int penSize;
};

class MIKTEXNOVTABLE TpicSpecialRoot :
  public SpecialRoot
{
public:
  static TpicContext TpicSpecialRoot::tpicContext;
};

template<class T> class MIKTEXNOVTABLE TpicSpecialObject : public T
{
public:
  const TpicSpecial::path & MIKTEXTHISCALL GetPath()
  {
    return tpicPath;
  }

public:
  float MIKTEXTHISCALL GetShade()
  {
    return shade;
  }

public:
  int MIKTEXTHISCALL GetPenSize()
  {
    return penSize;
  }

protected:
  TpicSpecialObject()
  {
    tpicPath = TpicSpecialRoot::tpicContext.tpicPath;
    shade = TpicSpecialRoot::tpicContext.shade;
    penSize = TpicSpecialRoot::tpicContext.penSize;
    TpicSpecialRoot::tpicContext.Reset();
  }

protected:
  TpicSpecial::path tpicPath;

protected:
  float shade;

protected:
  int penSize;
};

class MIKTEXNOVTABLE TpicPolySpecialImpl :
  public TpicPolySpecial,
  public TpicSpecialRoot
{
public:
  OutlineStyle MIKTEXTHISCALL GetOutlineStyle(float & length)
  {
    length = m_length;
    return m_outline;
  }

public:
  bool MIKTEXTHISCALL IsSpline()
  {
    return m_bSpline;
  }

protected:
  DviSpecialType Parse();

protected:
  OutlineStyle m_outline = OutlineStyle::None;

protected:
  float m_length;

protected:
  bool m_bSpline;
};

class MIKTEXNOVTABLE TpicArcSpecialImpl :
  public TpicArcSpecial,
  public TpicSpecialRoot
{
public:
  int MIKTEXTHISCALL GetCx()
  {
    return cx;
  }

public:
  int MIKTEXTHISCALL GetCy()
  {
    return cy;
  }

public:
  int MIKTEXTHISCALL GetRx()
  {
    return m_rx;
  }

public:
  int MIKTEXTHISCALL GetRy()
  {
    return m_ry;
  }

public:
  float
    MIKTEXTHISCALL GetS()
  {
    return m_s;
  }

public:
  float
    MIKTEXTHISCALL GetE()
  {
    return m_e;
  }

public:
  bool MIKTEXTHISCALL HasOutline()
  {
    return m_bOutline;
  }

protected:
  DviSpecialType Parse();

protected:
  int cx, cy, m_rx, m_ry;

protected:
  float m_s, m_e;

protected:
  bool m_bOutline;
};

class MIKTEXNOVTABLE HyperTeXSpecialImpl :
  public HypertexSpecial,
  public SpecialRoot
{
public:
  HyperTeXSpecialImpl() :
    trace_hypertex(TraceStream::Open(MIKTEX_TRACE_DVIHYPERTEX))
  {
  }

public:
  virtual ~HyperTeXSpecialImpl()
  {
    try
    {
      if (trace_hypertex != nullptr)
      {
        trace_hypertex->Close();
        trace_hypertex = nullptr;
      }
    }
    catch (const exception &)
    {
    }
  }

public:
  const char * MIKTEXTHISCALL GetName()
  {
    return name.c_str();
  }

public:
  void MIKTEXTHISCALL GetBbox(int & llx, int & lly, int & urx, int & ury)
  {
    llx = this->llx;
    lly = this->lly;
    urx = this->urx;
    ury = this->ury;
  }

public:
  bool MIKTEXTHISCALL IsName()
  {
    return isName;
  }

protected:
  DviSpecialType Parse();

protected:
  string name;

protected:
  int llx, lly, urx, ury;

protected:
  bool isName;

protected:
  unique_ptr<TraceStream> trace_hypertex;

public:
  static struct State
  {
    string nameOrHref;
    string baseUrl;
    int llx, lly, urx, ury;
    bool isName;
    bool isHref;
  } state;
};

class MIKTEXNOVTABLE GraphicsSpecialImpl :
  public GraphicsSpecial,
  public SpecialRoot
{
public:
  const char * MIKTEXTHISCALL GetFileName()
  {
    return fileName.c_str();
  }

public:
  int MIKTEXTHISCALL GetWidth()
  {
    return width;
  }

public:
  int MIKTEXTHISCALL GetHeight()
  {
    return height;
  }

public:
  GraphicsSpecialAction MIKTEXTHISCALL GetAction()
  {
    return graphicsAction;
  }

protected:
  DviSpecialType Parse();

protected:
  string fileName;

protected:
  int width;

protected:
  int height;

protected:
  GraphicsSpecialAction graphicsAction = GraphicsSpecialAction::None;
};

class MIKTEXNOVTABLE PsdefSpecialImpl :
  public PsdefSpecial,
  public SpecialRoot
{
public:
  const char * MIKTEXTHISCALL GetDef()
  {
    return isFileName ? nullptr : str.c_str();
  }

public:
  const char * MIKTEXTHISCALL GetFileName()
  {
    return isFileName ? str.c_str() : nullptr;
  }

protected:
  DviSpecialType Parse();

protected:
  string str;

protected:
  bool isFileName;
};

class MIKTEXNOVTABLE DvipsSpecialImpl :
  public DvipsSpecial,
  public SpecialRoot
{
public:
  const char * MIKTEXTHISCALL GetString()
  {
    return isFileName ? nullptr : str.c_str();
  }

public:
  const char * MIKTEXTHISCALL GetFileName()
  {
    return isFileName ? str.c_str() : nullptr;
  }

public:
  bool MIKTEXTHISCALL GetProtection()
  {
    return protection;
  }

protected:
  DviSpecialType Parse();

protected:
  string str;

protected:
  bool protection;

protected:
  bool isFileName;
};

class MIKTEXNOVTABLE PaperSizeSpecialImpl :
  public PaperSizeSpecial,
  public SpecialRoot
{
public:
  PaperSizeInfo MIKTEXTHISCALL GetPaperSizeInfo()
  {
    return paperSizeInfo;
  }

protected:
  DviSpecialType Parse();

protected:
  PaperSizeInfo paperSizeInfo;
};

class MIKTEXNOVTABLE LandscapeSpecialImpl :
  public LandscapeSpecial,
  public SpecialRoot
{
protected:
  DviSpecialType Parse()
  {
    return DviSpecialType::Landscape;
  }
};

class MIKTEXNOVTABLE PsfileSpecialImpl :
  public PsfileSpecial,
  public SpecialRoot
{
public:
  const char * MIKTEXTHISCALL GetFileName()
  {
    return fileName.c_str();
  }

public:
  int MIKTEXTHISCALL GetHsize()
  {
    return hSize;
  }

public:
  int MIKTEXTHISCALL GetVsize()
  {
    return vSize;
  }

public:
  int MIKTEXTHISCALL GetHoffset()
  {
    return hOffset;
  }

public:
  int MIKTEXTHISCALL GetVoffset()
  {
    return vOffset;
  }

public:
  int MIKTEXTHISCALL GetHscale()
  {
    return hScale;
  }

public:
  int MIKTEXTHISCALL GetVscale()
  {
    return vScale;
  }

public:
  int MIKTEXTHISCALL GetAngke()
  {
    return angle;
  }

public:
  int MIKTEXTHISCALL GetLlx()
  {
    return llx;
  }

public:
  int MIKTEXTHISCALL GetLly()
  {
    return lly;
  }

public:
  int MIKTEXTHISCALL GetUrx()
  {
    return urx;
  }

public:
  int MIKTEXTHISCALL GetUry()
  {
    return ury;
  }

public:
  int MIKTEXTHISCALL GetRwi()
  {
    return rwi;
  }

public:
  int MIKTEXTHISCALL GetRhi()
  {
    return rhi;
  }

public:
  bool MIKTEXTHISCALL IsClipped()
  {
    return isClipped;
  }

public:
  bool MIKTEXTHISCALL HasHsize()
  {
    return hasHSize;
  }

public:
  bool MIKTEXTHISCALL HasVsize()
  {
    return hasVSize;
  }

public:
  bool MIKTEXTHISCALL HasHoffset()
  {
    return hasHOffset;
  }

public:
  bool MIKTEXTHISCALL HasVoffset()
  {
    return hasVOffset;
  }

public:
  bool MIKTEXTHISCALL HasHscale()
  {
    return hasHSale;
  }

public:
  bool MIKTEXTHISCALL HasVscale()
  {
    return hasVScale;
  }

public:
  bool MIKTEXTHISCALL HasAngle()
  {
    return hasAngle;
  }

public:
  bool MIKTEXTHISCALL HasLlx()
  {
    return hasLlx;
  }

public:
  bool MIKTEXTHISCALL HasLly()
  {
    return hasLLy;
  }

public:
  bool MIKTEXTHISCALL HasUrx()
  {
    return hasUrx;
  }

public:
  bool MIKTEXTHISCALL HasUry()
  {
    return hasUry;
  }

public:
  bool MIKTEXTHISCALL HasRwi()
  {
    return hasRwi;
  }

public:
  bool MIKTEXTHISCALL HasRhi()
  {
    return hasRhi;
  }

public:
  bool MIKTEXTHISCALL HasClipFlag()
  {
    return hasClipFlag;
  }

public:
  bool MIKTEXTHISCALL GetBoundingBox(float & left, float & bottom, float & right, float & top);

public:
  bool MIKTEXTHISCALL GetBoundingBox(int shrinkFactor, int & left, int & bottom, int & right, int & top);

protected:
  DviSpecialType Parse();

protected:
  string fileName;

protected:
  int hSize, vSize;

protected:
  int hOffset, vOffset;

protected:
  int hScale, vScale;

protected:
  int angle;

protected:
  int llx, lly, urx, ury;

protected:
  int rwi, rhi;

protected:
  bool isClipped;

protected:
  bool hasHSize : 1;

protected:
  bool hasVSize : 1;

protected:
  bool hasHOffset : 1;

protected:
  bool hasVOffset : 1;

protected:
  bool hasHSale : 1;

protected:
  bool hasVScale : 1;

protected:
  bool hasAngle : 1;

protected:
  bool hasLlx : 1;

protected:
  bool hasLLy : 1;

protected:
  bool hasUrx : 1;

protected:
  bool hasUry : 1;

protected:
  bool hasRwi : 1;

protected:
  bool hasRhi : 1;

protected:
  bool hasClipFlag : 1;
};

#include "inliners.h"
