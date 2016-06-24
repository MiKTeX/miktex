/* FinishPage.cpp:

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

#include "FinishPage.h"
#include "UpdateWizard.h"

IMPLEMENT_DYNCREATE(FinishPage, CPropertyPage);

BEGIN_MESSAGE_MAP(FinishPage, CPropertyPage)
END_MESSAGE_MAP();

FinishPage::FinishPage()
  : CPropertyPage(IDD)
{
#if HAVE_MIKTEX_USER_INFO
  MiKTeXUserInfo info;
  if (session->TryGetMiKTeXUserInfo(info) && info.IsMember())
  {
    visitWebSite = BST_UNCHECKED;
  }
#endif
  m_psp.dwFlags |= PSP_HIDEHEADER;
}

BOOL FinishPage::OnInitDialog()
{
  pSheet = reinterpret_cast<UpdateWizard *>(GetParent());
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
    pSheet->SetFinishText(T_(_T("Finish")));
    if (pSheet->GetErrorFlag())
    {
      CString str;
      if (!str.LoadString(IDS_FINISH_ERROR))
      {
        MIKTEX_UNEXPECTED();
      }
      CWnd * pWnd;
      pWnd = GetDlgItem(IDC_STATUS);
      if (pWnd == nullptr)
      {
        MIKTEX_UNEXPECTED();
      }
      pWnd->SetWindowText(str);
      if (!str.LoadString(IDS_REMEDY))
      {
        MIKTEX_UNEXPECTED();
      }
      pWnd = GetDlgItem(IDC_MESSAGE);
      if (pWnd == nullptr)
      {
        MIKTEX_UNEXPECTED();
      }
      pWnd->SetWindowText(str);
      viewReport = BST_CHECKED;
      visitWebSite = BST_UNCHECKED;
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
  DDX_Check(pDX, IDC_VIEW_REPORT, viewReport);
  DDX_Check(pDX, IDC_VISIT_WEB_SITE, visitWebSite);
}

BOOL FinishPage::OnWizardFinish()
{
  BOOL ret = CPropertyPage::OnWizardFinish();
  if (ret)
  {
    try
    {
      if (viewReport == BST_CHECKED)
      {
        log4cxx::RollingFileAppenderPtr appender = log4cxx::Logger::getRootLogger()->getAppender(LOG4CXX_STR("RollingLogFile"));
        if (appender != nullptr)
        {
          if (ShellExecuteW(nullptr, L"open", appender->getFile().c_str(), nullptr, nullptr, SW_SHOWNORMAL) <= reinterpret_cast<HINSTANCE>(32))
          {
            Process::Start("notepad.exe", WU_(appender->getFile()));
          }
        }
      }
      if (visitWebSite == BST_CHECKED)
      {
        Utils::ShowWebPage(MIKTEX_URL_WWW_PACKAGING);
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
