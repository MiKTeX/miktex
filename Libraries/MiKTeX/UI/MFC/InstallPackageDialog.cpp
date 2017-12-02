/* InstallPackageDialog.cpp:

   Copyright (C) 2000-2017 Christian Schenk

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
#include "miktex/UI/MFC/SiteWizSheet.h"

#include "InstallPackageDialog.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::MFC;
using namespace MiKTeX::Util;
using namespace std;

InstallPackageDialog::InstallPackageDialog(CWnd* parent, shared_ptr<PackageManager> packageManager, const string& packageName, const string& trigger) :
  CDialog(InstallPackageDialog::IDD, parent),
  pManager(packageManager),
  trigger(UT_(trigger))
{
  PackageInfo packageInfo = pManager->GetPackageInfo(packageName);
  this->packageName = UT_(packageInfo.deploymentName);
  packageTitle = UT_(packageInfo.title);
}

void InstallPackageDialog::FormatControlText(UINT ctrlId, const char * lpszFormat, ...)
{
  CWnd * pWnd = GetDlgItem(ctrlId);
  if (pWnd == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  va_list marker;
  va_start(marker, lpszFormat);
  string str = StringUtil::FormatStringVA(lpszFormat, marker);
  va_end(marker);
  pWnd->SetWindowText(UT_(str.c_str()));
}

BOOL InstallPackageDialog::OnInitDialog()
{
  BOOL ret = CDialog::OnInitDialog();
  try
  {
    shared_ptr<Session> session = Session::Get();
    string repository;
    RepositoryType repositoryType(RepositoryType::Unknown);
    if (pManager->TryGetDefaultPackageRepository(repositoryType, repository))
    {
      FormatControlText(IDC_EDIT_REPOSITORY_LOCATION, "%s", repository.c_str());
    }
    else
    {
      FormatControlText(IDC_EDIT_REPOSITORY_LOCATION, "%s", T_("<Random package repository>"));
    }
    if (session->IsAdminMode())
    {
      HWND hwnd = ::GetDlgItem(m_hWnd, IDOK);
      if (hwnd == nullptr)
      {
	MIKTEX_UNEXPECTED();
      }
      Button_SetElevationRequiredState(hwnd, TRUE);
    }
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

void InstallPackageDialog::DoDataExchange(CDataExchange * pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Check(pDX, IDC_CHECK_ALWAYS_ASK, alwaysAsk);
  DDX_Text(pDX, IDC_PACKAGE_NAME, packageName);
  DDX_Text(pDX, IDC_PACKAGE_FILE, trigger);
  DDX_Text(pDX, IDC_PACKAGE_TITLE, packageTitle);
}

BEGIN_MESSAGE_MAP(InstallPackageDialog, CDialog)
  ON_BN_CLICKED(IDC_BUTTON_CHANGE, OnChangeRepository)
END_MESSAGE_MAP();

void InstallPackageDialog::OnChangeRepository()
{
  try
  {
    if (SiteWizSheet::DoModal(this) != ID_WIZFINISH)
    {
      return;
    }
    string repository;
    RepositoryType repositoryType(RepositoryType::Unknown);
    if (pManager->TryGetDefaultPackageRepository(repositoryType, repository))
    {
      FormatControlText(IDC_EDIT_REPOSITORY_LOCATION, "%s", repository.c_str());
    }
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
