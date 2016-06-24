/* winSession.cpp: Windows specials

   Copyright (C) 1996-2016 Christian Schenk

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/Directory.h"
#include "miktex/Core/Paths.h"
#include "miktex/Core/Registry.h"
#include "miktex/Core/win/HResult.h"
#include "miktex/Core/win/WindowsVersion.h"
#include "miktex/Core/win/winAutoResource.h"

#include "Session/SessionImpl.h"
#include "win/winRegistry.h"

using namespace MiKTeX::Core;
using namespace std;

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
  const char * MSG_CANNOT_START_SERVER = T_("Cannot start MiKTeX session.");
  if (localServer.pSession == nullptr)
  {
    if (WindowsVersion::IsWindowsVistaOrGreater())
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
    else
    {
      HResult hr = localServer.pSession.CoCreateInstance(__uuidof(MiKTeXSessionLib::MAKE_CURVER_ID(MiKTeXSession)), nullptr, CLSCTX_LOCAL_SERVER);
      if (hr == CO_E_NOTINITIALIZED)
      {
	MyCoInitialize();
	hr = localServer.pSession.CoCreateInstance(__uuidof(MiKTeXSessionLib::MAKE_CURVER_ID(MiKTeXSession)), nullptr, CLSCTX_LOCAL_SERVER);
      }
      if (hr.Failed())
      {
	MIKTEX_FATAL_ERROR(MSG_CANNOT_START_SERVER, "hr", hr.GetText());
      }
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
  bool elevationRequired = (WindowsVersion::IsWindowsVistaOrGreater() && IsAdminMode() && !RunningAsAdministrator());
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

/* _________________________________________________________________________

   SessionImpl::DefaultConfig

   UserInstall:   %USERPROFILE%\AppData\Roaming\MiKTeX\X.Y\
   UserConfig:    %USERPROFILE%\AppData\Roaming\MiKTeX\X.Y\
   UserData:      %USERPROFILE%\AppData\Local\MiKTeX\X.Y\
   CommonInstall: C:\Program Files\MiKTeX X.Y\
   CommonConfig:  C:\ProgramData\MiKTeX\X.Y\
   CommonData:    C:\ProgramData\MiKTeX\X.Y\
   _________________________________________________________________________ */

