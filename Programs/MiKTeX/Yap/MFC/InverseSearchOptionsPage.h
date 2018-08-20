/* InverseSearchOptionsPage.h:                          -*- C++ -*-

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

class InverseSearchOptionsPage :
  public CPropertyPage
{
private:
  enum { IDD = IDD_INVERSE_SEARCH };

protected:
  DECLARE_MESSAGE_MAP();

public:
  InverseSearchOptionsPage();

protected:
  BOOL OnInitDialog() override;

protected:
  void DoDataExchange(CDataExchange* pDX) override;

protected:
  BOOL OnApply() override;

protected:
  afx_msg void OnSelChangeEditor();

protected:
  afx_msg void OnAddEditor();

protected:
  afx_msg void OnDeleteEditor();

protected:
  afx_msg void OnChangeEditor();

private:
  void EnableButtons();

private:
  void CollectEditors();

private:
  vector<EditorInfo> editors;

private:
  bool mustWrite = false;

private:
  int commandLineIdx = -1;

private:
  int firstCustomIdx = -1;

private:
  CListBox editorListBox;

private:
  CEdit commandLineDisplay;

private:
  CButton deleteButton;

private:
  CButton changeButton;

private:
  shared_ptr<Session> session = Session::Get();
};
