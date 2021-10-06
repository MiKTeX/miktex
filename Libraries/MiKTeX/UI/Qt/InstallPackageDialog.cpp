/* InstallPackageDialog.cpp:

   Copyright (C) 2008-2021 Christian Schenk

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
#  define NOMINMAX
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

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::Qt;
using namespace MiKTeX::Util;

InstallPackageDialog::InstallPackageDialog(QWidget* parent, shared_ptr<PackageManager> packageManager, const string& packageName, const string& trigger) :
  QDialog(parent),
  packageManager(packageManager)
{
  setupUi(this);
  try
  {
    shared_ptr<Session> session = MIKTEX_SESSION();
    QPushButton* okayButton = buttonBox->button(QDialogButtonBox::Ok);
    if (okayButton == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    okayButton->setText(tr("Install"));
    lblPackageName->setText(QString::fromStdString(packageName));
    lblMissingFile->setText(QString::fromStdString(PathName(trigger).GetFileName().ToDisplayString()));
    PackageInfo packageInfo = packageManager->GetPackageInfo(packageName);
    string repository;
    RepositoryType repositoryType(RepositoryType::Unknown);
    if (packageManager->TryGetDefaultPackageRepository(repositoryType, repository) && !repository.empty())
    {
      leInstallationSource->setText(QString::fromUtf8(repository.c_str()));
    }
    else
    {
      leInstallationSource->setText(tr("<Random package repository>"));
    }
    PathName commonInstallRoot = session->IsSharedSetup() ? session->GetSpecialPath(SpecialPath::CommonInstallRoot) : PathName();
    PathName userInstallRoot = session->IsAdminMode() ? PathName() : session->GetSpecialPath(SpecialPath::UserInstallRoot);
    bool enableCommonInstall = session->IsSharedSetup() && (session->IsAdminMode() || session->IsUserAnAdministrator());
    enableCommonInstall = enableCommonInstall && Directory::Exists(commonInstallRoot);
    if (enableCommonInstall)
    {
      cbInstallationDirectory->addItem(tr("<All users>"), true);
    }
    if (!session->IsAdminMode())
    {
      QString currentUser;
#if defined(MIKTEX_WINDOWS)
      wchar_t szLogonName[30];
      DWORD sizeLogonName = sizeof(szLogonName) / sizeof(szLogonName[0]);
      if (GetUserNameW(szLogonName, &sizeLogonName))
      {
        currentUser = QString::fromWCharArray(szLogonName);
      }
      else if (GetLastError() == ERROR_NOT_LOGGED_ON)
      {
        currentUser = tr("<Unknown user>");
      }
      else
      {
        MIKTEX_FATAL_WINDOWS_ERROR("GetUserNameW");
      }
      wchar_t szDisplayName[30];
      ULONG sizeDisplayName = sizeof(szDisplayName) / sizeof(szDisplayName[0]);
      if (GetUserNameExW(NameDisplay, szDisplayName, &sizeDisplayName))
      {
        currentUser += " (";
        currentUser += QString::fromWCharArray(szDisplayName);
        currentUser += ")";
      }
#else
      currentUser = tr("<Current user>");
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
