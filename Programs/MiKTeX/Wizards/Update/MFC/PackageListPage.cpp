/* PackageListPag.cpp:

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

#include "PackageListPage.h"
#include "UpdateWizard.h"
#include "resource.h"

IMPLEMENT_DYNCREATE(PackageListPage, CPropertyPage);

const unsigned int WM_FILL_LIST = WM_APP + 1;

const unsigned int ExtendedStyles = 0
| LVS_EX_CHECKBOXES
| LVS_EX_UNDERLINECOLD
| LVS_EX_UNDERLINEHOT
| LVS_EX_ONECLICKACTIVATE;


BEGIN_MESSAGE_MAP(PackageListPage, CPropertyPage)
  ON_BN_CLICKED(IDC_DESELECT_ALL, OnDeselectAll)
  ON_BN_CLICKED(IDC_SELECT_ALL, OnSelectAll)
  ON_MESSAGE(WM_FILL_LIST, OnFillList)
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemChanged)
  ON_NOTIFY(LVN_ITEMCHANGING, IDC_LIST, OnItemChanging)
  ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LIST, OnItemActivate)
END_MESSAGE_MAP();

PackageListPage::PackageListPage() :
  CPropertyPage(IDD, 0, IDS_HEADER_PACKAGE_LIST, IDS_SUBHEADER_PACKAGE_LIST)
{
}

PackageListPage::~PackageListPage()
{
  try
  {
    if (hWorkerThread != nullptr)
    {
      CloseHandle(hWorkerThread);
      hWorkerThread = nullptr;
    }
    if (pInstaller != nullptr)
    {
      pInstaller->Dispose();
      pInstaller.reset();
    }
  }
  catch (const exception &)
  {
  }
}

BOOL PackageListPage::OnInitDialog()
{
  pSheet = reinterpret_cast<UpdateWizard*>(GetParent());
  ASSERT_KINDOF(UpdateWizard, pSheet);

  BOOL ret = CPropertyPage::OnInitDialog();

  try
  {
    MIKTEX_ASSERT(pInstaller == nullptr);
    pInstaller = UpdateWizardApplication::packageManager->CreateInstaller();
    listControl.SetExtendedStyle(listControl.GetExtendedStyle() | ExtendedStyles);
    InsertColumn(0, T_("Name"), T_("xxxx yet another package"));
    InsertColumn(1, T_("Old"), T_("xxxx v1.1234 19-December-2000"));
    InsertColumn(2, T_("New"), T_("xxxx v1.1234 19-December-2000"));
  }
  catch (const MiKTeXException & e)
  {
    pSheet->ReportError(e);
  }
  catch (const exception & e)
  {
    pSheet->ReportError(e);
  }

  return ret;
}

BOOL PackageListPage::OnSetActive()
{
  BOOL ret = CPropertyPage::OnSetActive();

  if (ret)
  {
    try
    {
      pSheet->ClearErrorFlag();

      EnableSelectButtons();

      string repository = pSheet->GetRepository();

      // start search thread, if this is the first activation or
      // if the package repository URL has changed; otherwise fill
      // the list control with the information we have
      if (!ready || repository != this->repository)
      {
        ready = false;
        this->repository = repository;

        listControl.SetExtendedStyle(listControl.GetExtendedStyle() & ~ExtendedStyles);

        // a kind of progress info
        SetProgressText(T_("Searching..."));

        // disable wizard buttons
        pSheet->SetWizardButtons(0);

        // start thread
        CWinThread * pThread = AfxBeginThread(WorkerThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
        MIKTEX_ASSERT(pThread != nullptr);
        MIKTEX_ASSERT(pThread->m_hThread != nullptr);
        if (!DuplicateHandle(GetCurrentProcess(), pThread->m_hThread, GetCurrentProcess(), &hWorkerThread, 0, FALSE, DUPLICATE_SAME_ACCESS))
        {
          MIKTEX_FATAL_WINDOWS_ERROR("DuplicateHandle");
        }
        pThread->ResumeThread();
      }
      else
      {
        OnFillList(0, 0);
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

void PackageListPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LIST, listControl);
}

LRESULT PackageListPage::OnWizardNext()
{
  pSheet->SetCameFrom(IDD);
  return 0;
}

LRESULT PackageListPage::OnWizardBack()
{
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(pSheet->GetCameFrom()));
}


BOOL PackageListPage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  if (ret)
  {
    vector<string> toBeUpdated;
    toBeUpdated.reserve(updates.size());
    vector<string> toBeRemoved;
    toBeRemoved.reserve(updates.size());
    int n = listControl.GetItemCount();
    for (int i = 0; i < n; ++i)
    {
      size_t idx = listControl.GetItemData(i);
      if (listControl.GetCheck(i))
      {
        switch (updates[idx].action)
        {
        case PackageInstaller::UpdateInfo::Repair:
        case PackageInstaller::UpdateInfo::ReleaseStateChange:
        case PackageInstaller::UpdateInfo::Update:
        case PackageInstaller::UpdateInfo::ForceUpdate:
          toBeUpdated.push_back(updates[idx].deploymentName);
          break;
        case PackageInstaller::UpdateInfo::ForceRemove:
          toBeRemoved.push_back(updates[idx].deploymentName);
          break;
        }
      }
    }
    pSheet->SetUpdateList(toBeUpdated);
    pSheet->SetRemoveList(toBeRemoved);
  }
  return ret;
}


BOOL PackageListPage::OnQueryCancel()
{
  if (ready)
  {
    return TRUE;
  }
  try
  {
    pSheet->SetCancelFlag();
    SetProgressText(T_("Cancelling..."));
  }
  catch (const MiKTeXException & e)
  {
    pSheet->ReportError(e);
  }
  catch (const exception & e)
  {
    pSheet->ReportError(e);
  }
  return FALSE;
}

LRESULT PackageListPage::OnFillList(WPARAM wParam, LPARAM lParam)
{
  UNUSED_ALWAYS(wParam);
  UNUSED_ALWAYS(lParam);

  try
  {
    if (pSheet->GetCancelFlag() || pSheet->GetErrorFlag())
    {
      SetProgressText(pSheet->GetCancelFlag() ? T_("Cancelled") : T_("Unsuccessful"));
      pSheet->EnableCloseButton();
      return 0;
    }

    CWaitCursor wait;

    AUTO_TOGGLE(fillingTheListView);

    listControl.SetExtendedStyle(listControl.GetExtendedStyle() | ExtendedStyles);

    if (!listControl.DeleteAllItems())
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::DeleteAllItems");
    }

    int idx = 0;

    bool somethingChecked = false;

    for (vector<PackageInstaller::UpdateInfo>::const_iterator it = updates.begin(); it != updates.end(); ++it, ++idx)
    {
      LV_ITEM lvitem;

      lvitem.iItem = idx;
      lvitem.mask = LVIF_PARAM;
      lvitem.lParam = idx;
      lvitem.iSubItem = 0;

      if (listControl.InsertItem(&lvitem) < 0)
      {
        MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::InsertItem");
      }

      // try to get the package info
      // TODO: we need someting like IsKnownPackage()
      PackageInfo oldPackageInfo;
      bool locallyKnown;
      try
      {
        oldPackageInfo = UpdateWizardApplication::packageManager->GetPackageInfo(it->deploymentName);
        locallyKnown = true;
      }
      catch (const MiKTeXException &)
      {
        locallyKnown = false;
      }

      // display the deployment name
      listControl.SetItemText(idx, 0, UT_(it->deploymentName));

      // display the 'old' package time-stamp, if the package is known
      if (locallyKnown && oldPackageInfo.timeInstalled > 0)
      {
        CString strOld = UT_(oldPackageInfo.version.c_str());
        if (!strOld.IsEmpty())
        {
          strOld += " / ";
        }
        CTime timeOld(oldPackageInfo.timePackaged);
        strOld += timeOld.Format("%d-%b-%y");
        listControl.SetItemText(idx, 1, strOld);
      }

      // display the 'new' package time-stamp, if the package is
      // correctly installed
      if (it->action == PackageInstaller::UpdateInfo::Repair)
      {
        listControl.SetItemText(idx, 2, T_(_T("to be repaired")));
      }
      else if (it->action == PackageInstaller::UpdateInfo::ReleaseStateChange)
      {
        listControl.SetItemText(idx, 2, T_(_T("release state change")));
      }
      else if (it->action == PackageInstaller::UpdateInfo::ForceRemove || it->action == PackageInstaller::UpdateInfo::KeepObsolete)
      {
        listControl.SetItemText(idx, 2, T_(_T("obsolete (to be removed)")));
      }
      else
      {
        CString strNew = UT_(it->version.c_str());
        if (!strNew.IsEmpty())
        {
          strNew += " / ";
        }
        CTime timeNew(it->timePackaged);
        strNew += timeNew.Format("%d-%b-%y");
        listControl.SetItemText(idx, 2, strNew);
      }

      bool toBeChecked = false;

      if (UpdateWizardApplication::upgrading)
      {
        toBeChecked = true;
      }

      if (it->action == PackageInstaller::UpdateInfo::ForceUpdate
        || it->action == PackageInstaller::UpdateInfo::ForceRemove
        || it->action == PackageInstaller::UpdateInfo::Update
        || it->action == PackageInstaller::UpdateInfo::Repair
        || it->action == PackageInstaller::UpdateInfo::ReleaseStateChange)
      {
        toBeChecked = true;
      }

      if (toBeChecked)
      {
        listControl.SetCheck(idx);
        somethingChecked = true;
      }
    }

    EnableSelectButtons();

    // disable Next, if no package is selected
    pSheet->SetWizardButtons(PSWIZB_BACK | (somethingChecked ? PSWIZB_NEXT : 0));
  }
  catch (const MiKTeXException & e)
  {
    pSheet->ReportError(e);
  }
  catch (const exception & e)
  {
    pSheet->ReportError(e);
  }

  return 0;
}

void PackageListPage::OnSelectAll()
{
  int n = listControl.GetItemCount();
  for (int i = 0; i < n; ++i)
  {
    listControl.SetCheck(i, TRUE);
  }
}

void PackageListPage::OnDeselectAll()
{
  int n = listControl.GetItemCount();
  for (int i = 0; i < n; ++i)
  {
    listControl.SetCheck(i, FALSE);
  }
}

void PackageListPage::OnItemActivate(NMHDR * pNMHDR, LRESULT * pResult)
{
  LPNMITEMACTIVATE pInfo = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
  *pResult = FALSE;
  if (fillingTheListView
    || pInfo == nullptr
    || pInfo->iItem < 0
    || pInfo->iItem >= static_cast<int>(updates.size()))
  {
    return;
  }
  string url = MIKTEX_URL_WWW_PACKAGE_INFO_FORMAT;
  string::size_type pos = url.find("%s");
  MIKTEX_ASSERT(pos != string::npos);
  url.replace(pos, strlen("%s"), updates[pInfo->iItem].deploymentName);
  Utils::ShowWebPage(url.c_str());
}

inline int GetStateImageIndex(UINT state)
{
  return (state & LVIS_STATEIMAGEMASK) >> 12;
}

void PackageListPage::OnItemChanging(NMHDR * pNMHDR, LRESULT * pResult)
{
  LPNMLISTVIEW pnmlv = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

  *pResult = FALSE;

  if (fillingTheListView
    || pnmlv == nullptr
    || pnmlv->iItem < 0
    || pnmlv->iItem >= static_cast<int>(updates.size()))
  {
    return;
  }

  if ((pnmlv->uChanged & LVIF_STATE) == 0)
  {
    return;
  }

  bool beingUnchecked = GetStateImageIndex(pnmlv->uNewState) == 1;

  const PackageInstaller::UpdateInfo & updateInfo = updates[pnmlv->iItem];

  if (beingUnchecked)
  {
    if (UpdateWizardApplication::upgrading && IsMiKTeXPackage(updateInfo.deploymentName))
    {
      AfxMessageBox(T_(_T("The MiKTeX upgrade operation must be executed as a whole.")), MB_OK | MB_ICONEXCLAMATION);
      *pResult = TRUE;
    }
    else if (updateInfo.action == PackageInstaller::UpdateInfo::KeepAdmin)
    {
      AfxMessageBox(T_(_T("This package is currently installed for all users. Only the admin variant of the wizard can update this package.")), MB_OK | MB_ICONEXCLAMATION);
      *pResult = TRUE;
    }
    else if (updateInfo.action == PackageInstaller::UpdateInfo::KeepObsolete)
    {
      AfxMessageBox(T_(_T("This package is currently installed for all users. Only the admin variant of the wizard can remove this package.")), MB_OK | MB_ICONEXCLAMATION);
      *pResult = TRUE;
    }
    else if (updateInfo.action == PackageInstaller::UpdateInfo::Repair)
    {
      AfxMessageBox(T_(_T("This package needs to be repaired.")), MB_OK | MB_ICONEXCLAMATION);
      *pResult = TRUE;
    }
    else if (updateInfo.action == PackageInstaller::UpdateInfo::ReleaseStateChange)
    {
      AfxMessageBox(T_(_T("This package needs to be installed.")), MB_OK | MB_ICONEXCLAMATION);
      *pResult = TRUE;
    }
    else if (updateInfo.action == PackageInstaller::UpdateInfo::ForceRemove)
    {
      AfxMessageBox(T_(_T("This package must be removed.")), MB_OK | MB_ICONEXCLAMATION);
      *pResult = TRUE;
    }
    else if (updateInfo.action == PackageInstaller::UpdateInfo::ForceUpdate)
    {
      AfxMessageBox(T_(_T("This package has to be updated.")), MB_OK | MB_ICONEXCLAMATION);
      *pResult = TRUE;
    }
  }
  else
  {
    if (updateInfo.action == PackageInstaller::UpdateInfo::Keep)
    {
      AfxMessageBox(T_(_T("This package cannot be updated right now. Let the wizard conclude. Then run the wizard again.")), MB_OK | MB_ICONEXCLAMATION);
      *pResult = TRUE;
    }
  }
}

void PackageListPage::OnItemChanged(NMHDR * pNMHDR, LRESULT * pResult)
{
  UNUSED_ALWAYS(pNMHDR);
  *pResult = FALSE;
  if (!ready || fillingTheListView || pSheet->GetCancelFlag() || pSheet->GetErrorFlag())
  {
    return;
  }
  bool somethingChecked = false;
  int n = listControl.GetItemCount();
  for (int i = 0; !somethingChecked && i < n; ++i)
  {
    if (listControl.GetCheck(i))
    {
      somethingChecked = true;
    }
  }
  pSheet->SetWizardButtons(PSWIZB_BACK | (somethingChecked ? PSWIZB_NEXT : 0));
}

UINT PackageListPage::WorkerThread(void * pv)
{
  PackageListPage * This = reinterpret_cast<PackageListPage*>(pv);

  try
  {
    This->DoFindUpdates();
  }
  catch (const OperationCancelledException &)
  {
  }
  catch (const MiKTeXException & e)
  {
    This->pSheet->ReportError(e);
  }
  catch (const exception & e)
  {
    This->pSheet->ReportError(e);
  }

  try
  {
    if (!This->PostMessage(WM_FILL_LIST))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CWnd::PostMessage");
    }
  }
  catch (const MiKTeXException & e)
  {
    This->pSheet->ReportError(e);
  }
  catch (const exception & e)
  {
    This->pSheet->ReportError(e);
  }

  This->ready = true;

  return 0;
}

class UpdateInfoComparer
{
public:
  bool operator() (const PackageInstaller::UpdateInfo & ui1, const PackageInstaller::UpdateInfo & ui2) const
  {
    return PathName::Compare(ui1.deploymentName, ui2.deploymentName) < 0;
  }
};

void PackageListPage::DoFindUpdates()
{
  pInstaller->SetCallback(this);

  updates.reserve(100);

  pInstaller->SetRepository(repository);

  // find updateable packages
  pInstaller->FindUpdates();

  // get list of updateable packages
  updates = pInstaller->GetUpdates();

  // sort by package name
  sort(updates.begin(), updates.end(), UpdateInfoComparer());

  canSelectAll = true;
  canDeselectAll = true;
  for (vector<PackageInstaller::UpdateInfo>::const_iterator it = updates.begin(); it != updates.end(); ++it)
  {
    if (it->action == PackageInstaller::UpdateInfo::Repair
      || it->action == PackageInstaller::UpdateInfo::ReleaseStateChange
      || it->action == PackageInstaller::UpdateInfo::ForceRemove
      || it->action == PackageInstaller::UpdateInfo::ForceUpdate)
    {
      canDeselectAll = false;
    }
    if (it->action == PackageInstaller::UpdateInfo::Keep
      || it->action == PackageInstaller::UpdateInfo::KeepAdmin)
    {
      canSelectAll = false;
    }
  }

  if (updates.size() == 0)
  {
    AfxMessageBox(T_(_T("There are currently no updates available.")), MB_OK | MB_ICONINFORMATION);
  }
}

void PackageListPage::ReportLine(const string & str)
{
  UNUSED_ALWAYS(str);
}

bool PackageListPage::OnRetryableError(const string & message)
{
  UINT style = MB_ICONSTOP;
  style |= MB_RETRYCANCEL;
  string str = message;
  str += T_("  Then click Retry to complete the operation.");
  return ::MessageBoxW(0, UW_(str.c_str()), 0, style) != IDCANCEL;
}

bool PackageListPage::OnProgress(Notification nf)
{
  UNUSED_ALWAYS(nf);
  return !(pSheet->GetErrorFlag() || pSheet->GetCancelFlag());
}

void PackageListPage::EnableSelectButtons()
{
  BOOL enable =
    (ready
      && !pSheet->GetCancelFlag()
      && !pSheet->GetErrorFlag()
      && listControl.GetItemCount() > 0);
  CWnd * pWnd;
  if ((pWnd = GetDlgItem(IDC_SELECT_ALL)) == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  pWnd->EnableWindow(enable && canSelectAll);
  if ((pWnd = GetDlgItem(IDC_DESELECT_ALL)) == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  pWnd->EnableWindow(enable && canDeselectAll);
}

void PackageListPage::SetProgressText(const char * lpszText)
{
  AUTO_TOGGLE(fillingTheListView);
  if (!listControl.DeleteAllItems())
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::DeleteAllItems");
  }
  LV_ITEM lvitem;
  lvitem.iItem = 0;
  lvitem.mask = 0;
  lvitem.iSubItem = 0;
  if (listControl.InsertItem(&lvitem) < 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::InsertItem");
  }
  if (!listControl.SetItemText(0, 0, UT_(lpszText)))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SetItemText");
  }
  if (!listControl.SetItemState(0, 0, LVIS_STATEIMAGEMASK))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SetItemState");
  }
}

void PackageListPage::InsertColumn(int colIdx, const char * lpszLabel, const char * lpszLongest)
{
  if (listControl.InsertColumn(colIdx, UT_(lpszLabel), LVCFMT_LEFT, listControl.GetStringWidth(UT_(lpszLongest)), colIdx) < 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::InsertColumn");
  }
}
