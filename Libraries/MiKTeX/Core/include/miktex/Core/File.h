/* miktex/Core/File.h:                                  -*- C++ -*-

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

#if !defined(BBAFDEB7FCB3409B88E23A547CF822B6)
#define BBAFDEB7FCB3409B88E23A547CF822B6

#include <miktex/Core/config.h>

#include <cstddef>
#include <cstdio>
#include <ctime>

#include <vector>

#include "OptionSet.h"
#include "PathName.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

/// File mode enum class.
enum class FileMode
{
  /// Open a file for append access; that is, writing at the end of
  /// file only. If the file already exists, its initial contents
  /// are unchanged and output to the stream is appended to the end
  /// of the file. Otherwise, a new, empty file is created.
  Append,
  /// The file will be created, if it doesn't already exist.
  Create,
  /// Open an existing file.
  Open,
  /// Execute a command and create a pipe.
  Command
};

/// File access enum class.
enum class FileAccess
{
  None,
  /// Read access.
  Read,
  /// Write access.
  Write,
  /// Read/Write access.
  ReadWrite
};

/// File share enum class.
enum class FileShare
{
  /// No other stream can be opened on the file.
  None,
  /// Other streams can be opened for reading only.
  Read,
  /// Other streams can be opened for writing only.
  Write,
  /// Other streams can be opened for reading and writing.
  ReadWrite
};

/// File attribute enum class.
enum class FileAttribute
{
  /// File is a directory.
  Directory,
  /// File is read-only.
  ReadOnly,
#if defined(MIKTEX_WINDOWS)
  /// File is hidden.
  Hidden,
#endif
};

typedef OptionSet<FileAttribute> FileAttributeSet;

enum class FileOpenOption
{
  None,
  DeleteOnClose
};

typedef OptionSet<FileOpenOption> FileOpenOptionSet;

enum class FileDeleteOption
{
  UpdateFndb,
  TryHard,
};

typedef OptionSet<FileDeleteOption> FileDeleteOptionSet;

enum class FileMoveOption
{
  UpdateFndb,
  ReplaceExisting,
};

typedef OptionSet<FileMoveOption> FileMoveOptionSet;

enum class FileCopyOption
{
  UpdateFndb,
  ReplaceExisting,
  PreserveAttributes,
};

typedef OptionSet<FileCopyOption> FileCopyOptionSet;

enum class CreateLinkOption
{
  UpdateFndb,
  ReplaceExisting,
  Symbolic,
};

typedef OptionSet<CreateLinkOption> CreateLinkOptionSet;

/// File class.
class MIKTEXNOVTABLE File
{
public:
  File() = delete;

public:
  File(const File & other) = delete;

public:
  File & operator= (const File & other) = delete;

public:
  File(File && other) = delete;

public:
  File & operator= (File && other) = delete;

public:
  ~File() = delete;

  /// Deletes a file.
public:
  static MIKTEXCORECEEAPI(void) Delete(const PathName & path, FileDeleteOptionSet options);

public:
  static MIKTEXCORECEEAPI(void) Delete(const PathName & path);

  /// Tests if a file exists.
public:
  static MIKTEXCORECEEAPI(bool) Exists(const PathName & path);

public:
  static MIKTEXCORECEEAPI(FileAttributeSet) GetAttributes(const PathName & path);

public:
  static MIKTEXCORECEEAPI(unsigned long) GetNativeAttributes(const PathName & path);

  /// Gets the size of a file.
public:
  static MIKTEXCORECEEAPI(std::size_t) GetSize(const PathName & path);

  /// Renames a file.
public:
  static MIKTEXCORECEEAPI(void) Move(const PathName & source, const PathName & dest, FileMoveOptionSet option);

public:
  static void Move(const PathName & source, const PathName & dest)
  {
    Move(source, dest, {});
  }

  /// Copies a file (with options).
public:
  static MIKTEXCORECEEAPI(void) Copy(const PathName & source, const PathName &dest, FileCopyOptionSet options);

  /// Copies a file.
public:
  static void Copy(const PathName & source, const PathName & dest)
  {
    Copy(source, dest, { FileCopyOption::ReplaceExisting });
  }

  /// Creates a hard/symbolic link
public:
  static MIKTEXCORECEEAPI(void) CreateLink(const PathName & oldName, const PathName & newName, CreateLinkOptionSet options);

public:
  static MIKTEXCORECEEAPI(bool) Equals(const PathName & path1, const PathName & path2);

public:
  /// Sets the maximum number of simultaneously open files.
  static MIKTEXCORECEEAPI(size_t) SetMaxOpen(size_t newMax);

  /// Opens a text stream on a file.
public:
  static MIKTEXCORECEEAPI(FILE *) Open(const PathName & path, FileMode mode, FileAccess access);

  /// Opens a stream on a file.
public:
  static MIKTEXCORECEEAPI(FILE *) Open(const PathName & path, FileMode mode, FileAccess access, bool isTextFile);

  /// Opens a stream on a file.
public:
  static MIKTEXCORECEEAPI(FILE *) Open(const PathName & path, FileMode mode, FileAccess access, bool isTextFile, FileShare share);

  /// Opens a stream on a file.
public:
  static MIKTEXCORECEEAPI(FILE *) Open(const PathName & path, FileMode mode, FileAccess access, bool isTextFile, FileShare share, FileOpenOptionSet options);

  /// Sets file attributes.
public:
  static MIKTEXCORECEEAPI(void) SetAttributes(const PathName & path, FileAttributeSet attributes);

  /// Sets native file attributes.
public:
  static MIKTEXCORECEEAPI(void) SetNativeAttributes(const PathName & path, unsigned long nativeAttributes);

public:
  static MIKTEXCORECEEAPI(void) SetTimes(int fd, time_t creationTime, time_t lastAccessTime, time_t lastWriteTime);

public:
  static MIKTEXCORECEEAPI(void) SetTimes(FILE * stream, time_t creationTime, time_t lastAccessTime, time_t lastWriteTime);

  /// Set file time-stamps.
public:
  static MIKTEXCORECEEAPI(void) SetTimes(const PathName & path, time_t creationTime, time_t lastAccessTime, time_t lastWriteTime);

  /// Gets file time-stamps.
public:
  static MIKTEXCORECEEAPI(void) GetTimes(const PathName & path, time_t & creationTime, time_t & lastAccessTime, time_t & lastWriteTime);

public:
  static time_t GetLastWriteTime(const PathName & path)
  {
    time_t creationTime;
    time_t lastAccessTime;
    time_t lastWriteTime;
    GetTimes(path, creationTime, lastAccessTime, lastWriteTime);
    return lastWriteTime;
  }

  /// Reads a file into memory.
  /// @param path File to read.
  /// @return Returns the read bytes.
public:
  static MIKTEXCORECEEAPI(std::vector<unsigned char>) ReadAllBytes(const PathName & path);
};

MIKTEX_CORE_END_NAMESPACE;

#endif
