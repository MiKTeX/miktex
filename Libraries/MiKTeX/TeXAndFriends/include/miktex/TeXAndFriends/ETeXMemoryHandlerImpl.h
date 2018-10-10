/* miktex/TeXAndFriends/ETeXMemoryHandlerImpl.h:        -*- C++ -*-

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

#if !defined(FC37AFDF33BF45C38B6179E5737B0F56)
#define FC37AFDF33BF45C38B6179E5737B0F56

#include <miktex/TeXAndFriends/config.h>

#include "TeXMemoryHandlerImpl.h"

MIKTEXMF_BEGIN_NAMESPACE;

template<class PROGRAM_CLASS> class ETeXMemoryHandlerImpl :
  public TeXMemoryHandlerImpl<PROGRAM_CLASS>
{
public:
  ETeXMemoryHandlerImpl(PROGRAM_CLASS& program, TeXMFApp& texmfapp) :
    TeXMemoryHandlerImpl<PROGRAM_CLASS>(program, texmfapp)
  {
  }

public:
  void Allocate(const std::unordered_map<std::string, int>& userParams) override
  {
    TeXMemoryHandlerImpl<PROGRAM_CLASS>::Allocate(userParams);
    this->AllocateArray("eofseen", this->program.eofseen, this->program.maxinopen);
    this->AllocateArray("grpstack", this->program.grpstack, this->program.maxinopen);
    this->AllocateArray("ifstack", this->program.ifstack, this->program.maxinopen);
  }

public:
  void Free() override
  {
    TeXMemoryHandlerImpl<PROGRAM_CLASS>::Free();
    this->FreeArray("eofseen", this->program.eofseen);
    this->FreeArray("grpstack", this->program.grpstack);
    this->FreeArray("ifstack", this->program.ifstack);
  }

public:
  void Check() override
  {
    TeXMemoryHandlerImpl<PROGRAM_CLASS>::Check();
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.eofseen);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.grpstack);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.ifstack);
  }
};

MIKTEXMF_END_NAMESPACE;

#endif
