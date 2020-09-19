/* RepositoryListModel.cpp:

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

#include <miktex/Core/AutoResource>
#include <miktex/Core/Uri>
#include <miktex/PackageManager/PackageManager>

#include "RepositoryTableModel.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace std;

RepositoryInfo RandomRepository()
{
  RepositoryInfo rand;
  rand.url.clear();
  rand.type = RepositoryType::Remote;
  return rand;
}

RepositoryTableModel::RepositoryTableModel(QObject* parent) :
  QAbstractTableModel(parent)
{
  InternalReload();
}

int RepositoryTableModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : repositories.size();
}

int RepositoryTableModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 2;
}

QVariant RepositoryTableModel::data(const QModelIndex& index, int role) const
{
  if (!(index.isValid() && index.row() >= 0 && index.row() < repositories.size()))
  {
    return QVariant();
  }

  const RepositoryInfo& repositoryInfo = repositories[index.row()];

  if (role == Qt::DisplayRole)
  {
    switch (index.column())
    {
    case 0:
      if (repositoryInfo == RandomRepository())
      {
        return tr("a random package repository on the Internet");
      }
      else if (repositoryInfo.type == RepositoryType::Remote)
      {
        Uri uri(repositoryInfo.url);
        return QString::fromUtf8((uri.GetScheme() + "://" + uri.GetHost()).c_str());
      }
      else
      {
        return QString::fromUtf8(repositoryInfo.url.c_str());
      }
    case 1:
      return repositoryInfo.releaseState == RepositoryReleaseState::Next ? "NEXT" : tr("stable");
    }
  }
  else if (role == Qt::UserRole)
  {
    switch (index.column())
    {
    case 0:
      return QString::fromUtf8(repositoryInfo.url.c_str());
    case 1:
      return (int)repositoryInfo.releaseState;
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
      return tr("Url");
    case 1:
      return tr("Release State");
    }
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

void RepositoryTableModel::InternalReload()
{
  RepositoryInfo def = PackageManager::GetDefaultPackageRepository();
  RepositoryInfo random = RandomRepository();
  random.releaseState = def.releaseState;
  repositories.clear();
  repositories.push_back(def);
  if (def != random)
  {
    repositories.push_back(random);
  }
}

void RepositoryTableModel::Reload()
{
  beginResetModel();
  MIKTEX_AUTO(endResetModel());
  InternalReload();
}

int RepositoryTableModel::GetDefaultIndex()
{
  auto it = std::find(repositories.begin(), repositories.end(), PackageManager::GetDefaultPackageRepository());
  if (it == repositories.end())
  {
    it = std::find(repositories.begin(), repositories.end(), RandomRepository());
  }
  return it == repositories.end() ? -1 : std::distance(repositories.begin(), it);
}
