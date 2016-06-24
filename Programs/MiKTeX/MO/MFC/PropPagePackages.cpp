/* PropPagePackages.cpp:

   Copyright (C) 2000-2016 Christian Schenk

   This file is part of MiKTeX Options.

   MiKTeX Options is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX Options is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Options; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include "StdAfx.h"
#include "mo.h"

#include "PropPagePackages.h"
#include "PropSheet.h"
#include "resource.hm"

PropPagePackages::PropPagePackages(std::shared_ptr<MiKTeX::Packages::PackageManager> pManager) :
  CPropertyPage(PropPagePackages::IDD),
  pManager(pManager),
  pPackageTreeCtrl(PackageTreeCtrl::Create(pManager))
{
  m_psp.dwFlags &= ~(PSP_HASHELP);
  string repository;
  RepositoryType repositoryType(RepositoryType::Unknown);
  if (pManager->TryGetDefaultPackageRepository(repositoryType, repository))
  {
    this->url = UT_(repository);
  }
  else
  {
    this->url = T_(_T("<Random package repository>"));
  }
}

PropPagePackages::~PropPagePackages()
{
  try
  {
    PackageTreeCtrl::Destroy(pPackageTreeCtrl);
  }
  catch (const exception &)
  {
  }
}

void PropPagePackages::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_TREE, *pPackageTreeCtrl);
  DDX_Text(pDX, IDC_URL, url);
}

BEGIN_MESSAGE_MAP(PropPagePackages, CPropertyPage)
  ON_BN_CLICKED(IDC_CHANGE_URL, OnChangeUrl)
  ON_BN_CLICKED(IDC_PACKAGE_MANAGER, OnBnClickedPackageManager)
  ON_MESSAGE(PackageTreeCtrl::WM_ONTOGGLE, OnToggle)
  ON_NOTIFY(TVN_GETINFOTIP, IDC_TREE, OnGetInfoTip)
  ON_NOTIFY(TVN_SELCHANGED, IDC_TREE, OnSelchangedTree)
  ON_WM_CONTEXTMENU()
  ON_WM_HELPINFO()
END_MESSAGE_MAP();

BOOL PropPagePackages::OnInitDialog()
{
  BOOL ret = CPropertyPage::OnInitDialog();
  try
  {
    CWaitCursor cur;
    pPackageTreeCtrl->Refresh();
    ShowPackageInfo(0);
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

BOOL PropPagePackages::OnApply()
{
  try
  {
    vector<string> toBeInstalled;
    vector<string> toBeRemoved;
    pPackageTreeCtrl->GetDelta(toBeInstalled, toBeRemoved);
    if (toBeInstalled.size() == 0 && toBeRemoved.size() == 0)
    {
      return CPropertyPage::OnApply();
    }
    CString str1;
    str1.Format(_T("%u"), toBeInstalled.size());
    CString str2;
    str2.Format(_T("%u"), toBeRemoved.size());
    CString str;
    AfxFormatString2(str, IDP_UPDATE_MESSAGE, str1, str2);
#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
    if (WindowsVersion::IsWindowsVistaOrGreater() && session->IsAdminMode())
    {
      DllProc4<HRESULT, const TASKDIALOGCONFIG *, int *, int *, BOOL *> taskDialogIndirect("comctl32.dll", "TaskDialogIndirect");
      TASKDIALOGCONFIG taskDialogConfig;
      memset(&taskDialogConfig, 0, sizeof(taskDialogConfig));
      taskDialogConfig.cbSize = sizeof(TASKDIALOGCONFIG);
      taskDialogConfig.hwndParent = nullptr;
      taskDialogConfig.hInstance = nullptr;
      taskDialogConfig.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION;
      taskDialogConfig.pszMainIcon = MAKEINTRESOURCEW(TD_SHIELD_ICON);
      taskDialogConfig.pszWindowTitle = MAKEINTRESOURCEW(AFX_IDS_APP_TITLE);
      taskDialogConfig.pszMainInstruction = T_(L"Do you want to proceed?");
      CStringW strContent(str);
      taskDialogConfig.pszContent = strContent;
      taskDialogConfig.cButtons = 2;
      TASKDIALOG_BUTTON const buttons[] = {
        {IDOK, T_(L"Proceed")},
        {IDCANCEL, T_(L"Cancel")}
      };
      taskDialogConfig.pButtons = buttons;
      taskDialogConfig.nDefaultButton = IDOK;
      int result = 0;
      if (SUCCEEDED(taskDialogIndirect(&taskDialogConfig, &result, nullptr, nullptr)))
      {
        if (IDOK != result)
        {
          return FALSE;
        }
      }
      else
      {
        MIKTEX_UNEXPECTED();
      }
    }
    else
#endif
    {
      if (AfxMessageBox(str, MB_OKCANCEL | MB_ICONINFORMATION) == IDCANCEL)
      {
        return FALSE;
      }
    }
    if (UpdateDialog::DoModal(this, pManager, toBeInstalled, toBeRemoved) == IDOK)
    {
      SetElevationRequired(false);
      return TRUE;
    }
    else
    {
      SetElevationRequired(true);
      return FALSE;
    }
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

#define MAKE_ID_HID_PAIR(id) id, H##id

namespace
{
  const DWORD aHelpIDs[] = {
    MAKE_ID_HID_PAIR(IDC_CHANGE_URL),
    MAKE_ID_HID_PAIR(IDC_INFO),
    MAKE_ID_HID_PAIR(IDC_INSTALLED),
    MAKE_ID_HID_PAIR(IDC_PACKAGE_MANAGER),
    MAKE_ID_HID_PAIR(IDC_PACKAGE_SIZE),
    MAKE_ID_HID_PAIR(IDC_PACKED),
    MAKE_ID_HID_PAIR(IDC_TREE),
    MAKE_ID_HID_PAIR(IDC_URL),
    MAKE_ID_HID_PAIR(IDC_VERSION),
    0, 0,
  };
}

BOOL PropPagePackages::OnHelpInfo(HELPINFO * pHelpInfo)
{
  return ::OnHelpInfo(pHelpInfo, aHelpIDs, "PackagesPage.txt");
}

void PropPagePackages::OnContextMenu(CWnd * pWnd, CPoint point)
{
  try
  {
    DoWhatsThisMenu(pWnd, point, aHelpIDs, "PackagesPage.txt");
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

void PropPagePackages::OnSelchangedTree(NMHDR * pNMHDR, LRESULT * pResult)
{
  try
  {
    NM_TREEVIEW * pNMTreeView = reinterpret_cast<NM_TREEVIEW*>(pNMHDR);
    MIKTEX_ASSERT(pNMTreeView != nullptr);
    PackageInfo packageInfo;
    *pResult = 0;
    if (pNMTreeView->itemNew.hItem == nullptr)
    {
      return;
    }
    if (pPackageTreeCtrl->GetPackageInfo(pNMTreeView->itemNew.hItem, packageInfo))
    {
      ShowPackageInfo(&packageInfo);
    }
    else
    {
      ShowPackageInfo(0);
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

void PropPagePackages::SetWindowText(UINT controlId, const char * lpszText)
{
  CWnd * pWnd = GetDlgItem(controlId);
  if (pWnd == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  ::SetWindowTextW(pWnd->GetSafeHwnd(), UW_(lpszText));
}

void PropPagePackages::ShowPackageInfo(const PackageInfo * pPackageInfo)
{
  if (pPackageInfo == nullptr)
  {
    SetWindowText(IDC_INFO, "");
    SetWindowText(IDC_INSTALLED, "");
    SetWindowText(IDC_PACKAGE_SIZE, "");
    SetWindowText(IDC_VERSION, "");
  }
  else
  {
    SetWindowText(IDC_INFO, pPackageInfo->title.c_str());
    SetWindowText(IDC_VERSION, pPackageInfo->version.c_str());
    if (pPackageInfo->timeInstalled > 0)
    {
      CTime timeInstall(pPackageInfo->timeInstalled);
      SetWindowText(IDC_INSTALLED, TU_(timeInstall.Format(_T("%x %X"))));
    }
    else
    {
      SetWindowText(IDC_INSTALLED, T_("<not installed>"));
    }
    if (!pPackageInfo->IsPureContainer() && pPackageInfo->timePackaged > 0)
    {
      CTime timePackaged(pPackageInfo->timePackaged);
      SetWindowText(IDC_PACKED, TU_(timePackaged.Format(_T("%x %X"))));
    }
    else
    {
      SetWindowText(IDC_PACKED, "");
    }
    CString text;
    text.Format(T_(_T("%u Bytes")), (pPackageInfo->sizeRunFiles + pPackageInfo->sizeDocFiles + pPackageInfo->sizeSourceFiles));
    SetWindowText(IDC_PACKAGE_SIZE, TU_(text));
  }
}

LRESULT PropPagePackages::OnToggle(WPARAM wParam, LPARAM lParam)
{
  UNUSED_ALWAYS(wParam);
  UNUSED_ALWAYS(lParam);
  SetChanged(true);
  return 0;
}

void PropPagePackages::OnChangeUrl()
{
  try
  {
    if (SiteWizSheet::DoModal(this) == ID_WIZFINISH)
    {
      string repository;
      RepositoryType repositoryType = pManager->GetDefaultPackageRepository(repository);
      this->url = UT_(repository);
      SetWindowText(IDC_URL, TU_(this->url));
      Scan();
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

void PropPagePackages::Scan()
{
  pManager->UnloadDatabase();
  unique_ptr<PackageInstaller> pInstaller(pManager->CreateInstaller());
  pInstaller->UpdateDbAsync();
  auto_ptr<ProgressDialog> pProgDlg(ProgressDialog::Create());
  pProgDlg->SetTitle(T_("Package Database Maintenance"));
  pProgDlg->SetLine(1, T_("Downloading package database..."));
  pProgDlg->SetLine(2, "");
  pProgDlg->StartProgressDialog(GetSafeHwnd());
  while (!pProgDlg->HasUserCancelled())
  {
    PackageInstaller::ProgressInfo progressinfo = pInstaller->GetProgressInfo();
    if (progressinfo.ready)
    {
      break;
    }
    Sleep(1000);
  }
  if (!pProgDlg->HasUserCancelled())
  {
    pPackageTreeCtrl->Refresh();
    ShowPackageInfo(nullptr);
  }
  pProgDlg->StopProgressDialog();
  pInstaller->Dispose();
}

void PropPagePackages::OnBnClickedPackageManager()
{
  try
  {
    PathName path;
    if (!session->FindFile(session->IsAdminMode() ? MIKTEX_MPM_MFC_ADMIN_EXE : MIKTEX_MPM_MFC_EXE, FileType::EXE, path))
    {
      MIKTEX_UNEXPECTED();
    }
    Process::Start(path);
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

void PropPagePackages::OnGetInfoTip(NMHDR * pNMHDR, LRESULT * pResult)
{
  try
  {
    NMTVGETINFOTIP * pInfoTip = reinterpret_cast<NMTVGETINFOTIP*>(pNMHDR);
    MIKTEX_ASSERT(pInfoTip != nullptr);
    *pResult = 0;
    PackageInfo pi;
    if (!pPackageTreeCtrl->GetPackageInfo(pInfoTip->hItem, pi))
    {
      return;
    }
#if 0
    set<string> directories;
    for (vector<string>::const_iterator it = pi.runFiles.begin(); it != pi.runFiles.end(); ++it)
    {
      PathName path(it->c_str());
      if (!path.HasExtension(_T(".tpm")))
      {
        directories.insert(path.RemoveFileSpec().Get());
      }
    }
    for (vector<string>::const_iterator it = pi.docFiles.begin(); it != pi.docFiles.end(); ++it)
    {
      PathName path(it->c_str());
      directories.insert(path.RemoveFileSpec().Get());
    }
#endif
    string info;
    if (!pi.title.empty())
    {
      info += pi.title;
    }
    if (!pi.description.empty())
    {
      if (!info.empty())
      {
        info += "\r\n\r\n";
        info += pi.description;
      }
    }
#if 0
    if (directories.size() > 0)
    {
      if (!info.empty())
      {
        info += _T("\r\n\r\n");
      }
      for (set<string>::const_iterator it = directories.begin(); it != directories.end(); ++it)
      {
        if (it != directories.begin())
        {
          info += _T("\r\n");
        }
        info += *it;
      }
    }
#endif
    _tcsncpy_s(pInfoTip->pszText, pInfoTip->cchTextMax, UT_(info), _TRUNCATE);
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


void PropPagePackages::SetElevationRequired(bool f)
{
  if (WindowsVersion::IsWindowsVistaOrGreater() && session->IsAdminMode())
  {
    HWND hwnd = ::GetDlgItem(::GetParent(m_hWnd), IDOK);
    if (hwnd == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    Button_SetElevationRequiredState(hwnd, f ? TRUE : FALSE);
    hwnd = ::GetDlgItem(::GetParent(m_hWnd), ID_APPLY_NOW);
    if (hwnd == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    Button_SetElevationRequiredState(hwnd, f ? TRUE : FALSE);
  }
}

void PropPagePackages::SetChanged(bool f)
{
  SetElevationRequired(f);
  SetModified(f ? TRUE : FALSE);
}
