/* mikui.cpp:

   Copyright (C) 2000-2016 Christian Schenk

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

#include <afxdllx.h>

#include "internal.h"

#include "InstallPackageDialog.h"
#include "GiveBackDialog.h"
#include "ProxyAuthenticationDialog.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace std;

AFX_EXTENSION_MODULE MikuiDLL = { FALSE, nullptr };

extern "C" int APIENTRY DllMain(HINSTANCE hInstance, DWORD reason, LPVOID pReserved)
{
  UNREFERENCED_PARAMETER(pReserved);

  if (reason == DLL_PROCESS_ATTACH)
  {
    if (!AfxInitExtensionModule(MikuiDLL, hInstance))
    {
      return 0;
    }
    new CDynLinkLibrary(MikuiDLL);
  }
  else if (reason == DLL_PROCESS_DETACH)
  {
    AfxTermExtensionModule(MikuiDLL);
  }
  return 1;
}

MIKTEXUIEXPORT void MIKTEXCEECALL MiKTeX::UI::MFC::InitializeFramework()
{
  if (!AfxWinInit(::GetModuleHandle(nullptr), nullptr, ::GetCommandLine(), 0))
  {
    MIKTEX_UNEXPECTED();
  }
}

MIKTEXUIEXPORT unsigned int MIKTEXCEECALL MiKTeX::UI::MFC::InstallPackageMessageBox(CWnd * pParent, std::shared_ptr<MiKTeX::Packages::PackageManager> pManager, const char * lpszPackageName, const char * lpszTrigger)
{
  shared_ptr<Session> pSession = Session::Get();
  TriState enableInstaller = pSession->GetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_AUTO_INSTALL, TriState(TriState::Undetermined));
  unsigned int ret;
  if (enableInstaller != TriState::Undetermined)
  {
    ret = DONTASKAGAIN;
    ret |= (enableInstaller == TriState::True ? YES : NO);
  }
  else
  {
    InstallPackageDialog dlg(pParent, pManager, lpszPackageName, lpszTrigger);
    dlg.alwaysAsk = (enableInstaller == TriState::True ? false : true);
    INT_PTR dlgRet = dlg.DoModal();
    if (dlgRet != IDOK && dlgRet != IDCANCEL)
    {
      ret = (NO | DONTASKAGAIN);
    }
    else
    {
      ret = (dlgRet == IDOK ? YES : NO);
      if (dlgRet == IDOK && !dlg.alwaysAsk)
      {
	pSession->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_AUTO_INSTALL, "1");
      }
    }
  }
  return ret;
}

MIKTEXUIEXPORT bool MIKTEXCEECALL MiKTeX::UI::MFC::ProxyAuthenticationDialog(CWnd * pParent)
{
  ProxySettings proxySettings;

  bool done = true;

  if (PackageManager::TryGetProxy(proxySettings)
    && proxySettings.useProxy
    && proxySettings.authenticationRequired
    && proxySettings.user.empty())
  {
    ::ProxyAuthenticationDialog dlg(pParent);
    if (dlg.DoModal() == IDOK)
    {
      proxySettings.user = dlg.GetName();
      proxySettings.password = dlg.GetPassword();
      PackageManager::SetProxy(proxySettings);
    }
    else
    {
      done = false;
    }
  }

  return done;
}

MIKTEXUIEXPORT bool MIKTEXCEECALL MiKTeX::UI::MFC::GiveBackDialog(CWnd * pParent, bool force)
{
  shared_ptr<Session> session = Session::Get();
  bool bonus;
#if HAVE_MIKTEX_USER_INFO
  MiKTeXUserInfo info;
  bonus = session->TryGetMiKTeXUserInfo(info) && info.IsMember();
#else
  bonus = Utils::IsRegisteredMiKTeXUser();
#endif
  static time_t lastShowTime = 0;
  if (force || (difftime(time(nullptr), lastShowTime) > 3600) && !bonus)
  {
    ::GiveBackDialog dlg(pParent);
    lastShowTime = time(nullptr);
    return dlg.DoModal() == IDOK;
  }
  else
  {
    return true;
  }
}
