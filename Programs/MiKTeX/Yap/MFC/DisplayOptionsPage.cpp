/* DisplayOptionsPage.cpp:

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

#include "DisplayOptionsPage.h"
#include "DviDoc.h"

BEGIN_MESSAGE_MAP(DisplayOptionsPage, CPropertyPage)
  ON_CBN_SELCHANGE(IDC_COMBO_DISPLAY_MODE, OnChangeMode)
  ON_CBN_SELCHANGE(IDC_COMBO_DISPLAY_SHRINK_FACTOR, &DisplayOptionsPage::OnChangeShrinkFactor)
  ON_CBN_SELCHANGE(IDC_COMBO_UNIT, &DisplayOptionsPage::OnChangeUnit)
END_MESSAGE_MAP();

DisplayOptionsPage::DisplayOptionsPage() :
  CPropertyPage(IDD),
  metafontModeIdx(static_cast<int>(g_pYapConfig->displayMetafontMode))
{
  shrinkFactor.Format(_T("%d"), g_pYapConfig->displayShrinkFactor);
}

BOOL DisplayOptionsPage::OnInitDialog()
{
  BOOL ret = CPropertyPage::OnInitDialog();

  try
  {
    MIKTEXMFMODE mode;
    for (int idx = 0; session->GetMETAFONTMode(idx, mode); ++idx)
    {
      string modeString = mode.mnemonic;
      modeString += " (";
      modeString += mode.description;
      modeString += ")";
      if (modeComboBox.AddString(UT_(modeString.c_str())) < 0)
      {
        MIKTEX_FATAL_WINDOWS_ERROR("CComboBox::AddString");
      }
      if (idx == metafontModeIdx)
      {
        if (modeComboBox.SetCurSel(idx) < 0)
        {
          MIKTEX_FATAL_WINDOWS_ERROR("CComboBox::SetCurSel");
        }
        resolution = mode.horizontalResolution;
        resolutionControl.SetWindowText(UT_(std::to_string(resolution)));
      }
    }
    int idx = -1;
    switch (g_pYapConfig->dviPageMode)
    {
    case DviPageMode::Pk:
      idx = 0;
      break;
    case DviPageMode::Dvips:
      idx = 1;
      break;
    }
    if (comboPageMode.SetCurSel(idx) < 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CComboBox::SetCurSel");
    }
    idx = -1;
    switch (g_pYapConfig->unit)
    {
    case Unit::Centimeters:
      idx = 0;
      break;
    case Unit::Inches:
      idx = 1;
      break;
    case Unit::Millimeters:
      idx = 2;
      break;
    case Unit::Picas:
      idx = 3;
      break;
    case Unit::BigPoints:
      idx = 4;
      break;
    }
    if (unitComboBox.SetCurSel(idx) < 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CComboBox::SetCurSel");
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

  return ret;
}

void DisplayOptionsPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);

  DDX_CBString(pDX, IDC_COMBO_DISPLAY_SHRINK_FACTOR, shrinkFactor);
  DDX_Control(pDX, IDC_COMBO_DISPLAY_MODE, modeComboBox);
  DDX_Control(pDX, IDC_COMBO_DISPLAY_SHRINK_FACTOR, shrinkFactorComboBox);
  DDX_Control(pDX, IDC_STATIC_RESOLUTION, resolutionControl);
  DDX_Control(pDX, IDC_COMBO_PAGE_MODE, comboPageMode);
  DDX_Control(pDX, IDC_COMBO_UNIT, unitComboBox);
}

BOOL DisplayOptionsPage::OnApply()
{
  try
  {
    g_pYapConfig->displayMetafontMode = metafontModeIdx;
    g_pYapConfig->displayShrinkFactor = _ttoi(shrinkFactor);
    if (g_pYapConfig->displayShrinkFactor == 0)
    {
      g_pYapConfig->displayShrinkFactor = 1;
    }
    int idx = comboPageMode.GetCurSel();
    if (idx < 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CComboBox::GetCurSel");
    }
    switch (idx)
    {
    case 0:
      g_pYapConfig->dviPageMode = DviPageMode::Pk;
      break;
    case 1:
      g_pYapConfig->dviPageMode = DviPageMode::Dvips;
      break;
    }
    idx = unitComboBox.GetCurSel();
    if (idx < 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CComboBox::GetCurSel");
    }
    switch (idx)
    {
    case 0:
      g_pYapConfig->unit = Unit::Centimeters;
      break;
    case 1:
      g_pYapConfig->unit = Unit::Inches;
      break;
    case 2:
      g_pYapConfig->unit = Unit::Millimeters;
      break;
    case 3:
      g_pYapConfig->unit = Unit::Picas;
      break;
    case 4:
      g_pYapConfig->unit = Unit::BigPoints;
      break;
    }
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

void DisplayOptionsPage::OnChangeMode()
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
    }
    SetModified(TRUE);
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

void DisplayOptionsPage::OnChangeShrinkFactor()
{
  SetModified(TRUE);
}

void DisplayOptionsPage::OnChangePageMode()
{
  SetModified(TRUE);
}

void DisplayOptionsPage::OnChangeUnit()
{
  SetModified(TRUE);
}
