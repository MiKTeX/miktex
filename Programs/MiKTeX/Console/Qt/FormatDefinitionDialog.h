/* FormatDefinitionDialog.h:                            -*- C++ -*-

   Copyright (C) 2018 Christian Schenk

   This file is part of MiKTeX Console.

   MiKTeX Console is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX Console is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Console; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#pragma once

#if !defined(BEF9440D13F941EFAD2F6F25C11F17CB)
#define BEF9440D13F941EFAD2F6F25C11F17CB

#include <miktex/Core/Session>
#include <QDialog>

namespace Ui
{
  class FormatDefinitionDialog;
}

class FormatDefinitionDialog :
  public QDialog
{
private:
  Q_OBJECT;

public:
  explicit FormatDefinitionDialog(QWidget* parent);

public:
  explicit FormatDefinitionDialog(QWidget* parent, const MiKTeX::Core::FormatInfo& format);

public:
  ~FormatDefinitionDialog();

public:
  void done(int r) override;

public:
  MiKTeX::Core::FormatInfo GetFormatInfo() const;

private:
  Ui::FormatDefinitionDialog* ui;

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();
};

#endif
