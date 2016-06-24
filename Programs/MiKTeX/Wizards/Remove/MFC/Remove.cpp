/* Remove.cpp:

   Copyright (C) 2000-2016 Christian Schenk

   This file is part of the Remove MiKTeX! Wizard.

   The Remove MiKTeX! Wizard is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The Remove MiKTeX! Wizard is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the Remove MiKTeX! Wizard; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"

#include "Remove.h"

#include "RemoveWizard.h"

BEGIN_MESSAGE_MAP(RemoveWizardApp, CWinApp)
#if 0
  ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
#endif
END_MESSAGE_MAP();

RemoveWizardApp::RemoveWizardApp()
{
  SetAppID(UT_("MiKTeXorg.MiKTeX.Remove." MIKTEX_COMPONENT_VERSION_STR));
}

RemoveWizardApp theApp;

BOOL RemoveWizardApp::InitInstance()
{
  INITCOMMONCONTROLSEX initCtrls;

  initCtrls.dwSize = sizeof(initCtrls);
  initCtrls.dwICC = ICC_WIN95_CLASSES;

  if (!InitCommonControlsEx(&initCtrls))
  {
    AfxMessageBox(T_(_T("The application could not be initialized (1).")), MB_ICONSTOP | MB_OK);
    return FALSE;
  }

  if (FAILED(CoInitialize(0)))
  {
    AfxMessageBox(T_(_T("The application could not be initialized (2).")), MB_ICONSTOP | MB_OK);
    return FALSE;
  }

  try
  {
    shared_ptr<Session> pSession = Session::Create(Session::InitInfo("remove"));

    {
      RemoveWizard dlg;
      m_pMainWnd = &dlg;
      dlg.DoModal();
    }

    pSession = nullptr;
  }
  catch (const MiKTeXException & e)
  {
    ReportError(e);
  }
  catch (const exception & e)
  {
    ReportError(e);
  }

  return FALSE;
}

void ReportError(const MiKTeXException & e)
{
  string str;
  str = T_("The operation could not be completed for the following reason: ");
  str += "\n\n";
  str += e.what();
  if (!e.GetInfo().empty())
  {
    str += "\n\n";
    str += T_("Details: ");
    str += e.GetInfo();
  }
  AfxMessageBox(UT_(str), MB_OK | MB_ICONSTOP);
}

void ReportError(const exception & e)
{
  string str;
  str = T_("The operation could not be completed for the following reason: ");
  str += "\n\n";
  str += e.what();
  AfxMessageBox(UT_(str), MB_OK | MB_ICONSTOP);
}

bool Contains(const vector<PathName> & vec, const PathName & pathName)
{
  for (vector<PathName>::const_iterator it = vec.begin(); it != vec.end(); ++it)
  {
    if (*it == pathName)
    {
      return true;
    }
  }
  return false;
}

vector<PathName> GetRoots()
{
  std::shared_ptr<Session> session = Session::Get();
  vector<PathName> vec;
  if (!session->IsMiKTeXDirect())
  {
    PathName installRoot = session->GetSpecialPath(SpecialPath::InstallRoot);
    vec.push_back(installRoot);
  }
  PathName userDataRoot = session->GetSpecialPath(SpecialPath::UserDataRoot);
  if (!Contains(vec, userDataRoot))
  {
    vec.push_back(userDataRoot);
  }
  PathName userConfigRoot = session->GetSpecialPath(SpecialPath::UserConfigRoot);
  if (!Contains(vec, userConfigRoot))
  {
    vec.push_back(userConfigRoot);
  }
  if (session->IsAdminMode())
  {
    PathName commonDataRoot = session->GetSpecialPath(SpecialPath::CommonDataRoot);
    if (!Contains(vec, commonDataRoot))
    {
      vec.push_back(commonDataRoot);
    }
    PathName commonConfigRoot = session->GetSpecialPath(SpecialPath::CommonConfigRoot);
    if (!Contains(vec, commonConfigRoot))
    {
      vec.push_back(commonConfigRoot);
    }
  }
  return vec;
}
