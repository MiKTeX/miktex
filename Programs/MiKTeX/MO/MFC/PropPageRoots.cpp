/* PropPageRoots.cpp:

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

#include "PropPageRoots.h"
#include "resource.hm"

#define MAKE_ID_HID_PAIR(id) id, H##id

namespace
{
  const DWORD aHelpIDs[] = {
    MAKE_ID_HID_PAIR(IDC_ADD),
    MAKE_ID_HID_PAIR(IDC_LIST),
    MAKE_ID_HID_PAIR(IDC_MOVEDOWN),
    MAKE_ID_HID_PAIR(IDC_MOVEUP),
    MAKE_ID_HID_PAIR(IDC_REMOVE),
    MAKE_ID_HID_PAIR(IDC_SCAN),
    0, 0,
  };
}

BEGIN_MESSAGE_MAP(PropPageTeXMFRoots, CPropertyPage)
  ON_BN_CLICKED(IDC_ADD, OnAdd)
  ON_BN_CLICKED(IDC_MOVEDOWN, OnMovedown)
  ON_BN_CLICKED(IDC_MOVEUP, OnMoveup)
  ON_BN_CLICKED(IDC_REMOVE, OnRemove)
  ON_BN_CLICKED(IDC_SHOW_HIDDEN_ROOTS, OnShowHiddenRoots)
  ON_BN_CLICKED(IDC_SCAN, OnScan)
  ON_NOTIFY(LVN_GETINFOTIP, IDC_LIST, OnGetInfoTip)
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnSelectionChange)
  ON_WM_CONTEXTMENU()
  ON_WM_HELPINFO()
END_MESSAGE_MAP();

PropPageTeXMFRoots::PropPageTeXMFRoots() :
  CPropertyPage(PropPageTeXMFRoots::IDD)
{
  commonInstallRoot = session->GetSpecialPath(SpecialPath::CommonInstallRoot);
  commonDataRoot = session->GetSpecialPath(SpecialPath::CommonDataRoot);
  commonConfigRoot = session->GetSpecialPath(SpecialPath::CommonConfigRoot);
  if (!session->IsAdminMode())
  {
    userInstallRoot = session->GetSpecialPath(SpecialPath::UserInstallRoot);
    userDataRoot = session->GetSpecialPath(SpecialPath::UserDataRoot);
    userConfigRoot = session->GetSpecialPath(SpecialPath::UserConfigRoot);
  }
  m_psp.dwFlags &= ~(PSP_HASHELP);
}

BOOL PropPageTeXMFRoots::OnInitDialog()
{
  BOOL ret = CPropertyPage::OnInitDialog();

  try
  {
    listControl.SetExtendedStyle(listControl.GetExtendedStyle()
      | LVS_EX_FULLROWSELECT
      | LVS_EX_INFOTIP);
    InsertColumn(0, T_("Path"), "C:\\Documents and Settings\\All Users\\...\\MiKTeX xxx");
    InsertColumn(1, T_("Description"), "common config xxx");
    Refresh();
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

void PropPageTeXMFRoots::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Check(pDX, IDC_SHOW_HIDDEN_ROOTS, showHiddenRoots);
  DDX_Control(pDX, IDC_ADD, addButton);
  DDX_Control(pDX, IDC_LIST, listControl);
  DDX_Control(pDX, IDC_MOVEDOWN, downButton);
  DDX_Control(pDX, IDC_MOVEUP, upButton);
  DDX_Control(pDX, IDC_REMOVE, removeButton);
  DDX_Control(pDX, IDC_SCAN, scanButton);
}

void PropPageTeXMFRoots::OnScan()
{
  try
  {
    unique_ptr<ProgressDialog> pProgressDialog(ProgressDialog::Create());
    pProgressDialog->StartProgressDialog(GetParent()->GetSafeHwnd());
    pProgressDialog->SetTitle(T_("MiKTeX Maintenance"));
    pProgressDialog->SetLine(1, T_("Collecting file information..."));
    POSITION pos = listControl.GetFirstSelectedItemPosition();
    this->pProgressDialog = pProgressDialog.get();
    MIKTEX_ASSERT(pos != nullptr);
    while (pos != nullptr)
    {
      int idx = listControl.GetNextSelectedItem(pos);
      int rootOrdinal = session->TryDeriveTEXMFRoot(roots[idx]);
      if (rootOrdinal == INVALID_ROOT_INDEX)
      {
        continue;
      }
      bool isCommonRoot = (session->IsCommonRootDirectory(rootOrdinal));
      if (session->IsAdminMode() && !isCommonRoot || !session->IsAdminMode() && isCommonRoot)
      {
        continue;
      }
      Fndb::Refresh(roots[idx], this);
    }
    pProgressDialog->StopProgressDialog();
    pProgressDialog.reset();
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  pProgressDialog = 0;
}

INT CALLBACK PropPageTeXMFRoots::BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM pData)
{
  UNUSED_ALWAYS(lParam);
  UNUSED_ALWAYS(pData);

  try
  {
#if 0
    PropPageTeXMFRoots * This = reinterpret_cast<PropPageTeXMFRoots*>(pData);
#endif
    switch (uMsg)
    {
    case BFFM_INITIALIZED:
      char szDrive[BufferSizes::MaxPath];
      PathName::Split(PathName().SetToCurrentDirectory().Get(), szDrive, BufferSizes::MaxPath, nullptr, 0, nullptr, 0, nullptr, 0);
      if (szDrive[0] != 0)
      {
        PathName root(szDrive, "\\", nullptr, nullptr);
        CString rootStr(root.Get());
        ::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, reinterpret_cast<LPARAM>(rootStr.GetString()));
      }
      return 0;
    default:
      return 0;
    }
  }
  catch (const exception &)
  {
    return 0;
  }
}

void PropPageTeXMFRoots::OnAdd()
{
  try
  {
    BROWSEINFO browseInfo;
    ZeroMemory(&browseInfo, sizeof(browseInfo));
    browseInfo.hwndOwner = GetSafeHwnd();
    browseInfo.ulFlags = BIF_RETURNONLYFSDIRS;
#if 0
    browseInfo.ulFlags |= BIF_USENEWUI;
#else
    // We cannot use BIF_USENEWUI because we are running in a
    // multithreaded apartment.  See MSDN "INFO: Calling Shell
    // Functions and Interfaces from a Multithreaded Apartment".
#endif
    browseInfo.lpfn = BrowseCallbackProc;
    browseInfo.lParam = reinterpret_cast<LPARAM>(this);
    browseInfo.lpszTitle = T_(_T("Select the root directory to be added:"));
    LPITEMIDLIST pidlRoot = nullptr;
    if (SHGetSpecialFolderLocation(0, CSIDL_DRIVES, &pidlRoot) != S_OK)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("SHGetSpecialFolderLocation");
    }
    AutoCoTaskMem autoFree(pidlRoot);
    browseInfo.pidlRoot = pidlRoot;
    LPITEMIDLIST pidl = SHBrowseForFolder(&browseInfo);
    if (pidl == nullptr)
    {
      return;
    }
    _TCHAR szDir[BufferSizes::MaxPath];
    BOOL done = SHGetPathFromIDList(pidl, szDir);
    CoTaskMemFree(pidl);
    if (!done)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("SHGetPathFromIDList");
    }
    CheckRoot(szDir);
    LVITEM lvitem;
    lvitem.iItem = listControl.GetItemCount();
    lvitem.mask = LVIF_TEXT | LVIF_PARAM;
    lvitem.iSubItem = 0;
    lvitem.pszText = szDir;
    lvitem.lParam = lvitem.iItem;
    if (listControl.InsertItem(&lvitem) < 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::InsertItem");
    }
    roots.push_back(szDir);
    SetChanged(true);
    EnableButtons();
    isModified = true;
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

const char * tdsDirs[] = {
  "bibtex",
  "dvipdfm",
  "dvips",
  "fontname",
  "fonts",
  "makeindex",
  "metafont",
  "metapost",
  "mft",
  "miktex",
  "pdftex",
  "psutils",
  "scripts",
  "tex",
  "tpm",
  "ttf2pfb",
  "ttf2tfm",
};

void PropPageTeXMFRoots::CheckRoot(const PathName & root)
{
  unique_ptr<DirectoryLister> pLister = DirectoryLister::Open(root);
  DirectoryEntry entry;
  bool isEmpty = true;
  while (pLister->GetNext(entry))
  {
    isEmpty = false;
    if (entry.isDirectory)
    {
      PathName name(entry.name);
      for (size_t idx = 0; idx < sizeof(tdsDirs) / sizeof(tdsDirs[0]); ++idx)
      {
        if (name == tdsDirs[idx])
        {
          return;
        }
      }
    }
  }
  if (!isEmpty)
  {
    MIKTEX_FATAL_ERROR_2(T_("Not a TDS-compliant root directory."), "path", root.ToString());
  }
}

static int CALLBACK CompareItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
  UNUSED_ALWAYS(lParamSort);
  return static_cast<int>(lParam1 - lParam2);
}

void PropPageTeXMFRoots::OnMovedown()
{
  try
  {
    int idx = GetSelectedItem();

    PathName tmp = roots[idx];
    roots[idx] = roots[idx + 1];
    roots[idx + 1] = tmp;

    LVITEM lvitem;
    lvitem.mask = LVIF_PARAM;
    lvitem.iSubItem = 0;

    lvitem.iItem = idx;
    if (!listControl.GetItem(&lvitem))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::GetItem");
    }
    lvitem.lParam = idx + 1;
    if (!listControl.SetItem(&lvitem))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SetItem");
    }

    lvitem.iItem = idx + 1;
    if (!listControl.GetItem(&lvitem))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::GetItem");
    }
    lvitem.lParam = idx;
    if (!listControl.SetItem(&lvitem))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SetItem");
    }

    if (!listControl.SortItems(CompareItems, 0))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SortItem");
    }

    EnableButtons();

    SetChanged(true);

    isModified = true;
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

void PropPageTeXMFRoots::OnMoveup()
{
  try
  {
    int idx = GetSelectedItem();

    PathName tmp = roots[idx];
    roots[idx] = roots[idx - 1];
    roots[idx - 1] = tmp;

    LVITEM lvitem;
    lvitem.mask = LVIF_PARAM;
    lvitem.iSubItem = 0;

    lvitem.iItem = idx - 1;
    if (!listControl.GetItem(&lvitem))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::GetItem");
    }
    lvitem.lParam = idx;
    if (!listControl.SetItem(&lvitem))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SetItem");
    }

    lvitem.iItem = idx;
    if (!listControl.GetItem(&lvitem))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::GetItem");
    }
    lvitem.lParam = idx - 1;
    if (!listControl.SetItem(&lvitem))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SetItem");
    }

    if (!listControl.SortItems(CompareItems, 0))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SortItem");
    }

    EnableButtons();

    SetChanged(true);

    isModified = true;
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

void PropPageTeXMFRoots::OnRemove()
{
  try
  {
    UINT n = listControl.GetSelectedCount();
    for (UINT i = 0; i < n; ++i)
    {
      int idx = GetSelectedItem();
      if (!listControl.DeleteItem(idx))
      {
        MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::DeleteItem");
      }
      roots.erase(roots.begin() + idx);
    }
    EnableButtons();
    SetChanged(true);
    isModified = true;
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

void PropPageTeXMFRoots::OnShowHiddenRoots()
{
  try
  {
    UpdateData(TRUE);
    Refresh();
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

void PropPageTeXMFRoots::InsertColumn(int colIdx, const char * lpszLabel, const char * lpszLongest)
{
  if (listControl.InsertColumn(colIdx, UT_(lpszLabel), LVCFMT_LEFT, listControl.GetStringWidth(UT_(lpszLongest)), colIdx) < 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::InsertColumn");
  }
}

void PropPageTeXMFRoots::OnSelectionChange(NMHDR * pNMHDR, LRESULT * pResult)
{
  UNUSED_ALWAYS(pNMHDR);
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
  *pResult = 0;
}

bool PropPageTeXMFRoots::IsHiddenRoot(const PathName & root)
{
  int rootOrdinal = session->TryDeriveTEXMFRoot(root);
  if (rootOrdinal == INVALID_ROOT_INDEX)
  {
    return false;
  }
  if (!session->IsAdminMode() && (session->IsCommonRootDirectory(rootOrdinal) || root == userInstallRoot || root == userConfigRoot || root == userDataRoot))
  {
    return true;
  }
  return root == commonInstallRoot || root == commonConfigRoot || root == commonDataRoot;
}

void PropPageTeXMFRoots::EnableButtons()
{
  int nItems = listControl.GetItemCount();

  UINT nSelected = listControl.GetSelectedCount();

  bool canAdd = true;
  bool canRemove = true;
  bool canMoveUp = true;
  bool canMoveDown = true;
  bool canScan = true;

  int idx = -1;

  for (POSITION pos = listControl.GetFirstSelectedItemPosition(); pos != nullptr; )
  {
    idx = listControl.GetNextSelectedItem(pos);
    PathName root = roots[idx];
    int rootOrdinal = session->TryDeriveTEXMFRoot(root);
    if (rootOrdinal == INVALID_ROOT_INDEX)
    {
      canScan = false;
    }
    else
    {
      bool isCommonRoot = session->IsCommonRootDirectory(rootOrdinal);
      if (session->IsAdminMode() && !isCommonRoot || !session->IsAdminMode() && isCommonRoot)
      {
        canScan = false;
      }
    }
    if (IsHiddenRoot(root))
    {
      canRemove = false;
      canMoveUp = false;
      canMoveDown = false;
    }
    else
    {
      if (idx > 0 && IsHiddenRoot(roots[idx - 1]))
      {
        canMoveUp = false;
      }
      if (idx < roots.size() - 1 && IsHiddenRoot(roots[idx + 1]))
      {
        canMoveDown = false;
      }
    }
  }

  scanButton.EnableWindow(canScan && nSelected > 0);

  upButton.EnableWindow(!session->IsMiKTeXDirect()
    && canMoveUp
    && nSelected == 1
    && idx > 0);

  downButton.EnableWindow(!session->IsMiKTeXDirect()
    && canMoveDown
    && nSelected == 1
    && idx + 1 < nItems);

  addButton.EnableWindow(!session->IsMiKTeXDirect()
    && canAdd);

  removeButton.EnableWindow(!session->IsMiKTeXDirect()
    && canRemove
    && nSelected > 0);
}

int PropPageTeXMFRoots::GetSelectedItem()
{
  POSITION pos = listControl.GetFirstSelectedItemPosition();
  if (pos == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  return listControl.GetNextSelectedItem(pos);
}

void PropPageTeXMFRoots::Refresh()
{
  if (!listControl.DeleteAllItems())
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::DeleteAllItems");
  }
  roots.clear();
  unsigned nRoots = session->GetNumberOfTEXMFRoots();
  for (unsigned rootOrd = 0; rootOrd < nRoots; ++rootOrd)
  {
    if (session->IsAdminMode() && !session->IsCommonRootDirectory(rootOrd))
    {
      continue;
    }
    PathName root = session->GetRootDirectory(rootOrd);
    if (!showHiddenRoots && IsHiddenRoot(root))
    {
      continue;
    }
    LVITEM lvitem;
    lvitem.iItem = roots.size();
    lvitem.mask = LVIF_TEXT | LVIF_PARAM;
    lvitem.iSubItem = 0;
    CString compacted;
    if (!PathCompactPathEx(compacted.GetBuffer(BufferSizes::MaxPath), UT_(root.Get()), 45, 0))
    {
      compacted = root.Get();
    }
    lvitem.pszText = compacted.GetBuffer();
    lvitem.lParam = roots.size();
    if (listControl.InsertItem(&lvitem) < 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::InsertItem");
    }
    string description;
    if (root == userInstallRoot)
    {
      if (!description.empty())
      {
        description += ", ";
      }
      description += "UserInstall";
    }
    if (root == commonInstallRoot)
    {
      if (!description.empty())
      {
        description += ", ";
      }
      description += "CommonInstall";
    }
    if (root == userDataRoot)
    {
      if (!description.empty())
      {
        description += ", ";
      }
      description += "UserData";
    }
    if (root == commonDataRoot)
    {
      if (!description.empty())
      {
        description += ", ";
      }
      description += "CommonData";
    }
    if (root == userConfigRoot)
    {
      if (!description.empty())
      {
        description += ", ";
      }
      description += "UserConfig";
    }
    if (root == commonConfigRoot)
    {
      if (!description.empty())
      {
        description += ", ";
      }
      description += "CommonConfig";
    }
    if (!description.empty())
    {
      lvitem.mask = LVIF_TEXT;
      lvitem.iSubItem = 1;
      CString descriptionString(UT_(description));
      lvitem.pszText = descriptionString.GetBuffer();
      if (!listControl.SetItem(&lvitem))
      {
        MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SetItem");
      }
    }
    roots.push_back(root);
  }
  EnableButtons();
}

BOOL PropPageTeXMFRoots::OnApply()
{
  if (isModified)
  {
    try
    {
      if (session->IsMiKTeXDirect())
      {
        MIKTEX_UNEXPECTED();
      }

      string str;

      for (vector<PathName>::const_iterator it = roots.begin(); it != roots.end(); ++it)
      {
        if (IsHiddenRoot(*it))
        {
          continue;
        }
        if (!str.empty())
        {
          str += ';';
        }
        str += it->Get();
      }

      session->RegisterRootDirectories(str);

      unique_ptr<ProgressDialog> pProgressDialog(ProgressDialog::Create());

      pProgressDialog->StartProgressDialog(GetParent()->GetSafeHwnd());
      pProgressDialog->SetTitle(T_("MiKTeX Maintenance"));
      pProgressDialog->SetLine(1, T_("Collecting file information..."));
      this->pProgressDialog = pProgressDialog.get();

      isModified = !Fndb::Refresh(this);

      pProgressDialog->StopProgressDialog();

      SetElevationRequired(isModified);
    }
    catch (const MiKTeXException & e)
    {
      ErrorDialog::DoModal(this, e);
    }
    catch (const exception & e)
    {
      ErrorDialog::DoModal(this, e);
    }

    pProgressDialog = nullptr;
  }
  return !isModified;
}

BOOL PropPageTeXMFRoots::OnHelpInfo(HELPINFO * pHelpInfo)
{
  return ::OnHelpInfo(pHelpInfo, aHelpIDs, "RootsPage.txt");
}

void PropPageTeXMFRoots::OnContextMenu(CWnd * pWnd, CPoint point)
{
  try
  {
    DoWhatsThisMenu(pWnd, point, aHelpIDs, "RootsPage.txt");
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

bool PropPageTeXMFRoots::ReadDirectory(const char * lpszPath, char ** ppSubDirNames, char ** ppFileNames, char ** ppFileNameInfos)

{
  UNUSED_ALWAYS(lpszPath);
  UNUSED_ALWAYS(ppSubDirNames);
  UNUSED_ALWAYS(ppFileNames);
  UNUSED_ALWAYS(ppFileNameInfos);
  return false;
}

bool PropPageTeXMFRoots::OnProgress(unsigned level, const char * lpszDirectory)
{
  UNUSED_ALWAYS(level);
  pProgressDialog->SetLine(2, lpszDirectory);
  return !pProgressDialog->HasUserCancelled();
}

void PropPageTeXMFRoots::OnGetInfoTip(NMHDR * pNMHDR, LRESULT * pResult)
{
  *pResult = 0;
  try
  {
    NMLVGETINFOTIP * pInfoTip = reinterpret_cast<NMLVGETINFOTIP*>(pNMHDR);
    MIKTEX_ASSERT(pInfoTip != nullptr);
    PathName path = roots[pInfoTip->iItem];
    string info = path.Get();
    bool maintainedByMiKTeX =
      (path == userInstallRoot
        || path == commonInstallRoot
        || path == userDataRoot
        || path == commonDataRoot
        || path == userConfigRoot
        || path == commonConfigRoot);
    if (path == userInstallRoot)
    {
      info += T_("\r\n\r\nThis is the per-user installation directory.");
    }
    if (path == commonInstallRoot)
    {
      info += T_("\r\n\r\nThis is the system-wide installation directory.");
    }
    if (path == userDataRoot)
    {
      info += T_("\r\n\r\nThis is the per-user data directory.");
    }
    if (path == commonDataRoot)
    {
      info += T_("\r\n\r\nThis is the system-wide data directory.");
    }
    if (path == userConfigRoot)
    {
      info += T_("\r\n\r\nThis is the per-user configuration directory.");
    }
    if (path == commonConfigRoot)
    {
      info +=
        T_("\r\n\r\nThis is the system-wide configuration directory.");
    }
    if (maintainedByMiKTeX)
    {
      info += T_("\r\n\r\nThis directory is maintained by MiKTeX. You should not install your own files here, because they might get lost when MiKTeX is updated.");
    }
    else
    {
      info += T_("\r\n\r\nThis directory can be used for local additions.");
    }
    StringUtil::CopyString(pInfoTip->pszText, pInfoTip->cchTextMax, UT_(info.c_str()));
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

void PropPageTeXMFRoots::SetElevationRequired(bool f)
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

void PropPageTeXMFRoots::SetChanged(bool f)
{
  SetElevationRequired(f);
  SetModified(f ? TRUE : FALSE);
}
