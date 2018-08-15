/* ErrorDialog.cpp:

   Copyright (C) 2008-2018 Christian Schenk

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
  isMiKTeXException(true),
  miktexException(e)
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
  catch (const exception&)
  {
  }
}

string ErrorDialogImpl::CreateReport()
{
  ostringstream s;
  try
  {
    SetupService::WriteReport(s, { ReportOption::General, ReportOption::RootDirectories, ReportOption::CurrentUser, ReportOption::Processes });
  }
  catch (const exception&)
  {
  }
  return s.str();
}
