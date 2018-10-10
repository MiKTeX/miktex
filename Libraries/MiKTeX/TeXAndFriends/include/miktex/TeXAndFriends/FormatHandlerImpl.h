/* miktex/TeXAndFriends/FormatHandlerImpl.h:            -*- C++ -*-

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

#if !defined(F88D2BEEDAF04F49B3B45BBFEEA3E17F)
#define F88D2BEEDAF04F49B3B45BBFEEA3E17F

#include <miktex/TeXAndFriends/config.h>

#include "TeXApp.h"

MIKTEXMF_BEGIN_NAMESPACE;

template<class PROGRAM_CLASS> class FormatHandlerImpl :
  public IFormatHandler
{
public:
  FormatHandlerImpl(PROGRAM_CLASS& program) :
    program(program)
  {
  }
private:
  PROGRAM_CLASS& program;
public:
  C4P::C4P_signed32& formatident() override
  {
    return program.formatident;
  }
};

MIKTEXMF_END_NAMESPACE;

#endif
