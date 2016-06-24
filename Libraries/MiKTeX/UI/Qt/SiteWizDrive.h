/* SiteWizDrive.h:                                      -*- C++ -*-

   Copyright (C) 2008-2016 Christian Schenk

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

#if !defined(CD26AA1939BF4CA0862C2BA8B6C24678)
#define CD26AA1939BF4CA0862C2BA8B6C24678

#include <vector>

#include <miktex/PackageManager/PackageManager>

#include "ui_SiteWizDrive.h"

class SiteWizDrive :
  public QWizardPage,
  private Ui::SiteWizDrive
{
private:
  Q_OBJECT;

public:
  SiteWizDrive(std::shared_ptr<MiKTeX::Packages::PackageManager> pManager);

public:
  virtual void initializePage();

public:
  virtual bool isComplete() const;

public:
  virtual bool validatePage();

public:
  virtual int nextId() const
  {
    return -1;
  }

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> pManager;

private:
  struct Location
  {
    std::string directory;
    std::string description;
  };

#if defined(MIKTEX_WINDOWS)
private:
  void FindMiKTeXCDs(std::vector<Location> & locations);
#endif

private:
  std::vector<Location> locations;

private:
  bool firstVisit = true;
};

#endif
