/* miktex/Configuration/config.h:

   Copyright (C) 2021 Christian Schenk

   This file is part of the MiKTeX Configuration Library.

   The MiKTeX Configuration Library is free software; you can
   redistribute it and/or modify it under the terms of the GNU General
   Public License as published by the Free Software Foundation; either
   version 2, or (at your option) any later version.
   
   The MiKTeX Configuration Library is distributed in the hope that it
   will be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX Configuration Library; if not, write to the
   Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

/* This file was generated from
   miktex/Configuration/config.h.cmake. */

#pragma once

#include <miktex/First>
#include <miktex/Definitions>

#if !defined(B967F942274040EE9C705A791BAED737)
#  if defined(MIKTEX_CONFIG_SHARED)
#    define MIKTEXCONFIGEXPORT MIKTEXDLLIMPORT
#  else
#    define MIKTEXCONFIGEXPORT
#  endif
#endif

#define MIKTEXCONFIGTHISAPI(type) MIKTEXCONFIGEXPORT type MIKTEXTHISCALL
#define MIKTEXCONFIGCEEAPI(type) MIKTEXCONFIGEXPORT type MIKTEXCEECALL

#if defined(__GNUC__)
#  define MIKTEXCONFIGTYPEAPI(type) MIKTEXCONFIGEXPORT type
#else
#  define MIKTEXCONFIGTYPEAPI(type) type
#endif

#define MIKTEX_CONFIG_BEGIN_NAMESPACE           \
  namespace MiKTeX {                            \
    namespace Configuration {

#define MIKTEX_CONFIG_END_NAMESPACE             \
    }                                           \
  }
