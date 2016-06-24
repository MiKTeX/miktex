/* SiteWizRemote.cpp:

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

#include "SiteWizRemote.h"
#include "resource.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::MFC;
using namespace MiKTeX::Util;
using namespace std;

SiteWizRemote::SiteWizRemote(shared_ptr<PackageManager> pManager) :
  CPropertyPage(SiteWizRemote::IDD, IDS_SITEWIZ),
  pManager(pManager)
{
  m_psp.dwFlags &= ~PSP_HASHELP;
}

void SiteWizRemote::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LIST, listControl);
}

BEGIN_MESSAGE_MAP(SiteWizRemote, CPropertyPage)
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemSelection)
  ON_MESSAGE(FILL_LIST, FillList)
END_MESSAGE_MAP();

BOOL SiteWizRemote::OnSetActive()
{
  BOOL ret = CPropertyPage::OnSetActive();
  try
  {
    CPropertySheet * pSheet = reinterpret_cast<CPropertySheet*>(GetParent());
    ASSERT_KINDOF(CPropertySheet, pSheet);
    if (firstSetActive)
    {
      LV_ITEM lvitem;
      lvitem.iItem = 0;
      lvitem.mask = 0;
      lvitem.iSubItem = 0;
      if (listControl.InsertItem(&lvitem) < 0)
      {
	MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::InsertItem");
      }
      if (!listControl.SetItemText(0, 0, T_(_T("Connecting..."))))
      {
	MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SetItemText");
      }
      pSheet->SetWizardButtons(0);
      firstSetActive = false;
      AfxBeginThread(DownloadList, this);
    }
    else
    {
      FillList(0, 0);
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
  return ret;
}

class CountryComparer
{
public:
  inline bool operator() (const RepositoryInfo & lhs, const RepositoryInfo & rhs)
  {
    return StringCompare(lhs.country.c_str(), rhs.country.c_str(), true) < 0;
  }
};

UINT SiteWizRemote::DownloadList(void * pv)
{
  SiteWizRemote * This = reinterpret_cast<SiteWizRemote*>(pv);
  try
  {
    This->pManager->SetRepositoryReleaseState(This->pSheet->GetRepositoryReleaseState());
    This->pManager->DownloadRepositoryList();
    This->repositories = This->pManager->GetRepositories();
    sort(This->repositories.begin(), This->repositories.end(), CountryComparer());
    if (!This->PostMessage(FILL_LIST))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CWnd::PostMessage");
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(This, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(This, e);
  }
  return 0;
}

void SiteWizRemote::InsertColumn(int colIdx, const char * lpszLabel, const char * lpszLongest)
{
  if (listControl.InsertColumn(colIdx, UT_(lpszLabel), LVCFMT_LEFT, listControl.GetStringWidth(UT_(lpszLongest)), colIdx) < 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::InsertColumn");
  }
}

BOOL SiteWizRemote::OnInitDialog()
{
  pSheet = reinterpret_cast<SiteWizSheetImpl*>(GetParent());
  BOOL ret = CPropertyPage::OnInitDialog();

  try
  {
    listControl.SetExtendedStyle(listControl.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

    int colIdx = 0;

    InsertColumn(colIdx, T_("Country"), "xxxx Australia");
    ++colIdx;

    InsertColumn(colIdx, T_("Protocol"), "xxxx HTTP");
    ++colIdx;

    InsertColumn(colIdx, T_("Host"), "xxxx scratchy.emate.ucr.ac.cr");
    ++colIdx;

    InsertColumn(colIdx, T_("Version"), "xxxx 30-Aug-04");
    ++colIdx;

    InsertColumn(colIdx, T_("Description"), "xxxx Primary Package Repository");
    ++colIdx;
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

BOOL SiteWizRemote::OnWizardFinish()
{
  try
  {
    POSITION pos = listControl.GetFirstSelectedItemPosition();
    MIKTEX_ASSERT(pos != nullptr);
    if (pos == nullptr)
    {
      return FALSE;
    }
    int idx = listControl.GetNextSelectedItem(pos);
    pManager->SetDefaultPackageRepository(RepositoryType::Remote, pSheet->GetRepositoryReleaseState(), repositories[idx].url);
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

LRESULT SiteWizRemote::OnWizardBack()
{
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(IDD_SITEWIZ_TYPE));
}

void SiteWizRemote::OnItemSelection(NMHDR * pNMHDR, LRESULT * pResult)
{
  UNUSED_ALWAYS(pNMHDR);
  CPropertySheet * pSheet = reinterpret_cast<CPropertySheet*>(GetParent());
  ASSERT_KINDOF(CPropertySheet, pSheet);
  pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);
  *pResult = 0;
}

void SiteWizRemote::SetItemText(int itemIdx, int colIdx, const char * lpszText)
{
  if (!listControl.SetItemText(itemIdx, colIdx, UT_(lpszText)))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SetItemText");
  }
}

void SplitUrl(const string & url, string & protocol, string & host)
{
  _TCHAR szProtocol[200];
  _TCHAR szHost[200];
  URL_COMPONENTS url_comp = { 0 };
  url_comp.dwStructSize = sizeof(url_comp);
  url_comp.lpszScheme = szProtocol;
  url_comp.dwSchemeLength = 200;
  url_comp.lpszHostName = szHost;
  url_comp.dwHostNameLength = 200;
  if (!InternetCrackUrl(UT_(url), 0, 0, &url_comp))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("InternetCrackUrl");
  }
  protocol = TU_(szProtocol);
  host = TU_(szHost);
}

LRESULT SiteWizRemote::FillList(WPARAM wParam, LPARAM lParam)
{
  UNUSED_ALWAYS(wParam);
  UNUSED_ALWAYS(lParam);

  CPropertySheet * pSheet = reinterpret_cast<CPropertySheet*>(GetParent());
  ASSERT_KINDOF(CPropertySheet, pSheet);

  try
  {
    if (!listControl.DeleteAllItems())
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::DeleteAllItems");
    }

    bool selected = false;

    string url;

    if (!pManager->TryGetRemotePackageRepository(url))
    {
      url = "";
    }

    int idx = 0;

    for (vector<RepositoryInfo>::const_iterator it = repositories.begin(); it != repositories.end(); ++it, ++idx)
    {
#if MIKTEX_RELEASE_STATE < 4
      if (it->packageLevel == PackageLevel::None)
      {
	continue;
      }
#endif

      LV_ITEM lvitem;

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
      SetItemText(idx, 1, TU_(protUC.GetString()));
      SetItemText(idx, 2, host.c_str());
      SetItemText(idx, 3, TU_(CTime(it->timeDate).Format(_T("%d-%b-%y"))));
      SetItemText(idx, 4, it->description.c_str());

      if (it->url == url)
      {
	if (!listControl.SetItemState(idx, LVIS_SELECTED, LVIS_SELECTED))
	{
	  MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SetItemState");
	}
	selected = true;
      }
    }

    pSheet->SetWizardButtons(PSWIZB_BACK | (selected ? PSWIZB_FINISH : 0));
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }

  return 0;
}
