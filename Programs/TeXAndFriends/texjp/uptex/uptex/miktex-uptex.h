/* miktex-uptex.h:

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

#include "miktex-uptex-config.h"

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
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

#include <miktex/uptex.h>

extern UPTEXPROGCLASS UPTEXPROG;

class MemoryHandlerImpl :
    public MiKTeX::TeXAndFriends::TeXMemoryHandlerImpl<UPTEXPROGCLASS>
{
public:
    MemoryHandlerImpl(UPTEXPROGCLASS& program, MiKTeX::TeXAndFriends::TeXMFApp& texmfapp) :
        TeXMemoryHandlerImpl<UPTEXPROGCLASS>(program, texmfapp)
    {
    }

public:
    void Allocate(const std::unordered_map<std::string, int>& userParams) override
    {
        TeXMemoryHandlerImpl<UPTEXPROGCLASS>::Allocate(userParams);
        MIKTEX_ASSERT(program.constfontbase == 0);
        size_t nFonts = program.fontmax - program.constfontbase;
        AllocateArray("fontdir", program.fontdir, nFonts);
        AllocateArray("fontnumext", program.fontnumext, nFonts);
        AllocateArray("ctypebase", program.ctypebase, nFonts);
    }

public:
    void Free() override
    {
        TeXMemoryHandlerImpl<UPTEXPROGCLASS>::Free();
        FreeArray("fontdir", program.fontdir);
        FreeArray("fontnumext", program.fontnumext);
        FreeArray("ctypebase", program.ctypebase);
    }

public:
    void Check() override
    {
        TeXMemoryHandlerImpl<UPTEXPROGCLASS>::Check();
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontdir);
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.fontnumext);
        MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.ctypebase);
    }
};

class UPTEXAPPCLASS :
    public MiKTeX::TeXAndFriends::TeXApp
{
public:
    enum {
        OPT_KANJI = 10000,
        OPT_KANJI_INTERNAL,
    };

private:
    MiKTeX::TeXAndFriends::CharacterConverterImpl<UPTEXPROGCLASS> charConv{ UPTEXPROG };

private:
    MiKTeX::TeXAndFriends::ErrorHandlerImpl<UPTEXPROGCLASS> errorHandler{ UPTEXPROG };

private:
    MiKTeX::TeXAndFriends::FormatHandlerImpl<UPTEXPROGCLASS> formatHandler{ UPTEXPROG };

private:
    MiKTeX::TeXAndFriends::InitFinalizeImpl<UPTEXPROGCLASS> initFinalize{ UPTEXPROG };

private:
    MiKTeX::TeXAndFriends::InputOutputImpl<UPTEXPROGCLASS> inputOutput{ UPTEXPROG };

private:
    MiKTeX::TeXAndFriends::StringHandlerImpl<UPTEXPROGCLASS> stringHandler{ UPTEXPROG };

private:
    MemoryHandlerImpl memoryHandler{ UPTEXPROG, *this };

private:
    std::string T_(const char* msgId)
    {
        return msgId;
    }

public:
    void AddOptions() override
    {
        TeXApp::AddOptions();
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
            done = TeXApp::ProcessOption(opt, optArg);
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
        return MiKTeX::Util::PathName("uptex.fmt");
    }

public:
    std::string GetInitProgramName() const override
    {
        return "iniuptex";
    }

public:
    std::string GetVirginProgramName() const override
    {
        return "viruptex";
    }

public:
    std::string TheNameOfTheGame() const override
    {
        return "upTeX";
    }
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
