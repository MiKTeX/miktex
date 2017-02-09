/* xetex-miktex.h:                                      -*- C++ -*-
   
   Copyright (C) 2007-2017 Christian Schenk

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

#include "xetex-miktex-config.h"

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif

#include <miktex/Core/FileType>
#include <miktex/Core/MD5>
#include <miktex/Core/Paths>
#include <miktex/KPSE/Emulation>
#include <miktex/TeXAndFriends/CharacterConverterImpl>
#include <miktex/TeXAndFriends/InitFinalizeImpl>
#include <miktex/TeXAndFriends/InputOutputImpl>
#include <miktex/TeXAndFriends/ETeXApp>
#include <miktex/TeXAndFriends/ETeXMemoryHandlerImpl>
#include <miktex/W2C/Emulation>

#include "xetexd.h"

#include "xetex-version.h"

#if 0
namespace xetex {
#include "xetex.defaults.h"
}
#endif

#if defined(MIKTEX_WINDOWS)
#  include "xetex.rc"
#endif

#if !defined(MIKTEXHELP_XETEX)
#  include <miktex/Core/Help>
#endif

extern XETEXPROGCLASS XETEXPROG;

class MemoryHandlerImpl :
  public MiKTeX::TeXAndFriends::ETeXMemoryHandlerImpl<XETEXPROGCLASS>
{
public:
  MemoryHandlerImpl(XETEXPROGCLASS& program, MiKTeX::TeXAndFriends::TeXMFApp& texmfapp) :
    ETeXMemoryHandlerImpl<XETEXPROGCLASS>(program, texmfapp)
  {
  }

public:
  void Allocate(const std::unordered_map<std::string, int>& userParams) override
  {
    ETeXMemoryHandlerImpl<XETEXPROGCLASS>::Allocate(userParams);
    size_t nFonts = program.fontmax - program.constfontbase;
    AllocateArray("fontmapping", program.fontmapping, nFonts);
#if 1
    for (int idx = 0; idx < nFonts; ++idx)
    {
      program.fontmapping[idx] = 0;
    }
#endif
    AllocateArray("fontlayoutengine", program.fontlayoutengine, nFonts);
    AllocateArray("fontflags", program.fontflags, nFonts);
    AllocateArray("fontletterspace", program.fontletterspace, nFonts);
  }

public:
  void Free() override
  {
    ETeXMemoryHandlerImpl<XETEXPROGCLASS>::Free();
    FreeArray("fontmapping", program.fontmapping);
    FreeArray("fontlayoutengine", program.fontlayoutengine);
    FreeArray("fontflags", program.fontflags);
    FreeArray("fontletterspace", program.fontletterspace);
  }

public:
  void Check() override
  {
    ETeXMemoryHandlerImpl<XETEXPROGCLASS>::Check();
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontmapping);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontlayoutengine);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontflags);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontletterspace);
  }
};

class XETEXAPPCLASS :
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
  bool ProcessOption(int opt, const std::string& optArg) override
  {
    bool done = true;
    switch (opt)
    {
    case OPT_NO_PDF:
      XETEXPROG.nopdfoutput = true;
      break;
    case OPT_OUTPUT_DRIVER:
      extern const char* outputdriver;
      outputdriver = strdup(optArg.c_str());
      break;
    case OPT_PAPERSIZE:
      extern const char* papersize;
      papersize = strdup(optArg.c_str());
      break;
    default:
      done = ETeXApp::ProcessOption(opt, optArg);
      break;
    }
    return done;
  }

private:
  MiKTeX::TeXAndFriends::CharacterConverterImpl<XETEXPROGCLASS> charConv{ XETEXPROG };

private:
  MiKTeX::TeXAndFriends::InitFinalizeImpl<XETEXPROGCLASS> initFinalize{ XETEXPROG };

private:
  MiKTeX::TeXAndFriends::InputOutputImpl<XETEXPROGCLASS> inputOutput{ XETEXPROG };

private:
  MemoryHandlerImpl memoryHandler{ XETEXPROG, *this };

public:
  void Init(const std::string& programInvocationName) override
  {
    SetCharacterConverter(&charConv);
    SetInitFinalize(&initFinalize);
    SetInputOutput(&inputOutput);
    SetTeXMFMemoryHandler(&memoryHandler);
    ETeXApp::Init(programInvocationName);
    EnableFeature(MiKTeX::TeXAndFriends::Feature::EightBitChars);
  }

public:
  MiKTeX::Core::PathName GetMemoryDumpFileName() const override
  {
    return "xetex.fmt";
  }

public:
  std::string GetInitProgramName() const override
  {
    return "inixetex";
  }
  
public:
  std::string GetVirginProgramName() const override
  {
    return "virxetex";
  }
  
public:
  std::string TheNameOfTheGame() const override
  {
    return "XeTeX";
  }

public:
  unsigned long GetHelpId() const override
  {
    return MIKTEXHELP_XETEX;
  }

public:
  void GetLibraryVersions(std::vector<MiKTeX::Core::LibraryVersion>& versions) const override;
};

using halfword = XETEXPROGCLASS::halfword;
using memoryword = XETEXPROGCLASS::memoryword;
using scaled = XETEXPROGCLASS::scaled;

constexpr auto filenamesize = XETEXPROGCLASS::filenamesize;

extern XETEXPROGCLASS::unicodescalar*& buffer;
extern C4P::C4P_integer& bufsize;
extern XETEXPROGCLASS::scaled& curh;
extern XETEXPROGCLASS::instaterecord& curinput;
extern XETEXPROGCLASS::scaled& curv;
extern C4P::C4P_integer*& depthbase;
extern XETEXPROGCLASS::memoryword*& eqtb;
extern C4P::C4P_signed32& first;
extern XETEXPROGCLASS::strnumber*& fontarea;
extern char*& fontflags;
extern voidpointer*& fontlayoutengine;
extern XETEXPROGCLASS::scaled*& fontletterspace;
extern XETEXPROGCLASS::scaled*& fontsize;
extern C4P::C4P_integer*& heightbase;
extern XETEXPROGCLASS::strnumber& jobname;
extern C4P::C4P_signed32& last;
extern XETEXPROGCLASS::scaled& loadedfontdesignsize;
extern char& loadedfontflags;
extern XETEXPROGCLASS::scaled& loadedfontletterspace;
extern voidpointer& loadedfontmapping;
extern XETEXPROGCLASS::utf16code*& mappedtext;
extern C4P::C4P_signed32& maxbufstack;
extern C4P::C4P_signed16& namelength;
extern C4P::C4P_signed16& namelength16;
extern XETEXPROGCLASS::utf16code*& nameoffile16;
extern C4P::C4P_integer& nativefonttypeflag;
extern C4P::C4P_boolean& nopdfoutput;
extern XETEXPROGCLASS::scaled& ruledp;
extern XETEXPROGCLASS::scaled& ruleht;
extern XETEXPROGCLASS::scaled& rulewd;
extern C4P::C4P_integer& synctexoffset;
extern C4P::C4P_integer& synctexoption;
extern XETEXPROGCLASS::strnumber& texmflogname;
extern C4P::C4P_integer& totalpages;
extern char*& xdvbuffer;
extern XETEXPROGCLASS::memoryword*& zmem;

extern XETEXPROGCLASS::utf8code* nameoffile;

inline void badutf8warning()
{
  XETEXPROG.badutf8warning();
}

inline auto begindiagnostic()
{
  XETEXPROG.begindiagnostic();
}

inline void enddiagnostic(C4P::C4P_boolean blankline)
{
  XETEXPROG.enddiagnostic(blankline);
}

inline void fontfeaturewarning(voidpointer featurenamep, C4P::C4P_integer featlen, voidpointer settingnamep, C4P::C4P_integer setlen)
{
  XETEXPROG.fontfeaturewarning(featurenamep, featlen, settingnamep, setlen);
}

inline void fontmappingwarning(voidpointer mappingnamep, C4P::C4P_integer mappingnamelen, C4P::C4P_integer warningtype)
{
  XETEXPROG.fontmappingwarning(mappingnamep, mappingnamelen, warningtype);
}

inline auto getinputnormalizationstate()
{
  return XETEXPROG.getinputnormalizationstate();
}

inline auto gettracingfontsstate()
{
  return XETEXPROG.gettracingfontsstate();
}

inline void printchar(C4P::C4P_integer s)
{
  XETEXPROG.printchar(s);
}

inline void printint(C4P::C4P_integer n)
{
  XETEXPROG.printint(n);
}

inline auto printnl(XETEXPROGCLASS::strnumber s)
{
  return XETEXPROG.printnl(s);
}

inline void printrawchar(XETEXPROGCLASS::utf16code s, C4P::C4P_boolean incroffset)
{
  XETEXPROG.printrawchar(s, incroffset);
}

inline void zenddiagnostic(C4P::C4P_boolean blankline)
{
  XETEXPROG.enddiagnostic(blankline);
}

inline void zprintchar(C4P::C4P_integer s)
{
  XETEXPROG.printchar(s);
}

inline void zprintnl(XETEXPROGCLASS::strnumber s)
{
  XETEXPROG.printnl(s);
}

inline auto zxnoverd(XETEXPROGCLASS::scaled x, C4P::C4P_integer n, C4P::C4P_integer d)
{
  return XETEXPROG.xnoverd(x, n, d);
}

#include "xetex.h"
#include "synctex.h"

int miktexloadpoolstrings(int size);

inline int loadpoolstrings(int size)
{
  return miktexloadpoolstrings(size);
}

extern XETEXAPPCLASS XETEXAPP;

inline int miktexwrite18(XETEXPROGCLASS::strnumber stringNumber)
{
  // TODO
  MIKTEX_UNEXPECTED();
}


inline char* gettexstring(XETEXPROGCLASS::strnumber stringNumber)
{
  int stringStart = XETEXAPP.GetTeXStringStart(stringNumber);
  int stringLength = XETEXAPP.GetTeXStringLength(stringNumber);
  size_t sizeUtf8 = stringLength * 4 + 1;
  char* lpsz = (char*)xmalloc(sizeUtf8);
  return XETEXAPP.GetTeXString(lpsz, sizeUtf8, stringStart, stringLength);
}

inline XETEXPROGCLASS::strnumber maketexstring(const char* lpsz)
{
  std::wstring str = MiKTeX::Util::StringUtil::UTF8ToWideChar(lpsz);
  return XETEXAPP.MakeTeXString(str.c_str());
}

inline bool eightbitp()
{
  return XETEXAPP.Enable8BitCharsP();
}

inline bool miktexopentfmfile(XETEXPROGCLASS::bytefile& f, const XETEXPROGCLASS::utf8code* lpszFileName_)
{
  const char* lpszFileName = (const char*)lpszFileName_;
  MiKTeX::Core::PathName fileName(MiKTeX::Util::StringUtil::UTF8ToWideChar(lpszFileName));
  return MiKTeX::TeXAndFriends::OpenTFMFile(&f, fileName);
}

inline char* xmallocchararray(size_t size)
{
  return (char*)xmalloc(size + 1);
}

inline int uopenin(unicodefile& f, int mode, int encodingData)
{
  return u_open_in(&f, mode, encodingData);
}

inline int otpartcount(const voidpointer a)
{
  return otpartcount((const GlyphAssembly*) a);
}

inline void printutf8str(const voidpointer str, int len)
{
  printutf8str((const unsigned char*)str, len);
}

inline int otpartisextender(const voidpointer a, int i)
{
  return otpartisextender((const GlyphAssembly*)a, i);
}

inline int otpartstartconnector(int f, const voidpointer a, int i)
{
  return otpartstartconnector(f, (const GlyphAssembly*)a, i);
}

inline int otpartfulladvance(int f, const voidpointer a, int i)
{
  return otpartfulladvance(f, (const GlyphAssembly*)a, i);
}

inline int otpartendconnector(int f, const voidpointer a, int i)
{
  return otpartendconnector(f, (const GlyphAssembly*)a, i);
}

inline int otpartglyph(const voidpointer a, int i)
{
  return otpartglyph((GlyphAssembly*)a, i);
}

inline void c4p_break(unicodefile& f)
{
  if (fflush(f->f) == EOF)
  {
    MIKTEX_FATAL_CRT_ERROR("fflush");
  }
}

inline bool inputln(unicodefile& f, C4P::C4P_boolean bypassEndOfLine = true)
{
  bypassEndOfLine;
  return input_line(f);
}

inline void miktexopenin()
{
  static UFILE termin_file;
  if (XETEXPROG.termin == nullptr)
  {
    XETEXPROG.termin = &termin_file;
    XETEXPROG.termin->f = stdin;
    XETEXPROG.termin->savedChar = -1;
    XETEXPROG.termin->skipNextLF = 0;
    XETEXPROG.termin->encodingMode = UTF8;
    XETEXPROG.termin->conversionData = nullptr;
    XETEXPROG.inputfile[0] = XETEXPROG.termin;
  }
}

inline boolean usinggraphite(const voidpointer p)
{
  return usingGraphite((XeTeXLayoutEngine)p);
}

inline boolean usingopentype(const voidpointer p)
{
  return usingOpenType((XeTeXLayoutEngine)p);
}

inline void initgraphitebreaking(const voidpointer p, const XETEXPROGCLASS::utf16code* pUtf16Code, int len)
{
  initGraphiteBreaking((XeTeXLayoutEngine)p, (const UniChar*)pUtf16Code, len);
}

inline int isopentypemathfont(const voidpointer p)
{
  return isOpenTypeMathFont((XeTeXLayoutEngine)p);
}

template<class CharType> void printcstring(const CharType* lpsz)
{
  for (; *lpsz != 0; ++ lpsz)
  {
    XETEXPROG.printchar(*lpsz);
  }
}

inline void getmd5sum(XETEXPROGCLASS::strnumber s, boolean isFile)
{
  using namespace MiKTeX::Core;
  char* lpsz = gettexstring(s);
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
  for (const char& ch : Utils::Hexify(&md5[0], md5.size(), false))
  {
    XETEXPROG.strpool[XETEXPROG.poolptr] = (XETEXPROGCLASS::packedutf16code)ch;
    XETEXPROG.poolptr += 1;
  }
}

inline auto c4p_nullptr()
{
  return nullptr;
}

template<typename T> std::size_t c4p_sizeof(const T& x)
{
  return sizeof(x);
}

inline bool isnullptr(void* ptr)
{
  return ptr == nullptr;
}

template<typename T> auto xfield(const T& t)
{
  return xField(t);
}

template<typename T> auto yfield(const T& t)
{
  return yField(t);
}

template<typename P, typename X, typename Y> void setpoint(P& p, const X& x, const Y& y)
{
  setPoint(p, x, y);
}

template<typename R> auto htfield(const R& r)
{
  return htField(r);
}

template<typename R> auto wdfield(const R& r)
{
  return wdField(r);
}

template<typename F> auto fix2d(const F& f)
{
  return Fix2D(f);
}

template<typename P> auto xcoord(const P& p)
{
  return xCoord(p);
}

template<typename P> auto ycoord(const P& p)
{
  return yCoord(p);
}

template<typename D> auto d2fix(const D& d)
{
  return D2Fix(d);
}

template<typename T> auto afield(const T& t)
{
  return aField(t);
}

template<typename T> auto bfield(const T& t)
{
  return bField(t);
}

template<typename T> auto cfield(const T& t)
{
  return cField(t);
}

template<typename T> auto dfield(const T& t)
{
  return dField(t);
}

inline bool insertsrcspecialauto()
{
  return MiKTeX::TeXAndFriends::miktexinsertsrcspecialauto();
}