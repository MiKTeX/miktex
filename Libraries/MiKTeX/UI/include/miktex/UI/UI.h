/* miktex/UI/UI.h:                                      -*- C++ -*-

   Copyright (C) 2000-2021 Christian Schenk

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

#pragma once

#if !defined(DD977A74EEC1466EAF194BE92B20C9C4)
#define DD977A74EEC1466EAF194BE92B20C9C4

#if defined(MIKTEX_UI_QT)
#  include <miktex/UI/Qt/Prototypes>
#endif

#include <miktex/Configuration/ConfigNames>

#include <miktex/Core/Session>

#include <miktex/PackageManager/PackageManager>

#if defined(MIKTEX_WINDOWS)
#  include <miktex/Core/win/WindowsVersion>
#endif

#define MIKUI_BEGIN_NAMESPACE                   \
  namespace MiKTeX {                            \
    namespace UI {                              

#define MIKUI_END_NAMESPACE                     \
      }                                         \
    }

MIKUI_BEGIN_NAMESPACE;

enum class Framework
{
  None, Qt
};

inline int defaultFramework()
{
  // FIXME: multi-threading
  static int cached = -1;
  if (cached < 0)
  {
    int defaultGUIFramework = (int)Framework::None;
#if defined(MIKTEX_UI_QT)
    if (defaultGUIFramework == (int)Framework::None)
    {
      defaultGUIFramework = (int)Framework::Qt;
    }
#endif
    cached = MIKTEX_SESSION()->GetConfigValue(MIKTEX_CONFIG_SECTION_GENERAL, MIKTEX_CONFIG_VALUE_GUI_FRAMEWORK, MiKTeX::Configuration::ConfigValue(defaultGUIFramework)).GetInt();
  }
  return cached;
}

const unsigned int YES = 1;
const unsigned int NO = 2;
const unsigned int DONTASKAGAIN = 4;
const unsigned int ADMIN = 8;

inline void InitializeFramework()
{
#if defined(MIKTEX_UI_QT)
  if (defaultFramework() == (int)Framework::Qt)
  {
    Qt::InitializeFramework();
  }
#endif
}

inline void FinalizeFramework()
{
#if defined(MIKTEX_UI_QT)
  if (defaultFramework() == (int)Framework::Qt)
  {
    Qt::FinalizeFramework();
  }
#endif
}

inline unsigned int InstallPackageMessageBox(std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager, const std::string& packageName, const std::string& trigger)
{
  unsigned ret = NO | DONTASKAGAIN;
#if defined(MIKTEX_UI_QT)
  if (defaultFramework() == (int)Framework::Qt)
  {
    ret = Qt::InstallPackageMessageBox(packageManager, packageName, trigger);
  }
#endif
  return ret;
}

inline bool ProxyAuthenticationDialog()
{
  bool ret = false;
#if defined(MIKTEX_UI_QT)
  if (defaultFramework() == (int)Framework::Qt)
  {
    ret = Qt::ProxyAuthenticationDialog();
}
#endif
  return ret;
}

MIKUI_END_NAMESPACE;

#endif
