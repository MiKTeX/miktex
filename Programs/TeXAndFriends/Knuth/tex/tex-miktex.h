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

#include <miktex/TeXAndFriends/config.h>

#if defined(MIKTEX_TRIPTEX)
#  include "triptexdefs.h"
#else
#  include "texdefs.h"
#endif

#if ! defined(C4PEXTERN)
#  define C4PEXTERN extern
#endif

#if defined(MIKTEX_TRIPTEX)
#  include "triptex.h"
#else
#  include "tex.h"
#endif

#if !defined(THEDATA)
#  define THEDATA(x) C4P_VAR(x)
#endif

#if defined(MIKTEX_WINDOWS)
#  include "tex.rc"
#endif

#include <miktex/TeXAndFriends/TeXApp>

#if !defined(MIKTEXHELP_TEX)
#  include <miktex/Core/Help>
#endif

#include <miktex/W2C/Emulation>

using namespace MiKTeX::TeXAndFriends;

#if defined(MIKTEX_TRIPTEX)
class TRIPTEXCLASS
#else
class TEXCLASS
#endif

  : public TeXApp

{
public:
  void Init(const std::string & programInvocationName) override
  {
    TeXApp::Init(programInvocationName);
#if defined(IMPLEMENT_TCX)
    EnableFeature(Feature::EightBitChars);
    EnableFeature(Feature::TCX);
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
extern TRIPTEXCLASS TRIPTEXAPP;
#define THEAPP TRIPTEXAPP
#else
extern TEXCLASS TEXAPP;
#define THEAPP TEXAPP
#endif

#include <miktex/TeXAndFriends/TeXApp.inl>

int miktexloadpoolstrings(int size);

inline int loadpoolstrings(int size)
{
  return miktexloadpoolstrings(size);
}
