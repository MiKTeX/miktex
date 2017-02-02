/* tangle-miktex.h:                                     -*- C++ -*-

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

#include <miktex/TeXAndFriends/config.h>

#include "tangle-version.h"
#include <miktex/TeXAndFriends/WebApp>

using namespace MiKTeX::Core;
using namespace MiKTeX::TeXAndFriends;

#define OPT_OMEGA 1000

extern TANGLEPROGCLASS TANGLEPROG;

class TANGLEAPPCLASS :
  public MiKTeX::TeXAndFriends::WebApp
{
private:
  MiKTeX::TeXAndFriends::CharacterConverterImpl<TANGLEPROGCLASS> charConv{ TANGLEPROG };

private:
  MiKTeX::TeXAndFriends::InitFinalizeImpl<TANGLEPROGCLASS> initFinalize{ TANGLEPROG };

public:
  void Init(const std::string& programInvocationName) override
  {
    SetCharacterConverter(&charConv);
    SetInitFinalize(&initFinalize);
    MiKTeX::TeXAndFriends::WebApp::Init(programInvocationName);
    TANGLEPROG.maxchar = 255;
  }

public:
  void AddOptions() override
  {
    AddOption(MIKTEXTEXT("omega\0Create Omega compatible output file."), OPT_OMEGA);
    WebApp::AddOptions();
  }

public:
  std::string GetUsage() const override
  {
    return MIKTEXTEXT("[OPTION...] INPUTFILE CHANGEFILE OUTPUTFILE POOLFILE");
  }

public:
  bool ProcessOption(int opt, const std::string& optArg) override
  {
    bool done = true;
    switch (opt)
    {
    case OPT_OMEGA:
      TANGLEPROG.maxchar = 0xffff;
      break;
    default:
      done = WebApp::ProcessOption(opt, optArg);
      break;
    }
    return done;
  }

public:
  void ProcessCommandLineOptions() override
  {
    WebApp::ProcessCommandLineOptions();
    if (C4P::GetArgC() != 5)
    {
      BadUsage();
    }
  }
};

extern TANGLEAPPCLASS TANGLEAPP;

#define THEAPP TANGLEAPP
#include <miktex/TeXAndFriends/WebApp.inl>
