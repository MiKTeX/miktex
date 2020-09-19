/* mpm.cpp: MiKTeX Package Manager (cli version)

   Copyright (C) 2003-2020 Christian Schenk

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

#if defined(HAVE_CONFIG_H)
#  include <config.h>
#endif

#if defined(_WIN32)
#  include <Windows.h>
#endif

#include <climits>
#include <cstdio>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include <signal.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <log4cxx/logger.h>
#include <log4cxx/rollingfileappender.h>
#include <log4cxx/xml/domconfigurator.h>

#include "mpm-version.h"

#include <miktex/Core/Cfg>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Exceptions>
#include <miktex/Core/File>
#include <miktex/Core/FileType>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Core/Session>
#include <miktex/Core/StreamReader>
#include <miktex/PackageManager/PackageManager>
#include <miktex/Setup/SetupService>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceCallback>
#include <miktex/Util/StringUtil>
#include <miktex/Util/Tokenizer>
#include <miktex/Wrappers/PoptWrapper>

#if defined(MIKTEX_WINDOWS)
#  include <MiKTeX/Core/Help>
#endif

using namespace MiKTeX::Packages;
using namespace MiKTeX::Core;
using namespace MiKTeX::Setup;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;
using namespace std;

#include "internal.h"

#if !defined(THE_NAME_OF_THE_GAME)
#  define THE_NAME_OF_THE_GAME T_("MiKTeX Package Manager")
#endif

#define ENABLE_OPT_INSTALL_SOME 1
#define ENABLE_OPT_UPDATE_SOME 1

#if defined(MIKTEX_WINDOWS)
const char PATH_DELIMITER = ';';
#define PATH_DELIMITER_STRING ";"
#else
const char PATH_DELIMITER = ':';
#define PATH_DELIMITER_STRING ":"
#endif

vector<string> DEFAULT_TRACE_OPTIONS = {
  TraceStream::MakeOption("", "", TraceLevel::Info),
  TraceStream::MakeOption(MIKTEX_TRACE_MPM, "", TraceLevel::Trace),
};

static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("mpmcli"));
static bool isLog4cxxConfigured = false;

enum class OutputFormat
{
  None = 0,
  Listing,
  CSV,
  PackageIdentifiers,
};

enum class SortKey
{
  None = 0,
  PackageId,
  InstalledOn,
  PackagedOn,
};

class PackageInfoComparer
{
public:
  bool operator() (const PackageInfo& pi1, const PackageInfo& pi2) const
  {
    bool cmp;
    switch (sortKey)
    {
    case SortKey::PackageId:
      cmp = (PathName::Compare(pi1.id, pi2.id) < 0);
      break;
    case SortKey::InstalledOn:
      cmp = (pi1.GetTimeInstalled() < pi2.GetTimeInstalled());
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

SortKey PackageInfoComparer::sortKey(SortKey::PackageId);

bool PackageInfoComparer::reverse = false;

class UpdateInfoComparer
{
public:
  bool operator() (const PackageInstaller::UpdateInfo& ui1, const PackageInstaller::UpdateInfo& ui2) const
  {
    return PathName::Compare(ui1.packageId, ui2.packageId) < 0;
  }
};

class UpgradeInfoComparer
{
public:
  bool operator() (const PackageInstaller::UpgradeInfo& upg1, const PackageInstaller::UpgradeInfo& upg2) const
  {
    return PathName::Compare(upg1.packageId, upg2.packageId) < 0;
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
  void MIKTEXTHISCALL ReportLine(const string& str) override;

public:
  bool MIKTEXTHISCALL OnRetryableError(const string& message) override;

public:
  bool MIKTEXTHISCALL OnProgress(MiKTeX::Packages::Notification nf) override;

public:
  void Main(int argc, const char** argv);

private:
  void Verbose(const string& s);

private:
  void Warn(const string& s);

private:
  void SecurityRisk(const string& s);

private:
  void Message(const string& s);

private:
  MIKTEXNORETURN void Error(const string& s);

private:
  void UpdateDb();

private:
  void Install(const vector<string>& toBeInstalled, const vector<string>& toBeRemoved);

#if defined(MIKTEX_WINDOWS)
private:
  void RegisterComponents(bool doRegister);
#endif

private:
  void Verify(const vector<string>& toBeVerified);

private:
  void VerifyMiKTeX();

private:
  void FindConflicts();

private:
  void ImportPackage(const string& packageId, vector<string>& toBeinstalled);

private:
  void ImportPackages(vector<string>& toBeinstalled);

private:
  void FindUpdates();

private:
  void Update(const vector<string>& updates);

private:
  void FindUpgrades(PackageLevel packageLevel);

private:
  void Upgrade(PackageLevel packageLevel);

private:
  string GetDirectories(const string& packageId);

private:
  void List(OutputFormat outputFormat, int maxCount);

private:
  void ListRepositories(OutputFormat outputFormat);

private:
  void CheckRepositories();

private:
  void PickRepositoryUrl();

private:
  void PrintFiles(const vector<string>& files);

private:
  void PrintPackageInfo(const string& packageId);

private:
  void RestartWindowed();

private:
  static void SignalHandler(int sig);

private:
  shared_ptr<PackageManager> packageManager;

private:
  shared_ptr<Session> session;

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
  vector<TraceCallback::TraceMessage> pendingTraceMessages;

public:
  bool MIKTEXTHISCALL Trace(const TraceCallback::TraceMessage& traceMessage) override
  {
    if (!isLog4cxxConfigured)
    {
      if (pendingTraceMessages.size() > 100)
      {
        pendingTraceMessages.clear();
      }
      pendingTraceMessages.push_back(traceMessage);
      return true;
    }
    FlushPendingTraceMessages();
    TraceInternal(traceMessage);
    return true;
  }

private:
  void FlushPendingTraceMessages()
  {
    for (const TraceCallback::TraceMessage& msg : pendingTraceMessages)
    {
      TraceInternal(msg);
    }
    pendingTraceMessages.clear();
  }

private:
  void TraceInternal(const TraceCallback::TraceMessage& traceMessage)
  {
    log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(string("trace.mpmcli.") + traceMessage.facility);
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

public:
  static void Sorry(const string& description, const string& remedy, const string& url);

public:
  static void Sorry()
  {
    Sorry("", "", "");
  }
};

enum Option
{
  OPT_AAA = 1,
  OPT_ADMIN,
  OPT_CHECK_REPOSITORIES,       // EXPERIMENTAL
  OPT_CSV,                      // deprecated
  OPT_FIND_CONFLICTS,           // internal
  OPT_FIND_UPDATES,
  OPT_FIND_UPGRADES,
  OPT_HHELP,
  OPT_IMPORT,
  OPT_IMPORT_ALL,
  OPT_INSTALL,
  OPT_INSTALL_SOME,             // deprecated
  OPT_LIST,
  OPT_LIST_PACKAGE_NAMES,
  OPT_LIST_REPOSITORIES,
  OPT_MAX_COUNT,                // experimental
  OPT_OUTPUT_FORMAT,            // experimental
  OPT_PACKAGE_LEVEL,
  OPT_PICK_REPOSITORY_URL,
  OPT_PRINT_PACKAGE_INFO,
  OPT_PROXY,                    // experimental
  OPT_PROXY_PASSWORD,           // experimental
  OPT_PROXY_USER,               // experimental
  OPT_QUIET,
  OPT_REGISTER_COMPONENTS,      // experimental
  OPT_REPOSITORY,
  OPT_REPOSITORY_RELEASE_STATE,
  OPT_REQUIRE,
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
  OPT_UPDATE_SOME,              // deprecated
  OPT_UPGRADE,
  OPT_VERBOSE,
  OPT_VERIFY,
  OPT_VERIFY_MIKTEX,
  OPT_VERSION,
};

const struct poptOption Application::aoption[] = {

  {
    "admin", 0, POPT_ARG_NONE, 0, OPT_ADMIN,
    T_("Run in administrator mode."),
    nullptr,
  },

  {
    // EXPERIMENTAL
    "check-repositories", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_CHECK_REPOSITORIES,
    nullptr,
    nullptr
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

  {
    "find-upgrades", 0, POPT_ARG_NONE, nullptr, OPT_FIND_UPGRADES,
    T_("Search for packages that must be installed in order to complete the setup.  Works in conjunction with --package-level."),
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
    T_("Install the specified packages."),
    T_("[@]PACKAGELIST")
  },

#if ENABLE_OPT_INSTALL_SOME
  {                             // deprecated
    "install-some", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_INSTALL_SOME,
    nullptr,
    nullptr
  },
#endif

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
    "package-level", 0, POPT_ARG_STRING, nullptr, OPT_PACKAGE_LEVEL,
    T_("Use the specified package level when doing an upgrade.  Level must be one of: essential, basic, complete."),
    T_("LEVEL")
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
    "require", 0, POPT_ARG_STRING, nullptr, OPT_REQUIRE,
    T_("Make sure that the specified packages are installed."),
    T_("[@]PACKAGELIST")
  },

  {
    "reverse", 0, POPT_ARG_NONE, nullptr, OPT_REVERSE,
    T_("Reverse the result of comparisons."),
    nullptr
  },

  {
    "set-repository", 0, POPT_ARG_STRING, nullptr, OPT_SET_REPOSITORY,
    T_("Register the location of the default package repository.  The location can be either a fully qualified path name (a local package repository) or an URL (a remote package repository)."),
    T_("LOCATION")
  },

  {
    "trace", 0, POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr, OPT_TRACE,
    T_("Turn on tracing.  TRACESTREAMS, if specified, is a comma-separated list of trace stream names (see the MiKTeX manual)."),
    T_("TRACESTREAMS")
  },

  {
    "uninstall", 0, POPT_ARG_STRING, nullptr, OPT_UNINSTALL,
    T_("Uninstall the specified packages."),
    T_("[@]PACKAGELIST")
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
    T_("Update specified packages, if an updated version is available in the package repository.  Install all updateable packages, if the package list is omitted."),
    T_("[@]PACKAGELIST")
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

#if ENABLE_OPT_UPDATE_SOME
  {                             // deprecated
    "update-some", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_UPDATE_SOME,
    nullptr,
    nullptr
  },
#endif

  {
    "upgrade", 0, POPT_ARG_NONE, nullptr, OPT_UPGRADE,
    T_("Upgrade the MiKTeX setup to a package level (works in conjunction with --package-level)."),
    nullptr
  },

  {
    "verbose", 0, POPT_ARG_NONE, nullptr, OPT_VERBOSE,
    T_("Turn on verbose output mode."),
    nullptr
  },

  {
    "verify", 0, POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr, OPT_VERIFY,
    T_("Verify the integrity of the installed packages."),
    T_("[@]PACKAGELIST")
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

void Application::Message(const string& s)
{
  if (isLog4cxxConfigured)
  {
    LOG4CXX_INFO(logger, s);
  }
  if (!quiet)
  {
    cout << s << endl;
  }
}

void Application::Verbose(const string& s)
{
  if (isLog4cxxConfigured)
  {
    LOG4CXX_INFO(logger, s);
  }
  if (verbose)
  {
    cout << s << endl;
  }
}

void Application::Warn(const string& s)
{
  if (isLog4cxxConfigured)
  {
    LOG4CXX_WARN(logger, s);
  }
  cerr << T_("Warning:") << " " << s << endl;
}

void Application::SecurityRisk(const string& s)
{
  if (isLog4cxxConfigured)
  {
    LOG4CXX_WARN(logger, T_("security risk:") << " " << s);
  }
  cerr << T_("security risk:") << " " << s << endl;
}

void Application::Sorry(const string& description, const string& remedy, const string& url)
{
  if (cerr.fail())
  {
    return;
  }

  cerr << endl;
  if (description.empty())
  {
    cerr << fmt::format(T_("Sorry, but {0} did not succeed."), Q_(THE_NAME_OF_THE_GAME)) << endl;
  }
  else
  {
    cerr
      << fmt::format(T_("Sorry, but {0} did not succeed for the following reason:"), Q_(THE_NAME_OF_THE_GAME)) << "\n"
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
  if (isLog4cxxConfigured)
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

MIKTEXNORETURN void Application::Error(const string& s)
{
  if (isLog4cxxConfigured)
  {
    LOG4CXX_FATAL(logger, s);
  }
  Sorry(s, "", "");
  throw 1;
}

void Application::ReportLine(const string& str)
{
  Verbose(str);
}

bool Application::OnRetryableError(const string& message)
{
  return false;
}

bool Application::OnProgress(MiKTeX::Packages::Notification nf)
{
  if (interrupted != 0)
  {
    return false;
  }
  return interrupted == 0;
}

void Application::UpdateDb()
{
  shared_ptr<PackageInstaller> installer(packageManager->CreateInstaller({ this, true, true }));
  if (!repository.empty())
  {
    installer->SetRepository(repository);
  }
  installer->UpdateDb({});
  installer->Dispose();
}

void Application::Install(const vector<string>& toBeInstalled, const vector<string>& toBeRemoved)
{
  for (const string& packageId : toBeInstalled)
  {
    PackageInfo packageInfo = packageManager->GetPackageInfo(packageId);
    if (packageInfo.IsInstalled())
    {
      Error(fmt::format(T_("Package \"{0}\" is already installed."), packageId));
    }
  }

  for (const string& packageId : toBeRemoved)
  {
    PackageInfo packageInfo = packageManager->GetPackageInfo(packageId);
    if (!packageInfo.IsInstalled())
    {
      Error(fmt::format(T_("Package \"{0}\" is not installed."), packageId));
    }
  }

  shared_ptr<PackageInstaller> installer(packageManager->CreateInstaller({ this, true, true }));

  if (!repository.empty())
  {
    installer->SetRepository(repository);
  }

  installer->SetFileLists(toBeInstalled, toBeRemoved);
  installer->InstallRemove(PackageInstaller::Role::Application);
  installer->Dispose();
  if (toBeInstalled.size() == 1)
  {
    Message(fmt::format(T_("Package \"{0}\" has been successfully installed."), toBeInstalled[0]));
  }
  else if (toBeInstalled.size() > 1)
  {
    Message(fmt::format(T_("{0} packages have been successfully installed."), toBeInstalled.size()));
  }
  if (toBeRemoved.size() == 1)
  {
    Message(fmt::format(T_("Package \"{0}\" has been successfully removed."), toBeRemoved[0]));
  }
  else if (toBeRemoved.size() > 1)
  {
    Message(fmt::format(T_("{0} packages have been successfully removed."), toBeRemoved.size()));
  }
}

#if defined(MIKTEX_WINDOWS)

void Application::RegisterComponents(bool doRegister)
{
  shared_ptr<PackageInstaller> installer(packageManager->CreateInstaller({ this, true, true }));
  installer->RegisterComponents(doRegister);
  installer->Dispose();
}

#endif

void Application::FindConflicts()
{
  map<string, vector<string> > filesAndPackages;
  unique_ptr<PackageIterator> packageIterator(packageManager->CreateIterator());
  PackageInfo packageInfo;
  while (packageIterator->GetNext(packageInfo))
  {
    for (const string& fileName : packageInfo.runFiles)
    {
      PathName file(fileName);
      file.TransformForComparison();
      filesAndPackages[file.GetData()].push_back(packageInfo.id);
    }
    for (const string& fileName : packageInfo.docFiles)
    {
      PathName file(fileName);
      file.TransformForComparison();
      filesAndPackages[file.GetData()].push_back(packageInfo.id);
    }
    for (const string& fileName : packageInfo.sourceFiles)
    {
      PathName file(fileName);
      file.TransformForComparison();
      filesAndPackages[file.GetData()].push_back(packageInfo.id);
    }
  }
  for (const auto& package : filesAndPackages)
  {
    if (package.second.size() > 1)
    {
      cout << package.first << endl;
      for (const string& fileName : package.second)
      {
        cout << "  " << fileName << endl;
      }
    }
  }
}

void Application::VerifyMiKTeX()
{
  vector<string> toBeVerified;
  unique_ptr<PackageIterator> packageIterator(packageManager->CreateIterator());
  PackageInfo packageInfo;
  while (packageIterator->GetNext(packageInfo))
  {
    if (!packageInfo.IsPureContainer()
      && packageInfo.IsInstalled()
      && packageInfo.id.compare(0, 7, "miktex-") == 0)
    {
      toBeVerified.push_back(packageInfo.id);
    }
  }
  Verify(toBeVerified);
}

void Application::Verify(const vector<string>& toBeVerifiedArg)
{
  vector<string> toBeVerified = toBeVerifiedArg;
  bool verifyAll = toBeVerified.empty();
  if (verifyAll)
  {
    unique_ptr<PackageIterator> packageIterator(packageManager->CreateIterator());
    PackageInfo packageInfo;
    while (packageIterator->GetNext(packageInfo))
    {
      if (!packageInfo.IsPureContainer() && packageInfo.IsInstalled())
      {
        toBeVerified.push_back(packageInfo.id);
      }
    }
  }
  bool ok = true;
  for (const string& packageId : toBeVerified)
  {
    if (!packageManager->TryVerifyInstalledPackage(packageId))
    {
      Message(fmt::format(T_("{0}: this package needs to be reinstalled."), packageId));
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
        Message(fmt::format(T_("Package {0} is correctly installed."), toBeVerified[0]));
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

void Application::ImportPackage(const string& packageId, vector<string>& toBeinstalled)
{
  if (repository.empty())
  {
    Error(T_("You have to use --repository=/PATH/TO/MIKTEX."));
  }
  PathName packagesIni(repository);
  packagesIni /= MIKTEX_PATH_PACKAGES_INI;
  if (!File::Exists(packagesIni))
  {
    Error(fmt::format(T_("Not a MiKTeX installation directory: {0}"), repository));
  }
  unique_ptr<Cfg> cfg = Cfg::Create();
  cfg->Read(packagesIni);
  if (strncmp(packageId.c_str(), "miktex-", 7) == 0)
  {
    Error(fmt::format(T_("Cannot import package {0}."), packageId));
  }
  string str;
  if (!cfg->TryGetValueAsString(packageId, "TimeInstalled", str) || str.empty() || str == "0")
  {
    Error(fmt::format(T_("Package {0} is not installed."), packageId));
  }
  if (cfg->TryGetValueAsString(packageId, T_("Obsolete"), str) && str == "1")
  {
    Error(fmt::format(T_("Package {0} is obsolete."), packageId));
  }
  PackageInfo packageInfo;
  if (!packageManager->TryGetPackageInfo(packageId.c_str(), packageInfo))
  {
    Error(fmt::format(T_("Unknown package: {0}."), packageId));
  }
  if (packageInfo.IsInstalled())
  {
    Error(fmt::format(T_("Package {0} is already installed."), packageId));
  }
  toBeinstalled.push_back(packageId);
}

void Application::ImportPackages(vector<string>& toBeinstalled)
{
  if (repository.empty())
  {
    Error(T_("You have to use --repository=/PATH/TO/MIKTEX."));
  }
  PathName packagesIni(repository);
  packagesIni /= MIKTEX_PATH_PACKAGES_INI;
  if (!File::Exists(packagesIni))
  {
    Error(fmt::format(T_("Not a MiKTeX installation directory: {0}"), repository));
  }
  unique_ptr<Cfg> cfg = Cfg::Create();
  cfg->Read(packagesIni);
  for (const shared_ptr<Cfg::Key>& key : *cfg)
  {
    if (strncmp(key->GetName().c_str(), "miktex-", 7) == 0)
    {
      continue;
    }
    string str;
    if (!cfg->TryGetValueAsString(key->GetName(), "TimeInstalled", str) || str.empty() || str == "0")
    {
      continue;
    }
    if (cfg->TryGetValueAsString(key->GetName(), "Obsolete", str) && str == "1")
    {
      continue;
    }
    PackageInfo packageInfo;
    if (!packageManager->TryGetPackageInfo(key->GetName(), packageInfo) || packageInfo.IsInstalled())
    {
      continue;
    }
    toBeinstalled.push_back(key->GetName());
  }
}

void Application::FindUpdates()
{
  shared_ptr<PackageInstaller> installer(packageManager->CreateInstaller({ this, true, true }));
  if (!repository.empty())
  {
    installer->SetRepository(repository);
  }
  installer->FindUpdates();
  vector<PackageInstaller::UpdateInfo> updates = installer->GetUpdates();
  installer->Dispose();
  if (updates.empty())
  {
    Message(T_("There are currently no updates available."));
  }
  else
  {
    sort(updates.begin(), updates.end(), UpdateInfoComparer());
    for (const PackageInstaller::UpdateInfo& upd : updates)
    {
      switch (upd.action)
      {
      case PackageInstaller::UpdateInfo::Repair:
      case PackageInstaller::UpdateInfo::ReleaseStateChange:
      case PackageInstaller::UpdateInfo::Update:
      case PackageInstaller::UpdateInfo::ForceUpdate:
        cout << upd.packageId << endl;
        break;
      default:
        break;
      }      
    }
  }
}

void Application::Update(const vector<string>& requestedUpdates)
{
  shared_ptr<PackageInstaller> installer(packageManager->CreateInstaller({ this, true, true }));
  if (!repository.empty())
  {
    installer->SetRepository(repository);
  }
  installer->FindUpdates();
  vector<string> serverUpdates;
  vector<string> toBeRemoved;
  for (const PackageInstaller::UpdateInfo& upd : installer->GetUpdates())
  {
    switch (upd.action)
    {
    case PackageInstaller::UpdateInfo::Repair:
    case PackageInstaller::UpdateInfo::ReleaseStateChange:
    case PackageInstaller::UpdateInfo::Update:
    case PackageInstaller::UpdateInfo::ForceUpdate:
      serverUpdates.push_back(upd.packageId);
      break;
    case PackageInstaller::UpdateInfo::ForceRemove:
      toBeRemoved.push_back(upd.packageId);
      break;
    default:
      break;
    }
  }
  vector<string> toBeInstalled;
  if (requestedUpdates.empty())
  {
    if (serverUpdates.empty())
    {
      Message(T_("There are currently no updates available."));
    }
    else
    {
      toBeInstalled = serverUpdates;
    }
  }
  else
  {
    toBeRemoved.clear();
    sort(serverUpdates.begin(), serverUpdates.end());
    for (const string& packageId : requestedUpdates)
    {
      PackageInfo packageInfo = packageManager->GetPackageInfo(packageId);
      if (!packageInfo.IsInstalled())
      {
        Error(fmt::format(T_("Package \"{0}\" is not installed."), packageId));
      }
      if (binary_search(serverUpdates.begin(), serverUpdates.end(), packageId))
      {
        toBeInstalled.push_back(packageId);
      }
      else
      {
        Message(fmt::format(T_("Package \"{0}\" is up to date."), packageId));
      }
    }
  }
  if (toBeInstalled.empty() && toBeRemoved.empty())
  {
    return;
  }
  installer->SetFileLists(toBeInstalled, toBeRemoved);
  installer->InstallRemove(PackageInstaller::Role::Updater);
  installer = nullptr;
  unique_ptr<SetupService> service = SetupService::Create();
  SetupOptions options = service->GetOptions();
  options.Task = SetupTask::FinishUpdate;
  options = service->SetOptions(options);
  service->Run();
  service = nullptr;
  if (toBeInstalled.size() == 1)
  {
    Message(fmt::format(T_("Package \"{0}\" has been successfully updated."), toBeInstalled[0]));
  }
  else if (toBeInstalled.size() > 1)
  {
    Message(fmt::format(T_("{0} packages have been successfully updated."), toBeInstalled.size()));
  }
}

void Application::FindUpgrades(PackageLevel packageLevel)
{
  if (packageLevel == PackageLevel::None)
  {
    Error("No package level (--package-level) was specified.");
  }
  shared_ptr<PackageInstaller> installer(packageManager->CreateInstaller({ this, true, true }));
  if (!repository.empty())
  {
    installer->SetRepository(repository);
  }
  installer->FindUpgrades(packageLevel);
  vector<PackageInstaller::UpgradeInfo> upgrades = installer->GetUpgrades();
  installer->Dispose();
  if (upgrades.empty())
  {
    Message(T_("There are currently no upgrades available."));
    return;
  }
  sort(upgrades.begin(), upgrades.end(), UpgradeInfoComparer());
  for (const PackageInstaller::UpgradeInfo& upg : upgrades)
  {
    cout << upg.packageId << endl;
  }
}

void Application::Upgrade(PackageLevel packageLevel)
{
  if (packageLevel == PackageLevel::None)
  {
    Error(T_("No package level (--package-level) was specified."));
  }
  shared_ptr<PackageInstaller> installer(packageManager->CreateInstaller({ this, true, true }));
  if (!repository.empty())
  {
    installer->SetRepository(repository);
  }
  installer->FindUpgrades(packageLevel);
  vector<PackageInstaller::UpgradeInfo> upgrades = installer->GetUpgrades();
  if (upgrades.empty())
  {
    Message(T_("There are currently no upgrades available."));
    return;
  }
  vector<string> toBeInstalled;
  for (const PackageInstaller::UpgradeInfo& upg : upgrades)
  {
    toBeInstalled.push_back(upg.packageId);
  }
  sort(toBeInstalled.begin(), toBeInstalled.end());
  installer->SetFileLists(toBeInstalled, vector<string>());
  installer->InstallRemove(PackageInstaller::Role::Application);
  if (toBeInstalled.size() == 1)
  {
    Message(fmt::format(T_("Package \"{0}\" has been successfully installed."), toBeInstalled[0]));
  }
  else if (toBeInstalled.size() > 1)
  {
    Message(fmt::format(T_("{0} packages have been successfully installed."), toBeInstalled.size()));
  }
}

string Application::GetDirectories(const string& packageId)
{
  set<string> directories;
  PackageInfo pi = packageManager->GetPackageInfo(packageId);
  for (const string& fileName : pi.runFiles)
  {
    PathName path(fileName);
    if (!path.HasExtension(MIKTEX_PACKAGE_MANIFEST_FILE_SUFFIX))
    {
      directories.insert(path.RemoveFileSpec().ToString());
    }
  }
  string ret;
  for (const string& dir : directories)
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
  unique_ptr<PackageIterator> packageIterator(packageManager->CreateIterator());
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
        << fmt::format("{} {:05} {:10} {}", it->IsInstalled() ? 'i' : '-', it->GetNumFiles(), it->GetSize(), it->id)
        << endl;
    }
    else if (outputFormat == OutputFormat::CSV)
    {
      string path = packageManager->GetContainerPath(it->id, false);
      string directories = GetDirectories(it->id);
      cout << fmt::format("{}\\{},{}", path, it->id, directories) << endl;
    }
    else if (outputFormat == OutputFormat::PackageIdentifiers)
    {
      cout << it->id << endl;
    }
  }
}

class CountryComparer
{
public:
  inline bool operator() (const RepositoryInfo& lhs, const RepositoryInfo& rhs)
  {
    if (lhs.ranking == rhs.ranking)
    {
      return StringCompare(lhs.country.c_str(), rhs.country.c_str(), true) < 0;
    }
    else
    {
      return lhs.ranking < rhs.ranking;
    }
  }
};

class DataTransferRateComparer
{
public:
  inline bool operator() (const RepositoryInfo& lhs, const RepositoryInfo& rhs)
  {
    return lhs.dataTransferRate > rhs.dataTransferRate;
  }
};

void Application::ListRepositories(OutputFormat outputFormat)
{
  packageManager->DownloadRepositoryList();
  vector<RepositoryInfo> repositories = packageManager->GetRepositories();
  if (repositories.empty())
  {
    Message(T_("No package repositories are currently available."));
  }
  sort(repositories.begin(), repositories.end(), CountryComparer());
  for (const RepositoryInfo& ri : repositories)
  {
    cout << ri.url << endl;
  }
}

void Application::CheckRepositories()
{
  packageManager->DownloadRepositoryList();
  vector<RepositoryInfo> repositories = packageManager->GetRepositories();
  if (repositories.empty())
  {
    Message(T_("No package repositories are currently available."));
  }
  sort(repositories.begin(), repositories.end(), CountryComparer());
  for (RepositoryInfo& ri : repositories)
  {
    ri = packageManager->CheckPackageRepository(ri.url);
    cout << std::fixed << std::setprecision(2) << ri.dataTransferRate / 125000.0 << " Mbit/s - " << ri.url << endl;
  }
#if 0
  sort(repositories.begin(), repositories.end(), DataTransferRateComparer());
  for (const RepositoryInfo& ri : repositories)
  {
    cout << std::fixed << std::setprecision(2) << ri.dataTransferRate / 125000.0 << " Mbit/s - " << ri.url << endl;
  }
#endif
}

void Application::PickRepositoryUrl()
{
  cout << packageManager->PickRepositoryUrl() << endl;
}

void Application::PrintFiles(const vector<string>& files)
{
  for (const string& fileName : files)
  {
    PathName path = session->GetSpecialPath(SpecialPath::InstallRoot);
    string disp;
    if (!PackageManager::StripTeXMFPrefix(fileName, disp))
    {
      disp = fileName;
    }
    path /= disp;
    cout << "  " << path << endl;
  }
}

void Application::PrintPackageInfo(const string& packageId)
{
  PackageInfo packageInfo = packageManager->GetPackageInfo(packageId);
  cout << T_("name:") << " " << packageInfo.id << endl;
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

  vector<string> options{ "", "--start-page", "packages" };

  string miktexConsoleName;
#if defined(MIKTEX_WINDOWS)
  miktexConsoleName = session->IsAdminMode() ? MIKTEX_CONSOLE_ADMIN_EXE : MIKTEX_CONSOLE_EXE;
#elif defined(MIKTEX_MACOS_BUNDLE)
  miktexConsoleName = MIKTEX_MACOS_BUNDLE_NAME;
#else
  miktexConsoleName = MIKTEX_CONSOLE_EXE;
#endif

  options[0] = miktexConsoleName;

  if (session->IsAdminMode())
  {
    options.push_back("--admin");
  }  

  // locate miktex-console
  PathName miktexConsole;
#if defined(MIKTEX_MACOS_BUNDLE)
  miktexConsole = session->GetSpecialPath(SpecialPath::MacOsDirectory) / PathName(MIKTEX_MACOS_BUNDLE_NAME);
#else
  if (!session->FindFile(miktexConsoleName, FileType::EXE, miktexConsole))
  {
    Error(T_("Could not restart in windowed mode."));
  }
#endif

  Process::Start(miktexConsole, options);
}

void ReadNames(const PathName& path, vector<string>& list)
{
  StreamReader reader(path);
  string line;
  while (reader.ReadLine(line))
  {
    Tokenizer tok(line, " \t\n\r");
    if (tok)
    {
      string name = *tok;
      if (name[0] == '@')
      {
        // RECURSION
        ReadNames(PathName(&name[1]), list);
      }
      else
      {
        list.push_back(name);
      }
    }
  }
  reader.Close();
}

void ParseList(const string& s, vector<string>& list)
{
  if (s.length() > 0 && s[0] == '@')
  {
    ReadNames(PathName(&s[1]), list);
  }
  else
  {
    for (Tokenizer tok(s, " ,;"); tok; ++tok)
    {
      list.push_back(*tok);
    }
  }
}

void Application::Main(int argc, const char** argv)
{
#if defined(MIKTEX_WINDOWS)
  UINT activeOutputCodePage = GetConsoleOutputCP();
  if (activeOutputCodePage != CP_UTF8)
  {
    SetConsoleOutputCP(CP_UTF8);
  }
#endif
  bool optAdmin = false;
  bool optCheckRepositories = false;
  bool optFindConflicts = false;
  bool optFindUpdates = false;
  bool optFindUpgrades = false;
  bool optImport = false;
  bool optImportAll = false;
  bool optList = false;
  bool optListRepositories = false;
  PackageLevel optPackageLevel = PackageLevel::None;
  bool optPickRepositoryUrl = false;
  bool optPrintPackageInfo = false;
  bool optSetRepository = false;
  bool optSort = false;
  bool optUpdate = false;
  bool optUpdateAll = false;
  bool optUpdateDb = false;
  bool optUpdateFndb = false;
  bool optUpgrade = false;
  bool optVerify = false;
  bool optVerifyMiKTeX = false;
  bool optVersion = false;
  int optMaxCount = INT_MAX;
  int optProxyPort = -1;
#if defined(MIKTEX_WINDOWS)
  bool optRegisterComponents = false;
  bool optUnregisterComponents = false;
#endif
  OutputFormat outputFormat(OutputFormat::Listing);
  string packageId;
  string optProxy;
  string optProxyPassword;
  string optProxyUser;
  string toBeImported;
  vector<string> toBeInstalled;
  vector<string> toBeRemoved;
  vector<string> toBeVerified;
  vector<string> updates;
  vector<string> required;
  RepositoryReleaseState optRepositoryReleaseState = RepositoryReleaseState::Unknown;

  bool changeProxy = false;

  PoptWrapper popt(argc, argv, aoption);
  int option;

  // initialize session
  Session::InitInfo initInfo;
  initInfo.SetTraceCallback(this);
  initInfo.SetProgramInvocationName(argv[0]);
  while ((option = popt.GetNextOpt()) >= 0)
  {
    string optArg = popt.GetOptArg();
    switch (option)
    {
    case OPT_TRACE:
      if (optArg.empty())
      {
        initInfo.SetTraceFlags(StringUtil::Flatten(DEFAULT_TRACE_OPTIONS, ','));
      }
      else
      {
        initInfo.SetTraceFlags(optArg);
      }
    }
  }
  session = Session::Create(initInfo);

  // process command-line options
  popt.Reset();
  while ((option = popt.GetNextOpt()) >= 0)
  {
    string optArg = popt.GetOptArg();
    switch (option)
    {
    case OPT_ADMIN:
      optAdmin = true;
      break;
    case OPT_CHECK_REPOSITORIES:
      optCheckRepositories = true;
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
    case OPT_FIND_UPGRADES:
      optFindUpgrades = true;
      break;
#if defined (MIKTEX_WINDOWS)
    case OPT_HHELP:
    {
      session->ShowManualPageAndWait(0, MIKTEXHELP_MPMCON);
      session = nullptr;
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
      ParseList(optArg, toBeInstalled);
      break;
    case OPT_INSTALL_SOME:
#if 0
      // TODO
      Warn(T_("Option --install-some is deprecated"));
#endif
      ReadNames(PathName(optArg), toBeInstalled);
      break;
    case OPT_LIST:
      optList = true;
      break;
    case OPT_LIST_PACKAGE_NAMES:
      optList = true;
      outputFormat = OutputFormat::PackageIdentifiers;
      break;
    case OPT_LIST_REPOSITORIES:
      optListRepositories = true;
      break;
    case OPT_PACKAGE_LEVEL:
      if (optArg == "essential")
      {
        optPackageLevel = PackageLevel::Essential;
      }
      else if (optArg == "basic")
      {
        optPackageLevel = PackageLevel::Basic;
      }
      else if (optArg == "complete")
      {
        optPackageLevel = PackageLevel::Complete;
      }
      else
      {
        Error(T_("Unknown package level."));
      }
      break;
    case OPT_PICK_REPOSITORY_URL:
      optPickRepositoryUrl = true;
      break;
    case OPT_PRINT_PACKAGE_INFO:
      optPrintPackageInfo = true;
      packageId = optArg;
      break;
    case OPT_PROXY:
    {
      changeProxy = true;
      Tokenizer tok(optArg, ":");
      optProxy = *tok;
      ++tok;
      if (tok)
      {
        optProxyPort = std::stoi(*tok);
      }
    }
    break;
    case OPT_SORT:
      optSort = true;
      if (Utils::EqualsIgnoreCase(optArg, "deploymentname"))
      {
        PackageInfoComparer::sortKey = SortKey::PackageId;
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
        outputFormat = OutputFormat::PackageIdentifiers;
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
    case OPT_REQUIRE:
      ParseList(optArg, required);
      break;
    case OPT_REVERSE:
      PackageInfoComparer::reverse = true;
      break;
    case OPT_SET_REPOSITORY:
      optSetRepository = true;
      repository = optArg;
      break;
    case OPT_TRACE:
      // see above
      break;
    case OPT_UNINSTALL:
      ParseList(optArg, toBeRemoved);
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
        ParseList(optArg, updates);
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
#if 0
      // TODO
      Warn(T_("Option --update-some is deprecated"));
#endif
      if (optUpdateAll)
      {
        Error(T_("Already updating all packages."));
      }
      optUpdate = true;
      ReadNames(PathName(optArg), updates);
      break;
    case OPT_UPGRADE:
      optUpgrade = true;
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
        ParseList(optArg, toBeVerified);
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
    Error(msg);
  }

  if (!popt.GetLeftovers().empty())
  {
    Error(T_("This utility does not accept non-option arguments."));
  }

  if (optVersion)
  {
    cout
      << Utils::MakeProgramVersionString(THE_NAME_OF_THE_GAME, VersionNumber(MIKTEX_COMPONENT_VERSION_STR)) << endl
      << endl
      << MIKTEX_COMP_COPYRIGHT_STR << endl
      << endl
      << "This is free software; see the source for copying conditions.  There is NO" << endl
      << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
    return;
  }

  if (optAdmin)
  {
    if (!session->IsSharedSetup())
    {
      Error(T_("Option --admin only makes sense for a shared MiKTeX setup."));
    }
    if (!session->RunningAsAdministrator())
    {
      Warn(T_("Option --admin may require administrator privileges"));
    }
    session->SetAdminMode(true);
  }

  if (session->RunningAsAdministrator() && !session->IsAdminMode())
  {
    SecurityRisk(T_("running with elevated privileges"));
  }

  PathName xmlFileName;
  if (session->FindFile("mpmcli." MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName)
    || session->FindFile(MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName))
  {
    PathName logDir = session->GetSpecialPath(SpecialPath::LogDirectory);
    string logName = "mpmcli";
    if (optAdmin && session->RunningAsAdministrator())
    {
      logName += MIKTEX_ADMIN_SUFFIX;
    }
    Utils::SetEnvironmentString("MIKTEX_LOG_DIR", logDir.ToString());
    Utils::SetEnvironmentString("MIKTEX_LOG_NAME", logName);
    log4cxx::xml::DOMConfigurator::configure(xmlFileName.ToWideCharString());
    isLog4cxxConfigured = true;
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
    LOG4CXX_INFO(logger, "this is " << Utils::MakeProgramVersionString("mpmcli", VersionNumber(MIKTEX_COMPONENT_VERSION_STR)));
    LOG4CXX_INFO(logger, "this process (" << thisProcess->GetSystemId() << ") started by '" << invokerName << "' with command line: " << CommandLineBuilder(argc, argv));
#if defined(MIKTEX_WINDOWS)
    if (activeOutputCodePage != CP_UTF8)
    {
      LOG4CXX_DEBUG(logger, fmt::format("changed console output code page from {0} to {1}", activeOutputCodePage, GetConsoleOutputCP()));
    }
#endif
  }

  if (session->IsAdminMode())
  {
    Verbose(T_("Operating on the shared (system-wide) MiKTeX setup"));
  }
  else
  {
    Verbose(T_("Operating on the private (per-user) MiKTeX setup"));
  }

  packageManager = PackageManager::Create(PackageManager::InitInfo(this));

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
    packageManager->SetProxy(proxySettings);
  }

  bool restartWindowed = true;

  if (optSetRepository)
  {
    packageManager->SetDefaultPackageRepository(RepositoryType::Unknown, repository);
    restartWindowed = false;
  }

  if (optRepositoryReleaseState != RepositoryReleaseState::Unknown)
  {
    packageManager->SetRepositoryReleaseState(optRepositoryReleaseState);
  }

  if (optUpdateFndb && !optUpdateDb)
  {
    packageManager->CreateMpmFndb();
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

  if (optFindUpgrades)
  {
    FindUpgrades(optPackageLevel);
    restartWindowed = false;
  }

  if (!required.empty())
  {
    restartWindowed = false;
  }

  for (const string& package : required)
  {
    PackageInfo packageInfo;
    if (!packageManager->TryGetPackageInfo(package, packageInfo))
    {
      Error(fmt::format(T_("{0}: unknown package"), package));
    }
    if (!packageInfo.IsInstalled())
    {
      toBeInstalled.push_back(package);
    }
  }

  if (!toBeInstalled.empty() || !toBeRemoved.empty())
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

  if (optUpdateAll || !updates.empty())
  {
    Update(updates);
    restartWindowed = false;
  }

  if (optUpgrade)
  {
    Upgrade(optPackageLevel);
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

  if (optCheckRepositories)
  {
    CheckRepositories();
    restartWindowed = false;
  }

  if (optPickRepositoryUrl)
  {
    PickRepositoryUrl();
    restartWindowed = false;
  }

  if (optPrintPackageInfo)
  {
    PrintPackageInfo(packageId);
    restartWindowed = false;
  }

  if (restartWindowed)
  {
    RestartWindowed();
  }

  packageManager = nullptr;
  session = nullptr;
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

int MAIN(int argc, MAINCHAR* argv[])
{
#if defined(MIKTEX_WINDOWS)
  HRESULT hr = CoInitialize(nullptr);
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
    vector<const char*> newargv;
    newargv.reserve(static_cast<size_t>(argc) + 1);
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
  catch (const MiKTeXException& e)
  {
    if (isLog4cxxConfigured)
    {
      LOG4CXX_FATAL(logger, e.GetErrorMessage());
      LOG4CXX_FATAL(logger, "Info: " << e.GetInfo());
      LOG4CXX_FATAL(logger, "Source: " << e.GetSourceFile());
      LOG4CXX_FATAL(logger, "Line: " << e.GetSourceLine());
    }
    Application::Sorry(e.GetDescription(), e.GetRemedy(), e.GetUrl());
    e.Save();
    retCode = 1;
  }
  catch (const exception& e)
  {
    if (isLog4cxxConfigured)
    {
      LOG4CXX_FATAL(logger, e.what());
    }
    Application::Sorry();
    retCode = 1;
  }
  catch (int rc)
  {
    retCode = rc;
  }
#if defined(MIKTEX_WINDOWS)
  CoUninitialize();
#endif
  if (logger != nullptr)
  {
    LOG4CXX_INFO(logger, "this process (" << Process::GetCurrentProcess()->GetSystemId() << ") finishes with exit code " << retCode);
    logger = nullptr;
  }
  return retCode;
}
