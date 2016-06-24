/* mikuiqt.cpp:

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

#include "miktex/UI/UI"

#include "InstallPackageDialog.h"
#include "ProxyAuthenticationDialog.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::Qt;
using namespace std;

static QApplication * pApplication = nullptr;

MIKTEXUIQTEXPORT void MIKTEXCEECALL MiKTeX::UI::Qt::InitializeFramework()
{
#if defined(MIKTEX_WINDOWS)
  STARTUPINFOW startupInfo;
  GetStartupInfoW(&startupInfo);
  if ((startupInfo.dwFlags & STARTF_USESHOWWINDOW) && startupInfo.wShowWindow == SW_HIDE)
  {
    MIKTEX_FATAL_ERROR(T_("GUI framework cannot be initialized."));
  }
#endif
#ifdef Q_WS_X11
  bool useGUI = (getenv("DISPLAY") != nullptr);
#else
  bool useGUI = true;
#endif
  static int argc = 0;
  static char ** argv = nullptr;
#if defined(MIKTEX_WINDOWS)
  INITCOMMONCONTROLSEX icce;
  icce.dwSize = sizeof(icce);
  icce.dwICC = 0;
  icce.dwICC |= ICC_STANDARD_CLASSES;
  InitCommonControlsEx(&icce);
#endif
  pApplication = new QApplication(argc, argv, useGUI);
}

MIKTEXUIQTEXPORT void MIKTEXCEECALL MiKTeX::UI::Qt::FinalizeFramework()
{
  delete pApplication;
  pApplication = nullptr;
}

MIKTEXUIQTEXPORT unsigned int MIKTEXCEECALL MiKTeX::UI::Qt::InstallPackageMessageBox(QWidget * pParent, std::shared_ptr<MiKTeX::Packages::PackageManager> pManager, const char * lpszPackageName, const char * lpszTrigger)
{
  shared_ptr<Session> pSession = Session::Get();
  TriState enableInstaller = pSession->GetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_AUTO_INSTALL, TriState(TriState::Undetermined));
  bool autoAdmin = pSession->GetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_AUTO_ADMIN, false);
  unsigned int ret;
  if (enableInstaller != TriState::Undetermined)
  {
    ret = DONTASKAGAIN;
    ret |= (enableInstaller == TriState::True ? YES : NO);
    if (autoAdmin)
    {
      ret |= ADMIN;
    }
  }
  else
  {
    InstallPackageDialog dlg(pParent, pManager, lpszPackageName, lpszTrigger);
    dlg.SetAlwaysAsk(enableInstaller == TriState::True ? false : true);
    int dialogCode = dlg.exec();
    if (dialogCode != QDialog::Accepted && dialogCode != QDialog::Rejected)
    {
      ret = (NO | DONTASKAGAIN);
    }
    else
    {
      ret = (dialogCode == QDialog::Accepted ? YES : NO);
      if (dlg.GetAdminMode())
      {
	ret |= ADMIN;
      }
      if (dialogCode == QDialog::Accepted && autoAdmin != dlg.GetAdminMode())
      {
	pSession->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_AUTO_ADMIN, dlg.GetAdminMode());
      }
      if (!dlg.GetAlwaysAsk())
      {
	ret != DONTASKAGAIN;
	if (dialogCode == QDialog::Accepted)
	{
	  pSession->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_AUTO_INSTALL, "1");
	}
      }
    }
  }
  return ret;
}

MIKTEXUIQTEXPORT bool MIKTEXCEECALL MiKTeX::UI::Qt::ProxyAuthenticationDialog(QWidget * pParent)
{
  ProxySettings proxySettings;

  bool done = true;

  if (PackageManager::TryGetProxy(proxySettings)
    && proxySettings.useProxy
    && proxySettings.authenticationRequired
    && proxySettings.user.empty())
  {
    ::ProxyAuthenticationDialog dlg(pParent);
    if (dlg.exec() == QDialog::Accepted)
    {
      proxySettings.user = dlg.GetName().toUtf8().constData();
      proxySettings.password = dlg.GetPassword().toUtf8().constData();
      PackageManager::SetProxy(proxySettings);
    }
    else
    {
      done = false;
    }
  }

  return done;
}