StartupConfig SessionImpl::DefaultConfig(MiKTeXConfiguration config, const PathName & commonPrefixArg, const PathName & userPrefixArg)
{
  StartupConfig ret;
  if (config == MiKTeXConfiguration::None)
  {
    config = MiKTeXConfiguration::Regular;
  }
  ret.config = config;
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
      bool isLegacy = !Utils::GetPathNamePrefix(commonPrefix, MIKTEX_PORTABLE_REL_INSTALL_DIR, portableRoot);
      ret.commonInstallRoot = commonPrefix;
      if (!isLegacy)
      {
	ret.commonConfigRoot = portableRoot / MIKTEX_PORTABLE_REL_CONFIG_DIR;
	ret.commonDataRoot = portableRoot / MIKTEX_PORTABLE_REL_DATA_DIR;
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
      bool isLegacy = !Utils::GetPathNamePrefix(userPrefix, MIKTEX_PORTABLE_REL_INSTALL_DIR, portableRoot);
      ret.userInstallRoot = userPrefix;
      if (!isLegacy)
      {
	ret.userConfigRoot = portableRoot / MIKTEX_PORTABLE_REL_CONFIG_DIR;
	ret.userDataRoot = portableRoot / MIKTEX_PORTABLE_REL_DATA_DIR;
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
      if (Utils::GetPathNamePrefix(myloc, MIKTEX_PATH_INTERNAL_BIN_DIR, prefix)
	|| Utils::GetPathNamePrefix(myloc, MIKTEX_PATH_BIN_DIR, prefix)
	|| Utils::GetPathNamePrefix(myloc, MIKTEX_PATH_MIKTEX_TEMP_DIR, prefix))
      {
	PathName portableRoot;
	bool isLegacy = !Utils::GetPathNamePrefix(prefix, MIKTEX_PORTABLE_REL_INSTALL_DIR, portableRoot);
	if (commonPrefix.Empty())
	{
	  ret.commonInstallRoot = prefix;
	  if (!isLegacy)
	  {
	    ret.commonConfigRoot = portableRoot / MIKTEX_PORTABLE_REL_CONFIG_DIR;
	    ret.commonDataRoot = portableRoot / MIKTEX_PORTABLE_REL_DATA_DIR;
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
	    ret.userConfigRoot = portableRoot / MIKTEX_PORTABLE_REL_CONFIG_DIR;
	    ret.userDataRoot = portableRoot / MIKTEX_PORTABLE_REL_DATA_DIR;
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
    if (config == MiKTeXConfiguration::Direct)
    {
      product = "MiKTeXDirect";
      PathName myloc(GetMyLocation(false));
      PathName prefix;
      if (!Utils::GetPathNamePrefix(myloc, MIKTEX_PATH_BIN_DIR, prefix))
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
	ret.commonInstallRoot /= "MiKTeX" " " MIKTEX_SERIES_STR;
      }
    }
    wchar_t szPath[MAX_PATH];
    if (SHGetFolderPathW(nullptr, CSIDL_COMMON_APPDATA, nullptr, SHGFP_TYPE_CURRENT, szPath) == S_OK)
    {
      ret.commonDataRoot = szPath;
      ret.commonDataRoot /= product;
      ret.commonDataRoot /= MIKTEX_SERIES_STR;
    }
    ret.commonConfigRoot = ret.commonDataRoot;
    if (SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, szPath) == S_OK
      || SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, szPath) == S_OK)
    {
      ret.userDataRoot = szPath;
      ret.userDataRoot /= product;
      ret.userDataRoot /= MIKTEX_SERIES_STR;
    }
    if (SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, szPath) == S_OK)
    {
      ret.userConfigRoot = szPath;
      ret.userConfigRoot /= product;
      ret.userConfigRoot /= MIKTEX_SERIES_STR;
    }
    ret.userInstallRoot = ret.userConfigRoot;
  }
  return ret;
}

StartupConfig SessionImpl::ReadRegistry(bool common)
{
  MIKTEX_ASSERT(!IsMiKTeXDirect());

  StartupConfig ret;

  string str;

  if (common)
  {
    if (winRegistry::TryGetRegistryValue(TriState::True, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_COMMON_ROOTS, str))
    {
      ret.commonRoots = str;
    }
    if (winRegistry::TryGetRegistryValue(TriState::True, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_COMMON_INSTALL, str))
    {
      ret.commonInstallRoot = str;
    }
    if (winRegistry::TryGetRegistryValue(TriState::True, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_COMMON_DATA, str))
    {
      ret.commonDataRoot = str;
    }
    if (winRegistry::TryGetRegistryValue(TriState::True, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_COMMON_CONFIG, str))
    {
      ret.commonConfigRoot = str;
    }
  }
  if (!common || AdminControlsUserConfig())
  {
    TriState shared = AdminControlsUserConfig() ? TriState::Undetermined : TriState::False;
    if (winRegistry::TryGetRegistryValue(shared, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_USER_ROOTS, str))
    {
      ret.userRoots = str;
    }
    if (winRegistry::TryGetRegistryValue(shared, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_USER_INSTALL, str))
    {
      ret.userInstallRoot = str;
    }
    if (winRegistry::TryGetRegistryValue(shared, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_USER_DATA, str))
    {
      ret.userDataRoot = str;
    }
    if (winRegistry::TryGetRegistryValue(shared, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_USER_CONFIG, str))
    {
      ret.userConfigRoot = str;
    }
  }

  return ret;
}

