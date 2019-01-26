/* FileNameDatabase.h: file name database                 -*- C++ -*-

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

#if !defined(BA15DC038D4549859111D4B075360D81)
#define BA15DC038D4549859111D4B075360D81

#include <chrono>
#include <tuple>

#include <miktex/Core/Debug>
#include <miktex/Core/DirectoryLister>
#include <miktex/Core/Fndb>
#include <miktex/Core/MemoryMappedFile>
#include <miktex/Core/PathName>

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

public:
  std::chrono::time_point<std::chrono::high_resolution_clock> GetLastAccessTime() const
  {
    return lastAccessTime;
  }

private:
  struct Record
  {
  public:
    Record(const FileNameDatabase* fndb, std::string&& fileName, FndbByteOffset foDirectory, FndbByteOffset foInfo) :
      fndb(fndb),
      fileName(std::move(fileName)),
      foDirectory(foDirectory),
      foInfo(foInfo)
    {
    }
  public:
    Record(const std::string& fileName, const std::string& directory, const std::string& info) :
      fileName(fileName),
      directory(directory),
      info(info)
    {
    }
  public:
    Record(std::string&& fileName, std::string&& directory, std::string&& info) :
      fileName(std::move(fileName)),
      directory(std::move(directory)),
      info(std::move(info))
    {
    }
  public:
    std::string GetDirectory() const
    {
      if (foDirectory != 0)
      {
        MIKTEX_ASSERT(fndb != nullptr);
        return fndb->GetString(foDirectory);
      }
      else
      {
        return directory;
      }
    }
  public:
    std::string GetInfo() const
    {
      if (foInfo != 0)
      {
        MIKTEX_ASSERT(fndb != nullptr);
        return fndb->GetString(foInfo);
      }
      else
      {
        return info;
      }
    }
  private:
    const FileNameDatabase* fndb = nullptr;
  public:
    std::string fileName;
  private:
    FndbByteOffset foDirectory = 0;
  private:
    std::string directory;
  private:
    FndbByteOffset foInfo = 0;
  private:
    std::string info;
  };

private:
  std::tuple<std::string, std::string> SplitPath(const MiKTeX::Core::PathName& path) const;

private:
  std::string MakeKey(const std::string& fileName) const;

private:
  std::string MakeKey(const MiKTeX::Core::PathName& fileName) const;

private:
  void FastInsertRecord(Record&& record);

private:
  bool InsertRecord(Record&& record);

private:
  void EraseRecord(const Record& record);
  
private:
  void ReadFileNames();

private:
  void ReadFileNames(const FileNameDatabaseRecord* table);
private:
  void Finalize();

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
    return static_cast<FndbByteOffset>(d);
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

private:
  const FileNameDatabaseRecord* GetTable() const
  {
    return reinterpret_cast<const FileNameDatabaseRecord*>(GetPointer(fndbHeader->foTable));
  }

private:
  void Initialize(const MiKTeX::Core::PathName& fndbPath, const MiKTeX::Core::PathName& rootDirectory);

private:
  void ApplyChangeFile();

private:
  FILE* OpenChangeFileExclusively();

private:
  void OpenFileNameDatabase(const MiKTeX::Core::PathName& fndbPath);

private:
  void CloseFileNameDatabase();

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
  std::chrono::time_point<std::chrono::high_resolution_clock> lastAccessTime = std::chrono::high_resolution_clock::now();

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_fndb;
};

END_INTERNAL_NAMESPACE;

#endif
