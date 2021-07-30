/* mainwindow_documentation.cpp:

   Copyright (C) 2017-2021 Christian Schenk

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

#include <QDesktopServices>
#include <QMessageBox>
#include <QToolBar>
#include <QUrl>
#include <QWidget>

#include <miktex/Core/AutoResource>
#include <miktex/Core/Paths>
#include <miktex/UI/Qt/ErrorDialog>
#include <miktex/UI/Qt/UpdateDialog>

#include "ui_mainwindow.h"

#include "DocumentationPage.h"
#include "DocumentationTableModel.h"
#include "DocumentationProxyModel.h"

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::Qt;
using namespace MiKTeX::Util;

DocumentationPage::DocumentationPage(QWidget* parent, Ui::MainWindow* ui, ErrorReporter* errorReporter, BackgroundWorkerChecker* backgroundWorkerChecker, shared_ptr<Session> session, shared_ptr<PackageManager> packageManager) :
    backgroundWorkerChecker(backgroundWorkerChecker),
    errorReporter(errorReporter),
    packageManager(packageManager),
    parent(parent),
    session(session),
    ui(ui)
{
    toolBarDocumentation = new QToolBar(parent);
    toolBarDocumentation->setIconSize(QSize(16, 16));
    toolBarDocumentation->addAction(ui->actionViewDocument);
    toolBarDocumentation->addAction(ui->actionInstallDocumentation);
    toolBarDocumentation->addSeparator();
    lineEditDocumentationFilter = new QLineEdit(toolBarDocumentation);
    lineEditDocumentationFilter->setClearButtonEnabled(true);
    toolBarDocumentation->addWidget(lineEditDocumentationFilter);
    toolBarDocumentation->addAction(ui->actionFilterDocumentation);
    (void)connect(lineEditDocumentationFilter, SIGNAL(returnPressed()), this, SLOT(FilterDocumentation()));
    ui->hboxDocumentationToolBar->addWidget(toolBarDocumentation);
    ui->hboxDocumentationToolBar->addStretch();
    documentationModel = new DocumentationTableModel(session, packageManager, parent);
    documentationProxyModel = new DocumentationProxyModel(parent);
    documentationProxyModel->setSourceModel(documentationModel);
    documentationProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    ui->treeViewDocumentation->setModel(documentationProxyModel);
    ui->treeViewDocumentation->sortByColumn(0, Qt::AscendingOrder);
    contextMenuDocumentationBackground = new QMenu(ui->treeViewPackages);
    contextMenuDocumentationBackground->addAction(ui->actionUpdatePackageDatabase);
    contextMenuDocumentation = new QMenu(ui->treeViewDocumentation);
    contextMenuDocumentation->addAction(ui->actionViewDocument);
    contextMenuDocumentation->addAction(ui->actionInstallDocumentation);
    ui->treeViewDocumentation->setContextMenuPolicy(Qt::CustomContextMenu);
    (void)connect(ui->treeViewDocumentation, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnContextMenuDocumentation(const QPoint&)));
    (void)connect(ui->treeViewDocumentation->selectionModel(),
        SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this,
        SLOT(UpdateActions()));
    (void)connect(ui->actionFilterDocumentation,
        SIGNAL(triggered()),
        this,
        SLOT(FilterDocumentation()));
    (void)connect(ui->actionInstallDocumentation,
        SIGNAL(triggered()),
        this,
        SLOT(InstallDocumentation()));
    (void)connect(ui->actionViewDocument,
        SIGNAL(triggered()),
        this,
        SLOT(ViewDocument()));
    connect(ui->treeViewDocumentation, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(ViewDocument()));
}

void DocumentationPage::UpdateUi()
{
}

void DocumentationPage::UpdateActions()
{
    try
    {
        ui->actionFilterDocumentation->setEnabled(!backgroundWorkerChecker->IsBackgroundWorkerActive());
        QModelIndexList selectedRows = ui->treeViewDocumentation->selectionModel()->selectedRows();
        bool enableInstall = (selectedRows.count() > 0);
        if (session->IsMiKTeXDirect())
        {
            enableInstall = false;
        }
        for (QModelIndexList::const_iterator it = selectedRows.begin(); it != selectedRows.end() && enableInstall; ++it)
        {
            PackageInfo packageInfo;
            if (!documentationModel->TryGetPackageInfo(documentationProxyModel->mapToSource(*it), packageInfo))
            {
                MIKTEX_UNEXPECTED();
            }
            if (packageInfo.IsInstalled())
            {
                enableInstall = false;
            }
        }
        ui->actionInstallDocumentation->setEnabled(!backgroundWorkerChecker->IsBackgroundWorkerActive() && enableInstall);
        bool enableView = false;
        if (selectedRows.count() == 1)
        {
            PackageInfo packageInfo;
            if (!documentationModel->TryGetPackageInfo(documentationProxyModel->mapToSource(selectedRows[0]), packageInfo))
            {
                MIKTEX_UNEXPECTED();
            }
            enableView = packageInfo.IsInstalled();
        }
        ui->actionViewDocument->setEnabled(enableView);
    }
    catch (const MiKTeXException& e)
    {
        errorReporter->CriticalError(e);
    }
    catch (const exception& e)
    {
        errorReporter->CriticalError(e);
    }
}

void DocumentationPage::Activate()
{
    if (documentationModel->rowCount() == 0)
    {
        try
        {
            documentationModel->Reload();
        }
        catch (const MiKTeXException& e)
        {
            errorReporter->CriticalError(e);
        }
        catch (const exception& e)
        {
            errorReporter->CriticalError(e);
        }
    }
}

void DocumentationPage::FilterDocumentation()
{
    documentationProxyModel->SetFilter(lineEditDocumentationFilter->text().toUtf8().constData());
}

void DocumentationPage::InstallDocumentation()
{
  try
  {
    vector<string> toBeInstalled;
    for (const QModelIndex& ind : ui->treeViewDocumentation->selectionModel()->selectedRows())
    {
      PackageInfo packageInfo;
      if (!documentationModel->TryGetPackageInfo(documentationProxyModel->mapToSource(ind), packageInfo))
      {
        MIKTEX_UNEXPECTED();
      }
      else if (!packageInfo.IsInstalled())
      {
        toBeInstalled.push_back(packageInfo.id);
      }
    }
    QString message =
      tr("Your MiKTeX installation will now be updated:\n\n")
      + tr("%n package(s) will be installed\n", "", toBeInstalled.size());
    if (QMessageBox::Ok != QMessageBox::information(parent, TheNameOfTheGame, message, QMessageBox::Ok | QMessageBox::Cancel))
    {
      return;
    }
    MIKTEX_AUTO(ui->treeViewPackages->update());
    UpdateDialog::DoModal(parent, packageManager, toBeInstalled, {});
    documentationModel->Reload();
  }
  catch (const MiKTeXException& e)
  {
    ErrorDialog::DoModal(parent, e);
  }
  catch (const exception& e)
  {
    ErrorDialog::DoModal(parent, e);
  }
}

bool SkipPrefix(const string& str, const char* lpszPrefix, string& result)
{
  size_t n = StrLen(lpszPrefix);
  if (str.compare(0, n, lpszPrefix) != 0)
  {
    return false;
  }
  result = str.c_str() + n;
  return true;
}

bool SkipTeXMFPrefix(const string& str, string& result)
{
  return SkipPrefix(str, "texmf/", result)
    || SkipPrefix(str, "texmf\\", result)
    || SkipPrefix(str, "./texmf/", result)
    || SkipPrefix(str, ".\\texmf\\", result);
}

void DocumentationPage::ViewDocument()
{
    QModelIndexList selectedRows = ui->treeViewDocumentation->selectionModel()->selectedRows();
    if (selectedRows.count() != 1)
    {
        return;
    }
    PackageInfo packageInfo;
    if (!documentationModel->TryGetPackageInfo(documentationProxyModel->mapToSource(selectedRows[0]), packageInfo))
    {
        MIKTEX_UNEXPECTED();
    }
    string fileName = packageInfo.docFiles[0];
    string file;
    PathName path;
    if (SkipTeXMFPrefix(fileName, file) && session->FindFile(file, MIKTEX_PATH_TEXMF_PLACEHOLDER, path))
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(path.GetData()));
    }
}

void DocumentationPage::OnContextMenuDocumentation(const QPoint& pos)
{
    QModelIndex index = ui->treeViewDocumentation->indexAt(pos);
    if (index.isValid())
    {
        contextMenuDocumentation->exec(ui->treeViewDocumentation->mapToGlobal(pos));
    }
    else
    {
        contextMenuDocumentationBackground->exec(ui->treeViewDocumentation->mapToGlobal(pos));
    }
}
