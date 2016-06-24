/* TypePage.h:                                          -*- C++ -*-

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

class TypePage :
  public CPropertyPage
{
private:
  DECLARE_DYNCREATE(TypePage);

protected:
  DECLARE_MESSAGE_MAP();

private:
  enum { IDD = IDD_TYPE };

public:
  TypePage();

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
  afx_msg void OnInstallFromInternet();

protected:
  afx_msg void OnInstallFromLocalRepository();

protected:
  afx_msg void OnInstallFromCd();

protected:
  afx_msg void OnConnectionSettings();

protected:
  afx_msg void OnMiKTeXNext();

protected:
  afx_msg void OnNearestRepository();

protected:
  afx_msg void OnLastUsedRepository();

protected:
  afx_msg void OnChooseRepository();

protected:
  void EnableButtons();

private:
  CWnd * GetControl(UINT controlId);

private:
  void EnableControl(UINT controlId, bool enable);

private:
  class UpdateWizard * pSheet = nullptr;

private:
  int sourceChoice;

private:
  int remoteChoice;

private:
  int localChoice;

private:
  string remoteRepository;

private:
  RepositoryReleaseState repositoryReleaseState;

private:
  bool haveRemoteRepository;

private:
  PathName localRepository;

private:
  bool haveLocalRepository;

private:
  bool noDdv;

private:
  int isMiKTeXNextSelected;

private:
  bool isMiKTeXNextWarningIssued = false;

private:
  std::shared_ptr<Session> session = Session::Get();
};
