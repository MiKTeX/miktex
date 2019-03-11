/* miktex/Core/Directory.h:                             -*- C++ -*-

   Copyright (C) 1996-2019 Christian Schenk

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

#if !defined(EFF174AE02944E9C9B9B9F35E67C2E54)
#define EFF174AE02944E9C9B9B9F35E67C2E54

#include <miktex/Core/config.h>

#include <ctime>

#include "OptionSet.h"
#include "PathName.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

/// Directory copy options.
enum class DirectoryCopyOption
{
  /// Update the file name database.
  UpdateFndb,
  /// Replace an existing directory.
  ReplaceExisting,
  /// Preserve attributes.
  PreserveAttributes,
  /// Deep copy.
  CopySubDirectories
};

typedef OptionSet<DirectoryCopyOption> DirectoryCopyOptionSet;

/// @brief Directory class.
///
/// This is a static class which offers directory operations.
class MIKTEXNOVTABLE Directory
{
public:
  Directory() = delete;

public:
  Directory(const Directory& other) = delete;

public:
  Directory& operator=(const Directory& other) = delete;

public:
  Directory(Directory&& other) = delete;

public:
  Directory& operator=(Directory&& other) = delete;

public:
  ~Directory() = delete;

  /// Creates a new directory.
  /// @param path File system path to the directory.
public:
  static MIKTEXCORECEEAPI(void) Create(const PathName& path);

  /// Gets the current (working) directory.
  /// @param Returns the current directory.
public:
  static MIKTEXCORECEEAPI(PathName) GetCurrent();

  /// Sets the current (working) directory.
  /// @param path File system path to the directory.
public:
  static MIKTEXCORECEEAPI(void) SetCurrent(const PathName& path);

  /// Deletes a directory.
  /// @param path File system path to the directory.
public:
  static MIKTEXCORECEEAPI(void) Delete(const PathName& path);

  /// Deletes a directory.
  /// @param path File system path to the directory.
  /// @param recursive Indicates, whether sub-directories shall be removed.
public:
  static MIKTEXCORECEEAPI(void) Delete(const PathName& path, bool recursive);

  /// Copies a directory.
  /// @param source The file system path to the source directory.
  /// @param dest The file system path to the destination directory.
  /// @param options Copy options.
public:
  static MIKTEXCORECEEAPI(void) Copy(const PathName& source, const PathName& dest, DirectoryCopyOptionSet options);

  /// Tests if a directory exists.
  /// @param File system path to the directory.
public:
  static MIKTEXCORECEEAPI(bool) Exists(const PathName& path);

  /// Sets directory timestamps.
  /// @param path The file system path to the directory.
  /// @param creationTime Creation timestamp.
  /// @param lastAccessTime Last access timestamp.
  /// @param lastWriteTime Last modification timestamp.
public:
  static MIKTEXCORECEEAPI(void) SetTimes(const PathName& path, time_t creationTime, time_t lastAccessTime, time_t lastWriteTime);

  /// Renames (moves) a directory.
  /// @param source The file system path to the source directory.
  /// @param dest The file system path to the destination directory.
public:
  static MIKTEXCORECEEAPI(void) Move(const PathName& source, const PathName& dest);
};

MIKTEX_CORE_END_NAMESPACE;

#endif
