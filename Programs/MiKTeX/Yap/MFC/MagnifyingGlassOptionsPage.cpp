/* MagnifyingGlassOptionsPage.cpp:

   Copyright (C) 1996-2020 Christian Schenk

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

#include "MagnifyingGlassOptionsPage.h"

BEGIN_MESSAGE_MAP(MagnifyingGlassOptionsPage, CPropertyPage)
  ON_BN_CLICKED(IDC_HIDE_CURSOR, &MagnifyingGlassOptionsPage::OnClickHideMouseCursor)
  ON_CBN_SELCHANGE(IDC_MAGGLASS_LARGE_SHRINK_FACTOR, &MagnifyingGlassOptionsPage::OnChangeLargeShrinkFactor)
  ON_CBN_SELCHANGE(IDC_MAGGLASS_MEDIUM_SHRINK_FACTOR, &MagnifyingGlassOptionsPage::OnChangeMediumShrinkFactor)
  ON_CBN_SELCHANGE(IDC_MAGGLASS_SMALL_SHRINK_FACTOR, &MagnifyingGlassOptionsPage::OnChangeSmallShrinkFactor)
  ON_EN_CHANGE(IDC_MAGGLASS_LARGE_HEIGHT, &MagnifyingGlassOptionsPage::OnChangeLargeHeight)
  ON_EN_CHANGE(IDC_MAGGLASS_LARGE_WIDTH, &MagnifyingGlassOptionsPage::OnChangeLargeWidth)
  ON_EN_CHANGE(IDC_MAGGLASS_MEDIUM_HEIGHT, &MagnifyingGlassOptionsPage::OnChangeMediumHeight)
  ON_EN_CHANGE(IDC_MAGGLASS_MEDIUM_WIDTH, &MagnifyingGlassOptionsPage::OnChangeMediumWidth)
  ON_EN_CHANGE(IDC_MAGGLASS_SMALL_HEIGHT, &MagnifyingGlassOptionsPage::OnChangeSmallHeight)
  ON_EN_CHANGE(IDC_MAGGLASS_SMALL_WIDTH, &MagnifyingGlassOptionsPage::OnChangeSmallWidth)
  ON_CBN_SELCHANGE(IDC_COMBO_OPACITY, &MagnifyingGlassOptionsPage::OnChangeOpacity)
END_MESSAGE_MAP();

MagnifyingGlassOptionsPage::MagnifyingGlassOptionsPage() :
  CPropertyPage(IDD),
  hideCursor(g_pYapConfig->magGlassHidesCursor),
  smallWidth(g_pYapConfig->magGlassSmallWidth),
  smallHeight(g_pYapConfig->magGlassSmallHeight),
  mediumWidth(g_pYapConfig->magGlassMediumWidth),
  mediumHeight(g_pYapConfig->magGlassMediumHeight),
  largeWidth(g_pYapConfig->magGlassLargeWidth),
  largeHeight(g_pYapConfig->magGlassLargeHeight)
{
  smallShrinkFactor.Format(_T("%d"), g_pYapConfig->magGlassSmallShrinkFactor);
  mediumShrinkFactor.Format(_T("%d"), g_pYapConfig->magGlassMediumShrinkFactor);
  largeShrinkFactor.Format(_T("%d"), g_pYapConfig->magGlassLargeShrinkFactor);
}

BOOL MagnifyingGlassOptionsPage::OnInitDialog()
{
  BOOL ret = CPropertyPage::OnInitDialog();
  try
  {
    int idx;
    if (g_pYapConfig->magGlassOpacity < 10)
    {
      idx = 0;
    }
    else if (g_pYapConfig->magGlassOpacity >= 100)
    {
      idx = 9;
    }
    else
    {
      idx = (g_pYapConfig->magGlassOpacity / 10 - 1);
    }
    if (comboOpacity.SetCurSel(idx) < 0)
    {
      MIKTEX_UNEXPECTED();
    }
  }
  catch (const MiKTeXException& e)
  {
    ShowError(this, e);
  }
  catch (const exception& e)
  {
    ShowError(this, e);
  }
  return ret;
}

void MagnifyingGlassOptionsPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_MAGGLASS_MEDIUM_HEIGHT, mediumHeight);
  DDX_Text(pDX, IDC_MAGGLASS_LARGE_HEIGHT, largeHeight);
  DDX_Text(pDX, IDC_MAGGLASS_SMALL_HEIGHT, smallHeight);
  DDX_Text(pDX, IDC_MAGGLASS_LARGE_WIDTH, largeWidth);
  DDX_Text(pDX, IDC_MAGGLASS_MEDIUM_WIDTH, mediumWidth);
  DDX_Text(pDX, IDC_MAGGLASS_SMALL_WIDTH, smallWidth);
  DDX_CBString(pDX,
    IDC_MAGGLASS_LARGE_SHRINK_FACTOR,
    largeShrinkFactor);
  DDX_CBString(pDX,
    IDC_MAGGLASS_MEDIUM_SHRINK_FACTOR,
    mediumShrinkFactor);
  DDX_CBString(pDX,
    IDC_MAGGLASS_SMALL_SHRINK_FACTOR,
    smallShrinkFactor);
  DDX_Check(pDX, IDC_HIDE_CURSOR, hideCursor);
  DDX_Control(pDX, IDC_COMBO_OPACITY, comboOpacity);
}

BOOL MagnifyingGlassOptionsPage::OnApply()
{
  try
  {
    g_pYapConfig->magGlassHidesCursor = (hideCursor ? true : false);

    int idx = comboOpacity.GetCurSel();

    if (idx < 0 || idx > 9)
    {
      MIKTEX_UNEXPECTED();
    }

    g_pYapConfig->magGlassOpacity = (idx + 1) * 10;

    g_pYapConfig->magGlassSmallWidth = smallWidth;
    g_pYapConfig->magGlassSmallHeight = smallHeight;
    g_pYapConfig->magGlassSmallShrinkFactor = _ttoi(smallShrinkFactor);
    if (g_pYapConfig->magGlassSmallShrinkFactor == 0)
    {
      g_pYapConfig->magGlassSmallShrinkFactor = 1;
    }

    g_pYapConfig->magGlassMediumWidth = mediumWidth;
    g_pYapConfig->magGlassMediumHeight = mediumHeight;
    g_pYapConfig->magGlassMediumShrinkFactor = _ttoi(mediumShrinkFactor);
    if (g_pYapConfig->magGlassMediumShrinkFactor == 0)
    {
      g_pYapConfig->magGlassMediumShrinkFactor = 1;
    }

    g_pYapConfig->magGlassLargeWidth = largeWidth;
    g_pYapConfig->magGlassLargeHeight = largeHeight;
    g_pYapConfig->magGlassLargeShrinkFactor = _ttoi(largeShrinkFactor);
    if (g_pYapConfig->magGlassLargeShrinkFactor == 0)
    {
      g_pYapConfig->magGlassLargeShrinkFactor = 1;
    }

    return CPropertyPage::OnApply();
  }
  catch (const MiKTeXException& e)
  {
    ShowError(this, e);
    return FALSE;
  }
  catch (const exception& e)
  {
    ShowError(this, e);
    return FALSE;
  }
}

void MagnifyingGlassOptionsPage::OnChangeSmallWidth()
{
  SetModified(TRUE);
}

void MagnifyingGlassOptionsPage::OnChangeMediumWidth()
{
  SetModified(TRUE);
}

void MagnifyingGlassOptionsPage::OnChangeLargeWidth()
{
  SetModified(TRUE);
}

void MagnifyingGlassOptionsPage::OnChangeSmallHeight()
{
  SetModified(TRUE);
}

void MagnifyingGlassOptionsPage::OnChangeMediumHeight()
{
  SetModified(TRUE);
}

void MagnifyingGlassOptionsPage::OnChangeLargeHeight()
{
  SetModified(TRUE);
}

void MagnifyingGlassOptionsPage::OnChangeSmallShrinkFactor()
{
  SetModified(TRUE);
}

void MagnifyingGlassOptionsPage::OnChangeMediumShrinkFactor()
{
  SetModified(TRUE);
}

void MagnifyingGlassOptionsPage::OnChangeLargeShrinkFactor()
{
  SetModified(TRUE);
}

void MagnifyingGlassOptionsPage::OnClickHideMouseCursor()
{
  SetModified(TRUE);
}

void MagnifyingGlassOptionsPage::OnChangeOpacity()
{
  SetModified(TRUE);
}
