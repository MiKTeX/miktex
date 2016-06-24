/* SourceSpecialsDialog.cpp:

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of Yap.

   Yap is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Yap is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Yap; if not, write to the Free Software Foundation, 59
   Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include "StdAfx.h"

#include "yap.h"

#include "DviDoc.h"
#include "Dviview.h"
#include "ProgressIndicatorDialog.h"
#include "SourceSpecialsDialog.h"

BEGIN_MESSAGE_MAP(SourceSpecialsDialog, CDialog)
  ON_BN_CLICKED(IDC_CLOSE, OnClose)
  ON_BN_CLICKED(IDC_EDIT_SOURCE, OnEditSource)
  ON_BN_CLICKED(IDC_GOTO, OnGoto)
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_SOURCE_SPECIALS, OnItemchangedSourceSpecials)
  ON_NOTIFY(NM_DBLCLK, IDC_SOURCE_SPECIALS, OnDblclkSourceSpecials)
END_MESSAGE_MAP();

SourceSpecialsDialog::SourceSpecialsDialog(CWnd * pParent, DviDoc * pDoc) :
  CDialog(IDD, pParent),
  pDviDoc(pDoc),
  pView(reinterpret_cast<DviView*>(pParent))
{
  ProgressIndicatorDialog pi;
  pi.Create(IDD_PROGRESS_INDICATOR);
  pi.progressBar.SetRange(0, static_cast<short>(pDviDoc->GetPageCount()));

  for (int p = 0; p < pDviDoc->GetPageCount(); ++p)
  {
    pi.progressBar.SetPos(p);

    DviPage * pDviPage = pDviDoc->GetLoadedPage(p);

    if (pDviPage == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }

    AutoUnlockPage autoUnlockPage(pDviPage);

    DviSpecial * pDviSpecial;
    int j = 0;
    while ((pDviSpecial = pDviPage->GetSpecial(j++)) != nullptr)
    {
      SourceSpecial * pSrcSpecial;
      pSrcSpecial = dynamic_cast<SourceSpecial *>(pDviSpecial);
      if (pSrcSpecial != nullptr)
      {
        SrcSpecial s;
        s.pageName = pDviPage->GetName();
        s.fileName = pSrcSpecial->GetFileName();
        s.line = pSrcSpecial->GetLineNum();
        sourceSpecials.push_back(s);
      }
    }
  }

  if (sourceSpecials.size() == 0)
  {
    MIKTEX_FATAL_ERROR(T_("The document contains no source links."));
  }
}

void SourceSpecialsDialog::DoDataExchange(CDataExchange * pDX)
{
  CDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_EDIT_SOURCE, editButton);
  DDX_Control(pDX, IDC_GOTO, goToButton);
  DDX_Control(pDX, IDC_SOURCE_SPECIALS, listControl);
}

BOOL SourceSpecialsDialog::OnInitDialog()
{
  BOOL ret = CDialog::OnInitDialog();

  try
  {
    CenterWindow();

    listControl.SetExtendedStyle(listControl.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

    if (listControl.InsertColumn(0, T_(_T("Page")), LVCFMT_LEFT, listControl.GetStringWidth(T_(_T("  Page  "))), 0) < 0)
    {
      MIKTEX_UNEXPECTED();
    }

    if (listControl.InsertColumn(1, T_(_T("Source File")), LVCFMT_LEFT, listControl.GetStringWidth(T_(_T("  abrakadabra.tex  "))), 1) < 0)
    {
      MIKTEX_UNEXPECTED();
    }

    if (listControl.InsertColumn(2, T_(_T("Line")), LVCFMT_LEFT, listControl.GetStringWidth(T_(_T("  99999  "))), 2) < 0)
    {
      MIKTEX_UNEXPECTED();
    }

    int idx = 0;

    for (vector<SrcSpecial>::const_iterator it = sourceSpecials.begin(); it != sourceSpecials.end(); ++it, ++idx)
    {
      LV_ITEM lvitem;
      lvitem.iItem = idx;
      lvitem.mask = LVIF_TEXT;
      lvitem.iSubItem = 0;
      CString pageName(UT_(it->pageName));
      lvitem.pszText = pageName.GetBuffer();
      if (listControl.InsertItem(&lvitem) < 0)
      {
        MIKTEX_UNEXPECTED();
      }
      lvitem.iSubItem = 1;
      CString fileName(it->fileName.Get());
      lvitem.pszText = fileName.GetBuffer();
      if (!listControl.SetItem(&lvitem))
      {
        MIKTEX_UNEXPECTED();
      }
      lvitem.iSubItem = 2;
      CString line(std::to_string(it->line).c_str());
      lvitem.pszText = line.GetBuffer();
      if (!listControl.SetItem(&lvitem))
      {
        MIKTEX_UNEXPECTED();
      }
    }

    goToButton.EnableWindow(FALSE);
    editButton.EnableWindow(FALSE);
  }

  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(0, e);
  }

  catch (const exception & e)
  {
    ErrorDialog::DoModal(0, e);
  }

  return ret;
}

void SourceSpecialsDialog::OnGoto()

{
  try
  {
    int idx = listControl.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
    MIKTEX_ASSERT(idx >= 0);
    LV_ITEM item;
    _TCHAR szPageNum[BufferSizes::MaxPath];
    _TCHAR szLineNum[BufferSizes::MaxPath];
    _TCHAR szFileName[BufferSizes::MaxPath];
    item.mask = LVIF_TEXT;
    item.cchTextMax = BufferSizes::MaxPath;
    item.iItem = idx;
    item.iSubItem = 0;
    item.pszText = szPageNum;
    if (!listControl.GetItem(&item))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::GetItem");
    }
    item.iSubItem = 1;
    item.pszText = szFileName;
    if (!listControl.GetItem(&item))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::GetItem");
    }
    item.iSubItem = 2;
    item.pszText = szLineNum;
    if (!listControl.GetItem(&item))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::GetItem");
    }
    pView->GotoSrcSpecial(_ttoi(szLineNum), TU_(szFileName));
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

void SourceSpecialsDialog::OnItemchangedSourceSpecials(NMHDR * pNMHDR, LRESULT * pResult)

{
  UNUSED_ALWAYS(pNMHDR);
  bool en = listControl.GetSelectedCount() > 0;
  goToButton.EnableWindow(en);
  editButton.EnableWindow(en);
  *pResult = 0;
}

void SourceSpecialsDialog::OnEditSource()
{
  try
  {
    int idx = listControl.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
    MIKTEX_ASSERT(idx >= 0);
    LV_ITEM item;
    _TCHAR szFileName[BufferSizes::MaxPath];
    _TCHAR szLineNum[BufferSizes::MaxPath];
    item.mask = LVIF_TEXT;
    item.cchTextMax = BufferSizes::MaxPath;
    item.iItem = idx;
    item.iSubItem = 1;
    item.pszText = szFileName;
    if (!listControl.GetItem(&item))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::GetItem");
    }
    item.iSubItem = 2;
    item.pszText = szLineNum;
    if (!listControl.GetItem(&item))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::GetItem");
    }
    StartEditor(TU_(szFileName), pDviDoc->GetDocDir().Get(), _ttoi(szLineNum));
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

void SourceSpecialsDialog::OnClose()
{
  try
  {
    pView->CloseSourceSpecialsDialog();
    DestroyWindow();
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

void SourceSpecialsDialog::PostNcDestroy()
{
  CDialog::PostNcDestroy();
  delete this;
}

void SourceSpecialsDialog::OnCancel()
{
  OnClose();
}

void SourceSpecialsDialog::OnDblclkSourceSpecials(NMHDR * pNMHDR, LRESULT * pResult)
{
  UNUSED_ALWAYS(pNMHDR);
  try
  {
    OnGoto();
    *pResult = 0;
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
