/* miktex/UI/Qt/PackageInfoDialog.h:                    -*- C++ -*-

   Copyright (C) 2017-2018 Christian Schenk

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

#if !defined(E99E278BF7794F128543A2811B07D6CB)
#define E99E278BF7794F128543A2811B07D6CB

#include "Prototypes"

namespace MiKTeX
{
  namespace Packages
  {
    struct PackageInfo;
  }
}

class QWidget;

MIKUI_QT_BEGIN_NAMESPACE;

class MIKTEXNOVTABLE PackageInfoDialog
{
public:
  PackageInfoDialog() = delete;

public:
  PackageInfoDialog(const PackageInfoDialog& other) = delete;

public:
  PackageInfoDialog& operator=(const PackageInfoDialog& other) = delete;

public:
  PackageInfoDialog(PackageInfoDialog&& other) = delete;

public:
  PackageInfoDialog& operator=(PackageInfoDialog&& other) = delete;

public:
  ~PackageInfoDialog() = delete;

public:
  static MIKTEXUIQTCEEAPI(int) DoModal(QWidget* parent, const MiKTeX::Packages::PackageInfo& packageInfo);
};

MIKUI_QT_END_NAMESPACE;

#endif
