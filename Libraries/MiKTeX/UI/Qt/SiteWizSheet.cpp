/* SiteWizSheet.cpp:

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

#include <QtWidgets>

#include "internal.h"

#include "miktex/UI/Qt/ErrorDialog.h"
#include "miktex/UI/Qt/SiteWizSheet.h"

#include "SiteWizDrive.h"
#include "SiteWizLocal.h"
#include "SiteWizRemote.h"
#include "SiteWizSheet.h"
#include "SiteWizType.h"

using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::Qt;
using namespace std;

SiteWizSheetImpl::SiteWizSheetImpl(QWidget* parent) :
  QWizard(parent),
  packageManager(PackageManager::Create())
{
  setWindowTitle(T_("Change Package Repository"));
  setPage(Page_Type, new SiteWizType(packageManager));
  setPage(Page_Remote, new SiteWizRemote(packageManager));
  setPage(Page_Local, new SiteWizLocal(packageManager));
  setPage(Page_CD, new SiteWizDrive(packageManager));
}

int SiteWizSheet::DoModal(QWidget* parent)
{
  SiteWizSheetImpl dlg(parent);
  return dlg.exec();
}
