/* SiteWizType.cpp:

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

#include "miktex/UI/MFC/ErrorDialog.h"

#include "SiteWizType.h"
#include "resource.h"

#include "ConnectionSettingsDialog.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::MFC;
using namespace std;

SiteWizType::SiteWizType(shared_ptr<PackageManager> pManager) :
  CPropertyPage(IDD_SITEWIZ_TYPE, IDS_SITEWIZ),
  pManager(pManager)
{
  m_psp.dwFlags &= ~PSP_HASHELP;
  string urlOrPath;
  RepositoryType repositoryType(RepositoryType::Unknown);
  RepositoryReleaseState repositoryReleaseState = RepositoryReleaseState::Unknown;
  if (PackageManager::TryGetDefaultPackageRepository(repositoryType, repositoryReleaseState, urlOrPath))
  {
    switch (repositoryType)
    {
    case RepositoryType::Remote:
      this->repositoryType = 0;
      break;
    case RepositoryType::Local:
      this->repositoryType = 1;
      break;
    case RepositoryType::MiKTeXDirect:
      this->repositoryType = 2;
      break;
    }
  }
  isMiKTeXNextSelected = repositoryReleaseState == RepositoryReleaseState::Next ? BST_CHECKED : BST_UNCHECKED;
}

BEGIN_MESSAGE_MAP(SiteWizType, CPropertyPage)
  ON_BN_CLICKED(IDC_INSTALL_FROM_CD, OnInstallFromCD)
  ON_BN_CLICKED(IDC_INSTALL_FROM_INTERNET, OnInstallFromInternet)
  ON_BN_CLICKED(IDC_INSTALL_FROM_LOCAL_REPOSITORY, OnInstallFromLocalRepository)
  ON_BN_CLICKED(IDC_CONNECTION_SETTINGS, &SiteWizType::OnConnectionSettings)
  ON_BN_CLICKED(IDC_MIKTEX_NEXT, &SiteWizType::OnMiKTeXNext)
END_MESSAGE_MAP();

void SiteWizType::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Radio(pDX, IDC_INSTALL_FROM_INTERNET, repositoryType);
  DDX_Check(pDX, IDC_MIKTEX_NEXT, isMiKTeXNextSelected);
}

BOOL SiteWizType::OnSetActive()
{
  BOOL ret = CPropertyPage::OnSetActive();
  try
  {
    CPropertySheet * pSheet = reinterpret_cast<CPropertySheet*>(GetParent());
    ASSERT_KINDOF(CPropertySheet, pSheet);
    pSheet->SetWizardButtons(repositoryType >= 0 ? PSWIZB_NEXT : 0);
    EnableButtons();
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  return ret;
}

void SiteWizType::OnInstallFromLocalRepository()
{
  try
  {
    CPropertySheet * pSheet = reinterpret_cast<CPropertySheet*>(GetParent());
    ASSERT_KINDOF(CPropertySheet, pSheet);
    pSheet->SetWizardButtons(PSWIZB_NEXT);
    EnableButtons();
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void SiteWizType::OnInstallFromInternet()
{
  try
  {
    CPropertySheet * pSheet = reinterpret_cast<CPropertySheet*>(GetParent());
    ASSERT_KINDOF(CPropertySheet, pSheet);
    pSheet->SetWizardButtons(PSWIZB_NEXT);
    EnableButtons();
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void SiteWizType::OnInstallFromCD()
{
  try
  {
    CPropertySheet * pSheet = reinterpret_cast<CPropertySheet*>(GetParent());
    ASSERT_KINDOF(CPropertySheet, pSheet);
    pSheet->SetWizardButtons(PSWIZB_NEXT);
    EnableButtons();
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

LRESULT SiteWizType::OnWizardNext()
{
  try
  {
    SiteWizSheetImpl * pSheet = reinterpret_cast<SiteWizSheetImpl*>(GetParent());
    ASSERT_KINDOF(SiteWizSheetImpl, pSheet);
    UpdateData();
    ASSERT(IDC_INSTALL_FROM_INTERNET < IDC_INSTALL_FROM_CD);
    int checked = GetCheckedRadioButton(IDC_INSTALL_FROM_INTERNET, IDC_INSTALL_FROM_CD);
    DWORD nextPage;
    if (checked == IDC_INSTALL_FROM_INTERNET)
    {
      if (!ProxyAuthenticationDialog(this))
      {
	return -1;
      }
      nextPage = IDD_SITEWIZ_REMOTE_REPOSITORY;
    }
    else if (checked == IDC_INSTALL_FROM_LOCAL_REPOSITORY)
    {
      nextPage = IDD_SITEWIZ_LOCAL_REPOSITORY;
    }
    else
    {
      nextPage = IDD_SITEWIZ_DRIVE;
    }
    pSheet->SetRepositoryReleaseState(isMiKTeXNextSelected == BST_CHECKED ? RepositoryReleaseState::Next : RepositoryReleaseState::Stable);
    return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(nextPage));
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
    return -1;
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
    return -1;
  }
}

void SiteWizType::OnConnectionSettings()
{
  ConnectionSettingsDialog dlg(this);
  dlg.DoModal();
}

void SiteWizType::EnableButtons()
{
  int checked = GetCheckedRadioButton(IDC_INSTALL_FROM_INTERNET, IDC_INSTALL_FROM_CD);
  CWnd * pWnd = GetDlgItem(IDC_CONNECTION_SETTINGS);
  if (pWnd == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  pWnd->EnableWindow(checked == IDC_INSTALL_FROM_INTERNET);
  pWnd = GetDlgItem(IDC_MIKTEX_NEXT);
  if (pWnd == 0)
  {
    MIKTEX_UNEXPECTED();
  }
  pWnd->EnableWindow(checked == IDC_INSTALL_FROM_INTERNET);
}

void SiteWizType::OnMiKTeXNext()
{
  CWnd * pWnd = GetDlgItem(IDC_MIKTEX_NEXT);
  if (pWnd == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  if (((CButton*)pWnd)->GetCheck() == BST_CHECKED && !isMiKTeXNextWarningIssued)
  {
    AfxMessageBox(UT_(T_("You have chosen to get untested packages. Although every effort has been made to ensure the correctness of these packages, a hassle-free operation cannot be guaranteed.\r\n\r\nPlease visit http://miktex.org/kb/miktex-next, for more information.")), MB_OK | MB_ICONWARNING);
    isMiKTeXNextWarningIssued = true;
  }
}
