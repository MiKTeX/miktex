/* SetupService.cpp:

   Copyright (C) 2013-2016 Christian Schenk

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

#define LICENSE_FILE "LICENSE.TXT"
#define DOWNLOAD_INFO_FILE "README.TXT"

#define BASIC_MIKTEX "\"Basic MiKTeX\""
#define BASIC_MIKTEX_LEGACY "\"Small MiKTeX\""
#define COMPLETE_MIKTEX "\"Complete MiKTeX\""
#define COMPLETE_MIKTEX_LEGACY "\"Total MiKTeX\""
#define ESSENTIAL_MIKTEX "\"Essential MiKTeX\""

SETUPSTATICFUNC(int) ComparePaths(const PathName & path1, const PathName & path2, bool shortify)
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
#else
  return PathName::Compare(path1, path2);
#endif
}

BEGIN_INTERNAL_NAMESPACE;

void RemoveEmptyDirectoryChain(const PathName & directory)
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

bool Contains(const vector<PathName> & vec, const PathName & pathName)
{
  for (const PathName & p : vec)
  {
    if (p == pathName)
    {
      return true;
    }
  }
  return false;
}

END_INTERNAL_NAMESPACE;

SetupService::~SetupService()
{
}

SetupServiceImpl::SetupServiceImpl()
{
  traceStream = TraceStream::Open("setup");
  TraceStream::SetTraceFlags("error,extractor,mpm,process,config,setup");
  pManager = PackageManager::Create();
  shared_ptr<Session> session = Session::Get();
  PathName logFileName = session->GetSpecialPath(SpecialPath::InstallRoot);
  logFileName /= MIKTEX_PATH_UNINST_LOG;
  logFile.SetCallback(this);
  logFile.SetLogFileName(logFileName);
  options.IsCommonSetup = session->RunningAsAdministrator();
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
  MIKTEX_UNEXPECTED();
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

PackageLevel SetupService::SearchLocalRepository(PathName & localRepository, PackageLevel requestedPackageLevel, bool & prefabricated)
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
  localRepository = session->GetMyLocation();
  packageLevel_ = SetupService::TestLocalRepository(localRepository, requestedPackageLevel);
  if (packageLevel_ != PackageLevel::None)
  {
    prefabricated = true;
    return packageLevel_;
  }

  // try ..\tm\packages
  localRepository = session->GetMyLocation();
  localRepository /= "..";
  localRepository /= "tm";
  localRepository /= "packages";
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

PackageLevel SetupService::TestLocalRepository(const PathName & pathRepository, PackageLevel requestedPackageLevel)
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

bool SetupService::IsMiKTeXDirect(PathName & root)
{
  // check ..\texmf\miktex\config\miktexstartup.ini
  shared_ptr<Session> session = Session::Get();
  root = session->GetMyLocation();
  root /= "..";
  root.MakeAbsolute();
  PathName pathStartupConfig = root;
  pathStartupConfig /= "texmf";
  pathStartupConfig /= MIKTEX_PATH_STARTUP_CONFIG_FILE;
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

SetupOptions SetupServiceImpl::SetOptions(const SetupOptions & options)
{
  this->options = options;
  CompleteOptions(false);
  return this->options;
}

void SetupServiceImpl::OpenLog()
{
  if (logStream.IsOpen())
  {
    return;
  }

  // make the intermediate log file name
  intermediateLogFile.SetToTempFile();

  // open the intermediate log file
  lock_guard<mutex> lockGuard(logStreamMutex);
  logStream.Attach(File::Open(intermediateLogFile, FileMode::Create, FileAccess::Write));
  logStream.WriteLine();
  logStream.WriteLine();
}

PathName SetupServiceImpl::CloseLog(bool cancel)
{
  // we must have an intermediate log file
  if (!logStream.IsOpen())
  {
    return "";
  }

  // close the intermediate log file
  logStream.Close();

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
        pathLogDir.Set(GetInstallRoot(), MIKTEX_PATH_MIKTEX_CONFIG_DIR);
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
  time_t t = time(0);
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
  pathLogFile.SetExtension(".log");

  // install the log file
  // <todo>add the log file to the uninstall script</todo>
  File::Copy(intermediateLogFile, pathLogFile);

  // remove the intermediate log file
  File::Delete(intermediateLogFile);

  return pathLogFile;
}

void SetupServiceImpl::LogHeader()
{
  Log(T_("%s %s Report\n\n"), options.Banner.c_str(), options.Version.c_str());
  time_t t = time(0);
  struct tm * pTm = localtime(&t);
  char dateString[128];
  strftime(dateString, 128, "%A, %B %d, %Y", pTm);
  char timeString[128];
  strftime(timeString, 128, "%H:%M:%S", pTm);
  Log(T_("Date: %s\n"), dateString);
  Log(T_("Time: %s\n"), timeString);
  Log(T_("OS version: %s\n"), Utils::GetOSVersionString().c_str());
  shared_ptr<Session> session = Session::Get();
#if defined(MIKTEX_WINDOWS)
  Log("SystemAdmin: %s\n", (session->RunningAsAdministrator() ? "yes" : "false"));
  Log("PowerUser: %s\n", (session->RunningAsPowerUser() ? "yes" : "false"));
#endif
  if (options.Task != SetupTask::Download)
  {
    Log("SharedSetup: %s\n", (options.IsCommonSetup ? "yes" : "false"));
  }
#if MIKTEX_WINDOWS
  wchar_t szSetupPath[BufferSizes::MaxPath];
  if (GetModuleFileNameW(0, szSetupPath, BufferSizes::MaxPath) == 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetModuleFileNameW");
  }
  Log(T_("Setup path: %s\n"), WU_(szSetupPath));
#else
  // TODO: log setup path
#endif
  if (options.Task != SetupTask::Download)
  {
    Log("UserRoots: %s\n", (options.Config.userRoots.empty() ? T_("<none specified>") : options.Config.userRoots.c_str()));
    Log("UserData: %s\n", (options.Config.userDataRoot.Empty() ? T_("<none specified>") : options.Config.userDataRoot.Get()));
    Log("UserConfig: %s\n", (options.Config.userConfigRoot.Empty() ? T_("<none specified>") : options.Config.userConfigRoot.Get()));
    Log("CommonRoots: %s\n", (options.Config.commonRoots.empty() ? T_("<none specified>") : options.Config.commonRoots.c_str()));
    Log("CommonData: %s\n", (options.Config.commonDataRoot.Empty() ? T_("<none specified>") : options.Config.commonDataRoot.Get()));
    Log("CommonConfig: %s\n", (options.Config.commonConfigRoot.Empty() ? T_("<none specified>") : options.Config.commonConfigRoot.Get()));
    PathName installRoot = GetInstallRoot();
    Log("Installation: %s\n", installRoot.Empty() ? T_("<none specified>") : installRoot.Get());
  }
}

void SetupServiceImpl::Log(const char * lpszFormat, ...)
{
  va_list argList;
  va_start(argList, lpszFormat);
  LogV(lpszFormat, argList);
  va_end(argList);
}

void SetupServiceImpl::LogV(const char * lpszFormat, va_list argList)
{
#if 0
  lock_guard<mutex> lockGuard(logStreamMutex);
#endif
  if (!logging)
  {
    logging = true;
    LogHeader();
  }
  string formatted = StringUtil::FormatString(lpszFormat, argList);
  static string currentLine;
  for (const char * lpsz = formatted.c_str(); *lpsz != 0; ++lpsz)
  {
    if (lpsz[0] == '\n' || (lpsz[0] == '\r' && lpsz[1] == '\n'))
    {
      traceStream->WriteFormattedLine("setup", "%s", currentLine.c_str());
      if (logStream.IsOpen())
      {
        logStream.WriteLine(currentLine);
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
  PathName uninstLog(GetULogFileName());
  FileMode mode = (File::Exists(uninstLog) ? FileMode::Append : FileMode::Create);
  uninstStream.Attach(File::Open(uninstLog, mode, FileAccess::Write));;
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
    ret = GetInstallRoot();
    ret /= MIKTEX_PATH_MIKTEX_CONFIG_DIR;
  }
  Directory::Create(ret);
  ret /= MIKTEX_UNINSTALL_LOG;
  return ret;
}

void SetupServiceImpl::ULogClose(bool finalize)
{
  if (!uninstStream.IsOpen())
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
    uninstStream.Close();
    throw;
  }

  uninstStream.Close();
}

void SetupServiceImpl::ULogAddFile(const PathName & path)
{
  if (!uninstStream.IsOpen())
  {
    return;
  }
  if (section != Files)
  {
    uninstStream.WriteLine("[files]");
    section = Files;
  }
  PathName absolutePath(path);
  absolutePath.MakeAbsolute();
#if defined(MIKTEX_WINDOWS)
  absolutePath.ToDos();
#endif
  uninstStream.WriteLine(absolutePath.Get());
}

void SetupServiceImpl::SetCallback(SetupServiceCallback * pCallback)
{
  this->pCallback = pCallback;
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
  case SetupTask::Uninstall:
    DoTheUninstallation();
    break;
  default:
    MIKTEX_UNEXPECTED();
  }
  ULogClose(true);
}

void SetupServiceImpl::CompleteOptions(bool allowRemoteCalls)
{
  shared_ptr<Session> session = Session::Get();

  if (options.Task == SetupTask::Uninstall)
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
      options.Config.userInstallRoot = "";
    }
    else
    {
      if (options.Config.userInstallRoot.Empty())
      {
        options.Config.userInstallRoot = GetDefaultUserInstallDir();
      }
      options.Config.commonInstallRoot = "";
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
    if (!pManager->TryGetRemotePackageRepository(options.RemotePackageRepository) && allowRemoteCalls)
    {
      options.RemotePackageRepository = pManager->PickRepositoryUrl();
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

  pInstaller = pManager->CreateInstaller();
  pInstaller->SetNoPostProcessing(true);
  pInstaller->SetNoLocalServer(true);
  cancelled = false;

  pInstaller->SetCallback(this);

  CompleteOptions(true);

  if (options.Task == SetupTask::Uninstall)
  {
    return;
  }

  // initialize installer
  if (options.Task == SetupTask::InstallFromCD)
  {
    pInstaller->SetRepository(options.MiKTeXDirectRoot.Get());
  }
  else if (options.Task == SetupTask::Download)
  {
    pInstaller->SetRepository(options.RemotePackageRepository);
    pInstaller->SetDownloadDirectory(options.LocalPackageRepository);
  }
  else if (options.Task == SetupTask::InstallFromLocalRepository)
  {
    pInstaller->SetRepository(options.LocalPackageRepository.Get());
    // remember local repository folder
    if (!options.IsPrefabricated)
    {
      pManager->SetLocalPackageRepository(options.LocalPackageRepository);
    }
  }
  pInstaller->SetPackageLevel(options.PackageLevel);
}

void SetupServiceImpl::DoTheDownload()
{
  ReportLine("starting downloader...");

  shared_ptr<Session> session = Session::Get();

  // remember local repository folder
  session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_LOCAL_REPOSITORY, options.LocalPackageRepository.Get());

  // create the local repository directory
  Directory::Create(options.LocalPackageRepository);

  // start downloader in the background
  pInstaller->DownloadAsync();

  // wait for downloader thread
  pInstaller->WaitForCompletion();

  if (cancelled)
  {
    return;
  }

  // copy the license file
  PathName licenseFile;
  if (FindFile(LICENSE_FILE, licenseFile))
  {
    PathName licenseFileDest(options.LocalPackageRepository, LICENSE_FILE);
    if (ComparePaths(licenseFile.Get(), licenseFileDest.Get(), true) != 0)
    {
      File::Copy(licenseFile, licenseFileDest);
    }
  }

  // now copy the setup program
#if MIKTEX_WINDOWS
  wchar_t szSetupPath[BufferSizes::MaxPath];
  if (GetModuleFileNameW(0, szSetupPath, BufferSizes::MaxPath) == 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetModuleFileNameW");
  }
  char szFileName[BufferSizes::MaxPath];
  char szExt[BufferSizes::MaxPath];
  PathName::Split(
    WU_(szSetupPath), 0, 0, 0, 0, szFileName, BufferSizes::MaxPath, szExt, BufferSizes::MaxPath);
  PathName pathDest(options.LocalPackageRepository, szFileName, szExt);
  if (ComparePaths(WU_(szSetupPath), pathDest.Get(), true) != 0)
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
    startupConfig.commonInstallRoot = options.PortableRoot;
    startupConfig.commonInstallRoot /= MIKTEX_PORTABLE_REL_INSTALL_DIR;
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
    pathDB = options.MiKTeXDirectRoot;
    pathDB /= "texmf";
    pathDB /= MIKTEX_PATH_PACKAGE_DEFINITION_DIR;
  }
  else
  {
    pathDB = options.LocalPackageRepository;
    pathDB /= MIKTEX_MPM_DB_FULL_FILE_NAME;
  }
  ReportLine(T_("Loading package database..."));
  pManager->LoadDatabase(pathDB);

  // create the destination directory
  Directory::Create(GetInstallRoot());

  // open the uninstall script
  ULogOpen();
#if 0
  // FIXME
  ULogAddFile(g_strLogFile);
#endif

  // run installer
  pInstaller->InstallRemove();

  if (cancelled)
  {
    return;
  }

  // install package definition files
  pManager->UnloadDatabase();
  pInstaller->UpdateDb();

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
    PathName taskbarIcon(options.PortableRoot);
    taskbarIcon /= MIKTEX_PORTABLE_REL_INSTALL_DIR;
    taskbarIcon /= MIKTEX_PATH_BIN_DIR;
    taskbarIcon /= MIKTEX_TASKBAR_ICON_EXE;
    PathName cmdScriptFileName(options.PortableRoot);
    cmdScriptFileName /= "miktex-portable.cmd";
    StreamWriter cmdScript(cmdScriptFileName);
    cmdScript.WriteLine("@echo off");
    cmdScript.WriteFormattedLine("start %s", taskbarIcon.ToDos().GetData());
    cmdScript.Close();
  }
}

void SetupServiceImpl::DoTheUninstallation()
{
  shared_ptr<Session> session = Session::Get();

  try
  {
    UnregisterShellFileTypes();
  }
  catch (const exception & e)
  {
    ReportLine(e.what());
  }

  try
  {
    if (session->RunningAsAdministrator()
#if defined(MIKTEX_WINDOWS)
      || session->RunningAsPowerUser()
#endif
        )
    {
      UnregisterPath(true);
    }
    UnregisterPath(false);
  }
  catch (const exception & e)
  {
    ReportLine(e.what());
  }

  try
  {
    UnregisterComponents();
  }
  catch (const exception & e)
  {
    ReportLine(e.what());
  }

  if (!session->IsMiKTeXDirect())
  {
    try
    {
      logFile.Process();
    }
    catch (const exception & e)
    {
      ReportLine(e.what());
    }
  }

  try
  {
    PathName parent;
    vector<PathName> roots = GetRoots();
    shared_ptr<Session> session = Session::Get();
    session->UnloadFilenameDatabase();
    if (options.IsThoroughly)
    {
      for (vector<PathName>::const_iterator it = roots.begin();
      it != roots.end();
        ++it)
      {
        if (Directory::Exists(*it))
        {
          Directory::Delete(*it, true);
        }
      }
    }
    else
    {
      PathName dir;
      if (!session->IsMiKTeXDirect())
      {
        dir = session->GetSpecialPath(SpecialPath::InstallRoot);
        dir/= MIKTEX_PATH_MIKTEX_DIR;
        if (Directory::Exists(dir))
        {
          Directory::Delete(dir, true);
        }
      }
      dir = session->GetSpecialPath(SpecialPath::UserDataRoot);
      dir /= MIKTEX_PATH_MIKTEX_DIR;
      if (Directory::Exists(dir))
      {
        Directory::Delete(dir, true);
      }
      dir = session->GetSpecialPath(SpecialPath::UserConfigRoot);
      dir /= MIKTEX_PATH_MIKTEX_DIR;
      if (Directory::Exists(dir))
      {
        Directory::Delete(dir, true);
      }
      if (session->IsAdminMode())
      {
        dir = session->GetSpecialPath(SpecialPath::CommonDataRoot);
        dir /= MIKTEX_PATH_MIKTEX_DIR;
        if (Directory::Exists(dir))
        {
          Directory::Delete(dir, true);
        }
        dir = session->GetSpecialPath(SpecialPath::CommonConfigRoot);
        dir /= MIKTEX_PATH_MIKTEX_DIR;
        if (Directory::Exists(dir))
        {
          Directory::Delete(dir, true);
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
  catch (const exception & e)
  {
    ReportLine(e.what());
  }

  try
  {
    RemoveRegistryKeys();
  }
  catch (const exception & e)
  {
    ReportLine(e.what());
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
    std::shared_ptr<MiKTeX::Packages::PackageManager> pManager(PackageManager::Create());
    shared_ptr<PackageInstaller> pInstaller(pManager->CreateInstaller());
    pInstaller->RegisterComponents(false);
    pInstaller->Dispose();
  }
}

void SetupServiceImpl::ConfigureMiKTeX()
{
  PathName initexmf(GetInstallRoot());
  initexmf /= MIKTEX_PATH_BIN_DIR;
  initexmf /= MIKTEX_INITEXMF_EXE;

  if (pCallback != nullptr && !pCallback->OnProgress(MiKTeX::Setup::Notification::ConfigureBegin))
  {
    cancelled = true;
    return;
  }

  CommandLineBuilder cmdLine;

  if (options.Task != SetupTask::PrepareMiKTeXDirect)
  {
    // define roots & remove old fndbs
    cmdLine.Clear();
    if (options.IsPortable)
    {
      cmdLine.AppendOption("--portable=", GetInstallRoot());
    }
    else
    {
      if (!options.Config.userInstallRoot.Empty())
      {
        cmdLine.AppendOption("--user-install=", options.Config.userInstallRoot);
      }
      if (!options.Config.userDataRoot.Empty())
      {
        cmdLine.AppendOption("--user-data=", options.Config.userDataRoot);
      }
      if (!options.Config.userConfigRoot.Empty())
      {
        cmdLine.AppendOption("--user-config=", options.Config.userConfigRoot);
      }
      if (!options.Config.commonDataRoot.Empty())
      {
        cmdLine.AppendOption("--common-data=", options.Config.commonDataRoot);
      }
      if (!options.Config.commonConfigRoot.Empty())
      {
        cmdLine.AppendOption("--common-config=", options.Config.commonConfigRoot);
      }
      if (!options.Config.commonInstallRoot.Empty())
      {
        cmdLine.AppendOption("--common-install=", options.Config.commonInstallRoot);
      }
#if defined(MIKTEX_WINDOWS)
      if (!options.IsRegistryEnabled)
      {
        cmdLine.AppendOption("--no-registry");
        cmdLine.AppendOption("--create-config-file=", MIKTEX_PATH_MIKTEX_INI);
        cmdLine.AppendOption("--set-config-value=", "[" MIKTEX_REGKEY_CORE "]" MIKTEX_REGVAL_NO_REGISTRY "=1");
      }
#endif
      if (options.IsCommonSetup)
      {
        cmdLine.AppendOption("--set-config-value=", "[" MIKTEX_REGKEY_CORE "]" MIKTEX_REGVAL_SHARED_SETUP "=1");
      }
    }
    if (!options.Config.commonRoots.empty())
    {
      cmdLine.AppendOption("--common-roots=", options.Config.commonRoots);
    }
    if (!options.Config.userRoots.empty())
    {
      cmdLine.AppendOption("--user-roots=", options.Config.userRoots);
    }
    RunIniTeXMF(cmdLine);
    if (cancelled)
    {
      return;
    }

    RunIniTeXMF(CommandLineBuilder("--rmfndb"));

    // register components, configure files
    RunMpm(CommandLineBuilder("--register-components"));

    // create filename database files
    cmdLine.Clear();
    cmdLine.AppendOption("--update-fndb");
    RunIniTeXMF(cmdLine);
    if (cancelled)
    {
      return;
    }

    // create latex.exe, ...
    RunIniTeXMF(CommandLineBuilder("--force", "--mklinks"));
    if (cancelled)
    {
      return;
    }

    // create font map files and language.dat
    RunIniTeXMF(CommandLineBuilder("--mkmaps", "--mklangs"));
    if (cancelled)
    {
      return;
    }
  }

  // set paper size
  if (!options.PaperSize.empty())
  {
    cmdLine.Clear();
    cmdLine.AppendOption("--default-paper-size=", options.PaperSize);
    RunIniTeXMF(cmdLine);
  }

  // set auto-install
  string valueSpec = "[" MIKTEX_REGKEY_PACKAGE_MANAGER "]";
  valueSpec += MIKTEX_REGVAL_AUTO_INSTALL;
  valueSpec += "=";
  valueSpec += std::to_string((int)options.IsInstallOnTheFlyEnabled);
  cmdLine.Clear();
  cmdLine.AppendOption("--set-config-value=", valueSpec);
  RunIniTeXMF(cmdLine);

  if (options.Task != SetupTask::PrepareMiKTeXDirect)
  {
    // refresh file name database again
    RunIniTeXMF(CommandLineBuilder("--update-fndb"));
    if (cancelled)
    {
      return;
    }
  }

  if (!options.IsPortable)
  {
    RunIniTeXMF(CommandLineBuilder("--register-shell-file-types"));
  }

  if (!options.IsPortable && options.IsRegisterPathEnabled)
  {
    RunIniTeXMF(CommandLineBuilder("--modify-path"));
  }

  // create report
  RunIniTeXMF(CommandLineBuilder("--report"));
  if (cancelled)
  {
    return;
  }
}

void SetupServiceImpl::RunIniTeXMF(const CommandLineBuilder & cmdLine1)
{
  shared_ptr<Session> session = Session::Get();

  // make absolute exe path name
  PathName exePath;
  exePath = GetInstallRoot();
  exePath /= MIKTEX_PATH_BIN_DIR;
  exePath /= MIKTEX_INITEXMF_EXE;

  // make command line
  CommandLineBuilder cmdLine(cmdLine1);
  if (options.IsCommonSetup)
  {
    cmdLine.AppendOption("--admin");
  }
  cmdLine.AppendOption("--log-file=", GetULogFileName());
  cmdLine.AppendOption("--verbose");

  // run initexmf.exe
  if (!options.IsDryRun)
  {
    Log("%s %s:\n", Q_(exePath), cmdLine.ToString().c_str());
    ULogClose(false);
    session->UnloadFilenameDatabase();
    Process::Run(exePath, cmdLine.ToString(), this);
    ULogOpen();
  }
}

void SetupServiceImpl::RunMpm(const CommandLineBuilder & cmdLine1)
{
  shared_ptr<Session> session = Session::Get();
  // make absolute exe path name
  PathName exePath;
  exePath = GetInstallRoot();
  exePath /= MIKTEX_PATH_BIN_DIR;
  exePath /= MIKTEX_MPM_EXE;

  // make command line
  CommandLineBuilder cmdLine(cmdLine1);
  if (options.IsCommonSetup)
  {
    cmdLine.AppendOption("--admin");
  }
  cmdLine.AppendOption("--verbose");

  // run mpm.exe
  if (!options.IsDryRun)
  {
    Log("%s %s:\n", Q_(exePath), cmdLine.ToString().c_str());
    ULogClose(false);
    session->UnloadFilenameDatabase();
    Process::Run(exePath.Get(), cmdLine.ToString(), this);
    ULogOpen();
  }
}

void SetupServiceImpl::CreateInfoFile()
{
  StreamWriter stream(PathName(options.LocalPackageRepository, DOWNLOAD_INFO_FILE));
  const char * lpszPackageSet;
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
  wchar_t szSetupPath[BufferSizes::MaxPath];
#if defined(MIKTEX_WINDOWS)
  if (GetModuleFileNameW(0, szSetupPath, BufferSizes::MaxPath) == 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetModuleFileNameW");
  }
#else
  // TODO: get path of running executable
  UNIMPLEMENTED();
#endif
  PathName setupExe(szSetupPath);
  setupExe.RemoveDirectorySpec();
  stream.WriteFormattedLine(
    T_("\
This folder contains the %s package set.\n\
\n\
To install MiKTeX, run %s.\n\
\n\
For more information, visit the MiKTeX project page at\n\
http://miktex.org.\n"), lpszPackageSet, setupExe.Get());
  stream.Close();
  RepositoryInfo repositoryInfo;
  if (pManager->TryGetRepositoryInfo(options.RemotePackageRepository, repositoryInfo))
  {
    StreamWriter stream(PathName(options.LocalPackageRepository, "pr.ini"));
    stream.WriteFormattedLine("[repository]");
    stream.WriteFormattedLine("date=%d", static_cast<int>(repositoryInfo.timeDate));
    stream.WriteFormattedLine("version=%u", static_cast<unsigned>(repositoryInfo.version));
    stream.Close();
  }
}

SetupService::ProgressInfo SetupServiceImpl::GetProgressInfo()
{
  ProgressInfo progressInfo;
  if (options.Task == SetupTask::Uninstall)
  {
    LogFile::ProgressInfo pi = logFile.GetProgressInfo();
    progressInfo.fileName = pi.fileName;
    progressInfo.cFilesRemoveTotal = pi.total;
    progressInfo.cFilesRemoveCompleted = pi.completed;
  }
  else
  {
    PackageInstaller::ProgressInfo pi = pInstaller->GetProgressInfo();
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

bool SetupServiceImpl::OnProcessOutput(const void * pOutput, size_t n)
{
  if (pCallback != nullptr && !pCallback->OnProcessOutput(pOutput, n))
  {
    cancelled = true;
    return false;
  }
  return true;
}

void SetupServiceImpl::ReportLine(const string & str)
{
  if (pCallback != nullptr)
  {
    pCallback->ReportLine(str);
  }
}

bool SetupServiceImpl::OnRetryableError(const string & message)
{
  if (pCallback != nullptr && !pCallback->OnRetryableError(message))
  {
    cancelled = true;
    return false;
  }
  return true;
}

bool SetupServiceImpl::OnProgress(MiKTeX::Packages::Notification nf)
{
  if (pCallback != nullptr)
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
    if (!pCallback->OnProgress(setupNotification))
    {
      cancelled = true;
      return false;
    }
  }
  return true;
}

wstring & SetupServiceImpl::Expand(const char * lpszSource, wstring & dest)
{
  dest = StringUtil::UTF8ToWideChar(lpszSource);
  wstring::size_type pos;
  while ((pos = dest.find(L"%MIKTEX_INSTALL%")) != wstring::npos)
  {
    dest = dest.replace(pos, 16, GetInstallRoot().ToWideCharString());
  }
  return dest;
}

bool SetupServiceImpl::FindFile(const PathName & fileName, PathName & result)
{
  shared_ptr<Session> session = Session::Get();

  // try my directory
  result = session->GetMyLocation();
  result /= fileName;
  if (File::Exists(result))
  {
    return true;
  }

  // try the current directory
  result.SetToCurrentDirectory();
  result /= fileName;
  if (File::Exists(result))
  {
    return true;
  }

  return false;
}
