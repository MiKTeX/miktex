/* PackageInfoDialog.h:                                 -*- C++ -*-

   Copyright (C) 2017-2018 Christian Schenk

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

#if !defined(C555C707F6764B24A91B67B35FBAFCBA)
#define C555C707F6764B24A91B67B35FBAFCBA

#include <miktex/PackageManager/PackageManager>
#include <QDialog>

namespace Ui
{
  class PackageInfoDialog;
}

class PackageInfoDialogImpl :
  public QDialog
{
private:
  Q_OBJECT;

public:
  explicit PackageInfoDialogImpl(QWidget* parent, const MiKTeX::Packages::PackageInfo& packageInfo);

public:
  ~PackageInfoDialogImpl();

private:
  Ui::PackageInfoDialog* ui;

private:
  MiKTeX::Packages::PackageInfo packageInfo;
};

#endif
