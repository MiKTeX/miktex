/* miktex/Core/LockFile.h:                              -*- C++ -*-

   Copyright (C) 2018-2021 Christian Schenk

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

#if !defined(AC4770A4B46740C1A3C4AEA7DB977164)
#define AC4770A4B46740C1A3C4AEA7DB977164

#include <miktex/Core/config.h>

#include <cstddef>

#include <chrono>
#include <memory>

#include <miktex/Util/PathName>

MIKTEX_CORE_BEGIN_NAMESPACE;

/// An instance of this class provides an interface to a lock file.
class MIKTEXNOVTABLE LockFile
{
public:
  virtual MIKTEXTHISCALL ~LockFile() noexcept = 0;

  /// Tries to create the lock file.
  /// @param time The maximum time waited for the operation to succeed.
  /// @return Returns `true`, if the lock file has been created.
public:
  virtual bool MIKTEXTHISCALL TryLock(std::chrono::milliseconds timeout) = 0;

  /// Removes the lock file.
public:
  virtual void MIKTEXTHISCALL Unlock() = 0;

  /// Creates a new `LockFile` object.
  /// @param path The file system path to the lock file.
  /// @return Returns a smart pointer to the new `LockFile` object.
public:
  static MIKTEXCORECEEAPI(std::unique_ptr<LockFile>) Create(const MiKTeX::Util::PathName& path);
};

MIKTEX_CORE_END_NAMESPACE;

#endif
