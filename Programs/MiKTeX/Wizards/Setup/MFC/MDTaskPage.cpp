/* MDTaskPage.cpp:

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
#include "MDTaskPage.h"

BEGIN_MESSAGE_MAP(MDTaskPage, CPropertyPage)
  ON_BN_CLICKED(IDC_INSTALL, OnInstall)
  ON_BN_CLICKED(IDC_PREPARE_MIKTEXDIRECT, OnPrepareMiKTeXDirect)
END_MESSAGE_MAP();

MDTaskPage::MDTaskPage() :
  CPropertyPage(IDD, 0, IDS_HEADER_TASK, IDS_SUBHEADER_TASK)
{
}

BOOL MDTaskPage::OnInitDialog()
{
  root = SetupApp::Instance->Service->GetOptions().MiKTeXDirectRoot.Get();
  pSheet = reinterpret_cast<SetupWizard *>(GetParent());
  switch (SetupApp::Instance->GetTask())
  {
  case SetupTask::PrepareMiKTeXDirect:
    task = 0;
    break;
  case SetupTask::InstallFromCD:
    task = 1;
    break;
  default:
    ASSERT(false);
    __assume (false);
    break;
  }
  return CPropertyPage::OnInitDialog();
}

BOOL MDTaskPage::OnSetActive()
{
  BOOL ret = CPropertyPage::OnSetActive();
  if (ret)
  {
    pSheet->SetWizardButtons(PSWIZB_BACK | (task >= 0 ? PSWIZB_NEXT : 0));
  }
  return ret;
}

void MDTaskPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Radio(pDX, IDC_PREPARE_MIKTEXDIRECT, task);
  DDX_Text(pDX, IDC_TEXMF_ROOT, root);
}

LRESULT MDTaskPage::OnWizardNext()
{
  pSheet->PushPage(IDD);
  UINT next;
  switch (GetCheckedRadioButton(IDC_PREPARE_MIKTEXDIRECT, IDC_INSTALL))
  {
  case IDC_PREPARE_MIKTEXDIRECT:
    next = IDD_SHARED;
    break;
  case IDC_INSTALL:
    next = IDD_PACKAGE_SET_INSTALL;
    break;
  default:
    ASSERT(false);
    __assume (false);
  }
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(next));
}

LRESULT MDTaskPage::OnWizardBack()
{
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(pSheet->PopPage()));
}

BOOL MDTaskPage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  if (ret)
  {
    SetupApp::Instance->SetTask(task == 0 ? SetupTask::PrepareMiKTeXDirect : SetupTask::InstallFromCD);
  }
  return ret;
}

void MDTaskPage::OnPrepareMiKTeXDirect()
{
  pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
}

void MDTaskPage::OnInstall()
{
  pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
}
