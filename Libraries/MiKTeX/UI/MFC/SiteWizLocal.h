/* SiteWizLocal.h:                                      -*- C++ -*-

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

class SiteWizLocal : public CPropertyPage
{
private:
  enum { IDD = IDD_SITEWIZ_LOCAL_REPOSITORY };

protected:
  DECLARE_MESSAGE_MAP();

protected:
  BOOL OnSetActive() override;

protected:
  BOOL OnWizardFinish() override;

protected:
  LRESULT OnWizardBack() override;

protected:
  void DoDataExchange(CDataExchange * pDX) override;

protected:
  afx_msg void OnBrowse();

protected:
  afx_msg void OnChangeDirectory();

public:
  SiteWizLocal(std::shared_ptr<MiKTeX::Packages::PackageManager> pManager);

private:
  CString directory;

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> pManager;
};
