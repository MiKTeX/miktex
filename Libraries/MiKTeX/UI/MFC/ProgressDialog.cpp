/* ProgressDialog.cpp:

   Copyright (C) 2000-2016 Christian Schenk

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

#include "miktex/UI/MFC/ProgressDialog"

#include "resource.h"

using namespace MiKTeX::UI::MFC;
using namespace std;

class ProgressUIDialog : public CDialog
{
private:
  enum { IDD = IDD_PROGRESS };

protected:
  DECLARE_MESSAGE_MAP();

protected:
  virtual void DoDataExchange(CDataExchange * pDX);

protected:
  virtual void PostNcDestroy();

protected:
  virtual void OnCancel();

protected:
  afx_msg LRESULT HasUserCancelled(WPARAM wParam, LPARAM lParam);

protected:
  afx_msg LRESULT Destroy(WPARAM wParam, LPARAM lParam);

private:
  CCriticalSection criticalSectionMonitor;

private:
  bool cancelled = false;
};

enum {
  DESTROY = WM_APP + 314,
  HASUSERCANCELLED
};

BEGIN_MESSAGE_MAP(ProgressUIDialog, CDialog)
  ON_MESSAGE(DESTROY, Destroy)
  ON_MESSAGE(HASUSERCANCELLED, HasUserCancelled)
END_MESSAGE_MAP();

void ProgressUIDialog::DoDataExchange(CDataExchange * pDX)
{
  CDialog::DoDataExchange(pDX);
}

void ProgressUIDialog::OnCancel()
{
  CSingleLock singleLock(&criticalSectionMonitor, TRUE);
  cancelled = true;
}

LRESULT ProgressUIDialog::HasUserCancelled(WPARAM wParam, LPARAM lParam)
{
  UNUSED_ALWAYS(wParam);
  UNUSED_ALWAYS(lParam);
  CSingleLock singleLock(&criticalSectionMonitor, TRUE);
  return static_cast<LRESULT>(cancelled);
}

LRESULT ProgressUIDialog::Destroy(WPARAM wParam, LPARAM lParam)
{
  UNUSED_ALWAYS(wParam);
  UNUSED_ALWAYS(lParam);
  return static_cast<LRESULT>(DestroyWindow());
}

void ProgressUIDialog::PostNcDestroy()
{
  CDialog::PostNcDestroy();
  delete this;
}

class ProgressUIThread : public CWinThread
{
public:
  ProgressUIThread()
  {
  }

public:
  ProgressUIThread(HWND hwndParent, CEvent * pReady);

public:
  HWND GetProgressWindow();

public:
  virtual BOOL InitInstance();

protected:
  HWND hParentWindow = nullptr;

protected:
  CEvent * pReadyEvent = nullptr;

protected:
  HWND hWindow = nullptr;

protected:
  CCriticalSection criticalSectionMonitor;
};

ProgressUIThread::ProgressUIThread(HWND hwndParent, CEvent * pReady) :
  hParentWindow(hwndParent),
  pReadyEvent(pReady)
{
}

BOOL ProgressUIThread::InitInstance()
{
  try
  {
    ProgressUIDialog * pDlg = new ProgressUIDialog;
    m_pMainWnd = pDlg;
    MIKTEX_ASSERT(hParentWindow == nullptr || IsWindow(hParentWindow));
    CWnd * pWnd = nullptr;
    if (hParentWindow != nullptr)
    {
      pWnd = CWnd::FromHandle(hParentWindow);
    }
    BEGIN_USE_MY_RESOURCES()
    {
      if (!pDlg->Create(IDD_PROGRESS, pWnd))
      {
	MIKTEX_FATAL_WINDOWS_ERROR("CDialog::Create");
      }
    }
    END_USE_MY_RESOURCES();
    hWindow = pDlg->GetSafeHwnd();
    MIKTEX_ASSERT(hWindow != nullptr);
    pReadyEvent->SetEvent();
    return TRUE;
  }
  catch (const exception &)
  {
    return FALSE;
  }
}

HWND ProgressUIThread::GetProgressWindow()
{
  CSingleLock singleLock(&criticalSectionMonitor, TRUE);
  return hWindow;
}

class ProgressDialogImpl : public ProgressDialog
{
public:
  virtual ~ProgressDialogImpl();

public:
  virtual bool HasUserCancelled();

public:
  virtual bool SetLine(unsigned lineNum, const char * lpszText);

public:
  virtual bool SetTitle(const char * lpszTitle);

public:
  virtual bool StartProgressDialog(HWND hwndParent);

public:
  virtual bool StopProgressDialog();

private:
  ProgressUIThread * pThread = nullptr;

private:
  HWND hWindow = nullptr;

private:
  string lines[2];

private:
  string title;

private:
  CEvent readyEvent;
};

ProgressDialogImpl::~ProgressDialogImpl()
{
  try
  {
    StopProgressDialog();
  }
  catch (const exception &)
  {
  }
}

bool ProgressDialogImpl::HasUserCancelled()
{
  if (hWindow == nullptr)
  {
    return false;
  }

  DWORD_PTR res;

  if (!SendMessageTimeoutW(hWindow, HASUSERCANCELLED, 0, 0, 0, 1000, &res))
  {
    if (::GetLastError() != ERROR_SUCCESS)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("SendMessageTimeoutW");
    }
    MIKTEX_FATAL_ERROR(T_("The progress window does not respond."));
  }

  return res ? true : false;
}

bool ProgressDialogImpl::SetLine(unsigned lineNum, const char * lpszText)
{
  MIKTEX_ASSERT(lineNum > 0 && lineNum <= 2);
  if (pThread == nullptr)
  {
    lines[lineNum - 1] = lpszText;
    return true;
  }
  HWND hwndLine = ::GetDlgItem(hWindow, lineNum == 1 ? IDC_LINE1 : IDC_LINE2);
  if (hwndLine == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetDlgItem");
  }
  if (!SetWindowText(hwndLine, UT_(lpszText)))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("SetWindowText");
  }
  return true;
}

bool ProgressDialogImpl::SetTitle(const char * lpszTitle)
{
  if (pThread == nullptr)
  {
    title = lpszTitle;
    return true;
  }
  if (!SetWindowText(hWindow, UT_(lpszTitle)))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("SetWindowText");
  }
  return true;
}

namespace {
  // FIXME
  HWND hParentWindow = nullptr;
  bool haveProgressDialog = false;
}

bool ProgressDialogImpl::StartProgressDialog(HWND hwndParent)
{
  if (haveProgressDialog)
  {
    MIKTEX_UNEXPECTED();
  }

  CWnd * pParent = nullptr;

  if (hwndParent != nullptr)
  {
    pParent = CWnd::FromHandlePermanent(hwndParent);
    if (pParent == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
  }

  pParent = CWnd::GetSafeOwner(pParent, nullptr);

  if (pParent != nullptr)
  {
    hwndParent = pParent->GetSafeHwnd();
  }

  hParentWindow = hwndParent;

  // disable mouse and keyboard input in the parent window
  if (hParentWindow != nullptr)
  {
    EnableWindow(hParentWindow, FALSE);
  }

  // create the user interface thread
  pThread = new ProgressUIThread(nullptr, &readyEvent);
  pThread->CreateThread();

  // wait for the progress window to become available
  CSingleLock singleLock(&readyEvent);
  if (!singleLock.Lock(1000))
  {
    MIKTEX_UNEXPECTED();
  }
  hWindow = pThread->GetProgressWindow();
  MIKTEX_ASSERT(IsWindow(hWindow));
  haveProgressDialog = true;

  // set the window texts
  SetTitle(title.c_str());
  SetLine(1, lines[0].c_str());
  SetLine(2, lines[1].c_str());

  // make the progress window visible
  ShowWindow(hWindow, SW_SHOW);

  return true;
}

bool ProgressDialogImpl::StopProgressDialog()
{
  if (hWindow == nullptr)
  {
    return false;
  }

  HWND hwnd = hWindow;
  hWindow = nullptr;

  // enable mouse and keyboard input in the parent window
  if (hParentWindow != nullptr)
  {
    EnableWindow(hParentWindow, TRUE);
  }

  // destroy the dialog window
  haveProgressDialog = false;
  DWORD_PTR res;
  if (!SendMessageTimeout(hwnd, DESTROY, 0, 0, 0, 1000, &res))
  {
    if (::GetLastError() != ERROR_SUCCESS)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("SendMessageTimeout");
    }
    MIKTEX_FATAL_ERROR(T_("The progress window does not respond."));
  }

  return res ? true : false;
}

ProgressDialog * ProgressDialog::Create()
{
  return new ProgressDialogImpl();
}

ProgressDialog::~ProgressDialog()
{
}
