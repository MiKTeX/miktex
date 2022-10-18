/**
 * @file PackageInstallerImpl.h
 * @author Christian Schenk
 * @brief PackageInstaller implementation
 *
 * @copyright Copyright © 2001-2022 Christian Schenk
 *
 * This file is part of MiKTeX Package Manager.
 *
 * MiKTeX Package Manager is licensed under GNU General Public License version 2
 * or any later version.
 */

#pragma once

#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

#include <miktex/Core/Cfg>
#include <miktex/Core/Session>
#include <miktex/Core/TemporaryFile>
#include <miktex/Extractor/Extractor>
#include <miktex/Trace/Trace>

#include "PackageManagerImpl.h"
#include "RepositoryManifest.h"

#if defined(MIKTEX_WINDOWS)
#include <miktex/Core/win/COMInitializer>
#endif

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
#include "COM/com-internal.h"
#endif

MPM_INTERNAL_BEGIN_NAMESPACE;

class PackageInstallerImpl :
#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
    public MiKTeXPackageManagerLib::IPackageInstallerCallback,
#endif
    public IProgressNotify_,
    public MiKTeX::Core::ICreateFndbCallback,
    public MiKTeX::Core::IRunProcessCallback,
    public MiKTeX::Extractor::IExtractCallback,
    public MiKTeX::Packages::PackageInstaller
{

public:

    PackageInstallerImpl(std::shared_ptr<MiKTeX::Packages::D6AAD62216146D44B580E92711724B78::PackageManagerImpl> manager, const InitInfo& initInfo);
    MIKTEXTHISCALL ~PackageInstallerImpl() override;

    void MIKTEXTHISCALL Dispose() override;
    void MIKTEXTHISCALL Download() override;
    void MIKTEXTHISCALL DownloadAsync() override;
    void MIKTEXTHISCALL FindUpdatesAsync() override;
    void MIKTEXTHISCALL FindUpgradesAsync(PackageLevel packageLevel) override;
    void MIKTEXTHISCALL InstallRemove(Role role) override;
    void MIKTEXTHISCALL InstallRemoveAsync(Role role) override;
    void MIKTEXTHISCALL OnBeginFileExtraction(const std::string& fileName, std::size_t uncompressedSize) override;
    void MIKTEXTHISCALL OnEndFileExtraction(const std::string& fileName, std::size_t uncompressedSize) override;
    bool MIKTEXTHISCALL OnError(const std::string& message) override;
    bool MIKTEXTHISCALL OnProcessOutput(const void* pOutput, std::size_t n) override;
    void OnProgress() override;
    void MIKTEXTHISCALL RegisterComponents(bool doRegister) override;
    void MIKTEXTHISCALL SetCallback(PackageInstallerCallback* callback) override;
    void MIKTEXTHISCALL UpdateDbAsync() override;
    void MIKTEXTHISCALL WaitForCompletion() override;

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
    STDMETHOD_(ULONG, AddRef)();
    HRESULT __stdcall OnProgress(LONG nf, VARIANT_BOOL* pDoContinue) override;
    HRESULT __stdcall OnRetryableError(BSTR message, VARIANT_BOOL* pDoContinue) override;
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);
    STDMETHOD_(ULONG, Release)();
    HRESULT __stdcall ReportLine(BSTR line) override;
