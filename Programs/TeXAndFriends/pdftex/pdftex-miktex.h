/* pdftex-miktex.h:                                     -*- C++ -*-
   
   Copyright (C) 1998-2017 Christian Schenk

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

#if defined(HAVE_PRAGMA_ONCE)
#  pragma once
#endif

#if !defined(B9AE601D55FC414A8D93C81CF3517D1A)
#define B9AE601D55FC414A8D93C81CF3517D1A

#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>

#include <miktex/Core/Paths>
#include <miktex/KPSE/Emulation>
#include <miktex/TeXAndFriends/CharacterConverterImpl>
#include <miktex/TeXAndFriends/InitFinalizeImpl>
#include <miktex/TeXAndFriends/InputOutputImpl>
#include <miktex/TeXAndFriends/ETeXApp>
#include <miktex/TeXAndFriends/ETeXMemoryHandlerImpl>

#include "pdftexdefs.h"

//#if defined(C4P_HEADER_GUARD_pdftex)
//#  error pdftex-miktex.h must be included before pdftexd.h
//#endif
#include "pdftexd.h"

#include "pdftex-version.h"

namespace pdftex {
#include <miktex/pdftex.defaults.h>
}

#if defined(MIKTEX_WINDOWS) && !defined(MIKTEXHELP_PDFTEX)
#  include <miktex/Core/Help>
#endif

#include "pdftex-miktex-config.h"

extern PDFTEXPROGCLASS PDFTEXPROG;

class MemoryHandlerImpl :
  public MiKTeX::TeXAndFriends::ETeXMemoryHandlerImpl<PDFTEXPROGCLASS>
{
public:
  MemoryHandlerImpl(PDFTEXPROGCLASS& program, MiKTeX::TeXAndFriends::TeXMFApp& mfapp) :
    ETeXMemoryHandlerImpl(program, mfapp)
  {
  }

public:
  void Allocate(const std::unordered_map<std::string, int>& userParams) override
  {
    ETeXMemoryHandlerImpl::Allocate(userParams);
    program.pdfmemsize = GetCheckedParameter("pdf_mem_size", program.infpdfmemsize, program.suppdfmemsize, userParams, pdftex::pdftex::pdf_mem_size());
    program.objtabsize = GetCheckedParameter("obj_tab_size", program.infobjtabsize, program.supobjtabsize, userParams, pdftex::pdftex::obj_tab_size());
    program.destnamessize = GetCheckedParameter("dest_names_size", program.infdestnamessize, program.supdestnamessize, userParams, pdftex::pdftex::dest_names_size());
    program.pdfosbufsize = GetCheckedParameter("pdf_os_buf_size", program.infpdfosbufsize, program.suppdfosbufsize, userParams, pdftex::pdftex::pdf_os_buf_size());
    MIKTEX_ASSERT(program.constfontbase == 0);
    size_t nFonts = program.fontmax - program.constfontbase;
    AllocateArray("destnames", program.destnames, program.destnamessize);
    AllocateArray("objtabsize", program.objtab, program.objtabsize);
    AllocateArray("pdfcharused", program.pdfcharused, nFonts);
    AllocateArray("pdffontautoexpand", program.pdffontattr, nFonts);
    AllocateArray("pdffontautoexpand", program.pdffontautoexpand, nFonts);
    AllocateArray("pdffontblink", program.pdffontblink, nFonts);
    AllocateArray("pdffontefbase", program.pdffontefbase, nFonts);
    AllocateArray("pdffontelink", program.pdffontelink, nFonts);
    AllocateArray("pdffontexpandratio", program.pdffontexpandratio, nFonts);
    AllocateArray("pdffontknacbase", program.pdffontknacbase, nFonts);
    AllocateArray("pdffontknbcbase", program.pdffontknbcbase, nFonts);
    AllocateArray("pdffontknbsbase", program.pdffontknbsbase, nFonts);
    AllocateArray("pdffontlpbase", program.pdffontlpbase, nFonts);
    AllocateArray("pdffontmap", program.pdffontmap, nFonts);
    AllocateArray("pdffontnobuiltintounicode", program.pdffontnobuiltintounicode, nFonts);
    AllocateArray("pdffontnum", program.pdffontnum, nFonts);
    AllocateArray("pdffontrpbase", program.pdffontrpbase, nFonts);
    AllocateArray("pdffontshbsbase", program.pdffontshbsbase, nFonts);
    AllocateArray("pdffontshrink", program.pdffontshrink, nFonts);
    AllocateArray("pdffontsize", program.pdffontsize, nFonts);
    AllocateArray("pdffontstbsbase", program.pdffontstbsbase, nFonts);
    AllocateArray("pdffontstep", program.pdffontstep, nFonts);
    AllocateArray("pdffontstretch", program.pdffontstretch, nFonts);
    AllocateArray("pdffonttype", program.pdffonttype, nFonts);
    AllocateArray("pdfmem", program.pdfmem, program.pdfmemsize);
    AllocateArray("pdfopbuf", program.pdfopbuf, program.pdfopbufsize);
    AllocateArray("pdfosbuf", program.pdfosbuf, program.pdfosbufsize);
    AllocateArray("pdfosobjnum", program.pdfosobjnum, program.pdfosmaxobjs);
    AllocateArray("pdfosobjoff", program.pdfosobjoff, program.pdfosmaxobjs);
    AllocateArray("vfdefaultfont", program.vfdefaultfont, nFonts);
    AllocateArray("vfefnts", program.vfefnts, nFonts);
    AllocateArray("vfifnts", program.vfifnts, nFonts);
    AllocateArray("vflocalfontnum", program.vflocalfontnum, nFonts);
    AllocateArray("vfpacketbase", program.vfpacketbase, nFonts);
  }

public:
  void Free() override
  {
    ETeXMemoryHandlerImpl::Free();
    FreeArray("destnames", program.destnames);
    FreeArray("objtab", program.objtab);
    FreeArray("pdfcharused", program.pdfcharused);
    FreeArray("pdffontattr", program.pdffontattr);
    FreeArray("pdffontautoexpand", program.pdffontautoexpand);
    FreeArray("pdffontblink", program.pdffontblink);
    FreeArray("pdffontefbase", program.pdffontefbase);
    FreeArray("pdffontelink", program.pdffontelink);
    FreeArray("pdffontexpandratio", program.pdffontexpandratio);
    FreeArray("pdffontknacbase", program.pdffontknacbase);
    FreeArray("pdffontknbcbase", program.pdffontknbcbase);
    FreeArray("pdffontknbsbase", program.pdffontknbsbase);
    FreeArray("pdffontlpbase", program.pdffontlpbase);
    FreeArray("pdffontmap", program.pdffontmap);
    FreeArray("pdffontnum", program.pdffontnum);
    FreeArray("pdffontnobuiltintounicode", program.pdffontnobuiltintounicode);
    FreeArray("pdffontrpbase", program.pdffontrpbase);
    FreeArray("pdffontshbsbase", program.pdffontshbsbase);
    FreeArray("pdffontshrink", program.pdffontshrink);
    FreeArray("pdffontsize", program.pdffontsize);
    FreeArray("pdffontstbsbase", program.pdffontstbsbase);
    FreeArray("pdffontstep", program.pdffontstep);
    FreeArray("pdffontstretch", program.pdffontstretch);
    FreeArray("pdffonttype", program.pdffonttype);
    FreeArray("pdfmem", program.pdfmem);
    FreeArray("pdfopbuf", program.pdfopbuf);
    FreeArray("pdfosbuf", program.pdfosbuf);
    FreeArray("pdfosobjnum", program.pdfosobjnum);
    FreeArray("pdfosobjoff", program.pdfosobjoff);
    FreeArray("vfdefaultfont", program.vfdefaultfont);
    FreeArray("vfefnts", program.vfefnts);
    FreeArray("vfifnts", program.vfifnts);
    FreeArray("vflocalfontnum", program.vflocalfontnum);
    FreeArray("vfpacketbase", program.vfpacketbase);
  }

public:
  void Check() override
  {
    ETeXMemoryHandlerImpl::Check();
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.destnames);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.objtab);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdfcharused);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontattr);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontautoexpand);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontblink);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontefbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontelink);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontexpandratio);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontknacbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontknbcbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontknbsbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontlpbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontmap);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontnobuiltintounicode);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontnum);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontrpbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontshbsbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontshrink);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontsize);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontstbsbase);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontstep);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffontstretch);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdffonttype);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdfmem);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdfopbuf);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdfosbuf);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdfosobjnum);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.pdfosobjoff);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.vfdefaultfont);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.vfefnts);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.vfifnts);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.vflocalfontnum);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.vfpacketbase);
  }

};

class PDFTEXAPPCLASS :
  public MiKTeX::TeXAndFriends::ETeXApp
{
public:
  enum {
    OPT_DRAFTMODE = 10000,
    OPT_OUTPUT_FORMAT,
  };

public:
  void AddOptions() override
  {
    ETeXApp::AddOptions();
    AddOption(MIKTEXTEXT("draftmode\0Switch on draft mode (generates no output)."), OPT_DRAFTMODE);
    AddOption(MIKTEXTEXT("output-format\0Set the output format."), OPT_OUTPUT_FORMAT, POPT_ARG_STRING, "FORMAT");
  }

public:
  bool ProcessOption(int opt, const std::string& optArg) override
  {
    bool done = true;
    switch (opt)
    {
    case OPT_DRAFTMODE:
      PDFTEXPROG.pdfdraftmodeoption = 1;
      PDFTEXPROG.pdfdraftmodevalue = 1;
      break;
    case OPT_OUTPUT_FORMAT:
      PDFTEXPROG.pdfoutputoption = 1;
      if (optArg == "dvi")
      {
        PDFTEXPROG.pdfoutputvalue = 0;
      }
      else if (optArg == "pdf")
      {
        PDFTEXPROG.pdfoutputvalue = 2;
      }
      else
      {
        FatalError(MIKTEXTEXT("Unkown output option value."));
      }
      break;
    default:
      done = ETeXApp::ProcessOption(opt, optArg);
      break;
    }
    return (done);
  }

private:
  MiKTeX::TeXAndFriends::CharacterConverterImpl<pdfTeXProgram> charConv{ PDFTEXPROG };

private:
  MiKTeX::TeXAndFriends::InitFinalizeImpl<pdfTeXProgram> initFinalize{ PDFTEXPROG };

private:
  MiKTeX::TeXAndFriends::InputOutputImpl<pdfTeXProgram> inputOutput{ PDFTEXPROG };

private:
  MemoryHandlerImpl memoryHandler{ PDFTEXPROG, *this };

public:
  void Init(const std::string& programInvocationName) override
  {
    SetCharacterConverter(&charConv);
    SetInitFinalize(&initFinalize);
    SetInputOutput(&inputOutput);
    SetTeXMFMemoryHandler(&memoryHandler);
    ETeXApp::Init(programInvocationName);
    kpse_set_program_name(programInvocationName.c_str(), nullptr);
#if defined(IMPLEMENT_TCX)
    EnableFeature(MiKTeX::TeXAndFriends::Feature::EightBitChars);
    EnableFeature(MiKTeX::TeXAndFriends::Feature::TCX);
#endif
  }

public:
  MiKTeX::Core::PathName GetMemoryDumpFileName() const override
  {
    return "pdftex.fmt";
  }

public:
  std::string GetInitProgramName() const override
  {
    return "pdfinitex";
  }

public:
  std::string GetVirginProgramName() const override
  {
    return "pdfvirtex";
  }

public:
  std::string TheNameOfTheGame() const override
  {
    return "pdfTeX";
  }

public:
  void GetLibraryVersions(std::vector<MiKTeX::Core::LibraryVersion>& versions) const override;

#if defined(MIKTEX_WINDOWS)
public:
  unsigned long GetHelpId() const override
  {
    return MIKTEXHELP_PDFTEX;
  }
#endif
};

extern PDFTEXAPPCLASS PDFTEXAPP;
#define THEAPP PDFTEXAPP
#include <miktex/TeXAndFriends/ETeXApp.inl>

inline bool miktexptrequal(const void* ptr1, const void* ptr2)
{
  return ptr1 == ptr2;
}

using eightbits = PDFTEXPROGCLASS::eightbits;
using fmentryptr = PDFTEXPROGCLASS::fmentryptr;
using halfword = PDFTEXPROGCLASS::halfword;
using internalfontnumber = PDFTEXPROGCLASS::internalfontnumber;
using poolpointer = PDFTEXPROGCLASS::poolpointer;
using scaled = PDFTEXPROGCLASS::scaled;
using strnumber = PDFTEXPROGCLASS::strnumber;

constexpr auto pdfobjtypemax = PDFTEXPROGCLASS::pdfobjtypemax;

auto& curh = PDFTEXPROG.curh;
auto& curv = PDFTEXPROG.curv;
auto& curinput = PDFTEXPROG.curinput;
auto& eqtb = PDFTEXPROG.eqtb;
auto& fixedgentounicode = PDFTEXPROG.fixedgentounicode;
auto& fixedinclusioncopyfont = PDFTEXPROG.fixedinclusioncopyfont;
auto& fixedpdfdraftmode = PDFTEXPROG.fixedpdfdraftmode;
auto& fontmax = PDFTEXPROG.fontmax;
auto& fontname = PDFTEXPROG.fontname;
auto& formatident = PDFTEXPROG.formatident;
auto& jobname = PDFTEXPROG.jobname;
auto& lasttokensstring = PDFTEXPROG.lasttokensstring;
auto& objtab = PDFTEXPROG.objtab;
auto& outputfilename = PDFTEXPROG.outputfilename;
auto& pdfboxspecart = PDFTEXPROG.pdfboxspecart;
auto& pdfboxspecbleed = PDFTEXPROG.pdfboxspecbleed;
auto& pdfboxspeccrop = PDFTEXPROG.pdfboxspeccrop;
auto& pdfboxspecmedia = PDFTEXPROG.pdfboxspecmedia;
auto& pdfboxspectrim = PDFTEXPROG.pdfboxspectrim;
auto& pdfbuf = PDFTEXPROG.pdfbuf;
auto& pdfbufsize = PDFTEXPROG.pdfbufsize;
auto& pdffile = PDFTEXPROG.pdffile;
auto& pdffontmap = PDFTEXPROG.pdffontmap;
auto& pdfgone = PDFTEXPROG.pdfgone;
auto& pdflastbyte = PDFTEXPROG.pdflastbyte;
auto& pdfosmode = PDFTEXPROG.pdfosmode;
auto& pdfoutputvalue = PDFTEXPROG.pdfoutputvalue;
auto& pdfpagegroupval = PDFTEXPROG.pdfpagegroupval;
auto& pdfptr = PDFTEXPROG.pdfptr;
auto& pdftexbanner = PDFTEXPROG.pdftexbanner;
auto& poolptr = PDFTEXPROG.poolptr;
auto& poolsize = PDFTEXPROG.poolsize;
auto& ruledp = PDFTEXPROG.ruledp;
auto& ruleht = PDFTEXPROG.ruleht;
auto& rulewd = PDFTEXPROG.rulewd;
auto& strpool = PDFTEXPROG.strpool;
auto& strstart = PDFTEXPROG.strstart;
auto& texmflogname = PDFTEXPROG.logname;
auto& totalpages = PDFTEXPROG.totalpages;
auto& vfefnts = PDFTEXPROG.vfefnts;
auto& vfifnts = PDFTEXPROG.vfifnts;
auto& zmem = PDFTEXPROG.zmem;

auto flushstr(PDFTEXPROGCLASS::strnumber s)
{
  PDFTEXPROG.flushstr(s);
}

auto getnullstr()
{
  return PDFTEXPROG.getnullstr();
}

auto getpdfsuppresswarningdupmap()
{
  return PDFTEXPROG.getpdfsuppresswarningdupmap();
}

auto makestring()
{
  return PDFTEXPROG.makestring();
}

auto packfilename(PDFTEXPROGCLASS::strnumber n, PDFTEXPROGCLASS::strnumber a, PDFTEXPROGCLASS::strnumber e)
{
  return PDFTEXPROG.packfilename(n, a, e);
}

auto pdfbegindict(C4P::C4P_integer i, C4P::C4P_integer pdfoslevel)
{
  PDFTEXPROG.pdfbegindict(i, pdfoslevel);
}

auto pdfbeginobj(C4P::C4P_integer i, C4P::C4P_integer pdfoslevel)
{
  PDFTEXPROG.pdfbeginobj(i, pdfoslevel);
}

auto pdfbeginstream()
{
  PDFTEXPROG.pdfbeginstream();
}

auto pdfendobj()
{
  PDFTEXPROG.pdfendobj();
}

auto pdfendstream()
{
  PDFTEXPROG.pdfendstream();
}
auto pdfflush()
{
  PDFTEXPROG.pdfflush();
}

auto pdfnewobjnum()
{
  return PDFTEXPROG.pdfnewobjnum();
}

auto pdfosgetosbuf(C4P::C4P_integer s)
{
  PDFTEXPROG.pdfosgetosbuf(s);
}

auto print(C4P::C4P_integer s)
{
  PDFTEXPROG.print(s);
}

auto println()
{
  PDFTEXPROG.println();
}

auto tokenstostring(PDFTEXPROGCLASS::halfword p)
{
  return PDFTEXPROG.tokenstostring(p);
}


#include "pdftex.h"
#if WITH_SYNCTEX
#include "synctex.h"
#endif

#define printid printID
#define printidalt printIDalt

template<typename FileType> int getbyte(FileType& f)
{
  unsigned char ret = *f;
  if (!feof(f))
  {
    get(f);
  }
  return ret & 0xff;
}

// REMOVE: #define zpdfosgetosbuf pdfosgetosbuf
// REMOVE: #define zpdfbeginobj pdfbeginobj

#if defined(texbopenin)
#  undef texbopenin
#  define texbopenin(f) \
    open_input(&f.fileref(), kpse_tex_format, FOPEN_RBIN_MODE)
#endif

#if defined(vfbopenin)
#  undef vfbopenin
#  define vfbopenin(f) \
  miktexopenvffile(f, GetNameOfFileForMiKTeX())
#endif

#if !defined(__GNUC__)
#  define __attribute__(x)
#endif

inline char* GetNameOfFileForMiKTeX()
{
  return &((PDFTEXPROG.nameoffile)[0]);
}

// special case: Web2C likes to add 1 to the nameoffile base address
inline char* GetNameOfFileForWeb2C()
{
  return GetNameOfFileForMiKTeX() - 1;
}

#define nameoffile (GetNameOfFileForWeb2C())

#if WITH_SYNCTEX
#define synctexoption PDFTEXPROG.synctexoption
#define synctexoffset PDFTEXPROG.synctexoffset
#endif

#if 0
extern C4P_integer k;
#endif

#if 0
#define curh program.curh
#define curinput program.curinput
#define curv program.curv
#define dim100bp THEDATA(dim100bp)
#define dim100in THEDATA(dim100in)
#define dim1bp THEDATA(dim1bp)
#define dim1in THEDATA(dim1in)
#define dim1inoverpkres THEDATA(dim1inoverpkres)
#define eqtb program.eqtb
#define f program.f
#define fixedcompresslevel program.fixedcompresslevel
#define fixeddecimaldigits program.fixeddecimaldigits
#define fixedgamma program.fixedgamma
#define fixedgentounicode program.fixedgentounicode
#define fixedimageapplygamma program.fixedimageapplygamma
#define fixedimagegamma program.fixedimagegamma
#define fixedimagehicolor program.fixedimagehicolor
#define fixedinclusioncopyfont program.fixedinclusioncopyfont
#define fixedmovechars program.fixedmovechars
#define fixedpdfdraftmode program.fixedpdfdraftmode
#define fixedpdfminorversion program.fixedpdfminorversion
#define fixedpkresolution program.fixedpkresolution
#define fontbc program.fontbc
#define fontdsize program.fontdsize
#define fontec program.fontec
#define fontmax program.fontmax
#define fontname program.fontname
#define fontptr program.fontptr
#define fontsize program.fontsize
#define fontused program.fontused
#define formatident program.formatident
#define jobname program.jobname
#define lasttokensstring program.lasttokensstring
#define objptr program.objptr
#define objtab program.objtab
#define onehundredbp program.onehundredbp
#define outputfilename program.outputfilename
#define pdfboxspecart program.pdfboxspecart
#define pdfboxspecbleed program.pdfboxspecbleed
#define pdfboxspeccrop program.pdfboxspeccrop
#define pdfboxspecmedia program.pdfboxspecmedia
#define pdfboxspectrim program.pdfboxspectrim
#define pdfbuf program.pdfbuf
#define pdfbufsize program.pdfbufsize
#define pdfcharmap program.pdfcharmap
#define pdfcharused program.pdfcharused
#define pdfcryptdate program.pdfcryptdate
#define pdfcryptid1 THEDATA(pdfcryptid1)
#define pdfcryptid2 THEDATA(pdfcryptid2)
#define pdfcrypting program.pdfcrypting
#define pdfcryptobjnum program.pdfcryptobjnum
#define pdfcryptovalue program.pdfcryptovalue
#define pdfcryptpermit program.pdfcryptpermit
#define pdfcryptuvalue program.pdfcryptuvalue
#define pdfcryptversion program.pdfcryptversion
#define pdffile program.pdffile
#define pdffontattr program.pdffontattr
#define pdffontnobuiltintounicode program.pdffontnobuiltintounicode
#define pdffontefbase program.pdffontefbase
#define pdffontexpandfont program.pdffontexpandfont
#define pdffontexpandratio program.pdffontexpandratio
#define pdffontknacbase program.pdffontknacbase
#define pdffontknbcbase program.pdffontknbcbase
#define pdffontknbsbase program.pdffontknbsbase
#define pdffontlpbase program.pdffontlpbase
#define pdffontmap program.pdffontmap
#define pdffontrpbase program.pdffontrpbase
#define pdffontshbsbase program.pdffontshbsbase
#define pdffontsize program.pdffontsize
#define pdffontstbsbase program.pdffontstbsbase
#define pdfgone program.pdfgone
#define pdfimageprocset program.pdfimageprocset
#define pdflastbyte program.pdflastbyte
#define pdflastpdfboxspec program.pdflastpdfboxspec
#define pdfmem program.pdfmem
#define pdfosmode program.pdfosmode
#define pdfoutputvalue program.pdfoutputvalue
#define pdfpagegroupval program.pdfpagegroupval
#define pdfptr program.pdfptr
#define pdfsaveoffset program.pdfsaveoffset
#define pdfstreamlength program.pdfstreamlength
#define pdftexbanner program.pdftexbanner
#define pkscalefactor program.pkscalefactor
#define poolptr program.poolptr
#define poolsize program.poolsize
#define ruledp program.ruledp
#define ruleht program.ruleht
#define rulewd program.rulewd
#define strpool program.strpool
#define strstart program.strstart
#define texmflogname program.logname
#define tmpf program.tmpf
#define totalpages program.totalpages
#define vfefnts program.vfefnts
#define vfifnts program.vfifnts
#define vfpacketbase program.vfpacketbase
#define vfpacketlength program.vfpacketlength
#define zmem program.zmem
#endif

int miktexloadpoolstrings(int size);

inline int loadpoolstrings(int size)
{
  return miktexloadpoolstrings(size);
}

inline char* gettexstring(PDFTEXPROGCLASS::strnumber stringNumber)
{
  int stringStart = PDFTEXAPP.GetTeXStringStart(stringNumber);
  int stringLength = PDFTEXAPP.GetTeXStringLength(stringNumber);
  char* lpsz = (char*)xmalloc(stringLength + 1);
  return PDFTEXAPP.GetTeXString(lpsz, stringLength + 1, stringStart, stringLength);
}

#endif
