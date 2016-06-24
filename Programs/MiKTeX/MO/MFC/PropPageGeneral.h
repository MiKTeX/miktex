/* PropPageGeneral.h:                                   -*- C++ -*-

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

class PropPageGeneral :
  public CPropertyPage,
  public ICreateFndbCallback
{
private:
  enum { IDD = IDD_PROPPAGE_GENERAL };

protected:
  DECLARE_MESSAGE_MAP();

public:
  PropPageGeneral(std::shared_ptr<MiKTeX::Packages::PackageManager> pManager);

public:
  virtual BOOL OnInitDialog();

protected:
  virtual void DoDataExchange(CDataExchange * pDX);

protected:
  virtual BOOL OnApply();

protected:
  afx_msg void OnChangeInstallOnTheFly();

protected:
  afx_msg BOOL OnHelpInfo(HELPINFO * pHelpInfo);

protected:
  afx_msg void OnContextMenu(CWnd * pWnd, CPoint point);

protected:
  afx_msg void OnRefreshFndb();

protected:
  afx_msg void OnUpdateFmt();

protected:
  afx_msg void OnChangePaperSize();

private:
  virtual bool MIKTEXTHISCALL ReadDirectory(const char * lpszPath, char ** ppSubDirNames, char ** ppFileNames, char ** ppFileNameInfos);

private:
  virtual bool MIKTEXTHISCALL OnProgress(unsigned level, const char * lpszDirectory);

private:
  ProgressDialog * pProgressDialog = nullptr;

private:
  int installOnTheFly = -1;

private:
  int previousInstallOnTheFly = -1;

private:
  int paperSizeIndex = -1;

private:
  int previousPaperSizeIndex = -1;

private:
  CComboBox paperSizeComboBox;

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> pManager;

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();
};
