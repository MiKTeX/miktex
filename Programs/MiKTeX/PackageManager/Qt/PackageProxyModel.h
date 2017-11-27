/* PackageProxyModel.h:                                 -*- C++ -*-

   Copyright (C) 2017 Christian Schenk

   This file is part of MiKTeX Package Manager.

   MiKTeX Package Manager is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Package Manager is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Package Manager; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(E627FC0AA1944170B5EEE7CD6FD04E6F)
#define E627FC0AA1944170B5EEE7CD6FD04E6F

#include <QSortFilterProxyModel>

class PackageProxyModel :
  public QSortFilterProxyModel
{
private:
  Q_OBJECT;

public:
  PackageProxyModel(QObject* parent = nullptr);

public:
  void SetFileNamePattern(const std::string& fileNamePattern)
  {
    this->fileNamePattern = fileNamePattern;
    invalidateFilter();
  }

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
  std::string fileNamePattern;
};

#endif
