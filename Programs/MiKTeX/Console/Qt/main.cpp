/* main.cpp:

   Copyright (C) 2017-2018 Christian Schenk

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

#include <QApplication>
#include <QtWidgets>
#include <QSystemTrayIcon>

#include <miktex/Core/Exceptions>
#include <miktex/Core/Session>
#include <miktex/Wrappers/PoptWrapper>

#include "mainwindow.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Wrappers;
using namespace std;

enum {
  OPT_AAA = 1,
  OPT_ADMIN,
  OPT_FINISH_SETUP,
  OPT_HIDE
};

namespace {
  struct poptOption const aoption[] = {
    {
      "admin", 0, POPT_ARG_NONE, nullptr, OPT_ADMIN,
      "Run in administrator mode.", nullptr
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
    POPT_TABLEEND
  };
}

int main(int argc, char* argv[])
{
#if QT_VERSION >= 0x050600
  QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
  QApplication application(argc, argv);
  int ret = 0;
  bool optAdmin = false;
  bool optFinishSetup = false;
  bool optHide = false;
  try
  {
    Session::InitInfo initInfo;
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
      case OPT_HIDE:
        optHide = true;
        break;
      case OPT_FINISH_SETUP:
        optFinishSetup = true;
        break;
      }
    }
    initInfo.SetProgramInvocationName(argv[0]);
    shared_ptr<Session> session = Session::Create(initInfo);
    QString displayName = "MiKTeX Console";
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
      session->SetAdminMode(true);
      displayName += " (Admin)";
    }
#if QT_VERSION >= 0x050000
    application.setApplicationDisplayName(displayName);
#endif
    MainWindow mainWindow;
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
    ret = application.exec();
  }
  catch (const MiKTeXException& e)
  {
    ret = 1;
  }
  catch (const exception& e)
  {
    ret = 1;
  }
  return ret;
}
