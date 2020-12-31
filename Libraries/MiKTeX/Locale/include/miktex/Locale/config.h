/* miktex/Locale/config.h:

   Copyright (C) 2020 Christian Schenk

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

#include <miktex/First>
#include <miktex/Definitions>

// DLL import/export switch
#if !defined(E85DABD27D3C467CBD620EA8FFF876CF)
#  if defined(MIKTEX_LOC_SHARED)
#    define MIKTEXLOCEXPORT MIKTEXDLLIMPORT
#  else
#    define MIKTEXLOCEXPORT
#  endif
#endif

// API decoration for exported member functions
#define MIKTEXLOCTHISAPI(type) MIKTEXLOCEXPORT type MIKTEXTHISCALL
#define MIKTEXLOCCEEAPI(type) MIKTEXLOCEXPORT type MIKTEXCEECALL

#if defined(__GNUC__)
#  define MIKTEXLOCTYPEAPI(type) MIKTEXLOCEXPORT type
#else
#  define MIKTEXLOCTYPEAPI(type) type
#endif

#define MIKTEX_LOC_BEGIN_NAMESPACE              \
  namespace MiKTeX {                            \
    namespace Locale {

#define MIKTEX_LOC_END_NAMESPACE                \
    }                                           \
  }
