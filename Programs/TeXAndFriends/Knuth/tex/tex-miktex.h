/* tex-miktex.h:                                        -*- C++ -*-

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

#include "tex-miktex-config.h"

#include "tex-version.h"

#include <miktex/TeXAndFriends/CharacterConverterImpl>
#include <miktex/TeXAndFriends/ErrorHandlerImpl>
#include <miktex/TeXAndFriends/FormatHandlerImpl>
#include <miktex/TeXAndFriends/InitFinalizeImpl>
#include <miktex/TeXAndFriends/InputOutputImpl>
#include <miktex/TeXAndFriends/StringHandlerImpl>
#include <miktex/TeXAndFriends/TeXApp>
#include <miktex/TeXAndFriends/TeXMemoryHandlerImpl>
#include <miktex/W2C/Emulation>

#include "tex.h"

#if defined(MIKTEX_WINDOWS)
#  include "tex.rc"
#endif

#if !defined(MIKTEXHELP_TEX)
#  include <miktex/Core/Help>
#endif

extern TEXPROGCLASS TEXPROG;

class TEXAPPCLASS

  : public MiKTeX::TeXAndFriends::TeXApp

{
private:
  MiKTeX::TeXAndFriends::CharacterConverterImpl<TEXPROGCLASS> charConv{ TEXPROG };

private:
  MiKTeX::TeXAndFriends::ErrorHandlerImpl<TEXPROGCLASS> errorHandler{ TEXPROG };

private:
  MiKTeX::TeXAndFriends::FormatHandlerImpl<TEXPROGCLASS> formatHandler{ TEXPROG };

private:
  MiKTeX::TeXAndFriends::InitFinalizeImpl<TEXPROGCLASS> initFinalize{ TEXPROG };

private:
  MiKTeX::TeXAndFriends::InputOutputImpl<TEXPROGCLASS> inputOutput{ TEXPROG };

private:
  MiKTeX::TeXAndFriends::StringHandlerImpl<TEXPROGCLASS> stringHandler{ TEXPROG };

private:
  MiKTeX::TeXAndFriends::TeXMemoryHandlerImpl<TEXPROGCLASS> memoryHandler { TEXPROG, *this };

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

int miktexloadpoolstrings(int size);

inline int loadpoolstrings(int size)
{
  return miktexloadpoolstrings(size);
}
