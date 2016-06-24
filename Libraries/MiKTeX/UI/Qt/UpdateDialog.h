/* UpdateDialog.h:                                      -*- C++ -*-

   Copyright (C) 2008-2016 Christian Schenk

   This file is part of the MiKTeX UI Library.

   The MiKTeX UI Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX UI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX UI Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(A3C9B533501744AF97F67F3F0E3161DA)
#define A3C9B533501744AF97F67F3F0E3161DA

#include <memory>
#include <mutex>

#include <QThread>

#include <miktex/PackageManager/PackageManager>
#include <miktex/UI/Qt/ErrorDialog>

#include "ui_UpdateDialog.h"

class UpdateDialogImpl :
  public QDialog,
  private Ui::UpdateDialog,
  public MiKTeX::Packages::PackageInstallerCallback
{
private:
  Q_OBJECT;

public:
  UpdateDialogImpl(QWidget * pParent, std::shared_ptr<MiKTeX::Packages::PackageManager> pManager, const std::vector<std::string> & toBeInstalled, const std::vector<std::string> & toBeRemoved);

public:
  ~UpdateDialogImpl();

public:
  virtual void MIKTEXTHISCALL ReportLine(const std::string & str);

public:
  virtual bool MIKTEXTHISCALL OnRetryableError(const std::string & message);

public:
  virtual bool MIKTEXTHISCALL OnProgress(MiKTeX::Packages::Notification nf);

public:
  bool GetCancelFlag() const
  {
    return cancelled;
  }

public:
  bool GetErrorFlag() const
  {
    return errorOccured;
  }

private slots:
  void ShowProgress();
  void Cancel();

signals:
  void ProgressChanged();

private:
  void Report(bool immediate, const char * lpszFmt, ...);

private:
  void ReportError(const MiKTeX::Core::MiKTeXException & e)
  {
    errorOccured = true;
    MiKTeX::UI::Qt::ErrorDialog::DoModal(this, e);
  }

private:
  void ReportError(const std::exception & e)
  {
    errorOccured = true;
    MiKTeX::UI::Qt::ErrorDialog::DoModal(this, e);
  }

private:
  std::mutex sharedDataMutex;

private:
  bool errorOccured = false;

private:
  bool cancelled = false;

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> pManager;

private:
  class WorkerThread :
    public QThread
  {
  public:
    WorkerThread(UpdateDialogImpl * pParent) :
      QThread(pParent)
    {
    }
  public:
    virtual void run();
  public:
    MiKTeX::Core::MiKTeXException threadMiKTeXException;
  public:
    bool error = false;
  };

private:
  WorkerThread * pWorkerThread;

private:
  struct SharedData
  {
    MiKTeX::Packages::PackageInstaller::ProgressInfo progressInfo;
    QString report;
    unsigned secondsRemaining = 0;
    bool newPackage = false;
    bool ready = false;
    bool reportUpdate = false;
    bool waitingForClickOnClose = false;
    int progress1Pos = 0;
    int progress2Pos = 0;
    std::string packageName;
  };

private:
  SharedData sharedData;

private:
  std::shared_ptr<MiKTeX::Packages::PackageInstaller> pInstaller;
};

#endif
