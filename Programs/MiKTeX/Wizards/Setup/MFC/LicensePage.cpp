/* LicensePage.cpp:

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
#include "LicensePage.h"

BEGIN_MESSAGE_MAP(LicensePage, CPropertyPage)
  ON_BN_CLICKED(IDC_ACCEPT_LICENSE, OnAcceptLicense)
END_MESSAGE_MAP();

LicensePage::LicensePage() :
  CPropertyPage(IDD, 0, IDS_HEADER_LICENSE, IDS_SUBHEADER_LICENSE)
{
}

static void * pLicense = nullptr;
static long offset;
static long sizeLicense;

static DWORD CALLBACK StreamInCallback(DWORD_PTR cookie, unsigned char * pBuf, long sizeBuf, long * pRead)
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
  memcpy(pBuf, static_cast<char *>(pLicense) + offset, *pRead);
  offset += *pRead;
  return 0;
}

BOOL LicensePage::OnInitDialog()
{
  pSheet = reinterpret_cast<SetupWizard *>(GetParent());
  BOOL ret = CPropertyPage::OnInitDialog();
  try
  {
    CHARFORMATW cf;
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_FACE | CFM_SIZE;
    cf.dwEffects = 0;
    cf.bPitchAndFamily = FIXED_PITCH | FF_MODERN;
    StringUtil::CopyString(cf.szFaceName, LF_FACESIZE, _T("Courier New"));
    cf.yHeight = 160;
    licenseControl.SetDefaultCharFormat(cf);
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
    pLicense = LockResource(hglobal);
    if (pLicense == nullptr)
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
    licenseControl.StreamIn(SF_TEXT, editStream);
  }
  catch (const MiKTeXException & e)
  {
    pSheet->ReportError(e);
  }
  catch (const exception & e)
  {
    pSheet->ReportError(e);
  }
  return ret;
}

BOOL LicensePage::OnSetActive()
{
  BOOL ret = CPropertyPage::OnSetActive();
  if (ret)
  {
    pSheet->SetWizardButtons(PSWIZB_BACK | (acceptLicenseButton.GetCheck() == BST_CHECKED ? PSWIZB_NEXT : 0));
  }
  return ret;
}

void LicensePage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LICENSE, licenseControl);
  DDX_Control(pDX, IDC_ACCEPT_LICENSE, acceptLicenseButton);
}

LRESULT LicensePage::OnWizardNext()
{
  pSheet->PushPage(IDD);
  UINT next =
    (SetupApp::Instance->IsMiKTeXDirect
      ? IDD_MD_TASK
      : (SetupApp::Instance->Service->GetOptions().IsPrefabricated
	? (SetupApp::Instance->prefabricatedPackageLevel == PackageLevel::Complete
	  ? IDD_PACKAGE_SET_INSTALL
	  : IDD_SHARED)
	: IDD_TASK));
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(next));
}

LRESULT LicensePage::OnWizardBack()
{
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(pSheet->PopPage()));
}

BOOL LicensePage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  return ret;
}

void LicensePage::OnAcceptLicense()
{
  pSheet->SetWizardButtons(PSWIZB_BACK | (acceptLicenseButton.GetCheck() == BST_CHECKED ? PSWIZB_NEXT : 0));
}
