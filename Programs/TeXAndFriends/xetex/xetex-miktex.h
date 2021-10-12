/* xetex-miktex.h:                                      -*- C++ -*-
   
   Copyright (C) 2007-2021 Christian Schenk

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

#include "xetex-version.h"

#include <miktex/Core/FileType>
#include <miktex/Core/MD5>
#include <miktex/Core/Paths>
#include <miktex/KPSE/Emulation>
#include <miktex/TeXAndFriends/CharacterConverterImpl>
#include <miktex/TeXAndFriends/ErrorHandlerImpl>
#include <miktex/TeXAndFriends/FormatHandlerImpl>
#include <miktex/TeXAndFriends/InitFinalizeImpl>
#include <miktex/TeXAndFriends/InputOutputImpl>
#include <miktex/TeXAndFriends/StringHandlerImpl>
#include <miktex/TeXAndFriends/ETeXApp>
#include <miktex/TeXAndFriends/ETeXMemoryHandlerImpl>
#include <miktex/W2C/Emulation>

#include "xetexd.h"

#include "texmfmp.h"

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

extern XETEXPROGCLASS::utf8code* nameoffile;

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
      program.fontmapping[idx] = nullptr;
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
    {
      extern std::string dvipdfmxExecutable;
      extern std::vector<std::string> dvipdfmxArgs;
      MiKTeX::Core::Argv argv(optArg);
      if (argv.GetArgc() == 0)
      {
        MIKTEX_FATAL_ERROR("--output-driver requires a value");
      }
      dvipdfmxExecutable = argv[0];
      dvipdfmxArgs.clear();
      for (int idx = 1; idx < argv.GetArgc(); ++idx)
      {
        dvipdfmxArgs.push_back(argv[idx]);
      }
      break;
    }
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
  MiKTeX::TeXAndFriends::ErrorHandlerImpl<XETEXPROGCLASS> errorHandler{ XETEXPROG };

private:
  MiKTeX::TeXAndFriends::FormatHandlerImpl<XETEXPROGCLASS> formatHandler{ XETEXPROG };

private:
  MiKTeX::TeXAndFriends::InitFinalizeImpl<XETEXPROGCLASS> initFinalize{ XETEXPROG };

private:
  MiKTeX::TeXAndFriends::InputOutputImpl<XETEXPROGCLASS> inputOutput{ XETEXPROG };

private:
  MiKTeX::TeXAndFriends::StringHandlerImpl<XETEXPROGCLASS> stringHandler{ XETEXPROG };

private:
  MemoryHandlerImpl memoryHandler{ XETEXPROG, *this };

public:
  void Init(std::vector<char*>& args) override
  {
    SetCharacterConverter(&charConv);
    SetErrorHandler(&errorHandler);
    SetFormatHandler(&formatHandler);
    SetInitFinalize(&initFinalize);
    SetInputOutput(&inputOutput);
    SetStringHandler(&stringHandler);
    SetTeXMFMemoryHandler(&memoryHandler);
    ETeXApp::Init(args);
    EnableFeature(MiKTeX::TeXAndFriends::Feature::EightBitChars);
  }

public:
  void AllocateMemory() override
  {
    ETeXApp::AllocateMemory();
  }

public:
  void FreeMemory() override
  {
    ETeXApp::FreeMemory();
  }

public:
  MiKTeX::Util::PathName GetMemoryDumpFileName() const override
  {
    return MiKTeX::Util::PathName("xetex.fmt");
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
  void SetNameOfFile(const MiKTeX::Util::PathName& fileName) override
  {
    MiKTeX::TeXAndFriends::IInputOutput* inputOutput = GetInputOutput();
    MiKTeX::TeXAndFriends::ITeXMFMemoryHandler* texmfMemoryHandler = GetTeXMFMemoryHandler();
    inputOutput->nameoffile() = reinterpret_cast<char*>(texmfMemoryHandler->ReallocateArray("nameoffile", inputOutput->nameoffile(), sizeof(inputOutput->nameoffile()[0]), fileName.GetLength() + 1, MIKTEX_SOURCE_LOCATION()));
    MiKTeX::Util::StringUtil::CopyString(inputOutput->nameoffile(), fileName.GetLength() + 1, fileName.GetData());
    inputOutput->namelength() = static_cast<C4P::C4P_signed32>(fileName.GetLength());
    // special case: Web2C likes to add 1 to the nameoffile base address
    nameoffile = reinterpret_cast<C4P::C4P_unsigned8*>(&(inputOutput->nameoffile()[-1]));
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
using strnumber = XETEXPROGCLASS::strnumber;

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
extern C4P::C4P_signed32& namelength;
extern C4P::C4P_signed32& namelength16;
extern XETEXPROGCLASS::utf16code*& nameoffile16;
extern C4P::C4P_integer& nativefonttypeflag;
extern C4P::C4P_boolean& nopdfoutput;
extern XETEXPROGCLASS::poolpointer& poolptr;
extern C4P::C4P_integer& poolsize;
extern XETEXPROGCLASS::scaled& ruledp;
extern XETEXPROGCLASS::scaled& ruleht;
extern XETEXPROGCLASS::scaled& rulewd;
extern XETEXPROGCLASS::packedutf16code*& strpool;
extern C4P::C4P_integer& synctexoffset;
extern C4P::C4P_integer& synctexoption;
extern C4P::C4P_integer& termoffset;
extern XETEXPROGCLASS::strnumber& texmflogname;
extern C4P::C4P_integer& totalpages;
extern char*& xdvbuffer;
extern XETEXPROGCLASS::memoryword*& zmem;

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

inline void print(C4P::C4P_integer s)
{
  XETEXPROG.print(s);
}

inline void printchar(C4P::C4P_integer s)
{
  XETEXPROG.printchar(s);
}

inline void printint(C4P::C4P_integer n)
{
  XETEXPROG.printint(n);
}

inline void println()
{
  XETEXPROG.println();
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

inline int miktexwrite18(XETEXPROGCLASS::strnumber stringNumber, int stringLength)
{
  int exitCode;
  return static_cast<int>(XETEXAPP.Write18(XETEXAPP.GetTeXString(XETEXAPP.GetTeXStringStart(stringNumber), stringLength) , exitCode));
}

inline char* gettexstring(XETEXPROGCLASS::strnumber stringNumber)
{
  return xstrdup(XETEXAPP.GetTeXString(stringNumber).c_str());
}

inline XETEXPROGCLASS::strnumber maketexstring(const char* lpsz)
{
  return XETEXAPP.MakeTeXString(lpsz);
}

inline bool eightbitp()
{
  return XETEXAPP.Enable8BitCharsP();
}

inline bool miktexopentfmfile(XETEXPROGCLASS::bytefile& f, const XETEXPROGCLASS::utf8code* lpszFileName_)
{
  const char* lpszFileName = (const char*)lpszFileName_;
  MiKTeX::Util::PathName fileName(MiKTeX::Util::StringUtil::UTF8ToWideChar(lpszFileName));
  return MiKTeX::TeXAndFriends::OpenTFMFile(&f, fileName);
}

inline char* xmallocchararray(size_t size)
{
  return (char*)xmalloc(size + 1);
}

inline void uclose(unicodefile& f)
{
  u_close_inout(&f);
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

inline void miktexreallocatenameoffile(size_t n)
{

  XETEXPROG.nameoffile = reinterpret_cast<C4P::C4P_unsigned8*>(XETEXAPP.GetTeXMFMemoryHandler()->ReallocateArray("name_of_file", XETEXPROG.nameoffile, sizeof(*XETEXPROG.nameoffile), n, MIKTEX_SOURCE_LOCATION()));
  // special case: Web2C likes to add 1 to the nameoffile base address
  nameoffile = &XETEXPROG.nameoffile[-1];
}
