/**
 * @file miktex-euptex.h
 * @author Christian Schenk
 * @brief MiKTeX e-upTeX
 *
 * @copyright Copyright © 2021-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#pragma once

#include "miktex-euptex-config.h"

#if defined(MIKTEX_WINDOWS)
#   define MIKTEX_UTF8_WRAP_ALL 1
#   include <miktex/utf8wrap.h>
#endif

#include <iostream>

#include "miktex-euptex-version.h"

#include <miktex/TeXAndFriends/CharacterConverterImpl>
#include <miktex/TeXAndFriends/ETeXApp>
#include <miktex/TeXAndFriends/ETeXMemoryHandlerImpl>
#include <miktex/TeXAndFriends/ErrorHandlerImpl>
#include <miktex/TeXAndFriends/FormatHandlerImpl>
#include <miktex/TeXAndFriends/InitFinalizeImpl>
#include <miktex/TeXAndFriends/InputOutputImpl>
#include <miktex/TeXAndFriends/StringHandlerImpl>
#include <miktex/W2C/Emulation>

#include "euptexd.h"

#include "texmfmp.h"

#include <miktex/TeXjp/common.h>

extern EUPTEXPROGCLASS EUPTEXPROG;

class EUPTEXAPPCLASS :
    public MiKTeX::TeXjp::TeXEngineBase<MiKTeX::TeXAndFriends::ETeXApp>
{

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
        IAm(MiKTeX::TeXAndFriends::TeXjpEngine);
        InitKanji();
        kpse_set_program_name(args[0], nullptr);
        EnableFeature(MiKTeX::TeXAndFriends::Feature::EightBitChars);
        EnableFeature(MiKTeX::TeXAndFriends::Feature::TCX);
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
        return MiKTeX::Util::PathName("euptex.fmt");
    }

    std::string GetInitProgramName() const override
    {
        return "inieuptex";
    }

    std::string GetVirginProgramName() const override
    {
        return "vireuptex";
    }

    std::string TheNameOfTheGame() const override
    {
        return "eu-pTeX";
    }

private:

    MiKTeX::TeXAndFriends::CharacterConverterImpl<EUPTEXPROGCLASS> charConv{ EUPTEXPROG };
    MiKTeX::TeXAndFriends::ErrorHandlerImpl<EUPTEXPROGCLASS> errorHandler{ EUPTEXPROG };
    MiKTeX::TeXAndFriends::FormatHandlerImpl<EUPTEXPROGCLASS> formatHandler{ EUPTEXPROG };
    MiKTeX::TeXAndFriends::InitFinalizeImpl<EUPTEXPROGCLASS> initFinalize{ EUPTEXPROG };
    MiKTeX::TeXAndFriends::StringHandlerImpl<EUPTEXPROGCLASS> stringHandler{ EUPTEXPROG };
    MiKTeX::TeXjp::PTeXInputOutputImpl<MiKTeX::TeXAndFriends::InputOutputImpl<EUPTEXPROGCLASS>, EUPTEXPROGCLASS> inputOutput{ EUPTEXPROG };
    MiKTeX::TeXjp::PTeXMemoryHandlerImpl<MiKTeX::TeXAndFriends::ETeXMemoryHandlerImpl<EUPTEXPROGCLASS>, EUPTEXPROGCLASS> memoryHandler{ EUPTEXPROG, *this };
};

int miktexloadpoolstrings(int size);

inline int loadpoolstrings(int size)
{
    return miktexloadpoolstrings(size);
}

extern EUPTEXAPPCLASS EUPTEXAPP;

inline char* gettexstring(EUPTEXPROGCLASS::strnumber stringNumber)
{
  return xstrdup(EUPTEXAPP.GetTeXString(stringNumber).c_str());
}

inline void miktexreallocatenameoffile(size_t n)
{  
    EUPTEXPROG.nameoffile = reinterpret_cast<char*>(EUPTEXAPP.GetTeXMFMemoryHandler()->ReallocateArray("name_of_file", EUPTEXPROG.nameoffile, sizeof(*EUPTEXPROG.nameoffile), n, MIKTEX_SOURCE_LOCATION()));
}

#if defined(MIKTEX_EUPTEX_CPP)
#define EUPTEX_PROG_VAR2(alias, name, type) type& alias = EUPTEXPROG.name
#define EUPTEX_PROG_VAR(name, type) type& name = EUPTEXPROG.name
#else
#define EUPTEX_PROG_VAR2(alias, name, type) extern type& alias
#define EUPTEX_PROG_VAR(name, type) extern type& name
#endif

EUPTEX_PROG_VAR(curh, EUPTEXPROGCLASS::scaled);
EUPTEX_PROG_VAR(curinput, EUPTEXPROGCLASS::instaterecord);
EUPTEX_PROG_VAR(curv, EUPTEXPROGCLASS::scaled);
EUPTEX_PROG_VAR(eqtb, EUPTEXPROGCLASS::memoryword*);
EUPTEX_PROG_VAR(jobname, EUPTEXPROGCLASS::strnumber);
EUPTEX_PROG_VAR(poolptr, EUPTEXPROGCLASS::poolpointer);
EUPTEX_PROG_VAR(poolsize, C4P::C4P_integer);
EUPTEX_PROG_VAR(ruledp, EUPTEXPROGCLASS::scaled);
EUPTEX_PROG_VAR(ruleht, EUPTEXPROGCLASS::scaled);
EUPTEX_PROG_VAR(rulewd, EUPTEXPROGCLASS::scaled);
EUPTEX_PROG_VAR(strpool, EUPTEXPROGCLASS::packedasciicode*);
EUPTEX_PROG_VAR(strstart, EUPTEXPROGCLASS::poolpointer*);
EUPTEX_PROG_VAR(termoffset, C4P::C4P_integer);
EUPTEX_PROG_VAR(totalpages, C4P::C4P_integer);
EUPTEX_PROG_VAR(zmem, EUPTEXPROGCLASS::memoryword*);
EUPTEX_PROG_VAR2(texmflogname, logname, EUPTEXPROGCLASS::strnumber);

using halfword = EUPTEXPROGCLASS::halfword;
using poolpointer = EUPTEXPROGCLASS::poolpointer;
using strnumber = EUPTEXPROGCLASS::strnumber;

inline auto print(C4P::C4P_integer s)
{
    EUPTEXPROG.print(s);
}

inline auto println()
{
    EUPTEXPROG.println();
}

#if WITH_SYNCTEX
EUPTEX_PROG_VAR(synctexoffset, C4P::C4P_integer);
EUPTEX_PROG_VAR(synctexoption, C4P::C4P_integer);
#include "synctex.h"
#endif
