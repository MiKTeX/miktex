/* InstallPackageDialog.h:                              -*- C++ -*-

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

class InstallPackageDialog : public CDialog
{
private:
  enum { IDD = IDD_INSTALL_PACKAGE };

protected:
  DECLARE_MESSAGE_MAP();

protected:
  void DoDataExchange(CDataExchange * pDX) override;

protected:
  BOOL OnInitDialog() override;

public:
  afx_msg void OnChangeRepository();

public:
  InstallPackageDialog(CWnd* parent, std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager, const std::string& packageName, const std::string& trigger);

private:
  void FormatControlText(UINT ctrlId, const char * lpszFormat, ...);

public:
  BOOL alwaysAsk = TRUE;

private:
  CString trigger;

private:
  CString packageName;

public:
  CString packageTitle;

public:
  std::shared_ptr<MiKTeX::Packages::PackageManager> pManager;
};
