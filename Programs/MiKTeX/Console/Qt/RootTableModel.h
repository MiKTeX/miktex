/* RootTableModel.h:                                     -*- C++ -*-

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

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(FC4537A8F7134B64BFC4AEF045615338)
#define FC4537A8F7134B64BFC4AEF045615338

#include <QAbstractTableModel>

#include <memory>
#include <vector>

#include <miktex/Core/RootDirectoryInfo>
#include <miktex/Core/Session>

class RootTableModel :
  public QAbstractTableModel
{
private:
  Q_OBJECT;

public:
  RootTableModel(QObject* parent = nullptr);

public:
  int rowCount(const QModelIndex& parent) const override;

public:
  int columnCount(const QModelIndex& parent) const override;

public:
  QVariant data(const QModelIndex& index, int role) const override;

public:
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

public:
  void Reload();

public:
  bool CanRemove(const QModelIndex& index);

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();

private:
  std::vector<MiKTeX::Core::RootDirectoryInfo> roots;
};

#endif
