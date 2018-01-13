/* UpdateTableModel.cpp:

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

#include <memory>

#include <QDateTime>

#include <miktex/Util/StringUtil>

#include "UpdateTableModel.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Util;
using namespace std;

UpdateTableModel::UpdateTableModel(shared_ptr<PackageManager> packageManager, QObject* parent) :
  QAbstractTableModel(parent),
  packageManager(packageManager)
{
}

int UpdateTableModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : updates.size();
}

int UpdateTableModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 3;
}

template<typename T> QString FormatPackageVersion(const T& packageInfo)
{
  QString str = QString::fromUtf8(packageInfo.version.c_str());
  if (!str.isEmpty())
  {
    str += " / ";
  }
  return str + QDateTime::fromTime_t(packageInfo.timePackaged).date().toString();
}

QVariant UpdateTableModel::data(const QModelIndex& index, int role) const
{
  if (!(index.isValid() && index.row() >= 0 && index.row() < updates.size()))
  {
    return QVariant();
  }

  if (role == Qt::DisplayRole)
  {
    const PackageInstaller::UpdateInfo& update = updates[index.row()];
    switch (index.column())
    {
    case 0:
      return QString::fromUtf8(update.deploymentName.c_str());
    case 1:
    {
      PackageInfo oldPackageInfo;
      if (!packageManager->TryGetPackageInfo(update.deploymentName, oldPackageInfo))
      {
        return QVariant();
      }
      if (oldPackageInfo.timeInstalled > 0)
      {
        return FormatPackageVersion(oldPackageInfo);
      }
      return QVariant();
    }
    case 2:
      if (update.action == PackageInstaller::UpdateInfo::Repair)
      {
        return tr("to be repaired");
      }
      else if (update.action == PackageInstaller::UpdateInfo::ReleaseStateChange)
      {
        return tr("release state change");
      }
      else if (update.action == PackageInstaller::UpdateInfo::ForceRemove || update.action == PackageInstaller::UpdateInfo::KeepObsolete)
      {
        return tr("obsolete (to be removed)");
      }
      else
      {
        return FormatPackageVersion(update);
      }
    }
  }

  return QVariant();
}

QVariant UpdateTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
  {
    switch (section)
    {
    case 0:
      return tr("Name");
    case 1:
      return tr("Installed");
    case 2:
      return tr("New");
    }
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

void UpdateTableModel::SetData(const vector<PackageInstaller::UpdateInfo>& updates)
{
  beginResetModel();
  this->updates = updates;
  endResetModel();
}
