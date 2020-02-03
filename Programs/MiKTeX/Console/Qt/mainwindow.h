/* mainwindow.h:                                        -*- C++ -*-

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

#pragma once

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <miktex/Core/Session>
#include <miktex/PackageManager/PackageManager>
#include <miktex/Setup/SetupService>

#include <atomic>
#include <memory>

#include <QMainWindow>
#include <QSystemTrayIcon>

class QLineEdit;

class FormatTableModel;
class LanguageTableModel;
class PackageProxyModel;
class PackageTableModel;
class RepositoryTableModel;
class RootTableModel;
class UpdateTableModel;

namespace Ui
{
  class MainWindow;
}

class MainWindow :
  public QMainWindow
{
  Q_OBJECT;

private:
  Ui::MainWindow* ui = nullptr;

public:
  enum class Pages
  {
    Setup = 0,
    Overview = 1,
    Settings = 2,
    Updates = 3,
    Packages = 4,
    Diagnose = 5,
    Cleanup = 6
  };

public:
  explicit MainWindow(QWidget* parent = nullptr, Pages startPage = Pages::Overview);

public:
  ~MainWindow() override;

private:
  void closeEvent(QCloseEvent* event) override;

private:
  void setVisible(bool visible) override;

private:
  void CriticalError(const QString& shortText, const MiKTeX::Core::MiKTeXException& e);

private:
  void CriticalError(const MiKTeX::Core::MiKTeXException& e)
  {
    CriticalError(tr("Sorry, something went wrong."), e);
  }

private:
  void CriticalError(const std::exception& e)
  {
    CriticalError(MiKTeX::Core::MiKTeXException(e.what()));
  }

private slots:
  void ShowMajorIssue();

private:
  RepositoryTableModel* repositoryModel = nullptr;

private:
  void UpdateUi();

private:
  bool checkedIssues = false;

private:
  std::vector<MiKTeX::Setup::Issue> issues;

private:
  void FindIssues()
  {
    issues = MiKTeX::Setup::SetupService::FindIssues(true, false);
    checkedIssues = true;
  }

private:
  std::pair<bool, MiKTeX::Setup::Issue> CheckIssue(MiKTeX::Setup::IssueType issueType)
  {
    if (!checkedIssues)
    {
      FindIssues();
    }
    for (const auto& iss : issues)
    {
      if (iss.type == issueType)
      {
        return std::make_pair(true, iss);
      }
    }
    return std::make_pair(false, MiKTeX::Setup::Issue());
  }

private slots:
  void UpdateActions();

#if !defined(QT_NO_SYSTEMTRAYICON)
private:
  QSystemTrayIcon* trayIcon = nullptr;

private:
  QMenu* trayIconMenu = nullptr;

private:
  void CreateTrayIcon();

private slots:
  void TrayIconActivated(QSystemTrayIcon::ActivationReason reason);

private slots:
  void TrayMessageClicked();

private:
  enum class TrayMessageContext
  {
    None,
    Error,
    Updates
  };

private:
  TrayMessageContext trayMessageContext = TrayMessageContext::None;

private:
  void ShowTrayMessage(TrayMessageContext context, const QString& message);
#endif

private:
  void SetCurrentPage(Pages p);

private slots:
  void on_buttonOverview_clicked()
  {
    SetCurrentPage(Pages::Overview);
  }

private slots:
  void on_buttonSettings_clicked()
  {
    SetCurrentPage(Pages::Settings);
  }

private slots:
  void on_buttonUpdates_clicked()
  {
    SetCurrentPage(Pages::Updates);
  }

private slots:
  void on_buttonPackages_clicked()
  {
    SetCurrentPage(Pages::Packages);
  }

private slots:
  void on_buttonDiagnose_clicked()
  {
    SetCurrentPage(Pages::Diagnose);
  }

private slots:
  void on_buttonCleanup_clicked()
  {
    SetCurrentPage(Pages::Cleanup);
  }

  private slots:
  void StartTeXworks();

private slots:
  void on_buttonTeXworks_clicked()
  {
    StartTeXworks();
  }

private slots:
  void StartTerminal();

private slots:
  void on_buttonTerminal_clicked()
  {
    StartTerminal();
  }

private slots:
  void AboutDialog();

private slots:
  void RestartAdmin();

private slots:
  void on_buttonRestartAdmin_clicked()
  {
    RestartAdmin();
  }

private:
  bool okayUserMode = false;

private:
  void OkayUserMode();

private slots:
  void on_buttonOkayUserMode_clicked()
  {
    OkayUserMode();
  }

private:
  bool IsUserModeBlocked()
  {
    return session->IsSharedSetup() && !session->IsAdminMode() && !okayUserMode;
  }

private:
  void RestartAdminWithArguments(const std::vector<std::string>& args);

private slots:
  void on_buttonAdminSetup_clicked();

private slots:
  void on_buttonUserSetup_clicked()
  {
    FinishSetup();
  }

private:
  bool isSetupMode = false;

public slots:
  void FinishSetup();

#if defined(MIKTEX_WINDOWS)
private slots:
  void on_buttonFixPath_clicked();
#endif

private slots:
  void on_buttonUpgrade_clicked();

private slots:
  void RefreshFndb();

public slots:
  void RefreshFontMaps();

private:
  QToolBar* toolBarUpdates = nullptr;

private:
  void SetupUiUpdates();

private slots:
  void UpdateUiUpdates();

private slots:
  void UpdateActionsUpdates();

private:
  UpdateTableModel* updateModel = nullptr;

private slots:
  void CheckUpdates();

private slots:
  void on_buttonUpdateCheck_clicked()
  {
    CheckUpdates();
  }

private slots:
  void on_buttonCheckUpdates_clicked()
  {
    CheckUpdates();
  }

private slots:
  void on_buttonChangeRepository3_clicked()
  {
    ChangeRepository();
  }

private:
  void Update();

private slots:
  void on_buttonUpdateNow_clicked()
  {
    Update();
  }

private slots:
  void on_labelUpdatesAvailable_linkActivated(const QString& link)
  {
    if (link == "#updates")
    {
      SetCurrentPage(Pages::Updates);
    }
  }

private:
  QMenu* contextMenuUpdate = nullptr;

private slots:
  void OnContextMenuUpdates(const QPoint& pos);

private:
  void SetupUiPackageInstallation();

private:
  void UpdateUiPackageInstallation();

private:
  void ChangeRepository();

private slots:
  void OnRepositorySelected(int index);

private slots:
  void on_radioAutoInstallAsk_clicked();

private slots:
  void on_radioAutoInstallYes_clicked();

private slots:
  void on_radioAutoInstallNo_clicked();

private:
  void UpdateUiPaper();

private slots:
  void on_comboPaper_activated(int idx);

private slots:
  void on_buttonChangeRepository2_clicked()
  {
    ChangeRepository();
  }

private:
  QToolBar* toolBarRootDirectories = nullptr;

private:
  void SetupUiDirectories();

private:
  void UpdateUiDirectories();
  
private slots:
  void UpdateActionsDirectories();

private:
  RootTableModel* rootDirectoryModel = nullptr;

private slots:
  void AddRootDirectory();

private slots:
  void RemoveRootDirectory();

private slots:
  void MoveRootDirectoryUp();

private slots:
  void MoveRootDirectoryDown();

private slots:
  void OpenRootDirectory();

private:
  QMenu* contextMenuRootDirectory = nullptr;

private:
  QMenu* contextMenuRootDirectoriesBackground = nullptr;

private slots:
  void OnContextMenuRootDirectories(const QPoint& pos);

#if !defined(MIKTEX_WINDOWS)
private:
  void ChangeLinkTargetDirectory();
#endif

#if !defined(MIKTEX_WINDOWS)
private slots:
  void on_buttonChangeLinkTargetDirectory_clicked()
  {
    ChangeLinkTargetDirectory();
  }
#endif

private:
  QToolBar* toolBarFormats = nullptr;

private:
  void SetupUiFormats();

private:
  void UpdateUiFormats();

private slots:
  void UpdateActionsFormats();

private:
  FormatTableModel* formatModel = nullptr;

private slots:
  void AddFormat();

private slots:
  void RemoveFormat();

private slots:
  void FormatPropertyDialog();

private slots:
  void BuildFormat();

private:
  QMenu* contextMenuFormat = nullptr;

private:
  QMenu* contextMenuFormatsBackground = nullptr;

private slots:
  void OnContextMenuFormats(const QPoint& pos);


private:
  void SetupUiLanguages();

private:
  void UpdateUiLanguages();

private slots:
  void UpdateActionsLanguages();

private:
  LanguageTableModel* languageModel = nullptr;

private:
  PackageTableModel* packageModel = nullptr;

private:
  PackageProxyModel* packageProxyModel = nullptr;

private:
  QToolBar* toolBarPackages = nullptr;

private:
  QLineEdit* lineEditPackageFilter = nullptr;

private:
  void SetupUiPackages();

private:
  void UpdateUiPackages();

private slots:
  void UpdateActionsPackages();

private slots:
  void PackageInfoDialog();

private slots:
  void InstallPackage();

private slots:
  void UninstallPackage();

private slots:
  void FilterPackages();

private slots:
  void UpdatePackageDatabase();

private slots:
  void on_buttonChangeRepository_clicked()
  {
    ChangeRepository();
  }

private:
  QMenu* contextMenuPackage = nullptr;

private:
  QMenu* contextMenuPackagesBackground = nullptr;

private slots:
  void OnContextMenuPackages(const QPoint& pos);

private:
  void SetupUiDiagnose();

private:
  void UpdateUiDiagnose();

private slots:
  void UpdateActionsDiagnose();

private slots:
  void on_pushButtonShowLogDirectory_clicked();

private slots:
  void on_pushButtonOpenReport_clicked();

private:
  void CreateReport();

private:
  MiKTeX::Core::PathName GetReportFileName();

private:
  void SetupUiCleanup();

private:
  void UpdateUiCleanup();

private slots:
  void UpdateActionsCleanup();

private slots:
  void UserReset();

private slots:
  void FactoryReset();

private slots:
  void Uninstall();

private slots:
  void on_buttonUserReset_clicked()
  {
    UserReset();
  }

private slots:
  void on_buttonFactoryReset_clicked()
  {
    FactoryReset();
  }

private slots:
  void on_buttonUninstall_clicked()
  {
    Uninstall();
  }

private:
  bool IsUserResetPossible();

private:
  bool IsFactoryResetPossible();

private:
  bool IsUninstallPossible();

private:
  void ReadSettings();

private:
  void WriteSettings();

private:
  bool saveSettingsOnClose = true;

private:
  bool isCleaningUp = false;

public:
  bool IsCleaningUp() const
  {
    return isCleaningUp;
  }

private:
  std::atomic_int backgroundWorkers{ 0 };

private:
  bool IsBackgroundWorkerActive()
  {
    return backgroundWorkers > 0;
  }

private:
  void Restart();

private slots:
  void UnloadFileNameDatabase();

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager = MiKTeX::Packages::PackageManager::Create();
};

class PackageInstallerCallbackImpl :
  public MiKTeX::Packages::PackageInstallerCallback
{
private:
  void ReportLine(const std::string& str) override;

private:
  bool OnRetryableError(const std::string& message) override
  {
    return false;
  }

private:
  bool OnProgress(MiKTeX::Packages::Notification notification) override
  {
    return true;
  }
};

class SetupServiceCallbackImpl :
  public MiKTeX::Setup::SetupServiceCallback
{
protected:
  void ReportLine(const std::string& str) override;

protected:
  bool OnRetryableError(const std::string& message) override
  {
    return false;
  }

protected:
  bool OnProgress(MiKTeX::Setup::Notification nf) override
  {
    return true;
  }

protected:
  bool OnProcessOutput(const void* output, size_t n) override
  {
    return true;
  }
};

class BackgroundWorker :
  public QObject
{
private:
  Q_OBJECT;

private:
  bool result;

public:
  bool GetResult() const
  {
    return result;
  }

public slots:
  virtual void Process()
  {
    result = Run();
    emit OnFinish();
  }

protected:
  virtual bool Run() = 0;

protected:
  void RunIniTeXMF(const std::vector<std::string>& args);

public:
  MiKTeX::Core::MiKTeXException GetMiKTeXException() const
  {
    return e;
  }

protected:
  MiKTeX::Core::MiKTeXException e;

signals:
  void OnFinish();
};

class FinishSetupWorker :
  public BackgroundWorker,
  public SetupServiceCallbackImpl
{
private:
  Q_OBJECT;

protected:
  bool Run() override;
};

class UpgradeWorker :
  public BackgroundWorker,
  public PackageInstallerCallbackImpl
{
private:
  Q_OBJECT;

public:
  UpgradeWorker(std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager) :
    packageManager(packageManager)
  {
  }

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;

private:
  std::unique_ptr<MiKTeX::Packages::PackageInstaller> packageInstaller;

private:
  MiKTeX::Packages::PackageInstaller::ProgressInfo progressInfo;

public:
  MiKTeX::Packages::PackageInstaller::ProgressInfo GetProgressInfo() const
  {
    return progressInfo;
  }

public:
  enum class Status {
    None,
    Synchronize,
    Download,
    Install
  };
  
private:
  Status status;

public:
  Status GetStatus() const
  {
    return status;
  }

protected:
  bool Run() override;

signals:
  void OnUpgradeProgress();

private:
  bool OnProgress(MiKTeX::Packages::Notification notification) override
  {
    switch (notification)
    {
    case MiKTeX::Packages::Notification::DownloadPackageStart:
      status = Status::Download;
      break;
    case MiKTeX::Packages::Notification::InstallPackageStart:
      status = Status::Install;
      break;
    case MiKTeX::Packages::Notification::DownloadPackageEnd:
    case MiKTeX::Packages::Notification::InstallPackageEnd:
      status = Status::None;
      break;
    default:
      break;
    }
    progressInfo = packageInstaller->GetProgressInfo();
    emit OnUpgradeProgress();
    return true;
  }
};

class RefreshFndbWorker :
  public BackgroundWorker
{
private:
  Q_OBJECT;

public:
  RefreshFndbWorker(std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager) :
    packageManager(packageManager)
  {
  }

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;

protected:
  bool Run() override;
};

class RefreshFontMapsWorker :
  public BackgroundWorker
{
private:
  Q_OBJECT;

protected:
  bool Run() override;
};

class CkeckUpdatesWorker :
  public BackgroundWorker,
  public PackageInstallerCallbackImpl
{
private:
  Q_OBJECT;

public:
  CkeckUpdatesWorker(std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager) :
    packageManager(packageManager)
  {
  }

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;

public:
  enum class Status {
    None,
    Checking
  };

private:
  Status status;

public:
  Status GetStatus() const
  {
    return status;
  }

protected:
  bool Run() override;

private:
  std::vector<MiKTeX::Packages::PackageInstaller::UpdateInfo> updates;


public:
  std::vector<MiKTeX::Packages::PackageInstaller::UpdateInfo> GetUpdates() const
  {
    return updates;
  }
};

class UpdateWorker :
  public BackgroundWorker,
  public PackageInstallerCallbackImpl
{
private:
  Q_OBJECT;

public:
  UpdateWorker(std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager, const std::vector<MiKTeX::Packages::PackageInstaller::UpdateInfo>& updates) :
    packageManager(packageManager),
    updates(updates)
  {
  }

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;

private:
  std::unique_ptr<MiKTeX::Packages::PackageInstaller> packageInstaller;

private:
  MiKTeX::Packages::PackageInstaller::ProgressInfo progressInfo;

public:
  MiKTeX::Packages::PackageInstaller::ProgressInfo GetProgressInfo() const
  {
    return progressInfo;
  }

public:
  enum class Status {
    None,
    Synchronize,
    Download,
    Install
  };

private:
  Status status;

public:
  Status GetStatus() const
  {
    return status;
  }

protected:
  bool Run() override;

signals:
  void OnUpdateProgress();

private:
  bool OnProgress(MiKTeX::Packages::Notification notification) override
  {
    switch (notification)
    {
    case MiKTeX::Packages::Notification::DownloadPackageStart:
      status = Status::Download;
      break;
    case MiKTeX::Packages::Notification::InstallPackageStart:
      status = Status::Install;
      break;
    case MiKTeX::Packages::Notification::DownloadPackageEnd:
    case MiKTeX::Packages::Notification::InstallPackageEnd:
      status = Status::None;
      break;
    default:
      break;
    }
    progressInfo = packageInstaller->GetProgressInfo();
    emit OnUpdateProgress();
    return true;
  }

private:
  std::vector<MiKTeX::Packages::PackageInstaller::UpdateInfo> updates;
};

class UpdateDbWorker :
  public BackgroundWorker,
  public PackageInstallerCallbackImpl
{
private:
  Q_OBJECT;

public:
  UpdateDbWorker(std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager) :
    packageManager(packageManager)
  {
  }

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;

protected:
  bool Run() override;
};

class UserResetWorker :
  public BackgroundWorker,
  public SetupServiceCallbackImpl
{
private:
  Q_OBJECT;

protected:
  bool Run() override;
};

class FactoryResetWorker :
  public BackgroundWorker,
  public SetupServiceCallbackImpl
{
private:
  Q_OBJECT;

protected:
  bool Run() override;
};

class UninstallWorker :
  public BackgroundWorker,
  public SetupServiceCallbackImpl
{
private:
  Q_OBJECT;

protected:
  bool Run() override;
};

class BuildFormatsWorker :
  public BackgroundWorker
{
private:
  Q_OBJECT;

public:
  BuildFormatsWorker(const std::vector<std::string>& formats) :
    formats(formats)
  {
  }

private:
  std::vector<std::string> formats;

protected:
  bool Run() override;
};

#if !defined(MIKTEX_WINDOWS)
class ChangeLinkTargetDirectoryWorker :
  public BackgroundWorker
{
private:
  Q_OBJECT;

public:
  ChangeLinkTargetDirectoryWorker(const MiKTeX::Core::PathName& linkTargetDirectory) :
    linkTargetDirectory(linkTargetDirectory)
  {
  }

private:
  MiKTeX::Core::PathName linkTargetDirectory;

protected:
  bool Run() override;
};
#endif

#endif
