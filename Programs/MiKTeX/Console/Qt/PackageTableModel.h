/* PackageTableModel.h:                                 -*- C++ -*-

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

#pragma once

#if !defined(A767D31C530F42158B96C0AF14BBF92B)
#define A767D31C530F42158B96C0AF14BBF92B

#include <miktex/PackageManager/PackageManager>
#include <QAbstractTableModel>

class PackageTableModel :
  public QAbstractTableModel
{
private:
  Q_OBJECT;

public:
  PackageTableModel(std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager, QObject* parent = nullptr);

public:
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

public:
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

public:
  QVariant data(const QModelIndex& index, int role) const override;

public:
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

public:
  void Reload();

public:
  bool TryGetPackageInfo(const QModelIndex& index, MiKTeX::Packages::PackageInfo& packageInfo) const;

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;

private:
  std::map<int, MiKTeX::Packages::PackageInfo> packages;

public:
  const std::map<int, MiKTeX::Packages::PackageInfo>& GetData() const
  {
    return packages;
  }
};

#endif
