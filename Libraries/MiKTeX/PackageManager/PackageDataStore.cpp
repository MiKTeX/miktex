/* PackageDataStore.cpp

   Copyright (C) 2018-2020 Christian Schenk

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
#include "PackageManagerImpl.h"
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

void PackageDataStore::LoadAllPackageManifests(const PathName& packageManifestsPath, bool mustBeSigned)
{
  trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("loading all package manifests ({0})"), Q_(packageManifestsPath)));

  if (!File::Exists(packageManifestsPath))
  {
    trace_mpm->WriteLine(TRACE_FACILITY, TraceLevel::Warning, fmt::format(T_("file {0} does not exist"), Q_(packageManifestsPath)));
    return;
  }

  unique_ptr<Cfg> cfg = Cfg::Create();
  cfg->Read(packageManifestsPath, mustBeSigned);

  Load(*cfg);

  loadedAllPackageManifests = true;
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

tuple<bool, PackageInfo> PackageDataStore::TryGetPackage(const string& packageId)
{
  MIKTEX_EXPECT(loadedAllPackageManifests);
  auto it = packageTable.find(packageId);
  if (it == packageTable.end())
  {
    return make_tuple(false, PackageInfo());
  }
  else
  {
    return make_tuple(true, it->second);
  }
}

void PackageDataStore::SetTimeInstalled(const string& packageId, time_t timeInstalled)
{
  (*this)[packageId].SetTimeInstalled(timeInstalled, session->IsAdminMode() ? ConfigurationScope::Common : ConfigurationScope::User);
  if (IsValidTimeT(timeInstalled))
  {
    comboCfg.PutValue(packageId, "TimeInstalled", std::to_string(timeInstalled));
  }
  else
  {
    comboCfg.DeleteKey(packageId);
  }
}

void PackageDataStore::DeclareObsolete(const string& packageId)
{
  (*this)[packageId].isObsolete = true;
  comboCfg.PutValue(packageId, "Obsolete", "1");
}

void PackageDataStore::SetReleaseState(const string& packageId, RepositoryReleaseState releaseState)
{
  (*this)[packageId].releaseState = releaseState;
  comboCfg.PutValue(packageId, "ReleaseState", releaseState == RepositoryReleaseState::Next ? "next" : releaseState == RepositoryReleaseState::Stable ? "stable" : "");
}

PackageDataStore::iterator PackageDataStore::begin()
{
  MIKTEX_EXPECT(loadedAllPackageManifests);
  return iterator(packageTable.begin());
}

PackageDataStore::iterator PackageDataStore::end()
{
  MIKTEX_EXPECT(loadedAllPackageManifests);
  return iterator(packageTable.end());
}

void PackageDataStore::DefinePackage(const PackageInfo& packageInfo)
{
  pair<PackageDefinitionTable::iterator, bool> p = packageTable.insert(make_pair(packageInfo.id, packageInfo));
  if (session->IsMiKTeXDirect())
  {
    // installed from the start
    p.first->second.isRemovable = false;
    p.first->second.isObsolete = false;
    p.first->second.timeInstalledCommon = packageInfo.timePackaged;
    p.first->second.timeInstalledUser = packageInfo.timePackaged;
  }
  else
  {
    p.first->second.isRemovable = IsRemovable(p.first->second.id);
    p.first->second.isObsolete = IsObsolete(p.first->second.id);
    p.first->second.timeInstalledCommon = GetTimeInstalled(p.first->second.id, ConfigurationScope::Common);
    p.first->second.timeInstalledUser = GetTimeInstalled(p.first->second.id, ConfigurationScope::User);
    if (p.first->second.IsInstalled())
    {
      p.first->second.releaseState = GetReleaseState(p.first->second.id);
    }
  }
}

void PackageDataStore::IncrementFileRefCounts(const string& packageId)
{
  const PackageInfo& package = (*this)[packageId];
  IncrementFileRefCounts(package.runFiles);
  IncrementFileRefCounts(package.docFiles);
  IncrementFileRefCounts(package.sourceFiles);
}

unsigned long PackageDataStore::GetFileRefCount(const PathName& path)
{
  MIKTEX_EXPECT(loadedAllPackageManifests);
  InstalledFileInfoTable::const_iterator it = installedFileInfoTable.find(path.ToString());
  if (it == installedFileInfoTable.end())
  {
    return 0;
  }
  return it->second.refCount;
}

unsigned long PackageDataStore::DecrementFileRefCount(const PathName& path)
{
  MIKTEX_EXPECT(loadedAllPackageManifests);
  InstalledFileInfoTable::iterator it = installedFileInfoTable.find(path.ToString());
  if (it == installedFileInfoTable.end() || it->second.refCount == 0)
  {
    MIKTEX_UNEXPECTED();
  }
  it->second.refCount--;
  return it->second.refCount;
}

void PackageDataStore::NeedPackageManifestsIni()
{
  PathName existingPackageManifestsIni = session->GetSpecialPath(SpecialPath::InstallRoot) / PathName(MIKTEX_PATH_PACKAGE_MANIFESTS_INI);
  if (File::Exists(existingPackageManifestsIni))
  {
    return;
  }
  PathName tpmDir = session->GetSpecialPath(SpecialPath::InstallRoot) / PathName(MIKTEX_PATH_PACKAGE_MANIFEST_DIR);
  if (Directory::Exists(tpmDir))
  {
    trace_mpm->WriteLine(TRACE_FACILITY, TraceLevel::Info, fmt::format("starting migration: {} -> {}", tpmDir, existingPackageManifestsIni));
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
    trace_mpm->WriteLine(TRACE_FACILITY, TraceLevel::Info, fmt::format("successfully migrated {} package manifest files", count));
  }
}

PackageDataStore& PackageDataStore::Load()
{
  if (loadedAllPackageManifests)
  {
    // we do this once
    return *this;
  }
  unique_ptr<StopWatch> stopWatch = StopWatch::Start(trace_stopwatch.get(), TRACE_FACILITY, "loading all package manifests");
  NeedPackageManifestsIni();
  unique_ptr<Cfg> cfg = Cfg::Create();
  if (!session->IsAdminMode())
  {
    PathName userPath = session->GetSpecialPath(SpecialPath::UserInstallRoot) / PathName(MIKTEX_PATH_PACKAGE_MANIFESTS_INI);
    if (File::Exists(userPath))
    {
      cfg->Read(userPath);
    }
  }
  PathName commonPath = session->GetSpecialPath(SpecialPath::CommonInstallRoot) / PathName(MIKTEX_PATH_PACKAGE_MANIFESTS_INI);
  if ((session->IsAdminMode() || session->GetSpecialPath(SpecialPath::UserInstallRoot).Canonicalize() != session->GetSpecialPath(SpecialPath::CommonInstallRoot).Canonicalize()) && File::Exists(commonPath))
  {
    cfg->SetOptions({ Cfg::Option::NoOverwriteKeys });
    cfg->Read(commonPath);
  }
  Load(*cfg);
  loadedAllPackageManifests = true;
  return *this;
}

void PackageDataStore::Load(Cfg& cfg)
{
  unsigned count = 0;
  for (const auto& key : cfg)
  {
    // ignore redefinition
    if (packageTable.find(key->GetName()) != packageTable.end())
    {
      continue;
    }

    PackageInfo packageInfo = PackageManager::GetPackageManifest(cfg, key->GetName(), TEXMF_PREFIX_DIRECTORY);

#if IGNORE_OTHER_SYSTEMS
    string targetSystems = packageInfo.targetSystem;
    if (targetSystems != "" && !StringUtil::Contains(targetSystems.c_str(), MIKTEX_SYSTEM_TAG))
    {
      continue;
    }
#endif

    count += 1;

    // insert into database
    DefinePackage(packageInfo);

    // increment file ref counts, if package is installed
    if (packageInfo.IsInstalled())
    {
      IncrementFileRefCounts(packageInfo.runFiles);
      IncrementFileRefCounts(packageInfo.docFiles);
      IncrementFileRefCounts(packageInfo.sourceFiles);
    }
  }

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
        trace_mpm->WriteLine(TRACE_FACILITY, TraceLevel::Warning, fmt::format(T_("dependancy problem: {0} is required by {1}"), req, pkg.id));
      }
      else
      {
        it3->second.requiredBy.push_back(pkg.id);
        if (it3->second.GetTimeInstalled() < timeInstalledMin)
        {
          timeInstalledMin = it3->second.GetTimeInstalled();
        }
        if (it3->second.GetTimeInstalled() > timeInstalledMax)
        {
          timeInstalledMax = it3->second.GetTimeInstalled();
        }
      }
    }
    if (timeInstalledMin > 0)
    {
      if (pkg.IsPureContainer() || (pkg.IsInstalled() && pkg.GetTimeInstalled() < timeInstalledMax))
      {
        if (session->IsAdminMode())
        {
          pkg.timeInstalledCommon = timeInstalledMax;
        }
        else
        {
          pkg.timeInstalledUser = timeInstalledMax;
        }
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
    DefinePackage(piObsolete);
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
    DefinePackage(piOther);
  }
}

void PackageDataStore::LoadVarData()
{
  if (!comboCfg.Loaded())
  {
    comboCfg.Load(
      session->IsAdminMode() ? PathName() : session->GetSpecialPath(SpecialPath::UserInstallRoot) / PathName(MIKTEX_PATH_PACKAGES_INI),
      session->GetSpecialPath(SpecialPath::CommonInstallRoot) / PathName(MIKTEX_PATH_PACKAGES_INI));
  }
}

PackageInfo& PackageDataStore::operator[](const string& packageId)
{
  MIKTEX_EXPECT(loadedAllPackageManifests);
  auto it = packageTable.find(packageId);
  if (it == packageTable.end())
  {
    MIKTEX_FATAL_ERROR_2(T_("The requested package is unknown."), "name", packageId);
  }
  return it->second;
}

time_t PackageDataStore::GetTimeInstalled(const string& packageId, ConfigurationScope scope)
{
  LoadVarData();
  string str;
  if (comboCfg.TryGetValueAsString(scope, packageId, "TimeInstalled", str))
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
  if ((!session->IsAdminMode() && comboCfg.TryGetValueAsString(ConfigurationScope::User, packageId, "TimeInstalled", str))
    || comboCfg.TryGetValueAsString(ConfigurationScope::Common, packageId, "TimeInstalled", str))
  {
    return Utils::ToTimeT(str);
  }
  else
  {
    return InvalidTimeT;
  }
}

bool PackageDataStore::IsRemovable(const string& packageId)
{
  LoadVarData();
  bool ret;
  string str;
  if (session->IsAdminMode())
  {
    // administrator can remove system-wide packages
    ret = IsValidTimeT(GetTimeInstalled(packageId, ConfigurationScope::Common));
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
      ret = IsValidTimeT(GetTimeInstalled(packageId, ConfigurationScope::User));
    }
  }
  return ret;
}

bool PackageDataStore::IsObsolete(const string& packageId)
{
  LoadVarData();
  string str;
  if ((!session->IsAdminMode() && comboCfg.TryGetValueAsString(ConfigurationScope::User, packageId, "Obsolete", str))
    || comboCfg.TryGetValueAsString(ConfigurationScope::Common, packageId, "Obsolete", str))
  {
    return std::stoi(str) != 0;
  }
  else
  {
    return false;
  }
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

void PackageDataStore::IncrementFileRefCounts(const vector<string>& files)
{
  for (const string& file : files)
  {
    ++installedFileInfoTable[file].refCount;
#if POLLUTE_THE_DEBUG_STREAM
    if (installedFileInfoTable[file].refCount >= 2)
    {
      trace_mpm->WriteLine(TRACE_FACILITY, TraceLevel::Debug, fmt::format(T_("{0}: ref count > 1"), Q_(file)));
    }
#endif
  }
}

size_t PackageDataStore::GetNumberOfInstalledPackages(bool userScope)
{
  LoadVarData();
  return comboCfg.GetSize(userScope ? ConfigurationScope::User : ConfigurationScope::Common);
}
