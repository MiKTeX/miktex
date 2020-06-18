/* winSession.cpp: Windows specials

   Copyright (C) 1996-2020 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "config.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <Windows.h>
#include <htmlhelp.h>
#include <shlobj.h>

#include <miktex/Core/ConfigNames>
#include <miktex/Core/Directory>
#include <miktex/Core/Paths>
#include <miktex/Core/win/HResult>
#include <miktex/Core/win/WindowsVersion>
#include <miktex/Core/win/winAutoResource>

#include "internal.h"

#include "Session/SessionImpl.h"
#include "win/winRegistry.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

void SessionImpl::MyCoInitialize()
{
  HResult hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  if (hr.Failed())
  {
    MIKTEX_FATAL_ERROR_2(T_("The COM library could not be initialized."), "hr", hr.GetText());
  }
  ++numCoInitialize;
}

void SessionImpl::MyCoUninitialize()
{
  if (numCoInitialize == 0)
  {
    MIKTEX_UNEXPECTED();
  }
  CoUninitialize();
  --numCoInitialize;
}

#if USE_LOCAL_SERVER
void SessionImpl::ConnectToServer()
{
  const char* MSG_CANNOT_START_SERVER = T_("Cannot start MiKTeX session.");
  if (localServer.pSession == nullptr)
  {
    WCHAR wszCLSID[50];
    if (StringFromGUID2(__uuidof(MiKTeXSessionLib::MAKE_CURVER_ID(MiKTeXSession)), wszCLSID, sizeof(wszCLSID) / sizeof(wszCLSID[0])) < 0)
    {
      MIKTEX_FATAL_ERROR(MSG_CANNOT_START_SERVER);
    }
    wstring monikerName;
    monikerName = L"Elevation:Administrator!new:";
    monikerName += wszCLSID;
    BIND_OPTS3 bo;
    memset(&bo, 0, sizeof(bo));
    bo.cbStruct = sizeof(bo);
    bo.hwnd = GetForegroundWindow();
    bo.dwClassContext = CLSCTX_LOCAL_SERVER;
    HResult hr = CoGetObject(monikerName.c_str(), &bo, __uuidof(MiKTeXSessionLib::ISession), reinterpret_cast<void**>(&localServer.pSession));
    if (hr == CO_E_NOTINITIALIZED)
    {
      MyCoInitialize();
      hr = CoGetObject(monikerName.c_str(), &bo, __uuidof(MiKTeXSessionLib::ISession), reinterpret_cast<void**>(&localServer.pSession));
    }
    if (hr.Failed())
    {
      MIKTEX_FATAL_ERROR_2(MSG_CANNOT_START_SERVER, "hr", hr.GetText());
    }
  }
}
#endif

#if USE_LOCAL_SERVER

bool SessionImpl::runningAsLocalServer = false;

bool SessionImpl::UseLocalServer()
{
  if (SessionImpl::runningAsLocalServer)
  {
    // already running as local server
    return false;
  }
#if defined(MIKTEX_WINDOWS)
  bool elevationRequired = (IsAdminMode() && !RunningAsAdministrator());
  return elevationRequired;
#else
  return false;
#endif
}

#endif

#if defined(MIKTEX_CORE_SHARED)

#if defined(_MANAGED)
#  pragma managed(push, off)
#endif

#if ! defined(MIKTEX_PREVENT_DYNAMIC_LOADS)
#  define MIKTEX_PREVENT_DYNAMIC_LOADS 0
#endif

HINSTANCE SessionImpl::hinstDLL = nullptr;
TriState SessionImpl::dynamicLoad = TriState::Undetermined;

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD reason, LPVOID lpReserved)
{
  BOOL retCode = TRUE;

  switch (reason)
  {
    // initialize primary thread
  case DLL_PROCESS_ATTACH:
    SessionImpl::dynamicLoad = lpReserved == nullptr ? TriState::True : TriState::False;
#if MIKTEX_PREVENT_DYNAMIC_LOADS
    if (SessionImpl::dynamicLoad == TriState::True)
    {
      retCode = FALSE;
    }
#endif
    SessionImpl::hinstDLL = hInstance;
    break;

    // finalize primary thread
  case DLL_PROCESS_DETACH:
    SessionImpl::dynamicLoad = TriState::Undetermined;
    SessionImpl::hinstDLL = nullptr;
    break;
  }

#if defined(HAVE_ATLBASE_H) && defined(MIKTEX_CORE_SHARED)
  if (retCode)
  {
    retCode = SessionImpl::AtlDllMain(reason, lpReserved);
  }
#endif

  return retCode;
}

#if defined(_MANAGED)
#  pragma managed(pop)
#endif

#endif

PathName SessionImpl::GetMyProgramFile(bool canonicalized)
{
  // we do this once
  if (myProgramFile.Empty())
  {
    wchar_t szPath[BufferSizes::MaxPath];
    DWORD n = GetModuleFileNameW(nullptr, szPath, BufferSizes::MaxPath);
    if (n == 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("GetModuleFileNameW");
    }
    if (n == BufferSizes::MaxPath)
    {
      BUF_TOO_SMALL();
    }
    myProgramFile = szPath;
    myProgramFileCanon = myProgramFile;
    myProgramFileCanon.Canonicalize();
  }
  if (canonicalized)
  {
    return myProgramFileCanon;
  }
  else
  {
    return myProgramFile;
  }
}

#if defined(MIKTEX_CORE_SHARED)
PathName SessionImpl::GetDllPathName(bool canonicalized)
{
  // we do this once
  if (dllPathName.Empty())
  {
    wchar_t szPath[BufferSizes::MaxPath];
    DWORD n = GetModuleFileNameW(hinstDLL, szPath, BufferSizes::MaxPath);
    if (n == 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("GetModuleFileNameW");
    }
    if (n == BufferSizes::MaxPath)
    {
      BUF_TOO_SMALL();
    }
    dllPathName = szPath;
    dllPathNameCanon = dllPathName;
    dllPathNameCanon.Canonicalize();
  }
  if (canonicalized)
  {
    return dllPathNameCanon;
  }
  else
  {
    return dllPathName;
  }
}

#endif

/*
 * UserConfig:    %USERPROFILE%\AppData\Roaming\MiKTeX
 * UserData:      %USERPROFILE%\AppData\Local\MiKTeX
 * UserInstall:   %USERPROFILE%\AppData\Local\Programs\MiKTeX
 * CommonConfig:  C:\ProgramData\MiKTeX
 * CommonData:    C:\ProgramData\MiKTeX
 * CommonInstall: C:\Program Files\MiKTeX
 */
