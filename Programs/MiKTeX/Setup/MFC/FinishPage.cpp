/* FinishPage.cpp:

   Copyright (C) 1999-2018 Christian Schenk

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
  sheet = reinterpret_cast<SetupWizard*>(GetParent());
  BOOL ret = CPropertyPage::OnInitDialog();
  try
  {
    extern CFont fntWelcome;
    CWnd* wnd = GetDlgItem(IDC_WELCOME);
    if (wnd == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    wnd->SetFont(&fntWelcome);
  }
  catch (const MiKTeXException& e)
  {
    ReportError(e);
  }
  catch (const exception& e)
  {
    ReportError(e);
  }
  return ret;
}

BOOL FinishPage::OnSetActive()
{
  try
  {
    sheet->SetFinishText(T_(_T("Close")));
    if (sheet->GetErrorFlag())
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
      petition.ShowWindow(SW_HIDE);
      checkBoxValue = BST_CHECKED;
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
      checkBoxValue = BST_UNCHECKED;
      UpdateData(FALSE);
    }
    else
    {
      checkBoxValue = BST_CHECKED;
      UpdateData(FALSE);
    }
    CancelToClose();
    return CPropertyPage::OnSetActive();
  }
  catch (const MiKTeXException& e)
  {
    sheet->ReportError(e);
    return FALSE;
  }
  catch (const exception& e)
  {
    sheet->ReportError(e);
    return FALSE;
  }
}

void FinishPage::DoDataExchange(CDataExchange* dx)
{
  CPropertyPage::DoDataExchange(dx);
  DDX_Check(dx, IDC_CHECK, checkBoxValue);
  DDX_Control(dx, IDC_STATUS, status);
  DDX_Control(dx, IDC_MESSAGE, message);
  DDX_Control(dx, IDC_PETITION, petition);
  DDX_Control(dx, IDC_CHECK, checkBox);
}

BOOL FinishPage::OnWizardFinish()
{
  BOOL ret = CPropertyPage::OnWizardFinish();
  if (ret)
  {
    try
    {
      if (checkBoxValue == BST_CHECKED)
      {
        if (sheet->GetErrorFlag())
        {
          SetupApp::Instance->ShowLogFileOnExit = true;
        }
        else
        {
          Utils::ShowWebPage(MIKTEX_URL_WWW_GIVE_BACK);
        }
      }
    }
    catch (const MiKTeXException& e)
    {
      sheet->ReportError(e);
      ret = FALSE;
    }
    catch (const exception& e)
    {
      sheet->ReportError(e);
      ret = FALSE;
    }
  }
  return ret;
}
