/* ErrorDialog.cpp:

   Copyright (C) 2008-2021 Christian Schenk

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

#include <QtWidgets>

#include <miktex/Setup/SetupService>

#include "internal.h"

#include "miktex/UI/Qt/ErrorDialog.h"

#include "ErrorDialog.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Setup;
using namespace MiKTeX::UI::Qt;
using namespace MiKTeX::Util;
using namespace std;

int MiKTeX::UI::Qt::ErrorDialog::DoModal(QWidget* parent, const MiKTeXException& e)
{
  ErrorDialogImpl dlg(parent, e);
  return dlg.exec();
}

int MiKTeX::UI::Qt::ErrorDialog::DoModal(QWidget* parent, const exception& e)
{
  ErrorDialogImpl dlg(parent, e);
  return dlg.exec();
}

ErrorDialogImpl::ErrorDialogImpl(QWidget* parent, const MiKTeXException& e) :
  QDialog(parent),
  miktexException(e),
  isMiKTeXException(true)
{
  setupUi(this);
  QString message;
  string description = e.GetDescription();
  if (!description.empty())
  {
    message = QString::fromUtf8(description.c_str());
    string remedy = e.GetRemedy();
    if (!remedy.empty())
    {
      message += "\n\nRemedy: " + QString::fromUtf8(remedy.c_str());
    }
  }
  else
  {
    message = QString::fromUtf8(e.GetErrorMessage().c_str());
  }
  string url = e.GetUrl();
  if (!url.empty())
  {
    message += "\n\nMore info: " + QString::fromUtf8(url.c_str());
  }
  tbMessage->setText(message);
  tbInfo->setText(QString::fromUtf8(e.GetInfo().ToString().c_str()));
}

ErrorDialogImpl::ErrorDialogImpl(QWidget* parent, const exception& e) :
  QDialog(parent),
  stdException(e),
  isMiKTeXException(false)
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
    QMessageBox::information(this, tr("Report Copied"), tr("The error report has been copied to the Clipboard."));
  }
  catch (const exception&)
  {
  }
}

string Trim(const string& str_)
{
  string str = str_;
  constexpr const char* WHITESPACE = " \t\r\n";
  size_t pos = str.find_last_not_of(WHITESPACE);
  if (pos != string::npos)
  {
    str.erase(pos + 1);
  }
  pos = str.find_first_not_of(WHITESPACE);
  if (pos == string::npos)
  {
    str.erase();
  }
  else if (pos != 0)
  {
    str.erase(0, pos);
  }
  return str;
}

string ErrorDialogImpl::CreateReport()
{
  ostringstream s;
  try
  {
    s << T_("GENERAL MIKTEX INFORMATION") << "\n";
    auto setupService = SetupService::Create();
    setupService->WriteReport(s, { ReportOption::General, ReportOption::RootDirectories, ReportOption::CurrentUser, ReportOption::Processes });
    s << "\n";
    s << T_("ERROR DETAILS") << "\n";
    if (isMiKTeXException)
    {
      string programInvocationName = Trim(miktexException.GetProgramInvocationName());
      if (!programInvocationName.empty())
      {
        s << T_("Program: ") << programInvocationName << "\n";
      }
      SourceLocation loc = miktexException.GetSourceLocation();
      if (!loc.fileName.empty())
      {
        s << T_("Source: ") << loc << "\n";
      }
      string errorMessage = Trim(miktexException.GetErrorMessage());
      if (!errorMessage.empty())
      {
        s << T_("Error: ") << errorMessage << "\n";
      }
      string description = Trim(miktexException.GetDescription());
      if (!description.empty())
      {
        s << T_("Description: ") << description << "\n";
      }
      string remedy = Trim(miktexException.GetRemedy());
      if (!remedy.empty())
      {
        s << T_("Remedy: ") << remedy << "\n";
      }
      auto info = miktexException.GetInfo();
      if (!info.empty())
      {
        s << T_("Details: ") << "\n";
        for (const auto& kv : info)
        {
          s << "  " << kv.first << ": " << kv.second << "\n";
        }
      }
    }
    else
    {
      s << T_("Error: ") << stdException.what() << "\n";
    }
  }
  catch (const exception&)
  {
  }
  return s.str();
}
