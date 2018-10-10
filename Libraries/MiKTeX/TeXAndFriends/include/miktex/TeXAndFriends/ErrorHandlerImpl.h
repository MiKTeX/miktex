/* miktex/TeXAndFriends/ErrorHandlerImpl.h:             -*- C++ -*-

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

#if !defined(B2574392AC1042F29EC75F1CCA848F79)
#define B2574392AC1042F29EC75F1CCA848F79

#include <miktex/TeXAndFriends/config.h>

#include "TeXMFApp.h"

MIKTEXMF_BEGIN_NAMESPACE;

template<class PROGRAM_CLASS> class ErrorHandlerImpl :
  public IErrorHandler
{
public:
  ErrorHandlerImpl(PROGRAM_CLASS& program) :
    program(program)
  {
  }
private:
  PROGRAM_CLASS& program;
public:
  C4P::C4P_integer& interrupt() override
  {
    return program.interrupt;
  }
};

MIKTEXMF_END_NAMESPACE;

#endif
