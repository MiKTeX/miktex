/* PropPageFormats.h:                                   -*- C++ -*-

   Copyright (C) 2000-2016 Christian Schenk

   This file is part of MiKTeX Options.

   MiKTeX Options is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX Options is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Options; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

class PropPageFormats :
  public CPropertyPage
{
private:
  enum { IDD = IDD_PROPPAGE_FMT };

protected:
  DECLARE_MESSAGE_MAP();

public:
  PropPageFormats();

public:
  CButton editButton;

public:
  CButton removeButton;

public:
  CButton makeButton;

public:
  CButton newButton;

public:
  CListCtrl listControl;

public:
  virtual BOOL OnInitDialog();

public:
  virtual void DoDataExchange(CDataExchange * pDX);

protected:
  afx_msg BOOL OnHelpInfo(HELPINFO * pHelpInfo);

public:
  afx_msg void OnContextMenu(CWnd * pWnd, CPoint point);

public:
  afx_msg void OnListDoubleClick(NMHDR * pNMHDR, LRESULT * pResult);

public:
  afx_msg void OnEdit();

public:
  afx_msg void OnSelectionChange(NMHDR * pNMHDR, LRESULT * pResult);

public:
  afx_msg void OnMake();

public:
  afx_msg void OnNew();

public:
  afx_msg void OnRemove();

private:
  void Refresh();

private:
  int GetSelectedItem();

private:
  void EnableButtons();

private:
  void MakeAlias(const FormatInfo & formatInfo);

private:
  void InsertColumn(int colIdx, const char * lpszLabel, const char * lpszLongest);

private:
  bool modified = false;

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();
};
