/* SecurityOptionsPage.cpp:

   Copyright (C) 1996-2018 Christian Schenk

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

#include "ErrorDialog.h"

#include "SecurityOptionsPage.h"

BEGIN_MESSAGE_MAP(SecurityOptionsPage, CPropertyPage)
  ON_EN_CHANGE(IDC_SECURITY_EDIT_SECURE_COMMANDS, OnChangeSecureCommands)
  ON_BN_CLICKED(IDC_SECURITY_DISABLE_COMMANDS, &SecurityOptionsPage::OnClickNever)
  ON_BN_CLICKED(IDC_SECURITY_ASK_USER, &SecurityOptionsPage::OnClickAsk)
  ON_BN_CLICKED(IDC_SECURITY_SECURE_COMMANDS, &SecurityOptionsPage::OnClickSecure)
  ON_BN_CLICKED(IDC_SECURITY_ENABLE_ALL_COMMANDS, &SecurityOptionsPage::OnClickAll)
END_MESSAGE_MAP();

SecurityOptionsPage::SecurityOptionsPage() :
  CPropertyPage(SecurityOptionsPage::IDD),
  enableShellCommands(g_pYapConfig->enableShellCommands),
  secureCommands(UT_(g_pYapConfig->secureCommands))
{
}

void SecurityOptionsPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Radio(pDX, IDC_SECURITY_DISABLE_COMMANDS, enableShellCommands);
  DDX_Text(pDX, IDC_SECURITY_EDIT_SECURE_COMMANDS, secureCommands);
}

BOOL SecurityOptionsPage::OnApply()
{
  try
  {
    g_pYapConfig->enableShellCommands = enableShellCommands;
    g_pYapConfig->secureCommands = TU_(secureCommands);
    return CPropertyPage::OnApply();
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

void SecurityOptionsPage::OnChangeSecureCommands()
{
  CheckRadioButton(IDC_SECURITY_DISABLE_COMMANDS, IDC_SECURITY_ENABLE_ALL_COMMANDS, IDC_SECURITY_SECURE_COMMANDS);
  SetModified(TRUE);
}

void SecurityOptionsPage::OnClickNever()
{
  SetModified(TRUE);
}

void SecurityOptionsPage::OnClickAsk()
{
  SetModified(TRUE);
}

void SecurityOptionsPage::OnClickSecure()
{
  SetModified(TRUE);
}

void SecurityOptionsPage::OnClickAll()
{
  SetModified(TRUE);
}
