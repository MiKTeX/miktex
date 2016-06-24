/* miktex/PackageManager/PackageManager.h:              -*- C++ -*-

   Copyright (C) 2001-2016 Christian Schenk

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

/// @file mpm.h
/// @brief MiKTeX package manager API.

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(D5760AC70396FF41BBDD7DD560BDBBBC)
#define D5760AC70396FF41BBDD7DD560BDBBBC

#include "config.h"

#include <ctime>

#include <memory>
#include <string>
#include <vector>

#include <miktex/Core/PathName>

#include <miktex/Trace/TraceCallback>
#include <miktex/Trace/TraceStream>

#include "PackageInfo.h"
#include "PackageInstaller.h"
#include "PackageIterator.h"
#include "RepositoryInfo.h"

/// @namespace MiKTeX::Packages
/// @brief The package manager namespace.
MPM_BEGIN_NAMESPACE;

struct ProxySettings
{
  bool useProxy = false;
  std::string proxy;
  int port = 0;
  bool authenticationRequired = false;
  std::string user;
  std::string password;
};

/// The package manager interface.
class MIKTEXNOVTABLE PackageManager
{
public:
  virtual MIKTEXTHISCALL ~PackageManager() = 0;

  /// Creates a new package installer object. The caller is responsible
  /// for deleting the object.
public:
  virtual std::unique_ptr<class PackageInstaller> MIKTEXTHISCALL CreateInstaller() = 0;

  /// Creates a new package iterator object. The caller is responsible
  /// for deleting the object.
public:
  virtual std::unique_ptr<class PackageIterator> MIKTEXTHISCALL CreateIterator() = 0;

  /// Recreates the file name database of the package manager.
public:
  virtual void MIKTEXTHISCALL CreateMpmFndb() = 0;

  /// Gets the reference count of a file.
  /// @param path Path name of the file.
  /// @returns Returns the number of packages this file is a member of.
public:
  virtual unsigned long MIKTEXTHISCALL GetFileRefCount(const MiKTeX::Core::PathName & path) = 0;

  /// Gets information about a specified package.
  /// @param lpszDeploymentName Package key.
  /// @param packageInfo Package info struct to be filled..
public:
  virtual PackageInfo MIKTEXTHISCALL GetPackageInfo(const std::string & deploymentName) = 0;

public:
  virtual void MIKTEXTHISCALL LoadDatabase(const MiKTeX::Core::PathName & path) = 0;

public:
  virtual void MIKTEXTHISCALL UnloadDatabase() = 0;

public:
  virtual RepositoryReleaseState MIKTEXTHISCALL GetRepositoryReleaseState() = 0;

public:
  virtual void MIKTEXTHISCALL SetRepositoryReleaseState(RepositoryReleaseState repositoryReleaseState) = 0;

public:
  virtual void MIKTEXTHISCALL DownloadRepositoryList() = 0;

public:
  virtual std::vector<RepositoryInfo> MIKTEXTHISCALL GetRepositories() = 0;
  
  /// Picks a random package repository.
  /// @return Returns a string object.
public:
  virtual std::string MIKTEXTHISCALL PickRepositoryUrl() = 0;

public:
  virtual bool MIKTEXTHISCALL TryGetPackageInfo(const std::string & deploymentName, PackageInfo & packageInfo) = 0;

public:
  virtual bool MIKTEXTHISCALL TryGetRepositoryInfo(const std::string & url, RepositoryInfo & repositoryInfo) = 0;

public:
  virtual RepositoryInfo MIKTEXTHISCALL VerifyPackageRepository(const std::string & url) = 0;

public:
  virtual bool MIKTEXTHISCALL TryVerifyInstalledPackage(const std::string & deploymentName) = 0;

public:
  virtual std::string MIKTEXTHISCALL GetContainerPath(const std::string & deploymentName, bool useDisplayNames) = 0;

public:
  struct InitInfo
  {
    InitInfo()
    {
    }
    InitInfo(MiKTeX::Trace::TraceCallback * traceCallback) :
      traceCallback(traceCallback)
    {
    }
    MiKTeX::Trace::TraceCallback * traceCallback = nullptr;
  };

public:
  static MIKTEXMPMCEEAPI(std::shared_ptr<PackageManager>) Create(const InitInfo & initInfo);

public:
  static std::shared_ptr<PackageManager> Create()
  {
    return Create(InitInfo());
  }

public:
  static MIKTEXMPMCEEAPI(std::string) GetRemotePackageRepository(RepositoryReleaseState & repositoryReleaseState);

public:
  static std::string GetRemotePackageRepository()
  {
    RepositoryReleaseState unusedRepositoryReleaseState;
    return GetRemotePackageRepository(unusedRepositoryReleaseState);
  }

public:
  static MIKTEXMPMCEEAPI(bool) TryGetRemotePackageRepository(std::string & url, RepositoryReleaseState & repositoryReleaseState);

public:
  static bool TryGetRemotePackageRepository(std::string & url)
  {
    RepositoryReleaseState unusedRepositoryReleaseState;
    return TryGetRemotePackageRepository(url, unusedRepositoryReleaseState);
  }

public:
  static MIKTEXMPMCEEAPI(MiKTeX::Core::PathName) GetLocalPackageRepository();

public:
  static MIKTEXMPMCEEAPI(bool) TryGetLocalPackageRepository(MiKTeX::Core::PathName & path);

public:
  static MIKTEXMPMCEEAPI(MiKTeX::Core::PathName) GetMiKTeXDirectRoot();

public:
  static MIKTEXMPMCEEAPI(bool) TryGetMiKTeXDirectRoot(MiKTeX::Core::PathName & path);

public:
  static MIKTEXMPMCEEAPI(RepositoryType) GetDefaultPackageRepository(RepositoryReleaseState & repositoryReleaseState, std::string & urlOrPath);

public:
  static RepositoryType GetDefaultPackageRepository(std::string & urlOrPath)
  {
    RepositoryReleaseState unusedRepositoryReleaseState;
    return GetDefaultPackageRepository(unusedRepositoryReleaseState, urlOrPath);
  }

public:
  static MIKTEXMPMCEEAPI(bool) TryGetDefaultPackageRepository(RepositoryType & repositoryType, RepositoryReleaseState & repositoryReleaseState, std::string & urlOrPath);

public:
  static bool TryGetDefaultPackageRepository(RepositoryType & repositoryType, std::string & urlOrPath)
  {
    RepositoryReleaseState unusedRepositoryReleaseState;
    return TryGetDefaultPackageRepository(repositoryType, unusedRepositoryReleaseState, urlOrPath);
  }

public:
  static MIKTEXMPMCEEAPI(void) SetDefaultPackageRepository(RepositoryType repositoryType, RepositoryReleaseState repositoryReleaseState, const std::string & urlOrPath);

public:
  static void SetDefaultPackageRepository(RepositoryType repositoryType, const std::string & urlOrPath)
  {
    SetDefaultPackageRepository(repositoryType, RepositoryReleaseState::Unknown, urlOrPath);
  }

public:
  static MIKTEXMPMCEEAPI(bool) IsLocalPackageRepository(const MiKTeX::Core::PathName & path);

public:
  static MIKTEXMPMCEEAPI(void) SetRemotePackageRepository(const std::string & url, RepositoryReleaseState repositoryReleaseState);

public:
  static void SetRemotePackageRepository(const std::string & url)
  {
    SetRemotePackageRepository(url, RepositoryReleaseState::Unknown);
  }

public:
  static MIKTEXMPMCEEAPI(void) SetLocalPackageRepository(const MiKTeX::Core::PathName & path);

public:
  static MIKTEXMPMCEEAPI(void) SetMiKTeXDirectRoot(const MiKTeX::Core::PathName & path);

public:
  static MIKTEXMPMCEEAPI(PackageInfo) ReadPackageDefinitionFile(const MiKTeX::Core::PathName & path, const std::string & texmfPrefix);

public:
  static MIKTEXMPMCEEAPI(void) WritePackageDefinitionFile(const MiKTeX::Core::PathName & path, const PackageInfo & packageInfo, time_t timePackaged);

  /// Strips the TEXMF prefix from a string.
  /// @param str The string to strip.
  /// @param[out] result The result to be filled with the stripped string.
  /// @return Returns true, if the prefix was stripped.
public:
  static MIKTEXMPMCEEAPI(bool) StripTeXMFPrefix(const std::string & str, std::string & result);

public:
  static MIKTEXMPMCEEAPI(bool) TryGetProxy(const std::string & url, ProxySettings & proxySettings);

public:
  static MIKTEXMPMCEEAPI(bool) TryGetProxy(ProxySettings & proxySettings);

public:
  static MIKTEXMPMCEEAPI(ProxySettings) GetProxy(const std::string & url);

public:
  static MIKTEXMPMCEEAPI(ProxySettings) GetProxy();

public:
  static MIKTEXMPMCEEAPI(void) SetProxy(const ProxySettings & proxySettings);
};

MPM_END_NAMESPACE;

#endif
