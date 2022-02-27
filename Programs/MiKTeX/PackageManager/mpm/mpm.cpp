/**
 * @file mpm.cpp
 * @author Christian Schenk
 * @brief MiKTeX Package Manager (cli version)
 *
 * @copyright Copyright © 2003-2022 Christian Schenk
 *
 * This file is part of MpmCon.
 *
 * MpmCon is licensed under GNU General Public License version 2 or any later
 * version.
 */

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#if defined(_WIN32)
#include <Windows.h>
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
#if defined(LOG4CXX_INFO_FMT)
#define MIKTEX_LOG4CXX_12 1
#endif

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
#include <miktex/PackageManager/PackageManager>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceCallback>
#include <miktex/Util/StringUtil>
#include <miktex/Util/Tokenizer>
#include <miktex/Wrappers/PoptWrapper>

#if defined(MIKTEX_WINDOWS)
#include <MiKTeX/Core/Help>
#include <miktex/Core/win/ConsoleCodePageSwitcher>
#endif

#include "internal.h"

#if !defined(THE_NAME_OF_THE_GAME)
#define THE_NAME_OF_THE_GAME T_("MiKTeX Package Manager")
#endif

#if defined(MIKTEX_WINDOWS)
const char PATH_DELIMITER = ';';
#define PATH_DELIMITER_STRING ";"
#else
const char PATH_DELIMITER = ':';
#define PATH_DELIMITER_STRING ":"
#endif

class Application :
    public MiKTeX::Trace::TraceCallback
{
public:

    Application()
    {
        InstallSignalHandler(SIGINT);
        InstallSignalHandler(SIGTERM);
    }

    bool MIKTEXTHISCALL Trace(const TraceCallback::TraceMessage& traceMessage) override;
    void Main(int argc, const char** argv);

    static void Sorry(const std::string& description, const std::string& remedy, const std::string& url);

    static void Sorry()
    {
        Sorry("", "", "");
    }

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

    void DeprecateOption(const std::string& opt);
    void Verbose(const std::string& s);
    void Warn(const std::string& s);
    void SecurityRisk(const std::string& s);
    void Message(const std::string& s);
    MIKTEXNORETURN void Error(const std::string& s);
    void UpdateDb();
    void Require(const std::vector<std::string>& required, const std::vector<std::string>& required2);
    void Install(const std::vector<std::string>& toBeInstalled, const std::vector<std::string>& toBeInstalled2, const std::vector<std::string>& toBeRemoved, const std::vector<std::string>& toBeRemoved2);
    void Verify(const std::vector<std::string>& toBeVerified, const std::vector<std::string>& toBeVerified2);
    void FindConflicts();
    void ImportPackage(const std::string& packageId, std::vector<std::string>& toBeinstalled);
    void ImportPackages(std::vector<std::string>& toBeinstalled);
    void FindUpdates();
    void Update(const std::vector<std::string>& requestedUpdates, const std::vector<std::string>& requestedUpdates2);
    void FindUpgrades(const std::string& packageLevel);
    void Upgrade(const std::string& packageLevel);
    void List(const std::string& outputTemplate);
    void ListRepositories();
    void PickRepositoryUrl();
    void PrintFiles(const std::vector<std::string>& files);
    void PrintPackageInfo(const std::string& packageId);
    void RestartWindowed();
    void RunOneMiKTeXUtility(const std::vector<std::string>& arguments);

    void FlushPendingTraceMessages()
    {
        for (const TraceCallback::TraceMessage& msg : pendingTraceMessages)
        {
            TraceInternal(msg);
        }
        pendingTraceMessages.clear();
    }

    void TraceInternal(const TraceCallback::TraceMessage& traceMessage)
    {
        log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(std::string("trace.mpmcli.") + traceMessage.facility);
        switch (traceMessage.level)
        {
        case MiKTeX::Trace::TraceLevel::Fatal:
            LOG4CXX_FATAL(logger, traceMessage.message);
            break;
        case MiKTeX::Trace::TraceLevel::Error:
            LOG4CXX_ERROR(logger, traceMessage.message);
            break;
        case MiKTeX::Trace::TraceLevel::Warning:
            LOG4CXX_WARN(logger, traceMessage.message);
            break;
        case MiKTeX::Trace::TraceLevel::Info:
            LOG4CXX_INFO(logger, traceMessage.message);
            break;
        case MiKTeX::Trace::TraceLevel::Trace:
            LOG4CXX_TRACE(logger, traceMessage.message);
            break;
        case MiKTeX::Trace::TraceLevel::Debug:
        default:
            LOG4CXX_DEBUG(logger, traceMessage.message);
            break;
        }
    }

#if defined(MIKTEX_WINDOWS)
    void RegisterComponents(bool doRegister);
#endif

    static void SignalHandler(int sig);

    std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;
    std::shared_ptr<MiKTeX::Core::Session> session;
    bool verbose = false;
    bool quiet = false;
    std::string repository;
    std::vector<MiKTeX::Trace::TraceCallback::TraceMessage> pendingTraceMessages;

    static const struct poptOption aoption[];
    static volatile sig_atomic_t interrupted;
};

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;