void SessionImpl::WriteRegistry(bool common, const StartupConfig & startupConfig)
{
  MIKTEX_ASSERT(!IsMiKTeXDirect());

  StartupConfig defaultConfig = DefaultConfig();

  // clean registry values
  if (common)
  {
    winRegistry::TryDeleteRegistryValue(TriState::True, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_COMMON_ROOTS);
    winRegistry::TryDeleteRegistryValue(TriState::True, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_COMMON_INSTALL);
    winRegistry::TryDeleteRegistryValue(TriState::True, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_COMMON_DATA);
    winRegistry::TryDeleteRegistryValue(TriState::True, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_COMMON_CONFIG);
#if ADMIN_CONTROLS_USER_CONFIG
    if (AdminControlsUserConfig())
    {
      winRegistry::TryDeleteRegistryValue(TriState::True, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_USER_ROOTS);
      winRegistry::TryDeleteRegistryValue(TriState::True, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_USER_INSTALL);
      winRegistry::TryDeleteRegistryValue(TriState::True, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_USER_DATA);
      winRegistry::TryDeleteRegistryValue(TriState::True, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_USER_CONFIG);
    }
#endif
  }
  else
  {
    winRegistry::TryDeleteRegistryValue(TriState::False, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_USER_ROOTS);
    winRegistry::TryDeleteRegistryValue(TriState::False, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_USER_INSTALL);
    winRegistry::TryDeleteRegistryValue(TriState::False, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_USER_DATA);
    winRegistry::TryDeleteRegistryValue(TriState::False, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_USER_CONFIG);
  }

  if (common)
  {
    if (!startupConfig.commonRoots.empty()
      && startupConfig.commonRoots != startupConfig.commonInstallRoot)
    {
      winRegistry::SetRegistryValue(TriState::True, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_COMMON_ROOTS, startupConfig.commonRoots.c_str());
    }
    if (!startupConfig.commonInstallRoot.Empty()
      && startupConfig.commonInstallRoot != defaultConfig.commonInstallRoot)
    {
      winRegistry::SetRegistryValue(TriState::True, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_COMMON_INSTALL, startupConfig.commonInstallRoot.Get());
    }
    if (!startupConfig.commonDataRoot.Empty()
      && startupConfig.commonDataRoot != defaultConfig.commonDataRoot)
    {
      winRegistry::SetRegistryValue(TriState::True, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_COMMON_DATA, startupConfig.commonDataRoot.Get());
    }
    if (!startupConfig.commonConfigRoot.Empty()
      && startupConfig.commonConfigRoot != defaultConfig.commonConfigRoot)
    {
      winRegistry::SetRegistryValue(TriState::True, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_COMMON_CONFIG, startupConfig.commonConfigRoot.Get());
    }
  }
  if (!common || AdminControlsUserConfig())
  {
    TriState shared = AdminControlsUserConfig() ? TriState::Undetermined : TriState::False;
    if (!startupConfig.userRoots.empty()
      && startupConfig.userRoots != startupConfig.userInstallRoot)
    {
      winRegistry::SetRegistryValue(shared, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_USER_ROOTS, startupConfig.userRoots.c_str());
    }
    if (!startupConfig.userInstallRoot.Empty()
      && startupConfig.userInstallRoot != defaultConfig.userInstallRoot)
    {
      winRegistry::SetRegistryValue(shared, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_USER_INSTALL, startupConfig.userInstallRoot.Get());
    }
    if (!startupConfig.userDataRoot.Empty()
      && startupConfig.userDataRoot != defaultConfig.userDataRoot)
    {
      winRegistry::SetRegistryValue(shared, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_USER_DATA, startupConfig.userDataRoot.Get());
    }
    if (!startupConfig.userConfigRoot.Empty()
      && startupConfig.userConfigRoot != defaultConfig.userConfigRoot)
    {
      winRegistry::SetRegistryValue(shared, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_USER_CONFIG, startupConfig.userConfigRoot.Get());
    }
  }
}

