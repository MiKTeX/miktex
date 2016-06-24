/* miktex/Core/Directory.h:                             -*- C++ -*-

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

#if !defined(EFF174AE02944E9C9B9B9F35E67C2E54)
#define EFF174AE02944E9C9B9B9F35E67C2E54

#include <miktex/Core/config.h>

#include <ctime>

#include "PathName.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

/// Directory class.
class MIKTEXNOVTABLE Directory
{
  /// Creates a new directory.
public:
  static MIKTEXCORECEEAPI(void) Create(const PathName & path);

  /// Gets the current (working) directory.
public:
  static MIKTEXCORECEEAPI(PathName) GetCurrent();

  /// Sets the current (working) directory.
public:
  static MIKTEXCORECEEAPI(void) SetCurrent(const PathName & path);

  /// Deletes a directory.
public:
  static MIKTEXCORECEEAPI(void) Delete(const PathName & path);

  /// Deletes a directory (recursively).
public:
  static MIKTEXCORECEEAPI(void) Delete(const PathName & path, bool recursive);

  /// Tests if a directory exists.
public:
  static MIKTEXCORECEEAPI(bool) Exists(const PathName & path);

  /// Set directory time-stamps.
public:
  static MIKTEXCORECEEAPI(void) SetTimes(const PathName & path, time_t creationTime, time_t lastAccessTime, time_t lastWriteTime);
};

MIKTEX_CORE_END_NAMESPACE;

#endif