VersionedStartupConfig SessionImpl::DefaultConfig(MiKTeXConfiguration config, VersionNumber setupVersion, const PathName& commonPrefixArg, const PathName& userPrefixArg)
{
  VersionedStartupConfig ret;
  if (config == MiKTeXConfiguration::None)
  {
    config = MiKTeXConfiguration::Regular;
  }
  ret.config = config;
  ret.setupVersion = setupVersion;
  if (config == MiKTeXConfiguration::Portable)
  {
    PathName commonPrefix(commonPrefixArg);
    PathName userPrefix(userPrefixArg);
    if (commonPrefix.Empty() && !userPrefix.Empty())
    {
      commonPrefix = userPrefix;
    }
    else if (userPrefix.Empty() && !commonPrefix.Empty())
    {
      userPrefix = commonPrefix;
    }
    if (!commonPrefix.Empty())
    {
      PathName portableRoot;
      bool isLegacy = !Utils::GetPathNamePrefix(commonPrefix, PathName(MIKTEX_PORTABLE_REL_INSTALL_DIR), portableRoot);
      ret.commonInstallRoot = commonPrefix;
      if (!isLegacy)
      {
        ret.commonConfigRoot = portableRoot / PathName(MIKTEX_PORTABLE_REL_CONFIG_DIR);
        ret.commonDataRoot = portableRoot / PathName(MIKTEX_PORTABLE_REL_DATA_DIR);
      }
      else
      {
        ret.commonConfigRoot = commonPrefix;
        ret.commonDataRoot = commonPrefix;
      }
    }
    if (!userPrefix.Empty())
    {
      PathName portableRoot;
      bool isLegacy = !Utils::GetPathNamePrefix(userPrefix, PathName(MIKTEX_PORTABLE_REL_INSTALL_DIR), portableRoot);
      ret.userInstallRoot = userPrefix;
      if (!isLegacy)
      {
        ret.userConfigRoot = portableRoot / PathName(MIKTEX_PORTABLE_REL_CONFIG_DIR);
        ret.userDataRoot = portableRoot / PathName(MIKTEX_PORTABLE_REL_DATA_DIR);
      }
      else
      {
        ret.userConfigRoot = userPrefix;
        ret.userDataRoot = userPrefix;
      }
    }
    if (userPrefix.Empty() || commonPrefix.Empty())
    {
      PathName myloc(GetMyLocation(false));
      PathName prefix;
      if (Utils::GetPathNamePrefix(myloc, PathName(MIKTEX_PATH_INTERNAL_BIN_DIR), prefix)
        || Utils::GetPathNamePrefix(myloc, PathName(MIKTEX_PATH_BIN_DIR), prefix)
        || Utils::GetPathNamePrefix(myloc, PathName(MIKTEX_PATH_MIKTEX_TEMP_DIR), prefix))
      {
        PathName portableRoot;
        bool isLegacy = !Utils::GetPathNamePrefix(prefix, PathName(MIKTEX_PORTABLE_REL_INSTALL_DIR), portableRoot);
        if (commonPrefix.Empty())
        {
          ret.commonInstallRoot = prefix;
          if (!isLegacy)
          {
            ret.commonConfigRoot = portableRoot / PathName(MIKTEX_PORTABLE_REL_CONFIG_DIR);
            ret.commonDataRoot = portableRoot / PathName(MIKTEX_PORTABLE_REL_DATA_DIR);
          }
          else
          {
            ret.commonConfigRoot = prefix;
            ret.commonDataRoot = prefix;
          }
        }
        if (userPrefix.Empty())
        {
          ret.userInstallRoot = prefix;
          if (!isLegacy)
          {
            ret.userConfigRoot = portableRoot / PathName(MIKTEX_PORTABLE_REL_CONFIG_DIR);
            ret.userDataRoot = portableRoot / PathName(MIKTEX_PORTABLE_REL_DATA_DIR);
          }
          else
          {
            ret.userConfigRoot = prefix;
            ret.userDataRoot = prefix;
          }
        }
      }
    }
  }
  else
  {
    string product;
    bool isLegacy = setupVersion < VersionNumber("20.6");
    if (config == MiKTeXConfiguration::Direct)
    {
      product = "MiKTeXDirect";
      PathName myloc(GetMyLocation(false));
      PathName prefix;
      if (!Utils::GetPathNamePrefix(myloc, PathName(MIKTEX_PATH_BIN_DIR), prefix))
      {
        MIKTEX_UNEXPECTED();
      }
      ret.commonInstallRoot = prefix;
    }
    else
    {
      product = "MiKTeX";
      wchar_t szProgramFiles[MAX_PATH];
      if (SHGetFolderPathW(nullptr, CSIDL_PROGRAM_FILES, nullptr, SHGFP_TYPE_CURRENT, szProgramFiles) == S_OK)
      {
        ret.commonInstallRoot = szProgramFiles;
        ret.commonInstallRoot /= product;
        if (isLegacy)
        {
          ret.commonInstallRoot += " " MIKTEX_LEGACY_MAJOR_MINOR_STR;
        }
      }
    }
    wchar_t szPath[MAX_PATH];
    if (SHGetFolderPathW(nullptr, CSIDL_COMMON_APPDATA, nullptr, SHGFP_TYPE_CURRENT, szPath) == S_OK)
    {
      ret.commonDataRoot = szPath;
      ret.commonDataRoot /= product;
      if (isLegacy)
      {
        ret.commonDataRoot /= MIKTEX_LEGACY_MAJOR_MINOR_STR;
      }
    }
    ret.commonConfigRoot = ret.commonDataRoot;
    if (SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, szPath) == S_OK
      || SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, szPath) == S_OK)
    {
      ret.userDataRoot = szPath;
      ret.userDataRoot /= product;
      if (isLegacy)
      {
        ret.userDataRoot /= MIKTEX_LEGACY_MAJOR_MINOR_STR;
      }
    }
    if (SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, szPath) == S_OK)
    {
      ret.userConfigRoot = szPath;
      ret.userConfigRoot /= product;
      if (isLegacy)
      {
        ret.userConfigRoot /= MIKTEX_LEGACY_MAJOR_MINOR_STR;
      }
    }
    ret.userInstallRoot = ret.userConfigRoot;
  }
  return ret;
}

