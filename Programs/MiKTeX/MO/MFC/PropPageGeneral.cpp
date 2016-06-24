/* PropPageGeneral.cpp:

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

#include "PropPageGeneral.h"
#include "PropSheet.h"
#include "resource.hm"

BEGIN_MESSAGE_MAP(PropPageGeneral, CPropertyPage)
  ON_BN_CLICKED(IDC_REFRESH_FNDB, OnRefreshFndb)
  ON_BN_CLICKED(IDC_UPDATE_FMT, OnUpdateFmt)
  ON_CBN_SELCHANGE(IDC_COMBO_PAPER, &PropPageGeneral::OnChangePaperSize)
  ON_CBN_SELCHANGE(IDC_INSTALL_ON_THE_FLY, OnChangeInstallOnTheFly)
  ON_WM_CONTEXTMENU()
  ON_WM_HELPINFO()
END_MESSAGE_MAP();

PropPageGeneral::PropPageGeneral(std::shared_ptr<MiKTeX::Packages::PackageManager> pManager) :
  CPropertyPage(PropPageGeneral::IDD),
  pManager(pManager)
{
  m_psp.dwFlags &= ~(PSP_HASHELP);
}

BOOL PropPageGeneral::OnInitDialog()
{
  BOOL ret = TRUE;
  try
  {
    switch (session->GetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_AUTO_INSTALL, TriState::Undetermined))
    {
    case TriState::True:
      installOnTheFly = 0;
      break;
    case TriState::False:
      installOnTheFly = 1;
      break;
    case TriState::Undetermined:
      installOnTheFly = 2;
      break;
    }
    previousInstallOnTheFly = installOnTheFly;
    ret = CPropertyPage::OnInitDialog();
    PaperSizeInfo defaultPaperSizeInfo;
    session->GetPaperSizeInfo(-1, defaultPaperSizeInfo);
    PaperSizeInfo paperSizeInfo;
    for (int idx = 0; session->GetPaperSizeInfo(idx, paperSizeInfo); ++idx)
    {
      CString displayName(paperSizeInfo.name.c_str());
      if (!Utils::EqualsIgnoreCase(paperSizeInfo.name, paperSizeInfo.dvipsName))
      {
        displayName += _T(" (");
        displayName += UT_(paperSizeInfo.dvipsName);
        displayName += _T(")");
      }
      if (paperSizeComboBox.AddString(displayName) < 0)
      {
        MIKTEX_FATAL_WINDOWS_ERROR("CComboBox::AddString");
      }
      if (Utils::EqualsIgnoreCase(paperSizeInfo.dvipsName, defaultPaperSizeInfo.dvipsName))
      {
        paperSizeIndex = idx;
        previousPaperSizeIndex = idx;
        if (paperSizeComboBox.SetCurSel(idx) < 0)
        {
          MIKTEX_FATAL_WINDOWS_ERROR("CComboBox::SetCurSel");
        }
      }
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  return ret;
}

void PropPageGeneral::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_CBIndex(pDX, IDC_COMBO_PAPER, paperSizeIndex);
  DDX_CBIndex(pDX, IDC_INSTALL_ON_THE_FLY, installOnTheFly);
  DDX_Control(pDX, IDC_COMBO_PAPER, paperSizeComboBox);
}

BOOL PropPageGeneral::OnApply()
{
  try
  {
    if (installOnTheFly != previousInstallOnTheFly)
    {
      TriState tri(TriState::Undetermined);
      switch (installOnTheFly)
      {
      case 0:
        tri = TriState::True;
        break;
      case 1:
        tri = TriState::False;
        break;
      default:
        tri = TriState::Undetermined;
        break;
      }
      session->SetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_AUTO_INSTALL, (int)tri);
      previousInstallOnTheFly = installOnTheFly;
    }
    if (paperSizeIndex != previousPaperSizeIndex)
    {
      PaperSizeInfo paperSizeInfo;
      if (!session->GetPaperSizeInfo(paperSizeIndex, paperSizeInfo))
      {
        MIKTEX_UNEXPECTED();
      }
      session->SetDefaultPaperSize(paperSizeInfo.dvipsName.c_str());
      previousPaperSizeIndex = paperSizeIndex;
      PropSheet * pSheet = reinterpret_cast<PropSheet*>(GetParent());
      pSheet->ScheduleBuildFormats();
    }
    return TRUE;
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
    return FALSE;
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
    return FALSE;
  }
}

void PropPageGeneral::OnRefreshFndb()
{
  try
  {
    unique_ptr<ProgressDialog> pProgressDialog(ProgressDialog::Create());
    pProgressDialog->StartProgressDialog(GetParent()->GetSafeHwnd());
    pProgressDialog->SetTitle(T_("MiKTeX Maintenance"));
    pProgressDialog->SetLine(1, T_("Collecting file information..."));
    this->pProgressDialog = pProgressDialog.get();
    Fndb::Refresh(this);
    if (!pProgressDialog->HasUserCancelled())
    {
      pProgressDialog->SetLine(1, T_("Scanning package definitions"));
      pProgressDialog->SetLine(2, T_("create the MPM file name database..."));
      pManager->CreateMpmFndb();
    }
    pProgressDialog->StopProgressDialog();
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  this->pProgressDialog = 0;
}

void PropPageGeneral::OnUpdateFmt()
{
  try
  {
    PropSheet * pSheet = reinterpret_cast<PropSheet*>(GetParent());
    pSheet->BuildFormats();
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

void PropPageGeneral::OnChangeInstallOnTheFly()
{
  SetModified(TRUE);
}

void PropPageGeneral::OnChangePaperSize()
{
  SetModified(TRUE);
}

#define MAKE_ID_HID_PAIR(id) id, H##id

namespace
{
  const DWORD aHelpIDs[] = {
    MAKE_ID_HID_PAIR(IDC_INSTALL_ON_THE_FLY),
    MAKE_ID_HID_PAIR(IDC_REFRESH_FNDB),
    MAKE_ID_HID_PAIR(IDC_UPDATE_FMT),
    0, 0,
  };
}

BOOL PropPageGeneral::OnHelpInfo(HELPINFO * pHelpInfo)
{
  try
  {
    return ::OnHelpInfo(pHelpInfo, aHelpIDs, "GeneralPage.txt");
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
    return FALSE;
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
    return FALSE;
  }
}

void PropPageGeneral::OnContextMenu(CWnd * pWnd, CPoint point)
{
  try
  {
    DoWhatsThisMenu(pWnd, point, aHelpIDs, "GeneralPage.txt");
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

bool PropPageGeneral::ReadDirectory(const char * lpszPath, char * * ppSubDirNames, char * * ppFileNames, char * * ppFileNameInfos)

{
  UNUSED_ALWAYS(lpszPath);
  UNUSED_ALWAYS(ppSubDirNames);
  UNUSED_ALWAYS(ppFileNames);
  UNUSED_ALWAYS(ppFileNameInfos);
  return false;
}

bool PropPageGeneral::OnProgress(unsigned level, const char * lpszDirectory)
{
  UNUSED_ALWAYS(level);
  pProgressDialog->SetLine(2, lpszDirectory);
  return !pProgressDialog->HasUserCancelled();
}
