/* ProxyAuthenticationDialog.cpp:

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

#include "ProxyAuthenticationDialog.h"

IMPLEMENT_DYNAMIC(ProxyAuthenticationDialog, CDialog);

BEGIN_MESSAGE_MAP(ProxyAuthenticationDialog, CDialog)
  ON_EN_CHANGE(IDC_PROXY_NAME, &ProxyAuthenticationDialog::OnChangeName)
END_MESSAGE_MAP();

ProxyAuthenticationDialog::ProxyAuthenticationDialog(CWnd * pParent) :
  CDialog(ProxyAuthenticationDialog::IDD, pParent)
{
}

BOOL ProxyAuthenticationDialog::OnInitDialog()
{
  BOOL ret = CDialog::OnInitDialog();
  OnChangeName();
  return ret;
}

void ProxyAuthenticationDialog::DoDataExchange(CDataExchange * pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_PROXY_NAME, name);
  DDX_Text(pDX, IDC_PROXY_PASSWORD, password);
}

void ProxyAuthenticationDialog::OnChangeName()
{
  CWnd * pWnd = GetDlgItem(IDC_PROXY_NAME);
  if (pWnd == nullptr)
  {
    return;
  }
  CWnd * pOK = GetDlgItem(IDOK);
  if (pOK == nullptr)
  {
    return;
  }
  CString str;
  pWnd->GetWindowText(str);
  pOK->EnableWindow(!str.IsEmpty());
}
