/* InfoListPage.cpp:

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

#include "InfoListPage.h"
#include "SetupWizard.h"

BEGIN_MESSAGE_MAP(InfoListPage, CPropertyPage)
END_MESSAGE_MAP();

InfoListPage::InfoListPage() :
  CPropertyPage(IDD, 0, IDS_HEADER_INFOLIST, IDS_SUBHEADER_INFOLIST)
{
}

BOOL InfoListPage::OnInitDialog()
{
  pSheet = reinterpret_cast<SetupWizard *>(GetParent());
  return CPropertyPage::OnInitDialog();
}

BOOL InfoListPage::OnSetActive()
{
  BOOL ret = CPropertyPage::OnSetActive();

  if (ret)
  {
    try
    {
      CreateReport();
      oldNextText = pSheet->SetNextText(T_(_T("&Start")));
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

void InfoListPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_INFO, info);
}

BOOL InfoListPage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  if (ret)
  {
    try
    {
      pSheet->SetNextText(oldNextText);
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


LRESULT InfoListPage::OnWizardNext()
{
  pSheet->PushPage(IDD);
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(IDD_FILECOPY));
}

LRESULT InfoListPage::OnWizardBack()
{
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(pSheet->PopPage()));
}

void InfoListPage::CreateReport()
{
  CString CRLF("\r\n");
  CString TAB("\t");

  CString packageSet;

  switch (SetupApp::Instance->GetPackageLevel())
  {
  case PackageLevel::Essential:
    packageSet = T_("essential packages");
    break;
  case PackageLevel::Basic:
    packageSet = T_("basic packages");
    break;
  case PackageLevel::Complete:
    packageSet = T_("all packages");
    break;
  default:
    MIKTEX_ASSERT(false);
    __assume (false);
    break;
  }

  info = "";

  switch (SetupApp::Instance->GetTask())
  {
  case SetupTask::Download:
    info += (T_(_T("Download ")) + packageSet + T_(_T(" from ")) + CRLF
      + TAB + UT_(SetupApp::Instance->GetRemotePackageRepository().c_str()) + CRLF
      + T_(_T(" to ")) + CRLF
      + TAB + UT_(SetupApp::Instance->GetLocalPackageRepository().Get()));
    break;
  case SetupTask::InstallFromCD:
    info += (T_(_T("Install ")) + packageSet + T_(_T(" from CD/DVD")));
    break;
  case SetupTask::InstallFromLocalRepository:
    if (SetupApp::Instance->Service->GetOptions().IsPrefabricated)
    {
      info += (T_(_T("Install ")) + packageSet + T_(_T(" to ")) + CRLF
	+ TAB + UT_(SetupApp::Instance->GetInstallRoot().Get()));
    }
    else
    {
      info += (T_(_T("Install ")) + packageSet + T_(_T(" from ")) + CRLF
	+ TAB + UT_(SetupApp::Instance->GetLocalPackageRepository().Get()) + CRLF
	+ T_(_T(" to ")) + CRLF
	+ TAB + UT_(SetupApp::Instance->GetInstallRoot().Get()));
    }
    break;
  case SetupTask::InstallFromRemoteRepository:
    info += (T_(_T("Install ")) + packageSet + T_(_T(" from ")) + CRLF
      + TAB + UT_(SetupApp::Instance->GetRemotePackageRepository().c_str()) + CRLF
      + T_(_T(" to ")) + CRLF
      + TAB + UT_(SetupApp::Instance->GetInstallRoot().Get()));
    break;
  case SetupTask::PrepareMiKTeXDirect:
    info += T_("Prepare to run MiKTeX from ");
    info += PathName(SetupApp::Instance->Service->GetOptions().MiKTeXDirectRoot, "texmf").Get();
    break;
  default:
    MIKTEX_ASSERT(false);
    __assume (false);
    break;
  }

  info += CRLF + CRLF;

  if (SetupApp::Instance->GetTask() != SetupTask::Download)
  {
    if (SetupApp::Instance->GetTask() != SetupTask::PrepareMiKTeXDirect)
    {
      if (SetupApp::Instance->IsPortable())
      {
	info += T_("Install MiKTeX Portable");
      }
      else if (SetupApp::Instance->IsCommonSetup())
      {
	info += T_("Install MiKTeX for all users");
      }
      else
      {
	info += T_("Install MiKTeX only for the current user");
      }
      info += CRLF;
      info += CRLF;
    }
#if SHOW_FOLDER_PAGE
    info += T_("Add shortcuts to ");
    info += SetupApp::Instance->folderName.c_str();
    info += CRLF;
    info += CRLF;
#endif
    info += T_("Preferred paper size is ");
    info += SetupApp::Instance->Service->GetOptions().PaperSize.c_str();
    info += CRLF;
    info += CRLF;
    if (SetupApp::Instance->GetTask() != SetupTask::PrepareMiKTeXDirect
      && SetupApp::Instance->GetTask() != SetupTask::Download)
    {
      switch (SetupApp::Instance->Service->GetOptions().IsInstallOnTheFlyEnabled)
      {
      case TriState::True:
	info += T_("Packages will be installed on-the-fly");
	break;
      case TriState::False:
	info += T_("Packages will not be installed on-the-fly");
	break;
      case TriState::Undetermined:
	info +=
	  T_("Packages will be installed after confirmation by user");
	break;
      }
      info += CRLF;
      info += CRLF;
    }
  }

  CWnd * pWnd = GetDlgItem(IDC_INFO);
  if (pWnd == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  pWnd->SetWindowText(info);
}
