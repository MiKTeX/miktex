/* PackageInstaller.cpp:

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

#include "config.h"

#include <unordered_set>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>
#include <miktex/Core/FileStream>
#include <miktex/Core/Registry>
#include <miktex/Core/TemporaryDirectory>
#include <miktex/Core/TemporaryFile>
#include <miktex/Extractor/Extractor>

#if defined(MIKTEX_WINDOWS)
#include <miktex/Core/win/winAutoResource>
#include <miktex/Core/win/DllProc>
#include <miktex/Core/win/HResult>
#endif

#include "miktex/PackageManager/PackageManager"

#include "internal.h"
#include "PackageInstallerImpl.h"
#include "TpmParser.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Extractor;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

using namespace MiKTeX::Packages::D6AAD62216146D44B580E92711724B78;

#define LF "\n"

inline double Divide(double a, double b)
{
  return a / b;
}

string PackageInstallerImpl::MakeUrl(const string& relPath)
{
  return ::MakeUrl(repository, relPath);
}

PackageInstaller::~PackageInstaller() noexcept
{
}

MPMSTATICFUNC(bool) IsPureContainer(const string& deploymentName)
{
  return strncmp(deploymentName.c_str(), "_miktex-", 8) == 0;
}

MPMSTATICFUNC(bool) IsMiKTeXPackage(const string& deploymentName)
{
  return strncmp(deploymentName.c_str(), "miktex-", 7) == 0;
}

MPMSTATICFUNC(PathName) PrefixedPackageDefinitionFile(const string& deploymentName)
{
  PathName path(TEXMF_PREFIX_DIRECTORY);
  path /= MIKTEX_PATH_PACKAGE_DEFINITION_DIR;
  path /= deploymentName;
  path.AppendExtension(MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX);
  return path;
}

PackageInstallerImpl::PackageInstallerImpl(shared_ptr<PackageManagerImpl> manager) :
  packageManager(manager),
  session(Session::Get()),
  trace_error(TraceStream::Open(MIKTEX_TRACE_ERROR)),
  trace_mpm(TraceStream::Open(MIKTEX_TRACE_MPM))
{
  MIKTEX_ASSERT(
    PackageLevel::None < PackageLevel::Essential
    && PackageLevel::Essential < PackageLevel::Basic
    && PackageLevel::Basic < PackageLevel::Advanced
    && PackageLevel::Advanced < PackageLevel::Complete);
}

void PackageInstallerImpl::NeedRepository()
{
  if (repositoryType != RepositoryType::Unknown)
  {
    return;
  }
  string repository;
  RepositoryType repositoryType(RepositoryType::Unknown);
  if (packageManager->TryGetDefaultPackageRepository(repositoryType, repository) && !repository.empty())
  {
    SetRepository(repository);
  }
}

#if defined(MIKTEX_WINDOWS)
void PackageInstallerImpl::MyCoInitialize()
{
  HResult hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  if (hr.Failed())
  {
    MIKTEX_FATAL_ERROR_2(T_("The COM library could not be initialized."), "hr", hr.GetText());
  }
  ++numCoInitialize;
}
#endif

#if defined(MIKTEX_WINDOWS)
void PackageInstallerImpl::MyCoUninitialize()
{
  if (numCoInitialize == 0)
  {
    MIKTEX_UNEXPECTED();
  }
  CoUninitialize();
  --numCoInitialize;
}
#endif

void PackageInstallerImpl::SetCallback(PackageInstallerCallback* callback)
{
  this->callback = callback;
}

PackageInstallerImpl::~PackageInstallerImpl()
{
  try
  {
    Dispose();
  }
  catch (const exception&)
  {
  }
}

void PackageInstallerImpl::OnProgress()
{
  Notify();
}

void PackageInstallerImpl::Download(const string& url, const PathName& dest, size_t expectedSize)
{
  trace_mpm->WriteFormattedLine("libmpm", T_("going to download: %s => %s"), Q_(url), Q_(dest));

  if (expectedSize > 0)
  {
    ReportLine(fmt::format(T_("downloading {0} (expecting {1} bytes)..."), Q_(url), expectedSize));
  }
  else
  {
    ReportLine(fmt::format(T_("downloading {0}..."), Q_(url)));
  }

  // open the remote file
  unique_ptr<WebFile> webFile(packageManager->GetWebSession()->OpenUrl(url.c_str()));

  // open the local file
  FileStream destStream(File::Open(dest, FileMode::Create, FileAccess::Write, false));
  unique_ptr<TemporaryFile> downloadedFile = TemporaryFile::Create(dest);

  // receive the data
  trace_mpm->WriteFormattedLine("libmpm", T_("start writing on %s"), Q_(dest));
#if defined(CURL_MAX_WRITE_SIZE)
  const size_t bufsize = 2 * CURL_MAX_WRITE_SIZE;
#else
  const size_t bufsize = 32 * 1024;
#endif
  char buf[bufsize];
  size_t n;
  size_t received = 0;
  clock_t start = clock();
  clock_t start1 = start;
  size_t received1 = 0;
  while ((n = webFile->Read(buf, sizeof(buf))) > 0)
  {
    clock_t end1 = clock();

    destStream.Write(buf, n);

    received += n;
    received1 += n;

    // update progress info
    {
      lock_guard<mutex> lockGuard(progressIndicatorMutex);
      progressInfo.cbPackageDownloadCompleted += n;
      progressInfo.cbDownloadCompleted += n;
      if (end1 > start1 + 1 * CLOCKS_PER_SEC)
      {
        progressInfo.bytesPerSecond = static_cast<unsigned long>(Divide(received1, Divide(end1 - start1, CLOCKS_PER_SEC)));
        start1 = end1;
        received1 = 0;
      }
      double timePassed = clock() - timeStarted;
      double timeTotal = ((timePassed / progressInfo.cbDownloadCompleted) * progressInfo.cbDownloadTotal);
      progressInfo.timeRemaining = static_cast<unsigned long>((timeTotal - timePassed) / CLOCKS_PER_SEC);
    }

    Notify();
  }

  // close files
  destStream.Close();
  webFile->Close();

  clock_t end = clock();

  if (start == end)
  {
    ++end;
  }

  // report statistics
  double mb = Divide(received, 1000000);
  double seconds = Divide(end - start, CLOCKS_PER_SEC);
  trace_mpm->WriteFormattedLine("libmpm", T_("downloaded %.2f MB in %.2f seconds"), mb, seconds);
  ReportLine(fmt::format(T_("{0:.2f} MB, {1:.2f} Mbit/s"), mb, Divide(8 * mb, seconds)));

  if (expectedSize > 0 && expectedSize != received)
  {
    MIKTEX_FATAL_ERROR_2(FatalError(ERROR_SIZE_MISMATCH), "dest", dest.ToString(), "expectecSize", std::to_string(expectedSize), "received", std::to_string(received));
  }

  // keep the downloaded file
  downloadedFile->Keep();
}

void PackageInstallerImpl::OnBeginFileExtraction(const string& fileName, size_t uncompressedSize)
{
  UNUSED_ALWAYS(uncompressedSize);

  // update progress info
  {
    lock_guard<mutex> lockGuard(progressIndicatorMutex);
    progressInfo.fileName = fileName;
  }

  // update file name database
  if (autoFndbSync)
  {
    if (!Fndb::FileExists(fileName))
    {
      Fndb::Add(fileName);
    }
  }

  // notify client: beginning of file extraction
  Notify(Notification::InstallFileStart);
}

void PackageInstallerImpl::OnEndFileExtraction(const string& fileName, size_t uncompressedSize)
{
  // update file name database
  if (autoFndbSync && !fileName.empty())
  {
    if (!Fndb::FileExists(fileName))
    {
      Fndb::Add(fileName);
    }
  }

  // update progress info
  {
    lock_guard<mutex> lockGuard(progressIndicatorMutex);
    progressInfo.fileName = "";
    progressInfo.cFilesPackageInstallCompleted += 1;
    progressInfo.cFilesInstallCompleted += 1;
    progressInfo.cbPackageInstallCompleted += uncompressedSize;
    progressInfo.cbInstallCompleted += uncompressedSize;
  }

  // notify client: end of file extraction
  Notify(Notification::InstallFileEnd);
}

bool PackageInstallerImpl::OnError(const string& message)
{
  // we have a problem: let the client decide how to proceed
  return !AbortOrRetry(message);
}

void PackageInstallerImpl::ExtractFiles(const PathName& archiveFileName, ArchiveFileType archiveFileType)
{
  unique_ptr<MiKTeX::Extractor::Extractor> pExtractor(MiKTeX::Extractor::Extractor::CreateExtractor(archiveFileType));
  pExtractor->Extract(archiveFileName, session->GetSpecialPath(SpecialPath::InstallRoot), true, this, TEXMF_PREFIX_DIRECTORY);
}

void PackageInstallerImpl::InstallDbLight()
{
  // we must have a package repository
  NeedRepository();
  if (repositoryType == RepositoryType::Unknown)
  {
    repository = packageManager->PickRepositoryUrl();
    repositoryType = RepositoryType::Remote;
  }

  ReportLine(fmt::format(T_("visiting repository {0}..."), Q_(repository)));
  ReportLine(fmt::format(T_("repository type: {0}"), (repositoryType == RepositoryType::Remote
    ? T_("remote package repository")
    : (repositoryType == RepositoryType::Local
      ? T_("local package repository")
      : (repositoryType == RepositoryType::MiKTeXInstallation
        ? "other MiKTeX installation"
        : "MiKTeXDirect")))));

  // path to config dir
  PathName pathConfigDir(session->GetSpecialPath(SpecialPath::InstallRoot), MIKTEX_PATH_MIKTEX_CONFIG_DIR);

  if (repositoryType == RepositoryType::Remote || repositoryType == RepositoryType::Local)
  {
    // we need a temporary file when we download from the Internet
    unique_ptr<TemporaryFile> tempFile;

    ReportLine(T_("loading lightweight database..."));

    // full path to the database file
    PathName pathZzdb1;

    // pick up the database file
    if (repositoryType == RepositoryType::Remote)
    {
      // create a temporary file
      tempFile = TemporaryFile::Create();

      pathZzdb1 = tempFile->GetPathName();

      // update progress indicator
      {
        lock_guard<mutex> lockGuard(progressIndicatorMutex);
        progressInfo.deploymentName = MIKTEX_MPM_DB_LIGHT_FILE_NAME_NO_SUFFIX;
        progressInfo.displayName = T_("Lightweight package database");
        progressInfo.cbPackageDownloadCompleted = 0;
        progressInfo.cbPackageDownloadTotal = MPM_APSIZE_DB_LIGHT;
      }

      // download the database file
      Download(MakeUrl(MIKTEX_MPM_DB_LIGHT_FILE_NAME), pathZzdb1);
    }
    else
    {
      MIKTEX_ASSERT(repositoryType == RepositoryType::Local);
      pathZzdb1 = repository / MIKTEX_MPM_DB_LIGHT_FILE_NAME;
    }

    // unpack database
    unique_ptr<MiKTeX::Extractor::Extractor> pExtractor(MiKTeX::Extractor::Extractor::CreateExtractor(DB_ARCHIVE_FILE_TYPE));
    pExtractor->Extract(pathZzdb1, pathConfigDir);
  }
  else if (repositoryType == RepositoryType::MiKTeXDirect)
  {
    PathName pathMpmIniSrc(repository);
    pathMpmIniSrc /= MIKTEXDIRECT_PREFIX_DIR;
    pathMpmIniSrc /= MIKTEX_PATH_MPM_INI;
    PathName pathMpmIniDst = session->GetSpecialPath(SpecialPath::InstallRoot);
    pathMpmIniDst /= MIKTEX_PATH_MPM_INI;
    size_t size;
    MyCopyFile(pathMpmIniSrc, pathMpmIniDst, size);
  }
  else if (repositoryType == RepositoryType::MiKTeXInstallation)
  {
    PathName pathMpmIniSrc(repository);
    pathMpmIniSrc /= MIKTEX_PATH_MPM_INI;
    PathName pathMpmIniDst = session->GetSpecialPath(SpecialPath::InstallRoot);
    pathMpmIniDst /= MIKTEX_PATH_MPM_INI;
    size_t size;
    MyCopyFile(pathMpmIniSrc, pathMpmIniDst, size);
  }
  else
  {
    MIKTEX_UNEXPECTED();
  }
}

void PackageInstallerImpl::LoadDbLight(bool download)
{
  dbLight.Clear();

#if 1
  // path to mpm.ini
  PathName pathMpmIni(session->GetSpecialPath(SpecialPath::InstallRoot), MIKTEX_PATH_MPM_INI);
#else
  PathName commonMpmIni(
    session->GetSpecialPath(SpecialPath::CommonInstallRoot), MIKTEX_PATH_MPM_INI);

  PathName userMpmIni(
    session->GetSpecialPath(SpecialPath::UserInstallRoot), MIKTEX_PATH_MPM_INI);

  PathName pathMpmIni;

  if (session->IsAdminMode()
    || (
      !download
      && userMpmIni != commonMpmIni
      && File::Exists(commonMpmIni)
      && (!File::Exists(userMpmIni) || File::GetLastWriteTime(commonMpmIni) > File::GetLastWriteTime(userMpmIni))))
  {
    pathMpmIni = commonMpmIni;
  }
  else
  {
    pathMpmIni = commonMpmIni;
  }
#endif

  // install (if necessary)
  time_t ONE_DAY_IN_SECONDS = 86400;
  if (download || !File::Exists(pathMpmIni) || File::GetLastWriteTime(pathMpmIni) + ONE_DAY_IN_SECONDS < time(nullptr))
  {
    InstallDbLight();
  }

  // load mpm.ini
  dbLight.Read(pathMpmIni);

  // report digest
#if 0
  MD5 md5 = MD5::FromFile(pathMpmIni.GetData());
#endif
  ReportLine(fmt::format(T_("lightweight database digest: {0}"), dbLight.GetDigest().ToString()));
}

int CompareSerieses(const string& ver1, const string& ver2)
{
  if (ver1.empty() || ver2.empty())
  {
    return 0;
  }
  VersionNumber verNum1;
  VersionNumber verNum2;
  if (VersionNumber::TryParse(ver1, verNum1) && VersionNumber::TryParse(ver2, verNum2))
  {
    verNum1.n3 = 0;
    verNum1.n4 = 0;
    verNum2.n3 = 0;
    verNum2.n4 = 0;
    return verNum1.CompareTo(verNum2);
  }
  else
  {
    return -1;
  }
}

void PackageInstallerImpl::FindUpdates()
{
  trace_mpm->WriteLine("libmpm", T_("searching for updateable packages"));

  UpdateDb();

  LoadDbLight(false);

  updates.clear();

  for (string deploymentName = dbLight.FirstPackage(); !deploymentName.empty(); deploymentName = dbLight.NextPackage())
  {
    Notify();

    UpdateInfo updateInfo;
    updateInfo.deploymentName = deploymentName;
    updateInfo.timePackaged = dbLight.GetTimePackaged(deploymentName);
    updateInfo.version = dbLight.GetPackageVersion(deploymentName);

#if IGNORE_OTHER_SYSTEMS
    string targetSystem = dbLight.GetPackageTargetSystem(deploymentName);
    bool isOthertargetSystem = !(targetSystem.empty() || targetSystem == MIKTEX_SYSTEM_TAG);
    if (isOthertargetSystem)
    {
      continue;
    }
#endif

    bool isEssential = dbLight.GetPackageLevel(deploymentName) <= PackageLevel::Essential;

    const PackageInfo* package = packageManager->TryGetPackageInfo(deploymentName);

    if (package == nullptr || !packageManager->IsPackageInstalled(deploymentName))
    {
      if (isEssential)
      {
        trace_mpm->WriteFormattedLine("libmpm", T_("%s: new essential package"), deploymentName.c_str());
        updateInfo.action = UpdateInfo::ForceUpdate;
        updates.push_back(updateInfo);
      }
      continue;
    }

    // clean the user-installation directory
    if (!session->IsAdminMode()
      && session->GetSpecialPath(SpecialPath::UserInstallRoot) != session->GetSpecialPath(SpecialPath::CommonInstallRoot)
      && packageManager->GetUserTimeInstalled(deploymentName) != static_cast<time_t>(0)
      && packageManager->GetCommonTimeInstalled(deploymentName) != static_cast<time_t>(0))
    {
      if (!package->isRemovable)
      {
        MIKTEX_UNEXPECTED();
      }
      trace_mpm->WriteFormattedLine("libmpm", T_("%s: double installed"), deploymentName.c_str());
      updateInfo.action = UpdateInfo::ForceRemove;
      updates.push_back(updateInfo);
      continue;
    }

    // check the integrity of installed MiKTeX packages
    if (IsMiKTeXPackage(deploymentName)
      && !packageManager->TryVerifyInstalledPackage(deploymentName)
      && package->isRemovable)
    {
      // the package has been tampered with
      trace_mpm->WriteFormattedLine("libmpm", T_("%s: package is broken"), deploymentName.c_str());
      updateInfo.timePackaged = static_cast<time_t>(-1);
      updateInfo.action = UpdateInfo::Repair;
      updates.push_back(updateInfo);
      continue;
    }

    // compare digests, version numbers and time stamps
    MD5 md5 = dbLight.GetPackageDigest(deploymentName);
    if (md5 == package->digest)
    {
      // digests do match => no update necessary
      continue;
    }

    // check release state mismatch
    bool isReleaseStateDiff = package->releaseState != RepositoryReleaseState::Unknown
      && repositoryReleaseState != RepositoryReleaseState::Unknown
      && package->releaseState != repositoryReleaseState;
    if (isReleaseStateDiff)
    {
      trace_mpm->WriteFormattedLine("libmpm", T_("%s: package release state changed"), deploymentName.c_str());
    }
    else
    {
      trace_mpm->WriteFormattedLine("libmpm", T_("%s: server has a different version"), deploymentName.c_str());
    }
    trace_mpm->WriteFormattedLine("libmpm", T_("server digest: %s"), md5.ToString().c_str());
    trace_mpm->WriteFormattedLine("libmpm", T_("local digest: %s"), package->digest.ToString().c_str());
    if (!isReleaseStateDiff)
    {
      // compare time stamps
      time_t timePackaged = dbLight.GetTimePackaged(deploymentName);
      if (timePackaged <= package->timePackaged)
      {
        // server has an older package => no update
        // necessary
        continue;
      }
      // server has a newer package
      trace_mpm->WriteFormattedLine("libmpm", T_("%s: server has new version"), deploymentName.c_str());
    }

    if (!package->isRemovable)
    {
      trace_mpm->WriteFormattedLine("libmpm", T_("%s: no permission to update package"), deploymentName.c_str());
      updateInfo.action = UpdateInfo::KeepAdmin;
    }
    else
    {
      if (isReleaseStateDiff)
      {
        updateInfo.action = UpdateInfo::ReleaseStateChange;
      }
      else if (isEssential)
      {
        updateInfo.action = UpdateInfo::ForceUpdate;
      }
      else
      {
        updateInfo.action = UpdateInfo::Update;
      }
    }

    updates.push_back(updateInfo);
  }

  shared_ptr<PackageIterator> pIter(packageManager->CreateIterator());
  pIter->AddFilter({ PackageFilter::Obsolete });
  PackageInfo package;
  while (pIter->GetNext(package))
  {
    trace_mpm->WriteFormattedLine("libmpm", T_("%s: package is obsolete"), package.deploymentName.c_str());
    UpdateInfo updateInfo;
    updateInfo.deploymentName = package.deploymentName;
    updateInfo.timePackaged = package.timePackaged;
    if (package.isRemovable)
    {
      updateInfo.action = UpdateInfo::ForceRemove;
    }
    else
    {
      updateInfo.action = UpdateInfo::KeepObsolete;
    }
    updates.push_back(updateInfo);
  }

  session->SetConfigValue(
    MIKTEX_REGKEY_PACKAGE_MANAGER,
    session->IsAdminMode() ? MIKTEX_REGVAL_LAST_ADMIN_UPDATE_CHECK : MIKTEX_REGVAL_LAST_USER_UPDATE_CHECK,
    std::to_string(time(nullptr)));
}

void PackageInstallerImpl::FindUpdatesAsync()
{
  StartWorkerThread(&PackageInstallerImpl::FindUpdatesThread);
}

void PackageInstallerImpl::FindUpdatesThread()
{
#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
  HResult hr = E_FAIL;
#endif
  try
  {
#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
    HResult hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (hr.Failed())
    {
      MIKTEX_FATAL_ERROR_2(T_("Cannot start updater thread."), "hr", hr.GetText());
    }
#endif
    FindUpdates();
    progressInfo.ready = true;
    Notify();
  }
  catch (const OperationCancelledException& e)
  {
    progressInfo.ready = true;
    progressInfo.cancelled = true;
    threadMiKTeXException = e;
  }
  catch (const MiKTeXException& e)
  {
    progressInfo.ready = true;
    progressInfo.numErrors += 1;
    threadMiKTeXException = e;
  }
#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
  if (hr.Succeeded())
  {
    CoUninitialize();
  }
#endif
}

void PackageInstallerImpl::FindUpgrades(PackageLevel packageLevel)
{
  trace_mpm->WriteLine("libmpm", T_("searching for upgrades"));
  UpdateDb();
  LoadDbLight(false);
  upgrades.clear();
  for (string deploymentName = dbLight.FirstPackage(); !deploymentName.empty(); deploymentName = dbLight.NextPackage())
  {
    Notify();
    const PackageInfo* package = packageManager->TryGetPackageInfo(deploymentName);
    if (package != nullptr && packageManager->IsPackageInstalled(deploymentName))
    {
      continue;
    }
#if IGNORE_OTHER_SYSTEMS
    string targetSystem = dbLight.GetPackageTargetSystem(deploymentName);
    if (!targetSystem.empty() && targetSystem != MIKTEX_SYSTEM_TAG)
    {
      continue;
    }
#endif
    if (dbLight.GetPackageLevel(deploymentName) > packageLevel)
    {
      continue;
    }
    trace_mpm->WriteFormattedLine("libmpm", T_("%s: upgrade"), deploymentName.c_str());
    UpgradeInfo upgrade;
    upgrade.deploymentName = deploymentName;
    upgrade.timePackaged = dbLight.GetTimePackaged(deploymentName);
    upgrade.version = dbLight.GetPackageVersion(deploymentName);
    upgrades.push_back(upgrade);
  }
}

void PackageInstallerImpl::FindUpgradesAsync(PackageLevel packageLevel)
{
  upgradeLevel = packageLevel;
  StartWorkerThread(&PackageInstallerImpl::FindUpgradesThread);
}

void PackageInstallerImpl::FindUpgradesThread()
{
#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
  HResult hr = E_FAIL;
#endif
  try
  {
#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
    HResult hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (hr.Failed())
    {
      MIKTEX_FATAL_ERROR_2(T_("Cannot start upgrader thread."), "hr", hr.GetText());
    }
#endif
    FindUpgrades(upgradeLevel);
    progressInfo.ready = true;
    Notify();
  }
  catch (const OperationCancelledException& e)
  {
    progressInfo.ready = true;
    progressInfo.cancelled = true;
    threadMiKTeXException = e;
  }
  catch (const MiKTeXException& e)
  {
    progressInfo.ready = true;
    progressInfo.numErrors += 1;
    threadMiKTeXException = e;
  }
#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
  if (hr.Succeeded())
  {
    CoUninitialize();
  }
#endif
}

void PackageInstallerImpl::RemoveFiles(const vector<string>& toBeRemoved, bool silently)
{
  for (const string& f : toBeRemoved)
  {
    Notify(Notification::RemoveFileStart);

    // only consider texmf files
    string fileName;
    if (!PackageManager::StripTeXMFPrefix(f, fileName))
    {
      continue;
    }

    bool done = false;

    // get information about the installed file
    InstalledFileInfo* pInstalledFileInfo = packageManager->GetInstalledFileInfo(f.c_str());

    // decrement the file reference counter
    if (pInstalledFileInfo != nullptr && pInstalledFileInfo->refCount > 0)
    {
      pInstalledFileInfo->refCount -= 1;
    }

    // make an absolute path name
    PathName path(session->GetSpecialPath(SpecialPath::InstallRoot), fileName);

    // only delete if the reference count reached zero
    if (pInstalledFileInfo != nullptr && pInstalledFileInfo->refCount > 0)
    {
      trace_mpm->WriteFormattedLine("libmpm", T_("will not delete %s (ref count is %u)"), Q_(path), pInstalledFileInfo->refCount);
      done = true;
    }
    else if (File::Exists(path))
    {
      // remove the file
      try
      {
        FileDeleteOptionSet deleteOptions = { FileDeleteOption::TryHard };
        if (autoFndbSync)
        {
          deleteOptions += FileDeleteOption::UpdateFndb;
        }
        File::Delete(path, deleteOptions);
        done = true;
      }
      catch (const MiKTeXException& e)
      {
        done = false;
        if (!silently)
        {
          MIKTEX_FATAL_ERROR_2(FatalError(ERROR_CANNOT_DELETE), "path", path.ToString(), "reason", e.GetErrorMessage());
        }
      }
    }
    else
    {
      trace_mpm->WriteFormattedLine("libmpm", T_("file %s does not exist"), Q_(path));
      done = true;
    }


    // remove from MPM FNDB
#if 0
    if (autoFndbSync
      && Fndb::Exists(PathName(session->GetMpmRootPath(), fileName)))
    {
      Fndb::Remove(PathName(session->GetMpmRootPath(), fileName));
    }
#endif

    // update progress info
    if (done && !silently)
    {
      lock_guard<mutex> lockGuard(progressIndicatorMutex);
      progressInfo.cFilesRemoveCompleted += 1;
    }

    // notify client
    Notify(Notification::RemoveFileEnd);
  }
}

void PackageInstallerImpl::RemovePackage(const string& deploymentName)
{
  trace_mpm->WriteFormattedLine("libmpm", T_("going to remove %s"), Q_(deploymentName));

  // notify client
  Notify(Notification::RemovePackageStart);
  ReportLine(fmt::format(T_("removing package {0}..."), Q_(deploymentName)));

  // get package info
  PackageInfo* package = packageManager->TryGetPackageInfo(deploymentName);
  if (package == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }

  // check to see whether it is installed
  if (packageManager->GetTimeInstalled(deploymentName) == 0)
  {
    MIKTEX_UNEXPECTED();
  }

  // clear the installTime value => package is not installed
  trace_mpm->WriteFormattedLine("libmpm", T_("removing %s from the variable package table"), Q_(deploymentName));
  packageManager->SetTimeInstalled(deploymentName, 0);
  packageManager->FlushVariablePackageTable();
  package->timeInstalled = 0;

  if (packageManager->IsPackageObsolete(deploymentName))
  {
    // it's an obsolete package: make sure that the package
    // definition file gets removed too
    AddToFileList(package->runFiles, PrefixedPackageDefinitionFile(deploymentName));
  }
  else
  {
    // make sure that the package definition file does not get removed
    RemoveFromFileList(package->runFiles, PrefixedPackageDefinitionFile(deploymentName));
  }

  // remove the files
  size_t nTotal = (package->runFiles.size()
    + package->docFiles.size()
    + package->sourceFiles.size());
  trace_mpm->WriteFormattedLine("libmpm", T_("going to remove %u file(s)"), static_cast<unsigned>(nTotal));
  RemoveFiles(package->runFiles);
  RemoveFiles(package->docFiles);
  RemoveFiles(package->sourceFiles);

  trace_mpm->WriteFormattedLine("libmpm", T_("package %s successfully removed"), Q_(deploymentName));

  // update progress info
  {
    lock_guard<mutex> lockGuard(progressIndicatorMutex);
    progressInfo.cPackagesRemoveCompleted += 1;
  }

  // notify client
  Notify(Notification::RemovePackageEnd);
}

void PackageInstallerImpl::MyCopyFile(const PathName& source, const PathName& dest, size_t& size)
{
  // reset the read-only attribute, if the destination file exists
  if (File::Exists(dest))
  {
    FileAttributeSet attributesOld = File::GetAttributes(dest);
    FileAttributeSet attributesNew = attributesOld;
    if (attributesOld[FileAttribute::ReadOnly])
    {
      attributesNew -= FileAttribute::ReadOnly;
    }
#if defined(MIKTEX_WINDOWS)
    if (attributesOld[FileAttribute::Hidden])
    {
      attributesNew -= FileAttribute::Hidden;
    }
#endif
    if (attributesOld != attributesNew)
    {
      File::SetAttributes(dest, attributesNew);
    }
  }

  FILE* destinationFile;

  // open the destination file
  do
  {
    try
    {
      destinationFile = File::Open(dest, FileMode::Create, FileAccess::Write, false);
    }
    catch (const MiKTeXException& e)
    {
      ostringstream text;
      text
        << T_("The following file could not be written:")
        << LF
        << LF
        << "  " << dest.GetData()
        << LF
        << LF
        << T_("The write operation failed for the following reason:")
        << LF
        << LF
        << "  " << e.GetErrorMessage()
        << LF
        << LF
        << T_("Make sure that no other application uses the file and that you have write permission on the file.");
      if (AbortOrRetry(text.str().c_str()))
      {
        throw;
      }
      destinationFile = nullptr;
    }
  } while (destinationFile == nullptr);

  FileStream toStream(destinationFile);

  // open the source file
  FileStream fromStream(File::Open(source, FileMode::Open, FileAccess::Read, false));

  // copy the file
  char buffer[4096];
  size_t n;
  size = 0;
  while ((n = fromStream.Read(buffer, 4096)) > 0)
  {
    toStream.Write(buffer, n);
    size += n;
  }

  fromStream.Close();
  toStream.Close();

  if (autoFndbSync)
  {
    if (!Fndb::FileExists(dest))
    {
      Fndb::Add(dest);
    }
  }
}

void PackageInstallerImpl::CopyFiles(const PathName& pathSourceRoot, const vector<string>& fileList)
{
  for (const string& f : fileList)
  {
    Notify();

    // only consider texmf files
    string fileName;
    if (!(PackageManager::StripTeXMFPrefix(f, fileName)))
    {
      continue;
    }

    // make sure the source file exists
    PathName pathSource(pathSourceRoot, fileName);
    if (!File::Exists(pathSource))
    {
      MIKTEX_FATAL_ERROR_2(FatalError(ERROR_SOURCE_FILE_NOT_FOUND), "file", pathSource.ToString());
    }

    PathName pathDest(session->GetSpecialPath(SpecialPath::InstallRoot), fileName);

    PathName pathDestFolder(pathDest);
    pathDestFolder.RemoveFileSpec();

    // notify client: beginning of file copy operation
    Notify(Notification::InstallFileStart);

    // create the destination folder
    Directory::Create(pathDestFolder);

    {
      lock_guard<mutex> lockGuard(progressIndicatorMutex);
      progressInfo.fileName = pathDest;
    }

    size_t size;

    // copy the file
    MyCopyFile(pathSource, pathDest, size);

    // update progress info
    {
      lock_guard<mutex> lockGuard(progressIndicatorMutex);
      progressInfo.fileName = "";
      progressInfo.cFilesPackageInstallCompleted += 1;
      progressInfo.cFilesInstallCompleted += 1;
      progressInfo.cbPackageInstallCompleted += size;
      progressInfo.cbInstallCompleted += size;
    }

    // notify client: end of file copy operation
    Notify(Notification::InstallFileEnd);
  }
}

void PackageInstallerImpl::AddToFileList(vector<string>& fileList, const PathName& fileName) const
{
  // avoid duplicates
  for (const string& f : fileList)
  {
    if (PathName::Compare(f, fileName.ToString()) == 0)
    {
      return;
    }
  }
  fileList.push_back(fileName.GetData());
}

void PackageInstallerImpl::RemoveFromFileList(vector<string>& fileList, const PathName& fileName) const
{
  for (vector<string>::iterator it = fileList.begin(); it != fileList.end(); ++it)
  {
    if (PathName::Compare(*it, fileName.ToString()) == 0)
    {
      fileList.erase(it);
      return;
    }
  }
}

void PackageInstallerImpl::CopyPackage(const PathName& pathSourceRoot, const string& deploymentName)
{
  // parse the package definition file
  PathName pathPackageFile = pathSourceRoot;
  pathPackageFile /= MIKTEX_PATH_PACKAGE_DEFINITION_DIR;
  pathPackageFile /= deploymentName;
  pathPackageFile.AppendExtension(MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX);
  unique_ptr<TpmParser> tpmparser = TpmParser::Create();
  tpmparser->Parse(pathPackageFile);

  // get the package info from the parser; set the package name
  PackageInfo package = tpmparser->GetPackageInfo();
  package.deploymentName = deploymentName;

  // make sure that the package definition file is included in the
  // file list
  AddToFileList(package.runFiles, PrefixedPackageDefinitionFile(deploymentName));

  // copy the files
  CopyFiles(pathSourceRoot, package.runFiles);
  CopyFiles(pathSourceRoot, package.docFiles);
  CopyFiles(pathSourceRoot, package.sourceFiles);
}

typedef unordered_set<string> StringSet;

MPMSTATICFUNC(void) GetFiles(const PackageInfo& package, StringSet& files)
{
  files.insert(package.runFiles.begin(), package.runFiles.end());
  files.insert(package.docFiles.begin(), package.docFiles.end());
  files.insert(package.sourceFiles.begin(), package.sourceFiles.end());
}

void PackageInstallerImpl::UpdateMpmFndb(const vector<string>& installedFiles, const vector<string>& removedFiles, const char* lpszPackageName)
{
#if 0
  ReportLine(T_("updating MPM file name database:"));
  ReportLine(T_("  %u records to be added"), installedFiles.size());
  ReportLine(T_("  %u records to be removed"), removedFiles.size());
#endif
  for (const string& f : installedFiles)
  {
    PathName path(session->GetMpmRootPath(), f);
    if (!Fndb::FileExists(path))
    {
      Fndb::Add(path, lpszPackageName);
    }
    else
    {
      trace_mpm->WriteFormattedLine("libmpm", T_("%s already exists in mpm fndb"), Q_(path));
    }
  }
  for (const string& f : removedFiles)
  {
    PathName path(session->GetMpmRootPath(), f);
    if (Fndb::FileExists(path))
    {
      Fndb::Remove(path);
    }
    else
    {
      trace_mpm->WriteFormattedLine("libmpm", T_("%s does not exist in mpm fndb"), Q_(path));
    }
  }
}

void PackageInstallerImpl::InstallPackage(const string& deploymentName)
{
  trace_mpm->WriteFormattedLine("libmpm", T_("installing package %s"), Q_(deploymentName));

  // search the package table
  PackageInfo* package = packageManager->TryGetPackageInfo(deploymentName);
  if (package == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }

  NeedRepository();

  // initialize progress info
  {
    lock_guard<mutex> lockGuard(progressIndicatorMutex);
    progressInfo.deploymentName = deploymentName;
    progressInfo.displayName = package->displayName;
    progressInfo.cFilesPackageInstallCompleted = 0;
    progressInfo.cFilesPackageInstallTotal = package->GetNumFiles();
    progressInfo.cbPackageInstallCompleted = 0;
    progressInfo.cbPackageInstallTotal = package->GetSize();
    if (repositoryType == RepositoryType::Remote)
    {
      progressInfo.cbPackageDownloadCompleted = 0;
      progressInfo.cbPackageDownloadTotal = dbLight.GetArchiveFileSize(deploymentName);
    }
  }

  // notify client: beginning of package installation
  Notify(Notification::InstallPackageStart);

  PathName pathArchiveFile;
  ArchiveFileType aft = dbLight.GetArchiveFileType(deploymentName);
  unique_ptr<TemporaryFile> temporaryFile;

  // get hold of the archive file
  if (repositoryType == RepositoryType::Remote
    || repositoryType == RepositoryType::Local)
  {
    PathName packageFileName = deploymentName;
    packageFileName.AppendExtension(MiKTeX::Extractor::Extractor::GetFileNameExtension(aft));

    if (repositoryType == RepositoryType::Remote)
    {
      // take hold of the package
      temporaryFile = TemporaryFile::Create();
      pathArchiveFile = temporaryFile->GetPathName();
      Download(MakeUrl(packageFileName.ToString()), pathArchiveFile);
    }
    else
    {
      MIKTEX_ASSERT(repositoryType == RepositoryType::Local);
      pathArchiveFile = repository / deploymentName;
      pathArchiveFile.AppendExtension(MiKTeX::Extractor::Extractor::GetFileNameExtension(aft));
    }

    // check to see whether the digest is good
    if (!CheckArchiveFile(deploymentName, pathArchiveFile, false))
    {
      LoadDbLight(true);
      CheckArchiveFile(deploymentName, pathArchiveFile, true);
    }
  }

  // silently uninstall the package (this also decrements the file
  // reference counts)
  if (packageManager->IsPackageInstalled(deploymentName))
  {
    trace_mpm->WriteFormattedLine("libmpm", T_("%s: removing old files"), deploymentName.c_str());
    // make sure that the package info file does not get removed
    RemoveFromFileList(package->runFiles, PrefixedPackageDefinitionFile(deploymentName));
    RemoveFiles(package->runFiles, true);
    RemoveFiles(package->docFiles, true);
    RemoveFiles(package->sourceFiles, true);
    // temporarily set the status to "not installed"
    packageManager->SetTimeInstalled(deploymentName, 0);
    packageManager->FlushVariablePackageTable();
  }

  if (repositoryType == RepositoryType::Remote || repositoryType == RepositoryType::Local)
  {
    // unpack the archive file
    ReportLine(fmt::format(T_("extracting files from {0}..."), Q_(deploymentName + MiKTeX::Extractor::Extractor::GetFileNameExtension(aft))));
    ExtractFiles(pathArchiveFile, aft);
  }
  else if (repositoryType == RepositoryType::MiKTeXDirect)
  {
    // copy from CD
    PathName pathSourceRoot(repository);
    pathSourceRoot /= MIKTEXDIRECT_PREFIX_DIR;
    CopyPackage(pathSourceRoot, deploymentName);
  }
  else if (repositoryType == RepositoryType::MiKTeXInstallation)
  {
    // import from another MiKTeX installation
    ReportLine(fmt::format(T_("importing package {0}..."), deploymentName));
    PathName pathSourceRoot(repository);
    CopyPackage(pathSourceRoot, deploymentName);
  }
  else
  {
    MIKTEX_UNEXPECTED();
  }

  // parse the new package definition file
  PathName pathPackageFile = session->GetSpecialPath(SpecialPath::InstallRoot) / MIKTEX_PATH_PACKAGE_DEFINITION_DIR / deploymentName;
  pathPackageFile.AppendExtension(MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX);
  unique_ptr<TpmParser> tpmparser = TpmParser::Create();
  tpmparser->Parse(pathPackageFile);

  // get new package info
  PackageInfo newPackage = tpmparser->GetPackageInfo();
  newPackage.deploymentName = deploymentName;

  // find recycled and brand new files
  StringSet set1;
  GetFiles(*package, set1);
  StringSet set2;
  GetFiles(newPackage, set2);
  vector<string> recycledFiles;
  for (const string& s : set1)
  {
    if (set2.find(s) == set2.end())
    {
      string str;
      if (PackageManager::StripTeXMFPrefix(s, str))
      {
        recycledFiles.push_back(str);
      }
    }
  }
  vector<string> newFiles;
  for (const string& s : set2)
  {
    if (set1.find(s) == set1.end())
    {
      string str;
      if (PackageManager::StripTeXMFPrefix(s, str))
      {
        newFiles.push_back(str);
      }
    }
  }

  // update the MPM file name database
  if (autoFndbSync)
  {
    UpdateMpmFndb(newFiles, recycledFiles, deploymentName.c_str());
  }

  // set the timeInstalled value => package is installed
  newPackage.timeInstalled = time(nullptr);
  packageManager->SetTimeInstalled(deploymentName, newPackage.timeInstalled);
  packageManager->SetReleaseState(deploymentName, repositoryReleaseState);
  packageManager->FlushVariablePackageTable();

  // update package info table
  *package = newPackage;

  // increment file ref counts
  packageManager->IncrementFileRefCounts(deploymentName);

  // update progress info
  {
    lock_guard<mutex> lockGuard(progressIndicatorMutex);
    progressInfo.cPackagesInstallCompleted += 1;
  }

  // notify client: end of package installation
  Notify(Notification::InstallPackageEnd);
}

void PackageInstallerImpl::DownloadPackage(const string& deploymentName)
{
  size_t expectedSize;

  NeedRepository();

  // update progress info
  {
    lock_guard<mutex> lockGuard(progressIndicatorMutex);
    progressInfo.deploymentName = deploymentName;
    progressInfo.displayName = deploymentName;
    MIKTEX_ASSERT(repositoryType == RepositoryType::Remote);
    progressInfo.cbPackageDownloadCompleted = 0;
    progressInfo.cbPackageDownloadTotal = dbLight.GetArchiveFileSize(deploymentName);
    expectedSize = progressInfo.cbPackageDownloadTotal;
  }

  // notify client: beginning of package download
  Notify(Notification::DownloadPackageStart);

  // make the archive file name
  ArchiveFileType aft = dbLight.GetArchiveFileType(deploymentName);
  PathName pathArchiveFile = deploymentName;
  pathArchiveFile.AppendExtension(MiKTeX::Extractor::Extractor::GetFileNameExtension(aft));

  // download the archive file
  Download(pathArchiveFile, expectedSize);

  // check to see whether the archive file is ok
  CheckArchiveFile(deploymentName, downloadDirectory / pathArchiveFile, true);

  // notify client: end of package download
  Notify(Notification::DownloadPackageEnd);
}

void PackageInstallerImpl::CalculateExpenditure(bool downloadOnly)
{
  ProgressInfo package;

  if (!downloadOnly)
  {
    package.cPackagesInstallTotal = static_cast<unsigned long>(toBeInstalled.size());
  }

  NeedRepository();

  for (const string& p : toBeInstalled)
  {
    if (!downloadOnly)
    {
      PackageInfo* installCandidate = packageManager->TryGetPackageInfo(p);
      if (installCandidate == nullptr)
      {
        MIKTEX_UNEXPECTED();
      }
      package.cFilesInstallTotal += installCandidate->GetNumFiles();
      package.cbInstallTotal += installCandidate->GetSize();
    }
    if (repositoryType == RepositoryType::Remote)
    {
      int iSize = dbLight.GetArchiveFileSize(p);
      if (iSize == 0)
      {
        LoadDbLight(true);
        if ((iSize = dbLight.GetArchiveFileSize(p)) == 0)
        {
          MIKTEX_UNEXPECTED();
        }
      }
      package.cbDownloadTotal += iSize;
    }
  }

  if (package.cbDownloadTotal > 0)
  {
    ReportLine(fmt::format(T_("going to download {0} bytes"), package.cbDownloadTotal));
  }

  if (!downloadOnly && !toBeInstalled.empty())
  {
    ReportLine(fmt::format(T_("going to install {0} file(s) ({1} package(s))"), package.cFilesInstallTotal, package.cPackagesInstallTotal));
  }

  if (!downloadOnly && !toBeRemoved.empty())
  {
    package.cPackagesRemoveTotal = static_cast<unsigned long>(toBeRemoved.size());

    for (const string& p : toBeRemoved)
    {
      PackageInfo* removeCandidate = packageManager->TryGetPackageInfo(p);
      if (removeCandidate == nullptr)
      {
        MIKTEX_UNEXPECTED();
      }
      package.cFilesRemoveTotal += removeCandidate->GetNumFiles();
    }

    ReportLine(fmt::format(T_("going to remove {0} file(s) ({1} package(s))"), package.cFilesRemoveTotal, package.cPackagesRemoveTotal));
  }

  lock_guard<mutex> lockGuard(progressIndicatorMutex);
  progressInfo = package;
}

bool PackageInstallerImpl::ReadDirectory(const PathName& path, vector<string>& subDirNames, vector<string>& fileNames, vector<string>& fileNameInfos)
{
  UNUSED_ALWAYS(path);
  UNUSED_ALWAYS(subDirNames);
  UNUSED_ALWAYS(fileNames);
  UNUSED_ALWAYS(fileNameInfos);
  return false;
}

bool PackageInstallerImpl::OnProgress(unsigned level, const PathName& directory)
{
  UNUSED_ALWAYS(level);
  UNUSED_ALWAYS(directory);
  try
  {
    Notify();
    return true;
  }
  catch (const OperationCancelledException&)
  {
    return false;
  }
}

bool PackageInstallerImpl::CheckArchiveFile(const std::string& deploymentName, const PathName& archiveFileName, bool mustBeOk)
{
  if (!File::Exists(archiveFileName))
  {
    MIKTEX_FATAL_ERROR_2(FatalError(ERROR_MISSING_PACKAGE), "package", deploymentName, "archiveFile", archiveFileName.ToString());
  }
  MD5 digest1 = dbLight.GetArchiveFileDigest(deploymentName);
  MD5 digest2 = MD5::FromFile(archiveFileName.GetData());
  bool ok = (digest1 == digest2);
  if (!ok && mustBeOk)
  {
    MIKTEX_FATAL_ERROR_2(FatalError(ERROR_CORRUPTED_PACKAGE), "package", deploymentName, "arhiveFile", archiveFileName.ToString(), "expectedMD5", digest1.ToString(), "actualMD5", digest2.ToString());
  }
  return ok;
}

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER

void PackageInstallerImpl::ConnectToServer()
{
  const char* MSG_CANNOT_START_SERVER = T_("Cannot start MiKTeX package manager.");
  if (!session->UnloadFilenameDatabase())
  {
    // ignore for now
  }
  if (localServer.pInstaller == nullptr)
  {
    if (localServer.pManager == nullptr)
    {
      WCHAR wszCLSID[50];
      if (StringFromGUID2(__uuidof(MiKTeXPackageManagerLib::MAKE_CURVER_ID(PackageManager)), wszCLSID, sizeof(wszCLSID) / sizeof(wszCLSID[0])) < 0)
      {
        MIKTEX_FATAL_ERROR(MSG_CANNOT_START_SERVER);
      }
      wstring monikerName;
      monikerName = L"Elevation:Administrator!new:";
      monikerName += wszCLSID;
      BIND_OPTS3 bo;
      memset(&bo, 0, sizeof(bo));
      bo.cbStruct = sizeof(bo);
      bo.hwnd = GetForegroundWindow();
      bo.dwClassContext = CLSCTX_LOCAL_SERVER;
      HResult hr = CoGetObject(monikerName.c_str(), &bo, __uuidof(MiKTeXPackageManagerLib::IPackageManager), reinterpret_cast<void**>(&localServer.pManager));
      if (hr == CO_E_NOTINITIALIZED || hr == MK_E_SYNTAX)
      {
        MyCoInitialize();
        hr = CoGetObject(monikerName.c_str(), &bo, __uuidof(MiKTeXPackageManagerLib::IPackageManager), reinterpret_cast<void**>(&localServer.pManager));
      }
      if (hr.Failed())
      {
        MIKTEX_FATAL_ERROR_2(MSG_CANNOT_START_SERVER, "hr", hr.GetText());
      }
    }
    HResult hr = localServer.pManager->CreateInstaller(&localServer.pInstaller);
    if (hr.Failed())
    {
      localServer.pManager.Release();
      MIKTEX_FATAL_ERROR_2(MSG_CANNOT_START_SERVER, "hr", hr.GetText());
    }
  }
}

#endif

#if defined(MIKTEX_WINDOWS)

#undef USE_REGSVR32

void PackageInstallerImpl::RegisterComponent(bool doRegister, const PathName& path, bool mustSucceed)
{
  MIKTEX_ASSERT(!session->IsMiKTeXPortable());
  ReportLine(fmt::format("{0} {1}", (doRegister ? "registering" : "unregistering"), path));
#if !USE_REGSVR32
  MIKTEX_ASSERT(path.HasExtension(MIKTEX_SHARED_LIB_FILE_SUFFIX));
  DllProc0<HRESULT> regsvr(path.GetData(), doRegister ? "DllRegisterServer" : "DllUnregisterServer");
  HResult hr = regsvr();
  if (hr.Failed())
  {
    if (mustSucceed)
    {
      MIKTEX_FATAL_ERROR_2(T_("COM registration/unregistration did not succeed."), "path", path.ToString(), "hr", hr.GetText());
    }
    else
    {
      trace_error->WriteFormattedLine("libmpm", T_("registration/unregistration of %s did not succeed; hr=%s"), Q_(path), Q_(hr.ToString()));
    }
  }
#else
  PathName regExe;
  CommandLineBuilder cmdLine;
  if (path.HasExtension(MIKTEX_SHARED_LIB_FILE_SUFFIX))
  {
    regExe = "regsvr32.exe";
    cmdLine.AppendOption("/s");
    if (!doRegister)
    {
      cmdLine.AppendOption("/u");
    }
    cmdLine.AppendArgument(path);
  }
  else
  {
    regExe = path;
    cmdLine.AppendOption(doRegister ? "/RegServer" : "/UnregServer");
  }
  int exitCode;
  if (!Process::Run(regExe, cmdLine.Get(), nullptr, &exitCode, nullptr))
  {
    MIKTEX_UNEXPECTED();
  }
  if (exitCode != 0)
  {
    if (mustSucceed)
    {
      MIKTEX_FATAL_ERROR_2(T_("COM registration/unregistration did not succeed."), "path", path.ToString(), "exitCode", std::to_string(exitCode));
    }
    else
    {
      trace_error->WriteFormattedLine("libmpm", T_("%s %s did not succeed (exit code: %d)"), regExe.Get(), cmdLine.Get(), exitCode);
    }
  }
#endif
}

#endif

#if defined(MIKTEX_WINDOWS)

static const char* const components[] = {
  MIKTEX_PATH_CORE_DLL, MIKTEX_PATH_CORE_PS_DLL, MIKTEX_PATH_MPM_DLL, MIKTEX_PATH_MPM_PS_DLL,
};

#endif

#if defined(MIKTEX_WINDOWS)

static const char* const toBeConfigured[] = {
  MIKTEX_PATH_FONTCONFIG_CONFIG_FILE,
};

#endif

void PackageInstallerImpl::RegisterComponents(bool doRegister, const vector<string>& packages)
{
  for (const string& p : packages)
  {
    PackageInfo* package = packageManager->TryGetPackageInfo(p);
    if (package == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    for (const string& f : package->runFiles)
    {
      string fileName;
      if (!PackageManager::StripTeXMFPrefix(f, fileName))
      {
        continue;
      }
      if (doRegister)
      {
#if defined(MIKTEX_WINDOWS)
        for (const char* file : toBeConfigured)
        {
          PathName relPath(file);
          PathName relPathIn(relPath);
          relPathIn.AppendExtension(".in");
          if (PathName(fileName) != relPathIn)
          {
            continue;
          }
          PathName pathIn = session->GetSpecialPath(SpecialPath::InstallRoot);
          pathIn /= relPathIn;
          if (File::Exists(pathIn))
          {
            ReportLine(fmt::format(T_("configuring {0}"), relPath));
            session->ConfigureFile(relPath);
          }
          else
          {
            ReportLine(fmt::format(T_("problem: {0} does not exist"), pathIn));
          }
        }
#endif
      }
#if defined(MIKTEX_WINDOWS)
      if (!session->IsMiKTeXPortable() && (session->RunningAsAdministrator() || session->RunningAsPowerUser()))
      {
        for (const char* comp : components)
        {
          if (PathName(fileName) != comp)
          {
            continue;
          }
          PathName path = session->GetSpecialPath(SpecialPath::InstallRoot);
          path /= comp;
          if (File::Exists(path))
          {
            RegisterComponent(doRegister, path, doRegister);
          }
          else
          {
#if 0
            ReportLine(T_("problem: %s does not exist"), path.Get());
#endif
          }
        }
      }
#endif
    }
  }
}

void PackageInstallerImpl::RegisterComponents(bool doRegister)
{
  if (doRegister)
  {
#if defined(MIKTEX_WINDOWS)
    for (const char* comp : toBeConfigured)
    {
      PathName relPath(comp);
      PathName pathIn(session->GetSpecialPath(SpecialPath::InstallRoot));
      pathIn /= relPath;
      pathIn.AppendExtension(".in");
      if (File::Exists(pathIn))
      {
        ReportLine(fmt::format(T_("configuring {0}"), relPath));
        session->ConfigureFile(relPath);
      }
      else
      {
        ReportLine(fmt::format(T_("problem: {0} does not exist"), pathIn));
      }
    }
#endif
  }
#if defined(MIKTEX_WINDOWS)
  if (!session->IsMiKTeXPortable()
    && (session->RunningAsAdministrator() || session->RunningAsPowerUser()))
  {
    for (const char* comp : components)
    {
      PathName path(session->GetSpecialPath(SpecialPath::InstallRoot));
      path /= comp;
      if (File::Exists(path))
      {
        RegisterComponent(doRegister, path, doRegister);
      }
      else
      {
        ReportLine(fmt::format(T_("problem: {0} does not exist"), path));
      }
    }
  }
#endif
}

bool MIKTEXTHISCALL PackageInstallerImpl::OnProcessOutput(const void* pOutput, size_t n)
{
  return true;
}

void PackageInstallerImpl::RunIniTeXMF(const vector<string>& extraArguments)
{
  // find initexmf
  PathName initexmf;
  if (!session->FindFile(MIKTEX_INITEXMF_EXE, FileType::EXE, initexmf))
  {
    MIKTEX_UNEXPECTED();
  }

  // run initexmf
  vector<string> arguments{ "initexmf" };
  if (session->IsAdminMode())
  {
    arguments.push_back("--admin");
  }
  arguments.insert(arguments.end(), extraArguments.begin(), extraArguments.end());
  // TODO: propagate --enable-installer
  Process::Run(initexmf, arguments, this);
}

void PackageInstallerImpl::CheckDependencies(set<string>& packages, const string& deploymentName, bool force, int level)
{
  if (level > 10)
  {
    MIKTEX_UNEXPECTED();
  }
  PackageInfo* package = packageManager->TryGetPackageInfo(deploymentName);
  if (package != nullptr)
  {
    for (const string& p : package->requiredPackages)
    {
      CheckDependencies(packages, p, force, level + 1);
    }
  }
  if (force || !packageManager->IsPackageInstalled(deploymentName))
  {
    packages.insert(deploymentName);
  }
}

void PackageInstallerImpl::InstallRemove(Role role)
{
  NeedRepository();

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
  if (UseLocalServer())
  {
    HResult hr;
    ConnectToServer();
    for (const string& p : toBeInstalled)
    {
      hr = localServer.pInstaller->Add(_bstr_t(p.c_str()), VARIANT_TRUE);
      if (hr.Failed())
      {
        MIKTEX_FATAL_ERROR_2(T_("Cannot communicate with mpmsvc."), "hr", hr.GetText());
      }
    }
    for (const string& p : toBeRemoved)
    {
      HResult hr = localServer.pInstaller->Add(_bstr_t(p.c_str()), VARIANT_FALSE);
      if (hr.Failed())
      {
        MIKTEX_FATAL_ERROR_2(T_("Cannot communicate with mpmsvc."), "hr", hr.GetText());
      }
    }
    localServer.pInstaller->SetCallback(this);
    if (repositoryType != RepositoryType::Unknown)
    {
      hr = localServer.pInstaller->SetRepository(_bstr_t(repository.c_str()));
      if (hr.Failed())
      {
        localServer.pInstaller->SetCallback(0);
        MIKTEX_FATAL_ERROR_2(T_("mpmsvc failed for some reason."), "hr", hr.GetText());
      }
    }
    hr = localServer.pInstaller->InstallRemove();
    localServer.pInstaller->SetCallback(0);
    if (hr.Failed())
    {
      MiKTeXPackageManagerLib::ErrorInfo errorInfo;
      HResult hr2 = localServer.pInstaller->GetErrorInfo(&errorInfo);
      if (hr2.Failed())
      {
        MIKTEX_FATAL_ERROR_2(T_("mpmsvc failed for some reason."), "hr", hr.GetText());
      }
      AutoSysString a(errorInfo.message);
      AutoSysString b(errorInfo.info);
      AutoSysString c(errorInfo.sourceFile);
      Session::FatalMiKTeXError(string(WU_(errorInfo.message)), "", "", "", MiKTeXException::KVMAP("", string(WU_(errorInfo.info))), SourceLocation("", string(WU_(errorInfo.sourceFile)), errorInfo.sourceLine));
    }
    return;
  }
#endif

  bool upgrade = toBeInstalled.empty() && toBeRemoved.empty();
  bool installing = upgrade || !toBeInstalled.empty();

  if (installing)
  {
    if (repositoryType == RepositoryType::Unknown)
    {
      // we must have a package repository
      repository = packageManager->PickRepositoryUrl();
      repositoryType = RepositoryType::Remote;
    }
    else if (repositoryType == RepositoryType::Remote)
    {
      repositoryReleaseState = packageManager->VerifyPackageRepository(repository).releaseState;
    }
  }

  ReportLine(T_("starting package maintenance..."));
  ReportLine(fmt::format(T_("installation directory: {0}"), Q_(session->GetSpecialPath(SpecialPath::InstallRoot))));
  if (installing)
  {
    ReportLine(fmt::format(T_("package repository: {0}"), Q_(repository)));
  }

  SetAutoFndbSync(true);

  // make sure that mpm.fndb exists
  if (autoFndbSync && !File::Exists(session->GetMpmDatabasePathName()))
  {
    packageManager->CreateMpmFndb();
  }

  if (toBeInstalled.size() > 1 || !toBeRemoved.empty())
  {
    packageManager->NeedInstalledFileInfoTable();
  }

  // collect all packages, if no packages were specified by the caller
  if (upgrade)
  {
    LoadDbLight(true);

    string deploymentName = dbLight.FirstPackage();
    if (deploymentName.empty())
    {
      MIKTEX_FATAL_ERROR(T_("No packages on server."));
    }
    do
    {
      // search dblight
      PackageLevel lvl = dbLight.GetPackageLevel(deploymentName);
      if (lvl > taskPackageLevel)
      {
        // not found or not required
        continue;
      }

#if IGNORE_OTHER_SYSTEMS
      // check target system
      string targetSystem = dbLight.GetPackageTargetSystem(deploymentName);
      if (!(targetSystem.empty() || targetSystem == MIKTEX_SYSTEM_TAG))
      {
        continue;
      }
#endif

      if (repositoryType == RepositoryType::Local || repositoryType == RepositoryType::Remote)
      {
        // ignore pure containers
        if (IsPureContainer(deploymentName))
        {
          continue;
        }

        // check to see whether the archive file exists
        ArchiveFileType aft = dbLight.GetArchiveFileType(deploymentName);
        PathName pathLocalArchiveFile = repository / deploymentName;
        pathLocalArchiveFile.AppendExtension(MiKTeX::Extractor::Extractor::GetFileNameExtension(aft));
        if (!File::Exists(pathLocalArchiveFile))
        {
          MIKTEX_FATAL_ERROR_2(FatalError(ERROR_MISSING_PACKAGE), "package", deploymentName, "archiveFile", pathLocalArchiveFile.ToString());
        }

        // check to see if the archive file is valid
        CheckArchiveFile(deploymentName, pathLocalArchiveFile, true);
      }

      // collect the package
      toBeInstalled.push_back(deploymentName);
    } while (!(deploymentName = dbLight.NextPackage()).empty());
  }
  else if (!toBeInstalled.empty())
  {
    // we need mpm.ini, if packages are to be installed
    LoadDbLight(false);
  }

  // check dependencies
  set<string> tmp;
  for (const string& p : toBeInstalled)
  {
    CheckDependencies(tmp, p, true, 0);
  }
  toBeInstalled.assign(tmp.begin(), tmp.end());

  // calculate total size and more
  CalculateExpenditure();

  RegisterComponents(false, toBeInstalled, toBeRemoved);

  // install packages
  for (const string& p : toBeInstalled)
  {
    InstallPackage(p);
  }

  // remove packages
  for (const string& p : toBeRemoved)
  {
    RemovePackage(p);
  }

  if (role == Role::Updater)
  {
    session->SetConfigValue(
      MIKTEX_REGKEY_PACKAGE_MANAGER,
      session->IsAdminMode() ? MIKTEX_REGVAL_LAST_ADMIN_UPDATE : MIKTEX_REGVAL_LAST_USER_UPDATE,
      std::to_string(time(nullptr)));
  }

  // check dependencies (install missing required packages)
  tmp.clear();
  for (const string& p : toBeInstalled)
  {
    CheckDependencies(tmp, p, false, 0);
  }
  for (const string& p : tmp)
  {
    InstallPackage(p);
  }

  if (!noPostProcessing)
  {
    RegisterComponents(true, toBeInstalled);
  }

  if (!autoFndbSync)
  {
    // refresh file name database now
    ReportLine(T_("refreshing file name database..."));
    if (!Fndb::Refresh(session->GetSpecialPath(SpecialPath::InstallRoot), this))
    {
      throw OperationCancelledException();
    }
    packageManager->CreateMpmFndb();
  }

  if (!noPostProcessing)
  {
    vector<string> args = { "--mkmaps" };
#if defined(MIKTEX_WINDOWS)
    bool mklinks = session->IsAdminMode();
#else
    bool mklinks = true;
#endif
    if (mklinks)
    {
      args.push_back("--mklinks");
    }
    RunIniTeXMF(args);
  }
}

void PackageInstallerImpl::InstallRemoveAsync(Role role)
{
  currentRole = role;
  StartWorkerThread(&PackageInstallerImpl::InstallRemoveThread);
}

void PackageInstallerImpl::InstallRemoveThread()
{
#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
  HResult hr = E_FAIL;
#endif
  try
  {
#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (hr.Failed())
    {
      MIKTEX_FATAL_ERROR_2(T_("Cannot start installer thread."), "hr", hr.GetText());
    }
#endif
    InstallRemove(currentRole);
    progressInfo.ready = true;
    Notify();
  }
  catch (const OperationCancelledException& e)
  {
    progressInfo.ready = true;
    progressInfo.cancelled = true;
    threadMiKTeXException = e;
  }
  catch (const MiKTeXException& e)
  {
    progressInfo.ready = true;
    progressInfo.numErrors += 1;
    threadMiKTeXException = e;
  }
#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
  if (hr.Succeeded())
  {
    CoUninitialize();
  }
#endif
}

void PackageInstallerImpl::Download(const PathName& fileName, size_t expectedSize)
{
  Download(MakeUrl(fileName.ToString()), downloadDirectory / fileName, expectedSize);
}

void PackageInstallerImpl::Download()
{
  NeedRepository();

  if (repositoryType == RepositoryType::Remote)
  {
    repositoryReleaseState = packageManager->VerifyPackageRepository(repository).releaseState;
  }

  if (repositoryType == RepositoryType::Unknown)
  {
    repository = packageManager->PickRepositoryUrl();
    repositoryType = RepositoryType::Remote;
  }

  MIKTEX_ASSERT(repositoryType == RepositoryType::Remote);

  ReportLine(T_("starting download..."));
  ReportLine(fmt::format(T_("repository: {0}"), Q_(repository)));
  ReportLine(fmt::format(T_("download directory: {0}"), Q_(downloadDirectory)));

  // download and load the lightweight database
  LoadDbLight(true);

  // collect required packages
  string deploymentName;
  if (!(deploymentName = dbLight.FirstPackage()).empty())
  {
    do
    {
      // don't add pure containers
      if (IsPureContainer(deploymentName))
      {
        continue;
      }

      // check package level
      if (taskPackageLevel < dbLight.GetPackageLevel(deploymentName))
      {
        // package is not required
        continue;
      }

      // check to see whether the file was downloaded previously
      ArchiveFileType aft = dbLight.GetArchiveFileType(deploymentName);
      PathName pathLocalArchiveFile = downloadDirectory / deploymentName;
      pathLocalArchiveFile.AppendExtension(MiKTeX::Extractor::Extractor::GetFileNameExtension(aft));
      if (File::Exists(pathLocalArchiveFile))
      {
        // the archive file exists;  check to see if it is valid
        MD5 digest1 = dbLight.GetArchiveFileDigest(deploymentName);
        MD5 digest2 = MD5::FromFile(pathLocalArchiveFile.GetData());
        if (digest1 == digest2)
        {
          // valid => don't download again
          ReportLine(fmt::format(T_("{0} already exists - keep it"), Q_(pathLocalArchiveFile)));
          continue;
        }
        ReportLine(fmt::format(T_("{0} already exists but seems to be damaged"), Q_(pathLocalArchiveFile)));
      }

      // pick up the package
      toBeInstalled.push_back(deploymentName);
    } while (!(deploymentName = dbLight.NextPackage()).empty());
  }

  // count bytes
  CalculateExpenditure(true);

  // download dblight & dbfull
  ReportLine(T_("downloading package database..."));
  {
    lock_guard<mutex> lockGuard(progressIndicatorMutex);
    progressInfo.deploymentName = MIKTEX_MPM_DB_LIGHT_FILE_NAME_NO_SUFFIX;
    progressInfo.displayName = T_("Lightweight package database");
    progressInfo.cbPackageDownloadCompleted = 0;
    progressInfo.cbPackageDownloadTotal = MPM_APSIZE_DB_LIGHT;
  }
  Download(MIKTEX_MPM_DB_LIGHT_FILE_NAME);
  {
    lock_guard<mutex> lockGuard(progressIndicatorMutex);
    progressInfo.deploymentName = MIKTEX_MPM_DB_FULL_FILE_NAME_NO_SUFFIX;
    progressInfo.displayName = T_("complete package database");
    progressInfo.cbPackageDownloadCompleted = 0;
    progressInfo.cbPackageDownloadTotal = MPM_APSIZE_DB_FULL;
  }
  Download(MIKTEX_MPM_DB_FULL_FILE_NAME);

  // download archive files
  for (const string& p : toBeInstalled)
  {
    DownloadPackage(p);
  }
}

void PackageInstallerImpl::DownloadAsync()
{
  StartWorkerThread(&PackageInstallerImpl::DownloadThread);
}

void PackageInstallerImpl::DownloadThread()
{
#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
  HResult hr = E_FAIL;
#endif
  try
  {
#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (hr.Failed())
    {
      MIKTEX_FATAL_ERROR_2(T_("Cannot start downloader thread."), "hr", hr.GetText());
    }
#endif
    Download();
    progressInfo.ready = true;
    Notify();
  }
  catch (const OperationCancelledException& e)
  {
    progressInfo.ready = true;
    progressInfo.cancelled = true;
    threadMiKTeXException = e;
  }
  catch (const MiKTeXException& e)
  {
    progressInfo.ready = true;
    progressInfo.numErrors += 1;
    threadMiKTeXException = e;
  }
#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
  if (hr.Succeeded())
  {
    CoUninitialize();
  }
#endif
}

void PackageInstallerImpl::SetUpPackageDefinitionFiles(const PathName& directory)
{
  // path to the database file
  PathName pathDatabase;

  NeedRepository();

  if (repositoryType == RepositoryType::Remote)
  {
    // download the database file
    pathDatabase = directory / MIKTEX_MPM_DB_FULL_FILE_NAME;
    Download(MakeUrl(MIKTEX_MPM_DB_FULL_FILE_NAME), pathDatabase);
  }
  else
  {
    MIKTEX_ASSERT(repositoryType == RepositoryType::Local);
    pathDatabase = repository / MIKTEX_MPM_DB_FULL_FILE_NAME;
  }

  // extract package defintion files
  unique_ptr<MiKTeX::Extractor::Extractor> pExtractor(MiKTeX::Extractor::Extractor::CreateExtractor(DB_ARCHIVE_FILE_TYPE));
  pExtractor->Extract(pathDatabase, directory);
}

void PackageInstallerImpl::CleanUpUserDatabase()
{
  PathName userDir(session->GetSpecialPath(SpecialPath::UserInstallRoot), MIKTEX_PATH_PACKAGE_DEFINITION_DIR);

  PathName commonDir(session->GetSpecialPath(SpecialPath::CommonInstallRoot), MIKTEX_PATH_PACKAGE_DEFINITION_DIR);

  if (!Directory::Exists(userDir) || !Directory::Exists(commonDir))
  {
    return;
  }

  if (userDir.Canonicalize() == commonDir.Canonicalize())
  {
    return;
  }

  vector<PathName> toBeRemoved;

  // check all package definition files
  unique_ptr<DirectoryLister> pLister = DirectoryLister::Open(userDir);
  DirectoryEntry direntry;
  while (pLister->GetNext(direntry))
  {
    PathName name(direntry.name);

    if (direntry.isDirectory
      || !name.HasExtension(MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX))
    {
      continue;
    }

    // check to see whether the system-wide file exists
    PathName commonPackageDefinitionFile(commonDir, name);
    if (!File::Exists(commonPackageDefinitionFile))
    {
      continue;
    }

    // compare files
    PathName userPackageDefinitionFile(userDir, name);
    if (File::GetSize(userPackageDefinitionFile) == File::GetSize(commonPackageDefinitionFile)
      && MD5::FromFile(userPackageDefinitionFile.GetData()) == MD5::FromFile(commonPackageDefinitionFile.GetData()))
    {
      // files are identical; remove user file later
      toBeRemoved.push_back(userPackageDefinitionFile);
    }
  }
  pLister->Close();

  // remove redundant user package definition files
  for (const PathName& p : toBeRemoved)
  {
    trace_mpm->WriteFormattedLine("libmpm", T_("removing redundant package definition file: %s"), Q_(p));
    File::Delete(p, { FileDeleteOption::TryHard });
  }
}

void PackageInstallerImpl::HandleObsoletePackageDefinitionFiles(const PathName& temporaryDirectory)
{
  PathName pathPackageDir(session->GetSpecialPath(SpecialPath::InstallRoot), MIKTEX_PATH_PACKAGE_DEFINITION_DIR);

  if (!Directory::Exists(pathPackageDir))
  {
    return;
  }

  unique_ptr<DirectoryLister> pLister = DirectoryLister::Open(pathPackageDir);
  DirectoryEntry direntry;
  while (pLister->GetNext(direntry))
  {
    PathName name(direntry.name);

    if (direntry.isDirectory || !name.HasExtension(MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX))
    {
      continue;
    }

    // it's not an obsolete package if the temporary directory
    // contains a corresponding package definition file
    if (File::Exists(temporaryDirectory / name))
    {
      continue;
    }

    // now we know that the package is obsolete

    MIKTEX_ASSERT(PathName(MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX) == (PathName(MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX).GetExtension()));
    string deploymentName = name.GetFileNameWithoutExtension().ToString();

    // check to see whether the obsolete package is installed
    if (packageManager->GetTimeInstalled(deploymentName) == 0 || IsPureContainer(deploymentName))
    {
      // not installed: remove the package definition file
      trace_mpm->WriteFormattedLine("libmpm", T_("removing obsolete %s"), Q_(name));
      File::Delete(pathPackageDir / name, { FileDeleteOption::TryHard });
    }
    else
    {
      // installed: declare the package as obsolete (we wont
      // uninstall obsolete packages)
      trace_mpm->WriteFormattedLine("libmpm", T_("declaring %s obsolete"), Q_(deploymentName));
      packageManager->DeclarePackageObsolete(deploymentName, true);
    }
  }

  pLister->Close();

  packageManager->FlushVariablePackageTable();
}

void PackageInstallerImpl::UpdateDb()
{
  NeedRepository();

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
  if (UseLocalServer())
  {
    ConnectToServer();
    localServer.pInstaller->SetRepository(_bstr_t(repository.c_str()));
    HResult hr = localServer.pInstaller->UpdateDb();
    if (hr.Failed())
    {
      MiKTeXPackageManagerLib::ErrorInfo errorInfo;
      HResult hr2 = localServer.pInstaller->GetErrorInfo(&errorInfo);
      if (hr2.Failed())
      {
        MIKTEX_FATAL_ERROR_2(T_("The service failed for some reason."), "hr", hr.GetText());
      }
      AutoSysString a(errorInfo.message);
      AutoSysString b(errorInfo.info);
      AutoSysString c(errorInfo.sourceFile);
      Session::FatalMiKTeXError(string(WU_(errorInfo.message)), "", "", "", MiKTeXException::KVMAP("", string(WU_(errorInfo.info))), SourceLocation("", string(WU_(errorInfo.sourceFile)), errorInfo.sourceLine));
    }
    return;
  }
#endif

  if (repositoryType == RepositoryType::Unknown)
  {
    repository = packageManager->PickRepositoryUrl();
    repositoryType = RepositoryType::Remote;
  }
  else if (repositoryType == RepositoryType::Remote)
  {
    repositoryReleaseState = packageManager->VerifyPackageRepository(repository).releaseState;
  }

  // we might need a temporary directory
  unique_ptr<TemporaryDirectory> tempDir;

  // path to the package definition directory
  PathName pkgDir;

  // copy the new package definition files into a temporary directory
  if (repositoryType == RepositoryType::Remote || repositoryType == RepositoryType::Local)
  {
    tempDir = TemporaryDirectory::Create();
    pkgDir = tempDir->GetPathName();
    SetUpPackageDefinitionFiles(pkgDir);
  }
  else if (repositoryType == RepositoryType::MiKTeXDirect)
  {
    // installing from the CD
    pkgDir = repository;
    pkgDir /= MIKTEXDIRECT_PREFIX_DIR;
    pkgDir /= MIKTEX_PATH_PACKAGE_DEFINITION_DIR;
  }
  else
  {
    MIKTEX_UNEXPECTED();
  }

  // handle obsolete package definition files
  HandleObsoletePackageDefinitionFiles(pkgDir);

  // update the package definition directory
  PathName packageDefinitionDir(session->GetSpecialPath(SpecialPath::InstallRoot), MIKTEX_PATH_PACKAGE_DEFINITION_DIR);
  ReportLine(fmt::format(T_("updating package definition directory ({0})..."), Q_(packageDefinitionDir)));
  size_t count = 0;
  unique_ptr<DirectoryLister> pLister = DirectoryLister::Open(pkgDir);
  DirectoryEntry direntry;
  unique_ptr<TpmParser> tpmparser = TpmParser::Create();
  while (pLister->GetNext(direntry))
  {
    Notify();

    PathName name(direntry.name);

    if (direntry.isDirectory || !name.HasExtension(MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX))
    {
      continue;
    }

    // get external package name
    MIKTEX_ASSERT(PathName(MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX) == (PathName(MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX).GetExtension()));
    string deploymentName = name.GetFileNameWithoutExtension().ToString();

    // build name of current package definition file
    PathName currentPackageDefinitionfile(packageDefinitionDir, name);

    // ignore package, if package is already installed
    if (!IsPureContainer(deploymentName) && packageManager->IsPackageInstalled(deploymentName))
    {
#if 0
      if (File::Exists(currentPackageDefinitionfile))
#endif
        continue;
    }

    // parse new package definition file
    PathName newPackageDefinitionFile(pkgDir, name);
    tpmparser->Parse(newPackageDefinitionFile);

#if 0
    PackageInfo currentPackageInfo;
    if (!IsPureContainer(szDeploymentName)
      && packageManager->TryGetPackageInfo(szDeploymentName, currentPackageInfo)
      && tpmparser.GetPackageInfo().digest == currentPackageInfo.digest)
    {
      // nothing new
      continue;
    }
#endif

    // move the new package definition file into the package
    // definition directory
    Directory::Create(packageDefinitionDir);
    if (File::Exists(currentPackageDefinitionfile))
    {
      // move the current file out of the way
      File::Delete(currentPackageDefinitionfile, { FileDeleteOption::TryHard });
    }
    File::Copy(newPackageDefinitionFile, currentPackageDefinitionfile);

    // update the database
    packageManager->DefinePackage(deploymentName, tpmparser->GetPackageInfo());

    ++count;
  }

  pLister->Close();

  ReportLine(fmt::format(T_("installed {0} package definition files"), count));

  // clean up the user database
  if (!session->IsAdminMode())
  {
    CleanUpUserDatabase();
  }

  // install mpm.ini
  InstallDbLight();

  // force a reload of the database
  dbLight.Clear();
  packageManager->ClearAll();

  // create the MPM file name database
  packageManager->CreateMpmFndb();
}

void PackageInstallerImpl::UpdateDbAsync()
{
  StartWorkerThread(&PackageInstallerImpl::UpdateDbThread);
}

void PackageInstallerImpl::UpdateDbThread()
{
  try
  {
    UpdateDb();
    progressInfo.ready = true;
    Notify();
  }
  catch (const OperationCancelledException& e)
  {
    progressInfo.ready = true;
    progressInfo.cancelled = true;
    threadMiKTeXException = e;
  }
  catch (const MiKTeXException& e)
  {
    progressInfo.ready = true;
    progressInfo.numErrors += 1;
    threadMiKTeXException = e;
  }
}

void PackageInstallerImpl::StartWorkerThread(void (PackageInstallerImpl::*method) ())
{
  progressInfo = ProgressInfo();
  timeStarted = clock();
  SetAutoFndbSync(false);
  workerThread = thread(method, this);
}

void PackageInstallerImpl::WaitForCompletion()
{
  workerThread.join();
  if (progressInfo.numErrors > 0 || progressInfo.cancelled)
  {
    throw threadMiKTeXException;
  }
}

string PackageInstallerImpl::FatalError(ErrorCode error)
{
  {
    lock_guard<mutex> lockGuard(thisMutex);
    progressInfo.numErrors += 1;
  }
  switch (error)
  {
  case ERROR_CANNOT_DELETE:
    return T_("The operation could not be completed because the removal of a file did not succeed.");
  case ERROR_MISSING_PACKAGE:
    return T_("The operation could not be completed because a required file does not exist.");
  case ERROR_CORRUPTED_PACKAGE:
    return T_("The operation could not be completed because a required file failed verification.");
  case ERROR_SOURCE_FILE_NOT_FOUND:
    return T_("The operation could not be completed because a required file does not exist.");
  case ERROR_SIZE_MISMATCH:
    return T_("The operation could not be completed because a required file failed verification:");
  default:
    return T_("The operation could not be completed.");
  }
}

void PackageInstallerImpl::ReportLine(const string& s)
{
  if (callback != nullptr)
  {
    callback->ReportLine(s);
  }
}

void PackageInstallerImpl::Dispose()
{
  if (workerThread.joinable())
  {
    workerThread.detach();
  }
  if (trace_mpm.get() != nullptr)
  {
    trace_mpm->Close();
    trace_mpm.reset();
  }
  if (trace_error.get() != nullptr)
  {
    trace_error->Close();
    trace_error.reset();
  }
#if defined(MIKTEX_WINDOWS)
  while (numCoInitialize > 0)
  {
    MyCoUninitialize();
  }
#endif
}

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER

bool PackageInstallerImpl::UseLocalServer()
{
  if (PackageManagerImpl::localServer)
  {
    // already running as local server
    return false;
  }
  if (noLocalServer)
  {
    return false;
  }
  if (!session->IsAdminMode())
  {
    return false;
  }
#if defined(MIKTEX_WINDOWS)
  bool elevationRequired = !session->RunningAsAdministrator();
  bool forceLocalServer = session->GetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_FORCE_LOCAL_SERVER, false).GetBool();
  return elevationRequired || forceLocalServer;
#else
  return false;
#endif
}

#endif

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER

HRESULT PackageInstallerImpl::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
  using namespace MiKTeXPackageManagerLib;
  if (trace_mpm->IsEnabled())
  {
    WCHAR szRiid[100];
    if (StringFromGUID2(riid, szRiid, 100) > 0)
    {
      trace_mpm->WriteFormattedLine("libmpm", "QI %S", szRiid);
    }
  }
  if (riid == __uuidof(IUnknown))
  {
    *ppvObj = static_cast<IUnknown*>(this);
  }
  else if (riid == __uuidof(IPackageInstallerCallback))

  {
    *ppvObj = static_cast<IPackageInstallerCallback*>(this);
  }
  else
  {
    *ppvObj = nullptr;
    return E_NOINTERFACE;
  }
  AddRef();
  return S_OK;
}

#endif

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER

ULONG PackageInstallerImpl::AddRef()
{
  return 1;
}

#endif

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER

ULONG PackageInstallerImpl::Release()
{
  return 1;
}

#endif

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER

HRESULT PackageInstallerImpl::ReportLine(BSTR line)
{
  try
  {
    if (callback != nullptr)
    {
      _bstr_t bstr(line, false);
      callback->ReportLine(static_cast<const char*>(bstr));
    }
    return S_OK;
  }
  catch (const exception&)
  {
    return E_FAIL;
  }
}

#endif

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER

HRESULT PackageInstallerImpl::OnRetryableError(BSTR message, VARIANT_BOOL* pDoContinue)
{
  try
  {
    if (callback != nullptr)
    {
      _bstr_t bstr(message, false);
      *pDoContinue = callback->OnRetryableError(static_cast<const char*>(bstr)) ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else
    {
      *pDoContinue = VARIANT_FALSE;
    }
    return S_OK;
  }
  catch (const exception&)
  {
    return E_FAIL;
  }
}

#endif

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER

HRESULT PackageInstallerImpl::OnProgress(LONG nf, VARIANT_BOOL* pDoContinue)
{
  try
  {
    if (callback != nullptr)
    {
      Notification notification((Notification)nf);
      *pDoContinue = callback->OnProgress(notification) ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else
    {
      *pDoContinue = VARIANT_TRUE;
    }
    return S_OK;
  }
  catch (const exception&)
  {
    return E_FAIL;
  }
}

#endif
