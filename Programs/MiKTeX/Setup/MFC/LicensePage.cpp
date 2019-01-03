/* LicensePage.cpp:

   Copyright (C) 1999-2019 Christian Schenk

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

#include "LicensePage.h"
#include "SetupWizard.h"

BEGIN_MESSAGE_MAP(LicensePage, CPropertyPage)
  ON_BN_CLICKED(IDC_ACCEPT_LICENSE, OnAcceptLicense)
END_MESSAGE_MAP();

LicensePage::LicensePage() :
  CPropertyPage(IDD, 0, IDS_HEADER_LICENSE, IDS_SUBHEADER_LICENSE)
{
}

static void* licenseText = nullptr;
static long offset;
static long sizeLicense;

static DWORD CALLBACK StreamInCallback(DWORD_PTR cookie, unsigned char* pBuf, long sizeBuf, long* pRead)
{
  UNUSED_ALWAYS(cookie);
  if (sizeBuf < sizeLicense - offset)
  {
    *pRead = sizeBuf;
  }
  else
  {
    *pRead = sizeLicense - offset;
  }
  memcpy(pBuf, static_cast<char*>(licenseText) + offset, *pRead);
  offset += *pRead;
  return 0;
}

BOOL LicensePage::OnInitDialog()
{
  sheet = reinterpret_cast<SetupWizard *>(GetParent());
  BOOL ret = CPropertyPage::OnInitDialog();
  try
  {
    HRSRC hrsrc = FindResourceW(nullptr, MAKEINTRESOURCE(IDR_LICENSE), _T("LICENSE"));
    if (hrsrc == nullptr)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("FindResourceW");
    }
    HGLOBAL hglobal = LoadResource(nullptr, hrsrc);
    if (hglobal == nullptr)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("LoadResource");
    }
    licenseText = LockResource(hglobal);
    if (licenseText == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    sizeLicense = SizeofResource(nullptr, hrsrc);
    if (sizeLicense == 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("SizeofResource");
    }
    offset = 0;
    EDITSTREAM editStream;
    editStream.dwCookie = 0;
    editStream.pfnCallback = StreamInCallback;
    licenseControl.SetTargetDevice(nullptr, 0);
    licenseControl.StreamIn(SF_RTF, editStream);

  }
  catch (const MiKTeXException& e)
  {
    sheet->ReportError(e);
  }
  catch (const exception& e)
  {
    sheet->ReportError(e);
  }
  return ret;
}

BOOL LicensePage::OnSetActive()
{
  BOOL ret = CPropertyPage::OnSetActive();
  if (ret)
  {
    sheet->SetWizardButtons(PSWIZB_BACK | (acceptLicenseButton.GetCheck() == BST_CHECKED ? PSWIZB_NEXT : 0));
  }
  return ret;
}

void LicensePage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LICENSE, licenseControl);
  DDX_Control(pDX, IDC_ACCEPT_LICENSE, acceptLicenseButton);
}

LRESULT LicensePage::OnWizardNext()
{
  sheet->PushPage(IDD);
  UINT next;
  if (SetupApp::Instance->IsMiKTeXDirect)
  {
    next = IDD_MD_TASK;
  }
  else if (SetupApp::Instance->Service->GetOptions().IsPrefabricated)
  {
    if (SetupApp::Instance->Service->GetOptions().IsPortable)
    {
      next = IDD_INSTALLDIR;
    }
    else
    {
      next = SetupApp::Instance->prefabricatedPackageLevel == PackageLevel::Complete ? IDD_PACKAGE_SET_INSTALL : IDD_SHARED;
    }
  }
  else
  {
    next = IDD_TASK;
  }
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(next));
}

LRESULT LicensePage::OnWizardBack()
{
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(sheet->PopPage()));
}

BOOL LicensePage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  return ret;
}

void LicensePage::OnAcceptLicense()
{
  sheet->SetWizardButtons(PSWIZB_BACK | (acceptLicenseButton.GetCheck() == BST_CHECKED ? PSWIZB_NEXT : 0));
}
