/* PackageTableModel.cpp:

   Copyright (C) 2008-2017 Christian Schenk

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

#include <memory>

#include <QDateTime>

#include "PackageTableModel.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace std;

PackageTableModel::PackageTableModel(std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager, QObject* parent) :
  QAbstractTableModel(parent),
  packageManager(packageManager)
{
  Reload();
}

int PackageTableModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : packages.size();
}

int PackageTableModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 7;
}

QVariant PackageTableModel::data(const QModelIndex& index, int role) const
{
  if (!(index.isValid() && index.row() >= 0 && index.row() < packages.size()))
  {
    return QVariant();
  }

  if (role == Qt::DisplayRole)
  {
    PackageInfo packageInfo;
    if (TryGetPackageInfo(index, packageInfo))
    {
      switch (index.column())
      {
      case 0:
        return packageInfo.deploymentName.c_str();
      case 1:
        return packageManager->GetContainerPath(packageInfo.deploymentName, true).c_str();
      case 2:
        return (qlonglong)packageInfo.GetSize();
      case 3:
        return QDateTime::fromTime_t(packageInfo.timePackaged).date();
      case 4:
        if (packageInfo.timeInstalled == static_cast<time_t>(0))
        {
          return "";
        }
        else
        {
          return QDateTime::fromTime_t(packageInfo.timeInstalled).date();
        }
      case 5:
        return packageInfo.title.c_str();
      case 6:
        if (!packageInfo.runFiles.empty())
        {
          return QString("%1 +%2").arg(QString::fromUtf8(PathName(packageInfo.runFiles[0]).GetFileName().GetData())).arg(packageInfo.runFiles.size());
        }
      }
    }
  }

  return QVariant();
}

QVariant PackageTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
  {
    switch (section)
    {
    case 0:
      return tr("Name");
    case 1:
      return tr("Category");
    case 2:
      return tr("Size");
    case 3:
      return tr("Packaged on");
    case 4:
      return tr("Installed on");
    case 5:
      return tr("Title");
    case 6:
      return tr("Files");
    }
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

void PackageTableModel::Reload()
{
  beginResetModel();
  packages.clear();
  packageManager->UnloadDatabase();
  unique_ptr<PackageIterator> iter(packageManager->CreateIterator());
  PackageInfo packageInfo;
  int row = 0;
  while (iter->GetNext(packageInfo))
  {
    if (!packageInfo.IsPureContainer())
    {
      packages[row] = packageInfo;
      ++row;
    }
  }
  iter->Dispose();
  endResetModel();
}
