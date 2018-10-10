/* miktex/TeXAndFriends/config.h:                       -*- C++ -*-

   Copyright (C) 2009-2018 Christian Schenk

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

/* This file was generated from
   miktex/TeXAndFriends/config.h.cmake. */

#pragma once

#if !defined(D6EDC25DF28A4C09A02D177CFBE34499)
#define D6EDC25DF28A4C09A02D177CFBE34499

#include <miktex/First>
#include <miktex/Definitions>

// DLL import/export switch
#if !defined(B8C7815676699B4EA2DE96F0BD727276)
#  if defined(MIKTEX_TEXMF_SHARED)
#    define MIKTEXMFEXPORT MIKTEXDLLIMPORT
#  else
#    define MIKTEXMFEXPORT
#  endif
#endif

#define MIKTEXMFAPI_(type, cc) MIKTEXMFEXPORT type cc

// API decoration for exported member functions
#define MIKTEXMFTHISAPI(type) MIKTEXMFEXPORT type MIKTEXTHISCALL

// API decoration for exported functions
#define MIKTEXMFCEEAPI(type) MIKTEXMFEXPORT type MIKTEXCEECALL

// API decoration for exported types
#if defined(__GNUC__)
#  define MIKTEXMFTYPEAPI(type) MIKTEXMFEXPORT type
#else
#  define MIKTEXMFTYPEAPI(type) type
#endif

// API decoration for exported data
#define MIKTEXMFDATA(type) MIKTEXMFEXPORT type

#define MIKTEXMF_BEGIN_NAMESPACE                \
  namespace MiKTeX {                            \
    namespace TeXAndFriends {

#define MIKTEXMF_END_NAMESPACE                  \
    }                                           \
  }

#cmakedefine WITH_PDFTEX 1
#cmakedefine WITH_SYNCTEX 1
#cmakedefine WITH_OMEGA 1

#if defined(MIKTEX_TEX) || defined(MIKTEX_TRIPTEX)
#  define MIKTEX_TEX_COMPILER 1
#endif

#if defined(MIKTEX_PDFTEX)
#  define MIKTEX_TEX_COMPILER 1
#endif

#if defined(MIKTEX_XETEX)
#  define MIKTEX_TEX_COMPILER 1
#  define MIKTEX_TEXMF_UNICODE 1
#endif

#if defined(MIKTEX_OMEGA)
#  define MIKTEX_TEX_COMPILER 1
#  define MIKTEX_TEXMF_UNICODE 1
#endif

#if defined(MIKTEX_METAFONT)
#  define MIKTEX_META_COMPILER 1
#  define ENABLE_8BIT_CHARS 1
#  define HAVE_MAIN_MEMORY 1
#  define IMPLEMENT_TCX 1
#endif

#if defined(MIKTEX_TEX_COMPILER)
#  define HAVE_EXTRA_MEM_BOT 1
#  define HAVE_EXTRA_MEM_TOP 1
#  define HAVE_MAIN_MEMORY 1
#  define HAVE_POOL_FREE 1
#  define HAVE_STRINGS_FREE 1
#  if !(defined(MIKTEX_XETEX) || defined(MIKTEX_OMEGA))
#    define IMPLEMENT_TCX 1
#  endif
#  if !defined(MIKTEX_OMEGA)
#    define ENABLE_8BIT_CHARS 1
#  endif
#endif

#if defined(MIKTEX_BIBTEX)
#  define IMPLEMENT_TCX 1
#endif

#endif
