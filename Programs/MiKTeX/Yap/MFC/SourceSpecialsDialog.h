/* SourceSpecialsDialog.h:                              -*- C++ -*-

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

#pragma once

class DviDoc;
class DviView;

class SourceSpecialsDialog :
  public CDialog
{
private:
  enum { IDD = IDD_SOURCE_SPECIALS };

protected:
  DECLARE_MESSAGE_MAP();

public:
  SourceSpecialsDialog(CWnd * pParent, DviDoc * pDoc);

protected:
  virtual BOOL OnInitDialog();

protected:
  virtual void DoDataExchange(CDataExchange * pDX);

protected:
  virtual void OnCancel();

protected:
  virtual void PostNcDestroy();

protected:
  afx_msg void OnClose();

protected:
  afx_msg void OnDblclkSourceSpecials(NMHDR * pNMHDR, LRESULT * pResult);

protected:
  afx_msg void OnEditSource();

protected:
  afx_msg void OnGoto();

protected:
  afx_msg void OnItemchangedSourceSpecials(NMHDR * pNMHDR, LRESULT * pResult);

private:
  DviDoc * pDviDoc;

private:
  DviView * pView;

private:
  struct SrcSpecial
  {
    string pageName;
    PathName fileName;
    int line;
  };

private:
  CButton editButton;

private:
  CButton goToButton;

private:
  CListCtrl listControl;

private:
  vector<SrcSpecial> sourceSpecials;
};
