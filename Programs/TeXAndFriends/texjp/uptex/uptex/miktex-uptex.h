/**
 * @file miktex-uptex.h
 * @author Christian Schenk
 * @brief MiKTeX upTeX
 *
 * @copyright Copyright © 2021-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#pragma once

#include "miktex-uptex-config.h"

#if defined(MIKTEX_WINDOWS)
#   define MIKTEX_UTF8_WRAP_ALL 1
#   include <miktex/utf8wrap.h>
#endif

#include <iostream>

#include "miktex-uptex-version.h"

#include <miktex/TeXAndFriends/CharacterConverterImpl>
#include <miktex/TeXAndFriends/ErrorHandlerImpl>
#include <miktex/TeXAndFriends/FormatHandlerImpl>
#include <miktex/TeXAndFriends/InitFinalizeImpl>
#include <miktex/TeXAndFriends/InputOutputImpl>
#include <miktex/TeXAndFriends/StringHandlerImpl>
#include <miktex/TeXAndFriends/TeXApp>
#include <miktex/TeXAndFriends/TeXMemoryHandlerImpl>
#include <miktex/W2C/Emulation>

#include "uptexd.h"

#include "texmfmp.h"

#include <miktex/TeXjp/common.h>

extern UPTEXPROGCLASS UPTEXPROG;

class UPTEXAPPCLASS :
    public MiKTeX::TeXjp::TeXEngineBase<MiKTeX::TeXAndFriends::TeXApp>
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
        TeXApp::Init(args);
        IAm(MiKTeX::TeXAndFriends::TeXjpEngine);
        kpse_set_program_name(args[0], nullptr);
        initkanji();
        EnableFeature(MiKTeX::TeXAndFriends::Feature::EightBitChars);
        EnableFeature(MiKTeX::TeXAndFriends::Feature::TCX);
    }

    void AllocateMemory() override
    {
        TeXApp::AllocateMemory();
    }

    void FreeMemory() override
    {
        TeXApp::FreeMemory();
    }

    MiKTeX::Util::PathName GetMemoryDumpFileName() const override
    {
        return MiKTeX::Util::PathName("uptex.fmt");
    }

    std::string GetInitProgramName() const override
    {
        return "iniuptex";
    }

    std::string GetVirginProgramName() const override
    {
        return "viruptex";
    }

    std::string TheNameOfTheGame() const override
    {
        return "upTeX";
    }

private:

    MiKTeX::TeXAndFriends::CharacterConverterImpl<UPTEXPROGCLASS> charConv{ UPTEXPROG };
    MiKTeX::TeXAndFriends::ErrorHandlerImpl<UPTEXPROGCLASS> errorHandler{ UPTEXPROG };
    MiKTeX::TeXAndFriends::FormatHandlerImpl<UPTEXPROGCLASS> formatHandler{ UPTEXPROG };
    MiKTeX::TeXAndFriends::InitFinalizeImpl<UPTEXPROGCLASS> initFinalize{ UPTEXPROG };
    MiKTeX::TeXAndFriends::StringHandlerImpl<UPTEXPROGCLASS> stringHandler{ UPTEXPROG };
    MiKTeX::TeXjp::PTeXInputOutputImpl<MiKTeX::TeXAndFriends::InputOutputImpl<UPTEXPROGCLASS>, UPTEXPROGCLASS> inputOutput{ UPTEXPROG };
    MiKTeX::TeXjp::PTeXMemoryHandlerImpl<MiKTeX::TeXAndFriends::TeXMemoryHandlerImpl<UPTEXPROGCLASS>, UPTEXPROGCLASS> memoryHandler{ UPTEXPROG, *this };

};

int miktexloadpoolstrings(int size);

inline int loadpoolstrings(int size)
{
    return miktexloadpoolstrings(size);
}

extern UPTEXAPPCLASS UPTEXAPP;

inline char* gettexstring(UPTEXPROGCLASS::strnumber stringNumber)
{
  return xstrdup(UPTEXAPP.GetTeXString(stringNumber).c_str());
}

inline void miktexreallocatenameoffile(size_t n)
{  
    UPTEXPROG.nameoffile = reinterpret_cast<char*>(UPTEXAPP.GetTeXMFMemoryHandler()->ReallocateArray("name_of_file", UPTEXPROG.nameoffile, sizeof(*UPTEXPROG.nameoffile), n, MIKTEX_SOURCE_LOCATION()));
}

#if defined(MIKTEX_UPTEX_CPP)
#define UPTEX_PROG_VAR2(alias, name, type) type& alias = UPTEXPROG.name
#define UPTEX_PROG_VAR(name, type) type& name = UPTEXPROG.name
#else
#define UPTEX_PROG_VAR2(alias, name, type) extern type& alias
#define UPTEX_PROG_VAR(name, type) extern type& name
#endif

UPTEX_PROG_VAR(curh, UPTEXPROGCLASS::scaled);
UPTEX_PROG_VAR(curinput, UPTEXPROGCLASS::instaterecord);
UPTEX_PROG_VAR(curv, UPTEXPROGCLASS::scaled);
UPTEX_PROG_VAR(eqtb, UPTEXPROGCLASS::memoryword*);
UPTEX_PROG_VAR(jobname, UPTEXPROGCLASS::strnumber);
UPTEX_PROG_VAR(ruledp, UPTEXPROGCLASS::scaled);
UPTEX_PROG_VAR(ruleht, UPTEXPROGCLASS::scaled);
UPTEX_PROG_VAR(rulewd, UPTEXPROGCLASS::scaled);
UPTEX_PROG_VAR(termoffset, C4P::C4P_integer);
UPTEX_PROG_VAR(totalpages, C4P::C4P_integer);
UPTEX_PROG_VAR(zmem, UPTEXPROGCLASS::memoryword*);
UPTEX_PROG_VAR2(texmflogname, logname, UPTEXPROGCLASS::strnumber);

using halfword = UPTEXPROGCLASS::halfword;

#if WITH_SYNCTEX
UPTEX_PROG_VAR(synctexoffset, C4P::C4P_integer);
UPTEX_PROG_VAR(synctexoption, C4P::C4P_integer);
#include "synctex.h"
#endif
