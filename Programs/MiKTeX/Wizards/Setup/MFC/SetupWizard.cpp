/* SetupWizard.cpp:

   Copyright (C) 1999-2016 Christian Schenk

   This file is part of the MiKTeX Setup Wizard.

   The MiKTeX Setup Wizard is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Setup Wizard is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Setup Wizard; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"

#include "Setup.h"

#include "SetupWizard.h"

BEGIN_MESSAGE_MAP(SetupWizard, CPropertySheet)
END_MESSAGE_MAP();

CFont fntWelcome;

SetupWizard::SetupWizard(shared_ptr<PackageManager> packageManager) :
  packageManager(packageManager),
  downloadPackageSetPage(true),
  installPackageSetPage(false)
{
  NONCLIENTMETRICSW ncm;
  memset(&ncm, 0, sizeof(ncm));
  ncm.cbSize = sizeof(ncm);
  if (WINVER >= 0x0600 && !WindowsVersion::IsWindowsVistaOrGreater())
  {
    // work-around SDK bug
    ncm.cbSize -= sizeof(int/*NONCLIENTMETRICS::iPaddedBorderWidth*/);
  }
  if (!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("SystemParametersInfoW");
  }
  LOGFONT TitleLogFont = ncm.lfMessageFont;
  TitleLogFont.lfWeight = FW_BOLD;
  StringUtil::CopyString(TitleLogFont.lfFaceName, LF_FACESIZE, _T("Verdana Bold"));
  HDC hdc = ::GetDC(nullptr);
  TitleLogFont.lfHeight = 0 - GetDeviceCaps(hdc, LOGPIXELSY) * 12 / 72;
  ::ReleaseDC(nullptr, hdc);
  if (!fntWelcome.CreateFontIndirect(&TitleLogFont))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CFont::CreateFontIndirect");
  }

  if (!watermarkBitmap.LoadBitmap(IDB_WATERMARK))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CBitmap::LoadBitmap");
  }

  if (!headerBitmap.LoadBitmap(IDB_HEADER))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CBitmap::LoadBitmap");
  }

  Construct(IDS_SETUPWIZ, nullptr, 0, watermarkBitmap, nullptr, headerBitmap);

  if (!SetupApp::Instance->IsUnattended)
  {
    AddPage(&welcomePage);
    AddPage(&licensePage);
    AddPage(&mdTaskPage);
    AddPage(&taskPage);
    AddPage(&remoteRepositoryPage);
    AddPage(&localRepositoryPage);
    AddPage(&downloadPackageSetPage);
    AddPage(&installPackageSetPage);
    AddPage(&sharedInstallationPage);
    AddPage(&installDirPage);
#if SHOW_FOLDER_PAGE
    AddPage(&programFolderPage);
#endif
    AddPage(&settingsPage);
#if ENABLE_ADDTEXMF
    AddPage(&addTEXMFPage);
    AddPage(&editTEXMFPage);
#endif
    AddPage(&infoListPage);
  }

  AddPage(&fileCopyPage);

  if (!SetupApp::Instance->IsUnattended)
  {
    AddPage(&finishPage);
  }

  m_psh.dwFlags |= PSH_WIZARD97;
}

BOOL SetupWizard::OnInitDialog()
{
  BOOL ret = CPropertySheet::OnInitDialog();
  try
  {
    SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), TRUE);
    CString title;
    if (SetupApp::Instance->IsMiKTeXDirect)
    {
      title.Format(T_(_T("MiKTeX %s Setup (%d-bit)")), UT_(MIKTEX_FULL_VERSION_STR), static_cast<int>(sizeof(void*)) * 8);
    }
    else if (SetupApp::Instance->Service->GetOptions().IsPrefabricated)
    {
      PathName configFile(SetupApp::Instance->GetLocalPackageRepository());
      configFile /= "pr.ini";
      unique_ptr<Cfg> pConfig(Cfg::Create());
      pConfig->Read(configFile);
      CString prefix;
      CString version(UT_(MIKTEX_SERIES_STR));
      version += _T('.');
      version += UT_(pConfig->GetValue("repository", "version").c_str());
      switch (SetupApp::Instance->GetPackageLevel())
      {
      case PackageLevel::Essential:
	prefix = T_(_T("Essential "));
	break;
      case PackageLevel::Basic:
	prefix = T_(_T("Basic "));
	break;
      case PackageLevel::Complete:
	prefix = _T("");
	break;
      default:
	MIKTEX_ASSERT(false);
	__assume (false);
	break;
      }
      title.Format(T_(_T("%sMiKTeX %s Installer (%d-bit)")),
	static_cast<LPCTSTR>(prefix),
	static_cast<LPCTSTR>(version),
	static_cast<int>(sizeof(void*)) * 8);
    }
    else
    {
      title.Format(T_(_T("MiKTeX %s Net Installer (%d-bit)")), UT_(MIKTEX_VERSION_STR), static_cast<int>(sizeof(void*)) * 8);
    }
    SetTitle(title);
    SetActivePage(&licensePage);
  }
  catch (const MiKTeXException & e)
  {
    ReportError(e);
  }
  catch (const exception & e)
  {
    ReportError(e);
  }
  return ret;
}

void SetupWizard::EnableCancelButton(bool enable)
{
  CWnd * pWnd = GetDlgItem(IDCANCEL);
  if (pWnd == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  pWnd->EnableWindow(enable ? TRUE : FALSE);
}

CString SetupWizard::SetNextText(LPCTSTR lpszText)
{
  CWnd * pWnd = GetDlgItem(ID_WIZNEXT);
  if (pWnd == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  CString ret;
  pWnd->GetWindowText(ret);
  pWnd->SetWindowText(lpszText);
  return ret;
}

void SetupWizard::ReportError(const MiKTeXException & e)
{
  SetErrorFlag();
  ::ReportError(e);
}

void SetupWizard::ReportError(const exception & e)
{
  SetErrorFlag();
  ::ReportError(e);
}
