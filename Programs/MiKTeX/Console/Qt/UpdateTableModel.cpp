/* UpdateTableModel.cpp:

   Copyright (C) 2018-2020 Christian Schenk

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
  return parent.isValid() ? 0 : 5;
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

  const UpdateTableModel::InternalUpdateInfo& update = updates[index.row()];

  if (role == Qt::DisplayRole)
  {
    switch (index.column())
    {
    case 0:
      return QString::fromUtf8(update.packageId.c_str());
    case 1:
    {
      PackageInfo oldPackageInfo;
      if (packageManager->TryGetPackageInfo(update.packageId, oldPackageInfo) && oldPackageInfo.IsInstalled())
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
        return tr("");
      case PackageInstaller::UpdateInfo::KeepAdmin:
        return tr("install");
      case PackageInstaller::UpdateInfo::KeepObsolete:
        return tr("remove");
      case PackageInstaller::UpdateInfo::Update:
      case PackageInstaller::UpdateInfo::ForceUpdate:
      case PackageInstaller::UpdateInfo::Repair:
      case PackageInstaller::UpdateInfo::ReleaseStateChange:
        return tr("install");
      case PackageInstaller::UpdateInfo::ForceRemove:
        return tr("remove");
      default:
        break;
      }
      break;
    case 4:
      switch (update.action)
      {
      case PackageInstaller::UpdateInfo::Keep:
        return tr("update not possible");
      case PackageInstaller::UpdateInfo::KeepAdmin:
        return tr("update only possible in admin mode");
      case PackageInstaller::UpdateInfo::KeepObsolete:
        return tr("removal only possible in admin mode");
      case PackageInstaller::UpdateInfo::Update:
        return tr("optional");
      case PackageInstaller::UpdateInfo::ForceUpdate:
      case PackageInstaller::UpdateInfo::ForceRemove:
        return tr("required");
      case PackageInstaller::UpdateInfo::Repair:
        return tr("to be repaired");
      case PackageInstaller::UpdateInfo::ReleaseStateChange:
        return tr("release state change");
      default:
        break;
      }
      break;
    }
  }
  else if (role == Qt::CheckStateRole)
  {
    if (index.column() == 0)
    {
      return update.checked ? Qt::Checked : Qt::Unchecked;
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
    if (!IsCheckable(index))
    {
      return false;
    }
    Qt::CheckState oldValue = update.checked ? Qt::Checked : Qt::Unchecked;
    Qt::CheckState newValue = static_cast<Qt::CheckState>(value.toInt());
    if (oldValue != newValue)
    {
      update.checked = newValue == Qt::Checked;
      emit dataChanged(index, index, { role });
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
      return tr("Available");
    case 3:
      return tr("Action");
    case 4:
      return tr("");
    }
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

void UpdateTableModel::SetData(const vector<PackageInstaller::UpdateInfo>& updates)
{
  beginResetModel();
  this->updates.clear();
  this->pending = 0;
  for (const auto& u : updates)
  {
    this->updates.push_back(u);
    if (this->updates.back().checked)
    {
      this->pending += 1;
    }
  }
  endResetModel();
}

bool UpdateTableModel::IsCheckable(const QModelIndex& index) const
{
  switch (updates[index.row()].action)
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
