/* PackageListPag.h:                                    -*- C++ -*-

   Copyright (C) 2002-2018 Christian Schenk

   This file is part of the MiKTeX Update Wizard.

   The MiKTeX Update Wizard is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Update Wizard is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Update Wizard; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#include "resource.h"

class PackageListPage :
  public CPropertyPage,
  public PackageInstallerCallback
{
private:
  DECLARE_DYNCREATE(PackageListPage);

protected:
  DECLARE_MESSAGE_MAP();

private:
  enum { IDD = IDD_PACKAGE_LIST };

public:
  PackageListPage();

public:
  virtual ~PackageListPage();

protected:
  virtual BOOL OnInitDialog();

protected:
  virtual BOOL OnSetActive();

protected:
  virtual void DoDataExchange(CDataExchange * pDX);

protected:
  virtual LRESULT OnWizardNext();

protected:
  virtual LRESULT OnWizardBack();

protected:
  virtual BOOL OnKillActive();

protected:
  virtual BOOL OnQueryCancel();

protected:
  afx_msg LRESULT OnFillList(WPARAM wParam, LPARAM lParam);

protected:
  afx_msg void OnSelectAll();

protected:
  afx_msg void OnDeselectAll();

protected:
  afx_msg void OnItemActivate(NMHDR * pNMHDR, LRESULT * pResult);

protected:
  afx_msg void OnItemChanging(NMHDR * pNMHDR, LRESULT * pResult);

protected:
  afx_msg void OnItemChanged(NMHDR * pNMHDR, LRESULT * pResult);

private:
  static UINT WorkerThread(void * pv);

private:
  void DoFindUpdates();

public:
  virtual void MIKTEXTHISCALL ReportLine(const std::string & str);

public:
  virtual bool MIKTEXTHISCALL OnRetryableError(const std::string & message);

public:
  virtual bool MIKTEXTHISCALL OnProgress(MiKTeX::Packages::Notification nf);

private:
  void EnableSelectButtons();

private:
  void InsertColumn(int colIdx, const char * lpszLabel, const char * lpszLongest);

private:
  void SetProgressText(const char * lpszText);

private:
  HANDLE hWorkerThread = nullptr;

private:
  vector<PackageInstaller::UpdateInfo> updates;

private:
  shared_ptr<PackageInstaller> pInstaller;

private:
  bool canSelectAll;

private:
  bool canDeselectAll;

private:
  string repository;

private:
  bool fillingTheListView = false;

private:
  bool ready = false;

private:
  CListCtrl listControl;


private:
  class UpdateWizard * pSheet = nullptr;
};
