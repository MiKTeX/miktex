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

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT;

private slots:
  void on_buttonOverview_clicked();

private slots:
  void on_buttonUpdates_clicked();

private slots:
  void on_buttonPackages_clicked();

private slots:
  void on_buttonAdminSetup_clicked();

private slots:
  void on_buttonUserSetup_clicked();

private slots:
  void on_buttonUpgrade_clicked();

private slots:
  void on_buttonTeXworks_clicked();

private slots:
  void EnableActions();

private slots:
  void AboutDialog();

private slots:
  void RestartAdmin();

public slots:
  void FinishSetup();

private:
  void closeEvent(QCloseEvent* event);

public:
  explicit MainWindow(QWidget* parent = nullptr);

public:
  ~MainWindow();

private:
  void RestartAdminWithArguments(const std::vector<std::string>& args);

private:
  void UpdateWidgets();

private:
  enum class Pages {
    Setup = 0,
    Overview = 1,
    Updates = 2,
    Packages = 3,
  };

private:
  void SetCurrentPage(Pages p);

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
  bool isSetupMode = false;

private:
  Ui::MainWindow* ui;

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;
};

class BackgroundWorker :
  public QObject
{
private:
  Q_OBJECT;

public slots:
  virtual void Process() = 0;

private:
  static std::atomic_int instances;

public:
  BackgroundWorker()
  {
    instances++;
  }

public:
  virtual ~BackgroundWorker()
  {
    instances--;
  }

public:
  static int GetCount()
  {
    return instances;
  }

public:
  MiKTeX::Core::MiKTeXException GetMiKTeXException() const
  {
    return e;
  }

protected:
  MiKTeX::Core::MiKTeXException e;

signals:
  void OnMiKTeXException();

signals:
  void OnFinish();
};

class FinishSetupWorker :
  public BackgroundWorker
{
private:
  Q_OBJECT;

private:
  static std::atomic_bool running;

public:
  FinishSetupWorker()
  {
    MIKTEX_ASSERT(!running);
    running = true;
  }

public:
  virtual ~FinishSetupWorker()
  {
    MIKTEX_ASSERT(running);
    running = false;
  }

public:
  static bool IsRunnning()
  {
    return running;
  }

public slots:
  void Process() override;
};

class UpgradeWorker :
  public BackgroundWorker,
  public MiKTeX::Packages::PackageInstallerCallback
{
private:
  Q_OBJECT;

private:
  static std::atomic_bool running;

public:
  static bool IsRunnning()
  {
    return running;
  }

public:
  UpgradeWorker(std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager) :
    packageManager(packageManager)
  {
    MIKTEX_ASSERT(!running);
    running = true;
  }

public:
  virtual ~UpgradeWorker()
  {
    MIKTEX_ASSERT(!running);
    running = false;
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

public slots:
  void Process() override;

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

#endif
