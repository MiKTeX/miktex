/* mainwindow.cpp:

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

#include <QThread>
#include <QTimer>
#include <QtWidgets>

#include "RootTableModel.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "console-version.h"

#include <miktex/Core/ConfigNames>
#include <miktex/Core/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Registry>
#include <miktex/Core/Session>
#include <miktex/Setup/SetupService>
#include <miktex/UI/Qt/ErrorDialog>
#include <miktex/UI/Qt/SiteWizSheet>
#include <miktex/UI/Qt/UpdateDialog>
#include <miktex/Util/StringUtil>

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Setup;
using namespace MiKTeX::UI::Qt;
using namespace MiKTeX::Util;
using namespace std;

inline double Divide(double a, double b)
{
  return a / b;
}

MainWindow::MainWindow(QWidget* parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  packageManager(PackageManager::Create())
{
  ui->setupUi(this);

  rootModel = new RootTableModel(this);
  ui->treeViewRoots->setModel(rootModel);

  time_t lastAdminMaintenance = static_cast<time_t>(std::stoll(session->GetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_LAST_ADMIN_MAINTENANCE, "0").GetString()));
  time_t lastUserMaintenance = static_cast<time_t>(std::stoll(session->GetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_LAST_USER_MAINTENANCE, "0").GetString()));
  isSetupMode = lastAdminMaintenance == 0 && lastUserMaintenance == 0;

#if defined(MIKTEX_WINDOWS)
  bool withTrayIcon = true; // session->IsMiKTeXPortable();
#else
  bool withTrayIcon = false;
#endif

#if !defined(QT_NO_SYSTEMTRAYICON)
  if (withTrayIcon)
  {
    CreateTrayIcon();
  }
#endif

  SetCurrentPage(isSetupMode ? Pages::Setup : Pages::Overview);

  if (session->IsAdminMode())
  {
    setWindowTitle(windowTitle() + " (Admin)");
  }

  connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(AboutDialog()));
  connect(ui->actionRestartAdmin, SIGNAL(triggered()), this, SLOT(RestartAdmin()));
  connect(ui->actionTeXworks, SIGNAL(triggered()), this, SLOT(StartTeXworks()));
  connect(ui->actionTerminal, SIGNAL(triggered()), this, SLOT(StartTerminal()));

  UpdateWidgets();
  EnableActions();
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::CriticalError(const QString& text, const MiKTeXException& e)
{
  if (QMessageBox::critical(this, tr("MiKTeX Console"), text + "\n\n" + tr("Do you want to see the error details?"),
    QMessageBox::StandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No))
    == QMessageBox::StandardButton::Yes)
  {
    ErrorDialog::DoModal(this, e);
  }
}

#if !defined(QT_NO_SYSTEMTRAYICON)
void MainWindow::CreateTrayIcon()
{
  if (!QSystemTrayIcon::isSystemTrayAvailable())
  {
    return;
  }
  trayIconMenu = new QMenu(this);
  trayIconMenu->addAction(ui->actionTeXworks);
  trayIconMenu->addAction(ui->actionTerminal);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(ui->actionMinimize);
  trayIconMenu->addAction(ui->actionRestore);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(ui->actionExit);
  trayIcon = new QSystemTrayIcon(this);
  trayIcon->setContextMenu(trayIconMenu);
#if defined(MIKTEX_WINDOWS)
  trayIcon->setIcon(QIcon((":/Icons/miktex-16x16.png")));
#else
  trayIcon->setIcon(QIcon((":/Icons/miktex-32x32.png")));
#endif
  trayIcon->setToolTip(tr("MiKTeX Console"));
  trayIcon->show();

  connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::TrayIconActivated);
  connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::TrayMessageClicked);
}

void MainWindow::TrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
}

void MainWindow::TrayMessageClicked()
{
}

#endif

void MainWindow::UpdateWidgets()
{
  try
  {
    if (isSetupMode && IsBackgroundWorkerActive())
    {
      ui->labelSetupWait->show();
    }
    else
    {
      ui->labelSetupWait->hide();
    }
    if (session->IsAdminMode())
    {
      ui->privateMode->hide();
      ui->userMode->hide();
      ui->buttonAdminSetup->setText(tr("Finish shared setup"));
    }
    else
    {
      ui->adminMode->hide();
      if (session->IsSharedSetup())
      {
        ui->privateMode->hide();
      }
      else
      {
        ui->userMode->hide();
      }
    }
    ui->buttonOverview->setEnabled(!isSetupMode);
    ui->buttonSettings->setEnabled(!isSetupMode);
    ui->buttonUpdates->setEnabled(!isSetupMode);
    ui->buttonPackages->setEnabled(!isSetupMode);
    ui->buttonTeXworks->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && !session->IsAdminMode());
    ui->buttonTerminal->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode);
    if (isSetupMode)
    {
      return;
    }
    ui->groupPaper->setEnabled(!IsBackgroundWorkerActive());
    ui->groupPackageInstallation->setEnabled(!IsBackgroundWorkerActive());
    if (Utils::CheckPath(false))
    {
      ui->hintPath->hide();
    }
    ui->bindir->setText(QString::fromUtf8(session->GetSpecialPath(SpecialPath::LocalBinDirectory).GetData()));
    if (!IsBackgroundWorkerActive())
    {
      if (ui->labelUpgradeStatus->text().isEmpty())
      {
        if (packageManager->GetPackageInfo("ltxbase").IsInstalled() && packageManager->GetPackageInfo("amsfonts").IsInstalled())
        {
          ui->hintUpgrade->hide();
        }
      }
    }
    string repository;
    RepositoryType repositoryType(RepositoryType::Unknown);
    if (packageManager->TryGetDefaultPackageRepository(repositoryType, repository))
    {
      ui->editRepository->setText(QString::fromUtf8(repository.c_str()));
    }
    switch (session->GetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOINSTALL).GetTriState())
    {
    case TriState::True:
      ui->radioAutoInstallYes->setChecked(true);
      break;
    case TriState::False:
      ui->radioAutoInstallNo->setChecked(true);
      break;
    case TriState::Undetermined:
      ui->radioAutoInstallAsk->setChecked(true);
      break;
    }
    if (ui->comboPaper->count() == 0)
    {
      PaperSizeInfo defaultPaperSizeInfo;
      session->GetPaperSizeInfo(-1, defaultPaperSizeInfo);
      PaperSizeInfo paperSizeInfo;
      int currentIndex = -1;
      for (int idx = 0; session->GetPaperSizeInfo(idx, paperSizeInfo); ++idx)
      {
        QString displayName = QString::fromUtf8(paperSizeInfo.name.c_str());
        if (!Utils::EqualsIgnoreCase(paperSizeInfo.name, paperSizeInfo.dvipsName))
        {
          displayName += " (";
          displayName += QString::fromUtf8(paperSizeInfo.dvipsName.c_str());
          displayName += ")";
        }
        ui->comboPaper->addItem(displayName);
        if (Utils::EqualsIgnoreCase(paperSizeInfo.dvipsName, defaultPaperSizeInfo.dvipsName))
        {
          currentIndex = idx;
        }
      }
      if (currentIndex >= 0)
      {
        ui->comboPaper->setCurrentIndex(currentIndex);
      }
    }
    rootModel->Reload();
    ui->treeViewRoots->resizeColumnToContents(0);
  }
  catch (const MiKTeXException& e)
  {
    CriticalError(e);
  }
  catch (const exception& e)
  {
    CriticalError(e);
  }
}

void MainWindow::EnableActions()
{
  try
  {
    ui->actionRestartAdmin->setEnabled(!IsBackgroundWorkerActive() && session->IsSharedSetup() && !session->IsAdminMode());
    ui->actionTeXworks->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && !session->IsAdminMode());
    ui->actionTerminal->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode);
  }
  catch (const MiKTeXException& e)
  {
    CriticalError(e);
  }
  catch (const exception& e)
  {
    CriticalError(e);
  }
}

void MainWindow::SetCurrentPage(MainWindow::Pages p)
{
  switch (p)
  {
  case Pages::Setup:
    break;
  case Pages::Overview:
    ui->buttonOverview->setChecked(true);
    break;
  case Pages::Settings:
    ui->buttonSettings->setChecked(true);
    break;
  case Pages::Updates:
    ui->buttonUpdates->setChecked(true);
    break;
  case Pages::Packages:
    ui->buttonPackages->setChecked(true);
    break;
  }
  ui->pages->setCurrentIndex((int)p);
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

void MainWindow::on_buttonOverview_clicked()
{
  SetCurrentPage(Pages::Overview);
}

void MainWindow::on_buttonSettings_clicked()
{
  SetCurrentPage(Pages::Settings);
}

void MainWindow::on_buttonUpdates_clicked()
{
  SetCurrentPage(Pages::Updates);
}

void MainWindow::on_buttonPackages_clicked()
{
  SetCurrentPage(Pages::Packages);
}

void MainWindow::on_buttonAdminSetup_clicked()
{
  try
  {
    if (session->IsAdminMode())
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
    CriticalError(e);
  }
  catch (const exception& e)
  {
    CriticalError(e);
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
    CriticalError(e);
  }
  catch (const exception& e)
  {
    CriticalError(e);
  }
}

bool UpgradeWorker::Run()
{
  bool result = false;
  try
  {
    status = Status::Synchronize;
    packageInstaller = packageManager->CreateInstaller();
    packageInstaller->SetCallback(this);
    packageInstaller->FindUpgrades(PackageLevel::Basic);
    vector<PackageInstaller::UpgradeInfo> upgrades = packageInstaller->GetUpgrades();
    if (!upgrades.empty())
    {
      vector<string> toBeInstalled;
      for (const PackageInstaller::UpgradeInfo& upg : upgrades)
      {
        toBeInstalled.push_back(upg.deploymentName);
      }
      packageInstaller->SetFileLists(toBeInstalled, vector<string>());
      packageInstaller->InstallRemove();
    }
    result = true;
  }
  catch (const MiKTeXException& e)
  {
    this->e = e;
  }
  catch (const exception& e)
  {
    this->e = MiKTeXException(e.what());
  }
  return result;
}

void MainWindow::on_buttonUpgrade_clicked()
{
  ui->buttonUpgrade->setEnabled(false);
  QThread* thread = new QThread;
  UpgradeWorker* worker = new UpgradeWorker(packageManager);
  backgroundWorkers++;
  worker->moveToThread(thread);
  connect(thread, SIGNAL(started()), worker, SLOT(Process()));
  connect(worker, &UpgradeWorker::OnFinish, this, [this]() {
    ui->labelUpgradeStatus->setText(tr("done"));
    ui->labelUpgradePercent->setText("");
    ui->labelUpgradeDetails->setText("");
    backgroundWorkers--;
    UpdateWidgets();
    EnableActions();
  });
  connect(worker, &UpgradeWorker::OnUpgradeProgress, this, [this]() {
    PackageInstaller::ProgressInfo progressInfo = ((UpgradeWorker*)sender())->GetProgressInfo();
    UpgradeWorker::Status status = ((UpgradeWorker*)sender())->GetStatus();
    if (progressInfo.cbDownloadTotal > 0)
    {
      int percent = static_cast<int>(Divide(progressInfo.cbDownloadCompleted, progressInfo.cbDownloadTotal) * 100);
      ui->labelUpgradePercent->setText(percent == 100 ? tr("we're almost done") : QString("%1%").arg(percent));
    }
    if (status == UpgradeWorker::Status::Synchronize)
    {
      ui->labelUpgradeDetails->setText(tr("(synchronizing package database)"));
    }
    else if (status == UpgradeWorker::Status::Download)
    {
      ui->labelUpgradeDetails->setText(tr("(downloading: %1)").arg(QString::fromUtf8(progressInfo.deploymentName.c_str())));
    }
    else if (status == UpgradeWorker::Status::Install)
    {
      ui->labelUpgradeDetails->setText(tr("(installing: %1)").arg(QString::fromUtf8(progressInfo.deploymentName.c_str())));
    }
    else
    {
      ui->labelUpgradeDetails->setText("");
    }
  });
  connect(worker, &UpgradeWorker::OnMiKTeXException, this, [this]() {
    CriticalError(tr("Something went wrong while installing packages."), ((FinishSetupWorker*)sender())->GetMiKTeXException());
    ui->labelUpgradeStatus->setText(tr("Error"));
    ui->labelUpgradePercent->setText("");
    ui->labelUpgradeDetails->setText("");
    backgroundWorkers--;
    UpdateWidgets();
    EnableActions();
  });
  connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
  connect(worker, SIGNAL(OnFinish()), worker, SLOT(deleteLater()));
  connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  ui->labelUpgradeStatus->setText(tr("Upgrade in progress..."));
  ui->labelUpgradePercent->setText("0%");
  ui->labelUpgradeDetails->setText(tr("(initializing)"));
  thread->start();
  UpdateWidgets();
  EnableActions();
}

void MainWindow::RestartAdmin()
{
  try
  {
    RestartAdminWithArguments({ "--admin" });
  }
  catch (const MiKTeXException& e)
  {
    CriticalError(e);
  }
  catch (const exception& e)
  {
    CriticalError(e);
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

bool FinishSetupWorker::Run()
{
  bool result = false;
  try
  {
    shared_ptr<Session> session = Session::Get();
    unique_ptr<SetupService> service = SetupService::Create();
    SetupOptions options = service->GetOptions();
    options.Task = SetupTask::FinishSetup;
    options.IsCommonSetup = session->IsAdminMode();
    service->SetOptions(options);
    service->Run();
    result = true;
  }
  catch (const MiKTeXException& e)
  {
    this->e = e;
  }
  catch (const exception& e)
  {
    this->e = MiKTeXException(e.what());
  }
  return result;
}

void MainWindow::FinishSetup()
{
  ui->buttonAdminSetup->setEnabled(false);
  ui->buttonUserSetup->setEnabled(false);
  QThread* thread = new QThread;
  FinishSetupWorker* worker = new FinishSetupWorker;
  backgroundWorkers++;
  worker->moveToThread(thread);
  connect(thread, SIGNAL(started()), worker, SLOT(Process()));
  connect(worker, &FinishSetupWorker::OnFinish, this, [this]() {
    isSetupMode = false;
    try
    {
      bool isAdminMode = session->IsAdminMode();
      session->Reset();
      session->SetAdminMode(isAdminMode);
    }
    catch (const MiKTeXException& e)
    {
      CriticalError(e);
    }
    catch (const exception& e)
    {
      CriticalError(e);
    }
    backgroundWorkers--;
    UpdateWidgets();
    EnableActions();
    SetCurrentPage(Pages::Overview);
  });
  connect(worker, &FinishSetupWorker::OnMiKTeXException, this, [this]() {
    CriticalError(tr("Something went wrong while finishing the MiKTeX setup."), ((FinishSetupWorker*)sender())->GetMiKTeXException());
    backgroundWorkers--;
    UpdateWidgets();
    EnableActions();
  });
  connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
  connect(worker, SIGNAL(OnFinish()), worker, SLOT(deleteLater()));
  connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  thread->start();
  UpdateWidgets();
  EnableActions();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  if (IsBackgroundWorkerActive())
  {
    if (QMessageBox::question(this, tr("MiKTeX Console"), tr("A task is running in the background. Are you sure you want to quit?"))
      != QMessageBox::Yes)
    {
      event->ignore();
      return;
    }
  }
  event->accept();
}

void MainWindow::setVisible(bool visible)
{
  ui->actionMinimize->setEnabled(visible);
  ui->actionRestore->setEnabled(!visible);
  QMainWindow::setVisible(visible);
}

void MainWindow::StartTeXworks()
{
  try
  {
    PathName exePath;
    if (session->FindFile(MIKTEX_TEXWORKS_EXE, FileType::EXE, exePath))
    {
      session->UnloadFilenameDatabase();
      Process::Start(exePath);
    }
  }
  catch (const MiKTeXException& e)
  {
    CriticalError(e);
  }
  catch (const exception& e)
  {
    CriticalError(e);
  }
}

void MainWindow::StartTerminal()
{
  try
  {
    PathName localBinDir = session->GetSpecialPath(SpecialPath::LocalBinDirectory);
    string newPath = localBinDir.ToString();
    string oldPath;
    bool haveOldPath = Utils::GetEnvironmentString("PATH", oldPath);
    if (haveOldPath)
    {
      newPath += PathName::PathNameDelimiter;
      newPath += oldPath;
    }
    Utils::SetEnvironmentString("PATH", newPath);
    PathName cmd;
#if defined(MIKTEX_WINDOWS)
    if (!Utils::GetEnvironmentString("COMSPEC", cmd))
    {
      cmd = "cmd.exe";
    }
#elif defined(MIKTEX_MACOS_BUNDLE)
    cmd = session->GetMyProgramFile(true);
    cmd.RemoveFileSpec();
    cmd /= "Terminal";
#else
    const static string terminals[] = { "konsole", "gnome-terminal", "xterm" };
    for (const string& term : terminals)
    {
      if (session->FindFile(term, newPath, cmd))
      {
        break;
      }
    }
    if (cmd.Empty())
    {
      cmd = "xterm";
    }
#endif
    session->UnloadFilenameDatabase();
    Process::Start(cmd);
    if (haveOldPath)
    {
      Utils::SetEnvironmentString("PATH", oldPath);
    }
  }
  catch (const MiKTeXException& e)
  {
    CriticalError(e);
  }
  catch (const exception& e)
  {
    CriticalError(e);
  }
}

void MainWindow::on_comboPaper_activated(int idx)
{
  try
  {
    PaperSizeInfo paperSizeInfo;
    if (!session->GetPaperSizeInfo(idx, paperSizeInfo))
    {
      MIKTEX_UNEXPECTED();
    }
    session->SetDefaultPaperSize(paperSizeInfo.dvipsName.c_str());
    for (const FormatInfo& formatInfo : session->GetFormats())
    {
      vector<PathName> files;
      if (session->FindFile(formatInfo.name, FileType::FMT, files))
      {
        for (const PathName& file : files)
        {
          File::Delete(file, { FileDeleteOption::UpdateFndb });
        }
      }
    }
  }
  catch (const MiKTeXException& e)
  {
    CriticalError(e);
  }
  catch (const exception& e)
  {
    CriticalError(e);
  }
}

void MainWindow::on_radioAutoInstallAsk_clicked()
{
  session->SetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOINSTALL, (int)TriState::Undetermined);
}

void MainWindow::on_radioAutoInstallYes_clicked()
{
  session->SetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOINSTALL, (int)TriState::True);
}

void MainWindow::on_radioAutoInstallNo_clicked()
{
  session->SetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOINSTALL, (int)TriState::False);
}

void MainWindow::on_buttonChangeRepository_clicked()
{
  try
  {
    if (SiteWizSheet::DoModal(this) == QDialog::Accepted)
    {
      UpdateWidgets();
      EnableActions();
    }
  }
  catch (const MiKTeXException& e)
  {
    CriticalError(e);
  }
  catch (const exception& e)
  {
    CriticalError(e);
  }
}
