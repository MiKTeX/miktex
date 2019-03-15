/* miktex/PackageManager/PackageManager.h:              -*- C++ -*-

   Copyright (C) 2001-2019 Christian Schenk

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

#if !defined(D5760AC70396FF41BBDD7DD560BDBBBC)
/// @cond
#define D5760AC70396FF41BBDD7DD560BDBBBC
/// @endcond

#include "config.h"

#include <ctime>

#include <memory>
#include <string>
#include <vector>

#include <miktex/Core/Cfg>
#include <miktex/Core/PathName>

#include <miktex/Trace/TraceCallback>
#include <miktex/Trace/TraceStream>

#include "PackageInfo.h"
#include "PackageInstaller.h"
#include "PackageIterator.h"
#include "RepositoryInfo.h"

/// @namespace MiKTeX::Packages
/// @brief The package manager namespace.
MIKTEX_PACKAGES_BEGIN_NAMESPACE;

struct ProxySettings
{
  bool useProxy = false;
  std::string proxy;
  int port = 0;
  bool authenticationRequired = false;
  std::string user;
  std::string password;
};

/// Installation summary.
struct InstallationSummary
{
  /// Time of last update check.
  std::time_t lastUpdateCheck = InvalidTimeT;
  /// Time of last update.
  std::time_t lastUpdate = InvalidTimeT;
  /// Number of installed packages.
  std::size_t packageCount = 0;
};

/// The package manager interface.
class MIKTEXNOVTABLE PackageManager
{
public:
  virtual MIKTEXTHISCALL ~PackageManager() noexcept = 0;

  /// Creates a new package installer object.
  /// @param initInfo Initialization options.
  /// @return Returns a smart pointer to the new package installer object.
public:
  virtual std::unique_ptr<class PackageInstaller> MIKTEXTHISCALL CreateInstaller(const PackageInstaller::InitInfo& initInfo) = 0;

  /// Creates a new package installer object.
  /// @return Returns a smart pointer to the new package installer object.
public:
  virtual std::unique_ptr<class PackageInstaller> MIKTEXTHISCALL CreateInstaller() = 0;

  /// Creates a new package iterator object.
public:
  virtual std::unique_ptr<class PackageIterator> MIKTEXTHISCALL CreateIterator() = 0;

  /// Recreates the file name database of the package manager.
public:
  virtual void MIKTEXTHISCALL CreateMpmFndb() = 0;

  /// Gets information about a specified package.
  /// @param packageId Package key.
  /// @return Returns the package record.
public:
  virtual PackageInfo MIKTEXTHISCALL GetPackageInfo(const std::string& packageId) = 0;

  /// Loads the package database.
  /// @param Path to the package database.
  /// @param isArchive Indicates whether this is an archive file or an INI file.
public:
  virtual void MIKTEXTHISCALL LoadDatabase(const MiKTeX::Core::PathName& path, bool isArchive) = 0;

  /// Unloads the package database.
public:
  virtual void MIKTEXTHISCALL UnloadDatabase() = 0;

  /// Gets the release state of the current package repository.
  /// @return Returns the release state.
public:
  virtual RepositoryReleaseState MIKTEXTHISCALL GetRepositoryReleaseState() = 0;

  /// Sets the release state of the current package repository.
  /// @param repositoryReleaseState The release state to set.
public:
  virtual void MIKTEXTHISCALL SetRepositoryReleaseState(RepositoryReleaseState repositoryReleaseState) = 0;

  /// Downloads the repositiry list from the server.
public:
  virtual void MIKTEXTHISCALL DownloadRepositoryList() = 0;

  /// Gets the downloaded repoistory list.
  /// @return Returns the repository list.
public:
  virtual std::vector<RepositoryInfo> MIKTEXTHISCALL GetRepositories() = 0;
  
  /// Picks a random package repository.
  /// @return Returns the URL of a package repository.
public:
  virtual std::string MIKTEXTHISCALL PickRepositoryUrl() = 0;

  /// Tries to get a record from the package database.
  /// @param url Identifies the package.
  /// @param[out] packageInfo The loaded record.
  /// @return Returns `true`, if the record could be retrieved.
public:
  virtual bool MIKTEXTHISCALL TryGetPackageInfo(const std::string& packageId, PackageInfo& packageInfo) = 0;

  /// Tries to get a record from the repository data store.
  /// @param url Identifies the remote package repository.
  /// @param[out] repositoryInfo The loaded record.
  /// @return Returns `true`, if the record could be retrieved.
public:
  virtual bool MIKTEXTHISCALL TryGetRepositoryInfo(const std::string& url, RepositoryInfo& repositoryInfo) = 0;

  /// Checks a remote package repository.
  /// @param url Identifies the remote package repository.
  /// @param[out] repositoryInfo Information about the remote package repository.
  /// @return Returns `true`, if the package repository is usable.
public:
  virtual RepositoryInfo MIKTEXTHISCALL CheckPackageRepository(const std::string& url) = 0;

  /// Lets the server verify a remote package repository.
  /// @param url Identifies the repository to verify.
  /// @return repositoryInfo Returns the repository record returned by the server.
  /// @exception MiKTeXException The repository is not valid.
public:
  virtual RepositoryInfo MIKTEXTHISCALL VerifyPackageRepository(const std::string& url) = 0;

  /// @brief Verifies an installed package.
  ///
  /// This method reads all files in order to verify the integrity of
  /// the package.
  /// 
  /// @param packageId Identifies the package.
  /// @return Returns `true`, if the package is correctly installed.
public:
  virtual bool MIKTEXTHISCALL TryVerifyInstalledPackage(const std::string& packageId) = 0;

  /// Builds the container path of a package.
  /// @param packageId Identifies the package.
  /// @param useDisplayNames Indicates whether to use user friendly names.
  /// @return Returns the container path of the package.
public:
  virtual std::string MIKTEXTHISCALL GetContainerPath(const std::string& packageId, bool useDisplayNames) = 0;

  /// Gets the installation summary.
  /// @param common Indicates whether to retrieve a summary for the current user.
  /// @return Returns the installation summary.
public:
  virtual InstallationSummary MIKTEXTHISCALL GetInstallationSummary(bool userScope) = 0;

public:
  /// Initialization options.
  struct InitInfo
  {
    InitInfo() = default;
    InitInfo(const InitInfo& other) = default;
    InitInfo& operator=(const InitInfo& other) = default;
    InitInfo(InitInfo&& other) = default;
    InitInfo& operator=(InitInfo&& other) = default;
    ~InitInfo() = default;
    InitInfo(MiKTeX::Trace::TraceCallback* traceCallback) :
      traceCallback(traceCallback)
    {
    }
    MiKTeX::Trace::TraceCallback* traceCallback = nullptr;
  };

  /// Creates a new package manager object.
  /// @param initInfo Initialization options.
  /// @return Returns a smart pointer to the new package manager object.
public:
  static MIKTEXMPMCEEAPI(std::shared_ptr<PackageManager>) Create(const InitInfo& initInfo);

  /// Creates a new package manager object.
  /// @return Returns a smart pointer to the new package manager object.
public:
  static std::shared_ptr<PackageManager> Create()
  {
    return Create(InitInfo());
  }

  /// Gets the configured remote package repository.
  /// @param[out] repositoryReleaseState The release state of the package repository.
  /// @return Returns the URL of the remote package repository.
  /// @exception MiKTeXException No remote package repository is configured.
  /// @see SetRemotePackageRepository
public:
  static MIKTEXMPMCEEAPI(std::string) GetRemotePackageRepository(RepositoryReleaseState& repositoryReleaseState);

  /// Gets the configured remote package repository.
  /// @param[out] repositoryReleaseState The release state of the package repository.
  /// @return Returns the URL of the remote package repository.
  /// @exception MiKTeXException No remote package repository is configured.
  /// @see SetRemotePackageRepository
public:
  static std::string GetRemotePackageRepository()
  {
    RepositoryReleaseState unusedRepositoryReleaseState;
    return GetRemotePackageRepository(unusedRepositoryReleaseState);
  }

  /// Gets the configured remote package repository.
  /// @param[out] url The URL of the remote package repository.
  /// @param[out] repositoryReleaseState The release state of the package repository.
  /// @return Returns `false`, if the remote package repository is not configured.
  /// @see SetRemotePackageRepository
public:
  static MIKTEXMPMCEEAPI(bool) TryGetRemotePackageRepository(std::string& url, RepositoryReleaseState& repositoryReleaseState);

  /// Gets the configured remote package repository.
  /// @param[out] url The URL of the remote package repository.
  /// @return Returns `false`, if the remote package repository is not configured.
  /// @see SetRemotePackageRepository
public:
  static bool TryGetRemotePackageRepository(std::string& url)
  {
    RepositoryReleaseState unusedRepositoryReleaseState;
    return TryGetRemotePackageRepository(url, unusedRepositoryReleaseState);
  }

  /// Gets the configured local package repository.
  /// @return Returns the path to the local package repository.
  /// @exception MiKTeXException No local package repository is configured.
  /// @see SetLocalPackageRepository
public:
  static MIKTEXMPMCEEAPI(MiKTeX::Core::PathName) GetLocalPackageRepository();

  /// Gets the configured local package repository.
  /// @param[out] path The path to the local package repository.
  /// @return Returns `false`, if the local package repository is not configured.
  /// @see SetLocalPackageRepository
public:
  static MIKTEXMPMCEEAPI(bool) TryGetLocalPackageRepository(MiKTeX::Core::PathName& path);

  /// GetMiKTeXDirectRoot
  /// @todo internal
public:
  static MIKTEXMPMCEEAPI(MiKTeX::Core::PathName) GetMiKTeXDirectRoot();

  /// TryGetMiKTeXDirectRoot
  /// @todo internal
public:
  static MIKTEXMPMCEEAPI(bool) TryGetMiKTeXDirectRoot(MiKTeX::Core::PathName& path);

  /// Gets the configured package repository.
  /// @return Returns the configured package repository.
  /// @see SetDefaultPackageRepository
public:
  static MIKTEXMPMCEEAPI(RepositoryInfo) GetDefaultPackageRepository();

  /// Gets the configured package repository.
  /// @param[out] repository The repository type.
  /// @param[out] repositoryReleaseState The repository release state.
  /// @param[out] urlOrPath The configured package repository.
  /// @return Returns `true`.
  /// @see SetDefaultPackageRepository
public:
  static MIKTEXMPMCEEAPI(bool) TryGetDefaultPackageRepository(RepositoryType& repositoryType, RepositoryReleaseState& repositoryReleaseState, std::string& urlOrPath);

  /// Gets the configured package repository.
  /// @param[out] repository The repository type.
  /// @param[out] urlOrPath The configured package repository.
  /// @return Returns `true`.
  /// @see SetDefaultPackageRepository
public:
  static bool TryGetDefaultPackageRepository(RepositoryType& repositoryType, std::string& urlOrPath)
  {
    RepositoryReleaseState unusedRepositoryReleaseState;
    return TryGetDefaultPackageRepository(repositoryType, unusedRepositoryReleaseState, urlOrPath);
  }

  /// Configures the default package repository.
  /// @param repository The new default package repository.
  /// @see GetDefaultPackageRepository
public:
  static MIKTEXMPMCEEAPI(void) SetDefaultPackageRepository(const RepositoryInfo& repository);

  /// Configures the default package repository.
  /// @param repository The repository type.
  /// @param repositoryReleaseState The repository release state.
  /// @param urlOrPath The path to (URL of) the default package repository.
  /// @see GetDefaultPackageRepository
public:
  static MIKTEXMPMCEEAPI(void) SetDefaultPackageRepository(RepositoryType repositoryType, RepositoryReleaseState repositoryReleaseState, const std::string& urlOrPath);

  /// Configures the default package repository.
  /// @param repository The repository type.
  /// @param urlOrPath The path to (URL of) the default package repository.
  /// @see GetDefaultPackageRepository
public:
  static void SetDefaultPackageRepository(RepositoryType repositoryType, const std::string& urlOrPath)
  {
    SetDefaultPackageRepository(repositoryType, RepositoryReleaseState::Unknown, urlOrPath);
  }

  /// Tests a local package repository.
  /// @param path Path to the local package repository.
  /// @return Returns `true`, if this is a local package repository.
public:
  static MIKTEXMPMCEEAPI(bool) IsLocalPackageRepository(const MiKTeX::Core::PathName& path);

  /// Configures the remote package repository.
  /// @param url Identifies the remote package repository.
  /// @param repositoryReleaseState The release state of the package repository.
  /// @see GetRemotePackageRepository
public:
  static MIKTEXMPMCEEAPI(void) SetRemotePackageRepository(const std::string& url, RepositoryReleaseState repositoryReleaseState);

  /// Configures the remote package repository.
  /// @param url Identifies the remote package repository.
  /// @see GetRemotePackageRepository
public:
  static void SetRemotePackageRepository(const std::string& url)
  {
    SetRemotePackageRepository(url, RepositoryReleaseState::Unknown);
  }

  /// Configures the local package repository.
  /// @param path Path to the local package repository.
  /// @see GetLocalPackageRepository
public:
  static MIKTEXMPMCEEAPI(void) SetLocalPackageRepository(const MiKTeX::Core::PathName& path);

  /// SetMiKTeXDirectRoot
  /// @todo internal
public:
  static MIKTEXMPMCEEAPI(void) SetMiKTeXDirectRoot(const MiKTeX::Core::PathName& path);

  /// Loads a package manifest from an INI file.
  /// @param path The path to the INI file.
  /// @param texmfPrefix The TEXMF prefix to be stripped from file names.
  /// @return Returns the parsed package manifest.
public:
  static MIKTEXMPMCEEAPI(PackageInfo) ReadPackageManifestFile(const MiKTeX::Core::PathName& path, const std::string& texmfPrefix);

  /// Writes a package manifest into an INI file.
  /// @param path The path to the INI file.
  /// @param packageInfo The package manifest.
  /// @param timePackaged The packaging timestamp.
public:
  static MIKTEXMPMCEEAPI(void) WritePackageManifestFile(const MiKTeX::Core::PathName& path, const PackageInfo& packageInfo, std::time_t timePackaged);

  /// Adds (updates) a package manifest to (in) a `Cfg` container.
  /// @param cfg The `Cfg` instance to modify.
  /// @param packageInfo The package manifest.
  /// @param timePackaged The packaging timestamp.
public:
  static MIKTEXMPMCEEAPI(void) PutPackageManifest(MiKTeX::Core::Cfg& cfg, const PackageInfo& packageInfo, std::time_t timePackaged);

  /// Loads a package manifest from a `Cfg` container.
  /// @param cfg The `Cfg` instance.
  /// @param packageId Identifies the package manifest.
  /// @param texmfPrefix The TEXMF prefix to be stripped from file names.
  /// @return Returns the package manifest.
public:
  static MIKTEXMPMCEEAPI(PackageInfo) GetPackageManifest(const MiKTeX::Core::Cfg& cfg, const std::string& packageId, const std::string& texmfPrefix);

  /// Strips the TEXMF prefix from a string.
  /// @param str The string to be stripped.
  /// @param[out] result The result to be filled with the stripped string.
  /// @return Returns `true`, if the prefix was stripped.
public:
  static MIKTEXMPMCEEAPI(bool) StripTeXMFPrefix(const std::string& str, std::string& result);

  /// Gets the configured proxy server.
  /// @param url Identifies the scheme the proxy must support.
  /// @param[out] proxySettings The proxy settings.
  /// @return Returns `true`, if the proxy server is configured.
  /// @see SetProxy
public:
  static MIKTEXMPMCEEAPI(bool) TryGetProxy(const std::string& url, ProxySettings& proxySettings);

  /// Gets the configured proxy server.
  /// @param[out] proxySettings The proxy settings.
  /// @return Returns `true`, if the proxy server is configured.
  /// @see SetProxy
public:
  static MIKTEXMPMCEEAPI(bool) TryGetProxy(ProxySettings& proxySettings);

  /// Gets the configured proxy server.
  /// @param url Identifies the scheme the proxy must support.
  /// @exception MiKTeXException No proxy is configured.
  /// @see SetProxy
public:
  static MIKTEXMPMCEEAPI(ProxySettings) GetProxy(const std::string& url);

  /// Gets the configured proxy server.
  /// @exception MiKTeXException No proxy is configured.
  /// @see SetProxy
public:
  static MIKTEXMPMCEEAPI(ProxySettings) GetProxy();

  /// Configures a proxy server.
  /// @param proxySettings The proxy settings.
  /// @see GetProxy
public:
  static MIKTEXMPMCEEAPI(void) SetProxy(const ProxySettings& proxySettings);
};

MIKTEX_PACKAGES_END_NAMESPACE;

#endif
