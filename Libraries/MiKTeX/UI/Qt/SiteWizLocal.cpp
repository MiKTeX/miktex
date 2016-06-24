/* SiteWizLocal.cpp:

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/UI/Qt/ErrorDialog.h"

#include "SiteWizSheet.h"
#include "SiteWizLocal.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::Qt;
using namespace std;

SiteWizLocal::SiteWizLocal(shared_ptr<PackageManager> pManager) :
  QWizardPage(nullptr),
  pManager(pManager)
{
  setupUi(this);
}

void SiteWizLocal::initializePage()
{
  try
  {
    PathName path;
    if (pManager->TryGetLocalPackageRepository(path))
    {
      leDirectory->setText(QString::fromUtf8(path.Get()));
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

bool SiteWizLocal::isComplete() const
{
  return !leDirectory->text().isEmpty();
}

bool SiteWizLocal::validatePage()
{
  try
  {
    if (!QWizardPage::validatePage())
    {
      return false;
    }
    PathName directory(leDirectory->text().toLocal8Bit().data());
    if (!Directory::Exists(directory))
    {
      QMessageBox::critical(this, QString(), T_("The specified directory does not exist."));
      return false;
    }
    if (!pManager->IsLocalPackageRepository(directory))
    {
      PathName mpmIni(directory);
      mpmIni /= "texmf";
      mpmIni /= MIKTEX_PATH_MPM_INI;
      if (!File::Exists(mpmIni))
      {
	QMessageBox::critical(this, QString(), T_("Not a local package repository."));
	return false;
      }
    }
    pManager->SetDefaultPackageRepository(RepositoryType::Local, directory.Get());
    return true;
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
    return false;
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
    return false;
  }
}

void SiteWizLocal::on_btnBrowse_clicked()
{
  QString directory = QFileDialog::getExistingDirectory(this, QString(), leDirectory->text());
  if (!directory.isEmpty())
  {
    leDirectory->setText(directory);
    emit completeChanged();
  }
}
