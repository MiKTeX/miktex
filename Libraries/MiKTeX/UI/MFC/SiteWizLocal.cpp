/* SiteWizLocal.cpp:

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

#include "SiteWizLocal.h"
#include "resource.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::MFC;
using namespace std;

SiteWizLocal::SiteWizLocal(shared_ptr<PackageManager> pManager) :
  CPropertyPage(SiteWizLocal::IDD, IDS_SITEWIZ),
  pManager(pManager)
{
  m_psp.dwFlags &= ~PSP_HASHELP;

  PathName path;

  if (pManager->TryGetLocalPackageRepository(path))
  {
    directory = path.Get();
  }
}

void SiteWizLocal::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_FILENAME, directory);
}

BEGIN_MESSAGE_MAP(SiteWizLocal, CPropertyPage)
  ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
  ON_EN_CHANGE(IDC_FILENAME, OnChangeDirectory)
END_MESSAGE_MAP();

BOOL SiteWizLocal::OnSetActive()
{
  CPropertySheet * pSheet = reinterpret_cast<CPropertySheet*>(GetParent());
  ASSERT_KINDOF(CPropertySheet, pSheet);
  GetDlgItem(IDC_FILENAME)->SetWindowText(directory);
  if (!directory.IsEmpty())
  {
    pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);
  }
  else
  {
    pSheet->SetWizardButtons(PSWIZB_BACK);
  }
  return CPropertyPage::OnSetActive();
}

void SiteWizLocal::OnBrowse()
{
  try
  {
    BROWSEINFO browseInfo;
    ZeroMemory(&browseInfo, sizeof(browseInfo));
    browseInfo.hwndOwner = GetSafeHwnd();
    browseInfo.pszDisplayName = nullptr;
    CString title;
    VERIFY(title.LoadString(IDS_BROWSE_LOCAL));
    browseInfo.lpszTitle = title;
    browseInfo.ulFlags = BIF_RETURNONLYFSDIRS;
    LPCITEMIDLIST pidl = SHBrowseForFolder(&browseInfo);
    if (pidl == nullptr)
    {
      return;
    }
    _TCHAR path[BufferSizes::MaxPath];
    BOOL havePath = SHGetPathFromIDList(pidl, path);
    CoTaskMemFree(const_cast<LPITEMIDLIST>(pidl));
    if (!havePath)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("SHGetPathFromIDList");
    }
    CPropertySheet * pSheet = reinterpret_cast<CPropertySheet*>(GetParent());
    ASSERT_KINDOF(CPropertySheet, pSheet);
    directory = path;
    UpdateData(FALSE);
    pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);
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

void SiteWizLocal::OnChangeDirectory()
{
  try
  {
    CWnd * pWnd = reinterpret_cast<CEdit*>(GetDlgItem(IDC_FILENAME));
    if (pWnd == nullptr)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CWnd::GetDlgItem");
    }
    CPropertySheet * pSheet = reinterpret_cast<CPropertySheet*>(GetParent());
    ASSERT_KINDOF(CPropertySheet, pSheet);
    if (pWnd->GetWindowTextLength() > 0)
    {
      pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);
    }
    else
    {
      pSheet->SetWizardButtons(PSWIZB_BACK);
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

BOOL SiteWizLocal::OnWizardFinish()
{
  try
  {
    if (!Directory::Exists(PathName(directory.GetString())))
    {
      CString prompt;
      AfxFormatString1(prompt, IDP_NOT_A_FOLDER, directory);
      MIKTEX_FATAL_ERROR_2(TU_(prompt.GetString()), "directory", TU_(directory.GetString()));
    }
    else
    {
      if (!(pManager->IsLocalPackageRepository(PathName(directory.GetString()))))
      {
	PathName mpmIni(directory.GetString());
	mpmIni /= "texmf";
	mpmIni /= MIKTEX_PATH_MPM_INI;
	if (!File::Exists(mpmIni))
	{
	  CString prompt;
	  AfxFormatString1(prompt, IDP_NOT_LOCAL_REPOSITORY, directory);
	  MIKTEX_FATAL_ERROR_2(TU_(prompt.GetString()), "directory", TU_(directory.GetString()));
	}
      }
    }
    pManager->SetDefaultPackageRepository(RepositoryType::Local, TU_(directory.GetString()));
    return CPropertyPage::OnWizardFinish();
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
    return FALSE;
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
    return FALSE;
  }
}

LRESULT SiteWizLocal::OnWizardBack()
{
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(IDD_SITEWIZ_TYPE));
}
