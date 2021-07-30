/* DocumentationPage.h:

   Copyright (C) 2021 Christian Schenk

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

#include <QObject>

#include <miktex/Core/Session>
#include <miktex/PackageManager/PackageManager>

#include "common.h"

class QLineEdit;
class QMenu;
class QPoint;
class QToolBar;
class QWidget;

class DocumentationProxyModel;
class DocumentationTableModel;

namespace Ui
{
    class MainWindow;
}

class DocumentationPage :
    public QObject
{
private:
    Q_OBJECT;

public:
    DocumentationPage(QWidget* parent, Ui::MainWindow* ui, ErrorReporter* errorReporter, BackgroundWorkerChecker* backgroundWorkerChecker, std::shared_ptr<MiKTeX::Core::Session> session, std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager);

public:
    void UpdateUi();

public slots:
    void UpdateActions();

public:
    void Activate();

private slots:
    void FilterDocumentation();

private slots:
    void InstallDocumentation();

private slots:
    void ViewDocument();

private:
    QMenu* contextMenuDocumentation = nullptr;

private:
    QMenu* contextMenuDocumentationBackground = nullptr;

private slots:
    void OnContextMenuDocumentation(const QPoint& pos);

private:
    BackgroundWorkerChecker* backgroundWorkerChecker = nullptr;
    DocumentationTableModel* documentationModel = nullptr;
    DocumentationProxyModel* documentationProxyModel = nullptr;
    ErrorReporter* errorReporter = nullptr;
    QLineEdit* lineEditDocumentationFilter = nullptr;
    std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;
    QWidget* parent = nullptr;
    std::shared_ptr<MiKTeX::Core::Session> session;
    QToolBar* toolBarDocumentation = nullptr;
    Ui::MainWindow* ui = nullptr;
};
