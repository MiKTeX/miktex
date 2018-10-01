/* miktex/Core/IntegerTypes.h:                          -*- C++ -*-

   Copyright (C) 1996-2018 Christian Schenk

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

#pragma once

#if !defined(B8790D0F8ECA42169AD3373FC845E5D7)
#define B8790D0F8ECA42169AD3373FC845E5D7

#if !defined(HAVE_INTTYPES_H)
#cmakedefine HAVE_INTTYPES_H 1
#endif

#if defined(HAVE_INTTYPES_H)
#  include <inttypes.h>
#endif

#if !defined(HAVE_STDINT_H)
#cmakedefine HAVE_STDINT_H 1
#endif

#if defined(HAVE_STDINT_H)
#  include <stdint.h>
#endif

#if defined(_MSC_VER)

typedef __int8 MIKTEX_INT8;
typedef __int16 MIKTEX_INT16;
typedef __int32 MIKTEX_INT32;
typedef __int64 MIKTEX_INT64;

typedef unsigned __int8 MIKTEX_UINT8;
typedef unsigned __int16 MIKTEX_UINT16;
typedef unsigned __int32 MIKTEX_UINT32;
typedef unsigned __int64 MIKTEX_UINT64;

#else

typedef int8_t MIKTEX_INT8;
typedef int16_t MIKTEX_INT16;
typedef int32_t MIKTEX_INT32;
typedef int64_t MIKTEX_INT64;

typedef uint8_t MIKTEX_UINT8;
typedef uint16_t MIKTEX_UINT16;
typedef uint32_t MIKTEX_UINT32;
typedef uint64_t MIKTEX_UINT64;

#endif

#endif
