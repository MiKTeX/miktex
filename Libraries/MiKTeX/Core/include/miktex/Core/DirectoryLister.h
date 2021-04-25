/* miktex/Core/DirectoryLister.h:                       -*- C++ -*-

   Copyright (C) 1996-2021 Christian Schenk

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

#if !defined(DE9EC6DA9018481D82C6DF5E08822CE3)
#define DE9EC6DA9018481D82C6DF5E08822CE3

#include <miktex/Core/config.h>

#include <cstddef>
#include <memory>
#include <string>

#include <miktex/Util/PathName>

MIKTEX_CORE_BEGIN_NAMESPACE;

/// Directory entry information.
struct DirectoryEntry
{
  /// Name of the entry.
  std::string name;
#if defined(MIKTEX_WINDOWS)
  /// Native (Unicode) name of the entry.
  std::wstring wname;
#endif
  /// Indicates whether the entry is a directory.
  bool isDirectory;
};

/// Extended directory entry information.
struct DirectoryEntry2 :
  public DirectoryEntry
{
  /// The size (in bytes) of the file.
  std::size_t size;
};

/// An instances can be used to read entries of a file system directory.
class MIKTEXNOVTABLE DirectoryLister
{
  /// Read options.
public:
  enum class Options :
    int
  {
    None,
    /// Read directory entries only.
    DirectoriesOnly = 1,
    /// Read file entries only.
    FilesOnly = 2,
    /// Include . and .. in result.
    IncludeDotAndDotDot = 4
  };

public:
  virtual MIKTEXTHISCALL ~DirectoryLister() noexcept = 0;

  /// Dispose resources associated with this object.
public:
  virtual void MIKTEXTHISCALL Close() = 0;

  /// Gets the next entry.
  /// @param[out] The next entry.
  /// @return Returns `true`, if the next entry could be retrieved. Returns `false`, if
  /// there are no more entries.
public:
  virtual bool MIKTEXTHISCALL GetNext(DirectoryEntry& direntry) = 0;

  /// Gets the next entry.
  /// @param[out] The next entry.
  /// @return Returns `true`, if the next entry could be retrieved. Returns `false`, if
  /// there are no more entries.
public:
  virtual bool MIKTEXTHISCALL GetNext(DirectoryEntry2& direntry2) = 0;

  /// Creates a new `DirectoryLister` instance.
  /// @param directory File system path to the directory.
  /// @return Returns a smart pointer to the `DirectoryLister` interface.
public:
  static MIKTEXCORECEEAPI(std::unique_ptr<DirectoryLister>) Open(const MiKTeX::Util::PathName& directory);

  /// Creates a new `DirectoryLister` instance.
  /// @param directory File system path to the directory.
  /// @param pattern The glob pattern to be used as the filter.
  /// @return Returns a smart pointer to the `DirectoryLister` interface.
public:
  static MIKTEXCORECEEAPI(std::unique_ptr<DirectoryLister>) Open(const MiKTeX::Util::PathName& directory, const char* pattern);

  /// Creates a new `DirectoryLister` instance.
  /// @param directory File system path to the directory.
  /// @param pattern The glob pattern to be used as the filter.
  /// @param options Read options.
  /// @return Returns a smart pointer to the `DirectoryLister` interface.
public:
  static MIKTEXCORECEEAPI(std::unique_ptr<DirectoryLister>) Open(const MiKTeX::Util::PathName& directory, const char* pattern, int options);
};

MIKTEX_CORE_END_NAMESPACE;

#endif
