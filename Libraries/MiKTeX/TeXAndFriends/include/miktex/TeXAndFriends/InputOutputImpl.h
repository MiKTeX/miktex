/* miktex/TeXAndFriends/InputOutputImpl.h:              -*- C++ -*-

   Copyright (C) 2017-2020 Christian Schenk

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

#if !defined(CBBC6CEB392E4FBAB56BA538A0EBBCDD)
#define CBBC6CEB392E4FBAB56BA538A0EBBCDD

#include <miktex/TeXAndFriends/config.h>

#include "WebAppInputLine.h"

MIKTEX_TEXMF_BEGIN_NAMESPACE;

template<class PROGRAM_CLASS> class InputOutputImpl :
  public IInputOutput
{
public:
  InputOutputImpl(PROGRAM_CLASS& program) :
    program(program)
  {
  }

private:
  PROGRAM_CLASS& program;

public:
  C4P::C4P_signed32& loc() override
  {
#if defined(MIKTEX_META_COMPILER) || defined(MIKTEX_TEX_COMPILER)
    return program.curinput.locfield;
#else
    MIKTEX_UNEXPECTED();
#endif
  }

public:
  C4P::C4P_signed32& limit() override
  {
#if defined(MIKTEX_META_COMPILER) || defined(MIKTEX_TEX_COMPILER)
    return program.curinput.limitfield;
#else
    MIKTEX_UNEXPECTED();
#endif
  }

public:
  C4P::C4P_signed32 first() override
  {
#if defined(MIKTEX_META_COMPILER) || defined(MIKTEX_TEX_COMPILER)
    return program.first;
#else
    return 0;
#endif
  }

public:
  C4P::C4P_signed32& last() override
  {
    return program.last;
  }

public:
  C4P::C4P_signed32 bufsize() override
  {
    return program.bufsize;
  }

public:
  char*& nameoffile() override
  {
    MIKTEX_ASSERT(sizeof(program.nameoffile[0]) == sizeof(char));
    return reinterpret_cast<char*&>(program.nameoffile);
  }

public:
  C4P::C4P_signed32& namelength() override
  {
    return program.namelength;
  }

public:
  char* buffer() override
  {
#if defined(MIKTEX_TEXMF_UNICODE)
    MIKTEX_UNEXPECTED();
#else
    MIKTEX_ASSERT(sizeof(program.buffer[0]) == sizeof(char));
    return (char*)program.buffer;
#endif
  }

public:
  char32_t* buffer32() override
  {
#if defined(MIKTEX_XETEX)
    MIKTEX_ASSERT(sizeof(program.buffer[0]) == sizeof(char32_t));
    return (char32_t*)program.buffer;
#else
    MIKTEX_UNEXPECTED();
#endif
  }

public:
  C4P::C4P_signed32& maxbufstack() override
  {
#if defined(MIKTEX_META_COMPILER) || defined(MIKTEX_TEX_COMPILER)
    return program.maxbufstack;
#else
    MIKTEX_UNEXPECTED();
#endif
  }

public:
  void overflow(C4P::C4P_signed32 s, C4P::C4P_integer n) override
  {
#if defined(MIKTEX_META_COMPILER) || defined(MIKTEX_TEX_COMPILER)
    program.overflow(s, n);
#else
    MIKTEX_UNEXPECTED();
#endif
  }
};

MIKTEX_TEXMF_END_NAMESPACE;

#endif
