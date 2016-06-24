/* mo.cpp: MiKTeX Options

   Copyright (C) 2000-2016 Christian Schenk

   This file is part of MiKTeX Options.

   MiKTeX Options is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX Options is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Options; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include "StdAfx.h"
#include "mo.h"

#include "PropSheet.h"

MiKTeXOptionsApplication theApp;

BEGIN_MESSAGE_MAP(MiKTeXOptionsApplication, CWinApp)
  ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP();

MiKTeXOptionsApplication::MiKTeXOptionsApplication()
{
  SetAppID(UT_("MiKTeXorg.MiKTeX.MiKTeXOptions." MIKTEX_COMPONENT_VERSION_STR));
  // EnableHtmlHelp ();
}

BOOL MiKTeXOptionsApplication::InitInstance()
{
  INITCOMMONCONTROLSEX initCtrls;

  initCtrls.dwSize = sizeof(initCtrls);
  initCtrls.dwICC = ICC_WIN95_CLASSES;

  if (!InitCommonControlsEx(&initCtrls))
  {
    MIKTEX_UNEXPECTED();
    return FALSE;
  }

  if (!CWinApp::InitInstance())
  {
    MIKTEX_UNEXPECTED();
    return FALSE;
  }

  if (FAILED(CoInitializeEx(0, COINIT_APARTMENTTHREADED)))
  {
    MIKTEX_UNEXPECTED();
    return FALSE;
  }

  try
  {
    shared_ptr<Session> pSession = Session::Create(Session::InitInfo("mo"));

    if (Utils::RunningOnAServer())
    {
      UI::MFC::GiveBackDialog(0);
    }

    if (!pSession->IsMiKTeXPortable() && !Utils::CheckPath(false))
    {
      if (AfxMessageBox(T_(_T("MiKTeX is not correctly configured: the location of the MiKTeX executables is not known to the operating system.\r\n\r\nClick OK to repair the MiKTeX configuration.")), MB_OKCANCEL) == IDOK)
      {
        Utils::CheckPath(true);
      }
      else
      {
        if (pSession ->GetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_INSIST_ON_REPAIR, false))
        {
          MIKTEX_FATAL_ERROR(T_("Broken MiKTeX configuration."));
        }
      }
    }

    std::shared_ptr<MiKTeX::Packages::PackageManager> pManager(PackageManager::Create());

    {                  // destroy dlg before MiKTeX is uninitialized
      PropSheet dlg(pManager);
      INT_PTR ret = dlg.DoModal();
      if (ret == IDOK
        && dlg.MustBuildFormats()
        && (AfxMessageBox(T_(_T("To apply the new settings, it is necessary to rebuild the format files.")), MB_OKCANCEL) == IDOK))
      {
        dlg.BuildFormats();
      }
    }

    pManager = nullptr;
    pSession = nullptr;
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(0, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(0, e);
  }

  CoUninitialize();

  return FALSE;
}

void MiKTeXOptionsApplication::WinHelp(DWORD data, UINT cmd)
{
  UNUSED_ALWAYS(data);
  UNUSED_ALWAYS(cmd);
}

PathName FindHelpFile()
{
  shared_ptr<Session> session = Session::Get();
  PathName path;
  if (!session->FindFile("mo409.chm", "%R\\doc\\miktex//", path))
  {
    MIKTEX_FATAL_ERROR(T_("The help file could not be found."));
  }
  return path;
}

BOOL OnHelpInfo(HELPINFO * pHelpInfo, const DWORD * pHelpIds, const char * lpszTopicFile)
{
  try
  {
    PathName helpFile = FindHelpFile();
    bool found = false;
    for (size_t i = 0; !found && pHelpIds[i] != 0; i += 2)
    {
      if (pHelpIds[i] == static_cast<DWORD>(pHelpInfo->iCtrlId))
      {
        found = true;
      }
    }
    if (!found)
    {
      return FALSE;
    }
    CString htmlHelpUrl(UT_(helpFile.Get()));
    htmlHelpUrl += _T("::/");
    htmlHelpUrl += lpszTopicFile;
    HtmlHelp(reinterpret_cast<HWND>(pHelpInfo->hItemHandle), htmlHelpUrl, HH_TP_HELP_WM_HELP, reinterpret_cast<DWORD_PTR>(const_cast<DWORD*>(pHelpIds)));
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(0, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(0, e);
  }
  return TRUE;
}

void DoWhatsThisMenu(CWnd * pWnd, CPoint point, const DWORD * pHelpIds, const char * lpszTopicFile)
{
  // HtmlHelp() (unlike WinHelp()) doesn't display the "What's This?"
  // menu
  CMenu menu;
  if (!menu.LoadMenu(IDR_MENU_WHATS_THIS))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CMenu::LoadMenu");
  }
  CMenu * pPopup = menu.GetSubMenu(0);
  if (pPopup == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CMenu::GetSubMenu");
  }
  if (pPopup->TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, pWnd) != ID_WHATS_THIS)
  {
    if (::GetLastError() != ERROR_SUCCESS)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CMenu::TrackPopupMenu");
    }
    return;
  }
  string helpFileUrl = FindHelpFile().ToString();
  helpFileUrl += "::/";
  helpFileUrl += lpszTopicFile;
  HtmlHelp(pWnd->GetSafeHwnd(), UT_(helpFileUrl), HH_TP_HELP_CONTEXTMENU, reinterpret_cast<DWORD_PTR>(const_cast<DWORD*>(pHelpIds)));
}

#define PACKVERSION(major,minor) MAKELONG(minor,major)

// See MSDN: Shell and Common Controls Versions
DWORD GetDllVersion(const char * lpszDllName)
{
  DWORD version = 0;

  HINSTANCE hinstDll = LoadLibraryW(UW_(lpszDllName));

  if (hinstDll != nullptr)
  {
    DLLGETVERSIONPROC pDllGetVersion = reinterpret_cast<DLLGETVERSIONPROC>(GetProcAddress(hinstDll, "DllGetVersion"));
    if (pDllGetVersion != nullptr)
    {
      DLLVERSIONINFO dvi;
      ZeroMemory(&dvi, sizeof(dvi));
      dvi.cbSize = sizeof(dvi);
      HRESULT hr = (*pDllGetVersion)(&dvi);
      if (SUCCEEDED(hr))
      {
        version = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
      }
    }
    FreeLibrary(hinstDll);
  }

  return version;
}
