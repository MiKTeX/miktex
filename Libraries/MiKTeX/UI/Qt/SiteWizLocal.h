/* SiteWizLocal.h:                                      -*- C++ -*-

   Copyright (C) 2008-2018 Christian Schenk

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

#if !defined(E228BEC9ADEE41C98FA4CB48036D22E0)
#define E228BEC9ADEE41C98FA4CB48036D22E0

#include <vector>
#include <miktex/PackageManager/PackageManager>

#include "ui_SiteWizLocal.h"

class SiteWizLocal :
  public QWizardPage,
  private Ui::SiteWizLocal
{
private:
  Q_OBJECT;

public:
  SiteWizLocal(std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager);

public:
  void initializePage() override;

public:
  bool isComplete() const override;

public:
  bool validatePage() override;

public:
  int nextId() const override
  {
    return -1;
  }

private slots:
  void on_btnBrowse_clicked();

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;
};

#endif
