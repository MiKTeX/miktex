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

#include "FormatDefinitionDialog.h"
#include "FormatTableModel.h"
#include "LanguageTableModel.h"
#include "PackageProxyModel.h"
#include "PackageTableModel.h"
#include "RootTableModel.h"
#include "UpdateTableModel.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "console-version.h"

#include <miktex/Core/Cfg>
#include <miktex/Core/ConfigNames>
#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>
#include <miktex/Core/FileStream>
#include <miktex/Core/Fndb.h>
#include <miktex/Core/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Registry>
#include <miktex/Core/Session>
#include <miktex/Core/StreamWriter>
#include <miktex/Core/TemporaryFile>
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

MainWindow::MainWindow(QWidget* parent, MainWindow::Pages startPage) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  if (IsUserModeBlocked())
  {
    startPage = Pages::Overview;
  }

  ui->setupUi(this);

  resize(800, 600);
  ReadSettings();

  SetupUiDirectories();
  SetupUiFormats();
  SetupUiLanguages();
  SetupUiUpdates();
  SetupUiPackageInstallation();
  SetupUiPackages();
  SetupUiDiagnose();
  SetupUiCleanup();

  time_t lastAdminMaintenance = static_cast<time_t>(std::stoll(session->GetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_LAST_ADMIN_MAINTENANCE, "0").GetString()));
  time_t lastUserMaintenance = static_cast<time_t>(std::stoll(session->GetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_LAST_USER_MAINTENANCE, "0").GetString()));
  isSetupMode = lastAdminMaintenance == 0 && lastUserMaintenance == 0 && !session->IsMiKTeXPortable();

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

  SetCurrentPage(isSetupMode ? Pages::Setup : startPage);
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
  WriteSettings();
  event->accept();
}

void MainWindow::setVisible(bool visible)
{
  ui->actionHide->setEnabled(visible);
  ui->actionRestore->setEnabled(!visible);
  QMainWindow::setVisible(visible);
}

void MainWindow::CriticalError(const QString& text, const MiKTeXException& e)
{
  if (this->isHidden())
  {
    ShowTrayMessage(TrayMessageContext::Error, text);
  }
  else
  {
    if (QMessageBox::critical(this, tr("MiKTeX Console"), text + "\n\n" + tr("Do you want to see the error details?"),
      QMessageBox::StandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No))
      == QMessageBox::StandardButton::Yes)
    {
      ErrorDialog::DoModal(this, e);
    }
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
    ui->buttonOverview->setEnabled(!isSetupMode && !IsUserModeBlocked());
    ui->buttonSettings->setEnabled(!isSetupMode && !IsUserModeBlocked());
    ui->buttonUpdates->setEnabled(!isSetupMode && !IsUserModeBlocked());
    ui->buttonPackages->setEnabled(!isSetupMode && !IsUserModeBlocked());
    ui->buttonDiagnose->setEnabled(!isSetupMode && !IsUserModeBlocked());
    ui->buttonCleanup->setEnabled(!isSetupMode && !IsUserModeBlocked());
    ui->buttonTeXworks->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && !session->IsAdminMode());
    ui->buttonTerminal->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && !IsUserModeBlocked());
    if (isSetupMode)
    {
      return;
    }
    if (!pathChecked)
    {
      if (!Utils::CheckPath())
      {
        ui->groupPathIssue->show();
      }
      else
      {
        ui->groupPathIssue->hide();
      }
      ui->bindir->setText(QString::fromUtf8(session->GetSpecialPath(SpecialPath::LocalBinDirectory).GetData()));
    }
    if (!IsBackgroundWorkerActive())
    {
      if (ui->labelUpgradeStatus->text().isEmpty())
      {
        if (packageManager->GetPackageInfo("ltxbase").IsInstalled() && packageManager->GetPackageInfo("amsfonts").IsInstalled())
        {
          ui->groupUpgrade->hide();
          ui->groupCheckUpdates->setEnabled(!IsUserModeBlocked());
        }
        else
        {
          ui->groupCheckUpdates->hide();
          ui->groupUpgrade->setEnabled(!IsUserModeBlocked());
        }
      }
    }
    UpdateUiPaper();
    UpdateUiPackageInstallation();
    UpdateUiDirectories();
    UpdateUiFormats();
    UpdateUiLanguages();
    UpdateUiUpdates();
    UpdateUiPackages();
    UpdateUiDiagnose();
    UpdateUiCleanup();
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
    ui->actionRefreshFileNameDatabase->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && !IsUserModeBlocked());
    ui->actionRefreshFontMapFiles->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && !IsUserModeBlocked());
    ui->actionTeXworks->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && !session->IsAdminMode());
    ui->actionTerminal->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && !IsUserModeBlocked());
    UpdateActionsDirectories();
    UpdateActionsFormats();
    UpdateActionsLanguages();
    UpdateActionsUpdates();
    UpdateActionsPackages();
    UpdateActionsDiagnose();
    UpdateActionsCleanup();
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
  trayIconMenu->addAction(ui->actionCheckUpdates);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(ui->actionTeXworks);
  trayIconMenu->addAction(ui->actionTerminal);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(ui->actionHide);
  trayIconMenu->addAction(ui->actionRestore);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(ui->actionExit);
  trayIcon = new QSystemTrayIcon(this);
  trayIcon->setContextMenu(trayIconMenu);
