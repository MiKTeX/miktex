/* RemoteRepositoryPag.h:                               -*- C++ -*-

   Copyright (C) 1999-2018 Christian Schenk

   This file is part of the MiKTeX Setup Wizard.

   The MiKTeX Setup Wizard is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Setup Wizard is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Setup Wizard; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#include "Setup.h"

class RemoteRepositoryPage :
  public CPropertyPage
{
protected:
  DECLARE_MESSAGE_MAP();

private:
  enum { IDD = IDD_REMOTE_REPOSITORY };

public:
  RemoteRepositoryPage();

protected:
  BOOL OnInitDialog() override;

protected:
  BOOL OnSetActive() override;

protected:
  void DoDataExchange(CDataExchange* dx) override;

protected:
  LRESULT OnWizardNext() override;

protected:
  LRESULT OnWizardBack() override;

protected:
  BOOL OnKillActive() override;

protected:
  BOOL OnQueryCancel() override;

protected:
  afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);

protected:
  afx_msg LRESULT OnFillList(WPARAM wParam, LPARAM lParam);

private:
  static UINT WorkerThread(void* remoteRepositoryPage);

private:
  void InsertColumn(int column, const char* label, const char* longest);

private:
  void SetItemText(int index, int column, const char* text);

private:
  void SetProgressText(const char* text);

private:
  CListCtrl listControl;

private:
  vector<RepositoryInfo> repositories;

private:
  bool firstSetActive = true;

private:
  bool ready = false;

private:
  bool noDdv;

private:
  class SetupWizard* sheet = nullptr;
};
