/* SiteWizDrive.cpp:

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

#include "internal.h"

#include "miktex/UI/MFC/ErrorDialog.h"

#include "SiteWizDrive.h"
#include "Resource.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::MFC;
using namespace std;

SiteWizDrive::SiteWizDrive(shared_ptr<PackageManager> pManager) :
  CPropertyPage(IDD_SITEWIZ_DRIVE, IDS_SITEWIZ),
  pManager(pManager)
{
  m_psp.dwFlags &= ~PSP_HASHELP;
}

BEGIN_MESSAGE_MAP(SiteWizDrive, CPropertyPage)
  ON_CBN_SELCHANGE(IDC_COMBO_DRIVE, OnSelchangeComboDrive)
END_MESSAGE_MAP();

void SiteWizDrive::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_COMBO_DRIVE, driveComboBox);
  DDX_CBIndex(pDX, IDC_COMBO_DRIVE, drive);
}

BOOL SiteWizDrive::OnSetActive()
{
  BOOL ret = CPropertyPage::OnSetActive();
  try
  {
    ShowDrives();
    EnableButtons();
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

void SiteWizDrive::ShowDrives()
{
#if 0
  // this requires that we are called by an MFC application
  CWaitCursor curWait;
#endif

  driveComboBox.ResetContent();
  drives.clear();
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

    if (!GetVolumeInformation(szDrive, volumeName, BufferSizes::MaxPath, nullptr, &maximumComponentLength, &fileSystemFlags, fileSystemName, BufferSizes::MaxPath))
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

    drives.push_back(string(TU_(szDrive)));

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

  driveComboBox.SetCurSel(0);
}

void SiteWizDrive::OnSelchangeComboDrive()
{
  try
  {
    EnableButtons();
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

void SiteWizDrive::EnableButtons()
{
  DWORD flags = PSWIZB_BACK;
  if (driveComboBox.GetCurSel() >= 0 && !noDriveFound)
  {
    flags |= PSWIZB_FINISH;
  }
  CPropertySheet * pSheet = reinterpret_cast<CPropertySheet*>(GetParent());
  ASSERT_KINDOF(CPropertySheet, pSheet);
  pSheet->SetWizardButtons(flags);
}

BOOL SiteWizDrive::OnWizardFinish()
{
  try
  {
    UpdateData();
    PathName path(drives[drive].c_str(), "\\", nullptr, nullptr);
    pManager->SetDefaultPackageRepository(RepositoryType::MiKTeXDirect, path.GetData());
    return CPropertyPage::OnWizardFinish();
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

LRESULT SiteWizDrive::OnWizardBack()
{
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(IDD_SITEWIZ_TYPE));
}
