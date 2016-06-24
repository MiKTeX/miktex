/* miktex/Core/config.h: core library configuration     -*- C++ -*-

   Copyright (C) 2008-2016 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

/* This file was generated from miktex/Core/config.h.cmake. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(EAFC4257E4C00445B61E2528E8344FE6)
#define EAFC4257E4C00445B61E2528E8344FE6

#include <miktex/First>
#include <miktex/Definitions>

#define MIKTEX_SYSTEM_TAG "${MIKTEX_TARGET_SYSTEM_TAG}"

#if !defined(EAD86981C92C904D808A5E6CEC64B90E)
#  if defined(MIKTEX_CORE_SHARED)
#    define MIKTEXCOREEXPORT MIKTEXDLLIMPORT
#  else
#    define MIKTEXCOREEXPORT
#  endif
#endif

#define MIKTEXCORETHISAPI(type) MIKTEXCOREEXPORT type MIKTEXTHISCALL
#define MIKTEXCORECEEAPI(type) MIKTEXCOREEXPORT type MIKTEXCEECALL

#if defined(__GNUC__)
#  define MIKTEXCORETYPEAPI(type) MIKTEXCOREEXPORT type
#else
#  define MIKTEXCORETYPEAPI(type) type
#endif

#define MIKTEX_CORE_BEGIN_NAMESPACE             \
  namespace MiKTeX {                            \
    namespace Core {

#define MIKTEX_CORE_END_NAMESPACE               \
    }                                           \
  }

#define HAVE_MIKTEX_USER_INFO 1

#endif
