/* mpm.cpp:

   Copyright (C) 2008-2016 Christian Schenk

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

#include <QApplication>
#include <QtWidgets>

#include <miktex/Core/Exceptions>
#include <miktex/Wrappers/PoptWrapper>

#include "MainWindow.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Wrappers;
using namespace std;

enum {
  OPT_AAA = 1,
  OPT_ADMIN,
};

namespace {
  struct poptOption const aoption[] = {
    {
      "admin", 0, POPT_ARG_NONE, nullptr, OPT_ADMIN,
      "Run in administrative mode.", nullptr
    },
    POPT_TABLEEND
  };
}

int main(int argc, char ** argv)
{
  QApplication application(argc, argv);
  int ret = 0;
  bool optAdmin = false;
  try
  {
    Session::InitInfo initInfo;
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
      }
    }
    shared_ptr<Session> pSession;
    initInfo.SetProgramInvocationName(argv[0]);
    pSession = Session::Create(initInfo);
    if (optAdmin)
    {
      if (!pSession->RunningAsAdministrator())
      {
#if defined(MIKTEX_WINDOWS)
        QMessageBox::critical(0, "MiKTeX Package Manager", "Not running as administrator.");
#else
        QMessageBox::critical(0, "MiKTeX Package Manager", "Not running as root.");
#endif
        return 1;;
      }
      pSession->SetAdminMode(true);
    }
    MainWindow mainWindow;
    mainWindow.show();
    ret = application.exec();
  }
  catch (const MiKTeXException & e)
  {
    ret = 1;
  }
  catch (const exception & e)
  {
    ret = 1;
  }
  return ret;
}
