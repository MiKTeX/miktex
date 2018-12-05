/* FileNameDatabase.cpp: file name database

   Copyright (C) 1996-2018 Christian Schenk

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

#if defined(HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Trace/Trace>

#include "internal.h"

#include "miktex/Core/PathNameParser.h"

#include "FileNameDatabase.h"
#include "Utils/CoreStopWatch.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace std;

FileNameDatabase::FileNameDatabase() :
  mmap(MemoryMappedFile::Create()),
  traceStream(TraceStream::Open(MIKTEX_TRACE_FNDB))
{
}

void FileNameDatabase::Finalize()
{
  if (traceStream != nullptr)
  {
    traceStream->WriteFormattedLine("core", T_("unloading fndb %p"), this);
  }
  if (mmap != nullptr)
  {
    if (mmap->GetPtr() != nullptr)
    {
      mmap->Close();
    }
    mmap = nullptr;
  }
  if (traceStream != nullptr)
  {
    traceStream->Close();
    traceStream = nullptr;
  }
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

void FileNameDatabase::OpenFileNameDatabase(const PathName& fndbPath, bool readWrite)
{
#if defined(MIKTEX_WINDOWS)
  // check file attributes
  FileAttributeSet attributes = File::GetAttributes(fndbPath);
  if (attributes[FileAttribute::ReadOnly])
  {
    traceStream->WriteFormattedLine("core", T_("file name database file is readonly"));
    readWrite = false;
  }
#endif

  mmap->Open(fndbPath, readWrite);

  if (mmap->GetSize() < sizeof(*pHeader))
  {
    MIKTEX_FATAL_ERROR_2(T_("Not a file name database file (wrong size)."), "path", fndbPath.ToString());
  }

  pHeader = reinterpret_cast<FileNameDatabaseHeader*>(mmap->GetPtr());

  foEnd = static_cast<FndbByteOffset>(mmap->GetSize());

  // check signature
  if (pHeader->signature != FileNameDatabaseHeader::Signature)
  {
    MIKTEX_FATAL_ERROR_2(T_("Not a file name database file (wrong signature)."), "path", fndbPath.ToString());
  }

  // check version number
  if (pHeader->version != FileNameDatabaseHeader::Version)
  {
    MIKTEX_FATAL_ERROR_2(T_("Unknown file name database file version."), "path", fndbPath.ToString(), "versionFound", std::to_string(pHeader->Version), "versionExpected", std::to_string(FileNameDatabaseHeader::Version));
  }

  if (!readWrite || (pHeader->flags & FileNameDatabaseHeader::FndbFlags::Frozen) != 0)
  {
    isInvariable = true;
    readWrite = false;
  }

  if (!isInvariable)
  {
    // grow file-mapping object if necessary
    if (pHeader->size + 131072 > foEnd)
    {
      size_t newSize = ((pHeader->size + FNDB_EXTRA + 1) / FNDB_GRAN) * FNDB_GRAN;
      traceStream->WriteFormattedLine("core", T_("enlarging fndb file %s (%u -> %u)..."), Q_(fndbPath), static_cast<unsigned>(foEnd), static_cast<unsigned>(newSize));
      pHeader = reinterpret_cast<FileNameDatabaseHeader*>(mmap->Resize(newSize));
#if defined(MIKTEX_WINDOWS) && REPORT_EVENTS
      ReportMiKTeXEvent(EVENTLOG_INFORMATION_TYPE, MIKTEX_EVENT_FNDB_ENLARGED, mmap->GetName(), std::to_string(foEnd), std::to_string(static_cast<unsigned>(newSize)), 0);
#endif
      foEnd = static_cast<FndbByteOffset>(newSize);
    }
  }

  timeStamp = pHeader->timeStamp;
}

void FileNameDatabase::Initialize(const PathName& fndbPath, const PathName& rootDirectory, bool readWrite)
{
  this->rootDirectory = rootDirectory;
  isInvariable = !readWrite;

  OpenFileNameDatabase(fndbPath, readWrite);

  if ((pHeader->flags & FileNameDatabaseHeader::FndbFlags::Frozen) != 0)
  {
    isInvariable = true;
  }

  ReadFileNames();
}

FileNameDatabaseDirectory* FileNameDatabase::FindSubDirectory(const FileNameDatabaseDirectory* dir, const PathName& relPath) const
{
  MIKTEX_ASSERT(dir != nullptr);
  FndbByteOffset fo = 0;
  for (PathNameParser dirName(relPath); dirName; ++dirName)
  {
    if (PathName::Compare(*dirName, CURRENT_DIRECTORY) == 0)
    {
      fo = GetByteOffset(dir);
      continue;
    }
    bool matching = false;
    FndbWord subidx = 0;
    while (!matching && dir != nullptr)
    {
      FndbWord lo = 0;
      FndbWord hi = dir->numSubDirs;
      while (!matching && hi > lo)
      {
        subidx = lo + (hi - lo) / 2;
        FndbByteOffset foSubDirName = dir->GetSubDirName(subidx);
        const char* lpszSubdirName = GetString(foSubDirName);
        int cmp = PathName::Compare(lpszSubdirName, *dirName);
        if (cmp > 0)
        {
          hi = subidx;
        }
        else if (cmp < 0)
        {
          lo = subidx + 1;
        }
        else
        {
          matching = true;
        }
      }
      if (!matching)
      {
        dir = GetDirectoryAt(dir->foExtension);
      }
    }
    if (!matching)
    {
      fo = 0;
      break;
    }
    fo = dir->GetSubDir(subidx);
    dir = GetDirectoryAt(fo);
  }
  return GetDirectoryAt(fo);
}

FileNameDatabaseDirectory* FileNameDatabase::SearchFileName(FileNameDatabaseDirectory* dir, const PathName& fileName, FndbWord& index) const
{
  for (; dir != nullptr; dir = GetDirectoryAt(dir->foExtension))
  {
    FndbWord lo = 0;
    FndbWord hi = dir->numFiles;
    while (hi > lo)
    {
      index = lo + (hi - lo) / 2;
      const char* lpszCandidate = GetString(dir->GetFileName(index));
      int cmp = PathName::Compare(lpszCandidate, fileName);
      if (cmp > 0)
      {
        hi = index;
      }
      else if (cmp < 0)
      {
        lo = index + 1;
      }
      else
      {
        return dir;
      }
    }
  }
  return nullptr;
}

void FileNameDatabase::MakePathName(const FileNameDatabaseDirectory* dir, PathName& path) const
{
  if (dir == nullptr || dir->foParent == 0)
  {
    return;
  }
  // RECURSION
  MakePathName(GetDirectoryAt(dir->foParent), path);
  path /= GetString(dir->foName);
}

#define ROUND2(n, pow2) ((n + pow2 - 1) & ~(pow2 - 1))

FileNameDatabaseDirectory* FileNameDatabase::ExtendDirectory(FileNameDatabaseDirectory* dir) const
{
  FndbWord neededSlots = 10;         // make room for 10 files
  if (HasFileNameInfo())
  {
    neededSlots *= 2;
  }
  neededSlots += 2;             // and 1 sub-directory
  FndbWord neededBytes = offsetof(FileNameDatabaseDirectory, table);
  neededBytes += neededSlots * sizeof(FndbByteOffset);
  FndbByteOffset foExtension = ROUND2(pHeader->size, 16);
  if (foExtension + neededBytes > foEnd)
  {
    MIKTEX_UNEXPECTED();
  }
  pHeader->size = foExtension + neededBytes;
  FileNameDatabaseDirectory* extDir = reinterpret_cast<FileNameDatabaseDirectory*>(GetPointer(foExtension));
  extDir->Init();
  extDir->foName = dir->foName;
  extDir->foParent = dir->foParent;
  extDir->numFiles = 0;
  extDir->numSubDirs = 0;
  extDir->foExtension = 0;
  extDir->capacity = neededSlots;
  memset(extDir->table, 0, sizeof(FndbByteOffset) * neededSlots);
  dir->foExtension = foExtension;
  return extDir;
}

FndbByteOffset FileNameDatabase::CreateString(const string& name) const
{
  FndbByteOffset foName;
  size_t neededBytes = name.length() + 1;
  foName = ROUND2(pHeader->size, 2);
  if (foName + neededBytes > foEnd)
  {
    MIKTEX_UNEXPECTED();
  }
  memcpy(GetPointer(foName), name.c_str(), neededBytes);
  pHeader->size = foName + static_cast<FndbWord>(neededBytes);
  return foName;
}

FndbWord FileNameDatabase::FindLowerBound(const FndbByteOffset& begin, FndbWord count, const char* name, bool& isDuplicate) const
{
  const FndbByteOffset* pBegin = &begin;
  const FndbByteOffset* pEnd = &pBegin[count];
  const FndbByteOffset* iter = pBegin;
  while (iter != pEnd && PathName::Compare(name, GetString(*iter)) > 0)
  {
    ++iter;
  }
  isDuplicate = iter != pEnd && PathName::Compare(name, GetString(*iter)) == 0;
  return static_cast<FndbWord>(iter - pBegin);
}

void FileNameDatabase::InsertFileName(FileNameDatabaseDirectory* dir, FndbByteOffset foFileName, FndbByteOffset foFileNameInfo) const
{
  MIKTEX_ASSERT(dir->capacity >= (dir->SizeOfTable(HasFileNameInfo()) + (HasFileNameInfo() ? 2 : 1)));
  bool isDuplicate;
  FndbWord idx = FindLowerBound(dir->table[0], dir->numFiles, GetString(foFileName), isDuplicate);
  if (isDuplicate)
  {
    return;
  }
  dir->TableInsert(idx, foFileName);
  if (HasFileNameInfo())
  {
    dir->TableInsert(dir->numFiles + 1 + 2 * dir->numSubDirs + idx, foFileNameInfo);
  }
  dir->numFiles += 1;
  pHeader->numFiles += 1;
  pHeader->timeStamp = static_cast<FndbWord>(time(nullptr)); // <sixtyfourbit/>
}

void FileNameDatabase::InsertDirectory(FileNameDatabaseDirectory* dir, const FileNameDatabaseDirectory* subDir) const
{
  MIKTEX_ASSERT(dir->capacity >= dir->SizeOfTable(HasFileNameInfo()) + 2);
  bool isDuplicate;
  FndbWord idx = FindLowerBound(dir->table[dir->numFiles], dir->numSubDirs, GetString(subDir->foName), isDuplicate);
  if (isDuplicate)
  {
    return;
  }
  dir->TableInsert(dir->numFiles + idx, subDir->foName);
  dir->TableInsert(dir->numFiles + dir->numSubDirs + 1 + idx, GetByteOffset(subDir));
  dir->numSubDirs += 1;
  pHeader->numDirs += 1;
}

FileNameDatabaseDirectory* FileNameDatabase::CreateFndbDirectory(FileNameDatabaseDirectory* dir, const string& name) const
{
  while (dir->capacity < dir->SizeOfTable(HasFileNameInfo()) + 2)
  {
    if (dir->foExtension != 0)
    {
      dir = GetDirectoryAt(dir->foExtension);
    }
    else
    {
      dir = ExtendDirectory(dir);
    }
    MIKTEX_ASSERT(dir != nullptr);
  }

  FndbByteOffset foName = CreateString(name);

  MIKTEX_ASSERT(foName != 0);

  FndbWord neededSlots = 10;         // make room for 10 files

  if (HasFileNameInfo())
  {
    neededSlots *= 2;
  }

  neededSlots += 2;             // and 1 sub-directory

  FndbWord neededBytes = offsetof(FileNameDatabaseDirectory, table);
  neededBytes += neededSlots * sizeof(FndbByteOffset);
  if (pHeader->size + neededBytes > foEnd)
  {
    MIKTEX_UNEXPECTED();
  }
  FndbByteOffset foSub = pHeader->size;
  pHeader->size += neededBytes;
  FileNameDatabaseDirectory* subDir = reinterpret_cast<FileNameDatabaseDirectory*>(GetPointer(foSub));
  subDir->Init();
  subDir->foName = foName;
  subDir->foParent = GetByteOffset(dir);
  subDir->numFiles = 0;
  subDir->numSubDirs = 0;
  subDir->foExtension = 0;
  subDir->capacity = neededSlots;
  memset(subDir->table, 0, sizeof(FndbByteOffset) * neededSlots);
  InsertDirectory(dir, subDir);

  return subDir;
}

FileNameDatabaseDirectory* FileNameDatabase::CreateDirectoryPath(FileNameDatabaseDirectory* dir, const PathName& relPath) const
{
  bool create = false;
  FndbWord level = 0;
  for (PathNameParser dirName(relPath); dirName; ++dirName)
  {
    FileNameDatabaseDirectory* subDir = nullptr;
    if (!create)
    {
      subDir = FindSubDirectory(dir, *dirName);
      if (subDir == nullptr)
      {
        create = true;
      }
    }
    if (create)
    {
      subDir = CreateFndbDirectory(dir, (*dirName));
      if (subDir == nullptr)
      {
        MIKTEX_UNEXPECTED();
      }
    }
    dir = subDir;
    ++level;
  }
  if (level > pHeader->depth)
  {
    MIKTEX_ASSERT(create);
    pHeader->depth = level;
  }
  return dir;
}

FileNameDatabaseDirectory* FileNameDatabase::RemoveFileName(FileNameDatabaseDirectory* dir, const PathName& fileName) const
{
  FndbWord index;

  dir = SearchFileName(dir, fileName, index);

  if (dir == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }

  dir->TableRemove(index);

  if (HasFileNameInfo())
  {
    dir->TableRemove(dir->numFiles - 1 + 2 * dir->numSubDirs + index);
  }

  dir->numFiles -= 1;
  pHeader->numFiles -= 1;
  pHeader->timeStamp = static_cast<FndbWord>(time(nullptr)); // FIXME: 64-bit

  return dir;
}

void FileNameDatabase::Flush() const
{
  traceStream->WriteFormattedLine("core", T_("flushing file name database"));
  mmap->Flush();
}

// FIXME: not UTF-8 safe
MIKTEXSTATICFUNC(bool) Match(const char* pathPattern, const char* path)
{
  MIKTEX_ASSERT(PathName(pathPattern).IsComparable());
  MIKTEX_ASSERT(PathName(lpszPath).IsComparable());
  int lastch = 0;
  for (; *pathPattern != 0 && *path != 0; ++pathPattern, ++path)
  {
    if (*pathPattern == *path)
    {
      lastch = *path;
      continue;
    }
    MIKTEX_ASSERT(RECURSION_INDICATOR_LENGTH == 2);
    MIKTEX_ASSERT(IsDirectoryDelimiter(RECURSION_INDICATOR[0]));
    MIKTEX_ASSERT(IsDirectoryDelimiter(RECURSION_INDICATOR[1]));
    if (*pathPattern == RECURSION_INDICATOR[1] && IsDirectoryDelimiter(lastch))
    {
      for (; IsDirectoryDelimiter(*pathPattern); ++pathPattern)
      {
      };
      if (*pathPattern == 0)
      {
        return true;
      }
      for (; *path != 0; ++path)
      {
        if (IsDirectoryDelimiter(lastch))
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

bool FileNameDatabase::Search(const PathName& relativePath, const string& pathPattern_, bool firstMatchOnly, vector<PathName>& result, vector<string>& fileNameInfo) const
{
  string pathPattern = pathPattern_;

  traceStream->WriteFormattedLine("core", T_("fndb search: rootDirectory=%s, relativePath=%s, pathpattern=%s"), Q_(rootDirectory), Q_(relativePath), Q_(pathPattern));

  MIKTEX_ASSERT(result.size() == 0);
  MIKTEX_ASSERT(fileNameInfo.size() == 0);
  MIKTEX_ASSERT(!Utils::IsAbsolutePath(relativePath));
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

  PathName comparableFileName = fileName;
  comparableFileName.TransformForComparison();

  // check to see whether we have this file name
  pair<FileNameHashTable::const_iterator, FileNameHashTable::const_iterator> range = fileNames.equal_range(comparableFileName.ToString());

  if (range.first == range.second)
  {
    return false;
  }

  // path pattern must be relative to root directory
  if (Utils::IsAbsolutePath(pathPattern))
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
    MakePathName(it->second, relativeDirectory);
    if (Match(comparablePathPattern.GetData(), PathName(relativeDirectory).TransformForComparison().GetData()))
    {
      PathName path;
      path = rootDirectory;
      path /= relativeDirectory;
      path /= fileName;
      result.push_back(path);
      if (HasFileNameInfo())
      {
        FndbWord idx;
        const FileNameDatabaseDirectory* dir = SearchFileName(it->second, fileName, idx);
        if (dir == nullptr)
        {
          MIKTEX_UNEXPECTED();
        }
        fileNameInfo.push_back(GetString(dir->GetFileNameInfo(idx)));
        traceStream->WriteFormattedLine("core", T_("found: %s (%s)"), Q_(path), GetString(dir->GetFileNameInfo(idx)));
      }
      else
      {
        fileNameInfo.push_back("");
        traceStream->WriteFormattedLine("core", T_("found: %s"), Q_(path));
      }
      if (firstMatchOnly)
      {
        break;
      }
    }
  }

  return !result.empty();
}

shared_ptr<FileNameDatabase> FileNameDatabase::Create(const PathName& fndbPath, const PathName& rootDirectory, bool readOnly)
{
  shared_ptr<FileNameDatabase> fndb = make_shared<FileNameDatabase>();
  fndb->Initialize(fndbPath, rootDirectory, !readOnly);
  return fndb;
}

void FileNameDatabase::AddFile(const PathName& path_, const string& fileNameInfo)
{
  PathName path = path_;

  traceStream->WriteFormattedLine("core", T_("adding %s to the file name database"), Q_(path));

  // make sure we can add files
  if (IsInvariable())
  {
    MIKTEX_UNEXPECTED();
  }

  // make sure that the path is relative to the texmf root directory
  if (Utils::IsAbsolutePath(path))
  {
    const char* lpsz = Utils::GetRelativizedPath(path.GetData(), rootDirectory.GetData());
    if (lpsz == nullptr)
    {
      MIKTEX_FATAL_ERROR_2(T_("File name is not covered by file name database."), "path", path.ToString());
    }
    path = lpsz;
  }

  // make a working copy of the path; separate file name from directory name
  PathName pathDirectory(path);
  pathDirectory.RemoveFileSpec();
  PathName pathFile(path);
  pathFile.RemoveDirectorySpec();

  // get (possibly create) the parent directory
  FileNameDatabaseDirectory* dir;
  if (pathDirectory.GetLength() > 0)
  {
    dir = CreateDirectoryPath(GetTopDirectory(), pathDirectory);
  }
  else
  {
    // no sub-directory, i.e. create file in top directory
    dir = GetTopDirectory();
  }

  if (dir == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }

  // extend the directory, if necessary
  while (dir->capacity < (dir->SizeOfTable(HasFileNameInfo()) + (HasFileNameInfo() ? 2 : 1)))
  {
    if (dir->foExtension != 0)
    {
      // get next extension
      dir = GetDirectoryAt(dir->foExtension);
    }
    else
    {
      // create an extension
      dir = ExtendDirectory(dir);
      if (dir == nullptr)
      {
        MIKTEX_UNEXPECTED();
      }
    }
  }

  // create a new table entry
  InsertFileName(dir, CreateString(pathFile.GetData()), CreateString(fileNameInfo));

  // add the name to the hash table
  PathName comparableFileName(pathFile);
  comparableFileName.TransformForComparison();
  fileNames.insert(pair<string, FileNameDatabaseDirectory*>(comparableFileName.ToString(), dir));
}

bool FileNameDatabase::Enumerate(const PathName& fndbPath_, IEnumerateFndbCallback* callback) const
{
  PathName fndbPath = fndbPath_;

  if (!fndbPath.Empty() && Utils::IsAbsolutePath(fndbPath))
  {
    if (PathName::Compare(fndbPath, rootDirectory) == 0)
    {
      fndbPath = "";
    }
    else
    {
      const char* lpsz = Utils::GetRelativizedPath(fndbPath.GetData(), rootDirectory.GetData());
      if (lpsz == nullptr)
      {
        MIKTEX_FATAL_ERROR_2(T_("Path is not covered by file name database."), "path", fndbPath.ToString());
      }
      fndbPath = lpsz;
    }
  }

  const FileNameDatabaseDirectory* dir = fndbPath.Empty() ? GetTopDirectory() : FindSubDirectory(GetTopDirectory(), fndbPath);

  if (dir == nullptr)
  {
    MIKTEX_FATAL_ERROR_2(T_("Directory not found in file name database."), "path", fndbPath.ToString());
  }

  PathName path(rootDirectory, fndbPath);

  for (const FileNameDatabaseDirectory* dirIt = dir; dirIt != nullptr; dirIt = GetDirectoryAt(dirIt->foExtension))
  {
    for (FndbWord i = 0; i < dirIt->numSubDirs; ++i)
    {
      if (!callback->OnFndbItem(path, GetString(dirIt->GetSubDirName(i)), "", true))
      {
        return false;
      }
    }
  }

  for (const FileNameDatabaseDirectory* dirIt = dir; dirIt != nullptr; dirIt = GetDirectoryAt(dirIt->foExtension))
  {
    for (FndbWord i = 0; i < dirIt->numFiles; ++i)
    {
      const char* fileNameInfo = nullptr;
      if (HasFileNameInfo())
      {
        FndbByteOffset fo = dirIt->GetFileNameInfo(i);
        if (fo != 0)
        {
          fileNameInfo = GetString(fo);
        }
      }
      if (!callback->OnFndbItem(path, GetString(dirIt->GetFileName(i)), fileNameInfo == nullptr ? "" : fileNameInfo, false))
      {
        return false;
      }
    }
  }

  return true;
}

FileNameDatabaseDirectory* FileNameDatabase::TryGetParent(const PathName& path_) const
{
  PathName path = path_;

  // make sure that the path is relative to the texmf root directory
  if (Utils::IsAbsolutePath(path))
  {
    const char* lpsz = Utils::GetRelativizedPath(path.GetData(), rootDirectory.GetData());
    if (lpsz == nullptr)
    {
      MIKTEX_FATAL_ERROR_2(T_("The path name is not covered by the file name database."), "path", path.ToString());
    }
    path = lpsz;
  }

  // make a working copy; separate file name from directory name
  PathName pathDirectory(path);
  pathDirectory.RemoveFileSpec();

  // get the parent directory
  FileNameDatabaseDirectory* dir;
  if (pathDirectory.GetLength() > 0)
  {
    dir = FindSubDirectory(GetTopDirectory(), pathDirectory);
  }
  else
  {
    dir = GetTopDirectory();
  }

  return dir;
}

void FileNameDatabase::RemoveFile(const MiKTeX::Core::PathName& path)
{
  traceStream->WriteFormattedLine ("core", T_("removing %s from the file name database"), Q_(path));

  if (IsInvariable())
  {
    MIKTEX_UNEXPECTED();
  }

  PathName pathFile(path);
  pathFile.RemoveDirectorySpec();

  FileNameDatabaseDirectory* dir = TryGetParent(path);

  if (dir == nullptr)
  {
    MIKTEX_FATAL_ERROR_2(T_("The path could not be found in the file name database."), "path", path.ToString());
  }

  // remove the file name
  dir = RemoveFileName(dir, pathFile);

  // also from the hash table
  PathName comparableFileName(pathFile);
  comparableFileName.TransformForComparison();
  pair<FileNameHashTable::const_iterator, FileNameHashTable::const_iterator> range = fileNames.equal_range(comparableFileName.ToString());
  if (range.first == range.second)
  {
    MIKTEX_FATAL_ERROR_2(T_("The file name could not be found in the hash table."), "path", path.ToString());
  }
  bool removed = false;
  for (FileNameHashTable::const_iterator it = range.first; it != range.second; ++it)
  {
    if (it->second == dir)
    {
      fileNames.erase(it);
      removed = true;
      break;
    }
  }
  if (!removed)
  {
    MIKTEX_FATAL_ERROR_2(T_("The file name could not be removed from the hash table."), "path", path.ToString());
  }
}

bool FileNameDatabase::FileExists(const PathName& path) const
{
  FileNameDatabaseDirectory* dir = TryGetParent(path);
  if (dir != nullptr)
  {
    PathName fileName(path);
    fileName.RemoveDirectorySpec();
    FndbWord index;
    dir = SearchFileName(dir, fileName, index);
  }
  return dir != nullptr;
}

void FileNameDatabase::ReadFileNames()
{
  fileNames.clear();
  fileNames.rehash(pHeader->numFiles);
  CoreStopWatch stopWatch(fmt::format("fndb read file names {}", Q_(rootDirectory)));
  ReadFileNames(GetTopDirectory());
}

void FileNameDatabase::ReadFileNames(FileNameDatabaseDirectory* dir)
{
  for (FileNameDatabaseDirectory* dirIt = dir; dirIt != nullptr; dirIt = GetDirectoryAt(dirIt->foExtension))
  {
    for (FndbWord i = 0; i < dirIt->numSubDirs; ++i)
    {
      // RECURSION
      ReadFileNames(GetDirectoryAt(dirIt->GetSubDir(i)));
    }
    for (FndbWord i = 0; i < dirIt->numFiles; ++i)
    {
      fileNames.insert(pair<string, FileNameDatabaseDirectory*>(PathName(GetString(dirIt->GetFileName(i))).TransformForComparison().ToString(), dirIt));
    }
  }
}

#if 0 // experimental

class FndbDirectoryLister : public DirectoryLister
{
public:
  void MIKTEXTHISCALL Close() override;

public:
  bool MIKTEXTHISCALL GetNext(DirectoryEntry& direntry) override;

public:
  bool MIKTEXTHISCALL GetNext(DirectoryEntry2& direntry2) override;

public:
  FndbDirectoryLister(const PathName& directory);

public:
  MIKTEXTHISCALL ~FndbDirectoryLister() override;

private:
  PathName directory;

private:
  string pattern;

private:
  friend class DirectoryLister;
};

unique_ptr<DirectoryLister> FileNameDatabase::OpenDirectory(const char* lpszPath)
{
  MIKTEX_ASSERT_STRING_OR_NIL(lpszPath);

  if (lpszPath != nullptr && Utils::IsAbsolutePath(lpszPath))
  {
    if (PathName::Compare(lpszPath, rootDirectory.Get()) == 0)
    {
      lpszPath = nullptr;
    }
    else
    {
      const char* lpsz = Utils::GetRelativizedPath(lpszPath, rootDirectory.Get());
      if (lpsz == nullptr)
      {
        MIKTEX_FATAL_ERROR_2(T_("The path is not covered by file name database."), "path", lpszPath);
      }
      lpszPath = lpsz;
    }
  }

  const FileNameDatabaseDirectory* dir = lpszPath == nullptr || *lpszPath == 0 ? GetTopDirectory() : FindSubDirectory(GetTopDirectory(), lpszPath);

  if (dir == nullptr)
  {
    MIKTEX_FATAL_ERROR_2(T_("Directory not found in file name database."), "path", lpszPath);
  }

  PathName path(rootDirectory, lpszPath);

#if 0
  for (const FileNameDatabaseDirectory* dirIt = dir; dirIt != nullptr; dirIt = GetDirectoryAt(dirIt->foExtension))
  {
    for (FndbWord i = 0; i < dirIt->numSubDirs; ++i)
    {
      if (!pCallback->OnFndbItem(path, GetString(dirIt->GetSubDirName(i)), "", true))
      {
        return false;
      }
    }

  }

  for (const FileNameDatabaseDirectory* dirIt = dir; dirIt != nullptr; dirIt = GetDirectoryAt(dirIt->foExtension))
  {
    for (FndbWord i = 0; i < dirIt->numFiles; ++i)
    {
      const char* lpszFileNameInfo = 0;
      if (HasFileNameInfo())
      {
        FndbByteOffset fo = dirIt->GetFileNameInfo(i);
        if (fo != 0)
        {
          lpszFileNameInfo = GetString(fo);
        }
      }
      if (!pCallback->OnFndbItem(path, GetString(dirIt->GetFileName(i)), lpszFileNameInfo == nullptr ? "" : lpszFileNameInfo, false))
      {
        return false;
      }
    }
  }
#endif

  return nullptr;
}

#endif // experimental
