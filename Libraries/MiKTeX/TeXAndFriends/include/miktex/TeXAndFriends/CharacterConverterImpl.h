/* miktex/TeXAndFriends/CharacterConverterImpl.h:       -*- C++ -*-

   Copyright (C) 2017 Christian Schenk

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

#if !defined(C15F91C9207D456688F9B7CE2537DB4E)
#define C15F91C9207D456688F9B7CE2537DB4E

#include <miktex/TeXAndFriends/config.h>

#include "WebApp.h"

MIKTEXMF_BEGIN_NAMESPACE;

template<class PROGRAM_CLASS> class CharacterConverterImpl :
  public ICharacterConverter
{
public:
  CharacterConverterImpl(PROGRAM_CLASS& program) :
    program(program)
  {
  }
private:
  PROGRAM_CLASS& program;
public:
  char* xchr() override
  {
#if defined(MIKTEX_TEXMF_UNICODE)
    MIKTEX_UNEXPECTED();
#else
    MIKTEX_ASSERT(sizeof(program.xchr[0]) == sizeof(char));
    return (char*)&program.xchr[0];
#endif
  }
public:
  char* xord() override
  {
#if defined(MIKTEX_TEXMF_UNICODE)
    MIKTEX_UNEXPECTED();
#else
    MIKTEX_ASSERT(sizeof(program.xord[0]) == sizeof(char));
    return (char*)&program.xord[0];
#endif
  }
public:
  char* xprn() override
  {
#if (defined(MIKTEX_META_COMPILER) || defined(MIKTEX_TEX_COMPILER)) && !defined(MIKTEX_TEXMF_UNICODE)
    MIKTEX_ASSERT(sizeof(program.xprn[0]) == sizeof(char));
    return (char*)&program.xprn[0];
#else
    MIKTEX_UNEXPECTED();
#endif
  }
};

MIKTEXMF_END_NAMESPACE;

#endif
