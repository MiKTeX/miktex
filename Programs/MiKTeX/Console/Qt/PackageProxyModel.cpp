/* PackageProxyModel.cpp:

   Copyright (C) 2018-2021 Christian Schenk

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
#include <miktex/Util/PathName>

#include "PackageProxyModel.h"
#include "PackageTableModel.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Util;

PackageProxyModel::PackageProxyModel(QObject* parent) :
  QSortFilterProxyModel(parent)
{
}

void PackageProxyModel::SetFilter(const string& filter)
{
  this->filterText = filter;
  invalidateFilter();
}

bool PackageProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
  if (filterText.empty())
  {
    return true;
  }
  PackageTableModel* packageTableModel = dynamic_cast<PackageTableModel*>(sourceModel());
  MIKTEX_ASSERT(packageTableModel != nullptr);
  PackageInfo packageInfo;
  if (!packageTableModel->TryGetPackageInfo(sourceModel()->index(sourceRow, 0, sourceParent), packageInfo))
  {
    return false;
  }
  bool accept = false;
  if (!accept)
  {
    accept = packageInfo.id.find(filterText) != string::npos;
  }
  if (!accept)
  {
    accept = packageInfo.title.find(filterText) != string::npos;
  }
  if (!accept)
  {
    for (const string& f : packageInfo.runFiles)
    {
      accept = PathName::Match(filterText.c_str(), PathName(f).RemoveDirectorySpec());
      if (accept)
      {
        break;
      }
    }
  }
  return accept;
}

bool PackageProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  MIKTEX_ASSERT(left.column() == right.column());
  PackageTableModel* packageTableModel = dynamic_cast<PackageTableModel*>(sourceModel());
  MIKTEX_ASSERT(packageTableModel != nullptr);
  PackageInfo packageInfoLeft;
  PackageInfo packageInfoRight;
  if (packageTableModel->TryGetPackageInfo(sourceModel()->index(left.row(), 0, left.parent()), packageInfoLeft)
    && packageTableModel->TryGetPackageInfo(sourceModel()->index(right.row(), 0, right.parent()), packageInfoRight))
  {
    switch (left.column())
    {
    case 2:
      return packageInfoLeft.GetSize() < packageInfoRight.GetSize();
    default:
      break;
    }
  }
  return QSortFilterProxyModel::lessThan(left, right);
}
