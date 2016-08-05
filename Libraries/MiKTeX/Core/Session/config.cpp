/* config.cpp: MiKTeX configuration settings

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

#include "miktex/Core/CSVList.h"
#include "miktex/Core/Directory.h"
#include "miktex/Core/Environment.h"
#include "miktex/Core/PathName.h"
#include "miktex/Core/Paths.h"
#include "miktex/Core/Registry.h"

#if defined(MIKTEX_WINDOWS)
#  include "win/winRegistry.h"
#endif

#include "Session/SessionImpl.h"
#include "Utils/inliners.h"

using namespace MiKTeX::Core;
using namespace std;

#if 0
struct ConfigMapping
{
  const char * lpszConfigSection;
  const char * lpszConfigValueName;
  const char * lpszEnvVarName;
#if defined(MIKTEX_WINDOWS)
  const char * lpszRegKey;
  const char * lpszRegValueName;
#endif
};
#endif

#if 0
namespace {
  const ConfigMapping configMappings[] = {
    {
      MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REMOTE_REPOSITORY, MIKTEX_ENV_REPOSITORY, }, };
}
#endif

#if 0
MIKTEXSTATICFUNC(const ConfigMapping *) FindConfigMapping(const char * lpszConfigSection, const char * lpszConfigValueName)
{
  for (size_t idx = 0; idx < sizeof(configMappings) / sizeof(configMappings); ++idx)
  {
    if ((Utils::EqualsIgnoreCase(configMappings[idx].lpszConfigSection, lpszConfigSection))
      && (StringCompare(configMappings[idx].lpszConfigValueName, lpszConfigValueName, true)) == 0)
    {
      return &configMappings[idx];
    }
  }
  return 0;
}
#endif

PathName SessionImpl::GetMyPrefix()
{
  PathName bindir = GetMyLocation(true);

  RemoveDirectoryDelimiter(bindir.GetData());

  PathName prefix(bindir);

  // /usr/local/bin => /usr/local
  // /usr/bin => /usr
  prefix.CutOffLastComponent();

  return prefix;
}

bool SessionImpl::FindStartupConfigFile(bool common, PathName & path)
{
  if (initInfo.GetOptions()[InitOption::NoConfigFiles])
  {
    return false;
  }

  string str;

  if (Utils::GetEnvironmentString(common ? MIKTEX_ENV_COMMON_STARTUP_FILE : MIKTEX_ENV_USER_STARTUP_FILE, str))
  {
    path = str;
    // don't check for existence; it's a fatal error (detected later)
    // if the environment value is incorrect
    return true;
  }

#if !NO_REGISTRY
  if (winRegistry::TryGetRegistryValue(common ? TriState::True : TriState::False, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_STARTUP_FILE, str))
  {
    // don't check for existence; it's a fatal error (detected later)
    // if the registry value is incorrect
    path = str;
    return true;
  }
#endif

  StartupConfig defaultStartupConfig = DefaultConfig();

  if (common)
  {
    // try the prefix of the bin directory
    PathName myloc = GetMyLocation(true);
    RemoveDirectoryDelimiter(myloc.GetData());
    PathName internalBindir(MIKTEX_PATH_INTERNAL_BIN_DIR);
    RemoveDirectoryDelimiter(internalBindir.GetData());
    PathName prefix;
    if (Utils::GetPathNamePrefix(myloc, internalBindir, prefix))
    {
      path = prefix;
      path /= MIKTEX_PATH_STARTUP_CONFIG_FILE;
      if (File::Exists(path))
      {
        return true;
      }
    }
    PathName bindir(MIKTEX_PATH_BIN_DIR);
    RemoveDirectoryDelimiter(bindir.GetData());
    if (Utils::GetPathNamePrefix(myloc, bindir, prefix))
    {
      path = prefix;
      path /= MIKTEX_PATH_STARTUP_CONFIG_FILE;
      if (File::Exists(path))
      {
        return true;
      }
    }
    // try /var/lib/miktex-texmf/miktex/config/miktexstartup.ini
    path = defaultStartupConfig.commonConfigRoot;
    path /= MIKTEX_PATH_STARTUP_CONFIG_FILE;
    if (File::Exists(path))
    {
      return true;
    }
#if MIKTEX_UNIX
    // try /usr/share/miktex-texmf/miktex/config/miktexstartup.ini
    prefix = GetMyPrefix();
    path = prefix;
    path /= MIKTEX_TEXMF;
    path /= MIKTEX_PATH_STARTUP_CONFIG_FILE;
    if (File::Exists(path))
    {
      return true;
    }
#endif
#if defined(MIKTEX_WINDOWS) && defined(_DEBUG)
    if (Utils::GetPathNamePrefix(myloc, "binlib/Debug", prefix))
    {
      path = prefix;
      path /= "miktexstartup.ini.debug";
      if (File::Exists(path))
      {
        return true;
      }
    }
#endif
  }
  else
  {
    // try $HOME/.miktex/miktex/config/miktexstartup.ini
    path = defaultStartupConfig.userConfigRoot;
    path /= MIKTEX_PATH_STARTUP_CONFIG_FILE;
    if (File::Exists(path))
    {
      return true;
    }
  }

  return false;
}

void Absolutize(string & paths, const PathName & relativeFrom)
{
  string result;
  for (CSVList path(paths, PATH_DELIMITER); path.GetCurrent() != nullptr; ++path)
  {
    if (!result.empty())
    {
      result += PATH_DELIMITER;
    }
    if (Utils::IsAbsolutePath(path.GetCurrent()))
    {
      result += path.GetCurrent();
    }
    else
    {
#if MIKTEX_WINDOWS
      MIKTEX_ASSERT(Utils::IsAbsolutePath(relativeFrom.Get()));
      PathName absPath(relativeFrom);
      absPath /= path.GetCurrent();
      PathName absPath2;
      MIKTEX_ASSERT(absPath2.GetCapacity() >= MAX_PATH);
      // FIXME: use wchar_t API
      if (!PathCanonicalizeA(absPath2.GetData(), absPath.Get()))
      {
        absPath2 = absPath;
      }
      result += absPath2.Get();
#else
      UNIMPLEMENTED();
#endif
    }
  }
  paths = result;
}

StartupConfig SessionImpl::ReadStartupConfigFile(bool common, const PathName & path)
{
  StartupConfig ret;

  unique_ptr<Cfg> pcfg(Cfg::Create());

  pcfg->Read(path);

  string str;

  if (pcfg->TryGetValue("Auto", "Config", str))
  {
    if (common)
    {
      ret.commonConfigRoot = "";
      ret.commonDataRoot = "";
      ret.commonInstallRoot = "";
    }
    else
    {
      ret.userConfigRoot = "";
      ret.userDataRoot = "";
      ret.userInstallRoot = "";
    }
    if (str == "Regular")
    {
      ret.config = MiKTeXConfiguration::Regular;
    }
    else if (str == "Portable")
    {
      ret.config = MiKTeXConfiguration::Portable;
    }
    else if (str == "Direct")
    {
      ret.config = MiKTeXConfiguration::Direct;
    }
    else
    {
      MIKTEX_UNEXPECTED();
    }
  }

  PathName relativeFrom(path);
  relativeFrom.RemoveFileSpec();

  if (common)
  {
    if (pcfg->TryGetValue("Paths", MIKTEX_REGVAL_COMMON_ROOTS, str))
    {
      Absolutize(str, relativeFrom);
      ret.commonRoots = str;
    }
    if (pcfg->TryGetValue("Paths", MIKTEX_REGVAL_COMMON_INSTALL, str))
    {
      Absolutize(str, relativeFrom);
      ret.commonInstallRoot = str;
    }
    if (pcfg->TryGetValue("Paths", MIKTEX_REGVAL_COMMON_DATA, str))
    {
      Absolutize(str, relativeFrom);
      ret.commonDataRoot = str;
    }
    if (pcfg->TryGetValue("Paths", MIKTEX_REGVAL_COMMON_CONFIG, str))
    {
      Absolutize(str, relativeFrom);
      ret.commonConfigRoot = str;
    }
  }
  if (!common || AdminControlsUserConfig())
  {
    if (pcfg->TryGetValue("Paths", MIKTEX_REGVAL_USER_ROOTS, str))
    {
      Absolutize(str, relativeFrom);
      ret.userRoots = str;
    }
    if (pcfg->TryGetValue("Paths", MIKTEX_REGVAL_USER_INSTALL, str))
    {
      Absolutize(str, relativeFrom);
      ret.userInstallRoot = str;
    }
    if (pcfg->TryGetValue("Paths", MIKTEX_REGVAL_USER_DATA, str))
    {
      Absolutize(str, relativeFrom);
      ret.userDataRoot = str;
    }
    if (pcfg->TryGetValue("Paths", MIKTEX_REGVAL_USER_CONFIG, str))
    {
      Absolutize(str, relativeFrom);
      ret.userConfigRoot = str;
    }
  }
  pcfg = nullptr;

  // inherit to child processes
  Utils::SetEnvironmentString(common ? MIKTEX_ENV_COMMON_STARTUP_FILE : MIKTEX_ENV_USER_STARTUP_FILE, path.Get());

  return ret;
}

void Relativize(string & paths, const PathName & relativeFrom)
{
#if MIKTEX_WINDOWS
  string result;
  for (CSVList path(paths, PATH_DELIMITER); path.GetCurrent() != nullptr; ++path)
  {
    if (!result.empty())
    {
      result += PATH_DELIMITER;
    }
    wchar_t szRelPath[MAX_PATH];
    if (PathRelativePathToW(szRelPath, relativeFrom.ToWideCharString().c_str(), FILE_ATTRIBUTE_DIRECTORY, UW_(path.GetCurrent()), FILE_ATTRIBUTE_DIRECTORY))
    {
      result += WU_(szRelPath);
    }
    else
    {
      result += path.GetCurrent();
    }
  }
  paths = result;
#else
  UNIMPLEMENTED();
#endif
}

void SessionImpl::WriteStartupConfigFile(bool common, const StartupConfig & startupConfig)
{
  MIKTEX_ASSERT(!IsMiKTeXDirect());

  string str;

  StartupConfig defaultConfig = DefaultConfig(startupConfig.config, "", "");

  PathName userStartupConfigFile;

  if (Utils::GetEnvironmentString(MIKTEX_ENV_USER_STARTUP_FILE, str))
  {
    userStartupConfigFile = str;
  }
#if ! NO_REGISTRY
  else if (winRegistry::TryGetRegistryValue(TriState::False, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_STARTUP_FILE, str))
  {
    userStartupConfigFile = str;
  }
#endif
  else
  {
    userStartupConfigFile = defaultConfig.userConfigRoot;
    userStartupConfigFile /= MIKTEX_PATH_STARTUP_CONFIG_FILE;
  }

  PathName commonStartupConfigFile;

  if (Utils::GetEnvironmentString(MIKTEX_ENV_COMMON_STARTUP_FILE, str))
  {
    commonStartupConfigFile = str;
  }
#if ! NO_REGISTRY
  else if (winRegistry::TryGetRegistryValue(TriState::True, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_STARTUP_FILE, str))
  {
    commonStartupConfigFile = str;
  }
#endif
  else
  {
    PathName myloc = GetMyLocation(true);
    RemoveDirectoryDelimiter(myloc.GetData());
    PathName internalBindir(MIKTEX_PATH_INTERNAL_BIN_DIR);
    RemoveDirectoryDelimiter(internalBindir.GetData());
    PathName bindir(MIKTEX_PATH_BIN_DIR);
    RemoveDirectoryDelimiter(bindir.GetData());
    PathName prefix;
    if (Utils::GetPathNamePrefix(myloc, internalBindir, prefix))
    {
      commonStartupConfigFile = prefix;
    }
    else if (Utils::GetPathNamePrefix(myloc, bindir, prefix))
    {
      commonStartupConfigFile = prefix;
    }
    else
    {
      MIKTEX_UNEXPECTED();
    }
    commonStartupConfigFile /= MIKTEX_PATH_STARTUP_CONFIG_FILE;
  }

  unique_ptr<Cfg> pcfg(Cfg::Create());

  const bool showAllValues = false;

  bool relativize = false;
  PathName relativeFrom;

  if (startupConfig.config == MiKTeXConfiguration::Portable)
  {
    pcfg->PutValue("Auto", "Config", "Portable");
    relativize = (userStartupConfigFile == commonStartupConfigFile);
    if (relativize)
    {
      relativeFrom = commonStartupConfigFile;
      relativeFrom.RemoveFileSpec();
    }
  }

  if (common || commonStartupConfigFile == userStartupConfigFile)
  {
    if (startupConfig.commonRoots != "" || showAllValues)
    {
      string val = startupConfig.commonRoots;
      if (relativize)
      {
        Relativize(val, relativeFrom);
      };
      pcfg->PutValue("Paths", MIKTEX_REGVAL_COMMON_ROOTS, val, T_("common TEXMF root directories"), startupConfig.commonRoots == "");
    }
    if (!startupConfig.commonInstallRoot.Empty() && (startupConfig.commonInstallRoot != defaultConfig.commonInstallRoot || showAllValues))
    {
      string val = startupConfig.commonInstallRoot.Get();
      if (relativize)
      {
        Relativize(val, relativeFrom);
      };
      pcfg->PutValue("Paths", MIKTEX_REGVAL_COMMON_INSTALL, val, T_("common install root"), startupConfig.commonInstallRoot == defaultConfig.commonInstallRoot);
    }
    if (!startupConfig.commonDataRoot.Empty()
      && (startupConfig.commonDataRoot != defaultConfig.commonDataRoot || showAllValues))
    {
      string val = startupConfig.commonDataRoot.Get();
      if (relativize)
      {
        Relativize(val, relativeFrom);
      };
      pcfg->PutValue("Paths", MIKTEX_REGVAL_COMMON_DATA, val, T_("common data root"), startupConfig.commonDataRoot == defaultConfig.commonDataRoot);
    }
    if (!startupConfig.commonConfigRoot.Empty() && (startupConfig.commonConfigRoot != defaultConfig.commonConfigRoot || showAllValues))
    {
      string val = startupConfig.commonConfigRoot.Get();
      if (relativize)
      {
        Relativize(val, relativeFrom);
      };
      pcfg->PutValue("Paths", MIKTEX_REGVAL_COMMON_CONFIG, val, T_("common config root"), startupConfig.commonConfigRoot == defaultConfig.commonConfigRoot);
    }
  }

  if (!common || commonStartupConfigFile == userStartupConfigFile || AdminControlsUserConfig())
  {
    if (startupConfig.userRoots != "" || showAllValues)
    {
      string val = startupConfig.userRoots;
      if (relativize)
      {
        Relativize(val, relativeFrom);
      };
      pcfg->PutValue("Paths", MIKTEX_REGVAL_USER_ROOTS, val, T_("user TEXMF root directories"), startupConfig.userRoots == "");
    }
    if (!startupConfig.userInstallRoot.Empty() && (startupConfig.userInstallRoot != defaultConfig.userInstallRoot || showAllValues))
    {
      string val = startupConfig.userInstallRoot.Get();
      if (relativize)
      {
        Relativize(val, relativeFrom);
      };
      pcfg->PutValue("Paths", MIKTEX_REGVAL_USER_INSTALL, val, T_("user install root"), startupConfig.userInstallRoot == defaultConfig.userInstallRoot);
    }
    if (!startupConfig.userDataRoot.Empty()
      && (startupConfig.userDataRoot != defaultConfig.userDataRoot || showAllValues))
    {
      string val = startupConfig.userDataRoot.Get();
      if (relativize)
      {
        Relativize(val, relativeFrom);
      };
      pcfg->PutValue("Paths", MIKTEX_REGVAL_USER_DATA, val, T_("user data root"), startupConfig.userDataRoot == defaultConfig.userDataRoot);
    }
    if (!startupConfig.userConfigRoot.Empty() && (startupConfig.userConfigRoot != defaultConfig.userConfigRoot || showAllValues))
    {
      string val = startupConfig.userConfigRoot.Get();
      if (relativize)
      {
        Relativize(val, relativeFrom);
      };
      pcfg->PutValue("Paths", MIKTEX_REGVAL_USER_CONFIG, val, T_("user config root"), startupConfig.userConfigRoot == defaultConfig.userConfigRoot);
    }
  }

  PathName startupConfigFile;
  if (common)
  {
    startupConfigFile = commonStartupConfigFile;
  }
  else
  {
    startupConfigFile = userStartupConfigFile;
  }

  PathName dir;
  dir = startupConfigFile;
  dir.RemoveFileSpec();
  Directory::Create(dir);

  pcfg->Write(startupConfigFile, T_("MiKTeX startup information"));
}

StartupConfig SessionImpl::ReadEnvironment(bool common)
{
  MIKTEX_ASSERT(!IsMiKTeXDirect());

  StartupConfig ret;

  string str;

  if (common)
  {
    if (Utils::GetEnvironmentString(MIKTEX_ENV_COMMON_ROOTS, str))
    {
      ret.commonRoots = str;
    }
    if (Utils::GetEnvironmentString(MIKTEX_ENV_COMMON_INSTALL, str))
    {
      ret.commonInstallRoot = str;
    }
    if (Utils::GetEnvironmentString(MIKTEX_ENV_COMMON_DATA, str))
    {
      ret.commonDataRoot = str;
    }
    if (Utils::GetEnvironmentString(MIKTEX_ENV_COMMON_CONFIG, str))
    {
      ret.commonConfigRoot = str;
    }
  }
  else
  {
    if (Utils::GetEnvironmentString(MIKTEX_ENV_USER_ROOTS, str))
    {
      ret.userRoots = str;
    }
    if (Utils::GetEnvironmentString(MIKTEX_ENV_USER_INSTALL, str))
    {
      ret.userInstallRoot = str;
    }
    if (Utils::GetEnvironmentString(MIKTEX_ENV_USER_DATA, str))
    {
      ret.userDataRoot = str;
    }
    if (Utils::GetEnvironmentString(MIKTEX_ENV_USER_CONFIG, str))
    {
      ret.userConfigRoot = str;
    }
  }

  return ret;
}

bool SessionImpl::IsMiKTeXDirect()
{
  return startupConfig.config == MiKTeXConfiguration::Direct;
}

bool SessionImpl::IsMiKTeXPortable()
{
  return startupConfig.config == MiKTeXConfiguration::Portable;
}

PathName SessionImpl::GetBinDirectory()
{
#if defined(MIKTEX_NATIVE_WINDOWS)
  PathName ret = GetRootDirectory(GetInstallRoot());
  ret += MIKTEX_PATH_BIN_DIR;
  return ret;
#else
  return GetMyLocation(true);
#endif
}

void SessionImpl::ReadAllConfigFiles(const string & baseName, Cfg & cfg)
{
  PathName fileName(MIKTEX_PATH_MIKTEX_CONFIG_DIR, baseName, ".ini");
  vector<PathName> configFiles;
  if (!FindFile(fileName.Get(), MIKTEX_PATH_TEXMF_PLACEHOLDER, { FindFileOption::All }, configFiles))
  {
    return;
  }
  for (vector<PathName>::const_reverse_iterator it = configFiles.rbegin(); it != configFiles.rend(); ++it)
  {
    unsigned r = TryDeriveTEXMFRoot(*it);
    if (r != INVALID_ROOT_INDEX && !IsManagedRoot(r))
    {
      continue;
    }
    cfg.Read(*it);
  }
}

MIKTEXSTATICFUNC(void) AppendToEnvVarName(string & name, const string & part)
{
  for (char ch : part)
  {
    if (IsAlpha(ch))
    {
      name += ToUpper(ch);
    }
    else if (IsDigit(ch))
    {
      name += ch;
    }
  }
}

bool SessionImpl::GetSessionValue(const char * lpszSectionName, const char * lpszValueName, string & value, const char * lpszDefaultValue)
{
  bool haveValue = false;

  // try special values, part 1
  if (!haveValue && Utils::EqualsIgnoreCase(lpszValueName, CFG_MACRO_NAME_ENGINE))
  {
    value = GetEngineName();
    haveValue = true;
  }

  // iterate over application tags, e.g.: latex;tex;miktex
  for (CSVList app(applicationNames, PATH_DELIMITER); !haveValue && app.GetCurrent() != nullptr; ++app)
  {
    Cfg * cfg = nullptr;

    // read configuration files
    if (!initInfo.GetOptions()[InitOption::NoConfigFiles])
    {
      ConfigurationSettings::iterator it = configurationSettings.find(app.GetCurrent());
      if (it != configurationSettings.end())
      {
        cfg = it->second.get();
      }
      else
      {
        pair<ConfigurationSettings::iterator, bool> p = configurationSettings.insert(ConfigurationSettings::value_type(app.GetCurrent(), Cfg::Create()));
        cfg = p.first->second.get();
        ReadAllConfigFiles(app.GetCurrent(), *cfg);
      }
    }

    // section name defaults to application name
    string sectionName = lpszSectionName == nullptr ? app.GetCurrent() : lpszSectionName;

#if 0
    const ConfigMapping * pMapping = FindConfigMapping(lpszSectionName2, lpszValueName);

    if (pMapping != nullptr && pMapping->lpszEnvVarName != nullptr && Utils::GetEnvironmentString(pMapping->lpszEnvVarName, value))
    {
      haveValue = true;
      break;
    }
#endif

    // try environment variable
    // MIKTEX_<APPLICATIONNAME>_<SECTIONNAME>_<VALUENAME>
    {
      string envVarName;
      envVarName.reserve(100);

      envVarName = MIKTEX_ENV_PREFIX_;
      AppendToEnvVarName(envVarName, app.GetCurrent());
      envVarName += '_';
      AppendToEnvVarName(envVarName, sectionName);
      envVarName += '_';
      AppendToEnvVarName(envVarName, lpszValueName);
      if (Utils::GetEnvironmentString(envVarName.c_str(), value))
      {
        haveValue = true;
        break;
      }
    }

#if defined(MIKTEX_WINDOWS)
    // try registry value
    if (!IsMiKTeXPortable() && winRegistry::TryGetRegistryValue(TriState::Undetermined, sectionName, lpszValueName, value))
    {
      haveValue = true;
      break;
    }
#endif

    // try configuration file
    if (cfg != nullptr && cfg->TryGetValue(sectionName, lpszValueName, value))
    {
      haveValue = true;
      break;
    }
  }

  // try environment variable
  // MIKTEX_<SECTIONNAME>_<VALUENAME>
  if (!haveValue && lpszSectionName != nullptr)
  {
    string envVarName(MIKTEX_ENV_PREFIX_);
    AppendToEnvVarName(envVarName, lpszSectionName);
    envVarName += '_';
    AppendToEnvVarName(envVarName, lpszValueName);
    if (Utils::GetEnvironmentString(envVarName.c_str(), value))
    {
      haveValue = true;
    }
  }

  // try environment variable
  // MIKTEX_<VALUENAME>
  if (!haveValue)
  {
    string envVarName(MIKTEX_ENV_PREFIX_);
    AppendToEnvVarName(envVarName, lpszValueName);
    if (Utils::GetEnvironmentString(envVarName.c_str(), value))
    {
      haveValue = true;
    }
  }

  // try environment variable
  // <VALUENAME>
  if (!haveValue && lpszSectionName == nullptr)
  {
    if (Utils::GetEnvironmentString(lpszValueName, value))
    {
      haveValue = true;
    }
  }

#if defined(MIKTEX_WINDOWS)
  // try registry value
  if (!haveValue && !IsMiKTeXPortable() && lpszSectionName != nullptr && winRegistry::TryGetRegistryValue(TriState::Undetermined, lpszSectionName, lpszValueName, value))
  {
    haveValue = true;
  }
#endif

  // try special values, part 2
  if (!haveValue && Utils::EqualsIgnoreCase(lpszValueName, CFG_MACRO_NAME_BINDIR))
  {
    value = SessionImpl::GetSession()->GetSpecialPath(SpecialPath::BinDirectory).ToString();
    haveValue = true;
  }
  else if (!haveValue && Utils::EqualsIgnoreCase(lpszValueName, CFG_MACRO_NAME_PROGNAME))
  {
    MIKTEX_ASSERT(!applicationNames.empty());
    value = CSVList(applicationNames, PATH_DELIMITER).GetCurrent();
    haveValue = true;
  }
#if defined(MIKTEX_WINDOWS)
  else if (!haveValue && Utils::EqualsIgnoreCase(lpszValueName, CFG_MACRO_NAME_WINDIR))
  {
    wchar_t szPath[BufferSizes::MaxPath];
    if (GetWindowsDirectoryW(szPath, BufferSizes::MaxPath) == 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("GetWindowsDirectoryW");
    }
    value = WU_(szPath);
    haveValue = true;
  }
#endif
  else if (!haveValue && Utils::EqualsIgnoreCase(lpszValueName, CFG_MACRO_NAME_LOCALFONTDIRS))
  {
    value = SessionImpl::GetSession()->GetLocalFontDirectories();
    haveValue = true;
  }
  else if (!haveValue && Utils::EqualsIgnoreCase(lpszValueName, CFG_MACRO_NAME_PSFONTDIRS))
  {
    string psFontDirs;
    if (SessionImpl::GetSession()->GetPsFontDirs(psFontDirs))
    {
      value = psFontDirs;
      haveValue = true;
    }
  }
  else if (!haveValue && Utils::EqualsIgnoreCase(lpszValueName, CFG_MACRO_NAME_TTFDIRS))
  {
    string ttfDirs;
    if (SessionImpl::GetSession()->GetTTFDirs(ttfDirs))
    {
      value = ttfDirs;
      haveValue = true;
    }
  }
  else if (!haveValue && Utils::EqualsIgnoreCase(lpszValueName, CFG_MACRO_NAME_OTFDIRS))
  {
    string otfDirs;
    if (SessionImpl::GetSession()->GetOTFDirs(otfDirs))
    {
      value = otfDirs;
      haveValue = true;
    }
  }

  // if we have found nothing, then we return the default value
  if (!haveValue && lpszDefaultValue != nullptr)
  {
    value = lpszDefaultValue;
    haveValue = true;
  }

#if 1
  // expand the value
  if (haveValue)
  {
    string expandedValue = Expand(value.c_str(), nullptr);
    value = expandedValue;
  }
#endif

  if (trace_values->IsEnabled())
  {
    if (lpszSectionName != nullptr)
    {
      trace_values->WriteFormattedLine("core", "[%s]%s => %s", lpszSectionName, lpszValueName, haveValue ? value.c_str() : "null");
    }
    else
    {
      trace_values->WriteFormattedLine("core", "%s => %s", lpszValueName, haveValue ? value.c_str() : "null");
    }
  }

  return haveValue;
}

bool SessionImpl::TryGetConfigValue(const char * lpszSectionName, const char * lpszValueName, string & value)
{
  MIKTEX_ASSERT_STRING_OR_NIL(lpszSectionName);
  MIKTEX_ASSERT_STRING(lpszValueName);

  return GetSessionValue(lpszSectionName, lpszValueName, value, nullptr);
}

string SessionImpl::GetConfigValue(const char * lpszSectionName, const char * lpszValueName, const char * lpszDefaultValue)
{
  MIKTEX_ASSERT_STRING_OR_NIL(lpszSectionName);
  MIKTEX_ASSERT_STRING(lpszValueName);
  MIKTEX_ASSERT_STRING(lpszDefaultValue);

  string value;

  if (!GetSessionValue(lpszSectionName, lpszValueName, value, lpszDefaultValue))
  {
    INVALID_ARGUMENT("valueName", lpszValueName);
  }

  return value;
}

char SessionImpl::GetConfigValue(const char * lpszSectionName, const char * lpszValueName, char defaultValue)
{
  MIKTEX_ASSERT_STRING_OR_NIL(lpszSectionName);
  MIKTEX_ASSERT_STRING(lpszValueName);

  string value;

  string defaultValueString;
  defaultValueString = defaultValue;

  if (!GetSessionValue(lpszSectionName, lpszValueName, value, defaultValueString.c_str()))
  {
    INVALID_ARGUMENT("valueName", lpszValueName);
  }

  if (value.length() != 1)
  {
    MIKTEX_UNEXPECTED();
  }

  return value[0];
}

int SessionImpl::GetConfigValue(const char * lpszSectionName, const char * lpszValueName, int defaultValue)
{
  MIKTEX_ASSERT_STRING_OR_NIL(lpszSectionName);
  MIKTEX_ASSERT_STRING(lpszValueName);

  string value;

  if (!GetSessionValue(lpszSectionName, lpszValueName, value, nullptr))
  {
    return defaultValue;
  }

  return atoi(value.c_str());
}

bool SessionImpl::GetConfigValue(const char * lpszSectionName, const char * lpszValueName, bool defaultValue)
{
  MIKTEX_ASSERT_STRING_OR_NIL(lpszSectionName);
  MIKTEX_ASSERT_STRING(lpszValueName);

  string value;

  if (!GetSessionValue(lpszSectionName, lpszValueName, value, nullptr))
  {
    return defaultValue;
  }

  if (value == "0"
    || value == "disable"
    || value == "off"
    || value == "f"
    || value == "false"
    || value == "n"
    || value == "no")
  {
    return false;
  }
  else if (!(value == "1"
    || value == "enable"
    || value == "on"
    || value == "t"
    || value == "true"
    || value == "y"
    || value == "yes"))
  {
    MIKTEX_UNEXPECTED();
  }
  return true;
}

TriState SessionImpl::GetConfigValue(const char * lpszSectionName, const char * lpszValueName, TriState defaultValue)
{
  MIKTEX_ASSERT_STRING_OR_NIL(lpszSectionName);
  MIKTEX_ASSERT_STRING(lpszValueName);

  string value;

  if (!GetSessionValue(lpszSectionName, lpszValueName, value, nullptr))
  {
    return defaultValue;
  }

  if (value == "0"
    || value == "disable"
    || value == "off"
    || value == "f"
    || value == "false"
    || value == "n"
    || value == "no")
  {
    return TriState::False;
  }
  else if (value == "1"
    || value == "enable"
    || value == "on"
    || value == "t"
    || value == "true"
    || value == "y"
    || value == "yes")
  {
    return TriState::True;
  }
  else if (!(value == ""
    || value == "2"
    || value == "?"
    || value == "undetermined"))
  {
    MIKTEX_UNEXPECTED();
  }
  return TriState::Undetermined;
}

void SessionImpl::SetConfigValue(const char * lpszSectionName, const char * lpszValueName, const char * lpszValue)
{
  MIKTEX_ASSERT_STRING(lpszSectionName);

  PathName pathConfigFile = GetSpecialPath(SpecialPath::ConfigRoot);
  pathConfigFile /= MIKTEX_PATH_MIKTEX_CONFIG_DIR;
  pathConfigFile /= MIKTEX_INI_FILE;

  unique_ptr<Cfg> pCfg(Cfg::Create());

  bool haveConfigFile = File::Exists(pathConfigFile);

  if (haveConfigFile)
  {
    pCfg->Read(pathConfigFile);
  }

#if defined(MIKTEX_WINDOWS)
  if (!haveConfigFile
    && !IsMiKTeXPortable()
    && !GetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_NO_REGISTRY, NO_REGISTRY ? true : false))
  {
    winRegistry::SetRegistryValue(IsAdminMode() ? TriState::True : TriState::False, lpszSectionName, lpszValueName, lpszValue);
    string newValue;
    if (GetSessionValue(lpszSectionName, lpszValueName, newValue, nullptr))
    {
      if (newValue != lpszValue)
      {
        MIKTEX_FATAL_ERROR_2(T_("The configuration value could not be changed. Possible reason: an environment variable definition is in the way."), "valueName", lpszValueName);
      }
    }
    return;
  }
#endif

  pCfg->PutValue(lpszSectionName, lpszValueName, lpszValue);
  pCfg->Write(pathConfigFile);
  configurationSettings.clear();
}

void SessionImpl::SetConfigValue(const char * lpszSectionName, const char * lpszValueName, bool value)
{
  SetConfigValue(lpszSectionName, lpszValueName, value ? "t" : "f");
}

void SessionImpl::SetConfigValue(const char * lpszSectionName, const char * lpszValueName, int value)
{
  SetConfigValue(lpszSectionName, lpszValueName, std::to_string(value).c_str());
}

void SessionImpl::SetAdminMode(bool adminMode, bool isSetup)
{
  if (this->adminMode == adminMode)
  {
    return;
  }
  if (adminMode && !isSetup && !IsSharedSetup())
  {
    MIKTEX_FATAL_ERROR(T_("Administrative mode cannot be enabled (makes no sense) because this is not a shared MiKTeX setup."));
  }
  trace_config->WriteFormattedLine("core", T_("turning %s administrative mode"), (adminMode ? "on" : "off"));
  fileTypes.clear();
  UnloadFilenameDatabase();
  this->adminMode = adminMode;
  if (rootDirectories.size() > 0)
  {
    // reinitialize
    InitializeRootDirectories();
  }
}

bool SessionImpl::IsAdminMode()
{
  return adminMode;
}

bool SessionImpl::IsSharedSetup()
{
  if (isSharedSetup == TriState::Undetermined)
  {
    isSharedSetup = GetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_SHARED_SETUP, TriState::Undetermined);
    if (isSharedSetup == TriState::Undetermined)
    {
      string value;
      isSharedSetup = TryGetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_LAST_ADMIN_MAINTENANCE, value) ? TriState::True : TriState::False;
    }
  }
  return isSharedSetup == TriState::True;
}

void SessionImpl::ConfigureFile(const PathName & pathRel, HasNamedValues * callback)
{
  PathName pathOut(GetSpecialPath(SpecialPath::ConfigRoot));
  pathOut /= pathRel;
  PathName relPathIn = pathRel;
  relPathIn.AppendExtension(".in");
  PathName pathIn;
  if (!FindFile(relPathIn.Get(), MIKTEX_PATH_TEXMF_PLACEHOLDER, pathIn))
  {
    MIKTEX_FATAL_ERROR_2(T_("The template file could not be found."), "templateFile", relPathIn.ToString());
  }
  ConfigureFile(pathIn, pathOut, callback);
}

class ConfigureFileCallback : public HasNamedValues
{
public:
  bool TryGetValue(const string & valueName, string & value)
  {
    if (valueName == "MIKTEX_INSTALL")
    {
      value = session->GetSpecialPath(SpecialPath::InstallRoot).ToString();
    }
    else if (valueName == "MIKTEX_CONFIG")
    {
      value = session->GetSpecialPath(SpecialPath::ConfigRoot).ToString();
    }
    else if (valueName == "MIKTEX_DATA")
    {
      value = session->GetSpecialPath(SpecialPath::DataRoot).ToString();
    }
    else
    {
      return false;
    }
    return true;
  }
public:
  string GetValue(const string & valueName)
  {
    string value;
    if (!TryGetValue(valueName, value))
    {
      MIKTEX_UNEXPECTED();
    }
    return value;
  }
public:
  ConfigureFileCallback() = delete;
public:
  ConfigureFileCallback(SessionImpl * session) :
    session(session)
  {
  }
private:
  SessionImpl * session;
};

void SessionImpl::ConfigureFile(const PathName & pathIn, const PathName & pathOut, HasNamedValues * callback)
{
  ConfigureFileCallback standardCallback(this);
  if (callback == nullptr)
  {
    callback = &standardCallback;
  }
  Directory::Create(PathName(pathOut).RemoveFileSpec());
  if (File::Exists(pathOut))
  {
    FileAttributeSet attr = File::GetAttributes(pathOut);
    attr -= FileAttribute::ReadOnly;
    File::SetAttributes(pathOut, attr);
  }
  FileStream streamIn(OpenFile(pathIn.Get(), FileMode::Open, FileAccess::Read, false));
  FileStream streamOut(OpenFile(pathOut.Get(), FileMode::Create, FileAccess::Write, false));
  char chr;
  bool readingName = false;
  string name;
  const size_t UTF8BOM_LEN = 3;
  char const utf8bom[UTF8BOM_LEN] = { (char)0xef, (char)0xbb, (char)0xbf };
  bool isUtf8 = true;
  for (int count = 0; streamIn.Read(&chr, 1) == 1; ++count)
  {
    if (count < UTF8BOM_LEN)
    {
      isUtf8 = isUtf8 && chr == utf8bom[count];
    }
    if (chr == '@')
    {
      if (readingName)
      {
        readingName = false;
        if (name.empty())
        {
          streamOut.Write(&chr, 1);
        }
        else
        {
          string value = callback->GetValue(name);
          streamOut.Write(value.c_str(), value.length());
        }
      }
      else
      {
        readingName = true;
        name = "";
      }
    }
    else if (readingName)
    {
      name += chr;
    }
    else
    {
      streamOut.Write(&chr, 1);
    }
  }
  streamIn.Close();
  streamOut.Close();
  FileAttributeSet attr = File::GetAttributes(pathOut);
  attr += FileAttribute::ReadOnly;
  File::SetAttributes(pathOut, attr);
  if (!Fndb::FileExists(pathOut))
  {
    Fndb::Add(pathOut);
  }
}

std::string SessionImpl::Expand(const char * lpszToBeExpanded)
{
  return Expand(lpszToBeExpanded, { ExpandOption::Values }, nullptr);
}

std::string SessionImpl::Expand(const char * lpszToBeExpanded, HasNamedValues * callback)
{
  return Expand(lpszToBeExpanded, { ExpandOption::Values }, callback);
}

std::string SessionImpl::Expand(const char * lpszToBeExpanded, ExpandOptionSet options, HasNamedValues * callback)
{
  string result;
  if (options[ExpandOption::Braces])
  {
    result = MakeSearchPath(ExpandBraces(lpszToBeExpanded));
    lpszToBeExpanded = result.c_str();
  }
  if (options[ExpandOption::Values])
  {
    result = ExpandValues(lpszToBeExpanded, callback);
    lpszToBeExpanded = result.c_str();
  }
  if (options[ExpandOption::Braces])
  {
    result = MakeSearchPath(ExpandBraces(lpszToBeExpanded));
    lpszToBeExpanded = result.c_str();
  }
  if (options[ExpandOption::PathPatterns])
  {
    result = MakeSearchPath(ExpandPathPatterns(lpszToBeExpanded));
    lpszToBeExpanded = result.c_str();
  }
  return result;
}

std::string SessionImpl::ExpandValues(const char * lpszToBeExpanded, HasNamedValues * callback)
{
  const char * lpsz = lpszToBeExpanded;
  string valueName;
  string expansion;
  expansion.reserve(strlen(lpsz));
  for (; *lpsz != 0; ++lpsz)
  {
    if (lpsz[0] == '$')
    {
      if (lpsz[1] == '$')
      {
        lpsz += 1;
        expansion += '$';
      }
      else if (lpsz[1] == '(' || lpsz[1] == '{' || isalpha(lpsz[1]) || lpsz[1] == '_')
      {
        const char * lpszBegin = lpsz;
        char endChar = (lpsz[1] == '(' ? ')' : (lpsz[1] == '{' ? '}' : 0));
        valueName = "";
        if (endChar == 0)
        {
          for (lpsz += 1; *lpsz != 0 && (isalnum(*lpsz) || *lpsz == '_'); ++lpsz)
          {
            valueName += *lpsz;
          }
          --lpsz;
        }
        else
        {
          for (lpsz += 2; *lpsz != 0 && *lpsz != endChar; ++lpsz)
          {
            valueName += *lpsz;
          }
          if (*lpsz != endChar)
          {
            MIKTEX_UNEXPECTED();
          }
          if (valueName.empty())
          {
            MIKTEX_UNEXPECTED();
          }
        }
        if (valuesBeingExpanded.find(valueName) != valuesBeingExpanded.end())
        {
          MIKTEX_UNEXPECTED();
        }
        set<string>::iterator it = valuesBeingExpanded.insert(valueName).first;
        string value;
        bool haveValue = false;
        if (callback != nullptr)
        {
          haveValue = callback->TryGetValue(valueName, value);
        }
        if (!haveValue)
        {
          haveValue = TryGetConfigValue(nullptr, valueName.c_str(), value);
        }
        if (haveValue)
        {
          expansion += value;
        }
        else
        {
          lpsz = lpszBegin;
          expansion += *lpsz;
        }
        valuesBeingExpanded.erase(it);
      }
      else
      {
        MIKTEX_UNEXPECTED();
      }
    }
    else
    {
      expansion += *lpsz;
    }
  }
  return expansion;
}
