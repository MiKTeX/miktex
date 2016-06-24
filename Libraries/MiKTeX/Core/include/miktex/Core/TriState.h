/* miktex/Core/TriState.h:                              -*- C++ -*-

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

#if !defined(C74307EE64704F3ABEF85C00DF29E71C)
#define C74307EE64704F3ABEF85C00DF29E71C

#include <miktex/Core/config.h>

MIKTEX_CORE_BEGIN_NAMESPACE;

/// Tri-state enum class.
enum class TriState
{
  /// False (no, off, disabled).
  False,
  /// True (yes, on, enabled).
  True,
  /// Undetermined.
  Undetermined,
};

MIKTEX_CORE_END_NAMESPACE;

#endif
