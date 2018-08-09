/* MagnifyingGlassOptionsPage.h:                        -*- C++ -*-

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

#pragma once

#include "afxwin.h"

class MagnifyingGlassOptionsPage :
  public CPropertyPage
{
private:
  enum { IDD = IDD_MAGNIFYING_GLASS };

protected:
  DECLARE_MESSAGE_MAP();

public:
  MagnifyingGlassOptionsPage();

protected:
  virtual
    BOOL OnInitDialog();

protected:
  virtual
    void DoDataExchange(CDataExchange* pDX);

protected:
  virtual BOOL OnApply();

protected:
  afx_msg void OnChangeSmallWidth();

protected:
  afx_msg void OnChangeMediumWidth();

protected:
  afx_msg void OnChangeLargeWidth();

protected:
  afx_msg void OnChangeSmallHeight();

protected:
  afx_msg void OnChangeMediumHeight();

protected:
  afx_msg void OnChangeLargeHeight();

protected:
  afx_msg void OnChangeSmallShrinkFactor();

protected:
  afx_msg void OnChangeMediumShrinkFactor();

protected:
  afx_msg void OnChangeLargeShrinkFactor();

protected:
  afx_msg void OnClickHideMouseCursor();

protected:
  afx_msg void OnChangeOpacity();

private:
  UINT mediumHeight;

private:
  UINT largeHeight;

private:
  UINT smallHeight;

private:
  UINT largeWidth;

private:
  UINT mediumWidth;

private:
  UINT smallWidth;

private:
  CString largeShrinkFactor;

private:
  CString mediumShrinkFactor;

private:
  CString smallShrinkFactor;

private:
  BOOL hideCursor;

private:
  CComboBox comboOpacity;
};
