/* miktex/Core/PathNameParser.h:                        -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(E46FF47278D7421D9657C798FB641B8C)
#define E46FF47278D7421D9657C798FB641B8C

#include <miktex/Core/config.h>

#include "BufferSizes.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

class PathNameParser
{
public:
  MIKTEXCOREEXPORT MIKTEXTHISCALL PathNameParser(const char * lpszPath);

public:
  PathNameParser(const PathNameParser & rhs) = delete;

public:
  PathNameParser & operator= (const PathNameParser & rhs) = delete;

public:
  const char * GetCurrent() const
  {
    return *lpszCurrent == 0 ? nullptr : lpszCurrent;
  }

public:
  MIKTEXCORETHISAPI(const char *) operator++ ();

private:
  char * lpszCurrent = nullptr;

private:
  char * lpszNext = nullptr;

private:
  char buffer[BufferSizes::MaxPath];
};

MIKTEX_CORE_END_NAMESPACE;

#endif
