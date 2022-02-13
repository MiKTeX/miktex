/* miktex-eptex.h:

   Copyright (C) 2021-2022 Christian Schenk

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

#include "miktex-eptex-config.h"

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
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
    public MiKTeX::TeXjp::WebAppInputLine<MiKTeX::TeXAndFriends::ETeXApp>
{
private:
    MiKTeX::TeXAndFriends::CharacterConverterImpl<EPTEXPROGCLASS> charConv{ EPTEXPROG };

private:
    MiKTeX::TeXAndFriends::ErrorHandlerImpl<EPTEXPROGCLASS> errorHandler{ EPTEXPROG };

private:
    MiKTeX::TeXAndFriends::FormatHandlerImpl<EPTEXPROGCLASS> formatHandler{ EPTEXPROG };

private:
    MiKTeX::TeXAndFriends::InitFinalizeImpl<EPTEXPROGCLASS> initFinalize{ EPTEXPROG };

private:
    MiKTeX::TeXjp::PTeXInputOutputImpl<MiKTeX::TeXAndFriends::InputOutputImpl<EPTEXPROGCLASS>, EPTEXPROGCLASS> inputOutput{ EPTEXPROG };

private:
    MiKTeX::TeXAndFriends::StringHandlerImpl<EPTEXPROGCLASS> stringHandler{ EPTEXPROG };

private:
    MiKTeX::TeXjp::PTeXMemoryHandlerImpl<MiKTeX::TeXAndFriends::ETeXMemoryHandlerImpl<EPTEXPROGCLASS>, EPTEXPROGCLASS> memoryHandler{ EPTEXPROG, *this };

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
        initkanji();
        kpse_set_program_name(args[0], nullptr);
        EnableFeature(MiKTeX::TeXAndFriends::Feature::EightBitChars);
#if defined(IMPLEMENT_TCX)
        EnableFeature(MiKTeX::TeXAndFriends::Feature::TCX);
#endif
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
        return MiKTeX::Util::PathName("eptex.fmt");
    }

public:
    std::string GetInitProgramName() const override
    {
        return "inieptex";
    }

public:
    std::string GetVirginProgramName() const override
    {
        return "vireptex";
    }

public:
    std::string TheNameOfTheGame() const override
    {
        return "e-pTeX";
    }
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
