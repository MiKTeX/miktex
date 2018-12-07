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

#include <tuple>

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
  static std::shared_ptr<FileNameDatabase> Create(const MiKTeX::Core::PathName& fndbPath, const MiKTeX::Core::PathName& rootDirectory);

public:
  FileNameDatabase();

public:
  FileNameDatabase(const FileNameDatabase& rhs) = delete;

public:
  virtual ~FileNameDatabase();

public:
  bool Search(const MiKTeX::Core::PathName& relativePath, const std::string& pathPattern, bool firstMatchOnly, std::vector<MiKTeX::Core::Fndb::Record>& result);

public:
  void Add(const std::vector<MiKTeX::Core::Fndb::Record>& records);

public:
  void Remove(const std::vector<MiKTeX::Core::PathName>& paths);

public:
  bool FileExists(const MiKTeX::Core::PathName& path);

private:
  struct Record
  {
    std::string fileName;
    std::string directory;
    std::string info;
  };

private:
  std::tuple<std::string, std::string> SplitPath(const MiKTeX::Core::PathName& path) const;

private:
  std::string MakeKey(const std::string& fileName) const;

private:
  std::string MakeKey(const MiKTeX::Core::PathName& fileName) const;

private:
  bool InsertRecord(const Record& record);

private:
  void EraseRecord(const Record& record);
  
private:
  void ReadFileNames();

#if MIKTEX_FNDB_VERSION == 5
private:
  void ReadFileNames(const FileNameDatabaseRecord* table);
#endif

#if MIKTEX_FNDB_VERSION == 4
private:
  void ReadFileNames(const FileNameDatabaseDirectory* dir);
#endif

private:
  void Finalize();

#if MIKTEX_FNDB_VERSION == 4
private:
  const FileNameDatabaseDirectory* GetDirectoryAt(FndbByteOffset fo) const
  {
    return reinterpret_cast<const FileNameDatabaseDirectory*>(GetPointer(fo));
  }
#endif

private:
  FndbWord GetHeaderFlags() const
  {
    return fndbHeader->flags;
  }

private:
  FndbByteOffset GetByteOffset(const void* p) const
  {
    ptrdiff_t d = reinterpret_cast<const uint8_t*>(p) - reinterpret_cast<const uint8_t*>(fndbHeader);
    MIKTEX_ASSERT(d == 0 || d >= sizeof(FileNameDatabaseHeader) && d < foEnd);
    return d;
  }

private:
  const void* GetPointer(FndbByteOffset fo) const
  {
    MIKTEX_ASSERT(fo == 0 || fo >= sizeof(FileNameDatabaseHeader) && fo < foEnd);
    return fo == 0 ? nullptr : reinterpret_cast<const uint8_t*>(fndbHeader) + fo;
  }

private:
  const char* GetString(FndbByteOffset fo) const
  {
    MIKTEX_ASSERT(fo >= sizeof(FileNameDatabaseHeader) && fo < foEnd);
    MIKTEX_ASSERT_STRING(reinterpret_cast<const char*>(GetPointer(fo)));
    return reinterpret_cast<const char*>(GetPointer(fo));
  }

#if MIKTEX_FNDB_VERSION == 5
private:
  const FileNameDatabaseRecord* GetTable() const
  {
    return reinterpret_cast<const FileNameDatabaseRecord*>(GetPointer(fndbHeader->foTable));
  }
#endif

#if MIKTEX_FNDB_VERSION == 4
private:
  const FileNameDatabaseDirectory* GetTopDirectory() const
  {
    return GetDirectoryAt(fndbHeader->foTopDir);
  }
#endif

private:
  bool HasFileNameInfo() const
  {
    return (fndbHeader->flags& FileNameDatabaseHeader::FndbFlags::FileNameInfo) != 0;
  }

private:
  void Initialize(const MiKTeX::Core::PathName& fndbPath, const MiKTeX::Core::PathName& rootDirectory);

private:
  void ApplyChangeFile();

#if MIKTEX_FNDB_VERSION == 4
private:
  void MakePathName(const FileNameDatabaseDirectory* dir, MiKTeX::Core::PathName& path) const;
#endif

private:
  void OpenFileNameDatabase(const MiKTeX::Core::PathName& fndbPath);

  // size (in bytes) of the FNDB file
private:
  FndbByteOffset foEnd = 0;

private:
  std::unique_ptr<MiKTeX::Core::MemoryMappedFile> mmap;

  // pointer to the FNDB header
private:
  FileNameDatabaseHeader* fndbHeader = nullptr;

  // file-system path to root directory
private:
  MiKTeX::Core::PathName rootDirectory;

private:
  typedef std::unordered_multimap<std::string, Record> FileNameHashTable;

private:
  FileNameHashTable fileNames;

private:
  MiKTeX::Core::PathName changeFile;

private:
  std::size_t changeFileSize = 0;

private:
  int changeFileRecordCount = 0;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_fndb;
};

END_INTERNAL_NAMESPACE;

#endif
