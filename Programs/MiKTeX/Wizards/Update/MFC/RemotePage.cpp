/* RemotePage.cpp:

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

#include "RemotePage.h"
#include "UpdateWizard.h"

IMPLEMENT_DYNCREATE(RemotePage, CPropertyPage);

const unsigned int WM_FILL_LIST = WM_APP + 1;

#define SHOW_DESCRIPTION 0

BEGIN_MESSAGE_MAP(RemotePage, CPropertyPage)
  ON_MESSAGE(WM_FILL_LIST, OnFillList)
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemChanged)
END_MESSAGE_MAP();

RemotePage::RemotePage() :
  CPropertyPage(IDD, 0, IDS_HEADER_REMOTE, IDS_SUBHEADER_REMOTE)
{
  m_psp.dwFlags &= ~PSP_HASHELP;
}

BOOL RemotePage::OnInitDialog()
{
  pSheet = reinterpret_cast<UpdateWizard*>(GetParent());
  ASSERT_KINDOF(UpdateWizard, pSheet);

  BOOL ret = CPropertyPage::OnInitDialog();

  try
  {
    listControl.SetExtendedStyle(listControl.GetExtendedStyle()
      | LVS_EX_FULLROWSELECT);

    int colIdx = 0;

    InsertColumn(colIdx, T_("Country"), T_("xxxx The Czech Republic"));
    ++colIdx;

    InsertColumn(colIdx, T_("Protocol"), "xxxx Protocol");
    ++colIdx;

    InsertColumn(colIdx, T_("Host"), T_("xxxx scratchy.emate.ucr.ac.cr"));
    ++colIdx;

    InsertColumn(colIdx, T_("Version"), T_("xxxx 30-Aug-04"));
    ++colIdx;

#if SHOW_DESCRIPTION
    InsertColumn(colIdx, T_("Description"), T_("xxxx Primary Package Repository"));
    ++colIdx;
#endif
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

BOOL RemotePage::OnSetActive()
{
  noDdv = false;
  BOOL ret = CPropertyPage::OnSetActive();
  if (ret)
  {
    try
    {
      pSheet->ClearErrorFlag();
      if (firstSetActive)
      {
        firstSetActive = false;
        showAll = (GetKeyState(VK_CONTROL) < 0);
        SetProgressText(T_("Connecting..."));
        pSheet->SetWizardButtons(0);
        AfxBeginThread(WorkerThread, this);
      }
      else
      {
        OnFillList(0, 0);
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
  return ret;
}

void RemotePage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LIST, listControl);
}

LRESULT RemotePage::OnWizardNext()
{
  pSheet->SetCameFrom(IDD);
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(IDD_PACKAGE_LIST));
}

LRESULT RemotePage::OnWizardBack()
{
  noDdv = true;
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(IDD_TYPE));
}

BOOL RemotePage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  if (ret && !noDdv)
  {
    try
    {
      POSITION pos = listControl.GetFirstSelectedItemPosition();
      if (pos != nullptr)
      {
        int i = listControl.GetNextSelectedItem(pos);
        UpdateWizardApplication::packageManager->SetRemotePackageRepository(repositories[i].url, pSheet->GetRepositoryReleaseState());
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

BOOL RemotePage::OnQueryCancel()
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

LRESULT RemotePage::OnFillList(WPARAM wParam, LPARAM lParam)
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

    if (!listControl.DeleteAllItems())
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::DeleteAllItems");
    }

    bool selected = false;

    string url;

    if (!UpdateWizardApplication::packageManager->TryGetRemotePackageRepository(url))
    {
      url = "";
    }

    int idx = 0;

    for (vector<RepositoryInfo>::const_iterator it = repositories.begin(); it != repositories.end(); ++it, ++idx)
    {
      if (it->packageLevel == PackageLevel::None && !showAll)
      {
        continue;
      }

      LVITEM lvitem;

      lvitem.iItem = idx;
      lvitem.mask = LVIF_PARAM;
      lvitem.lParam = idx;
      lvitem.iSubItem = 0;

      if (listControl.InsertItem(&lvitem) < 0)
      {
        MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::InsertItem");
      }

      string protocol;
      string host;

      SplitUrl(it->url, protocol, host);

      SetItemText(idx, 0, it->country.c_str());
      CString protUC(protocol.c_str());
      protUC.MakeUpper();
      SetItemText(idx, 1, TU_(protUC));
      SetItemText(idx, 2, host.c_str());
      SetItemText(idx, 3, TU_(CTime(it->timeDate).Format(_T("%d-%b-%y"))));
#if SHOW_DESCRIPTION
      SetItemText(idx, 4, it->description.c_str());
#endif

      if (it->url == url)
      {
        if (!listControl.SetItemState(idx, LVIS_SELECTED, LVIS_SELECTED))
        {
          MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SetItemState");
        }
        selected = true;
      }
    }

    pSheet->SetWizardButtons(PSWIZB_BACK | (selected ? PSWIZB_NEXT : 0));
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

void RemotePage::OnItemChanged(NMHDR * pNMHDR, LRESULT * pResult)
{
  UNUSED_ALWAYS(pNMHDR);
  *pResult = 0;
  if (ready && !pSheet->GetCancelFlag() || pSheet->GetErrorFlag())
  {
    pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
  }
}

class CountryComparer
{
public:
  inline bool operator() (const RepositoryInfo & lhs, const RepositoryInfo & rhs)
  {
    return StringCompare(lhs.country.c_str(), rhs.country.c_str(), true) < 0;
  }
};

UINT RemotePage::WorkerThread(void * pv)
{
  RemotePage * This = reinterpret_cast<RemotePage*>(pv);

  try
  {
    UpdateWizardApplication::packageManager->SetRepositoryReleaseState(This->pSheet->GetRepositoryReleaseState());
    UpdateWizardApplication::packageManager->DownloadRepositoryList();
    This->repositories = UpdateWizardApplication::packageManager->GetRepositories();
    sort(This->repositories.begin(), This->repositories.end(), CountryComparer());
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

void RemotePage::InsertColumn(int colIdx, const char * lpszLabel, const char * lpszLongest)
{
  if (listControl.InsertColumn(colIdx, UT_(lpszLabel), LVCFMT_LEFT, listControl.GetStringWidth(UT_(lpszLongest)), colIdx) < 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::InsertColumn");
  }
}

void RemotePage::SetItemText(int itemIdx, int colIdx, const char * lpszText)
{
  if (!listControl.SetItemText(itemIdx, colIdx, UT_(lpszText)))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SetItemText");
  }
}

void RemotePage::SetProgressText(const char * lpszText)
{
  if (!listControl.DeleteAllItems())
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::DeleteAllItems");
  }
  LVITEM lvitem;
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
  listControl.SetItemState(0, 0, LVIS_STATEIMAGEMASK);
}
