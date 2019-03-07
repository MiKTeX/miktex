/* miktex/TeXAndFriends/InitFinalizeImpl.h:             -*- C++ -*-

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

#pragma once

#if !defined(F643DF5113314C0D91F6E13E91FEDBAD)
#define F643DF5113314C0D91F6E13E91FEDBAD

#include <miktex/TeXAndFriends/config.h>

#include "WebApp.h"

MIKTEX_TEXMF_BEGIN_NAMESPACE;

template<class PROGRAM_CLASS> class InitFinalizeImpl :
  public IInitFinalize
{
public:
  InitFinalizeImpl(PROGRAM_CLASS& program) :
    program(program)
  {
  }
private:
  PROGRAM_CLASS& program;
public:
  C4P::C4P_signed8& history() override
  {
    return program.history;
  }
};

MIKTEX_TEXMF_END_NAMESPACE;

#endif
