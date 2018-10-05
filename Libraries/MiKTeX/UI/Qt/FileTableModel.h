/* FileTableModel.h:                                    -*- C++ -*-

   Copyright (C) 2017 Christian Schenk

   This file is part of the MiKTeX UI Library.

   The MiKTeX UI Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX UI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX UI Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(ACB59209CE7F49D8A061486660DC4B27)
#define ACB59209CE7F49D8A061486660DC4B27

#include <QAbstractTableModel>
#include <miktex/PackageManager/PackageManager>
#include <string>
#include <vector>

class FileTableModel :
  public QAbstractTableModel
{
private:
  Q_OBJECT;

public:
  FileTableModel(QObject* parent, const std::vector<std::string>& files);

public:
  int rowCount(const QModelIndex& parent) const override;

public:
  int columnCount(const QModelIndex& parent) const override;

public:
  QVariant data(const QModelIndex& index, int role) const override;

public:
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
  std::vector<std::string> files;
};

#endif
