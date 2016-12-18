/* miktex/UI/MFC/Prototypes.h:                          -*- C++ -*-

   Copyright (C) 2000-2016 Christian Schenk

   This file is part of MiKTeX UI Library.

   MiKTeX UI Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX UI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX UI Library; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#include <miktex/First>
#include <miktex/Definitions>

// DLL import/export switch
#if !defined(AD234BEB3E674BA7A4B3A5B557918D53)
#  define MIKTEXUIEXPORT MIKTEXDLLIMPORT
#endif

// API decoration for exported member functions
#define MIKTEXUITHISAPI(type) MIKTEXUIEXPORT type MIKTEXTHISCALL

#define MIKUI_MFC_BEGIN_NAMESPACE               \
  namespace MiKTeX {                            \
    namespace UI {                              \
      namespace MFC {

#define MIKUI_MFC_END_NAMESPACE                 \
      }                                         \
    }                                           \
  }

#include <memory>
#include <string>

#include <miktex/PackageManager/PackageManager>

class CWnd;

MIKUI_MFC_BEGIN_NAMESPACE;

MIKTEXUIEXPORT void MIKTEXCEECALL InitializeFramework();

const unsigned int YES = 1;
const unsigned int NO = 2;
const unsigned int DONTASKAGAIN = 4;

MIKTEXUIEXPORT unsigned int MIKTEXCEECALL InstallPackageMessageBox(CWnd* parent, std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager, const std::string& packageName, const std::string& trigger);

inline unsigned int InstallPackageMessageBox(std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager, const std::string& packageName, const std::string& trigger)
{
  return InstallPackageMessageBox(nullptr, packageManager, packageName, trigger);
}

MIKTEXUIEXPORT bool MIKTEXCEECALL ProxyAuthenticationDialog(CWnd* parent);

inline bool ProxyAuthenticationDialog()
{
  return ProxyAuthenticationDialog(nullptr);
}

MIKTEXUIEXPORT bool MIKTEXCEECALL GiveBackDialog(CWnd* parent, bool force);

inline bool GiveBackDialog(CWnd* parent)
{
  return GiveBackDialog(parent, false);
}

MIKUI_MFC_END_NAMESPACE;
