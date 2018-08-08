/* ProgramFolderPage.cpp:

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

#include "ProgramFolderPage.h"
#include "SetupWizard.h"

/* _________________________________________________________________________
   
   ProgramFolderPage Message Map
   _________________________________________________________________________ */

BEGIN_MESSAGE_MAP(ProgramFolderPage, CPropertyPage)
  ON_EN_CHANGE(IDC_FOLDER, OnChangeFolder)
END_MESSAGE_MAP();

/* _________________________________________________________________________

   ProgramFolderPage::ProgramFolderPage
   _________________________________________________________________________ */

ProgramFolderPage::ProgramFolderPage ()
  : CPropertyPage (IDD,
		   0,
		   IDS_HEADER_FOLDER,
		   IDS_SUBHEADER_FOLDER),
    pSheet (0)
{
}

/* _________________________________________________________________________

   ProgramFolderPage::OnInitDialog
   _________________________________________________________________________ */

BOOL
ProgramFolderPage::OnInitDialog ()
{
  pSheet = reinterpret_cast<SetupWizard *>(GetParent());
  folderName = SetupApp::Instance->GetFolderName().ToWideCharString().c_str();
  BOOL ret = CPropertyPage::OnInitDialog();
  return (ret);
}

/* _________________________________________________________________________

   ProgramFolderPage::DoDataExchange
   _________________________________________________________________________ */

void
ProgramFolderPage::DoDataExchange (/*[in]*/ CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange (pDX);
  
  DDX_Text (pDX, IDC_FOLDER, folderName);

  if (pDX->m_bSaveAndValidate)
    {
      folderName.TrimLeft ();
      folderName.TrimRight ();
      if (folderName.FindOneOf(_T("\\/:*?\"<>|")) >= 0)
	{
	  AfxMessageBox (T_(_T("The folder name is not valid.")), MB_OK | MB_ICONSTOP);
	  pDX->Fail ();
	}
    }
}

/* _________________________________________________________________________

   ProgramFolderPage::OnWizardNext
   _________________________________________________________________________ */

LRESULT
ProgramFolderPage::OnWizardNext ()
{
  return (reinterpret_cast<LRESULT>(MAKEINTRESOURCE(IDD_SETTINGS)));
}

/* _________________________________________________________________________

   ProgramFolderPage::OnWizardBack
   _________________________________________________________________________ */

LRESULT
ProgramFolderPage::OnWizardBack ()
{
  UINT uPrev;
  switch (SetupApp::Instance->GetTask())
    {
    case SetupTask::InstallFromCD:
    case SetupTask::InstallFromLocalRepository:
    case SetupTask::InstallFromRemoteRepository:
      uPrev = IDD_INSTALLDIR;
      break;
    case SetupTask::PrepareMiKTeXDirect:
      uPrev = IDD_SHARED;
      break;
    default:
      ASSERT (false);
      __assume (false);
      break;
    }
  return (reinterpret_cast<LRESULT>(MAKEINTRESOURCE(uPrev)));
}
/* _________________________________________________________________________

   ProgramFolderPage::OnKillActive
   _________________________________________________________________________ */

BOOL
ProgramFolderPage::OnKillActive ()
{
  BOOL ret = CPropertyPage::OnKillActive();
  if (ret)
  {
    SetupOptions options = SetupApp::Instance->Service->GetOptions();
    options.FolderName = static_cast<LPCTSTR>(folderName);
    SetupApp::Instance->Service->SetOptions(options);
  }
  return (ret);
}

/* _________________________________________________________________________

   ProgramFolderPage::OnChangeFolder
   _________________________________________________________________________ */

void
ProgramFolderPage::OnChangeFolder ()
{
  try
    {
      CWnd * pWnd = GetDlgItem(IDC_FOLDER);
      if (pWnd == 0)
	{
	  MIKTEX_UNEXPECTED();
	}
      CString str;
      pWnd->GetWindowText (str);
      str.TrimLeft ();
      str.TrimRight ();
      pSheet->SetWizardButtons (PSWIZB_BACK
				| (str.GetLength() > 0 ? PSWIZB_NEXT : 0));
    }
  catch (const MiKTeXException & e)
    {
      pSheet->ReportError (e);
    }
  catch (const exception & e)
    {
      pSheet->ReportError (e);
    }
}
