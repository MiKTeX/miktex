/* miktex/W2C/pre.h:                                    -*- C++ -*-

   Copyright (C) 2013-2018 Christian Schenk

   This file is part of the MiKTeX W2CEMU Library.

   The MiKTeX W2CEMU Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX W2CEMU Library is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX W2CEMU Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#if !defined(CD7FC931C9674841BD05A1D2BDF2C)
#define CD7FC931C9674841BD05A1D2BDF2C

#if defined(__cplusplus)
#include <miktex/Core/Session>
#else
#include <miktex/Core/c/api.h>
#endif
#include <miktex/Core/Debug>
#include <miktex/Core/IntegerTypes>
#include <miktex/Definitions>
#include <miktex/KPSE/Emulation>
#include <miktex/Version>

// DLL import/export switch
#if !defined(B96BCD894353492A9CF685C84B9AB5E0)
#  define MIKTEXW2CEXPORT MIKTEXDLLIMPORT
#endif

// API decoration for exported functions and data
#define MIKTEXW2CCEEAPI(type) MIKTEXW2CEXPORT type MIKTEXCEECALL
#define MIKTEXW2CDATA(type) MIKTEXW2CEXPORT type

#define MIKTEXWEB2C_BEGIN_NAMESPACE             \
  namespace MiKTeX {                            \
    namespace Web2C {

#define MIKTEXWEB2C_END_NAMESPACE               \
    }                                           \
  }

#endif
