/* makefndb.cpp: creating the file name database

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

#include "config.h"

#include <fstream>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/AutoResource>
#include <miktex/Core/Directory>
#include <miktex/Core/FileStream>
#include <miktex/Core/Paths>
#include <miktex/Core/Registry>

#include <miktex/Trace/Trace>

#include "internal.h"

#include "Session/SessionImpl.h"
#include "fndbmem.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;

const uint8_t null_byte = 0;

#define FN_MIKTEXIGNORE ".miktexignore"

struct FILENAMEINFO
{
  string FileName;
  const string* Directory = nullptr;
  const string* Info = nullptr;
};

class FndbManager
{
public:
  FndbManager() :
    trace_fndb(TraceStream::Open(MIKTEX_TRACE_FNDB)),
    trace_error(TraceStream::Open(MIKTEX_TRACE_ERROR))
  {
  }

public:
  bool Create(const PathName& fndbPath, const PathName& rootPath, ICreateFndbCallback* callback, bool enableStringPooling, bool storeFileNameInfo);

private:
  void* GetMemPointer()
  {
    return byteArray.data();
  }

private:
  FndbByteOffset GetMemTop() const
  {
    return static_cast<FndbByteOffset>(byteArray.size());
  }

private:
  void SetMem(FndbByteOffset fo, const void* data, size_t size)
  {
    MIKTEX_ASSERT(fo + size <= GetMemTop());
    const uint8_t* begin = reinterpret_cast<const uint8_t*>(data);
    const uint8_t* end = begin + size;
    std::copy(begin, end, byteArray.begin() + fo);
  }

private:
  void SetMem(FndbByteOffset fo, FndbByteOffset data)
  {
    SetMem(fo, &data, sizeof(data));
  }

private:
  FndbByteOffset ReserveMem(size_t size);

private:
  void FastPushBack(uint8_t data)
  {
    byteArray.push_back(data);
  }

private:
  FndbByteOffset PushBack(uint8_t data)
  {
    FndbByteOffset ret = GetMemTop();
    FastPushBack(data);
    return ret;
  }

private:
  FndbByteOffset PushBack(FndbWord data);

private:
  FndbByteOffset PushBack(const void* data, size_t size);

private:
  FndbByteOffset PushBack(const char* data);

private:
  void AlignMem(size_t align = 8);

private:
  static void GetIgnorableFiles(const PathName& dirPath, vector<string>& filesToBeIgnored);

public:
  void ReadDirectory(const PathName& dirPath, vector<string>& subDirectoryNames, vector<FILENAMEINFO>& fileNames, bool doCleanUp);

private:
  void CollectFiles(const PathName& parentPath, const PathName& folderName, vector<FILENAMEINFO>& fileNames);

private:
  PathName rootPath;

private:
  vector<uint8_t> byteArray;

private:
  size_t deepestLevel;

private:
  size_t currentLevel;

private:
  size_t numDirectories;

private:
  size_t numFiles;

private:
  ICreateFndbCallback* callback;

private:
  unordered_set<string> stringPool;
  
private:
  typedef unordered_map<string, FndbByteOffset> StringMap;

private:
  StringMap stringMap;

private:
  bool enableStringPooling;

private:
  bool storeFileNameInfo;

private:
  unique_ptr<TraceStream> trace_fndb;

private:
  unique_ptr<TraceStream> trace_error;
};

FndbByteOffset FndbManager::ReserveMem(size_t size)
{
  FndbByteOffset ret = GetMemTop();
  byteArray.reserve(byteArray.size() + size);
  for (size_t i = 0; i < size; ++i)
  {
    byteArray.push_back(null_byte);
  }
  return ret;
}

FndbByteOffset FndbManager::PushBack(FndbWord data)
{
  AlignMem(sizeof(FndbWord));
  const uint8_t* byteArray = reinterpret_cast<const uint8_t*>(&data);
  if (sizeof(FndbWord) == 4)
  {
    FndbByteOffset ret = GetMemTop();
    FastPushBack(byteArray[0]);
    FastPushBack(byteArray[1]);
    FastPushBack(byteArray[2]);
    FastPushBack(byteArray[3]);
    return ret;
  }
  else
  {
    return PushBack(byteArray, sizeof(FndbWord));
  }
}

FndbByteOffset FndbManager::PushBack(const void* data, size_t size)
{
  FndbByteOffset ret = GetMemTop();
  const uint8_t* byteArray = reinterpret_cast<const uint8_t*>(data);
  for (size_t i = 0; i < size; ++i)
  {
    FastPushBack(byteArray[i]);
  }
  return ret;
}

FndbByteOffset FndbManager::PushBack(const char* data)
{
  if (enableStringPooling)
  {
    StringMap::const_iterator it = stringMap.find(data);
    if (it != stringMap.end())
    {
      return it->second;
    }
  }
  FndbByteOffset ret = GetMemTop();
  MIKTEX_ASSERT(data != nullptr);
  PushBack(data, strlen(data));
  FastPushBack(null_byte);
  if (enableStringPooling)
  {
    stringMap[data] = ret;
  }
  return ret;
}

void FndbManager::AlignMem(size_t align)
{
  FndbByteOffset foTop = GetMemTop();
  while (((foTop++) % align) > 0)
  {
    FastPushBack(null_byte);
  }
}

void FndbManager::GetIgnorableFiles(const PathName& dirPath, vector<string>& filesToBeIgnored)
{
  PathName ignoreFile(dirPath, FN_MIKTEXIGNORE);
  if (!File::Exists(ignoreFile))
  {
    return;
  }
  ifstream reader = File::CreateInputStream(ignoreFile);
  filesToBeIgnored.reserve(10);
  for (string line; std::getline(reader, line); )
  {
    filesToBeIgnored.push_back(line);
  }
  sort(filesToBeIgnored.begin(), filesToBeIgnored.end(), StringComparerIgnoringCase());
}

void FndbManager::ReadDirectory(const PathName& dirPath, vector<string>& subDirectoryNames, vector<FILENAMEINFO>& fileNames, bool doCleanUp)
{
  if (!Directory::Exists(dirPath))
  {
    trace_fndb->WriteLine("core", fmt::format(T_("the directory {0} does not exist"), Q_(dirPath)));
    return;
  }
  vector<string> filesToBeIgnored;
  GetIgnorableFiles(dirPath, filesToBeIgnored);
  unique_ptr<DirectoryLister> lister = DirectoryLister::Open(dirPath);
  DirectoryEntry entry;
  vector<DirectoryEntry> toBeDeleted;
  PathName directory = Utils::GetRelativizedPath(dirPath.GetData(), rootPath.GetData());
  directory = directory.ToUnix();
  while (lister->GetNext(entry))
  {
    if (binary_search(filesToBeIgnored.begin(), filesToBeIgnored.end(), entry.name, StringComparerIgnoringCase()))
    {
      continue;
    }
    if (doCleanUp && PathName(entry.name).HasExtension(MIKTEX_TO_BE_DELETED_FILE_SUFFIX))
    {
      toBeDeleted.push_back(entry);
    }
    else if (entry.isDirectory)
    {
      subDirectoryNames.push_back(entry.name);
    }
    else
    {
      FILENAMEINFO filenameinfo;
      filenameinfo.FileName = entry.name;
      filenameinfo.Directory = &*stringPool.insert(directory.ToString()).first;
      fileNames.push_back(filenameinfo);
    }
  }
  lister->Close();

  // silent clean-up
  for (const DirectoryEntry& e : toBeDeleted)
  {
    try
    {
      PathName path(dirPath, e.name);
      if (e.isDirectory)
      {
        Directory::Delete(path, true);
      }
      else
      {
        File::Delete(path);
      }
    }
    catch (const exception&)
    {
    }
  }
}

void FndbManager::CollectFiles(const PathName& parentPath, const PathName& folderName, vector<FILENAMEINFO>& fileNames)
{
  if (currentLevel > deepestLevel)
  {
    deepestLevel = currentLevel;
  }

  vector<string> subDirectoryNames;
  subDirectoryNames.reserve(40);

  bool done = false;

  PathName path(parentPath, folderName);
  path.MakeAbsolute();

  PathName directory = Utils::GetRelativizedPath(path.GetData(), rootPath.GetData());
  directory = directory.ToUnix();

  if (callback != nullptr)
  {
    if (!callback->OnProgress(static_cast<unsigned>(currentLevel), path))
    {
      throw OperationCancelledException();
    }
    vector<string> subDirs;
    vector<string> files;
    vector<string> infos;
    done = callback->ReadDirectory(path, subDirs, files, infos);
    if (done)
    {
      subDirectoryNames = subDirs;
      MIKTEX_ASSERT(files.size() == infos.size());
      for (int i = 0; i < files.size(); ++i)
      {
        FILENAMEINFO filenameinfo;
        filenameinfo.FileName = files[i];
        filenameinfo.Directory = &*stringPool.insert(directory.ToString()).first;
        filenameinfo.Info = &*stringPool.insert(infos[i]).first;
        fileNames.push_back(filenameinfo);
      }
    }
  }

  if (!done)
  {
    ReadDirectory(path, subDirectoryNames, fileNames, true);
  }

  numDirectories += subDirectoryNames.size();

  // recurse into sub-directories
  PathName pathFolder(parentPath, folderName);
  size_t i = 0;
  ++currentLevel;
  for (const string& s : subDirectoryNames)
  {
    // RECURSION
    CollectFiles(pathFolder, s, fileNames);
    ++i;
  }
  --currentLevel;
}

bool FndbManager::Create(const PathName& fndbPath, const PathName& rootPath, ICreateFndbCallback* callback, bool enableStringPooling, bool storeFileNameInfo)
{
  trace_fndb->WriteLine("core", fmt::format(T_("creating fndb file {0}..."), Q_(fndbPath)));
  unsigned rootIdx = SessionImpl::GetSession()->DeriveTEXMFRoot(rootPath);
  this->rootPath = rootPath;
  this->enableStringPooling = enableStringPooling;
  this->storeFileNameInfo = storeFileNameInfo;
  byteArray.reserve(2 * 1024 * 1024);
  try
  {
    ReserveMem(sizeof(FileNameDatabaseHeader));
    FileNameDatabaseHeader fndb;
    fndb.Init();
    numDirectories = 0;
    numFiles = 0;
    deepestLevel = 0;
    currentLevel = 0;
    this->callback = callback;
    vector<FILENAMEINFO> fileNames;
    CollectFiles(rootPath, CURRENT_DIRECTORY, fileNames);
    numFiles = fileNames.size();
    AlignMem();
    fndb.foTable = ReserveMem(fileNames.size() * sizeof(FileNameDatabaseRecord));
    AlignMem();
    fndb.foStrings = GetMemTop();
    for (size_t idx = 0; idx < fileNames.size(); ++idx)
    {
      FileNameDatabaseRecord rec;
      rec.foFileName = PushBack(fileNames[idx].FileName.c_str());
      rec.foDirectory = PushBack(fileNames[idx].Directory->c_str());
      rec.foInfo = PushBack(fileNames[idx].Info == nullptr ? "" : fileNames[idx].Info->c_str());
      SetMem(static_cast<unsigned>(fndb.foTable + idx * sizeof(rec)), &rec, sizeof(rec));
    }
    fndb.numDirs = static_cast<unsigned>(numDirectories);
    fndb.numFiles = static_cast<unsigned>(numFiles);
    fndb.depth = static_cast<unsigned>(deepestLevel);
    fndb.size = GetMemTop();
    AlignMem(FNDB_PAGESIZE);
    SetMem(0, &fndb, sizeof(fndb));

    // <fixme>
    bool unloaded = false;
    for (size_t i = 0; !unloaded && i < 100; ++i)
    {
      unloaded = SessionImpl::GetSession()->UnloadFilenameDatabaseInternal(rootIdx, chrono::seconds(0));
      if (!unloaded)
      {
        trace_fndb->WriteLine("core", "sleep for 1ms");
        this_thread::sleep_for(chrono::milliseconds(1));
      }
    }
    if (!unloaded)
    {
      MIKTEX_FATAL_ERROR(T_("fndb cannot be unloaded"));
    }
    // </fixme>
    
    FileStream streamFndb;
#if defined(MIKTEX_WINDOWS)
    chrono::time_point<chrono::high_resolution_clock> tryUntil = chrono::high_resolution_clock::now() + chrono::seconds(10);
    do
    {
      try
      {
        streamFndb.Attach(File::Open(fndbPath, FileMode::Create, FileAccess::Write, false));
      }
      catch (const SharingViolationException&)
      {
        if (chrono::high_resolution_clock::now() > tryUntil)
        {
          throw;
        }
      }
    } while (streamFndb.GetFile() == nullptr);
#else
    streamFndb.Attach(File::Open(fndbPath, FileMode::Create, FileAccess::Write, false));
#endif
    if (!File::TryLock(streamFndb.GetFile(), File::LockType::Exclusive, 1s))
    {
      MIKTEX_FATAL_ERROR_2(T_("Could not acquire exclusive lock."), "path", fndbPath.ToString());
    }
    streamFndb.Write(reinterpret_cast<const char*>(GetMemPointer()), GetMemTop());
    PathName changeFile = fndbPath;
    changeFile.SetExtension(MIKTEX_FNDB_CHANGE_FILE_SUFFIX);
    if (File::Exists(changeFile))
    {
      File::Delete(changeFile);
    }
    File::Unlock(streamFndb.GetFile());
    streamFndb.Close();
    trace_fndb->WriteLine("core", T_("fndb creation completed"));
    SessionImpl::GetSession()->RecordMaintenance();
    return true;
  }
  catch (const OperationCancelledException&)
  {
    trace_fndb->WriteLine("core", T_("fndb creation cancelled"));
    return false;
  }
}

bool Fndb::Create(const PathName& fndbPath, const PathName& rootPath, ICreateFndbCallback* callback)
{
  return Fndb::Create(fndbPath, rootPath, callback, true, false);
}

bool Fndb::Create(const PathName& fndbPath, const PathName& rootPath, ICreateFndbCallback* callback, bool enableStringPooling, bool storeFileNameInfo)
{
  FndbManager fndbmngr;

  if (!fndbmngr.Create(fndbPath, rootPath, callback, enableStringPooling, storeFileNameInfo))
  {
    return false;
  }

#if defined(MIKTEX_WINDOWS) && REPORT_EVENTS
  ReportMiKTeXEvent(EVENTLOG_INFORMATION_TYPE, MIKTEX_EVENT_FNDB_CREATED, fndbPath, rootPath, 0);
#endif

  return true;
}

bool Fndb::Refresh(const PathName& path, ICreateFndbCallback* callback)
{
  unsigned root = SessionImpl::GetSession()->DeriveTEXMFRoot(path);
  PathName pathFndbPath = SessionImpl::GetSession()->GetFilenameDatabasePathName(root);
  return Fndb::Create(pathFndbPath, SessionImpl::GetSession()->GetRootDirectoryPath(root), callback);
}

bool Fndb::Refresh(ICreateFndbCallback* callback)
{
  shared_ptr<SessionImpl> session = SessionImpl::GetSession();
  unsigned n = session->GetNumberOfTEXMFRoots();
  for (unsigned ord = 0; ord < n; ++ord)
  {
    if (session->IsAdminMode() && !session->IsCommonRootDirectory(ord))
    {
      // skipping user root directory
      continue;
    }
    if (!session->IsAdminMode() && session->IsCommonRootDirectory(ord) && !session->IsMiKTeXPortable())
    {
      // skipping common root directory
      continue;
    }
    PathName rootDirectory = session->GetRootDirectoryPath(ord);
    PathName pathFndbPath = session->GetFilenameDatabasePathName(ord);
    if (!Fndb::Create(pathFndbPath, rootDirectory, callback))
    {
      return false;
    }
  }
  return true;
}
