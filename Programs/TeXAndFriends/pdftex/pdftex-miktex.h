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

#include <miktex/TeXAndFriends/config.h>

#include "pdftexdefs.h"

#if !defined(C4PEXTERN)
#  define C4PEXTERN extern
#endif

#include "pdftexd.h"

#if !defined(THEDATA)
#  define THEDATA(x) C4P_VAR(x)
#endif

#include "pdftex-version.h"

#include <miktex/Core/Paths>
#include <miktex/TeXAndFriends/ETeXApp>
#include <miktex/KPSE/Emulation>

namespace pdftex {
#include <miktex/pdftex.defaults.h>
}

#if defined(MIKTEX_WINDOWS) && !defined(MIKTEXHELP_PDFTEX)
#  include <miktex/Core/Help>
#endif

class PDFTEXCLASS :
  public MiKTeX::TeXAndFriends::ETeXApp
{
public:
  enum {
    OPT_DRAFTMODE = 10000,
    OPT_OUTPUT_FORMAT,
  };

public:
  void AddOptions () override
  {
    ETeXApp::AddOptions();
    AddOption(MIKTEXTEXT("draftmode\0Switch on draft mode (generates no output)."), OPT_DRAFTMODE);
    AddOption(MIKTEXTEXT("output-format\0Set the output format."), OPT_OUTPUT_FORMAT, POPT_ARG_STRING, "FORMAT");
  }

public:
  bool ProcessOption(int opt, const std::string & optArg) override
  {
    bool done = true;
    switch (opt)
    {
    case OPT_DRAFTMODE:
      THEDATA(pdfdraftmodeoption) = 1;
      THEDATA(pdfdraftmodevalue) = 1;
      break;
    case OPT_OUTPUT_FORMAT:
      THEDATA(pdfoutputoption) = 1;
      if (optArg == "dvi")
      {
        THEDATA(pdfoutputvalue) = 0;
      }
      else if (optArg == "pdf")
      {
        THEDATA(pdfoutputvalue) = 2;
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

public:
  void AllocateMemory()
  {
    ETeXApp::AllocateMemory();
    GETPARAMCHECK(-1, pdfmemsize, pdf_mem_size, pdftex::pdftex::pdf_mem_size());
    GETPARAMCHECK(-1, objtabsize, obj_tab_size, pdftex::pdftex::obj_tab_size());
    GETPARAMCHECK(-1, destnamessize, dest_names_size, pdftex::pdftex::dest_names_size());
    GETPARAMCHECK(-1, pdfosbufsize, pdf_os_buf_size, pdftex::pdftex::pdf_os_buf_size());
    MIKTEX_ASSERT(constfontbase == 0);
    size_t nFonts = THEDATA(fontmax) - constfontbase;
    Allocate("destnames", THEDATA(destnames), THEDATA(destnamessize));
    Allocate("objtabsize", THEDATA(objtab), THEDATA(objtabsize));
    Allocate("pdfcharused", THEDATA(pdfcharused), nFonts);
    Allocate("pdffontautoexpand", THEDATA(pdffontattr), nFonts);
    Allocate("pdffontautoexpand", THEDATA(pdffontautoexpand), nFonts);
    Allocate("pdffontblink", THEDATA(pdffontblink), nFonts);
    Allocate("pdffontefbase", THEDATA(pdffontefbase), nFonts);
    Allocate("pdffontelink", THEDATA(pdffontelink), nFonts);
    Allocate("pdffontexpandratio", THEDATA(pdffontexpandratio), nFonts);
    Allocate("pdffontknacbase", THEDATA(pdffontknacbase), nFonts);
    Allocate("pdffontknbcbase", THEDATA(pdffontknbcbase), nFonts);
    Allocate("pdffontknbsbase", THEDATA(pdffontknbsbase), nFonts);
    Allocate("pdffontlpbase", THEDATA(pdffontlpbase), nFonts);
    Allocate("pdffontmap", THEDATA(pdffontmap), nFonts);
    Allocate("pdffontnobuiltintounicode", THEDATA(pdffontnobuiltintounicode), nFonts);
    Allocate("pdffontnum", THEDATA(pdffontnum), nFonts);
    Allocate("pdffontrpbase", THEDATA(pdffontrpbase), nFonts);
    Allocate("pdffontshbsbase", THEDATA(pdffontshbsbase), nFonts);
    Allocate("pdffontshrink", THEDATA(pdffontshrink), nFonts);
    Allocate("pdffontsize", THEDATA(pdffontsize), nFonts);
    Allocate("pdffontstbsbase", THEDATA(pdffontstbsbase), nFonts);
    Allocate("pdffontstep", THEDATA(pdffontstep), nFonts);
    Allocate("pdffontstretch", THEDATA(pdffontstretch), nFonts);
    Allocate("pdffonttype", THEDATA(pdffonttype), nFonts);
    Allocate("pdfmem", THEDATA(pdfmem), THEDATA(pdfmemsize));
    Allocate("pdfopbuf", THEDATA(pdfopbuf), pdfopbufsize);
    Allocate("pdfosbuf", THEDATA(pdfosbuf), THEDATA(pdfosbufsize));
    Allocate("pdfosobjnum", THEDATA(pdfosobjnum), pdfosmaxobjs);
    Allocate("pdfosobjoff", THEDATA(pdfosobjoff), pdfosmaxobjs);
    Allocate("vfdefaultfont", THEDATA(vfdefaultfont), nFonts);
    Allocate("vfefnts", THEDATA(vfefnts), nFonts);
    Allocate("vfifnts", THEDATA(vfifnts), nFonts);
    Allocate("vflocalfontnum", THEDATA(vflocalfontnum), nFonts);
    Allocate("vfpacketbase", THEDATA(vfpacketbase), nFonts);
  }

public:
  void FreeMemory()
  {
    ETeXApp::FreeMemory();
    Free("destnames", THEDATA(destnames));
    Free("objtab", THEDATA(objtab));
    Free("pdfcharused", THEDATA(pdfcharused));
    Free("pdffontattr", THEDATA(pdffontattr));
    Free("pdffontautoexpand", THEDATA(pdffontautoexpand));
    Free("pdffontblink", THEDATA(pdffontblink));
    Free("pdffontefbase", THEDATA(pdffontefbase));
    Free("pdffontelink", THEDATA(pdffontelink));
    Free("pdffontexpandratio", THEDATA(pdffontexpandratio));
    Free("pdffontknacbase", THEDATA(pdffontknacbase));
    Free("pdffontknbcbase", THEDATA(pdffontknbcbase));
    Free("pdffontknbsbase", THEDATA(pdffontknbsbase));
    Free("pdffontlpbase", THEDATA(pdffontlpbase));
    Free("pdffontmap", THEDATA(pdffontmap));
    Free("pdffontnum", THEDATA(pdffontnum));
    Free("pdffontnobuiltintounicode", THEDATA(pdffontnobuiltintounicode));
    Free("pdffontrpbase", THEDATA(pdffontrpbase));
    Free("pdffontshbsbase", THEDATA(pdffontshbsbase));
    Free("pdffontshrink", THEDATA(pdffontshrink));
    Free("pdffontsize", THEDATA(pdffontsize));
    Free("pdffontstbsbase", THEDATA(pdffontstbsbase));
    Free("pdffontstep", THEDATA(pdffontstep));
    Free("pdffontstretch", THEDATA(pdffontstretch));
    Free("pdffonttype", THEDATA(pdffonttype));
    Free("pdfmem", THEDATA(pdfmem));
    Free("pdfopbuf", THEDATA(pdfopbuf));
    Free("pdfosbuf", THEDATA(pdfosbuf));
    Free("pdfosobjnum", THEDATA(pdfosobjnum));
    Free("pdfosobjoff", THEDATA(pdfosobjoff));
    Free("vfdefaultfont", THEDATA(vfdefaultfont));
    Free("vfefnts", THEDATA(vfefnts));
    Free("vfifnts", THEDATA(vfifnts));
    Free("vflocalfontnum", THEDATA(vflocalfontnum));
    Free("vfpacketbase", THEDATA(vfpacketbase));
  }

public:
  void Init(const std::string & programInvocationName) override
  {
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
  void GetLibraryVersions(std::vector<MiKTeX::Core::LibraryVersion> & versions) const override;

#if defined(MIKTEX_WINDOWS)
public:
  unsigned long GetHelpId() const override
  {
    return MIKTEXHELP_PDFTEX;
  }
#endif

#if defined(MIKTEX_DEBUG)
public:
  void CheckMemory() override
  {
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(destnames));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(objtab));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdfcharused));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontattr));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontautoexpand));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontblink));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontefbase));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontelink));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontexpandratio));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontknacbase));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontknbcbase));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontknbsbase));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontlpbase));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontmap));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontnobuiltintounicode));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontnum));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontrpbase));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontshbsbase));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontshrink));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontsize));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontstbsbase));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontstep));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffontstretch));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdffonttype));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdfmem));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdfopbuf));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdfosbuf));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdfosobjnum));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(pdfosobjoff));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(vfdefaultfont));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(vfefnts));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(vfifnts));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(vflocalfontnum));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(vfpacketbase));
    ETeXApp::CheckMemory();
  }
