/* ErrorDialog.cpp:

   Copyright (C) 2008-2016 Christian Schenk

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
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include "StdAfx.h"

#include "internal.h"

#include "miktex/UI/Qt/ErrorDialog.h"

#include "ErrorDialog.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::Qt;
using namespace std;

int MiKTeX::UI::Qt::ErrorDialog::DoModal(QWidget * pParent, const MiKTeXException & e)
{
  ErrorDialogImpl dlg(pParent, e);
  return dlg.exec();
}

int MiKTeX::UI::Qt::ErrorDialog::DoModal(QWidget * pParent, const exception & e)
{
  ErrorDialogImpl dlg(pParent, e);
  return dlg.exec();
}

ErrorDialogImpl::ErrorDialogImpl(QWidget * pParent, const MiKTeXException & e) :
  QDialog(pParent),
  isMiKTeXException(true),
  miktexException(e)
{
  setupUi(this);
  tbMessage->setText(QString::fromUtf8(e.what()));
  tbInfo->setText(QString::fromUtf8(e.GetInfo().c_str()));
}

ErrorDialogImpl::ErrorDialogImpl(QWidget * pParent, const exception & e) :
  QDialog(pParent),
  isMiKTeXException(false),
  stdException(e)
{
  setupUi(this);
  tbMessage->setText(QString::fromUtf8(e.what()));
  tbInfo->setText("");
}

void ErrorDialogImpl::on_btnCopy_clicked()
{
  try
  {
    string report = CreateReport();
    QApplication::clipboard()->setText(QString::fromUtf8(report.c_str()));
    QMessageBox::information(this, T_("Report Copied"), T_("The error report has been copied to the Clipboard."));
  }
  catch (const exception &)
  {
  }
}

string ErrorDialogImpl::CreateReport()
{
  ostringstream s;
  s << T_("MiKTeX Problem Report") << endl
    << T_("Message: ") << (isMiKTeXException ? miktexException.what() : stdException.what()) << endl;
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
      vector<string> invokerNames = Process2::GetInvokerNames();
      s << "MiKTeX: " << Utils::GetMiKTeXVersionString() << endl
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
      s << "SystemAdmin: " << (pSession->RunningAsAdministrator() ? T_("yes") : T_("no")) << endl;
#if defined(MIKTEX_WINDOWS)
      s << "PowerUser: " << (pSession->RunningAsPowerUser() ? T_("yes") : T_("no")) << endl;
#endif
      for (unsigned idx = 0; idx < pSession->GetNumberOfTEXMFRoots(); ++idx)
      {
	PathName absFileName;
	PathName root = pSession->GetRootDirectory(idx);
	s << "Root" << idx << ": " << root.Get() << endl;
      }
      s << "UserInstall: " << pSession->GetSpecialPath(SpecialPath::UserInstallRoot).Get() << endl;
      s << "UserConfig: " << pSession->GetSpecialPath(SpecialPath::UserConfigRoot).Get() << endl;
      s << "UserData: " << pSession->GetSpecialPath(SpecialPath::UserDataRoot).Get() << endl;
      s << "CommonInstall: " << pSession->GetSpecialPath(SpecialPath::CommonInstallRoot).Get() << endl;
      s << "CommonConfig: " << (pSession->GetSpecialPath(SpecialPath::CommonConfigRoot).Get()) << endl;
      s << "CommonData: " << pSession->GetSpecialPath(SpecialPath::CommonDataRoot).Get() << endl;
    }
    catch (const exception &)
    {
      pSession = nullptr;
    }
  }
  return s.str();
}
