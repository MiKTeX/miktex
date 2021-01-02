/* UILanguageTableModel.cpp:

   Copyright (C) 2020-2021 Christian Schenk

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

#include <config.h>

#include <QLocale>

#include <memory>

#include <miktex/Configuration/ConfigNames>
#include <miktex/Core/AutoResource>

#include "UILanguageTableModel.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

UILanguageTableModel::UILanguageTableModel(QObject* parent) :
  QAbstractTableModel(parent)
{
  InternalReload();
}

int UILanguageTableModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : uiLanguages.size();
}

int UILanguageTableModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 2;
}

QVariant UILanguageTableModel::data(const QModelIndex& index, int role) const
{
  if (!(index.isValid() && index.row() >= 0 && index.row() < uiLanguages.size()))
  {
    return QVariant();
  }

  const UILanguage& uiLanguage = uiLanguages[index.row()];

  if (role == Qt::DisplayRole)
  {
    switch (index.column())
    {
    case 0:
      if (uiLanguage.displayName == "")
      {
        return tr("<System Language>");
      }
      else
      {
        return QString::fromStdString(uiLanguage.displayName);
      }
    case 1:
      return QString::fromStdString(uiLanguage.localeIdentifier);
    }
  }
  else if (role == Qt::UserRole)
  {
    switch (index.column())
    {
    case 0:
      return QString::fromStdString(uiLanguage.displayName);
    case 1:
      return QString::fromStdString(uiLanguage.localeIdentifier);
    }
  }

  return QVariant();
}

QVariant UILanguageTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
  {
    switch (section)
    {
    case 0:
      return tr("Language");
    case 1:
      return tr("Locale");
    }
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

void UILanguageTableModel::InternalReload()
{
  uiLanguages.clear();
  uiLanguages.push_back(UILanguage());
  for (auto l : StringUtil::Split(UI_LANGUAGES, ';'))
  {
    QLocale qloc(QString::fromStdString(l));
    string language, country, encoding, variant;
    std::tie(language, country, encoding, variant) = Utils::ParseLocaleIdentifier(l);
    string displayName = QLocale::languageToString(qloc.language()).toStdString();
    if (!country.empty())
    {
      displayName += " (" + QLocale::countryToString(qloc.country()).toStdString() + ")";
    }
    uiLanguages.push_back(UILanguage{ displayName, l });
  }
  std::sort(uiLanguages.begin() + 1, uiLanguages.end(), [](const UILanguage& a, const UILanguage& b)
    {
      return StringCompare(a.displayName.c_str(), b.displayName.c_str()) < 0;
    });
}

void UILanguageTableModel::Reload()
{
  beginResetModel();
  MIKTEX_AUTO(endResetModel());
  InternalReload();
}

int UILanguageTableModel::GetDefaultIndex()
{
  auto configValue = session->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_UI_LANGUAGES);
  if (!configValue.HasValue())
  {
    return 0;
  }
  auto preferredUILanguages = configValue.GetStringArray();
  if (preferredUILanguages.empty())
  {
    return 0;
  }
  for (int idx = 1; idx < uiLanguages.size(); ++idx)
  {
    if (preferredUILanguages[0] == uiLanguages[idx].localeIdentifier)
    {
      return idx;
    }
  }
  return -1;
}
