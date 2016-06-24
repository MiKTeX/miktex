/* Update.h:                                            -*- C++ -*-

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

#include "CdPage.h"
#include "FileCopyPage.h"
#include "FinishPage.h"
#include "LocalPage.h"
#include "PackageListPage.h"
#include "RemotePage.h"
#include "TypePage.h"
#include "Update.h"
#include "WelcomePage.h"

class UpdateWizard :
  public CPropertySheet
{
private:
  DECLARE_DYNAMIC(UpdateWizard);

protected:
  DECLARE_MESSAGE_MAP();

public:
  UpdateWizard();

protected:
  virtual BOOL OnInitDialog();

public:
  void SetUpdateList(const vector<string> & toBeUpdated)
  {
    this->toBeUpdated = toBeUpdated;
  }

public:
  void SetRemoveList(const vector<string> & toBeRemoved)
  {
    this->toBeRemoved = toBeRemoved;
  }

public:
  const vector<string> & GetUpdateList() const
  {
    return toBeUpdated;
  }

public:
  const vector<string> & GetRemoveList() const
  {
    return toBeRemoved;
  }

public:
  void SetErrorFlag()
  {
    error = true;
  }

public:
  void
    ClearErrorFlag()
  {
    error = false;
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

public:
  void SetRandomRepositoryFlag(bool f)
  {
    randomRepository = "";
    randomRepositoryFlag = f;
  }

public:
  void SetRepositoryType(RepositoryType repositoryType)
  {
    this->repositoryType = repositoryType;
  }

public:
  RepositoryType GetRepositoryType() const
  {
    return repositoryType;
  }

public:
  void SetRepositoryReleaseState(RepositoryReleaseState repositoryReleaseState)
  {
    this->repositoryReleaseState = repositoryReleaseState;
  }

public:
  RepositoryReleaseState GetRepositoryReleaseState() const
  {
    return repositoryReleaseState;
  }

public:
  void SetCameFrom(int pageId)
  {
    cameFrom = pageId;
  }

public:
  int GetCameFrom()
  {
    int ret = cameFrom;
    cameFrom = 0;
    return ret;
  }

public:
  string GetRepository();

public:
  void ReportError(const exception & e);

public:
  void ReportError(const MiKTeXException & e);

private:
  void EnableCancelButton(bool enable);

private:
  CBitmap watermarkBitmap;

private:
  CBitmap headerBitmap;

private:
  WelcomePage welcomePage;

private:
  PackageListPage packageListPage;

private:
  FileCopyPage fileCopyPage;

private:
  TypePage typePage;

private:
  LocalPage localPage;

private:
  RemotePage remotePage;

private:
  CdPage cdPage;

private:
  FinishPage finishPage;

private:
  vector<string> toBeUpdated;

private:
  vector<string> toBeRemoved;

private:
  bool error = false;

private:
  bool cancelled = false;

private:
  bool randomRepositoryFlag = false;

private:
  RepositoryType repositoryType = RepositoryType::Unknown;

private:
  RepositoryReleaseState repositoryReleaseState = RepositoryReleaseState::Unknown;

private:
  string randomRepository;

private:
  int cameFrom;

private:
  shared_ptr<Session> session = Session::Get();
};
