/* SetupWizard.h:                                       -*- C++ -*-

   Copyright (C) 1999-2016 Christian Schenk

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

#if ENABLE_ADDTEXMF
#include "AddTEXMFPage.h"
#include "EditTEXMFPage.h"
#endif

#if SHOW_FOLDER_PAGE
#  include "ProgramFolderPage.h"
#endif

#include "FileCopyPage.h"
#include "FinishPage.h"
#include "InfoListPage.h"
#include "InstallDirPage.h"
#include "LicensePage.h"
#include "LocalRepositoryPage.h"
#include "MDTaskPage.h"
#include "PackageSetPage.h"
#include "RemoteRepositoryPage.h"
#include "SharedInstallationPage.h"
#include "SettingsPage.h"
#include "TaskPage.h"
#include "WelcomePage.h"

class SetupWizard :
  public CPropertySheet
{
protected:
  DECLARE_MESSAGE_MAP();

public:
  SetupWizard(shared_ptr<PackageManager> packageManager);

public:
  virtual BOOL OnInitDialog();

public:
  void ReportError(const exception & e);

public:
  void ReportError(const MiKTeXException & e);

public:
  void SetErrorFlag()
  {
    error = true;
  }

public:
  bool GetErrorFlag() const
  {
    return error;
  }

public:
  void SetCancelFlag()
  {
    EnableCancelButton(false);
    cancelled = true;
  }

public:
  bool GetCancelFlag() const
  {
    return cancelled;
  }

public:
  void EnableCloseButton()
  {
    SetWizardButtons(PSWIZB_FINISH);
    EnableCancelButton(false);
    SetFinishText(T_(_T("Close")));
  }

private:
  vector<int> pages;

public:
  void PushPage(int page)
  {
    this->pages.push_back(page);
  }

public:
  int PopPage()
  {
    int page = pages.back();
    pages.pop_back();
    return page;
  }

public:
  CString SetNextText(LPCTSTR lpszText);

private:
  void EnableCancelButton(bool enable);

private:
  CBitmap watermarkBitmap;

private:
  CBitmap headerBitmap;

private:
  WelcomePage welcomePage;

private:
  MDTaskPage mdTaskPage;

private:
  TaskPage taskPage;

private:
  RemoteRepositoryPage remoteRepositoryPage;

private:
  LocalRepositoryPage localRepositoryPage;

private:
  PackageSetPage downloadPackageSetPage;

private:
  PackageSetPage installPackageSetPage;

private:
  SharedInstallationPage sharedInstallationPage;

private:
  InstallDirPage installDirPage;

#if SHOW_FOLDER_PAGE
private:
  ProgramFolderPage programFolderPage;
#endif

private:
  SettingsPage settingsPage;

private:
  LicensePage licensePage;

#if ENABLE_ADDTEXMF
private:
  AddTEXMFPage addTEXMFPage;
#endif

#if ENABLE_ADDTEXMF
private:
  EditTEXMFPage editTEXMFPage;
#endif

private:
  InfoListPage infoListPage;

private:
  FileCopyPage fileCopyPage;

private:
  FinishPage finishPage;

private:
  bool error = false;

private:
  bool cancelled = false;

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;
};
