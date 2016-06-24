/* internal.h: internal definitions                     -*- C++ -*-

   Copyright (C) 2001-2016 Christian Schenk

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

#if defined(_MSC_VER) && defined(MIKTEX_EXTRACTOR_SHARED)
#  define MIKTEXEXTRACTOREXPORT __declspec(dllexport)
#else
#  define MIKTEXEXTRACTOREXPORT
#endif

#define DAA6476494C144C8BED9A9E8810BAABA
#include "miktex/Extractor/Extractor.h"

#define BEGIN_INTERNAL_NAMESPACE                        \
namespace MiKTeX {                                      \
  namespace Extractor {                                 \
    namespace AF1A1A64A53D45708F96161A1541D424 {

#define END_INTERNAL_NAMESPACE                  \
    }                                           \
  }                                             \
}

#define BEGIN_ANONYMOUS_NAMESPACE namespace {
#define END_ANONYMOUS_NAMESPACE }

#if ! defined(UNUSED)
#  if ! defined(NDEBUG)
#    define UNUSED(x)
#  else
#    define UNUSED(x) static_cast<void>(x)
#  endif
#endif

#if ! defined(UNUSED_ALWAYS)
#  define UNUSED_ALWAYS(x) static_cast<void>(x)
#endif

#define UNIMPLEMENTED() MIKTEX_INTERNAL_ERROR()

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).Get()

#define EXTRACTORSTATICFUNC(type) static type
#define EXTRACTORINTERNALFUNC(type) type
#define EXTRACTORINTERNALVAR(type) type

#define ARRAY_SIZE(buf) (sizeof(buf)/sizeof(buf[0]))

#include "miktex/Extractor/Extractor"

#define CURRENT_DIRECTORY "."

BEGIN_INTERNAL_NAMESPACE;
END_INTERNAL_NAMESPACE;

using namespace MiKTeX::Extractor::AF1A1A64A53D45708F96161A1541D424;