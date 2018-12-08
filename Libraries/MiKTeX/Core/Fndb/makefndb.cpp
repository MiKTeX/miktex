/* makefndb.cpp: creating the file name database

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

#include <fstream>
#include <thread>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Trace/Trace>

#include "internal.h"

#include "miktex/Core/AutoResource.h"
#include "miktex/Core/Directory.h"
#include "miktex/Core/Paths.h"
#include "miktex/Core/Registry.h"

#include "Session/SessionImpl.h"
#include "fndbmem.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace std;

const uint8_t null_byte = 0;

#define FN_MIKTEXIGNORE ".miktexignore"

struct FILENAMEINFO
{
  string FileName;
#if MIKTEX_FNDB_VERSION == 5
  string Directory;
#endif
  string Info;
};

struct COMPAREFILENAMEINFO
{
  bool operator()(const FILENAMEINFO& lhs, const FILENAMEINFO& rhs) const
  {
    return PathName::Compare(lhs.FileName, rhs.FileName) < 0;
  }
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

#if MIKTEX_FNDB_VERSION == 5
private:
  void CollectFiles(const PathName& parentPath, const PathName& folderName, vector<FILENAMEINFO>& fileNames);
#endif

#if MIKTEX_FNDB_VERSION == 4
private:
  FndbByteOffset ProcessFolder(FndbByteOffset foParent, const PathName& parentPath, const PathName& folderName, FndbByteOffset foFolderName);
#endif

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
#if MIKTEX_FNDB_VERSION == 5
  PathName directory = Utils::GetRelativizedPath(dirPath.GetData(), rootPath.GetData());
  directory = directory.ToUnix();
#endif
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
#if MIKTEX_FNDB_VERSION == 5
      filenameinfo.Directory = directory.ToString();
#endif
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

#if MIKTEX_FNDB_VERSION == 5
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
    if (!callback->OnProgress(currentLevel, path))
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
        filenameinfo.Directory = directory.ToString();
        filenameinfo.Info = infos[i];
        fileNames.push_back(filenameinfo);
      }
    }
  }

  if (!done)
  {
    ReadDirectory(path, subDirectoryNames, fileNames, true);
  }

  numDirectories += subDirectoryNames.size();

  sort(subDirectoryNames.begin(), subDirectoryNames.end(), StringComparerIgnoringCase());
  sort(fileNames.begin(), fileNames.end(), COMPAREFILENAMEINFO());

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
#endif

#if MIKTEX_FNDB_VERSION == 4
FndbByteOffset FndbManager::ProcessFolder(FndbByteOffset foParent, const PathName& parentPath, const PathName& folderName, FndbByteOffset foFolderName)
{
  const size_t cReservedEntries = 0;

  if (currentLevel > deepestLevel)
  {
    deepestLevel = currentLevel;
  }

  vector<string> subDirectoryNames;
  subDirectoryNames.reserve(40);

  vector<FILENAMEINFO> fileNames;
  fileNames.reserve(100);

  bool done = false;

  PathName path(parentPath, folderName);

  path.MakeAbsolute();

  if (callback != nullptr)
  {
    if (!callback->OnProgress(currentLevel, path))
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
        filenameinfo.Info = infos[i];
        fileNames.push_back(filenameinfo);
      }
    }
  }

  if (!done)
  {
    ReadDirectory(path, subDirectoryNames, fileNames, true);
  }

  numDirectories += subDirectoryNames.size();
  numFiles += fileNames.size();

  sort(subDirectoryNames.begin(), subDirectoryNames.end(), StringComparerIgnoringCase());
  sort(fileNames.begin(), fileNames.end(), COMPAREFILENAMEINFO());

  // store all names; build offset table
  vector<FndbByteOffset> vecfndboff;
  vecfndboff.reserve((storeFileNameInfo ? 2 : 1) * fileNames.size() + 2 * subDirectoryNames.size() + cReservedEntries);
  for (const FILENAMEINFO& fi : fileNames)
  {
    vecfndboff.push_back(PushBack(fi.FileName.c_str()));
  }
  for (const string& s : subDirectoryNames)
  {
    vecfndboff.push_back(PushBack(s.c_str()));
  }
  for (int n = 0; n < subDirectoryNames.size(); ++n)
  {
    vecfndboff.push_back(null_byte);
  }
  if (storeFileNameInfo)
  {
    for (const FILENAMEINFO& fi : fileNames)
    {
      vecfndboff.push_back(PushBack(fi.Info.c_str()));
    }
  }
  vecfndboff.insert(vecfndboff.end(), cReservedEntries, 0);

  // store directory data (excluding offsets)
  FileNameDatabaseDirectory dirdata;
  dirdata.Init();
  dirdata.foName = foFolderName;
  dirdata.foParent = foParent;
  dirdata.numSubDirs = subDirectoryNames.size();
  dirdata.numFiles = fileNames.size();
  dirdata.capacity = vecfndboff.size();
  AlignMem(sizeof(FndbByteOffset));
  FndbByteOffset foThis = PushBack(&dirdata, offsetof(FileNameDatabaseDirectory, table));

  if (vecfndboff.empty())
  {
    return foThis;
  }

  // reserve memory for offset table
  FndbByteOffset foOffsetTable = ReserveMem(vecfndboff.size() * sizeof(FndbByteOffset));

  // recurse into sub-directories and remember offsets
  PathName pathFolder(parentPath, folderName);
  size_t i = 0;
  ++currentLevel;
  for (const string& s : subDirectoryNames)
  {
    // RECURSION
    vecfndboff[dirdata.numFiles + dirdata.numSubDirs + i] = ProcessFolder(foThis, pathFolder, s, vecfndboff[dirdata.numFiles + i]);
    ++i;
  }
  --currentLevel;

  // store offset table
  SetMem(foOffsetTable, &vecfndboff[0], vecfndboff.size() * sizeof(FndbByteOffset));

  return foThis;
}
#endif

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
#if MIKTEX_FNDB_VERSION == 4
    if (callback == nullptr && FileIsOnROMedia(rootPath.GetData()))
    {
      fndb.flags |= FileNameDatabaseHeader::FndbFlags::Frozen;
    }
    if (storeFileNameInfo)
    {
      fndb.flags |= FileNameDatabaseHeader::FndbFlags::FileNameInfo;
    }
#endif
    numDirectories = 0;
    numFiles = 0;
    deepestLevel = 0;
    currentLevel = 0;
    this->callback = callback;
#if MIKTEX_FNDB_VERSION == 5
    AlignMem();
    fndb.foPath = ReserveMem(sizeof(FndbWord));
    vector<FILENAMEINFO> fileNames;
    CollectFiles(rootPath, CURRENT_DIRECTORY, fileNames);
    numFiles = fileNames.size();
    AlignMem();
    fndb.foTable = ReserveMem(fileNames.size() * sizeof(FileNameDatabaseRecord));
    AlignMem();
    SetMem(fndb.foPath, PushBack(rootPath.GetData()));
    for (size_t idx = 0; idx < fileNames.size(); ++idx)
    {
      FileNameDatabaseRecord rec;
      rec.foFileName = PushBack(fileNames[idx].FileName.c_str());
      rec.foDirectory = PushBack(fileNames[idx].Directory.c_str());
      rec.foInfo = PushBack(fileNames[idx].Info.c_str());
      rec.reserved = 0;
      SetMem(fndb.foTable + idx * sizeof(rec), &rec, sizeof(rec));
    }
#endif
#if MIKTEX_FNDB_VERSION == 4
    AlignMem();
    fndb.foPath = PushBack(rootPath.GetData());
    fndb.foTopDir = ProcessFolder(0, rootPath, CURRENT_DIRECTORY, fndb.foPath);
#endif
    fndb.numDirs = numDirectories;
    fndb.numFiles = numFiles;
    fndb.depth = deepestLevel;
#if MIKTEX_FNDB_VERSION == 5
    fndb.reserved = 0;
#endif
#if MIKTEX_FNDB_VERSION == 4
    fndb.timeStamp = static_cast<FndbWord>(time(nullptr)); // FIXME: 64-bit
#endif
    fndb.size = GetMemTop();
#if MIKTEX_FNDB_VERSION == 4
    if ((fndb.flags & FileNameDatabaseHeader::FndbFlags::Frozen) == 0)
    {
      size_t n = ((GetMemTop() + FNDB_EXTRA + 1) / FNDB_GRAN * FNDB_GRAN) - GetMemTop();
      ReserveMem(n);
    }
#endif
    AlignMem(FNDB_PAGESIZE);
    SetMem(0, &fndb, sizeof(fndb));
    // <fixme>
    bool unloaded = false;
    for (size_t i = 0; !unloaded && i < 100; ++i)
    {
      unloaded = SessionImpl::GetSession()->UnloadFilenameDatabaseInternal(rootIdx, true);
      if (!unloaded)
      {
        trace_fndb->WriteLine("core", "sleep 1");
        this_thread::sleep_for(chrono::milliseconds(1));
      }
    }
    if (!unloaded)
    {
      trace_error->WriteLine("core", T_("fndb cannot be unloaded"));
    }
    // </fixme>
    PathName directory = PathName(fndbPath).RemoveFileSpec();
    if (!Directory::Exists(directory))
    {
      Directory::Create(directory);
    }
    ofstream streamFndb = File::CreateOutputStream(fndbPath, ios_base::binary);
    streamFndb.write((const char*)GetMemPointer(), GetMemTop());
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
#if MIKTEX_FNDB_VERSION == 5
  return Fndb::Create(fndbPath, rootPath, callback, true, false);
#endif
#if MIKTEX_FNDB_VERSION == 4
  return Fndb::Create(fndbPath, rootPath, callback, false, false);
#endif
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
