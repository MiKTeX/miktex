/* PackageInstallerImpl.h:                              -*- C++ -*-

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

#if !defined(BF24CACAD93E4429BB9357433BBA2B22)
#define BF24CACAD93E4429BB9357433BBA2B22

#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

#include <miktex/Core/Cfg>
#include <miktex/Core/Session>
#include <miktex/Extractor/Extractor>
#include <miktex/Trace/Trace>

#include "PackageManagerImpl.h"

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
#include "COM/com-internal.h"
#endif

BEGIN_INTERNAL_NAMESPACE;

const time_t Y2000 = 946681200;

class DbLight
{
private:
  std::unique_ptr<MiKTeX::Core::Cfg> cfg;

public:
  DbLight() :
    cfg(MiKTeX::Core::Cfg::Create())
  {
  }

public:
  virtual ~DbLight()
  {
  }

public:
  MiKTeX::Core::MD5 GetDigest()
  {
    return cfg->GetDigest();
  }

private:
  bool TryGetValue(const std::string& deploymentName, const std::string& valueName, std::string& value)
  {
    return cfg->TryGetValue(deploymentName, valueName, value);
  }

public:
  void Read(const MiKTeX::Core::PathName& path)
  {
    cfg->Read(path);
  }

public:
  std::string FirstPackage()
  {
    std::shared_ptr<MiKTeX::Core::Cfg::Key> key = cfg->FirstKey();
    return key == nullptr ? "" : key->GetName();
  }

public:
  std::string NextPackage()
  {
    std::shared_ptr<MiKTeX::Core::Cfg::Key> key = cfg->NextKey();
    return key == nullptr ? "" : key->GetName();
  }

public:
  void Clear()
  {
    cfg = MiKTeX::Core::Cfg::Create();
  }

public:
  int GetArchiveFileSize(const std::string& deploymentName)
  {
    std::string str;
    if (!TryGetValue(deploymentName, "CabSize", str))
    {
      MIKTEX_FATAL_ERROR_2(T_("Unknown archive file size."), "package", deploymentName);
    }
    return atoi(str.c_str());
  }

public:
  MiKTeX::Core::MD5 GetArchiveFileDigest(const std::string& deploymentName)
  {
    std::string str;
    if (!cfg->TryGetValue(deploymentName, "CabMD5", str))
    {
      MIKTEX_FATAL_ERROR_2(T_("Unknown archive file digest."), "package", deploymentName);
    }
    return MiKTeX::Core::MD5::Parse(str.c_str());
  }

public:
  MiKTeX::Core::MD5 GetPackageDigest(const std::string& deploymentName)
  {
    std::string str;
    if (!cfg->TryGetValue(deploymentName, "MD5", str))
    {
      MIKTEX_FATAL_ERROR_2(T_("Unknown package digest."), "package", deploymentName);
    }
    return MiKTeX::Core::MD5::Parse(str.c_str());
  }

public:
  time_t GetTimePackaged(const std::string& deploymentName)
  {
    std::string str;
    if (!TryGetValue(deploymentName, "TimePackaged", str))
    {
      MIKTEX_FATAL_ERROR_2(T_("Unknown package time-stamp."), "package", deploymentName);
    }
    unsigned time = static_cast<unsigned>(atoi(str.c_str()));
    if (time < Y2000)
    {
      MIKTEX_FATAL_ERROR_2(T_("Invalid package time-stamp."), "package", deploymentName);
    }
    return time;
  }

public:
  PackageLevel GetPackageLevel(const std::string& deploymentName)
  {
    std::string str;
    if (!TryGetValue(deploymentName, "Level", str))
    {
      MIKTEX_FATAL_ERROR_2(T_("Unknown package level."), "package", deploymentName);
    }
    return CharToPackageLevel(str[0]);
  }

public:
  std::string GetPackageVersion(const std::string& deploymentName)
  {
    std::string version;
    if (!TryGetValue(deploymentName, "Version", version))
    {
      version = "";
    }
    return version;
  }

public:
  std::string GetPackageTargetSystem(const std::string& deploymentName)
  {
    std::string targetSystem;
    if (!TryGetValue(deploymentName, "TargetSystem", targetSystem))
    {
      targetSystem = "";
    }
    return targetSystem;
  }

public:
  MiKTeX::Extractor::ArchiveFileType GetArchiveFileType(const std::string& deploymentName)
  {
    std::string str;
    if (!TryGetValue(deploymentName, "Type", str))
    {
      return MiKTeX::Extractor::ArchiveFileType::MSCab;
    }
    if (str == "MSCab")
    {
      return MiKTeX::Extractor::ArchiveFileType::MSCab;
    }
    else if (str == "TarBzip2")
    {
      return MiKTeX::Extractor::ArchiveFileType::TarBzip2;
    }
    else if (str == "TarLzma")
    {
      return MiKTeX::Extractor::ArchiveFileType::TarLzma;
    }
    else
    {
      MIKTEX_FATAL_ERROR_2(T_("Unknown archive file type."), "package", deploymentName);
    }
  }

public:
  static PackageLevel CharToPackageLevel(int ch)
  {
    switch (toupper(ch))
    {
    case 'S': return PackageLevel::Essential;
    case 'M': return PackageLevel::Basic;
    case 'L': return PackageLevel::Advanced;
    case 'T': return PackageLevel::Complete;
    default:
      MIKTEX_FATAL_ERROR_2(T_("Invalid package level."), "level", std::to_string(ch));
    }
  }
};

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
  PackageInstallerImpl(std::shared_ptr<MiKTeX::Packages::D6AAD62216146D44B580E92711724B78::PackageManagerImpl> manager);

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
  void MIKTEXTHISCALL SetDownloadDirectory(const MiKTeX::Core::PathName& directory) override
  {
    this->downloadDirectory = directory;
  }

public:
  void MIKTEXTHISCALL UpdateDb() override;

public:
  void MIKTEXTHISCALL UpdateDbAsync() override;

public:
  void MIKTEXTHISCALL FindUpdates() override;

public:
  void MIKTEXTHISCALL FindUpdatesAsync() override;

public:
  std::vector<UpdateInfo> MIKTEXTHISCALL GetUpdates() override
  {
    return updates;
  }

public:
  void MIKTEXTHISCALL FindUpgrades(PackageLevel packageLevel) override;

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
  void StartWorkerThread(void (PackageInstallerImpl::*method) ());

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
  void MIKTEXTHISCALL OnBeginFileExtraction(const std::string& fileName, size_t uncompressedSize) override;

public:
  void MIKTEXTHISCALL OnEndFileExtraction(const std::string& fileName, size_t uncompressedSize) override;

public:
  bool MIKTEXTHISCALL OnError(const std::string& message) override;

public:
  bool MIKTEXTHISCALL OnProcessOutput(const void* pOutput, size_t n) override;

private:
  void NeedRepository();

private:
  void UpdateMpmFndb(const std::vector<std::string>& installedFiles, const std::vector<std::string>& removedFiles, const char* lpszPackageName);

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

public:
  void MIKTEXTHISCALL SetNoPostProcessing(bool noPostProcessing) override
  {
    this->noPostProcessing = noPostProcessing;
  }

public:
  void MIKTEXTHISCALL SetNoLocalServer(bool noLocalServer) override
  {
    this->noLocalServer = noLocalServer;
  }

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
  MiKTeX::Core::PathName downloadDirectory;

private:
  DbLight dbLight;

private:
  void InstallDbLight();

private:
  void LoadDbLight(bool download);

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
      trace_mpm->WriteLine("libmpm", T_("client wants to cancel"));
      trace_mpm->WriteLine("libmpm", T_("throwing OperationCancelledException"));
      throw MiKTeX::Core::OperationCancelledException();
    }
  }

#if defined(MIKTEX_WINDOWS)
private:
  void RegisterComponent(bool doRegister, const MiKTeX::Core::PathName& path, bool mustSucceed);
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
  bool autoFndbSync;

private:
  void SetAutoFndbSync(bool autoFndbSync)
  {
    if (this->autoFndbSync == autoFndbSync)
    {
      return;
    }
    trace_mpm->WriteFormattedLine("libmpm", T_("turning %s autoFndbSync"), autoFndbSync ? "on" : "off");
    this->autoFndbSync = autoFndbSync;
  }

private:
  bool noPostProcessing = false;

private:
  bool noLocalServer = false;

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
  void DoInstall();

private:
  void SetUpPackageManifestFiles(const MiKTeX::Core::PathName& directory);

private:
  void HandleObsoletePackageManifestFiles(const MiKTeX::Core::PathName& temporaryDirectory);

private:
  void CleanUpUserDatabase();

private:
  void Download(const std::string& url, const MiKTeX::Core::PathName& dest, size_t expectedSize = 0);

private:
  void Download(const MiKTeX::Core::PathName& fileName, size_t expectedSize = 0);

private:
  void RemoveFiles(const std::vector<std::string>& toBeRemoved, bool silently = false);

private:
  void ExtractFiles(const MiKTeX::Core::PathName& archiveFileName, MiKTeX::Extractor::ArchiveFileType archiveFileType);

private:
  void CopyFiles(const MiKTeX::Core::PathName& pathSourceRoot, const std::vector<std::string>& fileList);

private:
  void AddToFileList(std::vector<std::string>& fileList, const MiKTeX::Core::PathName& fileName) const;

private:
  void RemoveFromFileList(std::vector<std::string>& fileList, const MiKTeX::Core::PathName& fileName) const;

private:
  void CopyPackage(const MiKTeX::Core::PathName& pathSourceRoot, const std::string& deploymentName);

private:
  bool MIKTEXTHISCALL ReadDirectory(const MiKTeX::Core::PathName& path, std::vector<std::string>& subDirNames, std::vector<std::string>& fileNames, std::vector<std::string>& fileNameInfos) override;

private:
  bool MIKTEXTHISCALL OnProgress(unsigned level, const MiKTeX::Core::PathName& directory) override;

private:
  void RemovePackage(const std::string& deploymentName);

private:
  void InstallPackage(const std::string& deploymentName);

private:
  void MyCopyFile(const MiKTeX::Core::PathName& source, const MiKTeX::Core::PathName& dest, size_t& size);

private:
  void DownloadPackage(const std::string& deploymentName);

private:
  bool CheckArchiveFile(const std::string& deploymentName, const MiKTeX::Core::PathName& archiveFileName, bool mustBeOk);

private:
  void CheckDependencies(std::set<std::string>& packages, const std::string& deploymentName, bool force, int level);

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
private:
  bool UseLocalServer();
#endif

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
private:
  void ConnectToServer();
#endif

#if defined(MIKTEX_WINDOWS)
private:
  void MyCoInitialize();
#endif

#if defined(MIKTEX_WINDOWS)
private:
  void MyCoUninitialize();
#endif

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_error;

private:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mpm;

private:
  std::shared_ptr<PackageManagerImpl> packageManager;

private:
  MiKTeX::Packages::PackageInstallerCallback* callback = nullptr;

private:
  std::vector<UpdateInfo> updates;

private:
  std::vector<UpgradeInfo> upgrades;

private:
  PackageLevel upgradeLevel = PackageLevel::None;

#if defined(MIKTEX_WINDOWS)
private:
  int numCoInitialize = 0;
#endif

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
private:
  struct
  {
    ATL::CComQIPtr<MiKTeXPackageManagerLib::IPackageManager> pManager;
    ATL::CComPtr<MiKTeXPackageManagerLib::IPackageInstaller> pInstaller;
  } localServer;
#endif
};

END_INTERNAL_NAMESPACE;

#endif
