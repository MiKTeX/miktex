/**
 * @file miktex-eptex.h
 * @author Christian Schenk
 * @brief MiKTeX e-pTeX
 *
 * @copyright Copyright © 2021-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#pragma once

#include "miktex-eptex-config.h"

#if defined(MIKTEX_WINDOWS)
#   define MIKTEX_UTF8_WRAP_ALL 1
#   include <miktex/utf8wrap.h>
#endif

#include <iostream>

#include "miktex-eptex-version.h"

#include <miktex/TeXAndFriends/CharacterConverterImpl>
#include <miktex/TeXAndFriends/ETeXApp>
#include <miktex/TeXAndFriends/ETeXMemoryHandlerImpl>
#include <miktex/TeXAndFriends/ErrorHandlerImpl>
#include <miktex/TeXAndFriends/FormatHandlerImpl>
#include <miktex/TeXAndFriends/InitFinalizeImpl>
#include <miktex/TeXAndFriends/InputOutputImpl>
#include <miktex/TeXAndFriends/StringHandlerImpl>
#include <miktex/W2C/Emulation>

#include "eptexd.h"

#include "texmfmp.h"

#include <miktex/TeXjp/common.h>

extern EPTEXPROGCLASS EPTEXPROG;

class EPTEXAPPCLASS :
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
        initkanji();
        kpse_set_program_name(args[0], nullptr);
        EnableFeature(MiKTeX::TeXAndFriends::Feature::EightBitChars);
#if defined(IMPLEMENT_TCX)
        EnableFeature(MiKTeX::TeXAndFriends::Feature::TCX);
#endif
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
        return MiKTeX::Util::PathName("eptex.fmt");
    }

    std::string GetInitProgramName() const override
    {
        return "inieptex";
    }

    std::string GetVirginProgramName() const override
    {
        return "vireptex";
    }

    std::string TheNameOfTheGame() const override
    {
        return "e-pTeX";
    }

private:

    MiKTeX::TeXAndFriends::CharacterConverterImpl<EPTEXPROGCLASS> charConv{ EPTEXPROG };
    MiKTeX::TeXAndFriends::ErrorHandlerImpl<EPTEXPROGCLASS> errorHandler{ EPTEXPROG };
    MiKTeX::TeXAndFriends::FormatHandlerImpl<EPTEXPROGCLASS> formatHandler{ EPTEXPROG };
    MiKTeX::TeXAndFriends::InitFinalizeImpl<EPTEXPROGCLASS> initFinalize{ EPTEXPROG };
    MiKTeX::TeXAndFriends::StringHandlerImpl<EPTEXPROGCLASS> stringHandler{ EPTEXPROG };
    MiKTeX::TeXjp::PTeXInputOutputImpl<MiKTeX::TeXAndFriends::InputOutputImpl<EPTEXPROGCLASS>, EPTEXPROGCLASS> inputOutput{ EPTEXPROG };
    MiKTeX::TeXjp::PTeXMemoryHandlerImpl<MiKTeX::TeXAndFriends::ETeXMemoryHandlerImpl<EPTEXPROGCLASS>, EPTEXPROGCLASS> memoryHandler{ EPTEXPROG, *this };
};

int miktexloadpoolstrings(int size);

inline int loadpoolstrings(int size)
{
    return miktexloadpoolstrings(size);
}

extern EPTEXAPPCLASS EPTEXAPP;

inline char* gettexstring(EPTEXPROGCLASS::strnumber stringNumber)
{
  return xstrdup(EPTEXAPP.GetTeXString(stringNumber).c_str());
}

inline void miktexreallocatenameoffile(size_t n)
{  
    EPTEXPROG.nameoffile = reinterpret_cast<char*>(EPTEXAPP.GetTeXMFMemoryHandler()->ReallocateArray("name_of_file", EPTEXPROG.nameoffile, sizeof(*EPTEXPROG.nameoffile), n, MIKTEX_SOURCE_LOCATION()));
}

#if defined(MIKTEX_EPTEX_CPP)
#define EPTEX_PROG_VAR2(alias, name, type) type& alias = EPTEXPROG.name
#define EPTEX_PROG_VAR(name, type) type& name = EPTEXPROG.name
#else
#define EPTEX_PROG_VAR2(alias, name, type) extern type& alias
#define EPTEX_PROG_VAR(name, type) extern type& name
#endif

EPTEX_PROG_VAR(curh, EPTEXPROGCLASS::scaled);
EPTEX_PROG_VAR(curinput, EPTEXPROGCLASS::instaterecord);
EPTEX_PROG_VAR(curv, EPTEXPROGCLASS::scaled);
EPTEX_PROG_VAR(eqtb, EPTEXPROGCLASS::memoryword*);
EPTEX_PROG_VAR(jobname, EPTEXPROGCLASS::strnumber);
EPTEX_PROG_VAR(poolptr, EPTEXPROGCLASS::poolpointer);
EPTEX_PROG_VAR(poolsize, C4P::C4P_integer);
EPTEX_PROG_VAR(ruledp, EPTEXPROGCLASS::scaled);
EPTEX_PROG_VAR(ruleht, EPTEXPROGCLASS::scaled);
EPTEX_PROG_VAR(rulewd, EPTEXPROGCLASS::scaled);
EPTEX_PROG_VAR(strpool, EPTEXPROGCLASS::packedasciicode*);
EPTEX_PROG_VAR(strstart, EPTEXPROGCLASS::poolpointer*);
EPTEX_PROG_VAR(termoffset, C4P::C4P_integer);
EPTEX_PROG_VAR(totalpages, C4P::C4P_integer);
EPTEX_PROG_VAR(zmem, EPTEXPROGCLASS::memoryword*);
EPTEX_PROG_VAR2(texmflogname, logname, EPTEXPROGCLASS::strnumber);

using halfword = EPTEXPROGCLASS::halfword;
using poolpointer = EPTEXPROGCLASS::poolpointer;
using strnumber = EPTEXPROGCLASS::strnumber;

inline auto print(C4P::C4P_integer s)
{
    EPTEXPROG.print(s);
}

inline auto println()
{
    EPTEXPROG.println();
}

#if WITH_SYNCTEX
EPTEX_PROG_VAR(synctexoffset, C4P::C4P_integer);
EPTEX_PROG_VAR(synctexoption, C4P::C4P_integer);
#include "synctex.h"
#endif