#endif

    void MIKTEXTHISCALL FindUpdates() override
    {
        MPM_LOCK_BEGIN(this->packageManager)
        {
            FindUpdatesNoLock();
        }
        MPM_LOCK_END();
    }

    void MIKTEXTHISCALL FindUpgrades(PackageLevel packageLevel) override
    {
        // TODO: locking
        FindUpgradesNoLock(packageLevel);
    }

    ProgressInfo MIKTEXTHISCALL GetProgressInfo() override
    {
        std::lock_guard<std::mutex> lockGuard(progressIndicatorMutex);
        if (progressInfo.numErrors > 0)
        {
            throw threadMiKTeXException;
        }
        return progressInfo;
    }

    std::vector<UpdateInfo> MIKTEXTHISCALL GetUpdates() override
    {
        return updates;
    }

    std::vector<UpgradeInfo> MIKTEXTHISCALL GetUpgrades() override
    {
        return upgrades;
    }

    bool MIKTEXTHISCALL IsRunning() const override
    {
        if (!workerThread.joinable())
        {
            return false;
        }
        return !progressInfo.ready;
    }

    void MIKTEXTHISCALL SetDownloadDirectory(const MiKTeX::Util::PathName& directory) override
    {
        this->downloadDirectory = directory;
    }

    void MIKTEXTHISCALL SetFileList(const std::vector<std::string>& tbi) override
    {
        SetFileLists(tbi, std::vector<std::string>());
    }

    void MIKTEXTHISCALL SetFileLists(const std::vector<std::string>& tbi, const std::vector<std::string>& tbr) override
    {
        this->toBeInstalled = tbi;
        this->toBeRemoved = tbr;
    }

    void MIKTEXTHISCALL SetPackageLevel(PackageLevel packageLevel) override
    {
        taskPackageLevel = packageLevel;
    }

    void MIKTEXTHISCALL SetRepository(const std::string& repository) override
    {
        repositoryType = PackageRepositoryDataStore::DetermineRepositoryType(repository);
        this->repository = repository;
    }

    void MIKTEXTHISCALL UpdateDb(UpdateDbOptionSet options) override
    {
        MPM_LOCK_BEGIN(this->packageManager)
        {
            UpdateDbNoLock(options);
        }
        MPM_LOCK_END();
    }

private:

    enum ErrorCode
    {
        ERROR_CANNOT_DELETE,
        ERROR_CORRUPTED_PACKAGE,
        ERROR_MISSING_PACKAGE,
        ERROR_SIZE_MISMATCH,
        ERROR_SOURCE_FILE_NOT_FOUND,
    };

    void CalculateExpenditure(bool downloadOnly = false);
    bool CheckArchiveFile(const std::string& packageId, const MiKTeX::Util::PathName& archiveFileName, bool mustBeOk);
    void CheckDependencies(std::set<std::string>& packages, const std::string& packageId, bool force, int level);
    void CleanUpUserDatabase();
    void CopyFiles(const MiKTeX::Util::PathName& pathSourceRoot, const std::vector<std::string>& fileList);
    void CopyPackage(const MiKTeX::Util::PathName& pathSourceRoot, const std::string& packageId);
    void Download(const MiKTeX::Util::PathName& fileName, std::size_t expectedSize = 0);
    void Download(const std::string& url, const MiKTeX::Util::PathName& dest, std::size_t expectedSize = 0);
    void DownloadPackage(const std::string& packageId);
    void DownloadThread();
    void ExtractFiles(const MiKTeX::Util::PathName& archiveFileName, MiKTeX::Extractor::ArchiveFileType archiveFileType);
    std::string FatalError(ErrorCode error);
    void FindUpdatesNoLock();
    void FindUpdatesThread();
    void FindUpgradesNoLock(PackageLevel packageLevel);
    void FindUpgradesThread();
    void InstallPackage(const std::string& packageId, MiKTeX::Core::Cfg& packageManifests);
    void HandleObsoletePackageManifests(MiKTeX::Core::Cfg& cfgExisting, const MiKTeX::Core::Cfg& cfgNew);
    void InstallRemoveThread();
    void InstallRepositoryManifest(bool fromCache);
    void LoadRepositoryManifest(bool download);
    std::string MakeUrl(const std::string& relPath);
    void MyCopyFile(const MiKTeX::Util::PathName& source, const MiKTeX::Util::PathName& dest, std::size_t& size);
    void NeedRepository();
    bool MIKTEXTHISCALL OnProgress(unsigned level, const MiKTeX::Util::PathName& directory) override;
    bool MIKTEXTHISCALL ReadDirectory(const MiKTeX::Util::PathName& path, std::vector<std::string>& subDirNames, std::vector<std::string>& fileNames, std::vector<std::string>& fileNameInfos) override;
    void RegisterComponents(bool doRegister, const std::vector<std::string>& packages);
    void RemoveFiles(const std::vector<std::string>& toBeRemoved, bool silently = false);
    void RemovePackage(const std::string& packageId, MiKTeX::Core::Cfg& packageManifests);
    void ReportLine(const std::string& s);
    void RunOneMiKTeXUtility(const std::vector<std::string>& arguments);
    void StartWorkerThread(void (PackageInstallerImpl::* method)());
    void UpdateDbNoLock(UpdateDbOptionSet options);
    void UpdateDbThread();
    void UpdateFndb(const std::unordered_set<MiKTeX::Util::PathName>& installedFiles, const std::unordered_set<MiKTeX::Util::PathName>& removedFiles, const std::string& packageId);

