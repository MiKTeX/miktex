/* ProxyAuthenticationDialog.cpp:

   Copyright (C) 2000-2018 Christian Schenk

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

#include "ProxyAuthenticationDialog.h"

BEGIN_MESSAGE_MAP(ProxyAuthenticationDialog, CDialog)
  ON_EN_CHANGE(IDC_PROXY_NAME, &ProxyAuthenticationDialog::OnChangeName)
END_MESSAGE_MAP();

ProxyAuthenticationDialog::ProxyAuthenticationDialog(CWnd* parent) :
  CDialog(ProxyAuthenticationDialog::IDD, parent)
{
}

BOOL ProxyAuthenticationDialog::OnInitDialog()
{
  BOOL ret = CDialog::OnInitDialog();
  OnChangeName();
  return ret;
}

void ProxyAuthenticationDialog::DoDataExchange(CDataExchange* dx)
{
  CDialog::DoDataExchange(dx);
  DDX_Text(dx, IDC_PROXY_NAME, name);
  DDX_Text(dx, IDC_PROXY_PASSWORD, password);
}

void ProxyAuthenticationDialog::OnChangeName()
{
  CWnd* wnd = GetDlgItem(IDC_PROXY_NAME);
  if (wnd == nullptr)
  {
    return;
  }
  CWnd* okayButton = GetDlgItem(IDOK);
  if (okayButton == nullptr)
  {
    return;
  }
  CString str;
  wnd->GetWindowText(str);
  okayButton->EnableWindow(!str.IsEmpty());
}
