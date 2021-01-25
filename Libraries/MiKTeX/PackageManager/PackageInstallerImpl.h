/* PackageInstallerImpl.h:                              -*- C++ -*-

   Copyright (C) 2001-2021 Christian Schenk

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

#if !defined(BF24CACAD93E4429BB9357433BBA2B22)
#define BF24CACAD93E4429BB9357433BBA2B22

#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

#include <miktex/Core/Cfg>
#include <miktex/Core/Session>
#include <miktex/Core/TemporaryFile>
#include <miktex/Core/win/COMInitializer>
#include <miktex/Extractor/Extractor>
#include <miktex/Trace/Trace>

#include "PackageManagerImpl.h"
#include "RepositoryManifest.h"

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
#include "COM/com-internal.h"
#endif

MPM_INTERNAL_BEGIN_NAMESPACE;

class PackageInstallerImpl :
  public MiKTeX::Packages::PackageInstaller,
  public IProgressNotify_,
  public MiKTeX::Core::ICreateFndbCallback,
#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
  public MiKTeXPackageManagerLib::IPackageInstallerCallback,
#endif
  public MiKTeX::Extractor::IExtractCallback,
  public MiKTeX::Core::IRunProcessCallback
{
private:
  enum ErrorCode {
    ERROR_CANNOT_DELETE,
    ERROR_MISSING_PACKAGE, ERROR_CORRUPTED_PACKAGE,
    ERROR_SOURCE_FILE_NOT_FOUND,
    ERROR_SIZE_MISMATCH,
  };

public:
  PackageInstallerImpl(std::shared_ptr<MiKTeX::Packages::D6AAD62216146D44B580E92711724B78::PackageManagerImpl> manager, const InitInfo& initInfo);

public:
  MIKTEXTHISCALL ~PackageInstallerImpl() override;

public:
  void MIKTEXTHISCALL Dispose() override;

public:
  void MIKTEXTHISCALL SetRepository(const std::string& repository) override
  {
    repositoryType = PackageRepositoryDataStore::DetermineRepositoryType(repository);
    this->repository = repository;
  }

public:
  void MIKTEXTHISCALL SetDownloadDirectory(const MiKTeX::Util::PathName& directory) override
  {
    this->downloadDirectory = directory;
  }

public:
  void MIKTEXTHISCALL UpdateDb(UpdateDbOptionSet options) override
  {
    MPM_LOCK_BEGIN(this->packageManager)
    {
      UpdateDbNoLock(options);
    }
    MPM_LOCK_END();
  }

private:
  void UpdateDbNoLock(UpdateDbOptionSet options);

public:
  void MIKTEXTHISCALL UpdateDbAsync() override;

public:
  void MIKTEXTHISCALL FindUpdates() override
  {
    MPM_LOCK_BEGIN(this->packageManager)
    {
      FindUpdatesNoLock();
    }
    MPM_LOCK_END();
  }

private:
  void FindUpdatesNoLock();

public:
  void MIKTEXTHISCALL FindUpdatesAsync() override;

public:
  std::vector<UpdateInfo> MIKTEXTHISCALL GetUpdates() override
  {
    return updates;
  }

public:
  void MIKTEXTHISCALL FindUpgrades(PackageLevel packageLevel) override
  {
    FindUpgradesNoLock(packageLevel);
  }

private:
  void FindUpgradesNoLock(PackageLevel packageLevel);

public:
  void MIKTEXTHISCALL FindUpgradesAsync(PackageLevel packageLevel) override;

public:
  std::vector<UpgradeInfo> MIKTEXTHISCALL GetUpgrades() override
  {
    return upgrades;
  }

public:
  void MIKTEXTHISCALL InstallRemove(Role role) override;

public:
  void MIKTEXTHISCALL InstallRemoveAsync(Role role) override;

public:
  void MIKTEXTHISCALL WaitForCompletion() override;

public:
  void MIKTEXTHISCALL Download() override;

public:
  void MIKTEXTHISCALL DownloadAsync() override;

private:
  void StartWorkerThread(void (PackageInstallerImpl::*method)());

public:
  ProgressInfo MIKTEXTHISCALL GetProgressInfo() override
  {
    std::lock_guard<std::mutex> lockGuard(progressIndicatorMutex);
    if (progressInfo.numErrors > 0)
    {
      throw threadMiKTeXException;
    }
    return progressInfo;
  }

public:
  void MIKTEXTHISCALL SetCallback(PackageInstallerCallback* callback) override;

public:
  void MIKTEXTHISCALL SetFileLists(const std::vector<std::string>& tbi, const std::vector<std::string>& tbr) override
  {
    this->toBeInstalled = tbi;
    this->toBeRemoved = tbr;
  }

public:
  void MIKTEXTHISCALL SetPackageLevel(PackageLevel packageLevel) override
  {
    taskPackageLevel = packageLevel;
  }

public:
  void MIKTEXTHISCALL SetFileList(const std::vector<std::string>& tbi) override
  {
    SetFileLists(tbi, std::vector<std::string>());
  }

public:
  void OnProgress() override;

public:
  void MIKTEXTHISCALL OnBeginFileExtraction(const std::string& fileName, std::size_t uncompressedSize) override;

public:
  void MIKTEXTHISCALL OnEndFileExtraction(const std::string& fileName, std::size_t uncompressedSize) override;

public:
  bool MIKTEXTHISCALL OnError(const std::string& message) override;

public:
  bool MIKTEXTHISCALL OnProcessOutput(const void* pOutput, std::size_t n) override;

private:
  void NeedRepository();

private:
  void UpdateFndb(const std::unordered_set<MiKTeX::Util::PathName>& installedFiles, const std::unordered_set<MiKTeX::Util::PathName>& removedFiles, const std::string& packageId);

private:
  void CalculateExpenditure(bool downloadOnly = false);

private:
  std::string FatalError(ErrorCode error);

private:
  std::thread workerThread;

public:
  bool MIKTEXTHISCALL IsRunning() const override
  {
    if (!workerThread.joinable())
    {
      return false;
    }
    return !progressInfo.ready;
  }

public:
  void MIKTEXTHISCALL RegisterComponents(bool doRegister) override;

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
public:
  STDMETHOD(QueryInterface) (REFIID riid, LPVOID* ppvObj);

public:
  STDMETHOD_(ULONG, AddRef) ();

public:
  STDMETHOD_(ULONG, Release) ();

public:
  HRESULT __stdcall ReportLine(BSTR line) override;

public:
  HRESULT __stdcall OnRetryableError(BSTR message, VARIANT_BOOL* pDoContinue) override;

public:
  HRESULT __stdcall OnProgress(LONG nf, VARIANT_BOOL* pDoContinue) override;
#endif

private:
  Role currentRole;

private:
  std::string repository;

private:
  MiKTeX::Util::PathName downloadDirectory;

private:
  RepositoryManifest repositoryManifest;

private:
  void InstallRepositoryManifest(bool fromCache);

private:
  void LoadRepositoryManifest(bool download);

private:
  MiKTeX::Packages::RepositoryType repositoryType = MiKTeX::Packages::RepositoryType::Unknown;

private:
  MiKTeX::Packages::RepositoryReleaseState repositoryReleaseState = MiKTeX::Packages::RepositoryReleaseState::Unknown;

private:
  void ReportLine(const std::string& s);

private:
  std::string MakeUrl(const std::string& relPath);

private:
  bool AbortOrRetry(const std::string& message)
  {
    return callback == nullptr || !callback->OnRetryableError(message);
  }

private:
  void Notify(MiKTeX::Packages::Notification nf = MiKTeX::Packages::Notification::None)
  {
    if (callback != nullptr && !callback->OnProgress(nf))
    {
      trace_mpm->WriteLine(TRACE_FACILITY, T_("client wants to cancel"));
      trace_mpm->WriteLine(TRACE_FACILITY, T_("throwing OperationCancelledException"));
      throw MiKTeX::Core::OperationCancelledException();
    }
  }

#if defined(MIKTEX_WINDOWS)
private:
  void RegisterComponent(bool doRegister, const MiKTeX::Util::PathName& path, bool mustSucceed);
#endif

private:
  void RegisterComponents(bool doRegister, const std::vector<std::string>& packages);

private:
  void RegisterComponents(bool doRegister, const std::vector<std::string>& packages, const std::vector<std::string>& packages2)
  {
    RegisterComponents(doRegister, packages);
    RegisterComponents(doRegister, packages2);
  }

private:
  void RunIniTeXMF(const std::vector<std::string>& arguments);

private:
  std::unordered_set<MiKTeX::Util::PathName> installedFiles;

private:
  std::unordered_set<MiKTeX::Util::PathName> removedFiles;

private:
  bool enablePostProcessing = true;

private:
  ProgressInfo progressInfo;

private:
  std::shared_ptr<MiKTeX::Core::Session> session;

private:
  MiKTeX::Core::MiKTeXException threadMiKTeXException;

private:
  clock_t timeStarted;

private:
  std::vector<std::string> toBeInstalled;

private:
  std::vector<std::string> toBeRemoved;

private:
  std::mutex progressIndicatorMutex;

private:
  std::mutex thisMutex;

private:
  MiKTeX::Packages::PackageLevel taskPackageLevel = MiKTeX::Packages::PackageLevel::None;

private:
  void InstallRemoveThread();

private:
  void DownloadThread();

private:
  void UpdateDbThread();

private:
  void FindUpdatesThread();

private:
  void FindUpgradesThread();

private:
  void HandleObsoletePackageManifests(MiKTeX::Core::Cfg& cfgExisting, const MiKTeX::Core::Cfg& cfgNew);

private:
  void CleanUpUserDatabase();

private:
  void Download(const std::string& url, const MiKTeX::Util::PathName& dest, std::size_t expectedSize = 0);

private:
  void Download(const MiKTeX::Util::PathName& fileName, std::size_t expectedSize = 0);

private:
  void RemoveFiles(const std::vector<std::string>& toBeRemoved, bool silently = false);

private:
  void ExtractFiles(const MiKTeX::Util::PathName& archiveFileName, MiKTeX::Extractor::ArchiveFileType archiveFileType);

private:
  void CopyFiles(const MiKTeX::Util::PathName& pathSourceRoot, const std::vector<std::string>& fileList);

private:
  void CopyPackage(const MiKTeX::Util::PathName& pathSourceRoot, const std::string& packageId);

private:
  bool MIKTEXTHISCALL ReadDirectory(const MiKTeX::Util::PathName& path, std::vector<std::string>& subDirNames, std::vector<std::string>& fileNames, std::vector<std::string>& fileNameInfos) override;

private:
  bool MIKTEXTHISCALL OnProgress(unsigned level, const MiKTeX::Util::PathName& directory) override;

private:
  void RemovePackage(const std::string& packageId, MiKTeX::Core::Cfg& packageManifests);

private:
  void InstallPackage(const std::string& packageId, MiKTeX::Core::Cfg& packageManifests);

private:
  void MyCopyFile(const MiKTeX::Util::PathName& source, const MiKTeX::Util::PathName& dest, std::size_t& size);

private:
  void DownloadPackage(const std::string& packageId);

private:
  bool CheckArchiveFile(const std::string& packageId, const MiKTeX::Util::PathName& archiveFileName, bool mustBeOk);

private:
  void CheckDependencies(std::set<std::string>& packages, const std::string& packageId, bool force, int level);

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
private:
  bool UseLocalServer();
#endif

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
private:
  void ConnectToServer();
#endif

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_error;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mpm;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_stopwatch;

private:
  std::shared_ptr<PackageManagerImpl> packageManager;

private:
  PackageDataStore* packageDataStore = nullptr;

private:
  MiKTeX::Packages::PackageInstallerCallback* callback = nullptr;

private:
  std::vector<UpdateInfo> updates;

private:
  std::vector<UpgradeInfo> upgrades;

private:
  PackageLevel upgradeLevel = PackageLevel::None;

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
private:
  struct LocalServer :
    public MiKTeX::Core::COMInitializer
  {
    ATL::CComQIPtr<MiKTeXPackageManagerLib::IPackageManager> pManager;
    ATL::CComPtr<MiKTeXPackageManagerLib::IPackageInstaller> pInstaller;
  };
  std::unique_ptr<LocalServer> localServer;
#endif
};

MPM_INTERNAL_END_NAMESPACE;

#endif
