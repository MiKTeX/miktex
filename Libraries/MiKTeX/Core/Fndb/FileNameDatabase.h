/* FileNameDatabase.h: file name database                 -*- C++ -*-

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

#if !defined(BA15DC038D4549859111D4B075360D81)
#define BA15DC038D4549859111D4B075360D81

#include "miktex/Core/Debug.h"
#include "miktex/Core/DirectoryLister.h"
#include "miktex/Core/Fndb.h"
#include "miktex/Core/MemoryMappedFile.h"
#include "miktex/Core/PathName.h"

#include "fndbmem.h"

BEGIN_INTERNAL_NAMESPACE;

class FileNameDatabase
{
public:
  static std::shared_ptr<FileNameDatabase> Create(const char * lpszFndbPath, const char * lpszRoot, bool readOnly);

public:
  bool Search(const char * lpszFileName, const char * lpszPathPattern, bool firstMatchOnly, std::vector<MiKTeX::Core::PathName> & result, std::vector<std::string> & fileNameInfo) const;

public:
  void AddFile(const char * lpszPath, const char * lpszFileNameInfo);

public:
  void RemoveFile(const char * lpszPath);

public:
  bool IsInvariable() const
  {
    return isInvariable;
  }

public:
  bool Enumerate(const char * lpszPath, MiKTeX::Core::IEnumerateFndbCallback * pCallback) const;

public:
  bool FileExists(const MiKTeX::Core::PathName & path) const;

private:
  void ReadFileNames();

private:
  void ReadFileNames(FileNameDatabaseDirectory * pDir);

#if 1 // experimental
public:
  std::unique_ptr<MiKTeX::Core::DirectoryLister> OpenDirectory(const char * lpszPath);
#endif

public:
  FileNameDatabase();

public:
  FileNameDatabase(const FileNameDatabase & rhs) = delete;

public:
  virtual ~FileNameDatabase();

private:
  void Finalize();

private:
  FileNameDatabaseDirectory * CreateFndbDirectory(FileNameDatabaseDirectory * pDir, const char * lpszName) const;

private:
  FileNameDatabaseDirectory * CreateDirectoryPath(FileNameDatabaseDirectory * pDir, const char * lpszRelPath) const;

private:
  FndbByteOffset CreateString(const char * lpszName) const;

private:
  FileNameDatabaseDirectory * ExtendDirectory(FileNameDatabaseDirectory * pDir) const;

private:
  FndbWord FindLowerBound(const FndbByteOffset & begin, FndbWord count, const char * lpszName, bool & isDuplicate) const;

private:
  FileNameDatabaseDirectory * FindSubDirectory(const FileNameDatabaseDirectory * pDir, const char * lpszRelPath) const;

private:
  FileNameDatabaseDirectory * TryGetParent(const char * lpszPath) const;

private:
  void Flush() const;

private:
  FileNameDatabaseDirectory * GetDirectoryAt(FndbByteOffset fo) const
  {
    return reinterpret_cast<FileNameDatabaseDirectory*>(GetPointer(fo));
  }

private:
  FndbWord GetHeaderFlags() const
  {
    return pHeader->flags;
  }

private:
  FndbByteOffset GetByteOffset(const void * p) const
  {
    ptrdiff_t d = reinterpret_cast<const uint8_t *>(p) - reinterpret_cast<const uint8_t *>(pHeader);
    MIKTEX_ASSERT(d == 0 || d >= sizeof(FileNameDatabaseHeader) && d < foEnd);
    return d;
  }

private:
  void * GetPointer(FndbByteOffset fo) const
  {
    MIKTEX_ASSERT(fo == 0 || fo >= sizeof(FileNameDatabaseHeader) && fo < foEnd);
    return fo == 0 ? nullptr : reinterpret_cast<uint8_t*>(pHeader) + fo;
  }

private:
  char * GetString(FndbByteOffset fo) const
  {
    MIKTEX_ASSERT(fo >= sizeof(FileNameDatabaseHeader) && fo < foEnd);
    MIKTEX_ASSERT_STRING(reinterpret_cast<const char *>(GetPointer(fo)));
    return reinterpret_cast<char *>(GetPointer(fo));
  }

private:
  FileNameDatabaseDirectory * GetTopDirectory() const
  {
    return GetDirectoryAt(pHeader->foTopDir);
  }

private:
  bool HasFileNameInfo() const
  {
    return (pHeader->flags & FileNameDatabaseHeader::FndbFlags::FileNameInfo) != 0;
  }

private:
  void Initialize(const char * lpszFndbPath, const char * lpszRoot, bool readWrite = false);

private:
  void InsertDirectory(FileNameDatabaseDirectory * pDir, const FileNameDatabaseDirectory * pDirSub) const;

private:
  void InsertFileName(FileNameDatabaseDirectory * pDir, FndbByteOffset foFileName, FndbByteOffset foFileNameInfo) const;

private:
  bool IsDirty() const
  {
    MIKTEX_ASSERT(pHeader != nullptr);
    return pHeader->timeStamp != timeStamp;
  }

private:
  void MakePathName(const FileNameDatabaseDirectory * pDir, MiKTeX::Core::PathName & path) const;

private:
  void OpenFileNameDatabase(const char * lpszFndbPath, bool readWrite = false);

private:
  FileNameDatabaseDirectory * RemoveFileName(FileNameDatabaseDirectory * pDir, const char * lpszFileName) const;

private:
  FileNameDatabaseDirectory * SearchFileName(FileNameDatabaseDirectory * pDir, const char * lpszFileName, FndbWord & index) const;

  // true, if the FNDB is read-only
private:
  bool isInvariable = false;

  // last modification time
private:
  FndbWord timeStamp = 0;

  // size (in bytes) of the FNDB file
private:
  FndbByteOffset foEnd = 0;

private:
  std::unique_ptr<MiKTeX::Core::MemoryMappedFile> mmap;

  // pointer to the FNDB header
private:
  FileNameDatabaseHeader * pHeader = nullptr;

  // file-system path to root directory
private:
  MiKTeX::Core::PathName rootDirectory;

private:
  typedef std::unordered_multimap<std::string, FileNameDatabaseDirectory *> FileNameHashTable;

private:
  FileNameHashTable fileNames;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> traceStream;

private:
  std::mutex thisMutex;
};

END_INTERNAL_NAMESPACE;

#endif
