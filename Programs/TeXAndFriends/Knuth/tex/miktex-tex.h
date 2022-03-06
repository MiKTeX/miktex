/**
 * @file miktex-tex.h
 * @author Christian Schenk
 * @brief MiKTeX TeX
 *
 * @copyright Copyright © 1991-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#pragma once

#include "miktex-tex-config.h"

#include "miktex-tex-version.h"

#include <miktex/TeXAndFriends/CharacterConverterImpl>
#include <miktex/TeXAndFriends/ErrorHandlerImpl>
#include <miktex/TeXAndFriends/FormatHandlerImpl>
#include <miktex/TeXAndFriends/InitFinalizeImpl>
#include <miktex/TeXAndFriends/InputOutputImpl>
#include <miktex/TeXAndFriends/StringHandlerImpl>
#include <miktex/TeXAndFriends/TeXApp>
#include <miktex/TeXAndFriends/TeXMemoryHandlerImpl>
#include <miktex/W2C/Emulation>

#include "tex.h"

#if defined(MIKTEX_WINDOWS)
#   include "miktex-tex.rc"
#endif

#if !defined(MIKTEXHELP_TEX)
#   include <miktex/Core/Help>
#endif

extern TEXPROGCLASS TEXPROG;

class TEXAPPCLASS :
  public MiKTeX::TeXAndFriends::TeXApp
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
    #if defined(IMPLEMENT_TCX)
        EnableFeature(MiKTeX::TeXAndFriends::Feature::EightBitChars);
        EnableFeature(MiKTeX::TeXAndFriends::Feature::TCX);
    #endif
    }

    MiKTeX::Util::PathName GetMemoryDumpFileName() const override
    {
        return MiKTeX::Util::PathName("tex.fmt");
    }

    std::string GetInitProgramName() const override
    {
        return "initex";
    }

    std::string GetVirginProgramName() const override
    {
        return "virtex";
    }

    std::string TheNameOfTheGame() const override
    {
        return "TeX";
    }

    unsigned long GetHelpId() const override
    {
        return MIKTEXHELP_TEX;
    }

private:

    MiKTeX::TeXAndFriends::CharacterConverterImpl<TEXPROGCLASS> charConv{ TEXPROG };
    MiKTeX::TeXAndFriends::ErrorHandlerImpl<TEXPROGCLASS> errorHandler{ TEXPROG };
    MiKTeX::TeXAndFriends::FormatHandlerImpl<TEXPROGCLASS> formatHandler{ TEXPROG };
    MiKTeX::TeXAndFriends::InitFinalizeImpl<TEXPROGCLASS> initFinalize{ TEXPROG };
    MiKTeX::TeXAndFriends::InputOutputImpl<TEXPROGCLASS> inputOutput{ TEXPROG };
    MiKTeX::TeXAndFriends::StringHandlerImpl<TEXPROGCLASS> stringHandler{ TEXPROG };
    MiKTeX::TeXAndFriends::TeXMemoryHandlerImpl<TEXPROGCLASS> memoryHandler { TEXPROG, *this };
};

int miktexloadpoolstrings(int size);

inline int loadpoolstrings(int size)
{
    return miktexloadpoolstrings(size);
}

extern TEXAPPCLASS TEXAPP;

inline void miktexreallocatenameoffile(size_t n)
{
    TEXPROG.nameoffile = reinterpret_cast<char*>(TEXAPP.GetTeXMFMemoryHandler()->ReallocateArray("name_of_file", TEXPROG.nameoffile, sizeof(*TEXPROG.nameoffile), n, MIKTEX_SOURCE_LOCATION()));
}
