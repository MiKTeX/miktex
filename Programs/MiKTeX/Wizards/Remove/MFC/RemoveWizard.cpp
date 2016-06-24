/* RemoveWizard.cpp:

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

#include "FinishPage.h"
#include "RemoveFilesPage.h"
#include "RemoveWizard.h"
#include "WelcomePage.h"

BEGIN_MESSAGE_MAP(RemoveWizard, CPropertySheet)
END_MESSAGE_MAP();

CFont fntWelcome;

RemoveWizard::RemoveWizard()
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
    MIKTEX_FATAL_WINDOWS_ERROR(T_("SystemParametersInfo"));
  }
  LOGFONT TitleLogFont = ncm.lfMessageFont;
  TitleLogFont.lfWeight = FW_BOLD;
  StringUtil::CopyString(TitleLogFont.lfFaceName, LF_FACESIZE, _T("Verdana Bold"));
  HDC hdc = ::GetDC(nullptr);
  TitleLogFont.lfHeight = 0 - GetDeviceCaps(hdc, LOGPIXELSY) * 12 / 72;
  ::ReleaseDC(0, hdc);
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

  Construct(IDS_REMOVEWIZ, nullptr, 0, watermarkBitmap, 0, headerBitmap);

  AddPage(new WelcomePage());
  AddPage(new RemoveFilesPage());
  AddPage(new FinishPage());

  m_psh.dwFlags |= PSH_WIZARD97;
}

BOOL RemoveWizard::OnInitDialog()
{
  BOOL ret = CPropertySheet::OnInitDialog();
  try
  {
    SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), TRUE);
    CString title;
    if (!title.LoadString(IDS_REMOVEWIZ))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CString::LoadString");
    }
    SetTitle(title);
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

void RemoveWizard::EnableCancelButton(bool enable)
{
  CWnd * pWnd = GetDlgItem(IDCANCEL);
  if (pWnd == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  pWnd->EnableWindow(enable ? TRUE : FALSE);
}

void RemoveWizard::ReportError(const MiKTeXException & e)
{
  SetErrorFlag();
  ::ReportError(e);
}

void RemoveWizard::ReportError(const exception & e)
{
  SetErrorFlag();
  ::ReportError(e);
}

CString RemoveWizard::SetNextText(const char * lpszText)
{
  CWnd * pWnd = GetDlgItem(ID_WIZNEXT);
  if (pWnd == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  CString ret;
  pWnd->GetWindowText(ret);
  pWnd->SetWindowText(UT_(lpszText));
  return ret;
}
