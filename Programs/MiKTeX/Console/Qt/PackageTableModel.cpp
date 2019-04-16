/* PackageTableModel.cpp:

   Copyright (C) 2018-2019 Christian Schenk

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

#include <memory>

#include <QColor>
#include <QDateTime>
#include <QLocale>

#include "PackageTableModel.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;

PackageTableModel::PackageTableModel(std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager, QObject* parent) :
  QAbstractTableModel(parent),
  packageManager(packageManager)
{
}

int PackageTableModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : packages.size();
}

int PackageTableModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 8;
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
        return QString::fromUtf8(packageInfo.id.c_str());
      case 1:
        return QString::fromUtf8(packageManager->GetContainerPath(packageInfo.id, true).c_str());
      case 2:
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        return QLocale::system().formattedDataSize(packageInfo.GetSize());
#else
        return static_cast<qlonglong>(packageInfo.GetSize());
#endif
      case 3:
        return QDateTime::fromTime_t(packageInfo.timePackaged).date();
      case 4:
        if (IsValidTimeT(packageInfo.timeInstalled))
        {
          return QDateTime::fromTime_t(packageInfo.timeInstalled).date();
        }
        break;
      case 5:
        if (IsValidTimeT(packageInfo.timeInstalledByAdmin) && IsValidTimeT(packageInfo.timeInstalledByUser))
        {
          return "Admin, User";
        }
        else if (IsValidTimeT(packageInfo.timeInstalledByAdmin))
        {
          return "Admin";
        }
        else if (IsValidTimeT(packageInfo.timeInstalledByUser))
        {
          return "User";
        }
        break;
      case 6:
        return QString::fromUtf8(packageInfo.title.c_str());
      case 7:
        if (!packageInfo.runFiles.empty())
        {
          return QString("%1 +%2").arg(QString::fromUtf8(PathName(packageInfo.runFiles[0]).GetFileName().GetData())).arg(packageInfo.runFiles.size());
        }
        break;
      }
    }
  }
  else if (role == Qt::ForegroundRole)
  {
    PackageInfo packageInfo;
    if (TryGetPackageInfo(index, packageInfo) && IsValidTimeT(packageInfo.timeInstalledByAdmin) && IsValidTimeT(packageInfo.timeInstalledByUser))
    {
      return QColor("red");
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
      return tr("Installed by");
    case 6:
      return tr("Title");
    case 7:
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

bool PackageTableModel::TryGetPackageInfo(const QModelIndex& index, PackageInfo& packageInfo) const
{
  map<int, PackageInfo>::const_iterator it = packages.find(index.row());
  if (it == packages.end())
  {
    return false;
  }
  else
  {
    packageInfo = it->second;
    return true;
  }
}
