/* WelcomePage.cpp:

   Copyright (C) 2000-2016 Christian Schenk

   This file is part of the Remove MiKTeX! Wizard.

   The Remove MiKTeX! Wizard is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The Remove MiKTeX! Wizard is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the Remove MiKTeX! Wizard; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"
#include "Remove.h"

#include "RemoveWizard.h"
#include "WelcomePage.h"

BEGIN_MESSAGE_MAP(WelcomePage, CPropertyPage)
  ON_BN_CLICKED(IDC_THOROUGHLY, &WelcomePage::OnThoroughly)
END_MESSAGE_MAP();

WelcomePage::WelcomePage() :
  CPropertyPage(WelcomePage::IDD)
{
  m_psp.dwFlags |= PSP_HIDEHEADER;
  m_psp.dwFlags &= ~PSP_HASHELP;
}

void WelcomePage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LIST, listBox);
  DDX_Check(pDX, IDC_THOROUGHLY, thoroughly);
  DDX_Control(pDX, IDC_THOROUGHLY, thoroughlyButton);
}

BOOL WelcomePage::OnInitDialog()
{
  pSheet = reinterpret_cast<RemoveWizard *>(GetParent());
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
    ShowItems();
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
  BOOL ret = CPropertyPage::OnSetActive();
  if (ret)
  {
    try
    {
      pSheet->SetWizardButtons(PSWIZB_NEXT);
      oldNextText = pSheet->SetNextText(T_("&Remove"));
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
  }
  return ret;
}

BOOL WelcomePage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  if (ret)
  {
    try
    {
      pSheet->SetThoroughlyFlag(thoroughly != FALSE);
      pSheet->SetNextText(TU_(oldNextText));
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
  }
  return ret;
}

void WelcomePage::OnThoroughly()
{
  thoroughly = thoroughlyButton.GetCheck();
  ShowItems();
}

void WelcomePage::ShowItems()
{
  listBox.ResetContent();
  if (!session->IsMiKTeXDirect())
  {
    if (listBox.AddString(T_(_T("Installed packages"))) < 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListBox::AddString");
    }
  }
  if (!session->IsMiKTeXDirect())
  {
    if (listBox.AddString(T_(_T("Shortcuts"))) < 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListBox::AddString");
    }
  }
  if (!session->IsMiKTeXDirect())
  {
    if (listBox.AddString(T_(_T("MiKTeX registry settings"))) < 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CListBox::AddString");
    }
  }
  if (thoroughly != FALSE)
  {
    vector<PathName> vec = GetRoots();
    for (vector<PathName>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
      if (listBox.AddString(it->ToWideCharString().c_str()) < 0)
      {
        MIKTEX_FATAL_WINDOWS_ERROR("CListBox::AddString");
      }
    }
  }
}
