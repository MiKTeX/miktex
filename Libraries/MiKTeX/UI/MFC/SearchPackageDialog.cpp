/* SearchPackageDialog.cpp:

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
#include "miktex/UI/MFC/PropSheetPackage.h"

#include "SearchPackageDialog.h"
#include "resource.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::MFC;
using namespace std;

SearchPackageDialog::SearchPackageDialog(CWnd * pParent, shared_ptr<PackageManager> pManager) :
  CDialog(IDD_SEARCH_PACKAGE, pParent),
  pManager(pManager)
{
}

void SearchPackageDialog::DoDataExchange(CDataExchange * pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LIST_PACKAGES, listControl);
}

BEGIN_MESSAGE_MAP(SearchPackageDialog, CDialog)
  ON_BN_CLICKED(IDC_FIND_NOW, OnBnClickedFindNow)
  ON_EN_CHANGE(IDC_DESCRIPTION, OnEnChangeDescription)
  ON_EN_CHANGE(IDC_FILE, OnEnChangeFile)
  ON_EN_CHANGE(IDC_NAME, OnChangeName)
  ON_EN_CHANGE(IDC_TITLE, OnEnChangeTitle)
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PACKAGES, OnLvnItemchangedListPackages)
  ON_NOTIFY(NM_DBLCLK, IDC_LIST_PACKAGES, OnNMDblclkListPackages)
  ON_NOTIFY(NM_RCLICK, IDC_LIST_PACKAGES, OnNMRclickListPackages)
END_MESSAGE_MAP();

BOOL SearchPackageDialog::OnInitDialog()
{
  BOOL ret = CDialog::OnInitDialog();

  try
  {
    if (listControl.InsertColumn(0, T_(_T("Name")), LVCFMT_LEFT, listControl.GetStringWidth(_T("xxx mmmmmmmm")), 0) < 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::InsertColumn");
    }

    if (listControl.InsertColumn(1, T_(_T("Title")), LVCFMT_LEFT, listControl.GetStringWidth(_T("xxx mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm")), 1) < 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::InsertColumn");
    }

    EnableButtons();
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

void SearchPackageDialog::OnChangeName()
{
  try
  {
    EnableButtons();
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

void SearchPackageDialog::OnEnChangeTitle()
{
  try
  {
    EnableButtons();
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

void SearchPackageDialog::OnEnChangeDescription()
{
  try
  {
    EnableButtons();
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

void SearchPackageDialog::OnEnChangeFile()
{
  try
  {
    EnableButtons();
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

CString SearchPackageDialog::GetWindowText(UINT controlId)
{
  CWnd * pWnd = GetDlgItem(controlId);
  if (pWnd == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CWnd::GetDlgItem");
  }
  CString str;
  pWnd->GetWindowText(str);
  return str;
}

void SearchPackageDialog::EnableButtons()
{
  size_t l = 0;
  l += GetWindowText(IDC_NAME).GetLength();
  l += GetWindowText(IDC_DESCRIPTION).GetLength();
  l += GetWindowText(IDC_TITLE).GetLength();
  l += GetWindowText(IDC_FILE).GetLength();
  l += GetWindowText(IDC_FIND_NOW).GetLength();
  CWnd * pOk = GetDlgItem(IDOK);
  if (pOk == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CWnd::GetDlgItem");
  }
  CWnd * pFindNow = GetDlgItem(IDC_FIND_NOW);
  if (pFindNow == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CWnd::GetDlgItem");
  }
  pOk->EnableWindow(l > 0);
  CButton * pbn;
  if (l > 0)
  {
    pbn = reinterpret_cast<CButton*>(pOk);
    SetDefID(IDOK);
  }
  else
  {
    pbn = reinterpret_cast<CButton*>(pFindNow);
    SetDefID(IDC_FIND_NOW);
  }
  pbn->SetButtonStyle(BS_DEFPUSHBUTTON);
  if (l > 0)
  {
    POSITION pos = listControl.GetFirstSelectedItemPosition();
    if (pos != nullptr)
    {
      int nItem = listControl.GetNextSelectedItem(pos);
      name = listControl.GetItemText(nItem, 0);
    }
  }
}

void SearchPackageDialog::OnBnClickedFindNow()
{
  try
  {
    CWaitCursor cur;
    if (!listControl.DeleteAllItems())
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::DeleteAllItems");
    }
    packages.clear();
    unique_ptr<PackageIterator> pIter(pManager->CreateIterator());
    PackageInfo packageInfo;
    int idx = 0;
    while (pIter->GetNext(packageInfo))
    {
      bool haveMatch = false;
      CString givenName = GetWindowText(IDC_NAME);
      givenName.MakeLower();
      CString packageName(packageInfo.displayName.c_str());
      packageName.MakeLower();
      haveMatch = givenName.IsEmpty() || packageName.Find(givenName) >= 0;
      if (!haveMatch)
      {
	continue;
      }
      CString givenTitle = GetWindowText(IDC_TITLE);
      haveMatch = givenTitle.IsEmpty() || packageInfo.title.find(TU_(givenTitle)) != string::npos;
      if (!haveMatch)
      {
	continue;
      }
      CString givenDescription = GetWindowText(IDC_DESCRIPTION);
      haveMatch = givenDescription.IsEmpty() || (packageInfo.description.find(TU_(givenDescription)) != string::npos);
      if (!haveMatch)
      {
	continue;
      }
      CString givenFile = GetWindowText(IDC_FILE);
      if (!givenFile.IsEmpty())
      {
	bool found = false;
	vector<string>::const_iterator it;
	for (it = packageInfo.runFiles.begin(); !found && it != packageInfo.runFiles.end(); ++it)
	{
	  PathName path(it->c_str());
	  path.RemoveDirectorySpec();
	  found = PathName::Match(TU_(givenFile), path.Get());
	}
	for (it = packageInfo.docFiles.begin(); !found && it != packageInfo.docFiles.end(); ++it)
	{
	  PathName path(it->c_str());
	  path.RemoveDirectorySpec();
	  found = PathName::Match(TU_(givenFile), path.Get());
	}
	for (it = packageInfo.sourceFiles.begin(); !found && it != packageInfo.sourceFiles.end(); ++it)
	{
	  PathName path(it->c_str());
	  path.RemoveDirectorySpec();
	  found = PathName::Match(TU_(givenFile), path.Get());
	}
	haveMatch = found;
      }
      if (!haveMatch)
      {
	continue;
      }
      LVITEM lvitem;
      lvitem.iItem = idx;
      lvitem.mask = LVIF_TEXT | LVIF_PARAM;
      lvitem.iSubItem = 0;
      lvitem.lParam = idx;
      CString displayName(packageInfo.displayName.c_str());
      lvitem.pszText = displayName.GetBuffer();
      if (listControl.InsertItem(&lvitem) < 0)
      {
	MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::InsertItem");
      }
      lvitem.mask = LVIF_TEXT;
      lvitem.iItem = idx;
      lvitem.iSubItem = 1;
      CString title(packageInfo.title.c_str());
      lvitem.pszText = title.GetBuffer();
      if (!listControl.SetItem(&lvitem))
      {
	MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SetItem");
      }
      packages[idx] = packageInfo;
      ++idx;
    }
    pIter->Dispose();
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

void SearchPackageDialog::OnLvnItemchangedListPackages(NMHDR * pNMHDR, LRESULT * pResult)
{
  UNUSED_ALWAYS(pNMHDR);
  *pResult = 0;
  try
  {
    EnableButtons();
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

void SearchPackageDialog::OnNMRclickListPackages(NMHDR * pNMHDR, LRESULT * pResult)
{
  LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR); // FIXME: 4.71
  *pResult = 0;
  try
  {
    UINT n = listControl.GetSelectedCount();
    if (n != 1)
    {
      return;
    }
    int nItem = listControl.GetNextItem(-1, LVNI_SELECTED);
    if (nItem < 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::GetNextItem");
    }
    DWORD_PTR itemData = listControl.GetItemData(nItem);
    DoContextMenu(pNMIA->ptAction, packages[itemData].deploymentName.c_str());
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

void SearchPackageDialog::DoContextMenu(POINT pt, const char * lpszExternalName)
{
  CMenu menu;
  if (!menu.LoadMenu(ID_PACKAGE_MENU2))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CMenu::LoadMenu");
  }
  CMenu * pPopup = menu.GetSubMenu(0);
  if (pPopup == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CMenu::GetSubMenu");
  }
  PackageInfo packageInfo = pManager->GetPackageInfo(lpszExternalName);
  if (GetCursorPos(&pt))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetCursorPos");
  }
  UINT cmd = TrackPopupMenu(pPopup->GetSafeHmenu(), TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, GetSafeHwnd(), nullptr);
  if (cmd == 0)
  {
    if (::GetLastError() != ERROR_SUCCESS)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("TrackPopupMenu");
    }
    return;
  }
  switch (cmd)
  {
  case ID_PROPERTIES:
  {
    PropSheetPackage::DoModal(packageInfo, this->GetParent());
    break;
  }
  }
}

void SearchPackageDialog::OnNMDblclkListPackages(NMHDR * pNMHDR, LRESULT * pResult)
{
  UNUSED_ALWAYS(pNMHDR);
  *pResult = 0;
  try
  {
    UINT n = listControl.GetSelectedCount();
    if (n != 1)
    {
      return;
    }
    int nItem = listControl.GetNextItem(-1, LVNI_SELECTED);
    if (nItem < 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::GetNextItem");
    }
    DWORD_PTR itemData = listControl.GetItemData(nItem);
    PackageInfo packageInfo = pManager->GetPackageInfo(packages[itemData].deploymentName);
    PropSheetPackage::DoModal(packageInfo, this->GetParent());
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
