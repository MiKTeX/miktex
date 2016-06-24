/* PropPageFiles.h:                                     -*- C++ -*-

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

#pragma once

class PropPageFiles : public CPropertyPage
{
protected:
  DECLARE_MESSAGE_MAP();

protected:
  void DoDataExchange(CDataExchange * pDX) override;

public:
  BOOL OnInitDialog() override;

public:
  PropPageFiles(const MiKTeX::Packages::PackageInfo & packageInfo);

private:
  void AddFile(int idx, const char * lpszFileName);

private:
  CListCtrl listControl;

private:
  MiKTeX::Packages::PackageInfo packageInfo;
};
