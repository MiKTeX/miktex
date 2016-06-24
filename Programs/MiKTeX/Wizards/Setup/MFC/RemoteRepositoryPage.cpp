/* RemoteRepositoryPag.cpp:

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

#include "RemoteRepositoryPage.h"
#include "SetupWizard.h"

const unsigned int WM_FILL_LIST = WM_APP + 1;

#define SHOW_DESCRIPTION 0

BEGIN_MESSAGE_MAP(RemoteRepositoryPage, CPropertyPage)
  ON_MESSAGE(WM_FILL_LIST, OnFillList)
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemChanged)
END_MESSAGE_MAP();

RemoteRepositoryPage::RemoteRepositoryPage() :
  CPropertyPage(IDD, 0, IDS_HEADER_REMOTE_REPOSITORY, IDS_SUBHEADER_REMOTE_REPOSITORY)
{
}

BOOL RemoteRepositoryPage::OnInitDialog()
{
  pSheet = reinterpret_cast<SetupWizard *>(GetParent());

  BOOL ret = CPropertyPage::OnInitDialog();

  try
  {
    listControl.SetExtendedStyle(listControl.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

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

BOOL RemoteRepositoryPage::OnSetActive()
{
  noDdv = false;
  BOOL ret = CPropertyPage::OnSetActive();
  if (ret)
  {
    try
    {
      if (firstSetActive)
      {
	firstSetActive = false;
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

void RemoteRepositoryPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LIST, listControl);
}

LRESULT RemoteRepositoryPage::OnWizardNext()
{
  pSheet->PushPage(IDD);
  UINT next;
  switch (SetupApp::Instance->GetTask())
  {
  case SetupTask::Download:
  case SetupTask::InstallFromRemoteRepository:
    next = IDD_LOCAL_REPOSITORY;
    break;
  default:
    MIKTEX_ASSERT(false);
    __assume (false);
    break;
  }
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(next));
}

LRESULT RemoteRepositoryPage::OnWizardBack()
{
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(pSheet->PopPage()));
}

BOOL RemoteRepositoryPage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  if (ret && !noDdv)
  {
    try
    {
      POSITION pos = listControl.GetFirstSelectedItemPosition();
      if (pos != nullptr)
      {
	SetupOptions options = SetupApp::Instance->Service->GetOptions();
	int i = listControl.GetNextSelectedItem(pos);
	options.RemotePackageRepository = repositories[i].url;
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

BOOL RemoteRepositoryPage::OnQueryCancel()
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

void SplitUrl(const string & url, string & protocol, string & host)
{
  wchar_t szProtocol[200];
  wchar_t szHost[200];
  URL_COMPONENTSW url_comp = { 0 };
  url_comp.dwStructSize = sizeof(url_comp);
  url_comp.lpszScheme = szProtocol;
  url_comp.dwSchemeLength = 200;
  url_comp.lpszHostName = szHost;
  url_comp.dwHostNameLength = 200;
  if (!InternetCrackUrlW(UW_(url), 0, 0, &url_comp))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("InternetCrackUrlW");
  }
  protocol = WU_(szProtocol);
  host = WU_(szHost);
}

LRESULT RemoteRepositoryPage::OnFillList(WPARAM wParam, LPARAM lParam)
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

    int idx = 0;

    for (vector<RepositoryInfo>::const_iterator it = repositories.begin(); it != repositories.end(); ++it, ++idx)
    {
#if MIKTEX_RELEASE_STATE < 4
      if (it->packageLevel < SetupApp::Instance->GetPackageLevel())
      {
	continue;
      }
#endif

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
      CString protUC = UT_(protocol);
      protUC.MakeUpper();
      SetItemText(idx, 1, TU_(protUC));
      SetItemText(idx, 2, host.c_str());
      SetItemText(idx, 3, TU_(CTime(it->timeDate).Format(_T("%d-%b-%y"))));
#if SHOW_DESCRIPTION
      SetItemText(idx, 4, it->description.c_str());
#endif

      if (it->url == SetupApp::Instance->GetRemotePackageRepository())
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

void RemoteRepositoryPage::OnItemChanged(NMHDR * pNMHDR, LRESULT * pResult)
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

UINT RemoteRepositoryPage::WorkerThread(void * pv)
{
  RemoteRepositoryPage * This = reinterpret_cast<RemoteRepositoryPage *>(pv);
  try
  {
    SetupApp::Instance->packageManager->DownloadRepositoryList();
    This->repositories = SetupApp::Instance->packageManager->GetRepositories();
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

void RemoteRepositoryPage::InsertColumn(int colIdx, const char * lpszLabel, const char * lpszLongest)
{
  if (listControl.InsertColumn(colIdx, UT_(lpszLabel), LVCFMT_LEFT, listControl.GetStringWidth(UT_(lpszLongest)), colIdx) < 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::InsertColumn");
  }
}

void RemoteRepositoryPage::SetItemText(int itemIdx, int colIdx, const char * lpszText)
{
  if (!listControl.SetItemText(itemIdx, colIdx, UT_(lpszText)))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SetItemText");
  }
}

void RemoteRepositoryPage::SetProgressText(const char * lpszText)
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
