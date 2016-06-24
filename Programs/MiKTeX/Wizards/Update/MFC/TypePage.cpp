/* TypePage.cpp:

   Copyright (C) 2002-2016 Christian Schenk

   This file is part of the MiKTeX Update Wizard.

   The MiKTeX Update Wizard is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Update Wizard is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Update Wizard; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"
#include "Update.h"

#include "TypePage.h"
#include "UpdateWizard.h"

#include "ConnectionSettingsDialog.h"
#include "ProxyAuthenticationDialog.h"

IMPLEMENT_DYNCREATE(TypePage, CPropertyPage);

BEGIN_MESSAGE_MAP(TypePage, CPropertyPage)
  ON_BN_CLICKED(IDC_INSTALL_FROM_CD, &TypePage::OnInstallFromCd)
  ON_BN_CLICKED(IDC_INSTALL_FROM_INTERNET, &TypePage::OnInstallFromInternet)
  ON_BN_CLICKED(IDC_INSTALL_FROM_LOCAL_REPOSITORY,
    &TypePage::OnInstallFromLocalRepository)
  ON_BN_CLICKED(IDC_CONNECTION_SETTINGS, &TypePage::OnConnectionSettings)
  ON_BN_CLICKED(IDC_MIKTEX_NEXT, &TypePage::OnMiKTeXNext)
  ON_BN_CLICKED(IDC_RANDOM, &TypePage::OnNearestRepository)
  ON_BN_CLICKED(IDC_LAST_USED_REPOSITORY, &TypePage::OnLastUsedRepository)
  ON_BN_CLICKED(IDC_CHOOSE_REPOSITORY, &TypePage::OnChooseRepository)
END_MESSAGE_MAP();

const int SOURCE_CHOICE_REMOTE = 0;
const int SOURCE_CHOICE_LOCAL = 1;
const int SOURCE_CHOUCE_CD = 2;

const int REMOTE_CHOICE_NEAREST = 0;
const int REMOTE_CHOICE_LAST_USED = 1;
const int REMOTE_CHOICE_SELECT = 2;

const int LOCAL_CHOICE_LAST_USED = 0;
const int LOCAL_CHOICE_SELECT = 1;

TypePage::TypePage() :
  CPropertyPage(IDD, 0, IDS_HEADER_TYPE, IDS_SUBHEADER_TYPE, 0)
{
  m_psp.dwFlags &= ~PSP_HASHELP;

  remoteChoice = -1;
  localChoice = -1;

  sourceChoice = session->GetConfigValue("Update", "lastSource", -1);

  if (sourceChoice < 0)
  {
    RepositoryType repositoryType(RepositoryType::Unknown);
    string urlOrPath;
    if (PackageManager::TryGetDefaultPackageRepository(repositoryType, urlOrPath))
    {
      switch (repositoryType)
      {
      case RepositoryType::Remote:
        sourceChoice = SOURCE_CHOICE_REMOTE;
        remoteChoice = REMOTE_CHOICE_LAST_USED;
        break;
      case RepositoryType::Local:
        sourceChoice = SOURCE_CHOICE_LOCAL;
        localChoice = LOCAL_CHOICE_LAST_USED;
        break;
      case RepositoryType::MiKTeXDirect:
        sourceChoice = SOURCE_CHOUCE_CD;
        break;
      }
    }
  }

  if (sourceChoice < 0)
  {
    sourceChoice = SOURCE_CHOICE_REMOTE;
  }

  if (remoteChoice < 0)
  {
    remoteChoice = session->GetConfigValue("Update", "lastRemote", REMOTE_CHOICE_NEAREST);
  }

  if (localChoice < 0)
  {
    localChoice = session->GetConfigValue("Update", "lastLocal", LOCAL_CHOICE_LAST_USED);
  }

  haveRemoteRepository = UpdateWizardApplication::packageManager->TryGetRemotePackageRepository(remoteRepository, repositoryReleaseState);

  if (haveRemoteRepository && remoteChoice == REMOTE_CHOICE_SELECT)
  {
    remoteChoice = REMOTE_CHOICE_LAST_USED;
  }

  if (!haveRemoteRepository && remoteChoice == REMOTE_CHOICE_LAST_USED)
  {
    remoteChoice = REMOTE_CHOICE_SELECT;
  }

  haveLocalRepository = UpdateWizardApplication::packageManager->TryGetLocalPackageRepository(localRepository);

  if (haveLocalRepository && localChoice == LOCAL_CHOICE_SELECT)
  {
    localChoice = LOCAL_CHOICE_LAST_USED;
  }

  if (!haveLocalRepository && localChoice == LOCAL_CHOICE_LAST_USED)
  {
    localChoice = LOCAL_CHOICE_SELECT;
  }

  isMiKTeXNextSelected = haveRemoteRepository && repositoryReleaseState == RepositoryReleaseState::Next ? TRUE : FALSE;
  isMiKTeXNextSelected = isMiKTeXNextSelected
    || sourceChoice == SOURCE_CHOICE_REMOTE && remoteChoice == REMOTE_CHOICE_NEAREST && session->GetConfigValue("Update", "lastNearestState", "stable") == "next";
}

BOOL TypePage::OnInitDialog()
{
  pSheet = reinterpret_cast<UpdateWizard*>(GetParent());
  BOOL ret = CPropertyPage::OnInitDialog();
  try
  {
    if (haveRemoteRepository)
    {
      string protocol;
      string host;
      SplitUrl(remoteRepository, protocol, host);
      CString text;
      text.Format(T_(_T("%s://%s (last used)")),
        static_cast<LPCTSTR>(UT_(protocol.c_str())),
        static_cast<LPCTSTR>(UT_(host.c_str())));
      GetControl(IDC_LAST_USED_REPOSITORY)->SetWindowText(text);
    }

    if (haveLocalRepository)
    {
      CString text;
      text.Format(T_(_T("%s (last used)")), static_cast<LPCTSTR>(UT_(localRepository.Get())));
      GetControl(IDC_LAST_USED_DIRECTORY)->SetWindowText(text);
    }

    EnableButtons();
  }
  catch (const MiKTeXException & e)
  {
    pSheet->ReportError(e);
  }
  catch (const exception & e)
  {
    pSheet->ReportError(e);
  }
  return ret;
}

BOOL TypePage::OnSetActive()
{
  noDdv = false;
  if (UpdateWizardApplication::upgrading)
  {
    pSheet->SetWizardButtons(PSWIZB_NEXT);
  }
  else
  {
    pSheet->SetWizardButtons(PSWIZB_NEXT | PSWIZB_BACK);
  }
  return CPropertyPage::OnSetActive();
}

void TypePage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Radio(pDX, IDC_INSTALL_FROM_INTERNET, sourceChoice);
  DDX_Radio(pDX, IDC_RANDOM, remoteChoice);
  DDX_Radio(pDX, IDC_LAST_USED_DIRECTORY, localChoice);
  DDX_Check(pDX, IDC_MIKTEX_NEXT, isMiKTeXNextSelected);
}

LRESULT TypePage::OnWizardNext()
{
  try
  {
    ASSERT(IDC_INSTALL_FROM_INTERNET < IDC_INSTALL_FROM_CD);
    int controlId = GetCheckedRadioButton(IDC_INSTALL_FROM_INTERNET, IDC_INSTALL_FROM_CD);
    int nextPage = -1;
    if (controlId == IDC_INSTALL_FROM_INTERNET)
    {
      ASSERT(IDC_RANDOM < IDC_CHOOSE_REPOSITORY);
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
      int controlId2 =
        GetCheckedRadioButton(IDC_RANDOM, IDC_CHOOSE_REPOSITORY);
      if (controlId2 == IDC_RANDOM)
      {
        nextPage = IDD_PACKAGE_LIST;
      }
      else if (controlId2 == IDC_LAST_USED_REPOSITORY)
      {
        RepositoryInfo repositoryInfo =
          UpdateWizardApplication::packageManager->VerifyPackageRepository(remoteRepository);
        if (repositoryInfo.delay > 0
          && (AfxMessageBox(T_(_T("The last used pacakage repository does not contain the latest packages.\r\n\r\nContinue anyway?")), MB_YESNO) != IDYES))
        {
          return -1;
        }
        nextPage = IDD_PACKAGE_LIST;
      }
      else if (controlId2 == IDC_CHOOSE_REPOSITORY)
      {
        nextPage = IDD_REMOTE_REPOSITORY;
      }
    }
    else if (controlId == IDC_INSTALL_FROM_LOCAL_REPOSITORY)
    {
      ASSERT(IDC_LAST_USED_DIRECTORY < IDC_SPECIFY_DIRECTORY);
      int controlId2 = GetCheckedRadioButton(IDC_LAST_USED_DIRECTORY, IDC_SPECIFY_DIRECTORY);
      if (controlId2 == IDC_LAST_USED_DIRECTORY)
      {
        nextPage = IDD_PACKAGE_LIST;
      }
      else if (controlId2 == IDC_SPECIFY_DIRECTORY)
      {
        nextPage = IDD_LOCAL_REPOSITORY;
      }
    }
    else if (controlId == IDC_INSTALL_FROM_CD)
    {
      nextPage = IDD_CD;
    }
    pSheet->SetCameFrom(IDD);
    return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(nextPage));
  }
  catch (const MiKTeXException & e)
  {
    pSheet->ReportError(e);
    return -1;
  }
  catch (const exception & e)
  {
    pSheet->ReportError(e);
    return -1;
  }
}

LRESULT TypePage::OnWizardBack()
{
  noDdv = true;
  return CPropertyPage::OnWizardBack();
}

BOOL TypePage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  if (ret && !noDdv)
  {
    try
    {
      session->SetConfigValue("Update", "lastSource", sourceChoice);
      session->SetConfigValue("Update", "lastRemote", remoteChoice);
      session->SetConfigValue("Update", "lastLocal", localChoice);
      if (sourceChoice == SOURCE_CHOICE_REMOTE && remoteChoice == REMOTE_CHOICE_NEAREST)
      {
        session->SetConfigValue("Update", "lastNearestState", isMiKTeXNextSelected ? "next" : "stable");
      }
      pSheet->SetRandomRepositoryFlag(remoteChoice == REMOTE_CHOICE_NEAREST);
      pSheet->SetRepositoryType(sourceChoice == SOURCE_CHOICE_REMOTE
        ? RepositoryType::Remote
        : (sourceChoice == SOURCE_CHOICE_LOCAL
          ? RepositoryType::Local
          : RepositoryType::MiKTeXDirect));
      pSheet->SetRepositoryReleaseState(isMiKTeXNextSelected ? RepositoryReleaseState::Next : RepositoryReleaseState::Stable);
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

void TypePage::OnInstallFromInternet()
{
  try
  {
    sourceChoice = SOURCE_CHOICE_REMOTE;
    EnableButtons();
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

void TypePage::OnInstallFromLocalRepository()
{
  try
  {
    sourceChoice = SOURCE_CHOICE_LOCAL;
    EnableButtons();
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

void TypePage::OnInstallFromCd()
{
  try
  {
    sourceChoice = SOURCE_CHOUCE_CD;
    EnableButtons();
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

void TypePage::EnableButtons()
{
  EnableControl(IDC_RANDOM, sourceChoice == SOURCE_CHOICE_REMOTE);
  EnableControl(
    IDC_LAST_USED_REPOSITORY,
    sourceChoice == SOURCE_CHOICE_REMOTE && haveRemoteRepository
    && (isMiKTeXNextSelected ? repositoryReleaseState == RepositoryReleaseState::Next : repositoryReleaseState != RepositoryReleaseState::Next));
  EnableControl(IDC_CHOOSE_REPOSITORY, sourceChoice == SOURCE_CHOICE_REMOTE);
  EnableControl(IDC_CONNECTION_SETTINGS, sourceChoice == SOURCE_CHOICE_REMOTE);
  EnableControl(IDC_LAST_USED_DIRECTORY, sourceChoice == SOURCE_CHOICE_LOCAL && haveLocalRepository);
  EnableControl(IDC_SPECIFY_DIRECTORY, sourceChoice == SOURCE_CHOICE_LOCAL);
  EnableControl(IDC_MIKTEX_NEXT, sourceChoice == SOURCE_CHOICE_REMOTE && remoteChoice != REMOTE_CHOICE_LAST_USED);
}

void TypePage::EnableControl(UINT controlId, bool enable)
{
  GetControl(controlId)->EnableWindow(enable ? TRUE : FALSE);
}

CWnd * TypePage::GetControl(UINT controlId)
{
  CWnd * pWnd = GetDlgItem(controlId);
  if (pWnd == 0)
  {
    MIKTEX_UNEXPECTED();
  }
  return pWnd;
}

void TypePage::OnConnectionSettings()
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

void TypePage::OnMiKTeXNext()
{
  CWnd * pWnd = GetDlgItem(IDC_MIKTEX_NEXT);
  if (pWnd == 0)
  {
    MIKTEX_UNEXPECTED();
  }
  isMiKTeXNextSelected = ((CButton*)pWnd)->GetCheck() == BST_CHECKED;
  if (isMiKTeXNextSelected && !isMiKTeXNextWarningIssued)
  {
    AfxMessageBox(UT_(T_("You have chosen to get untested packages. Although every effort has been made to ensure the correctness of these packages, a hassle-free operation cannot be guaranteed.\r\n\r\nPlease visit http://miktex.org/kb/miktex-next, for more information.")), MB_OK | MB_ICONWARNING);
    isMiKTeXNextWarningIssued = true;
  }
  EnableButtons();
}

void TypePage::OnNearestRepository()
{
  try
  {
    remoteChoice = REMOTE_CHOICE_NEAREST;
    EnableButtons();
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

void TypePage::OnLastUsedRepository()
{
  try
  {
    remoteChoice = REMOTE_CHOICE_LAST_USED;
    EnableButtons();
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

void TypePage::OnChooseRepository()
{
  try
  {
    remoteChoice = REMOTE_CHOICE_SELECT;
    EnableButtons();
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
