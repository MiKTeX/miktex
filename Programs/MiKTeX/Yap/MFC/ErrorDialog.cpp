/* ErrorDialog.cpp:

   Copyright (C) 2000-2018 Christian Schenk

   This file is part of MiKTeX UI Library.

   MiKTeX UI Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX UI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX UI Library; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include "StdAfx.h"

#include "internal.h"

#include "miktex/UI/MFC/ErrorDialog.h"

#include "resource.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::UI::MFC;
using namespace MiKTeX::Util;
using namespace std;

class ErrorDialogImpl :
  public CDialog
{
private:
  DECLARE_MESSAGE_MAP();

private:
  enum { IDD = IDD_ERROR };

public:
  ErrorDialogImpl(CWnd * pParent, const MiKTeXException & e);

public:
  ErrorDialogImpl(CWnd * pParent, const exception & e);

private:
  BOOL OnInitDialog() override;

protected:
  afx_msg void OnCopy();

private:
  void DoDataExchange(CDataExchange * pDX) override;

private:
  string CreateReport();

private:
  CString message;

private:
  CString info;

private:
  MiKTeXException miktexException;

private:
  exception stdException;

private:
  bool isMiKTeXException;
};

BEGIN_MESSAGE_MAP(ErrorDialogImpl, CDialog)
  ON_BN_CLICKED(IDC_COPY, &ErrorDialogImpl::OnCopy)
END_MESSAGE_MAP();

ErrorDialogImpl::ErrorDialogImpl(CWnd * pParent, const MiKTeXException & e) :
  CDialog(IDD, pParent),
  isMiKTeXException(true),
  miktexException(e),
  message(e.what()),
  info(e.GetInfo().c_str())
{
}

ErrorDialogImpl::ErrorDialogImpl(CWnd * pParent, const exception & e) :
  CDialog(IDD, pParent),
  isMiKTeXException(false),
  stdException(e),
  message(e.what())
{
}

void ErrorDialogImpl::DoDataExchange(CDataExchange * pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_MESSAGE, message);
  DDX_Text(pDX, IDC_INFO, info);
}

BOOL ErrorDialogImpl::OnInitDialog()
{
  BOOL ret = CDialog::OnInitDialog();
  return ret;
}

INT_PTR ErrorDialog::DoModal(CWnd * pParent, const MiKTeXException & e)
{
  try
  {
    BEGIN_USE_MY_RESOURCES()
    {
      ErrorDialogImpl dlg(pParent, e);
      return dlg.DoModal();
    }
    END_USE_MY_RESOURCES()
  }
  catch (const exception & e)
  {
    AfxMessageBox(UT_(e.what()));
    return -1;
  }
}

INT_PTR ErrorDialog::DoModal(CWnd * pParent, const exception & e)
{
  try
  {
    BEGIN_USE_MY_RESOURCES()
    {
      ErrorDialogImpl dlg(pParent, e);
      return dlg.DoModal();
    }
    END_USE_MY_RESOURCES()
  }
  catch (const exception & e)
  {
    AfxMessageBox(UT_(e.what()));
    return -1;
  }
}

void ErrorDialogImpl::OnCopy()
{
  bool opened = false;
  HGLOBAL hGlobal = nullptr;
  try
  {
    string report = CreateReport();
    if (!OpenClipboard())
    {
      MIKTEX_UNEXPECTED();
    }
    opened = true;
    if (!EmptyClipboard())
    {
      MIKTEX_UNEXPECTED();
    }
    hGlobal = GlobalAlloc(GMEM_DDESHARE, report.length() + 1);
    if (hGlobal == nullptr)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("GlobalAlloc");
    }
    void * pGlobal = GlobalLock(hGlobal);
    if (pGlobal == nullptr)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("GlobalLock");
    }
    else
    {
      AutoGlobal autoGlobal(pGlobal);
      StringUtil::CopyString(reinterpret_cast<char *>(pGlobal), report.length() + 1, report.c_str());
    }
    if (SetClipboardData(CF_TEXT, hGlobal) == nullptr)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("SetClipboardData");
    }
    hGlobal = nullptr;
    AfxMessageBox(T_(_T("The error report has been copied to the Clipboard.")));
  }
  catch (const exception & e)
  {
    AfxMessageBox(UT_(e.what()));
  }
  if (hGlobal != nullptr)
  {
    GlobalFree(hGlobal);
  }
  if (opened)
  {
    CloseClipboard();
  }
}

string ErrorDialogImpl::CreateReport()
{
  ostringstream s;
  s << T_("MiKTeX Problem Report") << endl
    << T_("Message: ")
    << (isMiKTeXException ? miktexException.what() : stdException.what())
    << endl;
  if (isMiKTeXException)
  {
    s << T_("Data: ") << miktexException.GetInfo() << endl
      << T_("Source: ") << miktexException.GetSourceFile() << endl
      << T_("Line: ") << miktexException.GetSourceLine() << endl;
  }
  shared_ptr<Session> pSession = Session::TryGet();
  if (pSession != nullptr)
  {
    try
    {
      vector<string> invokerNames = Process::GetInvokerNames();
      s << "MiKTeX: "
	<< Utils::GetMiKTeXVersionString() << endl
	<< "OS: " << Utils::GetOSVersionString() << endl;
      s << "Invokers: ";
      for (vector<string>::const_iterator it = invokerNames.begin(); it != invokerNames.end(); ++it)
      {
	if (it != invokerNames.begin())
	{
	  s << "/";
	}
	s << *it;
      }
      s << endl;
      s << "SystemAdmin: " << (pSession->RunningAsAdministrator()
	? T_("yes")
	: T_("no"))
	<< endl;
      s << "PowerUser: " << (pSession->RunningAsPowerUser()
	? T_("yes")
	: T_("no"))
	<< endl;
      for (unsigned idx = 0; idx < pSession->GetNumberOfTEXMFRoots(); ++idx)
      {
	PathName absFileName;
	PathName root = pSession->GetRootDirectoryPath(idx);
	s << "Root" << idx << ": " << root.GetData() << endl;
      }
      s << "UserInstall: "
	<< pSession->GetSpecialPath(SpecialPath::UserInstallRoot).GetData()
	<< endl;
      s << "UserConfig: "
	<< pSession->GetSpecialPath(SpecialPath::UserConfigRoot).GetData()
	<< endl;
      s << "UserData: "
	<< pSession->GetSpecialPath(SpecialPath::UserDataRoot).GetData()
	<< endl;
      s << "CommonInstall: "
	<< pSession->GetSpecialPath(SpecialPath::CommonInstallRoot).GetData()
	<< endl;
      s << "CommonConfig: "
	<< (pSession->GetSpecialPath(SpecialPath::CommonConfigRoot).GetData())
	<< endl;
      s << "CommonData: "
	<< pSession->GetSpecialPath(SpecialPath::CommonDataRoot).GetData()
	<< endl;
    }
    catch (const exception &)
    {
      pSession = nullptr;
    }
  }
  return s.str();
}