VersionedStartupConfig SessionImpl::ReadRegistry(ConfigurationScope scope)
{
  MIKTEX_ASSERT(!IsMiKTeXDirect());

  VersionedStartupConfig ret;

  string str;

  if (scope == ConfigurationScope::Common)
  {
    if (winRegistry::TryGetRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_SETUP, MIKTEX_CONFIG_VALUE_VERSION, str))
    {
      ret.setupVersion = VersionNumber::Parse(str);
    }
    if (winRegistry::TryGetRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_ROOTS, str))
    {
      ret.commonRoots = str;
    }
    if (winRegistry::TryGetRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_OTHER_COMMON_ROOTS, str))
    {
      ret.otherCommonRoots = str;
    }
    if (winRegistry::TryGetRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_INSTALL, str))
    {
      ret.commonInstallRoot = str;
    }
    if (winRegistry::TryGetRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_DATA, str))
    {
      ret.commonDataRoot = str;
    }
    if (winRegistry::TryGetRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_CONFIG, str))
    {
      ret.commonConfigRoot = str;
    }
  }
  else if (scope == ConfigurationScope::User)
  {
    if (winRegistry::TryGetRegistryValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_SETUP, MIKTEX_CONFIG_VALUE_VERSION, str))
    {
      ret.setupVersion = VersionNumber::Parse(str);
    }
    if (winRegistry::TryGetRegistryValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_ROOTS, str))
    {
      ret.userRoots = str;
    }
    if (winRegistry::TryGetRegistryValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_OTHER_USER_ROOTS, str))
    {
      ret.otherUserRoots = str;
    }
    if (winRegistry::TryGetRegistryValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_INSTALL, str))
    {
      ret.userInstallRoot = str;
    }
    if (winRegistry::TryGetRegistryValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_DATA, str))
    {
      ret.userDataRoot = str;
    }
    if (winRegistry::TryGetRegistryValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_CONFIG, str))
    {
      ret.userConfigRoot = str;
    }
  }

  return ret;
}

