/* miktex/Core/test.h: test framework                   -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

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

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(F3C19AE97A4B49AC9E96B6A8293DC1A7)
#define F3C19AE97A4B49AC9E96B6A8293DC1A7

#include <cstdio>

#include <exception>
#include <memory>
#include <string>
#include <vector>

#if defined(_MSC_VER)
#  include <direct.h>
#  include <io.h>
#  include <fcntl.h>
#  include <process.h>
#endif

#include <miktex/Core/Exceptions>
#include <miktex/Core/PathName>
#include <miktex/Core/Process>
#include <miktex/Core/Session>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceCallback>
#include <miktex/Trace/TraceStream>
#include <miktex/Wrappers/PoptWrapper>

#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>

#define MIKTEXTEST_BEGIN_NAMESPACE              \
  namespace MiKTeX {                            \
    namespace Test {

#define MIKTEXTEST_END_NAMESPACE                \
    }                                           \
  }

MIKTEXTEST_BEGIN_NAMESPACE

#define T_(x) MIKTEXTEXT(x)

#if defined(MIKTEX_WINDOWS)
const char PATH_DELIMITER = ';';
#define PATH_DELIMITER_STRING ";"
#else
const char PATH_DELIMITER = ':';
#define PATH_DELIMITER_STRING ":"
#endif

enum class Option
{
  RootDirectories = 1000,
  InstallRoot,
  DataRoot,
  Trace,
};

#define FATAL()                                                         \
  LOG4CXX_FATAL(logger, "fatal error in " << __FILE__ << ", line " << __LINE__); \
  throw 1

class TestScript : public MiKTeX::Trace::TraceCallback
{
protected:
  TestScript(std::string name) :
    scriptName(name)
  {
#if defined(TRACEFLAGS)
    traceFlags = TRACEFLAGS;
#endif
  }

protected:
  std::string scriptName;

protected:
  log4cxx::LoggerPtr logger = nullptr;

private:
  bool isLog4cxxConfigured = false;

private:
  std::vector<MiKTeX::Trace::TraceCallback::TraceMessage> pendingTraceMessages;

public:
  void MIKTEXTHISCALL Trace(const MiKTeX::Trace::TraceCallback::TraceMessage & traceMessage) override
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
    for (const MiKTeX::Trace::TraceCallback::TraceMessage & msg : pendingTraceMessages)
    {
      TraceInternal(msg);
    }
    pendingTraceMessages.clear();
  }

private:
  void TraceInternal(const MiKTeX::Trace::TraceCallback::TraceMessage & traceMessage)
  {
    log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(std::string("trace.test.") + traceMessage.facility);

    if (traceMessage.streamName == MIKTEX_TRACE_ERROR)
    {
      LOG4CXX_ERROR(logger, traceMessage.message);
    }
    else
    {
      LOG4CXX_TRACE(logger, traceMessage.message);
    }
  }


public:
  virtual ~TestScript()
  {
  }

protected:
  std::vector<std::string> argv;

public:
  int Main(int argc, const char **argv)
  {
#if defined(_MSC_VER)
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
#endif
    int rc = 0;
    int chOption;
    MiKTeX::Wrappers::PoptWrapper popt(argc, argv, optionTable);
    while ((chOption = popt.GetNextOpt()) >= 0)
    {
      switch ((Option)chOption)
      {
      case Option::InstallRoot:
        installRoot = popt.GetOptArg();
        break;
      case Option::DataRoot:
        dataRoot = popt.GetOptArg();
        break;
      case Option::RootDirectories:
        rootDirectories = popt.GetOptArg();
        break;
      case Option::Trace:
        traceFlags = popt.GetOptArg();
        break;
      default:
        ;
      }
    }
    for (const std::string & arg : popt.GetLeftovers())
    {
      vecArgs.push_back(arg);
    }
    try
    {
      for (int idx = 0; idx < argc && argv[idx] != nullptr; ++idx)
      {
        this->argv.push_back(argv[idx]);
      }
      MiKTeX::Core::Session::InitInfo initInfo(argv[0]);
      MiKTeX::Core::StartupConfig startupConfig;

#if defined(INSTALLROOT) && defined(DATAROOT)
      startupConfig.userDataRoot = DATAROOT;
      startupConfig.userInstallRoot = INSTALLROOT;
#else
      startupConfig.userRoots = rootDirectories;
      startupConfig.userDataRoot = dataRoot;
      startupConfig.userInstallRoot = installRoot;
#endif
      initInfo.SetStartupConfig(startupConfig);

#if defined(INITFLAGS)
      initInfo.SetFlags(INITFLAGS);
#endif

#if defined(BINDIR)
#  error BINDIR is deprecated
#endif

      initInfo.SetTraceCallback(this);

      pSession = MiKTeX::Core::Session::Create(initInfo);

      MiKTeX::Core::PathName xmlFileName;
      if (pSession->FindFile("log4cxx.xml", "%R/miktex/config", xmlFileName))
      {
        logger = log4cxx::Logger::getLogger(scriptName);
#if defined(MIKTEX_WINDOWS)
        log4cxx::xml::DOMConfigurator::configure(xmlFileName.ToWideCharString());
#else
        log4cxx::xml::DOMConfigurator::configure(xmlFileName.ToString());
#endif
        isLog4cxxConfigured = true;
      }

      LOG4CXX_INFO(logger, "starting tests");
      if (!traceFlags.empty())
      {
        MiKTeX::Trace::TraceStream::SetTraceFlags(traceFlags.c_str());
      }
      Run();
      pSession = nullptr;
    }
    catch (const FAILURE & f)
    {
      if (isLog4cxxConfigured)
      {
        LOG4CXX_FATAL(logger, f.strFile << ":" << f.iLine << ": TEST FAILED");
      }
      rc = 1;
    }
    catch (const MiKTeX::Core::MiKTeXException & e)
    {
      if (isLog4cxxConfigured)
      {
        LOG4CXX_FATAL(logger, e.what());
        LOG4CXX_FATAL(logger, "Info: " << e.GetInfo());
        LOG4CXX_FATAL(logger, "Source: " << e.GetSourceFile());
        LOG4CXX_FATAL(logger, "Line: " << e.GetSourceLine());
      }
      rc = 1;
    }
    catch (const std::exception & e)
    {
      if (isLog4cxxConfigured)
      {
        LOG4CXX_FATAL(logger, e.what());
      }
      rc = 1;
    }
    catch (int code)
    {
      LOG4CXX_FATAL(logger, "test script failure " << code);
      rc = 1;
    }
    return rc;
  }

protected:
  struct FAILURE
  {
    int iNum;
    std::string strFile;
    int iLine;
    FAILURE(int i) :
      iNum(i)
    {
    }
  };

protected:
  virtual void Run() = 0;

protected:
  bool FindFile(const char * lpszFileName, const char * lpszPathList)
  {
    MiKTeX::Core::PathName path;
    return pSession->FindFile(lpszFileName, lpszPathList, path);
  }

protected:
  bool RunTestCommand(const char * lpszProgramName)
  {
  std::string commandLine = MiKTeX::Core::PathName(pSession->GetMyLocation(), lpszProgramName).ToString();
    for (std::vector<std::string>::const_iterator it = argv.begin(); it != argv.end(); ++it)
    {
      if (it != argv.begin())
      {
        commandLine += " " + *it;
      }
    }
    return system(commandLine.c_str()) == 0;
  }

protected:
  void Touch(const char * lpszPath)
  {
    FILE * pFile;
#if defined(_MSC_VER) && _MSC_VER >= 1400
    if (fopen_s(&pFile, lpszPath, "wb") != 0)
    {
      pFile = nullptr;
    }
#else
    pFile = fopen(lpszPath, "wb");
#endif
    if (pFile == nullptr)
    {
      FATAL();
    }
    fclose(pFile);
  }

protected:
  void Touch(const MiKTeX::Core::PathName & path)
  {
    Touch(path.GetData());
  }
  
protected:
  std::shared_ptr<MiKTeX::Core::Session> pSession;

protected:
  std::vector<std::string> vecArgs;

protected:
  std::string traceFlags;

protected:
  std::string rootDirectories;

protected:
  std::string installRoot;

protected:
  std::string dataRoot;

private:
  static const struct poptOption optionTable[];
};

const struct poptOption TestScript::optionTable[] = {
  { "install-root", 0, POPT_ARG_STRING, 0, (int)Option::InstallRoot,
    T_("Set the install root directory."), 0},
  { "data-root", 0, POPT_ARG_STRING, 0, (int)Option::DataRoot,
    T_("Set the data root directory."), 0},
  { "root-directories", 0, POPT_ARG_STRING, 0, (int)Option::RootDirectories,
    T_("Set the root directories."), 0},
  { "trace", 0, POPT_ARG_STRING, 0, (int)Option::Trace,
    T_("Set trace flags."), 0},
  POPT_TABLEEND
};

class ProcessOutput : public MiKTeX::Core::IRunProcessCallback
{
public:
  bool MIKTEXTHISCALL OnProcessOutput(const void * ptr, size_t n) override
  {
    output.append(reinterpret_cast<const char *>(ptr), n);
    return true;
  }
private:
  std::string output;
public:
  std::string GetOutput() const
  {
    return output;
  }
};

#define FAIL() f.iLine = __LINE__; f.strFile = __FILE__; throw (f)

#define TEST(exp)                               \
  LOG4CXX_INFO(logger, "Test: " << #exp);       \
  if (! (exp)) { FAIL(); }

#define TESTX(exp)                              \
  LOG4CXX_INFO(logger, "Test: " << #exp);       \
  try                                           \
  {                                             \
    exp;                                        \
  }                                             \
  catch (const MiKTeX::Core::MiKTeXException &) \
  {                                             \
    FAIL();                                     \
  }

#define BEGIN_TEST_FUNCTION(num)                \
  void TestFunc##num()                          \
  {                                             \
    FAILURE f(num);                             \
    LOG4CXX_INFO(logger, "Testfunc " << num)   \

#define END_TEST_FUNCTION() \
  }

#define CALL_TEST_FUNCTION(num) TestFunc##num ()

#define BEGIN_TEST_SCRIPT(name)                 \
  class MyTestScript : public TestScript        \
  {                                             \
  public:                                       \
    MyTestScript() : TestScript(name) {}        \
  protected:

#define END_TEST_SCRIPT()                       \
  };

#define BEGIN_TEST_PROGRAM()                    \
protected:                                      \
  virtual void Run()                            \
  {

#define END_TEST_PROGRAM()                      \
  }

#define RUN_TEST_SCRIPT()                       \
int main (int argc, const char **argv)          \
{                                               \
  MyTestScript script;                          \
  return script.Main(argc, argv);               \
}

MIKTEXTEST_END_NAMESPACE;

#endif
