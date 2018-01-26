/* miktexsetup.cpp:

   Copyright (C) 2014-2018 Christian Schenk

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

#include "StdAfx.h"

#include "internal.h"

#if !defined(THE_NAME_OF_THE_GAME)
#  define THE_NAME_OF_THE_GAME T_("MiKTeX Setup Utility")
#endif

const char* DEFAULT_TRACE_STREAMS =
MIKTEX_TRACE_CORE ","
MIKTEX_TRACE_CURL ","
MIKTEX_TRACE_ERROR ","
MIKTEX_TRACE_FNDB ","
MIKTEX_TRACE_MPM
;

class Application :
  SetupServiceCallback
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

private:
  void ListRepositories();

private:
  void PrintInfo();

public:
  Application()
  {
    InstallSignalHandler(SIGINT);
    InstallSignalHandler(SIGTERM);
  }

public:
  virtual void ReportLine(const string& str);

public:
  virtual bool MIKTEXTHISCALL OnRetryableError(const string& message);

public:
  virtual bool MIKTEXTHISCALL OnProgress(MiKTeX::Setup::Notification nf);

public:
  virtual bool MIKTEXTHISCALL OnProcessOutput(const void* output, size_t n);

public:
  void Main(int argc, const char** argv);

private:
  void Verbose(const char* format, ...);

private:
  void Message(const char* format, ...);

private:
  MIKTEXNORETURN void Error(const char* format, ...);

private:
  static void SignalHandler(int sig);

private:
  unique_ptr<SetupService> pSetupService;

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
};

enum Option
{
  OPT_AAA = 1,
  OPT_COMMON_CONFIG,
  OPT_COMMON_DATA,
  OPT_COMMON_INSTALL,
  OPT_COMMON_ROOTS,
  OPT_LIST_REPOSITORIES,
  OPT_LOCAL_PACKAGE_REPOSITORY,
  OPT_MODIFY_PATH,
  OPT_PACKAGE_SET,
  OPT_PORTABLE,
  OPT_PRINT_INFO_ONLY,
  OPT_PRINT_VERSION_NUMBER,
#if defined(MIKTEX_WINDOWS)
  OPT_PROGRAM_FOLDER,
#endif
  OPT_QUIET,
  OPT_REMOTE_PACKAGE_REPOSITORY,
  OPT_SHARED,
  OPT_TRACE,
#if defined(MIKTEX_WINDOWS)
  OPT_USE_REGISTRY,
#endif
  OPT_USER_CONFIG,
  OPT_USER_DATA,
  OPT_USER_INSTALL,
  OPT_USER_ROOTS,
  OPT_VERBOSE,
  OPT_VERSION,
};

const struct poptOption Application::aoption[] = {

  {
    "common-config", 0, POPT_ARG_STRING, nullptr, OPT_COMMON_CONFIG,
    T_("Set the location of the common configuration directory. This option requires administrator privileges."),
    T_("DIR")
  },

  {
    "common-data", 0, POPT_ARG_STRING, nullptr, OPT_COMMON_DATA,
    T_("Set the location of the common data directory. This option requires administrator privileges."),
    T_("DIR")
  },

  {
    "common-install", 0, POPT_ARG_STRING, nullptr, OPT_COMMON_INSTALL,
    T_("Set the location of the common installation directory. This option requires administrator privileges."),
    T_("DIR")
  },

  {
    "common-roots", 0, POPT_ARG_STRING, nullptr, OPT_COMMON_ROOTS,
    T_("Register additional directories for all users. DIRS must be a semicolon-separated list of fully qualified path names. This option requires administrator privileges."),
    T_("DIRS")
  },

  {
    "list-repositories", 0, POPT_ARG_NONE, nullptr, OPT_LIST_REPOSITORIES,
    T_("Download the list of known package repository URLs from the MiKTeX project server, then print the list."),
    nullptr
  },

  {
    "local-package-repository", 0, POPT_ARG_STRING, nullptr, OPT_LOCAL_PACKAGE_REPOSITORY,
    T_("Download into (install from) the specified directory."),
    T_("DIR")
  },

  {
    "modify-path", 0, POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr, OPT_MODIFY_PATH,
    T_("Add MiKTeX to the PATH variable (--modify-path=yes). With --modify-path=no, the modification of PATH will be prevented."),
    "yes|no",
  },

  {
    "package-set", 0, POPT_ARG_STRING, nullptr, OPT_PACKAGE_SET,
    T_("Set the package set (one of: essential, basic, complete)."),
    T_("SET")
  },

  {
    "portable", 0, POPT_ARG_STRING, nullptr, OPT_PORTABLE,
    T_("Set up MiKTeX Portable."),
    T_("DIR")
  },

  {
    "print-info-only", 0, POPT_ARG_NONE, nullptr, OPT_PRINT_INFO_ONLY,
    T_("Print information about what would be done and exit."),
    nullptr
  },

  {
    "print-version-number", 0, POPT_ARG_NONE, nullptr, OPT_PRINT_VERSION_NUMBER,
    T_("Print the program version number and exit."),
    nullptr
  },

#if defined(MIKTEX_WINDOWS)
  {
    "program-folder", 0, POPT_ARG_STRING, nullptr, OPT_PROGRAM_FOLDER,
    T_("Add shortcuts to the specified program folder."),
    T_("FOLDER")
  },
#endif
  
  {
    "quiet", 0, POPT_ARG_NONE, nullptr, OPT_QUIET,
    T_("Suppress all output (except errors)."),
    nullptr
  },

  {
    "remote-package-repository", 0, POPT_ARG_STRING, nullptr, OPT_REMOTE_PACKAGE_REPOSITORY,
    T_("Download from the specified URL. Use --list-repositories to download an up-to-date list of package repositories."),
    "URL"
  },

  {
    "shared", 0, POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr, OPT_SHARED,
    T_("Install MiKTeX for all users (--shared=yes) or for the current user (--shared=no)."),
    "yes|no",
  },

  {
    "trace", 0, POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr, OPT_TRACE,
    T_("Turn on tracing.  TRACESTREAMS, if specified, is a comma-separated list of trace stream names (see the MiKTeX manual)."),
    T_("TRACESTREAMS")
  },

#if defined(MIKTEX_WINDOWS)
  {
    "use-registry", 0, POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr, OPT_USE_REGISTRY,
    T_("Write configuration settings into the Windows Registry (--use-registry=yes) or into configuration files (--use-registry=no)."),
    "yes|no",
  },
#endif
  
  {
    "user-config", 0, POPT_ARG_STRING, nullptr, OPT_USER_CONFIG,
    T_("Set the location of the user configuration directory."),
    T_("DIR")
  },

  {
    "user-data", 0, POPT_ARG_STRING, nullptr, OPT_USER_DATA,
    T_("Set the location of the user data directory."),
    T_("DIR")
  },

  {
    "user-install", 0, POPT_ARG_STRING, nullptr, OPT_USER_INSTALL,
    T_("Set the location of the user installation directory."),
    T_("DIR")
  },

  {
    "user-roots", 0, POPT_ARG_STRING, nullptr, OPT_USER_ROOTS,
    T_("Register additional directories for the current user. DIRS must be a semicolon-separated list of fully qualified path names."),
    T_("DIRS")
  },

  {
    "verbose", 0, POPT_ARG_NONE, nullptr, OPT_VERBOSE,
    T_("Turn on verbose output mode."),
    nullptr
  },

  {
    "version", 0, POPT_ARG_NONE, nullptr, OPT_VERSION,
    T_("Print version information and exit."),
    nullptr
  },

  POPT_AUTOHELP
  POPT_TABLEEND
};

volatile sig_atomic_t Application::interrupted = false;

void Application::Message(const char*  format, ...)
{
  if (quiet)
  {
    return;
  }
  va_list arglist;
  VA_START(arglist, format);
  cout << StringUtil::FormatStringVA(format, arglist) << endl;
  VA_END(arglist);
}

void Application::Verbose(const char* format, ...)
{
  if (!verbose)
  {
    return;
  }
  va_list arglist;
  VA_START(arglist, format);
  cout << StringUtil::FormatStringVA(format, arglist) << endl;
  VA_END(arglist);
}

MIKTEXNORETURN void Application::Error(const char* format, ...)
{
  va_list arglist;
  VA_START(arglist, format);
  cerr << "miktexsetup: " << StringUtil::FormatStringVA(format, arglist) << endl;
  VA_END(arglist);
  throw 1;
}

void Application::ReportLine(const string& str)
{
  Verbose("%s", str.c_str());
}

bool Application::OnRetryableError(const string& message)
{
  return false;
}

bool Application::OnProgress(MiKTeX::Setup::Notification nf)
{
  return !interrupted;
}

bool Application::OnProcessOutput(const void* output, size_t n)
{
  if (verbose)
  {
    cout.write(reinterpret_cast<const char*>(output), n);
  }
  return !interrupted;
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

void Application::ListRepositories()
{
  std::shared_ptr<MiKTeX::Packages::PackageManager> pPackageManager = PackageManager::Create();
  pPackageManager->DownloadRepositoryList();
  vector<RepositoryInfo> repositories = pPackageManager->GetRepositories();
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

void Application::PrintInfo()
{
  pSetupService->Initialize();
  SetupOptions options = pSetupService->GetOptions();
  string task;
  switch (options.Task)
  {
  case SetupTask::Download:
    task = "download";
    break;
  case SetupTask::InstallFromLocalRepository:
    task = "install from local package repository";
    break;
  case SetupTask::FinishSetup:
    task = "finish setup";
    break;
  case SetupTask::CleanUp:
    task = "factory reset";
    break;
  default:
    break;
  }
  string level;
  switch (options.PackageLevel)
  {
  case PackageLevel::Essential:
    level = "essential";
    break;
  case PackageLevel::Basic:
    level = "basic";
    break;
  case PackageLevel::Complete:
    level = "complete";
    break;
  default:
    break;
  }
  printf("setup task: %s\n", task.c_str());
  if (options.Task != SetupTask::CleanUp)
  {
    cout << "local package repository: " << Q_(options.LocalPackageRepository) << endl;
    cout << "package set: " << level << endl;
  }
  if (options.Task == SetupTask::Download)
  {
    cout << "remote package repository: " << Q_(options.RemotePackageRepository) << endl;
  }
  if (options.Task == SetupTask::InstallFromLocalRepository || options.Task == SetupTask::FinishSetup)
  {
    cout << "install for all users?: " << (options.IsCommonSetup ? "yes" : "no") << endl;
    cout << "portable? : " << (options.IsPortable ? "yes" : "no") << endl;
    if (options.IsPortable)
    {
      cout << "portable root: " << Q_(options.PortableRoot) << endl;
    }
#if defined(MIKTEX_WINDOWS)
    cout << "use registry?: " << (options.IsRegistryEnabled ? "yes" : "no") << endl;
#endif
    cout << "modify path?: " << (options.IsRegisterPathEnabled ? "yes" : "no") << endl;
#if defined(MIKTEX_WINDOWS)
    if (!options.FolderName.Empty())
    {
      cout << "program folder name: " << Q_(options.FolderName) << endl;
    }
#endif
  }
  if (options.Task == SetupTask::CleanUp)
  {
  }
  if (options.Task != SetupTask::Download)
  {
    if (!options.Config.commonInstallRoot.Empty())
    {
      cout << "common install root: " << Q_(options.Config.commonInstallRoot) << endl;
    }
    if (!options.Config.commonConfigRoot.Empty())
    {
      cout << "common config root: " << Q_(options.Config.commonConfigRoot) << endl;
    }
    if (!options.Config.commonDataRoot.Empty())
    {
      cout << "common data root: " << Q_(options.Config.commonDataRoot) << endl;
    }
    if (!options.Config.userInstallRoot.Empty())
    {
      cout << "user install root: " << Q_(options.Config.userInstallRoot) << endl;
    }
    if (!options.Config.userConfigRoot.Empty())
    {
      cout << "user config root: " << Q_(options.Config.userConfigRoot) << endl;
    }
    if (!options.Config.userDataRoot.Empty())
    {
      cout << "user data root: " << Q_(options.Config.userDataRoot) << endl;
    }
  }
}

void Application::Main(int argc, const char** argv)
{
  Session::InitInfo initInfo;
  initInfo.SetProgramInvocationName(argv[0]);

  bool optShared = false;
  bool optModifyPath = true;
#if defined(MIKTEX_WINDOWS)
  string optProgramFolder;
  bool optUseRegistry = true;
#endif
  bool optListRepositories = false;
  bool optPortable = false;
  bool optPrintInfoOnly = false;
  bool optPrintVersionNumber = false;
  bool optVersion = false;
  PackageLevel optPackageLevel = PackageLevel::None;
  string optLocalPackageRepository;
  string optRemotePackageRepository;
  string optCommonConfigRoot;
  string optCommonDataRoot;
  string optCommonInstallRoot;
  string optCommonRoots;
  string optUserConfigRoot;
  string optUserDataRoot;
  string optUserInstallRoot;
  string optUserRoots;
  string optPortableRoot;

  PoptWrapper popt(argc, argv, aoption);
  popt.SetOtherOptionHelp("download|install|finish|factoryreset");

  int option;

  while ((option = popt.GetNextOpt()) >= 0)
  {
    string optArg = popt.GetOptArg();
    switch (option)
    {
    case OPT_COMMON_CONFIG:
      optCommonConfigRoot = optArg;
      break;
    case OPT_COMMON_DATA:
      optCommonDataRoot = optArg;
      break;
    case OPT_COMMON_INSTALL:
      optCommonInstallRoot = optArg;
      break;
    case OPT_COMMON_ROOTS:
      optCommonRoots = optArg;
      break;
    case OPT_LIST_REPOSITORIES:
      optListRepositories = true;
      break;
    case OPT_LOCAL_PACKAGE_REPOSITORY:
      optLocalPackageRepository = optArg;
      break;
    case OPT_MODIFY_PATH:
      optModifyPath = (optArg.empty() || Utils::EqualsIgnoreCase("yes", optArg));
      break;
    case OPT_REMOTE_PACKAGE_REPOSITORY:
      optRemotePackageRepository = optArg;
      break;
    case OPT_PACKAGE_SET:
      if (optArg == "essential")
      {
        optPackageLevel = PackageLevel::Essential;
      }
      else if (optArg == "basic")
      {
        optPackageLevel = PackageLevel::Basic;
      }
      else if (optArg == "advanced")
      {
        optPackageLevel = PackageLevel::Advanced;
      }
      else if (optArg == "complete")
      {
        optPackageLevel = PackageLevel::Complete;
      }
      else
      {
        Error(T_("Invalid package level."));
      }
      break;
    case OPT_PORTABLE:
      optPortable = true;
      optPortableRoot = optArg;
      break;
    case OPT_PRINT_INFO_ONLY:
      optPrintInfoOnly = true;
      break;
    case OPT_PRINT_VERSION_NUMBER:
      optPrintVersionNumber = true;
      break;
#if defined(MIKTEX_WINDOWS)
    case OPT_PROGRAM_FOLDER:
      optProgramFolder = optArg;
      break;
#endif
    case OPT_QUIET:
      if (verbose)
      {
        Error(T_("Cannot be --verbose and --quiet at the same time."));
      }
      quiet = true;
      break;
    case OPT_SHARED:
      optShared = (optArg.empty() || Utils::EqualsIgnoreCase("yes", optArg));
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
#if defined(MIKTEX_WINDOWS)
    case OPT_USE_REGISTRY:
      optUseRegistry = (optArg.empty() || Utils::EqualsIgnoreCase("yes", optArg));
      break;
#endif
    case OPT_USER_CONFIG:
      optUserConfigRoot = optArg;
      break;
    case OPT_USER_DATA:
      optUserDataRoot = optArg;
      break;
    case OPT_USER_INSTALL:
      optUserInstallRoot = optArg;
      break;
    case OPT_USER_ROOTS:
      optUserRoots = optArg;
      break;
    case OPT_VERBOSE:
      if (quiet)
      {
        Error(T_("Cannot be --verbose and --quiet at the same time."));
      }
      verbose = true;
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

  if (optVersion)
  {
    cout
      << Utils::MakeProgramVersionString(THE_NAME_OF_THE_GAME, VersionNumber(MIKTEX_MAJOR_VERSION, MIKTEX_MINOR_VERSION, MIKTEX_COMP_J2000_VERSION, 0)) << endl
      << "Copyright (C) 2014-2018 Christian Schenk" << endl
      << "This is free software; see the source for copying conditions.  There is NO" << endl
      << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
    return;
  }

  if (optPrintVersionNumber)
  {
    cout << VersionNumber(MIKTEX_MAJOR_VERSION, MIKTEX_MINOR_VERSION, MIKTEX_COMP_J2000_VERSION, 0) << endl;
    return;
  }

  initInfo.AddOption(Session::InitOption::NoFixPath);

  pSession = Session::Create(initInfo);

  if (optShared)
  {
    pSession->SetAdminMode(true, true);
  }

  if (optListRepositories)
  {
    ListRepositories();
    return;
  }

  vector<string> leftovers = popt.GetLeftovers();

  if (leftovers.empty())
  {
    Error(T_("Nothing to do?\nTry '%s --help' for more information."), argv[0]);
  }

  if (leftovers.size() > 1)
  {
    Error(T_("Too many arguments."));
  }

  pSetupService = SetupService::Create();
  SetupOptions setupOptions = pSetupService->GetOptions();

  if (leftovers[0] == "download")
  {
    setupOptions.Task = SetupTask::Download;
  }
  else if (leftovers[0] == "install")
  {
    setupOptions.Task = SetupTask::InstallFromLocalRepository;
  }
  else if (leftovers[0] == "finish")
  {
    setupOptions.Task = SetupTask::FinishSetup;
  }
  else if (leftovers[0] == "factoryreset")
  {
    setupOptions.Task = SetupTask::CleanUp;
    setupOptions.CleanupOptions = { CleanupOption::Links, CleanupOption::Path, CleanupOption::Registry, CleanupOption::RootDirectories };
  }
  else
  {
    Error(T_("Unknown/unsupported setup task: %s"), leftovers[0].c_str());
  }

  if (optShared)
  {
    setupOptions.IsCommonSetup = true;
  }

  if (optPortable)
  {
    setupOptions.IsPortable = true;
    setupOptions.PortableRoot = optPortableRoot;
  }    

#if defined(MIKTEX_WINDOWS)
  if (optUseRegistry)
  {
    setupOptions.IsRegistryEnabled = true;
  }
#endif

#if defined(MIKTEX_WINDOWS)
  if (!optProgramFolder.empty())
  {
    setupOptions.FolderName = optProgramFolder;
  }
#endif

  if (optModifyPath)
  {
    setupOptions.IsRegisterPathEnabled = true;
  }

  if (!optLocalPackageRepository.empty())
  {
    setupOptions.LocalPackageRepository = optLocalPackageRepository;
  }

  if (!optRemotePackageRepository.empty())
  {
    setupOptions.RemotePackageRepository = optRemotePackageRepository;
  }

  if (!optCommonConfigRoot.empty())
  {
    setupOptions.Config.commonConfigRoot = optCommonConfigRoot;
  }

  if (!optCommonDataRoot.empty())
  {
    setupOptions.Config.commonDataRoot = optCommonDataRoot;
  }

  if (!optCommonInstallRoot.empty())
  {
    setupOptions.Config.commonInstallRoot = optCommonInstallRoot;
  }

  if (!optCommonRoots.empty())
  {
    setupOptions.Config.commonRoots = optCommonRoots;
  }

  if (!optUserConfigRoot.empty())
  {
    setupOptions.Config.userConfigRoot = optUserConfigRoot;
  }

  if (!optUserDataRoot.empty())
  {
    setupOptions.Config.userDataRoot = optUserDataRoot;
  }

  if (!optUserInstallRoot.empty())
  {
    setupOptions.Config.userInstallRoot = optUserInstallRoot;
  }

  if (!optUserRoots.empty())
  {
    setupOptions.Config.userRoots = optUserRoots;
  }

  if (optPackageLevel != PackageLevel::None)
  {
    setupOptions.PackageLevel = optPackageLevel;
  }

  pSetupService->SetOptions(setupOptions);

  pSetupService->SetCallback(this);

  if (optPrintInfoOnly)
  {
    PrintInfo();
  }
  else
  {
    pSetupService->Run();
  }

  pSetupService = nullptr;
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

int MAIN(int argc, MAINCHAR** argv)
{
  int retCode = 0;
  try
  {
    vector<string> utf8args;
    utf8args.reserve(argc);
    vector<const char*> newargv;
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
  catch (const MiKTeXException& e)
  {
    // TODO: Sorry
    Utils::PrintException(e);
    retCode = 1;
  }
  catch (const exception& e)
  {
    // TODO: Sorry
    Utils::PrintException(e);
    retCode = 1;
  }
  catch (int rc)
  {
    retCode = rc;
  }
  return retCode;
}
