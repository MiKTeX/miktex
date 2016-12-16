/* mf-miktex.h:                                         -*- C++ -*-

   Copyright (C) 1991-2016 Christian Schenk

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

#include <miktex/TeXAndFriends/config.h>

#if defined(MIKTEX_TRAPMF)
#  include "trapmfdefs.h"
#else
#  include "mfdefs.h"
#endif

#if !defined(C4PEXTERN)
#  define C4PEXTERN extern
#endif

#if defined(MIKTEX_TRAPMF)
#  include "trapmf.h"
#else
#  include "mf.h"
#endif

#if !defined(THEDATA)
#  define THEDATA(x) C4P_VAR(x)
#endif

#include <miktex/TeXAndFriends/MetafontApp>

namespace mf {
#include <miktex/mf.defaults.h>
}

#include "screen.h"

#if !defined(MIKTEXHELP_MF)
#  include <miktex/Core/Help>
#endif

using namespace MiKTeX::Core;
using namespace MiKTeX::TeXAndFriends;
using namespace std;

#if defined(MIKTEX_TRAPMF)
class TRAPMFCLASS
#else
class MFCLASS
#endif

  : public MetafontApp

{
public:
#if defined(MIKTEX_TRAPMF)
  TRAPMFCLASS()
#else
  MFCLASS()
#endif
  {
  }

public:
  void Init(const std::string & programInvocationName) override
  {
    MetafontApp::Init(programInvocationName);
    SetProgramInfo ("mf", 0, 0, 0);
#ifdef IMPLEMENT_TCX
    EnableFeature(Feature::TCX);
#endif
    m_bScreen = false;
    m_max_wiggle = -1;
    m_move_size = -1;
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
    AddOption ("base", "undump");
    AddOption (MIKTEXTEXT("max-wiggle\0Set max_wiggle to N."), OPT_MAX_WIGGLE, POPT_ARG_STRING, "N");
    AddOption (MIKTEXTEXT("move-size\0Set move_size to N."), OPT_MOVE_SIZE, POPT_ARG_STRING, "N");
    AddOption (MIKTEXTEXT("screen\0Enable screen output."), OPT_SCREEN);
  }

public:
  bool ProcessOption(int opt, const std::string & optArg) override
  {
    bool done = true;
    switch (opt)
      {
      case OPT_MAX_WIGGLE:
        m_max_wiggle = std::stoi(optArg);
        break;
      case OPT_MOVE_SIZE:
        m_move_size = std::stoi(optArg);
        break;
      case OPT_SCREEN:
        m_bScreen = true;
        break;
      default:
        done = MetafontApp::ProcessOption(opt, optArg);
        break;
      }
    return (done);
  }

public:
  void FreeMemory()
  {
    MetafontApp::FreeMemory();
    Free(THEDATA(after));
    Free(THEDATA(before));
    Free(THEDATA(envmove));
    Free(THEDATA(move));
    Free(THEDATA(nodetoround));
  }

public:
  void AllocateMemory()
  {
    MetafontApp::AllocateMemory();

    GETPARAMCHECK(m_max_wiggle, maxwiggle, max_wiggle, mf::mf::max_wiggle()); GETPARAMCHECK(m_move_size, movesize, move_size, mf::mf::move_size());
    Allocate("after", THEDATA(after), THEDATA(maxwiggle));
    Allocate("before", THEDATA(before), THEDATA(maxwiggle));
    Allocate("envmove", THEDATA(envmove), THEDATA(movesize));
    Allocate("move", THEDATA(move), THEDATA(movesize));
    Allocate("nodetoround", THEDATA(nodetoround), THEDATA(maxwiggle));
  }

public:
  int GetFormatIdent() const override
  {
    return THEDATA(baseident);
  }

public:
  const char * GetMemoryDumpFileName() const override
  {
    return "mf.base";
  }

public:
  const char * GetInitProgramName() const override
  {
    return "inimf";
  }

public:
  const char * GetVirginProgramName() const override
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
    return m_bScreen;
  }

private:
  bool m_bScreen;

private:
  int m_max_wiggle;

private:
  int m_move_size;

public:
  unsigned long GetHelpId() const override
  {
    return MIKTEXHELP_MF;
  }
};

#if defined(MIKTEX_TRAPMF)
extern TRAPMFCLASS TRAPMFAPP;
#define THEAPP TRAPMFAPP
#else
extern MFCLASS MFAPP;
#define THEAPP MFAPP
#endif

inline bool miktexopenbasefile (wordfile & f, bool renew = false)
{
  return THEAPP.OpenMemoryDumpFile(f, renew);
}

#include <miktex/TeXAndFriends/MetafontApp.inl>

int miktexloadpoolstrings(int size);

inline int loadpoolstrings(int size)
{
  return miktexloadpoolstrings(size);
}