void SessionImpl::WriteRegistry(ConfigurationScope scope, const VersionedStartupConfig& startupConfig)
{
  MIKTEX_ASSERT(!IsMiKTeXDirect());

  VersionedStartupConfig defaultConfig = DefaultConfig(startupConfig.config, startupConfig.setupVersion, PathName(), PathName());

  // remove registry values
  if (scope == ConfigurationScope::Common)
  {
    winRegistry::TryDeleteRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_SETUP, MIKTEX_CONFIG_VALUE_VERSION);
    winRegistry::TryDeleteRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_ROOTS);
    winRegistry::TryDeleteRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_INSTALL);
    winRegistry::TryDeleteRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_DATA);
    winRegistry::TryDeleteRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_CONFIG);
    winRegistry::TryDeleteRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_OTHER_COMMON_ROOTS);
#if 1
    winRegistry::TryDeleteRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_ROOTS);
    winRegistry::TryDeleteRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_INSTALL);
    winRegistry::TryDeleteRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_DATA);
    winRegistry::TryDeleteRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_CONFIG);
    winRegistry::TryDeleteRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_OTHER_USER_ROOTS);
#endif
  }
  else if (scope == ConfigurationScope::User)
  {
    winRegistry::TryDeleteRegistryValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_SETUP, MIKTEX_CONFIG_VALUE_VERSION);
    winRegistry::TryDeleteRegistryValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_ROOTS);
    winRegistry::TryDeleteRegistryValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_INSTALL);
    winRegistry::TryDeleteRegistryValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_DATA);
    winRegistry::TryDeleteRegistryValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_CONFIG);
    winRegistry::TryDeleteRegistryValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_OTHER_USER_ROOTS);
  }

  if (scope == ConfigurationScope::Common)
  {
    if (!(startupConfig.setupVersion == VersionNumber()))
    {
      winRegistry::SetRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_SETUP, MIKTEX_CONFIG_VALUE_VERSION, startupConfig.setupVersion.ToString());
    }
    if (!startupConfig.commonRoots.empty() && startupConfig.commonRoots != defaultConfig.commonRoots)
    {
      winRegistry::SetRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_ROOTS, startupConfig.commonRoots);
    }
    if (!startupConfig.otherCommonRoots.empty() && startupConfig.otherCommonRoots != defaultConfig.otherCommonRoots)
    {
      winRegistry::SetRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_OTHER_COMMON_ROOTS, startupConfig.otherCommonRoots);
    }
    if (!startupConfig.commonInstallRoot.Empty() && startupConfig.commonInstallRoot != defaultConfig.commonInstallRoot)
    {
      winRegistry::SetRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_INSTALL, startupConfig.commonInstallRoot.ToString());
    }
    if (!startupConfig.commonDataRoot.Empty() && startupConfig.commonDataRoot != defaultConfig.commonDataRoot)
    {
      winRegistry::SetRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_DATA, startupConfig.commonDataRoot.ToString());
    }
    if (!startupConfig.commonConfigRoot.Empty() && startupConfig.commonConfigRoot != defaultConfig.commonConfigRoot)
    {
      winRegistry::SetRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMON_CONFIG, startupConfig.commonConfigRoot.ToString());
    }
