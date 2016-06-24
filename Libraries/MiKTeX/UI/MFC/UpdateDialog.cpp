/* UpdateDialog.cpp:

   Copyright (C) 2000-2016 Christian Schenk

   This file is part of MiKTeX UI Library.

   MiKTeX UI Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX UI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX UI Library; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include "StdAfx.h"

#include "internal.h"

#include "miktex/UI/MFC/ErrorDialog.h"
#include "miktex/UI/MFC/UpdateDialog.h"

#include "resource.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::MFC;
using namespace MiKTeX::Util;
using namespace std;

const SHORT PROGRESS_MAX = 1000;

class UpdateDialogImpl :
  public CDialog,
  public PackageInstallerCallback
{
protected:
  DECLARE_MESSAGE_MAP();

public:
  UpdateDialogImpl(CWnd * pParent, shared_ptr<PackageManager> pManager);

public:
  ~UpdateDialogImpl() override;

public:
  void SetFileLists(const vector<string> & toBeInstalled, const vector<string> & toBeRemoved);

protected:
  BOOL OnInitDialog() override;

protected:
  void DoDataExchange(CDataExchange * pDX) override;

protected:
  void OnCancel() override;

protected:
  afx_msg LRESULT OnStartFileCopy(WPARAM wParam, LPARAM lParam);

protected:
  afx_msg LRESULT OnProgress(WPARAM wParam, LPARAM lParam);

public:
  void MIKTEXTHISCALL ReportLine(const string & str) override;

public:
  bool MIKTEXTHISCALL OnRetryableError(const string & message) override;

public:
  bool MIKTEXTHISCALL OnProgress(Notification nf) override;

private:
  static UINT WorkerThread(void * pParam);

private:
  void DoTheUpdate();

private:
  void Report(bool immediate, const char * lpszFmt, ...);

private:
  void ReportError(const MiKTeXException & e)
  {
    errorOccured = true;
    ErrorDialog::DoModal(this, e);
  }

private:
  void ReportError(const exception & e)
  {
    errorOccured = true;
    ErrorDialog::DoModal(this, e);
  }

private:
  CWnd * GetControl(UINT controlId);

private:
  void EnableControl(UINT controlId, bool enable);

private:
  void FormatControlText(UINT ctrlId, const char * lpszFormat, ...);

private:
  void SetCancelFlag()
  {
    EnableControl(IDCANCEL, false);
    cancelled = true;
  }

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

private:
  CCriticalSection criticalSectionMonitor;

private:
  struct SharedData
  {
    PackageInstaller::ProgressInfo progressInfo;
    CString report;
    DWORD secondsRemaining = 0;
    bool newPackage = false;
    bool ready = false;
    bool reportUpdate = false;
    bool waitingForClickOnClose = false;
    int progress1Pos = 0;
    int progress2Pos = 0;
    string packageName;
  };

private:
  SharedData sharedData;

private:
  shared_ptr<PackageInstaller> pInstaller;

private:
  HANDLE hWorkerThread = nullptr;

private:
  CAnimateCtrl animationControl;

private:
  CEdit reportEditBox;

private:
  CProgressCtrl progressControl1;

private:
  CProgressCtrl progressControl2;

private:
  CString report;

private:
  enum { WM_STARTFILECOPY = WM_APP + 1, WM_PROGRESS };

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> pManager;

private:
  bool errorOccured = false;

private:
  bool cancelled = false;
};

BEGIN_MESSAGE_MAP(UpdateDialogImpl, CDialog)
  ON_MESSAGE(WM_PROGRESS, OnProgress)
  ON_MESSAGE(WM_STARTFILECOPY, OnStartFileCopy)
END_MESSAGE_MAP();

enum { IDD = IDD_MIKTEX_UPDATE };

UpdateDialogImpl::UpdateDialogImpl(CWnd * pParent, std::shared_ptr<MiKTeX::Packages::PackageManager> pManager) :
  CDialog(IDD, pParent),
  pManager(pManager),
  pInstaller(pManager->CreateInstaller())
{
}

UpdateDialogImpl::~UpdateDialogImpl()
{
  try
  {
    if (hWorkerThread != nullptr)
    {
      CloseHandle(hWorkerThread);
      hWorkerThread = nullptr;
    }
    if (pInstaller != nullptr)
    {
      pInstaller->Dispose();
      pInstaller.reset();
    }
  }
  catch (const exception &)
  {
  }
}

BOOL UpdateDialogImpl::OnInitDialog()
{
  BOOL ret = CDialog::OnInitDialog();
  try
  {
    reportEditBox.LimitText(100000);
    if (!PostMessage(WM_STARTFILECOPY))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CWnd::PostMessage");
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
  return ret;
}


void UpdateDialogImpl::DoDataExchange(CDataExchange * pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_ANI, animationControl);
  DDX_Control(pDX, IDC_PROGRESS1, progressControl1);
  DDX_Control(pDX, IDC_PROGRESS2, progressControl2);
  DDX_Control(pDX, IDC_REPORT, reportEditBox);
  DDX_Text(pDX, IDC_REPORT, report);
}

void UpdateDialogImpl::OnCancel()
{
  if (hWorkerThread == nullptr || sharedData.waitingForClickOnClose)
  {
    CDialog::OnCancel();
    return;
  }
  try
  {
    if (AfxMessageBox(IDS_CANCEL_UPDATE, MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
    {
      SetCancelFlag();
      if (!PostMessage(WM_PROGRESS))
      {
	MIKTEX_FATAL_WINDOWS_ERROR("CWnd::PostMessage");
      }
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

LRESULT UpdateDialogImpl::OnStartFileCopy(WPARAM wParam, LPARAM lParam)
{
  UNUSED_ALWAYS(wParam);
  UNUSED_ALWAYS(lParam);

  try
  {
    if (!animationControl.Open(IDA_DOWNLOAD))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CAnimateCtrl::Open");
    }

    if (!animationControl.Play(0, static_cast<unsigned int>(-1), static_cast<unsigned int>(-1)))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CAnimateCtrl::Play");
    }

    // initialize progress bar controls
    progressControl1.SetRange(0, 1000);
    progressControl1.SetPos(0);
    progressControl2.SetRange(0, 1000);
    progressControl2.SetPos(0);

    // create the worker thread
    CWinThread * pThread = AfxBeginThread(WorkerThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
    MIKTEX_ASSERT(pThread != nullptr);
    MIKTEX_ASSERT(pThread->m_hThread != nullptr);
    if (!DuplicateHandle(GetCurrentProcess(), pThread->m_hThread, GetCurrentProcess(), &hWorkerThread, 0, FALSE, DUPLICATE_SAME_ACCESS))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("DuplicateHandle");
    }
    pThread->ResumeThread();
  }
  catch (const MiKTeXException & e)
  {
    ReportError(e);
  }
  catch (const exception & e)
  {
    ReportError(e);
  }

  return 0;
}

LRESULT UpdateDialogImpl::OnProgress(WPARAM wParam, LPARAM lParam)
{
  UNUSED_ALWAYS(wParam);
  UNUSED_ALWAYS(lParam);

  try
  {
    CSingleLock singleLock(&criticalSectionMonitor, TRUE);

    // update the report
    if (sharedData.reportUpdate)
    {
      reportEditBox.SetWindowText(sharedData.report);
      reportEditBox.SetSel(100000, 100000);
      sharedData.reportUpdate = false;
    }

    // do we have to finish?
    if (sharedData.ready || GetCancelFlag() || GetErrorFlag())
    {
      // check to see if we are already ready
      if (sharedData.waitingForClickOnClose)
      {
	return 0;
      }

      sharedData.waitingForClickOnClose = true;

      // stop the video
      if (!animationControl.Stop())
      {
	MIKTEX_FATAL_WINDOWS_ERROR("CAnimateCtrl::Stop");
      }
      animationControl.Close(); // always returns FALSE
      animationControl.ShowWindow(SW_HIDE);

      // disable controls
      EnableControl(IDC_PROGRESS1_TITLE, false);
      FormatControlText(IDC_PACKAGE, "");
      EnableControl(IDC_PACKAGE, false);
      progressControl1.SetPos(0);
      progressControl1.EnableWindow(FALSE);
      EnableControl(IDC_PROGRESS2_TITLE, false);
      progressControl2.SetPos(0);
      progressControl2.EnableWindow(FALSE);
      FormatControlText(IDCANCEL, "%s", T_("Close"));
      EnableControl(IDCANCEL, true);
    }
    else
    {
      // show the package name
      if (sharedData.newPackage)
      {
	FormatControlText(IDC_PACKAGE, "%s", sharedData.packageName.c_str());
	sharedData.newPackage = false;
      }

      // update progress bars
      progressControl1.SetPos(sharedData.progress1Pos);
      progressControl2.SetPos(sharedData.progress2Pos);

      // update "Removed files (packages)"
      FormatControlText(IDC_REMOVED_FILES, "%u (%u)", sharedData.progressInfo.cFilesRemoveCompleted, sharedData.progressInfo.cPackagesRemoveCompleted);

      // update "New files (packages)"
      FormatControlText(IDC_NEW_FILES, "%u (%u)", sharedData.progressInfo.cFilesInstallCompleted, sharedData.progressInfo.cPackagesInstallCompleted);

      // update "Downloaded bytes"
      FormatControlText(IDC_DOWNLOADED_BYTES, "%u", sharedData.progressInfo.cbDownloadCompleted);

      // update "Package"
      FormatControlText(IDC_PACKAGE, "%s", sharedData.progressInfo.displayName.c_str());

      // update "KB/s"
      FormatControlText(IDC_KB_SEC, "%.2f", (static_cast<double>(sharedData.progressInfo.bytesPerSecond) / 1024.0));
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

  return 0;
}

void UpdateDialogImpl::ReportLine(const string & str)
{
  Report(true, "%s\n", str.c_str());
}

bool UpdateDialogImpl::OnRetryableError(const std::string & message)
{
  UNUSED_ALWAYS(message);
  return false;
}

bool UpdateDialogImpl::OnProgress(Notification nf)
{
  CSingleLock singlelock(&criticalSectionMonitor, TRUE);
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
    sharedData.progress1Pos = static_cast<int>(((static_cast<double>(progressInfo.cbPackageDownloadCompleted) / progressInfo.cbPackageDownloadTotal) * PROGRESS_MAX));
    visibleProgress = visibleProgress || (sharedData.progress1Pos != oldValue);
  }
  if (progressInfo.cbDownloadTotal > 0)
  {
    int oldValue = sharedData.progress2Pos;
    sharedData.progress2Pos = static_cast<int>(((static_cast<double>(progressInfo.cbDownloadCompleted) / progressInfo.cbDownloadTotal) * PROGRESS_MAX));
    visibleProgress = visibleProgress || (sharedData.progress2Pos != oldValue);
  }
  DWORD oldValue = sharedData.secondsRemaining;
  sharedData.secondsRemaining = static_cast<DWORD>(progressInfo.timeRemaining / 1000);
  visibleProgress = visibleProgress || (sharedData.secondsRemaining != oldValue);
  if (visibleProgress)
  {
    if (!PostMessage(WM_PROGRESS))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CWnd::PostMessage");
    }
  }
  return !(GetErrorFlag() || GetCancelFlag());
}

UINT UpdateDialogImpl::WorkerThread(void * pParam)
{
  UpdateDialogImpl * This = reinterpret_cast<UpdateDialogImpl*>(pParam);

  HRESULT hr = E_FAIL;

  try
  {
    hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
      MIKTEX_UNEXPECTED();
    }
    This->DoTheUpdate();
  }
  catch (const MiKTeXException & e)
  {
    This->ReportError(e);
  }
  catch (const exception & e)
  {
    This->ReportError(e);
  }

  This->sharedData.ready = true;

  if (SUCCEEDED(hr))
  {
    CoUninitialize();
  }

  try
  {
    if (!This->PostMessage(WM_PROGRESS))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CWnd::PostMessage");
    }
  }
  catch (const MiKTeXException & e)
  {
    This->ReportError(e);
  }
  catch (const exception & e)
  {
    This->ReportError(e);
  }

  return 0;
}

void UpdateDialogImpl::DoTheUpdate()
{
  pInstaller->SetCallback(this);
  pInstaller->InstallRemove();
  if (GetCancelFlag())
  {
    return;
  }
}

void UpdateDialogImpl::Report(bool immediate, const char * lpszFmt, ...)
{
  MIKTEX_ASSERT(lpszFmt != nullptr);
  va_list args;
  va_start(args, lpszFmt);
  CString str = UT_(StringUtil::FormatString(lpszFmt, args));
  va_end(args);
  int len = str.GetLength();
  CSingleLock singlelock(&criticalSectionMonitor, TRUE);
  for (int i = 0; i < len; ++i)
  {
    if (str[i] == '\n' && i > 0 && sharedData.report[i - 1] != '\r')
    {
      sharedData.report += '\r';
    }
    sharedData.report += str[i];
  }
  sharedData.reportUpdate = true;
  if (immediate)
  {
    if (!PostMessage(WM_PROGRESS))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CWnd::PostMessage");
    }
  }
}

CWnd * UpdateDialogImpl::GetControl(UINT controlId)
{
  CWnd * pWnd = GetDlgItem(controlId);
  if (pWnd == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  return pWnd;
}

void UpdateDialogImpl::EnableControl(UINT controlId, bool enable)
{
  GetControl(controlId)->EnableWindow(enable ? TRUE : FALSE);
}

void UpdateDialogImpl::FormatControlText(UINT ctrlId, const char * lpszFormat, ...)
{
  va_list marker;
  va_start(marker, lpszFormat);
  string str = StringUtil::FormatString(lpszFormat, marker);
  va_end(marker);
  GetControl(ctrlId)->SetWindowText(UT_(str));
}

void UpdateDialogImpl::SetFileLists(const vector<string> & toBeInstalled, const vector<string> & toBeRemoved)
{
  pInstaller->SetFileLists(toBeInstalled, toBeRemoved);
}

INT_PTR UpdateDialog::DoModal(CWnd * pParent, std::shared_ptr<MiKTeX::Packages::PackageManager> pManager, const vector<string> & toBeInstalled, const vector<string> & toBeRemoved)
{
  BEGIN_USE_MY_RESOURCES()
  {
    if (Utils::RunningOnAServer())
    {
      GiveBackDialog(pParent);
    }
    string url;
    RepositoryType repositoryType(RepositoryType::Unknown);
    if (toBeInstalled.size() > 0
      && PackageManager::TryGetDefaultPackageRepository(repositoryType, url)
      && repositoryType == RepositoryType::Remote
      && !ProxyAuthenticationDialog(pParent))
    {
      return IDCANCEL;
    }
    UpdateDialogImpl dlg(pParent, pManager);
    dlg.SetFileLists(toBeInstalled, toBeRemoved);
    dlg.DoModal();
    return !(dlg.GetErrorFlag() || dlg.GetCancelFlag()) ? IDOK : IDCANCEL;
  }
  END_USE_MY_RESOURCES();
}
