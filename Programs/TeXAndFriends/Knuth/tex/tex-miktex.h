/* tex-miktex.h:                                        -*- C++ -*-

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

#include <miktex/TeXAndFriends/CharacterConverterImpl>
#include <miktex/TeXAndFriends/InitFinalizeImpl>
#include <miktex/TeXAndFriends/InputOutputImpl>
#include <miktex/TeXAndFriends/TeXApp>
#include <miktex/TeXAndFriends/TeXMemoryHandlerImpl>
#include <miktex/W2C/Emulation>

#if defined(MIKTEX_TRIPTEX)
#  include "triptexdefs.h"
#else
#  include "texdefs.h"
#endif

#if defined(MIKTEX_TRIPTEX)
#  include "triptex.h"
#else
#  include "tex.h"
#endif

#if defined(MIKTEX_WINDOWS)
#  include "tex.rc"
#endif

#if !defined(MIKTEXHELP_TEX)
#  include <miktex/Core/Help>
#endif

extern TEXPROGCLASS TEXPROG;

#if defined(MIKTEX_TRIPTEX)
class TRIPTEXAPPCLASS
#else
class TEXAPPCLASS
#endif

  : public MiKTeX::TeXAndFriends::TeXApp

{
private:
  MiKTeX::TeXAndFriends::CharacterConverterImpl<TeXProgram> charConv{ TEXPROG };

private:
  MiKTeX::TeXAndFriends::InitFinalizeImpl<TeXProgram> initFinalize{ TEXPROG };

private:
  MiKTeX::TeXAndFriends::InputOutputImpl<TeXProgram> inputOutput{ TEXPROG };

private:
  MiKTeX::TeXAndFriends::TeXMemoryHandlerImpl<TeXProgram> memoryHandler { TEXPROG, *this };

public:
  void Init(const std::string& programInvocationName) override
  {
    SetCharacterConverter(&charConv);
    SetInitFinalize(&initFinalize);
    SetInputOutput(&inputOutput);
    SetTeXMFMemoryHandler(&memoryHandler);
    TeXApp::Init(programInvocationName);
#if defined(IMPLEMENT_TCX)
    EnableFeature(MiKTeX::TeXAndFriends::Feature::EightBitChars);
    EnableFeature(MiKTeX::TeXAndFriends::Feature::TCX);
#endif
  }

public:
  MiKTeX::Core::PathName GetMemoryDumpFileName() const override
  {
    return "tex.fmt";
  }

public:
  std::string GetInitProgramName() const override
  {
    return "initex";
  }

public:
  std::string GetVirginProgramName() const override
  {
    return "virtex";
  }

public:
  std::string TheNameOfTheGame() const override
  {
    return "TeX";
  }

public:
  unsigned long GetHelpId() const override
  {
    return MIKTEXHELP_TEX;
  }
};

#if defined(MIKTEX_TRIPTEX)
extern TRIPTEXAPPCLASS TRIPTEXAPP;
#define THEAPP TRIPTEXAPP
#else
extern TEXAPPCLASS TEXAPP;
#define THEAPP TEXAPP
#endif
#include <miktex/TeXAndFriends/TeXApp.inl>

int miktexloadpoolstrings(int size);

inline int loadpoolstrings(int size)
{
  return miktexloadpoolstrings(size);
}
