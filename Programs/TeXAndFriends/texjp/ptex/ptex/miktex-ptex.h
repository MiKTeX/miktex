/* miktex-ptex.h:

   Copyright (C) 2021 Christian Schenk

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

#include "miktex-ptex-config.h"

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif

#include <iostream>

#include "miktex-ptex-version.h"

#include <miktex/TeXAndFriends/CharacterConverterImpl>
#include <miktex/TeXAndFriends/ErrorHandlerImpl>
#include <miktex/TeXAndFriends/FormatHandlerImpl>
#include <miktex/TeXAndFriends/InitFinalizeImpl>
#include <miktex/TeXAndFriends/InputOutputImpl>
#include <miktex/TeXAndFriends/StringHandlerImpl>
#include <miktex/TeXAndFriends/TeXApp>
#include <miktex/TeXAndFriends/TeXMemoryHandlerImpl>
#include <miktex/W2C/Emulation>

#include "ptexd.h"

#include "texmfmp.h"

#include <miktex/ptex.h>

#include <miktex/TeXjp/common.h>

extern PTEXPROGCLASS PTEXPROG;

class PTEXAPPCLASS :
    public MiKTeX::TeXjp::WebAppInputLine<MiKTeX::TeXAndFriends::TeXApp>
{
private:
    MiKTeX::TeXAndFriends::CharacterConverterImpl<PTEXPROGCLASS> charConv{ PTEXPROG };

private:
    MiKTeX::TeXAndFriends::ErrorHandlerImpl<PTEXPROGCLASS> errorHandler{ PTEXPROG };

private:
    MiKTeX::TeXAndFriends::FormatHandlerImpl<PTEXPROGCLASS> formatHandler{ PTEXPROG };

private:
    MiKTeX::TeXAndFriends::InitFinalizeImpl<PTEXPROGCLASS> initFinalize{ PTEXPROG };

private:
    MiKTeX::TeXAndFriends::InputOutputImpl<PTEXPROGCLASS> inputOutput{ PTEXPROG };

private:
    MiKTeX::TeXAndFriends::StringHandlerImpl<PTEXPROGCLASS> stringHandler{ PTEXPROG };

private:
    MiKTeX::TeXjp::PTeXMemoryHandlerImpl<MiKTeX::TeXAndFriends::TeXMemoryHandlerImpl<PTEXPROGCLASS>, PTEXPROGCLASS> memoryHandler{ PTEXPROG, *this };

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
        kpse_set_program_name(args[0], nullptr);
        initkanji();
        EnableFeature(MiKTeX::TeXAndFriends::Feature::EightBitChars);
#if defined(IMPLEMENT_TCX)
        EnableFeature(MiKTeX::TeXAndFriends::Feature::TCX);
#endif
    }

public:
    void AllocateMemory() override
    {
        TeXApp::AllocateMemory();
    }

public:
    void FreeMemory() override
    {
        TeXApp::FreeMemory();
    }

public:
    MiKTeX::Util::PathName GetMemoryDumpFileName() const override
    {
        return MiKTeX::Util::PathName("ptex.fmt");
    }

public:
    std::string GetInitProgramName() const override
    {
        return "iniptex";
    }

public:
    std::string GetVirginProgramName() const override
    {
        return "virptex";
    }

public:
    std::string TheNameOfTheGame() const override
    {
        return "pTeX";
    }
};

int miktexloadpoolstrings(int size);

inline int loadpoolstrings(int size)
{
    return miktexloadpoolstrings(size);
}

extern PTEXAPPCLASS PTEXAPP;

inline char* gettexstring(PTEXPROGCLASS::strnumber stringNumber)
{
  return xstrdup(PTEXAPP.GetTeXString(stringNumber).c_str());
}

inline void miktexreallocatenameoffile(size_t n)
{  
    PTEXPROG.nameoffile = reinterpret_cast<char*>(PTEXAPP.GetTeXMFMemoryHandler()->ReallocateArray("name_of_file", PTEXPROG.nameoffile, sizeof(*PTEXPROG.nameoffile), n, MIKTEX_SOURCE_LOCATION()));
}

#if defined(MIKTEX_PTEX_CPP)
#define PTEX_PROG_VAR2(alias, name, type) type& alias = PTEXPROG.name
#define PTEX_PROG_VAR(name, type) type& name = PTEXPROG.name
#else
#define PTEX_PROG_VAR2(alias, name, type) extern type& alias
#define PTEX_PROG_VAR(name, type) extern type& name
#endif

PTEX_PROG_VAR(curh, PTEXPROGCLASS::scaled);
PTEX_PROG_VAR(curinput, PTEXPROGCLASS::instaterecord);
PTEX_PROG_VAR(curv, PTEXPROGCLASS::scaled);
PTEX_PROG_VAR(eqtb, PTEXPROGCLASS::memoryword*);
PTEX_PROG_VAR(jobname, PTEXPROGCLASS::strnumber);
PTEX_PROG_VAR(ruledp, PTEXPROGCLASS::scaled);
PTEX_PROG_VAR(ruleht, PTEXPROGCLASS::scaled);
PTEX_PROG_VAR(rulewd, PTEXPROGCLASS::scaled);
PTEX_PROG_VAR(termoffset, C4P::C4P_integer);
PTEX_PROG_VAR(totalpages, C4P::C4P_integer);
PTEX_PROG_VAR(zmem, PTEXPROGCLASS::memoryword*);
PTEX_PROG_VAR2(texmflogname, logname, PTEXPROGCLASS::strnumber);

using halfword = PTEXPROGCLASS::halfword;

#if WITH_SYNCTEX
PTEX_PROG_VAR(synctexoffset, C4P::C4P_integer);
PTEX_PROG_VAR(synctexoption, C4P::C4P_integer);
#include "synctex.h"
#endif
