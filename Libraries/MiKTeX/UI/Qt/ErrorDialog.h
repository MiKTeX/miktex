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

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(F69294E4CE49426DA1B2DAD529AB9B74)
#define F69294E4CE49426DA1B2DAD529AB9B74

#include <exception>
#include <string>

#include <miktex/Core/Exceptions>

#include "ui_ErrorDialog.h"

class ErrorDialogImpl :
  public QDialog,
  private Ui::ErrorDialog
{
private:
  Q_OBJECT;

public:
  ErrorDialogImpl(QWidget* parent, const MiKTeX::Core::MiKTeXException& e);

public:
  ErrorDialogImpl(QWidget* parent, const std::exception& e);

private slots:
  void on_btnCopy_clicked();

private:
  std::string CreateReport();

private:
  MiKTeX::Core::MiKTeXException miktexException;

private:
  std::exception stdException;

private:
  bool isMiKTeXException;
};

#endif
