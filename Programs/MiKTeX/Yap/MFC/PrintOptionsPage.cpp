/* PrintOptionsPage.cpp:

   Copyright (C) 1996-2016 Christian Schenk

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

#include "DviDoc.h"
#include "PrintOptionsPage.h"

BEGIN_MESSAGE_MAP(PrintOptionsPage, CPropertyPage)
  ON_CBN_SELCHANGE(IDC_COMBO_PRINTER_MODE, OnChangeMode)
  ON_EN_CHANGE(IDC_EDIT2, &PrintOptionsPage::OnChangeXShift)
  ON_EN_CHANGE(IDC_EDIT3, &PrintOptionsPage::OnChangeYShift)
END_MESSAGE_MAP();

PrintOptionsPage::PrintOptionsPage() :
  CPropertyPage(IDD),
  metafontModeIdx(static_cast<int>(g_pYapConfig->printerMetafontMode)),
  xShift(g_pYapConfig->pageXShift),
  yShift(g_pYapConfig->pageYShift)
{
}

BOOL PrintOptionsPage::OnInitDialog()
{
  BOOL ret = CPropertyPage::OnInitDialog();

  try
  {
    MIKTEXMFMODE mode;
    for (int idx = 0; session->GetMETAFONTMode(idx, mode); ++idx)
    {
      CString modeString = UT_(mode.mnemonic);
      modeString += _T(" (");
      modeString += UT_(mode.description);
      modeString += _T(")");
      if (modeComboBox.AddString(modeString) < 0)
      {
        MIKTEX_UNEXPECTED();
      }
      if (idx == metafontModeIdx)
      {
        if (modeComboBox.SetCurSel(idx) < 0)
        {
          MIKTEX_UNEXPECTED();
        }
        resolution = mode.horizontalResolution;
        resolutionControl.SetWindowText(UT_(std::to_string(resolution)));
      }
    }
  }

  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(0, e);
  }

  catch (const exception & e)
  {
    ErrorDialog::DoModal(0, e);
  }

  return ret;
}

void PrintOptionsPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);

  DDV_MinMaxLong(pDX, xShift, -1000, 1000);
  DDV_MinMaxLong(pDX, yShift, -1000, 1000);
  DDX_Control(pDX, IDC_COMBO_PRINTER_MODE, modeComboBox);
  DDX_Control(pDX, IDC_STATIC_RESOLUTION, resolutionControl);
  DDX_Text(pDX, IDC_EDIT2, xShift);
  DDX_Text(pDX, IDC_EDIT3, yShift);
}

BOOL PrintOptionsPage::OnApply()
{
  try
  {
    g_pYapConfig->printerMetafontMode = metafontModeIdx;
    g_pYapConfig->pageXShift = xShift;
    g_pYapConfig->pageYShift = yShift;
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

void PrintOptionsPage::OnChangeMode()
{
  try
  {
    int idx = modeComboBox.GetCurSel();
    if (idx < 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CComboBox::GetCurSel");
    }
    MIKTEXMFMODE mode;
    if (session->GetMETAFONTMode(idx, mode))
    {
      resolution = mode.horizontalResolution;
      resolutionControl.SetWindowText(UT_(std::to_string(resolution)));
      metafontModeIdx = idx;
      SetModified(TRUE);
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void PrintOptionsPage::OnChangeXShift()
{
  SetModified(TRUE);
}

void PrintOptionsPage::OnChangeYShift()
{
  SetModified(TRUE);
}
