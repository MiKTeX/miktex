/* DocumentationProxyModel.cpp:

   Copyright (C) 2021 Christian Schenk

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

#include "DocumentationProxyModel.h"
#include "DocumentationTableModel.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Util;

DocumentationProxyModel::DocumentationProxyModel(QObject* parent) :
    QSortFilterProxyModel(parent)
{
}

void DocumentationProxyModel::SetFilter(const string& filter)
{
    this->filterText = filter;
    invalidateFilter();
}

bool DocumentationProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (filterText.empty())
    {
        return true;
    }
    DocumentationTableModel* documentationTableModel = dynamic_cast<DocumentationTableModel*>(sourceModel());
    MIKTEX_ASSERT(documentationTableModel != nullptr);
    PackageInfo packageInfo;
    if (!documentationTableModel->TryGetPackageInfo(sourceModel()->index(sourceRow, 0, sourceParent), packageInfo))
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
        for (const string& f : packageInfo.docFiles)
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

bool DocumentationProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    return QSortFilterProxyModel::lessThan(left, right);
}
