/* RepositoryTableModel.cpp:

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

#include "StdAfx.h"

#include "RepositoryTableModel.h"

using namespace MiKTeX::Packages;
using namespace std;

RepositoryTableModel::RepositoryTableModel(QObject * pParent) :
  QAbstractTableModel(pParent)
{
}

int RepositoryTableModel::rowCount(const QModelIndex & parent) const
{
  return parent.isValid() ? 0 : repositories.size();
}

int RepositoryTableModel::columnCount(const QModelIndex & parent) const
{
  return parent.isValid() ? 0 : 5;
}

QVariant RepositoryTableModel::data(const QModelIndex & index, int role) const
{
  if (!(index.isValid() && index.row() >= 0 && index.row() < repositories.size()))
  {
    return QVariant();
  }

  if (role == Qt::DisplayRole)
  {
    const RepositoryInfo & repositoryInfo = repositories[index.row()];
    switch (index.column())
    {
    case 0:
      return QString::fromUtf8(repositoryInfo.country.c_str());
    case 1:
    {
      QUrl url(QString::fromUtf8(repositoryInfo.url.c_str()));
      return url.scheme().toUpper();
    }
    case 2:
    {
      QUrl url(QString::fromUtf8(repositoryInfo.url.c_str()));
      return url.host();
    }
    case 3:
      return QDateTime::fromTime_t(repositoryInfo.timeDate).date();
    case 4:
      return QString::fromUtf8(repositoryInfo.description.c_str());
    }
  }

  return QVariant();
}

QVariant RepositoryTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
  {
    switch (section)
    {
    case 0:
      return tr("Country");
    case 1:
      return tr("Protocol");
    case 2:
      return tr("Host");
    case 3:
      return tr("Version");
    case 4:
      return tr("Description");
    }
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}
