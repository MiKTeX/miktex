/* RootTableModel.cpp:

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

#include <miktex/Core/AutoResource>
#include <miktex/Util/StringUtil>

#include "RootTableModel.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

RootTableModel::RootTableModel(QObject* parent) :
  QAbstractTableModel(parent)
{
}

int RootTableModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : roots.size();
}

int RootTableModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 3;
}

vector<string> GetPurposesString(RootDirectoryInfo::Purposes purposes)
{
  vector<string> result;
  if (purposes[RootDirectoryInfo::Purpose::Generic])
  {
    result.push_back("Generic");
  }
  if (purposes[RootDirectoryInfo::Purpose::Config])
  {
    result.push_back("Config");
  }
  if (purposes[RootDirectoryInfo::Purpose::Data])
  {
    result.push_back("Data");
  }
  if (purposes[RootDirectoryInfo::Purpose::Install])
  {
    result.push_back("Install");
  }
  return result;
}

vector<string> GetAttributesString(RootDirectoryInfo::Attributes attributes)
{
  vector<string> result;
  if (attributes[RootDirectoryInfo::Attribute::Common])
  {
    result.push_back("Common");
  }
  if (attributes[RootDirectoryInfo::Attribute::Other])
  {
    result.push_back("Other");
  }
  if (attributes[RootDirectoryInfo::Attribute::User])
  {
    result.push_back("User");
  }
  return result;
}

QVariant RootTableModel::data(const QModelIndex& index, int role) const
{
  if (!(index.isValid() && index.row() >= 0 && index.row() < roots.size()))
  {
    return QVariant();
  }

  if (role == Qt::DisplayRole)
  {
    const RootDirectoryInfo& root = roots[index.row()];
    switch (index.column())
    {
    case 0:
      return QString::fromUtf8(root.path.ToDisplayString().c_str());
    case 1:
      return QString::fromUtf8(StringUtil::Flatten(GetPurposesString(root.purposes), ',').c_str());
    case 2:
      return QString::fromUtf8(StringUtil::Flatten(GetAttributesString(root.attributes), ',').c_str());
    }
  }

  return QVariant();
}

QVariant RootTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
  {
    switch (section)
    {
    case 0:
      return tr("Path");
    case 1:
      return tr("Purposes");
    case 2:
      return tr("Attributes");
    }
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

void RootTableModel::Reload()
{
  beginResetModel();
  MIKTEX_AUTO(endResetModel());
  roots = session->GetRootDirectories();
}

bool RootTableModel::CanMoveUp(const QModelIndex& index)
{
  const RootDirectoryInfo& root = roots[index.row()];
  bool canMoveUp = !root.IsManaged();
  canMoveUp = canMoveUp && (!session->IsAdminMode() || root.IsCommon());
  canMoveUp = canMoveUp && (session->IsAdminMode() || !root.IsCommon());
  canMoveUp = canMoveUp && index.row() > 0;
  canMoveUp = canMoveUp && !roots[index.row() - 1].IsManaged();
  return canMoveUp;
}

void RootTableModel::MoveUp(const QModelIndex& index)
{
  session->MoveRootDirectoryUp(index.row());
}

bool RootTableModel::CanMoveDown(const QModelIndex& index)
{
  const RootDirectoryInfo& root = roots[index.row()];
  bool canMoveDown = !root.IsManaged();
  canMoveDown = canMoveDown && (!session->IsAdminMode() || root.IsCommon());
  canMoveDown = canMoveDown && (session->IsAdminMode() || !root.IsCommon());
  canMoveDown = canMoveDown && index.row() < roots.size() - 1;
  canMoveDown = canMoveDown && !roots[index.row() + 1].IsManaged();
  return canMoveDown;
}

void RootTableModel::MoveDown(const QModelIndex& index)
{
  session->MoveRootDirectoryDown(index.row());
}

bool RootTableModel::CanRemove(const QModelIndex& index)
{
  const RootDirectoryInfo& root = roots[index.row()];
  bool canRemove = !root.IsManaged();
  canRemove = canRemove && (!session->IsAdminMode() || root.IsCommon());
  canRemove = canRemove && (session->IsAdminMode() || !root.IsCommon());
  return canRemove;
}

void RootTableModel::Remove(const QModelIndex& index)
{
  session->UnregisterRootDirectory(roots[index.row()].path, false);
}
