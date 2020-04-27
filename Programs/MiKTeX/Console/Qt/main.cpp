/* main.cpp:

   Copyright (C) 2017-2020 Christian Schenk

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
      "Run in MiKTeX administrator mode.", nullptr
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
  bool Trace(const TraceCallback::TraceMessage& traceMessage) override
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
    else
    {
#if defined(MIKTEX_WINDOWS)
      OutputDebugStringW(StringUtil::UTF8ToWideChar(traceMessage.message).c_str());
#else
      cerr << traceMessage << endl;
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

int main(int argc, char* argv[])
{
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
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
  QApplication application(argc, argv);
#if QT_VERSION >= 0x050000
  application.setApplicationDisplayName(displayName);
#endif
  unique_ptr<MiKTeX::Core::LockFile> lockFile;
  MiKTeX::Core::PathName lockFileName;
  try
  {
    lockFileName.SetToHomeDirectory();
    lockFileName /= "miktex-console.lock";
    lockFile = LockFile::Create(lockFileName);
    if (!lockFile->TryLock(500ms))
    {
      QMessageBox::warning(nullptr, displayName, "MiKTeX Console is already running.");
      return 1;
    }
  }
  catch (const exception& e)
  {
#if defined(MIKTEX_WINDOWS)
    OutputDebugStringW(StringUtil::UTF8ToWideChar(e.what()).c_str());
#endif
    QMessageBox::critical(nullptr, displayName, QString("MiKTeX Console cannot be started.\n\nRemedy: remove %1").arg(QString::fromUtf8(lockFileName.GetData())));
    return 1;
  }
  MainWindow::Pages startPage = MainWindow::Pages::Overview;
  if (argc > 0)
  {
    PathName name = PathName(argv[0]).GetFileNameWithoutExtension();
    name += MIKTEX_EXE_FILE_SUFFIX;
#if defined(MIKTEX_WINDOWS)
    if (name == PathName(MIKTEX_TASKBAR_ICON_EXE))
    {
      optHide = true;
      optMkmaps = true;
    }
    else if (name == PathName(MIKTEX_UPDATE_EXE) || name == PathName(MIKTEX_UPDATE_ADMIN_EXE))
    {
      startPage = MainWindow::Pages::Updates;
    }
#endif
  }
  TraceSink traceSink;
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
    shared_ptr<Session> session = Session::Create(initInfo);
    bool switchToAdmin = false;
    if (optAdmin)
    {
      if (!session->RunningAsAdministrator())
      {
#if defined(MIKTEX_WINDOWS)
        QMessageBox::critical(nullptr, displayName, "Administrator mode was requested (--admin), but the program is not running elevated (as Administrator).");
#else
        QMessageBox::critical(nullptr, displayName, "Administrator mode was requested (--admin), but the program is not running as root user.");
#endif
        return 1;
      }
      switchToAdmin = true;
    }
    if (switchToAdmin)
    {
      session->SetAdminMode(true);
      displayName += " (Admin)";
    }
    PathName xmlFileName;
    if (session->FindFile("console." MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName)
      || session->FindFile(MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName))
    {
      string logName = "miktex-console";
      if (session->IsAdminMode())
      {
        logName += MIKTEX_ADMIN_SUFFIX;
      }
      Utils::SetEnvironmentString("MIKTEX_LOG_DIR", session->GetSpecialPath(SpecialPath::LogDirectory).ToString());
      Utils::SetEnvironmentString("MIKTEX_LOG_NAME", logName);
      log4cxx::xml::DOMConfigurator::configure(xmlFileName.ToWideCharString());
      isLog4cxxConfigured = true;
      traceSink.FlushPendingTraceMessages();
      LOG4CXX_INFO(logger, "starting: " << Utils::MakeProgramVersionString("MiKTeX Console", VersionNumber(MIKTEX_COMPONENT_VERSION_STR)));
    }
    if (optVersion)
    {
      cout
        << Utils::MakeProgramVersionString("MiKTeX Console", VersionNumber(MIKTEX_COMPONENT_VERSION_STR)) << endl
        << "Copyright (C) 2018-2020 Christian Schenk" << endl
        << "This is free software; see the source for copying conditions.  There is NO" << endl
        << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
      return 0;
    }
    bool fastExit = false;
    {
      MainWindow mainWindow(nullptr, startPage, optFinishSetup || optCheckUpdates, &traceSink);
      if (optHide)
      {
        mainWindow.hide();
      }
      else
      {
        mainWindow.show();
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
      ret = application.exec();
      fastExit = mainWindow.IsCleaningUp();
    }
    if (session.use_count() > 1 && !fastExit)
    {
      LOG4CXX_WARN(logger, "session.use_count() == " << session.use_count());
    }
    session = nullptr;
    if (isLog4cxxConfigured && !fastExit)
    {
      LOG4CXX_INFO(logger, "finishing with exit code " << ret);
    }
    lockFile->Unlock();
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