#if 1
    if (!startupConfig.userRoots.empty() && startupConfig.userRoots != defaultConfig.userRoots)
    {
      winRegistry::SetRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_ROOTS, startupConfig.userRoots);
    }
    if (!startupConfig.otherUserRoots.empty() && startupConfig.otherUserRoots != defaultConfig.otherUserRoots)
    {
      winRegistry::SetRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_OTHER_USER_ROOTS, startupConfig.otherUserRoots);
    }
    if (!startupConfig.userInstallRoot.Empty() && startupConfig.userInstallRoot != defaultConfig.userInstallRoot)
    {
      winRegistry::SetRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_INSTALL, startupConfig.userInstallRoot.ToString());
    }
    if (!startupConfig.userDataRoot.Empty() && startupConfig.userDataRoot != defaultConfig.userDataRoot)
    {
      winRegistry::SetRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_DATA, startupConfig.userDataRoot.ToString());
    }
    if (!startupConfig.userConfigRoot.Empty() && startupConfig.userConfigRoot != defaultConfig.userConfigRoot)
    {
      winRegistry::SetRegistryValue(ConfigurationScope::Common, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_CONFIG, startupConfig.userConfigRoot.ToString());
    }
#endif
  }
  else if (scope == ConfigurationScope::User)
  {
    if (!(startupConfig.setupVersion == VersionNumber()))
    {
      winRegistry::SetRegistryValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_SETUP, MIKTEX_CONFIG_VALUE_VERSION, startupConfig.setupVersion.ToString());
    }
    if (!startupConfig.userRoots.empty() && startupConfig.userRoots != defaultConfig.userRoots)
    {
      winRegistry::SetRegistryValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_ROOTS, startupConfig.userRoots);
    }
    if (!startupConfig.otherUserRoots.empty() && startupConfig.otherUserRoots != defaultConfig.otherUserRoots)
    {
      winRegistry::SetRegistryValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_OTHER_USER_ROOTS, startupConfig.otherUserRoots);
    }
    if (!startupConfig.userInstallRoot.Empty() && startupConfig.userInstallRoot != defaultConfig.userInstallRoot)
    {
      winRegistry::SetRegistryValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_INSTALL, startupConfig.userInstallRoot.ToString());
    }
    if (!startupConfig.userDataRoot.Empty() && startupConfig.userDataRoot != defaultConfig.userDataRoot)
    {
      winRegistry::SetRegistryValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_DATA, startupConfig.userDataRoot.ToString());
    }
    if (!startupConfig.userConfigRoot.Empty() && startupConfig.userConfigRoot != defaultConfig.userConfigRoot)
    {
      winRegistry::SetRegistryValue(ConfigurationScope::None, MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USER_CONFIG, startupConfig.userConfigRoot.ToString());
    }
  }
}

