/* app.cpp:

   Copyright (C) 2005-2018 Christian Schenk
 
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
#include <miktex/Core/Registry>
#include <miktex/Core/Session>
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
  shared_ptr<Session> session;
public:
  bool isLog4cxxConfigured = false;
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
    FlushPendingTraceMessages();
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
    pimpl->isLog4cxxConfigured = true;
  }
  else
  {
    log4cxx::BasicConfigurator::configure();
  }
  logger = log4cxx::Logger::getLogger(myName);
}

void Application::AutoMaintenance()
{
  time_t lastAdminMaintenance = static_cast<time_t>(std::stoll(pimpl->session->GetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_LAST_ADMIN_MAINTENANCE, "0").GetString()));
  time_t lastUserMaintenance = static_cast<time_t>(std::stoll(pimpl->session->GetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_LAST_USER_MAINTENANCE, "0").GetString()));
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
  PathName mpmDatabasePath(pimpl->session->GetMpmDatabasePathName());
  bool mustRefreshFndb = !File::Exists(mpmDatabasePath) || (!pimpl->session->IsAdminMode() && lastAdminMaintenance + 30 > File::GetLastWriteTime(mpmDatabasePath));
  PathName userLanguageDat = pimpl->session->IsAdminMode() ? "" : pimpl->session->GetSpecialPath(SpecialPath::UserConfigRoot) / MIKTEX_PATH_LANGUAGE_DAT;
  bool mustRefreshUserLanguageDat = !pimpl->session->IsAdminMode() && File::Exists(userLanguageDat) && lastAdminMaintenance + 30 > File::GetLastWriteTime(userLanguageDat);
  PathName initexmf;
  if ((mustRefreshFndb || mustRefreshUserLanguageDat) && pimpl->session->FindFile(MIKTEX_INITEXMF_EXE, FileType::EXE, initexmf))
  {
    PathName lockdir = pimpl->session->GetSpecialPath(SpecialPath::DataRoot) / MIKTEX_PATH_MIKTEX_DIR / "locks";
    unique_ptr<MiKTeX::Core::LockFile> lockFile = LockFile::Create(lockdir / "A6D646EE9FBF44D6A3E6C1A3A72FF7E3.lock");
    if (!lockFile->TryLock(0ms))
    {
      return;
    }
    LOG4CXX_TRACE(logger, "running MIKTEX_HOOK_AUTO_MAINTENANCE")
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
    if (mustRefreshFndb)
    {
      vector<string> args = commonArgs;
      args.push_back("--update-fndb");
      LOG4CXX_INFO(logger, "running 'initexmf' to refresh the file name database");
      pimpl->session->UnloadFilenameDatabase();
      Process::Run(initexmf, args);
    }
    if (mustRefreshFndb)
    {
      vector<string> args = commonArgs;
      args.push_back("--mkmaps");
      LOG4CXX_INFO(logger, "running 'initexmf' to create font map files");
      Process::Run(initexmf, args);
    }
    if (mustRefreshUserLanguageDat)
    {
      MIKTEX_ASSERT(!pimpl->session->IsAdminMode());
      vector<string> args = commonArgs;
      args.push_back("--mklangs");
      LOG4CXX_INFO(logger, "running 'initexmf' to refresh language.dat");
      Process::Run(initexmf, args);
    }
  }
}

void Application::Init(const Session::InitInfo& initInfoArg)
{
  instance = this;
  pimpl->initialized = true;
  Session::InitInfo initInfo(initInfoArg);
  initInfo.SetTraceCallback(this);
  pimpl->session = Session::Create(initInfo);
  pimpl->session->SetFindFileCallback(this);
  ConfigureLogging();
  if (pimpl->commandLine.empty())
  {
    // TODO
  }
  else
  {
    LOG4CXX_INFO(logger, "starting with command line: " << pimpl->commandLine);
  }
  pimpl->beQuiet = false;
  if (pimpl->enableInstaller == TriState::Undetermined)
  {
    pimpl->enableInstaller = pimpl->session->GetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOINSTALL).GetTriState();
  }
  pimpl->mpmAutoAdmin = pimpl->session->GetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOADMIN).GetTriState();
  InstallSignalHandler(SIGINT);
  InstallSignalHandler(SIGTERM);
  if (pimpl->enableMaintenance == TriState::Undetermined)
  {
    pimpl->enableMaintenance = TriState::True;
  }
  if (pimpl->session->RunningAsAdministrator() && !pimpl->session->IsAdminMode())
  {
    Warning(T_("running with administrator privileges"));
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
    LOG4CXX_INFO(logger, "finishing with exit code " << exitCode);
  }
  Finalize();
}
  
void Application::Finalize()
{
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
}

void Application::ReportLine(const string& str)
{
  LOG4CXX_INFO(logger, "mpm: " << str);
  if (!GetQuietFlag())
  {
    fputs(str.c_str(), stdout);
    putc('\n', stdout);
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
    pimpl->packageManager = PackageManager::Create();
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

void Application::Trace(const TraceCallback::TraceMessage& traceMessage)
{
  if (!pimpl->isLog4cxxConfigured)
  {
    if (pimpl->pendingTraceMessages.size() > 100)
    {
      pimpl->pendingTraceMessages.clear();
    }
    pimpl->pendingTraceMessages.push_back(traceMessage);
    return;
  }
  FlushPendingTraceMessages();
  TraceInternal(traceMessage);
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
  if (pimpl->isLog4cxxConfigured)
  {
    log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(string("trace.") + Utils::GetExeName() + "." + traceMessage.facility);
    if (traceMessage.streamName == MIKTEX_TRACE_ERROR)
    {
      LOG4CXX_ERROR(logger, traceMessage.message);
    }
    else
    {
      LOG4CXX_TRACE(logger, traceMessage.message);
    }
  }
  else
  {
    cerr << traceMessage.message << endl;
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
    cerr << StringUtil::FormatString(T_("Sorry, but %s did not succeed."), Q_(name)) << endl;
  }
  else
  {
    cerr
      << StringUtil::FormatString(T_("Sorry, but %s did not succeed for the following reason:"), Q_(name)) << "\n"
      << "\n"
      << "  " << description << endl;
    if (!remedy.empty())
    {
      cerr
        << "\n"
        << T_("Remedy:") << "\n"
        << "\n"
        << "  " << remedy << endl;
    }
  }
  if (instance != nullptr && instance->pimpl->isLog4cxxConfigured)
  {
    log4cxx::RollingFileAppenderPtr appender = log4cxx::Logger::getRootLogger()->getAppender(LOG4CXX_STR("RollingLogFile"));
    if (appender != nullptr)
    {
      cerr
        << "\n"
        << "The log file hopefully contains the information to get MiKTeX going again:" << "\n"
        << "\n"
        << "  " << PathName(appender->getFile()) << endl;
    }
  }
  if (!url.empty())
  {
    cerr
      << "\n"
      << T_("For more information, visit:") << " " << url << endl;
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
    cerr
      << "ERROR: " << ex.GetErrorMessage() << "\n"
      << "ERROR: Info: " << ex.GetInfo() << "\n"
      << "ERROR: Source: " << ex.GetSourceFile() << "\n"
      << "ERROR: Line: " << ex.GetSourceLine() << "\n";
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
      << "ERROR: " << ex.what() << "\n";
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
    PathName yapIni = pimpl->session->GetSpecialPath(SpecialPath::UserConfigRoot) / MIKTEX_PATH_MIKTEX_CONFIG_DIR / MIKTEX_YAP_INI_FILENAME;
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

  string templ = pimpl->session->GetConfigValue("", MIKTEX_REGVAL_EDITOR, defaultEditor).GetString();

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

  Process::Start(fileName, Argv(commandLine).ToStringVector());
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
  if (pimpl->isLog4cxxConfigured)
  {
    LOG4CXX_INFO(logger, message);
  }
}

void Application::LogWarn(const std::string& message) const
{
  if (pimpl->isLog4cxxConfigured)
  {
    LOG4CXX_WARN(logger, message);
  }
}

void Application::LogError(const std::string& message) const
{
  if (pimpl->isLog4cxxConfigured)
  {
    LOG4CXX_ERROR(logger, message);
  }
}
