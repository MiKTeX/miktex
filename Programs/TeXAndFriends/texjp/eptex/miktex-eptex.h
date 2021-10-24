/* miktex-eptex.h:

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

#include <miktex/eptex.h>

extern EPTEXPROGCLASS EPTEXPROG;

class MemoryHandlerImpl :
    public MiKTeX::TeXAndFriends::ETeXMemoryHandlerImpl<EPTEXPROGCLASS>
{
public:
    MemoryHandlerImpl(EPTEXPROGCLASS& program, MiKTeX::TeXAndFriends::TeXMFApp& texmfapp) :
        ETeXMemoryHandlerImpl<EPTEXPROGCLASS>(program, texmfapp)
    {
    }

public:
    void Allocate(const std::unordered_map<std::string, int>& userParams) override
    {
        ETeXMemoryHandlerImpl<EPTEXPROGCLASS>::Allocate(userParams);
        MIKTEX_ASSERT(program.constfontbase == 0);
        size_t nFonts = program.fontmax - program.constfontbase;
        AllocateArray("fontdir", program.fontdir, nFonts);
        AllocateArray("fontnumext", program.fontnumext, nFonts);
        AllocateArray("ctypebase", program.ctypebase, nFonts);
    }

public:
    void Free() override
    {
        ETeXMemoryHandlerImpl<EPTEXPROGCLASS>::Free();
        FreeArray("fontdir", program.fontdir);
        FreeArray("fontnumext", program.fontnumext);
        FreeArray("ctypebase", program.ctypebase);
    }

public:
    void Check() override
    {
        ETeXMemoryHandlerImpl<EPTEXPROGCLASS>::Check();
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontdir);
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontnumext);
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.ctypebase);
    }
};

class EPTEXAPPCLASS :
    public MiKTeX::TeXAndFriends::ETeXApp
{
public:
    enum {
        OPT_KANJI = 10000,
        OPT_KANJI_INTERNAL,
    };

private:
    MiKTeX::TeXAndFriends::CharacterConverterImpl<EPTEXPROGCLASS> charConv{ EPTEXPROG };

private:
    MiKTeX::TeXAndFriends::ErrorHandlerImpl<EPTEXPROGCLASS> errorHandler{ EPTEXPROG };

private:
    MiKTeX::TeXAndFriends::FormatHandlerImpl<EPTEXPROGCLASS> formatHandler{ EPTEXPROG };

private:
    MiKTeX::TeXAndFriends::InitFinalizeImpl<EPTEXPROGCLASS> initFinalize{ EPTEXPROG };

private:
    MiKTeX::TeXAndFriends::InputOutputImpl<EPTEXPROGCLASS> inputOutput{ EPTEXPROG };

private:
    MiKTeX::TeXAndFriends::StringHandlerImpl<EPTEXPROGCLASS> stringHandler{ EPTEXPROG };

private:
    MemoryHandlerImpl memoryHandler{ EPTEXPROG, *this };

private:
    std::string T_(const char* msgId)
    {
        return msgId;
    }

public:
    void AddOptions() override
    {
        ETeXApp::AddOptions();
        AddOption("kanji", T_("set Japanese encoding (ENC=euc|jis|sjis|utf8)."), OPT_KANJI, POPT_ARG_STRING, "ENC");
        AddOption("kanji-internal", T_("set Japanese internal encoding (ENC=euc|sjis)."), OPT_KANJI_INTERNAL, POPT_ARG_STRING, "ENC");
    }

public:
    bool ProcessOption(int opt, const std::string& optArg) override
    {
        bool done = true;
        switch (opt)
        {
        case OPT_KANJI:
            if (!set_enc_string (optArg.c_str(), nullptr))
            {
                std::cerr << T_("Unknown encoding: ") << optArg << std::endl;
                throw 1;
            }
            break;
        case OPT_KANJI_INTERNAL:
            if (!set_enc_string (nullptr, optArg.c_str()))
            {
                std::cerr << T_("Unknown encoding: ") << optArg << std::endl;
                throw 1;
            }
            break;
        default:
            done = ETeXApp::ProcessOption(opt, optArg);
            break;
        }
        return done;
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
        initkanji();
        kpse_set_program_name(args[0], nullptr);
        EnableFeature(MiKTeX::TeXAndFriends::Feature::EightBitChars);
#if defined(IMPLEMENT_TCX)
        EnableFeature(MiKTeX::TeXAndFriends::Feature::TCX);
#endif
    }

public:
    size_t InputLineInternal(FILE* f, char* buffer, size_t bufferSize, size_t bufferPosition, int& lastChar) const override
    {
        return static_cast<size_t>(input_line2(f, reinterpret_cast<unsigned char*>(buffer), static_cast<long>(bufferPosition), static_cast<long>(bufferSize), &lastChar));
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
