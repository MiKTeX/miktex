/* FileCopyPage.cpp: the actual setup process

   Copyright (C) 1999-2016 Christian Schenk

   This file is part of MiKTeX Setup Wizard.

   MiKTeX Setup Wizard is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Setup Wizard is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Setup Wizard; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include "StdAfx.h"
#include "Setup.h"

#include "FileCopyPage.h"
#include "SetupWizard.h"

const SHORT PROGRESS_MAX = 1000;

const UINT WM_STARTFILECOPY = WM_APP + 1;
const UINT WM_PROGRESS = WM_APP + 2;
const UINT WM_REPORT = WM_APP + 3;

BEGIN_MESSAGE_MAP(FileCopyPage, CPropertyPage)
  ON_MESSAGE(WM_PROGRESS, OnProgress)
  ON_MESSAGE(WM_REPORT, OnReport)
  ON_MESSAGE(WM_STARTFILECOPY, OnStartFileCopy)
END_MESSAGE_MAP();

FileCopyPage::FileCopyPage() :
  CPropertyPage(IDD, 0, IDS_HEADER_FILECOPY, IDS_SUBHEADER_FILECOPY)
{
}

FileCopyPage::~FileCopyPage()
{
  try
  {
    if (hWorkerThread != nullptr)
    {
      CloseHandle(hWorkerThread);
      hWorkerThread = nullptr;
    }
  }
  catch (const exception &)
  {
  }
}

BOOL FileCopyPage::OnInitDialog()
{
  pSheet = reinterpret_cast<SetupWizard*>(GetParent());
  BOOL ret = CPropertyPage::OnInitDialog();
  reportControl.LimitText(100000);
  try
  {
    session->SetAdminMode(SetupApp::Instance->IsCommonSetup(), true);
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

BOOL FileCopyPage::OnSetActive()
{
  BOOL ret = CPropertyPage::OnSetActive();

  if (ret)
  {
    try
    {
      // disable buttons
      pSheet->SetWizardButtons(0);

#if 0
      if (SetupApp::Instance->GetTask() != SetupTask::Download)
      {
        GetControl(IDC_ETA_TITLE)->ShowWindow(SW_HIDE);
        GetControl(IDC_ETA)->ShowWindow(SW_HIDE);
      }
#endif

      // starting shot
      if (!PostMessage(WM_STARTFILECOPY))
      {
        MIKTEX_FATAL_WINDOWS_ERROR("CWnd::PostMessage");
      }
    }
    catch (const MiKTeXException & e)
    {
      ReportError(e);
      ret = FALSE;
    }
    catch (const exception & e)
    {
      ReportError(e);
      ret = FALSE;
    }
  }

  return TRUE;
}

void FileCopyPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_ANI, animationControl);
  DDX_Control(pDX, IDC_PROGRESS1, progressControl1);
  DDX_Control(pDX, IDC_PROGRESS2, progressControl2);
  DDX_Control(pDX, IDC_REPORT, reportControl);
}

LRESULT FileCopyPage::OnWizardNext()
{
  pSheet->PushPage(IDD);
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(IDD_FINISH));
}

BOOL FileCopyPage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  if (ret)
  {
    try
    {
      SetupApp::Instance->Service->ULogClose(!pSheet->GetErrorFlag());
    }
    catch (const MiKTeXException & e)
    {
      ReportError(e);
      ret = FALSE;
    }
    catch (const exception & e)
    {
      ReportError(e);
      ret = FALSE;
    }
  }
  return ret;
}

BOOL FileCopyPage::OnQueryCancel()
{
  if (hWorkerThread == nullptr)
  {
    return CPropertyPage::OnQueryCancel();
  }
  try
  {
    SetupApp::Instance->Service->Log(T_("\n<<<Cancel? "));
    if (AfxMessageBox((SetupApp::Instance->GetTask() == SetupTask::Download ? IDS_CANCEL_DOWNLOAD : IDS_CANCEL_SETUP), MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
    {
      SetupApp::Instance->Service->Log(T_("Yes!>>>\n"));
      pSheet->SetCancelFlag();
      if (!PostMessage(WM_PROGRESS))
      {
        MIKTEX_FATAL_WINDOWS_ERROR("CWnd::PostMessage");
      }
    }
    else
    {
      SetupApp::Instance->Service->Log(T_("No!>>>\n"));
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
  return FALSE;
}

LRESULT FileCopyPage::OnStartFileCopy(WPARAM wParam, LPARAM lParam)
{
  UNUSED_ALWAYS(wParam);
  UNUSED_ALWAYS(lParam);

  try
  {
    if (!animationControl.Open(SetupApp::Instance->GetTask() == SetupTask::Download ? IDA_DOWNLOAD : IDA_FILECOPY))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CAnimateCtrl::Open");
    }

    if (!animationControl.Play(0, static_cast<UINT>(-1), static_cast<UINT>(-1)))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CAnimateCtrl::Play");
    }

    // initialize progress bar controls
    progressControl1.SetRange(0, PROGRESS_MAX);
    progressControl1.SetPos(0);
    progressControl2.SetRange(0, PROGRESS_MAX);
    progressControl2.SetPos(0);

    if (SetupApp::Instance->GetTask() == SetupTask::Download)
    {
      GetControl(IDC_PROGRESS1_TITLE)->SetWindowText(T_(_T("Downloading:")));
    }

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

LRESULT FileCopyPage::OnProgress(WPARAM wParam, LPARAM lParam)
{
  UNUSED_ALWAYS(wParam);
  UNUSED_ALWAYS(lParam);

  try
  {
    CSingleLock singleLock(&criticalSectionMonitor, TRUE);

    // do we have to finish?
    if (sharedData.ready || pSheet->GetCancelFlag() || pSheet->GetErrorFlag())
    {
      // check to see if we are already ready
      if (sharedData.waitingForClickOnNext)
      {
        return 0;
      }

      sharedData.waitingForClickOnNext = true;

      // close the wizard, if it is running unattended
      if (SetupApp::Instance->IsUnattended)
      {
        SetupApp::Instance->Service->ULogClose(!pSheet->GetErrorFlag());
        EndDialog(IDOK);
      }

      // stop the video
      if (!animationControl.Stop())
      {
        MIKTEX_FATAL_WINDOWS_ERROR("CAnimateCtrl::Stop");
      }
      animationControl.Close(); // always returns FALSE
      animationControl.ShowWindow(SW_HIDE);

      // disable controls
      EnableControl(IDC_PROGRESS1_TITLE, false);
      GetControl(IDC_PACKAGE)->SetWindowText(_T(""));
      EnableControl(IDC_PACKAGE, false);
      progressControl1.SetPos(0);
      progressControl1.EnableWindow(FALSE);
      EnableControl(IDC_PROGRESS2_TITLE, false);
      progressControl2.SetPos(0);
      progressControl2.EnableWindow(FALSE);
#if 0
      GetControl(IDC_ETA_TITLE)->ShowWindow(SW_HIDE);
      GetControl(IDC_ETA)->SetWindowText("");
      GetControl(IDC_ETA)->ShowWindow(SW_HIDE);
#endif

      // enable Next button
      pSheet->SetWizardButtons(PSWIZB_NEXT);
    }
    else
    {
      // show the package name
      if (sharedData.newPackage)
      {
        GetControl(IDC_PACKAGE)->SetWindowText(UW_(sharedData.packageName));
        sharedData.newPackage = false;
      }

      // update progress bars
      progressControl1.SetPos(sharedData.progress1Pos);
      progressControl2.SetPos(sharedData.progress2Pos);

#if 0
      // update ETA
      if (GetTickCount() > timeOfLastProgressRefresh + 1000)
      {
        CTimeSpan timeRemaining(sharedData.secondsRemaining);
        CString str;
        if (timeRemaining.GetTotalHours() > 0)
        {
          str.Format(T_("%u hours, %u minutes"), timeRemaining.GetTotalHours(), timeRemaining.GetMinutes());
        }
        else if (timeRemaining.GetTotalMinutes() > 0)
        {
          str.Format(T_("%u minutes"), timeRemaining.GetTotalMinutes());
        }
        else
        {
          str.Format(T_("%u seconds"), timeRemaining.GetTotalSeconds());
        }
        timeOfLastProgressRefresh = GetTickCount();
        GetControl(IDC_ETA)->SetWindowText(str);
      }
#endif
    }
  }
  catch (const MiKTeXException & e)
  {
    ReportError(e);
    pSheet->SetWizardButtons(PSWIZB_NEXT);
    sharedData.waitingForClickOnNext = true;
  }
  catch (const exception & e)
  {
    ReportError(e);
    pSheet->SetWizardButtons(PSWIZB_NEXT);
    sharedData.waitingForClickOnNext = true;
  }
  return 0;
}

LRESULT FileCopyPage::OnReport(WPARAM wParam, LPARAM lParam)
{
  UNUSED_ALWAYS(lParam);
  long len = reportControl.GetTextLength();
  reportControl.SetSel(len, len);
  reportControl.ReplaceSel(reinterpret_cast<LPCTSTR>(wParam));
  reportControl.SendMessage(EM_SCROLLCARET);
  return 0;
}

bool FileCopyPage::OnProcessOutput(const void * pOutput, size_t n)
{
  Report(true, "%.*s", n, reinterpret_cast<const char *>(pOutput));
  return !(pSheet->GetErrorFlag() || pSheet->GetCancelFlag());
}

void FileCopyPage::ReportLine(const string & str)
{
  Report(true, "%s\n", str.c_str());
}

bool FileCopyPage::OnRetryableError(const string & message)
{
  UINT uType = MB_ICONSTOP;
  uType |= MB_RETRYCANCEL;
  string str = message;
  str += T_("  Then click Retry to complete the operation.");
  UINT u = ::MessageBoxW(nullptr, UW_(str), nullptr, uType);
  return u != IDCANCEL;
}

bool FileCopyPage::OnProgress(MiKTeX::Setup::Notification nf)
{
  CSingleLock singlelock(&criticalSectionMonitor, TRUE);

  bool visibleProgress = false;

  SetupService::ProgressInfo progressInfo = SetupApp::Instance->Service->GetProgressInfo();

  if (nf == MiKTeX::Setup::Notification::InstallPackageStart
    || nf == MiKTeX::Setup::Notification::DownloadPackageStart)
  {
    visibleProgress = true;
    sharedData.newPackage = true;
    sharedData.packageName = progressInfo.displayName;
  }

  if (SetupApp::Instance->GetTask() == SetupTask::Download)
  {
    if (progressInfo.cbPackageDownloadTotal > 0)
    {
      int oldValue = sharedData.progress1Pos;
      sharedData.progress1Pos
        = static_cast<int>(
          ((static_cast<double>(progressInfo.cbPackageDownloadCompleted)
            / progressInfo.cbPackageDownloadTotal)
            * PROGRESS_MAX));
      visibleProgress = visibleProgress || sharedData.progress1Pos != oldValue;
    }
    if (progressInfo.cbDownloadTotal > 0)
    {
      int oldValue = sharedData.progress2Pos;
      sharedData.progress2Pos
        = static_cast<int>(
          ((static_cast<double>(progressInfo.cbDownloadCompleted)
            / progressInfo.cbDownloadTotal)
            * PROGRESS_MAX));
      visibleProgress = visibleProgress || sharedData.progress2Pos != oldValue;
    }
    DWORD oldValue = sharedData.secondsRemaining;
    sharedData.secondsRemaining = static_cast<DWORD>(progressInfo.timeRemaining / 1000);
    visibleProgress = visibleProgress || sharedData.secondsRemaining != oldValue;
  }
  else if ((SetupApp::Instance->GetTask() == SetupTask::InstallFromLocalRepository
    || SetupApp::Instance->GetTask() == SetupTask::InstallFromCD) && progressInfo.cbInstallTotal > 0)
  {
    if (progressInfo.cbPackageInstallTotal > 0)
    {
      int oldValue = sharedData.progress1Pos;
      sharedData.progress1Pos
        = static_cast<int>(
          ((static_cast<double>(progressInfo.cbPackageInstallCompleted)
            / progressInfo.cbPackageInstallTotal)
            * PROGRESS_MAX));
      visibleProgress = visibleProgress || sharedData.progress1Pos != oldValue;
    }
    int oldValue = sharedData.progress2Pos;
    sharedData.progress2Pos
      = static_cast<int>(
        ((static_cast<double>(progressInfo.cbInstallCompleted)
          / progressInfo.cbInstallTotal)
          * PROGRESS_MAX));
    visibleProgress = visibleProgress || sharedData.progress2Pos != oldValue;
  }

  if (visibleProgress)
  {
    if (!PostMessage(WM_PROGRESS))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CWnd::PostMessage");
    }
  }

  return !(pSheet->GetErrorFlag() || pSheet->GetCancelFlag());
}

UINT FileCopyPage::WorkerThread(void * pParam)
{
  FileCopyPage * This = reinterpret_cast<FileCopyPage*>(pParam);

  This->timeOfLastProgressRefresh = 0;

  bool comInit = false;

  try
  {
    // initialize COM
    if (FAILED(CoInitialize(nullptr)))
    {
      MIKTEX_UNEXPECTED();
    }
    comInit = true;

    SetupApp::Instance->Service->SetCallback(This);
    SetupApp::Instance->Service->Run();
  }
  catch (const MiKTeXException & e)
  {
    This->ReportError(e);
  }
  catch (const exception & e)
  {
    This->ReportError(e);
  }

  if (comInit)
  {
    CoUninitialize();
  }

  This->sharedData.ready = true;

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

void FileCopyPage::Report(bool writeLog, const char * lpszFmt, ...)
{
  MIKTEX_ASSERT(lpszFmt != nullptr);
  va_list args;
  va_start(args, lpszFmt);
  string str(StringUtil::FormatString(lpszFmt, args));
  va_end(args);
  int len = str.length();
  CSingleLock singlelock(&criticalSectionMonitor, TRUE);
  string lines;
  for (int i = 0; i < len; ++i)
  {
    if (str[i] == '\n' && i > 0 && str[i] != '\r')
    {
      sharedData.currentLine += '\r';
    }
    sharedData.currentLine += str[i];
    if (str[i] == '\n')
    {
      lines += sharedData.currentLine;
      sharedData.currentLine.clear();
    }
  }
  if (writeLog)
  {
    SetupApp::Instance->Service->Log("%s", str.c_str());
  }
  if (!lines.empty())
  {
    singlelock.Unlock();
    SendMessage(WM_REPORT, reinterpret_cast<WPARAM>(static_cast<LPCTSTR>(UT_(lines))));
  }
}

CWnd * FileCopyPage::GetControl(UINT controlId)
{
  CWnd * pWnd = GetDlgItem(controlId);
  if (pWnd == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  return pWnd;
}

void FileCopyPage::EnableControl(UINT controlId, bool enable)
{
  GetControl(controlId)->EnableWindow(enable ? TRUE : FALSE);
}

void FileCopyPage::ReportError(const MiKTeXException & e)
{
  Report(false, T_("\nError: %s\n"), e.what());
  pSheet->ReportError(e);
}

void FileCopyPage::ReportError(const exception & e)
{
  Report(false, T_("\nError: %s\n"), e.what());
  pSheet->ReportError(e);
}
