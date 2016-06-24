/* MainWindow.h:                                        -*- C++ -*-

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

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(C8BEA09A85AB447CB4A4AC18DAEA430C)
#define C8BEA09A85AB447CB4A4AC18DAEA430C

#include <miktex/Core/Session>
#include <miktex/PackageManager/PackageManager>

#if defined(_UNICODE)
#if defined(_M_IX86)
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined(_M_IA64)
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined(_M_X64)
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#include "ui_MainWindow.h"

class PackageTableModel;
class QSortFilterProxyModel;

class MainWindow :
  public QMainWindow,
  private Ui::MainWindow
{
private:
  Q_OBJECT;

public:
  MainWindow();

private slots:
  void EnableActions();

private slots:
  void SelectInstallablePackages();

private slots:
  void PropertyDialog();

private slots:
  void Install();

private slots:
  void Uninstall();

private slots:
  void RepositoryWizard();

private slots:
  void Synchronize();

private slots:
  void AboutDialog();

private:
  void Unimplemented();

private:
  PackageTableModel * pModel;

private:
  QSortFilterProxyModel * pProxyModel;

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> pManager;

private:
  std::shared_ptr<MiKTeX::Core::Session> pSession = MiKTeX::Core::Session::Get();
};

#endif
