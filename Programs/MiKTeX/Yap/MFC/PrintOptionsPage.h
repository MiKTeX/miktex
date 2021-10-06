/* PrintOptionsPage.h:                                  -*- C++ -*-

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

class PrintOptionsPage :
  public CPropertyPage
{
private:
  enum { IDD = IDD_PRINTER };

protected:
  DECLARE_MESSAGE_MAP();

public:
  PrintOptionsPage();

protected:
  BOOL OnInitDialog() override;

protected:
  void DoDataExchange(CDataExchange* pDX) override;

protected:
  BOOL OnApply() override;

protected:
  afx_msg void OnChangeMode();

protected:
  afx_msg void OnChangeXShift();

protected:
  afx_msg void OnChangeYShift();

private:
  CStatic resolutionControl;

private:
  CComboBox modeComboBox;

private:
  int resolution;

private:
  int metafontModeIdx;

private:
  long xShift;

private:
  long yShift;

private:
  shared_ptr<Session> session = MIKTEX_SESSION();
};
