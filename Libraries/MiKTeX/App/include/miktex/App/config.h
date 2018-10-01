/* miktex/App/config.h: app library configuration       -*- C++ -*-

   Copyright (C) 2008-2018 Christian Schenk

   This file is part of the MiKTeX App Library.

   The MiKTeX App Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX App Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX App Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#if !defined(E29CD633E4AF46E0B5DA41D1A2FCC75F)
#define E29CD633E4AF46E0B5DA41D1A2FCC75F

#include <miktex/First>
#include <miktex/Definitions>

// DLL import/export switch
#if !defined(BDF6E2537F116547846406B5B2B65949)
#  if defined(MIKTEX_APP_SHARED)
#    define MIKTEXAPPEXPORT MIKTEXDLLIMPORT
#  else
#    define MIKTEXAPPEXPORT
#  endif
#endif

// API decoration for exported member functions
#define MIKTEXAPPTHISAPI(type) MIKTEXAPPEXPORT type MIKTEXTHISCALL
#define MIKTEXAPPCEEAPI(type) MIKTEXAPPEXPORT type MIKTEXCEECALL

#if defined(__GNUC__)
#  define MIKTEXAPPTYPEAPI(type) MIKTEXAPPEXPORT type
#else
#  define MIKTEXAPPTYPEAPI(type) type
#endif

#define MIKTEX_APP_BEGIN_NAMESPACE              \
  namespace MiKTeX {                            \
    namespace App {

#define MIKTEX_APP_END_NAMESPACE                \
    }                                           \
  }

#endif
