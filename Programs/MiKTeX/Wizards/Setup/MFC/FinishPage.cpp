/* FinishPage.cpp:

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

#include "FinishPage.h"
#include "SetupWizard.h"

BEGIN_MESSAGE_MAP(FinishPage, CPropertyPage)
END_MESSAGE_MAP();

FinishPage::FinishPage() :
  CPropertyPage(IDD)
{
  m_psp.dwFlags |= PSP_HIDEHEADER;
}

BOOL FinishPage::OnInitDialog()
{
  pSheet = reinterpret_cast<SetupWizard *>(GetParent());
  BOOL ret = CPropertyPage::OnInitDialog();
  try
  {
    extern CFont fntWelcome;
    CWnd * pWnd = GetDlgItem(IDC_WELCOME);
    if (pWnd == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    pWnd->SetFont(&fntWelcome);
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

BOOL FinishPage::OnSetActive()
{
  try
  {
    pSheet->SetFinishText(T_(_T("Close")));

    if (pSheet->GetErrorFlag())
    {
      CString str;

      if (!str.LoadString(IDS_FINISH_ERROR))
      {
	MIKTEX_UNEXPECTED();
      }

      status.SetWindowText(str);

      if (!str.LoadString(IDS_REMEDY))
      {
	MIKTEX_UNEXPECTED();
      }

      message.SetWindowText(str);

      viewReleaseNotes = BST_CHECKED;

      UpdateData(FALSE);

      if (!str.LoadString(IDS_VIEW_LOG_FILE))
      {
	MIKTEX_UNEXPECTED();
      }

      checkBox.SetWindowText(str);
    }
    else if (SetupApp::Instance->GetTask() == SetupTask::Download)
    {
      CString str;

      if (!str.LoadString(IDS_DOWNLOAD_COMPLETE))
      {
	MIKTEX_UNEXPECTED();
      }

      message.SetWindowText(str);

      viewReleaseNotes = BST_UNCHECKED;

      UpdateData(FALSE);

      checkBox.ShowWindow(SW_HIDE);
    }
    else
    {
      viewReleaseNotes = BST_UNCHECKED;
      UpdateData(FALSE);
    }

    CancelToClose();

    return CPropertyPage::OnSetActive();
  }
  catch (const MiKTeXException & e)
  {
    pSheet->ReportError(e);
    return FALSE;
  }
  catch (const exception & e)
  {
    pSheet->ReportError(e);
    return FALSE;
  }
}

void FinishPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Check(pDX, IDC_VIEW_RELNOTES, viewReleaseNotes);
  DDX_Control(pDX, IDC_STATUS, status);
  DDX_Control(pDX, IDC_MESSAGE, message);
  DDX_Control(pDX, IDC_VIEW_RELNOTES, checkBox);
}

BOOL FinishPage::OnWizardFinish()
{
  BOOL ret = CPropertyPage::OnWizardFinish();
  if (ret)
  {
    try
    {
      CWnd * pWnd = GetDlgItem(IDC_VIEW_RELNOTES);
      if (pWnd == nullptr)
      {
	MIKTEX_UNEXPECTED();
      }
      if (viewReleaseNotes == BST_CHECKED)
      {
	if (pSheet->GetErrorFlag())
	{
	  SetupApp::Instance->ShowLogFileOnExit = true;
	}
	else
	{
	  PathName pathRelNotes(SetupApp::Instance->GetInstallRoot(), MIKTEX_PATH_RELNOTES_HTML);
	  if (ShellExecuteW(nullptr, L"open", UW_(pathRelNotes.GetData()), nullptr, nullptr, SW_SHOWNORMAL) <= reinterpret_cast<HINSTANCE>(32))
	  {
	    MIKTEX_FATAL_ERROR_2(T_("The file could not be opened."), "path", pathRelNotes.ToString());
	  }
	}
      }
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
