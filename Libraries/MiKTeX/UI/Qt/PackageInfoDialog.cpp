/* PackageInfoDialog.cpp:

   Copyright (C) 2017 Christian Schenk

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/UI/Qt/PackageInfoDialog.h"

#include "PackageInfoDialog.h"
#include "ui_PackageInfoDialog.h"

using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::Qt;

PackageInfoDialogImpl::PackageInfoDialogImpl(QWidget* parent, const PackageInfo& packageInfo) :
  QDialog(parent),
  ui(new Ui::PackageInfoDialog)
{
  ui->setupUi(this);
}

PackageInfoDialogImpl::~PackageInfoDialogImpl()
{
  delete ui;
}

int PackageInfoDialog::DoModal(QWidget* parent, const PackageInfo& packageInfo)
{
  PackageInfoDialogImpl dlg(parent, packageInfo);
  return dlg.exec();
}
