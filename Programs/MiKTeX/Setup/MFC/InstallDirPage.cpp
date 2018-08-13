/* InstallDirPage.cpp:

   Copyright (C) 1999-2018 Christian Schenk

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

#include "InstallDirPage.h"
#include "SetupWizard.h"

BEGIN_MESSAGE_MAP(InstallDirPage, CPropertyPage)
  ON_EN_CHANGE(IDC_PATHNAME, OnChangePathName)
  ON_BN_CLICKED(IDC_BROWSE, &InstallDirPage::OnBrowse)
END_MESSAGE_MAP();

InstallDirPage::InstallDirPage() :
  CPropertyPage(IDD, 0, IDS_HEADER_INSTALLDIR, IDS_SUBHEADER_INSTALLDIR)
{
}

BOOL InstallDirPage::OnInitDialog()
{
  sheet = reinterpret_cast<SetupWizard *>(GetParent());
  installDir = SetupApp::Instance->GetInstallationDirectory().ToWideCharString().c_str();
  return CPropertyPage::OnInitDialog();
}

BOOL InstallDirPage::OnSetActive()
{
  noDdv = false;
  installDir = SetupApp::Instance->GetInstallationDirectory().ToWideCharString().c_str();
  BOOL ret = CPropertyPage::OnSetActive();
  sheet->SetWizardButtons(PSWIZB_BACK | (installDir.IsEmpty() ? 0 : PSWIZB_NEXT));
  return ret;
}

void InstallDirPage::DoDataExchange(CDataExchange* dx)
{
  CPropertyPage::DoDataExchange(dx);

  try
  {
    DDX_Text(dx, IDC_PATHNAME, installDir);
    if (dx->m_bSaveAndValidate && !noDdv)
    {
      installDir.Trim();
      installDir.Replace(_T('/'), _T('\\'));
      if (installDir.GetLength() > 3)
      {
        installDir.TrimRight(_T('\\'));
      }
      DDV_Path(dx, installDir);
    }
  }
  catch (const MiKTeXException& e)
  {
    sheet->ReportError(e);
    if (dx->m_bSaveAndValidate)
    {
      dx->Fail();
    }
  }
  catch (const exception& e)
  {
    sheet->ReportError(e);
    if (dx->m_bSaveAndValidate)
    {
      dx->Fail();
    }
  }
}

LRESULT InstallDirPage::OnWizardNext()
{
  sheet->PushPage(IDD);
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(IDD_SETTINGS));
}

LRESULT InstallDirPage::OnWizardBack()
{
  noDdv = true;
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(sheet->PopPage()));
}

BOOL InstallDirPage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  if (ret && !noDdv)
  {
    try
    {
      PathName dir(installDir.GetString());
      if (Directory::Exists(dir))
      {
        unique_ptr<DirectoryLister> lister = DirectoryLister::Open(dir);
        DirectoryEntry entry;
        if (lister->GetNext(entry))
        {
          AfxMessageBox(IDS_INSTALL_DIR_NOT_EMPTY, MB_OK | MB_ICONSTOP);
          ret = FALSE;
        }
      }
      if (ret)
      {
        SetupApp::Instance->SetInstallationDirectory(installDir.GetString());
      }
  }
    catch (const MiKTeXException& e)
    {
      sheet->ReportError(e);
      ret = FALSE;
    }
    catch (const exception& e)
    {
      sheet->ReportError(e);
      ret = FALSE;
    }
}
  return ret;
}

void InstallDirPage::OnChangePathName()
{
  try
  {
    CWnd* wnd = GetDlgItem(IDC_PATHNAME);
    if (wnd == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    CString str;
    wnd->GetWindowText(str);
    str.TrimLeft();
    str.TrimRight();
    sheet->SetWizardButtons(PSWIZB_BACK | (str.IsEmpty() ? 0 : PSWIZB_NEXT));
  }
  catch (const MiKTeXException& e)
  {
    sheet->ReportError(e);
  }
  catch (const exception& e)
  {
    sheet->ReportError(e);
  }
}

INT CALLBACK InstallDirPage::BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM pData)
{
  UNUSED_ALWAYS(lParam);
  UNUSED_ALWAYS(pData);

  try
  {
#if 0
    InstallDirPage* This = reinterpret_cast<InstallDirPage*>(pData);
#endif
    switch (uMsg)
    {
    case BFFM_INITIALIZED:
    {
      CString root(Utils::GetFolderPath(CSIDL_PROGRAM_FILES, CSIDL_PROGRAM_FILES, true).ToWideCharString().c_str());
      ::SendMessageW(hwnd, BFFM_SETSELECTION, TRUE, reinterpret_cast<LPARAM>(root.GetString()));
      ::SendMessageW(hwnd, BFFM_SETEXPANDED, TRUE, reinterpret_cast<LPARAM>(root.GetString()));
    }
    default:
      return 0;
    }
  }
  catch (const exception &)
  {
    return 0;
  }
}

void InstallDirPage::OnBrowse()
{
  try
  {
    BROWSEINFOW browseInfo;
    ZeroMemory(&browseInfo, sizeof(browseInfo));
    browseInfo.hwndOwner = GetSafeHwnd();
    browseInfo.ulFlags = BIF_USENEWUI | BIF_RETURNONLYFSDIRS;
    browseInfo.lpfn = BrowseCallbackProc;
    browseInfo.lParam = reinterpret_cast<LPARAM>(this);
    browseInfo.lpszTitle = T_(L"Select the MiKTeX installation directory:");
    LPITEMIDLIST pidl = SHBrowseForFolderW(&browseInfo);
    if (pidl == nullptr)
    {
      return;
    }
    WCHAR szDir[BufferSizes::MaxPath];
    BOOL done = SHGetPathFromIDListW(pidl, szDir);
    CoTaskMemFree(pidl);
    if (!done)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("SHGetPathFromIDListW");
    }
    CWnd* wnd = GetDlgItem(IDC_PATHNAME);
    if (wnd == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    wnd->SetWindowText(szDir);
    OnChangePathName();
  }
  catch (const MiKTeXException& e)
  {
    sheet->ReportError(e);
  }
  catch (const exception& e)
  {
    sheet->ReportError(e);
  }
}