vector<string> DEFAULT_TRACE_OPTIONS =
{
    TraceStream::MakeOption("", "", TraceLevel::Info),
    TraceStream::MakeOption(MIKTEX_TRACE_MPM, "", TraceLevel::Trace),
};

static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("mpmcli"));
static bool isLog4cxxConfigured = false;

enum Option
{
    OPT_AAA = 1,
    OPT_ADMIN,
    OPT_FIND_CONFLICTS,
    OPT_FIND_UPDATES,
    OPT_FIND_UPGRADES,
    OPT_HHELP,
    OPT_IMPORT,
    OPT_IMPORT_ALL,
    OPT_INSTALL,
    OPT_LIST,
    OPT_LIST_PACKAGE_NAMES,
    OPT_LIST_REPOSITORIES,
    OPT_PACKAGE_LEVEL,
    OPT_PICK_REPOSITORY_URL,
    OPT_PRINT_PACKAGE_INFO,
    OPT_PROXY,
    OPT_PROXY_PASSWORD,
    OPT_PROXY_USER,
    OPT_QUIET,
    OPT_REGISTER_COMPONENTS,
    OPT_REPOSITORY,
    OPT_REPOSITORY_RELEASE_STATE,
    OPT_REQUIRE,
    OPT_SET_REPOSITORY,
    OPT_TRACE,
    OPT_UNINSTALL,
    OPT_UNREGISTER_COMPONENTS,
    OPT_UPDATE,
    OPT_UPDATE_ALL,
    OPT_UPDATE_DB,
    OPT_UPGRADE,
    OPT_VERBOSE,
    OPT_VERIFY,
    OPT_VERSION,
};

