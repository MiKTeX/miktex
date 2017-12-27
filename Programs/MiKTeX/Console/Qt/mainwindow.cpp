/* mainwindow.cpp:

   Copyright (C) 2017 Christian Schenk

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

#include <QtWidgets>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "console-version.h"

#include <miktex/Core/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/Registry>
#include <miktex/Core/Session>
#include <miktex/UI/Qt/ErrorDialog>
#include <miktex/Util/StringUtil>

using namespace MiKTeX::Core;
using namespace MiKTeX::UI::Qt;
using namespace MiKTeX::Util;
using namespace std;

MainWindow::MainWindow(QWidget* parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  time_t lastAdminMaintenance = static_cast<time_t>(std::stoll(session->GetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_LAST_ADMIN_MAINTENANCE, "0").GetString()));
  time_t lastUserMaintenance = static_cast<time_t>(std::stoll(session->GetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_LAST_USER_MAINTENANCE, "0").GetString()));
  isSetupMode = lastAdminMaintenance == 0 && lastUserMaintenance == 0;

  int startPage = isSetupMode ? 0 : 1;

  ui->pages->setCurrentIndex(startPage);

  if (session->IsAdminMode())
  {
    setWindowTitle(windowTitle() + " (Admin)");
    ui->userMode->hide();
    ui->buttonAdminSetup->setText(tr("Finish shared setup"));
  }
  else
  {
    ui->adminMode->hide();
  }

  if (Utils::CheckPath(false))
  {
    ui->hintPath->hide();
  }

  ui->bindir->setText(QString::fromUtf8(session->GetSpecialPath(SpecialPath::LocalBinDirectory).GetData()));
  ui->installdir->setText(QString::fromUtf8(session->GetSpecialPath(SpecialPath::InstallRoot).GetData()));

  connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(AboutDialog()));
  connect(ui->actionRestartAdmin, SIGNAL(triggered()), this, SLOT(RestartAdmin()));

  EnableActions();
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::EnableActions()
{
  try
  {
    ui->actionRestartAdmin->setEnabled(!session->RunningAsAdministrator());
  }
  catch (const MiKTeXException& e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception& e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void MainWindow::AboutDialog()
{
  QString message;
  message = tr("MiKTeX Console");
  message += " ";
  message += MIKTEX_COMPONENT_VERSION_STR;
  message += "\n\n";
  message += tr("MiKTeX Console is free software. You are welcome to redistribute it under certain conditions. See the help file for more information.\n\nMiKTeX Console comes WITH ABSOLUTELY NO WARRANTY OF ANY KIND.");
  QMessageBox::about(this, tr("MiKTeX Console"), message);
}

void MainWindow::on_buttonAdminSetup_clicked()
{
  try
  {
    if (session->RunningAsAdministrator())
    {
      FinishSetup();
    }
    else
    {
      RestartAdminWithArguments({ "--admin", "--finish-setup" });
    }
  }
  catch (const MiKTeXException& e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception& e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void MainWindow::on_buttonUserSetup_clicked()
{
  try
  {
    FinishSetup();
  }
  catch (const MiKTeXException& e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception& e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void MainWindow::RestartAdmin()
{
  try
  {
    RestartAdminWithArguments({ "--admin" });
  }
  catch (const MiKTeXException& e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception& e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void MainWindow::RestartAdminWithArguments(const vector<string>& args)
{
#if defined(MIKTEX_WINDOWS) || defined(MIKTEX_MACOS_BUNDLE)
  PathName me = session->GetMyProgramFile(true);
  PathName adminFileName = me.GetFileNameWithoutExtension();
  adminFileName += MIKTEX_ADMIN_SUFFIX;
  PathName meAdmin(me);
  meAdmin.RemoveFileSpec();
  meAdmin /= adminFileName;
  meAdmin.SetExtension(me.GetExtension());
#if defined(MIKTEX_WINDOWS)
  ShellExecuteW(nullptr, L"open", meAdmin.ToWideCharString().c_str(), StringUtil::UTF8ToWideChar(StringUtil::Flatten(args, ' ')).c_str(), nullptr, SW_NORMAL);
#else
  vector<string> meAdminArgs{ adminFileName.ToString() };
  meAdminArgs.insert(meAdminArgs.end(), args.begin(), args.end());
  Process::Start(meAdmin, meAdminArgs);
#endif
#else
  // TODO
#endif
  this->close();
}

void MainWindow::FinishSetup()
{
  // TODO
}
