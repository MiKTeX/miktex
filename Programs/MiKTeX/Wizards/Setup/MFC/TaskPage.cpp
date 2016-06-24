/* TaskPage.cpp:

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
#include "TaskPage.h"

#include "ConnectionSettingsDialog.h"
#include "ProxyAuthenticationDialog.h"

BEGIN_MESSAGE_MAP(TaskPage, CPropertyPage)
  ON_BN_CLICKED(IDC_DOWNLOAD_ONLY, OnDownloadOnly)
  ON_BN_CLICKED(IDC_INSTALL_FROM_LOCAL_REPOSITORY, OnInstallFromLocalRepository)
  ON_BN_CLICKED(IDC_INSTALL_FROM_REMOTE_REPOSITORY, OnInstallFromRemoteRepository)
  ON_BN_CLICKED(IDC_CONNECTION_SETTINGS, &TaskPage::OnConnectionSettings)
END_MESSAGE_MAP();

TaskPage::TaskPage() :
  CPropertyPage(IDD, 0, IDS_HEADER_TASK, IDS_SUBHEADER_TASK)
{
}

BOOL TaskPage::OnInitDialog()
{
  BOOL ret = TRUE;
  pSheet = reinterpret_cast<SetupWizard*>(GetParent());
  try
  {
    switch (SetupApp::Instance->GetTask())
    {
    case SetupTask::Download:
      task = 0;
      break;
    case SetupTask::InstallFromLocalRepository:
      task = 1;
      break;
    }
    ret = CPropertyPage::OnInitDialog();
    EnableButtons();
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

BOOL TaskPage::OnSetActive()
{
  BOOL ret = CPropertyPage::OnSetActive();
  if (ret)
  {
    pSheet->SetWizardButtons(PSWIZB_BACK | (task >= 0 ? PSWIZB_NEXT : 0));
  }
  return ret;
}

void TaskPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Radio(pDX, IDC_DOWNLOAD_ONLY, task);
}

LRESULT TaskPage::OnWizardNext()
{
  pSheet->PushPage(IDD);
  UINT next;
#if 0
  // TODO
  const int iLast = IDC_INSTALL_FROM_REMOTE_REPOSITORY;
#else
  const int iLast = IDC_INSTALL_FROM_LOCAL_REPOSITORY;
#endif
  try
  {
    switch (GetCheckedRadioButton(IDC_DOWNLOAD_ONLY, iLast))
    {
    case IDC_DOWNLOAD_ONLY:
    {
      ProxySettings proxySettings;
      if (PackageManager::TryGetProxy(proxySettings)
	&& proxySettings.useProxy
	&& proxySettings.authenticationRequired
	&& proxySettings.user.empty())
      {
	ProxyAuthenticationDialog dlg(this);
	if (dlg.DoModal() != IDOK)
	{
	  return -1;
	}
	proxySettings.user = dlg.GetName();
	proxySettings.password = dlg.GetPassword();
	PackageManager::SetProxy(proxySettings);
      }
      next = IDD_PACKAGE_SET_DOWNLOAD;
    }
    break;
    case IDC_INSTALL_FROM_LOCAL_REPOSITORY:
      next = IDD_PACKAGE_SET_INSTALL;
      break;
    default:
      MIKTEX_UNEXPECTED();
    }
  }
  catch (const MiKTeXException & e)
  {
    ReportError(e);
    return -1;
  }
  catch (const exception & e)
  {
    ReportError(e);
    return -1;
  }
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(next));
}

LRESULT TaskPage::OnWizardBack()
{
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(pSheet->PopPage()));
}

BOOL TaskPage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  if (ret)
  {
    SetupOptions options = SetupApp::Instance->Service->GetOptions();
    options.Task = (task == 0 ? SetupTask::Download : (task == 1 ? SetupTask::InstallFromLocalRepository : SetupTask::InstallFromRemoteRepository));
    SetupApp::Instance->Service->SetOptions(options);
  }
  return ret;
}

void TaskPage::OnDownloadOnly()
{
  try
  {
    task = 0;
    EnableButtons();
    pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
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

void TaskPage::OnInstallFromLocalRepository()
{
  try
  {
    task = 1;
    EnableButtons();
    pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
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

void TaskPage::OnInstallFromRemoteRepository()
{
  pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
}

void TaskPage::OnConnectionSettings()
{
  try
  {
    ConnectionSettingsDialog dlg(this);
    dlg.DoModal();
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

void TaskPage::EnableButtons()
{
  CWnd * pWnd = GetDlgItem(IDC_CONNECTION_SETTINGS);
  if (pWnd == 0)
  {
    MIKTEX_UNEXPECTED();
  }
  pWnd->EnableWindow(task == 0);
}
