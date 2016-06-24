/* TextViewerDialog.cpp:

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

#include "miktex/UI/MFC/TextViewerDialog.h"

#include "resource.h"

using namespace MiKTeX::UI::MFC;

class TextViewerDlgImpl : public CDialog
{
public:
  TextViewerDlgImpl(CWnd * pParent, const char * lpszTitle, const char * lpszText);

private:
  BOOL OnInitDialog() override;

protected:
  void DoDataExchange(CDataExchange * pDX) override;

protected:
  DECLARE_MESSAGE_MAP();

private:
  CString title;

protected:
  CEdit editControl;

protected:
  CString text;
};

TextViewerDlgImpl::TextViewerDlgImpl(CWnd * pParent, const char * lpszTitle, const char * lpszText) :
  CDialog(IDD_TEXT_VIEWER, pParent)
{
  if (lpszTitle != nullptr)
  {
    title = lpszTitle;
  }
  if (lpszText != nullptr)
  {
    text = lpszText;
  }
}

void TextViewerDlgImpl::DoDataExchange(CDataExchange * pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_EDIT, editControl);
  DDX_Text(pDX, IDC_EDIT, text);
  editControl.SetSel(-1, -1, FALSE);
}

BEGIN_MESSAGE_MAP(TextViewerDlgImpl, CDialog)
END_MESSAGE_MAP();

BOOL TextViewerDlgImpl::OnInitDialog()
{
  BOOL ret = CDialog::OnInitDialog();
  SetWindowText(title);
  return ret;
}

INT_PTR TextViewerDialog::DoModal(CWnd * pParent, const char * lpszTitle, const char * lpszText)
{
  TextViewerDlgImpl dlg(pParent, lpszTitle, lpszText);
  return dlg.DoModal();
}
