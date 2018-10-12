/* mf-miktex.h:                                         -*- C++ -*-

   Copyright (C) 1991-2018 Christian Schenk

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

#include "mf-miktex-config.h"

#include "mf-version.h"

#include <miktex/Core/FileType>
#include <miktex/TeXAndFriends/CharacterConverterImpl>
#include <miktex/TeXAndFriends/ErrorHandlerImpl>
#include <miktex/TeXAndFriends/InitFinalizeImpl>
#include <miktex/TeXAndFriends/InputOutputImpl>
#include <miktex/TeXAndFriends/MetafontApp>
#include <miktex/TeXAndFriends/MetafontMemoryHandlerImpl>
#include <miktex/TeXAndFriends/StringHandlerImpl>
#include <miktex/TeXAndFriends/Prototypes>

#include "mf.h"

namespace mf {
#include <miktex/mf.defaults.h>
}

#include "screen.h"

#if !defined(MIKTEXHELP_MF)
#  include <miktex/Core/Help>
#endif

extern MFPROGCLASS MFPROG;

class MemoryHandlerImpl :
  public MiKTeX::TeXAndFriends::MetafontMemoryHandlerImpl<MFPROGCLASS>
{
public:
  MemoryHandlerImpl(MFPROGCLASS& program, MiKTeX::TeXAndFriends::TeXMFApp& mfapp) :
    MetafontMemoryHandlerImpl<MFPROGCLASS>(program, mfapp)
  {
  }

public:
  void Allocate(const std::unordered_map<std::string, int>& userParams) override
  {
    MetafontMemoryHandlerImpl<MFPROGCLASS>::Allocate(userParams);
    program.maxwiggle = GetCheckedParameter("max_wiggle", program.infmaxwiggle, program.supmaxwiggle, userParams, mf::mf::max_wiggle());
    program.movesize = GetCheckedParameter("move_size", program.infmovesize, program.supmovesize, userParams, mf::mf::move_size());
    AllocateArray("after", program.after, program.maxwiggle);
    AllocateArray("before", program.before, program.maxwiggle);
    AllocateArray("envmove", program.envmove, program.movesize);
    AllocateArray("move", program.move, program.movesize);
    AllocateArray("nodetoround", program.nodetoround, program.maxwiggle);
  }

public:
  void Free() override
  {
    MetafontMemoryHandlerImpl<MFPROGCLASS>::Free();
    FreeArray("after", program.after);
    FreeArray("before", program.before);
    FreeArray("envmove", program.envmove);
    FreeArray("move", program.move);
    FreeArray("nodetoround", program.nodetoround);
  }

public:
  void Check() override
  {
    MetafontMemoryHandlerImpl<MFPROGCLASS>::Check();
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.after);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.before);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.envmove);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.move);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(program.nodetoround);
  }
};

class MFAPPCLASS

  : public MiKTeX::TeXAndFriends::MetafontApp

{
private:
  MiKTeX::TeXAndFriends::CharacterConverterImpl<MFPROGCLASS> charConv{ MFPROG };

private:
  MiKTeX::TeXAndFriends::ErrorHandlerImpl<MFPROGCLASS> errorHandler{ MFPROG };

private:
  MiKTeX::TeXAndFriends::InitFinalizeImpl<MFPROGCLASS> initFinalize{ MFPROG };

private:
  MiKTeX::TeXAndFriends::InputOutputImpl<MFPROGCLASS> inputOutput{ MFPROG };

private:
  MiKTeX::TeXAndFriends::StringHandlerImpl<MFPROGCLASS> stringHandler{ MFPROG };

private:
  MemoryHandlerImpl memoryHandler { MFPROG, *this };

public:
  void Init(std::vector<char*>& args) override
  {
    SetCharacterConverter(&charConv);
    SetErrorHandler(&errorHandler);
    SetInitFinalize(&initFinalize);
    SetInputOutput(&inputOutput);
    SetStringHandler(&stringHandler);
    SetTeXMFMemoryHandler(&memoryHandler);
    MetafontApp::Init(args);
    SetProgramInfo("mf", MIKTEX_COMPONENT_VERSION_STR, MIKTEX_COMP_COPYRIGHT_STR, MIKTEX_COMP_TM_STR);
#ifdef IMPLEMENT_TCX
    EnableFeature(MiKTeX::TeXAndFriends::Feature::TCX);
#endif
    screenEnabled = false;
  }

  enum {
    OPT_MAX_WIGGLE = 10000,
    OPT_MOVE_SIZE,
    OPT_SCREEN,
  };

public:
  void AddOptions() override
  {
    MetafontApp::AddOptions();
    AddOption("base", "undump");
    AddOption(MIKTEXTEXT("max-wiggle\0Set max_wiggle to N."), OPT_MAX_WIGGLE, POPT_ARG_STRING, "N");
    AddOption(MIKTEXTEXT("move-size\0Set move_size to N."), OPT_MOVE_SIZE, POPT_ARG_STRING, "N");
    AddOption(MIKTEXTEXT("screen\0Enable screen output."), OPT_SCREEN);
  }

public:
  bool ProcessOption(int opt, const std::string& optArg) override
  {
    bool done = true;
    switch (opt)
      {
      case OPT_MAX_WIGGLE:
        GetUserParams()["max_wiggle"] = std::stoi(optArg);
        break;
      case OPT_MOVE_SIZE:
        GetUserParams()["move_size"] = std::stoi(optArg);
        break;
      case OPT_SCREEN:
        screenEnabled = true;
        break;
      default:
        done = MetafontApp::ProcessOption(opt, optArg);
        break;
      }
    return (done);
  }

public:
  int GetFormatIdent() const override
  {
    return MFPROG.baseident;
  }

public:
  MiKTeX::Core::PathName GetMemoryDumpFileName() const override
  {
    return "mf.base";
  }

public:
  std::string GetInitProgramName() const override
  {
    return "inimf";
  }

public:
  std::string GetVirginProgramName() const override
  {
    return "virmf";
  }

public:
  std::string TheNameOfTheGame() const override
  {
    return "METAFONT";
  }

public:
  bool ScreenEnabled() const
  {
    return screenEnabled;
  }

private:
  bool screenEnabled;

public:
  unsigned long GetHelpId() const override
  {
    return MIKTEXHELP_MF;
  }
};

int miktexloadpoolstrings(int size);

inline int loadpoolstrings(int size)
{
  return miktexloadpoolstrings(size);
}

extern MFAPPCLASS MFAPP;

inline bool miktexopenbasefile(MFPROGCLASS::wordfile& f, bool renew = false)
{
  return MFAPP.OpenMemoryDumpFile(f, renew);
}

inline void miktexallocatememory()
{
  MFAPP.AllocateMemory();
}

inline void miktexfreememory()
{
  MFAPP.FreeMemory();
}

inline C4P::C4P_integer makefraction(C4P::C4P_integer p, C4P::C4P_integer q)
{
#if defined(MIKTEXMFAPI_USE_ASM)
  return MiKTeX::TeXAndFriends::MakeFraction(p, q, MFPROG.aritherror);
#else
  return MFPROG.makefractionorig(p, q);
#endif
}

inline C4P::C4P_integer makescaled(C4P::C4P_integer p, C4P::C4P_integer q)
{
#if 0 && defined(MIKTEXMFAPI_USE_ASM)
  return MiKTeX::TeXAndFriends::MakeScaled(p, q, MFPROG.aritherror);
#else
  return MFPROG.makescaledorig(p, q);
#endif
}

inline C4P::C4P_integer takefraction(C4P::C4P_integer p, C4P::C4P_integer q)
{
#if defined(MIKTEXMFAPI_USE_ASM)
  return MiKTeX::TeXAndFriends::TakeFraction(p, q, MFPROG.aritherror);
#else
  return MFPROG.takefractionorig(p, q);
#endif
}

inline C4P::C4P_integer takescaled(C4P::C4P_integer p, C4P::C4P_integer q)
{
#if defined(MIKTEXMFAPI_USE_ASM)
  return MiKTeX::TeXAndFriends::TakeScaled(p, q, MFPROG.aritherror);
#else
  return MFPROG.takescaledorig(p, q);
#endif
}
