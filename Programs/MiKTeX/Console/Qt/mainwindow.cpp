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

#include <iomanip>

#include "PackageProxyModel.h"
#include "PackageTableModel.h"
#include "RootTableModel.h"
#include "UpdateTableModel.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "console-version.h"

#include <miktex/Core/ConfigNames>
#include <miktex/Core/DirectoryLister>
#include <miktex/Core/FileStream>
#include <miktex/Core/Fndb.h>
#include <miktex/Core/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Registry>
#include <miktex/Core/Session>
#include <miktex/Setup/SetupService>
#include <miktex/UI/Qt/ErrorDialog>
#include <miktex/UI/Qt/PackageInfoDialog>
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

void OpenDirectoryInFileBrowser(const QString& path_)
{
  QString path(path_);
  if (!path.startsWith('/'))
  {
    path.insert(0, '/');
  }
  QDesktopServices::openUrl(QUrl(QString("file://%1").arg(path), QUrl::TolerantMode));
}

void OpenDirectoryInFileBrowser(const PathName& dir_)
{
  PathName dir(dir_);
  dir.AppendDirectoryDelimiter();
#if defined(MIKTEX_WINDOWS)
  dir.ConvertToUnix();
#endif
  OpenDirectoryInFileBrowser(QString::fromUtf8(dir.GetData()));
}

MainWindow::MainWindow(QWidget* parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  resize(800, 600);

  SetupUiRootDirectories();
  SetupUiUpdates();
  SetupUiPackageInstallation();
  SetupUiPackages();
  SetupUiTroubleshoot();

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
  ui->tabWidgetSettings->setCurrentIndex(0);

  if (session->IsAdminMode())
  {
    setWindowTitle(windowTitle() + " (Admin)");
  }

  connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(AboutDialog()));
  connect(ui->actionRestartAdmin, SIGNAL(triggered()), this, SLOT(RestartAdmin()));
  connect(ui->actionRefreshFileNameDatabase, SIGNAL(triggered()), this, SLOT(RefreshFndb()));
  connect(ui->actionRefreshFontMapFiles, SIGNAL(triggered()), this, SLOT(RefreshFontMaps()));
  connect(ui->actionTeXworks, SIGNAL(triggered()), this, SLOT(StartTeXworks()));
  connect(ui->actionTerminal, SIGNAL(triggered()), this, SLOT(StartTerminal()));

  UpdateUi();
  UpdateActions();
}

MainWindow::~MainWindow()
{
  delete ui;
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

void MainWindow::CriticalError(const QString& text, const MiKTeXException& e)
{
  if (QMessageBox::critical(this, tr("MiKTeX Console"), text + "\n\n" + tr("Do you want to see the error details?"),
    QMessageBox::StandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No))
    == QMessageBox::StandardButton::Yes)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void MainWindow::UpdateUi()
{
  try
  {
    if (IsBackgroundWorkerActive())
    {
      ui->labelBackgroundTask->show();
    }
    else
    {
      ui->labelBackgroundTask->hide();
    }
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
    ui->buttonTroubleshoot->setEnabled(!isSetupMode);
    ui->buttonTeXworks->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && !session->IsAdminMode());
    ui->buttonTerminal->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode);
    if (isSetupMode)
    {
      return;
    }
    if (!Utils::CheckPath())
    {
      ui->groupPathIssue->show();
    }
    else
    {
      ui->groupPathIssue->hide();
    }
    ui->bindir->setText(QString::fromUtf8(session->GetSpecialPath(SpecialPath::LocalBinDirectory).GetData()));
    if (!IsBackgroundWorkerActive())
    {
      if (ui->labelUpgradeStatus->text().isEmpty())
      {
        if (packageManager->GetPackageInfo("ltxbase").IsInstalled() && packageManager->GetPackageInfo("amsfonts").IsInstalled())
        {
          ui->groupUpgrade->hide();
        }
        else
        {
          ui->groupCheckUpdates->hide();
        }
      }
    }
    UpdateUiPaper();
    UpdateUiPackageInstallation();
    UpdateUiRootDirectories();
    UpdateUiUpdates();
    UpdateUiPackages();
    UpdateUiTroubleshoot();
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

void MainWindow::UpdateActions()
{
  try
  {
    ui->actionRestartAdmin->setEnabled(!IsBackgroundWorkerActive() && session->IsSharedSetup() && !session->IsAdminMode());
    ui->actionRefreshFileNameDatabase->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode);
    ui->actionRefreshFontMapFiles->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode);
    ui->actionTeXworks->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && !session->IsAdminMode());
    ui->actionTerminal->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode);
    UpdateActionsRootDirectories();
    UpdateActionsUpdates();
    UpdateActionsPackages();
    UpdateActionsTroubleshoot();
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
    if (packageModel->rowCount() == 0)
    {
      packageModel->Reload();
    }
    break;
  case Pages::Troubleshoot:
    ui->buttonTroubleshoot->setChecked(true);
    break;
  }
  ui->pages->setCurrentIndex((int)p);
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
    cmd = session->GetMyLocation(true) / ".." / "Resources" / "Terminal";
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

