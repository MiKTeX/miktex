/* DocumentFontsPage.cpp:

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

#include "DocumentFontsPage.h"

BEGIN_MESSAGE_MAP(DocumentFontsPage, CPropertyPage)
  ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDoubleClick)
END_MESSAGE_MAP();

DocumentFontsPage::DocumentFontsPage() :
  CPropertyPage(IDD_DOC_PROP_FONTS)
{
}

void DocumentFontsPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LIST, listControl);
}

BOOL DocumentFontsPage::OnInitDialog()
{
  BOOL ret = CPropertyPage::OnInitDialog();

  try
  {
    listControl.SetExtendedStyle(listControl.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

    if (listControl.InsertColumn(0, T_(_T("Name")), LVCFMT_LEFT, listControl.GetStringWidth(_T("xxx cmbxti12")), 0) < 0)
    {
      MIKTEX_UNEXPECTED();
    }

    if (listControl.InsertColumn(1, T_(_T("File")), LVCFMT_LEFT, listControl.GetStringWidth(_T("xxx C:\\texmf\\fonts\\pk\\ljfour\\public\\cmextra\\dpi600\\cmbxti12.pk")), 1) < 0)
    {
      MIKTEX_UNEXPECTED();
    }

    int idx = 0;

    for (vector<DviFontInfo>::const_iterator it = fontTable.begin(); it != fontTable.end(); ++it, ++idx)
    {
      LVITEM lvitem;
      lvitem.iItem = idx;
      lvitem.mask = LVIF_TEXT;
      lvitem.iSubItem = 0;
      CString name(it->name.c_str());
      lvitem.pszText = name.GetBuffer();
      if (listControl.InsertItem(&lvitem) < 0)
      {
        MIKTEX_UNEXPECTED();
      }
      lvitem.iSubItem = 1;
      CString fileName(it->fileName.c_str());
      CString notLoadable = T_(_T("   >>> Not loadable - double-click to see details"));
      lvitem.pszText = (it->notLoadable ? notLoadable.GetBuffer() : fileName.GetBuffer());
      if (!listControl.SetItem(&lvitem))
      {
        MIKTEX_UNEXPECTED();
      }
    }
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

void DocumentFontsPage::OnDoubleClick(NMHDR * pNMHDR, LRESULT * pResult)
{
  UNUSED_ALWAYS(pNMHDR);

  try
  {
    int idx = listControl.GetNextItem(-1, LVNI_SELECTED);
    if (idx < 0)
    {
      MIKTEX_UNEXPECTED();
    }
    MIKTEX_ASSERT(idx < static_cast<int>(fontTable.size()));
    const DviFontInfo & info = fontTable[idx];
    if (!info.notLoadable)
    {
      return;
    }
    CString title;
    title.Format(T_(_T("%s Logbook")), static_cast<LPTSTR>(UT_(info.name.c_str())));
    TextViewerDialog::DoModal(this, TU_(title), info.transcript.c_str());
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
