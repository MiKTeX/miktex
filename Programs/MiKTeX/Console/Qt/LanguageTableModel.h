/* LanguageTableModel.h:                                  -*- C++ -*-

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

#if !defined(A40E28F8FC79461492CB7F7D2B7BC9D8)
#define A40E28F8FC79461492CB7F7D2B7BC9D8

#include <QAbstractTableModel>

#include <memory>
#include <vector>

#include <miktex/Core/Session>

class LanguageTableModel :
  public QAbstractTableModel
{
private:
  Q_OBJECT;

public:
  LanguageTableModel(QObject* parent = nullptr);

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

private:
  std::vector<std::string> WhichPackage(const std::string& texInputfile);

private:
  void ReadLanguageDat();

public:
  void Reload();

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();

private:
  struct InternalLanguageInfo :
    public MiKTeX::Core::LanguageInfo
  {
    InternalLanguageInfo(const MiKTeX::Core::LanguageInfo& languageInfo) :
      LanguageInfo(languageInfo)
    {
    }
    bool loaderExists = false;
    MiKTeX::Util::PathName loaderPath;
    std::vector<std::string> packageNames;
  };

public:
  bool IsExcludable(const QModelIndex& index) const
  {
    return index.row() != 0;
  }

private:
  std::vector<InternalLanguageInfo> languages;
};

#endif
