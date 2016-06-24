/* PackageTreeCtrl.cpp:

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
#include "miktex/UI/MFC/PackageTreeCtrl.h"
#include "miktex/UI/MFC/PropSheetPackage.h"

#include "SearchPackageDialog.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::MFC;
using namespace std;

#define ROOT_EXTERNAL_NAME "_miktex"

class PackageTreeCtrlImpl : public PackageTreeCtrl
{
public:
  PackageTreeCtrlImpl();

public:
  PackageTreeCtrlImpl(shared_ptr<PackageManager> pManager);

public:
  void Refresh() override;

public:
  bool GetPackageInfo(HTREEITEM hItem, PackageInfo & packageInfo) override;

public:
  void GetDelta(vector<string> & toBeInstalled, vector<string> & toBeRemoved) override;

protected:
  afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

protected:
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

protected:
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

protected:
  DECLARE_MESSAGE_MAP();

private:
  enum ITEMSTATE {
    ITEMSTATE_NONE = 0,
    ITEMSTATE_UNCHECKED,
    ITEMSTATE_CHECKED,
    ITEMSTATE_UNDETERMINED,
  };

private:
  void AddPackage(HTREEITEM hParent, const PackageInfo & packageInfo, LPARAM lvl);

private:
  void OnItemClicked(HTREEITEM hItem);

private:
  void PropagateState(HTREEITEM hItem, DWORD dwState, bool fSucc = true, bool fPred = true);

private:
  DWORD DetermineState(HTREEITEM hItem);

private:
  bool DoContextMenu(CPoint point, HTREEITEM hItem);

private:
  map<HTREEITEM, string> packageNames;

private:
  multimap<string, HTREEITEM> treeItems;

private:
  map<HTREEITEM, DWORD> treeItemStates;

private:
  CImageList imageList;

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> pManager;
};

PackageTreeCtrlImpl::PackageTreeCtrlImpl()
{
}

PackageTreeCtrlImpl::PackageTreeCtrlImpl(std::shared_ptr<MiKTeX::Packages::PackageManager> pManager) :
  pManager(pManager)
{
}

DWORD PackageTreeCtrlImpl::DetermineState(HTREEITEM hItem)
{
  HTREEITEM hChild = GetChildItem(hItem);
  if (hChild == nullptr)
  {
    return treeItemStates[hItem];
  }
  DWORD treeItemState = ITEMSTATE_NONE;
  do
  {
    // RECURSION
    DWORD treeItemState2 = DetermineState(hChild);
    MIKTEX_ASSERT(treeItemState2 >= ITEMSTATE_UNCHECKED && treeItemState2 <= ITEMSTATE_UNDETERMINED);
    if (treeItemState == ITEMSTATE_NONE)
    {
      treeItemState = treeItemState2;
    }
    else if (treeItemState != treeItemState2)
    {
      treeItemState = ITEMSTATE_UNDETERMINED;
    }
  } while ((hChild = GetNextItem(hChild, TVGN_NEXT)) != nullptr);
  DWORD oldTreeItemState = treeItemStates[hItem];
  if (oldTreeItemState != treeItemState)
  {
    if (!SetItemState(hItem, INDEXTOSTATEIMAGEMASK(treeItemState), TVIS_STATEIMAGEMASK))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CTreeCtrl::SetItemState");
    }
    treeItemStates[hItem] = treeItemState;
  }
  return treeItemState;
}

void PackageTreeCtrlImpl::Refresh()
{
  // create the image list
  if (imageList.m_hImageList == nullptr)
  {
    if (!imageList.Create(IDB_PACKAGE_STATES, 13, 1, RGB(255, 255, 255)))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CImageList::Create");
    }
    SetImageList(&imageList, TVSIL_STATE);
  }

  // cut down the tree
  if (!DeleteAllItems())
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CTreeCtrl::DeleteAllItems");
  }

  // clear maps
  packageNames.clear();
  treeItems.clear();

  // create the root item
  PackageInfo rootPackageInfo;
  rootPackageInfo.deploymentName = ROOT_EXTERNAL_NAME;
  rootPackageInfo.displayName = T_("MiKTeX Packages");
  TVINSERTSTRUCT ins;
  ins.hParent = TVI_ROOT;
  ins.hInsertAfter = TVI_ROOT;
  ins.item.mask = TVIF_PARAM | TVIF_TEXT;
  CString displayName(rootPackageInfo.displayName.c_str());
  ins.item.pszText = displayName.GetBuffer();
  ins.item.lParam = 0;
  HTREEITEM hRoot = InsertItem(&ins);
  if (hRoot == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CTreeCtrl::InsertItem");
  }
  packageNames[hRoot] = rootPackageInfo.deploymentName;
  treeItems.insert(make_pair(rootPackageInfo.deploymentName, hRoot));
  treeItemStates[hRoot] = ITEMSTATE_UNDETERMINED;

  // let the tree grow
  unique_ptr<PackageIterator> pIter(pManager->CreateIterator());
  pIter->AddFilter({ PackageFilter::Top });
  PackageInfo packageInfo;
  while (pIter->GetNext(packageInfo))
  {
    AddPackage(hRoot, packageInfo, 1);
  }
  pIter->Dispose();

  // set the states
  DetermineState(hRoot);

  Expand(hRoot, TVE_EXPAND);
}

void PackageTreeCtrlImpl::AddPackage(HTREEITEM hParent, const PackageInfo & packageInfo, LPARAM lvl)
{
  // insert this package
  TVINSERTSTRUCT ins;
  ins.hParent = hParent;
  ins.hInsertAfter = TVI_SORT;
  ins.item.mask = TVIF_PARAM | TVIF_STATE | TVIF_TEXT;
  ITEMSTATE state = packageInfo.IsInstalled() ? ITEMSTATE_CHECKED : ITEMSTATE_UNCHECKED;
  ins.item.state = INDEXTOSTATEIMAGEMASK(state);
  ins.item.stateMask = TVIS_STATEIMAGEMASK;
  MIKTEX_ASSERT(packageInfo.displayName.length() > 0);
  CString displayName(packageInfo.displayName.c_str());
  ins.item.pszText = displayName.GetBuffer();
  ins.item.lParam = lvl;
  HTREEITEM hItem = InsertItem(&ins);
  if (hItem == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CTreeCtrl::InsertItem");
  }
  packageNames[hItem] = packageInfo.deploymentName;
  treeItems.insert(make_pair(packageInfo.deploymentName, hItem));
  treeItemStates[hItem] = state;

  // insert sub-packages
  for (vector<string>::const_iterator it = packageInfo.requiredPackages.begin(); it != packageInfo.requiredPackages.end(); ++it)
  {
    PackageInfo packageInfo2 = pManager->GetPackageInfo(it->c_str());
    AddPackage(hItem, packageInfo2, lvl + 1);
  }
}

void PackageTreeCtrlImpl::PropagateState(HTREEITEM hItem, DWORD treeItemState, bool fSucc, bool fPred)
{
  // determine the state of this item, if treeItemState is undefined
  if (treeItemState == ITEMSTATE_NONE)
  {
    MIKTEX_ASSERT(!fSucc);
    for (HTREEITEM hChild = GetChildItem(hItem); hChild != nullptr && treeItemState != ITEMSTATE_UNDETERMINED; hChild = GetNextItem(hChild, TVGN_NEXT))
    {
      DWORD treeItemState2 = treeItemStates[hChild];
      MIKTEX_ASSERT(treeItemState2 >= ITEMSTATE_UNCHECKED && treeItemState2 <= ITEMSTATE_UNDETERMINED);
      if (treeItemState == ITEMSTATE_NONE)
      {
	treeItemState = treeItemState2;
      }
      else if (treeItemState != treeItemState2)
      {
	treeItemState = ITEMSTATE_UNDETERMINED;
      }
    }
  }

  MIKTEX_ASSERT(treeItemState != ITEMSTATE_NONE);

  // change the state of this item and all twin items
  string displayName = packageNames[hItem];
  multimap<string, HTREEITEM>::const_iterator it = treeItems.find(displayName);
  MIKTEX_ASSERT(it != treeItems.end());
  for (; it != treeItems.end() && it->first == displayName; ++it)
  {
    DWORD oldTreeItemState = treeItemStates[it->second];
    if (oldTreeItemState != treeItemState)
    {
      if (!SetItemState(it->second, INDEXTOSTATEIMAGEMASK(treeItemState), TVIS_STATEIMAGEMASK))
      {
	MIKTEX_FATAL_WINDOWS_ERROR("CTreeCtrl::SetItemState");
      }
      treeItemStates[it->second] = treeItemState;
    }

    // change the state of all successors
    if (fSucc)
    {
      for (HTREEITEM hChild = GetChildItem(it->second); hChild != nullptr; hChild = GetNextItem(hChild, TVGN_NEXT))
      {
	PropagateState(hChild, treeItemState, true, false);
      }
    }

    // determine the state of all predecessors
    if (fPred)
    {
      HTREEITEM hParent = GetParentItem(it->second);
      if (hParent != nullptr && hParent != GetRootItem())
      {
	PropagateState(hParent, ITEMSTATE_NONE, false, true);
      }
    }
  }
}

void PackageTreeCtrlImpl::OnItemClicked(HTREEITEM hItem)
{
  try
  {
    shared_ptr<Session> session = Session::Get();

    // get the state of the item
    DWORD treeItemState = treeItemStates[hItem];
    MIKTEX_ASSERT(treeItemState >= ITEMSTATE_UNCHECKED && treeItemState <= ITEMSTATE_UNDETERMINED);

    // toggle state
    treeItemState = treeItemState == ITEMSTATE_CHECKED ? ITEMSTATE_UNCHECKED : ITEMSTATE_CHECKED;

    // disallow package removal
    //   if this is the root item
    //   if running from CD
    //   if this is a container
    //   if the reference count is greater than 1
    if (treeItemState == ITEMSTATE_UNCHECKED)
    {
      if (hItem == GetRootItem())
      {
	return;
      }
      if (session->IsMiKTeXDirect())
      {
	return;
      }
      PackageInfo packageInfo;
      GetPackageInfo(hItem, packageInfo);
      if (packageInfo.IsContainer() || packageInfo.GetRefCount() > 1)
      {
	return;
      }
    }

    // change the state
    PropagateState(hItem, treeItemState);
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

void PackageTreeCtrlImpl::OnLButtonDown(UINT nFlags, CPoint point)
{
  try
  {
    // check to see if the button was clicked
    UINT flags = 0;
    HTREEITEM hItem = HitTest(point, &flags);
    if (hItem == nullptr || (flags & TVHT_ONITEMSTATEICON) == 0)
    {
      CTreeCtrl::OnLButtonDown(nFlags, point);
      return;
    }

    CWaitCursor cur;

    // set the state of the item
    OnItemClicked(hItem);

    // notify parent window
    CWnd * pParent = GetParent();
    MIKTEX_ASSERT(pParent != nullptr);
    if (pParent != nullptr)
    {
      GetParent()->SendMessage(WM_ONTOGGLE);
    }

    CTreeCtrl::OnLButtonDown(nFlags, point);
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

bool PackageTreeCtrlImpl::GetPackageInfo(HTREEITEM hItem, PackageInfo & packageInfo)
{
  if (GetItemData(hItem) == 0)
  {
    return false;
  }
  packageInfo = pManager->GetPackageInfo(packageNames[hItem]);
  return true;
}

BEGIN_MESSAGE_MAP(PackageTreeCtrlImpl, CTreeCtrl)
  ON_WM_LBUTTONDBLCLK()
  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP();

void PackageTreeCtrlImpl::OnRButtonDown(UINT nFlags, CPoint point)
{
  try
  {
    UINT flags;
    HTREEITEM hItem = HitTest(point, &flags);
    if (hItem == nullptr || (flags & TVHT_ONITEM) == 0)
    {
      CTreeCtrl::OnRButtonDown(nFlags, point);
      return;
    }
    HTREEITEM hItemSelected = GetSelectedItem();
    if (!Select(hItem, TVGN_CARET))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CTreeCtrl::Select");
    }
    if (!DoContextMenu(point, hItem) && hItemSelected != nullptr)
    {
      if (!Select(hItemSelected, TVGN_CARET))
      {
	MIKTEX_FATAL_WINDOWS_ERROR("CTreeCtrl::Select");
      }
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

bool PackageTreeCtrlImpl::DoContextMenu(CPoint point, HTREEITEM hItem)
{
  CMenu menu;
  if (!menu.LoadMenu(ID_PACKAGE_MENU))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CMenu::LoadMenu");
  }
  CMenu * pPopup = menu.GetSubMenu(0);
  if (pPopup == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  PackageInfo packageInfo;
  if (hItem == GetRootItem())
  {
    if (pPopup->EnableMenuItem(ID_PROPERTIES, MF_GRAYED) == static_cast<UINT>(-1))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CMenu::EnableItem");
    }
  }
  else
  {
    GetPackageInfo(hItem, packageInfo);
    if (!packageInfo.IsPureContainer())
    {
      if (pPopup->EnableMenuItem(ID_SEARCH_PACKAGE, MF_GRAYED) == static_cast<UINT>(-1))
      {
	MIKTEX_FATAL_WINDOWS_ERROR("CMenu::EnableItem");
      }
    }
  }
  ClientToScreen(&point);
  UINT cmd = TrackPopupMenu(pPopup->GetSafeHmenu(), (0
      | TPM_LEFTALIGN
      | TPM_RIGHTBUTTON
      | TPM_NONOTIFY
      | TPM_RETURNCMD
      | TPM_TOPALIGN
      | 0), point.x, point.y, 0, GetSafeHwnd(), nullptr);
  if (cmd == 0)
  {
    if (::GetLastError() != 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("TrackPopupMenu");
    }
    return false;
  }
  switch (cmd)
  {
  case ID_PROPERTIES:
  {
    PropSheetPackage::DoModal(packageInfo, this->GetParent());
    break;
  }
  case ID_SEARCH_PACKAGE:
  {
    SearchPackageDialog dlg(this->GetParent(), pManager);
    if (dlg.DoModal() != IDOK)
    {
      break;
    }
    unique_ptr<PackageIterator> pIter(pManager->CreateIterator());
    PackageInfo packageInfo;
    while (pIter->GetNext(packageInfo))
    {
      if (_tcsicmp(UT_(packageInfo.displayName), dlg.GetName()) == 0)
      {
	multimap<string, HTREEITEM>::const_iterator it = treeItems.find(packageInfo.deploymentName);
	MIKTEX_ASSERT(it != treeItems.end());
	if (it != treeItems.end())
	{
	  if (!Select(it->second, TVGN_CARET))
	  {
	    MIKTEX_FATAL_WINDOWS_ERROR("CTreeCtrl::Select");
	  }
	  return true;
	}
	break;
      }
    }
    pIter->Dispose();
    break;
  }
  }
  return false;
}

void PackageTreeCtrlImpl::GetDelta(vector<string> & toBeInstalled, vector<string> & toBeRemoved)
{
  for (map<HTREEITEM, string>::const_iterator it = packageNames.begin(); it != packageNames.end(); ++it)
  {
    if (it->second == ROOT_EXTERNAL_NAME)
    {
      continue;
    }
    DWORD treeItemState = treeItemStates[it->first];
    PackageInfo packageInfo = pManager->GetPackageInfo(it->second.c_str());
    if (treeItemState == ITEMSTATE_UNCHECKED && packageInfo.IsInstalled())
    {
      if (!(packageInfo.IsPureContainer() || find(toBeRemoved.begin(), toBeRemoved.end(), packageInfo.deploymentName) != toBeRemoved.end()))
      {
	toBeRemoved.push_back(packageInfo.deploymentName);
      }
    }
    else if (treeItemState == ITEMSTATE_CHECKED && !packageInfo.IsInstalled())
    {
      if (!(packageInfo.IsPureContainer() || find(toBeInstalled.begin(), toBeInstalled.end(), packageInfo.deploymentName) != toBeInstalled.end()))
      {
	toBeInstalled.push_back(packageInfo.deploymentName);
      }
    }
  }
}

void PackageTreeCtrlImpl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
  try
  {
    // check to see if the button was clicked
    UINT flags = 0;
    HTREEITEM hItem = HitTest(point, &flags);
    if (hItem == nullptr || (flags & TVHT_ONITEM) == 0)
    {
      return;
    }

    if (ItemHasChildren(hItem))
    {
      CTreeCtrl::OnLButtonDblClk(nFlags, point);
      return;
    }

    // "open" the package
    PackageInfo packageInfo;
    GetPackageInfo(hItem, packageInfo);
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

PackageTreeCtrl * PackageTreeCtrl::Create(std::shared_ptr<MiKTeX::Packages::PackageManager> pManager)
{
  return new PackageTreeCtrlImpl(pManager);
}

void PackageTreeCtrl::Destroy(PackageTreeCtrl * pCtrl)
{
  delete pCtrl;
}
