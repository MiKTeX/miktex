/* CdPage.cpp

   Copyright (C) 2002-2016 Christian Schenk

   This file is part of MiKTeX Update Wizard.

   MiKTeX Update Wizard is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Update Wizard is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Update Wizard; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include "StdAfx.h"
#include "Update.h"

#include "CdPage.h"
#include "UpdateWizard.h"

IMPLEMENT_DYNCREATE(CdPage, CPropertyPage);

BEGIN_MESSAGE_MAP(CdPage, CPropertyPage)
  ON_CBN_SELCHANGE(IDC_CD, OnSelchangeComboDrive)
END_MESSAGE_MAP()

CdPage::CdPage() :
  CPropertyPage(IDD, 0, IDS_HEADER_CD, IDS_SUBHEADER_CD)
{
  m_psp.dwFlags &= ~PSP_HASHELP;
}

BOOL CdPage::OnInitDialog()
{
  pSheet = reinterpret_cast<UpdateWizard *>(GetParent());
  return CPropertyPage::OnInitDialog();
}

BOOL CdPage::OnSetActive()
{
  noDdv = false;
  BOOL ret = CPropertyPage::OnSetActive();
  if (ret)
  {
    try
    {
      ShowDrives();
      EnableButtons();
    }
    catch (const MiKTeXException & e)
    {
      pSheet->ReportError(e);
    }
    catch (const exception & e)
    {
      pSheet->ReportError(e);
    }
  }
  return ret;
}

void CdPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_CBIndex(pDX, IDC_CD, drive);
  DDX_Control(pDX, IDC_CD, driveComboBox);
}

LRESULT CdPage::OnWizardNext()
{
  pSheet->SetCameFrom(IDD);
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(IDD_PACKAGE_LIST));
}

LRESULT CdPage::OnWizardBack()
{
  noDdv = true;
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(IDD_TYPE));
}

BOOL CdPage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  if (ret && !noDdv && !noDriveFound)
  {
    try
    {
      UpdateWizardApplication::packageManager->SetMiKTeXDirectRoot(PathName(drives[drive].c_str(), "\\", nullptr, nullptr));
    }
    catch (const MiKTeXException & e)
    {
      pSheet->ReportError(e);
      ret = FALSE;
    }
    catch (const exception & e)
    {
      pSheet->ReportError(e);
      ret = FALSE;
    }
  }
  return ret;
}

void CdPage::OnSelchangeComboDrive()
{
  try
  {
    EnableButtons();
  }
  catch (const MiKTeXException & e)
  {
    pSheet->ReportError(e);
  }
  catch (const exception & e)
  {
    pSheet->ReportError(e);
  }
}

void CdPage::ShowDrives()
{
  CWaitCursor curWait;

  driveComboBox.ResetContent();
  drives.clear();
  drive = -1;
  noDriveFound = true;

  AutoErrorMode autoMode(SetErrorMode(SEM_FAILCRITICALERRORS));

  DWORD logicalDrives = GetLogicalDrives();
  for (int drv = 0; logicalDrives != 0; logicalDrives >>= 1, ++drv)
  {
    if ((logicalDrives & 1) == 0)
    {
      continue;
    }

    _TCHAR szDrive[4];
    szDrive[0] = _T('A') + static_cast<_TCHAR>(drv);
    szDrive[1] = _T(':');
    szDrive[2] = _T('\\');
    szDrive[3] = 0;

    DWORD fileSystemFlags;
    DWORD maximumComponentLength;
    _TCHAR fileSystemName[BufferSizes::MaxPath];
    _TCHAR volumeName[BufferSizes::MaxPath];

    if (!GetVolumeInformation(szDrive, volumeName, BufferSizes::MaxPath, 0, &maximumComponentLength, &fileSystemFlags, fileSystemName, BufferSizes::MaxPath))
    {
      continue;
    }

    if (!Utils::IsMiKTeXDirectRoot(szDrive))
    {
      continue;
    }

    szDrive[2] = 0;

    CString comboBoxItem(szDrive);
    comboBoxItem += _T(" (");
    comboBoxItem += volumeName;
    comboBoxItem += _T(')');
    if (driveComboBox.AddString(comboBoxItem) < 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CComboBox::AddString");
    }

    drives.push_back(static_cast<const char *>(CW2A(szDrive)));

    noDriveFound = false;
  }

  if (noDriveFound)
  {
    CString str;
    if (!str.LoadString(IDS_NO_DISK))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CString::LoadString");
    }
    driveComboBox.AddString(str);
  }

  drive = 0;
  driveComboBox.SetCurSel(0);
}

void CdPage::EnableButtons()
{
  DWORD flags = PSWIZB_BACK;
  if (driveComboBox.GetCurSel() >= 0 && !noDriveFound)
  {
    flags |= PSWIZB_NEXT;
  }
  pSheet->SetWizardButtons(flags);
}

