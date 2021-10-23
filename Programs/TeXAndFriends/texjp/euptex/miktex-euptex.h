/* miktex-euptex.h:

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

#include "miktex-euptex-config.h"

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
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

#include <miktex/euptex.h>

extern EUPTEXPROGCLASS EUPTEXPROG;

class MemoryHandlerImpl :
    public MiKTeX::TeXAndFriends::ETeXMemoryHandlerImpl<EUPTEXPROGCLASS>
{
public:
    MemoryHandlerImpl(EUPTEXPROGCLASS& program, MiKTeX::TeXAndFriends::TeXMFApp& texmfapp) :
        ETeXMemoryHandlerImpl<EUPTEXPROGCLASS>(program, texmfapp)
    {
    }

public:
    void Allocate(const std::unordered_map<std::string, int>& userParams) override
    {
        ETeXMemoryHandlerImpl<EUPTEXPROGCLASS>::Allocate(userParams);
        MIKTEX_ASSERT(program.constfontbase == 0);
        size_t nFonts = program.fontmax - program.constfontbase;
        AllocateArray("fontdir", program.fontdir, nFonts);
        AllocateArray("fontnumext", program.fontnumext, nFonts);
        AllocateArray("ctypebase", program.ctypebase, nFonts);
    }

public:
    void Free() override
    {
        ETeXMemoryHandlerImpl<EUPTEXPROGCLASS>::Free();
        FreeArray("fontdir", program.fontdir);
        FreeArray("fontnumext", program.fontnumext);
        FreeArray("ctypebase", program.ctypebase);
    }

public:
    void Check() override
    {
        ETeXMemoryHandlerImpl<EUPTEXPROGCLASS>::Check();
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontdir);
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontnumext);
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.ctypebase);
    }
};

class EUPTEXAPPCLASS :
    public MiKTeX::TeXAndFriends::ETeXApp
{
public:
    enum {
        OPT_KANJI = 10000,
        OPT_KANJI_INTERNAL,
    };

private:
    MiKTeX::TeXAndFriends::CharacterConverterImpl<EUPTEXPROGCLASS> charConv{ EUPTEXPROG };

private:
    MiKTeX::TeXAndFriends::ErrorHandlerImpl<EUPTEXPROGCLASS> errorHandler{ EUPTEXPROG };

private:
    MiKTeX::TeXAndFriends::FormatHandlerImpl<EUPTEXPROGCLASS> formatHandler{ EUPTEXPROG };

private:
    MiKTeX::TeXAndFriends::InitFinalizeImpl<EUPTEXPROGCLASS> initFinalize{ EUPTEXPROG };

private:
    MiKTeX::TeXAndFriends::InputOutputImpl<EUPTEXPROGCLASS> inputOutput{ EUPTEXPROG };

private:
    MiKTeX::TeXAndFriends::StringHandlerImpl<EUPTEXPROGCLASS> stringHandler{ EUPTEXPROG };

private:
    MemoryHandlerImpl memoryHandler{ EUPTEXPROG, *this };

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
                std::cerr << MIKTEXTEXT("Unknown encoding: ") << optArg << std::endl;
                throw 1;
            }
            break;
        case OPT_KANJI_INTERNAL:
            if (!set_enc_string (nullptr, optArg.c_str()))
            {
                std::cerr << MIKTEXTEXT("Unknown encoding: ") << optArg << std::endl;
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
        return MiKTeX::Util::PathName("euptex.fmt");
    }

public:
    std::string GetInitProgramName() const override
    {
        return "inieuptex";
    }

public:
    std::string GetVirginProgramName() const override
    {
        return "vireuptex";
    }

public:
    std::string TheNameOfTheGame() const override
    {
        return "eu-pTeX";
    }
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
