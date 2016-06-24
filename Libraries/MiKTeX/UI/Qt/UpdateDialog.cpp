/* UpdateDialog.cpp:

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/UI/Qt/ErrorDialog.h"
#include "miktex/UI/Qt/UpdateDialog.h"

#include "UpdateDialog.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::Qt;
using namespace std;

const int PROGRESS_MAX = 1000;

int UpdateDialog::DoModal(QWidget * pParent, shared_ptr<PackageManager> pManager, const vector<string> & toBeInstalled, const vector<string> & toBeRemoved)
{
  string url;
  RepositoryType repositoryType(RepositoryType::Unknown);
  if (toBeInstalled.size() > 0
    && PackageManager::TryGetDefaultPackageRepository(repositoryType, url)
    && repositoryType == RepositoryType::Remote
    && !ProxyAuthenticationDialog(pParent))
  {
    return QDialog::Rejected;
  }
  UpdateDialogImpl dlg(pParent, pManager, toBeInstalled, toBeRemoved);
  return dlg.exec();
}

void UpdateDialogImpl::WorkerThread::run()
{
  UpdateDialogImpl * This = reinterpret_cast<UpdateDialogImpl *>(parent());
#if defined(MIKTEX_WINDOWS)
  HRESULT hr = E_FAIL;
#endif
  try
  {
#if defined(MIKTEX_WINDOWS)
    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
      MIKTEX_UNEXPECTED();
    }
#endif
    This->pInstaller->SetCallback(This);
    This->pInstaller->InstallRemove();
  }
  catch (const MiKTeXException & e)
  {
    threadMiKTeXException = e;
    error = true;
  }
  catch (const exception &)
  {
  }
  This->sharedData.ready = true;
#if defined(MIKTEX_WINDOWS)
  if (SUCCEEDED(hr))
  {
    CoUninitialize();
  }
#endif
  emit This->ProgressChanged();
}

UpdateDialogImpl::UpdateDialogImpl(QWidget * pParent, std::shared_ptr<MiKTeX::Packages::PackageManager> pManager, const vector<string> & toBeInstalled, const vector<string> & toBeRemoved) :
  QDialog(pParent),
  pManager(pManager),
  pInstaller(pManager->CreateInstaller())
{
  setupUi(this);
  connect(this, SIGNAL(ProgressChanged()), this, SLOT(ShowProgress()));
  connect(pushButton, SIGNAL(clicked()), this, SLOT(Cancel()));
  progressBar1->setMinimum(0);
  progressBar1->setMaximum(PROGRESS_MAX);
  progressBar1->setValue(0);
  progressBar2->setMinimum(0);
  progressBar2->setMaximum(PROGRESS_MAX);
  progressBar2->setValue(0);
  pInstaller->SetFileLists(toBeInstalled, toBeRemoved);
  pWorkerThread = new WorkerThread(this);
  pWorkerThread->start();
}

UpdateDialogImpl::~UpdateDialogImpl()
{
  try
  {
    if (pInstaller != nullptr)
    {
      pInstaller->Dispose();
      pInstaller = nullptr;
    }
  }
  catch (const exception &)
  {
  }
}

void UpdateDialogImpl::Report(bool immediate, const char * lpszFmt, ...)
{
  MIKTEX_ASSERT(lpszFmt != nullptr);
  QString str;
  va_list args;
  va_start(args, lpszFmt);
  str.vsprintf(lpszFmt, args);
  va_end(args);
  {
    lock_guard<mutex> lockGuard(sharedDataMutex);
    sharedData.report += str;
    sharedData.reportUpdate = true;
  }
  if (immediate)
  {
    emit ProgressChanged();
  }
}

void UpdateDialogImpl::ReportLine(const string & str)
{
  Report(true, "%s\n", str.c_str());
}

bool UpdateDialogImpl::OnRetryableError(const string & message)
{
  return false;
}

bool UpdateDialogImpl::OnProgress(Notification nf)
{
  lock_guard<mutex> lockGuard(sharedDataMutex);
  bool visibleProgress =
    (nf == Notification::DownloadPackageEnd
      || nf == Notification::InstallFileEnd
      || nf == Notification::InstallPackageEnd
      || nf == Notification::RemoveFileEnd
      || nf == Notification::RemovePackageEnd);
  PackageInstaller::ProgressInfo progressInfo = pInstaller->GetProgressInfo();
  sharedData.progressInfo = progressInfo;
  if (nf == Notification::InstallPackageStart
    || nf == Notification::DownloadPackageStart)
  {
    visibleProgress = true;
    sharedData.newPackage = true;
    sharedData.packageName = progressInfo.displayName;
  }
  if (progressInfo.cbPackageDownloadTotal > 0)
  {
    int oldValue = sharedData.progress1Pos;
    sharedData.progress1Pos
      = static_cast<int>
      (((static_cast<double>(progressInfo.cbPackageDownloadCompleted)
        / progressInfo.cbPackageDownloadTotal)
        * PROGRESS_MAX));
    visibleProgress =
      (visibleProgress || (sharedData.progress1Pos != oldValue));
  }
  if (progressInfo.cbDownloadTotal > 0)
  {
    int oldValue = sharedData.progress2Pos;
    sharedData.progress2Pos
      = static_cast<int>
      (((static_cast<double>(progressInfo.cbDownloadCompleted)
        / progressInfo.cbDownloadTotal)
        * PROGRESS_MAX));
    visibleProgress =
      (visibleProgress || (sharedData.progress2Pos != oldValue));
  }
  unsigned oldValue = sharedData.secondsRemaining;
  sharedData.secondsRemaining
    = static_cast<unsigned>(progressInfo.timeRemaining / 1000);
  visibleProgress =
    (visibleProgress || (sharedData.secondsRemaining != oldValue));
  if (visibleProgress)
  {
    emit ProgressChanged();
  }
  return !(GetErrorFlag() || GetCancelFlag());
}

void UpdateDialogImpl::ShowProgress()
{
  try
  {
    if (pWorkerThread->error)
    {
      pWorkerThread->error = false;
      ReportError(pWorkerThread->threadMiKTeXException);
    }

    lock_guard<mutex> lockGuard(sharedDataMutex);

    // update the report
    if (sharedData.reportUpdate)
    {
      textBrowser->setPlainText(sharedData.report);
      sharedData.reportUpdate = false;
    }

    // do we have to finish?
    if (sharedData.ready || GetCancelFlag() || GetErrorFlag())
    {
      // check to see if we are already ready
      if (sharedData.waitingForClickOnClose)
      {
        return;
      }

      sharedData.waitingForClickOnClose = true;

      // disable controls
      labelProgress1->setEnabled(false);
      labelPackageName->setText("");
      labelPackageName->setEnabled(false);
      progressBar1->setValue(0);
      progressBar1->setEnabled(false);
      labelProgress2->setEnabled(false);
      progressBar2->setValue(0);
      progressBar2->setEnabled(false);

      pushButton->setText(tr("Close"));
      pushButton->setEnabled(true);

      disconnect(pushButton, SIGNAL(clicked()), this, SLOT(Cancel()));
      if (GetCancelFlag() || GetErrorFlag())
      {
        connect(pushButton, SIGNAL(clicked()), this, SLOT(reject()));
      }
      else
      {
        connect(pushButton, SIGNAL(clicked()), this, SLOT(accept()));
      }
    }
    else
    {
      // show the package name
      if (sharedData.newPackage)
      {
        labelPackageName->setText(sharedData.packageName.c_str());
        sharedData.newPackage = false;
      }

      // update progress bars
      progressBar1->setValue(sharedData.progress1Pos);
      progressBar2->setValue(sharedData.progress2Pos);

      // update "Removed files (packages)"
      QString format;
      format = "%1 (%2)";
      labelRemovedFiles->setText(format
        .arg(sharedData.progressInfo.cFilesRemoveCompleted)
        .arg(sharedData.progressInfo.cPackagesRemoveCompleted));

      // update "New files (packages)"
      format = "%1 (%2)";
      labelInstalledFiles->setText(format
        .arg(sharedData.progressInfo.cFilesInstallCompleted)
        .arg(sharedData.progressInfo.cPackagesInstallCompleted));

      // update "Downloaded bytes"
      format = "%1";
      labelDownloadedBytes->setText(format
        .arg(sharedData.progressInfo.cbDownloadCompleted));

      // update "Package"
      labelPackageName->setText(
        sharedData.progressInfo.displayName.c_str());

      // update "KB/s"
      format = "%1";
      labelKbytesSec->setText(format
        .arg((static_cast<double>(sharedData.progressInfo.bytesPerSecond)
          / 1024.0)));
    }
  }
  catch (const MiKTeXException & e)
  {
    ReportError(e);
  }
  catch (const exception & e)
  {
    ReportError(e);
  }
}

void UpdateDialogImpl::Cancel()
{
  try
  {
    if (QMessageBox::Ok == QMessageBox::information(this, "MiKTeX Package Manager", "The update operation will now be cancelled.", QMessageBox::Ok | QMessageBox::Cancel))
    {
      cancelled = true;
      disconnect(pushButton, SIGNAL(clicked()), this, SLOT(Cancel()));
      pushButton->setEnabled(false);
      emit ProgressChanged();
    }
  }
  catch (const MiKTeXException & e)
  {
    ReportError(e);
  }
  catch (const exception & e)
  {
    ReportError(e);
  }
}