#if defined(MIKTEX_WINDOWS)
  trayIcon->setIcon(QIcon((":/Icons/miktex-console-16x16.png")));
#else
  trayIcon->setIcon(QIcon((":/Icons/miktex-console-32x32.png")));
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
  if (this->isHidden())
  {
    this->showNormal();
  }
  switch (trayMessageContext)
  {
  case TrayMessageContext::Updates:
    SetCurrentPage(Pages::Updates);
    break;
  }
}

void MainWindow::ShowTrayMessage(TrayMessageContext context, const QString& message)
{
  if (trayIcon != nullptr && QSystemTrayIcon::supportsMessages())
  {
    this->trayMessageContext = context;
    QString title;
    QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon::Information;
    switch (context)
    {
    default:
      title = tr("MiKTeX Console");
      break;
    case TrayMessageContext::Error:
      title = tr("MiKTeX Problem");
      icon = QSystemTrayIcon::MessageIcon::Critical;
      break;
    case TrayMessageContext::Updates:
      title = tr("MiKTeX Update");
      break;
    }
    trayIcon->showMessage(title, message, icon);
  }
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
  case Pages::Diagnose:
    ui->buttonDiagnose->setChecked(true);
    break;
  case Pages::Cleanup:
    ui->buttonCleanup->setChecked(true);
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
  message = tr("<p>MiKTeX Console ");
  message += MIKTEX_COMPONENT_VERSION_STR;
  message += "</p>";
  message += tr("<p>MiKTeX Console is free software. You are welcome to redistribute it under certain conditions.</p>");
  message += tr("<p>MiKTeX Console comes WITH ABSOLUTELY NO WARRANTY OF ANY KIND.</p>");
  message += tr("<p>You can support the project by giving back: <a href=\"https://miktex.org/giveback\">https://miktex.org/giveback</a><br>Thank you!</p>");
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
  PathName me = session->GetMyProgramFile(true);
#if defined(MIKTEX_WINDOWS)
  PathName meAdmin(me);
  meAdmin.RemoveFileSpec();
  meAdmin /= me.GetFileNameWithoutExtension();
  meAdmin += MIKTEX_ADMIN_SUFFIX;
  meAdmin.SetExtension(me.GetExtension());
  CharBuffer<wchar_t> file(meAdmin.GetData());
  CharBuffer<wchar_t> parameters(StringUtil::Flatten(args, ' '));
  SHELLEXECUTEINFOW sei = SHELLEXECUTEINFOW();
  sei.cbSize = sizeof(sei);
  sei.lpFile = file.GetData();
  sei.lpParameters = parameters.GetData();
  sei.nShow = SW_NORMAL;
  session->UnloadFilenameDatabase();
  if (!ShellExecuteExW(&sei))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("ShellExecuteExW");
  }
