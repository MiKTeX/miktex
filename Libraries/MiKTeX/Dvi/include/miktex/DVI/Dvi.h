/* miktex/DVI/Dvi.h: DVI interfaces                     -*- C++ -*-

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

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(B371214BB940374EA689C48ED44141CD)
#define B371214BB940374EA689C48ED44141CD

#include <miktex/First>
#include <miktex/Definitions>

// DLL import/export switch
#if !defined(A86DAFD3F286A340B6BD420E71BCD906)
#  define MIKTEXDVIEXPORT MIKTEXDLLEXPORT
#endif

// API decoration for exported member functions
#define MIKTEXDVICEEAPI(type) MIKTEXDVIEXPORT type MIKTEXCEECALL

#define MIKTEX_DVI_BEGIN_NAMESPACE              \
  namespace MiKTeX {                            \
    namespace DVI {

#define MIKTEX_DVI_END_NAMESPACE                \
    }                                           \
  }

#include <memory>
#include <string>
#include <vector>

#include <miktex/Core/AutoResource>
#include <miktex/Core/Exceptions>
#include <miktex/Core/PathName>
#include <miktex/Core/Session>

#include <miktex/Graphics/DibChunker>

MIKTEX_DVI_BEGIN_NAMESPACE;

class DviException : public MiKTeX::Core::MiKTeXException
{
public:
  MIKTEXDVIEXPORT MIKTEXTHISCALL DviException();

public:
  MIKTEXDVIEXPORT MIKTEXTHISCALL DviException(const std::string & programInvocationName, const std::string & message, const MiKTeX::Core::MiKTeXException::KVMAP & info, const MiKTeX::Core::SourceLocation & sourceLocation);
};

class DviPageNotFoundException : public DviException
{
public:
  MIKTEXDVIEXPORT MIKTEXTHISCALL DviPageNotFoundException();

public:
  MIKTEXDVIEXPORT MIKTEXTHISCALL DviPageNotFoundException(const std::string & programInvocationName, const std::string & message, const MiKTeX::Core::MiKTeXException::KVMAP & info, const MiKTeX::Core::SourceLocation & sourceLocation);
};

class DviFileInUseException : public DviException
{
public:
  MIKTEXDVIEXPORT MIKTEXTHISCALL DviFileInUseException();

public:
  MIKTEXDVIEXPORT MIKTEXTHISCALL DviFileInUseException(const std::string & programInvocationName, const std::string & message, const MiKTeX::Core::MiKTeXException::KVMAP & info, const MiKTeX::Core::SourceLocation & sourceLocation);
};

class MIKTEXNOVTABLE DviRule
{
public:
  virtual int MIKTEXTHISCALL GetLeft(int shrinkFactor) = 0;

public:
  virtual int MIKTEXTHISCALL GetRight(int shrinkFactor) = 0;

public:
  virtual int MIKTEXTHISCALL GetTop(int shrinkFactor) = 0;

public:
  virtual int MIKTEXTHISCALL GetBottom(int shrinkFactor) = 0;

public:
  virtual unsigned long MIKTEXTHISCALL GetBackgroundColor() = 0;

public:
  virtual unsigned long MIKTEXTHISCALL GetForegroundColor() = 0;

public:
  virtual bool MIKTEXTHISCALL IsBlackboard() = 0;
};

enum class DviSpecialType
{
  Unknown,
  Source,
  Psdef,
  Psfile,
  Ps,
  IncludeGraphics,
  Tpic,
  Hypertex,
  SolidLine,
  LogMessage,
  PaperSize,
  Landscape,
};

class MIKTEXNOVTABLE DviSpecial
{
public:
  virtual int MIKTEXTHISCALL GetX() = 0;

public:
  virtual int MIKTEXTHISCALL GetY() = 0;

public:
  virtual const char * MIKTEXTHISCALL GetXXX() = 0;

public:
  virtual DviSpecialType MIKTEXTHISCALL GetType() = 0;
};

class MIKTEXNOVTABLE SolidLineSpecial : public DviSpecial
{
public:
  virtual unsigned long MIKTEXTHISCALL GetColor() = 0;

public:
  virtual unsigned MIKTEXTHISCALL GetWidth() = 0;

public:
  virtual int MIKTEXTHISCALL GetStartX() = 0;

public:
  virtual int MIKTEXTHISCALL GetStartY() = 0;

public:
  virtual int MIKTEXTHISCALL GetEndX() = 0;

public:
  virtual int MIKTEXTHISCALL GetEndY() = 0;
};

class MIKTEXNOVTABLE SourceSpecial : public DviSpecial
{
public:
  virtual const char * MIKTEXTHISCALL GetFileName() = 0;

public:
  virtual int MIKTEXTHISCALL GetLineNum() = 0;

public:
  virtual int MIKTEXTHISCALL GetId() = 0;
};

class MIKTEXNOVTABLE TpicSpecial : public DviSpecial
{
public:
  struct point { int x, y; };

public:
  typedef std::vector<point> path;

public:
  virtual const path & MIKTEXTHISCALL GetPath() = 0;

public:
  virtual float MIKTEXTHISCALL GetShade() = 0;

public:
  virtual int MIKTEXTHISCALL GetPenSize() = 0;
};

enum class OutlineStyle
{
  None,
  Solid,
  Dashes,
  Dots
};

class MIKTEXNOVTABLE TpicPolySpecial : public TpicSpecial
{
public:
  virtual OutlineStyle MIKTEXTHISCALL GetOutlineStyle(float & length) = 0;

public:
  virtual bool MIKTEXTHISCALL IsSpline() = 0;
};

class MIKTEXNOVTABLE TpicArcSpecial : public TpicSpecial
{
public:
  virtual int MIKTEXTHISCALL GetCx() = 0;

public:
  virtual int MIKTEXTHISCALL GetCy() = 0;

public:
  virtual int MIKTEXTHISCALL GetRx() = 0;

public:
  virtual int MIKTEXTHISCALL GetRy() = 0;

public:
  virtual float MIKTEXTHISCALL GetS() = 0;

public:
  virtual float MIKTEXTHISCALL GetE() = 0;

public:
  virtual bool MIKTEXTHISCALL HasOutline() = 0;
};

class MIKTEXNOVTABLE HypertexSpecial : public DviSpecial
{
public:
  virtual const char * MIKTEXTHISCALL GetName() = 0;

public:
  virtual void MIKTEXTHISCALL GetBbox(int & llx, int & llr, int & urx, int & ury) = 0;

public:
  virtual bool MIKTEXTHISCALL IsName() = 0;
};

enum class GraphicsSpecialAction
{
  None,
  DviWinIsoScale,
  DviWinAnisoScale,
  DviWinCenter,
  EmGraph
};

class MIKTEXNOVTABLE GraphicsSpecial : public DviSpecial
{
public:
  virtual const char * MIKTEXTHISCALL GetFileName() = 0;

public:
  virtual int MIKTEXTHISCALL GetWidth() = 0;

public:
  virtual int MIKTEXTHISCALL GetHeight() = 0;

public:
  virtual GraphicsSpecialAction MIKTEXTHISCALL GetAction() = 0;
};

class MIKTEXNOVTABLE PsdefSpecial : public DviSpecial
{
public:
  virtual const char * MIKTEXTHISCALL GetDef() = 0;

public:
  virtual const char * MIKTEXTHISCALL GetFileName() = 0;
};

class MIKTEXNOVTABLE DvipsSpecial : public DviSpecial
{
public:
  virtual const char * MIKTEXTHISCALL GetString() = 0;

public:
  virtual const char * MIKTEXTHISCALL GetFileName() = 0;

public:
  virtual bool MIKTEXTHISCALL GetProtection() = 0;
};

class MIKTEXNOVTABLE PaperSizeSpecial : public DviSpecial
{
public:
  virtual MiKTeX::Core::PaperSizeInfo MIKTEXTHISCALL GetPaperSizeInfo() = 0;
};

class MIKTEXNOVTABLE LandscapeSpecial : public DviSpecial
{
};

class MIKTEXNOVTABLE PsfileSpecial : public DviSpecial
{
public:
  virtual const char * MIKTEXTHISCALL GetFileName() = 0;

public:
  virtual int MIKTEXTHISCALL GetHsize() = 0;

public:
  virtual int MIKTEXTHISCALL GetVsize() = 0;

public:
  virtual int MIKTEXTHISCALL GetHoffset() = 0;

public:
  virtual int MIKTEXTHISCALL GetVoffset() = 0;

public:
  virtual int MIKTEXTHISCALL GetHscale() = 0;

public:
  virtual int MIKTEXTHISCALL GetVscale() = 0;

public:
  virtual int MIKTEXTHISCALL GetAngke() = 0;

public:
  virtual int MIKTEXTHISCALL GetLlx() = 0;

public:
  virtual int MIKTEXTHISCALL GetLly() = 0;

public:
  virtual int MIKTEXTHISCALL GetUrx() = 0;

public:
  virtual int MIKTEXTHISCALL GetUry() = 0;

public:
  virtual int MIKTEXTHISCALL GetRwi() = 0;

public:
  virtual int MIKTEXTHISCALL GetRhi() = 0;

public:
  virtual bool MIKTEXTHISCALL IsClipped() = 0;

public:
  virtual bool MIKTEXTHISCALL HasHsize() = 0;

public:
  virtual bool MIKTEXTHISCALL HasVsize() = 0;

public:
  virtual bool MIKTEXTHISCALL HasHoffset() = 0;

public:
  virtual bool MIKTEXTHISCALL HasVoffset() = 0;

public:
  virtual bool MIKTEXTHISCALL HasHscale() = 0;

public:
  virtual bool MIKTEXTHISCALL HasVscale() = 0;

public:
  virtual bool MIKTEXTHISCALL HasAngle() = 0;

public:
  virtual bool MIKTEXTHISCALL HasLlx() = 0;

public:
  virtual bool MIKTEXTHISCALL HasLly() = 0;

public:
  virtual bool MIKTEXTHISCALL HasUrx() = 0;

public:
  virtual bool MIKTEXTHISCALL HasUry() = 0;

public:
  virtual bool MIKTEXTHISCALL HasRwi() = 0;

public:
  virtual bool MIKTEXTHISCALL HasRhi() = 0;

public:
  virtual bool MIKTEXTHISCALL HasClipFlag() = 0;

public:
  virtual bool MIKTEXTHISCALL GetBoundingBox(float & left, float & bottom, float & right, float & top) = 0;

public:
  virtual bool MIKTEXTHISCALL GetBoundingBox(int shrinkFactor, int & left, int & bottom, int & right, int & top) = 0;
};

class MIKTEXNOVTABLE GraphicsInclusion
{
public:
  virtual MIKTEXTHISCALL ~GraphicsInclusion() = 0;

public:
  virtual void MIKTEXTHISCALL Render(HDC hdc) = 0;
};

struct DviBitmap
{
  const void * pPixels;
  int x;
  int y;
  int width;
  int height;
  int bytesPerLine;
  bool monochrome;
  unsigned long foregroundColor;
  unsigned long backgroundColor;
};

enum class DviPageMode
{
  None,
  Pk,
  Dvips
};

class MIKTEXNOVTABLE DviPage
{
public:
  virtual const DviBitmap & MIKTEXTHISCALL GetDviBitmap(int shrinkFactor, int idx) = 0;

public:
  virtual int MIKTEXTHISCALL GetNumberOfDviBitmaps(int shrinkFactor) = 0;

public:
  virtual DviSpecial * MIKTEXTHISCALL GetSpecial(int idx) = 0;

public:
  virtual DviRule * MIKTEXTHISCALL GetRule(int idx) = 0;

public:
  virtual int MIKTEXTHISCALL GetReg(int idx) = 0;

public:
  virtual const char * MIKTEXTHISCALL GetName() = 0;

public:
  virtual unsigned long MIKTEXTHISCALL GetBackgroundColor() = 0;

public:
  virtual void MIKTEXTHISCALL Lock() = 0;

public:
  virtual void MIKTEXTHISCALL Unlock() = 0;

public:
  virtual HypertexSpecial * MIKTEXTHISCALL GetNextHyperref(int & idx) = 0;

public:
  virtual std::shared_ptr<MiKTeX::Graphics::DibChunk> MIKTEXTHISCALL GetDibChunk(int shrinkFactor, int idx) = 0;

public:
  virtual int MIKTEXTHISCALL GetNumberOfDibChunks(int shrinkFactor) = 0;

public:
  virtual DviPageMode MIKTEXTHISCALL GetDviPageMode() = 0;

public:
  virtual int MIKTEXTHISCALL GetNumberOfGraphicsInclusions(int shrinkFactor) = 0;

public:
  virtual std::shared_ptr<GraphicsInclusion> MIKTEXTHISCALL GetGraphicsInclusion(int shrinkFactor, int idx) = 0;

};

struct DviFontInfo
{
  bool notLoadable;
  bool isVirtualFont;
  std::string name;
  std::string fileName;
  std::string transcript;
  std::string comment;
};

enum class PageStatus
{
  Unknown,
  NotLoaded,
  Lost,
  Changed,
  Loaded
};

enum class DviNotification
{
  None,
  BeginLoadPage,
  BeginLoadFont,
};

enum class DviAccess
{
  None,
  Random,
  Sequential,
};

class MIKTEXNOVTABLE IDviCallback
{
public:
  virtual void MIKTEXTHISCALL OnProgress(DviNotification nf) = 0;
};

struct DviPosition
{
public:
  DviPosition()
  {
  }

public:
  DviPosition(int pageIdx, int x, int y) :
    pageIdx(pageIdx),
    x(x),
    y(y)
  {
  }

public:
  int pageIdx = 0;

public:
  int x = 0;

public:
  int y = 0;
};

class MIKTEXNOVTABLE Dvi
{
public:
  virtual MIKTEXTHISCALL ~Dvi() = 0;

public:
  virtual void MIKTEXTHISCALL Dispose() = 0;

public:
  static MIKTEXDVICEEAPI(Dvi*) Create(const char * lpszFileName, const char * lpszMetafontMode, int resolution, int shrinkFactor, DviAccess access, IDviCallback * pCallback);

public:
  static MIKTEXDVICEEAPI(Dvi*) Create(const char * lpszFileName, const char * lpszMetafontMode, int resolution, int shrinkFactor, DviAccess access, DviPageMode pageMode, const MiKTeX::Core::PaperSizeInfo & defaultPaperSizeInfo, bool landscape, IDviCallback * pCallback);

public:
  virtual int MIKTEXTHISCALL GetNumberOfPages() = 0;

public:
  virtual bool MIKTEXTHISCALL GetSource(const DviPosition & pos, MiKTeX::Core::PathName & fileName, int * pLineNum = 0) = 0;

public:
  virtual bool MIKTEXTHISCALL FindSource(const char * lpszFileName, int line, DviPosition & position) = 0;

public:
  virtual DviPage * MIKTEXTHISCALL GetPage(int pageIdx) = 0;

public:
  virtual int MIKTEXTHISCALL GetMinPageNumber() = 0;

public:
  virtual int MIKTEXTHISCALL GetMaxPageNumber() = 0;

public:
  virtual int MIKTEXTHISCALL GetMagnification() = 0;

public:
  virtual int MIKTEXTHISCALL GetMaxH() = 0;

public:
  virtual int MIKTEXTHISCALL GetMaxV() = 0;

public:
  virtual PageStatus MIKTEXTHISCALL GetPageStatus(int pageIdx) = 0;

public:
  virtual DviPage * MIKTEXTHISCALL GetLoadedPage(int pageIdx) = 0;

public:
  virtual bool MIKTEXTHISCALL FindHyperLabel(const char * lpszLabel, DviPosition & position) = 0;

public:
  virtual std::string MIKTEXTHISCALL GetStatusText() = 0;

public:
  virtual bool MIKTEXTHISCALL MakeFonts() = 0;

public:
  virtual std::vector<DviFontInfo> MIKTEXTHISCALL GetFontTable() = 0;

public:
  virtual MiKTeX::Core::PaperSizeInfo MIKTEXTHISCALL GetPaperSizeInfo() = 0;

public:
  virtual bool MIKTEXTHISCALL Landscape() = 0;

public:
  virtual void MIKTEXTHISCALL Lock() = 0;

public:
  virtual void MIKTEXTHISCALL Unlock() = 0;

public:
  virtual void MIKTEXTHISCALL Scan() = 0;
};

class UnlockDviPage_
{
public:
  void operator() (DviPage * p)
  {
    p->Unlock();
  }
};

typedef MiKTeX::Core::AutoResource<DviPage *, UnlockDviPage_> AutoUnlockPage;

MIKTEX_DVI_END_NAMESPACE;

#undef MIKTEX_DVI_BEGIN_NAMESPACE
#undef MIKTEX_DVI_END_NAMESPACE

#endif
