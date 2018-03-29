/* FormatTableModel.h:                                  -*- C++ -*-

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

#if !defined(FF2E9B2F13444E1CA317D41EE41C27A4)
#define FF2E9B2F13444E1CA317D41EE41C27A4

#include <QAbstractTableModel>

#include <memory>
#include <vector>

#include <miktex/Core/Session>

class FormatTableModel :
  public QAbstractTableModel
{
private:
  Q_OBJECT;

public:
  FormatTableModel(QObject* parent = nullptr);

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
  bool CanRemove(const QModelIndex& index);

public:
  void Remove(const QModelIndex& index);

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();

private:
  std::vector<MiKTeX::Core::FormatInfo> formats;
};

#endif
