/* miktex.cpp: One MiKTeX utility

   Copyright (C) 2021 Christian Schenk

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
   USA.  */

#if defined(HAVE_CONFIG_H)
#  include <config.h>
#endif

#include <atomic>
#include <iostream>
#include <string>
#include <vector>
#include <map>

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

#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Exceptions>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Core/Session>
#include <miktex/Core/Text>
#include <miktex/Core/Utils>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>
#include <miktex/Util/StringUtil>

#if defined(MIKTEX_WINDOWS)
#include <miktex/Core/win/ConsoleCodePageSwitcher>
#endif

#include "internal.h"
#include "topics/Topic.h"
#include "topics/filesystem/topic.h"

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

using namespace OneMiKTeXUtility;

#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()
#define T_(x) MIKTEXTEXT(x)

const char* const TheNameOfTheGame = T_("One MiKTeX Utility");

#define PROGNAME "miktex"

static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger(PROGNAME));
static bool isLog4cxxConfigured = false;

static std::atomic<bool> canceled;

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

static void Sorry(const string& description, const string& remedy, const string& url)
{
  if (cerr.fail())
  {
    return;
  }
  cerr << endl;
  if (description.empty())
  {
    cerr << fmt::format(T_("Sorry, but {0} did not succeed."), Q_(TheNameOfTheGame)) << endl;
  }
  else
  {
    cerr
      << fmt::format(T_("Sorry, but {0} did not succeed for the following reason:"), Q_(TheNameOfTheGame)) << "\n"
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
        << T_("The log file hopefully contains the information to get MiKTeX going again:") << "\n"
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

static void Sorry()
{
    Sorry("", "", "");
}

class MiKTeXApp :
    public TraceCallback,
    public OneMiKTeXUtility::Controller,
    public OneMiKTeXUtility::UI
{
public:
    int Init(std::vector<std::string>& args);

public:
    void Finalize();

public:
    int Run(const std::vector<std::string>& args);

private:
    bool Canceled() override
    {
        return canceled;
    }

private:
    void RegisterTopic(unique_ptr<Topics::Topic> t)
    {
        string name = t->Name();
        topics[name] = std::move(t);
    }

private:
    void RegisterTopics()
    {
        RegisterTopic(Topics::FileSystem::Create());
    }

private:
    void Verbose(const std::string& s);

private:
    MIKTEXNORETURN void FatalError(const std::string& s);

private:
    void BadUsage(const std::string& s);

private:
    void ShowUsage();

private:
    void ShowVersion();

private:
    void Output(const std::string& s) override;

private:
    void Error(const std::string& s) override;

private:
    void Warning(const std::string& s);

private:
    void SecurityRisk(const std::string& s);

private:
    void PushTraceMessage(const MiKTeX::Trace::TraceCallback::TraceMessage& traceMessage);

private:
    void PushTraceMessage(const std::string& message);
  
private:
    bool Trace(const MiKTeX::Trace::TraceCallback::TraceMessage& traceMessage) override;

private:
    void FlushPendingTraceMessages();

private:
    void LogTraceMessage(const MiKTeX::Trace::TraceCallback::TraceMessage& traceMessage);

private:
    ApplicationContext ctx;
    std::vector<MiKTeX::Trace::TraceCallback::TraceMessage> pendingTraceMessages;
    bool quiet;
    std::shared_ptr<MiKTeX::Core::Session> session;
    std::map<std::string, std::unique_ptr<Topics::Topic>> topics;
    bool verbose = false;
};

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

void MiKTeXApp::Verbose(const string& s)
{
    if (verbose)
    {
        Output(s);
    }
}

void MiKTeXApp::Output(const string& s)
{
    cout << s << endl;
}

void MiKTeXApp::Error(const string& s)
{
    if (isLog4cxxConfigured)
    {
        LOG4CXX_ERROR(logger, s);
    }
    cerr << PROGNAME << ": " << T_("error") << ": " << s << endl;
}

void MiKTeXApp::Warning(const string& s)
{
    if (isLog4cxxConfigured)
    {
        LOG4CXX_WARN(logger, s);
    }
    if (!quiet)
    {
        cerr << PROGNAME << ": " << T_("warning") << ": " << s << endl;
    }
}

void MiKTeXApp::SecurityRisk(const string& s)
{
    if (isLog4cxxConfigured)
    {
        LOG4CXX_WARN(logger, T_("security risk") << ": " << s);
    }
    if (!quiet)
    {
        cerr << PROGNAME << ": " << T_("security risk") << ": " << s << endl;
    }
}

MIKTEXNORETURN void MiKTeXApp::FatalError(const string& s)
{
    if (isLog4cxxConfigured)
    {
        LOG4CXX_FATAL(logger, s);
    }
    else
    {
        cerr << s << endl;
    }
    Sorry(s, "", "");
    throw 1;
}

void MiKTeXApp::BadUsage(const string& s)
{
    cerr << T_("bad usage: ") << s << endl;
}

void MiKTeXApp::ShowUsage()
{
    cout << T_("Usage: miktex [options] topic command") << "\n"
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

int MiKTeXApp::Init(vector<string>& args)
{
    ctx.controller = this;
    ctx.ui = this;
    RegisterTopics();
    bool adminMode = false;
    bool forceAdminMode = false;
    Session::InitOptionSet options;
    MIKTEX_ASSERT(args.size() > 0);
    size_t idx = 1;
    for (; idx < args.size() && args[idx].length() > 0 && args[idx][0] == '-'; ++idx)
    {
        const string& opt = args[idx];
        if (opt == "--admin" || opt == "-admin")
        {
            adminMode = true;
        }
        else if (opt == "--help" || opt == "-help")
        {
            ShowUsage();
            return -1;
        }
        else if (opt == "--principal=setup" || opt == "-principal=setup")
        {
            options += Session::InitOption::SettingUp;
            forceAdminMode = true;
        }
        else if (opt == "--version" || opt == "-version")
        {
            ShowVersion();
            return -1;
        }
        else
        {
            BadUsage(fmt::format(T_("unknown option: {0}"), opt));
            return 1;
        }
    }
    Session::InitInfo initInfo(args[0]);
    initInfo.SetOptions(options);
    initInfo.SetTraceCallback(this);
    session = Session::Create(initInfo);
    if (adminMode)
    {
        if (!forceAdminMode && !session->IsSharedSetup())
        {
            FatalError(T_("Option --admin only makes sense for a shared MiKTeX setup."));
        }
        if (!session->RunningAsAdministrator())
        {
            Warning(T_("Option --admin may require administrator privileges"));
        }
        session->SetAdminMode(true, forceAdminMode);
    }
    if (session->RunningAsAdministrator() && !session->IsAdminMode())
    {
        SecurityRisk(T_("running with elevated privileges"));
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
        Utils::SetEnvironmentString("MIKTEX_LOG_NAME",logName);
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
    LOG4CXX_INFO(logger, "this process (" << thisProcess->GetSystemId() << ") started by '" << invokerName << "' with command line: " << CommandLineBuilder(args));
    FlushPendingTraceMessages();
    args.erase(args.begin(), args.begin() + idx);
    InstallSignalHandler(SIGINT);
    InstallSignalHandler(SIGTERM);
    return 0;
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
        BadUsage(T_("missing topic"));
        return 1;
    }
    auto it = topics.find(args[0]);
    if (it == topics.end())
    {
        BadUsage(fmt::format(T_("unknown topic: {0}"), args[0]));
        return 1;
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
        vector<string> utf8args;
        utf8args.reserve(argc);
        for (int idx = 0; idx < argc; ++idx)
        {
#if defined(_UNICODE)
            utf8args.push_back(StringUtil::WideCharToUTF8(argv[idx]));
#elif defined(MIKTEX_WINDOWS)
            utf8args.push_back(StringUtil::AnsiToUTF8(argv[idx]));
#else
            utf8args.push_back(argv[idx]);
#endif
        }
        MiKTeXApp app;
        auto initSuccess = app.Init(utf8args);
        if (initSuccess == 0)
        {
            retCode = app.Run(utf8args);
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
        else
        {
            cerr << e.GetErrorMessage() << endl
                 << "Info: " << e.GetInfo() << endl
                 << "Source: " << e.GetSourceFile() << endl
                 << "Line: " << e.GetSourceLine() << endl;
        }
        Sorry(e.GetDescription(), e.GetRemedy(), e.GetUrl());
        e.Save();
        retCode = 1;
    }
    catch (const exception& e)
    {
        if (logger != nullptr && isLog4cxxConfigured)
        {
            LOG4CXX_FATAL(logger, e.what());
        }
        else
        {
            cerr << e.what() << endl;
        }
        Sorry();
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
