/* ProxyAuthenticationDialog.cpp:

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

#include <QPushButton>

#include <miktex/Core/Session>

#include "internal.h"

#include "miktex/UI/Qt/ErrorDialog.h"

#include "ProxyAuthenticationDialog.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::UI::Qt;
using namespace std;

ProxyAuthenticationDialog::ProxyAuthenticationDialog(QWidget* parent) :
  QDialog(parent)
{
  setupUi(this);
#if 0
  QRegExp namePattern("\\w+");
  QValidator* validator = new QRegExpValidator(namePattern, this);
  leName->setValidator(validator);
#endif
  leName->setText("");
}

void ProxyAuthenticationDialog::on_leName_textChanged(const QString& newText)
{
  try
  {
    QPushButton* okayButton = buttonBox->button(QDialogButtonBox::Ok);
    if (okayButton == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    okayButton->setEnabled(newText.length() > 0);
  }
  catch (const MiKTeXException& e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception& e)
  {
    ErrorDialog::DoModal(this, e);
  }
}
