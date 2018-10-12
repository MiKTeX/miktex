/* InstallPackageDialog.cpp:

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

#if defined(MIKTEX_WINDOWS)
#  include <Windows.h>
#  define SECURITY_WIN32
#  include <Security.h>
#endif

#include <QtWidgets>

#if defined(MIKTEX_WINDOWS)
#  include <QtWinExtras/qwinfunctions.h>
#endif

#include <miktex/Core/Directory>
#include <miktex/Core/Session>

#include "internal.h"

#include "miktex/UI/Qt/ErrorDialog.h"
#include "miktex/UI/Qt/SiteWizSheet.h"

#include "InstallPackageDialog.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::Qt;
using namespace std;

InstallPackageDialog::InstallPackageDialog(QWidget* parent, shared_ptr<PackageManager> packageManager, const string& packageName, const string& trigger) :
  QDialog(parent),
  packageManager(packageManager)
{
  setupUi(this);
  try
  {
    shared_ptr<Session> session = Session::Get();
    QPushButton* okayButton = buttonBox->button(QDialogButtonBox::Ok);
    if (okayButton == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    okayButton->setText(T_("Install"));
    lblPackageName->setText(QString::fromLocal8Bit(packageName.c_str()));
    lblMissingFile->setText(QString::fromLocal8Bit(trigger.c_str()));
    PackageInfo packageInfo = packageManager->GetPackageInfo(packageName);
    string repository;
    RepositoryType repositoryType(RepositoryType::Unknown);
    if (packageManager->TryGetDefaultPackageRepository(repositoryType, repository) && !repository.empty())
    {
      leInstallationSource->setText(QString::fromUtf8(repository.c_str()));
    }
    else
    {
      leInstallationSource->setText(T_("<Random package repository>"));
    }
    PathName commonInstallRoot = session->GetSpecialPath(SpecialPath::CommonInstallRoot);
    PathName userInstallRoot = session->GetSpecialPath(SpecialPath::UserInstallRoot);
    bool enableCommonInstall = (commonInstallRoot != userInstallRoot);
    enableCommonInstall = (enableCommonInstall && Directory::Exists(commonInstallRoot));
#if defined(MIKTEX_WINDOWS)
    enableCommonInstall = enableCommonInstall && (session->IsUserAnAdministrator() || session->IsUserAPowerUser());
#else
    enableCommonInstall = enableCommonInstall && session->IsUserAnAdministrator();
#endif
    if (enableCommonInstall)
    {
      cbInstallationDirectory->addItem(T_("Anyone who uses this computer (all users)"), true);
    }
    if (!enableCommonInstall || commonInstallRoot != userInstallRoot)
    {
      QString currentUser;
#if defined(MIKTEX_WINDOWS)
      char szLogonName[30];
      DWORD sizeLogonName = sizeof(szLogonName) / sizeof(szLogonName[0]);
      if (GetUserNameA(szLogonName, &sizeLogonName))
      {
        currentUser = szLogonName;
      }
      else if (GetLastError() == ERROR_NOT_LOGGED_ON)
      {
        currentUser = T_("Unknown user");
      }
      else
      {
        MIKTEX_FATAL_WINDOWS_ERROR("GetUserNameA");
      }
      char szDisplayName[30];
      ULONG sizeDisplayName = sizeof(szDisplayName) / sizeof(szDisplayName[0]);
      if (GetUserNameExA(NameDisplay, szDisplayName, &sizeDisplayName))
      {
        currentUser += " (";
        currentUser += szDisplayName;
        currentUser += ")";
      }
#else
      currentUser = T_("The current user");
#endif
      cbInstallationDirectory->addItem(currentUser, false);
    }
    cbInstallationDirectory->setCurrentIndex(0);
  }
  catch (const MiKTeXException& e)
  {
    ErrorDialog::DoModal(0, e);
  }
  catch (const exception& e)
  {
    ErrorDialog::DoModal(0, e);
  }
}

void InstallPackageDialog::on_btnChange_clicked()
{
  try
  {
    if (SiteWizSheet::DoModal(this) != QDialog::Accepted)
    {
      return;
    }
    string repository;
    RepositoryType repositoryType(RepositoryType::Unknown);
    if (packageManager->TryGetDefaultPackageRepository(repositoryType, repository) && !repository.empty())
    {
      leInstallationSource->setText(QString::fromLocal8Bit(repository.c_str()));
    }
  }
  catch (const MiKTeXException& e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception& e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void InstallPackageDialog::on_cbInstallationDirectory_currentIndexChanged(int idx)
{
  if (idx < 0)
  {
    return;
  }
  bool elevationRequired = cbInstallationDirectory->itemData(idx).toBool();
  QPushButton* okayButton = buttonBox->button(QDialogButtonBox::Ok);
  if (elevationRequired)
  {
    bool iconSet = false;
#if defined(MIKTEX_WINDOWS)
    SHSTOCKICONINFO sii;
    sii.cbSize = sizeof(sii);
    if (SUCCEEDED(SHGetStockIconInfo(SIID_SHIELD, SHGSI_ICON | SHGSI_SMALLICON, &sii)))
    {
      HICON hiconShield = sii.hIcon;
      ICONINFO iconInfo;
      if (GetIconInfo(hiconShield, &iconInfo))
      {
        QPixmap pixmapShield = QtWin::fromHBITMAP(iconInfo.hbmColor, QtWin::HBitmapAlpha);
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        okayButton->setIcon(QIcon(pixmapShield));
        iconSet = true;
      }
      DestroyIcon(hiconShield);
    }
#endif
    if (!iconSet)
    {
      okayButton->setIcon(QIcon(":/Icons/elevationrequired16x16.png"));
    }
  }
  else
  {
    okayButton->setIcon(QIcon());
  }
}
