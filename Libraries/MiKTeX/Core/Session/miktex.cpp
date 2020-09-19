/* miktex.cpp:

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

// FIXME: must come first
#include "core-version.h"

#include <miktex/Core/ConfigNames>
#include <miktex/Core/Directory>
#include <miktex/Core/Environment>
#include <miktex/Core/FileStream>
#include <miktex/Core/Paths>
#include <miktex/Core/Urls>
#include <miktex/Trace/Trace>

#include "internal.h"

#if defined(MIKTEX_WINDOWS)
#  include "miktex/Core/win/WindowsVersion.h"
#endif

#include "Session/SessionImpl.h"

#if defined(MIKTEX_WINDOWS)
#  include "win/winRegistry.h"
#endif

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

MIKTEXSTATICFUNC(bool) IsGoodTempDirectory(const char* lpszPath)
{
  return PathNameUtil::IsAbsolutePath(lpszPath) && Directory::Exists(PathName(lpszPath));
}

PathName SessionImpl::GetTempDirectory()
{
  // 1: try MiKTeX temp directory
  {
    string tempDirectory;
    if (GetSessionValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_TEMPDIR, tempDirectory, nullptr) && IsGoodTempDirectory(tempDirectory.c_str()))
    {
      return PathName(tempDirectory);
    }
  }

  // 2: try designated temp directory
  {
    PathName tempDirectory;
    tempDirectory.SetToTempDirectory();
    if (IsGoodTempDirectory(tempDirectory.GetData()))
    {
      return tempDirectory;
    }
  }

  MIKTEX_FATAL_ERROR(T_("No suitable temporary directory found."));
}


void SessionImpl::RegisterLibraryTraceStreams()
{
  TraceCallback* callback = initInfo.GetTraceCallback();
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
  trace_process = TraceStream::Open(MIKTEX_TRACE_PROCESS, callback);
  trace_stopwatch = TraceStream::Open(MIKTEX_TRACE_STOPWATCH, callback);
  trace_tempfile = TraceStream::Open(MIKTEX_TRACE_TEMPFILE, callback);
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
  trace_process->Close();
  trace_stopwatch->Close();
  trace_tempfile->Close();
  trace_values->Close();
}

PathName SessionImpl::GetSpecialPath(SpecialPath specialPath)
{
  PathName path;
  switch (specialPath)
  {
  case SpecialPath::BinDirectory:
    path = GetBinDirectory(true);
    break;
  case SpecialPath::InternalBinDirectory:
#if defined(MIKTEX_WINDOWS)
    // FIXME: hard-coded sub-directory
    path = GetSpecialPath(SpecialPath::BinDirectory) / PathName("internal");
#else
    path = GetMyPrefix(true) / PathName(MIKTEX_INTERNAL_BINARY_DESTINATION_DIR);
#endif
    break;
  case SpecialPath::LinkTargetDirectory:
#if defined(MIKTEX_WINDOWS)
    path = GetSpecialPath(SpecialPath::BinDirectory);
#else
    if (IsSharedSetup())
    {
      path = GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMONLINKTARGETDIRECTORY, ConfigValue(MIKTEX_SYSTEM_LINK_TARGET_DIR)).GetString();
    }
    else
    {
      string s = GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USERLINKTARGETDIRECTORY, ConfigValue(MIKTEX_USER_LINK_TARGET_DIR)).GetString();
      auto p = Utils::ExpandTilde(s);
      if (p.first)
      {
        path = p.second;
      }
      else
      {
        path = s;
      }
    }
#endif
    break;
  case SpecialPath::LogDirectory:
    if (IsAdminMode())
    {
#if defined(MIKTEX_UNIX)
      // FIXME: hard-coded sub-directory
      path = GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMONLOGDIRECTORY, ConfigValue((PathName(MIKTEX_SYSTEM_VAR_LOG_DIR) / PathName("miktex")).ToString())).GetString();
#else
      path = GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_COMMONLOGDIRECTORY, ConfigValue((GetSpecialPath(SpecialPath::DataRoot) / PathName(MIKTEX_PATH_MIKTEX_LOG_DIR)).ToString())).GetString();
#endif
    }
    else
    {
      path = GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_USERLOGDIRECTORY, ConfigValue((GetSpecialPath(SpecialPath::DataRoot) / PathName(MIKTEX_PATH_MIKTEX_LOG_DIR)).ToString())).GetString();
    }
    break;
  case SpecialPath::CommonInstallRoot:
    path = GetRootDirectoryPath(GetCommonInstallRoot());
    break;
  case SpecialPath::UserInstallRoot:
    path = GetRootDirectoryPath(GetUserInstallRoot());
    break;
  case SpecialPath::InstallRoot:
    path = GetRootDirectoryPath(GetInstallRoot());
    break;
  case SpecialPath::DistRoot:
    path = GetDistRootDirectory();
    break;
  case SpecialPath::CommonDataRoot:
    path = GetRootDirectoryPath(GetCommonDataRoot());
    break;
  case SpecialPath::UserDataRoot:
    path = GetRootDirectoryPath(GetUserDataRoot());
    break;
  case SpecialPath::DataRoot:
    path = GetRootDirectoryPath(GetDataRoot());
    break;
  case SpecialPath::CommonConfigRoot:
    path = GetRootDirectoryPath(GetCommonConfigRoot());
    break;
  case SpecialPath::UserConfigRoot:
    path = GetRootDirectoryPath(GetUserConfigRoot());
    break;
  case SpecialPath::ConfigRoot:
    path = GetRootDirectoryPath(GetConfigRoot());
    break;
  case SpecialPath::PortableRoot:
    if (!IsMiKTeXPortable())
    {
      MIKTEX_UNEXPECTED();
    }
    path = GetRootDirectoryPath(GetInstallRoot());
    break;
  case SpecialPath::PortableMount:
    if (!IsMiKTeXPortable())
    {
      MIKTEX_UNEXPECTED();
    }
#if MIKTEX_WINDOWS
    // FIXME: ANSI
    if (!GetVolumePathNameA(GetRootDirectoryPath(GetInstallRoot()).GetData(), path.GetData(), static_cast<DWORD>(path.GetCapacity())))
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("GetVolumePathNameA", "path", GetRootDirectoryPath(GetInstallRoot()).ToString());
    }
#else
    MIKTEX_UNEXPECTED();
#endif
    break;
#if defined(MIKTEX_MACOS_BUNDLE)
  case SpecialPath::MacOsDirectory:
    path = (GetMyLocation(true) / PathName("..") / PathName("MacOS")).MakeFullyQualified();
    break;
#endif
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
  PathName result;
#if defined(MIKTEX_WINDOWS)
  if (!GetUserProfileDirectory(result))
  {
    string homeDrive;
    string homePath;
    if (Utils::GetEnvironmentString("HOMEDRIVE", homeDrive)
        && Utils::GetEnvironmentString("HOMEPATH", homePath))
    {
      result = homeDrive + homePath;
    }
    else
    {
      result = "";
    }
  }
#else
  if (!Utils::GetEnvironmentString("HOME", result))
  {
    result = "";
  }
#endif
  if (result.Empty())
  {
    MIKTEX_FATAL_ERROR(T_("Home directory is not defined."));
  }
  if (!Directory::Exists(result))
  {
    MIKTEX_FATAL_ERROR_2(T_("Home directory ({path}) does not exist."), "path", result.ToString());
  }
  return result;
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

vector<string> SessionImpl::MakeMakePkCommandLine(const string& fontName, int dpi, int baseDpi, const string& mfMode, PathName& fileName, TriState enableInstaller)
{
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

  vector<string> args{ fileName.GetFileNameWithoutExtension().ToString() };

  if (IsAdminMode())
  {
    args.push_back("--miktex-admin");
  }

  switch (enableInstaller)
  {
  case TriState::False:
    args.push_back("--disable-installer");
    break;
  case TriState::True:
    args.push_back("--enable-installer");
    break;
  default:
    break;
  }

  args.push_back("--verbose");
  args.push_back(fontName);
  args.push_back(std::to_string(dpi));
  args.push_back(std::to_string(baseDpi));
  args.push_back(strMagStep);

  if (!mfMode.empty())
  {
    args.push_back(mfMode);
  }

  return args;
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
bool SessionImpl::TryGetMiKTeXUserInfo(MiKTeXUserInfo& info)
{
  static TriState haveResult = TriState::Undetermined;
  static MiKTeXUserInfo result;
  if (haveResult == TriState::Undetermined)
  {
    haveResult = TriState::False;
    string userInfoFile;
    if (!TryGetConfigValue(MIKTEX_CONFIG_SECTION_GENERAL, MIKTEX_CONFIG_VALUE_USERINFO_FILE, userInfoFile))
    {
      return false;
    }
    if (!File::Exists(PathName(userInfoFile)))
    {
      return false;
    }
    unique_ptr<Cfg> cfg = Cfg::Create();
    cfg->Read(PathName(userInfoFile), true);
    if (!cfg->TryGetValueAsString("user", "id", result.userid))
    {
      result.userid = "";
    }
    if (!cfg->TryGetValueAsString("user", "name", result.name))
    {
      result.name = "";
    }
    if (!cfg->TryGetValueAsString("user", "organization", result.organization))
    {
      result.organization = "";
    }
    if (!cfg->TryGetValueAsString("user", "email", result.email))
    {
      result.email = "";
    }
    string str;
    int year, month, day;
    // FIXME: don't use sscanf
    if (cfg->TryGetValueAsString("membership", "expirationdate", str) && sscanf(str.c_str(), "%d-%d-%d", &year, &month, &day) == 3 && year >= 1970 && month >= 1 && month <= 12 && day >= 1 && day <= 31)
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
    else
    {
      result.expirationDate = static_cast<time_t>(-1);
    }
    if (cfg->TryGetValueAsString("membership", "level", str))
    {
      if (Utils::EqualsIgnoreCase(str, "individual"))
      {
        result.level = MiKTeXUserInfo::Individual;
      }
      else
      {
        result.level = std::stoi(str);
      }
    }
    else
    {
      result.level = 0;
    }
    result.role = 0;
    vector<string> roles;
    if (cfg->TryGetValueAsStringVector("membership", "roles[]", roles))
    {
      for (const auto& r : roles)
      {
        if (Utils::EqualsIgnoreCase(r, "developer"))
        {
          result.role |= MiKTeXUserInfo::Developer;
        }
        else if (Utils::EqualsIgnoreCase(r, "contributor"))
        {
          result.role |= MiKTeXUserInfo::Contributor;
        }
        else if (Utils::EqualsIgnoreCase(r, "sponsor"))
        {
          result.role |= MiKTeXUserInfo::Sponsor;
        }
        else if (Utils::EqualsIgnoreCase(r, "knownuser"))
        {
          result.role |= MiKTeXUserInfo::KnownUser;
        }
        else
        {
          result.role |= std::stoi(r);
        }
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
MiKTeXUserInfo SessionImpl::RegisterMiKTeXUser(const MiKTeXUserInfo& info)
{
  Utils::ShowWebPage(MIKTEX_URL_WWW_GIVE_BACK);
  // TODO
  throw new OperationCancelledException();
}
#endif

MIKTEXINTERNALFUNC(bool) GetEnvironmentString(const string& name, string& value)
{
#if defined(MIKTEX_WINDOWS)
  wchar_t* lpszValue = _wgetenv(UW_(name));
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
  const char* lpszValue = getenv(name.c_str());
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

MIKTEXINTERNALFUNC(bool) HaveEnvironmentString(const char* lpszName)
{
  string value;
  return GetEnvironmentString(lpszName, value);
}

namespace {
#define PUBLIC_KEY_NAME DC13376B_CCAB_4B4B_B795_6AB245A77596
#define miktex_der PUBLIC_KEY_NAME
#include "miktex.der.h"
}

MIKTEXINTERNALFUNC(CryptoLib) GetCryptoLib()
{
#if defined(ENABLE_OPENSSL)
  static bool initDone = false;
  if (!initDone)
  {
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    initDone = true;
  }
  return CryptoLib::OpenSSL;
#else
  return CryptoLib::None;
#endif
}

#if defined(ENABLE_OPENSSL)
extern "C" int OnOpenSSLError(const char* str, size_t len, void* u)
{
  CharBuffer<char>* message = reinterpret_cast<CharBuffer<char>*>(u);
  message->Append(str, len);
  return 1;
}

MIKTEXINTERNALFUNC(void) FatalOpenSSLError()
{
  CharBuffer<char> message;
  ERR_print_errors_cb(OnOpenSSLError, &message);
  MIKTEX_FATAL_ERROR(message.ToString());
}
#endif

#if defined(ENABLE_OPENSSL)
MIKTEXINTERNALFUNC(RSA_ptr) LoadPublicKey_OpenSSL(const PathName& publicKeyFile)
{
  BIO_ptr mem(BIO_new(BIO_s_mem()), BIO_free);
  if (mem == nullptr)
  {
    FatalOpenSSLError();
  }
  RSA* rsa;
  if (publicKeyFile.Empty())
  {
    if (BIO_write(mem.get(), &PUBLIC_KEY_NAME[0], sizeof(PUBLIC_KEY_NAME)) != sizeof(PUBLIC_KEY_NAME))
    {
      FatalOpenSSLError();
    }
    if (BIO_flush(mem.get()) != 1)
    {
      FatalOpenSSLError();
    }
    rsa = d2i_RSA_PUBKEY_bio(mem.get(), nullptr);
  }
  else
  {
    FileStream stream(File::Open(publicKeyFile, FileMode::Open, FileAccess::Read));
    rsa = PEM_read_RSA_PUBKEY(stream.GetFile(), nullptr, nullptr, nullptr);
  }
  if (rsa == nullptr)
  {
    FatalOpenSSLError();
  }
  return RSA_ptr(rsa, RSA_free);
}
#endif

void SessionImpl::SetCWDEnv()
{
  string str;
  str.reserve(256);
  for (const PathName& dir : inputDirectories)
  {
    if (!str.empty())
    {
      str += PATH_DELIMITER;
    }
    str += dir.ToString();
  }
  Utils::SetEnvironmentString(MIKTEX_ENV_CWD_LIST, str);
}

void SessionImpl::AddInputDirectory(const PathName& path, bool atEnd)
{
  if (!path.IsAbsolute())
  {
    INVALID_ARGUMENT("path", path.ToString());
  }

  // clear the search path cache
  ClearSearchVectors();

  if (atEnd)
  {
    inputDirectories.push_back(path);
  }
  else
  {
    inputDirectories.push_front(path);
  }

#if 1
  SetCWDEnv();
#endif
}

bool SessionImpl::GetWorkingDirectory(unsigned n, PathName& path)
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
