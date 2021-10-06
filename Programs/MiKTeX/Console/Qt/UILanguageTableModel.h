/* UILanguageTableModel.h:                                  -*- C++ -*-

   Copyright (C) 2020 Christian Schenk

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

#include <QAbstractTableModel>

#include <string>
#include <vector>

#include <miktex/Core/Session>

class UILanguageTableModel :
  public QAbstractTableModel
{
private:
  Q_OBJECT;

public:
  UILanguageTableModel(QObject* parent = nullptr);

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
  int GetDefaultIndex();

public:
  std::string GetData(int index) const
  {
    return uiLanguages[index].localeIdentifier;
  }

private:
  void InternalReload();

private:
  struct UILanguage
  {
    std::string displayName;
    std::string localeIdentifier;
  };

private:
  std::vector<UILanguage> uiLanguages;

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MIKTEX_SESSION();
};
