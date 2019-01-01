/* FileTableModel.cpp:

   Copyright (C) 2017-2018 Christian Schenk

   This file is part of the MiKTeX UI Library.

   The MiKTeX UI Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX UI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX UI Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "FileTableModel.h"

using namespace MiKTeX::Core;
using namespace std;

FileTableModel::FileTableModel(QObject* parent, const vector<string>& files) :
  QAbstractTableModel(parent)
{
  this->files = files;
}

int FileTableModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : static_cast<int>(files.size());
}

int FileTableModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 2;
}

QVariant FileTableModel::data(const QModelIndex& index, int role) const
{
  if (!(index.isValid() && index.row() >= 0 && index.row() < files.size()))
  {
    return QVariant();
  }

  if (role == Qt::DisplayRole)
  {
    PathName file(files[index.row()]);
    switch (index.column())
    {
    case 0:
      return QString::fromUtf8(file.RemoveDirectorySpec().GetData());
    case 1:
      return QString::fromUtf8(file.RemoveFileSpec().GetData());
    }
  }

  return QVariant();
}

QVariant FileTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
  {
    switch (section)
    {
    case 0:
      return tr("File name");
    case 1:
      return tr("Path");
    }
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}
