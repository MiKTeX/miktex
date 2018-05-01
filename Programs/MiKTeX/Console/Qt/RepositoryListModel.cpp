/* RepositoryListModel.cpp:

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

#include <miktex/PackageManager/PackageManager>

#include "RepositoryListModel.h"

using namespace MiKTeX::Packages;
using namespace std;

QStringList RepositoryListModel::BuildRepositoryStringList(shared_ptr<PackageManager> packageManager)
{
  QStringList list;
  RepositoryType repositoryType;
  string repository;
  if (packageManager->TryGetDefaultPackageRepository(repositoryType, repository) && !repository.empty())
  {
    list.append(QString::fromUtf8(repository.c_str()));
  }
  list.append(tr("a random package repository on the Internet"));
  return list;
}

RepositoryListModel::RepositoryListModel(shared_ptr<PackageManager> packageManager, QObject* parent) :
  packageManager(packageManager),
  QStringListModel(BuildRepositoryStringList(packageManager), parent)
{
}

void RepositoryListModel::Reload()
{
  setStringList(BuildRepositoryStringList(packageManager));
}

int RepositoryListModel::GetCurrentIndex()
{
  string url;
  RepositoryType repositoryType;
  if (PackageManager::TryGetDefaultPackageRepository(repositoryType, url) && !url.empty())
  {
    return 0;
  }
  else
  {
    return rowCount() - 1;
  }
}
