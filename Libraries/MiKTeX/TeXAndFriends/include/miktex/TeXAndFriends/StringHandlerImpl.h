/* miktex/TeXAndFriends/StringHandlerImpl.h:            -*- C++ -*-

   Copyright (C) 2017-2018 Christian Schenk

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

#if !defined(D29D04223B0E46AC810F4ADDBE5667D1)
#define D29D04223B0E46AC810F4ADDBE5667D1

#include <miktex/TeXAndFriends/config.h>

#include "TeXMFApp.h"

MIKTEX_TEXMF_BEGIN_NAMESPACE;

template<class PROGRAM_CLASS> class StringHandlerImpl :
  public IStringHandler
{
public:
  StringHandlerImpl(PROGRAM_CLASS& program) :
    program(program)
  {
  }
private:
  PROGRAM_CLASS& program;
public:
  char* strpool() override
  {
#if defined(MIKTEX_TEXMF_UNICODE)
    MIKTEX_UNEXPECTED();
#else
    MIKTEX_ASSERT(sizeof(program.strpool[0]) == sizeof(char));
    return (char*)program.strpool;
#endif
  }
public:
  char16_t* strpool16() override
  {
#if defined(MIKTEX_TEXMF_UNICODE)
    MIKTEX_ASSERT(sizeof(program.strpool[0]) == sizeof(char16_t));
    return (char16_t*)program.strpool;
#else
    MIKTEX_UNEXPECTED();
#endif
  }
public:
  C4P::C4P_signed32& strptr() override
  {
    return program.strptr;
  }
public:
  C4P::C4P_signed32* strstart() override
  {
#if defined(MIKTEX_OMEGA)
    MIKTEX_UNEXPECTED();
#else
    return program.strstart;
#endif
  }
public:
  C4P::C4P_signed32& poolsize() override
  {
    return program.poolsize;
  }
public:
  C4P::C4P_signed32& poolptr() override
  {
    return program.poolptr;
  }
public:
  C4P::C4P_signed32 makestring() override
  {
    return program.makestring();
  }
};

MIKTEX_TEXMF_END_NAMESPACE;

#endif
