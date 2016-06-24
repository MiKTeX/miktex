/* SettingsPage.cpp:

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

#include "SettingsPage.h"
#include "SetupWizard.h"

BEGIN_MESSAGE_MAP(SettingsPage, CPropertyPage)
END_MESSAGE_MAP();

SettingsPage::SettingsPage() :
  CPropertyPage(IDD, 0, IDS_HEADER_SETTINGS, IDS_SUBHEADER_SETTINGS)
{
}

BOOL SettingsPage::OnInitDialog()
{
  pSheet = reinterpret_cast<SetupWizard *>(GetParent());
  paperSize = SetupApp::Instance->Service->GetOptions().PaperSize.c_str();
  switch (SetupApp::Instance->Service->GetOptions().IsInstallOnTheFlyEnabled)
  {
  case TriState::True:
    installOnTheFly = 0;
    break;
  case TriState::False:
    installOnTheFly = 1;
    break;
  case TriState::Undetermined:
    installOnTheFly = 2;
    break;
  default:
    ASSERT(false);
    __assume (false);
    break;
  }
  BOOL ret = CPropertyPage::OnInitDialog();
  return ret;
}

void SettingsPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_CBString(pDX, IDC_COMBO1, paperSize);
  DDX_CBIndex(pDX, IDC_INSTALL_ON_THE_FLY, installOnTheFly);
}

LRESULT SettingsPage::OnWizardNext()
{
  pSheet->PushPage(IDD);
  UINT next;
  switch (SetupApp::Instance->GetTask())
  {
  case SetupTask::InstallFromCD:
  case SetupTask::InstallFromLocalRepository:
  case SetupTask::InstallFromRemoteRepository:
#if ENABLE_ADDTEXMF
    next = IDD_ADD_TEXMFROOTS;
#else
    next = IDD_INFOLIST;
#endif
    break;
  case SetupTask::PrepareMiKTeXDirect:
    next = IDD_INFOLIST;
    break;
  default:
    ASSERT(false);
    __assume (false);
    break;
  }
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(next));
}

LRESULT SettingsPage::OnWizardBack()
{
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(pSheet->PopPage()));
}

BOOL SettingsPage::OnKillActive()
{
  SetupOptions options = SetupApp::Instance->Service->GetOptions();
  BOOL ret = CPropertyPage::OnKillActive();
  options.PaperSize = TU_(paperSize);
  switch (installOnTheFly)
  {
  case 0:
    options.IsInstallOnTheFlyEnabled = TriState::True;
    break;
  case 1:
    options.IsInstallOnTheFlyEnabled = TriState::False;
    break;
  case 2:
    options.IsInstallOnTheFlyEnabled = TriState::Undetermined;
    break;
  default:
    ASSERT(false);
    __assume (false);
    break;
  }
  SetupApp::Instance->Service->SetOptions(options);
  return ret;
}
