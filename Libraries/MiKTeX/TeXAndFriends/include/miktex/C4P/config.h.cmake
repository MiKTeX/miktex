/* miktex/C4P/config.h: C4P compile-time configuration  -*- C++ -*-

   Copyright (C) 2008-2017 Christian Schenk

   This file is part of the MiKTeX TeXMF Library.

   The MiKTeX TeXMF Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX TeXMF Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX TeXMF Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */


/* This file was generated from miktex/C4P/config.h.cmake. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(E5FE95F6CC5C4570A1DACB0E4FAE06B2)
#define E5FE95F6CC5C4570A1DACB0E4FAE06B2

#include <miktex/First>
#include <miktex/Definitions>

// DLL import/export switch
#if !defined(C1F0C63F01D5114A90DDF8FC10FF410B)
#  if defined(MIKTEX_TEXMF_SHARED)
#    define C4PEXPORT MIKTEXDLLIMPORT
#  else
#    define C4PEXPORT
#  endif
#endif

// API decoration for exported member functions
#define C4PTHISAPI(type) C4PEXPORT type MIKTEXTHISCALL

// API decoration for exported functions
#define C4PCEEAPI(type) C4PEXPORT type MIKTEXCEECALL

#define C4PCEECALL MIKTEXCEECALL

#define C4P_BEGIN_NAMESPACE namespace C4P {
#define C4P_END_NAMESPACE }

#cmakedefine HAVE_ROUND 1
#cmakedefine HAVE_TRUNC 1

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wdangling-else"
#pragma clang diagnostic ignored "-Wparentheses-equality"
#endif

#endif
