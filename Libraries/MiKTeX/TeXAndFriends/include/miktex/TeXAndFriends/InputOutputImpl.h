/* miktex/TeXAndFriends/InputOutputImpl.h:              -*- C++ -*-

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

#if !defined(CBBC6CEB392E4FBAB56BA538A0EBBCDD)
#define CBBC6CEB392E4FBAB56BA538A0EBBCDD

#include <miktex/TeXAndFriends/config.h>

#include "WebAppInputLine.h"

MIKTEXMF_BEGIN_NAMESPACE;

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
#if defined(MIKTEX_TEX_COMPILER) || defined(MIKTEX_TEX_COMPILER)
    return program.curinput.locfield;
#else
    MIKTEX_UNEXPECTED();
#endif
  }
public:
  C4P::C4P_signed32& limit() override
  {
#if defined(MIKTEX_TEX_COMPILER) || defined(MIKTEX_TEX_COMPILER)
    return program.curinput.limitfield;
#else
    MIKTEX_UNEXPECTED();
#endif
  }
public:
  C4P::C4P_signed32 first() override
  {
#if defined(MIKTEX_TEX_COMPILER) || defined(MIKTEX_TEX_COMPILER)
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
  char* nameoffile() override
  {
    return (char*)program.nameoffile;
  }
public:
  C4P::C4P_signed16& namelength() override
  {
    return program.namelength;
  }
public:
  char* buffer() override
  {
    MIKTEX_ASSERT(sizeof(program.buffer[0]) == sizeof(char));
    return (char*)program.buffer;
  }
public:
  C4P::C4P_signed16* buffer16() override
  {
    MIKTEX_ASSERT(sizeof(program.buffer[0]) == sizeof(C4P::C4P_signed16));
    return (C4P::C4P_signed16*)program.buffer;
  }
public:
  C4P::C4P_signed32* buffer32() override
  {
    MIKTEX_ASSERT(sizeof(program.buffer[0]) == sizeof(C4P::C4P_signed32));
    return (C4P::C4P_signed32*)program.buffer;
  }
public:
  C4P::C4P_signed32& maxbufstack() override
  {
#if defined(MIKTEX_TEX_COMPILER) || defined(MIKTEX_TEX_COMPILER)
    return program.maxbufstack;
#else
    MIKTEX_UNEXPECTED();
#endif
  }
public:
  void overflow(C4P::C4P_signed32 s, C4P::C4P_integer n) override
  {
#if defined(MIKTEX_TEX_COMPILER) || defined(MIKTEX_TEX_COMPILER)
    program.overflow(s, n);
#else
    MIKTEX_UNEXPECTED();
#endif
  }
};

MIKTEXMF_END_NAMESPACE;

#endif
