/**
 * @file miktex/miktex-texworks.cpp:
 * @author Christian Schenk
 * @brief Version number
 *
 * @copyright Copyright © 2015-2025 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#include <sstream>

#include "miktex-texworks-version.h"

#include <miktex/Core/Exceptions>
#include <miktex/Util/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/Session>
#include <miktex/Core/Utils>
#include <miktex/Trace/Trace>
#include <miktex/Util/StringUtil>

#if defined(MIKTEX_WINDOWS)
#include <miktex/Core/win/Registry>
#endif

#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#if LOG4CXX_VERSION_MAJOR > 0
#include <log4cxx/rolling/rollingfileappender.h> 
#else
#include <log4cxx/rollingfileappender.h>
#endif
#if defined(LOG4CXX_INFO_FMT)
#   define MIKTEX_LOG4CXX_12 1
#endif

#include "miktex-texworks.hpp"

#include "Settings.h"
#include "TWUtils.h"

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

using namespace MiKTeX::TeXworks;

static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("texworks"));
static log4cxx::LoggerPtr synctexLogger(log4cxx::Logger::getLogger("synctex"));

Wrapper* Wrapper::instance;

Wrapper::Wrapper() :
    traceStream(TraceStream::Open("texworks", this))
{
    if (instance != nullptr)
    {
        MIKTEX_UNEXPECTED();
    }
    instance = this;
}

int Wrapper::Run(int(*Main)(int argc, char* argv[]), int argc, char* argv[])
{
    try
    {
        vector<char*> args;
        args.reserve(argc + 1);
#if defined(MIKTEX_WINDOWS)
        vector<string> utf8args;
        utf8args.reserve(argc);
        for (int idx = 0; idx < argc; ++idx)
        {
            utf8args.push_back(StringUtil::AnsiToUTF8(argv[idx]));
            args.push_back(const_cast<char*>(utf8args[idx].c_str()));
        }
#else
        for (int idx = 0; idx < argc; ++idx)
        {
            args.push_back(argv[idx]);
        }
#endif
        args.push_back(nullptr);
        Session::InitInfo initInfo;
        initInfo.SetProgramInvocationName(args[0]);
        initInfo.SetTraceCallback(this);
        shared_ptr<Session> session = Session::Create(initInfo);
        PathName xmlFileName;
        if (session->FindFile("texworks." MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName)
            || session->FindFile(MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName))
        {
            Utils::SetEnvironmentString("MIKTEX_LOG_DIR", PathName(session->GetSpecialPath(SpecialPath::DataRoot)).AppendComponent(MIKTEX_PATH_MIKTEX_LOG_DIR).ToString());
            Utils::SetEnvironmentString("MIKTEX_LOG_NAME", "texworks");
            log4cxx::xml::DOMConfigurator::configure(xmlFileName.ToWideCharString());
            LOG4CXX_INFO(logger, "starting: " << Utils::MakeProgramVersionString("MiKTeX TeXworks", VersionNumber(MIKTEX_COMP_ORIG_VERSION_STR)));
            FlushPendingTraceMessages();
            isLog4cxxConfigured = true;
            string cmdline;
            for (int idx = 1; args[idx]; ++idx)
            {
                cmdline += ' ';
                cmdline += args[idx];
            }
            if (!cmdline.empty())
            {
                LOG4CXX_INFO(logger, "  argument(s):" << cmdline);
            }
        }
        int exitCode = Main(argc, argv);
        LOG4CXX_INFO(logger, "exit code: " << exitCode);
        return exitCode;
    }
    catch (const MiKTeXException& e)
    {
        LOG4CXX_FATAL(logger, "MiKTeX exception: " << e.GetErrorMessage());
        LOG4CXX_FATAL(logger, "   Info: " << e.GetInfo());
        LOG4CXX_FATAL(logger, " Source: " << e.GetSourceFile());
        LOG4CXX_FATAL(logger, "   Line: " << e.GetSourceLine());
        Sorry();
        return 1;
    }
    catch (const exception& e)
    {
        LOG4CXX_FATAL(logger, "std exception: " << e.what());
        Sorry();
        return 1;
    }
}

bool Wrapper::Trace(const TraceCallback::TraceMessage & traceMessage)
{
    if (isLog4cxxConfigured)
    {
        TraceInternal(traceMessage);
    }
    else
    {
        pendingTraceMessages.push_back(traceMessage);
    }
    return true;
}

void Wrapper::FlushPendingTraceMessages()
{
    for (const TraceCallback::TraceMessage& msg : pendingTraceMessages)
    {
        TraceInternal(msg);
    }
    pendingTraceMessages.clear();
}

void Wrapper::TraceInternal(const TraceCallback::TraceMessage & traceMessage)
{
    log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(string("trace.texworks.") + traceMessage.facility);
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

void Wrapper::Sorry(string reason)
{
    stringstream serr;
    serr
        << "\n"
        << "Sorry, but something went wrong";
    if (reason.empty())
    {
        serr << "." << "\n";
    }
    else
    {
        serr
            << " for the following reason:" << "\n" << "\n"
            << "  " << reason << "\n";
    }
#if defined(MIKTEX_LOG4CXX_12)
    log4cxx::AppenderPtr appender = log4cxx::Logger::getRootLogger()->getAppender(LOG4CXX_STR("RollingLogFile"));
    log4cxx::FileAppenderPtr fileAppender = log4cxx::cast<log4cxx::FileAppender>(appender);
#else
    log4cxx::FileAppenderPtr fileAppender = log4cxx::Logger::getRootLogger()->getAppender(LOG4CXX_STR("RollingLogFile"));
#endif
    if (fileAppender != nullptr)
    {
        serr
            << endl
            << "The log file hopefully contains the information to get MiKTeX TeXworks going again:" << endl
            << endl
            << "  " << PathName(fileAppender->getFile()).ToUnix() << endl;
    }
    serr
        << endl
        << "You may want to visit the MiKTeX project page (https://miktex.org), if you need help." << endl;
#if defined(MIKTEX_WINDOWS)
    MessageBoxW(nullptr, StringUtil::UTF8ToWideChar(serr.str()).c_str(), L"MiKTeX TeXworks", MB_ICONERROR);
#else
    // TODO: cerr << serrstr() << endl;
#endif
}

namespace Tw {
#if defined(MIKTEX_WINDOWS)
    Settings::Settings() :
        m_s("\\HKEY_CURRENT_USER\\" MIKTEX_REGPATH_SERIES "\\" "TeXworks", QSettings::NativeFormat)
    {
        
    }
#else
    Settings::Settings()
    {
    }
#endif
}