#endif
};

extern PDFTEXCLASS PDFTEXAPP;
#define THEAPP PDFTEXAPP
#include <miktex/TeXAndFriends/ETeXApp.inl>

inline bool miktexptrequal(const void * ptr1, const void * ptr2)
{
  return ptr1 == ptr2;
}

#include <miktex/KPSE/Emulation>
#include "pdftex.h"
#if WITH_SYNCTEX
#include "synctex.h"
#endif

#define printid printID
#define printidalt printIDalt

inline int getbyte(bytefile & f)
{
  eightbits ret = *f;
  if (!feof(f))
  {
    get(f);
  }
  return ret & 0xff;
}

#define zpdfosgetosbuf pdfosgetosbuf
#define zpdfbeginobj pdfbeginobj

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

inline char * GetNameOfFileForMiKTeX()
{
  return &((THEDATA(nameoffile))[0]);
}

// special case: Web2C likes to add 1 to the nameoffile base address
inline char * GetNameOfFileForWeb2C()
{
  return GetNameOfFileForMiKTeX() - 1;
}

#define nameoffile (GetNameOfFileForWeb2C())

#if WITH_SYNCTEX
#define synctexoption THEDATA(synctexoption)
#define synctexoffset THEDATA(synctexoffset)
#endif

C4PEXTERN C4P_integer k;

