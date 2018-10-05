/* miktex/Util/config.h: util library configuration     -*- C++ -*-

   Copyright (C) 2008-2016 Christian Schenk

   This file is part of the MiKTeX Util Library.

   The MiKTeX Util Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX Util Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX Util Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(BBC120E074C14CF9A70F201CEA6ACFC2)
#define BBC120E074C14CF9A70F201CEA6ACFC2

#include <miktex/First>
#include <miktex/Definitions>

#if !defined(A7C88F5FBE5C45EB970B3796F331CD89)
#  if defined(MIKTEX_UTIL_SHARED)
#    define MIKTEXUTILEXPORT MIKTEXDLLIMPORT
#  else
#    define MIKTEXUTILEXPORT
#  endif
#endif

#define MIKTEXUTILTHISAPI(type) MIKTEXUTILEXPORT type MIKTEXTHISCALL
#define MIKTEXUTILCEEAPI(type) MIKTEXUTILEXPORT type MIKTEXCEECALL

#if defined(__GNUC__)
#  define MIKTEXUTILTYPEAPI(type) MIKTEXUTILEXPORT type
#else
#  define MIKTEXUTILTYPEAPI(type) type
#endif

#define MIKTEX_UTIL_BEGIN_NAMESPACE             \
  namespace MiKTeX {                            \
    namespace Util {

#define MIKTEX_UTIL_END_NAMESPACE               \
    }                                           \
  }

#endif
