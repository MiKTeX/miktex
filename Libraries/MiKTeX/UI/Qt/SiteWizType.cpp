/* SiteWizType.cpp:

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

#include "ConnectionSettingsDialog.h"
#include "SiteWizSheet.h"
#include "SiteWizType.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::Qt;
using namespace std;

SiteWizType::SiteWizType(shared_ptr<PackageManager> pManager) :
  QWizardPage(nullptr),
  pManager(pManager)
{
  setupUi(this);
  connect(rbRemote, SIGNAL(clicked()), this, SIGNAL(completeChanged()));
  connect(rbLocal, SIGNAL(clicked()), this, SIGNAL(completeChanged()));
  connect(rbCD, SIGNAL(clicked()), this, SIGNAL(completeChanged()));
}

void SiteWizType::initializePage()
{
  try
  {
    registerField("isMiKTeXNext", chkMiKTeXNext);
    string urlOrPath;
    RepositoryType repositoryType(RepositoryType::Unknown);
    RepositoryReleaseState repositoryReleaseState = RepositoryReleaseState::Unknown;
    if (PackageManager::TryGetDefaultPackageRepository(repositoryType, repositoryReleaseState, urlOrPath))
    {
      switch (repositoryType)
      {
      case RepositoryType::Remote:
	rbRemote->setChecked(true);
	break;
      case RepositoryType::Local:
	rbLocal->setChecked(true);
	break;
      case RepositoryType::MiKTeXDirect:
	rbCD->setChecked(true);
	break;
      }
    }
#if 0
    else
    {
      rbRemote->setChecked(true);
    }
#endif
    chkMiKTeXNext->setChecked(repositoryReleaseState == RepositoryReleaseState::Next);
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

bool SiteWizType::isComplete() const
{
  return rbRemote->isChecked() || rbLocal->isChecked() || rbCD->isChecked();
}

bool SiteWizType::validatePage()
{
  try
  {
    if (!QWizardPage::validatePage())
    {
      return false;
    }
    if (rbRemote->isChecked())
    {
      if (!ProxyAuthenticationDialog(this))
      {
	return false;
      }
    }
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

int SiteWizType::nextId() const
{
  if (rbRemote->isChecked())
  {
    return SiteWizSheetImpl::Page_Remote;
  }
  else if (rbLocal->isChecked())
  {
    return SiteWizSheetImpl::Page_Local;
  }
  else if (rbCD->isChecked())
  {
    return SiteWizSheetImpl::Page_CD;
  }
  else
  {
    return 4444;
  }
}

void SiteWizType::on_btnConnectionSettings_clicked()
{
  ConnectionSettingsDialog dlg(this);
  dlg.exec();
}

void SiteWizType::on_chkMiKTeXNext_clicked()
{
  if (chkMiKTeXNext->isChecked() && !isMiKTeXNextWarningIssued)
  {
    QMessageBox::warning(this,
      "MiKTeX Package Manager",
      "You have chosen to get untested packages. Although every effort has been made to ensure the correctness of these packages, a hassle-free operation cannot be guaranteed.\r\n\r\nPlease visit http://miktex.org/kb/miktex-next, for more information.",
      QMessageBox::Ok);
    isMiKTeXNextWarningIssued = true;
  }
}
