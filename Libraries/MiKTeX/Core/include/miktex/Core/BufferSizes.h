/* miktex/Core/BufferSizes.h:                           -*- C++ -*-

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

#pragma once

#if !defined(A5D72223A6D04880A4C8CE4B747AF2BD)
#define A5D72223A6D04880A4C8CE4B747AF2BD

#include <miktex/Core/config.h>

#include <cstddef>

MIKTEX_CORE_BEGIN_NAMESPACE;

/// Hardcoded buffer sizes.
struct BufferSizes
{
public:
  /// Initial capacity of a path name (including the terminating null-byte).
  static const std::size_t MaxPath = 260;
  /// Maximum length of a package key (including the terminating
  /// null-byte).
  static const std::size_t MaxPackageName = 256;
  /// Maximum length of a configuration key (including the
  /// terminating null-byte).
  static const std::size_t MaxCfgName = 256;
};

MIKTEX_CORE_END_NAMESPACE;

#endif
