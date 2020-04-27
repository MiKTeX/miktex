/* FileNameDatabase.cpp: file name database

   Copyright (C) 1996-2020 Christian Schenk

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

#include "config.h"

#if defined(MIKTEX_WINDOWS)
#include <io.h>
#endif

#if defined(MIKTEX_UNIX)
#include <unistd.h>
#endif

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/File>
#include <miktex/Core/FileStream>
#include <miktex/Core/LockFile>
#include <miktex/Core/PathNameParser>
#include <miktex/Core/Paths>
#include <miktex/Core/Utils>
#include <miktex/Trace/Trace>
#include <miktex/Util/PathNameUtil>
#include <miktex/Util/Tokenizer>

#include "internal.h"

#include "FileNameDatabase.h"
#include "Utils/CoreStopWatch.h"
#include "Utils/inliners.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

#define FNDB_DAMAGED_2(description, ...) \
  ThrowFndbDamaged(description, MiKTeXException::KVMAP(__VA_ARGS__), MIKTEX_SOURCE_LOCATION())

void MIKTEXNORETURN ThrowFndbDamaged(const string& description, const MiKTeXException::KVMAP& info, const SourceLocation& sourceLocation)
{
  Session::FatalMiKTeXError(T_("The file name database is damaged."), description, T_("Delete the file name database files. Then run 'initexmf -u' to recreate the FNDB."), "fndb-damaged", info, sourceLocation);
}

shared_ptr<FileNameDatabase> FileNameDatabase::Create(const PathName& fndbPath, const PathName& rootDirectory)
{
  shared_ptr<FileNameDatabase> fndb = make_shared<FileNameDatabase>();
  fndb->Initialize(fndbPath, rootDirectory);
  return fndb;
}

FileNameDatabase::FileNameDatabase() :
  mmap(MemoryMappedFile::Create()),
  trace_fndb(TraceStream::Open(MIKTEX_TRACE_FNDB))
{
}

FileNameDatabase::~FileNameDatabase()
{
  try
  {
    Finalize();
  }
  catch (const exception&)
  {
  }
}

// FIXME: not UTF-8 safe
MIKTEXSTATICFUNC(bool) Match(const char* pathPattern, const char* path)
{
  MIKTEX_ASSERT(PathName(pathPattern).IsComparable());
  MIKTEX_ASSERT(PathName(path).IsComparable());
  int lastch = 0;
  for (; *pathPattern != 0 && *path != 0; ++pathPattern, ++path)
  {
    if (*pathPattern == *path)
    {
      lastch = *path;
      continue;
    }
    MIKTEX_ASSERT(RECURSION_INDICATOR_LENGTH == 2);
    MIKTEX_ASSERT(PathNameUtil::IsDirectoryDelimiter(RECURSION_INDICATOR[0]));
    MIKTEX_ASSERT(PathNameUtil::IsDirectoryDelimiter(RECURSION_INDICATOR[1]));
    if (*pathPattern == RECURSION_INDICATOR[1] && PathNameUtil::IsDirectoryDelimiter(lastch))
    {
      for (; PathNameUtil::IsDirectoryDelimiter(*pathPattern); ++pathPattern)
      {
      };
      if (*pathPattern == 0)
      {
        return true;
      }
      for (; *path != 0; ++path)
      {
        if (PathNameUtil::IsDirectoryDelimiter(lastch))
        {
          // RECURSION
          if (Match(pathPattern, path))
          {
            return true;
          }
        }
        lastch = *path;
      }
    }
    return false;
  }
  return (*pathPattern == 0 || strcmp(pathPattern, RECURSION_INDICATOR) == 0 || strcmp(pathPattern, "/") == 0) && *path == 0;
}

bool FileNameDatabase::Search(const PathName& relativePath, const string& pathPattern_, bool all, vector<Fndb::Record>& result)
{
  string pathPattern = pathPattern_;

  ApplyChangeFile();

  trace_fndb->WriteLine("core", fmt::format(T_("fndb search: rootDirectory={0}, relativePath={1}, pathpattern={2}"), Q_(rootDirectory), Q_(relativePath), Q_(pathPattern)));

  MIKTEX_ASSERT(result.size() == 0);
  MIKTEX_ASSERT(!PathNameUtil::IsAbsolutePath(relativePath));
  MIKTEX_ASSERT(!IsExplicitlyRelativePath(relativePath.GetData()));

  PathName dir = relativePath.GetDirectoryName();
  PathName fileName = relativePath.GetFileName();

  PathName scratch1;

  if (!dir.Empty())
  {
    size_t l = dir.GetLength();
    if (dir.EndsWithDirectoryDelimiter())
    {
      dir[l - 1] = 0;
      --l;
    }
    scratch1 = pathPattern;
    scratch1 /= dir;
    pathPattern = scratch1.ToString();
  }

  // check to see whether we have this file name
  pair<FileNameHashTable::const_iterator, FileNameHashTable::const_iterator> range = fileNames.equal_range(MakeKey(fileName));
  if (range.first == range.second)
  {
    return false;
  }

  // path pattern must be relative to root directory
  if (PathName(pathPattern).IsAbsolute())
  {
    const char* lpsz = Utils::GetRelativizedPath(pathPattern.c_str(), rootDirectory.GetData());
    if (lpsz == nullptr)
    {
      MIKTEX_FATAL_ERROR_2(T_("Path pattern is not covered by file name database."), "pattern", pathPattern);
    }
    pathPattern = lpsz;
  }

  PathName comparablePathPattern(pathPattern);
  comparablePathPattern.TransformForComparison();

  for (FileNameHashTable::const_iterator it = range.first; it != range.second; ++it)
  {
    PathName relativeDirectory;
    relativeDirectory = it->second.GetDirectory();
    if (Match(comparablePathPattern.GetData(), PathName(relativeDirectory).TransformForComparison().GetData()))
    {
      PathName path;
      path = rootDirectory;
      path /= relativeDirectory;
      path /= fileName;
      trace_fndb->WriteLine("core", fmt::format(T_("found: {0} ({1})"), Q_(path), Q_(it->second.GetInfo())));
      result.push_back({ path, it->second.GetInfo() });
      if (!all)
      {
        break;
      }
    }
  }

  return !result.empty();
}

void FileNameDatabase::Add(const vector<Fndb::Record>& records)
{
  FileStream writer(OpenChangeFileExclusively());
  for (const auto& rec : records)
  {
    string fileName;
    string directory;
    std::tie(fileName, directory) = SplitPath(rec.path);
    if (InsertRecord(Record(fileName, directory, rec.fileNameInfo)))
    {
      string s = fmt::format("+{0}{1}{2}{1}{3}\n", fileName, char(PathNameUtil::PathNameDelimiter), directory, rec.fileNameInfo);
      fputs(s.c_str(), writer.GetFile());
      changeFileRecordCount++;
      changeFileSize += s.length();
    }
  }
  fflush(writer.GetFile());
#if 1
  // TODO: File::Sync API
#if defined(MIKTEX_WINDOWS)
  if (!FlushFileBuffers(reinterpret_cast<HANDLE>(_get_osfhandle(fileno(writer.GetFile())))))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("FlushFileBuffers");
  }
#else
  if (fsync(fileno(writer.GetFile())) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fsync");
  }
#endif
#endif
  File::Unlock(writer.GetFile());
  writer.Close();
}

void FileNameDatabase::Remove(const vector<PathName>& paths)
{
  FileStream writer(OpenChangeFileExclusively());
  for (const auto& path : paths)
  {
    string fileName;
    string directory;
    std::tie(fileName, directory) = SplitPath(path);
    EraseRecord(Record(fileName, directory, ""));
    string s = fmt::format("-{}{}{}\n", fileName, char(PathNameUtil::PathNameDelimiter), directory);
    fputs(s.c_str(), writer.GetFile());
    changeFileRecordCount++;
    changeFileSize += s.length();
  }
  fflush(writer.GetFile());
#if 1
  // TODO: File::Sync API
#if defined(MIKTEX_WINDOWS)
  if (!FlushFileBuffers(reinterpret_cast<HANDLE>(_get_osfhandle(fileno(writer.GetFile())))))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("FlushFileBuffers");
  }
#else
  if (fsync(fileno(writer.GetFile())) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fsync");
  }
#endif
#endif
  File::Unlock(writer.GetFile());
  writer.Close();
}

bool FileNameDatabase::FileExists(const PathName& path)
{
  ApplyChangeFile();
  string fileName;
  string directory;
  std::tie(fileName, directory) = SplitPath(path);
  pair<FileNameHashTable::const_iterator, FileNameHashTable::const_iterator> range = fileNames.equal_range(MakeKey(fileName));
  for (FileNameHashTable::const_iterator it = range.first; it != range.second; ++it)
  {
    if (PathName::Compare(it->second.GetDirectory(), directory) == 0)
    {
      return true;
    }
  }
  return false;
}

tuple<string, string> FileNameDatabase::SplitPath(const PathName& path_) const
{
  PathName path = path_;

  // make sure that the path is relative to the texmf root directory
  if (path.IsAbsolute())
  {
    const char* lpsz = Utils::GetRelativizedPath(path.GetData(), rootDirectory.GetData());
    if (lpsz == nullptr)
    {
      MIKTEX_FATAL_ERROR_2(T_("File name is not covered by file name database."), "path", path.ToString());
    }
    path = lpsz;
  }

  // get file name and directory
  PathName fileName = path;
  fileName.RemoveDirectorySpec();
  PathName directory = path;
  directory.RemoveFileSpec();
  directory = directory.ToUnix();

  return make_tuple(fileName.ToString(), directory.ToString());
}

void FileNameDatabase::FastInsertRecord(FileNameDatabase::Record&& record)
{
  fileNames.insert(pair<string, Record>(MakeKey(record.fileName), std::move(record)));
}

bool FileNameDatabase::InsertRecord(FileNameDatabase::Record&& record)
{
  string key = MakeKey(record.fileName);
  pair<FileNameHashTable::const_iterator, FileNameHashTable::const_iterator> range = fileNames.equal_range(key);
  for (FileNameHashTable::const_iterator it = range.first; it != range.second; ++it)
  {
    if (PathName::Compare(it->second.GetDirectory(), record.GetDirectory()) == 0)
    {
      return false;
    }
  }
  fileNames.insert(pair<string, Record>(std::move(key), std::move(record)));
  return true;
}

string FileNameDatabase::MakeKey(const string& fileName) const
{
  string key = fileName;
#if defined(MIKTEX_WINDOWS)
  for (char& ch : key)
  {
    ch = ToLower(ch);
  }
#endif
  return key;
}

string FileNameDatabase::MakeKey(const PathName& fileName) const
{
  return MakeKey(fileName.ToString());
}

void FileNameDatabase::EraseRecord(const FileNameDatabase::Record& record)
{
  pair<FileNameHashTable::const_iterator, FileNameHashTable::const_iterator> range = fileNames.equal_range(MakeKey(record.fileName));
  if (range.first == range.second)
  {
    FNDB_DAMAGED_2(T_("The file name record could not be found in the database."), "fileName", record.fileName);
  }
  vector<FileNameHashTable::const_iterator> toBeRemoved;
  for (FileNameHashTable::const_iterator it = range.first; it != range.second; ++it)
  {
    if (PathName::Compare(it->second.GetDirectory(), record.GetDirectory()) == 0)
    {
      toBeRemoved.push_back(it);
    }
  }
  if (toBeRemoved.empty())
  {
    FNDB_DAMAGED_2(T_("The file name record could not be found in the database."), "fileName", record.fileName, "directory", record.GetDirectory());
  }
  for (const auto& it : toBeRemoved)
  {
    fileNames.erase(it);
  }
}

void FileNameDatabase::ReadFileNames()
{
  fileNames.clear();
  fileNames.rehash(fndbHeader->numFiles);
  CoreStopWatch stopWatch(fmt::format("fndb read file names {}", Q_(rootDirectory)));
  ReadFileNames(GetTable());
}

void FileNameDatabase::ReadFileNames(const FileNameDatabaseRecord* table)
{
  for (size_t idx = 0; idx < fndbHeader->numFiles; ++idx)
  {
    const FileNameDatabaseRecord* rec = &table[idx];
    FastInsertRecord(Record(this, GetString(rec->foFileName), rec->foDirectory, rec->foInfo));
  }
}

void FileNameDatabase::Finalize()
{
  if (trace_fndb != nullptr)
  {
    trace_fndb->WriteLine("core", fmt::format(T_("unloading fndb {0}"), Q_(this->rootDirectory)));
  }
  CloseFileNameDatabase();
  if (trace_fndb != nullptr)
  {
    trace_fndb->Close();
    trace_fndb = nullptr;
  }
}

void FileNameDatabase::Initialize(const PathName& fndbPath, const PathName& rootDirectory)
{
  this->rootDirectory = rootDirectory;

  OpenFileNameDatabase(fndbPath);
  ReadFileNames();

  changeFile = fndbPath;
  changeFile.SetExtension(MIKTEX_FNDB_CHANGE_FILE_SUFFIX);
  
  ApplyChangeFile();
}

void FileNameDatabase::ApplyChangeFile()
{
  lastAccessTime = chrono::high_resolution_clock::now();
  if (!File::Exists(changeFile))
  {
    return;
  }
  size_t newChangeFileSize = File::GetSize(changeFile);
  if (newChangeFileSize == changeFileSize)
  {
    return;
  }
  MIKTEX_ASSERT(newChangeFileSize > changeFileSize);
  CoreStopWatch stopWatch(fmt::format(T_("applying FNDB change file {0} starting at record #{1}"), Q_(changeFile), changeFileRecordCount));
  FileStream reader(File::Open(changeFile, FileMode::Open, FileAccess::Read, false));
  if (!File::TryLock(reader.GetFile(), File::LockType::Shared, 2s))
  {
    MIKTEX_FATAL_ERROR_2(T_("Could not acquire shared lock."), "path", changeFile.ToString());
  }
  if (changeFileSize > 0)
  {
    reader.Seek(changeFileSize, SeekOrigin::Begin);
  }
  for (string line; Utils::ReadLine(line, reader.GetFile(), false); )
  {
    if (line.empty())
    {
      FNDB_DAMAGED_2(T_("FNDB change file has been tampered with."), "path", changeFile.ToString());
    }
    changeFileRecordCount++;
    changeFileSize += line.length() + sizeof('\n');
    string op = line.substr(0, 1);
    vector<string> data = StringUtil::Split(line.substr(1), PathNameUtil::PathNameDelimiter);
    if (data.size() < 2)
    {
      FNDB_DAMAGED_2(T_("FNDB change file has been tampered with."), "path", changeFile.ToString());
    }
    string& fileName = data[0];
    string& directory = data[1];
    if (op == "+")
    {
      if (data.size() < 3)
      {
        FNDB_DAMAGED_2(T_("FNDB change file has been tampered with."), "path", changeFile.ToString());
      }
      string& fileNameInfo = data[2];
      FastInsertRecord(Record(std::move(fileName), std::move(directory), std::move(fileNameInfo)));
    }
    else if (op == "-")
    {
      EraseRecord(Record(std::move(fileName), std::move(directory), ""));
    }
    else
    {
      FNDB_DAMAGED_2(T_("FNDB change file has been tampered with."), "path", changeFile.ToString());
    }
  }
  File::Unlock(reader.GetFile());
  reader.Close();
}

FILE* FileNameDatabase::OpenChangeFileExclusively()
{
  ApplyChangeFile();
  FileStream writer(File::Open(changeFile, FileMode::Append, FileAccess::Write, false));
  if (!File::TryLock(writer.GetFile(), File::LockType::Exclusive, 2s))
  {
    MIKTEX_FATAL_ERROR_2(T_("Could not acquire exclusive lock."), "path", changeFile.ToString());
  }
  return writer.Detach();
}

void FileNameDatabase::OpenFileNameDatabase(const PathName& fndbPath)
{
  mmap->Open(fndbPath, false);

  if (mmap->GetSize() < sizeof(*fndbHeader))
  {
    FNDB_DAMAGED_2(T_("Not a file name database file (wrong size)."), "path", fndbPath.ToString());
  }

  fndbHeader = reinterpret_cast<FileNameDatabaseHeader*>(mmap->GetPtr());

  foEnd = static_cast<FndbByteOffset>(mmap->GetSize());

  // check signature
  if (fndbHeader->signature != FileNameDatabaseHeader::Signature)
  {
    FNDB_DAMAGED_2(T_("Not a file name database file (wrong signature)."), "path", fndbPath.ToString());
  }

  // check version number
  if (fndbHeader->version != FileNameDatabaseHeader::Version)
  {
    FNDB_DAMAGED_2(T_("Unknown file name database file version."), "path", fndbPath.ToString(), "versionFound", std::to_string(fndbHeader->Version), "versionExpected", std::to_string(FileNameDatabaseHeader::Version));
  }
}

void FileNameDatabase::CloseFileNameDatabase()
{
  if (mmap != nullptr)
  {
    if (mmap->GetPtr() != nullptr)
    {
      mmap->Close();
    }
    mmap = nullptr;
  }
}
