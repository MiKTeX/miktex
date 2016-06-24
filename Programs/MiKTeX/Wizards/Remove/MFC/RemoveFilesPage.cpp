/* RemoveFilesPage.cpp:

   Copyright (C) 2000-2016 Christian Schenk

   This file is part of the Remove MiKTeX! Wizard.

   The Remove MiKTeX! Wizard is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The Remove MiKTeX! Wizard is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the Remove MiKTeX! Wizard; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"
#include "Remove.h"

#include "RemoveFilesPage.h"
#include "RemoveWizard.h"

const UINT nIDTimer = 314;

#define WM_REMOVEFILES (WM_USER + 100)

BEGIN_MESSAGE_MAP(RemoveFilesPage, CPropertyPage)

  ON_WM_TIMER()
  ON_MESSAGE(WM_REMOVEFILES, OnStartRemovingFiles)

END_MESSAGE_MAP();

RemoveFilesPage::RemoveFilesPage() :
  CPropertyPage(IDD, 0, IDS_HEADER_REMOVE_FILES, IDS_SUBHEADER_REMOVE_FILES)
{
  m_psp.dwFlags &= ~PSP_HASHELP;
}

RemoveFilesPage::~RemoveFilesPage()
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

BOOL RemoveFilesPage::OnInitDialog()
{
  pSheet = reinterpret_cast<RemoveWizard *>(GetParent());
  return CPropertyPage::OnInitDialog();
}

BOOL RemoveFilesPage::OnSetActive()
{
  BOOL ret = CPropertyPage::OnSetActive();

  if (ret)
  {
    try
    {
      // disable buttons
      pSheet->SetWizardButtons(0);

      // starting shot
      if (!PostMessage(WM_REMOVEFILES))
      {
        MIKTEX_FATAL_WINDOWS_ERROR("CWnd::PostMessage");
      }
    }
    catch (const MiKTeXException & e)
    {
      pSheet->ReportError(e);
      ret = FALSE;
    }
    catch (const exception & e)
    {
      pSheet->ReportError(e);
      ret = FALSE;
    }
  }

  return ret;
}

void RemoveFilesPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_PROGRESS, progressControl);
}

LRESULT RemoveFilesPage::OnStartRemovingFiles(WPARAM wParam, LPARAM lParam)
{
  UNUSED_ALWAYS(wParam);
  UNUSED_ALWAYS(lParam);

  try
  {
    progress = 0;
    total = 0;
    ready = false;

    // create the worker thread
    CWinThread * pThread = AfxBeginThread(WorkerThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
    MIKTEX_ASSERT(pThread != nullptr);
    MIKTEX_ASSERT(pThread->m_hThread != nullptr);
    if (!DuplicateHandle(GetCurrentProcess(), pThread->m_hThread, GetCurrentProcess(), &hWorkerThread, 0, FALSE, DUPLICATE_SAME_ACCESS))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("DuplicateHandle");
    }
    pThread->ResumeThread();

    if (SetTimer(nIDTimer, 100, nullptr) == 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CWnd::SetTime");
    }
  }
  catch (const MiKTeXException & e)
  {
    pSheet->ReportError(e);
  }
  catch (const exception & e)
  {
    pSheet->ReportError(e);
  }

  return 0;
}

UINT RemoveFilesPage::WorkerThread(void * pParam)
{
  RemoveFilesPage * This = reinterpret_cast<RemoveFilesPage *>(pParam);
  This->RemoveMiKTeX();
  This->SetReadyFlag();
  return 0;
};

void RemoveFilesPage::RemoveMiKTeX()
{
  pSetupService = SetupService::Create();
  SetupOptions setupOptions = pSetupService->GetOptions();
  setupOptions.IsThoroughly = pSheet->GetThoroughlyFlag();
  setupOptions.Task = SetupTask::Uninstall;
  pSetupService->SetOptions(setupOptions);
  pSetupService->SetCallback(this);
  pSetupService->Run();
};

BOOL RemoveFilesPage::OnQueryCancel()
{
  if (hWorkerThread == nullptr)
  {
    return CPropertyPage::OnQueryCancel();
  }
  try
  {
    if (AfxMessageBox(IDS_CANCEL, MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
    {
      pSheet->SetCancelFlag();
    }
  }
  catch (const MiKTeXException & e)
  {
    pSheet->ReportError(e);
  }
  catch (const exception & e)
  {
    pSheet->ReportError(e);
  }
  return FALSE;
}

void RemoveFilesPage::OnTimer(UINT_PTR nIDEvent)
{
  try
  {
    CSingleLock singleLock(&criticalSectionMonitor, TRUE);
    CWnd * pWnd = GetDlgItem(IDC_CURRENT_FILE);
    if (pWnd == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    pWnd->SetWindowText(currentFileName);
    if (total != 0)
    {
      progressControl.SetPos(static_cast<int>((static_cast<double>(progress) / total) * 100));
    }
    if (ready || pSheet->GetCancelFlag())
    {
      // notify user
      CString prompt;
      if (!prompt.LoadString(IDS_READY))
      {
        MIKTEX_UNEXPECTED();
      }
      CWnd * pWnd = GetDlgItem(IDC_PROMPT);
      if (pWnd == nullptr)
      {
        MIKTEX_UNEXPECTED();
      }
      pWnd->SetWindowText(prompt);

      // disable progress bars
      pWnd = GetDlgItem(IDC_CURRENT_FILE);
      if (pWnd == nullptr)
      {
        MIKTEX_UNEXPECTED();
      }
      pWnd->SetWindowText(_T(""));
      pWnd->EnableWindow(FALSE);
      progressControl.SetPos(0);
      progressControl.EnableWindow(FALSE);

      // enable Next button
      pSheet->SetWizardButtons(PSWIZB_NEXT);

      KillTimer(nIDTimer);
    }
    else
    {
      CPropertyPage::OnTimer(nIDEvent);
    }
  }
  catch (const MiKTeXException & e)
  {
    pSheet->ReportError(e);
  }
  catch (const exception & e)
  {
    pSheet->ReportError(e);
  }
}

void RemoveFilesPage::SetReadyFlag()
{
  CSingleLock singleLock(&criticalSectionMonitor, TRUE);
  ready = true;
}

bool RemoveFilesPage::GetCancelFlag()
{
  return pSheet->GetCancelFlag();
}

bool RemoveFilesPage::OnProcessOutput(const void * pOutput, size_t n)
{
  return !(pSheet->GetErrorFlag() || pSheet->GetCancelFlag());
}

void RemoveFilesPage::ReportLine(const string & str)
{
}

bool RemoveFilesPage::OnRetryableError(const string & message)
{
  return false;
}

bool RemoveFilesPage::OnProgress(MiKTeX::Setup::Notification nf)
{
  CSingleLock singleLock(&criticalSectionMonitor, TRUE);
  SetupService::ProgressInfo progressInfo = pSetupService->GetProgressInfo();
  currentFileName = UW_(progressInfo.fileName.Get());
  this->progress = progressInfo.cFilesRemoveCompleted;
  this->total = progressInfo.cFilesRemoveTotal;
  return !(pSheet->GetErrorFlag() || pSheet->GetCancelFlag());
}
