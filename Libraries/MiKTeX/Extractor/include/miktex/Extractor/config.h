/* miktex/Extractor/config.h: library configuration     -*- C++ -*-

   Copyright (C) 2008-2018 Christian Schenk

   This file is part of MiKTeX Extractor.

   MiKTeX Extractor is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX Extractor is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Extractor; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#pragma once

#if !defined(D7FF36DC8CE240E398C46F976819AD44)
#define D7FF36DC8CE240E398C46F976819AD44

#include <miktex/First>
#include <miktex/Definitions>

// DLL import/export switch
#if !defined(DAA6476494C144C8BED9A9E8810BAABA)
#  if defined(MIKTEX_EXTRACTOR_SHARED)
#    define MIKTEXEXTRACTOREXPORT MIKTEXDLLIMPORT
#  else
#    define MIKTEXEXTRACTOREXPORT
#  endif
#endif

// API decoration for exported member functions
#define MIKTEXEXTRACTORCEEAPI(type) MIKTEXEXTRACTOREXPORT type MIKTEXCEECALL

#define MIKTEX_EXTRACTOR_BEGIN_NAMESPACE        \
  namespace MiKTeX {                            \
    namespace Extractor {

#define MIKTEX_EXTRACTOR_END_NAMESPACE          \
    }                                           \
  }

#endif
