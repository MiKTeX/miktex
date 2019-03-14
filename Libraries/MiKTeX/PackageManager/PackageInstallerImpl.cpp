/* PackageInstaller.cpp:

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
#include <miktex/Trace/StopWatch>

#if defined(MIKTEX_WINDOWS)
#include <miktex/Core/win/winAutoResource>
#include <miktex/Core/win/DllProc>
#include <miktex/Core/win/HResult>
#endif

#include <miktex/PackageManager/PackageManager>

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

constexpr const char* LF = "\n";

template<typename T1, typename T2> double Divide(T1 a, T2 b)
{
  return static_cast<double>(a) / static_cast<double>(b);
}

string PackageInstallerImpl::MakeUrl(const string& relPath)
{
  return ::MakeUrl(repository, relPath);
}

PackageInstaller::~PackageInstaller() noexcept
{
}

MPMSTATICFUNC(bool) IsPureContainer(const string& packageId)
{
  return strncmp(packageId.c_str(), "_miktex-", 8) == 0;
}

MPMSTATICFUNC(bool) IsMiKTeXPackage(const string& packageId)
{
  return strncmp(packageId.c_str(), "miktex-", 7) == 0;
}

PackageInstallerImpl::PackageInstallerImpl(shared_ptr<PackageManagerImpl> manager) :
  session(Session::Get()),
  trace_error(TraceStream::Open(MIKTEX_TRACE_ERROR)),
  trace_mpm(TraceStream::Open(MIKTEX_TRACE_MPM)),
  trace_stopwatch(TraceStream::Open(MIKTEX_TRACE_STOPWATCH)),
  packageManager(manager),
  packageDataStore(manager->GetPackageDataStore())
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
  trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("going to download: {0} => {1}"), Q_(url), Q_(dest)));

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
  trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("start writing on {0}"), Q_(dest)));
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
  trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("downloaded {0:.2f} MB in {1:.2f} seconds"), mb, seconds));
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

  if (!fileName.empty())
  {
    installedFiles.insert(fileName);
  }

  // notify client: beginning of file extraction
  Notify(Notification::InstallFileStart);
}

void PackageInstallerImpl::OnEndFileExtraction(const string& fileName, size_t uncompressedSize)
{
  if (!fileName.empty())
  {
    installedFiles.insert(fileName);
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
  MiKTeX::Extractor::Extractor::CreateExtractor(archiveFileType)->Extract(archiveFileName, session->GetSpecialPath(SpecialPath::InstallRoot), true, this, TEXMF_PREFIX_DIRECTORY);
}

void PackageInstallerImpl::InstallRepositoryManifest()
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

    ReportLine(T_("loading package repository manifest..."));

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
        progressInfo.packageId = MIKTEX_REPOSITORY_MANIFEST_ARCHIVE_FILE_NAME_NO_SUFFIX;
        progressInfo.displayName = T_("Package repository manifest");
        progressInfo.cbPackageDownloadCompleted = 0;
        progressInfo.cbPackageDownloadTotal = ZZDB1_SIZE;
      }

      // download the database file
      Download(MakeUrl(MIKTEX_REPOSITORY_MANIFEST_ARCHIVE_FILE_NAME), pathZzdb1);
    }
    else
    {
      MIKTEX_ASSERT(repositoryType == RepositoryType::Local);
      pathZzdb1 = repository / MIKTEX_REPOSITORY_MANIFEST_ARCHIVE_FILE_NAME;
    }

    // unpack database
    MiKTeX::Extractor::Extractor::CreateExtractor(DB_ARCHIVE_FILE_TYPE)->Extract(pathZzdb1, pathConfigDir);
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

void PackageInstallerImpl::LoadRepositoryManifest(bool download)
{
  repositoryManifest.Clear();

  // path to mpm.ini
  PathName pathMpmIni(session->GetSpecialPath(SpecialPath::InstallRoot), MIKTEX_PATH_MPM_INI);

  // install (if necessary)
  time_t ONE_DAY_IN_SECONDS = 86400;
  if (download || !File::Exists(pathMpmIni) || File::GetLastWriteTime(pathMpmIni) + ONE_DAY_IN_SECONDS < time(nullptr))
  {
    InstallRepositoryManifest();
  }

  // load mpm.ini
  repositoryManifest.Load(pathMpmIni);

  // report digest
  ReportLine(fmt::format(T_("package repository digest: {0}"), repositoryManifest.GetDigest()));
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
  unique_ptr<StopWatch> stopWatch = StopWatch::Start(trace_stopwatch.get(), TRACE_FACILITY, "checking for updates");

  trace_mpm->WriteLine(TRACE_FACILITY, T_("searching for updateable packages"));

  UpdateDb();

  LoadRepositoryManifest(false);

  updates.clear();

  for (string packageId = repositoryManifest.FirstPackage(); !packageId.empty(); packageId = repositoryManifest.NextPackage())
  {
    Notify();

    UpdateInfo updateInfo;
    updateInfo.packageId = packageId;
    updateInfo.timePackaged = repositoryManifest.GetTimePackaged(packageId);
    updateInfo.version = repositoryManifest.GetPackageVersion(packageId);

#if IGNORE_OTHER_SYSTEMS
    string targetSystem = repositoryManifest.GetPackageTargetSystem(packageId);
    bool isOtherTargetSystem = !(targetSystem.empty() || targetSystem == MIKTEX_SYSTEM_TAG);
    if (isOtherTargetSystem)
    {
      continue;
    }
#endif

    bool isEssential = repositoryManifest.GetPackageLevel(packageId) <= PackageLevel::Essential;

    bool knownPackage;
    PackageInfo package;
    tie(knownPackage, package) = packageDataStore->TryGetPackage(packageId);

    if (!knownPackage || !package.IsInstalled())
    {
      if (isEssential)
      {
        trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("{0}: new essential package"), packageId));
        updateInfo.action = UpdateInfo::ForceUpdate;
        updates.push_back(updateInfo);
      }
      continue;
    }

    // clean the user-installation directory
    if (!session->IsAdminMode()
      && session->GetSpecialPath(SpecialPath::UserInstallRoot) != session->GetSpecialPath(SpecialPath::CommonInstallRoot)
      && IsValidTimeT(package.timeInstalledByUser)
      && IsValidTimeT(package.timeInstalledByAdmin))
    {
      if (!package.isRemovable)
      {
        MIKTEX_UNEXPECTED();
      }
      trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("{0}: double installed"), packageId));
      updateInfo.action = UpdateInfo::ForceRemove;
      updates.push_back(updateInfo);
      continue;
    }

    // check the integrity of installed MiKTeX packages
    if (IsMiKTeXPackage(packageId)
      && !packageManager->TryVerifyInstalledPackage(packageId)
      && package.isRemovable)
    {
      // the package has been tampered with
      trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("{0}: package is broken"), packageId));
      updateInfo.timePackaged = static_cast<time_t>(-1);
      updateInfo.action = UpdateInfo::Repair;
      updates.push_back(updateInfo);
      continue;
    }

    // compare digests, version numbers and time stamps
    MD5 md5 = repositoryManifest.GetPackageDigest(packageId);
    if (md5 == package.digest)
    {
      // digests do match => no update necessary
      continue;
    }

    // check release state mismatch
    bool isReleaseStateDiff = package.releaseState != RepositoryReleaseState::Unknown
      && repositoryReleaseState != RepositoryReleaseState::Unknown
      && package.releaseState != repositoryReleaseState;
    if (isReleaseStateDiff)
    {
      trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("{0}: package release state changed"), packageId));
    }
    else
    {
      trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("{0}: server has a different version"), packageId));
    }
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("server digest: {0}"), md5));;
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("local digest: {0}"), package.digest));
    if (!isReleaseStateDiff)
    {
      // compare time stamps
      time_t timePackaged = repositoryManifest.GetTimePackaged(packageId);
      if (timePackaged <= package.timePackaged)
      {
        // server has an older package => no update
        // necessary
        continue;
      }
      // server has a newer package
      trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("{0}: server has new version"), packageId));
    }

    if (!package.isRemovable)
    {
      trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("{0}: no permission to update package"), packageId));
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

  shared_ptr<PackageIterator> iter(packageManager->CreateIterator());
  iter->AddFilter({ PackageFilter::Obsolete });
  PackageInfo package;
  while (iter->GetNext(package))
  {
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("{0}: package is obsolete"), package.id));
    UpdateInfo updateInfo;
    updateInfo.packageId = package.id;
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
  trace_mpm->WriteLine(TRACE_FACILITY, T_("searching for upgrades"));
  UpdateDb();
  LoadRepositoryManifest(false);
  upgrades.clear();
  for (string packageId = repositoryManifest.FirstPackage(); !packageId.empty(); packageId = repositoryManifest.NextPackage())
  {
    Notify();
    bool knownPackage;
    PackageInfo package;
    tie(knownPackage, package) = packageDataStore->TryGetPackage(packageId);
    if (knownPackage && package.IsInstalled())
    {
      continue;
    }
#if IGNORE_OTHER_SYSTEMS
    string targetSystem = repositoryManifest.GetPackageTargetSystem(packageId);
    if (!targetSystem.empty() && targetSystem != MIKTEX_SYSTEM_TAG)
    {
      continue;
    }
#endif
    if (repositoryManifest.GetPackageLevel(packageId) > packageLevel)
    {
      continue;
    }
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("{0}: upgrade"), packageId));
    UpgradeInfo upgrade;
    upgrade.packageId = packageId;
    upgrade.timePackaged = repositoryManifest.GetTimePackaged(packageId);
    upgrade.version = repositoryManifest.GetPackageVersion(packageId);
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

    unsigned long refCount = packageDataStore->GetFileRefCount(f);

    // decrement the file reference counter
    if (refCount > 0)
    {
      refCount = packageDataStore->DecrementFileRefCount(f);
    }

    // make an absolute path name
    PathName path(session->GetSpecialPath(SpecialPath::InstallRoot), fileName);

    // only delete if the reference count reached zero
    if (refCount > 0)
    {
      trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("will not delete {0} (ref count is {1})"), Q_(path), refCount));
      done = true;
    }
    else if (File::Exists(path))
    {
      // remove the file
      try
      {
        File::Delete(path, { FileDeleteOption::TryHard });
        removedFiles.insert(path);
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
      trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("file {0} does not exist"), Q_(path)));
      done = true;
    }

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

void PackageInstallerImpl::RemovePackage(const string& packageId, Cfg& packageManifests)
{
  trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("going to remove {0}"), Q_(packageId)));

  // notify client
  Notify(Notification::RemovePackageStart);
  ReportLine(fmt::format(T_("removing package {0}..."), Q_(packageId)));

  // get package info
  PackageInfo package = packageDataStore->GetPackage(packageId);

  // check to see whether it is installed
  if (!package.IsInstalled())
  {
    MIKTEX_UNEXPECTED();
  }

  // clear the installTime value => package is not installed
  packageDataStore->SetTimeInstalled(packageId, InvalidTimeT);
  packageDataStore->SaveVarData();

  // remove the files
  trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("going to remove {0} file(s)"), package.runFiles.size() + package.docFiles.size() + package.sourceFiles.size()));
  RemoveFiles(package.runFiles);
  RemoveFiles(package.docFiles);
  RemoveFiles(package.sourceFiles);

  trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("package {0} successfully removed"), Q_(packageId)));

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

  installedFiles.insert(dest);
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

void PackageInstallerImpl::CopyPackage(const PathName& pathSourceRoot, const string& packageId)
{
  // parse the package manifest file
  PathName pathPackageFile = pathSourceRoot;
  pathPackageFile /= MIKTEX_PATH_PACKAGE_MANIFEST_DIR;
  pathPackageFile /= packageId;
  pathPackageFile.AppendExtension(MIKTEX_PACKAGE_MANIFEST_FILE_SUFFIX);
  unique_ptr<TpmParser> tpmparser = TpmParser::Create();
  tpmparser->Parse(pathPackageFile);

  // get the package info from the parser; set the package name
  PackageInfo package = tpmparser->GetPackageInfo();
  package.id = packageId;

  // copy the files
  CopyFiles(pathSourceRoot, package.runFiles);
  CopyFiles(pathSourceRoot, package.docFiles);
  CopyFiles(pathSourceRoot, package.sourceFiles);
}

MPMSTATICFUNC(unordered_set<PathName>) GetFiles(const PathName& rootDir, const PackageInfo& package)
{
  unordered_set<PathName> files;
  for (const string& s : package.runFiles)
  {
    string fileName;
    if (PackageManager::StripTeXMFPrefix(s, fileName))
    {
      files.insert((rootDir / fileName));
    }
  }
  for (const string& s : package.docFiles)
  {
    string fileName;
    if (PackageManager::StripTeXMFPrefix(s, fileName))
    {
      files.insert((rootDir / fileName));
    }
  }
  for (const string& s : package.sourceFiles)
  {
    string fileName;
    if (PackageManager::StripTeXMFPrefix(s, fileName))
    {
      files.insert((rootDir / fileName));
    }
  }
  return files;
}

void PackageInstallerImpl::UpdateFndb(const unordered_set<PathName>& installedFiles, const unordered_set<PathName>& removedFiles, const string& packageId)
{
  vector<PathName> toBeRemoved;
  for (const PathName& f : removedFiles)
  {
    if (installedFiles.find(f) == installedFiles.end() && Fndb::FileExists(f))
    {
      toBeRemoved.push_back(f);
    }
  }
  if (!toBeRemoved.empty())
  {
    Fndb::Remove(toBeRemoved);
  }
  vector<Fndb::Record> toBeAdded;
  for (const PathName& f : installedFiles)
  {
    if (!Fndb::FileExists(f))
    {
      toBeAdded.push_back({ f, packageId });
    }
  }
  if (!toBeAdded.empty())
  {
    Fndb::Add(toBeAdded);
  }
}

void PackageInstallerImpl::InstallPackage(const string& packageId, Cfg& packageManifests)
{
  trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("installing package {0}"), Q_(packageId)));

  // search the package table
  PackageInfo package = packageDataStore->GetPackage(packageId);

  NeedRepository();

  // initialize progress info
  {
    lock_guard<mutex> lockGuard(progressIndicatorMutex);
    progressInfo.packageId = packageId;
    progressInfo.displayName = package.displayName;
    progressInfo.cFilesPackageInstallCompleted = 0;
    progressInfo.cFilesPackageInstallTotal = package.GetNumFiles();
    progressInfo.cbPackageInstallCompleted = 0;
    progressInfo.cbPackageInstallTotal = package.GetSize();
    if (repositoryType == RepositoryType::Remote)
    {
      progressInfo.cbPackageDownloadCompleted = 0;
      progressInfo.cbPackageDownloadTotal = repositoryManifest.GetArchiveFileSize(packageId);
    }
  }

  // notify client: beginning of package installation
  Notify(Notification::InstallPackageStart);

  PathName pathArchiveFile;
  ArchiveFileType aft = repositoryManifest.GetArchiveFileType(packageId);
  unique_ptr<TemporaryFile> temporaryFile;

  // get hold of the archive file
  if (repositoryType == RepositoryType::Remote
    || repositoryType == RepositoryType::Local)
  {
    PathName packageFileName = packageId;
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
      pathArchiveFile = repository / packageId;
      pathArchiveFile.AppendExtension(MiKTeX::Extractor::Extractor::GetFileNameExtension(aft));
    }

    // check to see whether the digest is good
    if (!CheckArchiveFile(packageId, pathArchiveFile, false))
    {
      LoadRepositoryManifest(true);
      CheckArchiveFile(packageId, pathArchiveFile, true);
    }
  }

  installedFiles.clear();
  removedFiles.clear();

  // silently uninstall the package (this also decrements the file
  // reference counts)
  if (package.IsInstalled())
  {
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("{0}: removing old files"), packageId));
    RemoveFiles(package.runFiles, true);
    RemoveFiles(package.docFiles, true);
    RemoveFiles(package.sourceFiles, true);
    // temporarily set the status to "not installed"
    packageDataStore->SetTimeInstalled(packageId, InvalidTimeT);
    packageDataStore->SaveVarData();
  }

  if (repositoryType == RepositoryType::Remote || repositoryType == RepositoryType::Local)
  {
    // unpack the archive file
    ReportLine(fmt::format(T_("extracting files from {0}..."), Q_(packageId + MiKTeX::Extractor::Extractor::GetFileNameExtension(aft))));
    ExtractFiles(pathArchiveFile, aft);
  }
  else if (repositoryType == RepositoryType::MiKTeXDirect)
  {
    // copy from CD
    PathName pathSourceRoot(repository);
    pathSourceRoot /= MIKTEXDIRECT_PREFIX_DIR;
    CopyPackage(pathSourceRoot, packageId);
  }
  else if (repositoryType == RepositoryType::MiKTeXInstallation)
  {
    // import from another MiKTeX installation
    ReportLine(fmt::format(T_("importing package {0}..."), packageId));
    PathName pathSourceRoot(repository);
    CopyPackage(pathSourceRoot, packageId);
  }
  else
  {
    MIKTEX_UNEXPECTED();
  }

  // parse the new package manifest file
  PathName pathPackageFile = session->GetSpecialPath(SpecialPath::InstallRoot) / MIKTEX_PATH_PACKAGE_MANIFEST_DIR / packageId;
  pathPackageFile.AppendExtension(MIKTEX_PACKAGE_MANIFEST_FILE_SUFFIX);
  unique_ptr<TpmParser> tpmparser = TpmParser::Create();
  tpmparser->Parse(pathPackageFile);

  // get new package info
  PackageInfo newPackage = tpmparser->GetPackageInfo();

  // install new package manifest
  PackageManager::PutPackageManifest(packageManifests, newPackage, newPackage.timePackaged);

  // update file name database
  UpdateFndb(installedFiles, removedFiles, "");
  UpdateFndb(GetFiles(session->GetMpmRootPath(), newPackage), GetFiles(session->GetMpmRootPath(), package), packageId);

  // set the timeInstalled value => package is installed
  newPackage.timeInstalled = time(nullptr);
  packageDataStore->SetTimeInstalled(packageId, newPackage.timeInstalled);
  packageDataStore->SetReleaseState(packageId, repositoryReleaseState);
  packageDataStore->SaveVarData();

  // update package info table
  packageDataStore->SetPackage(newPackage);

  // increment file ref counts
  packageDataStore->IncrementFileRefCounts(packageId);

  // update progress info
  {
    lock_guard<mutex> lockGuard(progressIndicatorMutex);
    progressInfo.cPackagesInstallCompleted += 1;
  }

  // notify client: end of package installation
  Notify(Notification::InstallPackageEnd);
}

void PackageInstallerImpl::DownloadPackage(const string& packageId)
{
  size_t expectedSize;

  NeedRepository();

  // update progress info
  {
    lock_guard<mutex> lockGuard(progressIndicatorMutex);
    progressInfo.packageId = packageId;
    progressInfo.displayName = packageId;
    MIKTEX_ASSERT(repositoryType == RepositoryType::Remote);
    progressInfo.cbPackageDownloadCompleted = 0;
    progressInfo.cbPackageDownloadTotal = repositoryManifest.GetArchiveFileSize(packageId);
    expectedSize = progressInfo.cbPackageDownloadTotal;
  }

  // notify client: beginning of package download
  Notify(Notification::DownloadPackageStart);

  // make the archive file name
  ArchiveFileType aft = repositoryManifest.GetArchiveFileType(packageId);
  PathName pathArchiveFile = packageId;
  pathArchiveFile.AppendExtension(MiKTeX::Extractor::Extractor::GetFileNameExtension(aft));

  // download the archive file
  Download(pathArchiveFile, expectedSize);

  // check to see whether the archive file is ok
  CheckArchiveFile(packageId, downloadDirectory / pathArchiveFile, true);

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
      PackageInfo installCandidate = packageDataStore->GetPackage(p);
      package.cFilesInstallTotal += installCandidate.GetNumFiles();
      package.cbInstallTotal += installCandidate.GetSize();
    }
    if (repositoryType == RepositoryType::Remote)
    {
      size_t iSize = repositoryManifest.GetArchiveFileSize(p);
      if (iSize == 0)
      {
        LoadRepositoryManifest(true);
        if ((iSize = repositoryManifest.GetArchiveFileSize(p)) == 0)
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
      PackageInfo removeCandidate = packageDataStore->GetPackage(p);
      package.cFilesRemoveTotal += removeCandidate.GetNumFiles();
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

bool PackageInstallerImpl::CheckArchiveFile(const std::string& packageId, const PathName& archiveFileName, bool mustBeOk)
{
  if (!File::Exists(archiveFileName))
  {
    MIKTEX_FATAL_ERROR_2(FatalError(ERROR_MISSING_PACKAGE), "package", packageId, "archiveFile", archiveFileName.ToString());
  }
  MD5 digest1 = repositoryManifest.GetArchiveFileDigest(packageId);
  MD5 digest2 = MD5::FromFile(archiveFileName.GetData());
  bool ok = (digest1 == digest2);
  if (!ok && mustBeOk)
  {
    MIKTEX_FATAL_ERROR_2(FatalError(ERROR_CORRUPTED_PACKAGE), "package", packageId, "arhiveFile", archiveFileName.ToString(), "expectedMD5", digest1.ToString(), "actualMD5", digest2.ToString());
  }
  return ok;
}

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER

void PackageInstallerImpl::ConnectToServer()
{
  const char* MSG_CANNOT_START_SERVER = T_("Cannot start MiKTeX package manager.");
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
      trace_error->WriteLine(TRACE_FACILITY, fmt::format(T_("registration/unregistration of {0} did not succeed; hr={1}"), Q_(path), hr));
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
      trace_error->WriteLine(TRACE_FACILITY, fmt::format(T_("{0} {1} did not succeed (exit code: {2})"), regExe, cmdLine, exitCode));
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
    PackageInfo package = packageDataStore->GetPackage(p);
    for (const string& f : package.runFiles)
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
      if (!session->IsMiKTeXPortable() && session->RunningAsAdministrator())
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
  if (!session->IsMiKTeXPortable() && session->RunningAsAdministrator())
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

bool MIKTEXTHISCALL PackageInstallerImpl::OnProcessOutput(const void* output, size_t n)
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

void PackageInstallerImpl::CheckDependencies(set<string>& packages, const string& packageId, bool force, int level)
{
  if (level > 10)
  {
    MIKTEX_UNEXPECTED();
  }
  bool knownPackage;
  PackageInfo package;
  tie(knownPackage, package) = packageDataStore->TryGetPackage(packageId);
  if (knownPackage)
  {
    for (const string& p : package.requiredPackages)
    {
      CheckDependencies(packages, p, force, level + 1);
    }
  }
  if (force || (knownPackage && !package.IsInstalled()))
  {
    packages.insert(packageId);
  }
}

// FIXME: duplicate ocode
MPMSTATICFUNC(bool) EndsWith(const string& s, const string& suffix)
{
  return s.length() >= suffix.length() &&
    s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0;
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

  // make sure that mpm.fndb exists
  if (!File::Exists(session->GetMpmDatabasePathName()))
  {
    packageManager->CreateMpmFndb();
  }

  // collect all packages, if no packages were specified by the caller
  if (upgrade)
  {
    LoadRepositoryManifest(true);

    string packageId = repositoryManifest.FirstPackage();
    if (packageId.empty())
    {
      MIKTEX_FATAL_ERROR(T_("No packages on server."));
    }
    do
    {
      // search repository manifest
      PackageLevel lvl = repositoryManifest.GetPackageLevel(packageId);
      if (lvl > taskPackageLevel)
      {
        // not found or not required
        continue;
      }

#if IGNORE_OTHER_SYSTEMS
      // check target system
      string targetSystem = repositoryManifest.GetPackageTargetSystem(packageId);
      if (!(targetSystem.empty() || targetSystem == MIKTEX_SYSTEM_TAG))
      {
        continue;
      }
#endif

      if (repositoryType == RepositoryType::Local || repositoryType == RepositoryType::Remote)
      {
        // ignore pure containers
        if (IsPureContainer(packageId))
        {
          continue;
        }

        // check to see whether the archive file exists
        ArchiveFileType aft = repositoryManifest.GetArchiveFileType(packageId);
        PathName pathLocalArchiveFile = repository / packageId;
        pathLocalArchiveFile.AppendExtension(MiKTeX::Extractor::Extractor::GetFileNameExtension(aft));
        if (!File::Exists(pathLocalArchiveFile))
        {
          MIKTEX_FATAL_ERROR_2(FatalError(ERROR_MISSING_PACKAGE), "package", packageId, "archiveFile", pathLocalArchiveFile.ToString());
        }

        // check to see if the archive file is valid
        CheckArchiveFile(packageId, pathLocalArchiveFile, true);
      }

      // collect the package
      toBeInstalled.push_back(packageId);
    } while (!(packageId = repositoryManifest.NextPackage()).empty());
  }
  else if (!toBeInstalled.empty())
  {
    // we need mpm.ini, if packages are to be installed
    LoadRepositoryManifest(false);
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

  unique_ptr<Cfg> packageManifests = Cfg::Create();
  PathName packageManifestsIni = session->GetSpecialPath(SpecialPath::InstallRoot) / MIKTEX_PATH_PACKAGE_MANIFESTS_INI;
  if (File::Exists(packageManifestsIni))
  {
    packageManifests->Read(packageManifestsIni);
  }

  // install packages
  for (const string& p : toBeInstalled)
  {
    InstallPackage(p, *packageManifests);
  }

  // remove packages
  for (const string& p : toBeRemoved)
  {
    RemovePackage(p, *packageManifests);
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
    InstallPackage(p, *packageManifests);
  }

  if (File::Exists(packageManifestsIni))
  {
    packageManifests->Write(packageManifestsIni);
  }

  packageManifests = nullptr;

  if (!noPostProcessing)
  {
    RegisterComponents(true, toBeInstalled);
  }

  if (!noPostProcessing)
  {
    vector<string> args = { "--mkmaps" };
    if (session->IsAdminMode())
    {
      args.push_back("--mklinks");
    }
    else
    {
#if defined(MIKTEX_UNIX)
      // FIXME: duplicate code (initexmf.cpp)
      PathName scriptsIni;
      if (!session->FindFile(MIKTEX_PATH_SCRIPTS_INI, MIKTEX_PATH_TEXMF_PLACEHOLDER, scriptsIni))
      {
        MIKTEX_FATAL_ERROR(T_("Script configuration file not found."));
      }
      unique_ptr<Cfg> config(Cfg::Create());
      config->Read(scriptsIni, true);
      for (const shared_ptr<Cfg::Key>& key : *config)
      {
        if (key->GetName() != "sh" && key->GetName() != "exe")
        {
          continue;
        }
        for (const shared_ptr<Cfg::Value>& val : *key)
        {
          if (EndsWith(val->GetName(), "[]"))
          {
            continue;
          }
          PathName scriptPath;
          if (!session->FindFile(session->Expand(val->AsString()), MIKTEX_PATH_TEXMF_PLACEHOLDER_NO_MPM, scriptPath))
          {
            continue;
          }
          if (session->GetRootDirectories()[session->DeriveTEXMFRoot(scriptPath)].IsCommon() && !session->IsAdminMode())
          {
            continue;
          }
          File::SetAttributes(scriptPath, File::GetAttributes(scriptPath) + FileAttribute::Executable);
        }
      }
#endif
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

  // download and load the package repository manifest
  LoadRepositoryManifest(true);

  // collect required packages
  string packageId;
  if (!(packageId = repositoryManifest.FirstPackage()).empty())
  {
    do
    {
      // don't add pure containers
      if (IsPureContainer(packageId))
      {
        continue;
      }

      // check package level
      if (taskPackageLevel < repositoryManifest.GetPackageLevel(packageId))
      {
        // package is not required
        continue;
      }

      // check to see whether the file was downloaded previously
      ArchiveFileType aft = repositoryManifest.GetArchiveFileType(packageId);
      PathName pathLocalArchiveFile = downloadDirectory / packageId;
      pathLocalArchiveFile.AppendExtension(MiKTeX::Extractor::Extractor::GetFileNameExtension(aft));
      if (File::Exists(pathLocalArchiveFile))
      {
        // the archive file exists;  check to see if it is valid
        MD5 digest1 = repositoryManifest.GetArchiveFileDigest(packageId);
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
      toBeInstalled.push_back(packageId);
    } while (!(packageId = repositoryManifest.NextPackage()).empty());
  }

  // count bytes
  CalculateExpenditure(true);

  // download database archive files
  ReportLine(T_("downloading package database..."));
  {
    lock_guard<mutex> lockGuard(progressIndicatorMutex);
    progressInfo.packageId = MIKTEX_REPOSITORY_MANIFEST_ARCHIVE_FILE_NAME_NO_SUFFIX;
    progressInfo.displayName = T_("Package repository manifest");
    progressInfo.cbPackageDownloadCompleted = 0;
    progressInfo.cbPackageDownloadTotal = ZZDB1_SIZE;
  }
  Download(MIKTEX_REPOSITORY_MANIFEST_ARCHIVE_FILE_NAME);
  {
    lock_guard<mutex> lockGuard(progressIndicatorMutex);
    progressInfo.packageId = MIKTEX_PACKAGE_MANIFESTS_ARCHIVE_FILE_NAME_NO_SUFFIX;
    progressInfo.displayName = T_("Package manifests");
    progressInfo.cbPackageDownloadCompleted = 0;
    progressInfo.cbPackageDownloadTotal = ZZDB3_SIZE;
  }
  Download(MIKTEX_PACKAGE_MANIFESTS_ARCHIVE_FILE_NAME);

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

void PackageInstallerImpl::CleanUpUserDatabase()
{
  MIKTEX_ASSERT(!session->IsAdminMode());
  
  PathName userManifestsPath(session->GetSpecialPath(SpecialPath::UserInstallRoot), MIKTEX_PATH_PACKAGE_MANIFESTS_INI);
  PathName commonManifestsPath(session->GetSpecialPath(SpecialPath::CommonInstallRoot), MIKTEX_PATH_PACKAGE_MANIFESTS_INI);

  if (!File::Exists(userManifestsPath) || !File::Exists(commonManifestsPath))
  {
    return;
  }

  if (userManifestsPath.Canonicalize() == commonManifestsPath.Canonicalize())
  {
    return;
  }

  vector<string> toBeRemoved;

  unique_ptr<Cfg> userManifests = Cfg::Create();
  userManifests->Read(userManifestsPath);

  unique_ptr<Cfg> commonManifests = Cfg::Create();
  commonManifests->Read(commonManifestsPath);

  // check all user package manifests
  for (auto keyUser : *userManifests)
  {
    string packageId = keyUser->GetName();

    // check to see whether the system-wide manifest exists
    auto keyCommon = commonManifests->GetKey(packageId);
    if (keyCommon == nullptr)
    {
      continue;
    }

    // compare manifests
    if (PackageManager::GetPackageManifest(*userManifests, packageId, TEXMF_PREFIX_DIRECTORY) == PackageManager::GetPackageManifest(*commonManifests, packageId, TEXMF_PREFIX_DIRECTORY))
    {
      // manifests are identical; remove user manifest later
      toBeRemoved.push_back(packageId);
    }
  }

  // remove redundant user package manifests
  for (const string& packageId : toBeRemoved)
  {
    trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("removing redundant package manifest: {0}"), packageId));
    userManifests->DeleteKey(packageId);
  }

  userManifests->Write(userManifestsPath);
}

void PackageInstallerImpl::HandleObsoletePackageManifests(Cfg& existingManifests, const Cfg& newManifests)
{
  vector<string> toBeRemoved;
  for (auto keyExisting : existingManifests)
  {
    string packageId = keyExisting->GetName();

    // it's not an obsolete package if newManifests
    // contains a corresponding package manifest
    if (newManifests.GetKey(packageId) != nullptr)
    {
      continue;
    }

    bool knownPackage;
    PackageInfo packageInfo;
    tie(knownPackage, packageInfo) = packageDataStore->TryGetPackage(packageId);
    if (!knownPackage)
    {
      // might be a different architecture and hence not in the package store
      continue;
    }
    
    // now we know that the package is obsolete
    // check to see whether the obsolete package is installed
    if (!packageInfo.IsInstalled() || IsPureContainer(packageId))
    {
      // not installed: remove the package manifest (later)
      trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("removing obsolete package manifest '{0}'"), packageId));
      toBeRemoved.push_back(packageId);
    }
    else
    {
      // installed: declare the package as obsolete (we wont
      // uninstall obsolete packages)
      trace_mpm->WriteLine(TRACE_FACILITY, fmt::format(T_("declaring '{0}' obsolete"), packageId));
      packageDataStore->DeclareObsolete(packageId);
    }
  }
  for (const auto& p : toBeRemoved)
  {
    existingManifests.DeleteKey(p);
  }
  packageDataStore->SaveVarData();
}

void PackageInstallerImpl::UpdateDb()
{
  unique_ptr<StopWatch> stopWatch = StopWatch::Start(trace_stopwatch.get(), TRACE_FACILITY, "update package database");

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

  // we need a temporary directory
  unique_ptr<TemporaryDirectory> tempDir;
  tempDir = TemporaryDirectory::Create();

  PathName archivePath;

  if (repositoryType == RepositoryType::Remote)
  {
    // download the archive file
    archivePath = tempDir->GetPathName() / MIKTEX_PACKAGE_MANIFESTS_ARCHIVE_FILE_NAME;
    Download(MakeUrl(MIKTEX_PACKAGE_MANIFESTS_ARCHIVE_FILE_NAME), archivePath);
  }
  else
  {
    MIKTEX_ASSERT(repositoryType == RepositoryType::Local);
    archivePath = repository / MIKTEX_PACKAGE_MANIFESTS_ARCHIVE_FILE_NAME;
  }

  // extract new package-manifests.ini
  MiKTeX::Extractor::Extractor::CreateExtractor(DB_ARCHIVE_FILE_TYPE)->Extract(archivePath, tempDir->GetPathName());

  // load new package-manifests.ini
  unique_ptr<Cfg> newManifests = Cfg::Create();
  newManifests->Read(tempDir->GetPathName() / MIKTEX_PACKAGE_MANIFESTS_INI_FILENAME);

  // load existing package-manifests.ini
  unique_ptr<Cfg> existingManifests = Cfg::Create();
  packageDataStore->NeedPackageManifestsIni();
  PathName existingPackageManifestsIni = session->GetSpecialPath(SpecialPath::InstallRoot) / MIKTEX_PATH_PACKAGE_MANIFESTS_INI;
  if (File::Exists(existingPackageManifestsIni))
  {
    existingManifests->Read(existingPackageManifestsIni);
  }

  HandleObsoletePackageManifests(*existingManifests, *newManifests);

  // update the package manifests
  ReportLine(fmt::format(T_("updating package manifests ({0})..."), Q_(existingPackageManifestsIni)));
  size_t count = 0;
  for (auto key : *newManifests)
  {
    string packageId = key->GetName();

    Notify();

    // ignore manifest, if package is already installed
    bool knownPackage;
    PackageInfo existingPackage;
    tie(knownPackage, existingPackage) = packageDataStore->TryGetPackage(packageId);
    if (!IsPureContainer(packageId) && knownPackage && (session->IsAdminMode() ? existingPackage.IsInstalledByAdmin() : existingPackage.IsInstalledByUser()))
    {
      continue;
    }

    if (existingManifests->GetKey(packageId) != nullptr)
    {
      // remove the existing manifest
      existingManifests->DeleteKey(packageId);
    }

    // install the new manifest
    PackageInfo packageInfo = PackageManager::GetPackageManifest(*newManifests, packageId, TEXMF_PREFIX_DIRECTORY);
    PackageManager::PutPackageManifest(*existingManifests, packageInfo, packageInfo.timePackaged);

    // update the package table
    packageDataStore->DefinePackage(packageInfo);

    ++count;
  }

  // write package-manifests.ini
  existingManifests->Write(existingPackageManifestsIni);

  ReportLine(fmt::format(T_("installed {0} package manifests"), count));

  // clean up the user database
  if (!session->IsAdminMode())
  {
    CleanUpUserDatabase();
  }

  // install mpm.ini
  InstallRepositoryManifest();

  // force a reload of the database
  repositoryManifest.Clear();
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
  if (trace_stopwatch.get() != nullptr)
  {
    trace_stopwatch->Close();
    trace_stopwatch.reset();
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
  if (trace_mpm->IsEnabled(TRACE_FACILITY))
  {
    WCHAR szRiid[100];
    if (StringFromGUID2(riid, szRiid, 100) > 0)
    {
      trace_mpm->WriteLine(TRACE_FACILITY, fmt::format("QI {0}", StringUtil::WideCharToUTF8(szRiid)));
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
