/* UIOptionsPage.cpp:

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

#include "UIOptionsPage.h"

BEGIN_MESSAGE_MAP(UIOptionsPage, CPropertyPage)
  ON_BN_CLICKED(IDC_MAINTAIN_VERT_POS, &UIOptionsPage::OnClickMaintainVerticalPosition)
  ON_BN_CLICKED(IDC_MAINTAIN_HORZ_POS, &UIOptionsPage::OnClickMaintainHorizontalPosition)
  ON_BN_CLICKED(IDC_SHOW_SOURCE_SPECIALS, &UIOptionsPage::OnClickShowSourceSpecials)
END_MESSAGE_MAP();

UIOptionsPage::UIOptionsPage() :
  CPropertyPage(IDD),
  maintainHorizontalPosition(g_pYapConfig->maintainHorizontalPosition),
  maintainVerticalPosition(g_pYapConfig->maintainVerticalPosition),
  showSourceSpecials(g_pYapConfig->showSourceSpecials)
{
}

void UIOptionsPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Check(pDX, IDC_MAINTAIN_HORZ_POS, maintainHorizontalPosition);
  DDX_Check(pDX, IDC_MAINTAIN_VERT_POS, maintainVerticalPosition);
  DDX_Check(pDX, IDC_SHOW_SOURCE_SPECIALS, showSourceSpecials);
}

BOOL UIOptionsPage::OnApply()
{
  try
  {
    g_pYapConfig->maintainHorizontalPosition = maintainHorizontalPosition ? true : false;
    g_pYapConfig->maintainVerticalPosition = maintainVerticalPosition ? true : false;
    g_pYapConfig->showSourceSpecials = showSourceSpecials ? true : false;

    return CPropertyPage::OnApply();
  }
  catch (const MiKTeXException& e)
  {
    ErrorDialog::DoModal(this, e);
    return FALSE;
  }
  catch (const exception& e)
  {
    ErrorDialog::DoModal(this, e);
    return FALSE;
  }
}

void UIOptionsPage::OnClickMaintainVerticalPosition()
{
  SetModified(TRUE);
}

void UIOptionsPage::OnClickMaintainHorizontalPosition()
{
  SetModified(TRUE);
}

void UIOptionsPage::OnClickShowSourceSpecials()
{
  SetModified(TRUE);
}
