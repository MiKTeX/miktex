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

#pragma once

#if !defined(BA15DC038D4549859111D4B075360D81)
#define BA15DC038D4549859111D4B075360D81

#include <mutex>

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
  static std::shared_ptr<FileNameDatabase> Create(const MiKTeX::Core::PathName& fndbPath, const MiKTeX::Core::PathName& rootDirectory, bool readOnly);

public:
  bool Search(const MiKTeX::Core::PathName& relativePath, const std::string& pathPattern, bool firstMatchOnly, std::vector<MiKTeX::Core::PathName>& result, std::vector<std::string>& fileNameInfo) const;

public:
  void AddFile(const MiKTeX::Core::PathName& path, const std::string& fileNameInfo);

public:
  void RemoveFile(const MiKTeX::Core::PathName& path);

public:
  bool IsInvariable() const
  {
    return isInvariable;
  }

public:
  bool Enumerate(const MiKTeX::Core::PathName& fndbPath, MiKTeX::Core::IEnumerateFndbCallback* callback) const;

public:
  bool FileExists(const MiKTeX::Core::PathName& path) const;

private:
  void ReadFileNames();

private:
  void ReadFileNames(FileNameDatabaseDirectory* dir);

#if 0 // experimental
public:
  std::unique_ptr<MiKTeX::Core::DirectoryLister> OpenDirectory(const char* path);
#endif

public:
  FileNameDatabase();

public:
  FileNameDatabase(const FileNameDatabase& rhs) = delete;

public:
  virtual ~FileNameDatabase();

private:
  void Finalize();

private:
  FileNameDatabaseDirectory* CreateFndbDirectory(FileNameDatabaseDirectory* dir, const std::string& name) const;

private:
  FileNameDatabaseDirectory* CreateDirectoryPath(FileNameDatabaseDirectory* dir, const MiKTeX::Core::PathName& relPath) const;

private:
  FndbByteOffset CreateString(const std::string& name) const;

private:
  FileNameDatabaseDirectory* ExtendDirectory(FileNameDatabaseDirectory* dir) const;

private:
  FndbWord FindLowerBound(const FndbByteOffset& begin, FndbWord count, const char* name, bool& isDuplicate) const;

private:
  FileNameDatabaseDirectory* FindSubDirectory(const FileNameDatabaseDirectory* dir, const MiKTeX::Core::PathName& relPath) const;

private:
  FileNameDatabaseDirectory* TryGetParent(const MiKTeX::Core::PathName& path) const;

private:
  void Flush() const;

private:
  FileNameDatabaseDirectory* GetDirectoryAt(FndbByteOffset fo) const
  {
    return reinterpret_cast<FileNameDatabaseDirectory*>(GetPointer(fo));
  }

private:
  FndbWord GetHeaderFlags() const
  {
    return pHeader->flags;
  }

private:
  FndbByteOffset GetByteOffset(const void* p) const
  {
    ptrdiff_t d = reinterpret_cast<const uint8_t*>(p) - reinterpret_cast<const uint8_t*>(pHeader);
    MIKTEX_ASSERT(d == 0 || d >= sizeof(FileNameDatabaseHeader) && d < foEnd);
    return d;
  }

private:
  void* GetPointer(FndbByteOffset fo) const
  {
    MIKTEX_ASSERT(fo == 0 || fo >= sizeof(FileNameDatabaseHeader) && fo < foEnd);
    return fo == 0 ? nullptr : reinterpret_cast<uint8_t*>(pHeader) + fo;
  }

private:
  char* GetString(FndbByteOffset fo) const
  {
    MIKTEX_ASSERT(fo >= sizeof(FileNameDatabaseHeader) && fo < foEnd);
    MIKTEX_ASSERT_STRING(reinterpret_cast<const char*>(GetPointer(fo)));
    return reinterpret_cast<char*>(GetPointer(fo));
  }

private:
  FileNameDatabaseDirectory* GetTopDirectory() const
  {
    return GetDirectoryAt(pHeader->foTopDir);
  }

private:
  bool HasFileNameInfo() const
  {
    return (pHeader->flags& FileNameDatabaseHeader::FndbFlags::FileNameInfo) != 0;
  }

private:
  void Initialize(const MiKTeX::Core::PathName& fndbPath, const MiKTeX::Core::PathName& rootDirectory, bool readWrite = false);

private:
  void InsertDirectory(FileNameDatabaseDirectory* dir, const FileNameDatabaseDirectory* subDir) const;

private:
  void InsertFileName(FileNameDatabaseDirectory* dir, FndbByteOffset foFileName, FndbByteOffset foFileNameInfo) const;

private:
  bool IsDirty() const
  {
    MIKTEX_ASSERT(pHeader != nullptr);
    return pHeader->timeStamp != timeStamp;
  }

private:
  void MakePathName(const FileNameDatabaseDirectory* dir, MiKTeX::Core::PathName& path) const;

private:
  void OpenFileNameDatabase(const MiKTeX::Core::PathName& fndbPath, bool readWrite = false);

private:
  FileNameDatabaseDirectory* RemoveFileName(FileNameDatabaseDirectory* dir, const MiKTeX::Core::PathName& fileName) const;

private:
  FileNameDatabaseDirectory* SearchFileName(FileNameDatabaseDirectory* dir, const MiKTeX::Core::PathName& fileName, FndbWord& index) const;

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
  FileNameDatabaseHeader* pHeader = nullptr;

  // file-system path to root directory
private:
  MiKTeX::Core::PathName rootDirectory;

private:
  typedef std::unordered_multimap<std::string, FileNameDatabaseDirectory*> FileNameHashTable;

private:
  FileNameHashTable fileNames;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> traceStream;

private:
  std::mutex thisMutex;
};

END_INTERNAL_NAMESPACE;

#endif
