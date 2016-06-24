/* mpm.cpp: MiKTeX Package Manager (cli version)

   Copyright (C) 2003-2016 Christian Schenk

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

#include "StdAfx.h"

#include "internal.h"

#if !defined(THE_NAME_OF_THE_GAME)
#  define THE_NAME_OF_THE_GAME T_("MiKTeX Package Manager")
#endif

#if defined(MIKTEX_WINDOWS)
#  define ENABLE_OPT_INSTALL_ROOT 0
#else
#  define ENABLE_OPT_INSTALL_ROOT 1
#endif

#if defined(MIKTEX_WINDOWS)
const char PATH_DELIMITER = ';';
#define PATH_DELIMITER_STRING ";"
#else
const char PATH_DELIMITER = ':';
#define PATH_DELIMITER_STRING ":"
#endif

const char * DEFAULT_TRACE_STREAMS =
MIKTEX_TRACE_CORE ","
MIKTEX_TRACE_CURL ","
MIKTEX_TRACE_ERROR ","
MIKTEX_TRACE_FNDB ","
MIKTEX_TRACE_MPM
;

static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("mpmcli"));

enum class OutputFormat
{
  None = 0,
  Listing,
  CSV,
  DeploymentNames,
};

enum class SortKey
{
  None = 0,
  DeploymentName,
  InstalledOn,
  PackagedOn,
};

class PackageInfoComparer
{
public:
  bool operator() (const PackageInfo & pi1, const PackageInfo & pi2) const
  {
    bool cmp;
    switch (sortKey)
    {
    case SortKey::DeploymentName:
      cmp = (PathName::Compare(pi1.deploymentName, pi2.deploymentName) < 0);
      break;
    case SortKey::InstalledOn:
      cmp = (pi1.timeInstalled < pi2.timeInstalled);
      break;
    case SortKey::PackagedOn:
      cmp = (pi1.timePackaged < pi2.timePackaged);
      break;
    default:
      cmp = false;
      break;
    }
    return reverse ? !cmp : cmp;
  }
public:
  static SortKey sortKey;
public:
  static bool reverse;
};

SortKey PackageInfoComparer::sortKey(SortKey::DeploymentName);

bool PackageInfoComparer::reverse = false;

class UpdateInfoComparer
{
public:
  bool operator() (const PackageInstaller::UpdateInfo & ui1, const PackageInstaller::UpdateInfo & ui2) const
  {
    return PathName::Compare(ui1.deploymentName, ui2.deploymentName) < 0;
  }
};

class Application :
  public PackageInstallerCallback,
  public TraceCallback
{
private:
  void InstallSignalHandler(int sig)
  {
    void(*oldHandlerFunc) (int);
    oldHandlerFunc = signal(sig, Application::SignalHandler);
    if (oldHandlerFunc == SIG_ERR)
    {
      Error(T_("signal() failed for some reason."));
    }
    if (oldHandlerFunc != SIG_DFL)
    {
      if (signal(sig, oldHandlerFunc) == SIG_ERR)
      {
        Error(T_("signal() failed for some reason."));
      }
    }
  }

public:
  Application()
  {
    InstallSignalHandler(SIGINT);
    InstallSignalHandler(SIGTERM);
  }

public:
  void MIKTEXTHISCALL ReportLine(const string & str) override;

public:
  bool MIKTEXTHISCALL OnRetryableError(const string & message) override;

public:
  bool MIKTEXTHISCALL OnProgress(Notification nf) override;

public:
  void Main(int argc, const char ** argv);

private:
  void Verbose(const char * lpszFormat, ...);

private:
  void Message(const char * lpszFormat, ...);

private:
  MIKTEXNORETURN void Error(const char * lpszFormat, ...);

private:
  void UpdateDb();

private:
  void Install(const vector<string> & toBeInstalled, const vector<string> & toBeRemoved);

#if defined(MIKTEX_WINDOWS)
private:
  void RegisterComponents(bool doRegister);
#endif

private:
  void Verify(const vector<string> & toBeVerified);

private:
  void VerifyMiKTeX();

private:
  void FindConflicts();

private:
  void ImportPackage(const string & deploymentName, vector<string> & toBeinstalled);

private:
  void ImportPackages(vector<string> & toBeinstalled);

private:
  void FindUpdates();

private:
  void Update(const vector<string> & updates);

private:
  string GetDirectories(const string & deploymentName);

private:
  void List(OutputFormat outputFormat, int maxCount);

private:
  void ListRepositories(OutputFormat outputFormat);

private:
  void PickRepositoryUrl();

private:
  void PrintFiles(const vector<string> & files);

private:
  void PrintPackageInfo(const string & deploymentName);

private:
  void RestartWindowed();

private:
  void ReadFileList(const PathName & path, vector<string> & files);

private:
  static void SignalHandler(int sig);

private:
  shared_ptr<PackageManager> pPackageManager;

private:
  shared_ptr<Session> pSession;

private:
  static const struct poptOption aoption[];

private:
  static volatile sig_atomic_t interrupted;

private:
  bool verbose = false;

private:
  bool quiet = false;

private:
  string repository;

private:
  bool isLog4cxxConfigured = false;

private:
  vector<TraceCallback::TraceMessage> pendingTraceMessages;

public:
  void MIKTEXTHISCALL Trace(const TraceCallback::TraceMessage & traceMessage) override
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

private:
  void FlushPendingTraceMessages()
  {
    for (const TraceCallback::TraceMessage & msg : pendingTraceMessages)
    {
      TraceInternal(msg);
    }
    pendingTraceMessages.clear();
  }

private:
  void TraceInternal(const TraceCallback::TraceMessage & traceMessage)
  {
    log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(string("trace.mpmcli.") + traceMessage.facility);

    if (traceMessage.streamName == MIKTEX_TRACE_ERROR)
    {
      LOG4CXX_ERROR(logger, traceMessage.message);
    }
    else
    {
      LOG4CXX_TRACE(logger, traceMessage.message);
    }
  }

};

enum Option
{
  OPT_AAA = 1,
  OPT_ADMIN,
  OPT_CSV,                      // deprecated
  OPT_FIND_CONFLICTS,           // internal
  OPT_FIND_UPDATES,
  OPT_HHELP,
  OPT_IMPORT,
  OPT_IMPORT_ALL,
  OPT_INSTALL,
  OPT_INSTALL_ROOT,             // deprecated
  OPT_INSTALL_SOME,
  OPT_LIST,
  OPT_LIST_PACKAGE_NAMES,
  OPT_LIST_REPOSITORIES,
  OPT_MAX_COUNT,                // experimental
  OPT_OUTPUT_FORMAT,            // experimental
  OPT_PICK_REPOSITORY_URL,
  OPT_PRINT_PACKAGE_INFO,
  OPT_PROXY,                    // experimental
  OPT_PROXY_PASSWORD,           // experimental
  OPT_PROXY_USER,               // experimental
  OPT_QUIET,
  OPT_REGISTER_COMPONENTS,      // experimental
  OPT_REPOSITORY,
  OPT_REPOSITORY_RELEASE_STATE,
  OPT_REVERSE,                  // experimental
  OPT_SET_REPOSITORY,
  OPT_SORT,                     // experimental
  OPT_TRACE,
  OPT_UNINSTALL,
  OPT_UNREGISTER_COMPONENTS,    // experimental
  OPT_UPDATE,
  OPT_UPDATE_ALL,               // experimental
  OPT_UPDATE_DB,
  OPT_UPDATE_FNDB,              // experimental
  OPT_UPDATE_SOME,
  OPT_VERBOSE,
  OPT_VERIFY,
  OPT_VERIFY_MIKTEX,
  OPT_VERSION,
};

const struct poptOption Application::aoption[] = {

  {
    "admin", 0, POPT_ARG_NONE, 0, OPT_ADMIN,
    T_("Run in administrative mode."),
    nullptr,
  },

  {                             // deprecated
    "csv", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_CSV,
    T_("Output comma-separated value lists."),
    nullptr,
  },

  {                             // internal
    "find-conflicts", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_FIND_CONFLICTS,
    T_("Find file conflicts."),
    nullptr,
  },

  {
    "find-updates", 0, POPT_ARG_NONE, nullptr, OPT_FIND_UPDATES,
    T_("Test the package repository for updates, then print the list of updateable packages."),
    nullptr,
  },

#if defined(MIKTEX_WINDOWS)
  {
    "hhelp", 0, POPT_ARG_NONE, nullptr, OPT_HHELP,
    T_("Show the manual page in an HTMLHelp window and exit when the window is closed."),
    nullptr
  },
#endif

  {
    "import", 0, POPT_ARG_STRING, nullptr, OPT_IMPORT,
    T_("Import the specified package from another MiKTeX installation."),
    T_("PACKAGE")
  },

  {
    "import-all", 0, POPT_ARG_NONE, nullptr, OPT_IMPORT_ALL,
    T_("Import all installed packages from another MiKTeX installation."),
    nullptr
  },

  {
    "install", 0, POPT_ARG_STRING, nullptr, OPT_INSTALL,
    T_("Install the specified package."),
    T_("PACKAGE")
  },

#if ENABLE_OPT_INSTALL_ROOT
  {                             // deprecated
    "install-root", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_INSTALL_ROOT,
    T_("Use the specified directory as the installation destination."),
    T_("DIR")
  },
#endif

  {
    "install-some", 0, POPT_ARG_STRING, nullptr, OPT_INSTALL_SOME,
    T_("Install packages listed (line-by-line) in the specified file."),
    T_("FILE")
  },

  {
    "list", 0, POPT_ARG_NONE, nullptr, OPT_LIST,
    T_("List the contents of the package database: for each package, print the installation status, the number of files, the size, and the name."),
    nullptr
  },

  {
    "list-package-names", 0, POPT_ARG_NONE, nullptr, OPT_LIST_PACKAGE_NAMES,
    T_("List the package names."),
    nullptr
  },

  {
    "list-repositories", 0, POPT_ARG_NONE, nullptr, OPT_LIST_REPOSITORIES,
    T_("Download the list of known package repository URLs from the MiKTeX project server, then print the list."),
    nullptr
  },

  {
    "max-count", 0, POPT_ARG_STRING, nullptr, OPT_MAX_COUNT,
    T_("Stop after NUM packages."),
    T_("NUM")
  },

  {                             // experimental
    "output-format", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_OUTPUT_FORMAT,
    T_("Set the output format."),
    T_("FORMAT"),
  },

  {
    "pick-repository-url", 0, POPT_ARG_NONE, nullptr, OPT_PICK_REPOSITORY_URL,
    T_("Pick a suitable package repository URL and print it."),
    nullptr
  },

  {
    "print-package-info", 0, POPT_ARG_STRING, nullptr, OPT_PRINT_PACKAGE_INFO,
    T_("Print detailed information about the specified package."),
    T_("PACKAGE")
  },

  {                             // experimental
    "proxy", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_PROXY,
    T_("Use the specified proxy host[:port]."),
    T_("HOST[:PORT]")
  },

  {                             // experimental
    "proxy-password", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_PROXY_PASSWORD,
    T_("Use the specified password for proxy authentication."),
    T_("PASSWORD")
  },

  {                             // experimental
    "proxy-user", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_PROXY_USER,
    T_("Use the specified user for proxy authentication."),
    T_("USER")
  },

  {                             // experimental
    "sort", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_SORT,
    T_("Sort the package list."),
    T_("KEY")
  },

  {
    "quiet", 0, POPT_ARG_NONE, nullptr, OPT_QUIET,
    T_("Suppress all output (except errors)."),
    nullptr
  },

#if defined(MIKTEX_WINDOWS)
  {
    "register-components", 0, POPT_ARG_NONE, nullptr, OPT_REGISTER_COMPONENTS,
    T_("Register COMponents."),
    nullptr
  },
#endif

  {
    "repository", 0, POPT_ARG_STRING, nullptr, OPT_REPOSITORY,
    T_("Use the specified location as the package repository.  The location can be either a fully qualified path name (a local package repository) or an URL (a remote package repository)."),
    T_("LOCATION")
  },

  {
    "repository-release-state", 0, POPT_ARG_STRING, nullptr, OPT_REPOSITORY_RELEASE_STATE,
    T_("Select the repository release state (one of: stable, next)."),
    T_("STATE")
  },

  {
    "reverse", 0, POPT_ARG_NONE, nullptr, OPT_REVERSE,
    T_("Reverse the result of comparisons."),
    nullptr
  },

#if defined(MIKTEX_WINDOWS)
  {
    "set-repository", 0, POPT_ARG_STRING, nullptr, OPT_SET_REPOSITORY,
    T_("Register the location of the default package repository.  The location can be either a fully qualified path name (a local package repository) or an URL (a remote package repository)."),
    T_("LOCATION")
  },
#endif

  {
    "trace", 0, POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr, OPT_TRACE,
    T_("Turn on tracing.  TRACESTREAMS, if specified, is a comma-separated list of trace stream names (see the MiKTeX manual)."),
    T_("TRACESTREAMS")
  },

  {
    "uninstall", 0, POPT_ARG_STRING, nullptr, OPT_UNINSTALL,
    T_("Uninstall the specified package."),
    T_("PACKAGE")
  },

#if defined(MIKTEX_WINDOWS)
  {
    "unregister-components", 0, POPT_ARG_NONE, nullptr,
    OPT_UNREGISTER_COMPONENTS,
    T_("Unregister COMponents."),
    nullptr
  },
#endif

  {
    "update", 0, POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr, OPT_UPDATE,
    T_("Update the specified package, if an updated version is available in the package repository.  Install all updateable packages, if the package name is omitted."),
    T_("PACKAGE")
  },

  {                             // experimental
    "update-all", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_UPDATE_ALL,
    T_("Test the package repository for updates, then install all updateable packages."),
    nullptr,
  },

  {
    "update-db", 0, POPT_ARG_NONE, nullptr, OPT_UPDATE_DB,
    T_("Synchronize the local package database with the package repository."),
    nullptr
  },

  {                             // experimental
    "update-fndb", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_UPDATE_FNDB,
    T_("Update mpm.fndb."),
    nullptr
  },

  {
    "update-some", 0, POPT_ARG_STRING, nullptr, OPT_UPDATE_SOME,
    T_("Update packages listed (line-by-line) in the specified file."),
    T_("FILE")
  },

  {
    "verbose", 0, POPT_ARG_NONE, nullptr, OPT_VERBOSE,
    T_("Turn on verbose output mode."),
    nullptr
  },

  {
    "verify", 0, POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr, OPT_VERIFY,
    T_("Verify the integrity of the installed packages."),
    T_("PACKAGE")
  },

  {                             // experimental
    "verify-miktex", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_VERIFY_MIKTEX,
    T_("Verify the integrity of the installed MiKTeX packages."),
    nullptr
  },

  {
    "version", 0, POPT_ARG_NONE, nullptr, OPT_VERSION,
    T_("Show version information and exit."),
    nullptr
  },

  POPT_AUTOHELP
  POPT_TABLEEND
};

volatile sig_atomic_t Application::interrupted = false;

void Application::Message(const char * lpszFormat, ...)
{
  va_list arglist;
  string s;
  VA_START(arglist, lpszFormat);
  s = StringUtil::FormatString(lpszFormat, arglist);
  VA_END(arglist);
  LOG4CXX_INFO(logger, s);
  if (!quiet)
  {
    cout << s << endl;
  }
}

void Application::Verbose(const char * lpszFormat, ...)
{
  va_list arglist;
  string s;
  VA_START(arglist, lpszFormat);
  s = StringUtil::FormatString(lpszFormat, arglist);
  VA_END(arglist);
  LOG4CXX_INFO(logger, s);
  if (verbose)
  {
    cout << s << endl;
  }
}

static void Sorry(string reason)
{
  if (cerr.fail())
  {
    return;
  }

  cerr << endl;
  if (reason.empty())
  {
    cerr << StringUtil::FormatString(T_("Sorry, but %s did not succeed."), Q_(THE_NAME_OF_THE_GAME)) << endl;
  }
  else
  {
    cerr
      << StringUtil::FormatString(T_("Sorry, but %s did not succeed for the following reason:"), Q_(THE_NAME_OF_THE_GAME)) << endl << endl
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

static void Sorry()
{
  Sorry("");
}

MIKTEXNORETURN void Application::Error(const char * lpszFormat, ...)
{
  va_list arglist;
  string s;
  VA_START(arglist, lpszFormat);
  s = StringUtil::FormatString(lpszFormat, arglist);
  VA_END(arglist);
  LOG4CXX_FATAL(logger, s);
  Sorry(s);
  throw 1;
}

void Application::ReportLine(const string & str)
{
  Verbose("%s", str.c_str());
}

bool Application::OnRetryableError(const string & message)
{
  return false;
}

bool Application::OnProgress(Notification nf)
{
  if (interrupted != 0)
  {
    return false;
  }
  return interrupted == 0;
}

void Application::UpdateDb()
{
  shared_ptr<PackageInstaller> installer(pPackageManager->CreateInstaller());
  if (!repository.empty())
  {
    installer->SetRepository(repository);
  }
  installer->SetCallback(this);
  installer->UpdateDb();
  installer->Dispose();
}

void Application::Install(const vector<string> & toBeInstalled, const vector<string> & toBeRemoved)
{
  for (const string & deploymentName : toBeInstalled)
  {
    PackageInfo packageInfo = pPackageManager->GetPackageInfo(deploymentName);
    if (packageInfo.IsInstalled())
    {
      Error(T_("Package \"%s\" is already installed."), deploymentName.c_str());
    }
  }

  for (const string & deploymentName : toBeRemoved)
  {
    PackageInfo packageInfo = pPackageManager->GetPackageInfo(deploymentName);
    if (!packageInfo.IsInstalled())
    {
      Error(T_("Package \"%s\" is not installed."), deploymentName.c_str());
    }
  }

  shared_ptr<PackageInstaller> installer(pPackageManager->CreateInstaller());

  if (!repository.empty())
  {
    installer->SetRepository(repository);
  }

  installer->SetCallback(this);
  installer->SetFileLists(toBeInstalled, toBeRemoved);
  installer->InstallRemove();
  installer->Dispose();
  if (toBeInstalled.size() == 1)
  {
    Message(T_("Package \"%s\" has been successfully installed."), toBeInstalled[0].c_str());
  }
  else if (toBeInstalled.size() > 1)
  {
    Message(T_("%u packages have been successfully installed."), toBeInstalled.size());
  }
  if (toBeRemoved.size() == 1)
  {
    Message(T_("Package \"%s\" has been successfully removed."), toBeRemoved[0].c_str());
  }
  else if (toBeRemoved.size() > 1)
  {
    Message(T_("%u packages have been successfully removed."), toBeRemoved.size());
  }
}

#if defined(MIKTEX_WINDOWS)

void Application::RegisterComponents(bool doRegister)
{
  shared_ptr<PackageInstaller> installer(pPackageManager->CreateInstaller());
  installer->SetCallback(this);
  installer->RegisterComponents(doRegister);
  installer->Dispose();
}

#endif

void Application::FindConflicts()
{
  map<string, vector<string> > filesAndPackages;
  unique_ptr<PackageIterator> packageIterator(pPackageManager->CreateIterator());
  PackageInfo packageInfo;
  while (packageIterator->GetNext(packageInfo))
  {
    for (const string & fileName : packageInfo.runFiles)
    {
      PathName file(fileName);
      file.TransformForComparison();
      filesAndPackages[file.Get()].push_back(packageInfo.deploymentName);
    }
    for (const string & fileName : packageInfo.docFiles)
    {
      PathName file(fileName);
      file.TransformForComparison();
      filesAndPackages[file.Get()].push_back(packageInfo.deploymentName);
    }
    for (const string & fileName : packageInfo.sourceFiles)
    {
      PathName file(fileName);
      file.TransformForComparison();
      filesAndPackages[file.Get()].push_back(packageInfo.deploymentName);
    }
  }
  for (const auto & package : filesAndPackages)
  {
    if (package.second.size() > 1)
    {
      cout << package.first << endl;
      for (const string & fileName : package.second)
      {
        cout << "  " << fileName << endl;
      }
    }
  }
}

void Application::VerifyMiKTeX()
{
  vector<string> toBeVerified;
  unique_ptr<PackageIterator> packageIterator(pPackageManager->CreateIterator());
  PackageInfo packageInfo;
  while (packageIterator->GetNext(packageInfo))
  {
    if (!packageInfo.IsPureContainer()
      && packageInfo.IsInstalled()
      && packageInfo.deploymentName.compare(0, 7, "miktex-") == 0)
    {
      toBeVerified.push_back(packageInfo.deploymentName);
    }
  }
  Verify(toBeVerified);
}

void Application::Verify(const vector<string> & toBeVerifiedArg)
{
  vector<string> toBeVerified = toBeVerifiedArg;
  bool verifyAll = toBeVerified.empty();
  if (verifyAll)
  {
    unique_ptr<PackageIterator> packageIterator(pPackageManager->CreateIterator());
    PackageInfo packageInfo;
    while (packageIterator->GetNext(packageInfo))
    {
      if (!packageInfo.IsPureContainer() && packageInfo.IsInstalled())
      {
        toBeVerified.push_back(packageInfo.deploymentName);
      }
    }
  }
  bool ok = true;
  for (const string & deploymentName : toBeVerified)
  {
    if (!pPackageManager->TryVerifyInstalledPackage(deploymentName))
    {
      Message(T_("%s: this package needs to be reinstalled."), deploymentName.c_str());
      ok = false;
    }
  }
  if (ok)
  {
    if (verifyAll)
    {
      Message(T_("All packages are correctly installed."));
    }
    else
    {
      if (toBeVerified.size() == 1)
      {
        Message(T_("Package %s is correctly installed."), toBeVerified[0].c_str());
      }
      else
      {
        Message(T_("The packages are correctly installed."));
      }
    }
  }
  else
  {
    Error(T_("Some packages need to be reinstalled."));
  }
}

void Application::ImportPackage(const string & deploymentName, vector<string> & toBeinstalled)
{
  if (repository.empty())
  {
    Error(T_("You have to use --repository=/PATH/TO/MIKTEX."));
  }
  PathName packagesIni(repository);
  packagesIni /= MIKTEX_PATH_PACKAGES_INI;
  if (!File::Exists(packagesIni))
  {
    Error(T_("Not a MiKTeX installation directory: %s"), repository.c_str());
  }
  unique_ptr<Cfg> cfg = Cfg::Create();
  cfg->Read(packagesIni);
  if (strncmp(deploymentName.c_str(), "miktex-", 7) == 0)
  {
    Error(T_("Cannot import package %s."), deploymentName.c_str());
  }
  string str;
  if (!cfg->TryGetValue(deploymentName, "TimeInstalled", str) || str.empty() || str == "0")
  {
    Error(T_("Package %s is not installed."), deploymentName.c_str());
  }
  if (cfg->TryGetValue(deploymentName, T_("Obsolete"), str) && str == "1")
  {
    Error(T_("Package %s is obsolete."), deploymentName.c_str());
  }
  PackageInfo packageInfo;
  if (!pPackageManager->TryGetPackageInfo(deploymentName.c_str(), packageInfo))
  {
    Error(T_("Unknown package: %s."), deploymentName.c_str());
  }
  if (packageInfo.IsInstalled())
  {
    Error(T_("Package %s is already installed."), deploymentName.c_str());
  }
  toBeinstalled.push_back(deploymentName);
}

void Application::ImportPackages(vector<string> & toBeinstalled)
{
  if (repository.empty())
  {
    Error(T_("You have to use --repository=/PATH/TO/MIKTEX."));
  }
  PathName packagesIni(repository);
  packagesIni /= MIKTEX_PATH_PACKAGES_INI;
  if (!File::Exists(packagesIni))
  {
    Error(T_("Not a MiKTeX installation directory: %s"), repository.c_str());
  }
  unique_ptr<Cfg> cfg = Cfg::Create();
  cfg->Read(packagesIni);
  for (const shared_ptr<Cfg::Key> & key : cfg->GetKeys())
  {
    if (strncmp(key->GetName().c_str(), "miktex-", 7) == 0)
    {
      continue;
    }
    string str;
    if (!cfg->TryGetValue(key->GetName(), "TimeInstalled", str) || str.empty() || str == "0")
    {
      continue;
    }
    if (cfg->TryGetValue(key->GetName(), "Obsolete", str) && str == "1")
    {
      continue;
    }
    PackageInfo packageInfo;
    if (!pPackageManager->TryGetPackageInfo(key->GetName(), packageInfo) || packageInfo.IsInstalled())
    {
      continue;
    }
    toBeinstalled.push_back(key->GetName());
  }
}

void Application::FindUpdates()
{
  shared_ptr<PackageInstaller> installer(pPackageManager->CreateInstaller());
  if (!repository.empty())
  {
    installer->SetRepository(repository);
  }
  installer->SetCallback(this);
  installer->FindUpdates();
  vector<PackageInstaller::UpdateInfo> updates = installer->GetUpdates();
  installer->Dispose();
  if (updates.size() == 0)
  {
    Message(T_("There are currently no updates available."));
  }
  else
  {
    sort(updates.begin(), updates.end(), UpdateInfoComparer());
    for (vector<PackageInstaller::UpdateInfo>::const_iterator it = updates.begin(); it != updates.end(); ++it)
    {
      cout << it->deploymentName << endl;
    }
  }
}

void Application::Update(const vector<string> & updates)
{
  shared_ptr<PackageInstaller> installer(pPackageManager->CreateInstaller());
  if (!repository.empty())
  {
    installer->SetRepository(repository);
  }
  installer->SetCallback(this);
  installer->FindUpdates();
  vector<PackageInstaller::UpdateInfo> serverUpdates = installer->GetUpdates();
  vector<string> updates2;
  for (const PackageInstaller::UpdateInfo & upd : serverUpdates)
  {
    updates2.push_back(upd.deploymentName);
  }
  sort(updates2.begin(), updates2.end());
  vector<string> toBeInstalled;
  if (updates.empty())
  {
    if (updates2.empty())
    {
      Message(T_("There are currently no updates available."));
    }
    toBeInstalled = updates2;
  }
  else
  {
    for (const string & deploymentName : updates)
    {
      PackageInfo packageInfo = pPackageManager->GetPackageInfo(deploymentName);
      if (!packageInfo.IsInstalled())
      {
        Error(T_("Package \"%s\" is not installed."), deploymentName.c_str());
      }
      if (binary_search(updates2.begin(), updates2.end(), deploymentName))
      {
        toBeInstalled.push_back(deploymentName);
      }
      else
      {
        Message(T_("Package \"%s\" is up to date."), deploymentName.c_str());
      }
    }
  }
  if (toBeInstalled.empty())
  {
    return;
  }
  installer->SetFileLists(toBeInstalled, vector<string>());
  installer->InstallRemove();
  if (toBeInstalled.size() == 1)
  {
    Message(T_("Package \"%s\" has been successfully updated."), toBeInstalled[0].c_str());
  }
  else if (toBeInstalled.size() > 1)
  {
    Message(T_("%u packages have been successfully updated."), toBeInstalled.size());
  }
}

string Application::GetDirectories(const string & deploymentName)
{
  set<string> directories;
  PackageInfo pi = pPackageManager->GetPackageInfo(deploymentName);
  for (const string & fileName : pi.runFiles)
  {
    PathName path(fileName);
    if (!path.HasExtension(MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX))
    {
      directories.insert(path.RemoveFileSpec().ToString());
    }
  }
  string ret;
  for (const string & dir : directories)
  {
    if (!ret.empty())
    {
      ret += PATH_DELIMITER;
    }
    ret += dir;
  }
  return ret;
}

void Application::List(OutputFormat outputFormat, int maxCount)
{
  unique_ptr<PackageIterator> packageIterator(pPackageManager->CreateIterator());
  PackageInfo packageInfo;
  set<PackageInfo, PackageInfoComparer> setPi;
  while (packageIterator->GetNext(packageInfo))
  {
    if (packageInfo.IsPureContainer())
    {
      continue;
    }
    setPi.insert(packageInfo);
  }
  if (setPi.empty())
  {
    Message(T_("The package database files have not been installed."));
  }
  int count = 0;
  for (set<PackageInfo, PackageInfoComparer>::const_iterator it = setPi.begin(); it != setPi.end() && count < maxCount; ++it, ++count)
  {
    if (outputFormat == OutputFormat::Listing)
    {
      cout
        << StringUtil::FormatString("%c %.5d %10d %s", it->IsInstalled() ? 'i' : '-', static_cast<int>(it->GetNumFiles()), static_cast<int>(it->GetSize()), it->deploymentName.c_str())
        << endl;
    }
    else if (outputFormat == OutputFormat::CSV)
    {
      string path = pPackageManager->GetContainerPath(it->deploymentName, false);
      string directories = GetDirectories(it->deploymentName);
      cout << StringUtil::FormatString("%s\\%s,%s", path.c_str(), it->deploymentName.c_str(), directories.c_str()) << endl;
    }
    else if (outputFormat == OutputFormat::DeploymentNames)
    {
      cout << it->deploymentName << endl;
    }
  }
}

void Application::ListRepositories(OutputFormat outputFormat)
{
  pPackageManager->DownloadRepositoryList();
  vector<RepositoryInfo> repositories = pPackageManager->GetRepositories();
  if (repositories.empty())
  {
    Message(T_("No package repositories are currently available."));
  }
  for (const RepositoryInfo & ri : repositories)
  {
    cout << ri.url << endl;
  }
}

void Application::PickRepositoryUrl()
{
  cout << pPackageManager->PickRepositoryUrl() << endl;
}

void Application::PrintFiles(const vector<string> & files)
{
  for (const string & fileName : files)
  {
    PathName path = pSession->GetSpecialPath(SpecialPath::InstallRoot);
    string disp;
    if (!PackageManager::StripTeXMFPrefix(fileName, disp))
    {
      disp = fileName;
    }
    path /= disp;
    cout << "  " << path << endl;
  }
}

void Application::PrintPackageInfo(const string & deploymentName)
{
  PackageInfo packageInfo = pPackageManager->GetPackageInfo(deploymentName);
  cout << T_("name:") << " " << packageInfo.deploymentName << endl;
  cout << T_("title:") << " " << packageInfo.title << endl;
  if (!packageInfo.runFiles.empty())
  {
    cout << T_("run-time files:") << endl;
    PrintFiles(packageInfo.runFiles);
  }
  if (!packageInfo.docFiles.empty())
  {
    cout << T_("documentation files:") << endl;
    PrintFiles(packageInfo.docFiles);
  }
  if (!packageInfo.sourceFiles.empty())
  {
    cout << T_("source files:") << endl;
    PrintFiles(packageInfo.sourceFiles);
  }
}

void Application::RestartWindowed()
{
  Verbose(T_("Restarting in windowed mode..."));

#if defined(MIKTEX_ATLMFC)
  string mpmGuiName = pSession->IsAdminMode() ? MIKTEX_MPM_MFC_ADMIN_EXE : MIKTEX_MPM_MFC_EXE;
  string options;
#else
  string mpmGuiName = pSession->IsAdminMode() ? MIKTEX_MPM_QT_ADMIN_EXE : MIKTEX_MPM_QT_EXE;
  string options = pSession->IsAdminMode() ? "--admin" : "";
#endif

  PathName mpmgui;

  // locate mpm_mfc
  if (!pSession->FindFile(mpmGuiName.c_str(), FileType::EXE, mpmgui))
  {
    Error(T_("Could not restart in windowed mode."));
  }

  Process::Start(mpmgui, options);
}

void Application::ReadFileList(const PathName & path, vector<string> & files)
{
  StreamReader reader(path);
  string line;
  while (reader.ReadLine(line))
  {
    Tokenizer tok(line.c_str(), " \t\n\r");
    if (tok.GetCurrent() != nullptr)
    {
      files.push_back(tok.GetCurrent());
    }
  }
  reader.Close();
}

void Application::Main(int argc, const char ** argv)
{
  StartupConfig startupConfig;
  Session::InitInfo initInfo;
  initInfo.SetTraceCallback(this);
  initInfo.SetProgramInvocationName(argv[0]);

  bool optAdmin = false;
  bool optFindConflicts = false;
  bool optFindUpdates = false;
  bool optImport = false;
  bool optImportAll = false;
  bool optList = false;
  bool optListRepositories = false;
  bool optPickRepositoryUrl = false;
  bool optPrintPackageInfo = false;
  bool optRegisterComponents = false;
  bool optSetRepository = false;
  bool optSort = false;
  bool optUnregisterComponents = false;
  bool optUpdate = false;
  bool optUpdateAll = false;
  bool optUpdateDb = false;
  bool optUpdateFndb = false;
  bool optVerify = false;
  bool optVerifyMiKTeX = false;
  bool optVersion = false;
  int optMaxCount = INT_MAX;
  int optProxyPort = -1;
  OutputFormat outputFormat(OutputFormat::Listing);
  string deploymentName;
  string optProxy;
  string optProxyPassword;
  string optProxyUser;
  string toBeImported;
  vector<string> installSome;
  vector<string> toBeInstalled;
  vector<string> toBeRemoved;
  vector<string> toBeVerified;
  vector<string> updateSome;
  vector<string> updates;
  RepositoryReleaseState optRepositoryReleaseState = RepositoryReleaseState::Unknown;

  bool changeProxy = false;

  PoptWrapper popt(argc, argv, aoption);

  //  popt.SetOtherOptionHelp (T_("[OPTION...]"));

  // process command-line options
  int option;
  while ((option = popt.GetNextOpt()) >= 0)
  {
    string optArg = popt.GetOptArg();
    switch (option)
    {
    case OPT_ADMIN:
      optAdmin = true;
      break;
    case OPT_CSV:
      outputFormat = OutputFormat::CSV;
      break;
    case OPT_FIND_CONFLICTS:
      optFindConflicts = true;
      break;
    case OPT_FIND_UPDATES:
      optFindUpdates = true;
      break;
#if defined (MIKTEX_WINDOWS)
    case OPT_HHELP:
    {
      pSession = Session::Create(initInfo);
      pSession->ShowManualPageAndWait(0, MIKTEXHELP_MPMCON);
      pSession = nullptr;
      return;
    }
#endif
    case OPT_IMPORT:
      toBeImported = optArg;
      optImport = true;
      break;
    case OPT_IMPORT_ALL:
      optImportAll = true;
      break;
    case OPT_INSTALL:
      toBeInstalled.push_back(optArg);
      break;
    case OPT_INSTALL_ROOT:
      startupConfig.commonInstallRoot = optArg;
      startupConfig.userInstallRoot = optArg;
      break;
    case OPT_INSTALL_SOME:
      installSome.push_back(optArg);
      break;
    case OPT_LIST:
      optList = true;
      break;
    case OPT_LIST_PACKAGE_NAMES:
      optList = true;
      outputFormat = OutputFormat::DeploymentNames;
      break;
    case OPT_LIST_REPOSITORIES:
      optListRepositories = true;
      break;
    case OPT_PICK_REPOSITORY_URL:
      optPickRepositoryUrl = true;
      break;
    case OPT_PRINT_PACKAGE_INFO:
      optPrintPackageInfo = true;
      deploymentName = optArg;
      break;
    case OPT_PROXY:
    {
      changeProxy = true;
      Tokenizer tok(optArg.c_str(), ":");
      optProxy = tok.GetCurrent();
      ++tok;
      if (tok.GetCurrent() != nullptr)
      {
        optProxyPort = atoi(tok.GetCurrent());
      }
    }
    break;
    case OPT_SORT:
      optSort = true;
      if (Utils::EqualsIgnoreCase(optArg, "deploymentname"))
      {
        PackageInfoComparer::sortKey = SortKey::DeploymentName;
      }
      else if (Utils::EqualsIgnoreCase(optArg, "installedon"))
      {
        PackageInfoComparer::sortKey = SortKey::InstalledOn;
      }
      else if (Utils::EqualsIgnoreCase(optArg, "packagedon"))
      {
        PackageInfoComparer::sortKey = SortKey::PackagedOn;
      }
      else
      {
        Error(T_("Unknown sort key."));
      }
      break;
    case OPT_PROXY_USER:
      changeProxy = true;
      optProxyUser = optArg;
      break;
    case OPT_PROXY_PASSWORD:
      changeProxy = true;
      optProxyPassword = optArg;
      break;
    case OPT_QUIET:
      if (verbose)
      {
        Error(T_("Cannot be --verbose and --quiet at the same time."));
      }
      quiet = true;
      break;
#if defined (MIKTEX_WINDOWS)
    case OPT_REGISTER_COMPONENTS:
      optRegisterComponents = true;
      break;
#endif
    case OPT_MAX_COUNT:
      optMaxCount = std::stoi(optArg);
      break;
    case OPT_OUTPUT_FORMAT:
    {
      if (Utils::EqualsIgnoreCase(optArg, "listing"))
      {
        outputFormat = OutputFormat::Listing;
      }
      else if (Utils::EqualsIgnoreCase(optArg, "csv"))
      {
        outputFormat = OutputFormat::CSV;
      }
      else if (Utils::EqualsIgnoreCase(optArg, "deploymentnames"))
      {
        outputFormat = OutputFormat::DeploymentNames;
      }
      else
      {
        Error(T_("Unknown output format."));
      }
    }
    case OPT_REPOSITORY:
      repository = optArg;
      break;
    case OPT_REPOSITORY_RELEASE_STATE:
      if (Utils::EqualsIgnoreCase(optArg, "stable"))
      {
        optRepositoryReleaseState = RepositoryReleaseState::Stable;
      }
      else if (Utils::EqualsIgnoreCase(optArg, "next"))
      {
        optRepositoryReleaseState = RepositoryReleaseState::Next;
      }
      else
      {
        Error(T_("Repository release state must be one of: stable, next."));
      }
    case OPT_REVERSE:
      PackageInfoComparer::reverse = true;
      break;
    case OPT_SET_REPOSITORY:
      optSetRepository = true;
      repository = optArg;
      break;
    case OPT_TRACE:
      if (optArg.empty())
      {
        initInfo.SetTraceFlags(DEFAULT_TRACE_STREAMS);
      }
      else
      {
        initInfo.SetTraceFlags(optArg);
      }
      break;
    case OPT_UNINSTALL:
      toBeRemoved.push_back(optArg);
      break;
#if defined (MIKTEX_WINDOWS)
    case OPT_UNREGISTER_COMPONENTS:
      optUnregisterComponents = true;
      break;
#endif
    case OPT_UPDATE:
      if (!optArg.empty())
      {
        if (optUpdateAll)
        {
          Error(T_("Already updating all packages."));
        }
        optUpdate = true;
        updates.push_back(optArg);
      }
      else
      {
        if (optUpdate)
        {
          Error(T_("Already updating selected packages."));
        }
        optUpdateAll = true;
      }
      break;
    case OPT_UPDATE_ALL:
      if (optUpdate)
      {
        Error(T_("Already updating selected packages."));
      }
      optUpdateAll = true;
      break;
    case OPT_UPDATE_DB:
      optUpdateDb = true;
      break;
    case OPT_UPDATE_FNDB:
      optUpdateFndb = true;
      break;
    case OPT_UPDATE_SOME:
      updateSome.push_back(optArg);
      break;
    case OPT_VERBOSE:
      if (quiet)
      {
        Error(T_("Cannot be --verbose and --quiet at the same time."));
      }
      verbose = true;
      break;
    case OPT_VERIFY:
      if (!optArg.empty())
      {
        toBeVerified.push_back(optArg);
      }
      optVerify = true;
      break;
    case OPT_VERIFY_MIKTEX:
      optVerifyMiKTeX = true;
      break;
    case OPT_VERSION:
      optVersion = true;
      break;
    }
  }

  if (option != -1)
  {
    string msg = popt.BadOption(POPT_BADOPTION_NOALIAS);
    msg += ": ";
    msg += popt.Strerror(option);
    Error("%s", msg.c_str());
  }

  if (!popt.GetLeftovers().empty())
  {
    Error(T_("This utility does not accept non-option arguments."));
  }

  if (optVersion)
  {
    cout
      << Utils::MakeProgramVersionString(THE_NAME_OF_THE_GAME, VersionNumber(MIKTEX_MAJOR_VERSION, MIKTEX_MINOR_VERSION, MIKTEX_COMP_J2000_VERSION, 0)) << endl
      << "Copyright (C) 2005-2016 Christian Schenk" << endl
      << "This is free software; see the source for copying conditions.  There is NO" << endl
      << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
    return;
  }

  initInfo.SetStartupConfig(startupConfig);

  pSession = Session::Create(initInfo);

  PathName xmlFileName;
  if (pSession->FindFile("mpmcli." MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName)
    || pSession->FindFile(MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName))
  {
    Utils::SetEnvironmentString("MIKTEX_LOG_DIR", PathName(pSession->GetSpecialPath(SpecialPath::DataRoot)).AppendComponent(MIKTEX_PATH_MIKTEX_LOG_DIR).Get());
    Utils::SetEnvironmentString("MIKTEX_LOG_NAME", "mpmcli");
    log4cxx::xml::DOMConfigurator::configure(xmlFileName.ToWideCharString());
    isLog4cxxConfigured = true;
    LOG4CXX_INFO(logger, "starting: " << Utils::MakeProgramVersionString("mpmcli", MIKTEX_COMPONENT_VERSION_STR));
  }

  if (optAdmin)
  {
    if (!pSession->RunningAsAdministrator())
    {
#if defined(MIKTEX_WINDOWS)
      Error(T_("Option --admin requires administrator privileges."));
#else
      Error(T_("Option --admin requires root privileges."));
#endif
    }
    if (!pSession->IsSharedSetup())
    {
      Error(T_("Option --admin only makes sense for a shared MiKTeX setup."));
    }
    pSession->SetAdminMode(true);
  }

  pPackageManager = PackageManager::Create();

  if (changeProxy)
  {
    ProxySettings proxySettings;
    ProxySettings temp;
    if (PackageManager::TryGetProxy(temp))
    {
      proxySettings = temp;
    }
    proxySettings.useProxy = true;
    if (!optProxy.empty())
    {
      proxySettings.proxy = optProxy;
    }
    if (optProxyPort >= 0)
    {
      proxySettings.port = optProxyPort;
    }
    if (!optProxyUser.empty())
    {
      proxySettings.authenticationRequired = true;
      proxySettings.user = optProxyUser;
    }
    if (!optProxyPassword.empty())
    {
      proxySettings.password = optProxyPassword;
    }
    pPackageManager->SetProxy(proxySettings);
  }

  bool restartWindowed = true;

  if (optSetRepository)
  {
    pPackageManager->SetDefaultPackageRepository(RepositoryType::Unknown, repository);
    restartWindowed = false;
  }

  if (optRepositoryReleaseState != RepositoryReleaseState::Unknown)
  {
    pPackageManager->SetRepositoryReleaseState(optRepositoryReleaseState);
  }

  if (optUpdateFndb && !optUpdateDb)
  {
    pPackageManager->CreateMpmFndb();
    restartWindowed = false;
  }

  if (optUpdateDb)
  {
    UpdateDb();
    restartWindowed = false;
  }

  if (optImport)
  {
    ImportPackage(toBeImported, toBeInstalled);
    restartWindowed = false;
  }

  if (optImportAll)
  {
    ImportPackages(toBeInstalled);
    restartWindowed = false;
  }

  if (optFindUpdates)
  {
    FindUpdates();
    restartWindowed = false;
  }

 ;

  for (const string & name : installSome)
  {
    ReadFileList(name, toBeInstalled);
  }

  if (toBeInstalled.size() > 0 || toBeRemoved.size() > 0)
  {
    Install(toBeInstalled, toBeRemoved);
    restartWindowed = false;
  }

#if defined(MIKTEX_WINDOWS)
  if (optRegisterComponents || optUnregisterComponents)
  {
    if (optRegisterComponents && optUnregisterComponents)
    {
      Error(T_("--register-components conflicts with --unregister-components."));
    }
    RegisterComponents(optRegisterComponents);
    restartWindowed = false;
  }
#endif

  for (const string & name : updateSome)
  {
    ReadFileList(name, updates);
  }

  if (optUpdateAll || !updates.empty())
  {
    Update(updates);
    restartWindowed = false;
  }

  if (optFindConflicts)
  {
    FindConflicts();
    restartWindowed = false;
  }

  if (optVerifyMiKTeX)
  {
    VerifyMiKTeX();
    restartWindowed = false;
  }

  if (optVerify)
  {
    Verify(toBeVerified);
    restartWindowed = false;
  }

  if (optList)
  {
    List(outputFormat, optMaxCount);
    restartWindowed = false;
  }

  if (optListRepositories)
  {
    ListRepositories(outputFormat);
    restartWindowed = false;
  }

  if (optPickRepositoryUrl)
  {
    PickRepositoryUrl();
    restartWindowed = false;
  }

  if (optPrintPackageInfo)
  {
    PrintPackageInfo(deploymentName);
    restartWindowed = false;
  }

  if (restartWindowed)
  {
    RestartWindowed();
  }

  pPackageManager = nullptr;
  pSession = nullptr;
}

extern "C" void Application::SignalHandler(int signalToBeHandled)
{
  switch (signalToBeHandled)
  {
  case SIGINT:
  case SIGTERM:
    signal(SIGINT, SIG_IGN);
    interrupted = true;
  }
}

#if defined(_UNICODE)
#  define MAIN wmain
#  define MAINCHAR wchar_t
#else
#  define MAIN main
#  define MAINCHAR char
#endif

int MAIN(int argc, MAINCHAR * argv[])
{
#if defined(MIKTEX_WINDOWS)
  HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  if (FAILED(hr))
  {
    cerr << T_("mpm: The COM library could not be initialized.") << endl;
    return 1;
  }
#endif
  int retCode = 0;
  try
  {
    vector<string> utf8args;
    utf8args.reserve(argc);
    vector<const char *> newargv;
    newargv.reserve(argc + 1);
    for (int idx = 0; idx < argc; ++idx)
    {
#if defined(_UNICODE)
      utf8args.push_back(StringUtil::WideCharToUTF8(argv[idx]));
#elif defined(MIKTEX_WINDOWS)
      utf8args.push_back(StringUtil::AnsiToUTF8(argv[idx]));
#else
      utf8args.push_back(argv[idx]);
#endif
      newargv.push_back(utf8args[idx].c_str());
    }
    newargv.push_back(nullptr);
    Application app;
    app.Main(argc, &newargv[0]);
  }
  catch (const MiKTeXException & e)
  {
    LOG4CXX_FATAL(logger, e.what());
    LOG4CXX_FATAL(logger, "Info: " << e.GetInfo());
    LOG4CXX_FATAL(logger, "Source: " << e.GetSourceFile());
    LOG4CXX_FATAL(logger, "Line: " << e.GetSourceLine());
    Sorry();
    retCode = 1;
  }
  catch (const exception & e)
  {
    LOG4CXX_FATAL(logger, e.what());
    Sorry();
    retCode = 1;
  }
  catch (int rc)
  {
    retCode = rc;
  }
#if defined(MIKTEX_WINDOWS)
  CoUninitialize();
#endif
  return retCode;
}
