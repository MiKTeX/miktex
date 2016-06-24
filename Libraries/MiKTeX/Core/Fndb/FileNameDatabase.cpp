/* FileNameDatabase.cpp: file name database

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/PathNameParser.h"

#include "FileNameDatabase.h"
#include "Utils/AutoTraceTime.h"

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
  catch (const exception &)
  {
  }
}

void FileNameDatabase::OpenFileNameDatabase(const char * lpszFndbPath, bool readWrite)
{
#if defined(MIKTEX_WINDOWS)
  // check file attributes
  FileAttributeSet attributes = File::GetAttributes(lpszFndbPath);
  if (attributes[FileAttribute::ReadOnly])
  {
    traceStream->WriteFormattedLine("core", T_("file name database file is readonly"));
    readWrite = false;
  }
#endif

  mmap->Open(lpszFndbPath, readWrite);

  if (mmap->GetSize() < sizeof(*pHeader))
  {
    MIKTEX_FATAL_ERROR_2(T_("Not a file name database file (wrong size)."), "path", lpszFndbPath);
  }

  pHeader = reinterpret_cast<FileNameDatabaseHeader*>(mmap->GetPtr());

  foEnd = static_cast<FndbByteOffset>(mmap->GetSize());

  // check signature
  if (pHeader->signature != FileNameDatabaseHeader::Signature)
  {
    MIKTEX_FATAL_ERROR_2(T_("Not a file name database file (wrong signature)."), "path", lpszFndbPath);
  }

  // check version number
  if (pHeader->version != FileNameDatabaseHeader::Version)
  {
    MIKTEX_FATAL_ERROR_2(T_("Unknown file name database file version."), "path", lpszFndbPath, "versionFound", std::to_string(pHeader->Version), "versionExpected", std::to_string(FileNameDatabaseHeader::Version));
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
      traceStream->WriteFormattedLine("core", T_("enlarging fndb file %s (%u -> %u)..."), Q_(lpszFndbPath), static_cast<unsigned>(foEnd), static_cast<unsigned>(newSize));
      pHeader = reinterpret_cast<FileNameDatabaseHeader*>(mmap->Resize(newSize));
#if defined(MIKTEX_WINDOWS) && REPORT_EVENTS
      ReportMiKTeXEvent(EVENTLOG_INFORMATION_TYPE, MIKTEX_EVENT_FNDB_ENLARGED, mmap->GetName(), std::to_string(foEnd), std::to_string(static_cast<unsigned>(newSize)), 0);
#endif
      foEnd = static_cast<FndbByteOffset>(newSize);
    }
  }

  timeStamp = pHeader->timeStamp;
}

void FileNameDatabase::Initialize(const char * lpszFndbPath, const char * lpszRoot, bool readWrite)
{
  rootDirectory = lpszRoot;
  isInvariable = !readWrite;

  OpenFileNameDatabase(lpszFndbPath, readWrite);

  if ((pHeader->flags & FileNameDatabaseHeader::FndbFlags::Frozen) != 0)
  {
    isInvariable = true;
  }

  ReadFileNames();
}

FileNameDatabaseDirectory * FileNameDatabase::FindSubDirectory(const FileNameDatabaseDirectory * pDir, const char * lpszRelPath) const
{
  MIKTEX_ASSERT(pDir != nullptr);
  FndbByteOffset fo = 0;
  for (PathNameParser dirName(lpszRelPath); dirName.GetCurrent() != nullptr; ++dirName)
  {
    if (PathName::Compare(dirName.GetCurrent(), CURRENT_DIRECTORY) == 0)
    {
      fo = GetByteOffset(pDir);
      continue;
    }
    bool matching = false;
    FndbWord subidx = 0;
    while (!matching && pDir != nullptr)
    {
      FndbWord lo = 0;
      FndbWord hi = pDir->numSubDirs;
      while (!matching && hi > lo)
      {
	subidx = lo + (hi - lo) / 2;
	FndbByteOffset foSubDirName = pDir->GetSubDirName(subidx);
	const char * lpszSubdirName = GetString(foSubDirName);
	int cmp = PathName::Compare(lpszSubdirName, dirName.GetCurrent());
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
	pDir = GetDirectoryAt(pDir->foExtension);
      }
    }
    if (!matching)
    {
      fo = 0;
      break;
    }
    fo = pDir->GetSubDir(subidx);
    pDir = GetDirectoryAt(fo);
  }
  return GetDirectoryAt(fo);
}

FileNameDatabaseDirectory * FileNameDatabase::SearchFileName(FileNameDatabaseDirectory * pDir, const char * lpszFileName, FndbWord & index) const
{
  for (; pDir != nullptr; pDir = GetDirectoryAt(pDir->foExtension))
  {
    FndbWord lo = 0;
    FndbWord hi = pDir->numFiles;
    while (hi > lo)
    {
      index = lo + (hi - lo) / 2;
      const char * lpszCandidate = GetString(pDir->GetFileName(index));
      int cmp = PathName::Compare(lpszCandidate, lpszFileName);
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
	return pDir;
      }
    }
  }
  return nullptr;
}

void FileNameDatabase::MakePathName(const FileNameDatabaseDirectory * pDir, PathName & path) const
{
  if (pDir == nullptr || pDir->foParent == 0)
  {
    return;
  }
  // RECURSION
  MakePathName(GetDirectoryAt(pDir->foParent), path);
  path /= GetString(pDir->foName);
}

#define ROUND2(n, pow2) ((n + pow2 - 1) & ~(pow2 - 1))

FileNameDatabaseDirectory * FileNameDatabase::ExtendDirectory(FileNameDatabaseDirectory * pDir) const
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
  FileNameDatabaseDirectory * pDirExt = reinterpret_cast<FileNameDatabaseDirectory *>(GetPointer(foExtension));
  pDirExt->Init();
  pDirExt->foName = pDir->foName;
  pDirExt->foParent = pDir->foParent;
  pDirExt->numFiles = 0;
  pDirExt->numSubDirs = 0;
  pDirExt->foExtension = 0;
  pDirExt->capacity = neededSlots;
  memset(pDirExt->table, 0, sizeof(FndbByteOffset) * neededSlots);
  pDir->foExtension = foExtension;
  return pDirExt;
}

FndbByteOffset FileNameDatabase::CreateString(const char * lpszName) const
{
  FndbByteOffset foName;
  size_t neededBytes = strlen(lpszName) + 1;
  foName = ROUND2(pHeader->size, 2);
  if (foName + neededBytes > foEnd)
  {
    MIKTEX_UNEXPECTED();
  }
  memcpy(GetPointer(foName), lpszName, neededBytes);
  pHeader->size = foName + static_cast<FndbWord>(neededBytes);
  return foName;
}

FndbWord FileNameDatabase::FindLowerBound(const FndbByteOffset & begin, FndbWord count, const char * lpszName, bool & isDuplicate) const
{
  const FndbByteOffset * pBegin = &begin;
  const FndbByteOffset * pEnd = &pBegin[count];
  const FndbByteOffset * iter = pBegin;
  while (iter != pEnd && PathName::Compare(lpszName, GetString(*iter)) > 0)
  {
    ++iter;
  }
  isDuplicate = iter != pEnd && PathName::Compare(lpszName, GetString(*iter)) == 0;
  return static_cast<FndbWord>(iter - pBegin);
}

void FileNameDatabase::InsertFileName(FileNameDatabaseDirectory * pDir, FndbByteOffset foFileName, FndbByteOffset foFileNameInfo) const
{
  MIKTEX_ASSERT(pDir->capacity >= (pDir->SizeOfTable(HasFileNameInfo()) + (HasFileNameInfo() ? 2 : 1)));
  bool isDuplicate;
  FndbWord idx = FindLowerBound(pDir->table[0], pDir->numFiles, GetString(foFileName), isDuplicate);
  if (isDuplicate)
  {
    return;
  }
  pDir->TableInsert(idx, foFileName);
  if (HasFileNameInfo())
  {
    pDir->TableInsert(pDir->numFiles + 1 + 2 * pDir->numSubDirs + idx, foFileNameInfo);
  }
  pDir->numFiles += 1;
  pHeader->numFiles += 1;
  pHeader->timeStamp = static_cast<FndbWord>(time(0)); // <sixtyfourbit/>
}

void FileNameDatabase::InsertDirectory(FileNameDatabaseDirectory * pDir, const FileNameDatabaseDirectory * pDirSub) const
{
  MIKTEX_ASSERT(pDir->capacity >= pDir->SizeOfTable(HasFileNameInfo()) + 2);
  bool isDuplicate;
  FndbWord idx = FindLowerBound(pDir->table[pDir->numFiles], pDir->numSubDirs, GetString(pDirSub->foName), isDuplicate);
  if (isDuplicate)
  {
    return;
  }
  pDir->TableInsert(pDir->numFiles + idx, pDirSub->foName);
  pDir->TableInsert(pDir->numFiles + pDir->numSubDirs + 1 + idx, GetByteOffset(pDirSub));
  pDir->numSubDirs += 1;
  pHeader->numDirs += 1;
}

FileNameDatabaseDirectory * FileNameDatabase::CreateFndbDirectory(FileNameDatabaseDirectory * pDir, const char * lpszName) const
{
  while (pDir->capacity < pDir->SizeOfTable(HasFileNameInfo()) + 2)
  {
    if (pDir->foExtension != 0)
    {
      pDir = GetDirectoryAt(pDir->foExtension);
    }
    else
    {
      pDir = ExtendDirectory(pDir);
    }
    MIKTEX_ASSERT(pDir != nullptr);
  }

  FndbByteOffset foName = CreateString(lpszName);

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
  FileNameDatabaseDirectory * pDirSub = reinterpret_cast<FileNameDatabaseDirectory *>(GetPointer(foSub));
  pDirSub->Init();
  pDirSub->foName = foName;
  pDirSub->foParent = GetByteOffset(pDir);
  pDirSub->numFiles = 0;
  pDirSub->numSubDirs = 0;
  pDirSub->foExtension = 0;
  pDirSub->capacity = neededSlots;
  memset(pDirSub->table, 0, sizeof(FndbByteOffset) * neededSlots);
  InsertDirectory(pDir, pDirSub);

  return pDirSub;
}

FileNameDatabaseDirectory * FileNameDatabase::CreateDirectoryPath(FileNameDatabaseDirectory * pDir, const char * lpszRelPath) const
{
  bool create = false;
  FndbWord level = 0;
  for (PathNameParser dirName(lpszRelPath); dirName.GetCurrent() != nullptr; ++dirName)
  {
    FileNameDatabaseDirectory * pDirSub = nullptr;
    if (!create)
    {
      pDirSub = FindSubDirectory(pDir, dirName.GetCurrent());
      if (pDirSub == nullptr)
      {
	create = true;
      }
    }
    if (create)
    {
      pDirSub = CreateFndbDirectory(pDir, dirName.GetCurrent());
      if (pDirSub == nullptr)
      {
	MIKTEX_UNEXPECTED();
      }
    }
    pDir = pDirSub;
    ++level;
  }
  if (level > pHeader->depth)
  {
    MIKTEX_ASSERT(create);
    pHeader->depth = level;
  }
  return pDir;
}

FileNameDatabaseDirectory * FileNameDatabase::RemoveFileName(FileNameDatabaseDirectory * pDir, const char * lpszFileName) const
{
  FndbWord index;

  pDir = SearchFileName(pDir, lpszFileName, index);

  if (pDir == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }

  pDir->TableRemove(index);

  if (HasFileNameInfo())
  {
    pDir->TableRemove(pDir->numFiles - 1 + 2 * pDir->numSubDirs + index);
  }

  pDir->numFiles -= 1;
  pHeader->numFiles -= 1;
  pHeader->timeStamp = static_cast<FndbWord>(time(0)); // FIXME: 64-bit

  return pDir;
}

void FileNameDatabase::Flush() const
{
  traceStream->WriteFormattedLine("core", T_("flushing file name database"));
  mmap->Flush();
}

// FIXME: not UTF-8 safe
MIKTEXSTATICFUNC(bool) Match(const char * lpszPathPattern, const char * lpszPath)
{
  MIKTEX_ASSERT(PathName(lpszPathPattern).IsComparable());
  MIKTEX_ASSERT(PathName(lpszPath).IsComparable());
  int lastch = 0;
  for (; *lpszPathPattern != 0 && *lpszPath != 0; ++lpszPathPattern, ++lpszPath)
  {
    if (*lpszPathPattern == *lpszPath)
    {
      lastch = *lpszPath;
      continue;
    }
    MIKTEX_ASSERT(RECURSION_INDICATOR_LENGTH == 2);
    MIKTEX_ASSERT(IsDirectoryDelimiter(RECURSION_INDICATOR[0]));
    MIKTEX_ASSERT(IsDirectoryDelimiter(RECURSION_INDICATOR[1]));
    if (*lpszPathPattern == RECURSION_INDICATOR[1] && IsDirectoryDelimiter(lastch))
    {
      for (; IsDirectoryDelimiter(*lpszPathPattern); ++lpszPathPattern)
      {
      };
      if (*lpszPathPattern == 0)
      {
	return true;
      }
      for (; *lpszPath != 0; ++lpszPath)
      {
	if (IsDirectoryDelimiter(lastch))
	{
	  // RECURSION
	  if (Match(lpszPathPattern, lpszPath))
	  {
	    return true;
	  }
	}
	lastch = *lpszPath;
      }
    }
    return false;
  }
  return (*lpszPathPattern == 0 || strcmp(lpszPathPattern, RECURSION_INDICATOR) == 0) && *lpszPath == 0;
}

bool FileNameDatabase::Search(const char * lpszFileName, const char * lpszPathPattern, bool firstMatchOnly, vector<PathName> & result, vector<string> & fileNameInfo) const
{
  traceStream->WriteFormattedLine("core", T_("fndb search: rootDirectory=%s, filename=%s, pathpattern=%s"), Q_(rootDirectory), Q_(lpszFileName), Q_(lpszPathPattern));

  MIKTEX_ASSERT(result.size() == 0);
  MIKTEX_ASSERT(fileNameInfo.size() == 0);
  MIKTEX_ASSERT(!Utils::IsAbsolutePath(lpszFileName));
  MIKTEX_ASSERT(!IsExplicitlyRelativePath(lpszFileName));

  char szDir[BufferSizes::MaxPath];

  PathName scratch1;

  PathName::Split(lpszFileName, szDir, BufferSizes::MaxPath, nullptr, 0, nullptr, 0);

  if (szDir[0] != 0)
  {
    size_t l = strlen(szDir);
    if (IsDirectoryDelimiter(szDir[l - 1]))
    {
      szDir[l - 1] = 0;
      --l;
    }
    scratch1 = lpszPathPattern;
    scratch1 /= szDir;
    lpszPathPattern = scratch1.Get();
    lpszFileName += l + 1;
  }

  PathName comparableFileName(lpszFileName);
  comparableFileName.TransformForComparison();

  // check to see whether we have this file name
  pair<FileNameHashTable::const_iterator, FileNameHashTable::const_iterator> range = fileNames.equal_range(comparableFileName.ToString());

  if (range.first == range.second)
  {
    return false;
  }

  // path pattern must be relative to root directory
  if (Utils::IsAbsolutePath(lpszPathPattern))
  {
    const char * lpsz = Utils::GetRelativizedPath(lpszPathPattern, rootDirectory.Get());
    if (lpsz == nullptr)
    {
      MIKTEX_FATAL_ERROR_2(T_("Path pattern is not covered by file name database."), "pattern", lpszPathPattern);
    }
    lpszPathPattern = lpsz;
  }

  PathName comparablePathPattern(lpszPathPattern);
  comparablePathPattern.TransformForComparison();

  for (FileNameHashTable::const_iterator it = range.first; it != range.second; ++it)
  {
    PathName relPath;
    MakePathName(it->second, relPath);
    if (Match(comparablePathPattern.Get(), PathName(relPath).TransformForComparison().Get()))
    {
      PathName path;
      path = rootDirectory;
      path /= relPath;
      path /= lpszFileName;
      result.push_back(path);
      if (HasFileNameInfo())
      {
	FndbWord idx;
	const FileNameDatabaseDirectory * pDir = SearchFileName(it->second, lpszFileName, idx);
	if (pDir == nullptr)
	{
	  MIKTEX_UNEXPECTED();
	}
	fileNameInfo.push_back(GetString(pDir->GetFileNameInfo(idx)));
	traceStream->WriteFormattedLine("core", T_("found: %s (%s)"), Q_(path), GetString(pDir->GetFileNameInfo(idx)));
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

shared_ptr<FileNameDatabase> FileNameDatabase::Create(const char * lpszFndbPath, const char * lpszRoot, bool readOnly)
{
  MIKTEX_ASSERT_STRING(lpszFndbPath);
  MIKTEX_ASSERT_STRING(lpszRoot);

  shared_ptr<FileNameDatabase> fndb = make_shared<FileNameDatabase>();
  fndb->Initialize(lpszFndbPath, lpszRoot, !readOnly);
  return fndb;
}

void FileNameDatabase::AddFile(const char * lpszPath, const char * lpszFileNameInfo)
{
  MIKTEX_ASSERT_STRING(lpszPath);

  traceStream->WriteFormattedLine("core", T_("adding %s to the file name database"), Q_(lpszPath));

  // make sure we can add files
  if (IsInvariable())
  {
    MIKTEX_UNEXPECTED();
  }

  // make sure that the path is relative to the texmf root directory
  if (Utils::IsAbsolutePath(lpszPath))
  {
    const char * lpsz = Utils::GetRelativizedPath(lpszPath, rootDirectory.Get());
    if (lpsz == nullptr)
    {
      MIKTEX_FATAL_ERROR_2(T_("File name is not covered by file name database."), "path", lpszPath);
    }
    lpszPath = lpsz;
  }

  // make a working copy of the path; separate file name from directory name
  PathName pathDirectory(lpszPath);
  pathDirectory.RemoveFileSpec();
  PathName pathFile(lpszPath);
  pathFile.RemoveDirectorySpec();

  // get (possibly create) the parent directory
  FileNameDatabaseDirectory * pDir;
  if (pathDirectory.GetLength() > 0)
  {
    pDir = CreateDirectoryPath(GetTopDirectory(), pathDirectory.Get());
  }
  else
  {
    // no sub-directory, i.e. create file in top directory
    pDir = GetTopDirectory();
  }

  if (pDir == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }

  // extend the directory, if necessary
  while (pDir->capacity < (pDir->SizeOfTable(HasFileNameInfo()) + (HasFileNameInfo() ? 2 : 1)))
  {
    if (pDir->foExtension != 0)
    {
      // get next extension
      pDir = GetDirectoryAt(pDir->foExtension);
    }
    else
    {
      // create an extension
      pDir = ExtendDirectory(pDir);
      if (pDir == nullptr)
      {
	MIKTEX_UNEXPECTED();
      }
    }
  }

  // create a new table entry
  InsertFileName(pDir, CreateString(pathFile.Get()), (lpszFileNameInfo ? CreateString(lpszFileNameInfo) : 0));

  // add the name to the hash table
  PathName comparableFileName(pathFile);
  comparableFileName.TransformForComparison();
  fileNames.insert(pair<string, FileNameDatabaseDirectory *>(comparableFileName.ToString(), pDir));
}

bool FileNameDatabase::Enumerate(const char * lpszPath, IEnumerateFndbCallback * pCallback) const
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
      const char * lpsz = Utils::GetRelativizedPath(lpszPath, rootDirectory.Get());
      if (lpsz == nullptr)
      {
	MIKTEX_FATAL_ERROR_2(T_("Path is not covered by file name database."), "path", lpszPath);
      }
      lpszPath = lpsz;
    }
  }

  const FileNameDatabaseDirectory * pDir = lpszPath == nullptr || *lpszPath == 0 ? GetTopDirectory() : FindSubDirectory(GetTopDirectory(), lpszPath);

  if (pDir == nullptr)
  {
    MIKTEX_FATAL_ERROR_2(T_("Directory not found in file name database."), "path", lpszPath);
  }

  PathName path(rootDirectory, lpszPath);

  for (const FileNameDatabaseDirectory * pDirIter = pDir; pDirIter != nullptr; pDirIter = GetDirectoryAt(pDirIter->foExtension))
  {
    for (FndbWord i = 0; i < pDirIter->numSubDirs; ++i)
    {
      if (!pCallback->OnFndbItem(path.Get(), GetString(pDirIter->GetSubDirName(i)), nullptr, true))
      {
	return false;
      }
    }
  }

  for (const FileNameDatabaseDirectory * pDirIter = pDir; pDirIter != nullptr; pDirIter = GetDirectoryAt(pDirIter->foExtension))
  {
    for (FndbWord i = 0; i < pDirIter->numFiles; ++i)
    {
      const char * lpszFileNameInfo = nullptr;
      if (HasFileNameInfo())
      {
	FndbByteOffset fo = pDirIter->GetFileNameInfo(i);
	if (fo != 0)
	{
	  lpszFileNameInfo = GetString(fo);
	}
      }
      if (!pCallback->OnFndbItem(path.Get(), GetString(pDirIter->GetFileName(i)), lpszFileNameInfo, false))
      {
	return false;
      }
    }
  }

  return true;
}

FileNameDatabaseDirectory * FileNameDatabase::TryGetParent(const char * lpszPath) const
{
  MIKTEX_ASSERT_STRING(lpszPath);

  // make sure that the path is relative to the texmf root directory
  if (Utils::IsAbsolutePath(lpszPath))
  {
    const char * lpsz = Utils::GetRelativizedPath(lpszPath, rootDirectory.Get());
    if (lpsz == nullptr)
    {
      MIKTEX_FATAL_ERROR_2(T_("The path name is not covered by the file name database."), "path", lpszPath);
    }
    lpszPath = lpsz;
  }

  // make a working copy; separate file name from directory name
  PathName pathDirectory(lpszPath);
  pathDirectory.RemoveFileSpec();

  // get the parent directory
  FileNameDatabaseDirectory * pDir;
  if (pathDirectory.GetLength() > 0)
  {
    pDir = FindSubDirectory(GetTopDirectory(), pathDirectory.Get());
  }
  else
  {
    pDir = GetTopDirectory();
  }

  return pDir;
}

void FileNameDatabase::RemoveFile(const char * lpszPath)
{
  MIKTEX_ASSERT_STRING(lpszPath);

  traceStream->WriteFormattedLine ("core", T_("removing %s from the file name database"), Q_(lpszPath));

  if (IsInvariable())
  {
    MIKTEX_UNEXPECTED();
  }

  PathName pathFile(lpszPath);
  pathFile.RemoveDirectorySpec();

  FileNameDatabaseDirectory * pDir = TryGetParent(lpszPath);

  if (pDir == nullptr)
  {
    MIKTEX_FATAL_ERROR_2(T_("The path could not be found in the file name database."), "path", lpszPath);
  }

  // remove the file name
  pDir = RemoveFileName(pDir, pathFile.Get());

  // also from the hash table
  PathName comparableFileName(pathFile);
  comparableFileName.TransformForComparison();
  pair<FileNameHashTable::const_iterator, FileNameHashTable::const_iterator> range = fileNames.equal_range(comparableFileName.ToString());
  if (range.first == range.second)
  {
    MIKTEX_FATAL_ERROR_2(T_("The file name could not be found in the hash table."), "path", lpszPath);
  }
  bool removed = false;
  for (FileNameHashTable::const_iterator it = range.first; it != range.second; ++it)
  {
    if (it->second == pDir)
    {
      fileNames.erase(it);
      removed = true;
      break;
    }
  }
  if (!removed)
  {
    MIKTEX_FATAL_ERROR_2(T_("The file name could not be removed from the hash table."), "path", lpszPath);
  }
}

bool FileNameDatabase::FileExists(const PathName & path) const
{
  FileNameDatabaseDirectory * pDir = TryGetParent(path.Get());
  if (pDir != nullptr)
  {
    PathName fileName(path);
    fileName.RemoveDirectorySpec();
    FndbWord index;
    pDir = SearchFileName(pDir, fileName.Get(), index);
  }
  return pDir != nullptr;
}

void FileNameDatabase::ReadFileNames()
{
  fileNames.clear();
  fileNames.rehash(pHeader->numFiles);
  AutoTraceTime att("fndb read files", rootDirectory.Get());
  ReadFileNames(GetTopDirectory());
}

void FileNameDatabase::ReadFileNames(FileNameDatabaseDirectory * pDir)
{
  for (FileNameDatabaseDirectory * pDirIter = pDir; pDirIter != nullptr; pDirIter = GetDirectoryAt(pDirIter->foExtension))
  {
    for (FndbWord i = 0; i < pDirIter->numSubDirs; ++i)
    {
      // RECURSION
      ReadFileNames(GetDirectoryAt(pDirIter->GetSubDir(i)));
    }
    for (FndbWord i = 0; i < pDirIter->numFiles; ++i)
    {
      fileNames.insert(pair<string, FileNameDatabaseDirectory *>(PathName(GetString(pDirIter->GetFileName(i))).TransformForComparison().ToString(), pDirIter));
    }
  }
}

#if 0 // experimental

class FndbDirectoryLister : public DirectoryLister
{
public:
  void MIKTEXTHISCALL Close() override;

public:
  bool MIKTEXTHISCALL GetNext(DirectoryEntry & direntry) override;

public:
  bool MIKTEXTHISCALL GetNext(DirectoryEntry2 & direntry2) override;

public:
  FndbDirectoryLister(const PathName & directory);

public:
  MIKTEXTHISCALL ~FndbDirectoryLister() override;

private:
  PathName directory;

private:
  string pattern;

private:
  friend class DirectoryLister;
};

unique_ptr<DirectoryLister> FileNameDatabase::OpenDirectory(const char * lpszPath)
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
      const char * lpsz = Utils::GetRelativizedPath(lpszPath, rootDirectory.Get());
      if (lpsz == nullptr)
      {
	MIKTEX_FATAL_ERROR_2(T_("The path is not covered by file name database."), "path", lpszPath);
      }
      lpszPath = lpsz;
    }
  }

  const FileNameDatabaseDirectory * pDir = lpszPath == nullptr || *lpszPath == 0 ? GetTopDirectory() : FindSubDirectory(GetTopDirectory(), lpszPath);

  if (pDir == nullptr)
  {
    MIKTEX_FATAL_ERROR_2(T_("Directory not found in file name database."), "path", lpszPath);
  }

  PathName path(rootDirectory, lpszPath);

#if 0
  for (const FileNameDatabaseDirectory * pDirIter = pDir; pDirIter != nullptr; pDirIter = GetDirectoryAt(pDirIter->foExtension))
  {
    for (FndbWord i = 0; i < pDirIter->numSubDirs; ++i)
    {
      if (!pCallback->OnFndbItem(path.Get(), GetString(pDirIter->GetSubDirName(i)), 0, true))
      {
	return false;
      }
    }

  }

  for (const FileNameDatabaseDirectory * pDirIter = pDir; pDirIter != nullptr; pDirIter = GetDirectoryAt(pDirIter->foExtension))
  {
    for (FndbWord i = 0; i < pDirIter->numFiles; ++i)
    {
      const char * lpszFileNameInfo = 0;
      if (HasFileNameInfo())
      {
	FndbByteOffset fo =
	  pDirIter->GetFileNameInfo(i);
	if (fo != 0)
	{
	  lpszFileNameInfo = GetString(fo);
	}
      }
      if (!pCallback->OnFndbItem(path.Get(), GetString(pDirIter->GetFileName(i)), lpszFileNameInfo, false))
      {
	return false;
      }
    }
  }
#endif

  return nullptr;
}

#endif // experimental
