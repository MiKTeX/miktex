/* miktex/Core/StructChecker.h:                         -*- C++ -*-

   Copyright (C) 1996-2018 Christian Schenk

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

#pragma once

#if !defined(B1552F71B2934A77A3D98E19A53D0584)
#define B1552F71B2934A77A3D98E19A53D0584

#include <miktex/Core/config.h>

#include <cstddef>

MIKTEX_CORE_BEGIN_NAMESPACE;

template<class Derived> struct StructChecker
{
private:
  std::size_t sizeOfStruct = sizeof(Derived);

public:
  std::size_t GetSizeOfStruct() const
  {
    return sizeOfStruct;
  }
};

MIKTEX_CORE_END_NAMESPACE;

#endif
