/* mainwindow.h:                                        -*- C++ -*-

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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <miktex/Core/Session>
#include <miktex/PackageManager/PackageManager>

#include <atomic>
#include <memory>

#include <QMainWindow>
#include <QSystemTrayIcon>

class QLineEdit;

class PackageProxyModel;
class PackageTableModel;
class RootTableModel;
class UpdateTableModel;

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT;

private:
  Ui::MainWindow* ui = nullptr;

public:
  explicit MainWindow(QWidget* parent = nullptr);

public:
  ~MainWindow();

private:
  void closeEvent(QCloseEvent* event) override;

private:
  void setVisible(bool visible) override;

private:
  void CriticalError(const QString& text, const MiKTeX::Core::MiKTeXException& e);

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

private:
  void UpdateUi();

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
#endif

private:
  enum class Pages
  {
    Setup = 0,
    Overview = 1,
    Settings = 2,
    Updates = 3,
    Packages = 4,
    Troubleshoot = 5,
  };

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
  void on_buttonTroubleshoot_clicked()
  {
    SetCurrentPage(Pages::Troubleshoot);
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

private slots:
  void on_buttonUpgrade_clicked();

private slots:
  void RefreshFndb();

private slots:
  void RefreshFontMaps();

private:
  void SetupUiUpdates();

private:
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
  void UpdateUiPackageInstallation();

private slots:
  void on_buttonChangeRepository_clicked();

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

private:
  QToolBar* toolBarRootDirectories = nullptr;

private:
  void SetupUiRootDirectories();

private:
  void UpdateUiRootDirectories();
  
private slots:
  void UpdateActionsRootDirectories();

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

private:
  PackageTableModel* packageModel;

private:
  PackageProxyModel* packageProxyModel;

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
  void PackagePropertyDialog();

private slots:
  void InstallPackage();

private slots:
  void UninstallPackage();

private slots:
  void FilterPackages();

private slots:
  void UpdatePackageDatabase();

private:
  QMenu* contextMenuPackage = nullptr;

private:
  QMenu* contextMenuPackagesBackground = nullptr;

private slots:
  void OnContextMenuPackages(const QPoint& pos);

private:
  void SetupUiTroubleshoot();

private:
  void UpdateUiTroubleshoot();

private slots:
  void UpdateActionsTroubleshoot();

private slots:
  void on_pushButtonShowLogDirectory_clicked();

private:
  std::atomic_int backgroundWorkers{ 0 };

private:
  bool IsBackgroundWorkerActive()
  {
    return backgroundWorkers > 0;
  }

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager = MiKTeX::Packages::PackageManager::Create();
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
  public BackgroundWorker
{
private:
  Q_OBJECT;

protected:
  bool Run() override;
};

class UpgradeWorker :
  public BackgroundWorker,
  public MiKTeX::Packages::PackageInstallerCallback
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
  void ReportLine(const std::string& str) override
  {
  }

private:
  bool OnRetryableError(const std::string& message) override
  {
    return false;
  }

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
  public BackgroundWorker
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
  public MiKTeX::Packages::PackageInstallerCallback
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
  void ReportLine(const std::string& str) override
  {
  }

private:
  bool OnRetryableError(const std::string& message) override
  {
    return false;
  }

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
    }
    progressInfo = packageInstaller->GetProgressInfo();
    emit OnUpdateProgress();
    return true;
  }

private:
  std::vector<MiKTeX::Packages::PackageInstaller::UpdateInfo> updates;
};

class UpdateDbWorker :
  public BackgroundWorker
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

#endif
