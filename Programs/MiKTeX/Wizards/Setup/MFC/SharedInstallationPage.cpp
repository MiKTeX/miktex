/* SharedInstallationPage.cpp:

   Copyright (C) 1999-2016 Christian Schenk

   This file is part of the MiKTeX Setup Wizard.

   The MiKTeX Setup Wizard is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Setup Wizard is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Setup Wizard; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"

#include "Setup.h"

#include "SetupWizard.h"
#include "SharedInstallationPage.h"

BEGIN_MESSAGE_MAP(SharedInstallationPage, CPropertyPage)
  ON_BN_CLICKED(IDC_SHARED, &SharedInstallationPage::OnShared)
END_MESSAGE_MAP();

SharedInstallationPage::SharedInstallationPage() :
  CPropertyPage(IDD, 0, IDS_HEADER_SHARED, IDS_SUBHEADER_SHARED)
{
}

BOOL SharedInstallationPage::OnInitDialog()
{
  pSheet = reinterpret_cast<SetupWizard*>(GetParent());

  commonUserSetup = (SetupApp::Instance->IsCommonSetup() ? 0 : 1);

  BOOL ret = CPropertyPage::OnInitDialog();

  try
  {
    if (!(session->RunningAsAdministrator() || session->RunningAsPowerUser()))
    {
      CWnd * pWnd = GetDlgItem(IDC_SHARED);
      if (pWnd == nullptr)
      {
	MIKTEX_UNEXPECTED();
      }
      pWnd->EnableWindow(FALSE);
    }
    CWnd * pWnd = GetDlgItem(IDC_JUST_FOR_ME);
    if (pWnd == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    wchar_t szLogonName[30];
    DWORD sizeLogonName = sizeof(szLogonName) / sizeof(szLogonName[0]);
    if (!GetUserNameW(szLogonName, &sizeLogonName))
    {
      if (GetLastError() == ERROR_NOT_LOGGED_ON)
      {
	StringUtil::CopyString(szLogonName, 30, L"unknown user");
      }
      else
      {
	MIKTEX_FATAL_WINDOWS_ERROR("GetUserNameW");
      }
    }
    CString str;
    pWnd->GetWindowText(str);
    str += _T(" ");
    str += szLogonName;
    wchar_t szDisplayName[30];
    DllProc3<BOOLEAN, EXTENDED_NAME_FORMAT, LPWSTR, PULONG> getUserNameExW("Secur32.dll", "GetUserNameExW");
    ULONG sizeDisplayName = sizeof(szDisplayName) / sizeof(szDisplayName[0]);
    if (getUserNameExW(NameDisplay, szDisplayName, &sizeDisplayName))
    {
      str += _T(" (");
      str += szDisplayName;
      str += _T(')');
    }
    pWnd->SetWindowText(str);
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

BOOL SharedInstallationPage::OnSetActive()
{
  pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
  return CPropertyPage::OnSetActive();
}

void SharedInstallationPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Radio(pDX, IDC_SHARED, commonUserSetup);
}

LRESULT SharedInstallationPage::OnWizardNext()
{
  pSheet->PushPage(IDD);
  UINT next;
  if (SetupApp::Instance->Service->GetOptions().IsPrefabricated)
  {
    next = IDD_INSTALLDIR;
  }
  else
  {
    switch (SetupApp::Instance->GetTask())
    {
    case SetupTask::InstallFromCD:
      next = IDD_INSTALLDIR;
      break;
    case SetupTask::InstallFromLocalRepository:
      next = IDD_LOCAL_REPOSITORY;
      break;
    case SetupTask::InstallFromRemoteRepository:
      next = IDD_REMOTE_REPOSITORY;
      break;
    case SetupTask::PrepareMiKTeXDirect:
#if SHOW_FOLDER_PAGE
      next = IDD_FOLDER;
#else
      next = IDD_SETTINGS;
#endif
      break;
    default:
      ASSERT(false);
      __assume (false);
      break;
    }
  }
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(next));
}

LRESULT SharedInstallationPage::OnWizardBack()
{
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(pSheet->PopPage()));
}

BOOL SharedInstallationPage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  if (ret)
  {
    SetupOptions options = SetupApp::Instance->Service->GetOptions();
    options.IsCommonSetup = (commonUserSetup == 0);
    SetupApp::Instance->Service->SetOptions(options);
  }
  return ret;
}

void SharedInstallationPage::OnShared()
{
  try
  {
    if (!(session->RunningAsAdministrator() || session->RunningAsPowerUser()))
    {
      AfxMessageBox(T_(_T("You must have administrator privileges to set up a shared MiKTeX system.")), MB_OK | MB_ICONSTOP);
      commonUserSetup = 1;
      UpdateData(FALSE);
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
