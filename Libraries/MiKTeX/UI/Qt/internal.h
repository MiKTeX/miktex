/* internal.h: internal definitions                     -*- C++ -*-

   Copyright (C) 2008-2018 Christian Schenk

   This file is part of MiKTeX UI Library.

   MiKTeX UI Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX UI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX UI Library; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if defined(MIKTEX_UI_QT_SHARED)
#  define MIKTEXUIQTEXPORT MIKTEXDLLEXPORT
#else
#  define MIKTEXUIQTEXPORT
#endif

#define F752091EC06B4B4E827B2AACABAEE953
#include "miktex/UI/Qt/Prototypes.h"

#define BEGIN_INTERNAL_NAMESPACE                \
namespace MiKTeX {                              \
  namespace ED413B31CB18472688D97F1DDDE396B9 {

#define END_INTERNAL_NAMESPACE                  \
  }                                             \
}

BEGIN_INTERNAL_NAMESPACE;
END_INTERNAL_NAMESPACE;

using namespace MiKTeX::ED413B31CB18472688D97F1DDDE396B9;

#define WU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#define T_(x) MIKTEXTEXT(x)
