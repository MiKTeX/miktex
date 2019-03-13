/* miktex/Core/File.h:                                  -*- C++ -*-

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

#if !defined(BBAFDEB7FCB3409B88E23A547CF822B6)
#define BBAFDEB7FCB3409B88E23A547CF822B6

#include <miktex/Core/config.h>

#if defined(MIKTEX_WINDOWS)
#include <Windows.h>
#endif

#include <cstddef>
#include <cstdio>
#include <ctime>

#include <chrono>
#include <fstream>
#include <vector>

#include "OptionSet.h"
#include "PathName.h"

#if defined(_MSC_VER)
#pragma warning(push)
// "The compiler encountered a deprecated declaration."
#pragma warning( disable : 4996 )
#endif

MIKTEX_CORE_BEGIN_NAMESPACE;

/// How to open a file.
enum class FileMode
{
  /// Open a file for append access; that is, writing at the end of
  /// file only. If the file already exists, its initial contents
  /// are unchanged and output to the stream is appended to the end
  /// of the file. Otherwise, a new, empty file is created.
  Append,
  /// The file will be created, if it doesn't already exist.
  Create,
  /// The file will be created. Raise an error, if the file already exists.
  CreateNew,
  /// Open an existing file.
  Open,
  /// Execute a command and create a pipe.
  Command
};

/// File access options.
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

/// File attributes.
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
#if defined(MIKTEX_UNIX)
  /// File is executable.
  Executable
#endif
};

typedef OptionSet<FileAttribute> FileAttributeSet;

/// Options for opening a file.
enum class FileOpenOption
{
  None,
  /// Delete the file when it is closed.
  DeleteOnClose
};

typedef OptionSet<FileOpenOption> FileOpenOptionSet;

/// Options for deleting a file.
enum class FileDeleteOption
{
  /// Update the file name database.
  UpdateFndb,
  /// Try hard to make the file go away.
  TryHard,
};

typedef OptionSet<FileDeleteOption> FileDeleteOptionSet;

/// Options for renaming (moving) a file.
enum class FileMoveOption
{
  /// Update the file name database.
  UpdateFndb,
  /// Replace an existing file.
  ReplaceExisting,
};

typedef OptionSet<FileMoveOption> FileMoveOptionSet;

/// Options for copying a file.
enum class FileCopyOption
{
  /// Update the file name database.
  UpdateFndb,
  /// Replace an existing file.
  ReplaceExisting,
  /// Preserve file attributes.
  PreserveAttributes,
};

typedef OptionSet<FileCopyOption> FileCopyOptionSet;

/// Options for creating a link.
enum class CreateLinkOption
{
  /// Update the file name database.
  UpdateFndb,
  /// Replace an existing link.
  ReplaceExisting,
  /// Indicates wheter this is a symbolic link or a hard link.
  Symbolic,
};

typedef OptionSet<CreateLinkOption> CreateLinkOptionSet;

/// Options for checking file existance.
enum class FileExistsOption
{
  /// Follow symbolic links.
  SymbolicLink
};

typedef OptionSet<FileExistsOption> FileExistsOptionSet;

/// File class.
///
/// This is a static class which offers file operations.
class MIKTEXNOVTABLE File
{
public:
  File() = delete;

public:
  File(const File& other) = delete;

public:
  File& operator=(const File& other) = delete;

public:
  File(File&& other) = delete;

public:
  File& operator=(File&& other) = delete;

public:
  ~File() = delete;

  /// Deletes a file.
  /// @param path File system path to the file.
  /// @param options Options for deleting the file.
public:
  static MIKTEXCORECEEAPI(void) Delete(const PathName& path, FileDeleteOptionSet options);

  /// Deletes a file.
  /// @param path File system path to the file.
public:
  static MIKTEXCORECEEAPI(void) Delete(const PathName& path);

  /// Tests if a file exists.
  /// @param path File system path to the file.
  /// @param options Options for checking the existance.
public:
  static MIKTEXCORECEEAPI(bool) Exists(const PathName& path, FileExistsOptionSet options);

  /// Tests if a file exists.
  /// @param path File system path to the file.
public:
  static MIKTEXCORECEEAPI(bool) Exists(const PathName& path);

  /// Get attributes of a file.
  /// @param path File system path to the file.
  /// @return Returns the attributes of the file.
  /// @see SetAttributes
public:
  static MIKTEXCORECEEAPI(FileAttributeSet) GetAttributes(const PathName& path);

  /// Get native (operating system) attributes of a file.
  /// @param path File system path to the file.
  /// @return Returns the native attributes of the file.
  /// @see SetNativeAttributes
public:
  static MIKTEXCORECEEAPI(unsigned long) GetNativeAttributes(const PathName& path);

  /// Gets the size of a file.
  /// @param path File system path to the file.
  /// @return Returns the size (in bytes) of the file.
public:
  static MIKTEXCORECEEAPI(std::size_t) GetSize(const PathName& path);

  /// Renames (moves) a file.
  /// @param source The file system path to the source file.
  /// @param dest The file system path to the destination file.
  /// @param options Options for renaming the file.
public:
  static MIKTEXCORECEEAPI(void) Move(const PathName& source, const PathName& dest, FileMoveOptionSet option);

  /// Renames (moves) a file.
  /// @param source The file system path to the source file.
  /// @param dest The file system path to the destination file.
public:
  static void Move(const PathName& source, const PathName& dest)
  {
    Move(source, dest, {});
  }

  /// Copies a file.
  /// @param source The file system path to the source file.
  /// @param dest The file system path to the destination file.
  /// @param options Options for copying the file.  
public:
  static MIKTEXCORECEEAPI(void) Copy(const PathName& source, const PathName &dest, FileCopyOptionSet options);

  /// Copies a file.
  /// @param source The file system path to the source file.
  /// @param dest The file system path to the destination file.
public:
  static void Copy(const PathName& source, const PathName& dest)
  {
    Copy(source, dest, { FileCopyOption::ReplaceExisting });
  }

  /// Creates a file system link.
  /// @param oldName The file system path to the existing file.
  /// @param newName The file system path to link.
  /// @param options Options for creating the link.
public:
  static MIKTEXCORECEEAPI(void) CreateLink(const PathName& oldName, const PathName& newName, CreateLinkOptionSet options);

  /// Tests if a file is a symbolic link.
  /// @param path The file system path to the file.
  /// @return Returns `true`, if the file is a symbolic link.
public:
  static MIKTEXCORECEEAPI(bool) IsSymbolicLink(const PathName& path);

  /// Follows a symbolic link.
  /// @param path The file system path to the link.
  /// @return Returns the file system path to the link target.
public:
  static MIKTEXCORECEEAPI(PathName) ReadSymbolicLink(const PathName& path);

  /// Test equality of two files.
  /// @param path1 The file system path to the first file.
  /// @param path2 The file system path to the second file.
  /// @return Returns `true`, if both files compare equal.
public:
  static MIKTEXCORECEEAPI(bool) Equals(const PathName& path1, const PathName& path2);

  /// Sets the maximum number of simultaneously open files.
  /// @todo To be removed
public:
  static MIKTEXCORECEEAPI(size_t) SetMaxOpen(size_t newMax);

  /// Opens a file.
  /// @param path The file system path to the file.
  /// @param mode Specifies how the file is to open.
  /// @param access Specifies how the file will be accessed.
  /// @return Returns the pointer to a `FILE` object.
public:
  static MIKTEXCORECEEAPI(FILE*) Open(const PathName& path, FileMode mode, FileAccess access);

  /// Opens a file.
  /// @param path The file system path to the file.
  /// @param mode Specifies how the file is to open.
  /// @param access Specifies how the file will be accessed.
  /// @param isTextFile Specifies if the file should be opened in text mode.
  /// @return Returns the pointer to a `FILE` object.
public:
  static MIKTEXCORECEEAPI(FILE*) Open(const PathName& path, FileMode mode, FileAccess access, bool isTextFile);

  /// Opens a file.
  /// @param path The file system path to the file.
  /// @param mode Specifies how the file is to open.
  /// @param access Specifies how the file will be accessed.
  /// @param isTextFile Specifies if the file should be opened in text mode.
  /// @param options Options for opening the file.
  /// @return Returns the pointer to a `FILE` object.
public:
  static MIKTEXCORECEEAPI(FILE*) Open(const PathName& path, FileMode mode, FileAccess access, bool isTextFile, FileOpenOptionSet options);

  /// Opens an input stream on a file.
  /// @param path The file system path to the file.
  /// @param mode Specifies how the file is open.
  /// @param exceptions Specifies the exceptions to be thrown if an error occurs.
  /// @return Returns an `ifstream` object.
public:
  static MIKTEXCORECEEAPI(std::ifstream) CreateInputStream(const PathName& path, std::ios_base::openmode mode, std::ios_base::iostate exceptions);

  /// Opens an input stream on a file.
  /// @param path The file system path to the file.
  /// @return Returns an `ifstream` object.
public:
  static std::ifstream CreateInputStream(const PathName& path)
  {
    return CreateInputStream(path, std::ios_base::in, std::ios_base::badbit);
  }

  /// Opens an output stream on a file.
  /// @param path The file system path to the file.
  /// @param mode Specifies how the file is to open.
  /// @param exceptions Specifies the exceptions to be thrown if an error occurs.
  /// @return Returns an `ofstream` object.
public:
  static MIKTEXCORECEEAPI(std::ofstream) CreateOutputStream(const PathName& path, std::ios_base::openmode mode, std::ios_base::iostate exceptions);

  /// Opens an output stream on a file.
  /// @param path The file system path to the file.
  /// @param mode Specifies how the file is to open.
  /// @return Returns an `ofstream` object.
public:
  static std::ofstream CreateOutputStream(const PathName& path, std::ios_base::openmode mode)
  {
    return CreateOutputStream(path, mode, std::ios_base::badbit | std::ios_base::failbit);
  }

  /// Opens an output stream on a file.
  /// @param path The file system path to the file.
  /// @return Returns an `ofstream` object.
public:
  static std::ofstream CreateOutputStream(const PathName& path)
  {
    return CreateOutputStream(path, std::ios_base::out, std::ios_base::badbit | std::ios_base::failbit);
  }

  /// Sets file attributes.
  /// @param path The file system path to the file.
  /// @param attributes The attributes to set.
  /// @see GetAttributes
public:
  static MIKTEXCORECEEAPI(void) SetAttributes(const PathName& path, FileAttributeSet attributes);

  /// Sets native (OS) file attributes.
  /// @param path The file system path to the file.
  /// @param attributes The native attributes to set.
  /// @see GetNativeAttributes
public:
  static MIKTEXCORECEEAPI(void) SetNativeAttributes(const PathName& path, unsigned long nativeAttributes);

  /// Sets file timestamps.
  /// @param fd The file descriptor.
  /// @param creationTime Creation timestamp.
  /// @param lastAccessTime Last access timestamp.
  /// @param lastWriteTime Last modification timestamp.
public:
  static MIKTEXCORECEEAPI(void) SetTimes(int fd, time_t creationTime, time_t lastAccessTime, time_t lastWriteTime);

  /// Sets file timestamps.
  /// @param file The pointer to the `FILE` object.
  /// @param creationTime Creation timestamp.
  /// @param lastAccessTime Last access timestamp.
  /// @param lastWriteTime Last modification timestamp.
public:
  static MIKTEXCORECEEAPI(void) SetTimes(FILE* file, time_t creationTime, time_t lastAccessTime, time_t lastWriteTime);

  /// Sets file timestamps.
  /// @param path The file system path to the file.
  /// @param creationTime Creation timestamp.
  /// @param lastAccessTime Last access timestamp.
  /// @param lastWriteTime Last modification timestamp.
public:
  static MIKTEXCORECEEAPI(void) SetTimes(const PathName& path, time_t creationTime, time_t lastAccessTime, time_t lastWriteTime);

  /// Gets file timestamps.
  /// @param path The file system path to the file.
  /// @param[out] creationTime Creation timestamp.
  /// @param[out] lastAccessTime Last access timestamp.
  /// @param[out] lastWriteTime Last modification timestamp.
public:
  static MIKTEXCORECEEAPI(void) GetTimes(const PathName& path, time_t& creationTime, time_t& lastAccessTime, time_t& lastWriteTime);

  /// Gets the modification timestamp of a file.
  /// @param path The file system path to the file.
  /// @param[out] creationTime Creation timestamp.
  /// @param[out] lastAccessTime Last access timestamp.
  /// @param[out] lastWriteTime Last modification timestamp.
public:
  static time_t GetLastWriteTime(const PathName& path)
  {
    time_t creationTime;
    time_t lastAccessTime;
    time_t lastWriteTime;
    GetTimes(path, creationTime, lastAccessTime, lastWriteTime);
    return lastWriteTime;
  }

  /// Reads a file.
  /// @param path The file system path to the file.
  /// @return Returns the file contents.
public:
  static MIKTEXCORECEEAPI(std::vector<unsigned char>) ReadAllBytes(const PathName& path);

  /// Write a file.
  /// @param path The file system path to the file.
  /// @param The file contents.
public:
  static MIKTEXCORECEEAPI(void) WriteBytes(const PathName& path, const std::vector<unsigned char>& data);
  
  /// File lock type.
public:
  enum class LockType
  {
    /// Shared lock.
    Shared,
    /// Exclusive lock.
    Exclusive
  };
  
  /// Tries to lock a file.
  /// @param fd The file descriptor.
  /// @param lockType The requested lock type.
  /// @param timeout The maximum time waited for the file lock.
  /// @return Returns `true`, if the file could be locked.
public:
  static MIKTEXCORECEEAPI(bool) TryLock(int fd, LockType lockType, std::chrono::milliseconds timeout);

  /// Tries to lock a file.
  /// @param file The pointer to a `FILE` object.
  /// @param lockType The requested lock type.
  /// @param timeout The maximum time waited for the file lock.
  /// @return Returns `true`, if the file could be locked.
public:
  static bool TryLock(FILE* file, LockType lockType, std::chrono::milliseconds timeout)
  {
    return TryLock(fileno(file), lockType, timeout);
  }

#if defined(MIKTEX_WINDOWS)
  /// Tries to lock a file.
  /// @param hFile The file `HANDLE`.
  /// @param lockType The requested lock type.
  /// @param timeout The maximum time waited for the file lock.
  /// @return Returns `true`, if the file could be locked.
public:
  static MIKTEXCORECEEAPI(bool) TryLock(HANDLE hFile, LockType lockType, std::chrono::milliseconds timeout);
#endif

  /// Unlocks a locked file.
  /// @param fd The file descriptor.
public:
  static MIKTEXCORECEEAPI(void) Unlock(int fd);

  /// Unlocks a locked file.
  /// @param file The pointer to a `FILE` object.
public:
  static void Unlock(FILE* file)
  {
    Unlock(fileno(file));
  }

#if defined(MIKTEX_WINDOWS)
  /// Unlocks a locked file.
  /// @param hFile The file `HANDLE`.
public:
  static MIKTEXCORECEEAPI(void) Unlock(HANDLE hFile);
#endif
};

MIKTEX_CORE_END_NAMESPACE;

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif
