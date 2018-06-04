/* text.h:                                              -*- C++ -*-

   Copyright (C) 2001-2018 Christian Schenk

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

#if !defined(FFF7DC1F73A048E59858E6E2935606FB)
#define FFF7DC1F73A048E59858E6E2935606FB

#include <miktex/Core/Quoter>
#include <miktex/Core/Text>

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

#if defined(MIKTEX_WINDOWS)
#  define WU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#  define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif

#endif
