/* PackageManagerImpl.cpp: MiKTeX Package Manager

   Copyright (C) 2001-2018 Christian Schenk

   This file is part of MiKTeX Package Manager.

   MiKTeX Package Manager is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Package Manager is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Package Manager; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "config.h"

#include <fstream>
#include <future>
#include <locale>
#include <stack>
#include <unordered_set>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>
#include <miktex/Core/Environment>
#include <miktex/Core/PathNameParser>
#include <miktex/Core/Registry>
#include <miktex/Core/TemporaryDirectory>
#include <miktex/Core/Uri>

#include <miktex/Trace/StopWatch>
#include <miktex/Trace/Trace>

#include "internal.h"
#include "PackageManagerImpl.h"
#include "PackageInstallerImpl.h"
#include "PackageIteratorImpl.h"
#include "TpmParser.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Extractor;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

using namespace MiKTeX::Packages::D6AAD62216146D44B580E92711724B78;

string PackageManagerImpl::proxyUser;
string PackageManagerImpl::proxyPassword;

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
bool PackageManagerImpl::localServer = false;
#endif

PackageManager::~PackageManager() noexcept
{
}

PackageManagerImpl::PackageManagerImpl(const PackageManager::InitInfo& initInfo) :
  trace_error(TraceStream::Open(MIKTEX_TRACE_ERROR, initInfo.traceCallback)),
  trace_mpm(TraceStream::Open(MIKTEX_TRACE_MPM, initInfo.traceCallback)),
  trace_stopwatch(TraceStream::Open(MIKTEX_TRACE_STOPWATCH, initInfo.traceCallback)),
  repositories(webSession)
{
  trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("initializing MPM library version {0}"), MIKTEX_COMPONENT_VERSION_STR));
}

PackageManagerImpl::~PackageManagerImpl()
{
  try
  {
    Dispose();
  }
  catch (const exception &)
  {
  }
}

shared_ptr<PackageManager> PackageManager::Create(const PackageManager::InitInfo& initInfo)
{
  return make_shared<PackageManagerImpl>(initInfo);
}

void PackageManagerImpl::Dispose()
{
  ClearAll();
  if (webSession != nullptr)
  {
    webSession->Dispose();
    webSession = nullptr;
  }
}

unique_ptr<PackageInstaller> PackageManagerImpl::CreateInstaller()
{
  return make_unique<PackageInstallerImpl>(shared_from_this());
}

unique_ptr<PackageIterator> PackageManagerImpl::CreateIterator()
{
  return make_unique<PackageIteratorImpl>(shared_from_this());
}

void PackageManagerImpl::LoadVariablePackageTable()
{
  // only load once
  if (commonVariablePackageTable != nullptr)
  {
    return;
  }

  commonVariablePackageTable = Cfg::Create();

  PathName pathCommonPackagesIni(session->GetSpecialPath(SpecialPath::CommonInstallRoot), MIKTEX_PATH_PACKAGES_INI);

  if (File::Exists(pathCommonPackagesIni))
  {
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("loading common variable package table ({0})"), Q_(pathCommonPackagesIni)));
    commonVariablePackageTable->Read(pathCommonPackagesIni);
  }

  commonVariablePackageTable->SetModified(false);

  PathName pathUserPackagesIni(session->GetSpecialPath(SpecialPath::UserInstallRoot), MIKTEX_PATH_PACKAGES_INI);

  if (!session->IsAdminMode() && (pathCommonPackagesIni.Canonicalize() != pathUserPackagesIni.Canonicalize()))
  {
    userVariablePackageTable = Cfg::Create();
    if (File::Exists(pathUserPackagesIni))
    {
      trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("loading user variable package table ({0})"), Q_(pathUserPackagesIni)));
      userVariablePackageTable->Read(pathUserPackagesIni);
    }
    userVariablePackageTable->SetModified(false);
  }
}

void PackageManagerImpl::FlushVariablePackageTable()
{
  if (commonVariablePackageTable != nullptr
    && commonVariablePackageTable->IsModified())
  {
    PathName pathPackagesIni(session->GetSpecialPath(SpecialPath::CommonInstallRoot), MIKTEX_PATH_PACKAGES_INI);
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("flushing common variable package table ({0})"), Q_(pathPackagesIni)));
    commonVariablePackageTable->Write(pathPackagesIni);
  }
  if (userVariablePackageTable != nullptr && userVariablePackageTable->IsModified())
  {
    PathName pathPackagesIni(session->GetSpecialPath(SpecialPath::UserInstallRoot), MIKTEX_PATH_PACKAGES_INI);
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("flushing user variable package table ({0})"), Q_(pathPackagesIni)));
    userVariablePackageTable->Write(pathPackagesIni);
  }
}

bool PackageManagerImpl::IsRemovable(const string& packageId)
{
  bool ret;
  LoadVariablePackageTable();
  string str;
  if (session->IsAdminMode())
  {
    // administrator can remove system-wide packages
    ret = GetCommonTimeInstalled(packageId) != 0;
  }
  else
  {
    // user can remove private packages
    if (session->GetSpecialPath(SpecialPath::CommonInstallRoot).Canonicalize() == session->GetSpecialPath(SpecialPath::UserInstallRoot).Canonicalize())
    {
      ret = GetTimeInstalled(packageId) != 0;
    }
    else
    {
      ret = GetUserTimeInstalled(packageId) != 0;
    }
  }
  return ret;
}

time_t PackageManagerImpl::GetUserTimeInstalled(const string& packageId)
{
  if (session->IsAdminMode())
  {
    MIKTEX_UNEXPECTED();
  }
  LoadVariablePackageTable();
  string str;
  if (userVariablePackageTable != nullptr && userVariablePackageTable->TryGetValue(packageId, "TimeInstalled", str))
  {
    return std::stoi(str);
  }
  else
  {
    return 0;
  }
}

time_t PackageManagerImpl::GetCommonTimeInstalled(const string& packageId)
{
  LoadVariablePackageTable();
  string str;
  if (commonVariablePackageTable != nullptr && commonVariablePackageTable->TryGetValue(packageId, "TimeInstalled", str))
  {
    return std::stoi(str);
  }
  else
  {
    return 0;
  }
}

time_t PackageManagerImpl::GetTimeInstalled(const string& packageId)
{
  LoadVariablePackageTable();
  string str;
  if ((!session->IsAdminMode() && userVariablePackageTable != nullptr && userVariablePackageTable->TryGetValue(packageId, "TimeInstalled", str))
    || commonVariablePackageTable->TryGetValue(packageId, "TimeInstalled", str))
  {
    return std::stoi(str);
  }
  else
  {
    return 0;
  }
}

bool PackageManagerImpl::IsPackageInstalled(const string& packageId)
{
  return GetTimeInstalled(packageId) > 0;
}

bool PackageManagerImpl::IsPackageObsolete(const string& packageId)
{
  LoadVariablePackageTable();
  string str;
  if ((!session->IsAdminMode()
    && userVariablePackageTable != nullptr
    && userVariablePackageTable->TryGetValue(packageId, "Obsolete", str))
    || commonVariablePackageTable->TryGetValue(packageId, "Obsolete", str))
  {
    return std::stoi(str) != 0;
  }
  else
  {
    return false;
  }
}

void PackageManagerImpl::DeclarePackageObsolete(const string& packageId, bool obsolete)
{
  LoadVariablePackageTable();
  if (session->IsAdminMode() || userVariablePackageTable == nullptr)
  {
    commonVariablePackageTable->PutValue(packageId, "Obsolete", (obsolete ? "1" : "0"));
  }
  else
  {
    userVariablePackageTable->PutValue(packageId, "Obsolete", (obsolete ? "1" : "0"));
  }
}

void PackageManagerImpl::SetTimeInstalled(const string& packageId, time_t timeInstalled)
{
  LoadVariablePackageTable();
  if (session->IsAdminMode() || userVariablePackageTable == nullptr)
  {
    if (timeInstalled == 0)
    {
      commonVariablePackageTable->DeleteKey(packageId);
    }
    else
    {
      commonVariablePackageTable->PutValue(packageId, "TimeInstalled", std::to_string(timeInstalled));
    }
  }
  else
  {
    if (timeInstalled == 0)
    {
      userVariablePackageTable->DeleteKey(packageId);
    }
    else
    {
      userVariablePackageTable->PutValue(packageId, "TimeInstalled", std::to_string(timeInstalled));
    }
  }
}

void PackageManagerImpl::SetReleaseState(const string& packageId, RepositoryReleaseState releaseState)
{
  LoadVariablePackageTable();
  if (session->IsAdminMode() || userVariablePackageTable == nullptr)
  {
    commonVariablePackageTable->PutValue(packageId, "ReleaseState", releaseState == RepositoryReleaseState::Next ? "next" : releaseState == RepositoryReleaseState::Stable ? "stable" : "");
  }
  else
  {
    userVariablePackageTable->PutValue(packageId, "ReleaseState", releaseState == RepositoryReleaseState::Next ? "next" : releaseState == RepositoryReleaseState::Stable ? "stable" : "");
  }
}

RepositoryReleaseState PackageManagerImpl::GetReleaseState(const string& packageId)
{
  LoadVariablePackageTable();
  string str;
  if ((!session->IsAdminMode() && userVariablePackageTable != nullptr && userVariablePackageTable->TryGetValue(packageId, "ReleaseState", str))
    || commonVariablePackageTable->TryGetValue(packageId, "ReleaseState", str))
  {
    if (str == "stable")
    {
      return RepositoryReleaseState::Stable;
    }
    else if (str == "next")
    {
      return RepositoryReleaseState::Next;
    }
  }
  return RepositoryReleaseState::Unknown;
}

void PackageManagerImpl::IncrementFileRefCounts(const vector<string>& files)
{
  for (const string& file : files)
  {
    ++installedFileInfoTable[file].refCount;
#if POLLUTE_THE_DEBUG_STREAM
    if (installedFileInfoTable[file].refCount >= 2)
    {
      trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("{0}: ref count > 1"), Q_(file)));
    }
#endif
  }
}

void PackageManagerImpl::IncrementFileRefCounts(const string& packageId)
{
  NeedInstalledFileInfoTable();
  const PackageInfo& pi = packageTable[packageId];
  IncrementFileRefCounts(pi.runFiles);
  IncrementFileRefCounts(pi.docFiles);
  IncrementFileRefCounts(pi.sourceFiles);
}

PackageInfo* PackageManagerImpl::DefinePackage(const string& packageId, const PackageInfo& packageInfo)
{
  pair<PackageDefinitionTable::iterator, bool> p = packageTable.insert(make_pair(packageId, packageInfo));
  p.first->second.id = packageId;
  if (session->IsMiKTeXDirect())
  {
    // installed from the start
    p.first->second.isRemovable = false;
    p.first->second.isObsolete = false;
    p.first->second.timeInstalled = packageInfo.timePackaged;
  }
  else
  {
    p.first->second.isRemovable = IsRemovable(packageId);
    p.first->second.isObsolete = IsPackageObsolete(packageId);
    p.first->second.timeInstalled = GetTimeInstalled(packageId);
    if (p.first->second.IsInstalled())
    {
      p.first->second.releaseState = GetReleaseState(packageId);
    }
  }
  return &(p.first->second);
}

void PackageManagerImpl::LoadAllPackageManifests(const PathName& packageManifestsPath)
{
  trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("loading all package manifests ({0})"), Q_(packageManifestsPath)));

#if defined(MIKTEX_USE_ZZDB3)
  if (!File::Exists(packageManifestsPath))
  {
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("file {0} does not exist"), Q_(packageManifestsPath)));
    return;
  }

  unique_ptr<Cfg> cfg = Cfg::Create();
  cfg->Read(packageManifestsPath);

  unsigned count = 0;
  for (auto key : *cfg)
  {
    // ignore redefinition
    if (packageTable.find(key->GetName()) != packageTable.end())
    {
      continue;
    }
    PackageInfo packageInfo = GetPackageManifest(*cfg, key->GetName(), TEXMF_PREFIX_DIRECTORY);

#if IGNORE_OTHER_SYSTEMS
    string targetSystems = packageInfo.targetSystem;
    if (targetSystems != "" && !StringUtil::Contains(targetSystems.c_str(), MIKTEX_SYSTEM_TAG))
    {
      trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("{0}: ignoring {1} package"), packageInfo.id, targetSystems));
      continue;
    }
#endif

    count += 1;

    // insert into database
    PackageInfo* insertedPackageInfo = DefinePackage(packageInfo.id, packageInfo);

    // increment file ref counts, if package is installed
    if (IsValidTimeT(insertedPackageInfo->timeInstalled))
    {
      IncrementFileRefCounts(insertedPackageInfo->runFiles);
      IncrementFileRefCounts(insertedPackageInfo->docFiles);
      IncrementFileRefCounts(insertedPackageInfo->sourceFiles);
    }
  }
#else
  if (!Directory::Exists(packageManifestsPath))
  {
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("directory {0} does not exist"), Q_(packageManifestsPath)));
    return;
  }

  unique_ptr<DirectoryLister> dirLister = DirectoryLister::Open(packageManifestsPath, "*" MIKTEX_PACKAGE_MANIFEST_FILE_SUFFIX);

  vector<future<PackageInfo>> futurePackageInfoTable;

  // parse package manifest files
  if (((int)ASYNC_LAUNCH_POLICY & (int)launch::async) != 0)
  {
    const size_t maxPackageFiles = 4000;
    File::SetMaxOpen(maxPackageFiles);
  }
  unsigned count = 0;
  DirectoryEntry direntry;
  while (dirLister->GetNext(direntry))
  {
    PathName name(direntry.name);

    // get package ID
    string packageId = name.GetFileNameWithoutExtension().ToString();

    // ignore redefinition
    if (packageTable.find(packageId) != packageTable.end())
    {
#if 0
      trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("{0}: ignoring redefinition"), packageId));
#endif
      continue;
    }

    // parse package manifest file
    futurePackageInfoTable.push_back(async(ASYNC_LAUNCH_POLICY, [](const PathName& path)
    {
      unique_ptr<TpmParser> tpmParser = TpmParser::Create();
      tpmParser->Parse(path);
      return tpmParser->GetPackageInfo();
    }, PathName(packageManifestsPath, name)));
  }
  dirLister->Close();

  for (future<PackageInfo>& fpi : futurePackageInfoTable)
  {
    PackageInfo packageInfo = fpi.get();

#if IGNORE_OTHER_SYSTEMS
    string targetSystems = packageInfo.targetSystem;
    if (targetSystems != "" && !StringUtil::Contains(targetSystems.c_str(), MIKTEX_SYSTEM_TAG))
    {
      trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("{0}: ignoring {1} package"), packageInfo.id, targetSystems));
      continue;
    }
#endif

#if POLLUTE_THE_DEBUG_STREAM
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("  adding {0}"), packageInfo.id));
#endif

    count += 1;

    // insert into database
    PackageInfo* insertedPackageInfo = DefinePackage(packageInfo.id, packageInfo);

    // increment file ref counts, if package is installed
    if (IsValidTimeT(insertedPackageInfo->timeInstalled))
    {
      IncrementFileRefCounts(insertedPackageInfo->runFiles);
      IncrementFileRefCounts(insertedPackageInfo->docFiles);
      IncrementFileRefCounts(insertedPackageInfo->sourceFiles);
    }
  }
#endif

  trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("found {0} package manifests"), count));

  // determine dependencies
  for (auto& kv : packageTable)
  {
    PackageInfo& pkg = kv.second;
    // FIXME
    time_t timeInstalledMin = static_cast<time_t>(0xffffffffffffffffULL);
    time_t timeInstalledMax = 0;
    for (const string& req : pkg.requiredPackages)
    {
      PackageDefinitionTable::iterator it3 = packageTable.find(req);
      if (it3 == packageTable.end())
      {
        trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("dependancy problem: {0} is required by {1}"), req, pkg.id));
      }
      else
      {
        it3->second.requiredBy.push_back(pkg.id);
        if (it3->second.timeInstalled < timeInstalledMin)
        {
          timeInstalledMin = it3->second.timeInstalled;
        }
        if (it3->second.timeInstalled > timeInstalledMax)
        {
          timeInstalledMax = it3->second.timeInstalled;
        }
      }
    }
    if (timeInstalledMin > 0)
    {
      if (pkg.IsPureContainer() || (pkg.IsInstalled() && pkg.timeInstalled < timeInstalledMax))
      {
        pkg.timeInstalled = timeInstalledMax;
      }
    }
  }

  // create "Obsolete" container
  PackageInfo piObsolete;
  piObsolete.id = "_miktex-obsolete";
  piObsolete.displayName = T_("Obsolete");
  piObsolete.title = T_("Obsolete packages");
  piObsolete.description = T_("Packages that were removed from the MiKTeX package repository.");
  for (auto& kv : packageTable)
  {
    PackageInfo& pkg = kv.second;
    if (!pkg.IsContained() && !pkg.IsContainer() && IsPackageObsolete(pkg.id))
    {
      piObsolete.requiredPackages.push_back(pkg.id);
      pkg.requiredBy.push_back(piObsolete.id);
    }
  }
  if (!piObsolete.requiredPackages.empty())
  {
    // insert "Obsolete" into the database
    DefinePackage(piObsolete.id, piObsolete);
  }

  // create "Uncategorized" container
  PackageInfo piOther;
  piOther.id = "_miktex-all-the-rest";
  piOther.displayName = T_("Uncategorized");
  piOther.title = T_("Uncategorized packages");
  for (auto& kv : packageTable)
  {
    PackageInfo& pkg = kv.second;
    if (!pkg.IsContained() && !pkg.IsContainer())
    {
      piOther.requiredPackages.push_back(pkg.id);
      pkg.requiredBy.push_back(piOther.id);
    }
  }
  if (!piOther.requiredPackages.empty())
  {
    // insert "Other" into the database
    DefinePackage(piOther.id, piOther);
  }
}

#if MIKTEX_USE_ZZDB3
void PackageManagerImpl::NeedPackageManifestsIni()
{
  PathName existingPackageManifestsIni = session->GetSpecialPath(SpecialPath::InstallRoot) / MIKTEX_PATH_PACKAGE_MANIFESTS_INI;
  if (File::Exists(existingPackageManifestsIni))
  {
    return;
  }
  PathName tpmDir = session->GetSpecialPath(SpecialPath::InstallRoot) / MIKTEX_PATH_PACKAGE_MANIFEST_DIR;
  if (Directory::Exists(tpmDir))
  {
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format("starting migration: {} -> {}", tpmDir, existingPackageManifestsIni));
    unique_ptr<Cfg> cfgExisting = Cfg::Create();
    unique_ptr<DirectoryLister> lister = DirectoryLister::Open(tpmDir);
    DirectoryEntry direntry;
    unique_ptr<TpmParser> tpmParser = TpmParser::Create();
    int count = 0;
    while (lister->GetNext(direntry))
    {
      PathName name(direntry.name);
      if (direntry.isDirectory || !name.HasExtension(MIKTEX_PACKAGE_MANIFEST_FILE_SUFFIX))
      {
        continue;
      }
      tpmParser->Parse(tpmDir / name);
      PackageInfo packageInfo = tpmParser->GetPackageInfo();
      PackageManager::PutPackageManifest(*cfgExisting, packageInfo, packageInfo.timePackaged);
      count++;
    }
    cfgExisting->Write(existingPackageManifestsIni);
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format("successfully migrated {} package manifest files", count));
  }
}
#endif

void PackageManagerImpl::LoadAllPackageManifests()
{
  if (loadedAllPackageManifests)
  {
    // we do this once
    return;
  }
#if defined(MIKTEX_USE_ZZDB3)
  NeedPackageManifestsIni();
  PathName userPath = session->GetSpecialPath(SpecialPath::UserInstallRoot) / MIKTEX_PATH_PACKAGE_MANIFESTS_INI;
  PathName commonPath = session->GetSpecialPath(SpecialPath::CommonInstallRoot) / MIKTEX_PATH_PACKAGE_MANIFESTS_INI;
#else
  PathName userPath = session->GetSpecialPath(SpecialPath::UserInstallRoot) / MIKTEX_PATH_PACKAGE_MANIFEST_DIR;
  PathName commonPath = session->GetSpecialPath(SpecialPath::CommonInstallRoot) / MIKTEX_PATH_PACKAGE_MANIFEST_DIR;
#endif
  if (!session->IsAdminMode())
  {
    LoadAllPackageManifests(userPath);
    if (userPath.Canonicalize() == commonPath.Canonicalize())
    {
      loadedAllPackageManifests = true;
      return;
    }
  }
  LoadAllPackageManifests(commonPath);
  loadedAllPackageManifests = true;
}

void PackageManagerImpl::LoadDatabase(const PathName& path, bool isArchive)
{
  // get the full path name
  PathName absPath(path);
  absPath.MakeAbsolute();

  unique_ptr<TemporaryDirectory> tempDir;

  PathName packageManifestsPath;

  if (!isArchive)
  {
    packageManifestsPath = absPath;
  }
  else
  {
    // create temporary directory
    tempDir = TemporaryDirectory::Create();

    // extract from archive
    unique_ptr<MiKTeX::Extractor::Extractor> extractor(MiKTeX::Extractor::Extractor::CreateExtractor(DB_ARCHIVE_FILE_TYPE));
    extractor->Extract(absPath, tempDir->GetPathName());

#if MIKTEX_USE_ZZDB3
    packageManifestsPath = tempDir->GetPathName() / MIKTEX_PACKAGE_MANIFESTS_INI_FILENAME;
#else
    packageManifestsPath = tempDir->GetPathName();
#endif
  }

  // read package manifest files
  LoadAllPackageManifests(packageManifestsPath);

  loadedAllPackageManifests = true;
}

void PackageManagerImpl::ClearAll()
{
  packageTable.clear();
  installedFileInfoTable.clear();
  if (commonVariablePackageTable != nullptr)
  {
    commonVariablePackageTable = nullptr;
  }
  if (userVariablePackageTable != nullptr)
  {
    userVariablePackageTable = nullptr;
  }
  loadedAllPackageManifests = false;
}

void PackageManagerImpl::UnloadDatabase()
{
  ClearAll();
}

PackageInfo* PackageManagerImpl::TryGetPackageInfo(const string& packageId)
{
#if MIKTEX_USE_ZZDB3
  LoadAllPackageManifests();
  PackageDefinitionTable::iterator it = packageTable.find(packageId);
  return it == packageTable.end() ? nullptr : &it->second;
#else
  PackageDefinitionTable::iterator it = packageTable.find(packageId);
  if (it != packageTable.end())
  {
    return &it->second;
  }
  if (loadedAllPackageManifests)
  {
    return nullptr;
  }
  PathName pathPackageManifestFile;
  bool havePackageInfoFile = false;
  if (!session->IsAdminMode())
  {
    pathPackageManifestFile = session->GetSpecialPath(SpecialPath::UserInstallRoot);
    pathPackageManifestFile /= MIKTEX_PATH_PACKAGE_MANIFEST_DIR;
    pathPackageManifestFile /= packageId;
    pathPackageManifestFile.AppendExtension(MIKTEX_PACKAGE_MANIFEST_FILE_SUFFIX);
    havePackageInfoFile = File::Exists(pathPackageManifestFile);
  }
  if (!havePackageInfoFile)
  {
    pathPackageManifestFile = session->GetSpecialPath(SpecialPath::CommonInstallRoot);
    pathPackageManifestFile /= MIKTEX_PATH_PACKAGE_MANIFEST_DIR;
    pathPackageManifestFile /= packageId;
    pathPackageManifestFile.AppendExtension(MIKTEX_PACKAGE_MANIFEST_FILE_SUFFIX);
    havePackageInfoFile = File::Exists(pathPackageManifestFile);
  }
  if (!havePackageInfoFile)
  {
    return nullptr;
  }
  unique_ptr<TpmParser> tpmParser = TpmParser::Create();
  tpmParser->Parse(pathPackageManifestFile);
#if IGNORE_OTHER_SYSTEMS
  string targetSystems = tpmParser->GetPackageInfo().targetSystem;
  if (targetSystems != "" && !StringUtil::Contains(targetSystems.c_str(), MIKTEX_SYSTEM_TAG))
  {
    return nullptr;
  }
#endif
  return DefinePackage(packageId, tpmParser->GetPackageInfo());
#endif
}

bool PackageManagerImpl::TryGetPackageInfo(const string& packageId, PackageInfo& packageInfo)
{
  PackageInfo* packageInfoOrNull = TryGetPackageInfo(packageId);
  if (packageInfoOrNull == nullptr)
  {
    return false;
  }
  else
  {
    packageInfo = *packageInfoOrNull;
    return true;
  }
}

PackageInfo PackageManagerImpl::GetPackageInfo(const string& packageId)
{
  const PackageInfo* packageInfoOrNull = TryGetPackageInfo(packageId);
  if (packageInfoOrNull == nullptr)
  {
    MIKTEX_FATAL_ERROR_2(T_("The requested package is unknown."), "name", packageId);
  }
  return *packageInfoOrNull;
}

unsigned long PackageManagerImpl::GetFileRefCount(const PathName& path)
{
  NeedInstalledFileInfoTable();
  InstalledFileInfoTable::const_iterator it = installedFileInfoTable.find(path.GetData());
  if (it == installedFileInfoTable.end())
  {
    return 0;
  }
  return it->second.refCount;
}

void PackageManagerImpl::NeedInstalledFileInfoTable()
{
  LoadAllPackageManifests();
}

bool PackageManager::TryGetRemotePackageRepository(string& url, RepositoryReleaseState& repositoryReleaseState)
{
  shared_ptr<Session> session = Session::Get();
  repositoryReleaseState = RepositoryReleaseState::Unknown;
  if (session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REMOTE_REPOSITORY, url))
  {
    string str;
    if (session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REPOSITORY_RELEASE_STATE, str))
    {
      if (str == "stable")
      {
        repositoryReleaseState = RepositoryReleaseState::Stable;
      }
      else if (str == "next")
      {
        repositoryReleaseState = RepositoryReleaseState::Next;
      }
    }
    return true;
  }
  return Utils::GetEnvironmentString(MIKTEX_ENV_REPOSITORY, url)
    && (PackageRepositoryDataStore::DetermineRepositoryType(url) == RepositoryType::Remote);
}

string PackageManager::GetRemotePackageRepository(RepositoryReleaseState& repositoryReleaseState)
{
  string url;
  if (!TryGetRemotePackageRepository(url, repositoryReleaseState))
  {
    MIKTEX_UNEXPECTED();
  }
  return url;
}

void PackageManager::SetRemotePackageRepository(const string& url, RepositoryReleaseState repositoryReleaseState)
{
  shared_ptr<Session> session = Session::Get();
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REMOTE_REPOSITORY, url);
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REPOSITORY_RELEASE_STATE, repositoryReleaseState == RepositoryReleaseState::Stable ? "stable" : (repositoryReleaseState == RepositoryReleaseState::Next ? "next" : "unknown"));
}

bool PackageManager::TryGetLocalPackageRepository(PathName& path)
{
  shared_ptr<Session> session = Session::Get();
  string str;
  if (session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_LOCAL_REPOSITORY, str))
  {
    path = str;
    return true;
  }
  else if (Utils::GetEnvironmentString(MIKTEX_ENV_REPOSITORY, str) && (PackageRepositoryDataStore::DetermineRepositoryType(str) == RepositoryType::Local))
  {
    path = str;
    return true;
  }
  else
  {
    return false;
  }
}

PathName PackageManager::GetLocalPackageRepository()
{
  PathName path;
  if (!TryGetLocalPackageRepository(path))
  {
    MIKTEX_UNEXPECTED();
  }
  return path;
}

void PackageManager::SetLocalPackageRepository(const PathName& path)
{
  Session::Get()->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_LOCAL_REPOSITORY, path.ToString());
}

bool PackageManager::TryGetMiKTeXDirectRoot(PathName& path)
{
  shared_ptr<Session> session = Session::Get();
  string str;
  if (session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_MIKTEXDIRECT_ROOT, str))
  {
    path = str;
    return true;
  }
  else if (Utils::GetEnvironmentString(MIKTEX_ENV_REPOSITORY, str) && (PackageRepositoryDataStore::DetermineRepositoryType(str) == RepositoryType::MiKTeXDirect))
  {
    path = str;
    return true;
  }
  else
  {
    return false;
  }
}

PathName PackageManager::GetMiKTeXDirectRoot()
{
  PathName path;
  if (!TryGetMiKTeXDirectRoot(path))
  {
    MIKTEX_UNEXPECTED();
  }
  return path;
}

void PackageManager::SetMiKTeXDirectRoot(const PathName& path)
{
  shared_ptr<Session> session = Session::Get();
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_MIKTEXDIRECT_ROOT, path.ToString());
}

RepositoryInfo PackageManager::GetDefaultPackageRepository()
{
  RepositoryInfo result;
  shared_ptr<Session> session = Session::Get();
  string str;
  if (session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REPOSITORY_TYPE, str))
  {
    if (str == "remote")
    {
      result.url = GetRemotePackageRepository(result.releaseState);
      result.type = RepositoryType::Remote;
    }
    else if (str == "local")
    {
      result.url = GetLocalPackageRepository().ToString();
      result.type = RepositoryType::Local;
    }
    else if (str == "direct")
    {
      result.url = GetMiKTeXDirectRoot().ToString();
      result.type = RepositoryType::MiKTeXDirect;
    }
    else
    {
      MIKTEX_UNEXPECTED();
    }
  }
  else if (Utils::GetEnvironmentString(MIKTEX_ENV_REPOSITORY, result.url))
  {
    result.type = PackageRepositoryDataStore::DetermineRepositoryType(result.url);
  }
  else
  {
    result.url = "";
    result.type = RepositoryType::Remote;
  }
  return result;
}

bool PackageManager::TryGetDefaultPackageRepository(RepositoryType& repositoryType, RepositoryReleaseState& repositoryReleaseState, string& urlOrPath)
{
  RepositoryInfo defaultRepository = GetDefaultPackageRepository();
  repositoryType = defaultRepository.type;
  repositoryReleaseState = defaultRepository.releaseState;
  urlOrPath = defaultRepository.url;
  return true;
}

void PackageManager::SetDefaultPackageRepository(const RepositoryInfo& repository)
{
  shared_ptr<Session> session = Session::Get();
  string repositoryTypeStr;
  switch (repository.type)
  {
  case RepositoryType::MiKTeXDirect:
    repositoryTypeStr = "direct";
    SetMiKTeXDirectRoot(repository.url);
    break;
  case RepositoryType::Local:
    repositoryTypeStr = "local";
    SetLocalPackageRepository(repository.url);
    break;
  case RepositoryType::Remote:
    repositoryTypeStr = "remote";
    SetRemotePackageRepository(repository.url, repository.releaseState);
    break;
  default:
    MIKTEX_UNEXPECTED();
  }
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REPOSITORY_TYPE, repositoryTypeStr);
}

void PackageManager::SetDefaultPackageRepository(RepositoryType repositoryType, RepositoryReleaseState repositoryReleaseState, const string& urlOrPath)
{
  shared_ptr<Session> session = Session::Get();
  RepositoryInfo repository;
  repository.type = repositoryType != RepositoryType::Unknown ? repositoryType : PackageRepositoryDataStore::DetermineRepositoryType(urlOrPath);
  repository.releaseState = repositoryReleaseState;
  repository.url = urlOrPath;
  SetDefaultPackageRepository(repository);
}

namespace {

  typedef unordered_set<string, hash_path, equal_path> SubDirectoryTable;

  struct DirectoryInfo
  {
    SubDirectoryTable subDirectoryNames;
    vector<string> fileNames;
    vector<string> packageNames;
  };

  typedef unordered_map<string, DirectoryInfo, hash_path, equal_path> DirectoryInfoTable;

  // directory info table: written by
  // PackageManagerImpl::CreateMpmFndb(); read by
  // PackageManagerImpl::ReadDirectory()
  DirectoryInfoTable directoryInfoTable;

}

MPMSTATICFUNC(void) RememberFileNameInfo(const string& prefixedFileName, const string& packageName)
{
  shared_ptr<Session> session = Session::Get();

  string fileName;

  // ignore non-texmf files
  if (!PackageManager::StripTeXMFPrefix(prefixedFileName, fileName))
  {
    return;
  }

  PathNameParser pathtok(fileName);

  if (!pathtok)
  {
    return;
  }

  // initialize root path: "//MiKTeX/[MPM]"
  PathName path = session->GetMpmRootPath();
  //  path += CURRENT_DIRECTORY;

  // s1: current path name component
  string s1 = *pathtok;
  ++pathtok;

  // name: file name component
  string name = s1;

  while (pathtok)
  {
    string s2 = *pathtok;
    ++pathtok;
    directoryInfoTable[path.GetData()].subDirectoryNames.insert(s1);
    name = s2;
#if defined(MIKTEX_WINDOWS)
    // make sure the the rest of the path contains slashes (not
    // backslashes)
    path.AppendAltDirectoryDelimiter();
#else
    path.AppendDirectoryDelimiter();
#endif
    path /= s1;
    s1 = s2;
  }

  DirectoryInfo& directoryInfo = directoryInfoTable[path.ToString()];
  directoryInfo.fileNames.push_back(name);
  directoryInfo.packageNames.push_back(packageName);
}

bool PackageManagerImpl::ReadDirectory(const PathName& path, vector<string>& subDirNames, vector<string>& fileNames, vector<string>& fileNameInfos)
{
  const DirectoryInfo& directoryInfo = directoryInfoTable[path.ToString()];
  for (const string& name : directoryInfo.subDirectoryNames)
  {
    subDirNames.push_back(name);
  }
  fileNames = directoryInfo.fileNames;
  fileNameInfos = directoryInfo.packageNames;
  return true;
}

bool PackageManagerImpl::OnProgress(unsigned level, const PathName& directory)
{
  UNUSED_ALWAYS(level);
  UNUSED_ALWAYS(directory);
  return true;
}

void PackageManagerImpl::CreateMpmFndb()
{
  LoadAllPackageManifests();

  // collect the file names
  for (const auto& kv : packageTable)
  {
    const PackageInfo& pi = kv.second;
    for (const string& file : pi.runFiles)
    {
      RememberFileNameInfo(file, pi.id);
    }
    for (const string& file : pi.docFiles)
    {
      RememberFileNameInfo(file, pi.id);
    }
    for (const string& file : pi.sourceFiles)
    {
      RememberFileNameInfo(file, pi.id);
    }
  }

  // create the database
  Fndb::Create(session->GetMpmDatabasePathName().GetData(), session->GetMpmRootPath().GetData(), this, true, true);

  // free memory
  directoryInfoTable.clear();
}

void PackageManagerImpl::GetAllPackageDefinitions(vector<PackageInfo>& packages)
{
  LoadAllPackageManifests();
  for (const auto& kv : packageTable)
  {
    packages.push_back(kv.second);
  }
}

InstalledFileInfo* PackageManagerImpl::GetInstalledFileInfo(const char* lpszPath)
{
  LoadAllPackageManifests();
  InstalledFileInfoTable::iterator it = installedFileInfoTable.find(lpszPath);
  if (it == installedFileInfoTable.end())
  {
    return nullptr;
  }
  return &it->second;
}

bool PackageManager::IsLocalPackageRepository(const PathName& path)
{
  if (!Directory::Exists(path))
  {
    return false;
  }

  // local mirror of remote package repository?
  PathName file1 = PathName(path, MIKTEX_REPOSITORY_MANIFEST_ARCHIVE_FILE_NAME);
#if defined(MIKTEX_USE_ZZDB3)
  PathName file2 = PathName(path, MIKTEX_PACKAGE_MANIFESTS_ARCHIVE_FILE_NAME);
#else
  PathName file2 = PathName(path, MIKTEX_TPM_ARCHIVE_FILE_NAME);
#endif
  if (File::Exists(file1) && File::Exists(file2))
  {
    return true;
  }

  return false;
}

PackageInfo PackageManager::ReadPackageManifestFile(const PathName& path, const string& texmfPrefix)
{
  unique_ptr<TpmParser> tpmParser = TpmParser::Create();
  tpmParser->Parse(path, texmfPrefix);
  return tpmParser->GetPackageInfo();
}

class XmlWriter
{
public:
  XmlWriter(const PathName& path) :
    stream(File::CreateOutputStream(path))
  {
    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << "\n";
  }

public:
  void Close()
  {
    stream.close();
  }

public:
  void StartElement(const string& name)
  {
    if (freshElement)
    {
      stream << '>';
    }
    stream << fmt::format("<{}", name);
    freshElement = true;
    elements.push(name);
  }

public:
  void AddAttribute(const string& name, const string& value)
  {
    stream << fmt::format(" {}=\"{}\"", name, value);
  }

public:
  void EndElement()
  {
    if (elements.empty())
    {
      MIKTEX_UNEXPECTED();
    }
    if (freshElement)
    {
      stream << "/>";
      freshElement = false;
    }
    else
    {
      stream << fmt::format("</{}>", elements.top());
    }
    elements.pop();
  }

public:
  void EndAllElements()
  {
    while (!elements.empty())
    {
      EndElement();
    }
  }

public:
  void Text(const string& text)
  {
    if (freshElement)
    {
      stream << '>';
      freshElement = false;
    }
    for (const char& ch : text)
    {
      switch (ch)
      {
      case '&':
        stream << "&amp;";
        break;
      case '<':
        stream << "&lt;";
        break;
      case '>':
        stream << "&gt;";
        break;
      default:
        stream << ch;
        break;
      }
    }
  }

private:
  ofstream stream;

private:
  stack<string> elements;

private:
  bool freshElement = false;
};

void PackageManager::WritePackageManifestFile(const PathName& path, const PackageInfo& packageInfo, time_t timePackaged)
{
  XmlWriter xml(path);

  // create "rdf:Description" node
  xml.StartElement("rdf:RDF");
  xml.AddAttribute("xmlns:rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
  xml.AddAttribute("xmlns:TPM", "http://texlive.dante.de/");
  xml.StartElement("rdf:Description");
  string about("http://www.miktex.org/packages/");
  about += packageInfo.id;
  xml.AddAttribute("about", about);

  // create "TPM:Name" node
  xml.StartElement("TPM:Name");
  xml.Text(packageInfo.displayName);
  xml.EndElement();

  // create "TPM:Creator" node
  xml.StartElement("TPM:Creator");
  xml.Text("mpc");
  xml.EndElement();

  // create "TPM:Title" node
  xml.StartElement("TPM:Title");
  xml.Text(packageInfo.title);
  xml.EndElement();

  // create "TPM:Version" node
  xml.StartElement("TPM:Version");
  xml.Text(packageInfo.version);
  xml.EndElement();

  // create "TPM:TargetSystem" node
  xml.StartElement("TPM:TargetSystem");
  xml.Text(packageInfo.targetSystem);
  xml.EndElement();

  // create "TPM:Description" node
  xml.StartElement("TPM:Description");
  xml.Text(packageInfo.description);
  xml.EndElement();


  // create "TPM:RunFiles" node
  if (!packageInfo.runFiles.empty())
  {
    xml.StartElement("TPM:RunFiles");
    xml.AddAttribute("size", std::to_string(packageInfo.sizeRunFiles));
    bool start = true;
    for (const string& file : packageInfo.runFiles)
    {
      if (start)
      {
        start = false;
      }
      else
      {
        xml.Text(" ");
      }      
      xml.Text(PathName(file).ToDos().ToString());
    }
    xml.EndElement();
  }

  // create "TPM:DocFiles" node
  if (!packageInfo.docFiles.empty())
  {
    xml.StartElement("TPM:DocFiles");
    xml.AddAttribute("size", std::to_string(packageInfo.sizeDocFiles));
    bool start = true;
    for (const string& file : packageInfo.docFiles)
    {
      if (start)
      {
        start = false;
      }
      else
      {
        xml.Text(" ");
      }      
      xml.Text(PathName(file).ToDos().ToString());
    }
    xml.EndElement();
  }

  // create "TPM:SourceFiles" node
  if (!packageInfo.sourceFiles.empty())
  {
    xml.StartElement("TPM:SourceFiles");
    xml.AddAttribute("size", std::to_string(packageInfo.sizeSourceFiles));
    bool start = true;
    for (const string& file : packageInfo.sourceFiles)
    {
      if (start)
      {
        start = false;
      }
      else
      {
        xml.Text(" ");
      }      
      xml.Text(PathName(file).ToDos().ToString());
    }
    xml.EndElement();
  }

  // create "TPM:Requires" node
  if (!packageInfo.requiredPackages.empty())
  {
    xml.StartElement("TPM:Requires");
    for (const string& req : packageInfo.requiredPackages)
    {
      xml.StartElement("TPM:Package");
      xml.AddAttribute("name", req);
      xml.EndElement();
    }
    xml.EndElement();
  }

  // create "TPM:TimePackaged" node
  if (timePackaged != 0)
  {
    xml.StartElement("TPM:TimePackaged");
    xml.Text(std::to_string(timePackaged));
    xml.EndElement();
  }

  // create "TPM:MD5" node
  xml.StartElement("TPM:MD5");
  xml.Text(packageInfo.digest.ToString());
  xml.EndElement();

#if MIKTEX_EXTENDED_PACKAGEINFO
  if (!packageInfo.ctanPath.empty())
  {
    xml.StartElement("TPM:CTAN");
    xml.AddAttribute("path", packageInfo.ctanPath);
    xml.EndElement();
  }

  if (!(packageInfo.copyrightOwner.empty() && packageInfo.copyrightYear.empty()))
  {
    xml.StartElement("TPM:Copyright");
    xml.AddAttribute("owner", packageInfo.copyrightOwner);
    xml.AddAttribute("year", packageInfo.copyrightYear);
    xml.EndElement();
  }

  if (!packageInfo.licenseType.empty())
  {
    xml.StartElement("TPM:License");
    xml.AddAttribute("type", packageInfo.licenseType);
    xml.EndElement();
  }
#endif

  xml.EndAllElements();

  xml.Close();
}

void PackageManager::PutPackageManifest(Cfg& cfg, const PackageInfo& packageInfo, time_t timePackaged)
{
  if (cfg.GetKey(packageInfo.id) != nullptr)
  {
    cfg.DeleteKey(packageInfo.id);
  }
  if (!packageInfo.displayName.empty())
  {
    cfg.PutValue(packageInfo.id, "displayName", packageInfo.displayName);
  }
  cfg.PutValue(packageInfo.id, "creator", "mpc");
  if (!packageInfo.title.empty())
  {
    cfg.PutValue(packageInfo.id, "title", packageInfo.title);
  }
  if (!packageInfo.version.empty())
  {
    cfg.PutValue(packageInfo.id, "version", packageInfo.version);
  }
  if (!packageInfo.targetSystem.empty())
  {
    cfg.PutValue(packageInfo.id, "targetSystem", packageInfo.targetSystem);
  }
  if (!packageInfo.description.empty())
  {
    for (const string& line : StringUtil::Split(packageInfo.description, '\n'))
    {
      cfg.PutValue(packageInfo.id, "description[]", line);
    }
  }
  if (!packageInfo.requiredPackages.empty())
  {
    for (const string& pkg : packageInfo.requiredPackages)
    {
      cfg.PutValue(packageInfo.id, "requiredPackages[]", pkg);
    }
  }
  if (!packageInfo.runFiles.empty())
  {
    cfg.PutValue(packageInfo.id, "runSize", std::to_string(packageInfo.sizeRunFiles));
    for (const string& file : packageInfo.runFiles)
    {
      cfg.PutValue(packageInfo.id, "run[]", PathName(file).ToUnix().ToString());
    }
  }
  if (!packageInfo.docFiles.empty())
  {
    cfg.PutValue(packageInfo.id, "docSize", std::to_string(packageInfo.sizeDocFiles));
    for (const string& file : packageInfo.docFiles)
    {
      cfg.PutValue(packageInfo.id, "doc[]", PathName(file).ToUnix().ToString());
    }
  }
  if (!packageInfo.sourceFiles.empty())
  {
    cfg.PutValue(packageInfo.id, "sourceSize", std::to_string(packageInfo.sizeSourceFiles));
    for (const string& file : packageInfo.sourceFiles)
    {
      cfg.PutValue(packageInfo.id, "source[]", PathName(file).ToUnix().ToString());
    }
  }
  if (IsValidTimeT(timePackaged))
  {
    cfg.PutValue(packageInfo.id, "timePackaged", std::to_string(timePackaged));
  }
  cfg.PutValue(packageInfo.id, "digest", packageInfo.digest.ToString());
#if MIKTEX_EXTENDED_PACKAGEINFO
  if (!packageInfo.ctanPath.empty())
  {
    cfg.PutValue(packageInfo.id, "ctanPath", packageInfo.ctanPath);
  }
  if (!packageInfo.copyrightOwner.empty())
  {
    cfg.PutValue(packageInfo.id, "copyrightOwner", packageInfo.copyrightOwner);
  }
  if (!packageInfo.copyrightYear.empty())
  {
    cfg.PutValue(packageInfo.id, "copyrightYear", packageInfo.copyrightYear);
  }
  if (!packageInfo.licenseType.empty())
  {
    cfg.PutValue(packageInfo.id, "licenseType", packageInfo.licenseType);
  }
#endif
}

PackageInfo PackageManager::GetPackageManifest(const Cfg& cfg, const string& packageId, const std::string& texmfPrefix)
{
  auto key = cfg.GetKey(packageId);
  if (key == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  PackageInfo packageInfo;
  packageInfo.id = packageId;
  for (auto val : *key)
  {
    if (val->GetName() == "displayName")
    {
      packageInfo.displayName = val->GetValue();
    }
    else if (val->GetName() == "creator")
    {
      packageInfo.creator = val->GetValue();
    }
    else if (val->GetName() == "title")
    {
      packageInfo.title = val->GetValue();
    }
    else if (val->GetName() == "version")
    {
      packageInfo.version = val->GetValue();
    }
    else if (val->GetName() == "targetSystem")
    {
      packageInfo.targetSystem = val->GetValue();
    }
    else if (val->GetName() == "description[]")
    {
      packageInfo.description = StringUtil::Flatten(val->GetMultiValue(), '\n');
    }
    else if (val->GetName() == "requiredPackages[]")
    {
      packageInfo.requiredPackages = val->GetMultiValue();
    }
    else if (val->GetName() == "runSize")
    {
      packageInfo.sizeRunFiles = std::stoi(val->GetValue());
    }
    else if (val->GetName() == "run[]")
    {
      for (const string& s : val->GetMultiValue())
      {
        PathName path(s);
#if defined(MIKTEX_UNIX)
        path.ConvertToUnix();
#endif
        if (texmfPrefix.empty() || (PathName::Compare(texmfPrefix, path, texmfPrefix.length()) == 0))
        {
          packageInfo.runFiles.push_back(path.ToString());
        }
      }
    }
    else if (val->GetName() == "docSize")
    {
      packageInfo.sizeDocFiles = std::stoi(val->GetValue());
    }
    else if (val->GetName() == "doc[]")
    {
      for (const string& s : val->GetMultiValue())
      {
        PathName path(s);
#if defined(MIKTEX_UNIX)
        path.ConvertToUnix();
#endif
        if (texmfPrefix.empty() || (PathName::Compare(texmfPrefix, path, texmfPrefix.length()) == 0))
        {
          packageInfo.docFiles.push_back(path.ToString());
        }
      }
    }
    else if (val->GetName() == "sourceSize")
    {
      packageInfo.sizeSourceFiles = std::stoi(val->GetValue());
    }
    else if (val->GetName() == "source[]")
    {
      for (const string& s : val->GetMultiValue())
      {
        PathName path(s);
#if defined(MIKTEX_UNIX)
        path.ConvertToUnix();
#endif
        if (texmfPrefix.empty() || (PathName::Compare(texmfPrefix, path, texmfPrefix.length()) == 0))
        {
          packageInfo.sourceFiles.push_back(path.ToString());
        }
      }
    }
    else if (val->GetName() == "timePackaged")
    {
      packageInfo.timePackaged = std::stoi(val->GetValue());
    }
    else if (val->GetName() == "digest")
    {
      packageInfo.digest = MD5::Parse(val->GetValue());
    }
#if MIKTEX_EXTENDED_PACKAGEINFO
    else if (val->GetName() == "ctanPath")
    {
      packageInfo.ctanPath = val->GetValue();
    }
    else if (val->GetName() == "copyrightOwner")
    {
      packageInfo.copyrightOwner = val->GetValue();
    }
    else if (val->GetName() == "copyrightYear")
    {
      packageInfo.copyrightYear = val->GetValue();
    }
    else if (val->GetName() == "licenseType")
    {
      packageInfo.licenseType = val->GetValue();
    }
#endif
  }
  return packageInfo;
}

bool PackageManager::StripTeXMFPrefix(const string& str, string& result)
{
  if (StripPrefix(str, TEXMF_PREFIX_DIRECTORY, result))
  {
    return true;
  }
  PathName prefix2(".");
  prefix2 /= TEXMF_PREFIX_DIRECTORY;
  return StripPrefix(str, prefix2.GetData(), result);
}

void PackageManager::SetProxy(const ProxySettings& proxySettings)
{
  shared_ptr<Session> session = Session::Get();
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_USE_PROXY, proxySettings.useProxy);
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_PROXY_HOST, proxySettings.proxy);
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_PROXY_PORT, proxySettings.port);
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_PROXY_AUTH_REQ, proxySettings.authenticationRequired);
  PackageManagerImpl::proxyUser = proxySettings.user;
  PackageManagerImpl::proxyPassword = proxySettings.password;
}

bool PackageManager::TryGetProxy(const string& url, ProxySettings& proxySettings)
{
  shared_ptr<Session> session = Session::Get(); 
  proxySettings.useProxy = session->GetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_USE_PROXY, false).GetBool();
  if (proxySettings.useProxy)
  {
    if (!session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_PROXY_HOST, proxySettings.proxy))
    {
      return false;
    }
    proxySettings.port = session->GetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_PROXY_PORT, 8080).GetInt();
    proxySettings.authenticationRequired = session->GetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_PROXY_AUTH_REQ, false).GetBool();
    proxySettings.user = PackageManagerImpl::proxyUser;
    proxySettings.password = PackageManagerImpl::proxyPassword;
    return true;
  }
  string proxyEnv;
  if (!url.empty())
  {
    Uri uri(url.c_str());
    string scheme = uri.GetScheme();
    string envName;
    if (scheme == "https")
    {
      envName = "https_proxy";
    }
    else if (scheme == "http")
    {
      envName = "http_proxy";
    }
    else if (scheme == "ftp")
    {
      envName = "FTP_PROXY";
    }
    else
    {
      MIKTEX_UNEXPECTED();
    }
    Utils::GetEnvironmentString(envName, proxyEnv);
  }
  if (proxyEnv.empty())
  {
    Utils::GetEnvironmentString("ALL_PROXY", proxyEnv);
  }
  if (proxyEnv.empty())
  {
    return false;
  }
  Uri uri(proxyEnv);
  proxySettings.useProxy = true;
  proxySettings.proxy = uri.GetHost();
  proxySettings.port = uri.GetPort();
  string userInfo = uri.GetUserInfo();
  proxySettings.authenticationRequired = !userInfo.empty();
  if (proxySettings.authenticationRequired)
  {
    string::size_type idx = userInfo.find_first_of(":");
    if (idx == string::npos)
    {
      proxySettings.user = userInfo;
      proxySettings.password = "";
    }
    else
    {
      proxySettings.user = userInfo.substr(0, idx);
      proxySettings.password = userInfo.substr(idx + 1);
    }
  }
  else
  {
    proxySettings.user = "";
    proxySettings.password = "";
  }
  return true;
}

bool PackageManager::TryGetProxy(ProxySettings& proxySettings)
{
  return TryGetProxy("", proxySettings);
}

ProxySettings PackageManager::GetProxy(const string& url)
{
  ProxySettings proxySettings;
  if (!TryGetProxy(url, proxySettings))
  {
    MIKTEX_FATAL_ERROR(T_("No proxy host is configured."));
  }
  return proxySettings;
}

ProxySettings PackageManager::GetProxy()
{
  return GetProxy("");
}

void PackageManagerImpl::OnProgress()
{
}

bool PackageManagerImpl::TryGetFileDigest(const PathName& prefix, const string& fileName, bool& haveDigest, MD5& digest)
{
  string unprefixed;
  if (!StripTeXMFPrefix(fileName, unprefixed))
  {
    return true;
  }
  PathName path = prefix;
  path /= unprefixed;
  if (!File::Exists(path))
  {
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("package verification failed: file {0} does not exist"), Q_(path)));
    return false;
  }
  if (path.HasExtension(MIKTEX_PACKAGE_MANIFEST_FILE_SUFFIX))
  {
    haveDigest = false;
  }
  else
  {
    digest = MD5::FromFile(path);
    haveDigest = true;
  }
  return true;
}

bool PackageManagerImpl::TryCollectFileDigests(const PathName& prefix, const vector<string>& files, FileDigestTable& fileDigests)
{
  for (const string& fileName : files)
  {
    bool haveDigest;
    MD5 digest;
    if (!TryGetFileDigest(prefix, fileName, haveDigest, digest))
    {
      return false;
    }
    if (haveDigest)
    {
      fileDigests[fileName] = digest;
    }
  }
  return true;
}

bool PackageManagerImpl::TryVerifyInstalledPackage(const string& packageId)
{
  PackageInfo packageInfo = GetPackageInfo(packageId);

  PathName prefix;

  if (!session->IsAdminMode() && GetUserTimeInstalled(packageId) != static_cast<time_t>(0))
  {
    prefix = session->GetSpecialPath(SpecialPath::UserInstallRoot);
  }

  if (prefix.Empty())
  {
    prefix = session->GetSpecialPath(SpecialPath::CommonInstallRoot);
  }

  FileDigestTable fileDigests;

  if (!TryCollectFileDigests(prefix, packageInfo.runFiles, fileDigests)
    || !TryCollectFileDigests(prefix, packageInfo.docFiles, fileDigests)
    || !TryCollectFileDigests(prefix, packageInfo.sourceFiles, fileDigests))
  {
    return false;
  }

  MD5Builder md5Builder;

  for (const pair<string, MD5> p : fileDigests)
  {
    PathName path(p.first);
    // we must dosify the path name for backward compatibility
    path.ConvertToDos();
    md5Builder.Update(path.GetData(), path.GetLength());
    md5Builder.Update(p.second.data(), p.second.size());
  }

  bool ok = md5Builder.Final() == packageInfo.digest;

  if (!ok)
  {
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("package {0} verification failed: some files have been modified"), Q_(packageId)));
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("expected digest: {0}"), packageInfo.digest));
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("computed digest: {0}"), md5Builder.GetMD5()));
  }

  return ok;
}

string PackageManagerImpl::GetContainerPath(const string& packageId, bool useDisplayNames)
{
  string path;
  PackageInfo packageInfo = GetPackageInfo(packageId);
  for (const string& reqby : packageInfo.requiredBy)
  {
    PackageInfo packageInfo2 = GetPackageInfo(reqby);
    if (packageInfo2.IsPureContainer())
    {
      // RECUSION
      path = GetContainerPath(reqby, useDisplayNames);
      path += PathName::DirectoryDelimiter;
      if (useDisplayNames)
      {
        path += packageInfo2.displayName;
      }
      else
      {
        path += packageInfo2.id;
      }
      break;
    }
  }
  return path;
}

BEGIN_INTERNAL_NAMESPACE;

bool IsUrl(const string& url)
{
  string::size_type pos = url.find("://");
  if (pos == string::npos)
  {
    return false;
  }
  string scheme = url.substr(0, pos);
  for (const char& ch : scheme)
  {
    if (!isalpha(ch, locale()))
    {
      return false;
    }
  }
  return true;
}

string MakeUrl(const string& base, const string& rel)
{
  string url(base);
  size_t l = url.length();
  if (l == 0)
  {
    MIKTEX_UNEXPECTED();
  }
  if (url[l - 1] != '/')
  {
    url += '/';
  }
  if (rel[0] == '/')
  {
    MIKTEX_UNEXPECTED();
  }
  url += rel;
  return url;
}

END_INTERNAL_NAMESPACE;