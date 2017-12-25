/* mainwindow.cpp:

   Copyright (C) 2017 Christian Schenk

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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <miktex/Core/PathName>
#include <miktex/Core/Session>

using namespace MiKTeX::Core;

MainWindow::MainWindow(QWidget* parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  if (session->IsAdminMode())
  {
    ui->userMode->hide();
  }
  else
  {
    ui->adminMode->hide();
  }
  if (Utils::CheckPath(false))
  {
    ui->hintPath->hide();
  }
  ui->bindir->setText(QString::fromUtf8(session->GetSpecialPath(SpecialPath::LocalBinDirectory).GetData()));
  ui->installdir->setText(QString::fromUtf8(session->GetSpecialPath(SpecialPath::InstallRoot).GetData()));
}

MainWindow::~MainWindow()
{
  delete ui;
}
