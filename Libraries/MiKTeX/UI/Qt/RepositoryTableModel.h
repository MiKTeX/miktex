/* RepositoryTableModel.h:                              -*- C++ -*-

   Copyright (C) 2008-2018 Christian Schenk

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

#if !defined(EED8F292DA8F4313B3BB1E684E900634)
#define EED8F292DA8F4313B3BB1E684E900634

#include <vector>

#include <QAbstractTableModel>

#include <miktex/PackageManager/PackageManager>

#include "internal.h"

BEGIN_INTERNAL_NAMESPACE;

class RepositoryTableModel :
  public QAbstractTableModel
{
private:
  Q_OBJECT;

public:
  RepositoryTableModel(QObject* parent = nullptr);

public:
  void SetRepositories(std::vector<MiKTeX::Packages::RepositoryInfo>& repositories)
  {
    this->repositories = repositories;
  }

public:
  int rowCount(const QModelIndex& parent) const override;

public:
  int columnCount(const QModelIndex& parent) const override;

public:
  QVariant data(const QModelIndex& index, int role) const override;

public:
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
  std::vector<MiKTeX::Packages::RepositoryInfo> repositories;
};

END_INTERNAL_NAMESPACE;

#endif