#elif defined(MIKTEX_MACOS_BUNDLE)
  PathName console = session->GetMyLocation(true) / ".." / "Resources" / MIKTEX_CONSOLE_ADMIN_EXE;
  vector<string> consoleArgs{ MIKTEX_CONSOLE_ADMIN_EXE };
  consoleArgs.insert(consoleArgs.end(), args.begin(), args.end());
  Process::Start(console, consoleArgs);
#else
  PathName frontend;
  vector<string> frontendArgs;
  if (session->FindFile("kdesu", FileType::EXE, frontend))
  {
    frontendArgs = {
      "kdesu",
      "-c",
      // TODO: quote me
      me.ToString() + " "s + StringUtil::Flatten(args, ' '),
      "-i", "miktex-console"
    };
  }
  else if (session->FindFile("gksu", FileType::EXE, frontend))
  {
    frontendArgs = {
      "gksu",
      "-D", "MiKTeX Console",
      // TODO: quote me
      me.ToString() + " "s + StringUtil::Flatten(args, ' ')
    };
  }
  else
  {
    MIKTEX_FATAL_ERROR(tr("No graphical sudo frontend is available. Please install 'kdesu' (KDE) or 'gksu' (Gnome).").toStdString());
  }
  Process::Start(frontend, frontendArgs);
#endif
  this->close();
}

void MainWindow::OkayUserMode()
{
  okayUserMode = true;
  UpdateUi();
  UpdateActions();
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
      session->UnloadFilenameDatabase();
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
    session->UnloadFilenameDatabase();
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
    packageManager->CreateMpmFndb();
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
  RefreshFndbWorker* worker = new RefreshFndbWorker(packageManager);
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
    session->UnloadFilenameDatabase();
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

void BackgroundWorker::RunIniTeXMF(const std::vector<std::string>& args)
{
  shared_ptr<Session> session = Session::Get();
  PathName initexmf;
  if (!session->FindFile(MIKTEX_INITEXMF_EXE, FileType::EXE, initexmf))
  {
    MIKTEX_FATAL_ERROR(tr("The MiKTeX configuration utility executable (initexmf) could not be found.").toStdString());
  }
  vector<string> allArgs{
    initexmf.GetFileNameWithoutExtension().ToString(),
  };
  if (session->IsAdminMode())
  {
    allArgs.push_back("--admin");
  }
  allArgs.insert(allArgs.end(), args.begin(), args.end());
  ProcessOutput<4096> output;
  int exitCode;
  session->UnloadFilenameDatabase();
  Process::Run(initexmf, allArgs, &output, &exitCode, nullptr);
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
}

bool RefreshFontMapsWorker::Run()
{
  bool result = false;
  try
  {
    RunIniTeXMF({ "--mkmaps" });
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
    session->UnloadFilenameDatabase();
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
    }
    ui->buttonUpdates->setText(tr("Updates (%1)").arg(n));
  });
  ui->treeViewUpdates->setModel(updateModel);
  connect(ui->treeViewUpdates->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    this,
    SLOT(UpdateActionsUpdates()));
  ui->labelUpdatesAvailable->hide();
  ui->labelNoUpdatesAvailable->hide();
  toolBarUpdates = new QToolBar(this);
  toolBarUpdates->setIconSize(QSize(16, 16));
  // TODO
  ui->vboxTreeViewUpdates->insertWidget(0, toolBarUpdates);
  contextMenuUpdate = new QMenu(ui->treeViewUpdates);
  // TODO
  ui->treeViewUpdates->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->treeViewUpdates, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnContextMenuUpdates(const QPoint&)));
  connect(ui->treeViewUpdates->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    this,
    SLOT(UpdateActionsUpdates()));
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
  ui->actionCheckUpdates->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && !IsUserModeBlocked());
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
#if 1
  string url;
  RepositoryType repositoryType(RepositoryType::Unknown);
  if (PackageManager::TryGetDefaultPackageRepository(repositoryType, url)
    && repositoryType == RepositoryType::Remote
    && !ProxyAuthenticationDialog(this))
  {
    return;
  }