bool SessionImpl::GetAcrobatFontDir(PathName& path)
{
  if (!flags.test((size_t)InternalFlag::CachedAcrobatFontDir))
  {
    flags.set((size_t)InternalFlag::CachedAcrobatFontDir);

    const wchar_t* const ACRORD32 = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\AcroRd32.exe";

    wstring pathExe;

    if (!winRegistry::TryGetRegistryValue(HKEY_LOCAL_MACHINE, ACRORD32, L"", pathExe))
    {
      return false;
    }

    PathName dir(pathExe);
    dir.RemoveFileSpec();

    PathName fontDir;

    // try Acrobat Reader 3.0
    fontDir = dir / PathName("FONTS");
    if (!Directory::Exists(fontDir))
    {
      // try Acrobat Reader 4.0
      fontDir = dir / PathName("..") / PathName("Resource") / PathName("Font");
      if (!Directory::Exists(fontDir))
      {
        return false;
      }
    }

    RemoveDirectoryDelimiter(fontDir.GetData());

    acrobatFontDir = GetFullyQualifiedPath(fontDir.GetData());
  }

  if (acrobatFontDir.GetLength() == 0)
  {
    return false;
  }

  path = acrobatFontDir;

  return true;
}

bool SessionImpl::GetATMFontDir(PathName& path)
{
  if (!flags.test((size_t)InternalFlag::CachedAtmFontDir))
  {
    flags.set((size_t)InternalFlag::CachedAtmFontDir);

    const wchar_t* const ATMSETUP = L"SOFTWARE\\Adobe\\Adobe Type Manager\\Setup";

    wstring pfbDir;

    if (!winRegistry::TryGetRegistryValue(HKEY_LOCAL_MACHINE, ATMSETUP, L"PFB_DIR", pfbDir))
    {
      return false;
    }

    PathName fontDir(pfbDir);

    if (!Directory::Exists(fontDir))
    {
      return false;
    }

    RemoveDirectoryDelimiter(fontDir.GetData());

    atmFontDir = GetFullyQualifiedPath(fontDir.GetData());
  }

  if (atmFontDir.GetLength() == 0)
  {
    return false;
  }

  path = atmFontDir;

  return true;
}

MIKTEXSTATICFUNC(bool) GetPsFontDirectory(PathName& path)
{
  wchar_t szWinDir[BufferSizes::MaxPath];

  if (GetWindowsDirectoryW(szWinDir, BufferSizes::MaxPath) == 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetWindowsDirectoryW");
  }

  PathName path_ = PathName(szWinDir) / PathName("psfonts");

  if (!Directory::Exists(path_))
  {
    return false;
  }

  path = path_;

  return true;
}

bool SessionImpl::GetPsFontDirs(string& psFontDirs)
{
  if (!flags.test((size_t)InternalFlag::CachedPsFontDirs))
  {
    flags.set((size_t)InternalFlag::CachedPsFontDirs);
    PathName path;
    if (GetATMFontDir(path) || GetPsFontDirectory(path))
    {
      if (!this->psFontDirs.empty())
      {
        this->psFontDirs += PathNameUtil::PathNameDelimiter;
      }
      this->psFontDirs += path.GetData();
    }
    if (GetAcrobatFontDir(path))
    {
      if (!this->psFontDirs.empty())
      {
        this->psFontDirs += PathNameUtil::PathNameDelimiter;
      }
      this->psFontDirs += path.GetData();
    }
  }

  if (this->psFontDirs.empty())
  {
    return false;
  }

  psFontDirs = this->psFontDirs;

  return true;
}

