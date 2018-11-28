/* PackageDataStore.cpp

   Copyright (C) 2018 Christian Schenk

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

#include <future>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>
#include <miktex/Trace/StopWatch>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>
#include <miktex/Util/StringUtil>

#include "internal.h"

#include "PackageDataStore.h"
#include "TpmParser.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

using namespace MiKTeX::Packages::D6AAD62216146D44B580E92711724B78;

PackageDataStore::PackageDataStore() :
  // TODO: trace callback
  trace_mpm(TraceStream::Open(MIKTEX_TRACE_MPM)),
  trace_stopwatch(TraceStream::Open(MIKTEX_TRACE_STOPWATCH))
{
}

void PackageDataStore::LoadVarData()
{
  if (!comboCfg.Loaded())
  {
    comboCfg.Load(
      session->GetSpecialPath(SpecialPath::UserInstallRoot) / MIKTEX_PATH_PACKAGES_INI,
      session->GetSpecialPath(SpecialPath::CommonInstallRoot) / MIKTEX_PATH_PACKAGES_INI);
  }
}

void PackageDataStore::SaveVarData()
{
  comboCfg.Save();
}

void PackageDataStore::Clear()
{
  packageTable.clear();
  installedFileInfoTable.clear();
  loadedAllPackageManifests = false;
  comboCfg.Clear();
}

void PackageDataStore::SetTimeInstalled(const string& packageId, time_t timeInstalled)
{
  LoadVarData();
  if (IsValidTimeT(timeInstalled))
  {
    comboCfg.PutValue(packageId, "TimeInstalled", std::to_string(timeInstalled));
  }
  else
  {
    comboCfg.DeleteKey(packageId);
  }
}

time_t PackageDataStore::GetUserTimeInstalled(const string& packageId)
{
  LoadVarData();
  string str;
  if (comboCfg.TryGetValueAsString(ComboCfg::Scope::User, packageId, "TimeInstalled", str))
  {
    return Utils::ToTimeT(str);
  }
  else
  {
    return InvalidTimeT;
  }
}

time_t PackageDataStore::GetCommonTimeInstalled(const std::string& packageId)
{
  LoadVarData();
  string str;
  if (comboCfg.TryGetValueAsString(ComboCfg::Scope::Common, packageId, "TimeInstalled", str))
  {
    return Utils::ToTimeT(str);
  }
  else
  {
    return InvalidTimeT;
  }
}

time_t PackageDataStore::GetTimeInstalled(const string& packageId)
{
  LoadVarData();
  string str;
  if ((!session->IsAdminMode() && comboCfg.TryGetValueAsString(ComboCfg::Scope::User, packageId, "TimeInstalled", str))
    || comboCfg.TryGetValueAsString(ComboCfg::Scope::Common, packageId, "TimeInstalled", str))
  {
    return Utils::ToTimeT(str);
  }
  else
  {
    return InvalidTimeT;
  }
}

bool PackageDataStore::IsInstalled(const string& packageId)
{
  LoadVarData();
  return IsValidTimeT(GetTimeInstalled(packageId));
}

bool PackageDataStore::IsRemovable(const string& packageId)
{
  LoadVarData();
  bool ret;
  string str;
  if (session->IsAdminMode())
  {
    // administrator can remove system-wide packages
    ret = IsValidTimeT(GetCommonTimeInstalled(packageId));
  }
  else
  {
    // user can remove private packages
    if (session->GetSpecialPath(SpecialPath::CommonInstallRoot).Canonicalize() == session->GetSpecialPath(SpecialPath::UserInstallRoot).Canonicalize())
    {
      ret = IsValidTimeT(GetTimeInstalled(packageId));
    }
    else
    {
      ret = IsValidTimeT(GetUserTimeInstalled(packageId));
    }
  }
  return ret;
}

void PackageDataStore::DeclareObsolete(const string& packageId, bool obsolete)
{
  LoadVarData();
  comboCfg.PutValue(packageId, "Obsolete", (obsolete ? "1" : "0"));
}

bool PackageDataStore::IsObsolete(const string& packageId)
{
  LoadVarData();
  string str;
  if ((!session->IsAdminMode() && comboCfg.TryGetValueAsString(ComboCfg::Scope::User, packageId, "Obsolete", str))
    || comboCfg.TryGetValueAsString(ComboCfg::Scope::Common, packageId, "Obsolete", str))
  {
    return std::stoi(str) != 0;
  }
  else
  {
    return false;
  }
}

void PackageDataStore::SetReleaseState(const string& packageId, RepositoryReleaseState releaseState)
{
  LoadVarData();
  comboCfg.PutValue(packageId, "ReleaseState", releaseState == RepositoryReleaseState::Next ? "next" : releaseState == RepositoryReleaseState::Stable ? "stable" : "");
}

RepositoryReleaseState PackageDataStore::GetReleaseState(const string& packageId)
{
  LoadVarData();
  string str;
  if (comboCfg.TryGetValueAsString(packageId, "ReleaseState", str))
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

PackageInfo* PackageDataStore::DefinePackage(const string& packageId, const PackageInfo& packageInfo)
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
    p.first->second.isObsolete = IsObsolete(packageId);
    p.first->second.timeInstalled = GetTimeInstalled(packageId);
    if (p.first->second.IsInstalled())
    {
      p.first->second.releaseState = GetReleaseState(packageId);
    }
  }
  return &(p.first->second);
}

void PackageDataStore::IncrementFileRefCounts(const vector<string>& files)
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


void PackageDataStore::IncrementFileRefCounts(const string& packageId)
{
  LoadAllPackageManifests();
  const PackageInfo& pi = packageTable[packageId];
  IncrementFileRefCounts(pi.runFiles);
  IncrementFileRefCounts(pi.docFiles);
  IncrementFileRefCounts(pi.sourceFiles);
}

unsigned long PackageDataStore::GetFileRefCount(const PathName& path)
{
  LoadAllPackageManifests();
  InstalledFileInfoTable::const_iterator it = installedFileInfoTable.find(path.GetData());
  if (it == installedFileInfoTable.end())
  {
    return 0;
  }
  return it->second.refCount;
}

InstalledFileInfo* PackageDataStore::GetInstalledFileInfo(const char* lpszPath)
{
  LoadAllPackageManifests();
  InstalledFileInfoTable::iterator it = installedFileInfoTable.find(lpszPath);
  if (it == installedFileInfoTable.end())
  {
    return nullptr;
  }
  return &it->second;
}

void PackageDataStore::LoadAllPackageManifests(const PathName& packageManifestsPath)
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
    PackageInfo packageInfo = PackageManager::GetPackageManifest(*cfg, key->GetName(), TEXMF_PREFIX_DIRECTORY);

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
    if (!pkg.IsContained() && !pkg.IsContainer() && IsObsolete(pkg.id))
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

  loadedAllPackageManifests = true;
}

void PackageDataStore::LoadAllPackageManifests()
{
  unique_ptr<StopWatch> stopWatch = StopWatch::Start(trace_stopwatch.get(), TRACE_FACILITY, "loading all package manifests");
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
      return;
    }
  }
  LoadAllPackageManifests(commonPath);
}

#if defined(MIKTEX_USE_ZZDB3)
void PackageDataStore::NeedPackageManifestsIni()
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