#endif
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
#if !defined(QT_NO_SYSTEMTRAYICON)
      if (this->isHidden())
      {
        QString msg;
        if (updates.empty())
        {
          msg = tr("There are currently no updates available.");
        }
        else
        {
          if (updates.size() == 1)
          {
            msg = tr("There is an update available!");
          }
          else
          {
            msg = tr("There is are %1 updates available!").arg(updates.size());
          }
        }
        ShowTrayMessage(TrayMessageContext::Updates, msg);
      }
#endif
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
    session->UnloadFilenameDatabase();
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
    session->UnloadFilenameDatabase();
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

void MainWindow::OnContextMenuUpdates(const QPoint& pos)
{
  QModelIndex index = ui->treeViewUpdates->indexAt(pos);
  if (index.isValid())
  {
    contextMenuUpdate->exec(ui->treeViewUpdates->mapToGlobal(pos));
  }
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

void MainWindow::SetupUiDirectories()
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
    SLOT(UpdateActionsDirectories()));
}

void MainWindow::UpdateUiDirectories()
{
  if (!IsBackgroundWorkerActive())
  {
    rootDirectoryModel->Reload();
    ui->treeViewRootDirectories->resizeColumnToContents(0);
    ui->lineEditBinDir->setText(QString::fromUtf8(session->GetSpecialPath(SpecialPath::LocalBinDirectory).GetData()));
    ui->lineEditLogDir->setText(QString::fromUtf8(session->GetSpecialPath(SpecialPath::LogDirectory).GetData()));
  }
}

void MainWindow::UpdateActionsDirectories()
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

void MainWindow::SetupUiFormats()
{
  formatModel = new FormatTableModel(this);
  toolBarFormats = new QToolBar(this);
  toolBarFormats->setIconSize(QSize(16, 16));
  toolBarFormats->addAction(ui->actionAddFormat);
  toolBarFormats->addAction(ui->actionRemoveFormat);
  toolBarFormats->addSeparator();
  toolBarFormats->addAction(ui->actionBuildFormat);
  toolBarFormats->addSeparator();
  toolBarFormats->addAction(ui->actionFormatProperties);
  ui->vboxTreeViewFormats->insertWidget(0, toolBarFormats);
  ui->treeViewFormats->setModel(formatModel);
  contextMenuFormatsBackground = new QMenu(ui->treeViewFormats);
  contextMenuFormatsBackground->addAction(ui->actionAddFormat);
  contextMenuFormat = new QMenu(ui->treeViewFormats);
  contextMenuFormat->addAction(ui->actionAddFormat);
  contextMenuFormat->addAction(ui->actionRemoveFormat);
  contextMenuFormat->addSeparator();
  contextMenuFormat->addAction(ui->actionBuildFormat);
  contextMenuFormat->addSeparator();
  contextMenuFormat->addAction(ui->actionFormatProperties);
  ui->treeViewFormats->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->treeViewFormats, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnContextMenuFormats(const QPoint&)));
  connect(ui->actionAddFormat, SIGNAL(triggered()), this, SLOT(AddFormat()));
  connect(ui->actionRemoveFormat, SIGNAL(triggered()), this, SLOT(RemoveFormat()));
  connect(ui->actionFormatProperties, SIGNAL(triggered()), this, SLOT(FormatPropertyDialog()));
  connect(ui->actionBuildFormat, SIGNAL(triggered()), this, SLOT(BuildFormat()));
  connect(ui->treeViewFormats->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    this,
    SLOT(UpdateActionsFormats()));
}

void MainWindow::UpdateUiFormats()
{
  if (!IsBackgroundWorkerActive())
  {
    formatModel->Reload();
    ui->treeViewFormats->resizeColumnToContents(0);
  }
}

void MainWindow::UpdateActionsFormats()
{
  int selectedCount = ui->treeViewFormats->selectionModel()->selectedRows().count();
  bool enableRemove = selectedCount > 0;
  for (const QModelIndex& index : ui->treeViewFormats->selectionModel()->selectedRows())
  {
    enableRemove = enableRemove && formatModel->CanRemove(index);
  }
  ui->actionAddFormat->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode);
  ui->actionRemoveFormat->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && enableRemove);
  ui->actionFormatProperties->setEnabled(!IsBackgroundWorkerActive() && selectedCount == 1);
  ui->actionBuildFormat->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && selectedCount > 0);
}

