/* PackageDataStore.h:                                  -*- C++ -*-

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

#pragma once

#if !defined(C4939A5A964940F38CA68698DDEC6718)
#define C4939A5A964940F38CA68698DDEC6718

#include <ctime>

#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>

#include <miktex/Core/PathName>
#include <miktex/Core/Session>
#include <miktex/Core/equal_icase>
#include <miktex/Core/hash_icase>
#include <miktex/Core/less_icase_dos>

#include <miktex/PackageManager/PackageManager>

#include "ComboCfg.h"

BEGIN_INTERNAL_NAMESPACE;

struct InstalledFileInfo
{
  unsigned long refCount = 0;
};

struct hash_path
{
public:
  std::size_t operator()(const std::string& str) const
  {
    return MiKTeX::Core::PathName(str).GetHash();
  }
};

struct equal_path
{
public:
  bool operator()(const std::string& str1, const std::string& str2) const
  {
    return MiKTeX::Core::PathName::Compare(str1, str2) == 0;
  }
};

class PackageDataStore
{
public:
  PackageDataStore();

private:
  void LoadVarData();

public:
  void SaveVarData();

public:
  void LoadAllPackageManifests(const MiKTeX::Core::PathName& packageManifestsPath);

private:
  void LoadAllPackageManifests();

public:
  void Clear();

public:
  std::tuple<bool, MiKTeX::Packages::PackageInfo> TryGetPackage(const std::string& packageId);

public:
  void SetPackage(const MiKTeX::Packages::PackageInfo& packageInfo);

private:
  MiKTeX::Packages::PackageInfo& operator[](const std::string& packageId);

public:
  void SetTimeInstalled(const std::string& packageId, std::time_t timeInstalled);

public:
  std::time_t GetUserTimeInstalled(const std::string& packageId);

public:
  std::time_t GetCommonTimeInstalled(const std::string& packageId);

public:
  std::time_t GetTimeInstalled(const std::string& packageId);

public:
  bool IsInstalled(const std::string& packageId);

public:
  bool IsRemovable(const std::string& packageId);

public:
  void DeclareObsolete(const std::string& packageId, bool obsolete);

public:
  bool IsObsolete(const std::string& packageId);

public:
  void SetReleaseState(const std::string& packageId, MiKTeX::Packages::RepositoryReleaseState releaseState);

public:
  MiKTeX::Packages::RepositoryReleaseState GetReleaseState(const std::string& packageId);

private:
  ComboCfg comboCfg;

private:
  typedef std::unordered_map<std::string, MiKTeX::Packages::PackageInfo, MiKTeX::Core::hash_icase, MiKTeX::Core::equal_icase> PackageDefinitionTable;

private:
  PackageDefinitionTable packageTable;

public:
  class iterator
  {
  public:
    iterator(PackageDefinitionTable::iterator it) :
      it(it)
    {
    }
  public:
    MiKTeX::Packages::PackageInfo& operator*()
    {
      return it->second;
    }
  public:
    iterator& operator++()
    {
      it++;
      return *this;
    }
  public:
    bool operator==(const iterator& rhs)
    {
      return it == rhs.it;
    }
  public:
    bool operator!=(const iterator& rhs)
    {
      return it != rhs.it;
    }
  private:
    PackageDefinitionTable::iterator it;
  };

public:
  iterator begin()
  {
    LoadAllPackageManifests();
    return iterator(packageTable.begin());
  }

public:
  iterator end()
  {
    LoadAllPackageManifests();
    return iterator(packageTable.end());
  }

public:
  void DefinePackage(const MiKTeX::Packages::PackageInfo& packageinfo);

public:
  void IncrementFileRefCounts(const std::string& packageId);

private:
  void IncrementFileRefCounts(const std::vector<std::string>& files);

private:
  typedef std::unordered_map<std::string, InstalledFileInfo, hash_path, equal_path> InstalledFileInfoTable;

private:
  InstalledFileInfoTable installedFileInfoTable;

public:
  unsigned long GetFileRefCount(const MiKTeX::Core::PathName& path);

public:
  InstalledFileInfo* GetInstalledFileInfo(const char* path);

public:
  void NeedPackageManifestsIni();

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mpm;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_stopwatch;

private:
  bool loadedAllPackageManifests = false;

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();
};

END_INTERNAL_NAMESPACE;

#endif
