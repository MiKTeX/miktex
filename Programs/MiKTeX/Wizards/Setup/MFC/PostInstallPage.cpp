/* PostInstallPage.cpp:

   Copyright (C) 2018 Christian Schenk

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

#include "PostInstallPage.h"
#include "SetupWizard.h"

BEGIN_MESSAGE_MAP(PostInstallPage, CPropertyPage)
  ON_BN_CLICKED(IDC_CHECK_UPDATES, OnCheckUpdates)
END_MESSAGE_MAP();

PostInstallPage::PostInstallPage() :
  CPropertyPage(IDD, 0, IDS_HEADER_POST_INSTALL, IDS_SUBHEADER_POST_INSTALL)
{
}

BOOL PostInstallPage::OnInitDialog()
{
  sheet = reinterpret_cast<SetupWizard*>(GetParent());
  BOOL ret = CPropertyPage::OnInitDialog();
  if (sheet->PreviousPage() == IDD_FILECOPY)
  {
    sheet->SetWizardButtons(PSWIZB_NEXT);
  }
  return ret;
}

void PostInstallPage::DoDataExchange(CDataExchange* dx)
{
  CPropertyPage::DoDataExchange(dx);
  DDX_Control(dx, IDC_CHECK_UPDATES, checkUpdates);
}

LRESULT PostInstallPage::OnWizardNext()
{
  sheet->PushPage(IDD);
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(IDD_FINISH));
}

LRESULT PostInstallPage::OnWizardBack()
{
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(sheet->PopPage()));
}

BOOL PostInstallPage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  if (ret)
  {
    SetupApp::Instance->CheckUpdatesOnExit = checkUpdates.GetCheck() == BST_CHECKED;
  }
  return ret;
}

void PostInstallPage::OnCheckUpdates()
{
  if (checkUpdates.GetCheck() == BST_UNCHECKED && showMessageBox)
  {
    showMessageBox = false;
    AfxMessageBox(T_(_T("It is recommended that you update your MiKTeX installation as soon as possible. You can use \"MiKTeX Console\" to check for updates.")));
  }
}

