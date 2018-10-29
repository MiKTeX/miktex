/* PackageManagerImpl.h:                                -*- C++ -*-

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

#if !defined(D76F495437014794AC4EF6832E8EEE52)
#define D76F495437014794AC4EF6832E8EEE52

#include <map>
#include <unordered_map>

#include <miktex/Core/Cfg>
#include <miktex/Core/Fndb>
#include <miktex/Core/equal_icase>
#include <miktex/Core/hash_icase>
#include <miktex/Core/less_icase_dos>

#include "miktex/PackageManager/PackageManager.h"

#include "internal.h"
#include "PackageRepositoryDataStore.h"
#include "WebSession.h"

BEGIN_INTERNAL_NAMESPACE;

struct InstalledFileInfo
{
  unsigned long refCount = 0;
};

typedef std::map<std::string, MiKTeX::Core::MD5, MiKTeX::Core::less_icase_dos> FileDigestTable;

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
    return MiKTeX::Core::PathName::Compare(str1.c_str(), str2.c_str()) == 0;
  }
};

class PackageManagerImpl :
  public std::enable_shared_from_this<PackageManagerImpl>,
  public MiKTeX::Packages::PackageManager,
  public MiKTeX::Core::ICreateFndbCallback,
  public IProgressNotify_
{
public:
  MIKTEXTHISCALL ~PackageManagerImpl() override;

public:
  std::unique_ptr<MiKTeX::Packages::PackageInstaller> MIKTEXTHISCALL CreateInstaller() override;

public:
  std::unique_ptr<class MiKTeX::Packages::PackageIterator> MIKTEXTHISCALL CreateIterator() override;

public:
  void MIKTEXTHISCALL CreateMpmFndb() override;

public:
  unsigned long MIKTEXTHISCALL GetFileRefCount(const MiKTeX::Core::PathName& path) override;

public:
  MiKTeX::Packages::PackageInfo MIKTEXTHISCALL GetPackageInfo(const std::string& deploymentName) override;

public:
  void MIKTEXTHISCALL LoadDatabase(const MiKTeX::Core::PathName& path) override;

public:
  void MIKTEXTHISCALL UnloadDatabase() override;

public:
  MiKTeX::Packages::RepositoryReleaseState MIKTEXTHISCALL GetRepositoryReleaseState() override
  {
    return repositories.GetRepositoryReleaseState();
  }

public:
  void MIKTEXTHISCALL SetRepositoryReleaseState(MiKTeX::Packages::RepositoryReleaseState repositoryReleaseState) override
  {
    repositories.SetRepositoryReleaseState(repositoryReleaseState);
  }

public:
  void MIKTEXTHISCALL DownloadRepositoryList() override
  {
    repositories.Download();
  }

public:
  std::vector<MiKTeX::Packages::RepositoryInfo> MIKTEXTHISCALL GetRepositories() override
  {
    return repositories.GetRepositories();
  }

public:
  std::string MIKTEXTHISCALL PickRepositoryUrl() override
  {
    return repositories.PickRepositoryUrl();
  }

public:
  bool MIKTEXTHISCALL TryGetPackageInfo(const std::string& deploymentName, MiKTeX::Packages::PackageInfo& packageInfo) override;

private:
  bool MIKTEXTHISCALL ReadDirectory(const MiKTeX::Core::PathName& path, std::vector<std::string>& subDirNames, std::vector<std::string>& fileNames, std::vector<std::string>& fileNameInfos) override;

private:
  bool MIKTEXTHISCALL OnProgress(unsigned level, const MiKTeX::Core::PathName& directory) override;

public:
  void OnProgress() override;

public:
  bool MIKTEXTHISCALL TryGetRepositoryInfo(const std::string& url, MiKTeX::Packages::RepositoryInfo& repositoryInfo) override
  {
    return repositories.TryGetRepositoryInfo(url, repositoryInfo);
  }


public:
  MiKTeX::Packages::RepositoryInfo MIKTEXTHISCALL CheckPackageRepository(const std::string& url) override
  {
    return repositories.CheckPackageRepository(url);
  }

public:
  MiKTeX::Packages::RepositoryInfo MIKTEXTHISCALL VerifyPackageRepository(const std::string& url) override
  {
    return repositories.VerifyPackageRepository(url);
  }

public:
  bool MIKTEXTHISCALL TryVerifyInstalledPackage(const std::string& deploymentName) override;

public:
  std::string MIKTEXTHISCALL GetContainerPath(const std::string& deploymentName, bool useDisplayNames) override;

public:
  PackageManagerImpl(const MiKTeX::Packages::PackageManager::InitInfo& initInfo);

public:
  void ClearAll();

public:
  void IncrementFileRefCounts(const std::string& deploymentName);

public:
  void NeedInstalledFileInfoTable();

public:
  void FlushVariablePackageTable();

public:
  void GetAllPackageDefinitions(std::vector<MiKTeX::Packages::PackageInfo>& packages);

public:
  MiKTeX::Packages::PackageInfo* TryGetPackageInfo(const std::string& deploymentName);

public:
  InstalledFileInfo * GetInstalledFileInfo(const char* lpszPath);

public:
  bool IsRemovable(const std::string& deploymentName);

public:
  time_t GetUserTimeInstalled(const std::string& deploymentName);

public:
  time_t GetCommonTimeInstalled(const std::string& deploymentName);

public:
  time_t GetTimeInstalled(const std::string& deploymentName);

public:
  bool IsPackageInstalled(const std::string& deploymentName);

public:
  bool IsPackageObsolete(const std::string& deploymentName);

public:
  void DeclarePackageObsolete(const std::string& deploymentName, bool obsolete);

public:
  void SetTimeInstalled(const std::string& deploymentName, time_t timeInstalled);

public:
  void SetReleaseState(const std::string& deploymentName, MiKTeX::Packages::RepositoryReleaseState releaseState);

public:
  MiKTeX::Packages::RepositoryReleaseState GetReleaseState(const std::string& deploymentName);

public:
  MiKTeX::Packages::PackageInfo* DefinePackage(const std::string& deploymentName, const MiKTeX::Packages::PackageInfo& packageinfo);

private:
  void LoadVariablePackageTable();

private:
  void IncrementFileRefCounts(const std::vector<std::string>& files);

private:
  void ParseAllPackageManifestFilesInDirectory(const MiKTeX::Core::PathName& directory);

private:
  void ParseAllPackageManifestFiles();

private:
  bool TryGetFileDigest(const MiKTeX::Core::PathName& prefix, const std::string& fileName, bool& haveDigest, MiKTeX::Core::MD5& digest);

private:
  bool TryCollectFileDigests(const MiKTeX::Core::PathName& prefix, const std::vector<std::string>& files, FileDigestTable& fileDigests);

private:
  void Dispose();

private:
  std::string remoteServiceBaseUrl;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_error;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mpm;

private:
  typedef std::unordered_map<std::string, MiKTeX::Packages::PackageInfo, MiKTeX::Core::hash_icase, MiKTeX::Core::equal_icase> PackageDefinitionTable;

private:
  PackageDefinitionTable packageTable;

private:
  typedef std::unordered_map<std::string, InstalledFileInfo, hash_path, equal_path> InstalledFileInfoTable;

private:
  InstalledFileInfoTable installedFileInfoTable;

private:
  bool parsedAllPackageManifestFiles = false;

private:
  std::unique_ptr<MiKTeX::Core::Cfg> commonVariablePackageTable;

private:
  std::unique_ptr<MiKTeX::Core::Cfg> userVariablePackageTable;

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();

private:
  std::shared_ptr<WebSession> webSession = WebSession::Create(this);

public:
  WebSession* GetWebSession() const
  {
    return webSession.get();
  }

private:
  PackageRepositoryDataStore repositories;

public:
  static std::string proxyUser;

public:
  static std::string proxyPassword;

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
public:
  static bool localServer;
#endif
};

END_INTERNAL_NAMESPACE;

#endif
