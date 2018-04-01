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
  return parent.isValid() ? 0 : 4;
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
    const UpdateTableModel::InternalUpdateInfo& update = updates[index.row()];
    switch (index.column())
    {
    case 0:
      return QString::fromUtf8(update.deploymentName.c_str());
    case 1:
    {
      PackageInfo oldPackageInfo;
      if (packageManager->TryGetPackageInfo(update.deploymentName, oldPackageInfo) && oldPackageInfo.timeInstalled > 0)
      {
        return FormatPackageVersion(oldPackageInfo);
      }
      break;
    }
    case 2:
      if (update.timePackaged > 0)
      {
        return FormatPackageVersion(update);
      }
      break;
    case 3:
      switch (update.action)
      {
      case PackageInstaller::UpdateInfo::Keep:
        return tr("update not possible");
      case PackageInstaller::UpdateInfo::KeepAdmin:
        return tr("update not possible in user mode");
      case PackageInstaller::UpdateInfo::KeepObsolete:
        return tr("removal not possible in user mode");
      case PackageInstaller::UpdateInfo::Update:
        return tr("optional");
      case PackageInstaller::UpdateInfo::ForceUpdate:
        return tr("update required");
      case PackageInstaller::UpdateInfo::ForceRemove:
        return tr("removal required");
      case PackageInstaller::UpdateInfo::Repair:
        return tr("to be repaired");
      case PackageInstaller::UpdateInfo::ReleaseStateChange:
        return tr("release state change");
      }
      break;
    case Qt::CheckStateRole:
      if (index.column() == 0)
      {
        return update.exclude ? Qt::Unchecked : Qt::Checked;
      }
    }
  }

  return QVariant();
}

bool UpdateTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (!(index.isValid() && index.row() >= 0 && index.row() < updates.size()))
  {
    return false;
  }
  UpdateTableModel::InternalUpdateInfo& update = updates[index.row()];
  if (role == Qt::CheckStateRole && index.column() == 0)
  {
    Qt::CheckState oldValue = update.exclude ? Qt::Unchecked : Qt::Checked;
    Qt::CheckState newValue = static_cast<Qt::CheckState>(value.toInt());
    if (newValue == Qt::Unchecked && !IsExcludable(index))
    {
      return false;
    }
    if (oldValue != newValue)
    {
      update.exclude = newValue == Qt::Unchecked;
      emit dataChanged(index, index);
      return true;
    }
  }
  return false;
}

Qt::ItemFlags UpdateTableModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags = QAbstractTableModel::flags(index);
  if (index.isValid() && index.column() == 0)
  {
    flags |= Qt::ItemIsUserCheckable;
  }
  return flags;
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
      return tr("Repository");
    case 3:
      return tr("Action");
    }
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

void UpdateTableModel::SetData(const vector<PackageInstaller::UpdateInfo>& updates)
{
  beginResetModel();
  this->updates.clear();
  for (const auto& u : updates)
  {
    this->updates.push_back(u);
  }
  endResetModel();
}

bool UpdateTableModel::IsExcludable(const QModelIndex& index) const
{
  const PackageInstaller::UpdateInfo& upd = updates[index.row()];
  switch (upd.action)
  {
  case PackageInstaller::UpdateInfo::KeepAdmin:
  case PackageInstaller::UpdateInfo::KeepObsolete:
  case PackageInstaller::UpdateInfo::Repair:
  case PackageInstaller::UpdateInfo::ReleaseStateChange:
  case PackageInstaller::UpdateInfo::ForceRemove:
  case PackageInstaller::UpdateInfo::ForceUpdate:
    return false;
  default:
    return true;
  }
}
