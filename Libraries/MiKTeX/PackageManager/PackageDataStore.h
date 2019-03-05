/* PackageDataStore.h:                                  -*- C++ -*-

   Copyright (C) 2018-2019 Christian Schenk

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

MPM_INTERNAL_BEGIN_NAMESPACE;

/// @brief An instance of this class collects package records.
///
/// The record data is retrieved from two sources:
/// - `miktex/config/package-manifests.ini`: immutable package manifests
/// - `miktex/config/packages.ini`: mutable package data such as installation timestamps
class PackageDataStore
{
public:
  PackageDataStore();

  /// @brief Load package records into the data store.
  ///
  /// This method loads
  /// - mutable data from a given INI file
  /// - immutable package data from `miktex/config/packages.ini`
  /// 
  /// In addition, package dependencies and file reference counts are
  /// calculated.
  ///
  /// @param path Path to the INI file.
public:
  void LoadAllPackageManifests(const MiKTeX::Core::PathName& path);

  /// Save mutable package data.
public:
  void SaveVarData();

  /// Clear the contents of the data store.
public:
  void Clear();

  /// Try to retrieve a record from the data store.
  /// @param packageId The package ID.
  /// @return The first return value indicates whether the record (second return value) could be retrieved.
public:
  std::tuple<bool, MiKTeX::Packages::PackageInfo> TryGetPackage(const std::string& packageId);

  /// Retrieve a record from the data store.
  /// @exception std::exception Record not found.
  /// @param packageId The package ID.
  /// @return Returns the requested record.
public:
  MiKTeX::Packages::PackageInfo GetPackage(const std::string& packageId)
  {
    return (*this)[packageId];
  }

  /// Add a record to the data store.
  /// @param packageInfo The record to be added.
public:
  void DefinePackage(const MiKTeX::Packages::PackageInfo& packageinfo);

  /// Update a record in the data store.
  /// @param packageInfo The record to update.
public:
  void SetPackage(const MiKTeX::Packages::PackageInfo& packageInfo)
  {
    (*this)[packageInfo.id] = packageInfo;
  }

  /// @brief Set the package installation timestamp.
  ///
  /// If the timestamp is zero, the mutable package data will cleared
  /// (the package is not installed).
  /// 
  /// @param packageId The package ID.
  /// @param timeInstalled The installation timestamp.
public:
  void SetTimeInstalled(const std::string& packageId, std::time_t timeInstalled);

  /// Declare a package as obsolete.
  /// @param packageId The package ID.
public:
  void DeclareObsolete(const std::string& packageId);

  /// Set the release state of an installed package.
  /// @param packageId The package ID.
  /// @param releaseState The release state.
public:
  void SetReleaseState(const std::string& packageId, MiKTeX::Packages::RepositoryReleaseState releaseState);

private:
  typedef std::unordered_map<std::string, MiKTeX::Packages::PackageInfo, MiKTeX::Core::hash_icase, MiKTeX::Core::equal_icase> PackageDefinitionTable;

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

  /// Get an iterator to the first record.
public:
  iterator begin();

  /// Get an iterator referring to the past-the-end record.
public:
  iterator end();

  /// Increment the reference counts of all files in a package.
  /// @param path The package ID.
public:
  void IncrementFileRefCounts(const std::string& packageId);

  /// Get the reference count of a file.
  /// @param path The path to the file.
  /// @return Returns reference count of the file.
public:
  unsigned long GetFileRefCount(const MiKTeX::Core::PathName& path);

  /// Decrement the reference count of an installed file.
  /// @param path The path to the file.
  /// @return Returns the new (decremented) reference count.
public:
  unsigned long DecrementFileRefCount(const MiKTeX::Core::PathName& path);

  /// Migrate TPM files into a single INI file.
  ///
  /// If the INI file `miktex/config/package-manifests.ini` does not
  /// yet exists, it will be created by parsing all TPM files. This
  /// method can be used to migrate an old (pre 2019) package database
  /// into the new format.
public:
  void NeedPackageManifestsIni();

private:
  void Load();

private:
  void Load(MiKTeX::Core::Cfg& cfg);

private:
  void LoadVarData();

private:
  MiKTeX::Packages::PackageInfo& operator[](const std::string& packageId);

private:
  std::time_t GetUserTimeInstalled(const std::string& packageId);

private:
  std::time_t GetCommonTimeInstalled(const std::string& packageId);

private:
  std::time_t GetTimeInstalled(const std::string& packageId);

private:
  bool IsRemovable(const std::string& packageId);

private:
  bool IsObsolete(const std::string& packageId);

private:
  MiKTeX::Packages::RepositoryReleaseState GetReleaseState(const std::string& packageId);

private:
  void IncrementFileRefCounts(const std::vector<std::string>& files);

private:
  struct InstalledFileInfo
  {
    unsigned long refCount = 0;
  };

private:
  typedef std::unordered_map<std::string, InstalledFileInfo, hash_path, equal_path> InstalledFileInfoTable;

private:
  InstalledFileInfoTable installedFileInfoTable;

private:
  ComboCfg comboCfg;

private:
  PackageDefinitionTable packageTable;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mpm;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_stopwatch;

private:
  bool loadedAllPackageManifests = false;

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();
};

MPM_INTERNAL_END_NAMESPACE;

#endif