bool SessionImpl::GetTTFDirs(string& ttfDirs)
{
  if (!flags.test((size_t)InternalFlag::CachedTtfDirs))
  {
    flags.set((size_t)InternalFlag::CachedTtfDirs);
    PathName path;
    if (GetSystemFontDirectory(path))
    {
      if (!this->ttfDirs.empty())
      {
        this->ttfDirs += PathNameUtil::PathNameDelimiter;;
      }
      this->ttfDirs += path.GetData();
    }
  }

  if (this->ttfDirs.empty())
  {
    return false;
  }

  ttfDirs = this->ttfDirs;

  return true;
}

bool SessionImpl::GetOTFDirs(string& otfDirs)
{
  if (!flags.test((size_t)InternalFlag::CachedOtfDirs))
  {
    flags.set((size_t)InternalFlag::CachedOtfDirs);
    PathName path;
    if (GetSystemFontDirectory(path))
    {
      if (!this->otfDirs.empty())
      {
        this->otfDirs += PathNameUtil::PathNameDelimiter;
      }
      this->otfDirs += path.GetData();
    }
  }

  if (this->otfDirs.empty())
  {
    return false;
  }

  otfDirs = this->otfDirs;

  return true;
}

void Session::FatalWindowsError(const string& functionName, unsigned long errorCode, const std::string& description_, const string& remedy_, const string& tag_, const MiKTeXException::KVMAP& info, const SourceLocation& sourceLocation)
{
  string programInvocationName;
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  if (session != nullptr)
  {
    TraceWindowsError(functionName.c_str(), errorCode, info.ToString().c_str(), sourceLocation.fileName.c_str(), sourceLocation.lineNo);
    programInvocationName = session->initInfo.GetProgramInvocationName();
  }
  string errorMessage = T_("Windows API error ") + std::to_string(errorCode);
  string windowsErrorMessage;
  if (GetWindowsErrorMessage(errorCode, windowsErrorMessage))
  {
    errorMessage += ": ";
    errorMessage += windowsErrorMessage;
  }
  else
  {
    errorMessage += '.';
  }
#if 1
  string env;
  if (Utils::GetEnvironmentString("MIKTEX_DEBUG_BREAK", env) && env == "1")
  {
    DEBUG_BREAK();
  }
#endif
  string description = description_;
  string remedy = remedy_;
  string tag = tag_;
  switch (errorCode)
  {
  case ERROR_ACCESS_DENIED:
    throw UnauthorizedAccessException(programInvocationName, errorMessage, description, remedy, tag, info, sourceLocation);
  case ERROR_FILE_NOT_FOUND:
  case ERROR_PATH_NOT_FOUND:
    throw FileNotFoundException(programInvocationName, errorMessage, description, remedy, tag, info, sourceLocation);
  case ERROR_SHARING_VIOLATION:
  case ERROR_USER_MAPPED_FILE:
    if (description.empty() && info.find("path") != info.end())
    {
      description = T_("MiKTeX cannot access file '{path}' because it is either locked by another MiKTeX program or by the operating system.");
    }
    if (remedy.empty())
    {
      remedy = T_("Close running MiKTeX programs and try again.");
    }
    if (tag.empty())
    {
      tag = "file-in-use";
    }
    throw SharingViolationException(programInvocationName, errorMessage, description, remedy, tag, info, sourceLocation);
  case ERROR_DIR_NOT_EMPTY:
    throw DirectoryNotEmptyException(programInvocationName, errorMessage, description, remedy, tag, info, sourceLocation);
  default:
    throw MiKTeXException(programInvocationName, errorMessage, description, remedy, tag, info, sourceLocation);
  }
}

bool SessionImpl::ShowManualPageAndWait(HWND hWnd, unsigned long topic)
{
#if defined(_MSC_VER)
  PathName pathHelpFile;
  if (!FindFile("miktex.chm", "%R\\doc\\miktex//", pathHelpFile))
  {
    return false;
  }
  HWND hwnd = HtmlHelpW(hWnd, pathHelpFile.ToExtendedLengthPathName().ToWideCharString().c_str(), HH_HELP_CONTEXT, topic);
  if (hwnd == nullptr)
  {
    return false;
  }
  while (IsWindow(hwnd))
  {
    Sleep(20);
  }
  return true;
#else
  // TODO
  return false;
#endif
}

