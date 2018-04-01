/* UpdateTableModel.h:                                  -*- C++ -*-

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

#if !defined(CD58483D25BC46209CBD09828620E255)
#define CD58483D25BC46209CBD09828620E255

#include <QAbstractTableModel>

#include <memory>
#include <vector>

#include <miktex/PackageManager/PackageManager>
#include <miktex/Core/Session>

class UpdateTableModel :
  public QAbstractTableModel
{
private:
  Q_OBJECT;

public:
  UpdateTableModel(std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager, QObject* parent = nullptr);

public:
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

public:
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

public:
  QVariant data(const QModelIndex& index, int role) const override;

public:
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

public:
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

public:
  Qt::ItemFlags flags(const QModelIndex& index) const override;

public:
  void SetData(const std::vector<MiKTeX::Packages::PackageInstaller::UpdateInfo>& updates);

public:
  std::vector<MiKTeX::Packages::PackageInstaller::UpdateInfo> GetData() const
  {
    std::vector<MiKTeX::Packages::PackageInstaller::UpdateInfo> result;
    for(const auto& u : updates)
    {
      if (!u.exclude)
      {
        result.push_back(u);
      }
    }
    return result;
  }

public:
  bool IsExcludable(const QModelIndex& index) const;

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();

private:
  struct InternalUpdateInfo :
    public MiKTeX::Packages::PackageInstaller::UpdateInfo
  {
    InternalUpdateInfo(const MiKTeX::Packages::PackageInstaller::UpdateInfo& upd)
      : UpdateInfo(upd)
    {
    }
    bool exclude = false;
  };

private:
  std::vector<InternalUpdateInfo> updates;
};

#endif
