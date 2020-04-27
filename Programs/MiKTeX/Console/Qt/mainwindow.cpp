/* mainwindow.cpp:

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

#include <QThread>
#include <QTimer>
#include <QtWidgets>

#include <fstream>
#include <iomanip>
#include <set>

#include <log4cxx/logger.h>

#include "FormatDefinitionDialog.h"
#include "FormatTableModel.h"
#include "LanguageTableModel.h"
#include "PackageProxyModel.h"
#include "PackageTableModel.h"
#include "RepositoryTableModel.h"
#include "RootTableModel.h"
#include "UpdateTableModel.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "console-version.h"

#include <miktex/Core/AutoResource>
#include <miktex/Core/Cfg>
#include <miktex/Core/ConfigNames>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>
#include <miktex/Core/FileStream>
#include <miktex/Core/Fndb.h>
#include <miktex/Core/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Core/Session>
#include <miktex/Core/StreamWriter>
#include <miktex/Core/TemporaryFile>
#include <miktex/Setup/SetupService>
#include <miktex/UI/Qt/ErrorDialog>
#include <miktex/UI/Qt/PackageInfoDialog>
#include <miktex/UI/Qt/SiteWizSheet>
#include <miktex/UI/Qt/UpdateDialog>
#include <miktex/Util/StringUtil>

#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Setup;
using namespace MiKTeX::Trace;
using namespace MiKTeX::UI::Qt;
using namespace MiKTeX::Util;

static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("mainwindow"));

inline double Divide(double a, double b)
{
  return a / b;
}

void OpenDirectoryInFileBrowser(const QString& path)
{
  QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void OpenDirectoryInFileBrowser(const PathName& dir)
{
  OpenDirectoryInFileBrowser(QString::fromUtf8(dir.GetData()));
}

void PackageInstallerCallbackImpl::ReportLine(const string& str)
{
  LOG4CXX_INFO(logger, str);
}

void SetupServiceCallbackImpl::ReportLine(const string& str)
{
#if defined(MIKTEX_WINDOWS)
  OutputDebugStringW(StringUtil::UTF8ToWideChar(str.c_str()).c_str());
#endif
}

MainWindow::MainWindow(QWidget* parent, MainWindow::Pages startPage, bool dontFindIssues, TraceCallback* traceCallback) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  packageManager = PackageManager::Create(PackageManager::InitInfo(traceCallback));
  time_t lastAdminMaintenance = static_cast<time_t>(std::stoll(session->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_LAST_ADMIN_MAINTENANCE, ConfigValue("0")).GetString()));
  time_t lastUserMaintenance = static_cast<time_t>(std::stoll(session->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_LAST_USER_MAINTENANCE, ConfigValue("0")).GetString()));
  isSetupMode = lastAdminMaintenance == 0 && lastUserMaintenance == 0 && !session->IsMiKTeXPortable();
  this->dontFindIssues = isSetupMode || dontFindIssues;
  okayUserMode = isSetupMode || CheckIssue(IssueType::UserUpdateCheckOverdue).first;

  if (IsUserModeBlocked())
  {
    startPage = Pages::Overview;
  }

  ui->setupUi(this);

  resize(800, 600);
  ReadSettings();

  repositoryModel = new RepositoryTableModel(this);

  SetupUiDirectories();
  SetupUiFormats();
  SetupUiLanguages();
  SetupUiUpdates();
  SetupUiPackageInstallation();
  SetupUiPackages();
  SetupUiDiagnose();
  SetupUiCleanup();

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

  (void)connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(AboutDialog()));
  (void)connect(ui->actionRestartAdmin, SIGNAL(triggered()), this, SLOT(RestartAdmin()));
  (void)connect(ui->actionRefreshFileNameDatabase, SIGNAL(triggered()), this, SLOT(RefreshFndb()));
  (void)connect(ui->actionRefreshFontMapFiles, SIGNAL(triggered()), this, SLOT(RefreshFontMaps()));
  (void)connect(ui->actionTeXworks, SIGNAL(triggered()), this, SLOT(StartTeXworks()));
  (void)connect(ui->actionTerminal, SIGNAL(triggered()), this, SLOT(StartTerminal()));

  QTimer* timer = new QTimer(this);
  (void)connect(timer, SIGNAL(timeout()), this, SLOT(UnloadFileNameDatabase()));
  timer->start(5000);

  UpdateUi();
  UpdateActions();
  
  if (!this->isSetupMode)
  {
    QTimer::singleShot(0, this, SLOT(ShowMajorIssue()));
  }
}

void MainWindow::ShowMajorIssue()
{
  if (!checkedIssues)
  {
    FindIssues();
  }
  for (const Issue& issue : issues)
  {
    if (issue.severity == IssueSeverity::Critical || issue.severity == IssueSeverity::Major)
    {
      QString text = tr("A MiKTeX setup issue has been detected.");
      text += "<p>" + QString::fromUtf8(issue.message.c_str()) + "</p>";
      QString remedy = QString::fromUtf8(issue.remedy.c_str());
      if (!remedy.isEmpty())
      {
        text += "<p>" + tr("Remedy:") + " " + remedy + "</p>";
      }
      QString url = QString::fromUtf8(issue.GetUrl().c_str());
      if (!url.isEmpty())
      {
        text += "<p>" + tr("For more information, visit <a href='%1'>%2</a>").arg(url).arg(url) + "</p>";
      }
      if (this->isHidden())
      {
        ShowTrayMessage(TrayMessageContext::Error, text);
      }
      else
      {
        QMessageBox::critical(this, tr("MiKTeX Console"), text);
      }
      return;
    }
  }
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
  if (updateModel->Pending() > 0)
  {
    if (QMessageBox::question(this, tr("MiKTeX Console"), tr("There are pending updates. Are you sure you want to quit?"))
      != QMessageBox::Yes)
    {
      event->ignore();
      return;
    }
  }
  if (saveSettingsOnClose)
  {
    WriteSettings();
  }
  event->accept();
}

void MainWindow::setVisible(bool visible)
{
  ui->actionHide->setEnabled(visible);
  ui->actionRestore->setEnabled(!visible);
  QMainWindow::setVisible(visible);
}

void MainWindow::CriticalError(const QString& shortText, const MiKTeXException& e)
{
  LOG4CXX_ERROR(logger, e);
  QString description = QString::fromUtf8(e.GetDescription().c_str());
  QString text = shortText;
  if (!description.isEmpty())
  {
    text += "<p>" + description + "</p>";
    QString remedy = QString::fromUtf8(e.GetRemedy().c_str());
    if (!remedy.isEmpty())
    {
      text += "<p>" + tr("Remedy:") + " " + remedy + "</p>";
    }
  }
  QString url = QString::fromUtf8(e.GetUrl().c_str());
  if (!url.isEmpty())
  {
    text += "<p>" + tr("For more information, visit <a href='%1'>%2</a>").arg(url).arg(url) + "</p>";
  }
  if (this->isHidden())
  {
    ShowTrayMessage(TrayMessageContext::Error, text);
  }
  else
  {
    text += "<p>" + tr("Do you want to see the error details?") + "</p>";
    if (QMessageBox::critical(this, tr("MiKTeX Console"), text,
      QMessageBox::StandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No))
      == QMessageBox::StandardButton::Yes)
    {
      ErrorDialog::DoModal(this, e);
    }
  }
}

QString GetAlertStyleSheet()
{
  QString styleSheet;
  QTextStream s;
  s.setString(&styleSheet);
  s << "QGroupBox::title{\n"
    << "  color: red;\n"
    << "  padding: 0 3px;\n"
    << "  subcontrol-origin: margin;\n"
    << "}\n"
    << "\n"
    << "QGroupBox{\n"
    << "  border: 1px solid red;\n"
    << "  margin-top: 1ex;\n"
    << "}\n";
  s.flush();
  return styleSheet;
}

void MainWindow::UpdateUi()
{
  try
  {
    ui->labelBackgroundTask->setVisible(IsBackgroundWorkerActive());
    ui->labelSetupWait->setVisible(isSetupMode && IsBackgroundWorkerActive());
    ui->adminMode->setVisible(session->IsAdminMode());
    ui->userMode->setVisible(!session->IsAdminMode() && session->IsSharedSetup() && !CheckIssue(IssueType::UserUpdateCheckOverdue).first);
    ui->privateMode->setVisible(!session->IsAdminMode() && !session->IsSharedSetup());
    if (session->IsAdminMode())
    {
      ui->buttonAdminSetup->setText(tr("Finish shared setup"));
    }
    ui->buttonOverview->setEnabled(!isSetupMode && !IsUserModeBlocked());
    ui->buttonSettings->setEnabled(!isSetupMode && !IsUserModeBlocked());
    ui->buttonUpdates->setEnabled(!isSetupMode && !IsUserModeBlocked());
    ui->buttonPackages->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && !IsUserModeBlocked());
    ui->buttonDiagnose->setEnabled(!isSetupMode && !IsUserModeBlocked());
    ui->buttonCleanup->setEnabled(!isSetupMode && !IsUserModeBlocked());
    ui->buttonTeXworks->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && !session->IsAdminMode());
    ui->buttonTerminal->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && !IsUserModeBlocked());
    if (isSetupMode)
    {
      return;
    }
    auto issue = CheckIssue(IssueType::UserUpdateCheckOverdue);
    if (issue.first)
    {
      ui->labelLastUpdateCheck->setText(QString::fromUtf8(issue.second.message.c_str()));
      ui->groupCheckUpdates->setStyleSheet(GetAlertStyleSheet());
    }
    else
    {
      ui->labelLastUpdateCheck->setText(tr("You can now check for package updates."));
      ui->groupCheckUpdates->setStyleSheet("");
    }
    ui->groupPathIssue->setStyleSheet(GetAlertStyleSheet());
#if defined(MIKTEX_WINDOWS)
    ui->buttonFixPath->setEnabled(!IsUserModeBlocked());
#else
    ui->buttonFixPath->setEnabled(false);
#endif
    ui->bindir->setText(QString::fromUtf8(session->GetSpecialPath(SpecialPath::LinkTargetDirectory).ToDisplayString().c_str()));
    ui->groupPathIssue->setVisible(CheckIssue(IssueType::Path).first);
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
    ui->actionRestartAdmin->setEnabled(!IsBackgroundWorkerActive() && session->IsSharedSetup() && !session->IsAdminMode() && !CheckIssue(IssueType::UserUpdateCheckOverdue).first);
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
  trayIconMenu->setDefaultAction(ui->actionRestore);
  trayIcon->setContextMenu(trayIconMenu);
#if defined(MIKTEX_WINDOWS)
  trayIcon->setIcon(QIcon((":/Icons/miktex-console-16x16.png")));
#else
  trayIcon->setIcon(QIcon((":/Icons/miktex-console-32x32.png")));
#endif
  trayIcon->setToolTip(tr("MiKTeX Console"));
  trayIcon->show();

  (void)connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::TrayIconActivated);
  (void)connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::TrayMessageClicked);
}

void MainWindow::TrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
  switch (reason)
  {
  case QSystemTrayIcon::ActivationReason::DoubleClick:
    this->showNormal();
    break;
  default:
    break;
  }
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
  default:
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
      try
      {
        packageModel->Reload();
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
    PathName localBinDir = session->GetSpecialPath(SpecialPath::LinkTargetDirectory);
    string newPath = localBinDir.ToString();
    string oldPath;
    bool haveOldPath = Utils::GetEnvironmentString("PATH", oldPath);
    if (haveOldPath)
    {
      newPath += PathNameUtil::PathNameDelimiter;
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
    if (updateModel->Pending() > 0)
    {
      if (QMessageBox::question(this, tr("MiKTeX Console"), tr("There are pending updates. Are you sure you want to quit?"))
        != QMessageBox::Yes)
      {
        return;
      }
    }
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
  LOG4CXX_INFO(logger, "switching to MiKTeX admin mode");
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
  CommandLineBuilder cmd;
  if (session->FindFile("pkexec", FileType::EXE, frontend))
  {
    cmd.AppendArgument(frontend);
    cmd.AppendArgument("--disable-internal-agent");
    cmd.AppendArgument(me);
    cmd.AppendArguments(args);
  }
  else if (session->FindFile("kdesu", FileType::EXE, frontend))
  {
    cmd.AppendArgument(frontend);
    cmd.AppendArgument("-c");
    cmd.AppendArgument(me.ToString() + " "s + StringUtil::Flatten(args, ' '));
    cmd.AppendArgument("-i");
    cmd.AppendArgument("miktex-console");
  }
  else if (session->FindFile("gksu", FileType::EXE, frontend))
  {
    cmd.AppendArgument(frontend);
    cmd.AppendArgument("-D");
    cmd.AppendArgument("MiKTeX Console");
    cmd.AppendArgument(me.ToString() + " "s + StringUtil::Flatten(args, ' '));
  }
  else
  {
    MIKTEX_FATAL_ERROR(tr("No graphical sudo frontend is available. Please install 'pkexec', 'kdesu' (KDE) or 'gksu' (Gnome). Alternatively, you can enter 'sudo miktex-console %1' in a terminal window.").arg(QString::fromUtf8(StringUtil::Flatten(args, ' ').c_str())).toStdString());
  }
  LOG4CXX_INFO(logger, "scheduling restart with administrator privileges: " << cmd);
  string env;
  bool isWayland = Utils::GetEnvironmentString("XDG_SESSION_TYPE", env) && env == "wayland";
  if (isWayland)
  {
    session->ScheduleSystemCommand("xhost +si:localuser:root");
  }
  session->ScheduleSystemCommand(cmd.ToString());
  if (isWayland)
  {
    session->ScheduleSystemCommand("xhost -si:localuser:root");
  }
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
    options = service->SetOptions(options);
    service->SetCallback(this);
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
  LOG4CXX_INFO(logger, "finishing MiKTeX setup");
  try
  {
    ui->buttonAdminSetup->setEnabled(false);
    ui->buttonUserSetup->setEnabled(false);
    QThread* thread = new QThread;
    FinishSetupWorker* worker = new FinishSetupWorker;
    backgroundWorkers++;
    ui->labelBackgroundTask->setText(tr("Finishing the MiKTeX setup..."));
    worker->moveToThread(thread);
    (void)connect(thread, SIGNAL(started()), worker, SLOT(Process()));
    (void)connect(worker, &FinishSetupWorker::OnFinish, this, [this]() {
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
      Restart();
    });
    (void)connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
    (void)connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
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

#if defined(MIKTEX_WINDOWS)
void MainWindow::on_buttonFixPath_clicked()
{
  LOG4CXX_INFO(logger, "fixing PATH");
  if (Utils::CheckPath(true))
  {
    QMessageBox::information(this, tr("MiKTeX Console"), tr("The PATH environment variable has been successfully modified."), QMessageBox::Ok);
    ui->groupPathIssue->hide();
    FindIssues();
  }
  else
  {
    QMessageBox::warning(this, tr("MiKTeX Console"), tr("The PATH environment variable could not be modified."), QMessageBox::Ok);
  }
}
#endif

bool UpgradeWorker::Run()
{
  bool result = false;
  try
  {
    status = Status::Synchronize;
    packageInstaller = packageManager->CreateInstaller();
    packageInstaller->SetCallback(this);
    LOG4CXX_INFO(logger, "checking for upgrades...");
    packageInstaller->FindUpgrades(PackageLevel::Basic);
    vector<PackageInstaller::UpgradeInfo> upgrades = packageInstaller->GetUpgrades();
    LOG4CXX_INFO(logger, "found " << upgrades.size() << " upgrades");
    if (!upgrades.empty())
    {
      vector<string> toBeInstalled;
      for (const PackageInstaller::UpgradeInfo& upg : upgrades)
      {
        toBeInstalled.push_back(upg.packageId);
      }
      packageInstaller->SetFileLists(toBeInstalled, vector<string>());
      LOG4CXX_INFO(logger, "installing upgrades...");
      packageInstaller->InstallRemove(PackageInstaller::Role::Installer);
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
  LOG4CXX_INFO(logger, "upgrading MiKTeX");
  ui->buttonUpgrade->setEnabled(false);
  QThread* thread = new QThread;
  UpgradeWorker* worker = new UpgradeWorker(packageManager);
  backgroundWorkers++;
  ui->labelBackgroundTask->setText(tr("Installing packages..."));
  worker->moveToThread(thread);
  (void)connect(thread, SIGNAL(started()), worker, SLOT(Process()));
  (void)connect(worker, &UpgradeWorker::OnFinish, this, [this]() {
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
  (void)connect(worker, &UpgradeWorker::OnUpgradeProgress, this, [this]() {
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
      ui->labelUpgradeDetails->setText(tr("(downloading: %1)").arg(QString::fromUtf8(progressInfo.packageId.c_str())));
    }
    else if (status == UpgradeWorker::Status::Install)
    {
      ui->labelUpgradeDetails->setText(tr("(installing: %1)").arg(QString::fromUtf8(progressInfo.packageId.c_str())));
    }
    else
    {
      ui->labelUpgradeDetails->setText("");
    }
  });
  (void)connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
  (void)connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  ui->labelUpgradeStatus->setText(tr("Upgrade in progress..."));
  ui->labelUpgradePercent->setText("0%");
  ui->labelUpgradeDetails->setText(tr("(initializing)"));
  UpdateUi();
  UpdateActions();
  thread->start();
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
  (void)connect(thread, SIGNAL(started()), worker, SLOT(Process()));
  (void)connect(worker, &RefreshFndbWorker::OnFinish, this, [this]() {
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
  (void)connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
  (void)connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  UpdateUi();
  UpdateActions();
  thread->start();
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
  (void)connect(thread, SIGNAL(started()), worker, SLOT(Process()));
  (void)connect(worker, &RefreshFontMapsWorker::OnFinish, this, [this]() {
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
  (void)connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
  (void)connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  UpdateUi();
  UpdateActions();
  thread->start();
}

void MainWindow::SetupUiUpdates()
{
  ui->comboRepository3->setModel(repositoryModel);
  (void)connect(ui->comboRepository3, SIGNAL(currentIndexChanged(int)), this, SLOT(OnRepositorySelected(int)));
  (void)connect(ui->actionCheckUpdates, SIGNAL(triggered()), this, SLOT(CheckUpdates()));
  updateModel = new UpdateTableModel(packageManager, this);
  string lastUpdateCheck;
  if (session->TryGetConfigValue(
    MIKTEX_CONFIG_SECTION_MPM,
    session->IsAdminMode() ? MIKTEX_CONFIG_VALUE_LAST_ADMIN_UPDATE_CHECK : MIKTEX_CONFIG_VALUE_LAST_USER_UPDATE_CHECK,
    lastUpdateCheck))
  {
    ui->labelUpdateSummary->setText(tr("Last checked: %1").arg(QDateTime::fromTime_t(std::stoi(lastUpdateCheck)).date().toString()));
  }
  else
  {
    ui->labelUpdateSummary->setText(tr("You have not yet checked for updates."));
  }
  (void)connect(updateModel, &UpdateTableModel::modelReset, this, [this]() {
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
  (void)connect(ui->treeViewUpdates->selectionModel(),
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
  (void)connect(ui->treeViewUpdates, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnContextMenuUpdates(const QPoint&)));
  (void)connect(ui->treeViewUpdates->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    this,
    SLOT(UpdateActionsUpdates()));
  (void)connect(updateModel,
    SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)),
    this,
    SLOT(UpdateUiUpdates()));
}

void MainWindow::UpdateUiUpdates()
{
  ui->lineEditInstallRoot2->setText(QString::fromUtf8(session->GetSpecialPath(SpecialPath::InstallRoot).ToDisplayString().c_str()));
  ui->comboRepository3->setEnabled(!IsBackgroundWorkerActive());
  ui->comboRepository3->blockSignals(true);
  ui->comboRepository3->setCurrentIndex(repositoryModel->GetDefaultIndex());
  ui->comboRepository3->blockSignals(false);
  ui->buttonCheckUpdates->setEnabled(!IsBackgroundWorkerActive());
  ui->buttonUpdateCheck->setEnabled(!IsBackgroundWorkerActive());
  ui->buttonUpdateNow->setEnabled(!IsBackgroundWorkerActive() && !updateModel->GetCheckedPackages().empty());
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
    packageInstaller->SetCallback(this);
    LOG4CXX_INFO(logger, "checking for updates...");
    packageInstaller->FindUpdates();
    updates = packageInstaller->GetUpdates();
    LOG4CXX_INFO(logger, "found " << updates.size() << " updates");
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
  (void)connect(thread, SIGNAL(started()), worker, SLOT(Process()));
  (void)connect(worker, &CkeckUpdatesWorker::OnFinish, this, [this]() {
    CkeckUpdatesWorker* worker = (CkeckUpdatesWorker*)sender();
    if (worker->GetResult())
    {
      vector<PackageInstaller::UpdateInfo> updates = worker->GetUpdates();
      updateModel->SetData(updates);
      ui->labelUpdateStatus->setText("");
      ui->labelCheckUpdatesStatus->setText("");
      FindIssues();
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
            msg = tr("There are %1 updates available!").arg(updates.size());
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
    UpdateUi();
    UpdateActions();
    worker->deleteLater();
  });
  (void)connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
  (void)connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  ui->labelUpdateStatus->setText(tr("Checking..."));
  ui->labelCheckUpdatesStatus->setText(tr("Checking..."));
  ui->labelUpdatePercent->setText("");
  ui->labelUpdateDetails->setText("");
  UpdateUi();
  UpdateActions();
  thread->start();
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
        toBeUpdated.push_back(update.packageId);
        break;
      case PackageInstaller::UpdateInfo::ForceRemove:
        toBeRemoved.push_back(update.packageId);
        break;
      default:
        break;
      }
    }
    packageInstaller->SetFileLists(toBeUpdated, toBeRemoved);
    LOG4CXX_INFO(logger, "installing updates...");
    packageInstaller->InstallRemove(PackageInstaller::Role::Updater);
    packageInstaller = nullptr;
    unique_ptr<SetupService> service = SetupService::Create();
    SetupOptions options = service->GetOptions();
    options.Task = SetupTask::FinishUpdate;
    options = service->SetOptions(options);
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
  UpdateWorker* worker = new UpdateWorker(packageManager, updateModel->GetCheckedPackages());
  backgroundWorkers++;
  ui->labelBackgroundTask->setText(tr("Installing package updates..."));
  worker->moveToThread(thread);
  (void)connect(thread, SIGNAL(started()), worker, SLOT(Process()));
  (void)connect(worker, &UpdateWorker::OnFinish, this, [this]() {
    UpdateWorker* worker = (UpdateWorker*)sender();
    bool restart = false;
    if (worker->GetResult())
    {
      FindIssues();
      ui->labelUpdateStatus->setText(tr("Done"));
      for (const auto& u : updateModel->GetCheckedPackages())
      {
        // FIXME: logic must be in UpdateInfo
        if (strncmp(u.packageId.c_str(), "miktex-", 7) == 0)
        {
          restart = true;
          break;
        }
      }
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
    if (restart)
    {
      Restart();
    }
  });
  (void)connect(worker, &UpdateWorker::OnUpdateProgress, this, [this]() {
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
      ui->labelUpdateDetails->setText(tr("(downloading: %1)").arg(QString::fromUtf8(progressInfo.packageId.c_str())));
    }
    else if (status == UpdateWorker::Status::Install)
    {
      ui->labelUpdateDetails->setText(tr("(installing: %1)").arg(QString::fromUtf8(progressInfo.packageId.c_str())));
    }
    else
    {
      ui->labelUpdateDetails->setText("");
    }
  });
  (void)connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
  (void)connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  ui->labelUpdateStatus->setText(tr("Update in progress..."));
  ui->labelUpdatePercent->setText("0%");
  ui->labelUpdateDetails->setText(tr("(initializing)"));
  UpdateUi();
  UpdateActions();
  thread->start();
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
  ui->comboRepository2->setModel(repositoryModel);
  (void)connect(ui->comboRepository2, SIGNAL(currentIndexChanged(int)), this, SLOT(OnRepositorySelected(int)));
}

void MainWindow::UpdateUiPackageInstallation()
{
  TriState autoInstall = session->GetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOINSTALL).GetTriState();
  TriState autoAdmin = session->GetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOADMIN).GetTriState();
  switch (autoInstall)
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
  ui->chkAllUsers->setChecked(autoAdmin == TriState::True);
  ui->comboRepository2->setEnabled(!IsBackgroundWorkerActive());
  ui->comboRepository2->blockSignals(true);
  ui->comboRepository2->setCurrentIndex(repositoryModel->GetDefaultIndex());
  ui->comboRepository2->blockSignals(false);
  ui->radioAutoInstallAsk->setEnabled(!IsBackgroundWorkerActive());
  ui->radioAutoInstallYes->setEnabled(!IsBackgroundWorkerActive());
  ui->radioAutoInstallNo->setEnabled(!IsBackgroundWorkerActive());
  ui->chkAllUsers->setEnabled(!IsBackgroundWorkerActive() && autoInstall == TriState::True && session->IsSharedSetup());
}

void MainWindow::ChangeRepository()
{
  try
  {
    SiteWizSheet::DoModal(this);
    repositoryModel->Reload();
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

void MainWindow::OnRepositorySelected(int index)
{
  try
  {
    if (index >= 0)
    {
      PackageManager::SetDefaultPackageRepository(repositoryModel->GetData(index));
    }
    UpdateUi();
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
  LOG4CXX_INFO(logger, "setting AutoInstall: ask");
  session->SetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOINSTALL, ConfigValue(static_cast<int>(TriState::Undetermined)));
  ui->chkAllUsers->setEnabled(false);
}

void MainWindow::on_radioAutoInstallYes_clicked()
{
  LOG4CXX_INFO(logger, "setting AutoInstall: yes");
  session->SetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOINSTALL, ConfigValue(static_cast<int>(TriState::True)));
  ui->chkAllUsers->setEnabled(true);
}

void MainWindow::on_radioAutoInstallNo_clicked()
{
  LOG4CXX_INFO(logger, "setting AutoInstall: no");
  session->SetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOINSTALL, ConfigValue(static_cast<int>(TriState::False)));
  ui->chkAllUsers->setEnabled(false);
}

void MainWindow::on_chkAllUsers_clicked()
{
  LOG4CXX_INFO(logger, "setting AutoAdmin: " << ui->chkAllUsers->isChecked());
  session->SetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOADMIN, ConfigValue(static_cast<int>(ui->chkAllUsers->isChecked() ? TriState::True : TriState::False)));
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
  (void)connect(ui->treeViewRootDirectories, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnContextMenuRootDirectories(const QPoint&)));
  (void)connect(ui->actionRootDirectoryMoveUp, SIGNAL(triggered()), this, SLOT(MoveRootDirectoryUp()));
  (void)connect(ui->actionRootDirectoryMoveDown, SIGNAL(triggered()), this, SLOT(MoveRootDirectoryDown()));
  (void)connect(ui->actionRootDirectoryOpen, SIGNAL(triggered()), this, SLOT(OpenRootDirectory()));
  (void)connect(ui->actionRemoveRootDirectory, SIGNAL(triggered()), this, SLOT(RemoveRootDirectory()));
  (void)connect(ui->actionAddRootDirectory, SIGNAL(triggered()), this, SLOT(AddRootDirectory()));
  (void)connect(ui->treeViewRootDirectories->selectionModel(),
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
    ui->lineEditBinDir->setText(QString::fromUtf8(session->GetSpecialPath(SpecialPath::LinkTargetDirectory).ToDisplayString().c_str()));
    ui->lineEditLogDir->setText(QString::fromUtf8(session->GetSpecialPath(SpecialPath::LogDirectory).ToDisplayString().c_str()));
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
#if defined(MIKTEX_WINDOWS)
  ui->buttonChangeLinkTargetDirectory->hide();
#else
  ui->buttonChangeLinkTargetDirectory->setEnabled(!IsBackgroundWorkerActive() && !isSetupMode && (!session->IsSharedSetup() || session->IsAdminMode()));
#endif
}

string tdsDirs[] = {
  "bibtex/bib",
  "bibtex/bst",
  "fonts/afm",
  "fonts/enc",
  "fonts/map",
  "fonts/opentype",
  "fonts/pfb",
  "fonts/source",
  "fonts/tfm",
  "fonts/type1",
  "fonts/vf",
  "tex/generic",
  "tex/latex",
  "tex/lualatex",
  "tex/luatex",
  "tex/plain",
  "tex/xelatex",
};

bool CheckRoot(const PathName& root)
{
  for (const string& dir : tdsDirs)
  {
    if (Directory::Exists(root / PathName(dir)))
    {
      return true;
    }
  }
  return false;
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
    PathName root(directory.toUtf8().constData());
    if (!CheckRoot(root))
    {
      if (QMessageBox::question(this, tr("MiKTeX Console"), tr("This does not look like a <a href=\"https://miktex.org/kb/tds\">TDS-compliant</a> root directory. Are you sure you want to add it?"))
        != QMessageBox::Yes)
      {
        return;
      }
    }
    LOG4CXX_INFO(logger, "registering TEXMF root directory: " << Q_(root));
    session->RegisterRootDirectory(root, false);
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
      LOG4CXX_INFO(logger, "unregistering TEXMF root directory: " << Q_(rootDirectoryModel->data(index, Qt::DisplayRole).toString().toUtf8().constData()));
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
      LOG4CXX_INFO(logger, "moving TEXMF root directory up: " << Q_(rootDirectoryModel->data(index, Qt::DisplayRole).toString().toUtf8().constData()));
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
      LOG4CXX_INFO(logger, "moving TEXMF root directory down: " << Q_(rootDirectoryModel->data(index, Qt::DisplayRole).toString().toUtf8().constData()));
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

#if !defined(MIKTEX_WINDOWS)
bool ChangeLinkTargetDirectoryWorker::Run()
{
  bool result = false;
  try
  {
    shared_ptr<Session> session = Session::Get();
    RunIniTeXMF({ "--remove-links" });
    if (session->IsSharedSetup())
    {
      session->SetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMONLINKTARGETDIRECTORY, ConfigValue(linkTargetDirectory.ToString()));
    }
    else
    {
      session->SetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USERLINKTARGETDIRECTORY, ConfigValue(linkTargetDirectory.ToString()));
    }
    RunIniTeXMF({ "--mklinks" });
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
#endif

#if !defined(MIKTEX_WINDOWS)
void MainWindow::ChangeLinkTargetDirectory()
{
  try
  {
    QString currentLinkTargetDirectory = QString::fromUtf8(session->GetSpecialPath(SpecialPath::LinkTargetDirectory).ToDisplayString().c_str());
    QString directory = QFileDialog::getExistingDirectory(this, tr("Change Link Target Directory"), currentLinkTargetDirectory);
    if (directory.isNull() || directory == currentLinkTargetDirectory)
    {
      return;
    }
    QThread* thread = new QThread;
    ChangeLinkTargetDirectoryWorker* worker = new ChangeLinkTargetDirectoryWorker(directory.toUtf8().constData());
    backgroundWorkers++;
    ui->labelBackgroundTask->setText(tr("Changing link target directory..."));
    worker->moveToThread(thread);
    (void)connect(thread, SIGNAL(started()), worker, SLOT(Process()));
    (void)connect(worker, &ChangeLinkTargetDirectoryWorker::OnFinish, this, [this]() {
      ChangeLinkTargetDirectoryWorker* worker = (ChangeLinkTargetDirectoryWorker*)sender();
      if (!worker->GetResult())
      {
        CriticalError(tr("Something went wrong while changing the link target directory."), ((ChangeLinkTargetDirectoryWorker*)sender())->GetMiKTeXException());
      }
      backgroundWorkers--;
      UpdateUi();
      UpdateActions();
      worker->deleteLater();
    });
    (void)connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
    (void)connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
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
#endif

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
  (void)connect(ui->treeViewFormats, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnContextMenuFormats(const QPoint&)));
  (void)connect(ui->actionAddFormat, SIGNAL(triggered()), this, SLOT(AddFormat()));
  (void)connect(ui->actionRemoveFormat, SIGNAL(triggered()), this, SLOT(RemoveFormat()));
  (void)connect(ui->actionFormatProperties, SIGNAL(triggered()), this, SLOT(FormatPropertyDialog()));
  (void)connect(ui->actionBuildFormat, SIGNAL(triggered()), this, SLOT(BuildFormat()));
  (void)connect(ui->treeViewFormats->selectionModel(),
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
    (void)connect(thread, SIGNAL(started()), worker, SLOT(Process()));
    (void)connect(worker, &BuildFormatsWorker::OnFinish, this, [this]() {
      BuildFormatsWorker* worker = (BuildFormatsWorker*)sender();
      if (!worker->GetResult())
      {
        CriticalError(tr("Something went wrong while building formats."), ((BuildFormatsWorker*)sender())->GetMiKTeXException());
      }
      backgroundWorkers--;
      UpdateUi();
      UpdateActions();
      worker->deleteLater();
    });
    (void)connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
    (void)connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
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
  (void)connect(ui->treeViewLanguages->selectionModel(),
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
  toolBarPackages->addAction(ui->actionPackageInfo);
  toolBarPackages->addSeparator();
  toolBarPackages->addAction(ui->actionUpdatePackageDatabase);
  toolBarPackages->addSeparator();
  lineEditPackageFilter = new QLineEdit(toolBarPackages);
  lineEditPackageFilter->setClearButtonEnabled(true);
  toolBarPackages->addWidget(lineEditPackageFilter);
  toolBarPackages->addAction(ui->actionFilterPackages);
  (void)connect(lineEditPackageFilter, SIGNAL(returnPressed()), this, SLOT(FilterPackages()));
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
  contextMenuPackage->addAction(ui->actionPackageInfo);
  ui->treeViewPackages->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->comboRepository->setModel(repositoryModel);
  (void)connect(ui->comboRepository, SIGNAL(currentIndexChanged(int)), this, SLOT(OnRepositorySelected(int)));
  (void)connect(ui->treeViewPackages, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnContextMenuPackages(const QPoint&)));
  (void)connect(ui->treeViewPackages->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    this,
    SLOT(UpdateActionsPackages()));
  (void)connect(ui->actionPackageInfo,
    SIGNAL(triggered()),
    this,
    SLOT(PackageInfoDialog()));
  (void)connect(ui->actionInstallPackage,
    SIGNAL(triggered()),
    this,
    SLOT(InstallPackage()));
  (void)connect(ui->actionUninstallPackage,
    SIGNAL(triggered()),
    this,
    SLOT(UninstallPackage()));
  (void)connect(ui->actionUpdatePackageDatabase,
    SIGNAL(triggered()),
    this,
    SLOT(UpdatePackageDatabase()));
  (void)connect(ui->actionFilterPackages,
    SIGNAL(triggered()),
    this,
    SLOT(FilterPackages()));
}

void MainWindow::UpdateUiPackages()
{
  ui->lineEditInstallRoot->setText(QString::fromUtf8(session->GetSpecialPath(SpecialPath::InstallRoot).GetData()));
  ui->comboRepository->setEnabled(!IsBackgroundWorkerActive());
  ui->comboRepository->blockSignals(true);
  ui->comboRepository->setCurrentIndex(repositoryModel->GetDefaultIndex());
  ui->comboRepository->blockSignals(false);
}

void MainWindow::UpdateActionsPackages()
{
  try
  {
    ui->actionUpdatePackageDatabase->setEnabled(!IsBackgroundWorkerActive() && !IsUserModeBlocked());
    ui->actionFilterPackages->setEnabled(!IsBackgroundWorkerActive());
    QModelIndexList selectedRows = ui->treeViewPackages->selectionModel()->selectedRows();
    ui->actionPackageInfo->setEnabled(!IsBackgroundWorkerActive() && selectedRows.count() == 1);
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
      if (packageInfo.IsInstalled())
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

void MainWindow::PackageInfoDialog()
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
      else if (!packageInfo.IsInstalled())
      {
        toBeInstalled.push_back(packageInfo.id);
      }
      else
      {
        toBeRemoved.push_back(packageInfo.id);
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
    MIKTEX_AUTO(ui->treeViewPackages->update());
    UpdateDialog::DoModal(this, packageManager, toBeInstalled, toBeRemoved);
    packageModel->Reload();
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
    installer->SetCallback(this);
    LOG4CXX_INFO(logger, "updating package database...");
    installer->UpdateDb({});
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
  (void)connect(thread, SIGNAL(started()), worker, SLOT(Process()));
  (void)connect(worker, &UpdateDbWorker::OnFinish, this, [this]() {
    UpdateDbWorker* worker = (UpdateDbWorker*)sender();
    if (!worker->GetResult())
    {
      CriticalError(tr("Something went wrong while updating the package database."), worker->GetMiKTeXException());
    }
    try
    {
      packageModel->Reload();
    }
    catch (const MiKTeXException& e)
    {
      CriticalError(e);
    }
    catch (const exception& e)
    {
      CriticalError(e);
    }
    ui->treeViewPackages->update();
    backgroundWorkers--;
    UpdateUi();
    UpdateActions();
    worker->deleteLater();
  });
  (void)connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
  (void)connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  UpdateUi();
  UpdateActions();
  thread->start();
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
  ui->lineEditLogFiles->setText(QString::fromUtf8(logDir.ToDisplayString().c_str()));
}

void MainWindow::on_pushButtonShowLogDirectory_clicked()
{
  OpenDirectoryInFileBrowser(session->GetSpecialPath(SpecialPath::LogDirectory));
}

PathName MainWindow::GetReportFileName()
{
  return session->GetSpecialPath(SpecialPath::LogDirectory) / PathName("miktex-report.txt");
}

void MainWindow::CreateReport()
{
  ofstream ofs;
#if defined(MIKTEX_WINDOWS)
  ofs.open(GetReportFileName().ToWideCharString());
#else
  ofs.open(GetReportFileName().ToString());
#endif
  if (!ofs.is_open())
  {
    MIKTEX_FATAL_ERROR("The report could not be written.");
  }
  SetupService::WriteReport(ofs);
  ofs.close();
}

void MainWindow::on_pushButtonOpenReport_clicked()
{
  try
  {
    CreateReport();
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromUtf8(GetReportFileName().GetData()))))
    {
      MIKTEX_FATAL_ERROR("The report could not be opened.");
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

void MainWindow::SetupUiCleanup()
{
  (void)connect(ui->actionUserReset, SIGNAL(triggered()), this, SLOT(UserReset()));
  (void)connect(ui->actionFactoryReset, SIGNAL(triggered()), this, SLOT(FactoryReset()));
  (void)connect(ui->actionUninstall, SIGNAL(triggered()), this, SLOT(Uninstall()));
}

bool MainWindow::IsUserResetPossible()
{
  return session->IsSharedSetup() && !session->IsAdminMode();
}

bool MainWindow::IsFactoryResetPossible()
{
#if defined(MIKTEX_WINDOWS)
  return false;
#else
  return !session->IsSharedSetup() || session->IsAdminMode();
#endif
}

bool MainWindow::IsUninstallPossible()
{
#if defined(MIKTEX_WINDOWS)
  return !session->IsSharedSetup() || session->IsAdminMode();
#else
  return false;
#endif
}

void MainWindow::UpdateUiCleanup()
{
  ui->buttonUserReset->setEnabled(IsUserResetPossible() && !IsBackgroundWorkerActive());
  ui->buttonFactoryReset->setEnabled(IsFactoryResetPossible() && !IsBackgroundWorkerActive());
  ui->buttonUninstall->setEnabled(IsUninstallPossible() && !IsBackgroundWorkerActive());
}

void MainWindow::UpdateActionsCleanup()
{
  ui->actionUserReset->setEnabled(IsUserResetPossible() && !IsBackgroundWorkerActive());
  ui->actionFactoryReset->setEnabled(IsFactoryResetPossible() && !IsBackgroundWorkerActive());
  ui->actionUninstall->setEnabled(IsUninstallPossible() && !IsBackgroundWorkerActive());
}

bool UserResetWorker::Run()
{
  bool result = false;
  try
  {
    shared_ptr<Session> session = Session::Get();
    unique_ptr<SetupService> service = SetupService::Create();
    SetupOptions options = service->GetOptions();
    options.Task = SetupTask::CleanUp;
    options.IsCommonSetup = session->IsAdminMode();
    options.CleanupOptions = { CleanupOption::FileTypes, CleanupOption::Registry, CleanupOption::RootDirectories, CleanupOption::StartMenu };
    options = service->SetOptions(options);
    service->SetCallback(this);
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

void MainWindow::UserReset()
{
  QString message = tr("<h3>Reset personal MiKTeX configuration</h3>");
  message += tr("<p>You are about to remove:</p>");
  message += "<ul>";
#if defined(MIKTEX_WINDOWS)
  message += tr("<li>MiKTeX registry keys in <tt>HKEY_CURRENT_USER</tt></li>");
#endif
  set<PathName> roots{
    session->GetSpecialPath(SpecialPath::UserConfigRoot),
    session->GetSpecialPath(SpecialPath::UserDataRoot),
    session->GetSpecialPath(SpecialPath::UserInstallRoot)
  };
  for (const PathName& r : roots)
  {
    if (Directory::Exists(r))
    {
      message += tr("<li>Directory <tt>%1</tt></li>").arg(QString::fromUtf8(r.ToDisplayString().c_str()));
    }
  }
  message += "</ul>";
  message += tr("<p>Are you sure?</p>");
  if (QMessageBox::warning(this, tr("MiKTeX Console"), message, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
  {
    return;
  }
  try
  {
    QThread* thread = new QThread;
    UserResetWorker* worker = new UserResetWorker;
    backgroundWorkers++;
    ui->labelBackgroundTask->setText(tr("Resetting personal MiKTeX configuration..."));
    worker->moveToThread(thread);
    (void)connect(thread, SIGNAL(started()), worker, SLOT(Process()));
    (void)connect(worker, &UserResetWorker::OnFinish, this, [this]() {
      UserResetWorker* worker = (UserResetWorker*)sender();
      if (worker->GetResult())
      {
        QMessageBox::information(this, tr("MiKTeX Console"), tr("The personal MiKTeX configuration has been resetted.\n\nThe application window will now be closed."));
      }
      else
      {
        QMessageBox::warning(this, tr("MiKTeX Console"), tr("Something went wrong while resetting your personal MiKTeX configuration.\n\nThe application window will now be closed."));
      }
      backgroundWorkers--;
      session->UnloadFilenameDatabase();
      worker->deleteLater();
      saveSettingsOnClose = false;
      isCleaningUp = true;
      this->close();
    });
    (void)connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
    (void)connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
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
    options = service->SetOptions(options);
    service->SetCallback(this);
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
    (void)connect(thread, SIGNAL(started()), worker, SLOT(Process()));
    (void)connect(worker, &FactoryResetWorker::OnFinish, this, [this]() {
      FactoryResetWorker* worker = (FactoryResetWorker*)sender();
      if (worker->GetResult())
      {
        QMessageBox::information(this, tr("MiKTeX Console"), tr("The TeX installation has been restored to its initial state.\n\nThe application window will now be closed."));
      }
      else
      {
        QMessageBox::warning(this, tr("MiKTeX Console"), QString::fromUtf8(worker->GetMiKTeXException().GetErrorMessage().c_str()) + tr("\n\nThe application window will now be closed."));
      }
      backgroundWorkers--;
      session->UnloadFilenameDatabase();
      worker->deleteLater();
      saveSettingsOnClose = false;
      isCleaningUp = true;
      this->close();
    });
    (void)connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
    (void)connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
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

bool UninstallWorker::Run()
{
  bool result = false;
  try
  {
    shared_ptr<Session> session = Session::Get();
    unique_ptr<SetupService> service = SetupService::Create();
    SetupOptions options = service->GetOptions();
    options.Task = SetupTask::CleanUp;
    options.IsCommonSetup = session->IsAdminMode();
    options.CleanupOptions = { CleanupOption::Components, CleanupOption::FileTypes, CleanupOption::Links, CleanupOption::Path, CleanupOption::Registry, CleanupOption::RootDirectories, CleanupOption::StartMenu };
    options = service->SetOptions(options);
    service->SetCallback(this);
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

void MainWindow::Uninstall()
{
  QString message = tr("<h3>Uninstall MiKTeX</h3>");
  message += tr("<p>You are about to remove MiKTeX from your computer. All TEXMF root directories will be removed and you will loose all configuration settings, log files, data files and packages.</p>");
  message += tr("Are you sure?");
  if (QMessageBox::warning(this, tr("MiKTeX Console"), message, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
  {
    return;
  }
  try
  {
    QThread* thread = new QThread;
    UninstallWorker* worker = new UninstallWorker;
    backgroundWorkers++;
    ui->labelBackgroundTask->setText(tr("Uninstalling MiKTeX..."));
    worker->moveToThread(thread);
    (void)connect(thread, SIGNAL(started()), worker, SLOT(Process()));
    (void)connect(worker, &UninstallWorker::OnFinish, this, [this]() {
      UninstallWorker* worker = (UninstallWorker*)sender();
      if (worker->GetResult())
      {
        QMessageBox::information(this, tr("MiKTeX Console"), tr("MiKTeX has been removed from your computer.\n\nThe application window will now be closed."));
      }
      else
      {
        QMessageBox::warning(this, tr("MiKTeX Console"), QString::fromUtf8(worker->GetMiKTeXException().GetErrorMessage().c_str()) + tr("\n\nThe application window will now be closed."));
      }
      backgroundWorkers--;
      session->UnloadFilenameDatabase();
      worker->deleteLater();
      this->close();
    });
    (void)connect(worker, SIGNAL(OnFinish()), thread, SLOT(quit()));
    (void)connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
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
  PathName consoleIni = session->GetSpecialPath(SpecialPath::ConfigRoot) / PathName(MIKTEX_PATH_MIKTEX_CONFIG_DIR) / PathName("console.ini");
  if (!File::Exists(consoleIni))
  {
    return;
  }
  unique_ptr<Cfg> settings = Cfg::Create();
  settings->Read(consoleIni);
  string s;
  if (!settings->TryGetValueAsString("MainWindow", "geometry", s))
  {
    return;
  }
  restoreGeometry(QByteArray::fromHex(s.c_str()));
}

void MainWindow::WriteSettings()
{
  unique_ptr<Cfg> settings = Cfg::Create();
  settings->PutValue("MainWindow", "geometry", saveGeometry().toHex().constData());
  settings->Write(session->GetSpecialPath(SpecialPath::ConfigRoot) / PathName(MIKTEX_PATH_MIKTEX_CONFIG_DIR) / PathName("console.ini"));
}

void MainWindow::Restart()
{
  LOG4CXX_INFO(logger, "MiKTeX Console needs to be restarted");
  QMessageBox::information(this, tr("MiKTeX Console"), tr("MiKTeX Console needs to be restarted."));
  vector<string> args{ MIKTEX_CONSOLE_EXE };
  if (session->IsAdminMode())
  {
    args.push_back("--admin");
  }
  Process::Start(session->GetMyProgramFile(true), args);
  this->close();
}

void MainWindow::UnloadFileNameDatabase()
{
  session->UnloadFilenameDatabase(std::chrono::seconds(1));
}