bool SessionImpl::GetAcrobatFontDir(PathName & path)
{
  if (!flags.test((size_t)InternalFlag::CachedAcrobatFontDir))
  {
    flags.set((size_t)InternalFlag::CachedAcrobatFontDir);

    const wchar_t * const ACRORD32 = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\AcroRd32.exe";

    wstring pathExe;

    if (!winRegistry::TryGetRegistryValue(HKEY_LOCAL_MACHINE, ACRORD32, L"", pathExe))
    {
      return false;
    }

    PathName dir(pathExe);
    dir.RemoveFileSpec();

    PathName fontDir;

    // try Acrobat Reader 3.0
    fontDir.Set(dir.Get(), "FONTS", nullptr);
    if (!Directory::Exists(fontDir))
    {
      // try Acrobat Reader 4.0
      fontDir.Set(dir.Get(), "..\\Resource\\Font", nullptr);
      if (!Directory::Exists(fontDir))
      {
	return false;
      }
    }

    RemoveDirectoryDelimiter(fontDir.GetData());

    acrobatFontDir = GetFullPath(fontDir.Get());
  }

  if (acrobatFontDir.GetLength() == 0)
  {
    return false;
  }

  path = acrobatFontDir;

  return true;
}

bool SessionImpl::GetATMFontDir(PathName & path)
{
  if (!flags.test((size_t)InternalFlag::CachedAtmFontDir))
  {
    flags.set((size_t)InternalFlag::CachedAtmFontDir);

    const wchar_t * const ATMSETUP = L"SOFTWARE\\Adobe\\Adobe Type Manager\\Setup";

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

    atmFontDir = GetFullPath(fontDir.Get());
  }

  if (atmFontDir.GetLength() == 0)
  {
    return false;
  }

  path = atmFontDir;

  return true;
}

MIKTEXSTATICFUNC(bool) GetPsFontDirectory(PathName & path)
{
  wchar_t szWinDir[BufferSizes::MaxPath];

  if (GetWindowsDirectoryW(szWinDir, BufferSizes::MaxPath) == 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetWindowsDirectoryW");
  }

  char szWinDrive[BufferSizes::MaxPath];

  PathName::Split(PathName(szWinDir).Get(), szWinDrive, BufferSizes::MaxPath, nullptr, 0, nullptr, 0, nullptr, 0);

  PathName path_(szWinDrive, "\\psfonts", nullptr, nullptr);

  if (!Directory::Exists(path_))
  {
    return false;
  }

  path = path_;

  return true;
}

bool SessionImpl::GetPsFontDirs(string & psFontDirs)
{
  if (!flags.test((size_t)InternalFlag::CachedPsFontDirs))
  {
    flags.set((size_t)InternalFlag::CachedPsFontDirs);
    PathName path;
    if (GetATMFontDir(path) || GetPsFontDirectory(path))
    {
      if (!this->psFontDirs.empty())
      {
	this->psFontDirs += PathName::PathNameDelimiter;
      }
      this->psFontDirs += path.Get();
    }
    if (GetAcrobatFontDir(path))
    {
      if (!this->psFontDirs.empty())
      {
	this->psFontDirs += PathName::PathNameDelimiter;
      }
      this->psFontDirs += path.Get();
    }
  }

  if (this->psFontDirs.empty())
  {
    return false;
  }

  psFontDirs = this->psFontDirs;

  return true;
}

bool SessionImpl::GetTTFDirs(string & ttfDirs)
{
  if (!flags.test((size_t)InternalFlag::CachedTtfDirs))
  {
    flags.set((size_t)InternalFlag::CachedTtfDirs);
    PathName path;
    if (GetWindowsFontsDirectory(path))
    {
      if (!this->ttfDirs.empty())
      {
	this->ttfDirs += PathName::PathNameDelimiter;;
      }
      this->ttfDirs += path.Get();
    }
  }

  if (this->ttfDirs.empty())
  {
    return false;
  }

  ttfDirs = this->ttfDirs;

  return true;
}

