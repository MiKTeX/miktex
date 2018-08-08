/* AddTEXMFPage.cpp:

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

#include "AddTEXMFPage.h"

/* _________________________________________________________________________

   AddTEXMFPage Message Map
   _________________________________________________________________________ */

BEGIN_MESSAGE_MAP(AddTEXMFPage, CPropertyPage)
END_MESSAGE_MAP();

/* _________________________________________________________________________

   AddTEXMFPage::AddTEXMFPage
   _________________________________________________________________________ */

AddTEXMFPage::AddTEXMFPage ()
  : CPropertyPage(IDD,
		  0,
		  IDS_HEADER_ADDTREES,
		  IDS_SUBHEADER_ADDTREES),
    noAddTEXMF (0)
{
}

/* _________________________________________________________________________

   AddTEXMFPage::OnInitDialog
   _________________________________________________________________________ */

BOOL
AddTEXMFPage::OnInitDialog ()
{
  return (CPropertyPage::OnInitDialog());
}

/* _________________________________________________________________________

   AddTEXMFPage::DoDataExchange
   _________________________________________________________________________ */

void AddTEXMFPage::DoDataExchange (/*[in]*/ CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange (pDX);
  DDX_Radio (pDX, IDC_NOADDTEXMF, noAddTEXMF);
}

/* _________________________________________________________________________

   AddTEXMFPage::OnWizardNext
   _________________________________________________________________________ */

LRESULT
AddTEXMFPage::OnWizardNext ()
{
  UINT uNext;
  switch (GetCheckedRadioButton(IDC_NOADDTEXMF, IDC_ADDTEXMF))
    {
    case IDC_NOADDTEXMF:
      uNext = IDD_INFOLIST;
      break;
    case IDC_ADDTEXMF:
      uNext = IDD_EDIT_TEXMFROOTS;
      break;
    default:
      ASSERT (false);
      __assume (false);
      break;
    }
  return (reinterpret_cast<LRESULT>(MAKEINTRESOURCE(uNext)));
}

/* _________________________________________________________________________

   AddTEXMFPage::OnWizardBack
   _________________________________________________________________________ */

LRESULT
AddTEXMFPage::OnWizardBack ()
{
  return (reinterpret_cast<LRESULT>(MAKEINTRESOURCE(IDD_SETTINGS)));
}

/* _________________________________________________________________________

   AddTEXMFPage::OnKillActive
   _________________________________________________________________________ */

BOOL
AddTEXMFPage::OnKillActive ()
{
  BOOL ret = CPropertyPage::OnKillActive();
  if (ret)
    {
      theApp.noAddTEXMFDirs = (noAddTEXMF == 0);
    }
  return (ret);
}
