/* AdvancedOptionsPage.cpp:

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

#include "AdvancedOptionsPage.h"

BEGIN_MESSAGE_MAP(AdvancedOptionsPage, CPropertyPage)
  ON_BN_CLICKED(IDC_CHECK_ASSOCIATION, &AdvancedOptionsPage::OnClickCheckBox)
  ON_BN_CLICKED(IDC_CHECK_SPLASH, &AdvancedOptionsPage::OnClickCheckBox)
END_MESSAGE_MAP();

AdvancedOptionsPage::AdvancedOptionsPage() :
  CPropertyPage(IDD),
  checkFileTypeAssociations(g_pYapConfig->checkFileTypeAssociations ? BST_CHECKED : BST_UNCHECKED),
  showSplashWindow(g_pYapConfig->showSplashWindow ? BST_CHECKED : BST_UNCHECKED)
{
}

void AdvancedOptionsPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Check(pDX, IDC_CHECK_ASSOCIATION, checkFileTypeAssociations);
  DDX_Check(pDX, IDC_CHECK_SPLASH, showSplashWindow);
}

BOOL AdvancedOptionsPage::OnApply()
{
  try
  {
    g_pYapConfig->checkFileTypeAssociations = checkFileTypeAssociations ? true : false;
    g_pYapConfig->showSplashWindow = showSplashWindow ? true : false;
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

void AdvancedOptionsPage::OnClickCheckBox()
{
  SetModified(TRUE);
}

BOOL AdvancedOptionsPage::OnInitDialog()
{
  BOOL ret = CPropertyPage::OnInitDialog();
  return ret;
}
