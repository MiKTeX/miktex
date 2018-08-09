/* GotoPageDialog.h:                                    -*- C++ -*-

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

#include "yap.h"

class DviDoc;

class GotoPageDialog :
  public CDialog
{
private:
  enum { IDD = IDD_GOTOPAGE };

protected:
  DECLARE_MESSAGE_MAP();

public:
  GotoPageDialog(CWnd* pParent, DviDoc* pDoc, int pageIdx);

public:
  int GetPageIdx() const
  {
    return pageIdx;
  }

protected:
  BOOL OnInitDialog() override;

protected:
  void DoDataExchange(CDataExchange* pDX) override;

private:
  CComboBox pageComboBox;

private:
  int pageIdx;

private:
  DviDoc* pDviDoc;
};
