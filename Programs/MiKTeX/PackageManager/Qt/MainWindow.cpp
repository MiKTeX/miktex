/* MainWindow.cpp:

   Copyright (C) 2008-2016 Christian Schenk

   This file is part of MiKTeX Package Manager.

   MiKTeX Package Manager is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Package Manager is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Package Manager; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(__MINGW32__)
#define _WIN32_WINNT 0x0600
#endif

#include <QtWidgets>
#include <QProgressDialog>

#include <memory>
#include <thread>

#include <miktex/Core/Debug>
#include <miktex/Core/Exceptions>
#include <miktex/UI/Qt/ErrorDialog>
#include <miktex/UI/Qt/SiteWizSheet>
#include <miktex/UI/Qt/UpdateDialog>

#if defined(MIKTEX_WINDOWS)
#include <miktex/Core/win/WindowsVersion.h>
#include <miktex/Core/win/DllProc>
#include <commctrl.h>
#endif

#include "MainWindow.h"
#include "PackageTableModel.h"

#include "mpm-version.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::Qt;
using namespace std;

void MainWindow::Unimplemented()
{
  QMessageBox::information(this, tr("MiKTeX Package Manager"), tr("This is an unimplemented feature."));
}

MainWindow::MainWindow() :
  pManager(PackageManager::Create())
{
  setupUi(this);
  if (pSession->IsAdminMode())
  {
    setWindowTitle(windowTitle() + " (Admin)");
  }
  pModel = new PackageTableModel(pManager, this);
  pProxyModel = new QSortFilterProxyModel(this);
  pProxyModel->setSourceModel(pModel);
  pProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
  treeView->setModel(pProxyModel);
  treeView->sortByColumn(0, Qt::AscendingOrder);

  connect(treeView->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    this,
    SLOT(EnableActions()));
  connect(actionProperties,
    SIGNAL(triggered()),
    this,
    SLOT(PropertyDialog()));
  connect(actionSelectInstallablePackages,
    SIGNAL(triggered()),
    this,
    SLOT(SelectInstallablePackages()));
  connect(actionInstall,
    SIGNAL(triggered()),
    this,
    SLOT(Install()));
  connect(actionUninstall,
    SIGNAL(triggered()),
    this,
    SLOT(Uninstall()));
  connect(actionChangeRepository,
    SIGNAL(triggered()),
    this,
    SLOT(RepositoryWizard()));
  connect(actionSynchronize,
    SIGNAL(triggered()),
    this,
    SLOT(Synchronize()));
  connect(actionAbout,
    SIGNAL(triggered()), this,
    SLOT(AboutDialog()));

  EnableActions();
}

void MainWindow::EnableActions()
{
  try
  {
    QModelIndexList selectedRows = treeView->selectionModel()->selectedRows();
    actionProperties->setEnabled(selectedRows.count() == 1);
    bool enableInstall = (selectedRows.count() > 0);
    bool enableUninstall = (selectedRows.count() > 0);
    if (pSession->IsMiKTeXDirect())
    {
      enableInstall = false;
      enableUninstall = false;
    }
    for (QModelIndexList::const_iterator it = selectedRows.begin(); it != selectedRows.end() && (enableInstall || enableUninstall); ++it)
    {
      PackageInfo packageInfo;
      if (!pModel->TryGetPackageInfo(pProxyModel->mapToSource(*it), packageInfo))
      {
        MIKTEX_UNEXPECTED();
      }
      if (packageInfo.timeInstalled > 0)
      {
        enableInstall = false;
        enableUninstall = (enableUninstall && packageInfo.isRemovable);
      }
      else
      {
        enableUninstall = false;
      }
    }
    actionInstall->setEnabled(enableInstall);
    actionUninstall->setEnabled(enableUninstall);
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

void MainWindow::PropertyDialog()
{
  Unimplemented();
}

void MainWindow::SelectInstallablePackages()
{
  Unimplemented();
}

void MainWindow::Install()
{
  try
  {
    vector<string> toBeInstalled;
    vector<string> toBeRemoved;
    QModelIndexList selectedRows = treeView->selectionModel()->selectedRows();
    for (QModelIndexList::const_iterator it = selectedRows.begin(); it != selectedRows.end(); ++it)
    {
      PackageInfo packageInfo;
      if (!pModel->TryGetPackageInfo(pProxyModel->mapToSource(*it), packageInfo))
      {
        MIKTEX_UNEXPECTED();
      }
      else if (packageInfo.timeInstalled == 0)
      {
        toBeInstalled.push_back(packageInfo.deploymentName);
      }
      else
      {
        toBeRemoved.push_back(packageInfo.deploymentName);
      }
    }
    QString message =
      tr("Your MiKTeX installation will now be updated:\n\n")
      + tr("%n package(s) will be installed\n", "", toBeInstalled.size())
      + tr("%n package(s) will be removed", "", toBeRemoved.size());
#if defined(MIKTEX_WINDOWS) && _WIN32_WINNT >= _WIN32_WINNT_VISTA
    if (WindowsVersion::IsWindowsVistaOrGreater() && pSession->IsAdminMode())
    {
      DllProc4<HRESULT, const TASKDIALOGCONFIG *, int *, int *, BOOL *> taskDialogIndirect("comctl32.dll", "TaskDialogIndirect");
      TASKDIALOGCONFIG taskDialogConfig;
      memset(&taskDialogConfig, 0, sizeof(taskDialogConfig));
      taskDialogConfig.cbSize = sizeof(TASKDIALOGCONFIG);
      taskDialogConfig.hwndParent = nullptr;
      taskDialogConfig.hInstance = nullptr;
      taskDialogConfig.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION;
      taskDialogConfig.pszMainIcon = TD_SHIELD_ICON;
      taskDialogConfig.pszWindowTitle = L"MiKTeX Package Manager";
      taskDialogConfig.pszMainInstruction = L"Do you want to proceed?";
      taskDialogConfig.pszContent = reinterpret_cast<PCWSTR>(message.utf16());
      taskDialogConfig.cButtons = 2;
      TASKDIALOG_BUTTON const buttons[] = {
        {IDOK, L"Proceed"},
        {IDCANCEL, L"Cancel"}
      };
      taskDialogConfig.pButtons = buttons;
      taskDialogConfig.nDefaultButton = IDOK;
      int result = 0;
      if (SUCCEEDED(taskDialogIndirect(&taskDialogConfig, &result, nullptr, nullptr)))
      {
        if (IDOK != result)
        {
          return;
        }
      }
      else
      {
        MIKTEX_UNEXPECTED();
      }
    }
    else
#endif
    {
      if (QMessageBox::Ok != QMessageBox::information(this, "MiKTeX Package Manager", message, QMessageBox::Ok | QMessageBox::Cancel))
      {
        return;
      }
    }
    int ret = UpdateDialog::DoModal(this, pManager, toBeInstalled, toBeRemoved);
    if (ret == QDialog::Accepted)
    {
      pModel->Reload();
      treeView->update();
    }
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

void MainWindow::Uninstall()
{
  Install();
}

void MainWindow::RepositoryWizard()
{
  try
  {
    if (SiteWizSheet::DoModal(this) == QDialog::Accepted)
    {
      emit Synchronize();
    }
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

void MainWindow::Synchronize()
{
  try
  {
    unique_ptr<PackageInstaller> pInstaller(pManager->CreateInstaller());
    pInstaller->UpdateDbAsync();
    int numSteps = 10;
    QProgressDialog progress(tr("Synchronizing the package database..."), tr("Cancel"), 0, numSteps, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(1000);
    for (int step = 0; !progress.wasCanceled(); ++step)
    {
      if (step < numSteps)
      {
        progress.setValue(step);
      }
      PackageInstaller::ProgressInfo progressinfo = pInstaller->GetProgressInfo();
      if (progressinfo.ready)
      {
        break;
      }
      this_thread::sleep_for(chrono::milliseconds(1000));
    }
    pInstaller->Dispose();
    pModel->Reload();
    treeView->update();
    if (!progress.wasCanceled())
    {
      progress.setValue(numSteps);
    }
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

void MainWindow::AboutDialog()
{
  QString message;
  message = tr("MiKTeX Package Manager");
  message += " ";
  message += MIKTEX_COMPONENT_VERSION_STR;
  message += "\n\n";
  message += tr("MiKTeX Package Manager is free software. You are welcome to redistribute it under certain conditions. See the help file for more information.\n\nMiKTeX Package Manager comes WITH ABSOLUTELY NO WARRANTY OF ANY KIND.");
  QMessageBox::about(this, tr("MiKTeX Package Manager"), message);
}
