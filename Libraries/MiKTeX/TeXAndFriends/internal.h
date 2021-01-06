/* internal.h: internal definitions                     -*- C++ -*-

   Copyright (C) 1996-2021 Christian Schenk

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

#pragma once

#define FIRST_OPTION_VAL 256

#define STATICFUNC(type) type

#define MIKTEX_API_BEGIN(funcname) {
#define MIKTEX_API_END(funcname) }

#define T_(x) this->Translate(x)

#define MIKTEX_UNIMPLEMENTED(x) MIKTEX_UNEXPECTED(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

#define BEGIN_INTERNAL_NAMESPACE                        \
namespace MiKTeX {                                      \
  namespace TeXAndFriends {                             \
    namespace CDA7FC807F4A47E294AF2721867074A4 {

#define END_INTERNAL_NAMESPACE                  \
    }                                           \
  }                                             \
}

#include "TeXMFResources.h"

BEGIN_INTERNAL_NAMESPACE;

inline int GetC(FILE* file)
{
  MIKTEX_ASSERT(file != nullptr);
  int ch = getc(file);
  if (ch == EOF && ferror(file) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("getc");
  }
  return ch;
}

END_INTERNAL_NAMESPACE;


using namespace MiKTeX::TeXAndFriends::CDA7FC807F4A47E294AF2721867074A4;
