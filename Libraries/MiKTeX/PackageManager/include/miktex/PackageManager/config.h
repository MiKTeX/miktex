/* miktex/PackageManager/config.h:                      -*- C++ -*-

   Copyright (C) 2001-2016 Christian Schenk

   This file is part of MiKTeX Package Manager.

   MiKTeX Package Manager is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Package Manager is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Package Manager; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(E69D9A4DE14E48CDA0E007AA28588BA6)
#define E69D9A4DE14E48CDA0E007AA28588BA6

#include <miktex/First>
#include <miktex/Definitions>

// DLL import/export switch
#if !defined(F927BA187CB94546AB9CA9099D989E81)
#  if defined(MIKTEX_MPM_SHARED)
#    define MIKTEXMPMEXPORT MIKTEXDLLIMPORT
#  else
#    define MIKTEXMPMEXPORT
#  endif
#endif

// API decoration for exported member functions
#define MIKTEXMPMCEEAPI(type) MIKTEXMPMEXPORT type MIKTEXCEECALL

#define MPM_BEGIN_NAMESPACE                     \
  namespace MiKTeX {                            \
    namespace Packages {

#define MPM_END_NAMESPACE                       \
    }                                           \
  }

#define MIKTEX_EXTENDED_PACKAGEINFO 1

#endif
