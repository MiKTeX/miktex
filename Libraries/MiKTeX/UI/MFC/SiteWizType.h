/* SiteWizType.h:                                       -*- C++ -*-

   Copyright (C) 2000-2016 Christian Schenk

   This file is part of the MiKTeX UI Library.

   The MiKTeX UI Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX UI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX UI Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#include "resource.h"

class SiteWizType : public CPropertyPage
{
private:
  enum { IDD = IDD_SITEWIZ_TYPE };

protected:
  DECLARE_MESSAGE_MAP();

protected:
  BOOL OnSetActive() override;

protected:
  LRESULT OnWizardNext() override;

protected:
  void DoDataExchange(CDataExchange * pDX) override;

protected:
  afx_msg void OnInstallFromCD();

protected:
  afx_msg void OnInstallFromInternet();

protected:
  afx_msg void OnInstallFromLocalRepository();

protected:
  afx_msg void OnConnectionSettings();

protected:
  afx_msg void OnMiKTeXNext();

public:
  SiteWizType(std::shared_ptr<MiKTeX::Packages::PackageManager> pManager);

protected:
  void EnableButtons();

private:
  int repositoryType = -1;

private:
  int isMiKTeXNextSelected;

private:
  bool isMiKTeXNextWarningIssued = false;

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> pManager;
};
