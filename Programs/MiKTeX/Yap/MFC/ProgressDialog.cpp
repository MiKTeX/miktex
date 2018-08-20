/* ProgressDialog.cpp:

   Copyright (C) 2000-2018 Christian Schenk

   This file is part of Yap.

   Yap is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Yap is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Yap; if not, write to the Free Software Foundation, 59
   Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include "StdAfx.h"

#include "yap.h"

#include "ProgressDialog.h"

using namespace std;

class ProgressUIDialog :
  public CDialog
{
private:
  enum { IDD = IDD_PROGRESS };

protected:
  DECLARE_MESSAGE_MAP();

protected:
  void DoDataExchange(CDataExchange* pDX) override;

protected:
  void PostNcDestroy() override;

protected:
  void OnCancel() override;

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

void ProgressUIDialog::DoDataExchange(CDataExchange* pDX)
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

class ProgressUIThread :
  public CWinThread
{
public:
  ProgressUIThread()
  {
  }

public:
  ProgressUIThread(HWND hwndParent, CEvent* pReady);

public:
  HWND GetProgressWindow();

public:
  virtual BOOL InitInstance();

protected:
  HWND hParentWindow = nullptr;

protected:
  CEvent* pReadyEvent = nullptr;

protected:
  HWND hWindow = nullptr;

protected:
  CCriticalSection criticalSectionMonitor;
};

ProgressUIThread::ProgressUIThread(HWND hwndParent, CEvent* pReady) :
  hParentWindow(hwndParent),
  pReadyEvent(pReady)
{
}

BOOL ProgressUIThread::InitInstance()
{
  try
  {
    ProgressUIDialog* pDlg = new ProgressUIDialog;
    m_pMainWnd = pDlg;
    MIKTEX_ASSERT(hParentWindow == nullptr || IsWindow(hParentWindow));
    CWnd* pWnd = nullptr;
    if (hParentWindow != nullptr)
    {
      pWnd = CWnd::FromHandle(hParentWindow);
    }
    if (!pDlg->Create(IDD_PROGRESS, pWnd))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CDialog::Create");
    }
    hWindow = pDlg->GetSafeHwnd();
    MIKTEX_ASSERT(hWindow != nullptr);
    pReadyEvent->SetEvent();
    return TRUE;
  }
  catch (const exception&)
  {
    return FALSE;
  }
}

HWND ProgressUIThread::GetProgressWindow()
{
  CSingleLock singleLock(&criticalSectionMonitor, TRUE);
  return hWindow;
}

class ProgressDialogImpl :
  public ProgressDialog
{
public:
  ~ProgressDialogImpl() noexcept;

public:
  bool HasUserCancelled() override;

public:
  bool SetLine(unsigned lineNum, const string& text) override;

public:
  bool SetTitle(const string& text) override;

public:
  bool StartProgressDialog(HWND hwndParent) override;

public:
  bool StopProgressDialog() override;

private:
  ProgressUIThread* pThread = nullptr;

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
  catch (const exception&)
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

bool ProgressDialogImpl::SetLine(unsigned lineNum, const string& text)
{
  MIKTEX_ASSERT(lineNum > 0 && lineNum <= 2);
  if (pThread == nullptr)
  {
    lines[lineNum - 1] = text;
    return true;
  }
  HWND hwndLine = ::GetDlgItem(hWindow, lineNum == 1 ? IDC_LINE1 : IDC_LINE2);
  if (hwndLine == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetDlgItem");
  }
  if (!SetWindowText(hwndLine, UT_(text)))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("SetWindowText");
  }
  return true;
}

bool ProgressDialogImpl::SetTitle(const string& text)
{
  if (pThread == nullptr)
  {
    title = text;
    return true;
  }
  if (!SetWindowText(hWindow, UT_(text)))
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

  CWnd* pParent = nullptr;

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
  SetLine(1, lines[0]);
  SetLine(2, lines[1]);

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

ProgressDialog* ProgressDialog::Create()
{
  return new ProgressDialogImpl();
}

ProgressDialog::~ProgressDialog()
{
}
