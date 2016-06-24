/* SiteWizType.h:                                       -*- C++ -*-

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

#if !defined(E470AC5708664919BAD58B49DC624841)
#define E470AC5708664919BAD58B49DC624841

#include <miktex/PackageManager/PackageManager>

#include "ui_SiteWizType.h"

class SiteWizType :
  public QWizardPage,
  private Ui::SiteWizType
{
private:
  Q_OBJECT;

public:
  SiteWizType(std::shared_ptr<MiKTeX::Packages::PackageManager> pManager);

public:
  virtual void initializePage();

public:
  virtual bool isComplete() const;

public:
  virtual bool validatePage();

public:
  virtual int nextId() const;

private slots:
  void on_btnConnectionSettings_clicked();

private slots:
  void on_chkMiKTeXNext_clicked();

private:
  bool isMiKTeXNextWarningIssued = false;

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> pManager;
};

#endif