bool SessionImpl::IsFileAlreadyOpen(const PathName& fileName)
{
  unsigned long error = NO_ERROR;

  HANDLE hFile = CreateFileW(fileName.ToExtendedLengthPathName().ToWideCharString().c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

  if (hFile == INVALID_HANDLE_VALUE)
  {
    error = ::GetLastError();
  }
  else
  {
    if (!CloseHandle(hFile))
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("CloseHandle", "filename", fileName.ToString());
    }
  }

  return hFile == INVALID_HANDLE_VALUE && error == ERROR_SHARING_VIOLATION;
}

void SessionImpl::ScheduleFileRemoval(const PathName& fileName)
{
  string cmd = Directory::Exists(fileName) ? "rmdir /S /Q " : "del ";
  cmd += Q_(fileName.ToDos());
  onFinishScript.push_back(cmd);
}

bool SessionImpl::IsUserMemberOfGroup(DWORD localGroup)
{
  HANDLE hThread;

  if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hThread))
  {
    if (GetLastError() == ERROR_NO_TOKEN)
    {
      if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hThread))
      {
        return false;
      }
    }
    else
    {
      return false;
    }
  }

  AutoHANDLE autoClose(hThread);

  DWORD cbTokenGroups;

  if (GetTokenInformation(hThread, TokenGroups, nullptr, 0, &cbTokenGroups))
  {
    return false;
  }

  if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
  {
    return false;
  }

  TOKEN_GROUPS* ptg = reinterpret_cast<TOKEN_GROUPS*>(_alloca(cbTokenGroups));

  if (ptg == nullptr)
  {
    return false;
  }

  if (!GetTokenInformation(hThread, TokenGroups, ptg, cbTokenGroups, &cbTokenGroups))
  {
    return false;
  }

  SID_IDENTIFIER_AUTHORITY SystemSidAuthority = { SECURITY_NT_AUTHORITY };

  PSID psidAdmin;

  if (!AllocateAndInitializeSid(&SystemSidAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, localGroup, 0, 0, 0, 0, 0, 0, &psidAdmin))
  {
    return false;
  }

  AutoSid autoFree(psidAdmin);

  for (size_t i = 0; i < ptg->GroupCount; ++i)
  {
    if (EqualSid(ptg->Groups[i].Sid, psidAdmin))
    {
      return true;
    }
  }

  return false;
}

bool SessionImpl::IsUserAnAdministrator()
{
  if (isUserAnAdministrator == TriState::Undetermined)
  {
    if (IsUserMemberOfGroup(DOMAIN_ALIAS_RID_ADMINS))
    {
      isUserAnAdministrator = TriState::True;
    }
    else
    {
      isUserAnAdministrator = TriState::False;
    }
  }
  return isUserAnAdministrator == TriState::True ? true : false;
}

bool SessionImpl::RunningElevated()
{
  HANDLE hToken;
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hToken))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("OpenProcessToken");
  }
  DWORD infoLen;
  TOKEN_ELEVATION_TYPE elevationType;
  if (!GetTokenInformation(hToken, TokenElevationType, &elevationType, sizeof(elevationType), &infoLen))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetTokenInformation");
  }
  TOKEN_ELEVATION elevation;
  if (!GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &infoLen))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetTokenInformation");
  }
  switch (elevationType)
  {
  case TokenElevationTypeDefault:
    return elevation.TokenIsElevated == 0 ? false : true;
  case TokenElevationTypeFull:
    return true;
  case TokenElevationTypeLimited:
    return false;
  default:
    MIKTEX_UNEXPECTED();
  }
}

bool SessionImpl::RunningAsAdministrator()
{
  if (runningAsAdministrator == TriState::Undetermined)
  {
    if (IsUserAnAdministrator())
    {
      if (!RunningElevated())
      {
        runningAsAdministrator = TriState::False;
      }
      else
      {
        runningAsAdministrator = TriState::True;
      }
    }
    else
    {
      runningAsAdministrator = TriState::False;
    }
  }
  return runningAsAdministrator == TriState::True ? true : false;
}
