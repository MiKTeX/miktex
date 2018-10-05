/* TextViewerDialog.cpp:

   Copyright (C) 2000-2018 Christian Schenk

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

#include "TextViewerDialog.h"

using namespace std;

class TextViewerDlgImpl :
  public CDialog
{
public:
  TextViewerDlgImpl(CWnd* parent, const string& title, const string& text);

private:
  BOOL OnInitDialog() override;

protected:
  void DoDataExchange(CDataExchange* pDX) override;

protected:
  DECLARE_MESSAGE_MAP();

private:
  CString title;

protected:
  CEdit editControl;

protected:
  CString text;
};

TextViewerDlgImpl::TextViewerDlgImpl(CWnd* parent, const string& title, const string& text) :
  CDialog(IDD_TEXT_VIEWER, parent),
  title(UT_(title)),
  text(UT_(text))
{
}

void TextViewerDlgImpl::DoDataExchange(CDataExchange* pDX)
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

INT_PTR TextViewerDialog::DoModal(CWnd* parent, const string& title, const string& text)
{
  TextViewerDlgImpl dlg(parent, title, text);
  return dlg.DoModal();
}
