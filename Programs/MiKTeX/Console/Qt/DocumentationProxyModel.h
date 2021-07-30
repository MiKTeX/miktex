/* DocumentationProxyModel.h:

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

#pragma once

#include <QSortFilterProxyModel>

class DocumentationProxyModel :
    public QSortFilterProxyModel
{
private:
    Q_OBJECT;

public:
    DocumentationProxyModel(QObject* parent = nullptr);

public:
    void SetFilter(const std::string& filter);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    std::string filterText;
};
