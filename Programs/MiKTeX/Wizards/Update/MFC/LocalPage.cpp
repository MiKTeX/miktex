/* LocalPage.cpp:

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

#include "LocalPage.h"
#include "UpdateWizard.h"

IMPLEMENT_DYNCREATE(LocalPage, CPropertyPage);

BEGIN_MESSAGE_MAP(LocalPage, CPropertyPage)
  ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
  ON_EN_CHANGE(IDC_FILENAME, OnChangeFilename)
END_MESSAGE_MAP();

LocalPage::LocalPage() :
  CPropertyPage(IDD, 0, IDS_HEADER_LOCAL, IDS_SUBHEADER_LOCAL)
{
  m_psp.dwFlags &= ~PSP_HASHELP;
  PathName fileName;
  if (UpdateWizardApplication::packageManager->TryGetLocalPackageRepository(fileName))
  {
    this->fileName = fileName.Get();
  }
}

BOOL LocalPage::OnInitDialog()
{
  pSheet = reinterpret_cast<UpdateWizard *>(GetParent());
  return CPropertyPage::OnInitDialog();
}

BOOL LocalPage::OnSetActive()
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
      if (!fileName.IsEmpty())
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

void LocalPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_FILENAME, fileName);
}

LRESULT LocalPage::OnWizardNext()
{
  pSheet->SetCameFrom(IDD);
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(IDD_PACKAGE_LIST));
}

LRESULT LocalPage::OnWizardBack()
{
  noDdv = true;
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(IDD_TYPE));
}

BOOL LocalPage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  if (ret && !noDdv)
  {
    try
    {
      if (!(UpdateWizardApplication::packageManager->IsLocalPackageRepository(PathName(fileName))))
      {
        CString message;
        AfxFormatString1(message, IDP_NOT_LOCAL_REPOSITORY, fileName);
        AfxMessageBox(message, MB_OK | MB_ICONSTOP);
        ret = FALSE;
      }
      else
      {
        UpdateWizardApplication::packageManager->SetLocalPackageRepository(PathName(fileName));
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

void LocalPage::OnBrowse()
{
  try
  {
    BROWSEINFOW bi;
    ZeroMemory(&bi, sizeof(bi));
    bi.hwndOwner = GetSafeHwnd();
    bi.pszDisplayName = nullptr;
    CString title;
    if (!title.LoadString(IDS_BROWSE_LOCAL))
    {
      MIKTEX_UNEXPECTED();
    }
    bi.lpszTitle = title;
    bi.ulFlags = BIF_RETURNONLYFSDIRS;
    LPCITEMIDLIST pidl = SHBrowseForFolderW(&bi);
    if (pidl == nullptr)
    {
      return;
    }
    wchar_t szFolderPath[BufferSizes::MaxPath];
    BOOL havePath = SHGetPathFromIDListW(pidl, szFolderPath);
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

void LocalPage::OnChangeFilename()
{
  try
  {
    CWnd * pWnd = reinterpret_cast<CEdit*>(GetDlgItem(IDC_FILENAME));
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
