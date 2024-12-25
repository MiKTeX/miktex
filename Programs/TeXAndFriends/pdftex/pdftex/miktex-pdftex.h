/**
 * @file miktex-pdftex.h
 * @author Christian Schenk
 * @brief MiKTeX-pdfTeX
 *
 * @copyright Copyright © 1998-2024 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#pragma once

#include "miktex-pdftex-config.h"

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif

#include "miktex-pdftex-version.h"

#include <miktex/Core/Paths>
#include <miktex/KPSE/Emulation>
#include <miktex/Locale/Translator>
#include <miktex/Resources/ResourceRepository>
#include <miktex/TeXAndFriends/CharacterConverterImpl>
#include <miktex/TeXAndFriends/ErrorHandlerImpl>
#include <miktex/TeXAndFriends/FormatHandlerImpl>
#include <miktex/TeXAndFriends/InitFinalizeImpl>
#include <miktex/TeXAndFriends/InputOutputImpl>
#include <miktex/TeXAndFriends/StringHandlerImpl>
#include <miktex/TeXAndFriends/ETeXApp>
#include <miktex/TeXAndFriends/ETeXMemoryHandlerImpl>

#include "pdftexd.h"

#include "texmfmp.h"

namespace pdftex {
#include <miktex/pdftex.defaults.h>
}

#if defined(MIKTEX_WINDOWS) && !defined(MIKTEXHELP_PDFTEX)
#  include <miktex/Core/Help>
#endif

extern PDFTEXPROGCLASS PDFTEXPROG;

extern char* nameoffile;

class MemoryHandlerImpl:
    public MiKTeX::TeXAndFriends::ETeXMemoryHandlerImpl<PDFTEXPROGCLASS>
{

public:

    MemoryHandlerImpl(PDFTEXPROGCLASS& program, MiKTeX::TeXAndFriends::TeXMFApp& texmfapp):
        ETeXMemoryHandlerImpl<PDFTEXPROGCLASS>(program, texmfapp)
    {
    }

    void Allocate(const std::unordered_map<std::string, int>& userParams) override
    {
        ETeXMemoryHandlerImpl<PDFTEXPROGCLASS>::Allocate(userParams);
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
        AllocateArray("pdffonthasspacechar", program.pdffonthasspacechar, nFonts);
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

    void Free() override
    {
        ETeXMemoryHandlerImpl<PDFTEXPROGCLASS>::Free();
        FreeArray("destnames", program.destnames);
        FreeArray("objtab", program.objtab);
        FreeArray("pdfcharused", program.pdfcharused);
        FreeArray("pdffontattr", program.pdffontattr);
        FreeArray("pdffontautoexpand", program.pdffontautoexpand);
        FreeArray("pdffontblink", program.pdffontblink);
        FreeArray("pdffontefbase", program.pdffontefbase);
        FreeArray("pdffontelink", program.pdffontelink);
        FreeArray("pdffontexpandratio", program.pdffontexpandratio);
        FreeArray("pdffonthasspacechar", program.pdffonthasspacechar);
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

    void Check() override
    {
        ETeXMemoryHandlerImpl<PDFTEXPROGCLASS>::Check();
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

class PDFTEXAPPCLASS:
    public MiKTeX::TeXAndFriends::ETeXApp
{

public:

    enum {
        OPT_DRAFTMODE = 10000,
        OPT_OUTPUT_FORMAT,
    };

    void AddOptions() override
    {
        ETeXApp::AddOptions();
        AddOption("draftmode", T_("Switch on draft mode (generates no output)."), OPT_DRAFTMODE);
        AddOption("output-format", T_("Set the output format."), OPT_OUTPUT_FORMAT, POPT_ARG_STRING, "FORMAT");
    }

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
                FatalError(T_("Unknown output format value."));
            }
            break;
        default:
            done = ETeXApp::ProcessOption(opt, optArg);
            break;
        }
        return (done);
    }

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
        translator = std::make_unique<MiKTeX::Locale::Translator>(MIKTEX_COMP_ID, resources, GetSession());
        kpse_set_program_name(args[0], nullptr);
        EnableFeature(MiKTeX::TeXAndFriends::Feature::EightBitChars);
        EnableFeature(MiKTeX::TeXAndFriends::Feature::TCX);
    }

    void Finalize() override
    {
        translator = nullptr;
        ETeXApp::Finalize();
    }

    void AllocateMemory() override
    {
        ETeXApp::AllocateMemory();
    }

    void FreeMemory() override
    {
        ETeXApp::FreeMemory();
    }

    MiKTeX::Util::PathName GetMemoryDumpFileName() const override
    {
        return MiKTeX::Util::PathName("pdftex.fmt");
    }

    std::string GetInitProgramName() const override
    {
        return "pdfinitex";
    }

    std::string GetVirginProgramName() const override
    {
        return "pdfvirtex";
    }

    std::string TheNameOfTheGame() const override
    {
        return "pdfTeX";
    }

    void SetNameOfFile(const MiKTeX::Util::PathName& fileName) override
    {
        MiKTeX::TeXAndFriends::IInputOutput* inputOutput = GetInputOutput();
        MiKTeX::TeXAndFriends::ITeXMFMemoryHandler* texmfMemoryHandler = GetTeXMFMemoryHandler();
        inputOutput->nameoffile() = reinterpret_cast<char*>(texmfMemoryHandler->ReallocateArray("nameoffile", inputOutput->nameoffile(), sizeof(inputOutput->nameoffile()[0]), fileName.GetLength() + 1, MIKTEX_SOURCE_LOCATION()));
        MiKTeX::Util::StringUtil::CopyCeeString(inputOutput->nameoffile(), fileName.GetLength() + 1, fileName.GetData());
        inputOutput->namelength() = static_cast<C4P::C4P_signed32>(fileName.GetLength());
        // special case: Web2C likes to add 1 to the nameoffile base address
        nameoffile = &(inputOutput->nameoffile()[-1]);
    }

    void GetLibraryVersions(std::vector<MiKTeX::Core::LibraryVersion>& versions) const override;

#if defined(MIKTEX_WINDOWS)
    unsigned long GetHelpId() const override
    {
        return MIKTEXHELP_PDFTEX;
    }
#endif

private:

    std::string Translate(const char* msgId)
    {
        return translator->Translate(msgId);
    }

    std::string T_(const char* msgId)
    {
        return Translate(msgId);
    }

    MiKTeX::TeXAndFriends::CharacterConverterImpl<PDFTEXPROGCLASS> charConv{ PDFTEXPROG };
    MiKTeX::TeXAndFriends::ErrorHandlerImpl<PDFTEXPROGCLASS> errorHandler{ PDFTEXPROG };
    MiKTeX::TeXAndFriends::FormatHandlerImpl<PDFTEXPROGCLASS> formatHandler{ PDFTEXPROG };
    MiKTeX::TeXAndFriends::InitFinalizeImpl<PDFTEXPROGCLASS> initFinalize{ PDFTEXPROG };
    MiKTeX::TeXAndFriends::InputOutputImpl<PDFTEXPROGCLASS> inputOutput{ PDFTEXPROG };
    MemoryHandlerImpl memoryHandler{ PDFTEXPROG, *this };
    MiKTeX::TeXAndFriends::StringHandlerImpl<PDFTEXPROGCLASS> stringHandler{ PDFTEXPROG };
    std::unique_ptr<MiKTeX::Locale::Translator> translator;

    static MiKTeX::Resources::ResourceRepository* resources;
};

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

extern PDFTEXPROGCLASS::scaled& curh;
extern PDFTEXPROGCLASS::instaterecord& curinput;
extern PDFTEXPROGCLASS::scaled& curv;
extern PDFTEXPROGCLASS::memoryword*& eqtb;
extern C4P::C4P_integer& fixeddecimaldigits;
extern C4P::C4P_integer& fixedgamma;
extern C4P::C4P_integer& fixedgentounicode;
extern C4P::C4P_integer& fixedimageapplygamma;
extern C4P::C4P_integer& fixedimagegamma;
extern C4P::C4P_boolean& fixedimagehicolor;
extern C4P::C4P_integer& fixedinclusioncopyfont;
extern C4P::C4P_integer& fixedpdfdraftmode;
extern C4P::C4P_integer& fixedpdfmajorversion;
extern C4P::C4P_integer& fixedpdfminorversion;
extern C4P::C4P_integer& fixedpkresolution;
extern PDFTEXPROGCLASS::eightbits*& fontbc;
extern PDFTEXPROGCLASS::scaled*& fontdsize;
extern PDFTEXPROGCLASS::eightbits*& fontec;
extern C4P::C4P_integer& fontmax;
extern PDFTEXPROGCLASS::strnumber*& fontname;
extern PDFTEXPROGCLASS::strnumber& formatident;
extern C4P::C4P_signed8& interaction;
extern PDFTEXPROGCLASS::strnumber& jobname;
extern PDFTEXPROGCLASS::strnumber& lasttokensstring;
extern C4P::C4P_integer& objptr;
extern PDFTEXPROGCLASS::objentry*& objtab;
extern PDFTEXPROGCLASS::scaled& onehundredbp;
extern PDFTEXPROGCLASS::strnumber& outputfilename;
extern C4P::C4P_integer& pdfboxspecart;
extern C4P::C4P_integer& pdfboxspecbleed;
extern C4P::C4P_integer& pdfboxspeccrop;
extern C4P::C4P_integer& pdfboxspecmedia;
extern C4P::C4P_integer& pdfboxspectrim;
extern PDFTEXPROGCLASS::eightbits*& pdfbuf;
extern C4P::C4P_integer& pdfbufsize;
extern PDFTEXPROGCLASS::charusedarray*& pdfcharused;
extern C4P::C4P_integer& pdfcurform;
extern PDFTEXPROGCLASS::bytefile& pdffile;
extern PDFTEXPROGCLASS::strnumber*& pdffontattr;
extern PDFTEXPROGCLASS::fmentryptr*& pdffontmap;
extern C4P::C4P_boolean*& pdffontnobuiltintounicode;
extern PDFTEXPROGCLASS::scaled*& pdffontsize;
extern C4P::C4P_longinteger& pdfgone;
extern C4P::C4P_integer& pdfimageprocset;
extern PDFTEXPROGCLASS::eightbits& pdflastbyte;
extern C4P::C4P_boolean& pdfosmode;
extern C4P::C4P_integer& pdfoutputvalue;
extern C4P::C4P_integer& pdfpagegroupval;
extern C4P::C4P_integer& pdfptr;
extern C4P::C4P_longinteger& pdfsaveoffset;
extern C4P::C4P_longinteger& pdfstreamlength;
extern PDFTEXPROGCLASS::strnumber& pdftexbanner;
extern C4P::C4P_integer& pkscalefactor;
extern PDFTEXPROGCLASS::poolpointer& poolptr;
extern C4P::C4P_integer& poolsize;
extern PDFTEXPROGCLASS::scaled& ruledp;
extern PDFTEXPROGCLASS::scaled& ruleht;
extern PDFTEXPROGCLASS::scaled& rulewd;
extern PDFTEXPROGCLASS::packedasciicode*& strpool;
extern PDFTEXPROGCLASS::poolpointer*& strstart;
extern C4P::C4P_integer& termoffset;
extern PDFTEXPROGCLASS::strnumber& texmflogname;
extern C4P::C4P_integer& totalpages;
extern C4P::C4P_integer*& vfefnts;
extern PDFTEXPROGCLASS::internalfontnumber*& vfifnts;
extern C4P::C4P_integer*& vfpacketbase;
extern C4P::C4P_integer& vfpacketlength;
extern PDFTEXPROGCLASS::memoryword*& zmem;

#if WITH_SYNCTEX
extern C4P::C4P_integer& synctexoption;
extern C4P::C4P_integer& synctexoffset;
#endif

extern C4P::C4P_integer k;

inline auto dividescaled(PDFTEXPROGCLASS::scaled s, PDFTEXPROGCLASS::scaled m, C4P::C4P_integer dd)
{
    return PDFTEXPROG.dividescaled(s, m, dd);
}

inline auto flushstr(PDFTEXPROGCLASS::strnumber s)
{
    PDFTEXPROG.flushstr(s);
}

inline auto getchardepth(PDFTEXPROGCLASS::internalfontnumber f, PDFTEXPROGCLASS::eightbits c)
{
    return PDFTEXPROG.getchardepth(f, c);
}

inline auto getcharheight(PDFTEXPROGCLASS::internalfontnumber f, PDFTEXPROGCLASS::eightbits c)
{
    return PDFTEXPROG.getcharheight(f, c);
}

inline auto getcharwidth(PDFTEXPROGCLASS::internalfontnumber f, PDFTEXPROGCLASS::eightbits c)
{
    return PDFTEXPROG.getcharwidth(f, c);
}

inline auto getpdfcompresslevel()
{
    return PDFTEXPROG.getpdfcompresslevel();
}

inline auto getpdfomitcharset()
{
    return PDFTEXPROG.getpdfomitcharset();
}

inline auto getquad(PDFTEXPROGCLASS::internalfontnumber f)
{
    return PDFTEXPROG.getquad(f);
}

inline auto getxheight(PDFTEXPROGCLASS::internalfontnumber f)
{
    return PDFTEXPROG.getxheight(f);
}

inline auto getnullstr()
{
    return PDFTEXPROG.getnullstr();
}

inline auto getpdfsuppresswarningdupmap()
{
    return PDFTEXPROG.getpdfsuppresswarningdupmap();
}

inline auto getslant(PDFTEXPROGCLASS::internalfontnumber f)
{
    return PDFTEXPROG.getslant(f);
}

inline auto makenamestring()
{
    return PDFTEXPROG.makenamestring();
}

inline auto makestring()
{
    return PDFTEXPROG.makestring();
}

inline auto packfilename(PDFTEXPROGCLASS::strnumber n, PDFTEXPROGCLASS::strnumber a, PDFTEXPROGCLASS::strnumber e)
{
    return PDFTEXPROG.packfilename(n, a, e);
}

inline auto pdfbegindict(C4P::C4P_integer i, C4P::C4P_integer pdfoslevel)
{
    PDFTEXPROG.pdfbegindict(i, pdfoslevel);
}

inline auto pdfbeginobj(C4P::C4P_integer i, C4P::C4P_integer pdfoslevel)
{
    PDFTEXPROG.pdfbeginobj(i, pdfoslevel);
}

inline auto pdfbeginstream()
{
    PDFTEXPROG.pdfbeginstream();
}

inline auto pdfcreateobj(C4P::C4P_integer t, C4P::C4P_integer i)
{
    PDFTEXPROG.pdfcreateobj(t, i);
}

inline auto pdfenddict()
{
    PDFTEXPROG.pdfenddict();
}

inline auto pdfendobj()
{
    PDFTEXPROG.pdfendobj();
}

inline auto pdfendstream()
{
    PDFTEXPROG.pdfendstream();
}
inline auto pdfflush()
{
    PDFTEXPROG.pdfflush();
}

inline auto pdfnewdict(C4P::C4P_integer t, C4P::C4P_integer i, C4P::C4P_integer pdfos)
{
    PDFTEXPROG.pdfnewdict(t, i, pdfos);
}

inline auto pdfnewobjnum()
{
    return PDFTEXPROG.pdfnewobjnum();
}

inline auto pdfosgetosbuf(C4P::C4P_integer s)
{
    PDFTEXPROG.pdfosgetosbuf(s);
}

inline auto pdfprint(PDFTEXPROGCLASS::strnumber s)
{
    PDFTEXPROG.pdfprint(s);
}

inline auto pdfprintreal(C4P::C4P_integer m, C4P::C4P_integer d)
{
    PDFTEXPROG.pdfprintreal(m, d);
}

inline auto getpdfsuppressptexinfo()
{
    return PDFTEXPROG.getpdfsuppressptexinfo();
}

inline auto getpdfsuppresswarningpagegroup()
{
    return PDFTEXPROG.getpdfsuppresswarningpagegroup();
}

inline auto getptexuseunderscore()
{
    return PDFTEXPROG.getptexuseunderscore();
}

inline auto print(C4P::C4P_integer s)
{
    PDFTEXPROG.print(s);
}

inline auto println()
{
    PDFTEXPROG.println();
}

inline auto tokenstostring(PDFTEXPROGCLASS::halfword p)
{
    return PDFTEXPROG.tokenstostring(p);
}

#include "pdftex.h"

#if WITH_SYNCTEX
#include "synctex.h"
#endif

inline void printid(PDFTEXPROGCLASS::strnumber filename)
{
    extern void printID(strnumber filename);
    printID(filename);
}

inline void printidalt(integer toks)
{
    extern void printIDalt(integer toks);
    printIDalt(toks);
}

template<typename FileType> int getbyte(FileType& f)
{
    unsigned char ret = *f;
    if (!feof(f))
    {
        f.Read();
    }
    return ret & 0xff;
}

#if defined(texbopenin)
#  undef texbopenin
#endif
template<typename FileType> bool texbopenin(FileType& f)
{
    return open_input(&f.fileref(), kpse_tex_format, FOPEN_RBIN_MODE);
};

#if defined(vfbopenin)
#  undef vfbopenin
#endif
template<typename FileType> bool vfbopenin(FileType& f)
{
    return miktexopenvffile(f, PDFTEXPROG.nameoffile);
}

#if !defined(__GNUC__)
#  define __attribute__(x)
#endif

int miktexloadpoolstrings(int size);

inline int loadpoolstrings(int size)
{
    return miktexloadpoolstrings(size);
}

extern PDFTEXAPPCLASS PDFTEXAPP;

inline char* gettexstring(PDFTEXPROGCLASS::strnumber stringNumber)
{
    return xstrdup(PDFTEXAPP.GetTeXString(stringNumber).c_str());
}

inline void miktexreallocatenameoffile(size_t n)
{

    PDFTEXPROG.nameoffile = reinterpret_cast<char*>(PDFTEXAPP.GetTeXMFMemoryHandler()->ReallocateArray("name_of_file", PDFTEXPROG.nameoffile, sizeof(*PDFTEXPROG.nameoffile), n, MIKTEX_SOURCE_LOCATION()));
    // special case: Web2C likes to add 1 to the nameoffile base address
    nameoffile = &PDFTEXPROG.nameoffile[-1];
}
