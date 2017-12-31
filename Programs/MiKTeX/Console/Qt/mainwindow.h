/* mainwindow.h:                                        -*- C++ -*-

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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <miktex/Core/Session>
#include <miktex/PackageManager/PackageManager>

#include <memory>

#include <QMainWindow>

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT;

private slots:
  void on_buttonOverview_clicked();

private slots:
  void on_buttonPackages_clicked();

private slots:
  void on_buttonAdminSetup_clicked();

private slots:
  void on_buttonUserSetup_clicked();

private slots:
  void on_buttonUpgrade_clicked();

private slots:
  void EnableActions();

private slots:
  void AboutDialog();
  
private slots:
  void RestartAdmin();

public slots:
  void FinishSetup();

public:
  explicit MainWindow(QWidget* parent = nullptr);

public:
  ~MainWindow();

private:
  void RestartAdminWithArguments(const std::vector<std::string>& args);

private:
  void UpdateWidgets();

private:
  void SetCurrentPage(int idx);

private:
  bool isSetupMode = false;

private:
  Ui::MainWindow* ui;

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;
};

#endif
