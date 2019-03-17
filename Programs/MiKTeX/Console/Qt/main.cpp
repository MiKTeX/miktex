/* main.cpp:

   Copyright (C) 2017-2019 Christian Schenk

   This file is part of MiKTeX Console.

   MiKTeX Console is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX Console is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Console; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#if defined(MIKTEX_MACOS_BUNDLE)
#include <mach-o/dyld.h>
#endif

#include <iostream>

#include <cstdlib>

#include <QApplication>
#include <QtWidgets>
#include <QSystemTrayIcon>

#include <log4cxx/logger.h>
#include <log4cxx/rollingfileappender.h>
#include <log4cxx/xml/domconfigurator.h>

#include "console-version.h"

#include <miktex/Core/Exceptions>
#include <miktex/Core/LockFile>
#include <miktex/Core/Paths>
#include <miktex/Core/Session>
#include <miktex/UI/Qt/ErrorDialog>
#include <miktex/Util/StringUtil>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceCallback>
#include <miktex/Wrappers/PoptWrapper>

#include "mainwindow.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::UI::Qt;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;
using namespace std;

enum {
  OPT_AAA = 1,
  OPT_ADMIN,
  OPT_CHECK_UPDATES,
  OPT_FINISH_SETUP,
  OPT_HIDE,
  OPT_MKMAPS,
  OPT_START_PAGE,
  OPT_VERSION
};

namespace {
  struct poptOption const aoption[] = {
    {
      "admin", 0, POPT_ARG_NONE, nullptr, OPT_ADMIN,
      "Run in administrator mode.", nullptr
    },
    {
      "check-updates", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_CHECK_UPDATES,
      nullptr, nullptr
    },
    {
      "finish-setup", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_FINISH_SETUP,
      nullptr, nullptr
    },
#if !defined(QT_NO_SYSTEMTRAYICON)
    {
      "hide", 0, POPT_ARG_NONE, nullptr, OPT_HIDE,
      "Hide the main window.", nullptr
    },
#endif
    {
      "mkmaps", 0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr, OPT_MKMAPS,
      nullptr, nullptr
    },
    {
      "start-page", 0, POPT_ARG_STRING, nullptr, OPT_START_PAGE,
      "Start with the page (one of: settings, updates, packages, diagnose, cleanup).", "PAGE"
    },
    {
      "version", 0, POPT_ARG_NONE, nullptr, OPT_VERSION,
      "Show version information and exit.", nullptr
    },
    POPT_AUTOHELP
    POPT_TABLEEND
  };
}

static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("main"));
static bool isLog4cxxConfigured = false;

class TraceSink :
  public MiKTeX::Trace::TraceCallback
{
public:
  void Trace(const TraceCallback::TraceMessage& traceMessage) override
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
  vector<TraceCallback::TraceMessage> pendingTraceMessages;

public:
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
    if (isLog4cxxConfigured)
    {
      log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(string("trace.") + "miktex-console" + "." + traceMessage.facility);
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
#if defined(MIKTEX_WINDOWS)
      OutputDebugStringW(StringUtil::UTF8ToWideChar(traceMessage.message).c_str());
#else
      fprintf(stderr, "%s\n", traceMessage.message.c_str());
#endif
    }
  }
};

#if defined(MIKTEX_MACOS_BUNDLE)
PathName GetExecutablePath()
{
  CharBuffer<char> buf;
  uint32_t bufsize = buf.GetCapacity();
  if (_NSGetExecutablePath(buf.GetData(), &bufsize) < 0)
  {
    buf.Reserve(bufsize);
    if (_NSGetExecutablePath(buf.GetData(), &bufsize) != 0)
    {
      MIKTEX_UNEXPECTED();
    }
  }
  char resolved[PATH_MAX + 1];
  if (realpath(buf.GetData(), resolved) == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  return resolved;
}

PathName GetExecutableDir()
{
  return GetExecutablePath().RemoveFileSpec();
}
#endif

#if defined(MIKTEX_WINDOWS)
#  define ENABLE_TRACE_MAIN
#endif

#define TRACE_MAIN() TraceMain(__FILE__, __LINE__)

#if defined(ENABLE_TRACE_MAIN)
inline void TraceMain(const char* file, int line)
{
#if defined(MIKTEX_WINDOWS)
  string outputString = "MiKTeX Console: reached line "s + std::to_string(line);
  OutputDebugStringA(outputString.c_str());
#endif
}
#else
inline void TraceMain(const char* file, int line)
{
}
#endif

int main(int argc, char* argv[])
{
  TRACE_MAIN();
  int ret = 0;
  bool optAdmin = false;
  bool optCheckUpdates = false;
  bool optFinishSetup = false;
  bool optHide = false;
  bool optMkmaps = false;
  bool optVersion = false;
  QString displayName = "MiKTeX Console";
#if defined(MIKTEX_MACOS_BUNDLE)
  PathName plugIns = GetExecutableDir() / ".." / "PlugIns";
  plugIns.MakeAbsolute();
  QCoreApplication::addLibraryPath(QString::fromUtf8(plugIns.GetData()));
#endif
#if QT_VERSION >= 0x050600
  TRACE_MAIN();
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  TRACE_MAIN();
#endif
  TRACE_MAIN();
  QApplication application(argc, argv);
  TRACE_MAIN();
#if QT_VERSION >= 0x050000
  TRACE_MAIN();
  application.setApplicationDisplayName(displayName);
  TRACE_MAIN();
#endif
  TRACE_MAIN();
  unique_ptr<MiKTeX::Core::LockFile> lockFile;
  TRACE_MAIN();
  try
  {
    TRACE_MAIN();
    lockFile = LockFile::Create(PathName().SetToHomeDirectory() / "miktex-console.lock");
    TRACE_MAIN();
    if (!lockFile->TryLock(500ms))
    {
      QMessageBox::warning(nullptr, displayName, "MiKTeX Console is already running.");
      return 1;
    }
    TRACE_MAIN();
  }
  catch (const exception& e)
  {
#if defined(MIKTEX_WINDOWS)
    OutputDebugStringW(StringUtil::UTF8ToWideChar(e.what()).c_str());
#endif
    QMessageBox::critical(nullptr, displayName, "MiKTeX Console cannot be started.");
    return 1;
  }
  MainWindow::Pages startPage = MainWindow::Pages::Overview;
  if (argc > 0)
  {
    TRACE_MAIN();
    PathName name = PathName(argv[0]).GetFileNameWithoutExtension();
    TRACE_MAIN();
    name += MIKTEX_EXE_FILE_SUFFIX;
#if defined(MIKTEX_WINDOWS)
    if (name == MIKTEX_TASKBAR_ICON_EXE)
    {
      optHide = true;
      optMkmaps = true;
    }
    else if (name == MIKTEX_UPDATE_EXE || name == MIKTEX_UPDATE_ADMIN_EXE)
    {
      startPage = MainWindow::Pages::Updates;
    }
#endif
  }
  TRACE_MAIN();
  TraceSink traceSink;
  TRACE_MAIN();
  try
  {
    Session::InitInfo initInfo;
    initInfo.SetTraceCallback(&traceSink);
#if 0
    initInfo.SetOptions({ Session::InitOption::NoFixPath });
#endif
    PoptWrapper popt(argc, const_cast<const char**>(argv), aoption);
    int option;
    while ((option = popt.GetNextOpt()) >= 0)
    {
      string optArg = popt.GetOptArg();
      switch (option)
      {
      case OPT_ADMIN:
        optAdmin = true;
        break;
      case OPT_CHECK_UPDATES:
        optCheckUpdates = true;
        break;
      case OPT_HIDE:
        optHide = true;
        break;
      case OPT_FINISH_SETUP:
        optFinishSetup = true;
        break;
      case OPT_MKMAPS:
        optMkmaps = true;
        break;
      case OPT_START_PAGE:
        if (optArg == "overview")
        {
          startPage = MainWindow::Pages::Overview;
        }
        else if (optArg == "settings")
        {
          startPage = MainWindow::Pages::Settings;
        }
        else if (optArg == "updates")
        {
          startPage = MainWindow::Pages::Updates;
        }
        else if (optArg == "packages")
        {
          startPage = MainWindow::Pages::Packages;
        }
        else if (optArg == "diagnose")
        {
          startPage = MainWindow::Pages::Diagnose;
        }
        else if (optArg == "cleanup")
        {
          startPage = MainWindow::Pages::Cleanup;
        }
        else
        {
          QMessageBox::critical(nullptr, displayName, QString("Unknown page: %1").arg(optArg.c_str()));
          return 1;
        }
        break;
      case OPT_VERSION:
        optVersion = true;
        break;
      }
    }
    initInfo.SetProgramInvocationName(argv[0]);
    TRACE_MAIN();
    shared_ptr<Session> session = Session::Create(initInfo);
    TRACE_MAIN();
    bool switchToAdmin = false;
    if (optAdmin)
    {
      TRACE_MAIN();
      if (!session->RunningAsAdministrator())
      {
#if defined(MIKTEX_WINDOWS)
        QMessageBox::critical(nullptr, displayName, "Administrator mode was requested (--admin), but the program is not running elevated (as Administrator).");
#else
        QMessageBox::critical(nullptr, displayName, "Administrator mode was requested (--admin), but the program is not running as root user.");
#endif
        return 1;
      }
      TRACE_MAIN();
      switchToAdmin = true;
    }
    if (switchToAdmin)
    {
      TRACE_MAIN();
      session->SetAdminMode(true);
      TRACE_MAIN();
      displayName += " (Admin)";
    }
    PathName xmlFileName;
    TRACE_MAIN();
    if (session->FindFile("console." MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName)
      || session->FindFile(MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName))
    {
      TRACE_MAIN();
      string logName = "miktex-console";
      TRACE_MAIN();
      if (session->IsAdminMode())
      {
        logName += MIKTEX_ADMIN_SUFFIX;
      }
      TRACE_MAIN();
      Utils::SetEnvironmentString("MIKTEX_LOG_DIR", session->GetSpecialPath(SpecialPath::LogDirectory).ToString());
      TRACE_MAIN();
      Utils::SetEnvironmentString("MIKTEX_LOG_NAME", logName);
      TRACE_MAIN();
      log4cxx::xml::DOMConfigurator::configure(xmlFileName.ToWideCharString());
      isLog4cxxConfigured = true;
      TRACE_MAIN();
      traceSink.FlushPendingTraceMessages();
      TRACE_MAIN();
      LOG4CXX_INFO(logger, "starting: " << Utils::MakeProgramVersionString("MiKTeX Console", MIKTEX_COMPONENT_VERSION_STR));
      TRACE_MAIN();
    }
    TRACE_MAIN();
    if (optVersion)
    {
      cout
        << Utils::MakeProgramVersionString("MiKTeX Console", MIKTEX_COMPONENT_VERSION_STR) << endl
        << "Copyright (C) 2019 Christian Schenk" << endl
        << "This is free software; see the source for copying conditions.  There is NO" << endl
        << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
      return 0;
    }
    bool fastExit = false;
    {
      TRACE_MAIN();
      MainWindow mainWindow(nullptr, startPage);
      TRACE_MAIN();
      if (optHide)
      {
        mainWindow.hide();
      }
      else
      {
        TRACE_MAIN();
        mainWindow.show();
        TRACE_MAIN();
      }
      if (optFinishSetup)
      {
        QTimer::singleShot(100, &mainWindow, SLOT(FinishSetup()));
      }
      if (optCheckUpdates)
      {
        QTimer::singleShot(100, &mainWindow, SLOT(CheckUpdates()));
      }
      if (optMkmaps)
      {
        QTimer::singleShot(100, &mainWindow, SLOT(RefreshFontMaps()));
      }
      TRACE_MAIN();
      ret = application.exec();
      TRACE_MAIN();
      fastExit = mainWindow.IsCleaningUp();
    }
    if (session.use_count() > 1 && !fastExit)
    {
      LOG4CXX_WARN(logger, "session.use_count() == " << session.use_count());
    }
    TRACE_MAIN();
    session = nullptr;
    TRACE_MAIN();
    if (isLog4cxxConfigured && !fastExit)
    {
      LOG4CXX_INFO(logger, "finishing with exit code " << ret);
    }
    TRACE_MAIN();
    lockFile->Unlock();
    TRACE_MAIN();
  }
  catch (const MiKTeXException& e)
  {
    ret = 1;
    if (isLog4cxxConfigured)
    {
      LOG4CXX_FATAL(logger, e.GetErrorMessage());
      LOG4CXX_FATAL(logger, "finishing with exit code " << ret);
    }
    else
    {
      traceSink.FlushPendingTraceMessages();
    }
    if (QMessageBox::critical(nullptr, "MiKTeX Console", "Sorry, but something went wrong.\n\nDo you want to see the error details?",
      QMessageBox::StandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No))
      == QMessageBox::StandardButton::Yes)
    {
      ErrorDialog::DoModal(nullptr, e);
    }
  }
  catch (const exception& e)
  {
    ret = 1;
    if (isLog4cxxConfigured)
    {
      LOG4CXX_FATAL(logger, e.what());
      LOG4CXX_FATAL(logger, "finishing with exit code " << ret);
    }
    else
    {
      traceSink.FlushPendingTraceMessages();
    }
    if (QMessageBox::critical(nullptr, "MiKTeX Console", "Sorry, but something went wrong.\n\nDo you want to see the error details?",
      QMessageBox::StandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No))
      == QMessageBox::StandardButton::Yes)
    {
      ErrorDialog::DoModal(nullptr, e);
    }
  }
  return ret;
}
