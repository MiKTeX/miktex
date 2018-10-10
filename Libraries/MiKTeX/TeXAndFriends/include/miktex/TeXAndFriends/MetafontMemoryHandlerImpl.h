/* miktex/TeXAndFriends/MetafontMemoryHandlerImpl.h:    -*- C++ -*-

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

#if !defined(AA621B46CD4744989058A7EEE10B9F92)
#define AA621B46CD4744989058A7EEE10B9F92

#include <miktex/TeXAndFriends/config.h>

#include "TeXMFMemoryHandlerImpl.h"

MIKTEXMF_BEGIN_NAMESPACE;

namespace mfapp {
#include <miktex/mfapp.defaults.h>
}

template<class PROGRAM_CLASS> class MetafontMemoryHandlerImpl :
  public TeXMFMemoryHandlerImpl<PROGRAM_CLASS>
{
public:
  MetafontMemoryHandlerImpl(PROGRAM_CLASS& program, TeXMFApp& texmfapp) :
    TeXMFMemoryHandlerImpl<PROGRAM_CLASS>(program, texmfapp)
  {
  }

public:
  void Allocate(const std::unordered_map<std::string, int>& userParams) override
  {
    TeXMFMemoryHandlerImpl<PROGRAM_CLASS>::Allocate(userParams);

    this->program.bistacksize = this->GetCheckedParameter("bistack_size", this->program.infbistacksize, this->program.supbistacksize, userParams, mfapp::mfapp::bistack_size());
    this->program.ligtablesize = this->GetCheckedParameter("lig_table_size", this->program.infligtablesize, this->program.supligtablesize, userParams, mfapp::mfapp::lig_table_size());
    this->program.pathsize = this->GetCheckedParameter("path_size", this->program.infpathsize, this->program.suppathsize, userParams, mfapp::mfapp::path_size());

    this->AllocateArray("bisectstack", this->program.bisectstack, this->program.bistacksize);
    this->AllocateArray("delta", this->program.delta, this->program.pathsize);
    this->AllocateArray("deltax", this->program.deltax, this->program.pathsize);
    this->AllocateArray("deltay", this->program.deltay, this->program.pathsize);
    this->AllocateArray("ligkern", this->program.ligkern, this->program.ligtablesize);
    this->AllocateArray("psi", this->program.psi, this->program.pathsize);
    this->AllocateArray("strref", this->program.strref, this->program.maxstrings);
    this->AllocateArray("theta", this->program.theta, this->program.pathsize);
    this->AllocateArray("uu", this->program.uu, this->program.pathsize);
    this->AllocateArray("vv", this->program.vv, this->program.pathsize);
    this->AllocateArray("ww", this->program.ww, this->program.pathsize);

#if defined(TRAPMF)
    this->AllocateArray("free", this->program.c4p_free, this->program.memmax);
    this->AllocateArray("wasfree", this->program.wasfree, this->program.memmax);
#endif
  }

public:
  void Free() override
  {
    TeXMFMemoryHandlerImpl<PROGRAM_CLASS>::Free();
    this->FreeArray("bisectstack", this->program.bisectstack);
    this->FreeArray("delta", this->program.delta);
    this->FreeArray("deltax", this->program.deltax);
    this->FreeArray("deltay", this->program.deltay);
    this->FreeArray("ligkern", this->program.ligkern);
    this->FreeArray("psi", this->program.psi);
    this->FreeArray("strref", this->program.strref);
    this->FreeArray("theta", this->program.theta);
    this->FreeArray("uu", this->program.uu);
    this->FreeArray("vv", this->program.vv);
    this->FreeArray("ww", this->program.ww);
#if defined(TRAPMF)
    this->FreeArray("c4p_free", this->program.c4p_free);
    this->FreeArray("wasfree", this->program.wasfree);
#endif
  }

public:
  void Check() override
  {
    TeXMFMemoryHandlerImpl<PROGRAM_CLASS>::Check();
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.bisectstack);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.delta);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.deltax);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.deltay);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.ligkern);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.psi);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.strref);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.theta);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.uu);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.vv);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.ww);
#if defined(TRAPMF)
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.c4p_free);
    MIKTEX_ASSERT_VALID_HEAP_POINTER_OR_NIL(this->program.wasfree);
#endif
  }
};

MIKTEXMF_END_NAMESPACE;

#endif
