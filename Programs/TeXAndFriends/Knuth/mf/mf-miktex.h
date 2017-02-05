/* mf-miktex.h:                                         -*- C++ -*-

   Copyright (C) 1991-2017 Christian Schenk

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

#if defined(_MSC_VER)
#  pragma once
#endif

#include <miktex/Core/FileType>
#include <miktex/TeXAndFriends/CharacterConverterImpl>
#include <miktex/TeXAndFriends/InitFinalizeImpl>
#include <miktex/TeXAndFriends/InputOutputImpl>
#include <miktex/TeXAndFriends/MetafontApp>
#include <miktex/TeXAndFriends/MetafontMemoryHandlerImpl>

#if defined(MIKTEX_TRAPMF)
#  include "trapmfdefs.h"
#else
#  include "mfdefs.h"
#endif

#if defined(MIKTEX_TRAPMF)
#  include "trapmf.h"
#else
#  include "mf.h"
#endif

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
    MetafontMemoryHandlerImpl(program, mfapp)
  {
  }

public:
  void Allocate(const std::unordered_map<std::string, int>& userParams) override
  {
    MetafontMemoryHandlerImpl::Allocate(userParams);
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
    MetafontMemoryHandlerImpl::Free();
    FreeArray("", program.after);
    FreeArray("", program.before);
    FreeArray("", program.envmove);
    FreeArray("", program.move);
    FreeArray("", program.nodetoround);
  }
};

#if defined(MIKTEX_TRAPMF)
class TRAPMFAPPCLASS
#else
class MFAPPCLASS
#endif

  : public MiKTeX::TeXAndFriends::MetafontApp

{
private:
  MiKTeX::TeXAndFriends::CharacterConverterImpl<METAFONTProgram> charConv{ MFPROG };

private:
  MiKTeX::TeXAndFriends::InitFinalizeImpl<METAFONTProgram> initFinalize{ MFPROG };

private:
  MiKTeX::TeXAndFriends::InputOutputImpl<METAFONTProgram> inputOutput{ MFPROG };

private:
  MemoryHandlerImpl memoryHandler { MFPROG, *this };

public:
  void Init(const std::string& programInvocationName) override
  {
    SetCharacterConverter(&charConv);
    SetInitFinalize(&initFinalize);
    SetInputOutput(&inputOutput);
    SetTeXMFMemoryHandler(&memoryHandler);
    MetafontApp::Init(programInvocationName);
    SetProgramInfo("mf", "", "", "");
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

#if defined(MIKTEX_TRAPMF)
extern TRAPMFAPPCLASS TRAPMFAPP;
#define THEAPP TRAPMFAPP
#else
extern MFAPPCLASS MFAPP;
#define THEAPP MFAPP
#endif
#include <miktex/TeXAndFriends/MetafontApp.inl>

inline bool miktexopenbasefile (METAFONTProgram::wordfile& f, bool renew = false)
{
  return THEAPP.OpenMemoryDumpFile(f, renew);
}

int miktexloadpoolstrings(int size);

inline int loadpoolstrings(int size)
{
  return miktexloadpoolstrings(size);
}

