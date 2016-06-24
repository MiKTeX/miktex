/* PackageSet.cpp:

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

#include "PackageSetPage.h"
#include "SetupWizard.h"

BEGIN_MESSAGE_MAP(PackageSetPage, CPropertyPage)
  ON_BN_CLICKED(IDC_SMALL, OnSmall)
  ON_BN_CLICKED(IDC_TOTAL, OnTotal)
END_MESSAGE_MAP();

PackageSetPage::PackageSetPage(bool download) :
  CPropertyPage(download ? IDD_DOWNLOAD : IDD_INSTALL, 0, IDS_HEADER_PACKAGE_SET, IDS_SUBHEADER_PACKAGE_SET)
{
}

BOOL PackageSetPage::OnInitDialog()
{
  pSheet = reinterpret_cast<SetupWizard *>(GetParent());
  switch (SetupApp::Instance->GetPackageLevel())
  {
  case PackageLevel::Basic:
    packageSet = 0;
    break;
  case PackageLevel::Complete:
    packageSet = 1;
    break;
  }
  return CPropertyPage::OnInitDialog();
}

BOOL PackageSetPage::OnSetActive()
{
  BOOL ret = CPropertyPage::OnSetActive();
  if (ret)
  {
    pSheet->SetWizardButtons(PSWIZB_BACK | (packageSet >= 0 ? PSWIZB_NEXT : 0));
  }
  return ret;
}

void PackageSetPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Radio(pDX, IDC_SMALL, packageSet);
}

LRESULT PackageSetPage::OnWizardNext()
{
  pSheet->PushPage(SetupApp::Instance->GetTask() == SetupTask::Download ? IDD_DOWNLOAD : IDD_INSTALL);
  UINT next;
  switch (SetupApp::Instance->GetTask())
  {
  case SetupTask::Download:
    next = IDD_REMOTE_REPOSITORY;
    break;
  case SetupTask::InstallFromCD:
    if (SetupApp::Instance->IsPortable())
    {
      next = IDD_INSTALLDIR;
    }
    else
    {
      next = IDD_SHARED;
    }
    break;
  case SetupTask::InstallFromLocalRepository:
    if (SetupApp::Instance->IsPortable())
    {
      next = IDD_LOCAL_REPOSITORY;
    }
    else
    {
      next = IDD_SHARED;
    }
    break;
  case SetupTask::InstallFromRemoteRepository:
    if (SetupApp::Instance->IsPortable())
    {
      next = IDD_REMOTE_REPOSITORY;
    }
    else
    {
      next = IDD_SHARED;
    }
    break;
  default:
    ASSERT(false);
    __assume (false);
  }
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(next));
}

LRESULT PackageSetPage::OnWizardBack()
{
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(pSheet->PopPage()));
}

BOOL PackageSetPage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  if (ret)
  {
    SetupOptions options = SetupApp::Instance->Service->GetOptions();
    options.PackageLevel = (packageSet == 0 ? PackageLevel::Basic : PackageLevel::Complete);
    SetupApp::Instance->Service->SetOptions(options);
  }
  return ret;
}

void PackageSetPage::OnSmall()
{
  pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
}

void PackageSetPage::OnTotal()
{
  pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
}