#if defined(MIKTEX_WINDOWS)
    void RegisterComponent(bool doRegister, const MiKTeX::Util::PathName& path, bool mustSucceed);
#endif

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
    bool UseLocalServer();
    void ConnectToServer();
#endif

    bool AbortOrRetry(const std::string& message)
    {
        return callback == nullptr || !callback->OnRetryableError(message);
    }

    void Notify(MiKTeX::Packages::Notification nf = MiKTeX::Packages::Notification::None)
    {
        if (callback != nullptr && !callback->OnProgress(nf))
        {
            trace_mpm->WriteLine(TRACE_FACILITY, T_("client wants to cancel"));
            trace_mpm->WriteLine(TRACE_FACILITY, T_("throwing OperationCancelledException"));
            throw MiKTeX::Core::OperationCancelledException();
        }
    }

    void RegisterComponents(bool doRegister, const std::vector<std::string>& packages, const std::vector<std::string>& packages2)
    {
        RegisterComponents(doRegister, packages);
        RegisterComponents(doRegister, packages2);
    }

    MiKTeX::Packages::PackageInstallerCallback* callback = nullptr;
    Role currentRole;
    MiKTeX::Util::PathName downloadDirectory;
    bool enablePostProcessing = true;
    std::unordered_set<MiKTeX::Util::PathName> installedFiles;
    PackageDataStore* packageDataStore = nullptr;
    std::shared_ptr<PackageManagerImpl> packageManager;
    std::mutex progressIndicatorMutex;
    ProgressInfo progressInfo;
    std::unordered_set<MiKTeX::Util::PathName> removedFiles;
    std::string repository;
    RepositoryManifest repositoryManifest;
    MiKTeX::Packages::RepositoryReleaseState repositoryReleaseState = MiKTeX::Packages::RepositoryReleaseState::Unknown;
    MiKTeX::Packages::RepositoryType repositoryType = MiKTeX::Packages::RepositoryType::Unknown;
    std::shared_ptr<MiKTeX::Core::Session> session;
    MiKTeX::Packages::PackageLevel taskPackageLevel = MiKTeX::Packages::PackageLevel::None;
    MiKTeX::Core::MiKTeXException threadMiKTeXException;
    clock_t timeStarted;
    std::vector<std::string> toBeInstalled;
    std::vector<std::string> toBeRemoved;
    std::mutex thisMutex;
    std::unique_ptr<MiKTeX::Trace::TraceStream> trace_error;
    std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mpm;
    std::unique_ptr<MiKTeX::Trace::TraceStream> trace_stopwatch;
    std::vector<UpdateInfo> updates;
    PackageLevel upgradeLevel = PackageLevel::None;
    std::vector<UpgradeInfo> upgrades;
    std::thread workerThread;

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
    struct LocalServer :
        public MiKTeX::Core::COMInitializer
    {
        ATL::CComPtr<MiKTeXPackageManagerLib::IPackageInstaller> pInstaller;
        ATL::CComQIPtr<MiKTeXPackageManagerLib::IPackageManager> pManager;
    };
    std::unique_ptr<LocalServer> localServer;
#endif
};

MPM_INTERNAL_END_NAMESPACE;
