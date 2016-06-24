/* MpmView.h:                                           -*- C++ -*-

   Copyright (C) 2002-2016 Christian Schenk

   This file is part of MiKTeX Package Manager.

   MiKTeX Package Manager is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Package Manager is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Package Manager; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

class MpmView : public CListView
{
protected:
  DECLARE_DYNCREATE(MpmView);

protected:
  MpmView();

public:
  MpmDoc * GetDocument() const
  {
    return reinterpret_cast<MpmDoc*>(m_pDocument);
  }

public:
  virtual BOOL PreCreateWindow(CREATESTRUCT & cs);

protected:
  virtual void OnInitialUpdate();

protected:
  virtual BOOL OnPreparePrinting(CPrintInfo * pInfo);

protected:
  virtual void OnBeginPrinting(CDC * pDC, CPrintInfo * pInfo);

protected:
  virtual void OnEndPrinting(CDC * pDC, CPrintInfo * pInfo);

protected:
  enum { WM_FILLLISTVIEW = WM_APP + 1 };

protected:
  DECLARE_MESSAGE_MAP();

private:
  void FillListView();

public:
  static int CALLBACK CompareItems(LPARAM lParam1, LPARAM lParam2, LPARAM paramSort);

public:
  afx_msg LRESULT OnFillListView(WPARAM wParam, LPARAM lParam);

public:
  afx_msg void OnButtonSearchClicked();

public:
  afx_msg void OnContextMenu(CWnd * pWnd, CPoint point);

public:
  afx_msg void OnInstall();

public:
  afx_msg void OnLvnColumnclick(NMHDR * pNMHDR, LRESULT * pResult);

public:
  afx_msg void OnNMDblclk(NMHDR * pNMHDR, LRESULT * pResult);

public:
  afx_msg void OnProperties();

public:
  afx_msg void OnResetView();

public:
  afx_msg void OnSelectAll();

public:
  afx_msg void OnSelectInstallablePackages();

public:
  afx_msg void OnSelectUpdateablePackages();

public:
  afx_msg void OnUninstall();

public:
  afx_msg void OnUpdateDatabase();

public:
  afx_msg void OnUpdateInstall(CCmdUI * pCmdUI);

public:
  afx_msg void OnUpdateProperties(CCmdUI * pCmdUI);

public:
  afx_msg void OnUpdateResetView(CCmdUI * pCmdUI);

public:
  afx_msg void OnUpdateSelectUpdateablePackages(CCmdUI * pCmdUI);

public:
  afx_msg void OnUpdateSiteWizard(CCmdUI * pCmdUI);

public:
  afx_msg void OnUpdateUninstall(CCmdUI * pCmdUI);

public:
  afx_msg void OnUpdateUpdateDatabase(CCmdUI * pCmdUI);

public:
  afx_msg void OnUpdateUpdateWizard(CCmdUI * pCmdUI);

public:
  afx_msg void OnUpdateWizard();

protected:
  void InsertItem(int idx, const PackageInfo & packageInfo);

protected:
  void InsertColumn(int colIdx, const char * lpszLabel, const char * lpszLongest);

protected:
  void OnSiteWizard();

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> pManager;

private:
  map<LPARAM, PackageInfo> packages;

public:
  int GetNumberOfPackages() const
  {
    return packages.size();
  }

private:
  int clickedColumn;

private:
  int sortOrder;

private:
  shared_ptr<Session> pSession = Session::Get();
};
