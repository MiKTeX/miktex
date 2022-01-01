/* SetupService.cpp:

   Copyright (C) 2013-2021 Christian Schenk

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

#include "config.h"

#include "setup-version.h"

#include "internal.h"

#if defined(MIKTEX_WINDOWS)
#  include "win/winSetupService.h"
#endif

using namespace std;
using namespace std::string_literals;

using namespace nlohmann;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Extractor;
using namespace MiKTeX::Locale;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Setup;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

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
    return PathName::Compare(PathName(szShortPath1), PathName(szShortPath2));
  }
#endif
  return PathName::Compare(path1, path2);
}

BEGIN_INTERNAL_NAMESPACE;

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

string IssueSeverityString(IssueSeverity severity)
{
  switch (severity)
  {
  case IssueSeverity::Critical: return "critical issue";
  case IssueSeverity::Major: return "major issue";
  case IssueSeverity::Minor: return "minor issue";
  case IssueSeverity::Trivial: return "trivial issue";
  default: MIKTEX_UNEXPECTED();
  }
}

SetupResources SetupServiceImpl::resources;

END_INTERNAL_NAMESPACE;

namespace MiKTeX {
  namespace Setup {
    inline std::ostream& operator<<(std::ostream& os, const IssueSeverity& severity)
    {
      return os << IssueSeverityString(severity);
    }
    void to_json(json& j, const Issue& issue)
    {
      j = json{ {"type", issue.type}, {"severity", issue.severity}, {"message", issue.message}, {"remedy", issue.remedy}, {"tag", issue.tag} };
    }
    void from_json(const json& j, Issue& issue)
    {
      j.at("type").get_to(issue.type);
      j.at("severity").get_to(issue.severity);
      j.at("message").get_to(issue.message);
      j.at("remedy").get_to(issue.remedy);
      j.at("tag").get_to(issue.tag);
    }
  }
}

string Issue::GetUrl() const
{
  string url;
  if (!tag.empty())
  {
    url = MIKTEX_URL_WWW_KNOWLEDGE_BASE + "/fix-"s + tag;
  }
  return url;
}

string Issue::ToString() const
{
  return fmt::format("{}: {}", severity, message);
}

SetupService::~SetupService() noexcept
{
}

SetupServiceImpl::SetupServiceImpl()
{
  traceStream = TraceStream::Open("setup");
  packageManager = PackageManager::Create();
  session = MIKTEX_SESSION();
  logFile.SetCallback(this);
  options.IsCommonSetup = session->RunningAsAdministrator();
  translator = make_unique<Translator>(MIKTEX_COMP_ID, &resources, session);
}

SetupServiceImpl::~SetupServiceImpl()
{
  try
  {
  }
  catch (const exception&)
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

unique_ptr<TemporaryDirectory> SetupService::CreateSandbox(StartupConfig& startupConfig)
{
  unique_ptr<TemporaryDirectory> sandbox = TemporaryDirectory::Create();
  startupConfig.userInstallRoot = sandbox->GetPathName();
  startupConfig.userDataRoot = sandbox->GetPathName();
  startupConfig.userConfigRoot = sandbox->GetPathName();
  startupConfig.commonDataRoot = sandbox->GetPathName();
  startupConfig.commonConfigRoot = sandbox->GetPathName();
  startupConfig.commonInstallRoot = sandbox->GetPathName();
#if defined(MIKTEX_WINDOWS)
  PathName configDir = sandbox->GetPathName() / PathName(MIKTEX_PATH_MIKTEX_CONFIG_DIR);
  Directory::Create(configDir);
  PathName configFile = configDir / PathName(MIKTEX_INI_FILE);
  ofstream s(configFile.ToString());
  s << fmt::format("[{0}]", MIKTEX_CONFIG_SECTION_CORE) << "\n"
    << fmt::format("{0}=t", MIKTEX_CONFIG_VALUE_NO_REGISTRY) << "\n";
  s.close();
#endif
  return sandbox;
}

PathName SetupService::GetDefaultLocalRepository()
{
  PathName ret;
  string val;
  shared_ptr<Session> session = MIKTEX_SESSION();
  if (session->TryGetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_LOCAL_REPOSITORY, val))
  {
    ret = val;
  }
  else
  {
#if defined(MIKTEX_WINDOWS)
    // default is current users download folder
    wchar_t* downloadFolder = nullptr;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Downloads, 0, nullptr, &downloadFolder);
    MIKTEX_EXPECT(SUCCEEDED(hr));
    MIKTEX_AUTO(CoTaskMemFree(downloadFolder));
    ret = downloadFolder;
    ret /= MIKTEX_PRODUCTNAME_STR;
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
  shared_ptr<Session> session = MIKTEX_SESSION();
  localRepository = session->GetMyLocation(false);
  packageLevel_ = SetupService::TestLocalRepository(localRepository, requestedPackageLevel);
  if (packageLevel_ != PackageLevel::None)
  {
    prefabricated = true;
    return packageLevel_;
  }

  // try ..\tm\packages
  localRepository = session->GetMyLocation(false) / PathName("..") / PathName("tm") / PathName("packages");
  localRepository.MakeFullyQualified();
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
  PathName pathInfoFile(pathRepository, PathName(DOWNLOAD_INFO_FILE));
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
  shared_ptr<Session> session = MIKTEX_SESSION();
  root = session->GetMyLocation(false) / PathName("..");
  root.MakeFullyQualified();
  PathName pathStartupConfig = root / PathName("texmf") / PathName(MIKTEX_PATH_STARTUP_CONFIG_FILE);
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
  if (!pcfg->TryGetValueAsString("Auto", "Config", str) || str != "Direct")
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
  FileStream myImage(File::Open(PathName(szPath), FileMode::Open, FileAccess::Read, false));
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
    return PathName();
  }

  // close the intermediate log file
  logStream.close();

  if (cancel)
  {
    File::Delete(intermediateLogFile);
    return PathName();
  }

  // determine the final log directory
  PathName pathLogDir;
  if (options.IsDryRun || options.Task == SetupTask::PrepareMiKTeXDirect)
  {
    pathLogDir.SetToTempDirectory();
  }
  else
  {
    if (options.Task == SetupTask::InstallFromCD || options.Task == SetupTask::InstallFromLocalRepository || options.Task == SetupTask::InstallFromRemoteRepository || options.Task == SetupTask::FinishSetup)
    {
      if (Directory::Exists(GetInstallRoot()))
      {
        pathLogDir = GetInstallRoot() / PathName(MIKTEX_PATH_MIKTEX_CONFIG_DIR);
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
      return PathName();
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
  Log(fmt::format("{0} {1} Report\n\n", options.Banner, options.Version));
  time_t t = time(nullptr);
  struct tm* pTm = localtime(&t);
  Log(fmt::format("Date: {0:%A, %B %d, %Y}\n", *pTm));
  Log(fmt::format("Time: {0:%H:%M:%S}\n", *pTm));
  Log(fmt::format("OS version: {0}\n", Utils::GetOSVersionString()));
  Log(fmt::format("SystemAdmin: {}\n", session->RunningAsAdministrator()));
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
  Log(fmt::format("Setup path: {0}\n", WU_(szSetupPath)));
#else
  // TODO: log setup path
#endif
  if (options.Task != SetupTask::Download)
  {
    Log(fmt::format("UserRoots: {}\n", options.Config.userRoots.empty() ? "<none specified>" : options.Config.userRoots));
    Log(fmt::format("UserData: {}\n", options.Config.userDataRoot.Empty() ? "<none specified>" : options.Config.userDataRoot.ToString()));
    Log(fmt::format("UserConfig: {}\n", options.Config.userConfigRoot.Empty() ? "<none specified>" : options.Config.userConfigRoot.ToString()));
    Log(fmt::format("CommonRoots: {}\n", options.Config.commonRoots.empty() ? "<none specified>" : options.Config.commonRoots));
    Log(fmt::format("CommonData: {}\n", options.Config.commonDataRoot.Empty() ? "<none specified>" : options.Config.commonDataRoot.ToString()));
    Log(fmt::format("CommonConfig: {}\n", options.Config.commonConfigRoot.Empty() ? "<none specified>" : options.Config.commonConfigRoot.ToString()));
    PathName installRoot = GetInstallRoot();
    Log(fmt::format("Installation: {}\n", installRoot.Empty() ? "<none specified>" : installRoot.ToString()));
  }
}

void SetupServiceImpl::LogInternal(TraceLevel level, const string& s)
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
      traceStream->WriteLine("setup", level, currentLine);
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
  if (options.Task == SetupTask::Download || options.Task == SetupTask::CleanUp)
  {
    return;
  }
  PathName uninstLog(GetULogFileName());
  uninstStream = File::CreateOutputStream(uninstLog, File::Exists(uninstLog) ? ios_base::app : ios_base::out);
  section = None;
}

PathName SetupServiceImpl::GetULogFileName()
{
  PathName directory;
  if (options.IsDryRun || options.Task == SetupTask::PrepareMiKTeXDirect)
  {
    directory.SetToTempDirectory();
  }
  else
  {
    directory = GetInstallRoot() / PathName(MIKTEX_PATH_MIKTEX_CONFIG_DIR);
  }
  return directory / PathName(MIKTEX_UNINSTALL_LOG);
}

void SetupServiceImpl::ULogClose()
{
  if (uninstStream.is_open())
  {
    uninstStream.close();
  }
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
  absolutePath.MakeFullyQualified();
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
  ULogClose();
}

void SetupServiceImpl::CompleteOptions(bool allowRemoteCalls)
{
  if (options.Task == SetupTask::FinishUpdate || options.Task == SetupTask::CleanUp)
  {
    options.IsCommonSetup = session->IsSharedSetup();
    options.IsPortable = session->IsMiKTeXPortable();
  }
  if (options.Task == SetupTask::CleanUp)
  {
    if (!session->IsAdminMode())
    {
      options.Config.userInstallRoot = session->GetSpecialPath(SpecialPath::UserInstallRoot);
      options.Config.userConfigRoot = session->GetSpecialPath(SpecialPath::UserConfigRoot);
      options.Config.userDataRoot = session->GetSpecialPath(SpecialPath::UserDataRoot);
    }
    if (session->IsSharedSetup())
    {
      options.Config.commonInstallRoot = session->GetSpecialPath(SpecialPath::CommonInstallRoot);
      options.Config.commonConfigRoot = session->GetSpecialPath(SpecialPath::CommonConfigRoot);
      options.Config.commonDataRoot = session->GetSpecialPath(SpecialPath::CommonDataRoot);
    }
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
#if defined(MIKTEX_WINDOWS)
    if (options.FolderName.Empty())
    {
      options.FolderName = MIKTEX_PRODUCTNAME_STR;
    }
#endif
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
          MIKTEX_FATAL_ERROR("no local package directory found");
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
    if ((!packageManager->TryGetRemotePackageRepository(options.RemotePackageRepository) || options.RemotePackageRepository.empty()) && allowRemoteCalls)
    {
      options.RemotePackageRepository = packageManager->PickRepositoryUrl();
    }
  }
}

void SetupServiceImpl::Initialize()
{
  if (initialized)
  {
    return;
  }
  initialized = true;

  ReportLine(fmt::format("this is {0}", Utils::MakeProgramVersionString(MIKTEX_COMP_NAME, VersionNumber(MIKTEX_COMPONENT_VERSION_STR))));

  packageInstaller = packageManager->CreateInstaller({ nullptr, true, false });
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

  // remember local repository folder
  session->SetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_LOCAL_REPOSITORY, ConfigValue(options.LocalPackageRepository.ToString()));

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
  if (FindFile(PathName(LICENSE_FILE), licenseFile))
  {
    PathName licenseFileDest(options.LocalPackageRepository, PathName(LICENSE_FILE));
    if (ComparePaths(licenseFile, licenseFileDest, true) != 0)
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
  if (ComparePaths(PathName(szSetupPath), pathDest, true) != 0)
  {
    File::Copy(PathName(szSetupPath), pathDest);
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
    startupConfig.commonInstallRoot = options.PortableRoot / PathName(MIKTEX_PORTABLE_REL_INSTALL_DIR);
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
  RegisterRootDirectoriesOptionSet regOptions = { RegisterRootDirectoriesOption::Temporary };
#if defined(MIKTEX_WINDOWS)
  regOptions += RegisterRootDirectoriesOption::NoRegistry;
#endif
  session->RegisterRootDirectories(startupConfig, regOptions);
  
  // load package manifests
  PathName pathDB;
  bool isArchive;
  if (options.Task == SetupTask::InstallFromCD)
  {
    isArchive = false;
    pathDB = options.MiKTeXDirectRoot / PathName("texmf") / PathName(MIKTEX_PATH_PACKAGE_MANIFESTS_INI);
  }
  else
  {
    isArchive = true;
    pathDB = options.LocalPackageRepository / PathName(MIKTEX_PACKAGE_MANIFESTS_ARCHIVE_FILE_NAME);
  }
  ReportLine("Loading package database...");
  packageManager->LoadDatabase(pathDB, isArchive);

  // create the destination directory
  Directory::Create(GetInstallRoot());

  // open the uninstall script
  ULogOpen();

  // run installer
  packageInstaller->InstallRemove(PackageInstaller::Role::Installer);

  if (cancelled)
  {
    return;
  }

  // install package manifest files
  packageManager->UnloadDatabase();
  packageInstaller->UpdateDb({});

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

  if (!options.IsPortable)
  {
#if defined(MIKTEX_WINDOWS)
    RegisterUninstaller();
#endif
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
  RunOneMiKTeXUtility({ "fndb", "refresh" }, false);
  if (!session->IsSharedSetup() || session->IsAdminMode())
  {
    RunOneMiKTeXUtility({ "links", "update", "--force" }, false);
  }
  RunOneMiKTeXUtility({ "fontmaps", "refresh" }, false);
  RunOneMiKTeXUtility({ "languages", "update" }, false);
  if (!options.IsPortable && (!session->IsSharedSetup() || session->IsAdminMode()))
  {
#if defined(MIKTEX_WINDOWS)
    RunOneMiKTeXUtility({ "filetypes", "register" }, false);
    CreateProgramIcons();
    RegisterUninstaller();
#endif
  }
}

void SetupServiceImpl::DoCleanUp()
{
#if defined(MIKTEX_WINDOWS)
  logFile.Load(session->GetSpecialPath(SpecialPath::InstallRoot) / PathName(MIKTEX_PATH_UNINST_LOG));
#endif

  if (options.CleanupOptions[CleanupOption::Links])
  {
    ReportLine("removing links...");
    try
    {
      RunOneMiKTeXUtility({ "links", "remove" }, false);
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
      if (session->IsAdminMode())
      {
        ReportLine("cleaning system PATH...");
        UnregisterPath(true);
      }
      else
      {
        ReportLine("cleaning user PATH...");
        UnregisterPath(false);
      }
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
      logFile.RemoveStartMenu();
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
          Directory::RemoveEmptyDirectoryChain(parent);
        }
      }
      if (!session->IsAdminMode())
      {
        parent = session->GetSpecialPath(SpecialPath::UserDataRoot);
        parent.CutOffLastComponent();
        if (Directory::Exists(parent))
        {
          Directory::RemoveEmptyDirectoryChain(parent);
        }
        parent = session->GetSpecialPath(SpecialPath::UserConfigRoot);
        parent.CutOffLastComponent();
        if (Directory::Exists(parent))
        {
          Directory::RemoveEmptyDirectoryChain(parent);
        }
      }
      if (session->IsAdminMode())
      {
        parent = session->GetSpecialPath(SpecialPath::CommonDataRoot);
        parent.CutOffLastComponent();
        if (Directory::Exists(parent))
        {
          Directory::RemoveEmptyDirectoryChain(parent);
        }
        parent = session->GetSpecialPath(SpecialPath::CommonConfigRoot);
        parent.CutOffLastComponent();
        if (Directory::Exists(parent))
        {
          Directory::RemoveEmptyDirectoryChain(parent);
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
      logFile.RemoveRegistrySettings();
#else
      PathName fontConfig(MIKTEX_SYSTEM_ETC_FONTS_CONFD_DIR);
      // FIXME: hard-coded file name
      fontConfig /= "09-miktex.conf";
      if (session->IsAdminMode() && File::Exists(fontConfig))
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
  if (!session->IsMiKTeXDirect())
  {
    PathName installRoot = session->GetSpecialPath(SpecialPath::InstallRoot);
    vec.push_back(installRoot);
  }
  if (!session->IsAdminMode())
  {
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
  if (session->IsAdminMode())
  {
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

  if (options.Task == SetupTask::FinishSetup || options.Task == SetupTask::InstallFromCD || options.Task == SetupTask::InstallFromLocalRepository || options.Task == SetupTask::InstallFromRemoteRepository || options.Task == SetupTask::PrepareMiKTeXDirect)
  {
    args.push_back("--principal=setup");
  }

  if (options.Task != SetupTask::PrepareMiKTeXDirect)
  {
    // define roots & remove old fndbs
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
        args.push_back("--set-config-value="s + "[" + MIKTEX_CONFIG_SECTION_CORE + "]" + MIKTEX_CONFIG_VALUE_NO_REGISTRY + "=1");
      }
#else
      args.push_back("--create-config-file="s + MIKTEX_PATH_MIKTEX_INI);
#endif
      args.push_back("--set-config-value="s + "[" + MIKTEX_CONFIG_SECTION_CORE + "]" + MIKTEX_CONFIG_VALUE_SHARED_SETUP + "=" + (options.IsCommonSetup ? "1" : "0"));
    }
    if (!options.Config.commonRoots.empty())
    {
      args.push_back("--common-roots=" + options.Config.commonRoots);
    }
    if (!options.Config.userRoots.empty())
    {
      args.push_back("--user-roots=" + options.Config.userRoots);
    }
    if (!options.CommonLinkTargetDirectory.Empty())
    {
      args.push_back(fmt::format("--set-config-value=[{}]{}={}", MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMONLINKTARGETDIRECTORY, options.CommonLinkTargetDirectory));
    }
    if (!options.UserLinkTargetDirectory.Empty())
    {
      args.push_back(fmt::format("--set-config-value=[{}]{}={}", MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USERLINKTARGETDIRECTORY, options.UserLinkTargetDirectory));
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
      RunOneMiKTeXUtility({ "fndb", "remove" }, false);
    }

    // register components, configure files
#if defined(MIKTEX_WINDOWS)
    if (options.Task != SetupTask::FinishSetup)
    {
      RunMpm({ "--register-components" });
    }
#endif

    // create file name database files
    RunOneMiKTeXUtility({ "fndb", "refresh" }, false);
    if (cancelled)
    {
      return;
    }

    // create latex.exe, ...
    RunOneMiKTeXUtility({ "links", "update", "--force" }, false);
    if (cancelled)
    {
      return;
    }

    // create font map files and language.dat
    RunOneMiKTeXUtility({ "fontmaps", "refresh" }, false);
    RunOneMiKTeXUtility({ "languages", "update" }, false);

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
    RunOneMiKTeXUtility({ "fndb", "refresh" }, false);
    if (cancelled)
    {
      return;
    }
  }

  if (!options.IsPortable)
  {
#if defined(MIKTEX_WINDOWS)
    RunOneMiKTeXUtility({ "filetypes", "register" }, false);
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
    return options.PortableRoot / PathName(MIKTEX_PORTABLE_REL_INSTALL_DIR);
  }
  else if (options.Task == SetupTask::FinishSetup || options.Task == SetupTask::FinishUpdate || options.Task == SetupTask::CleanUp)
  {
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
    return session->GetSpecialPath(SpecialPath::BinDirectory);
  }
  else
  {
    return GetInstallRoot() / PathName(MIKTEX_PATH_BIN_DIR);
  }
}

void SetupServiceImpl::RunIniTeXMF(const vector<string>& args, bool mustSucceed)
{
  // make absolute exe path name
  PathName exePath = GetBinDir() / PathName(MIKTEX_INITEXMF_EXE);

  // make command line
  vector<string> allArgs{ exePath.GetFileNameWithoutExtension().ToString() };
  allArgs.insert(allArgs.end(), args.begin(), args.end());
  if (options.IsCommonSetup && session->IsAdminMode())
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
    ULogClose();
    // FIXME: only need to unload when building the FNDB
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

void SetupServiceImpl::RunOneMiKTeXUtility(const vector<string>& args, bool mustSucceed)
{
  // make absolute exe path name
  PathName exePath = GetBinDir() / PathName(MIKTEX_MIKTEX_EXE);

  // make command line
  vector<string> allArgs{ exePath.GetFileNameWithoutExtension().ToString() };
  allArgs.insert(allArgs.end(), args.begin(), args.end());
  if (options.IsCommonSetup && session->IsAdminMode())
  {
    allArgs.push_back("--admin");
  }
  if (options.Task != SetupTask::FinishSetup && options.Task != SetupTask::FinishUpdate && options.Task != SetupTask::CleanUp)
  {
    allArgs.push_back("--log-file=" + GetULogFileName().ToString());
  }
  allArgs.push_back("--disable-installer");
  allArgs.push_back("--verbose");

  // run One MiKTeX Utility
  if (!options.IsDryRun)
  {
    Log(fmt::format("{}:\n", CommandLineBuilder(allArgs).ToString()));
    ULogClose();
    // FIXME: only need to unload when building the FNDB
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
  // make absolute exe path name
  PathName exePath = GetBinDir() / PathName(MIKTEX_MPM_EXE);

  // make command line
  vector<string> allArgs{ exePath.GetFileNameWithoutExtension().ToString() };
  allArgs.insert(allArgs.end(), args.begin(), args.end());
  if (options.IsCommonSetup && session->IsAdminMode())
  {
    allArgs.push_back("--admin");
  }
  allArgs.push_back("--verbose");

  // run mpm.exe
  if (!options.IsDryRun)
  {
    Log(fmt::format("{}:\n", CommandLineBuilder(allArgs).ToString()));
    ULogClose();
    Process::Run(exePath, allArgs, this);
    ULogOpen();
  }
}

void SetupServiceImpl::CreateInfoFile()
{
  StreamWriter stream(PathName(options.LocalPackageRepository, PathName(DOWNLOAD_INFO_FILE)));
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
  stream.WriteLine(fmt::format("This folder contains the {0} package set.", lpszPackageSet));
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
  stream.WriteLine(fmt::format("To install MiKTeX, run {0}.", setupExe));
  stream.WriteLine();
  stream.WriteLine();
#endif
  stream.WriteLine("For more information, visit the MiKTeX project page at\n\nhttps://miktex.org.");
  stream.WriteLine();
  stream.Close();
  RepositoryInfo repositoryInfo;
  if (packageManager->TryGetRepositoryInfo(options.RemotePackageRepository, repositoryInfo))
  {
    StreamWriter stream(PathName(options.LocalPackageRepository, PathName("pr.ini")));
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
    // TODO
  }
  else
  {
    PackageInstaller::ProgressInfo pi = packageInstaller->GetProgressInfo();
    progressInfo.packageId = pi.packageId;
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

wstring& SetupServiceImpl::Expand(const string& source, wstring& dest)
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
  // try my directory
  result = session->GetMyLocation(false) / fileName;
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

void SetupServiceImpl::RemoveFormatFiles()
{
  vector<PathName> toBeDeleted;
  PathName pathFmt(session->GetSpecialPath(SpecialPath::DataRoot));
  pathFmt /= MIKTEX_PATH_FMT_DIR;
  if (Directory::Exists(pathFmt))
  {
    CollectFiles(toBeDeleted, pathFmt, MIKTEX_FORMAT_FILE_SUFFIX);
  }
  if (!session->IsAdminMode())
  {
    PathName pathFmt2(session->GetSpecialPath(SpecialPath::UserDataRoot));
    pathFmt2 /= MIKTEX_PATH_FMT_DIR;
    if (pathFmt2 != pathFmt && Directory::Exists(pathFmt2))
    {
      CollectFiles(toBeDeleted, pathFmt2, MIKTEX_FORMAT_FILE_SUFFIX);
    }
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
      PathName path(dir, PathName(entry.name));
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
    CollectFiles(vec, PathName(dir, PathName(s)), lpszExt);
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

constexpr time_t HALF_A_YEAR = 15768000;
constexpr time_t ONE_DAY = 86400;

inline string FormatTimestamp(time_t t)
{
  return IsValidTimeT(t) ? fmt::format("{:%F %T}", *localtime(&t)) : "not yet";
}

void SetupServiceImpl::WriteReport(ostream& s, ReportOptionSet options)
{
  time_t now = time(nullptr);
  if (options[ReportOption::General])
  {
    SetupConfig setupConfig = session->GetSetupConfig();
    auto p = Utils::CheckPath();
    s << "ReportDate: " << FormatTimestamp(now) << "\n"
      << "CurrentVersion: " << Utils::GetMiKTeXVersionString() << "\n"
      << "SetupDate: " << FormatTimestamp(setupConfig.setupDate) << "\n"
      << "SetupVersion: " << (setupConfig.setupVersion == VersionNumber() ? MIKTEX_LEGACY_MAJOR_MINOR_STR : setupConfig.setupVersion.ToString()) << "\n"
      << "Configuration: " << (session->IsMiKTeXPortable() ? "Portable" : "Regular") << "\n";
    if (Utils::HaveGetGitInfo())
    {
      s << "GitInfo: " << Utils::GetGitInfo() << "\n";
    }
    s << "OS: " << Utils::GetOSVersionString() << "\n"
      << "SharedSetup: " << (session->IsSharedSetup() ? "yes" : "no") << "\n"
      << "LinkTargetDirectory: " << session->GetSpecialPath(SpecialPath::LinkTargetDirectory) << "\n"
      << "PathOkay: " << (p.first ? "yes" : "no") << "\n";
    if (!session->IsAdminMode())
    {
      InstallationSummary userInstallation = packageManager->GetInstallationSummary(true);
      if (userInstallation.packageCount > 0)
      {
        s << "LastUpdateCheck: " << FormatTimestamp(userInstallation.lastUpdateCheck) << "\n";
        s << "LastUpdate: " << FormatTimestamp(userInstallation.lastUpdate) << "\n";
        s << "LastUpdateDb: " << FormatTimestamp(userInstallation.lastUpdateDb) << "\n";
      }
    }
    if (session->IsSharedSetup())
    {
      InstallationSummary commonInstallation = packageManager->GetInstallationSummary(false);
      s << "LastUpdateCheckAdmin: " << FormatTimestamp(commonInstallation.lastUpdateCheck) << "\n";
      s << "LastUpdateAdmin: " << FormatTimestamp(commonInstallation.lastUpdate) << "\n";
      s << "LastUpdateDbAdmin: " << FormatTimestamp(commonInstallation.lastUpdateDb) << "\n";

    }
  }
  if (options[ReportOption::CurrentUser])
  {
    s << "SystemAdmin: " << (session->IsUserAnAdministrator() ? "yes" : "no") << "\n"
      << "RootPrivileges: " << (session->RunningAsAdministrator() ? "yes" : "no") << "\n"
      << "AdminMode: " << (session->IsAdminMode() ? "yes" : "no") << "\n";
  }
  if (options[ReportOption::RootDirectories])
  {
    vector<RootDirectoryInfo> roots = session->GetRootDirectories();
    for (int idx = 0; idx < roots.size(); ++idx)
    {
      s << fmt::format("Root{}: {}", idx, roots[idx].path) << "\n";
    }
    if (!session->IsAdminMode())
    {
      s << "UserInstall: " << session->GetSpecialPath(SpecialPath::UserInstallRoot) << "\n"
        << "UserConfig: " << session->GetSpecialPath(SpecialPath::UserConfigRoot) << "\n"
        << "UserData: " << session->GetSpecialPath(SpecialPath::UserDataRoot) << "\n";
    }
    if (session->IsSharedSetup())
    {
      s << "CommonInstall: " << session->GetSpecialPath(SpecialPath::CommonInstallRoot) << "\n"
        << "CommonConfig: " << session->GetSpecialPath(SpecialPath::CommonConfigRoot) << "\n"
        << "CommonData: " << session->GetSpecialPath(SpecialPath::CommonDataRoot) << "\n";
    }
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
      for (const string& p : StringUtil::Split(env, PathNameUtil::PathNameDelimiter))
      {
        s << "PATH" << idx++ << p << "\n";
      }
    }
  }
  vector<Issue> issues = FindIssues(options[ReportOption::General], options[ReportOption::BrokenPackages]);
  if (!issues.empty())
  {
    s << "\n" << "The following issues were detected:" << "\n";
    int nr = 1;
    for (const auto& iss : issues)
    {
      s << fmt::format("  {}: {}: {}", nr, iss.severity, iss.message) << "\n";
      nr++;
    }
  }
}

void SetupServiceImpl::WriteReport(ostream& s)
{
  WriteReport(s, { ReportOption::General, ReportOption::RootDirectories, ReportOption::CurrentUser });
}

vector<Issue> SetupServiceImpl::FindIssues(bool checkPath, bool checkPackageIntegrity)
{
  vector<Issue> result;
  time_t now = time(nullptr);
  if (checkPath)
  {
    auto p = Utils::CheckPath();
    if (!p.first && p.second)
    {
      result.push_back({
        IssueType::Path,
        IssueSeverity::Minor,
        T_("The PATH variable does not include the MiKTeX executables."),
        T_("Find the directory which contains the MiKTeX executables and add it to the environment variable PATH."),
        "path-variable"
      });
    }
  }
  if (session->IsSharedSetup())
  {
    InstallationSummary commonInstallation = packageManager->GetInstallationSummary(false);
    if (session->IsAdminMode())
    {
      if (!IsValidTimeT(commonInstallation.lastUpdateCheck))
      {
        result.push_back({
          IssueType::AdminUpdateCheckOverdue,
          IssueSeverity::Major,
          T_("So far, no MiKTeX administrator has checked for updates."),
          T_("Switch to MiKTeX administrator mode and check for updates.")
        });
      }
      else if (now > commonInstallation.lastUpdateCheck + HALF_A_YEAR)
      {
        result.push_back({
          IssueType::AdminUpdateCheckOverdue,
          IssueSeverity::Minor,
          T_("It has been a long time since a MiKTeX administrator has checked for updates."),
          T_("Switch to MiKTeX administrator mode and check for updates.")
        });
      }
    }
    else
    {
      InstallationSummary userInstallation = packageManager->GetInstallationSummary(true);
      if (IsValidTimeT(userInstallation.lastUpdate) && (!IsValidTimeT(commonInstallation.lastUpdateCheck) || userInstallation.lastUpdate > commonInstallation.lastUpdateCheck))
      {
        result.push_back({
          IssueType::AdminUpdateCheckOverdue,
          IssueSeverity::Major,
          T_("User/administrator updates are out-of-sync."),
          T_("Switch to MiKTeX administrator mode and check for updates."),
          "user-admin-updates-out-of-sync"
        });
      }
      else if (!IsValidTimeT(commonInstallation.lastUpdateCheck))
      {
        result.push_back({
          IssueType::AdminUpdateCheckOverdue,
          IssueSeverity::Major,
          T_("So far, no MiKTeX administrator has checked for updates."),
          T_("Switch to MiKTeX administrator mode and check for updates.")
        });
      }
      else if (now > commonInstallation.lastUpdateCheck + HALF_A_YEAR)
      {
        result.push_back({
          IssueType::AdminUpdateCheckOverdue,
          IssueSeverity::Minor,
          T_("It has been a long time since a MiKTeX administrator has checked for updates."),
          T_("Switch to MiKTeX administrator mode and check for updates.")
        });
      }
      else if (userInstallation.packageCount > 0)
      {
        if (!IsValidTimeT(userInstallation.lastUpdateCheck))
        {
          result.push_back({
            IssueType::UserUpdateCheckOverdue,
            IssueSeverity::Major,
            T_("So far, you have not checked for updates as a MiKTeX user."),
            T_("Stay in MiKTeX user mode and check for updates.")
          });
        }
        else if (IsValidTimeT(commonInstallation.lastUpdate) && (!IsValidTimeT(userInstallation.lastUpdateCheck) || commonInstallation.lastUpdate > userInstallation.lastUpdateCheck))
        {
          result.push_back({
            IssueType::UserUpdateCheckOverdue,
            IssueSeverity::Major,
            T_("User/administrator updates are out-of-sync."),
            T_("Stay in MiKTeX user mode and check for updates."),
            "user-admin-updates-out-of-sync"
            });
        }
        else if (now > userInstallation.lastUpdateCheck + HALF_A_YEAR)
        {
          result.push_back({
            IssueType::UserUpdateCheckOverdue,
            IssueSeverity::Minor,
            T_("It has been a long time since you have checked for updates as a MiKTeX user."),
            T_("Stay in MiKTeX user mode and check for updates.")
          });
        }
      }
    }
  }
  else
  {
    MIKTEX_ASSERT(!session->IsAdminMode());
    InstallationSummary userInstallation = packageManager->GetInstallationSummary(true);
    MIKTEX_ASSERT(userInstallation.packageCount > 0);
    if (!IsValidTimeT(userInstallation.lastUpdateCheck))
    {
      result.push_back({
        IssueType::UserUpdateCheckOverdue,
        IssueSeverity::Major,
        T_("So far, you have not checked for MiKTeX updates."),
        T_("Check for MiKTeX updates.")
      });
    }
    else if (now > userInstallation.lastUpdateCheck + HALF_A_YEAR)
    {
      result.push_back({
        IssueType::UserUpdateCheckOverdue,
        IssueSeverity::Minor,
        T_("It has been a long time since you have checked for MiKTeX updates."),
        T_("Check for MiKTeX updates.")
      });
    }
  }
  vector<RootDirectoryInfo> roots = session->GetRootDirectories();
  for (int idx = 0; idx < roots.size(); ++idx)
  {
    for (int idx2 = 0; idx2 < idx; ++idx2)
    {
      if (Utils::IsParentDirectoryOf(roots[idx2].path, roots[idx].path))
      {
        result.push_back({
          IssueType::RootDirectoryCoverage,
          IssueSeverity::Major,
          fmt::format(T_("Root directory #{0} is covered by root directory #{1}."), idx, idx2),
          "" // TODO
        });
      }
      else if (Utils::IsParentDirectoryOf(roots[idx].path, roots[idx2].path))
      {
        result.push_back({
          IssueType::RootDirectoryCoverage,
          IssueSeverity::Major,
          fmt::format(T_("Root directory #{0} covers root directory #{1}."), idx, idx2),
          "" // TODO
        });
      }
    }
  }
  if (checkPackageIntegrity)
  {
    unique_ptr<PackageIterator> pkgIter(packageManager->CreateIterator());
    PackageInfo packageInfo;
    for (int idx = 0; pkgIter->GetNext(packageInfo); ++idx)
    {
      if (!packageInfo.IsPureContainer()
        && packageInfo.IsInstalled()
        && packageInfo.id.compare(0, 7, "miktex-") == 0)
      {
        if (!(packageManager->TryVerifyInstalledPackage(packageInfo.id)))
        {
          result.push_back({
            IssueType::PackageDamaged,
            IssueSeverity::Critical,
            fmt::format(T_("Package {0} has been tampered with."), packageInfo.id),
            "" // TODO
          });
        }
      }
    }
    pkgIter->Dispose();
  }
  PathName issuesJson = session->GetSpecialPath(SpecialPath::ConfigRoot) / PathName(MIKTEX_PATH_ISSUES_JSON);
  Directory::Create(issuesJson.GetDirectoryName());
  File::CreateOutputStream(issuesJson) << json(result);
  session->SetConfigValue(
    MIKTEX_CONFIG_SECTION_SETUP,
    session->IsAdminMode() ? MIKTEX_CONFIG_VALUE_LAST_ADMIN_DIAGNOSE : MIKTEX_CONFIG_VALUE_LAST_USER_DIAGNOSE,
    ConfigValue(std::to_string(time(nullptr))));
  return result;
}

vector<Issue> SetupServiceImpl::GetIssues()
{
  vector<Setup::Issue> issues;
  PathName issuesJson = session->GetSpecialPath(SpecialPath::ConfigRoot) / PathName(MIKTEX_PATH_ISSUES_JSON);
  if (File::Exists(issuesJson))
  {
    try
    {
      const json j_array = json::parse(File::CreateInputStream(issuesJson));
      if (j_array.is_array())
      {
        for (json::const_iterator it = j_array.begin(); it != j_array.end(); ++it)
        {
          Setup::Issue issue = it->get<Setup::Issue>();
          issues.push_back(issue);
        }
      }
    }
    catch (const nlohmann::json::exception& ex)
    {
      // TODO: logging
    }

  }
  return issues;
}