/* miktex.cpp:

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
#include "miktex/Core/Environment.h"
#include "miktex/Core/Paths.h"
#include "miktex/Core/Registry.h"
#include "miktex/Core/Urls.h"

#if defined(MIKTEX_WINDOWS)
#  include "miktex/Core/win/WindowsVersion.h"
#endif

#include "core-version.h"

#include "Session/SessionImpl.h"

#if defined(MIKTEX_WINDOWS)
#  include "win/winRegistry.h"
#endif

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace std;

MIKTEXSTATICFUNC(bool) IsGoodTempDirectory(const char * lpszPath)
{
  return Utils::IsAbsolutePath(lpszPath) && Directory::Exists(lpszPath);
}

PathName SessionImpl::GetTempDirectory()
{
  // 1: try MiKTeX temp directory
  {
    string tempDirectory;
    if (GetSessionValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_TEMPDIR, tempDirectory, nullptr) && IsGoodTempDirectory(tempDirectory.c_str()))
    {
      return tempDirectory;
    }
  }

  // 2: try designated temp directory
  {
    PathName tempDirectory;
    tempDirectory.SetToTempDirectory();
    if (IsGoodTempDirectory(tempDirectory.Get()))
    {
      return tempDirectory;
    }
  }

  MIKTEX_FATAL_ERROR(T_("No suitable temporary directory found."));
}


void SessionImpl::RegisterLibraryTraceStreams()
{
  TraceCallback * callback = initInfo.GetTraceCallback();
  trace_access = TraceStream::Open(MIKTEX_TRACE_ACCESS, callback);
  trace_config = TraceStream::Open(MIKTEX_TRACE_CONFIG, callback);
  trace_core = TraceStream::Open(MIKTEX_TRACE_CORE, callback);
  trace_env = TraceStream::Open(MIKTEX_TRACE_ENV, callback);
  trace_error = TraceStream::Open(MIKTEX_TRACE_ERROR, callback);
  trace_files = TraceStream::Open(MIKTEX_TRACE_FILES, callback);
  trace_filesearch = TraceStream::Open(MIKTEX_TRACE_FILESEARCH, callback);
  trace_fndb = TraceStream::Open(MIKTEX_TRACE_FNDB, callback);
  trace_fonts = TraceStream::Open(MIKTEX_TRACE_FONTINFO, callback);
  trace_mem = TraceStream::Open(MIKTEX_TRACE_MEM, callback);
  trace_mmap = TraceStream::Open(MIKTEX_TRACE_MMAP, callback);
  trace_packages = TraceStream::Open(MIKTEX_TRACE_PACKAGES, callback);
  trace_process = TraceStream::Open(MIKTEX_TRACE_PROCESS, callback);
  trace_tempfile = TraceStream::Open(MIKTEX_TRACE_TEMPFILE, callback);
  trace_time = TraceStream::Open(MIKTEX_TRACE_TIME, callback);
  trace_values = TraceStream::Open(MIKTEX_TRACE_VALUES, callback);
};

void SessionImpl::UnregisterLibraryTraceStreams()
{
  trace_access->Close();
  trace_config->Close();
  trace_core->Close();
  trace_error->Close();
  trace_env->Close();
  trace_files->Close();
  trace_filesearch->Close();
  trace_fndb->Close();
  trace_fonts->Close();
  trace_mem->Close();
  trace_packages->Close();
  trace_process->Close();
  trace_tempfile->Close();
  trace_time->Close();
  trace_values->Close();
}

PathName SessionImpl::GetSpecialPath(SpecialPath specialPath)
{
  PathName path;
  switch (specialPath)
  {
  case SpecialPath::BinDirectory:
    path = GetBinDirectory();
    break;
  case SpecialPath::InternalBinDirectory:
    path = GetBinDirectory();
    path /= "internal";
    break;
  case SpecialPath::CommonInstallRoot:
    path = GetRootDirectory(GetCommonInstallRoot());
    break;
  case SpecialPath::UserInstallRoot:
    path = GetRootDirectory(GetUserInstallRoot());
    break;
  case SpecialPath::InstallRoot:
    path = GetRootDirectory(GetInstallRoot());
    break;
  case SpecialPath::DistRoot:
    path = GetRootDirectory(GetDistRoot());
    break;
  case SpecialPath::CommonDataRoot:
    path = GetRootDirectory(GetCommonDataRoot());
    break;
  case SpecialPath::UserDataRoot:
    path = GetRootDirectory(GetUserDataRoot());
    break;
  case SpecialPath::DataRoot:
    path = GetRootDirectory(GetDataRoot());
    break;
  case SpecialPath::CommonConfigRoot:
    path = GetRootDirectory(GetCommonConfigRoot());
    break;
  case SpecialPath::UserConfigRoot:
    path = GetRootDirectory(GetUserConfigRoot());
    break;
  case SpecialPath::ConfigRoot:
    path = GetRootDirectory(GetConfigRoot());
    break;
  case SpecialPath::PortableRoot:
    if (!IsMiKTeXPortable())
    {
      MIKTEX_UNEXPECTED();
    }
    path = GetRootDirectory(GetInstallRoot());
    break;
  case SpecialPath::PortableMount:
    if (!IsMiKTeXPortable())
    {
      MIKTEX_UNEXPECTED();
    }
#if MIKTEX_WINDOWS
    // FIXME: ANSI
    if (!GetVolumePathNameA(GetRootDirectory(GetInstallRoot()).Get(), path.GetData(), path.GetCapacity()))
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("GetVolumePathNameA", "path", GetRootDirectory(GetInstallRoot()).ToString());
    }
#else
    MIKTEX_UNEXPECTED();
#endif
    break;
  default:
    MIKTEX_UNEXPECTED();
    break;
  }
  return path;
}

PathName SessionImpl::GetMyLocation(bool canonicalized)
{
#if defined(MIKTEX_WINDOWS) && defined(MIKTEX_CORE_SHARED)
  return GetDllPathName(canonicalized).RemoveFileSpec();
#else
  return GetMyProgramFile(canonicalized).RemoveFileSpec();
#endif
}

MIKTEXINTERNALFUNC(PathName) GetHomeDirectory()
{
#if defined(MIKTEX_WINDOWS)
  string homeDrive;
  string homePath;
  if (Utils::GetEnvironmentString("HOMEDRIVE", homeDrive)
    && Utils::GetEnvironmentString("HOMEPATH", homePath))
  {
    return homeDrive + homePath;
  }
#endif
  PathName ret;
  if (Utils::GetEnvironmentString("HOME", ret))
  {
    return ret;
  }
#if defined(MIKTEX_WINDOWS)
  if (GetUserProfileDirectory(ret))
  {
    return ret;
  }
  wchar_t szWinDir[_MAX_PATH];
  unsigned int n = GetWindowsDirectoryW(szWinDir, _MAX_PATH);
  if (n == 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("GetWindowsDirectoryW");
  }
  else if (n >= _MAX_PATH)
  {
    BUF_TOO_SMALL();
  }
  ret = szWinDir;
  return ret;
#else
  MIKTEX_UNEXPECTED();
#endif
}

MIKTEXSTATICFUNC(int) magstep(int n, int bdpi)
{
  double t;
  int neg = 0;
  if (n < 0)
  {
    neg = 1;
    n = -n;
  }
  if (n & 1)
  {
    n &= ~1;
    t = 1.095445115;
  }
  else
  {
    t = 1.0;
  }
  while (n > 8)
  {
    n -= 8;
    t = t * 2.0736;
  }
  while (n > 0)
  {
    n -= 2;
    t = t * 1.2;
  }
  if (neg)
  {
    return static_cast<int>(0.5 + bdpi / t);
  }
  else
  {
    return static_cast<int>(0.5 + bdpi * t);
  }
}

string SessionImpl::MakeMakePkCommandLine(const char * lpszFontName, int dpi, int baseDpi, const char * lpszMfMode, PathName & fileName, TriState enableInstaller)
{
  MIKTEX_ASSERT_STRING(lpszFontName);
  MIKTEX_ASSERT_STRING_OR_NIL(lpszMfMode);
  MIKTEX_ASSERT_CHAR_BUFFER(lpszArguments, maxArguments);

  MIKTEX_ASSERT(baseDpi != 0);

  if (!FindFile(MIKTEX_MAKEPK_EXE, FileType::EXE, fileName))
  {
    MIKTEX_UNEXPECTED();
  }

  int m = 0;
  int n;

  if (dpi < baseDpi)
  {
    for (;;)
    {
      --m;
      n = magstep(m, baseDpi);
      if (n == dpi)
      {
        break;
      }
      if (n < dpi || m < -40)
      {
        m = 9999;
        break;
      }
    }
  }
  else if (dpi > baseDpi)
  {
    for (;;)
    {
      ++m;
      n = magstep(m, baseDpi);
      if (n == dpi)
      {
        break;
      }
      if (n > dpi || m > 40)
      {
        m = 9999;
        break;
      }
    }
  }

  string strMagStep;

  if (m == 9999)
  {
    // a+b/c
    strMagStep = std::to_string(dpi / baseDpi);
    strMagStep += '+';
    strMagStep += std::to_string(dpi % baseDpi);
    strMagStep += '/';
    strMagStep += std::to_string(baseDpi);
  }
  else if (m >= 0)
  {
    // magstep(a.b)
    strMagStep = "magstep(";
    strMagStep += std::to_string(m / 2);
    strMagStep += '.';
    strMagStep += std::to_string((m & 1) * 5);
    strMagStep += ')';
  }
  else
  {
    // magstep(-a.b)
    strMagStep = "magstep(-";
    strMagStep += std::to_string((-m) / 2);
    strMagStep += '.';
    strMagStep += std::to_string((m & 1) * 5);
    strMagStep += ')';
  }

  string cmdline;
  cmdline.reserve(256);

  switch (enableInstaller)
  {
  case TriState::False:
    cmdline += " --disable-installer";
    break;
  case TriState::True:
    cmdline += " --enable-installer";
    break;
  default:
    break;
  }

  cmdline += " --verbose";
  cmdline += ' '; cmdline += lpszFontName;
  cmdline += ' '; cmdline += std::to_string(dpi);
  cmdline += ' '; cmdline += std::to_string(baseDpi);
  cmdline += ' '; cmdline += strMagStep;

  if (lpszMfMode != nullptr)
  {
    cmdline += ' '; cmdline += lpszMfMode;
  }

  return cmdline;
}

bool SessionImpl::EnableFontMaker(bool enable)
{

  bool prev = makeFonts;
  makeFonts = enable;
  return prev;
}

bool SessionImpl::GetMakeFontsFlag()
{
  return makeFonts;
}

#if HAVE_MIKTEX_USER_INFO
bool SessionImpl::TryGetMiKTeXUserInfo(MiKTeXUserInfo & info)
{
  static TriState haveResult = TriState::Undetermined;
  static MiKTeXUserInfo result;
  if (haveResult == TriState::Undetermined)
  {
    haveResult = TriState::False;
    string userInfoFile;
    if (!TryGetConfigValue(nullptr, MIKTEX_REGVAL_USERINFO_FILE, userInfoFile))
    {
      return false;
    }
    if (!File::Exists(userInfoFile))
    {
      return false;
    }
    unique_ptr<Cfg> cfg = Cfg::Create();
    cfg->Read(userInfoFile, true);
    if (!cfg->TryGetValue("user", "id", result.id))
    {
      return false;
    }
    if (!cfg->TryGetValue("user", "name", result.name))
    {
      return false;
    }
    if (!cfg->TryGetValue("user", "organization", result.organization))
    {
      result.organization = "";
    }
    if (!cfg->TryGetValue("user", "email", result.email))
    {
      result.email = "";
    }
    string str;
    if (cfg->TryGetValue("membership", "expirationdate", str))
    {
      int year, month, day;
      if (sscanf(str.c_str(), "%d-%d-%d", &year, &month, &day) == 3
        && year >= 1970
        && month >= 1 && month <= 12
        && day >= 1 && day <= 31)
      {
        struct tm date;
        memset(&date, 0, sizeof(date));
        date.tm_year = year - 1900;
        date.tm_mon = month - 1;
        date.tm_mday = day;
        date.tm_hour = 23;
        date.tm_min = 59;
        date.tm_sec = 59;
        date.tm_isdst = -1;
        result.expirationDate = mktime(&date);
      }
    }
    if (cfg->TryGetValue("membership", "level", str))
    {
      if (Utils::EqualsIgnoreCase(str.c_str(), "individual"))
      {
        result.level = MiKTeXUserInfo::Individual;
      }
      else
      {
        result.level = atoi(str.c_str());
      }
    }
    if (cfg->TryGetValue("membership", "role", str))
    {
      if (Utils::EqualsIgnoreCase(str.c_str(), "developer"))
      {
        result.role = MiKTeXUserInfo::Developer;
      }
      else if (Utils::EqualsIgnoreCase(str.c_str(), "contributor"))
      {
        result.role = MiKTeXUserInfo::Contributor;
      }
      else if (Utils::EqualsIgnoreCase(str.c_str(), "sponsor"))
      {
        result.role = MiKTeXUserInfo::Sponsor;
      }
      else if (Utils::EqualsIgnoreCase(str.c_str(), "knownuser"))
      {
        result.role = MiKTeXUserInfo::KnownUser;
      }
      else
      {
        result.role = atoi(str.c_str());
      }
    }
    haveResult = TriState::True;
  }
  if (haveResult == TriState::True)
  {
    info = result;
    return true;
  }
  return false;
}
#endif

#if HAVE_MIKTEX_USER_INFO
MiKTeXUserInfo SessionImpl::RegisterMiKTeXUser(const MiKTeXUserInfo & info)
{
  Utils::ShowWebPage(MIKTEX_URL_WWW_GIVE_BACK);
  // TODO
  throw new OperationCancelledException();
}
#endif

MIKTEXINTERNALFUNC(bool) GetEnvironmentString(const char * lpszName, string & value)
{
#if defined(MIKTEX_WINDOWS)
  wchar_t * lpszValue = _wgetenv(UW_(lpszName));
  if (lpszValue == nullptr)
  {
    return false;
  }
  else
  {
    value = WU_(lpszValue);
    return true;
  }
#else
  const char * lpszValue = getenv(lpszName);
  if (lpszValue == nullptr)
  {
    return false;
  }
  else
  {
    value = lpszValue;
    return true;
  }
#endif
}

MIKTEXINTERNALFUNC(bool) HaveEnvironmentString(const char * lpszName)
{
  string value;
  return GetEnvironmentString(lpszName, value);
}

namespace {
#define PUBLIC_KEY_NAME DC13376B_CCAB_4B4B_B795_6AB245A77596
#define miktex_der PUBLIC_KEY_NAME
#include "miktex.der.h"
}

MIKTEXINTERNALFUNC(Botan::Public_Key *) LoadPublicKey()
{
  return Botan::X509::load_key(Botan::MemoryVector<Botan::byte>(&PUBLIC_KEY_NAME[0], sizeof(PUBLIC_KEY_NAME)));
}

void SessionImpl::SetCWDEnv()
{
  string str;
  str.reserve(256);
  for (const PathName & dir : inputDirectories)
  {
    if (!str.empty())
    {
      str += PATH_DELIMITER;
    }
    str += dir.ToString();
  }
  Utils::SetEnvironmentString(MIKTEX_ENV_CWD_LIST, str.c_str());
}

void SessionImpl::AddInputDirectory(const char * lpszPath, bool atEnd)
{
  MIKTEX_ASSERT_STRING(lpszPath);

  if (!Utils::IsAbsolutePath(lpszPath))
  {
    INVALID_ARGUMENT("path", lpszPath);
  }

  // clear the search path cache
  ClearSearchVectors();

  if (atEnd)
  {
    inputDirectories.push_back(lpszPath);
  }
  else
  {
    inputDirectories.push_front(lpszPath);
  }

#if 1
  SetCWDEnv();
#endif
}

bool SessionImpl::GetWorkingDirectory(unsigned n, PathName & path)
{
  if (n == inputDirectories.size() + 1)
  {
    return false;
  }
  if (n > inputDirectories.size() + 1)
  {
    INVALID_ARGUMENT("index", std::to_string(n));
  }
  path = n == 0 ? startDirectory : inputDirectories[n - 1];
  return true;
}
