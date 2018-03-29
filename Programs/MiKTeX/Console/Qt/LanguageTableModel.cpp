/* LanguageTableModel.cpp:

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

#include <memory>

#include <miktex/Util/StringUtil>

#include "LanguageTableModel.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

LanguageTableModel::LanguageTableModel(QObject* parent) :
  QAbstractTableModel(parent)
{
}

int LanguageTableModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : languages.size();
}

int LanguageTableModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 2;
}

QVariant LanguageTableModel::data(const QModelIndex& index, int role) const
{
  if (!(index.isValid() && index.row() >= 0 && index.row() < languages.size()))
  {
    return QVariant();
  }

  if (role == Qt::DisplayRole)
  {
    const LanguageInfo& language = languages[index.row()];
    switch (index.column())
    {
    case 0:
      return QString::fromUtf8(language.key.c_str());
    case 1:
      return QString::fromUtf8(language.synonyms.c_str());
    }
  }

  return QVariant();
}

QVariant LanguageTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
  {
    switch (section)
    {
    case 0:
      return tr("Language");
    case 1:
      return tr("Synonyms");
    }
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

void LanguageTableModel::Reload()
{
  beginResetModel();
  languages = session->GetLanguages();
  endResetModel();
}
