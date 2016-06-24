/* PropPageFormats.cpp:

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

#include "FormatDefinitionDialog.h"
#include "miktex/UI/MFC/ProgressDialog"
#include "PropPageFormats.h"
#include "PropSheet.h"
#include "resource.hm"

PropPageFormats::PropPageFormats() :
  CPropertyPage(PropPageFormats::IDD)
{
  m_psp.dwFlags &= ~(PSP_HASHELP);
}

void PropPageFormats::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_EDIT, editButton);
  DDX_Control(pDX, IDC_LIST, listControl);
  DDX_Control(pDX, IDC_MAKE, makeButton);
  DDX_Control(pDX, IDC_NEW, newButton);
  DDX_Control(pDX, IDC_REMOVE, removeButton);
}

BEGIN_MESSAGE_MAP(PropPageFormats, CPropertyPage)
  ON_BN_CLICKED(IDC_EDIT, OnEdit)
  ON_BN_CLICKED(IDC_MAKE, OnMake)
  ON_BN_CLICKED(IDC_NEW, OnNew)
  ON_BN_CLICKED(IDC_REMOVE, OnRemove)
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnSelectionChange)
  ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnListDoubleClick)
  ON_WM_CONTEXTMENU()
  ON_WM_HELPINFO()
END_MESSAGE_MAP();

void PropPageFormats::OnMake()
{
  try
  {
    PropSheet * pSheet = reinterpret_cast<PropSheet*>(GetParent());
    unique_ptr<ProgressDialog> ppd(ProgressDialog::Create());
    ppd->StartProgressDialog(GetParent()->GetSafeHwnd());
    if (listControl.GetSelectedCount() > 1)
    {
      ppd->SetTitle(T_("Making Format Files"));
    }
    else
    {
      ppd->SetTitle(T_("Making Format File"));
    }
    ppd->SetLine(1, T_("Creating format file:"));
    POSITION pos = listControl.GetFirstSelectedItemPosition();
    while (pos != nullptr)
    {
      int idx = listControl.GetNextSelectedItem(pos);
      CString formatKey = listControl.GetItemText(idx, 0);
      FormatInfo formatInfo = session->GetFormatInfo(TU_(formatKey));
      CommandLineBuilder cmdLine;
      cmdLine.AppendOption("--dump=", formatInfo.key);
      if (!pSheet->RunIniTeXMF(formatInfo.description.c_str(), cmdLine, ppd.get()))
      {
        // TODO
      }
    }
    ppd->StopProgressDialog();
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

void PropPageFormats::OnNew()
{
  try
  {
    FormatDefinitionDialog dlg(this, nullptr);
    if (dlg.DoModal() != IDOK)
    {
      return;
    }
    FormatInfo formatInfo = dlg.GetFormatInfo();
    session->SetFormatInfo(formatInfo);
    Refresh();
    MakeAlias(formatInfo);
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

void PropPageFormats::MakeAlias(const FormatInfo & formatInfo)
{
  if (session->IsMiKTeXDirect())
  {
    MIKTEX_FATAL_ERROR(T_("Operation not supported."));
  }
  PathName compilerPath;
  if (!session->FindFile(formatInfo.compiler.c_str(), FileType::EXE, compilerPath))
  {
    MIKTEX_FATAL_ERROR_2(T_("The compiler could not be found."), "compuler", formatInfo.compiler);
  }
  PathName pathBinDir = session->GetSpecialPath(SpecialPath::BinDirectory);
  PathName pathAlias(pathBinDir, formatInfo.name, ".exe");
  if (compilerPath != pathAlias && !File::Exists(pathAlias))
  {
    Directory::Create(pathBinDir);
    File::Copy(compilerPath, pathAlias);
    if (!Fndb::FileExists(pathAlias))
    {
      Fndb::Add(pathAlias);
    }
  }
}

void PropPageFormats::OnEdit()
{
  try
  {
    int idx = GetSelectedItem();
    CString formatKey = listControl.GetItemText(idx, 0);
    FormatDefinitionDialog dlg(this, TU_(formatKey));
    if (dlg.DoModal() != IDOK)
    {
      return;
    }
    FormatInfo formatInfo = dlg.GetFormatInfo();
    if (PathName::Compare(formatInfo.key.c_str(), TU_(formatKey)) != 0)
    {
      // rename key: delete old, create new
      session->DeleteFormatInfo(TU_(formatKey));
      formatKey = UT_(formatInfo.key);
    }
    session->SetFormatInfo(formatInfo);
    Refresh();
    MakeAlias(formatInfo);
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

void PropPageFormats::OnRemove()
{
  try
  {
    int n = listControl.GetSelectedCount();
    for (int i = 0; i < n; ++i)
    {
      int idx = GetSelectedItem();
      CString formatKey = listControl.GetItemText(idx, 0);
      session->DeleteFormatInfo(TU_(formatKey));
      if (!listControl.DeleteItem(idx))
      {
        MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::DeleteItem");
      }
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
}

void PropPageFormats::InsertColumn(int colIdx, const char * lpszLabel, const char * lpszLongest)
{
  if (listControl.InsertColumn(colIdx, UT_(lpszLabel), LVCFMT_LEFT, listControl.GetStringWidth(UT_(lpszLongest)), colIdx) < 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::InsertColumn");
  }
}

BOOL PropPageFormats::OnInitDialog()
{
  BOOL ret = CPropertyPage::OnInitDialog();

  try
  {
    listControl.SetExtendedStyle(listControl.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

    InsertColumn(0, T_("Key"), T_("pdfjadetex     "));
    InsertColumn(1, T_("Description"), T_("pdfLaTeX bla bla     "));
    InsertColumn(2, T_("Attributes"), T_("Exclude, bla bla  "));

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

void PropPageFormats::EnableButtons()
{
  UINT n = listControl.GetSelectedCount();
  makeButton.EnableWindow(n > 0 && !modified);
  removeButton.EnableWindow(n > 0);
  editButton.EnableWindow(n == 1);
}

int PropPageFormats::GetSelectedItem()
{
  POSITION pos = listControl.GetFirstSelectedItemPosition();
  if (pos == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  return listControl.GetNextSelectedItem(pos);
}

void PropPageFormats::Refresh()
{
  if (!listControl.DeleteAllItems())
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::DeleteAllItems");
  }

  int idx = 0;
  for (const FormatInfo & formatInfo : session->GetFormats())
  {
    LV_ITEM lvitem;
    lvitem.iItem = static_cast<int>(idx);
    lvitem.mask = LVIF_TEXT | LVIF_PARAM;
    lvitem.iSubItem = 0;
    CString key(UT_(formatInfo.key));
    lvitem.pszText = key.GetBuffer();
    lvitem.lParam = idx;
    int whereIndex = listControl.InsertItem(&lvitem);
    if (whereIndex < 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::InsertItem");
    }
    lvitem.iItem = whereIndex;
    lvitem.mask = LVIF_TEXT;
    lvitem.iSubItem = 1;
    CString description(UT_(formatInfo.description));
    lvitem.pszText = description.GetBuffer();
    if (!listControl.SetItem(&lvitem))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SetItem");
    }
    lvitem.mask = LVIF_TEXT;
    lvitem.iSubItem = 2;
    lvitem.pszText = (formatInfo.exclude ? T_(_T("exclude")) : _T(""));
    if (!listControl.SetItem(&lvitem))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SetItem");
    }
    ++idx;
  }

  EnableButtons();
}

#define MAKE_ID_HID_PAIR(id) id, H##id

namespace
{
  const DWORD aHelpIDs[] = {
    MAKE_ID_HID_PAIR(IDC_EDIT),
    MAKE_ID_HID_PAIR(IDC_LIST),
    MAKE_ID_HID_PAIR(IDC_MAKE),
    MAKE_ID_HID_PAIR(IDC_NEW),
    MAKE_ID_HID_PAIR(IDC_REMOVE),
    0, 0, };
}

BOOL PropPageFormats::OnHelpInfo(HELPINFO * pHelpInfo)
{
  return ::OnHelpInfo(pHelpInfo, aHelpIDs, "FormatsPage.txt");
}

void PropPageFormats::OnContextMenu(CWnd * pWnd, CPoint point)
{
  try
  {
    DoWhatsThisMenu(pWnd, point, aHelpIDs, "FormatsPage.txt");
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

void PropPageFormats::OnSelectionChange(NMHDR * pNMHDR, LRESULT * pResult)
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

void PropPageFormats::OnListDoubleClick(NMHDR * pNMHDR, LRESULT * pResult)
{
  UNUSED_ALWAYS(pNMHDR);
  try
  {
    OnEdit();
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
