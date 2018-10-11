/* ConnectionSettingsDialog.h:                          -*- C++ -*-

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

#pragma once

#if !defined(C1B44147E5A740229CE88A97E1950EB7)
#define C1B44147E5A740229CE88A97E1950EB7

#include "ui_ConnectionSettingsDialog.h"

class ConnectionSettingsDialog :
  public QDialog,
  private Ui::ConnectionSettingsDialog
{
private:
  Q_OBJECT;

public:
  ConnectionSettingsDialog(QWidget* parent);

protected:
  void accept() override;
};

#endif
