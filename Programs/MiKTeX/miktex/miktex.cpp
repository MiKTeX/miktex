/**
 * @file miktex.cpp
 * @author Christian Schenk
 * @brief Main program
 *
 * @copyright Copyright © 2021 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#if defined(HAVE_CONFIG_H)
#  include <config.h>
#endif

#include <atomic>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include <csignal>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <log4cxx/basicconfigurator.h>
#include <log4cxx/logger.h>
#include <log4cxx/rollingfileappender.h>
#include <log4cxx/xml/domconfigurator.h>
#if defined(LOG4CXX_INFO_FMT)
#  define MIKTEX_LOG4CXX_12 1
#endif

#include "miktex-version.h"

#include <miktex/Configuration/ConfigNames>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Exceptions>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Session>
#include <miktex/Core/Utils>
#include <miktex/PackageManager/PackageManager>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>
#include <miktex/Util/StringUtil>

#if defined(MIKTEX_WINDOWS)
#include <miktex/Core/win/ConsoleCodePageSwitcher>
#endif

#include "internal.h"

#include "shims/mkfntmap.h"
#include "shims/updmap.h"

#include "topics/Topic.h"
#include "topics/filesystem/topic.h"
#include "topics/fndb/topic.h"
#include "topics/fontmaps/topic.h"
#include "topics/formats/topic.h"
#include "topics/languages/topic.h"
#include "topics/links/topic.h"

#if defined(MIKTEX_WINDOWS)
#include "topics/filetypes/topic.h"
#endif

const char* const TheNameOfTheGame = T_("One MiKTeX Utility");

static std::atomic<bool> canceled;

static bool isLog4cxxConfigured = false;
static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("miktex"));

class MiKTeXApp :
    public MiKTeX::Core::IFindFileCallback,
    public MiKTeX::Packages::PackageInstallerCallback,
    public MiKTeX::Trace::TraceCallback,
    public OneMiKTeXUtility::Installer,
    public OneMiKTeXUtility::Logger,
    public OneMiKTeXUtility::ProcessRunner,
    public OneMiKTeXUtility::Program,
    public OneMiKTeXUtility::UI
{
public:

    std::tuple<int, std::vector<std::string>> Init(const std::vector<std::string>& args);

    void Finalize();

    int Run(const std::vector<std::string>& args);

private:

    void RegisterTopics()
    {
        RegisterTopic(OneMiKTeXUtility::Topics::FileSystem::Create());
        RegisterTopic(OneMiKTeXUtility::Topics::FNDB::Create());
        RegisterTopic(OneMiKTeXUtility::Topics::FontMaps::Create());
        RegisterTopic(OneMiKTeXUtility::Topics::Formats::Create());
        RegisterTopic(OneMiKTeXUtility::Topics::Languages::Create());
        RegisterTopic(OneMiKTeXUtility::Topics::Links::Create());
#if defined(MIKTEX_WINDOWS)
        RegisterTopic(OneMiKTeXUtility::Topics::FileTypes::Create());
#endif
    }

    void RegisterTopic(std::unique_ptr<OneMiKTeXUtility::Topics::Topic> t)
    {
        auto name = t->Name();
        this->topics[name] = std::move(t);
    }

    std::string InvocationName() override
    {
        return this->args[0];
    }

    bool Canceled() override
    {
        return canceled;
    }

    void EnableInstaller(bool b) override
    {
        this->enableInstaller2 = b;
    }

    bool IsInstallerEnabled() override
    {
        return this->enableInstaller == MiKTeX::Configuration::TriState::True && this->enableInstaller2;
    }

    void EnsureInstaller()
    {
        if (this->packageInstaller == nullptr)
        {
            this->packageInstaller = this->packageManager->CreateInstaller({ this, true, false });
        }
    }

    bool InstallPackage(const std::string& packageId, const MiKTeX::Util::PathName& trigger, MiKTeX::Util::PathName& installRoot) override;

    void ReportLine(const std::string& str) override
    {
        Verbose(str);
    }

    bool OnRetryableError(const std::string& message) override
    {
        return false;
    }

    bool OnProgress(MiKTeX::Packages::Notification nf) override
    {
        return true;
    }

    bool TryCreateFile(const MiKTeX::Util::PathName& fileName, MiKTeX::Core::FileType fileType) override
    {
        return false;
    }

    void Verbose(int level, const std::string& s) override;

    void Verbose(const std::string& s)
    {
        Verbose(1, s);
    }

    int VerbosityLevel() override
    {
        return this->verbosityLevel;
    }

    bool BeingQuiet() override
    {
        return this->quiet;
    }

    void LogFatal(const std::string& message) override
    {
        LOG4CXX_FATAL(logger, message);
    }

    void LogInfo(const std::string& message) override
    {
        LOG4CXX_INFO(logger, message);
    }

    void LogTrace(const std::string& message) override
    {
        LOG4CXX_TRACE(logger, message);
    }

    void LogWarn(const std::string& message) override
    {
        LOG4CXX_WARN(logger, message);
    }

    MIKTEXNORETURN void FatalError(const std::string& message) override;

    MIKTEXNORETURN void IncorrectUsage(const std::string& message) override;

    void ShowUsage();

    void ShowVersion();

    void Output(const std::string& s) override;

    void Warning(const std::string& s) override;

    void SecurityRisk(const std::string& s);

    void PushTraceMessage(const MiKTeX::Trace::TraceCallback::TraceMessage& traceMessage);

    void PushTraceMessage(const std::string& message);

    bool Trace(const MiKTeX::Trace::TraceCallback::TraceMessage& traceMessage) override;

    void FlushPendingTraceMessages();

    void LogTraceMessage(const MiKTeX::Trace::TraceCallback::TraceMessage& traceMessage);

    void RunProcess(const MiKTeX::Util::PathName& fileName, const std::vector<std::string>& arguments) override;

    std::vector<std::string> args;
    OneMiKTeXUtility::ApplicationContext ctx;
    MiKTeX::Configuration::TriState enableInstaller = MiKTeX::Configuration::TriState::Undetermined;
    bool enableInstaller2 = true;
    std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;
    std::shared_ptr<MiKTeX::Packages::PackageInstaller> packageInstaller;
    std::vector<MiKTeX::Trace::TraceCallback::TraceMessage> pendingTraceMessages;
    bool quiet = false;
    std::shared_ptr<MiKTeX::Core::Session> session;
    std::map<std::string, std::unique_ptr<OneMiKTeXUtility::Topics::Topic>> topics;
    int verbosityLevel = 0;
};

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

using namespace OneMiKTeXUtility;

static void MIKTEXCEECALL SignalHandler(int signalToBeHandled)
{
    switch (signalToBeHandled)
    {
    case SIGINT:
    case SIGTERM:
        signal(SIGINT, SIG_IGN);
        canceled = true;
        break;
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

static void Sorry(const string& message, const string& description, const string& remedy, const string& url)
{
    if (cerr.fail())
    {
        return;
    }
    cerr << endl;
    cerr << fmt::format(T_("Sorry, but: {0}"), message) << endl;
    if (!description.empty())
    {
        cerr
            << "\n"
            << description << "\n"
            << endl;
    }
    if (!remedy.empty())
    {
        cerr << fmt::format(T_("Remedy: {0}"), remedy) << endl;
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
            cerr << fmt::format(T_("Log file: {0}"), PathName(fileAppender->getFile())) << endl;
        }
    }
    if (!url.empty())
    {
        cerr << fmt::format(T_("For more information, visit: {0}"), url) << endl;
    }
}


void MiKTeXApp::PushTraceMessage(const TraceCallback::TraceMessage& traceMessage)
{
    if (pendingTraceMessages.size() > 100)
    {
        pendingTraceMessages.clear();
    }
    pendingTraceMessages.push_back(traceMessage);
}

void MiKTeXApp::PushTraceMessage(const string& message)
{
    PushTraceMessage(TraceCallback::TraceMessage("miktex", "miktex", TraceLevel::Trace, message));
}

bool MiKTeXApp::Trace(const TraceCallback::TraceMessage& traceMessage)
{
    if (!isLog4cxxConfigured)
    {
        PushTraceMessage(traceMessage);
        return true;
    }
    FlushPendingTraceMessages();
    LogTraceMessage(traceMessage);
    return true;
}

void MiKTeXApp::FlushPendingTraceMessages()
{
    for (const TraceCallback::TraceMessage& msg : pendingTraceMessages)
    {
        if (isLog4cxxConfigured)
        {
            LogTraceMessage(msg);
        }
        else
        {
            cerr << msg.message << endl;
        }
    }
    pendingTraceMessages.clear();
}

void MiKTeXApp::LogTraceMessage(const TraceCallback::TraceMessage& traceMessage)
{
    MIKTEX_ASSERT(isLog4cxxConfigured);
    log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(string("trace.miktex.") + traceMessage.facility);
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

string Timestamp()
{
  auto now = time(nullptr);
  stringstream s;
  s << std::put_time(localtime(&now), "%Y-%m-%d-%H%M%S");
  return s.str();
}

void MiKTeXApp::RunProcess(const PathName& fileName, const vector<string>& arguments)
{
    ProcessOutput<4096> output;
    int exitCode;
    MiKTeXException miktexException;
    if (!Process::Run(fileName, arguments, &output, &exitCode, &miktexException, nullptr) || exitCode != 0)
    {
        auto outputBytes = output.GetStandardOutput();
        PathName outfile = this->session->GetSpecialPath(SpecialPath::LogDirectory) / fileName.GetFileNameWithoutExtension();
        outfile += "_";
        outfile += Timestamp().c_str();
        outfile.SetExtension(".out");
        File::WriteBytes(outfile, outputBytes);
        MIKTEX_ASSERT(isLog4cxxConfigured);
        LOG4CXX_ERROR(logger, "sub-process error output has been saved to '" << outfile.ToDisplayString() << "'");
        throw miktexException;
    }
}

void MiKTeXApp::Verbose(int level, const string& s)
{
    if (level >= 4)
    {
        LOG4CXX_TRACE(logger, s);
    }
    else if (level >= 2)
    {
        LOG4CXX_DEBUG(logger, s);
    }
    else
    {
        LOG4CXX_INFO(logger, s);
    }
    if (!this->quiet && this->verbosityLevel >= level)
    {
        Output(s);
    }
}

void MiKTeXApp::Output(const string& s)
{
    cout << s << endl;
}

void MiKTeXApp::Warning(const string& message)
{
    if (isLog4cxxConfigured)
    {
        LOG4CXX_WARN(logger, message);
    }
    if (!this->quiet)
    {
        cerr << fmt::format(T_("warning: {0}"), message) << endl;
    }
}

void MiKTeXApp::SecurityRisk(const string& message)
{
    if (isLog4cxxConfigured)
    {
        LOG4CXX_WARN(logger, fmt::format(T_("security risk: {0}"), message));
    }
    if (!this->quiet)
    {
        cerr << fmt::format(T_("security risk: {0}"), message) << endl;
    }
}

MIKTEXNORETURN void MiKTeXApp::FatalError(const string& message)
{
    if (isLog4cxxConfigured)
    {
        LOG4CXX_FATAL(logger, message);
    }
    Sorry(message, "", "", "");
    throw 1;
}

MIKTEXNORETURN void MiKTeXApp::IncorrectUsage(const string& message)
{
    if (isLog4cxxConfigured)
    {
        LOG4CXX_FATAL(logger, message);
    }
    cerr << fmt::format(T_("Incorrect usage: {0}"), message) << endl;
    throw 1;
}

void MiKTeXApp::ShowUsage()
{
    cout
        << fmt::format(T_("Usage: {0} [OPTION...] TOPIC COMMAND [COMMAND-OPTION...]"), this->InvocationName()) << "\n"
        << T_("Topics:") << endl;
    for (auto& t : topics)
    {
        cout << fmt::format("  {0}  {1}", t.second->Name(), t.second->Description()) << endl;
    }
}

void MiKTeXApp::ShowVersion()
{
    cout
        << Utils::MakeProgramVersionString(TheNameOfTheGame, VersionNumber(MIKTEX_COMPONENT_VERSION_STR)) << "\n"
        << "\n"
        << MIKTEX_COMP_COPYRIGHT_STR << "\n"
        << "\n"
        << "This is free software; see the source for copying conditions.  There is NO" << "\n"
        << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
}

bool MiKTeXApp::InstallPackage(const string& packageId, const PathName& trigger, PathName& installRoot)
{
  if (!this->IsInstallerEnabled())
  {
    return false;
  }
  LOG4CXX_INFO(logger, "installing package " << packageId << " triggered by " << trigger.ToString());
  Verbose(fmt::format(T_("Installing package {0}..."), packageId));
  EnsureInstaller();
  packageInstaller->SetFileLists({ packageId }, {});
  packageInstaller->InstallRemove(PackageInstaller::Role::Application);
  installRoot = session->GetSpecialPath(SpecialPath::InstallRoot);
  return true;
}

bool IsGlobalOption(const string& s, const string& optionName)
{
    return s == "-"s + optionName || s == "--"s + optionName;
}

tuple<int, vector<string>> MiKTeXApp::Init(const vector<string>& args)
{
    this->args = args;
    ctx.installer = this;
    ctx.logger = this;
    ctx.processRunner = this;
    ctx.program = this;
    ctx.ui = this;
    RegisterTopics();
    bool adminMode = false;
    bool forceAdminMode = false;
    Session::InitOptionSet options;
    bool optVersion = false;
    vector<string> newargs;
    bool processingGlobalOptions = true;
    MIKTEX_ASSERT(args.size() > 0);
    auto arg0 = args[0];
    for (size_t idx = 1; idx < args.size(); ++idx)
    {
        const string& arg = args[idx];
        bool isGlobalOption = arg.length() > 0 && arg[0] == '-';
        if (!isGlobalOption)
        {
            processingGlobalOptions = false;
        }
        if (processingGlobalOptions)
        {
            if (IsGlobalOption(arg, "admin"))
            {
                adminMode = true;
            }
            else if (IsGlobalOption(arg, "disable-installer"))
            {
                this->enableInstaller = TriState::False;
            }
            else if (IsGlobalOption(arg, "enable-installer"))
            {
                this->enableInstaller = TriState::True;
            }
            else if (IsGlobalOption(arg, "help"))
            {
                ShowUsage();
                return {-1, vector<string>()};
            }
            else if (IsGlobalOption(arg, "quiet"))
            {
                this->quiet = true;
            }
            else if (IsGlobalOption(arg, "principal=setup"))
            {
                options += Session::InitOption::SettingUp;
                forceAdminMode = true;
            }
            else if (IsGlobalOption(arg, "verbose"))
            {
                this->verbosityLevel++;
            }
            else if (IsGlobalOption(arg, "version"))
            {
                optVersion = true;
            }
            else
            {
                isGlobalOption = false;
            }
        }
        if (!isGlobalOption)
        {
            newargs.push_back(arg);
        }
    }
    Session::InitInfo initInfo(arg0);
    initInfo.SetOptions(options);
    initInfo.SetTraceCallback(this);
    this->session = Session::Create(initInfo);
    ctx.session = this->session;
    this->packageManager = PackageManager::Create(PackageManager::InitInfo(this));
    ctx.packageManager = this->packageManager;
    this->packageInstaller = this->packageManager->CreateInstaller({ this, true, false });
    ctx.packageInstaller = this->packageInstaller;
    if (optVersion)
    {
        ShowVersion();
        session->Close();
        session = nullptr;
        return {-1, vector<string>()};
    }
    if (adminMode)
    {
        if (!forceAdminMode && !session->IsSharedSetup())
        {
            FatalError(T_("option --admin only makes sense for a shared MiKTeX setup"));
        }
        if (!session->RunningAsAdministrator())
        {
            Warning(T_("option --admin may require administrator privileges"));
        }
        session->SetAdminMode(true, forceAdminMode);
    }
    if (session->RunningAsAdministrator() && !session->IsAdminMode())
    {
        SecurityRisk(T_("running with elevated privileges"));
    }
    if (this->enableInstaller == TriState::Undetermined)
    {
        this->enableInstaller = session->GetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOINSTALL).GetTriState();
    }
    PathName xmlFileName;
    if (session->FindFile("miktex." MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName) || session->FindFile(MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName))
    {
        Utils::SetEnvironmentString("MIKTEX_LOG_DIR", session->GetSpecialPath(SpecialPath::LogDirectory).ToString());
        string logName = "miktex";
        if (session->IsAdminMode() && session->RunningAsAdministrator())
        {
            logName += MIKTEX_ADMIN_SUFFIX;
        }
        Utils::SetEnvironmentString("MIKTEX_LOG_NAME", logName);
        log4cxx::xml::DOMConfigurator::configure(xmlFileName.ToWideCharString());
    }
    else
    {
        log4cxx::BasicConfigurator::configure();
    }
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
    LOG4CXX_INFO(logger, "this is " << Utils::MakeProgramVersionString(TheNameOfTheGame, VersionNumber(MIKTEX_COMPONENT_VERSION_STR)));
    PathName cwd;
    cwd.SetToCurrentDirectory();
    LOG4CXX_INFO(logger, fmt::format("this process ({0}) started by {1} in directory {2} with command line: {3}", thisProcess->GetSystemId(), Q_(invokerName), cwd.ToDisplayString(), CommandLineBuilder(args)));
    FlushPendingTraceMessages();
    string programName;
#if defined(MIKTEX_WINDOWS)
    programName = PathName(arg0).GetFileNameWithoutExtension().ToString();
#else
    programName = PathName(arg0).GetFileName().ToString();
#endif
    if (programName == "mkfntmap")
    {
        Shims::mkfntmap(newargs);
    }
    else if (programName == "updmap")
    {
        Shims::updmap(&ctx, newargs);
    }
    InstallSignalHandler(SIGINT);
    InstallSignalHandler(SIGTERM);
    return {0, newargs};
}

void MiKTeXApp::Finalize()
{
    FlushPendingTraceMessages();
    session->Close();
    session = nullptr;
}

int MiKTeXApp::Run(const vector<string>& args)
{
    if (args.size() == 0)
    {
        IncorrectUsage(fmt::format(T_("missing topic; try {0} --help"), this->InvocationName()));
    }
    auto it = topics.find(args[0]);
    if (it == topics.end())
    {
        IncorrectUsage(fmt::format(T_("{0}: unknown topic"), args[0]));
    }
    return it->second->Execute(ctx, args);
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
        vector<string> args;
        args.reserve(argc);
        for (int idx = 0; idx < argc; ++idx)
        {
#if defined(_UNICODE)
            args.push_back(StringUtil::WideCharToUTF8(argv[idx]));
#elif defined(MIKTEX_WINDOWS)
            args.push_back(StringUtil::AnsiToUTF8(argv[idx]));
#else
            args.push_back(argv[idx]);
#endif
        }
        MiKTeXApp app;
        auto [initSuccess, runArgs] = app.Init(args);
        if (initSuccess == 0)
        {
            retCode = app.Run(runArgs);
            app.Finalize();
        }
        else if (initSuccess > 0)
        {
            retCode = initSuccess;
        }
    }
    catch (const MiKTeXException& e)
    {
        if (logger != nullptr && isLog4cxxConfigured)
        {
            LOG4CXX_FATAL(logger, e.GetErrorMessage());
            LOG4CXX_FATAL(logger, "Info: " << e.GetInfo());
            LOG4CXX_FATAL(logger, "Source: " << e.GetSourceFile());
            LOG4CXX_FATAL(logger, "Line: " << e.GetSourceLine());
        }
        Sorry(e.GetErrorMessage(), e.GetDescription(), e.GetRemedy(), e.GetUrl());
        e.Save();
        retCode = 1;
    }
    catch (const exception& e)
    {
        if (logger != nullptr && isLog4cxxConfigured)
        {
            LOG4CXX_FATAL(logger, e.what());
        }
        Sorry(e.what(), "", "",  "");
        retCode = 1;
    }
    catch (int exitCode)
    {
        retCode = exitCode;
    }
    if (logger != nullptr && isLog4cxxConfigured)
    {
        LOG4CXX_INFO(logger, "this process (" << Process::GetCurrentProcess()->GetSystemId() << ") finishes with exit code " << retCode);
        logger = nullptr;
    }
    return retCode;
}
