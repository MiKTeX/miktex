/* UpdateWizard.cpp:

   Copyright (C) 2002-2016 Christian Schenk

   This file is part of the MiKTeX Update Wizard.

   The MiKTeX Update Wizard is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Update Wizard is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Update Wizard; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"
#include "Update.h"

#include "UpdateWizard.h"

IMPLEMENT_DYNAMIC(UpdateWizard, CPropertySheet);

BEGIN_MESSAGE_MAP(UpdateWizard, CPropertySheet)
END_MESSAGE_MAP();

CFont fntWelcome;

UpdateWizard::UpdateWizard()
{
  NONCLIENTMETRICS ncm;
  memset(&ncm, 0, sizeof(ncm));
  ncm.cbSize = sizeof(ncm);
  if (WINVER >= 0x0600 && !WindowsVersion::IsWindowsVistaOrGreater())
  {
    // work-around SDK bug
    ncm.cbSize -= sizeof(int/*NONCLIENTMETRICS::iPaddedBorderWidth*/);
  }
  if (!SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("SystemParametersInfo");
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

  Construct(IDS_UPDATEWIZ, nullptr, 0, watermarkBitmap, 0, headerBitmap);

  AddPage(&welcomePage);
  AddPage(&typePage);
  AddPage(&localPage);
  AddPage(&remotePage);
  AddPage(&cdPage);
  AddPage(&packageListPage);
  AddPage(&fileCopyPage);
  AddPage(&finishPage);

  m_psh.dwFlags |= PSH_WIZARD97;
}

BOOL UpdateWizard::OnInitDialog()
{
  BOOL ret = CPropertySheet::OnInitDialog();
  try
  {
    SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), TRUE);
    CString title;
    if (!title.LoadString(UpdateWizardApplication::upgrading ? IDS_UPGRADEWIZ : IDS_UPDATEWIZ))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CString::LoadString");
    }
    if (session->IsAdminMode())
    {
      title += T_(" (Admin)");
    }
    SetTitle(title);
    if (!UpdateWizardApplication::upgrading && session->GetConfigValue("Update", "alwaysWelcome", false))
    {
      SetActivePage(&welcomePage);
    }
    else
    {
      SetActivePage(&typePage);
    }
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

void UpdateWizard::EnableCancelButton(bool enable)
{
  CWnd * pWnd = GetDlgItem(IDCANCEL);
  if (pWnd == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  pWnd->EnableWindow(enable ? TRUE : FALSE);
}

void UpdateWizard::ReportError(const MiKTeXException & e)
{
  SetErrorFlag();
  ::ReportError(e);
}

void UpdateWizard::ReportError(const exception & e)
{
  SetErrorFlag();
  ::ReportError(e);
}

string UpdateWizard::GetRepository()
{
  string repository;
  switch (GetRepositoryType())
  {
  case RepositoryType::Remote:
    if (randomRepositoryFlag)
    {
      if (randomRepository.empty())
      {
        UpdateWizardApplication::packageManager->SetRepositoryReleaseState(GetRepositoryReleaseState());
        randomRepository = UpdateWizardApplication::packageManager->PickRepositoryUrl();
      }
      repository = randomRepository;
    }
    else
    {
      repository = UpdateWizardApplication::packageManager->GetRemotePackageRepository();
    }
    break;
  case RepositoryType::Local:
    repository = UpdateWizardApplication::packageManager->GetLocalPackageRepository().Get();
    break;
  case RepositoryType::MiKTeXDirect:
    repository = UpdateWizardApplication::packageManager->GetMiKTeXDirectRoot().Get();
    break;
  };
  return repository;
}
