/* miktex/Core/DirectoryLister.h:                       -*- C++ -*-

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

#if !defined(DE9EC6DA9018481D82C6DF5E08822CE3)
#define DE9EC6DA9018481D82C6DF5E08822CE3

#include <miktex/Core/config.h>

#include <cstddef>
#include <memory>
#include <string>

#include "PathName.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

/// Contains information about a directory entry.
struct DirectoryEntry
{
  /// Name of the entry.
  std::string name;
#if defined(MIKTEX_WINDOWS)
  /// Native (Unicode) name of the entry.
  std::wstring wname;
#endif
  /// Indicates whether the entry is a sub-directory.
  bool isDirectory;
};

/// Contains detailed information about a directory entry.
struct DirectoryEntry2 : public DirectoryEntry
{
  /// The size (in bytes) of the file.
  std::size_t size;
};

/// Instances of this class can be used to get the contents of a directory.
class MIKTEXNOVTABLE DirectoryLister
{
public:
  enum class Options : int
  {
    None,
    DirectoriesOnly = 1,
    FilesOnly = 2
  };

  /// Destructor.
public:
  virtual MIKTEXTHISCALL ~DirectoryLister() = 0;

  /// Closes this DirectoryLister.
public:
  virtual void MIKTEXTHISCALL Close() = 0;

  /// Gets the next directory entry.
  /// @param[out] The directory entry to be filled.
  /// @return Returns true, if an entry could be retrieved. Returns, if
  /// there are no more directory entries.
public:
  virtual bool MIKTEXTHISCALL GetNext(DirectoryEntry & direntry) = 0;

  /// Gets the next directory entry.
  /// @param[out] The directory entry to be filled.
  /// @return Returns true, if an entry could be retrieved. Returns, if
  /// there are no more directory entries.
public:
  virtual bool MIKTEXTHISCALL GetNext(DirectoryEntry2 & direntry2) = 0;

  /// Creates a DirectoryLister object. The caller is responsible for deleting
  /// the object.
  /// @param directory Path to the directory.
  /// @return Returns the DirectoryLister object.
public:
  static MIKTEXCORECEEAPI(std::unique_ptr<DirectoryLister>) Open(const PathName & directory);

  /// Creates a DirectoryLister object. The caller is responsible for deleting
  /// the object.
  /// @param directory Path to the directory.
  /// @param lpszPattern A filter pattern (e.g. "*.txt").
  /// @return Returns the DirectoryLister object.
public:
  static MIKTEXCORECEEAPI(std::unique_ptr<DirectoryLister>) Open(const PathName & directory, const char * lpszPattern);

  /// Creates a DirectoryLister object. The caller is responsible for deleting
  /// the object.
  /// @param directory Path to the directory.
  /// @param lpszPattern A filter pattern (e.g. "*.txt").
  /// @param options Options.
  /// @return Returns the DirectoryLister object.
public:
  static MIKTEXCORECEEAPI(std::unique_ptr<DirectoryLister>) Open(const PathName & directory, const char * lpszPattern, int options);
};

MIKTEX_CORE_END_NAMESPACE;

#endif
