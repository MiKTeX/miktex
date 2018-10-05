/* miktex/Trace/config.h: trace library configuration     -*- C++ -*-

   Copyright (C) 2008-2016 Christian Schenk

   This file is part of the MiKTeX Trace Library.

   The MiKTeX Trace Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX Trace Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX Trace Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(DAF2AEA10A434D889B6B6653C535C4D4)
#define DAF2AEA10A434D889B6B6653C535C4D4

#include <miktex/First>
#include <miktex/Definitions>

#if !defined(DE9EF9059C8744B48A68345CD5A8A2C8)
#  if defined(MIKTEX_TRACE_SHARED)
#    define MIKTEXTRACEEXPORT MIKTEXDLLIMPORT
#  else
#    define MIKTEXTRACEEXPORT
#  endif
#endif

#define MIKTEXTRACETHISAPI(type) MIKTEXTRACEEXPORT type MIKTEXTHISCALL
#define MIKTEXTRACECEEAPI(type) MIKTEXTRACEEXPORT type MIKTEXCEECALL

#if defined(__GNUC__)
#  define MIKTEXTRACETYPEAPI(type) MIKTEXTRACEEXPORT type
#else
#  define MIKTEXTRACETYPEAPI(type) type
#endif

#define MIKTEX_TRACE_BEGIN_NAMESPACE            \
  namespace MiKTeX {                            \
    namespace Trace {

#define MIKTEX_TRACE_END_NAMESPACE              \
    }                                           \
  }

#endif
