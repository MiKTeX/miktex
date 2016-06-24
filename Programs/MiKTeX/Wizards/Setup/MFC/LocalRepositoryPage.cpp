/* LocalRepositoryPag.cpp:

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

#include "LocalRepositoryPage.h"
#include "SetupWizard.h"

BEGIN_MESSAGE_MAP(LocalRepositoryPage, CPropertyPage)
  ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
  ON_EN_CHANGE(IDC_FILENAME, OnChangePathName)
END_MESSAGE_MAP()

LocalRepositoryPage::LocalRepositoryPage() :
  CPropertyPage(IDD, 0, IDS_HEADER_LOCAL_REPOSITORY, IDS_SUBHEADER_LOCAL_REPOSITORY)
{
}

BOOL LocalRepositoryPage::OnInitDialog()
{
  pSheet = reinterpret_cast<SetupWizard *>(GetParent());
  fileName = SetupApp::Instance->GetLocalPackageRepository().Get();
  return CPropertyPage::OnInitDialog();
}

BOOL LocalRepositoryPage::OnSetActive()
{
  noDdv = false;
  BOOL ret = CPropertyPage::OnSetActive();
  if (ret)
  {
    try
    {
      CWnd * pWnd = GetDlgItem(IDC_FILENAME);
      if (pWnd == nullptr)
      {
	MIKTEX_UNEXPECTED();
      }
      pWnd->SetWindowText(fileName);
      if (fileName.GetLength() > 0)
      {
	pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
      }
      else
      {
	pSheet->SetWizardButtons(PSWIZB_BACK);
      }
      pWnd = GetDlgItem(IDC_MESSAGE);
      if (pWnd == nullptr)
      {
	MIKTEX_UNEXPECTED();
      }
      if (SetupApp::Instance->GetTask() == SetupTask::Download)
      {
	pWnd->SetWindowText(T_(_T("Download the MiKTeX distribution to:")));
      }
      else
      {
	pWnd->SetWindowText(T_(_T("Install MiKTeX from:")));
      }
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

void LocalRepositoryPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_FILENAME, fileName);
}

LRESULT LocalRepositoryPage::OnWizardNext()
{
  pSheet->PushPage(IDD);
  UINT next;
  switch (SetupApp::Instance->GetTask())
  {
  case SetupTask::Download:
    next = IDD_INFOLIST;
    break;
  case SetupTask::InstallFromLocalRepository:
  case SetupTask::InstallFromRemoteRepository:
    next = IDD_INSTALLDIR;
    break;
  default:
    ASSERT(false);
    __assume (false);
    break;
  }
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(next));
}

LRESULT LocalRepositoryPage::OnWizardBack()
{
  noDdv = true;
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(pSheet->PopPage()));
}

BOOL LocalRepositoryPage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  if (ret && !noDdv)
  {
    try
    {
      if (SetupApp::Instance->GetTask() == SetupTask::InstallFromLocalRepository)
      {
	PackageLevel foundPackageLevel = SetupService::TestLocalRepository(PathName(fileName), SetupApp::Instance->GetPackageLevel());
	if (foundPackageLevel == PackageLevel::None)
	{
	  CString message;
	  AfxFormatString1(message, IDP_NOT_LOCAL_REPOSITORY, fileName);
	  AfxMessageBox(message, MB_OK | MB_ICONSTOP);
	  ret = FALSE;
	}
      }
      if (ret)
      {
	SetupOptions options = SetupApp::Instance->Service->GetOptions();
	options.LocalPackageRepository = static_cast<LPCTSTR>(fileName);
	SetupApp::Instance->Service->SetOptions(options);
      }
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

void LocalRepositoryPage::OnBrowse()
{
  try
  {
    BROWSEINFO bi;
    ZeroMemory(&bi, sizeof(bi));
    bi.hwndOwner = GetSafeHwnd();
    bi.pszDisplayName = nullptr;
    CString title;
    if (!title.LoadString(IDS_BROWSE_LOCAL))
    {
      MIKTEX_UNEXPECTED();
    }
    bi.lpszTitle = title;
    bi.ulFlags = (BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS);
    if (SetupApp::Instance->GetTask() == SetupTask::InstallFromLocalRepository)
    {
      bi.ulFlags |= BIF_NONEWFOLDERBUTTON;
    }
    LPCITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl == nullptr)
    {
      return;
    }
    _TCHAR szFolderPath[BufferSizes::MaxPath];
    BOOL havePath = SHGetPathFromIDList(pidl, szFolderPath);
    CoTaskMemFree(const_cast<LPITEMIDLIST>(pidl));
    if (!havePath)
    {
      MIKTEX_UNEXPECTED();
    }
    fileName = szFolderPath;
    UpdateData(FALSE);
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

void LocalRepositoryPage::OnChangePathName()
{
  try
  {
    CWnd * pWnd = GetDlgItem(IDC_FILENAME);
    if (pWnd == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    if (pWnd->GetWindowTextLength() > 0)
    {
      pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
    }
    else
    {
      pSheet->SetWizardButtons(PSWIZB_BACK);
    }
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
