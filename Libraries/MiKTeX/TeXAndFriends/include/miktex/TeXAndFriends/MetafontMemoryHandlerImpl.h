/* miktex/TeXAndFriends/MetafontMemoryHandlerImpl.h:    -*- C++ -*-

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

#if !defined(AA621B46CD4744989058A7EEE10B9F92)
#define AA621B46CD4744989058A7EEE10B9F92

#include <miktex/TeXAndFriends/config.h>

#include "TeXMFMemoryHandlerImpl.h"

MIKTEXMF_BEGIN_NAMESPACE;

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

    program.bistacksize = GetCheckedParameter("bistack_size", program.infbistacksize, program.supbistacksize, userParams, mfapp::mfapp::bistack_size());
    program.ligtablesize = GetCheckedParameter("lig_table_size", program.infligtablesize, program.supligtablesize, userParams, mfapp::mfapp::lig_table_size());
    program.pathsize = GetCheckedParameter("path_size", program.infpathsize, program.suppathsize, userParams, mfapp::mfapp::path_size());

    AllocateArray("bisectstack", program.bisectstack, program.bistacksize);
    AllocateArray("delta", program.delta, program.pathsize);
    AllocateArray("deltax", program.deltax, program.pathsize);
    AllocateArray("deltay", program.deltay, program.pathsize);
    AllocateArray("ligkern", program.ligkern, program.ligtablesize);
    AllocateArray("psi", program.psi, program.pathsize);
    if (texmfapp.IsInitProgram() || texmfapp.AmI("mf"))
    {
      AllocateArray("strref", program.strref, program.maxstrings);
    }
    AllocateArray("theta", program.theta, program.pathsize);
    AllocateArray("uu", program.uu, program.pathsize);
    AllocateArray("vv", program.vv, program.pathsize);
    AllocateArray("ww", program.ww, program.pathsize);

#if defined(TRAPMF)
    AllocateArray("free", program.c4p_free, program.memmax);
    AllocateArray("wasfree", program.wasfree, program.memmax);
#endif
  }

public:
  void Free() override
  {
    TeXMFMemoryHandlerImpl<PROGRAM_CLASS>::Free();
    FreeArray("", program.bisectstack);
    FreeArray("", program.delta);
    FreeArray("", program.deltax);
    FreeArray("", program.deltay);
    FreeArray("", program.ligkern);
    FreeArray("", program.psi);
    FreeArray("", program.strref);
    FreeArray("", program.theta);
    FreeArray("", program.uu);
    FreeArray("", program.vv);
    FreeArray("", program.ww);
#if defined(TRAPMF)
    FreeArray("", program.c4p_free);
    FreeArray("", program.wasfree);
#endif
  }
};

MIKTEXMF_END_NAMESPACE;

#endif
