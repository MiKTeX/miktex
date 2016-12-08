/* WelcomePage.cpp:

   Copyright (C) 2002-2016 Christian Schenk

   This file is part of the MiKTeX Update Wizard.

   The MiKTeX Update Wizard is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Update Wizard is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Update Wizard; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"
#include "Update.h"

#include "UpdateWizard.h"
#include "WelcomePage.h"

IMPLEMENT_DYNCREATE(WelcomePage, CPropertyPage);

BEGIN_MESSAGE_MAP(WelcomePage, CPropertyPage)
END_MESSAGE_MAP();

WelcomePage::WelcomePage() :
  CPropertyPage(IDD)
{
  m_psp.dwFlags |= PSP_HIDEHEADER;
  alwaysShow = session->GetConfigValue("Update", "alwaysWelcome", false).GetBool() ? TRUE : FALSE;
}

BOOL WelcomePage::OnInitDialog()
{
  pSheet = reinterpret_cast<UpdateWizard *>(GetParent());
  BOOL ret = CPropertyPage::OnInitDialog();
  try
  {
    CWnd * pWnd = GetDlgItem(IDC_WELCOME);
    if (pWnd == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    extern CFont fntWelcome;
    pWnd->SetFont(&fntWelcome);
  }
  catch (const MiKTeXException & e)
  {
    pSheet->ReportError(e);
  }
  catch (const exception & e)
  {
    pSheet->ReportError(e);
  }
  return ret;
}

BOOL WelcomePage::OnSetActive()
{
  pSheet->SetWizardButtons(PSWIZB_NEXT);
  return CPropertyPage::OnSetActive();
}

void WelcomePage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Check(pDX, IDC_ALWAYS_SHOW, alwaysShow);
}

LRESULT WelcomePage::OnWizardNext()
{
  pSheet->SetCameFrom(IDD);
  return CPropertyPage::OnWizardNext();
}

BOOL WelcomePage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  try
  {
    session->SetConfigValue("Update", "alwaysWelcome", alwaysShow == 0 ? false : true);
  }
  catch (const MiKTeXException & e)
  {
    pSheet->ReportError(e);
    ret = FALSE;
  }
  catch (const exception & e)
  {
    pSheet->ReportError(e);
    ret = FALSE;
  }
  return ret;
}
