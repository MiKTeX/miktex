/* RemotePage.h:                                        -*- C++ -*-

   Copyright (C) 2002-2016 Christian Schenk

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

class RemotePage :
  public CPropertyPage
{
private:
  DECLARE_DYNCREATE(RemotePage);

protected:
  DECLARE_MESSAGE_MAP();

private:
  enum { IDD = IDD_REMOTE_REPOSITORY };

public:
  RemotePage();

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
  afx_msg
    void
    OnItemChanged(NMHDR * pNMHDR, LRESULT * pResult);

protected:
  afx_msg LRESULT OnFillList(WPARAM wParam, LPARAM lParam);

private:
  static UINT WorkerThread(void * pv);

private:
  void InsertColumn(int colIdx, const char * lpszLabel, const char * lpszLongest);

private:
  void SetItemText(int itemIdx, int colIdx, const char * lpszText);

private:
  void SetProgressText(const char * lpszText);

private:
  CListCtrl listControl;

private:
  vector<RepositoryInfo> repositories;

private:
  bool firstSetActive = true;

private:
  bool showAll = false;

private:
  bool ready = false;

private:
  bool noDdv;

private:
  class UpdateWizard * pSheet = nullptr;
};