bool SessionImpl::GetOTFDirs(string & otfDirs)
{
  if (!flags.test((size_t)InternalFlag::CachedOtfDirs))
  {
    flags.set((size_t)InternalFlag::CachedOtfDirs);
    PathName path;
    if (GetWindowsFontsDirectory(path))
    {
      if (!this->otfDirs.empty())
      {
	this->otfDirs += PathName::PathNameDelimiter;
      }
      this->otfDirs += path.Get();
    }
  }

  if (this->otfDirs.empty())
  {
    return false;
  }

  otfDirs = this->otfDirs;

  return true;
}

void Session::FatalWindowsError(const string & functionName, unsigned long errorCode, const MiKTeXException::KVMAP & info, const SourceLocation & sourceLocation)
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
  switch (errorCode)
  {
  case ERROR_ACCESS_DENIED:
    throw UnauthorizedAccessException(programInvocationName, errorMessage, info, sourceLocation);
  case ERROR_FILE_NOT_FOUND:
  case ERROR_PATH_NOT_FOUND:
    throw FileNotFoundException(programInvocationName, errorMessage, info, sourceLocation);
  case ERROR_SHARING_VIOLATION:
    throw SharingViolationException(programInvocationName, errorMessage, info, sourceLocation);
  default:
    throw MiKTeXException(programInvocationName, errorMessage, info, sourceLocation);
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
  HWND hwnd = HtmlHelpW(hWnd, pathHelpFile.ToWideCharString().c_str(), HH_HELP_CONTEXT, topic);
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

bool SessionImpl::IsFileAlreadyOpen(const char * lpszFileName)
{
  MIKTEX_ASSERT_STRING(lpszFileName);

  unsigned long error = NO_ERROR;

  HANDLE hFile = CreateFileW(PathName(lpszFileName).ToWideCharString().c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

  if (hFile == INVALID_HANDLE_VALUE)
  {
    error = ::GetLastError();
  }
  else
  {
    if (!CloseHandle(hFile))
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("CloseHandle", "filename", lpszFileName);
    }
  }

  return hFile == INVALID_HANDLE_VALUE && error == ERROR_SHARING_VIOLATION;
}

void SessionImpl::ScheduleFileRemoval(const char * lpszFileName)
{
  MIKTEX_ASSERT_STRING(lpszFileName);
  if (IsMiKTeXPortable())
  {
    // todo
    return;
  }
  trace_files->WriteFormattedLine("core", T_("scheduling removal of %s"), Q_(lpszFileName));
  if (!MoveFileExW(PathName(lpszFileName).ToWideCharString().c_str(), 0, MOVEFILE_DELAY_UNTIL_REBOOT))
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("MoveFileExW", "filename", lpszFileName);
  }
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

  TOKEN_GROUPS * ptg = reinterpret_cast<TOKEN_GROUPS *>(_alloca(cbTokenGroups));

  if (ptg == nullptr)
  {
    return false;
  }

  if (!GetTokenInformation(hThread, TokenGroups, ptg, cbTokenGroups, &cbTokenGroups))
  {
    return false;
  }

  SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;

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

bool SessionImpl::IsUserAPowerUser()
{
  if (isUserAPowerUser == TriState::Undetermined)
  {
    if (IsUserMemberOfGroup(DOMAIN_ALIAS_RID_POWER_USERS))
    {
      isUserAPowerUser = TriState::True;
    }
    else
    {
      isUserAPowerUser = TriState::False;
    }
  }
  return isUserAPowerUser == TriState::True ? true : false;
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
      if (WindowsVersion::IsWindowsVistaOrGreater() && !RunningElevated())
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

bool SessionImpl::RunningAsPowerUser()
{
  if (runningAsPowerUser == TriState::Undetermined)
  {
    if (IsUserAPowerUser())
    {
      if (WindowsVersion::IsWindowsVistaOrGreater() && !RunningElevated())
      {
	runningAsPowerUser = TriState::False;
      }
      else
      {
	runningAsPowerUser = TriState::True;
      }
    }
    else
    {
      runningAsPowerUser = TriState::False;
    }
  }
  return runningAsPowerUser == TriState::True ? true : false;
}