void MainWindow::AboutDialog()
{
  QString message;
  message = tr("MiKTeX Console");
  message += " ";
  message += MIKTEX_COMPONENT_VERSION_STR;
  message += "\n\n";
  message += tr("MiKTeX Console is free software. You are welcome to redistribute it under certain conditions.\n\nMiKTeX Console comes WITH ABSOLUTELY NO WARRANTY OF ANY KIND.");
  QMessageBox::about(this, tr("MiKTeX Console"), message);
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
#if defined(MIKTEX_WINDOWSx)
  PathName me = session->GetMyProgramFile(true);
  PathName adminFileName = me.GetFileNameWithoutExtension();
  adminFileName += MIKTEX_ADMIN_SUFFIX;
  PathName meAdmin(me);
  meAdmin.RemoveFileSpec();
  meAdmin /= adminFileName;
  meAdmin.SetExtension(me.GetExtension());
  ShellExecuteW(nullptr, L"open", meAdmin.ToWideCharString().c_str(), StringUtil::UTF8ToWideChar(StringUtil::Flatten(args, ' ')).c_str(), nullptr, SW_NORMAL);
#elif defined(MIKTEX_MACOS_BUNDLE)
  PathName console = session->GetMyLocation(true) / ".." / "Resources" / MIKTEX_CONSOLE_ADMIN_EXE;
  vector<string> consoleArgs{ MIKTEX_CONSOLE_ADMIN_EXE };
  consoleArgs.insert(consoleArgs.end(), args.begin(), args.end());
  Process::Start(console, consoleArgs);
#else
  const static string frontends[] = { "kdesu", "gksu" };
  PathName frontend;
  for (const string& f : frontends)
  {
    if (session->FindFile(f, FileType::EXE, frontend))
    {
      break;
    }
  }
  if (frontend.Empty())
  {
    MIKTEX_FATAL_ERROR(tr("No graphical sudo frontend is available. Please install 'kdesu' (KDE) or 'gksu' (Gnome).").toStdString());
  }
  vector<string> frontendArgs{ frontend.GetFileNameWithoutExtension().ToString(), MIKTEX_CONSOLE_EXE };
  frontendArgs.insert(frontendArgs.end(), args.begin(), args.end());
  Process::Start(frontend, frontendArgs);
#endif
  this->close();
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
  try
  {
    ui->buttonAdminSetup->setEnabled(false);
    ui->buttonUserSetup->setEnabled(false);
    QThread* thread = new QThread;
    FinishSetupWorker* worker = new FinishSetupWorker;
    backgroundWorkers++;
    ui->labelBackgroundTask->setText(tr("Finishing the MiKTeX setup..."));
    worker->moveToThread(thread);
    connect(thread, SIGNAL(started()), worker, SLOT(Process()));
    connect(worker, &FinishSetupWorker::OnFinish, this, [this]() {
      FinishSetupWorker* worker = (FinishSetupWorker*)sender();
      if (worker->GetResult())
      {
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
        SetCurrentPage(Pages::Overview);
      }
      else
      {
        CriticalError(tr("Something went wrong while finishing the MiKTeX setup."), worker->GetMiKTeXException());
      }
      backgroundWorkers--;
      UpdateUi();
      UpdateActions();
      worker->deleteLater();
    });
    connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
    UpdateUi();
    UpdateActions();
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
  ui->labelBackgroundTask->setText(tr("Installing packages..."));
  worker->moveToThread(thread);
  connect(thread, SIGNAL(started()), worker, SLOT(Process()));
  connect(worker, &UpgradeWorker::OnFinish, this, [this]() {
    UpgradeWorker* worker = (UpgradeWorker*)sender();
    if (worker->GetResult())
    {
      ui->labelUpgradeStatus->setText(tr("Done"));
    }
    else
    {
      CriticalError(tr("Something went wrong while installing packages."), worker->GetMiKTeXException());
      ui->labelUpgradeStatus->setText(tr("Error"));
    }
    ui->labelUpgradePercent->setText("");
    ui->labelUpgradeDetails->setText("");
    backgroundWorkers--;
    UpdateUi();
    UpdateActions();
    worker->deleteLater();
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
      ui->labelUpgradeDetails->setText(tr("(updating package database)"));
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
  connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
  connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  ui->labelUpgradeStatus->setText(tr("Upgrade in progress..."));
  ui->labelUpgradePercent->setText("0%");
  ui->labelUpgradeDetails->setText(tr("(initializing)"));
  thread->start();
  UpdateUi();
  UpdateActions();
}

bool RefreshFndbWorker::Run()
{
  bool result = false;
  try
  {
    Fndb::Refresh(nullptr);
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

void MainWindow::RefreshFndb()
{
  QThread* thread = new QThread;
  RefreshFndbWorker* worker = new RefreshFndbWorker;
  backgroundWorkers++;
  ui->labelBackgroundTask->setText(tr("Refreshing file name database..."));
  worker->moveToThread(thread);
  connect(thread, SIGNAL(started()), worker, SLOT(Process()));
  connect(worker, &RefreshFndbWorker::OnFinish, this, [this]() {
    RefreshFndbWorker* worker = (RefreshFndbWorker*)sender();
    if (!worker->GetResult())
    {
      CriticalError(tr("Something went wrong while refreshing the file name database."), ((RefreshFndbWorker*)sender())->GetMiKTeXException());
    }
    backgroundWorkers--;
    UpdateUi();
    UpdateActions();
    worker->deleteLater();
  });
  connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
  connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  thread->start();
  UpdateUi();
  UpdateActions();
}

string Timestamp()
{
  auto now = time(nullptr);
  stringstream s;
  s << std::put_time(localtime(&now), "%Y-%m-%d-%H%M%S");
  return s.str();
}

bool RefreshFontMapsWorker::Run()
{
  bool result = false;
  try
  {
    shared_ptr<Session> session = Session::Get();
    PathName initexmf;
    if (!session->FindFile(MIKTEX_INITEXMF_EXE, FileType::EXE, initexmf))
    {
      MIKTEX_FATAL_ERROR(tr("The MiKTeX configuration utility executable (initexmf) could not be found.").toStdString());
    }
    vector<string> args{
      initexmf.GetFileNameWithoutExtension().ToString(),
      "--mkmaps"
    };
    if (session->IsAdminMode())
    {
      args.push_back("--admin");
    }
    ProcessOutput<4096> output;
    int exitCode;
    Process::Run(initexmf, args, &output, &exitCode, nullptr);
    if (exitCode != 0)
    {
      auto outputBytes = output.GetStandardOutput();
      PathName outfile = session->GetSpecialPath(SpecialPath::LogDirectory) / initexmf.GetFileNameWithoutExtension();
      outfile += "_";
      outfile += Timestamp().c_str();
      outfile.SetExtension(".out");
      FileStream outstream(File::Open(outfile, FileMode::Create, FileAccess::Write, false));
      outstream.Write(&outputBytes[0], outputBytes.size());
      outstream.Close();
      MIKTEX_FATAL_ERROR_2(tr("The MiKTeX configuration utility failed for some reason. The process output has been saved to a file.").toStdString(),
        "fileName", initexmf.ToString(), "exitCode", std::to_string(exitCode), "savedOutput", outfile.ToString());
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

void MainWindow::RefreshFontMaps()
{
  QThread* thread = new QThread;
  RefreshFontMapsWorker* worker = new RefreshFontMapsWorker;
  backgroundWorkers++;
  ui->labelBackgroundTask->setText(tr("Refreshing font map files..."));  
  worker->moveToThread(thread);
  connect(thread, SIGNAL(started()), worker, SLOT(Process()));
  connect(worker, &RefreshFontMapsWorker::OnFinish, this, [this]() {
    RefreshFontMapsWorker* worker = (RefreshFontMapsWorker*)sender();
    if (!worker->GetResult())
    {
      CriticalError(tr("Something went wrong while refreshing the font map files."), worker->GetMiKTeXException());
    }
    backgroundWorkers--;
    UpdateUi();
    UpdateActions();
    worker->deleteLater();
  });
  connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
  connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  thread->start();
  UpdateUi();
  UpdateActions();
}

void MainWindow::SetupUiUpdates()
{
  connect(ui->comboRepository3, static_cast<void(QComboBox::*)(int)> (&QComboBox::activated), this, [this](int index)
  {
    if (index == 1)
    {
      ChangeRepository();
    }
  });
  connect(ui->actionCheckUpdates, SIGNAL(triggered()), this, SLOT(CheckUpdates()));
  updateModel = new UpdateTableModel(packageManager, this);
  string lastUpdateCheck;
  if (session->TryGetConfigValue(
    MIKTEX_REGKEY_PACKAGE_MANAGER,
    session->IsAdminMode() ? MIKTEX_REGVAL_LAST_ADMIN_UPDATE_CHECK : MIKTEX_REGVAL_LAST_USER_UPDATE_CHECK,
    lastUpdateCheck))
  {
    ui->labelUpdateSummary->setText(tr("Last checked: %1").arg(QDateTime::fromTime_t(std::stoi(lastUpdateCheck)).date().toString()));
  }
  else
  {
    ui->labelUpdateSummary->setText(tr("You have not yet checked for updates."));
  }
  connect(updateModel, &UpdateTableModel::modelReset, this, [this]() {
    int n = updateModel->rowCount();
    QString text = tr("Updates");
    if (n == 0)
    {
      ui->labelUpdatesAvailable->hide();
      ui->labelNoUpdatesAvailable->show();
      ui->labelUpdateSummary->setText(tr("There are currently no updates available."));
    }
    else
    {
      ui->labelUpdatesAvailable->show();
      ui->labelNoUpdatesAvailable->hide();
      ui->labelUpdateSummary->setText(tr("The following updates are available:"));
      text += QString(" (%1)").arg(n);
    }
    ui->buttonUpdates->setText(text);
  });
  ui->treeViewUpdates->setModel(updateModel);
  connect(ui->treeViewUpdates->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    this,
    SLOT(UpdateActionsUpdates()));
  ui->labelUpdatesAvailable->hide();
  ui->labelNoUpdatesAvailable->hide();
}

void MainWindow::UpdateUiUpdates()
{
  ui->comboRepository3->clear();
  RepositoryType repositoryType(RepositoryType::Unknown);
  string repository;
  if (packageManager->TryGetDefaultPackageRepository(repositoryType, repository))
  {
    ui->comboRepository3->addItem(QString::fromUtf8(repository.c_str()));
  }
  else
  {
    ui->comboRepository3->addItem(tr("a random package repository on the Internet"));
  }
  ui->comboRepository3->addItem(tr("Change..."));
  ui->lineEditInstallRoot2->setText(QString::fromUtf8(session->GetSpecialPath(SpecialPath::InstallRoot).GetData()));
  ui->comboRepository3->setEnabled(!IsBackgroundWorkerActive());
  ui->buttonCheckUpdates->setEnabled(!IsBackgroundWorkerActive());
  ui->buttonUpdateCheck->setEnabled(!IsBackgroundWorkerActive());
  ui->buttonUpdateNow->setEnabled(!IsBackgroundWorkerActive() && updateModel->rowCount() > 0);
  ui->treeViewUpdates->setEnabled(!IsBackgroundWorkerActive());
}

void MainWindow::UpdateActionsUpdates()
{
  ui->actionCheckUpdates->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode);
}

bool CkeckUpdatesWorker::Run()
{
  bool result = false;
  try
  {
    status = Status::Checking;
    unique_ptr<PackageInstaller> packageInstaller = packageManager->CreateInstaller();
    packageInstaller->FindUpdates();
    updates = packageInstaller->GetUpdates();
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

void MainWindow::CheckUpdates()
{
  QThread* thread = new QThread;
  CkeckUpdatesWorker* worker = new CkeckUpdatesWorker(packageManager);
  backgroundWorkers++;
  ui->labelBackgroundTask->setText(tr("Checking for updates..."));
  worker->moveToThread(thread);
  connect(thread, SIGNAL(started()), worker, SLOT(Process()));
  connect(worker, &CkeckUpdatesWorker::OnFinish, this, [this]() {
    CkeckUpdatesWorker* worker = (CkeckUpdatesWorker*)sender();
    if (worker->GetResult())
    {
      vector<PackageInstaller::UpdateInfo> updates;
      for (const PackageInstaller::UpdateInfo& u : worker->GetUpdates())
      {
        if (u.action == PackageInstaller::UpdateInfo::ForceUpdate
          || u.action == PackageInstaller::UpdateInfo::ForceRemove
          || u.action == PackageInstaller::UpdateInfo::Update
          || u.action == PackageInstaller::UpdateInfo::Repair
          || u.action == PackageInstaller::UpdateInfo::ReleaseStateChange)
        {
          updates.push_back(u);
        }
        else if (u.action == PackageInstaller::UpdateInfo::KeepAdmin || u.action == PackageInstaller::UpdateInfo::KeepObsolete)
        {
          // TODO: adminPackage.push_back(u);
        }
      }
      updateModel->SetData(updates);
      ui->labelUpdateStatus->setText("");
      ui->labelCheckUpdatesStatus->setText("");
    }
    else
    {
      CriticalError(tr("Something went wrong while checking for updates."), worker->GetMiKTeXException());
      ui->labelUpdateStatus->setText(tr("Error"));
      ui->labelCheckUpdatesStatus->setText("Error");
    }
    ui->labelUpdatePercent->setText("");
    ui->labelUpdateDetails->setText("");
    backgroundWorkers--;
    UpdateUi();
    UpdateActions();
    worker->deleteLater();
  });
  connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
  connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  ui->labelUpdateStatus->setText(tr("Checking..."));
  ui->labelCheckUpdatesStatus->setText(tr("Checking..."));
  ui->labelUpdatePercent->setText("");
  ui->labelUpdateDetails->setText("");
  thread->start();
  UpdateUi();
  UpdateActions();
}

bool UpdateWorker::Run()
{
  bool result = false;
  try
  {
    status = Status::Synchronize;
    packageInstaller = packageManager->CreateInstaller();
    packageInstaller->SetCallback(this);
    vector<string> toBeUpdated;
    vector<string> toBeRemoved;
    for (const PackageInstaller::UpdateInfo& update : updates)
    {
      switch (update.action)
      {
      case PackageInstaller::UpdateInfo::Repair:
      case PackageInstaller::UpdateInfo::ReleaseStateChange:
      case PackageInstaller::UpdateInfo::Update:
      case PackageInstaller::UpdateInfo::ForceUpdate:
        toBeUpdated.push_back(update.deploymentName);
        break;
      case PackageInstaller::UpdateInfo::ForceRemove:
        toBeRemoved.push_back(update.deploymentName);
        break;
      }
    }
    packageInstaller->SetFileLists(toBeUpdated, toBeRemoved);
    packageInstaller->InstallRemove();
    packageInstaller = nullptr;
    unique_ptr<SetupService> service = SetupService::Create();
    SetupOptions options = service->GetOptions();
    options.Task = SetupTask::FinishUpdate;
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

void MainWindow::Update()
{
  QThread* thread = new QThread;
  UpdateWorker* worker = new UpdateWorker(packageManager, updateModel->GetData());
  backgroundWorkers++;
  ui->labelBackgroundTask->setText(tr("Installing package updates..."));
  worker->moveToThread(thread);
  connect(thread, SIGNAL(started()), worker, SLOT(Process()));
  connect(worker, &UpdateWorker::OnFinish, this, [this]() {
    UpdateWorker* worker = (UpdateWorker*)sender();
    if (worker->GetResult())
    {
      ui->labelUpdateStatus->setText(tr("Done"));
    }
    else
    {
      CriticalError(tr("Something went wrong while installing package updates."), worker->GetMiKTeXException());
      ui->labelUpdateStatus->setText(tr("Error"));
    }
    ui->labelUpdatePercent->setText("");
    ui->labelUpdateDetails->setText("");
    backgroundWorkers--;
    updateModel->SetData({});
    UpdateUi();
    UpdateActions();
    worker->deleteLater();
  });
  connect(worker, &UpdateWorker::OnUpdateProgress, this, [this]() {
    UpdateWorker* worker = (UpdateWorker*)sender();
    PackageInstaller::ProgressInfo progressInfo = worker->GetProgressInfo();
    UpdateWorker::Status status = ((UpdateWorker*)sender())->GetStatus();
    if (progressInfo.cbDownloadTotal > 0)
    {
      int percent = static_cast<int>(Divide(progressInfo.cbDownloadCompleted, progressInfo.cbDownloadTotal) * 100);
      ui->labelUpdatePercent->setText(percent == 100 ? tr("we're almost done") : QString("%1%").arg(percent));
    }
    if (status == UpdateWorker::Status::Synchronize)
    {
      ui->labelUpdateDetails->setText(tr("(updating package database)"));
    }
    else if (status == UpdateWorker::Status::Download)
    {
      ui->labelUpdateDetails->setText(tr("(downloading: %1)").arg(QString::fromUtf8(progressInfo.deploymentName.c_str())));
    }
    else if (status == UpdateWorker::Status::Install)
    {
      ui->labelUpdateDetails->setText(tr("(installing: %1)").arg(QString::fromUtf8(progressInfo.deploymentName.c_str())));
    }
    else
    {
      ui->labelUpdateDetails->setText("");
    }
  });
  connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
  connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  ui->labelUpdateStatus->setText(tr("Update in progress..."));
  ui->labelUpdatePercent->setText("0%");
  ui->labelUpdateDetails->setText(tr("(initializing)"));
  thread->start();
  UpdateUi();
  UpdateActions();
}

void MainWindow::SetupUiPackageInstallation()
{
  connect(ui->comboRepository2, static_cast<void(QComboBox::*)(int)> (&QComboBox::activated), this, [this](int index)
  {
    if (index == 1)
    {
      ChangeRepository();
    }
  });
}

void MainWindow::UpdateUiPackageInstallation()
{
  ui->comboRepository2->clear();
  string repository;
  RepositoryType repositoryType(RepositoryType::Unknown);
  if (packageManager->TryGetDefaultPackageRepository(repositoryType, repository))
  {
    ui->comboRepository2->addItem(QString::fromUtf8(repository.c_str()));
  }
  else
  {
    ui->comboRepository2->addItem(tr("a random package repository on the Internet"));
  }
  ui->comboRepository2->addItem(tr("Change..."));
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
  ui->comboRepository2->setEnabled(!IsBackgroundWorkerActive());
  ui->radioAutoInstallAsk->setEnabled(!IsBackgroundWorkerActive());
  ui->radioAutoInstallYes->setEnabled(!IsBackgroundWorkerActive());
  ui->radioAutoInstallNo->setEnabled(!IsBackgroundWorkerActive());
}

void MainWindow::ChangeRepository()
{
  try
  {
    SiteWizSheet::DoModal(this);
    UpdateUi();
    UpdateActions();
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

void MainWindow::UpdateUiPaper()
{
  ui->comboPaper->setEnabled(!IsBackgroundWorkerActive());
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

void MainWindow::SetupUiRootDirectories()
{
  rootDirectoryModel = new RootTableModel(this);
  toolBarRootDirectories = new QToolBar(this);
  toolBarRootDirectories->setIconSize(QSize(16, 16));
  toolBarRootDirectories->addAction(ui->actionAddRootDirectory);
  toolBarRootDirectories->addAction(ui->actionRemoveRootDirectory);
  toolBarRootDirectories->addSeparator();
  toolBarRootDirectories->addAction(ui->actionRootDirectoryMoveUp);
  toolBarRootDirectories->addAction(ui->actionRootDirectoryMoveDown);
  toolBarRootDirectories->addSeparator();
  toolBarRootDirectories->addAction(ui->actionRootDirectoryOpen);
  ui->vboxTreeViewRootDirectories->insertWidget(0, toolBarRootDirectories);
  ui->treeViewRootDirectories->setModel(rootDirectoryModel);
  contextMenuRootDirectoriesBackground = new QMenu(ui->treeViewRootDirectories);
  contextMenuRootDirectoriesBackground->addAction(ui->actionAddRootDirectory);
  contextMenuRootDirectory = new QMenu(ui->treeViewRootDirectories);
  contextMenuRootDirectory->addAction(ui->actionRootDirectoryMoveUp);
  contextMenuRootDirectory->addAction(ui->actionRootDirectoryMoveDown);
  contextMenuRootDirectory->addAction(ui->actionRootDirectoryOpen);
  contextMenuRootDirectory->addAction(ui->actionRemoveRootDirectory);
  contextMenuRootDirectory->addSeparator();
  contextMenuRootDirectory->addAction(ui->actionAddRootDirectory);
  ui->treeViewRootDirectories->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->treeViewRootDirectories, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnContextMenuRootDirectories(const QPoint&)));
  connect(ui->actionRootDirectoryMoveUp, SIGNAL(triggered()), this, SLOT(MoveRootDirectoryUp()));
  connect(ui->actionRootDirectoryMoveDown, SIGNAL(triggered()), this, SLOT(MoveRootDirectoryDown()));
  connect(ui->actionRootDirectoryOpen, SIGNAL(triggered()), this, SLOT(OpenRootDirectory()));
  connect(ui->actionRemoveRootDirectory, SIGNAL(triggered()), this, SLOT(RemoveRootDirectory()));
  connect(ui->actionAddRootDirectory, SIGNAL(triggered()), this, SLOT(AddRootDirectory()));
  connect(ui->treeViewRootDirectories->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    this,
    SLOT(UpdateActionsRootDirectories()));
}

void MainWindow::UpdateUiRootDirectories()
{
  if (!IsBackgroundWorkerActive())
  {
    rootDirectoryModel->Reload();
    ui->treeViewRootDirectories->resizeColumnToContents(0);
  }
}

void MainWindow::UpdateActionsRootDirectories()
{
  int selectedCount = ui->treeViewRootDirectories->selectionModel()->selectedRows().count();
  bool enableUp = selectedCount == 1;
  bool enableDown = selectedCount == 1;
  bool enableRemove = selectedCount > 0;
  for (const QModelIndex& index : ui->treeViewRootDirectories->selectionModel()->selectedRows())
  {
    enableUp = enableUp && rootDirectoryModel->CanMoveUp(index);
    enableDown = enableDown && rootDirectoryModel->CanMoveDown(index);
    enableRemove = enableRemove && rootDirectoryModel->CanRemove(index);
  }
  ui->actionRootDirectoryMoveUp->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && enableUp);
  ui->actionRootDirectoryMoveDown->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && enableDown);
  ui->actionRootDirectoryOpen->setEnabled(selectedCount > 0);
  ui->actionRemoveRootDirectory->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && enableRemove);
  ui->actionAddRootDirectory->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode);
}

const char* tdsDirs[] = {
  "bibtex",
  "dvipdfm",
  "dvips",
  "fontname",
  "fonts",
  "makeindex",
  "metafont",
  "metapost",
  "mft",
  "miktex",
  "pdftex",
  "psutils",
  "scripts",
  "tex",
  "tpm",
  "ttf2pfb",
  "ttf2tfm",
};

bool CheckRoot(const PathName& root)
{
  unique_ptr<DirectoryLister> lister = DirectoryLister::Open(root);
  DirectoryEntry entry;
  bool isEmpty = true;
  while (lister->GetNext(entry))
  {
    isEmpty = false;
    if (entry.isDirectory)
    {
      PathName name(entry.name);
      for (const char* dir : tdsDirs)
      {
        if (name == dir)
        {
          return true;
        }
      }
    }
  }
  return isEmpty;
}

void MainWindow::AddRootDirectory()
{
  try
  {
    QString directory = QFileDialog::getExistingDirectory(this);
    if (directory.isNull())
    {
      return;
    }
    PathName root = directory.toUtf8().constData();
    if (!CheckRoot(root))
    {
      if (QMessageBox::question(this, tr("MiKTeX Console"), tr("This does not look like a <a href=\"https://miktex.org/kb/tds\">TDS-compliant</a> root directory. Are you sure you want to add it?"))
        != QMessageBox::Yes)
      {
        return;
      }
    }
    session->RegisterRootDirectory(root);
    UpdateUi();
    UpdateActions();
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

void MainWindow::RemoveRootDirectory()
{
  try
  {
    for (const QModelIndex& index : ui->treeViewRootDirectories->selectionModel()->selectedRows())
    {
      rootDirectoryModel->Remove(index);
    }
    UpdateUi();
    UpdateActions();
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

void MainWindow::MoveRootDirectoryUp()
{
  try
  {
    for (const QModelIndex& index : ui->treeViewRootDirectories->selectionModel()->selectedRows())
    {
      rootDirectoryModel->MoveUp(index);
    }
    UpdateUi();
    UpdateActions();
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

void MainWindow::MoveRootDirectoryDown()
{
  try
  {
    for (const QModelIndex& index : ui->treeViewRootDirectories->selectionModel()->selectedRows())
    {
      rootDirectoryModel->MoveDown(index);
    }
    UpdateUi();
    UpdateActions();
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

void MainWindow::OpenRootDirectory()
{
  for (const QModelIndex& index : ui->treeViewRootDirectories->selectionModel()->selectedRows())
  {
    OpenDirectoryInFileBrowser(rootDirectoryModel->data(index, Qt::DisplayRole).toString());
  }
}

void MainWindow::OnContextMenuRootDirectories(const QPoint& pos)
{
  QModelIndex index = ui->treeViewRootDirectories->indexAt(pos);
  if (index.isValid())
  {
    contextMenuRootDirectory->exec(ui->treeViewRootDirectories->mapToGlobal(pos));
  }
  else
  {
    contextMenuRootDirectoriesBackground->exec(ui->treeViewRootDirectories->mapToGlobal(pos));
  }
}

void MainWindow::SetupUiPackages()
{
  toolBarPackages = new QToolBar(this);
  toolBarPackages->setIconSize(QSize(16, 16));
  toolBarPackages->addAction(ui->actionInstallPackage);
  toolBarPackages->addAction(ui->actionUninstallPackage);
  toolBarPackages->addSeparator();
  toolBarPackages->addAction(ui->actionPackageProperties);
  toolBarPackages->addSeparator();
  toolBarPackages->addAction(ui->actionUpdatePackageDatabase);
  toolBarPackages->addSeparator();
  lineEditPackageFilter = new QLineEdit(toolBarPackages);
  lineEditPackageFilter->setClearButtonEnabled(true);
  toolBarPackages->addWidget(lineEditPackageFilter);
  toolBarPackages->addAction(ui->actionFilterPackages);
  connect(lineEditPackageFilter, SIGNAL(returnPressed()), this, SLOT(FilterPackages()));
  ui->hboxPackageToolBar->addWidget(toolBarPackages);
  ui->hboxPackageToolBar->addStretch();
  packageModel = new PackageTableModel(packageManager, this);
  packageProxyModel = new PackageProxyModel(this);
  packageProxyModel->setSourceModel(packageModel);
  packageProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
  ui->treeViewPackages->setModel(packageProxyModel);
  ui->treeViewPackages->sortByColumn(0, Qt::AscendingOrder);
  contextMenuPackagesBackground = new QMenu(ui->treeViewPackages);
  contextMenuPackagesBackground->addAction(ui->actionUpdatePackageDatabase);
  contextMenuPackage = new QMenu(ui->treeViewPackages);
  contextMenuPackage->addAction(ui->actionInstallPackage);
  contextMenuPackage->addAction(ui->actionUninstallPackage);
  contextMenuPackage->addSeparator();
  contextMenuPackage->addAction(ui->actionUpdatePackageDatabase);
  contextMenuPackage->addSeparator();
  contextMenuPackage->addAction(ui->actionPackageProperties);
  ui->treeViewPackages->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->comboRepository, static_cast<void(QComboBox::*)(int)> (&QComboBox::activated), this, [this](int index)
  {
    if (index == 1)
    {
      ChangeRepository();
    }
  });
  connect(ui->treeViewPackages, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnContextMenuPackages(const QPoint&)));
  connect(ui->treeViewPackages->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    this,
    SLOT(UpdateActionsPackages()));
  connect(ui->actionPackageProperties,
    SIGNAL(triggered()),
    this,
    SLOT(PackagePropertyDialog()));
  connect(ui->actionInstallPackage,
    SIGNAL(triggered()),
    this,
    SLOT(InstallPackage()));
  connect(ui->actionUninstallPackage,
    SIGNAL(triggered()),
    this,
    SLOT(UninstallPackage()));
  connect(ui->actionUpdatePackageDatabase,
    SIGNAL(triggered()),
    this,
    SLOT(UpdatePackageDatabase()));
  connect(ui->actionFilterPackages,
    SIGNAL(triggered()),
    this,
    SLOT(FilterPackages()));
}

void MainWindow::UpdateUiPackages()
{
  ui->comboRepository->clear();
  RepositoryType repositoryType(RepositoryType::Unknown);
  string repository;
  if (packageManager->TryGetDefaultPackageRepository(repositoryType, repository))
  {
    ui->comboRepository->addItem(QString::fromUtf8(repository.c_str()));
  }
  else
  {
    ui->comboRepository->addItem(tr("a random package repository on the Internet"));
  }
  ui->comboRepository->addItem(tr("Change..."));
  ui->lineEditInstallRoot->setText(QString::fromUtf8(session->GetSpecialPath(SpecialPath::InstallRoot).GetData()));
  ui->comboRepository->setEnabled(!IsBackgroundWorkerActive());
}

void MainWindow::UpdateActionsPackages()
{
  try
  {
    ui->actionUpdatePackageDatabase->setEnabled(!IsBackgroundWorkerActive());
    ui->actionFilterPackages->setEnabled(!IsBackgroundWorkerActive());
    QModelIndexList selectedRows = ui->treeViewPackages->selectionModel()->selectedRows();
    ui->actionPackageProperties->setEnabled(!IsBackgroundWorkerActive() && selectedRows.count() == 1);
    bool enableInstall = (selectedRows.count() > 0);
    bool enableUninstall = (selectedRows.count() > 0);
    if (session->IsMiKTeXDirect())
    {
      enableInstall = false;
      enableUninstall = false;
    }
    for (QModelIndexList::const_iterator it = selectedRows.begin(); it != selectedRows.end() && (enableInstall || enableUninstall); ++it)
    {
      PackageInfo packageInfo;
      if (!packageModel->TryGetPackageInfo(packageProxyModel->mapToSource(*it), packageInfo))
      {
        MIKTEX_UNEXPECTED();
      }
      if (packageInfo.timeInstalled > 0)
      {
        enableInstall = false;
        enableUninstall = (enableUninstall && packageInfo.isRemovable);
      }
      else
      {
        enableUninstall = false;
      }
    }
    ui->actionInstallPackage->setEnabled(!IsBackgroundWorkerActive() && enableInstall);
    ui->actionUninstallPackage->setEnabled(!IsBackgroundWorkerActive() && enableUninstall);
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

void MainWindow::PackagePropertyDialog()
{
  try
  {
    for (const QModelIndex& ind : ui->treeViewPackages->selectionModel()->selectedRows())
    {
      PackageInfo packageInfo;
      if (!packageModel->TryGetPackageInfo(packageProxyModel->mapToSource(ind), packageInfo))
      {
        MIKTEX_UNEXPECTED();
      }
      PackageInfoDialog::DoModal(this, packageInfo);
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

void MainWindow::InstallPackage()
{
  try
  {
    vector<string> toBeInstalled;
    vector<string> toBeRemoved;
    for (const QModelIndex& ind : ui->treeViewPackages->selectionModel()->selectedRows())
    {
      PackageInfo packageInfo;
      if (!packageModel->TryGetPackageInfo(packageProxyModel->mapToSource(ind), packageInfo))
      {
        MIKTEX_UNEXPECTED();
      }
      else if (packageInfo.timeInstalled == 0)
      {
        toBeInstalled.push_back(packageInfo.deploymentName);
      }
      else
      {
        toBeRemoved.push_back(packageInfo.deploymentName);
      }
    }
    QString message =
      tr("Your MiKTeX installation will now be updated:\n\n")
      + tr("%n package(s) will be installed\n", "", toBeInstalled.size())
      + tr("%n package(s) will be removed", "", toBeRemoved.size());
    if (QMessageBox::Ok != QMessageBox::information(this, tr("MiKTeX Console"), message, QMessageBox::Ok | QMessageBox::Cancel))
    {
      return;
    }
    int ret = UpdateDialog::DoModal(this, packageManager, toBeInstalled, toBeRemoved);
    if (ret == QDialog::Accepted)
    {
      packageModel->Reload();
      ui->treeViewPackages->update();
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

void MainWindow::UninstallPackage()
{
  InstallPackage();
}

void MainWindow::FilterPackages()
{
  packageProxyModel->SetFilter(lineEditPackageFilter->text().toUtf8().constData());
}

bool UpdateDbWorker::Run()
{
  bool result = false;
  try
  {
    unique_ptr<PackageInstaller> installer = packageManager->CreateInstaller();
    installer->UpdateDb();
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

void MainWindow::UpdatePackageDatabase()
{
  QThread* thread = new QThread;
  UpdateDbWorker* worker = new UpdateDbWorker(packageManager);
  backgroundWorkers++;
  ui->labelBackgroundTask->setText(tr("Updating the package database..."));
  worker->moveToThread(thread);
  connect(thread, SIGNAL(started()), worker, SLOT(Process()));
  connect(worker, &UpdateDbWorker::OnFinish, this, [this]() {
    UpdateDbWorker* worker = (UpdateDbWorker*)sender();
    if (!worker->GetResult())
    {
      CriticalError(tr("Something went wrong while updating the package database."), worker->GetMiKTeXException());
    }
    packageModel->Reload();
    ui->treeViewPackages->update();
    backgroundWorkers--;
    UpdateUi();
    UpdateActions();
    worker->deleteLater();
  });
  connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
  connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  thread->start();
  UpdateUi();
  UpdateActions();
}

void MainWindow::OnContextMenuPackages(const QPoint& pos)
{
  QModelIndex index = ui->treeViewPackages->indexAt(pos);
  if (index.isValid())
  {
    contextMenuPackage->exec(ui->treeViewPackages->mapToGlobal(pos));
  }
  else
  {
    contextMenuPackagesBackground->exec(ui->treeViewPackages->mapToGlobal(pos));
  }
}

void MainWindow::SetupUiTroubleshoot()
{

}

void MainWindow::UpdateUiTroubleshoot()
{

}

void MainWindow::UpdateActionsTroubleshoot()
{
  PathName logDir = session->GetSpecialPath(SpecialPath::LogDirectory);
  ui->lineEditLogFiles->setText(QString::fromUtf8(logDir.GetData()));
}

void MainWindow::on_pushButtonShowLogDirectory_clicked()
{
  OpenDirectoryInFileBrowser(session->GetSpecialPath(SpecialPath::LogDirectory));
}
