/* AddChangeEditorDialog.cpp:                           -*- C++ -*-

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

class AddChangeEditorDialog :
  public CDialog
{
private:
  enum { IDD = IDD_ADD_EDITOR };

protected:
  DECLARE_MESSAGE_MAP();

public:
  AddChangeEditorDialog(CWnd* pParent, const vector<EditorInfo>& editors, int firstCustomIdx);

public:
  AddChangeEditorDialog(CWnd* pParent, const vector<EditorInfo>& editors, int firstCustomIdx, int currentIdx);

public:
  EditorInfo GetEditorInfo();

protected:
  BOOL OnInitDialog() override;

protected:
  void DoDataExchange(CDataExchange* pDX) override;

protected:
  afx_msg void OnChangeName();

protected:
  afx_msg void OnChangeArguments();

protected:
  afx_msg void OnChangePath();

public:
  afx_msg void OnBrowse();

private:
  void EnableButtons();

private:
  CEdit programEdit;

private:
  CEdit argumentsEdit;

private:
  CString program;

private:
  CString arguments;

private:
  CEdit nameEdit;

private:
  CString name;

private:
  bool adding;

private:
  vector<EditorInfo> editors;

private:
  int firstCustomIdx;

private:
  int currentIdx;
};
