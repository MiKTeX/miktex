/* app.cpp:

   Copyright (C) 2005-2020 Christian Schenk
 
   This file is part of the MiKTeX App Library.

   The MiKTeX App Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX App Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX App Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "config.h"

#include <csignal>
#include <cstdlib>
#include <ctime>

#include <iomanip>
#include <iostream>
#include <memory>
#include <set>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <log4cxx/basicconfigurator.h>
#include <log4cxx/logger.h>
#include <log4cxx/rollingfileappender.h>
#include <log4cxx/xml/domconfigurator.h>

#include <miktex/App/Application>
#include <miktex/Core/AutoResource>
#include <miktex/Core/Cfg>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/ConfigNames>
#include <miktex/Core/Exceptions>
#include <miktex/Core/File>
#include <miktex/Core/FileType>
#include <miktex/Core/LockFile>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Core/Session>
#include <miktex/Setup/SetupService>
#include <miktex/Trace/Trace>
#include <miktex/UI/UI>
#include <miktex/Util/StringUtil>

#include "internal.h"

#include "app-version.h"

using namespace std;
using namespace std::string_literals;

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

static log4cxx::LoggerPtr logger;
static bool isLog4cxxConfigured = false;

static Application* instance = nullptr;

static bool initUiFrameworkDone = false;

static volatile sig_atomic_t cancelled;

static void MIKTEXCEECALL SignalHandler(int signalToBeHandled)
{
  switch (signalToBeHandled)
  {
  case SIGINT:
  case SIGTERM:
    signal(SIGINT, SIG_IGN);
    cancelled = true;
    break;
  }
}

bool Application::Cancelled()
{
  return cancelled == 0 ? false : true;
}

void Application::CheckCancel()
{
  if (Cancelled())
  {
    throw MiKTeXException(Utils::GetExeName(), T_("The current operation has been cancelled (Ctrl-C)."), MiKTeXException::KVMAP(), SourceLocation());
  }
}

Application* Application::GetApplication()
{
  return instance;
}

class Impl
{
public:
  set<string> ignoredPackages;
public:
  TriState mpmAutoAdmin = TriState::Undetermined;
public:
  shared_ptr<PackageManager> packageManager;
public:
  shared_ptr<PackageInstaller> installer;
public:
  bool initialized = false;
public:
  vector<TraceCallback::TraceMessage> pendingTraceMessages;
public:
  TriState enableInstaller = TriState::Undetermined;
public:
  bool beQuiet = false;
public:
  TriState enableMaintenance = TriState::Undetermined;
public:
  TriState enableDiagnose = TriState::Undetermined;
public:
  shared_ptr<Session> session;
public:
  string commandLine;
};

class Application::impl :
  public Impl
{

};

Application::Application() :
  pimpl(make_unique<impl>())
{
}

Application::~Application() noexcept
{
  try
  {
    if (pimpl->initialized)
    {
      Finalize();
    }
  }
  catch (const exception&)
  {
  }
}

void InstallSignalHandler(int sig)
{
  void(*oldHandlerFunc) (int);
  oldHandlerFunc = signal(sig, SignalHandler);
  if (oldHandlerFunc == SIG_ERR)
  {
    MIKTEX_FATAL_CRT_ERROR("signal");
  }
  if (oldHandlerFunc != SIG_DFL)
  {
    if (signal(sig, oldHandlerFunc) == SIG_ERR)
    {
      MIKTEX_FATAL_CRT_ERROR("signal");
    }
  }
}

template<typename T> void ExamineArgs(vector<T>& args, Session::InitInfo& initInfo, Impl* pimpl)
{
  CommandLineBuilder commandLine;
  commandLine.SetQuotingConvention(QuotingConvention::None);
  auto it = args.begin();
  while (it != args.end() && *it != nullptr)
  {
    commandLine.AppendArgument(*it);
    bool keepArgument = false;
    if (strcmp(*it, "--miktex-admin") == 0)
    {
      initInfo.AddOption(Session::InitOption::AdminMode);
    }
    else if (strcmp(*it, "--miktex-disable-installer") == 0)
    {
      pimpl->enableInstaller = TriState::False;
    }
    else if (strcmp(*it, "--miktex-enable-installer") == 0)
    {
      pimpl->enableInstaller = TriState::True;
    }
    else if (strcmp(*it, "--miktex-disable-maintenance") == 0)
    {
      pimpl->enableMaintenance = TriState::False;
    }
    else if (strcmp(*it, "--miktex-enable-maintenance") == 0)
    {
      pimpl->enableMaintenance = TriState::True;
    }
    else if (strcmp(*it, "--miktex-disable-diagnose") == 0)
    {
      pimpl->enableDiagnose = TriState::False;
    }
    else if (strcmp(*it, "--miktex-enable-diagnose") == 0)
    {
      pimpl->enableDiagnose = TriState::True;
    }
    else
    {
      keepArgument = true;
    }
    if (keepArgument)
    {
      ++it;
    }
    else
    {
      it = args.erase(it);
    }
  }
  pimpl->commandLine = commandLine.ToString();
}

string Application::ExamineArgs(std::vector<const char*>& args, MiKTeX::Core::Session::InitInfo& initInfo)
{
  ::ExamineArgs(args, initInfo, pimpl.get());
  return pimpl->commandLine;
}

string Application::ExamineArgs(std::vector<char*>& args, MiKTeX::Core::Session::InitInfo& initInfo)
{
  ::ExamineArgs(args, initInfo, pimpl.get());
  return pimpl->commandLine;
}

void Application::Init(const Session::InitInfo& initInfoArg, vector<const char*>& args)
{
  Session::InitInfo initInfo(initInfoArg);
  MIKTEX_ASSERT(!args.empty() && args.back() == nullptr);
  ::ExamineArgs(args, initInfo, pimpl.get());
  Init(initInfo);
}

void Application::Init(const Session::InitInfo& initInfoArg, vector<char*>& args)
{
  Session::InitInfo initInfo(initInfoArg);
  MIKTEX_ASSERT(!args.empty() && args.back() == nullptr);
  ::ExamineArgs(args, initInfo, pimpl.get());
  Init(initInfo);
}

void Application::ConfigureLogging()
{
  string myName = Utils::GetExeName();
  PathName xmlFileName;
  if (pimpl->session->FindFile(myName + "." + MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName)
    || pimpl->session->FindFile(MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName))
  {
    PathName logDir = pimpl->session->GetSpecialPath(SpecialPath::LogDirectory);
    string logName = myName;
    if (pimpl->session->IsAdminMode())
    {
      logName += MIKTEX_ADMIN_SUFFIX;
    }
    Utils::SetEnvironmentString("MIKTEX_LOG_DIR", logDir.ToString());
    Utils::SetEnvironmentString("MIKTEX_LOG_NAME", logName);
    log4cxx::xml::DOMConfigurator::configure(xmlFileName.ToWideCharString());
  }
  else
  {
    log4cxx::BasicConfigurator::configure();
  }
  isLog4cxxConfigured = true;
  logger = log4cxx::Logger::getLogger(myName);
}

inline bool IsNewer(const PathName& path1, const PathName& path2)
{
  return File::Exists(path1) && File::Exists(path2) && File::GetLastWriteTime(path1) > File::GetLastWriteTime(path2);
}

void Application::AutoMaintenance()
{
  time_t lastAdminMaintenance = pimpl->session->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_LAST_ADMIN_MAINTENANCE, ConfigValue("0")).GetTimeT();
  time_t lastUserMaintenance = pimpl->session->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_LAST_USER_MAINTENANCE, ConfigValue("0")).GetTimeT();
  bool isSetupMode = lastAdminMaintenance == 0 && lastUserMaintenance == 0 && !pimpl->session->IsMiKTeXPortable();
  if (isSetupMode)
  {
    cerr
      << "\n"
      << "It seems that this is a fresh TeX installation.\n"
      << "Please finish the setup before proceeding.\n"
      << "For more information, visit:\n"
#if defined(MIKTEX_WINDOWS)
      << "https://miktex.org/howto/install-miktex-win" << "\n";
#elif defined(__APPLE__)
      << "https://miktex.org/howto/install-miktex-mac" << "\n";
#else
      << "https://miktex.org/howto/install-miktex-unx" << "\n";
#endif
    throw 1;
  }

  // must refresh FNDB if:
  //   (1) it doesn't exist
  //   (2) in user mode and an admin just modified the MiKTeX configuration
  PathName mpmDatabasePath(pimpl->session->GetMpmDatabasePathName());
  bool mustRefreshFndb = !File::Exists(mpmDatabasePath) || (!pimpl->session->IsAdminMode() && lastAdminMaintenance > File::GetLastWriteTime(mpmDatabasePath));

  // must build language.dat if:
  //   (1) in user mode and an admin just modified the MiKTeX configuration
  //   (2) in user mode and languages.ini is newer than languages.dat
  PathName userLanguageDat = pimpl->session->IsAdminMode() ? PathName() : pimpl->session->GetSpecialPath(SpecialPath::UserConfigRoot) / PathName(MIKTEX_PATH_LANGUAGE_DAT);
  bool mustRefreshUserLanguageDat = !pimpl->session->IsAdminMode() && File::Exists(userLanguageDat) && lastAdminMaintenance > File::GetLastWriteTime(userLanguageDat);
  PathName userLanguagesIni = pimpl->session->IsAdminMode() ? PathName() : pimpl->session->GetSpecialPath(SpecialPath::UserConfigRoot) / PathName(MIKTEX_PATH_LANGUAGES_INI);
  mustRefreshUserLanguageDat = mustRefreshUserLanguageDat || (!pimpl->session->IsAdminMode() && IsNewer(userLanguagesIni, userLanguageDat));

  // must update package db if:
  //   (1) in user mode and the system-wide package db is newer than the user package db
  bool mustUpdateDb = false;
  if (!pimpl->session->IsAdminMode())
  {
    time_t lastAdminUpdateDb = pimpl->session->GetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_LAST_ADMIN_UPDATE_DB, ConfigValue("0")).GetTimeT();
    PathName userPackageManifestsIni = pimpl->session->GetSpecialPath(SpecialPath::InstallRoot) / PathName(MIKTEX_PATH_PACKAGE_MANIFESTS_INI);
    mustUpdateDb = File::Exists(userPackageManifestsIni) && lastAdminUpdateDb > File::GetLastWriteTime(userPackageManifestsIni);
  }

  PathName initexmf;
  if ((mustRefreshFndb || mustRefreshUserLanguageDat || mustUpdateDb) && pimpl->session->FindFile(MIKTEX_INITEXMF_EXE, FileType::EXE, initexmf))
  {
    unique_ptr<MiKTeX::Core::LockFile> lockFile = LockFile::Create(pimpl->session->GetSpecialPath(SpecialPath::DataRoot) / PathName(MIKTEX_PATH_AUTO_MAINTENANCE_LOCK));
    if (!lockFile->TryLock(0ms))
    {
      return;
    }
    LOG4CXX_TRACE(logger, "running MIKTEX_HOOK_AUTO_MAINTENANCE")
    if (mustUpdateDb)
    {
      LOG4CXX_INFO(logger, "refreshing user's package database from cache");
      if (pimpl->packageManager == nullptr)
      {
        pimpl->packageManager = PackageManager::Create(PackageManager::InitInfo(this));
      }
      if (pimpl->installer == nullptr)
      {
        pimpl->installer = pimpl->packageManager->CreateInstaller();
      }
      AutoRestore<bool> restore(pimpl->beQuiet);
      pimpl->beQuiet = true;
      pimpl->installer->SetCallback(this);
      pimpl->installer->UpdateDb({ UpdateDbOption::FromCache });
    }
    vector<string> commonArgs{ initexmf.GetFileNameWithoutExtension().ToString() };
    switch (pimpl->enableInstaller)
    {
    case TriState::False:
      commonArgs.push_back("--disable-installer");
      break;
    case TriState::True:
      commonArgs.push_back("--enable-installer");
      break;
    case TriState::Undetermined:
      break;
    }
    if (pimpl->session->IsAdminMode())
    {
      commonArgs.push_back("--admin");
    }
    commonArgs.push_back("--quiet");
    int exitCode;
    if (mustRefreshFndb)
    {
      vector<string> args = commonArgs;
      args.push_back("--update-fndb");
      LOG4CXX_INFO(logger, "running 'initexmf' to refresh the file name database");
      pimpl->session->UnloadFilenameDatabase();
      if (!Process::Run(initexmf, args, nullptr, &exitCode, nullptr))
      {
        LOG4CXX_ERROR(logger, "initexmf exited with code " << exitCode);
      }
    }
    if (mustRefreshFndb)
    {
      vector<string> args = commonArgs;
      args.push_back("--mkmaps");
      LOG4CXX_INFO(logger, "running 'initexmf' to create font map files");
      if (!Process::Run(initexmf, args, nullptr, &exitCode, nullptr))
      {
        LOG4CXX_ERROR(logger, "initexmf exited with code " << exitCode);
      }
    }
    if (mustRefreshUserLanguageDat)
    {
      MIKTEX_ASSERT(!pimpl->session->IsAdminMode());
      vector<string> args = commonArgs;
      args.push_back("--mklangs");
      LOG4CXX_INFO(logger, "running 'initexmf' to refresh language.dat");
      if (!Process::Run(initexmf, args, nullptr, &exitCode, nullptr))
      {
        LOG4CXX_ERROR(logger, "initexmf exited with code " << exitCode);
      }
    }
  }
}

constexpr time_t ONE_DAY = 86400;
constexpr time_t ONE_WEEK = 7 * ONE_DAY;

void Application::AutoDiagnose()
{
  time_t now = time(nullptr);
  PathName issuesJson = pimpl->session->GetSpecialPath(SpecialPath::ConfigRoot) / PathName(MIKTEX_PATH_ISSUES_JSON);
  vector<Setup::Issue> issues;
  if (!File::Exists(issuesJson) || now > File::GetLastWriteTime(issuesJson) + ONE_WEEK)
  {
    issues = MiKTeX::Setup::SetupService::FindIssues(false, false);
  }
  else
  {
    issues = MiKTeX::Setup::SetupService::GetIssues();
  }

  for (const Setup::Issue& issue : issues)
  {
    if (isLog4cxxConfigured)
    {
      if (issue.severity == Setup::IssueSeverity::Critical || issue.severity == Setup::IssueSeverity::Major)
      {
        LOG4CXX_FATAL(logger, issue);
      }
      else if (issue.severity == Setup::IssueSeverity::Major)
      {
        LOG4CXX_ERROR(logger, issue);
      }
      else
      {
        LOG4CXX_ERROR(logger, issue);
      }
    }
    if ((issue.severity == Setup::IssueSeverity::Critical || issue.severity == Setup::IssueSeverity::Major) && !GetQuietFlag())
    {
      cerr << Utils::GetExeName() << ": " << issue << "\n";
    }
  }
}

void Application::Init(const Session::InitInfo& initInfoArg)
{
#if defined(MIKTEX_WINDOWS)
  UINT activeOutputCodePage = GetConsoleOutputCP();
  if (activeOutputCodePage != CP_UTF8)
  {
    SetConsoleOutputCP(CP_UTF8);
  }
#endif
  instance = this;
  pimpl->initialized = true;
  Session::InitInfo initInfo(initInfoArg);
  initInfo.SetTraceCallback(this);
  pimpl->session = Session::Create(initInfo);
  pimpl->session->SetFindFileCallback(this);
  ConfigureLogging();
  auto thisProcess = Process::GetCurrentProcess();
  auto parentProcess = thisProcess->get_Parent();
  string invokerName;
  if (parentProcess != nullptr)
  {
    invokerName = parentProcess->get_ProcessName();
  }
  if (invokerName.empty())
  {
    invokerName = "unknown process";
  }
  if (pimpl->commandLine.empty())
  {
    // TODO
  }
  else
  {
    LOG4CXX_INFO(logger, "this process (" << thisProcess->GetSystemId() << ") started by '" << invokerName << "' with command line: " << pimpl->commandLine);
  }
#if defined(MIKTEX_WINDOWS)
  if (activeOutputCodePage != CP_UTF8)
  {
    LOG4CXX_DEBUG(logger, fmt::format("changed console output code page from {0} to {1}", activeOutputCodePage, GetConsoleOutputCP()));
  }
#endif
  pimpl->beQuiet = false;
  if (pimpl->enableInstaller == TriState::Undetermined)
  {
    pimpl->enableInstaller = pimpl->session->GetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOINSTALL).GetTriState();
  }
  pimpl->mpmAutoAdmin = pimpl->session->GetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOADMIN).GetTriState();
  if (pimpl->mpmAutoAdmin == TriState::True && !pimpl->session->IsSharedSetup())
  {
    LogWarn(T_("ignoring AutoAdmin=t because this is not a shared setup"));
    pimpl->mpmAutoAdmin = TriState::False;
  }
  InstallSignalHandler(SIGINT);
  InstallSignalHandler(SIGTERM);
  if (pimpl->enableMaintenance == TriState::Undetermined)
  {
    pimpl->enableMaintenance = TriState::True;
  }
  if (pimpl->enableDiagnose == TriState::Undetermined)
  {
    pimpl->enableDiagnose = TriState::True;
  }
  if (pimpl->session->RunningAsAdministrator() && !pimpl->session->IsAdminMode())
  {
    SecurityRisk(T_("running with elevated privileges"));
  }
  if (pimpl->enableMaintenance == TriState::True)
  {
    AutoMaintenance();
  }
}

void Application::Init(vector<const char*>& args)
{
  MIKTEX_ASSERT(!args.empty() && args.back() == nullptr);
  Init(Session::InitInfo(args[0]), args);
}

void Application::Init(vector<char*>& args)
{
  MIKTEX_ASSERT(!args.empty() && args.back() == nullptr);
  Init(Session::InitInfo(args[0]), args);
}

void Application::Init(const string& programInvocationName, const string& theNameOfTheGame)
{
  Session::InitInfo initInfo(programInvocationName);
  if (!theNameOfTheGame.empty())
  {
    initInfo.SetTheNameOfTheGame(theNameOfTheGame);
  }
  vector<const char*> args{ programInvocationName.c_str(), nullptr };
  Init(initInfo, args);
}

void Application::Init(const string& programInvocationName)
{
  Init(programInvocationName, "");
}

void Application::Finalize2(int exitCode)
{
  if (logger != nullptr)
  {
    auto thisProcess = Process::GetCurrentProcess();
    LOG4CXX_INFO(logger, "this process (" << thisProcess->GetSystemId() << ") finishes with exit code " << exitCode);
  }
  Finalize();
}

void Application::Finalize()
{
  if (pimpl->enableDiagnose == TriState::True)
  {
    AutoDiagnose();
  }
  FlushPendingTraceMessages();
  if (pimpl->installer != nullptr)
  {
    pimpl->installer->Dispose();
    pimpl->installer = nullptr;
  }
  if (pimpl->packageManager != nullptr)
  {
    pimpl->packageManager = nullptr;
  }
  pimpl->session = nullptr;
  pimpl->ignoredPackages.clear();
  if (initUiFrameworkDone)
  {
    MiKTeX::UI::FinalizeFramework();
    initUiFrameworkDone = false;
  }
  logger = nullptr;
  pimpl->initialized = false;
  instance = nullptr;
}

void Application::ReportLine(const string& str)
{
  MIKTEX_ASSERT(logger != nullptr);
  LOG4CXX_INFO(logger, "mpm: " << str);
  if (!GetQuietFlag())
  {
    cout << str << endl;
  }
}

bool Application::OnRetryableError(const string& message)
{
  UNUSED_ALWAYS(message);
  return false;
}

bool Application::OnProgress(Notification nf)
{
  UNUSED_ALWAYS(nf);
  return true;
}

MIKTEXAPPTHISAPI(void) Application::ShowLibraryVersions() const
{
  vector<LibraryVersion> versions;
  GetLibraryVersions(versions);
  for (auto& ver : set<LibraryVersion>(versions.begin(), versions.end()))
  {
    if (!ver.fromHeader.empty() && !ver.fromRuntime.empty())
    {
      cout << "compiled with " << ver.name << " version " << ver.fromHeader << "; using " << ver.fromRuntime << endl;
    }
    else if (!ver.fromHeader.empty())
    {
      cout << "compiled with " << ver.name << " version " << ver.fromHeader << endl;
    }
    else if (!ver.fromRuntime.empty())
    {
      cout << "using " << ver.name << " version " << ver.fromRuntime << endl;
    }
  }
}

const char* const SEP = "======================================================================";

bool Application::InstallPackage(const string& packageId, const PathName& trigger, PathName& installRoot)
{
  if (pimpl->ignoredPackages.find(packageId) != pimpl->ignoredPackages.end())
  {
    return false;
  }
  if (pimpl->enableInstaller == TriState::False)
  {
    return false;
  }
  if (pimpl->packageManager == nullptr)
  {
    pimpl->packageManager = PackageManager::Create(PackageManager::InitInfo(this));
  }
  if (pimpl->enableInstaller == TriState::Undetermined)
  {
    if (!initUiFrameworkDone)
    {
      MiKTeX::UI::InitializeFramework();
      initUiFrameworkDone = true;
    }
    bool doInstall = false;
    unsigned int msgBoxRet = MiKTeX::UI::InstallPackageMessageBox(pimpl->packageManager, packageId, trigger.ToString());
    doInstall = ((msgBoxRet & MiKTeX::UI::YES) != 0);
    if ((msgBoxRet & MiKTeX::UI::DONTASKAGAIN) != 0)
    {
      pimpl->enableInstaller = (doInstall ? TriState::True : TriState::False);
    }
    if (!doInstall)
    {
      pimpl->ignoredPackages.insert(packageId);
      return false;
    }
    pimpl->mpmAutoAdmin = (((msgBoxRet & MiKTeX::UI::ADMIN) != 0) ? TriState::True : TriState::False);
  }
  string url;
  RepositoryType repositoryType(RepositoryType::Unknown);
  ProxySettings proxySettings;
  if (PackageManager::TryGetDefaultPackageRepository(repositoryType, url)
    && repositoryType == RepositoryType::Remote
    && PackageManager::TryGetProxy(proxySettings)
    && proxySettings.useProxy
    && proxySettings.authenticationRequired
    && proxySettings.user.empty())
  {
    if (!initUiFrameworkDone)
    {
      MiKTeX::UI::InitializeFramework();
      initUiFrameworkDone = true;
    }
    if (!MiKTeX::UI::ProxyAuthenticationDialog())
    {
      return false;
    }
  }
  if (pimpl->installer == nullptr)
  {
    pimpl->installer = pimpl->packageManager->CreateInstaller();
  }
  pimpl->installer->SetCallback(this);
  vector<string> fileList;
  fileList.push_back(packageId);
  pimpl->installer->SetFileLists(fileList, vector<string>());
  LOG4CXX_INFO(logger, "installing package " << packageId << " triggered by " << trigger.ToString())
  if (!GetQuietFlag())
  {
    cout << "\n" << SEP << endl;
  }
  bool done = false;
  bool switchToAdminMode = (pimpl->mpmAutoAdmin == TriState::True && !pimpl->session->IsAdminMode());
  if (switchToAdminMode)
  {
    pimpl->session->SetAdminMode(true);
  }
  try
  {
    pimpl->installer->InstallRemove(PackageInstaller::Role::Application);
    installRoot = pimpl->session->GetSpecialPath(SpecialPath::InstallRoot);
    done = true;
  }
  catch (const MiKTeXException& ex)
  {
    pimpl->enableInstaller = TriState::False;
    pimpl->ignoredPackages.insert(packageId);
    LOG4CXX_FATAL(logger, ex.GetErrorMessage());
    LOG4CXX_FATAL(logger, "Info: " << ex.GetInfo());
    LOG4CXX_FATAL(logger, "Source: " << ex.GetSourceFile());
    LOG4CXX_FATAL(logger, "Line: " << ex.GetSourceLine());
    cerr
      << "\n"
      << "Unfortunately, the package " << packageId << " could not be installed." << endl;
    log4cxx::RollingFileAppenderPtr appender = log4cxx::Logger::getRootLogger()->getAppender(LOG4CXX_STR("RollingLogFile"));
    if (appender != nullptr)
    {
      cerr
        << "Please check the log file:" << "\n"
        << PathName(appender->getFile()) << endl;
    }
  }
  if (switchToAdminMode)
  {
    pimpl->session->SetAdminMode(false);
  }
  if (!GetQuietFlag())
  {
    cout << SEP << endl;
  }
  return done;
}

bool Application::TryCreateFile(const PathName& fileName, FileType fileType)
{
  vector<string> args{ "" };
  switch (pimpl->enableInstaller)
  {
  case TriState::False:
    args.push_back("--disable-installer");
    break;
  case TriState::True:
    args.push_back("--enable-installer");
    break;
  case TriState::Undetermined:
    break;
  }
  if (pimpl->session->IsAdminMode())
  {
    args.push_back("--admin");
  }
  PathName makeUtility;
  PathName baseName = fileName.GetFileNameWithoutExtension();
  switch (fileType)
  {
  case FileType::BASE:
  case FileType::FMT:
    if (!pimpl->session->FindFile(MIKTEX_INITEXMF_EXE, FileType::EXE, makeUtility))
    {
      MIKTEX_FATAL_ERROR(T_("The MiKTeX configuration utility (initexmf) could not be found."));
    }
    args.push_back("--dump-by-name="s + baseName.ToString());
    if (fileType == FileType::FMT)
    {
      args.push_back("--engine="s + pimpl->session->GetEngineName());
    }
    break;
  case FileType::TFM:
    if (!pimpl->session->FindFile(MIKTEX_MAKETFM_EXE, FileType::EXE, makeUtility))
    {
      MIKTEX_FATAL_ERROR(T_("The MakeTFM utility could not be found."));
    }
    args.push_back(baseName.ToString());
    break;
  default:
    return false;
  }
  LOG4CXX_INFO(logger, "going to create file: " << fileName);
  ProcessOutput<50000> processOutput;
  int exitCode;
  args[0] = makeUtility.GetFileNameWithoutExtension().ToString();
  if (!Process::Run(makeUtility, args, &processOutput, &exitCode, nullptr))
  {
    LOG4CXX_ERROR(logger, makeUtility << " could not be started");
    return false;
  }
  if (exitCode != 0)
  {
    LOG4CXX_ERROR(logger, makeUtility << " did not succeed; exitCode: " << exitCode);
    LOG4CXX_ERROR(logger, "output:");
    LOG4CXX_ERROR(logger, processOutput.StdoutToString());
    return false;
  }
  return true;
}

void Application::EnableInstaller(TriState tri)
{
  pimpl->enableInstaller = tri;
}

TriState Application::GetEnableInstaller() const
{
  return pimpl->enableInstaller;
}

bool Application::Trace(const TraceCallback::TraceMessage& traceMessage)
{
  if (!isLog4cxxConfigured)
  {
    if (pimpl->pendingTraceMessages.size() > 100)
    {
      pimpl->pendingTraceMessages.clear();
    }
    pimpl->pendingTraceMessages.push_back(traceMessage);
    return true;
  }
  FlushPendingTraceMessages();
  TraceInternal(traceMessage);
  return true;
}

void Application::FlushPendingTraceMessages()
{
  for (const TraceCallback::TraceMessage& m : pimpl->pendingTraceMessages)
  {
    TraceInternal(m);
  }
  pimpl->pendingTraceMessages.clear();
}

void Application::TraceInternal(const TraceCallback::TraceMessage& traceMessage)
{
  if (isLog4cxxConfigured)
  {
    log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(string("trace.") + Utils::GetExeName() + "." + traceMessage.facility);
    switch (traceMessage.level)
    {
    case TraceLevel::Fatal:
      LOG4CXX_FATAL(logger, traceMessage.message);
      break;
    case TraceLevel::Error:
      LOG4CXX_ERROR(logger, traceMessage.message);
      break;
    case TraceLevel::Warning:
      LOG4CXX_WARN(logger, traceMessage.message);
      break;
    case TraceLevel::Info:
      LOG4CXX_INFO(logger, traceMessage.message);
      break;
    case TraceLevel::Trace:
      LOG4CXX_TRACE(logger, traceMessage.message);
      break;
    case TraceLevel::Debug:
    default:
      LOG4CXX_DEBUG(logger, traceMessage.message);
      break;
    }
  }
  else
  {
    cerr << traceMessage << endl;
  }
}

void Application::Sorry(const string& name, const string& description, const string& remedy, const string& url)
{
  if (cerr.fail())
  {
    return;
  }
  cerr << endl;
  if (description.empty())
  {
    cerr << fmt::format(T_("Sorry, but {0} did not succeed."), Q_(name)) << endl;
  }
  else
  {
    cerr
      << fmt::format(T_("Sorry, but {0} did not succeed for the following reason:"), Q_(name)) << "\n"
      << "\n"
      << "  " << description << endl;
    if (!remedy.empty())
    {
      cerr
        << "\n"
        << T_("Remedy:") << "\n"
        << "\n"
        << "  " << remedy
        << endl;
    }
  }
  if (isLog4cxxConfigured)
  {
    log4cxx::RollingFileAppenderPtr appender = log4cxx::Logger::getRootLogger()->getAppender(LOG4CXX_STR("RollingLogFile"));
    if (appender != nullptr)
    {
      cerr
        << "\n"
        << "The log file hopefully contains the information to get MiKTeX going again:" << "\n"
        << "\n"
        << "  " << PathName(appender->getFile())
        << endl;
    }
  }
  if (!url.empty())
  {
    cerr
      << "\n"
      << T_("For more information, visit:") << " " << url
      << endl;
  }
}

void Application::Sorry(const string& name, const MiKTeXException& ex)
{
  if (logger != nullptr)
  {
    LOG4CXX_FATAL(logger, ex.GetErrorMessage());
    LOG4CXX_FATAL(logger, "Info: " << ex.GetInfo());
    LOG4CXX_FATAL(logger, "Source: " << ex.GetSourceFile());
    LOG4CXX_FATAL(logger, "Line: " << ex.GetSourceLine());
  }
  else
  {
    Utils::PrintException(ex);
  }
  Sorry(name, ex.GetDescription(), ex.GetRemedy(), ex.GetUrl());
}

void Application::Sorry(const string& name, const exception& ex)
{
  if (logger != nullptr)
  {
    LOG4CXX_FATAL(logger, ex.what());
  }
  else
  {
    cerr
      << "ERROR: " << ex.what()
      << endl;
  }
  Sorry(name);
}

MIKTEXNORETURN void Application::FatalError(const string& s)
{
  if (logger != nullptr)
  {
    LOG4CXX_FATAL(logger, s);
  }
  Sorry(Utils::GetExeName(), s);
  throw 1;

}
void Application::Warning(const string& s)
{
  LogWarn(s);
  if (!pimpl->beQuiet)
  {
    cerr << Utils::GetExeName() << ": " << T_("warning") << ": " << s << endl;
  }
}

void Application::SecurityRisk(const string& s)
{
  LogWarn(T_("security risk") + ": "s + s);
  if (!pimpl->beQuiet)
  {
    cerr << Utils::GetExeName() << ": " << T_("security risk") << ": " << s << endl;
  }
}

void Application::InvokeEditor(const PathName& editFileName, int editLineNumber, FileType editFileType, const PathName& transcriptFileName) const
{
  string defaultEditor;

  PathName texworks;
  if (pimpl->session->FindFile(MIKTEX_TEXWORKS_EXE, FileType::EXE, texworks))
  {
    defaultEditor = Q_(texworks);
    defaultEditor += " -p=%l \"%f\"";
  }
  else
  {
    defaultEditor = "notepad \"%f\"";
  }

  if (!pimpl->session->IsAdminMode())
  {
    // read information from yap.ini
    // FIXME: use FindFile()
    PathName yapIni = pimpl->session->GetSpecialPath(SpecialPath::UserConfigRoot) / PathName(MIKTEX_PATH_MIKTEX_CONFIG_DIR) / PathName(MIKTEX_YAP_INI_FILENAME);
    if (File::Exists(yapIni))
    {
      unique_ptr<Cfg> yapConfig(Cfg::Create());
      yapConfig->Read(yapIni);
      string yapEditor;
      if (yapConfig->TryGetValueAsString("Settings", "Editor", yapEditor))
      {
        defaultEditor = yapEditor;
      }
    }
  }

  string templ = pimpl->session->GetConfigValue(MIKTEX_CONFIG_SECTION_GENERAL, MIKTEX_CONFIG_VALUE_EDITOR, ConfigValue(defaultEditor)).GetString();

  const char* lpszCommandLineTemplate = templ.c_str();

  string fileName;
  string commandLine;

  bool quoted = false;

  for (; *lpszCommandLineTemplate != ' ' || (*lpszCommandLineTemplate != 0 && quoted); ++lpszCommandLineTemplate)
  {
    commandLine += *lpszCommandLineTemplate;
    if (*lpszCommandLineTemplate == '"')
    {
      quoted = !quoted;
    }
    else
    {
      fileName += *lpszCommandLineTemplate;
    }
  }

  for (; *lpszCommandLineTemplate == ' '; ++lpszCommandLineTemplate)
  {
    commandLine += *lpszCommandLineTemplate;
  }

  while (*lpszCommandLineTemplate != 0)
  {
    if (lpszCommandLineTemplate[0] == '%' && lpszCommandLineTemplate[1] != 0)
    {
      switch (lpszCommandLineTemplate[1])
      {
      default:
        break;
      case '%':
        commandLine += '%';
        break;
      case 'f':
      {
        PathName path;
        if (pimpl->session->FindFile(editFileName.ToString(), editFileType, path))
        {
          commandLine += path.GetData();
        }
        else
        {
          commandLine += editFileName.GetData();
        }
        break;
      }
      case 'h':
        // TODO
        break;
      case 't':
        commandLine += transcriptFileName.GetData();
        break;
      case 'l':
        commandLine += std::to_string(editLineNumber);
        break;
      case 'm':
        // TODO
        break;
      }
      lpszCommandLineTemplate += 2;
    }
    else
    {
      commandLine += *lpszCommandLineTemplate;
      ++lpszCommandLineTemplate;
    }
  }

  Process::Start(PathName(fileName), Argv(commandLine).ToStringVector());
}

bool Application::GetQuietFlag() const
{
  return pimpl->beQuiet;
}

void Application::SetQuietFlag(bool b)
{
  pimpl->beQuiet = b;
}

shared_ptr<Session> Application::GetSession() const
{
  if (!pimpl->session)
  {
    MIKTEX_UNEXPECTED();
  }
  return pimpl->session;
}

void Application::LogInfo(const std::string& message) const
{
  if (logger != nullptr)
  {
    LOG4CXX_INFO(logger, message);
  }
}

void Application::LogWarn(const std::string& message) const
{
  if (logger != nullptr)
  {
    LOG4CXX_WARN(logger, message);
  }
}

void Application::LogError(const std::string& message) const
{
  if (logger != nullptr)
  {
    LOG4CXX_ERROR(logger, message);
  }
}
