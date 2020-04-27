/* LanguageTableModel.cpp:

   Copyright (C) 2018-2020 Christian Schenk

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

#include <miktex/Core/AutoResource>
#include <miktex/Core/Fndb>
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
  return parent.isValid() ? 0 : 5;
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
    case 2:
      return tr("Loader");
    case 3:
      return tr("Package");
    case 4:
      return tr("Installed");
    }
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant LanguageTableModel::data(const QModelIndex& index, int role) const
{
  if (!(index.isValid() && index.row() >= 0 && index.row() < languages.size()))
  {
    return QVariant();
  }
  const InternalLanguageInfo& language = languages[index.row()];
  switch (role)
  {
  case Qt::DisplayRole:
  {
    switch (index.column())
    {
    case 0:
      return QString::fromUtf8(language.key.c_str());
    case 1:
      return QString::fromUtf8(language.synonyms.c_str());
    case 2:
      return QString::fromUtf8(language.loader.c_str());
    case 3:
      return QString::fromUtf8(StringUtil::Flatten(language.packageNames, ',').c_str());
    case 4:
      return language.loaderExists ? QString::fromUtf8(u8"\u2713") : "";
    }
    break;
  }
  case Qt::CheckStateRole:
    if (index.column() == 0)
    {
      return language.exclude ? Qt::Unchecked : Qt::Checked;
    }
  }
  return QVariant();
}

bool LanguageTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (!(index.isValid() && index.row() >= 0 && index.row() < languages.size()))
  {
    return false;
  }
  LanguageInfo& language = languages[index.row()];
  if (role == Qt::CheckStateRole && index.column() == 0)
  {
    Qt::CheckState oldValue = language.exclude ? Qt::Unchecked : Qt::Checked;
    Qt::CheckState newValue = static_cast<Qt::CheckState>(value.toInt());
    if (newValue == Qt::Unchecked && !IsExcludable(index))
    {
      return false;
    }
    if (oldValue != newValue)
    {
      language.exclude = newValue == Qt::Unchecked;
      session->SetLanguageInfo(language);
      emit dataChanged(index, index);
      return true;
    }
  }
  return false;
}

Qt::ItemFlags LanguageTableModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags = QAbstractTableModel::flags(index);
  if (index.isValid() && index.column() == 0)
  {
    flags |= Qt::ItemIsUserCheckable;
  }
  return flags;
}

vector<string> LanguageTableModel::WhichPackage(const string& texInputfile)
{
  PathName directoryPattern(session->GetMpmRootPath());
  directoryPattern /= "tex//";
  vector<Fndb::Record> records;
  vector<string> result;
  if (Fndb::Search(PathName(texInputfile), directoryPattern.ToString(), false, records))
  {
    for (const auto& r : records)
    {
      result.push_back(r.fileNameInfo);
    }
  }
  return result;
}

void LanguageTableModel::ReadLanguageDat()
{
  languages.clear();
  for (const LanguageInfo& l : session->GetLanguages())
  {
    InternalLanguageInfo languageInfo(l);
    languageInfo.loaderExists = session->FindFile(languageInfo.loader, ".;%r/tex//", languageInfo.loaderPath);
    languageInfo.packageNames = WhichPackage(languageInfo.loader);
    languages.push_back(languageInfo);
  }
}

void LanguageTableModel::Reload()
{
  beginResetModel();
  MIKTEX_AUTO(endResetModel());
  ReadLanguageDat();
}