const struct poptOption Application::aoption[] = {

    {
        "admin", 0, POPT_ARG_NONE, 0, OPT_ADMIN,
        T_("Run in administrator mode."),
        nullptr,
    },

    {
        "find-conflicts", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_FIND_CONFLICTS,
        nullptr,
        nullptr,
    },

    {
        "find-updates", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_FIND_UPDATES,
        nullptr,
        nullptr,
    },

    {
        "find-upgrades", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_FIND_UPGRADES,
        nullptr,
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
        "install", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_INSTALL,
        nullptr,
        nullptr
    },

    {
        "list", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_LIST,
        nullptr,
        nullptr
    },

    {
        "list-package-names", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_LIST_PACKAGE_NAMES,
        nullptr,
        nullptr
    },

    {
        "list-repositories", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_LIST_REPOSITORIES,
        nullptr,
        nullptr
    },

    {
        "package-level", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_PACKAGE_LEVEL,
        nullptr,
        nullptr
    },

    {
        "pick-repository-url", 0, POPT_ARG_NONE, nullptr, OPT_PICK_REPOSITORY_URL,
        T_("Pick a suitable package repository URL and print it."),
        nullptr
    },

    {
        "print-package-info", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_PRINT_PACKAGE_INFO,
        nullptr,
        nullptr
    },

    {
        "proxy", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_PROXY,
        nullptr,
        nullptr
    },

    {
        "proxy-password", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_PROXY_PASSWORD,
        nullptr,
        nullptr
    },

    {
        "proxy-user", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_PROXY_USER,
        nullptr,
        nullptr
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
        "repository", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_REPOSITORY,
        nullptr,
        nullptr
    },

    {
        "repository-release-state", 0, POPT_ARG_STRING, nullptr, OPT_REPOSITORY_RELEASE_STATE,
        T_("Select the repository release state (one of: stable, next)."),
        T_("STATE")
    },

    {
        "require", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_REQUIRE,
        nullptr,
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
        "uninstall", 0, POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_UNINSTALL,
        nullptr,
        nullptr
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
        "update", 0, POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_UPDATE,
        nullptr,
        nullptr
    },

    {
        "update-all", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_UPDATE_ALL,
        nullptr,
        nullptr,
    },

    {
        "update-db", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_UPDATE_DB,
        nullptr,
        nullptr
    },

    {
        "upgrade", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_UPGRADE,
        nullptr,
        nullptr
    },

    {
        "verbose", 0, POPT_ARG_NONE, nullptr, OPT_VERBOSE,
        T_("Turn on verbose output mode."),
        nullptr
    },

    {
        "verify", 0, POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_VERIFY,
        nullptr,
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

void Application::DeprecateOption(const string& opt)
{
    Warn(fmt::format(T_("{0}: this option has been deprecated; it will be removed from a future version of MiKTeX"), opt));
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
#if defined(MIKTEX_LOG4CXX_12)
        log4cxx::AppenderPtr appender = log4cxx::Logger::getRootLogger()->getAppender(LOG4CXX_STR("RollingLogFile"));
        log4cxx::FileAppenderPtr fileAppender = log4cxx::cast<log4cxx::FileAppender>(appender);
#else
        log4cxx::FileAppenderPtr fileAppender = log4cxx::Logger::getRootLogger()->getAppender(LOG4CXX_STR("RollingLogFile"));
#endif
        if (fileAppender != nullptr)
        {
            cerr
                << "\n"
                << "The log file hopefully contains the information to get MiKTeX going again:" << "\n"
                << "\n"
                << "  " << PathName(fileAppender->getFile()) << endl;
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

void Application::UpdateDb()
{
    vector<string> args{ "packages", "update-package-database" };
    if (!repository.empty())
    {
        args.push_back("--repository");
        args.push_back(repository);
    }
    RunOneMiKTeXUtility(args);
}

void Application::Require(const std::vector<std::string>& required, const std::vector<std::string>& required2)
{
        vector<string> args{ "packages", "require" };
        if (!repository.empty())
        {
            args.push_back("--repository");
            args.push_back(repository);
        }
        for (auto p : required)
        {
            args.push_back(p);
        }
        for (auto p : required2)
        {
            args.push_back("--package-id-file");
            args.push_back(p);
        }
        RunOneMiKTeXUtility(args);
}

void Application::Install(const vector<string>& toBeInstalled, const vector<string>& toBeInstalled2, const vector<string>& toBeRemoved, const vector<string>& toBeRemoved2)
{
    if (!(toBeInstalled.empty() && toBeInstalled2.empty()))
    {
        vector<string> args{ "packages", "install" };
        if (!repository.empty())
        {
            args.push_back("--repository");
            args.push_back(repository);
        }
        for (auto p : toBeInstalled)
        {
            args.push_back(p);
        }
        for (auto p : toBeInstalled2)
        {
            args.push_back("--package-id-file");
            args.push_back(p);
        }
        RunOneMiKTeXUtility(args);
    }

    if (!(toBeRemoved.empty() && toBeRemoved2.empty()))
    {
        vector<string> args{ "packages", "remove" };
        for (auto p : toBeRemoved)
        {
            args.push_back(p);
        }
        for (auto p : toBeRemoved2)
        {
            args.push_back("--package-id-file");
            args.push_back(p);
        }
        RunOneMiKTeXUtility(args);
    }
}

#if defined(MIKTEX_WINDOWS)

void Application::RegisterComponents(bool doRegister)
{
    shared_ptr<PackageInstaller> installer(packageManager->CreateInstaller({ nullptr, true, true }));
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

void Application::Verify(const vector<string>& toBeVerifiedArg, const vector<string>& toBeVerifiedArg2)
{
    vector<string> args {"packages", "verify"};
    for (auto p : toBeVerifiedArg)
    {
        args.push_back(p);
    }
    for (auto p : toBeVerifiedArg2)
    {
        args.push_back("--package-id-file");
        args.push_back(p);
    }
    RunOneMiKTeXUtility(args);
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
    vector<string> args{ "packages", "check-update" };
    if (!repository.empty())
    {
        args.push_back("--repository");
        args.push_back(repository);
    }
    RunOneMiKTeXUtility(args);
}

void Application::Update(const vector<string>& requestedUpdates, const vector<string>& requestedUpdates2)
{
    vector<string> args {"packages", "update"};
    for (auto u : requestedUpdates)
    {
        args.push_back(u);
    }
    for (auto u : requestedUpdates2)
    {
        args.push_back("--package-id-file");
        args.push_back(u);
    }
    RunOneMiKTeXUtility(args);
}

void Application::FindUpgrades(const string& packageLevel)
{
    vector<string> args{ "packages", "check-upgrade", packageLevel };
    if (!repository.empty())
    {
        args.push_back("--repository");
        args.push_back(repository);
    }
    RunOneMiKTeXUtility(args);
}

void Application::Upgrade(const string& packageLevel)
{
    vector<string> args{ "packages", "upgrade", packageLevel };
    if (!repository.empty())
    {
        args.push_back("--repository");
        args.push_back(repository);
    }
    RunOneMiKTeXUtility(args);
}

void Application::List(const string& outputTemplate)
{
    vector<string> args = {"packages", "list", "--template", outputTemplate};
    RunOneMiKTeXUtility(args);
}

class DataTransferRateComparer
{
public:
    inline bool operator() (const RepositoryInfo& lhs, const RepositoryInfo& rhs)
    {
        return lhs.dataTransferRate > rhs.dataTransferRate;
    }
};

void Application::ListRepositories()
{
    RunOneMiKTeXUtility({"repositories", "list", "--template", "{url}"});
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
    RunOneMiKTeXUtility({ "packages", "info", "--template", "name: {id}\ntitle: {title}\nrun-time files: {runFiles}", packageId });
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

void ParseList(const string& s, vector<string>& packageIDs, vector<string>& packageIDfiles)
{
    if (s.length() > 0 && s[0] == '@')
    {
        packageIDfiles.push_back(&s[1]);
    }
    else
    {
        packageIDs.push_back(s);
    }
}

void Application::RunOneMiKTeXUtility(const vector<string>& arguments)
{
    PathName oneMiKTeXUtility;
    if (!session->FindFile("miktex", FileType::EXE, oneMiKTeXUtility))
    {
        Error(T_("The miktex executable could not be found."));
    }
    vector<string> allArguments{ "miktex" };
    if (verbose)
    {
        allArguments.push_back("--verbose");
    }
    if (session->IsAdminMode())
    {
        allArguments.push_back("--admin");
    }
    allArguments.insert(allArguments.end(), arguments.begin(), arguments.end());
    LOG4CXX_INFO(logger, "running: " << CommandLineBuilder(allArguments));
    int exitCode;
    MiKTeXException miktexException;
    if (!Process::Run(oneMiKTeXUtility, allArguments, nullptr, &exitCode, &miktexException, nullptr) || exitCode != 0)
    {
        // TODO
    }
}

void Application::Main(int argc, const char** argv)
{
    bool optAdmin = false;
    bool optFindConflicts = false;
    bool optFindUpdates = false;
    bool optFindUpgrades = false;
    bool optImport = false;
    bool optImportAll = false;
    bool optList = false;
    bool optListPackageNames = false;
    bool optListRepositories = false;
    string optPackageLevel;
    bool optPickRepositoryUrl = false;
    bool optPrintPackageInfo = false;
    bool optSetRepository = false;
    bool optSort = false;
    bool optUpdate = false;
    bool optUpdateAll = false;
    bool optUpdateDb = false;
    bool optUpgrade = false;
    bool optVerify = false;
    bool optVersion = false;
    int optMaxCount = INT_MAX;
    int optProxyPort = -1;
#if defined(MIKTEX_WINDOWS)
    bool optRegisterComponents = false;
    bool optUnregisterComponents = false;
#endif
    string packageId;
    string optProxy;
    string optProxyPassword;
    string optProxyUser;
    string toBeImported;
    vector<string> toBeInstalled;
    vector<string> toBeInstalled2;
    vector<string> toBeRemoved;
    vector<string> toBeRemoved2;
    vector<string> toBeVerified;
    vector<string> toBeVerified2;
    vector<string> requestedUpdates;
    vector<string> requestedUpdates2;
    vector<string> required;
    vector<string> required2;
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
        case OPT_FIND_CONFLICTS:
            optFindConflicts = true;
            break;
        case OPT_FIND_UPDATES:
            DeprecateOption("--find-updates");
            optFindUpdates = true;
            break;
        case OPT_FIND_UPGRADES:
            DeprecateOption("--find-upgrades");
            optFindUpgrades = true;
            break;
#if defined (MIKTEX_WINDOWS)
        case OPT_HHELP:
        {
            session->ShowManualPageAndWait(0, MIKTEXHELP_MPMCON);
            session->Close();
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
            DeprecateOption("--install");
            ParseList(optArg, toBeInstalled, toBeInstalled2);
            break;
        case OPT_LIST:
            DeprecateOption("--list");
            optList = true;
            break;
        case OPT_LIST_PACKAGE_NAMES:
            DeprecateOption("--list-package-names");
            optListPackageNames = true;
            break;
        case OPT_LIST_REPOSITORIES:
            DeprecateOption("--list-repositories");
            optListRepositories = true;
            break;
        case OPT_PACKAGE_LEVEL:
            optPackageLevel = optArg;
            break;
        case OPT_PICK_REPOSITORY_URL:
            optPickRepositoryUrl = true;
            break;
        case OPT_PRINT_PACKAGE_INFO:
            DeprecateOption("--print-package-info");
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
            DeprecateOption("--require");
            ParseList(optArg, required, required2);
            break;
        case OPT_SET_REPOSITORY:
            optSetRepository = true;
            repository = optArg;
            break;
        case OPT_TRACE:
            // see above
            break;
        case OPT_UNINSTALL:
            DeprecateOption("--uninstall");
            ParseList(optArg, toBeRemoved, toBeRemoved2);
            break;
#if defined (MIKTEX_WINDOWS)
        case OPT_UNREGISTER_COMPONENTS:
            optUnregisterComponents = true;
            break;
#endif
        case OPT_UPDATE:
            DeprecateOption("--update");
            if (!optArg.empty())
            {
                if (optUpdateAll)
                {
                    Error(T_("Already updating all packages."));
                }
                optUpdate = true;
                ParseList(optArg, requestedUpdates, requestedUpdates2);
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
            DeprecateOption("--update-all");
            if (optUpdate)
            {
                Error(T_("Already updating selected packages."));
            }
            optUpdateAll = true;
            break;
        case OPT_UPDATE_DB:
            DeprecateOption("--update-db");
            optUpdateDb = true;
            break;
        case OPT_UPGRADE:
            DeprecateOption("--upgrade");
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
            DeprecateOption("--verify");
            if (!optArg.empty())
            {
                ParseList(optArg, toBeVerified, toBeVerified2);
            }
            optVerify = true;
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

    if (!required.empty() || !required2.empty())
    {
        Require(required, required2);
        restartWindowed = false;
    }

    if (!toBeInstalled.empty() || !toBeInstalled2.empty() || !toBeRemoved.empty() || !toBeRemoved2.empty())
    {
        Install(toBeInstalled, toBeInstalled2, toBeRemoved, toBeRemoved2);
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

    if (optUpdateAll || !requestedUpdates.empty() || !requestedUpdates2.empty())
    {
        Update(requestedUpdates, requestedUpdates2);
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

    if (optVerify)
    {
        Verify(toBeVerified, toBeVerified2);
        restartWindowed = false;
    }

    if (optList)
    {
        List("{isInstalled} {numFiles} {size} {id}");
        restartWindowed = false;
    }
    if (optListPackageNames)
    {
        List("{id}");
        restartWindowed = false;
    }
    if (optListRepositories)
    {
        ListRepositories();
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
    session->Close();
    session = nullptr;
}

bool Application::Trace(const TraceCallback::TraceMessage& traceMessage)
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
#   define MAIN wmain
#   define MAINCHAR wchar_t
#else
#   define MAIN main
#   define MAINCHAR char
#endif

int MAIN(int argc, MAINCHAR* argv[])
{
#if defined(MIKTEX_WINDOWS)
    ConsoleCodePageSwitcher cpSwitcher;
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
    if (logger != nullptr)
    {
        LOG4CXX_INFO(logger, "this process (" << Process::GetCurrentProcess()->GetSystemId() << ") finishes with exit code " << retCode);
        logger = nullptr;
    }
    return retCode;
}