#define curh THEDATA(curh)
#define curinput THEDATA(curinput)
#define curv THEDATA(curv)
#define dim100bp THEDATA(dim100bp)
#define dim100in THEDATA(dim100in)
#define dim1bp THEDATA(dim1bp)
#define dim1in THEDATA(dim1in)
#define dim1inoverpkres THEDATA(dim1inoverpkres)
#define eqtb THEDATA(eqtb)
#define f THEDATA(f)
#define fixedcompresslevel THEDATA(fixedcompresslevel)
#define fixeddecimaldigits THEDATA(fixeddecimaldigits)
#define fixedgamma THEDATA(fixedgamma)
#define fixedgentounicode THEDATA(fixedgentounicode)
#define fixedimageapplygamma THEDATA(fixedimageapplygamma)
#define fixedimagegamma THEDATA(fixedimagegamma)
#define fixedimagehicolor THEDATA(fixedimagehicolor)
#define fixedinclusioncopyfont THEDATA(fixedinclusioncopyfont)
#define fixedmovechars THEDATA(fixedmovechars)
#define fixedpdfdraftmode THEDATA(fixedpdfdraftmode)
#define fixedpdfminorversion THEDATA(fixedpdfminorversion)
#define fixedpkresolution THEDATA(fixedpkresolution)
#define fontbc THEDATA(fontbc)
#define fontdsize THEDATA(fontdsize)
#define fontec THEDATA(fontec)
#define fontmax THEDATA(fontmax)
#define fontname THEDATA(fontname)
#define fontptr THEDATA(fontptr)
#define fontsize THEDATA(fontsize)
#define fontused THEDATA(fontused)
#define formatident THEDATA(formatident)
#define jobname THEDATA(jobname)
#define lasttokensstring THEDATA(lasttokensstring)
#define objptr THEDATA(objptr)
#define objtab THEDATA(objtab)
#define onehundredbp THEDATA(onehundredbp)
#define outputfilename THEDATA(outputfilename)
#define pdfboxspecart THEDATA(pdfboxspecart)
#define pdfboxspecbleed THEDATA(pdfboxspecbleed)
#define pdfboxspeccrop THEDATA(pdfboxspeccrop)
#define pdfboxspecmedia THEDATA(pdfboxspecmedia)
#define pdfboxspectrim THEDATA(pdfboxspectrim)
#define pdfbuf THEDATA(pdfbuf)
#define pdfbufsize THEDATA(pdfbufsize)
#define pdfcharmap THEDATA(pdfcharmap)
#define pdfcharused THEDATA(pdfcharused)
#define pdfcryptdate THEDATA(pdfcryptdate)
#define pdfcryptid1 THEDATA(pdfcryptid1)
#define pdfcryptid2 THEDATA(pdfcryptid2)
#define pdfcrypting THEDATA(pdfcrypting)
#define pdfcryptobjnum THEDATA(pdfcryptobjnum)
#define pdfcryptovalue THEDATA(pdfcryptovalue)
#define pdfcryptpermit THEDATA(pdfcryptpermit)
#define pdfcryptuvalue THEDATA(pdfcryptuvalue)
#define pdfcryptversion THEDATA(pdfcryptversion)
#define pdffile THEDATA(pdffile)
#define pdffontattr THEDATA(pdffontattr)
#define pdffontnobuiltintounicode THEDATA(pdffontnobuiltintounicode)
#define pdffontefbase THEDATA(pdffontefbase)
#define pdffontexpandfont THEDATA(pdffontexpandfont)
#define pdffontexpandratio THEDATA(pdffontexpandratio)
#define pdffontknacbase THEDATA(pdffontknacbase)
#define pdffontknbcbase THEDATA(pdffontknbcbase)
#define pdffontknbsbase THEDATA(pdffontknbsbase)
#define pdffontlpbase THEDATA(pdffontlpbase)
#define pdffontmap THEDATA(pdffontmap)
#define pdffontrpbase THEDATA(pdffontrpbase)
#define pdffontshbsbase THEDATA(pdffontshbsbase)
#define pdffontsize THEDATA(pdffontsize)
#define pdffontstbsbase THEDATA(pdffontstbsbase)
#define pdfgone THEDATA(pdfgone)
#define pdfimageprocset THEDATA(pdfimageprocset)
#define pdflastbyte THEDATA(pdflastbyte)
#define pdflastpdfboxspec THEDATA(pdflastpdfboxspec)
#define pdfmem THEDATA(pdfmem)
#define pdfosmode THEDATA(pdfosmode)
#define pdfoutputvalue THEDATA(pdfoutputvalue)
#define pdfpagegroupval THEDATA(pdfpagegroupval)
#define pdfptr THEDATA(pdfptr)
#define pdfsaveoffset THEDATA(pdfsaveoffset)
#define pdfstreamlength THEDATA(pdfstreamlength)
#define pdftexbanner THEDATA(pdftexbanner)
#define pkscalefactor THEDATA(pkscalefactor)
#define poolptr THEDATA(poolptr)
#define poolsize THEDATA(poolsize)
#define ruledp THEDATA(ruledp)
#define ruleht THEDATA(ruleht)
#define rulewd THEDATA(rulewd)
#define strpool THEDATA(strpool)
#define strstart THEDATA(strstart)
#define texmflogname THEDATA(logname)
#define tmpf THEDATA(tmpf)
#define totalpages THEDATA(totalpages)
#define vfefnts THEDATA(vfefnts)
#define vfifnts THEDATA(vfifnts)
#define vfpacketbase THEDATA(vfpacketbase)
#define vfpacketlength THEDATA(vfpacketlength)
#define zmem THEDATA(zmem)

int miktexloadpoolstrings(int size);

inline int loadpoolstrings(int size)
{
  return miktexloadpoolstrings(size);
}

inline char * gettexstring(strnumber stringNumber)
{
  int stringStart = MiKTeX::TeXAndFriends::GetTeXStringStart(stringNumber);
  int stringLength = MiKTeX::TeXAndFriends::GetTeXStringLength(stringNumber);
  char * lpsz = reinterpret_cast<char*>(xmalloc(stringLength + 1));
  return MiKTeX::TeXAndFriends::GetTeXString(lpsz, stringLength + 1, stringStart, stringLength);
}

#endif
