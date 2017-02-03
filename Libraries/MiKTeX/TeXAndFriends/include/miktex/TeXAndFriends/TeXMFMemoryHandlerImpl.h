/* miktex/TeXAndFriends/TeXMFMemoryHandlerImpl.h:       -*- C++ -*-

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

#if !defined(FEFFF218B53147ED8CDE64F68A13D234)
#define FEFFF218B53147ED8CDE64F68A13D234

#include <miktex/TeXAndFriends/config.h>

#include "TeXMFApp.h"

MIKTEXMF_BEGIN_NAMESPACE;

template<class PROGRAM_CLASS> class TeXMFMemoryHandlerImpl :
  public ITeXMFMemoryHandler
{
public:
  TeXMFMemoryHandlerImpl(PROGRAM_CLASS& program) :
    program(program)
  {
  }
private:
  PROGRAM_CLASS& program;
public:
  void Allocate() override
  {
  }
public:
  void Free() override
  {
  }
public:
  void Check() override
  {
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(buffer));
#  if defined(MIKTEX_TEX_COMPILER)
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(yzmem));
#  else
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(mem));
#  endif
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(paramstack));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(strpool));
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(trickbuf));
#  if !defined(MIKTEX_OMEGA)
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(THEDATA(strstart));
#  endif
  }
};

MIKTEXMF_END_NAMESPACE;

#endif
