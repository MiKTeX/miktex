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
  QAbstractTableModel(parent)
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
      return "TODO";
    case 2:
      return QDateTime::fromTime_t(update.timePackaged).date().toString() + " " + QString::fromUtf8(update.version.c_str());
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
      return tr("Old");
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
