/* xetex-miktex.h:                                      -*- C++ -*-
   
   Copyright (C) 2007-2016 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#pragma once

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif

#include "xetexdefs.h"

#if ! defined(C4PEXTERN)
#  define C4PEXTERN extern
#endif

#include <miktex/TeXAndFriends/config.h>

#include "xetexd.h"
//#include "XeTeXOTMath.h"

#if ! defined(THEDATA)
#  define THEDATA(x) C4P_VAR(x)
#endif

#include <miktex/Core/FileType>

#include "xetex-version.h"

#if defined(MIKTEX_WINDOWS)
#  include "xetex.rc"
#endif

#include <miktex/Core/MD5>
#include <miktex/Core/Paths>
#include <miktex/TeXAndFriends/ETeXApp>

#if 0
namespace xetex {
#include "xetex.defaults.h"
}
#endif

#if !defined(MIKTEXHELP_XETEX)
#  include <miktex/Core/Help>
#endif

extern const char * papersize;
extern const char * outputdriver;

class XETEXCLASS :
  public MiKTeX::TeXAndFriends::ETeXApp
{
public:
  enum {
    OPT_OUTPUT_DRIVER = 10000,
    OPT_NO_PDF,
    OPT_PAPERSIZE,
  };

public:
  void AddOptions() override
  {
    ETeXApp::AddOptions();
    AddOption(MIKTEXTEXT("no-pdf\0Generate XDV (extended DVI) output rather than PDF."), OPT_NO_PDF);
    AddOption(MIKTEXTEXT("output-driver\0Use CMD as the XDV-to-PDF driver instead of xdvipdfmx."), OPT_OUTPUT_DRIVER, POPT_ARG_STRING, "CMD");
    AddOption(MIKTEXTEXT("papersize\0Set PDF media size to STRING"), OPT_PAPERSIZE, POPT_ARG_STRING, "STRING");
  }

public:
  bool ProcessOption(int opt, const std::string & optArg) override
  {
    bool done = true;
    switch (opt)
    {
    case OPT_NO_PDF:
      THEDATA(nopdfoutput) = true;
      break;
    case OPT_OUTPUT_DRIVER:
      outputdriver = strdup(optArg.c_str());
      break;
    case OPT_PAPERSIZE:
      papersize = strdup(optArg.c_str());
      break;
    default:
      done = ETeXApp::ProcessOption(opt, optArg);
      break;
    }
    return done;
  }

public:
  void AllocateMemory()
  {
    ETeXApp::AllocateMemory();
    size_t nFonts = THEDATA(fontmax) - constfontbase;
    Allocate("fontmapping", THEDATA(fontmapping), nFonts);
#if 1
    for (int idx = 0; idx < nFonts; ++ idx)
    {
      THEDATA(fontmapping)[idx] = 0;
    }
#endif
    Allocate ("fontlayoutengine", THEDATA(fontlayoutengine), nFonts);
    Allocate ("fontflags", THEDATA(fontflags), nFonts);
    Allocate ("fontletterspace", THEDATA(fontletterspace), nFonts);
  }

public:
  void FreeMemory()
  {
    ETeXApp::FreeMemory();
    Free(THEDATA(fontmapping));
    Free(THEDATA(fontlayoutengine));
    Free(THEDATA(fontflags));
    Free(THEDATA(fontletterspace));
  }

public:
  void Init(const std::string & programInvocationName) override
  {
    ETeXApp::Init(programInvocationName);
    EnableFeature(MiKTeX::TeXAndFriends::Feature::EightBitChars);
  }

public:
  const char * GetMemoryDumpFileName() const override
  {
    return "xetex.fmt";
  }

public:
  const char * GetInitProgramName() const override
  {
    return "inixetex";
  }
  
public:
  const char * GetVirginProgramName() const override
  {
    return "virxetex";
  }
  
public:
  const char * TheNameOfTheGame() const override
  {
    return "XeTeX";
  }

public:
  unsigned long GetHelpId() const override
  {
    return MIKTEXHELP_XETEX;
  }

public:
  virtual void GetLibraryVersions(std::vector<LibraryVersion> & versions) const override;
};

extern XETEXCLASS XETEXAPP;
#define THEAPP XETEXAPP
#include <miktex/TeXAndFriends/ETeXApp.inl>

#include <miktex/KPSE/Emulation>
#include <miktex/W2C/Emulation>

#include "xetex.h"
#include "synctex.h"

// special case: Web2C likes to add 1 to the nameoffile base address
inline utf8code * GetNameOfFileForWeb2C()
{
  return &((THEDATA(nameoffile))[-1]);
}

#define nameoffile (GetNameOfFileForWeb2C())

#define eqtb THEDATA(eqtb)
#define nopdfoutput THEDATA(nopdfoutput)
#define depthbase THEDATA(depthbase)
#define heightbase THEDATA(heightbase)
#define parambase THEDATA(parambase)
#define bufsize THEDATA(bufsize)
#define nativefonttypeflag THEDATA(nativefonttypeflag)
#define namelength THEDATA(namelength)
#define namelength16 THEDATA(namelength16)
#define first THEDATA(first)
#define last THEDATA(last)
#define maxbufstack THEDATA(maxbufstack)
#define fontflags THEDATA(fontflags)
#define xdvbuffer THEDATA(xdvbuffer)
#define loadedfontflags THEDATA(loadedfontflags)
#define fontinfo THEDATA(fontinfo)
#define fontletterspace THEDATA(fontletterspace)
#define fontsize THEDATA(fontsize)
#define loadedfontletterspace THEDATA(loadedfontletterspace)
#define fontarea THEDATA(fontarea)
#define buffer THEDATA(buffer)
#define mappedtext THEDATA(mappedtext)
#define nameoffile16 THEDATA(nameoffile16)
#define fontlayoutengine THEDATA(fontlayoutengine)
#define loadedfontmapping THEDATA(loadedfontmapping)
#define loadedfontdesignsize THEDATA(loadedfontdesignsize)
#define zmem THEDATA(zmem)
#define mem THEDATA(mem)
#define eq THEDATA(eq)
#define jobname THEDATA(jobname)
#define texmflogname THEDATA(logname)
#define synctexoption THEDATA(synctexoption)
#define synctexoffset THEDATA(synctexoffset)
#define curinput THEDATA(curinput)
#define totalpages THEDATA(totalpages)
#define curh THEDATA(curh)
#define curv THEDATA(curv)
#define rulewd THEDATA(rulewd)
#define ruleht THEDATA(ruleht)
#define ruledp THEDATA(ruledp)

#define c4p_sizeof(x) sizeof(x)
#define c4p_nullptr() nullptr
#define isnullptr(p) ((p) == nullptr)

#define afield aField
#define bfield bField
#define cfield cField
#define dfield dField
#define fix2d Fix2D
#define htfield htField
#define setpoint setPoint
#define txfield txField
#define tyfield tyField
#define wdfield wdField
#define d2fix D2Fix
#define xcoord xCoord
#define xfield xField
#define ycoord yCoord
#define yfield yField
#define zxnoverd xnoverd
#define zenddiagnostic enddiagnostic
#define zprintnl printnl
#define zprintchar printchar
#define zprintint printint
#define zprintscaled printscaled

int miktexloadpoolstrings(int size);

inline int loadpoolstrings(int size)
{
  return miktexloadpoolstrings(size);
}

inline char * gettexstring(strnumber stringNumber)
{
  int stringStart = MiKTeX::TeXAndFriends::GetTeXStringStart(stringNumber);
  int stringLength = MiKTeX::TeXAndFriends::GetTeXStringLength(stringNumber);
  size_t sizeUtf8 = stringLength * 4 + 1;
  char * lpsz = reinterpret_cast<char*>(xmalloc(sizeUtf8));
  return MiKTeX::TeXAndFriends::GetTeXString(lpsz, sizeUtf8, stringStart, stringLength);
}

inline strnumber maketexstring(const char * lpsz)
{
  std::wstring str = MiKTeX::Util::StringUtil::UTF8ToWideChar(lpsz);
  return THEAPP.MakeTeXString(str.c_str());
}

inline bool eightbitp()
{
  return THEAPP.Enable8BitCharsP();
}

#define insertsrcspecialauto miktexinsertsrcspecialauto

inline bool miktexopentfmfile(bytefile & f, const utf8code * lpszFileName_)
{
  const char * lpszFileName = reinterpret_cast<const char *>(lpszFileName_);
  MiKTeX::Core::PathName fileName(MiKTeX::Util::StringUtil::UTF8ToWideChar(lpszFileName));
  return MiKTeX::TeXAndFriends::OpenTFMFile(&f, fileName.GetData());
}

inline char * xmallocchararray(size_t size)
{
  return reinterpret_cast<char*>(xmalloc(size + 1));
}

inline int uopenin(unicodefile & f, int mode, int encodingData)
{
  return u_open_in(&f, mode, encodingData);
}

inline int otpartcount(const voidpointer a)
{
  return otpartcount((const GlyphAssembly*) a);
}

inline void printutf8str(const voidpointer str, int len)
{
  printutf8str((const unsigned char*) str, len);
}

inline int otpartisextender(const voidpointer a, int i)
{
  return otpartisextender((const GlyphAssembly*) a, i);
}

inline int otpartstartconnector(int f, const voidpointer a, int i)
{
  return otpartstartconnector(f, (const GlyphAssembly*) a, i);
}

inline int otpartfulladvance(int f, const voidpointer a, int i)
{
  return otpartfulladvance(f, (const GlyphAssembly*) a, i);
}

inline int otpartendconnector(int f, const voidpointer a, int i)
{
  return otpartendconnector(f, (const GlyphAssembly*) a, i);
}

inline int otpartglyph(const voidpointer a, int i)
{
  return otpartglyph((GlyphAssembly*) a, i);
}

inline void c4p_break(unicodefile & f)
{
  if (fflush(f->f) == EOF)
  {
      MIKTEX_FATAL_CRT_ERROR("flush");
  }
}

inline bool inputln(unicodefile & f, C4P::C4P_boolean bypassEndOfLine = true)
{
  bypassEndOfLine;
  return input_line(f);
}

inline void miktexopenin()
{
  static UFILE termin_file;
  if (THEDATA(termin) == nullptr)
    {
      THEDATA(termin) = &termin_file;
      THEDATA(termin)->f = stdin;
      THEDATA(termin)->savedChar = -1;
      THEDATA(termin)->skipNextLF = 0;
      THEDATA(termin)->encodingMode = UTF8;
      THEDATA(termin)->conversionData = nullptr;
      THEDATA(inputfile)[0] = THEDATA(termin);
    }
}

inline boolean usinggraphite(const voidpointer p)
{
  return usingGraphite(reinterpret_cast<XeTeXLayoutEngine>(p));
}

inline boolean usingopentype(const voidpointer p)
{
  return usingOpenType(reinterpret_cast<XeTeXLayoutEngine>(p));
}

inline void initgraphitebreaking(const voidpointer p, const utf16code * pUtf16Code, int len)
{
  initGraphiteBreaking(reinterpret_cast<XeTeXLayoutEngine>(p), reinterpret_cast<const UniChar *>(pUtf16Code), len);
}

inline int isopentypemathfont(const voidpointer  p)
{
  return isOpenTypeMathFont(reinterpret_cast<XeTeXLayoutEngine>(p));
}

#define findnextgraphitebreak findNextGraphiteBreak

template<class CharType> void printcstring(const CharType * lpsz)
{
  for (; *lpsz != 0; ++ lpsz)
  {
    printchar(*lpsz);
  }
}

inline void getmd5sum(strnumber s, boolean isFile)
{
  using namespace MiKTeX::Core;
  char * lpsz = gettexstring(s);
  std::string str = lpsz;
  free(lpsz);
  MD5 md5;
  if (isFile)
  {
    PathName file;
    if (!Session::Get()->FindFile(str, FileType::TEX, file))
    {
      return;
    }
    md5 = MD5::FromFile(file);
  }
  else
  {
    MD5Builder md5Builder;
    md5Builder.Init();
    md5Builder.Update(str.c_str(), str.length());
    md5 = md5Builder.Final();
  }
  for (const char & ch : Utils::Hexify(&md5[0], md5.size(), false))
  {
    THEDATA(strpool)[THEDATA(poolptr)] = (packedutf16code)ch;
    THEDATA(poolptr) = THEDATA(poolptr) + 1;
  }
}
