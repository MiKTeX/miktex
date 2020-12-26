/* mikuiqt.cpp:

   Copyright (C) 2008-2020 Christian Schenk

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

#if defined(MIKTEX_MACOS_BUNDLE)
#include <mach-o/dyld.h>
#endif

#include <miktex/Core/ConfigNames>

#include "internal.h"

#include <QTranslator>

#include "miktex/UI/UI"

#include "InstallPackageDialog.h"
#include "ProxyAuthenticationDialog.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::UI::Qt;
using namespace MiKTeX::Util;
using namespace std;

static QCoreApplication* application = nullptr;

#if defined(MIKTEX_MACOS_BUNDLE)
PathName GetExecutablePath()
{
  CharBuffer<char> buf;
  uint32_t bufsize = buf.GetCapacity();
  if (_NSGetExecutablePath(buf.GetData(), &bufsize) < 0)
  {
    buf.Reserve(bufsize);
    if (_NSGetExecutablePath(buf.GetData(), &bufsize) != 0)
    {
      MIKTEX_UNEXPECTED();
    }
  }
  char resolved[PATH_MAX + 1];
  if (realpath(buf.GetData(), resolved) == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  return PathName(resolved);
}

PathName GetExecutableDir()
{
  return GetExecutablePath().RemoveFileSpec();
}
#endif

MIKTEXUIQTEXPORT void MIKTEXCEECALL MiKTeX::UI::Qt::InitializeFramework()
{
#if defined(MIKTEX_WINDOWS)
  STARTUPINFOW startupInfo;
  GetStartupInfoW(&startupInfo);
  if ((startupInfo.dwFlags & STARTF_USESHOWWINDOW) && startupInfo.wShowWindow == SW_HIDE)
  {
    MIKTEX_FATAL_ERROR(T_("GUI framework cannot be initialized."));
  }
#endif
#ifdef Q_WS_X11
  bool useGUI = (getenv("DISPLAY") != nullptr);
#else
  bool useGUI = true;
#endif
#if defined(MIKTEX_MACOS_BUNDLE)
  PathName plugIns = GetExecutableDir() / PathName("..") / PathName("PlugIns");
  plugIns.MakeFullyQualified();
  QCoreApplication::addLibraryPath(QString::fromUtf8(plugIns.GetData()));
#endif
  static int argc = 1;
  static PathName argv0;
  static char* argv[2];
  argv0 = Session::Get()->GetMyProgramFile(false).GetFileNameWithoutExtension();
  argv[0] = argv0.GetData();
  argv[1] = nullptr;
  if (useGUI)
  {
    application = new QApplication(argc, argv);
  }
  else
  {
    application = new QCoreApplication(argc, argv);
    QTranslator translator;
    if (translator.load(QLocale(), "ui", "_", ":/i18n", ".qm"))
    {
      QCoreApplication::installTranslator(&translator);
    }
  }
}

MIKTEXUIQTEXPORT void MIKTEXCEECALL MiKTeX::UI::Qt::FinalizeFramework()
{
  delete application;
  application = nullptr;
}

MIKTEXUIQTEXPORT unsigned int MIKTEXCEECALL MiKTeX::UI::Qt::InstallPackageMessageBox(QWidget* parent, shared_ptr<PackageManager> packageManager, const string& packageName, const string& trigger)
{
  shared_ptr<Session> session = Session::Get();
  TriState enableInstaller = session->GetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOINSTALL).GetTriState();
  bool autoAdmin = session->GetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOADMIN).GetTriState() == TriState::True;
  unsigned int ret;
  if (enableInstaller != TriState::Undetermined)
  {
    ret = DONTASKAGAIN;
    ret |= (enableInstaller == TriState::True ? YES : NO);
    if (autoAdmin)
    {
      ret |= ADMIN;
    }
  }
  else
  {
    InstallPackageDialog dlg(parent, packageManager, packageName, trigger);
    dlg.SetAlwaysAsk(enableInstaller == TriState::True ? false : true);
    int dialogCode = dlg.exec();
    if (dialogCode != QDialog::Accepted && dialogCode != QDialog::Rejected)
    {
      ret = (NO | DONTASKAGAIN);
    }
    else
    {
      ret = (dialogCode == QDialog::Accepted ? YES : NO);
      if (dlg.GetAdminMode())
      {
        ret |= ADMIN;
      }
      if (dialogCode == QDialog::Accepted && autoAdmin != dlg.GetAdminMode())
      {
        session->SetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOADMIN, ConfigValue(dlg.GetAdminMode()));
      }
      if (!dlg.GetAlwaysAsk())
      {
        ret |= DONTASKAGAIN;
        if (dialogCode == QDialog::Accepted)
        {
          session->SetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOINSTALL, ConfigValue("1"));
        }
      }
    }
  }
  return ret;
}

MIKTEXUIQTEXPORT bool MIKTEXCEECALL MiKTeX::UI::Qt::ProxyAuthenticationDialog(QWidget* parent)
{
  ProxySettings proxySettings;

  bool done = true;

  if (PackageManager::TryGetProxy(proxySettings)
    && proxySettings.useProxy
    && proxySettings.authenticationRequired
    && proxySettings.user.empty())
  {
    ::ProxyAuthenticationDialog dlg(parent);
    if (dlg.exec() == QDialog::Accepted)
    {
      proxySettings.user = dlg.GetName().toUtf8().constData();
      proxySettings.password = dlg.GetPassword().toUtf8().constData();
      PackageManager::SetProxy(proxySettings);
    }
    else
    {
      done = false;
    }
  }

  return done;
}
