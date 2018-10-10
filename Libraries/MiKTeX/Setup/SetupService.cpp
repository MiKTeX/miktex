/* SetupService.cpp:

   Copyright (C) 2013-2018 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include "internal.h"

#if defined(MIKTEX_WINDOWS)
#  include "win/winSetupService.h"
#endif

#include "setup-version.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Extractor;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Setup;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace std;
using namespace std::string_literals;

#define LICENSE_FILE "LICENSE.TXT"
#define DOWNLOAD_INFO_FILE "README.TXT"

#define BASIC_MIKTEX "\"Basic MiKTeX\""
#define BASIC_MIKTEX_LEGACY "\"Small MiKTeX\""
#define COMPLETE_MIKTEX "\"Complete MiKTeX\""
#define COMPLETE_MIKTEX_LEGACY "\"Total MiKTeX\""
#define ESSENTIAL_MIKTEX "\"Essential MiKTeX\""

SETUPSTATICFUNC(int) ComparePaths(const PathName& path1, const PathName& path2, bool shortify)
{
#if defined(MIKTEX_WINDOWS)
  wchar_t szShortPath1[BufferSizes::MaxPath];
  wchar_t szShortPath2[BufferSizes::MaxPath];

  if (shortify
    && (GetShortPathNameW(path1.ToWideCharString().c_str(), szShortPath1, BufferSizes::MaxPath) > 0)
    && (GetShortPathNameW(path2.ToWideCharString().c_str(), szShortPath2, BufferSizes::MaxPath) > 0))
  {
    return PathName::Compare(szShortPath1, szShortPath2);
  }
#endif
  return PathName::Compare(path1, path2);
}

BEGIN_INTERNAL_NAMESPACE;

void RemoveEmptyDirectoryChain(const PathName& directory)
{
  unique_ptr<DirectoryLister> pLister = DirectoryLister::Open(directory);
  DirectoryEntry dirEntry;
  bool empty = !pLister->GetNext(dirEntry);
  pLister->Close();
  if (!empty)
  {
    return;
  }
  FileAttributeSet attributes = File::GetAttributes(directory);
  if (attributes[FileAttribute::ReadOnly])
  {
    attributes -= FileAttribute::ReadOnly;
    File::SetAttributes(directory, attributes);
  }
  Directory::Delete(directory);
  PathName parentDir(directory);
  parentDir.CutOffLastComponent();
  if (parentDir == directory)
  {
    return;
  }
  RemoveEmptyDirectoryChain(parentDir);
}

bool Contains(const vector<PathName>& vec, const PathName& pathName)
{
  for (const PathName& p : vec)
  {
    if (p == pathName)
    {
      return true;
    }
  }
  return false;
}

END_INTERNAL_NAMESPACE;

SetupService::~SetupService() noexcept
{
}

SetupServiceImpl::SetupServiceImpl()
{
  traceStream = TraceStream::Open("setup");
  TraceStream::SetTraceFlags("error,extractor,mpm,process,config,setup");
  packageManager = PackageManager::Create();
  shared_ptr<Session> session = Session::Get();
  logFile.SetCallback(this);
  options.IsCommonSetup = session->RunningAsAdministrator();
  logFile.SetLogFileName(session->GetSpecialPath(SpecialPath::InstallRoot) / MIKTEX_PATH_UNINST_LOG);
}

SetupServiceImpl::~SetupServiceImpl()
{
  try
  {
  }
  catch (const exception &)
  {
  }
}

unique_ptr<SetupService> SetupService::Create()
{
#if defined(MIKTEX_WINDOWS)
  return make_unique<winSetupServiceImpl>();
#else
  return make_unique<SetupServiceImpl>();
#endif
}

PathName SetupService::GetDefaultLocalRepository()
{
  PathName ret;
  string val;
  shared_ptr<Session> session = Session::Get();
  if (session->TryGetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_LOCAL_REPOSITORY, val))
  {
    ret = val;
  }
  else
  {
    // default is current users Desktop\MiKTeX Download Files"
#if defined(MIKTEX_WINDOWS)
    ret = Utils::GetFolderPath(CSIDL_DESKTOPDIRECTORY, CSIDL_DESKTOPDIRECTORY, true);
    ret /= "MiKTeX Download Files";
#else
    // TODO
    MIKTEX_UNEXPECTED();
#endif
  }
  return ret;
}

PackageLevel SetupService::SearchLocalRepository(PathName& localRepository, PackageLevel requestedPackageLevel, bool& prefabricated)
{
  PackageLevel packageLevel_ = PackageLevel::None;

  // try current directory
  localRepository.SetToCurrentDirectory();
  packageLevel_ = SetupService::TestLocalRepository(localRepository, requestedPackageLevel);
  if (packageLevel_ != PackageLevel::None)
  {
    prefabricated = true;
    return packageLevel_;
  }

  // try my directory
  shared_ptr<Session> session = Session::Get();
  localRepository = session->GetMyLocation(false);
  packageLevel_ = SetupService::TestLocalRepository(localRepository, requestedPackageLevel);
  if (packageLevel_ != PackageLevel::None)
  {
    prefabricated = true;
    return packageLevel_;
  }

  // try ..\tm\packages
  localRepository = session->GetMyLocation(false) / ".." / "tm" / "packages";
  localRepository.MakeAbsolute();
  packageLevel_ = SetupService::TestLocalRepository(localRepository, requestedPackageLevel);
  if (packageLevel_ != PackageLevel::None)
  {
    prefabricated = true;
    return packageLevel_;
  }

  // try last directory
  if (PackageManager::TryGetLocalPackageRepository(localRepository))
  {
    packageLevel_ = SetupService::TestLocalRepository(localRepository, requestedPackageLevel);
    if (packageLevel_ != PackageLevel::None)
    {
      prefabricated = false;
      return packageLevel_;
    }
  }

  return PackageLevel::None;
}

PackageLevel SetupService::TestLocalRepository(const PathName& pathRepository, PackageLevel requestedPackageLevel)
{
  PathName pathInfoFile(pathRepository, DOWNLOAD_INFO_FILE);
  if (!File::Exists(pathInfoFile))
  {
    return PackageLevel::None;
  }
  StreamReader stream(pathInfoFile);
  string firstLine;
  bool haveFirstLine = stream.ReadLine(firstLine);
  stream.Close();
  if (!haveFirstLine)
  {
    return PackageLevel::None;
  }
  PackageLevel packageLevel_ = PackageLevel::None;
  if (firstLine.find(ESSENTIAL_MIKTEX) != string::npos)
  {
    packageLevel_ = PackageLevel::Essential;
  }
  else if (firstLine.find(BASIC_MIKTEX) != string::npos)
  {
    packageLevel_ = PackageLevel::Basic;
  }
  else if (firstLine.find(COMPLETE_MIKTEX) != string::npos
    || firstLine.find(COMPLETE_MIKTEX_LEGACY) != string::npos)
  {
    packageLevel_ = PackageLevel::Complete;
  }
  else
  {
    // README.TXT doesn't look right
    return PackageLevel::None;
  }
  if (requestedPackageLevel > packageLevel_)
  {
    // doesn't have the requested package set
    return PackageLevel::None;
  }
  return packageLevel_;
}

bool SetupService::IsMiKTeXDirect(PathName& root)
{
  // check ..\texmf\miktex\config\miktexstartup.ini
  shared_ptr<Session> session = Session::Get();
  root = session->GetMyLocation(false) / "..";
  root.MakeAbsolute();
  PathName pathStartupConfig = root / "texmf" / MIKTEX_PATH_STARTUP_CONFIG_FILE;
  if (!File::Exists(pathStartupConfig))
  {
    return false;
  }
  FileAttributeSet attributes = File::GetAttributes(pathStartupConfig);
  if (!attributes[FileAttribute::ReadOnly])
  {
    return false;
  }
  unique_ptr<Cfg> pcfg(Cfg::Create());
  pcfg->Read(pathStartupConfig);
  string str;
  if (!pcfg->TryGetValue("Auto", "Config", str) || str != "Direct")
  {
    return false;
  }
  return true;
}

unique_ptr<TemporaryDirectory> SetupService::ExtractFiles()
{
  wchar_t szPath[BufferSizes::MaxPath];
#if MIKTEX_WINDOWS
  if (GetModuleFileNameW(nullptr, szPath, BufferSizes::MaxPath) == 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetModuleFileNameW");
  }
#else
  // TODO: get path of running executable
  UNIMPLEMENTED();
#endif
  FileStream myImage(File::Open(szPath, FileMode::Open, FileAccess::Read, false));
  char magic[16];
  while (myImage.Read(magic, 16) == 16)
  {
    static char const MAGIC3[3] = { 'T', 'A', 'R' };
    if (memcmp(magic, MAGIC3, 3) == 0 &&
        memcmp(magic + 3, MAGIC3, 3) == 0 &&
        memcmp(magic + 6, MAGIC3, 3) == 0 &&
        memcmp(magic + 9, MAGIC3, 3) == 0 &&
        memcmp(magic + 12, MAGIC3, 3) == 0 &&
        memcmp(magic + 15, MAGIC3, 1) == 0)
    {
      unique_ptr<MiKTeX::Extractor::Extractor> extractor(MiKTeX::Extractor::Extractor::CreateExtractor(MiKTeX::Extractor::ArchiveFileType::Tar));
      unique_ptr<TemporaryDirectory> sfxDir = TemporaryDirectory::Create();
      extractor->Extract(&myImage, sfxDir->GetPathName(), true);
      return sfxDir;
    }
    else
    {
      myImage.Seek(512 - 16, SeekOrigin::Current);
    }
  }
  return nullptr;
}

SetupOptions SetupServiceImpl::SetOptions(const SetupOptions& options)
{
  this->options = options;
  CompleteOptions(false);
  return this->options;
}

void SetupServiceImpl::OpenLog()
{
  if (logStream.is_open())
  {
    return;
  }

  // make the intermediate log file name
  intermediateLogFile.SetToTempFile();

  // open the intermediate log file
  lock_guard<mutex> lockGuard(logStreamMutex);
  logStream = File::CreateOutputStream(intermediateLogFile);
  logStream
    << "\n"
    << endl;
}

PathName SetupServiceImpl::CloseLog(bool cancel)
{
  // we must have an intermediate log file
  if (!logStream.is_open())
  {
    return "";
  }

  // close the intermediate log file
  logStream.close();

  if (cancel)
  {
    File::Delete(intermediateLogFile);
    return "";
  }

  // determine the final log directory
  PathName pathLogDir;
  if (options.IsDryRun || options.Task == SetupTask::PrepareMiKTeXDirect)
  {
    pathLogDir.SetToTempDirectory();
  }
  else
  {
    if (options.Task == SetupTask::InstallFromCD || options.Task == SetupTask::InstallFromLocalRepository || options.Task == SetupTask::InstallFromRemoteRepository)
    {
      if (Directory::Exists(GetInstallRoot()))
      {
        pathLogDir = GetInstallRoot() / MIKTEX_PATH_MIKTEX_CONFIG_DIR;
      }
      else
      {
        pathLogDir.SetToTempDirectory();
      }
    }
    else if (options.Task == SetupTask::Download)
    {
      if (Directory::Exists(options.LocalPackageRepository))
      {
        pathLogDir = options.LocalPackageRepository;
      }
      else
      {
        pathLogDir.SetToTempDirectory();
      }
    }
    else
    {
      // remove the intermediate log file
      File::Delete(intermediateLogFile);
      return "";
    }
  }

  // create the log directory
  Directory::Create(pathLogDir);

  // make the final log path name
  PathName pathLogFile(pathLogDir);
  time_t t = time(nullptr);
  char dateTime[128];
  strftime(dateTime, 128, "%Y-%m-%d-%H-%M", localtime(&t));
  string fileName;
  if (options.Task == SetupTask::Download)
  {
    fileName = "download";
  }
  else
  {
    fileName = "setup";
  }
  fileName += "-";
  fileName += dateTime;
  pathLogFile /= fileName.c_str();
  pathLogFile.AppendExtension(".log");

  // install the log file
  // <todo>add the log file to the uninstall script</todo>
  File::Copy(intermediateLogFile, pathLogFile);

  // remove the intermediate log file
  File::Delete(intermediateLogFile);

  return pathLogFile;
}

void SetupServiceImpl::LogHeader()
{
  Log(fmt::format(T_("{0} {0} Report\n\n"), options.Banner, options.Version));
  time_t t = time(nullptr);
  struct tm* pTm = localtime(&t);
  Log(fmt::format(T_("Date: {0:%A, %B %d, %Y}\n"), *pTm));
  Log(fmt::format(T_("Time: {0:%H:%M:%S}\n"), *pTm));
  Log(fmt::format(T_("OS version: {0}\n"), Utils::GetOSVersionString()));
  shared_ptr<Session> session = Session::Get();
#if defined(MIKTEX_WINDOWS)
  Log(fmt::format("SystemAdmin: {}\n", session->RunningAsAdministrator()));
  Log(fmt::format("PowerUser: {}\n", session->RunningAsPowerUser()));
#endif
  if (options.Task != SetupTask::Download)
  {
    Log(fmt::format("SharedSetup: {}\n", options.IsCommonSetup));
  }
#if MIKTEX_WINDOWS
  wchar_t szSetupPath[BufferSizes::MaxPath];
  if (GetModuleFileNameW(0, szSetupPath, BufferSizes::MaxPath) == 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetModuleFileNameW");
  }
  Log(fmt::format(T_("Setup path: {0}\n"), WU_(szSetupPath)));
#else
  // TODO: log setup path
#endif
  if (options.Task != SetupTask::Download)
  {
    Log(fmt::format("UserRoots: {}\n", options.Config.userRoots.empty() ? T_("<none specified>") : options.Config.userRoots));
    Log(fmt::format("UserData: {}\n", options.Config.userDataRoot.Empty() ? T_("<none specified>") : options.Config.userDataRoot));
    Log(fmt::format("UserConfig: {}\n", options.Config.userConfigRoot.Empty() ? T_("<none specified>") : options.Config.userConfigRoot));
    Log(fmt::format("CommonRoots: {}\n", options.Config.commonRoots.empty() ? T_("<none specified>") : options.Config.commonRoots));
    Log(fmt::format("CommonData: {}\n", options.Config.commonDataRoot.Empty() ? T_("<none specified>") : options.Config.commonDataRoot));
    Log(fmt::format("CommonConfig: {}\n", options.Config.commonConfigRoot.Empty() ? T_("<none specified>") : options.Config.commonConfigRoot));
    PathName installRoot = GetInstallRoot();
    Log(fmt::format("Installation: {}\n", installRoot.Empty() ? T_("<none specified>") : installRoot));
  }
}

void SetupServiceImpl::Log(const string& s)
{
#if 0
  lock_guard<mutex> lockGuard(logStreamMutex);
#endif
  if (!logging)
  {
    logging = true;
    LogHeader();
  }
  static string currentLine;
  for (const char* lpsz = s.c_str(); *lpsz != 0; ++lpsz)
  {
    if (lpsz[0] == '\n' || (lpsz[0] == '\r' && lpsz[1] == '\n'))
    {
      traceStream->WriteFormattedLine("setup", "%s", currentLine.c_str());
      if (logStream.is_open())
      {
        logStream << currentLine << "\n";
      }
      currentLine = "";
      if (lpsz[0] == '\r')
      {
        ++lpsz;
      }
    }
    else
    {
      currentLine += *lpsz;
    }
  }
}

void SetupServiceImpl::ULogOpen()
{
  if (options.Task == SetupTask::Download || options.Task == SetupTask::FinishSetup || options.Task == SetupTask::FinishUpdate || options.Task == SetupTask::CleanUp)
  {
    return;
  }
  PathName uninstLog(GetULogFileName());
  uninstStream = File::CreateOutputStream(uninstLog, File::Exists(uninstLog) ? ios_base::app : ios_base::out);
  section = None;
}

PathName SetupServiceImpl::GetULogFileName()
{
  PathName ret;
  if (options.IsDryRun || options.Task == SetupTask::PrepareMiKTeXDirect)
  {
    ret.SetToTempDirectory();
  }
  else
  {
    ret = GetInstallRoot() / MIKTEX_PATH_MIKTEX_CONFIG_DIR;
  }
  Directory::Create(ret);
  ret /= MIKTEX_UNINSTALL_LOG;
  return ret;
}

void SetupServiceImpl::ULogClose(bool finalize)
{
  if (!uninstStream.is_open())
  {
    return;
  }

  try
  {
    if (finalize)
    {
      ULogAddFile(GetULogFileName());
      if (!options.IsPortable)
      {
        RegisterUninstaller();
      }
    }
  }
  catch (const exception &)
  {
    uninstStream.close();
    throw;
  }

  uninstStream.close();
}

void SetupServiceImpl::ULogAddFile(const PathName& path)
{
  if (!uninstStream.is_open())
  {
    return;
  }
  if (section != Files)
  {
    uninstStream << "[files]" << "\n";
    section = Files;
  }
  PathName absolutePath(path);
  absolutePath.MakeAbsolute();
#if defined(MIKTEX_WINDOWS)
  absolutePath.ConvertToDos();
#endif
  uninstStream << absolutePath << endl;
}

void SetupServiceImpl::SetCallback(SetupServiceCallback* callback)
{
  this->callback = callback;
}

void SetupServiceImpl::SetCallbacks(std::function<void(const string&)> f_ReportLine, function<bool(const string&)> f_OnRetryableError, function<bool(MiKTeX::Setup::Notification)> f_OnProgress, function<bool(const void*, size_t)> f_OnProcessOutput)
{
  myCallbacks.f_ReportLine = f_ReportLine;
  myCallbacks.f_OnRetryableError = f_OnRetryableError;
  myCallbacks.f_OnProgress = f_OnProgress;
  myCallbacks.f_OnProcessOutput = f_OnProcessOutput;
  this->callback = &myCallbacks;
}

void SetupServiceImpl::Run()
{
  Initialize();
  switch (options.Task)
  {
  case SetupTask::None:
    ReportLine("nothing to do");
    break;
  case SetupTask::Download:
    DoTheDownload();
    break;
  case SetupTask::PrepareMiKTeXDirect:
    DoPrepareMiKTeXDirect();
    break;
  case SetupTask::InstallFromCD:
  case SetupTask::InstallFromLocalRepository:
    DoTheInstallation();
    break;
  case SetupTask::FinishSetup:
    DoFinishSetup();
    break;
  case SetupTask::FinishUpdate:
    DoFinishUpdate();
    break;
  case SetupTask::CleanUp:
    DoCleanUp();
    break;
  default:
    MIKTEX_UNEXPECTED();
  }
  ULogClose(true);
}

void SetupServiceImpl::CompleteOptions(bool allowRemoteCalls)
{
  shared_ptr<Session> session = Session::Get();

  if (options.Task == SetupTask::FinishUpdate || options.Task == SetupTask::CleanUp)
  {
    options.IsCommonSetup = session->IsSharedSetup();
    options.IsPortable = session->IsMiKTeXPortable();
  }
  if (options.Task == SetupTask::CleanUp)
  {
    options.Config.commonInstallRoot = session->GetSpecialPath(SpecialPath::CommonInstallRoot);
    options.Config.commonConfigRoot = session->GetSpecialPath(SpecialPath::CommonConfigRoot);
    options.Config.commonDataRoot = session->GetSpecialPath(SpecialPath::CommonDataRoot);
    options.Config.userInstallRoot = session->GetSpecialPath(SpecialPath::UserInstallRoot);
    options.Config.userConfigRoot = session->GetSpecialPath(SpecialPath::UserConfigRoot);
    options.Config.userDataRoot = session->GetSpecialPath(SpecialPath::UserDataRoot);
    return;
  }
  if (options.Task == SetupTask::InstallFromLocalRepository)
  {
    if (options.IsPortable)
    {
      if (options.PortableRoot.Empty())
      {
        options.PortableRoot = GetDefaultPortableRoot();
      }
    }
    else if (options.IsCommonSetup)
    {
      if (options.Config.commonInstallRoot.Empty())
      {
        options.Config.commonInstallRoot = GetDefaultCommonInstallDir();
      }
#if 0
      options.Config.userInstallRoot = "";
#endif
    }
    else
    {
      if (options.Config.userInstallRoot.Empty())
      {
        options.Config.userInstallRoot = GetDefaultUserInstallDir();
      }
#if 0
      options.Config.commonInstallRoot = "";
#endif
    }
  }
  if (options.Task == SetupTask::Download || options.Task == SetupTask::InstallFromLocalRepository)
  {
    if (options.LocalPackageRepository.Empty())
    {
      PackageLevel foundPackageLevel = SearchLocalRepository(options.LocalPackageRepository, options.PackageLevel, options.IsPrefabricated);
      if (foundPackageLevel == PackageLevel::None)
      {
        // check the default location
        options.LocalPackageRepository = SetupService::GetDefaultLocalRepository();
        foundPackageLevel = SetupService::TestLocalRepository(options.LocalPackageRepository, options.PackageLevel);
      }
      if (options.Task == SetupTask::InstallFromLocalRepository)
      {
        if (foundPackageLevel < options.PackageLevel)
        {
          MIKTEX_FATAL_ERROR(T_("no local package directory found"));
        }
      }
      if (options.PackageLevel == PackageLevel::None)
      {
        options.PackageLevel = foundPackageLevel;
      }
    }
  }
  if (options.PackageLevel == PackageLevel::None)
  {
    options.PackageLevel = PackageLevel::Complete;
  }
  if ((options.RemotePackageRepository.empty() && options.Task == SetupTask::Download) || options.Task == SetupTask::InstallFromRemoteRepository)
  {
    if (!packageManager->TryGetRemotePackageRepository(options.RemotePackageRepository) && allowRemoteCalls)
    {
      options.RemotePackageRepository = packageManager->PickRepositoryUrl();
    }
  }
}

void SetupServiceImpl::Initialize()
{
  shared_ptr<Session> session = Session::Get();

  if (initialized)
  {
    return;
  }
  initialized = true;

  ReportLine("initializing setup service...");

  packageInstaller = packageManager->CreateInstaller();
  packageInstaller->SetNoPostProcessing(true);
  packageInstaller->SetNoLocalServer(true);
  cancelled = false;

  packageInstaller->SetCallback(this);

  CompleteOptions(true);

  if (options.Task == SetupTask::CleanUp)
  {
    return;
  }

  // initialize installer
  if (options.Task == SetupTask::InstallFromCD)
  {
    packageInstaller->SetRepository(options.MiKTeXDirectRoot.GetData());
  }
  else if (options.Task == SetupTask::Download)
  {
    packageInstaller->SetRepository(options.RemotePackageRepository);
    packageInstaller->SetDownloadDirectory(options.LocalPackageRepository);
  }
  else if (options.Task == SetupTask::InstallFromLocalRepository)
  {
    packageInstaller->SetRepository(options.LocalPackageRepository.GetData());
    // remember local repository folder
    if (!options.IsPrefabricated)
    {
      packageManager->SetLocalPackageRepository(options.LocalPackageRepository);
    }
  }
  packageInstaller->SetPackageLevel(options.PackageLevel);
}

void SetupServiceImpl::DoTheDownload()
{
  ReportLine("starting downloader...");

  shared_ptr<Session> session = Session::Get();

  // remember local repository folder
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_LOCAL_REPOSITORY, options.LocalPackageRepository.ToString());

  // create the local repository directory
  Directory::Create(options.LocalPackageRepository);

  // start downloader in the background
  packageInstaller->DownloadAsync();

  // wait for downloader thread
  packageInstaller->WaitForCompletion();

  if (cancelled)
  {
    return;
  }

  // copy the license file
  PathName licenseFile;
  if (FindFile(LICENSE_FILE, licenseFile))
  {
    PathName licenseFileDest(options.LocalPackageRepository, LICENSE_FILE);
    if (ComparePaths(licenseFile.GetData(), licenseFileDest.GetData(), true) != 0)
    {
      File::Copy(licenseFile, licenseFileDest);
    }
  }

  // now copy the setup program
#if MIKTEX_WINDOWS
  wchar_t szSetupPath[BufferSizes::MaxPath];
  if (GetModuleFileNameW(nullptr, szSetupPath, BufferSizes::MaxPath) == 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetModuleFileNameW");
  }
  PathName pathDest(options.LocalPackageRepository, PathName(szSetupPath).GetFileName());
  if (ComparePaths(WU_(szSetupPath), pathDest, true) != 0)
  {
    File::Copy(WU_(szSetupPath), pathDest);
  }
#else
  // TODO: copy setup program
#endif

  // create info file
  CreateInfoFile();
}

void SetupServiceImpl::DoPrepareMiKTeXDirect()
{
  PathName installRoot(options.MiKTeXDirectRoot);
  installRoot /= "texmf";
  if (options.IsCommonSetup)
  {
    options.Config.commonInstallRoot = installRoot;
  }
  else
  {
    options.Config.userInstallRoot = installRoot;
  }

  // open the uninstall script
  ULogOpen();
#if 0
  // FIXME
  ULogAddFile(g_strLogFile);
#endif

  // run IniTeXMF
  ConfigureMiKTeX();

  // create shell links
  if (!options.IsPortable)
  {
#if defined(MIKTEX_WINDOWS)
    CreateProgramIcons();
#endif
  }

  // register path
  if (!options.IsPortable && options.IsRegisterPathEnabled)
  {
#if defined(MIKTEX_WINDOWS)
    Utils::CheckPath(true);
#else
    // TODO: check path
    UNIMPLEMENTED();
#endif
  }
}

void SetupServiceImpl::DoTheInstallation()
{
  ReportLine("starting installer...");

  // register installation directory
  StartupConfig startupConfig;
  if (options.IsPortable)
  {
    startupConfig.commonInstallRoot = options.PortableRoot / MIKTEX_PORTABLE_REL_INSTALL_DIR;
    startupConfig.userInstallRoot = startupConfig.commonInstallRoot;
  }
  else if (options.IsCommonSetup)
  {
    startupConfig.commonInstallRoot = options.Config.commonInstallRoot;
    // FIXME
    startupConfig.userInstallRoot.SetToTempDirectory();
    startupConfig.userInstallRoot /= "miktexsetup";
  }
  else
  {
    startupConfig.userInstallRoot = options.Config.userInstallRoot;
  }
  shared_ptr<Session> session = Session::Get();
#if defined(MIKTEX_WINDOWS)
  session->RegisterRootDirectories(startupConfig, { RegisterRootDirectoriesOption::Temporary, RegisterRootDirectoriesOption::NoRegistry });
#else
  session->RegisterRootDirectories(startupConfig, { RegisterRootDirectoriesOption::Temporary });
#endif
  
  // parse package definition files
  PathName pathDB;
  if (options.Task == SetupTask::InstallFromCD)
  {
    pathDB = options.MiKTeXDirectRoot / "texmf" / MIKTEX_PATH_PACKAGE_DEFINITION_DIR;
  }
  else
  {
    pathDB = options.LocalPackageRepository / MIKTEX_MPM_DB_FULL_FILE_NAME;
  }
  ReportLine(T_("Loading package database..."));
  packageManager->LoadDatabase(pathDB);

  // create the destination directory
  Directory::Create(GetInstallRoot());

  // open the uninstall script
  ULogOpen();
#if 0
  // FIXME
  ULogAddFile(g_strLogFile);
#endif

  // run installer
  packageInstaller->InstallRemove();

  if (cancelled)
  {
    return;
  }

  // install package definition files
  packageManager->UnloadDatabase();
  packageInstaller->UpdateDb();

  if (cancelled)
  {
    return;
  }

  // run IniTeXMF
  ConfigureMiKTeX();

  if (cancelled)
  {
    return;
  }

  // remove obsolete files
#if 0
  RemoveObsoleteFiles();
#endif

  if (cancelled)
  {
    return;
  }

  // create shell links
  if (!options.IsPortable)
  {
#if defined(MIKTEX_WINDOWS)
    CreateProgramIcons();
#endif
  }

  if (cancelled)
  {
    return;
  }

  // register path
  if (!options.IsPortable && options.IsRegisterPathEnabled)
  {
#if defined(MIKTEX_WINDOWS)
    Utils::CheckPath(true);
#else
    // TODO: check path
    UNIMPLEMENTED();
#endif
  }

  if (options.IsPortable)
  {
    PathName console (MIKTEX_PORTABLE_REL_INSTALL_DIR);
    console /= MIKTEX_PATH_BIN_DIR;
    console /= MIKTEX_CONSOLE_EXE;
    PathName cmdScriptFileName(options.PortableRoot);
    cmdScriptFileName /= "miktex-portable.cmd";
    StreamWriter cmdScript(cmdScriptFileName);
    cmdScript.WriteLine("@echo off");
    cmdScript.WriteLine(fmt::format("start \"\" \"%~d0%~p0{}\" --hide --mkmaps", console.ToDos()));
    cmdScript.Close();
  }
}

void SetupServiceImpl::DoFinishSetup()
{
  ReportLine("finishing setup...");

  // run IniTeXMF
  ConfigureMiKTeX();

  if (cancelled)
  {
    return;
  }

  // register path
  if (options.IsRegisterPathEnabled)
  {
    Utils::CheckPath(true);
  }
}

void SetupServiceImpl::DoFinishUpdate()
{
  ReportLine("finishing update...");
  RemoveFormatFiles();
#if defined(MIKTEX_WINDOWS)
  RunMpm({ "--register-components" });
#endif
  RunIniTeXMF({ "--update-fndb" }, false);
  RunIniTeXMF({ "--force", "--mklinks" }, false);
  RunIniTeXMF({ "--mkmaps", "--mklangs" }, false);
  if (!options.IsPortable)
  {
#if defined(MIKTEX_WINDOWS)
    RunIniTeXMF({ "--register-shell-file-types" }, false);
    CreateProgramIcons();
    RegisterUninstaller();
#endif
  }
}

void SetupServiceImpl::DoCleanUp()
{
  shared_ptr<Session> session = Session::Get();

  if (options.CleanupOptions[CleanupOption::Links])
  {
    ReportLine("removing links...");
    try
    {
      RunIniTeXMF({ "--force", "--remove-links" }, false);
    }
    catch (const MiKTeXException& e)
    {
      ReportLine(e.GetErrorMessage());
    }
  }

  if (options.CleanupOptions[CleanupOption::FileTypes])
  {
    ReportLine("unregistering file types...");
    try
    {
      UnregisterShellFileTypes();
    }
    catch (const MiKTeXException& e)
    {
      ReportLine(e.GetErrorMessage());
    }
  }

  if (options.CleanupOptions[CleanupOption::Path])
  {
    try
    {
      if (session->RunningAsAdministrator()
#if defined(MIKTEX_WINDOWS)
        || session->RunningAsPowerUser()
#endif
        )
      {
        ReportLine("cleaning PATH...");
        UnregisterPath(true);
      }
      UnregisterPath(false);
    }
    catch (const MiKTeXException& e)
    {
      ReportLine(e.GetErrorMessage());
    }
  }

  if (options.CleanupOptions[CleanupOption::Components])
  {
    try
    {
      ReportLine("unregistering components...");
      UnregisterComponents();
    }
    catch (const MiKTeXException& e)
    {
      ReportLine(e.GetErrorMessage());
    }
  }

  if (options.CleanupOptions[CleanupOption::StartMenu] && !session->IsMiKTeXDirect())
  {
    try
    {
#if defined(MIKTEX_WINDOWS)
      ReportLine("processing uninstall log file...");
      logFile.SetLogFileName(session->GetSpecialPath(SpecialPath::InstallRoot) / MIKTEX_PATH_UNINST_LOG);
      logFile.Process();
#endif
    }
    catch (const MiKTeXException& e)
    {
      ReportLine(e.GetErrorMessage());
    }
  }

  if (options.CleanupOptions[CleanupOption::RootDirectories])
  {
    ReportLine("removing root directories...");
    try
    {
      PathName parent;
      vector<PathName> roots = GetRoots();
      shared_ptr<Session> session = Session::Get();
      session->UnloadFilenameDatabase();
      for (const PathName& root : roots)
      {
        if (Directory::Exists(root))
        {
          try
          {
            Directory::Delete(root, true);
          }
          catch (const DirectoryNotEmptyException&)
          {
#if defined(MIKTEX_WINDOWS)
            session->ScheduleFileRemoval(root);
#else
            // FIXME
#endif
          }
        }
      }
      if (!session->IsMiKTeXDirect())
      {
        parent = session->GetSpecialPath(SpecialPath::InstallRoot);
        parent.CutOffLastComponent();
        if (Directory::Exists(parent))
        {
          RemoveEmptyDirectoryChain(parent);
        }
      }
      parent = session->GetSpecialPath(SpecialPath::UserDataRoot);
      parent.CutOffLastComponent();
      if (Directory::Exists(parent))
      {
        RemoveEmptyDirectoryChain(parent);
      }
      parent = session->GetSpecialPath(SpecialPath::UserConfigRoot);
      parent.CutOffLastComponent();
      if (Directory::Exists(parent))
      {
        RemoveEmptyDirectoryChain(parent);
      }
      if (session->IsAdminMode())
      {
        parent = session->GetSpecialPath(SpecialPath::CommonDataRoot);
        parent.CutOffLastComponent();
        if (Directory::Exists(parent))
        {
          RemoveEmptyDirectoryChain(parent);
        }
        parent = session->GetSpecialPath(SpecialPath::CommonConfigRoot);
        parent.CutOffLastComponent();
        if (Directory::Exists(parent))
        {
          RemoveEmptyDirectoryChain(parent);
        }
      }
    }
    catch (const MiKTeXException& e)
    {
      ReportLine(e.GetErrorMessage());
    }
  }

  if (options.CleanupOptions[CleanupOption::Registry])
  {
    ReportLine("removing registry settings...");
    try
    {
#if defined(MIKTEX_WINDOWS)
      RemoveRegistryKeys();
#else
      PathName fontConfig(MIKTEX_SYSTEM_ETC_FONTS_CONFD_DIR);
      // FIXME: hard-coded file name
      fontConfig /= "09-miktex.conf";
      if (session->RunningAsAdministrator() && File::Exists(fontConfig))
      {
        File::Delete(fontConfig);
      }
#endif
    }
    catch (const MiKTeXException& e)
    {
      ReportLine(e.GetErrorMessage());
    }
  }

  if (options.CleanupOptions[CleanupOption::LogFiles])
  {
    ReportLine("removing log files...");
    try
    {
      PathName logDir = session->GetSpecialPath(SpecialPath::LogDirectory);
      if (Directory::Exists(logDir))
      {
	Directory::Delete(logDir, true);
      }
    }
    catch (const MiKTeXException& e)
    {
      ReportLine(e.GetErrorMessage());
    }
  }
}

vector<PathName> SetupServiceImpl::GetRoots()
{
  vector<PathName> vec;
  shared_ptr<Session> session = Session::Get();
  if (!session->IsMiKTeXDirect())
  {
    PathName installRoot = session->GetSpecialPath(SpecialPath::InstallRoot);
    vec.push_back(installRoot);
  }
  PathName userDataRoot = session->GetSpecialPath(SpecialPath::UserDataRoot);
  if (!Contains(vec, userDataRoot))
  {
    vec.push_back(userDataRoot);
  }
  PathName userConfigRoot = session->GetSpecialPath(SpecialPath::UserConfigRoot);
  if (!Contains(vec, userConfigRoot))
  {
    vec.push_back(userConfigRoot);
  }
  if (session->IsAdminMode())
  {
    PathName commonDataRoot = session->GetSpecialPath(SpecialPath::CommonDataRoot);
    if (!Contains(vec, commonDataRoot))
    {
      vec.push_back(commonDataRoot);
    }
    PathName commonConfigRoot = session->GetSpecialPath(SpecialPath::CommonConfigRoot);
    if (!Contains(vec, commonConfigRoot))
    {
      vec.push_back(commonConfigRoot);
    }
  }
  return vec;
}

void SetupServiceImpl::UnregisterComponents()
{
  shared_ptr<Session> session = Session::Get();
  if (session->RunningAsAdministrator()
#if defined(MIKTEX_WINDOWS)
      || session->RunningAsPowerUser()
#endif
      )
  {
    std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager(PackageManager::Create());
    shared_ptr<PackageInstaller> packageInstaller(packageManager->CreateInstaller());
    packageInstaller->RegisterComponents(false);
    packageInstaller->Dispose();
  }
}

void SetupServiceImpl::ConfigureMiKTeX()
{
  if (!callback->OnProgress(MiKTeX::Setup::Notification::ConfigureBegin))
  {
    cancelled = true;
    return;
  }

  vector<string> args;

  if (options.Task != SetupTask::PrepareMiKTeXDirect)
  {
    // define roots & remove old fndbs
    args.clear();
    if (options.IsPortable)
    {
      args.push_back("--portable=" + GetInstallRoot().ToString());
    }
    else
    {
      if (!options.Config.userInstallRoot.Empty())
      {
        args.push_back("--user-install=" + options.Config.userInstallRoot.ToString());
      }
      if (!options.Config.userDataRoot.Empty())
      {
        args.push_back("--user-data=" + options.Config.userDataRoot.ToString());
      }
      if (!options.Config.userConfigRoot.Empty())
      {
        args.push_back("--user-config=" + options.Config.userConfigRoot.ToString());
      }
      if (!options.Config.commonDataRoot.Empty())
      {
        args.push_back("--common-data=" + options.Config.commonDataRoot.ToString());
      }
      if (!options.Config.commonConfigRoot.Empty())
      {
        args.push_back("--common-config=" + options.Config.commonConfigRoot.ToString());
      }
      if (!options.Config.commonInstallRoot.Empty())
      {
        args.push_back("--common-install=" + options.Config.commonInstallRoot.ToString());
      }
#if defined(MIKTEX_WINDOWS)
      if (!options.IsRegistryEnabled)
      {
        args.push_back("--no-registry");
        args.push_back("--create-config-file="s + MIKTEX_PATH_MIKTEX_INI);
        args.push_back("--set-config-value="s + "[" + MIKTEX_REGKEY_CORE + "]" + MIKTEX_REGVAL_NO_REGISTRY + "=1");
      }
#else
      args.push_back("--create-config-file="s + MIKTEX_PATH_MIKTEX_INI);
#endif
      args.push_back("--set-config-value="s + "[" + MIKTEX_REGKEY_CORE + "]" + MIKTEX_REGVAL_SHARED_SETUP + "=" + (options.IsCommonSetup ? "1" : "0"));
    }
    if (!options.Config.commonRoots.empty())
    {
      args.push_back("--common-roots=" + options.Config.commonRoots);
    }
    if (!options.Config.userRoots.empty())
    {
      args.push_back("--user-roots=" + options.Config.userRoots);
    }
    if (!args.empty())
    {
      RunIniTeXMF(args, true);
    }
    if (cancelled)
    {
      return;
    }

    if (options.Task != SetupTask::FinishSetup)
    {
      RunIniTeXMF({ "--rmfndb" }, false);
    }

    // register components, configure files
#if defined(MIKTEX_WINDOWS)
    if (options.Task != SetupTask::FinishSetup)
    {
      RunMpm({ "--register-components" });
    }
#endif

    // create filename database files
    RunIniTeXMF({ "--update-fndb" }, false);
    if (cancelled)
    {
      return;
    }

    // create latex.exe, ...
    RunIniTeXMF({ "--force", "--mklinks" }, false);
    if (cancelled)
    {
      return;
    }

    // create font map files and language.dat
    RunIniTeXMF({ "--mkmaps", "--mklangs" }, false);

    if (cancelled)
    {
      return;
    }
  }

  // set paper size
  if (!options.PaperSize.empty())
  {
    RunIniTeXMF({ "--default-paper-size=" + options.PaperSize }, false);
  }
  
  // set auto-install
  string valueSpec = "["s + MIKTEX_CONFIG_SECTION_MPM + "]";
  valueSpec += MIKTEX_CONFIG_VALUE_AUTOINSTALL;
  valueSpec += "=";
  valueSpec += std::to_string((int)options.IsInstallOnTheFlyEnabled);
  RunIniTeXMF({ "--set-config-value=" + valueSpec }, false);

  if (options.Task != SetupTask::PrepareMiKTeXDirect)
  {
    // refresh file name database again
    RunIniTeXMF({ "--update-fndb" }, false);
    if (cancelled)
    {
      return;
    }
  }

  if (!options.IsPortable)
  {
#if defined(MIKTEX_WINDOWS)
    RunIniTeXMF({ "--register-shell-file-types" }, false);
#endif
  }

  if (!options.IsPortable && options.IsRegisterPathEnabled)
  {
    RunIniTeXMF({ "--modify-path" }, false);
  }

  // create report
  RunIniTeXMF({ "--report" }, false);
  if (cancelled)
  {
    return;
  }
}

PathName SetupServiceImpl::GetInstallRoot() const
{
  if (options.IsPortable)
  {
    return options.PortableRoot / MIKTEX_PORTABLE_REL_INSTALL_DIR;
  }
  else if (options.Task == SetupTask::FinishSetup || options.Task == SetupTask::FinishUpdate || options.Task == SetupTask::CleanUp)
  {
    shared_ptr<Session> session = Session::Get();
    return session->GetSpecialPath(SpecialPath::InstallRoot);
  }
  else
  {
    return options.IsCommonSetup ? options.Config.commonInstallRoot : options.Config.userInstallRoot;
  }
}

PathName SetupServiceImpl::GetBinDir() const
{
  if (options.Task == SetupTask::FinishSetup || options.Task == SetupTask::FinishUpdate || options.Task == SetupTask::CleanUp)
  {
    shared_ptr<Session> session = Session::Get();
    return session->GetSpecialPath(SpecialPath::BinDirectory);
  }
  else
  {
    return GetInstallRoot() / MIKTEX_PATH_BIN_DIR;
  }
}

void SetupServiceImpl::RunIniTeXMF(const vector<string>& args, bool mustSucceed)
{
  shared_ptr<Session> session = Session::Get();

  // make absolute exe path name
  PathName exePath = GetBinDir() / MIKTEX_INITEXMF_EXE;

  // make command line
  vector<string> allArgs{ exePath.GetFileNameWithoutExtension().ToString() };
  allArgs.insert(allArgs.end(), args.begin(), args.end());
  if (options.Task == SetupTask::FinishSetup)
  {
    allArgs.push_back("--principal=setup");
  }
  if (options.IsCommonSetup)
  {
    allArgs.push_back("--admin");
  }
  if (options.Task != SetupTask::FinishSetup && options.Task != SetupTask::FinishUpdate && options.Task != SetupTask::CleanUp)
  {
    allArgs.push_back("--log-file=" + GetULogFileName().ToString());
  }
  allArgs.push_back("--disable-installer");
  allArgs.push_back("--verbose");

  // run initexmf.exe
  if (!options.IsDryRun)
  {
    Log(fmt::format("{}:\n", CommandLineBuilder(allArgs).ToString()));
    ULogClose(false);
    session->UnloadFilenameDatabase();
    int exitCode;
    MiKTeXException miktexException;
    if (!Process::Run(exePath, allArgs, this, &exitCode, &miktexException, nullptr) || exitCode != 0)
    {
      if (mustSucceed)
      {
        throw miktexException;
      }
      else
      {
        Warning(miktexException);
      }
    }
    ULogOpen();
  }
}

void SetupServiceImpl::RunMpm(const vector<string>& args)
{
  shared_ptr<Session> session = Session::Get();
  // make absolute exe path name
  PathName exePath = GetBinDir() / MIKTEX_MPM_EXE;

  // make command line
  vector<string> allArgs{ exePath.GetFileNameWithoutExtension().ToString() };
  allArgs.insert(allArgs.end(), args.begin(), args.end());
  if (options.IsCommonSetup)
  {
    allArgs.push_back("--admin");
  }
  allArgs.push_back("--verbose");

  // run mpm.exe
  if (!options.IsDryRun)
  {
    Log(fmt::format("{}:\n", CommandLineBuilder(allArgs).ToString()));
    ULogClose(false);
    session->UnloadFilenameDatabase();
    Process::Run(exePath, allArgs, this);
    ULogOpen();
  }
}

void SetupServiceImpl::CreateInfoFile()
{
  StreamWriter stream(PathName(options.LocalPackageRepository, DOWNLOAD_INFO_FILE));
  const char* lpszPackageSet;
  switch (options.PackageLevel)
  {
  case PackageLevel::Essential:
    lpszPackageSet = ESSENTIAL_MIKTEX;
    break;
  case PackageLevel::Basic:
    lpszPackageSet = BASIC_MIKTEX;
    break;
  case PackageLevel::Complete:
    lpszPackageSet = COMPLETE_MIKTEX;
    break;
  default:
    MIKTEX_ASSERT(false);
  }
  stream.WriteLine(fmt::format(T_("This folder contains the {0} package set."), lpszPackageSet));
  stream.WriteLine();
  stream.WriteLine();
#if defined(MIKTEX_WINDOWS)
  wchar_t szSetupPath[BufferSizes::MaxPath];
  if (GetModuleFileNameW(0, szSetupPath, BufferSizes::MaxPath) == 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetModuleFileNameW");
  }
  PathName setupExe(szSetupPath);
  setupExe.RemoveDirectorySpec();
  stream.WriteLine(fmt::format(T_("To install MiKTeX, run {0}."), setupExe));
  stream.WriteLine();
  stream.WriteLine();
#endif
  stream.WriteLine(T_("For more information, visit the MiKTeX project page at\n\nhttps://miktex.org."));
  stream.WriteLine();
  stream.Close();
  RepositoryInfo repositoryInfo;
  if (packageManager->TryGetRepositoryInfo(options.RemotePackageRepository, repositoryInfo))
  {
    StreamWriter stream(PathName(options.LocalPackageRepository, "pr.ini"));
    stream.WriteLine("[repository]");
    stream.WriteLine(fmt::format("date={}", repositoryInfo.timeDate));
    stream.WriteLine(fmt::format("version={}", repositoryInfo.version));
    stream.Close();
  }
}

SetupService::ProgressInfo SetupServiceImpl::GetProgressInfo()
{
  ProgressInfo progressInfo;
  if (options.Task == SetupTask::CleanUp)
  {
    LogFile::ProgressInfo pi = logFile.GetProgressInfo();
    progressInfo.fileName = pi.fileName;
    progressInfo.cFilesRemoveTotal = pi.total;
    progressInfo.cFilesRemoveCompleted = pi.completed;
  }
  else
  {
    PackageInstaller::ProgressInfo pi = packageInstaller->GetProgressInfo();
    progressInfo.deploymentName = pi.deploymentName;
    progressInfo.displayName = pi.displayName;
    progressInfo.fileName = pi.fileName;
    progressInfo.cFilesRemoveCompleted = pi.cFilesRemoveCompleted;
    progressInfo.cFilesRemoveTotal = pi.cFilesRemoveTotal;
    progressInfo.cPackagesRemoveCompleted = pi.cPackagesRemoveCompleted;
    progressInfo.cPackagesRemoveTotal = pi.cPackagesRemoveTotal;
    progressInfo.cbPackageDownloadCompleted = pi.cbPackageDownloadCompleted;
    progressInfo.cbPackageDownloadTotal = pi.cbPackageDownloadTotal;
    progressInfo.cbDownloadCompleted = pi.cbDownloadCompleted;
    progressInfo.cbDownloadTotal = pi.cbDownloadTotal;
    progressInfo.cFilesPackageInstallCompleted = pi.cFilesPackageInstallCompleted;
    progressInfo.cFilesPackageInstallTotal = pi.cFilesPackageInstallTotal;
    progressInfo.cFilesInstallCompleted = pi.cFilesInstallCompleted;
    progressInfo.cFilesInstallTotal = pi.cFilesInstallTotal;
    progressInfo.cPackagesInstallCompleted = pi.cPackagesInstallCompleted;
    progressInfo.cPackagesInstallTotal = pi.cPackagesInstallTotal;
    progressInfo.cbPackageInstallCompleted = pi.cbPackageInstallCompleted;
    progressInfo.cbPackageInstallTotal = pi.cbPackageInstallTotal;
    progressInfo.cbInstallCompleted = pi.cbInstallCompleted;
    progressInfo.cbInstallTotal = pi.cbInstallTotal;
    progressInfo.bytesPerSecond = pi.bytesPerSecond;
    progressInfo.timeRemaining = pi.timeRemaining;
    progressInfo.ready = pi.ready;
    progressInfo.numErrors = pi.numErrors;
    progressInfo.cancelled = pi.cancelled;
  }
  return progressInfo;
}

bool SetupServiceImpl::OnProcessOutput(const void* pOutput, size_t n)
{
  if (!callback->OnProcessOutput(pOutput, n))
  {
    cancelled = true;
    return false;
  }
  return true;
}

void SetupServiceImpl::ReportLine(const string& str)
{
  callback->ReportLine(str);
}

bool SetupServiceImpl::OnRetryableError(const string& message)
{
  if (!callback->OnRetryableError(message))
  {
    cancelled = true;
    return false;
  }
  return true;
}

bool SetupServiceImpl::OnProgress(MiKTeX::Packages::Notification nf)
{
  MiKTeX::Setup::Notification setupNotification(Setup::Notification::None);
  switch (nf)
  {
  case MiKTeX::Packages::Notification::DownloadPackageStart:
    setupNotification = MiKTeX::Setup::Notification::DownloadPackageStart; break;
  case MiKTeX::Packages::Notification::DownloadPackageEnd:
    setupNotification = MiKTeX::Setup::Notification::DownloadPackageEnd; break;
  case MiKTeX::Packages::Notification::InstallFileStart:
    setupNotification = MiKTeX::Setup::Notification::InstallFileStart; break;
  case MiKTeX::Packages::Notification::InstallFileEnd:
    setupNotification = MiKTeX::Setup::Notification::InstallFileEnd; break;
  case MiKTeX::Packages::Notification::InstallPackageStart:
    setupNotification = MiKTeX::Setup::Notification::InstallPackageStart; break;
  case MiKTeX::Packages::Notification::InstallPackageEnd:
    setupNotification = MiKTeX::Setup::Notification::InstallPackageEnd; break;
  case MiKTeX::Packages::Notification::RemoveFileStart:
    setupNotification = MiKTeX::Setup::Notification::RemoveFileStart; break;
  case MiKTeX::Packages::Notification::RemoveFileEnd:
    setupNotification = MiKTeX::Setup::Notification::RemoveFileEnd; break;
  case MiKTeX::Packages::Notification::RemovePackageStart:
    setupNotification = MiKTeX::Setup::Notification::RemovePackageStart; break;
  case MiKTeX::Packages::Notification::RemovePackageEnd:
    setupNotification = MiKTeX::Setup::Notification::RemovePackageEnd; break;
  default:
    break;
  }
  if (!callback->OnProgress(setupNotification))
  {
    cancelled = true;
    return false;
  }
  return true;
}

wstring& SetupServiceImpl::Expand(const char* source, wstring& dest)
{
  dest = StringUtil::UTF8ToWideChar(source);
  wstring::size_type pos;
  while ((pos = dest.find(L"%MIKTEX_INSTALL%")) != wstring::npos)
  {
    dest = dest.replace(pos, 16, GetInstallRoot().ToWideCharString());
  }
  return dest;
}

bool SetupServiceImpl::FindFile(const PathName& fileName, PathName& result)
{
  shared_ptr<Session> session = Session::Get();

  // try my directory
  result = session->GetMyLocation(false) / fileName;
  if (File::Exists(result))
  {
    return true;
  }

  // try the current directory
  result.SetToCurrentDirectory() / fileName;
  if (File::Exists(result))
  {
    return true;
  }

  return false;
}

void SetupServiceImpl::RemoveFormatFiles()
{
  shared_ptr<Session> session = Session::Get();
  vector<PathName> toBeDeleted;
  PathName pathFmt(session->GetSpecialPath(SpecialPath::DataRoot));
  pathFmt /= MIKTEX_PATH_FMT_DIR;
  if (Directory::Exists(pathFmt))
  {
    CollectFiles(toBeDeleted, pathFmt, MIKTEX_FORMAT_FILE_SUFFIX);
  }
  PathName pathFmt2(session->GetSpecialPath(SpecialPath::UserDataRoot));
  pathFmt2 /= MIKTEX_PATH_FMT_DIR;
  if (pathFmt != pathFmt2 && Directory::Exists(pathFmt2))
  {
    CollectFiles(toBeDeleted, pathFmt2, MIKTEX_FORMAT_FILE_SUFFIX);
  }
  for (const PathName& f : toBeDeleted)
  {
    File::Delete(f);
  }
}

void SetupServiceImpl::CollectFiles(vector<PathName>& vec, const PathName& dir, const char* lpszExt)
{
  unique_ptr<DirectoryLister> lister = DirectoryLister::Open(dir);
  DirectoryEntry entry;
  vector<string> subDirs;
  while (lister->GetNext(entry))
  {
    if (entry.isDirectory)
    {
      subDirs.push_back(entry.name);
    }
    else
    {
      PathName path(dir, entry.name);
      if (path.HasExtension(lpszExt))
      {
        vec.push_back(path);
      }
    }
  }
  lister->Close();
  for (const string& s : subDirs)
  {
    // RECURSION
    CollectFiles(vec, PathName(dir, s), lpszExt);
  }
}

void SetupServiceImpl::Warning(const MiKTeX::Core::MiKTeXException& ex)
{
  string message = ex.GetErrorMessage();
  string description = ex.GetDescription();
  Log(fmt::format("Warning: {}\n", message));
  if (!description.empty())
  {
    Log(fmt::format("Warning: {}\n", description));
    ReportLine("Warning: " + description);
  }
  else
  {
    ReportLine("Warning: " + message);
  }
}

void SetupService::WriteReport(ostream& s, ReportOptionSet options)
{
  shared_ptr<Session> session = Session::Get();
  if (options[ReportOption::General])
  {
    s << "MiKTeX: " << Utils::GetMiKTeXVersionString() << "\n"
      << "OS: " << Utils::GetOSVersionString() << "\n"
      << "SharedSetup: " << (session->IsSharedSetup() ? T_("yes") : T_("no")) << "\n"
      << "PathOkay: " << (Utils::CheckPath() ? T_("yes") : T_("no")) << "\n";
  }
  if (options[ReportOption::CurrentUser])
  {
    s << "SystemAdmin: " << (session->RunningAsAdministrator() ? T_("yes") : T_("no")) << "\n"
      << "RootPrivileges: " << (session->RunningAsAdministrator() ? T_("yes") : T_("no")) << "\n";
  }
  if (options[ReportOption::RootDirectories])
  {
    for (unsigned idx = 0; idx < session->GetNumberOfTEXMFRoots(); ++idx)
    {
      s << "Root" << idx << ": " << session->GetRootDirectoryPath(idx) << "\n";
    }
    s << "UserInstall: " << session->GetSpecialPath(SpecialPath::UserInstallRoot) << "\n"
      << "UserConfig: " << session->GetSpecialPath(SpecialPath::UserConfigRoot) << "\n"
      << "UserData: " << session->GetSpecialPath(SpecialPath::UserDataRoot) << "\n"
      << "CommonInstall: " << session->GetSpecialPath(SpecialPath::CommonInstallRoot) << "\n"
      << "CommonConfig: " << session->GetSpecialPath(SpecialPath::CommonConfigRoot) << "\n"
      << "CommonData: " << session->GetSpecialPath(SpecialPath::CommonDataRoot) << "\n";
  }
  if (options[ReportOption::Processes])
  {
    s << "Invokers: " << StringUtil::Flatten(Process::GetInvokerNames(), '/') << "\n";
  }
  if (options[ReportOption::Environment])
  {
    string env;
    if (Utils::GetEnvironmentString("PATH", env))
    {
      int idx = 0;
      for (const string& p : StringUtil::Split(env, PathName::PathNameDelimiter))
      {
        s << "PATH" << idx++ << p << "\n";
      }
    }
  }
  if (options[ReportOption::BrokenPackages])
  {
    shared_ptr<PackageManager> packageManager = PackageManager::Create();
    vector<string> broken;
    unique_ptr<PackageIterator> pkgIter(packageManager->CreateIterator());
    PackageInfo packageInfo;
    for (int idx = 0; pkgIter->GetNext(packageInfo); ++idx)
    {
      if (!packageInfo.IsPureContainer()
        && packageInfo.IsInstalled()
        && packageInfo.deploymentName.compare(0, 7, "miktex-") == 0)
      {
        if (!(packageManager->TryVerifyInstalledPackage(packageInfo.deploymentName)))
        {
          broken.push_back(packageInfo.deploymentName);
        }
      }
    }
    pkgIter->Dispose();
    if (!broken.empty())
    {
      for (const string& name : broken)
      {
        s << name << ": " << T_("needs to be reinstalled") << "\n";
      }
    }
  }
}

void SetupService::WriteReport(ostream& s)
{
  WriteReport(s, { ReportOption::General, ReportOption::RootDirectories, ReportOption::CurrentUser });
}
