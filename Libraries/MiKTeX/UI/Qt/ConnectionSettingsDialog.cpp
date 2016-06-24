/* ConnectionSettingsDialog.cpp:

   Copyright (C) 2008-2016 Christian Schenk

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

#include "ConnectionSettingsDialog.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::Qt;
using namespace std;

ConnectionSettingsDialog::ConnectionSettingsDialog(QWidget * pParent) :
  QDialog(pParent)
{
  setupUi(this);
  leAddress->setValidator(new QRegExpValidator(QRegExp(".+"), this));
  leAddress->setText("127.0.0.1");
  lePort->setValidator(new QIntValidator(1, 65535, this));
  lePort->setText("8080");
  try
  {
    ProxySettings proxySettings;
    if (PackageManager::TryGetProxy(proxySettings))
    {
      grpUseProxy->setChecked(proxySettings.useProxy);
      leAddress->setText(QString::fromUtf8(proxySettings.proxy.c_str()));
      lePort->setText(QString::number(proxySettings.port));
      chkAuthRequired->setChecked(proxySettings.authenticationRequired);
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(0, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(0, e);
  }
}

void ConnectionSettingsDialog::accept()
{
  QDialog::accept();
  try
  {
    ProxySettings proxySettings;
    proxySettings.useProxy = grpUseProxy->isChecked();
    proxySettings.proxy = leAddress->text().toUtf8().constData();
    proxySettings.port = lePort->text().toInt();
    proxySettings.authenticationRequired = chkAuthRequired->isChecked();
    PackageManager::SetProxy(proxySettings);
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
}
