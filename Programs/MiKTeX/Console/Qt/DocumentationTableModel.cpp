/* DocumentationTableModel.cpp:

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

#include <memory>

#include <QColor>
#include <QDateTime>
#include <QLocale>

#include <miktex/Core/AutoResource>

#include "DocumentationTableModel.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Util;

struct DocumentSorter
{
    DocumentSorter(const std::string& packageId) : packageId(packageId) {}
    bool operator() (const std::string& l, const std::string& r)
    {
        MiKTeX::Util::PathName left(l);
        MiKTeX::Util::PathName right(r);
        auto leftExt = left.GetExtension();
        auto rightExt = right.GetExtension();
        for (const auto& e : preferredExtensions)
        {
            if (leftExt == e && rightExt != e)
            {
                return true;
            }
            if (leftExt != e && rightExt == e)
            {
                return false;
            }
        }
        auto leftName = left.GetFileNameWithoutExtension();
        auto rightName = right.GetFileNameWithoutExtension();
        if (leftName.ToString() == packageId && rightName.ToString() != packageId)
        {
            return true;
        }
        if (leftName.ToString() != packageId && rightName.ToString() == packageId)
        {
            return false;
        }
        return leftName < rightName;
    }
private:
    const std::vector<std::string> preferredExtensions = { ".pdf", ".html", ".txt" };
    std::string packageId;
};

DocumentationTableModel::DocumentationTableModel(shared_ptr<Session> session, shared_ptr<PackageManager> packageManager, QObject* parent) :
    QAbstractTableModel(parent),
    packageManager(packageManager),
    session(session)
{
}

int DocumentationTableModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : packages.size();
}

int DocumentationTableModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : 4;
}

QVariant DocumentationTableModel::data(const QModelIndex& index, int role) const
{
    if (!(index.isValid() && index.row() >= 0 && index.row() < packages.size()))
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        PackageInfo packageInfo;
        if (TryGetPackageInfo(index, packageInfo))
        {
            switch (index.column())
            {
            case 0:
                return QString::fromUtf8(packageInfo.id.c_str());
            case 1:
                if (!packageInfo.docFiles.empty())
                {
                    return QString::fromUtf8(PathName(packageInfo.docFiles[0]).GetFileName().GetData());
                }
                break;
            case 2:
                if (packageInfo.IsInstalled())
                {
                    return QString::fromUtf8(u8"\u2713");
                }
                break;
            case 3:
                return QString::fromUtf8(packageInfo.title.c_str());
            }
        }
    }

    return QVariant();
}

QVariant DocumentationTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case 0:
            return tr("Package");
        case 1:
            return tr("Document");
        case 2:
            return tr("Installed");
        case 3:
            return tr("Title");
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

void DocumentationTableModel::Reload()
{
    beginResetModel();
    MIKTEX_AUTO(endResetModel());
    packages.clear();
    packageManager->UnloadDatabase();
    unique_ptr<PackageIterator> iter(packageManager->CreateIterator());
    PackageInfo packageInfo;
    int row = 0;
    while (iter->GetNext(packageInfo))
    {
        if (!packageInfo.IsPureContainer() && packageInfo.docFiles.size() > 0)
        {
            std::sort(packageInfo.docFiles.begin(), packageInfo.docFiles.end(), DocumentSorter(packageInfo.id));
            packages[row] = packageInfo;
            ++row;
        }
    }
    iter->Dispose();
}

bool DocumentationTableModel::TryGetPackageInfo(const QModelIndex& index, PackageInfo& packageInfo) const
{
    map<int, PackageInfo>::const_iterator it = packages.find(index.row());
    if (it == packages.end())
    {
        return false;
    }
    else
    {
        packageInfo = it->second;
        return true;
    }
}
