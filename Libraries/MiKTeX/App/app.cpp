/* app.cpp:

   Copyright (C) 2005-2016 Christian Schenk
 
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

#include "StdAfx.h"

#include "internal.h"

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Util;
using namespace std;

static log4cxx::LoggerPtr logger;

static Application * instance = nullptr;

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
    string programInvocationName = Utils::GetExeName();
    throw MiKTeXException(programInvocationName.c_str(), T_("The current operation has been cancelled (Ctrl-C)."), MiKTeXException::KVMAP(), SourceLocation());
  }
}

Application * Application::GetApplication()
{
  return instance;
}

Application::Application() :
  enableInstaller(TriState::Undetermined),
  autoAdmin(TriState::Undetermined),
  initialized(false)
{
}

Application::~Application()
{
  try
  {
    if (initialized)
    {
      Finalize();
    }
    FlushPendingTraceMessages();
  }
  catch (const exception &)
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

void Application::Init(const Session::InitInfo & initInfo_)
{
  instance = this;
  initialized = true;
  Session::InitInfo initInfo(initInfo_);
  initInfo.SetTraceCallback(this);
  session = Session::Create(initInfo);
  session->SetFindFileCallback(this);
  string myName = Utils::GetExeName();
  PathName xmlFileName;
  if (session->FindFile((myName + "." + MIKTEX_LOG4CXX_CONFIG_FILENAME).c_str(), MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName)
    || session->FindFile(MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName))
  {
    Utils::SetEnvironmentString("MIKTEX_LOG_DIR", PathName(session->GetSpecialPath(SpecialPath::DataRoot), MIKTEX_PATH_MIKTEX_LOG_DIR).Get());
    Utils::SetEnvironmentString("MIKTEX_LOG_NAME", myName.c_str());
    log4cxx::xml::DOMConfigurator::configure(xmlFileName.ToWideCharString());
    isLog4cxxConfigured = true;
  }
  logger = log4cxx::Logger::getLogger(myName);
  beQuiet = false;
  if (enableInstaller == TriState::Undetermined)
  {
    enableInstaller = session->GetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_AUTO_INSTALL, TriState(TriState::Undetermined));
  }
  autoAdmin = session->GetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_AUTO_ADMIN, TriState(TriState::Undetermined));
  InstallSignalHandler(SIGINT);
  InstallSignalHandler(SIGTERM);
  time_t lastAdminMaintenance = static_cast<time_t>(std::stoll(session->GetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_LAST_ADMIN_MAINTENANCE, "0")));
  PathName mpmDatabasePath(session->GetMpmDatabasePathName());
  bool mustRefreshFndb = !File::Exists(mpmDatabasePath) || (!session->IsAdminMode() && lastAdminMaintenance + 30 > File::GetLastWriteTime(mpmDatabasePath));
  PathName userLanguageDat = session->GetSpecialPath(SpecialPath::UserConfigRoot);
  userLanguageDat /= MIKTEX_PATH_LANGUAGE_DAT;
  bool mustRefreshUserLanguageDat = !session->IsAdminMode() && File::Exists(userLanguageDat) && lastAdminMaintenance + 30 > File::GetLastWriteTime(userLanguageDat);
  PathName initexmf;
  if ((mustRefreshFndb || mustRefreshUserLanguageDat) && session->FindFile(MIKTEX_INITEXMF_EXE, FileType::EXE, initexmf))
  {
    CommandLineBuilder commandLine;
    switch (enableInstaller)
    {
    case TriState::False:
      commandLine.AppendOption("--disable-installer");
      break;
    case TriState::True:
      commandLine.AppendOption("--enable-installer");
      break;
    }
    if (session->IsAdminMode())
    {
      commandLine.AppendOption("--admin");
    }
    commandLine.AppendArgument("--quiet");
    if (mustRefreshFndb)
    {
      session->UnloadFilenameDatabase();
      CommandLineBuilder xCommandLine(commandLine);
      xCommandLine.AppendOption("--update-fndb");
      LOG4CXX_INFO(logger, "running 'initexmf " << xCommandLine.ToString() << "' to refresh the file name database");
      Process::Run(initexmf, xCommandLine.ToString());
    }
    if (mustRefreshUserLanguageDat)
    {
      MIKTEX_ASSERT(!session->IsAdminMode());
      CommandLineBuilder xCommandLine(commandLine);
      xCommandLine.AppendOption("--mklangs");
      LOG4CXX_INFO(logger, "running 'initexmf " << xCommandLine.ToString() << "' to refresh language.dat");
      Process::Run(initexmf, xCommandLine.ToString());
    }
  }
}

void Application::Init(vector<char *> & args)
{
  Session::InitInfo initInfo(args[0]);
  vector<char *>::iterator it = args.begin();
  while (it != args.end() && *it != nullptr)
  {
    bool keepArgument = false;
    if (strcmp(*it, "--miktex-admin") == 0)
    {
      initInfo.AddOption(Session::InitOption::AdminMode);
    }
    else if (strcmp(*it, "--miktex-disable-installer") == 0)
    {
      enableInstaller = TriState::False;
    }
    else if (strcmp(*it, "--miktex-enable-installer") == 0)
    {
      enableInstaller = TriState::True;
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
  Init(initInfo);
}

void Application::Init(const string & programInvocationName, const string & theNameOfTheGame)
{
  Session::InitInfo initInfo(programInvocationName);
  if (!theNameOfTheGame.empty())
  {
    initInfo.SetTheNameOfTheGame(theNameOfTheGame);
  }
#if defined(MIKTEX_WINDOWS) && 0
  initInfo.AddOption(Session::InitOption::InitializeCOM);
#endif
  Init(initInfo);
}

void Application::Init(const string & programInvocationName)
{
  Init(programInvocationName, "");
}

void Application::Finalize()
{
  if (installer != nullptr)
  {
    installer->Dispose();
    installer = nullptr;
  }
  if (packageManager != nullptr)
  {
    packageManager = nullptr;
  }
  session = nullptr;
  ignoredPackages.clear();
  if (initUiFrameworkDone)
  {
    MiKTeX::UI::FinalizeFramework();
    initUiFrameworkDone = false;
  }
  logger = nullptr;
  initialized = false;
}

void Application::ReportLine(const string & str)
{
  LOG4CXX_INFO(logger, "mpm: " << str);
  if (!GetQuietFlag())
  {
    fputs(str.c_str(), stdout);
    putc('\n', stdout);
  }
}

bool Application::OnRetryableError(const string & message)
{
  UNUSED_ALWAYS(message);
  return false;
}

bool Application::OnProgress(Notification nf)
{
  UNUSED_ALWAYS(nf);
  return true;
}

MIKTEXAPPTHISAPI(void) Application::GetLibraryVersions(std::vector<LibraryVersion>& versions) const
{
}

MIKTEXAPPTHISAPI(void) Application::ShowLibraryVersions() const
{
  vector<LibraryVersion> versions;
  GetLibraryVersions(versions);
  for (vector<LibraryVersion>::const_iterator it = versions.begin(); it != versions.end(); ++it)
  {
    if (!it->fromHeader.empty() && !it->fromRuntime.empty())
    {
      cout << "compiled with " << it->name << " version " << it->fromHeader << "; using " << it->fromRuntime << endl;
    }
    else if (!it->fromHeader.empty())
    {
      cout << "compiled with " << it->name << " version " << it->fromHeader << endl;
    }
    else if (!it->fromRuntime.empty())
    {
      cout << "using " << it->name << " version " << it->fromRuntime << endl;
    }
  }
}

const char * const SEP = "======================================================================";

bool Application::InstallPackage(const string & deploymentName, const PathName & trigger, PathName & installRoot)
{
  if (ignoredPackages.find(deploymentName) != ignoredPackages.end())
  {
    return false;
  }
  if (enableInstaller == TriState::False)
  {
    return false;
  }
  if (packageManager == nullptr)
  {
    packageManager = PackageManager::Create();
  }
  if (enableInstaller == TriState::Undetermined)
  {
    if (!initUiFrameworkDone)
    {
      MiKTeX::UI::InitializeFramework();
      initUiFrameworkDone = true;
    }
    bool doInstall = false;
    unsigned int msgBoxRet = MiKTeX::UI::InstallPackageMessageBox(packageManager, deploymentName.c_str(), trigger.Get());
    doInstall = ((msgBoxRet & MiKTeX::UI::YES) != 0);
    if ((msgBoxRet & MiKTeX::UI::DONTASKAGAIN) != 0)
    {
      enableInstaller = (doInstall ? TriState::True : TriState::False);
    }
    if (!doInstall)
    {
      ignoredPackages.insert(deploymentName);
      return false;
    }
    autoAdmin = (((msgBoxRet & MiKTeX::UI::ADMIN) != 0) ? TriState::True : TriState::False);
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
  if (installer == nullptr)
  {
    installer = packageManager->CreateInstaller();
  }
  installer->SetCallback(this);
  vector<string> fileList;
  fileList.push_back(deploymentName);
  installer->SetFileLists(fileList, vector<string>());
  LOG4CXX_INFO(logger, "installing package " << deploymentName << " triggered by " << trigger.ToString())
  if (!GetQuietFlag())
  {
    cout << endl << SEP << endl;
  }
  bool done = false;
  bool switchToAdminMode = (autoAdmin == TriState::True && !session->IsAdminMode());
  if (switchToAdminMode)
  {
    session->SetAdminMode(true);
  }
  try
  {
    installer->InstallRemove();
    installRoot = session->GetSpecialPath(SpecialPath::InstallRoot);
    done = true;
  }
  catch (const MiKTeXException & ex)
  {
    enableInstaller = TriState::False;
    ignoredPackages.insert(deploymentName);
    LOG4CXX_FATAL(logger, ex.what());
    LOG4CXX_FATAL(logger, "Info: " << ex.GetInfo());
    LOG4CXX_FATAL(logger, "Source: " << ex.GetSourceFile());
    LOG4CXX_FATAL(logger, "Line: " << ex.GetSourceLine());
    cerr << endl << "Unfortunately, the package " << deploymentName << " could not be installed.";
    log4cxx::RollingFileAppenderPtr appender = log4cxx::Logger::getRootLogger()->getAppender(LOG4CXX_STR("RollingLogFile"));
    if (appender != nullptr)
    {
      cerr << "Please check the log file:" << endl << PathName(appender->getFile()).ToUnix().ToString() << endl;
    }
  }
  if (switchToAdminMode)
  {
    session->SetAdminMode(false);
  }
  if (!GetQuietFlag())
  {
    cout << SEP << endl;
  }
  return done;
}

bool Application::TryCreateFile(const PathName & fileName, FileType fileType)
{
  CommandLineBuilder commandLine;
  switch (enableInstaller)
  {
  case TriState::False:
    commandLine.AppendOption("--disable-installer");
    break;
  case TriState::True:
    commandLine.AppendOption("--enable-installer");
    break;
  }
  if (session->IsAdminMode())
  {
    commandLine.AppendOption("--admin");
  }
  PathName makeUtility;
  PathName baseName = fileName.GetFileNameWithoutExtension();
  switch (fileType)
  {
  case FileType::BASE:
  case FileType::FMT:
    if (!session->FindFile(MIKTEX_INITEXMF_EXE, FileType::EXE, makeUtility))
    {
      MIKTEX_UNEXPECTED();
    }
    commandLine.AppendOption("--dump-by-name=", baseName);
    if (fileType == FileType::FMT)
    {
      commandLine.AppendOption("--engine=", session->GetEngineName());
    }
    break;
  case FileType::TFM:
    if (!session->FindFile(MIKTEX_MAKETFM_EXE, FileType::EXE, makeUtility))
    {
      MIKTEX_UNEXPECTED();
    }
    commandLine.AppendArgument(baseName);
    break;
  default:
    return false;
  }
  LOG4CXX_INFO(logger, "going to create file: " << fileName);
  ProcessOutput<50000> processOutput;
  int exitCode;
  if (!Process::Run(makeUtility, commandLine.ToString(), &processOutput, &exitCode, nullptr))
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
  enableInstaller = tri;
}

void Application::Trace(const TraceCallback::TraceMessage & traceMessage)
{
  if (!isLog4cxxConfigured)
  {
    if (pendingTraceMessages.size() > 100)
    {
      pendingTraceMessages.clear();
    }
    pendingTraceMessages.push_back(traceMessage);
    return;
  }
  FlushPendingTraceMessages();
  TraceInternal(traceMessage);
}

void Application::FlushPendingTraceMessages()
{
  for (const TraceCallback::TraceMessage & m : pendingTraceMessages)
  {
    TraceInternal(m);
  }
  pendingTraceMessages.clear();
}

void Application::TraceInternal(const TraceCallback::TraceMessage & traceMessage)
{
  if (isLog4cxxConfigured)
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

void Application::Sorry(const string & name, const string & reason)
{
  if (cerr.fail())
  {
    return;
  }
  cerr << endl;
  if (reason.empty())
  {
    cerr << StringUtil::FormatString(T_("Sorry, but %s did not succeed."), Q_(name)) << endl;
  }
  else
  {
    cerr
      << StringUtil::FormatString(T_("Sorry, but %s did not succeed for the following reason:"), Q_(name)) << endl << endl
      << "  " << reason << endl;
  }
  log4cxx::RollingFileAppenderPtr appender = log4cxx::Logger::getRootLogger()->getAppender(LOG4CXX_STR("RollingLogFile"));
  if (appender != nullptr)
  {
    cerr
      << endl
      << "The log file hopefully contains the information to get MiKTeX going again:" << endl
      << endl
      << "  " << PathName(appender->getFile()).ToUnix().Get() << endl;
  }
  cerr
    << endl
    << T_("You may want to visit the MiKTeX project page, if you need help.") << endl;
}

void Application::Sorry(const string & name, const MiKTeXException & ex)
{
  if (logger != nullptr)
  {
    LOG4CXX_FATAL(logger, ex.what());
    LOG4CXX_FATAL(logger, "Info: " << ex.GetInfo());
    LOG4CXX_FATAL(logger, "Source: " << ex.GetSourceFile());
    LOG4CXX_FATAL(logger, "Line: " << ex.GetSourceLine());
  }
  Sorry(name);
}

void Application::Sorry(const string & name, const exception & ex)
{
  if (logger != nullptr)
  {
    LOG4CXX_FATAL(logger, ex.what());
  }
  Sorry(name);
}

MIKTEXNORETURN void Application::FatalError(const char * lpszFormat, ...)
{
  va_list arglist;
  va_start(arglist, lpszFormat);
  string s;
  try
  {
    s = StringUtil::FormatString(lpszFormat, arglist);
  }
  catch (...)
  {
    va_end(arglist);
    throw;
  }
  va_end(arglist);
  if (logger != nullptr)
  {
    LOG4CXX_FATAL(logger, s);
  }
  Sorry(Utils::GetExeName(), s);
  throw 1;
}
