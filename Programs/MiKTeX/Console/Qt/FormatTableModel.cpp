/* FormatTableModel.cpp:

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

#include <miktex/Util/StringUtil>

#include "FormatTableModel.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

FormatTableModel::FormatTableModel(QObject* parent) :
  QAbstractTableModel(parent)
{
}

int FormatTableModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : formats.size();
}

int FormatTableModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 3;
}

vector<string> GetAttributeStrings(const FormatInfo& format)
{
  vector<string> result;
  if (format.exclude)
  {
    result.push_back("exclude");
  }
  return result;
}

QVariant FormatTableModel::data(const QModelIndex& index, int role) const
{
  if (!(index.isValid() && index.row() >= 0 && index.row() < formats.size()))
  {
    return QVariant();
  }
  if (role == Qt::DisplayRole)
  {
    const FormatInfo& format = formats[index.row()];
    switch (index.column())
    {
    case 0:
      return QString::fromUtf8(format.key.c_str());
    case 1:
      return QString::fromUtf8(format.description.c_str());
    case 2:
      return QString::fromUtf8(StringUtil::Flatten(GetAttributeStrings(format), ',').c_str());
    }
  }
  return QVariant();
}

QVariant FormatTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
  {
    switch (section)
    {
    case 0:
      return tr("Key");
    case 1:
      return tr("Description");
    case 2:
      return tr("Attributes");
    }
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

void FormatTableModel::Reload()
{
  beginResetModel();
  formats = session->GetFormats();
  endResetModel();
}

FormatInfo FormatTableModel::GetFormatInfo(const QModelIndex& index)
{
  return formats[index.row()];
}

bool FormatTableModel::CanRemove(const QModelIndex& index)
{
  return true;
}

bool FormatTableModel::removeRows(int row, int count, const QModelIndex& parent)
{
  beginRemoveRows(parent, row, row + count - 1);
  for (int idx = row; idx < row + count; ++idx)
  {
    session->DeleteFormatInfo(formats[idx].key);
  }
  formats.erase(formats.begin() + row, formats.begin() + row + count);
  endRemoveRows();
  return true;
}
