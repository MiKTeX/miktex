/* RepositoryTableModel.cpp:

   Copyright (C) 2008-2018 Christian Schenk

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

#include <QDateTime>
#include <QUrl>

#include "RepositoryTableModel.h"

using namespace MiKTeX::Packages;
using namespace std;

RepositoryTableModel::RepositoryTableModel(QObject* parent) :
  QAbstractTableModel(parent)
{
}

int RepositoryTableModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : repositories.size();
}

int RepositoryTableModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 6;
}

QVariant RepositoryTableModel::data(const QModelIndex& index, int role) const
{
  if (!(index.isValid() && index.row() >= 0 && index.row() < repositories.size()))
  {
    return QVariant();
  }

  if (role == Qt::DisplayRole)
  {
    const RepositoryInfo& repositoryInfo = repositories[index.row()];
    switch (index.column())
    {
    case 0:
      return repositoryInfo.ranking;
    case 1:
      return QString::fromUtf8(repositoryInfo.country.c_str());
    case 2:
    {
      QUrl url(QString::fromUtf8(repositoryInfo.url.c_str()));
      return url.scheme().toUpper();
    }
    case 3:
    {
      QUrl url(QString::fromUtf8(repositoryInfo.url.c_str()));
      return url.host();
    }
    case 4:
      return QDateTime::fromTime_t(repositoryInfo.timeDate).date();
    case 5:
      if (repositoryInfo.dataTransferRate != 0.0)
      {
        return repositoryInfo.dataTransferRate / 125000.0;
      }
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
      return tr("Ranking");
    case 1:
      return tr("Country");
    case 2:
      return tr("Protocol");
    case 3:
      return tr("Host");
    case 4:
      return tr("Date");
    case 5:
      return tr("Mbit/s");
    }
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}