void MainWindow::AddFormat()
{
  try
  {
    FormatDefinitionDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted)
    {
      session->SetFormatInfo(dlg.GetFormatInfo());
      UpdateUi();
      UpdateActions();
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

void MainWindow::RemoveFormat()
{
  try
  {
    if (QMessageBox::question(this, tr("MiKTeX Console"), tr("Are you sure you want to remove the selected format definition?")) != QMessageBox::Yes)
    {
      return;
    }
    for (const QModelIndex& ind : ui->treeViewFormats->selectionModel()->selectedRows())
    {
      session->DeleteFormatInfo(formatModel->GetFormatInfo(ind).key);
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

void MainWindow::FormatPropertyDialog()
{
  try
  {
    for (const QModelIndex& ind : ui->treeViewFormats->selectionModel()->selectedRows())
    {
      FormatDefinitionDialog dlg(this, formatModel->GetFormatInfo(ind));
      if (dlg.exec() == QDialog::Accepted)
      {
        session->SetFormatInfo(dlg.GetFormatInfo());
        UpdateUi();
        UpdateActions();
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

bool BuildFormatsWorker::Run()
{
  bool result = false;
  try
  {
    shared_ptr<Session> session = Session::Get();
    for (const string& key : formats)
    {
      FormatInfo formatInfo = session->GetFormatInfo(key);
      RunIniTeXMF({ "--dump="s + formatInfo.key });
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

void MainWindow::BuildFormat()
{
  try
  {
    vector<string> formats;
    for (const QModelIndex& ind : ui->treeViewFormats->selectionModel()->selectedRows())
    {
      formats.push_back(formatModel->GetFormatInfo(ind).key);
    }
    QThread* thread = new QThread;
    BuildFormatsWorker* worker = new BuildFormatsWorker(formats);
    backgroundWorkers++;
    ui->labelBackgroundTask->setText(formats.size() == 1 ? tr("Building format %1...").arg(QString::fromUtf8(formats[0].c_str())) : tr("Building formats..."));
    worker->moveToThread(thread);
    connect(thread, SIGNAL(started()), worker, SLOT(Process()));
    connect(worker, &BuildFormatsWorker::OnFinish, this, [this]() {
      BuildFormatsWorker* worker = (BuildFormatsWorker*)sender();
      if (!worker->GetResult())
      {
        CriticalError(tr("Something went wrong while building formats."), ((BuildFormatsWorker*)sender())->GetMiKTeXException());
      }
      backgroundWorkers--;
      session->UnloadFilenameDatabase();
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

void MainWindow::OnContextMenuFormats(const QPoint& pos)
{
  QModelIndex index = ui->treeViewFormats->indexAt(pos);
  if (index.isValid())
  {
    contextMenuFormat->exec(ui->treeViewFormats->mapToGlobal(pos));
  }
  else
  {
    contextMenuFormatsBackground->exec(ui->treeViewFormats->mapToGlobal(pos));
  }
}

void MainWindow::SetupUiLanguages()
{
  languageModel = new LanguageTableModel(this);
  ui->treeViewLanguages->setModel(languageModel);
  connect(ui->treeViewLanguages->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    this,
    SLOT(UpdateActionsLanguages()));
}

void MainWindow::UpdateUiLanguages()
{
  if (!IsBackgroundWorkerActive())
  {
    languageModel->Reload();
    ui->treeViewLanguages->resizeColumnToContents(0);
  }
}

void MainWindow::UpdateActionsLanguages()
{
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
    ui->actionUpdatePackageDatabase->setEnabled(!IsBackgroundWorkerActive() && !IsUserModeBlocked());
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
    session->UnloadFilenameDatabase();
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

void MainWindow::SetupUiDiagnose()
{

}

void MainWindow::UpdateUiDiagnose()
{

}

void MainWindow::UpdateActionsDiagnose()
{
  PathName logDir = session->GetSpecialPath(SpecialPath::LogDirectory);
  ui->lineEditLogFiles->setText(QString::fromUtf8(logDir.GetData()));
}

void MainWindow::on_pushButtonShowLogDirectory_clicked()
{
  OpenDirectoryInFileBrowser(session->GetSpecialPath(SpecialPath::LogDirectory));
}

void MainWindow::SetupUiCleanup()
{
  connect(ui->actionFactoryReset, SIGNAL(triggered()), this, SLOT(FactoryReset()));
}

bool MainWindow::IsFactoryResetPossible()
{
#if defined(MIKTEX_WINDOWS)
  return false;
#else
  return !session->IsSharedSetup() || session->IsAdminMode();
#endif
}

void MainWindow::UpdateUiCleanup()
{
  ui->buttonFactoryReset->setEnabled(IsFactoryResetPossible() && !IsBackgroundWorkerActive());
}

void MainWindow::UpdateActionsCleanup()
{
  ui->actionFactoryReset->setEnabled(IsFactoryResetPossible() && !IsBackgroundWorkerActive());
}

bool FactoryResetWorker::Run()
{
  bool result = false;
  try
  {
    shared_ptr<Session> session = Session::Get();
    unique_ptr<SetupService> service = SetupService::Create();
    SetupOptions options = service->GetOptions();
    options.Task = SetupTask::CleanUp;
    options.IsCommonSetup = session->IsAdminMode();
    options.CleanupOptions = { CleanupOption::Links, CleanupOption::LogFiles, CleanupOption::Path, CleanupOption::Registry, CleanupOption::RootDirectories };
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

void MainWindow::FactoryReset()
{
  QString message = tr("<h3>Reset the TeX installation to factory defaults</h3>");
  message += tr("<p>You are about to reset your TeX installation. All TEXMF root directories will be removed and you will loose all configuration settings, log files, data files and packages.");
  message += tr(" In other words: your TeX installation will be restored to its original state, as when it was first installed.</p>");
  message += tr("Are you sure?");
  if (QMessageBox::warning(this, tr("MiKTeX Console"), message, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
  {
    return;
  }
  try
  {
    QThread* thread = new QThread;
    FactoryResetWorker* worker = new FactoryResetWorker;
    backgroundWorkers++;
    ui->labelBackgroundTask->setText(tr("Resetting the TeX installation..."));
    worker->moveToThread(thread);
    connect(thread, SIGNAL(started()), worker, SLOT(Process()));
    connect(worker, &FactoryResetWorker::OnFinish, this, [this]() {
      FactoryResetWorker* worker = (FactoryResetWorker*)sender();
      if (worker->GetResult())
      {
        QMessageBox::information(this, tr("MiKTeX Console"), tr("The TeX installation has been restored to its initial state.\n\nThe application window will now be closed."));
      }
      else
      {
        QMessageBox::warning(this, tr("MiKTeX Console"), QString::fromUtf8(worker->GetMiKTeXException().what()) + tr("\n\nThe application window will now be closed."));
      }
      backgroundWorkers--;
      session->UnloadFilenameDatabase();
      worker->deleteLater();
      this->close();
    });
    connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    UpdateUi();
    UpdateActions();
    thread->start();
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

void MainWindow::ReadSettings()
{
  PathName consoleIni = session->GetSpecialPath(SpecialPath::ConfigRoot) / MIKTEX_PATH_MIKTEX_CONFIG_DIR / "console.ini";
  if (!File::Exists(consoleIni))
  {
    return;
  }
  unique_ptr<Cfg> settings = Cfg::Create();
  settings->Read(consoleIni);
  string s;
  if (!settings->TryGetValue("MainWindow", "geometry", s))
  {
    return;
  }
  restoreGeometry(QByteArray::fromHex(s.c_str()));
}

void MainWindow::WriteSettings()
{
  unique_ptr<Cfg> settings = Cfg::Create();
  settings->PutValue("MainWindow", "geometry", saveGeometry().toHex().constData());
  settings->Write(session->GetSpecialPath(SpecialPath::ConfigRoot) / MIKTEX_PATH_MIKTEX_CONFIG_DIR / "console.ini");
}
