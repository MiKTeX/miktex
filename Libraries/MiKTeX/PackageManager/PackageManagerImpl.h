/**
 * @file PackageManagerImpl.h
 * @author Christian Schenk
 * @brief PackageManager implementation
 *
 * @copyright Copyright © 2001-2022 Christian Schenk
 *
 * This file is part of MiKTeX Package Manager.
 *
 * MiKTeX Package Manager is licensed under GNU General Public License version 2
 * or any later version.
 */

#pragma once

#include <map>
#include <string>

#include <miktex/Core/AutoResource>
#include <miktex/Core/Fndb>
#include <miktex/Core/LockFile>
#include <miktex/Core/MD5>

#include <miktex/PackageManager/PackageManager>

#include "internal.h"

#include "PackageDataStore.h"
#include "PackageRepositoryDataStore.h"
#include "WebSession.h"

#define MPM_LOCK_BEGIN(packageManager)                                      \
    {                                                                       \
        packageManager->Lock(std::chrono::seconds(10));                     \
        MIKTEX_AUTO(packageManager->Unlock());

#define MPM_LOCK_END()                                                      \
    }

MPM_INTERNAL_BEGIN_NAMESPACE;

typedef std::map<std::string, MiKTeX::Core::MD5, MiKTeX::Core::less_icase_dos> FileDigestTable;

class PackageManagerImpl :
    public std::enable_shared_from_this<PackageManagerImpl>,
    public MiKTeX::Packages::PackageManager,
    public MiKTeX::Core::ICreateFndbCallback,
    public IProgressNotify_
{

public:

    MIKTEXTHISCALL ~PackageManagerImpl() override;

    std::unique_ptr<class PackageInstaller> MIKTEXTHISCALL CreateInstaller(const MiKTeX::Packages::PackageInstaller::InitInfo& initInfo) override;
    std::unique_ptr<MiKTeX::Packages::PackageInstaller> MIKTEXTHISCALL CreateInstaller() override;
    std::unique_ptr<class MiKTeX::Packages::PackageIterator> MIKTEXTHISCALL CreateIterator() override;

    void MIKTEXTHISCALL CreateMpmFndb() override
    {
        if (!packageDataStore.LoadedAllPackageManifests())
        {
            MPM_LOCK_BEGIN(this)
            {
                packageDataStore.Load();
            }
            MPM_LOCK_END();
        }
        return CreateMpmFndbNoLock();
    }

    void MIKTEXTHISCALL CreateMpmFndbNoLock();
    MiKTeX::Packages::PackageInfo MIKTEXTHISCALL GetPackageInfo(const std::string& packageId) override;
    void MIKTEXTHISCALL LoadDatabase(const MiKTeX::Util::PathName& path, bool isArchive) override;
    void MIKTEXTHISCALL UnloadDatabase() override;

    MiKTeX::Packages::RepositoryReleaseState MIKTEXTHISCALL GetRepositoryReleaseState() override
    {
        return repositories.GetRepositoryReleaseState();
    }

    void MIKTEXTHISCALL SetRepositoryReleaseState(MiKTeX::Packages::RepositoryReleaseState repositoryReleaseState) override
    {
        repositories.SetRepositoryReleaseState(repositoryReleaseState);
    }

    void MIKTEXTHISCALL DownloadRepositoryList() override
    {
        repositories.Download();
    }

    std::vector<MiKTeX::Packages::RepositoryInfo> MIKTEXTHISCALL GetRepositories() override
    {
        return repositories.GetRepositories();
    }

    std::string MIKTEXTHISCALL PickRepositoryUrl() override
    {
        return repositories.PickRepositoryUrl();
    }

    bool MIKTEXTHISCALL TryGetPackageInfo(const std::string& packageId, MiKTeX::Packages::PackageInfo& packageInfo) override;

private:

    bool MIKTEXTHISCALL ReadDirectory(const MiKTeX::Util::PathName& path, std::vector<std::string>& subDirNames, std::vector<std::string>& fileNames, std::vector<std::string>& fileNameInfos) override;
    bool MIKTEXTHISCALL OnProgress(unsigned level, const MiKTeX::Util::PathName& directory) override;

public:

    void OnProgress() override;

    bool MIKTEXTHISCALL TryGetRepositoryInfo(const std::string& url, MiKTeX::Packages::RepositoryInfo& repositoryInfo) override
    {
        return repositories.TryGetRepositoryInfo(url, repositoryInfo);
    }

    MiKTeX::Packages::RepositoryInfo MIKTEXTHISCALL CheckPackageRepository(const std::string& url) override
    {
        return repositories.CheckPackageRepository(url);
    }

    MiKTeX::Packages::RepositoryInfo MIKTEXTHISCALL VerifyPackageRepository(const std::string& url) override
    {
        return repositories.VerifyPackageRepository(url);
    }

    bool MIKTEXTHISCALL TryVerifyInstalledPackage(const std::string& packageId) override
    {
        if (!packageDataStore.LoadedAllPackageManifests())
        {
            MPM_LOCK_BEGIN(this)
            {
                packageDataStore.Load();
            }
            MPM_LOCK_END();
        }
        return TryVerifyInstalledPackageNoLock(packageId);
    }

    bool MIKTEXTHISCALL TryVerifyInstalledPackageNoLock(const std::string& packageId);

    std::string MIKTEXTHISCALL GetContainerPath(const std::string& packageId, bool useDisplayNames) override
    {
        if (!packageDataStore.LoadedAllPackageManifests())
        {
            MPM_LOCK_BEGIN(this)
            {
                packageDataStore.Load();
            }
            MPM_LOCK_END();
        }
        return GetContainerPathNoLock(packageId, useDisplayNames);
    }

    std::string MIKTEXTHISCALL GetContainerPathNoLock(const std::string& packageId, bool useDisplayNames);
    InstallationSummary MIKTEXTHISCALL GetInstallationSummary(bool userScope) override;
    PackageManagerImpl(const MiKTeX::Packages::PackageManager::InitInfo& initInfo);
    void Lock(std::chrono::milliseconds timeout);
    void Unlock();
    void ClearAll();

private:

    bool TryGetFileDigest(const MiKTeX::Util::PathName& prefix, const std::string& fileName, bool& haveDigest, MiKTeX::Core::MD5& digest);
    bool TryCollectFileDigests(const MiKTeX::Util::PathName& prefix, const std::vector<std::string>& files, FileDigestTable& fileDigests);
    void Dispose();

    std::unique_ptr<MiKTeX::Core::LockFile> lockFile;
    std::string remoteServiceBaseUrl;
    std::unique_ptr<MiKTeX::Trace::TraceStream> trace_error;
    std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mpm;
    std::unique_ptr<MiKTeX::Trace::TraceStream> trace_stopwatch;
    std::shared_ptr<MiKTeX::Core::Session> session = MIKTEX_SESSION();
    std::shared_ptr<WebSession> webSession = WebSession::Create(this);

public:

    WebSession* GetWebSession() const
    {
        return webSession.get();
    }

private:

    PackageDataStore packageDataStore;

public:

    PackageDataStore* GetPackageDataStore()
    {
        return &packageDataStore;
    }

private:

    PackageRepositoryDataStore repositories;

public:

    static std::string proxyUser;
    static std::string proxyPassword;
#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
    static bool localServer;
#endif
};

MPM_INTERNAL_END_NAMESPACE;
