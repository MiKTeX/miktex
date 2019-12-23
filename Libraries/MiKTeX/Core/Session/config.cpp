/* config.cpp: MiKTeX configuration settings

   Copyright (C) 1996-2019 Christian Schenk

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

#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/ConfigNames>
#include <miktex/Core/CsvList>
#include <miktex/Core/Directory>
#include <miktex/Core/Environment>
#include <miktex/Core/FileStream>
#include <miktex/Core/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/Registry>
#include <miktex/Util/Tokenizer>

#include "internal.h"

#if defined(MIKTEX_WINDOWS)
#  include "win/winRegistry.h"
#endif

#include "Session/SessionImpl.h"
#include "Utils/inliners.h"

namespace {
#include "miktex-config.ini.h"
}

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

#if 0
struct ConfigMapping
{
  const char* lpszConfigSection;
  const char* lpszConfigValueName;
  const char* lpszEnvVarName;
#if defined(MIKTEX_WINDOWS)
  const char* lpszRegKey;
  const char* lpszRegValueName;
#endif
};
#endif

#if 0
namespace {
  const ConfigMapping configMappings[] = {
    {
      MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_REMOTE_REPOSITORY, MIKTEX_ENV_REPOSITORY,
    },
  };
}
#endif

#if 0
MIKTEXSTATICFUNC(const ConfigMapping*) FindConfigMapping(const char* lpszConfigSection, const char* lpszConfigValueName)
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

PathName SessionImpl::GetMyPrefix(bool canonicalized)
{
  PathName bindir = GetMyLocation(canonicalized);
  RemoveDirectoryDelimiter(bindir.GetData());
  for (const string& subdir : {
#if !defined(MIKTEX_WINDOWS)
    MIKTEX_BINARY_DESTINATION_DIR,
    MIKTEX_INTERNAL_BINARY_DESTINATION_DIR,
#endif
    MIKTEX_PATH_BIN_DIR,
    MIKTEX_PATH_INTERNAL_BIN_DIR,
#if defined(MIKTEX_MACOS_BUNDLE)
    MIKTEX_MACOS_DESTINATION_DIR
#endif
  })
  {
    PathName prefix;
    if (Utils::GetPathNamePrefix(bindir, subdir, prefix))
    {
      return prefix;
    }
  }
  MIKTEX_FATAL_ERROR_2("Cannot derive the path prefix of the running executable.", "bindir", bindir.ToString(), "canonicalized", canonicalized ? "true" : "false");
}

bool SessionImpl::FindStartupConfigFile(ConfigurationScope scope, PathName& path)
{
  if (initInfo.GetOptions()[InitOption::NoConfigFiles])
  {
    return false;
  }

  string str;

  if (Utils::GetEnvironmentString(scope == ConfigurationScope::Common ? MIKTEX_ENV_COMMON_STARTUP_FILE : MIKTEX_ENV_USER_STARTUP_FILE, str))
  {
    path = str;
    // don't check for existence; it's a fatal error (detected later)
    // if the environment value is incorrect
    return true;
  }

#if !NO_REGISTRY
  if (winRegistry::TryGetRegistryValue(scope, MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_STARTUP_FILE, str))
  {
    // don't check for existence; it's a fatal error (detected later)
    // if the registry value is incorrect
    path = str;
    return true;
  }
#endif

  StartupConfig defaultStartupConfig = DefaultConfig();

  if (scope == ConfigurationScope::Common)
  {
    // try the prefix of the internal bin directory
    PathName myloc = GetMyLocation(true);
    RemoveDirectoryDelimiter(myloc.GetData());
    PathName internalBindir(MIKTEX_PATH_INTERNAL_BIN_DIR);
    RemoveDirectoryDelimiter(internalBindir.GetData());
    PathName prefix;
    if (Utils::GetPathNamePrefix(myloc, internalBindir, prefix))
    {
      path = prefix / MIKTEX_PATH_STARTUP_CONFIG_FILE;
      if (File::Exists(path))
      {
        return true;
      }
    }
    // try the prefix of the bin directory
    PathName bindir(MIKTEX_PATH_BIN_DIR);
    RemoveDirectoryDelimiter(bindir.GetData());
    if (Utils::GetPathNamePrefix(myloc, bindir, prefix))
    {
      path = prefix / MIKTEX_PATH_STARTUP_CONFIG_FILE;
      if (File::Exists(path))
      {
        return true;
      }
    }
    // try /var/lib/miktex-texmf/miktex/config/miktexstartup.ini
    path = defaultStartupConfig.commonConfigRoot / MIKTEX_PATH_STARTUP_CONFIG_FILE;
    if (File::Exists(path))
    {
      return true;
    }
#if defined(MIKTEX_UNIX) && !defined(MIKTEX_MACOS_BUNDLE)
    // try /usr/share/miktex-texmf/miktex/config/miktexstartup.ini
    prefix = GetMyPrefix(true);
    path = prefix / MIKTEX_INSTALL_DIR / MIKTEX_PATH_STARTUP_CONFIG_FILE;
    if (File::Exists(path))
    {
      return true;
    }
#endif
  }

  if (scope == ConfigurationScope::User)
  {
    // try $HOME/.miktex/miktex/config/miktexstartup.ini
    path = defaultStartupConfig.userConfigRoot / MIKTEX_PATH_STARTUP_CONFIG_FILE;
    if (File::Exists(path))
    {
      return true;
    }
  }

  return false;
}

bool SessionImpl::IsMiKTeXDirect()
{
  return initStartupConfig.config == MiKTeXConfiguration::Direct;
}

bool SessionImpl::IsMiKTeXPortable()
{
  return initStartupConfig.config == MiKTeXConfiguration::Portable;
}

pair<bool, PathName> SessionImpl::TryGetBinDirectory(bool canonicalized)
{
#if defined(MIKTEX_WINDOWS)
  auto distRoot = TryGetDistRootDirectory();
  if (distRoot.first)
  {
    return make_pair<bool, PathName>(true, distRoot.second / MIKTEX_PATH_BIN_DIR);
  }
  string env;
  if (!Utils::GetEnvironmentString(MIKTEX_ENV_BIN_DIR, env))
  {
    return make_pair<bool, PathName>(true, PathName());
  }
  return make_pair<bool, PathName>(true, env);
#elif defined(MIKTEX_MACOS_BUNDLE)
  return make_pair<bool, PathName>(true, GetMyPrefix(canonicalized) / MIKTEX_BINARY_DESTINATION_DIR);
#else
  return make_pair<bool, PathName>(true, GetMyLocation(canonicalized));
#endif
}

PathName SessionImpl::GetBinDirectory(bool canonicalized)
{
  auto p = TryGetBinDirectory(canonicalized);
  if (!p.first)
  {
    MIKTEX_UNEXPECTED();
  }
  return p.second;
}

void SessionImpl::ReadAllConfigFiles(const string& baseName, Cfg& cfg)
{
  PathName fileName = MIKTEX_PATH_MIKTEX_CONFIG_DIR / baseName;
  fileName.AppendExtension(".ini");
  vector<PathName> configFiles;
  if (!FindFile(fileName.ToString(), MIKTEX_PATH_TEXMF_PLACEHOLDER, { FindFileOption::All }, configFiles))
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

MIKTEXSTATICFUNC(void) AppendToEnvVarName(string& name, const string& part)
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

bool SessionImpl::GetSessionValue(const string& sectionName, const string& valueName, string& value)
{
  bool haveValue = false;

  // try special values, part 1
  if (!haveValue && Utils::EqualsIgnoreCase(valueName, CFG_MACRO_NAME_ENGINE))
  {
    value = GetEngineName();
    haveValue = true;
  }

  // iterate over application tags, e.g.: latex;tex;miktex
  for (CsvList app(applicationNames, PathName::PathNameDelimiter); !haveValue && app; ++app)
  {
    MIKTEX_ASSERT(!(*app).empty());

    string lookupKeyName = Utils::MakeLower(*app);

    Cfg* cfg = nullptr;

    // read configuration files
    if (!initInfo.GetOptions()[InitOption::NoConfigFiles])
    {
      ConfigurationSettings::iterator it = configurationSettings.find(lookupKeyName);
      if (it != configurationSettings.end())
      {
        cfg = it->second.get();
      }
      else
      {
        pair<ConfigurationSettings::iterator, bool> p = configurationSettings.insert(ConfigurationSettings::value_type(lookupKeyName, Cfg::Create()));
        cfg = p.first->second.get();
        ReadAllConfigFiles(lookupKeyName, *cfg);
      }
    }

    // section name defaults to application name
    string defaultSectionName = sectionName.empty() ? *app : sectionName;

    // try environment variable
    // MIKTEX_<APPLICATIONNAME>_<SECTIONNAME>_<VALUENAME>
    {
      string envVarName;
      envVarName.reserve(100);

      envVarName = MIKTEX_ENV_PREFIX_;
      AppendToEnvVarName(envVarName, *app);
      envVarName += '_';
      AppendToEnvVarName(envVarName, defaultSectionName);
      envVarName += '_';
      AppendToEnvVarName(envVarName, valueName);
      if (Utils::GetEnvironmentString(envVarName, value))
      {
        haveValue = true;
        break;
      }
    }

#if defined(MIKTEX_WINDOWS)
    // try registry value
    if (!IsMiKTeXPortable() && winRegistry::TryGetRegistryValue(ConfigurationScope::None, defaultSectionName, valueName, value))
    {
      haveValue = true;
      break;
    }
#endif

    // try configuration file
    if (cfg != nullptr && cfg->TryGetValueAsString(defaultSectionName, valueName, value))
    {
      haveValue = true;
      break;
    }
  }

  // try environment variable
  // MIKTEX_<SECTIONNAME>_<VALUENAME>
  if (!haveValue && !sectionName.empty())
  {
    string envVarName(MIKTEX_ENV_PREFIX_);
    AppendToEnvVarName(envVarName, sectionName);
    envVarName += '_';
    AppendToEnvVarName(envVarName, valueName);
    if (Utils::GetEnvironmentString(envVarName, value))
    {
      haveValue = true;
    }
  }

  // try environment variable
  // MIKTEX_<VALUENAME>
  if (!haveValue)
  {
    string envVarName(MIKTEX_ENV_PREFIX_);
    AppendToEnvVarName(envVarName, valueName);
    if (Utils::GetEnvironmentString(envVarName, value))
    {
      haveValue = true;
    }
  }

  // try environment variable
  // <VALUENAME>
  if (!haveValue && sectionName.empty())
  {
    if (Utils::GetEnvironmentString(valueName, value))
    {
      haveValue = true;
    }
  }

#if defined(MIKTEX_WINDOWS)
  // try registry value
  if (!haveValue && !IsMiKTeXPortable() && !sectionName.empty() && winRegistry::TryGetRegistryValue(ConfigurationScope::None, sectionName, valueName, value))
  {
    haveValue = true;
  }
#endif

  // try special values, part 2
  if (!haveValue && Utils::EqualsIgnoreCase(valueName, CFG_MACRO_NAME_BINDIR))
  {
    value = SessionImpl::GetSession()->GetSpecialPath(SpecialPath::BinDirectory).ToString();
    haveValue = true;
  }
  else if (!haveValue && Utils::EqualsIgnoreCase(valueName, CFG_MACRO_NAME_PROGNAME))
  {
    CsvList progname(applicationNames, PathName::PathNameDelimiter);
    MIKTEX_ASSERT(progname && !(*progname).empty());
    value = *progname;
    haveValue = true;
  }
#if defined(MIKTEX_WINDOWS)
  else if (!haveValue && Utils::EqualsIgnoreCase(valueName, CFG_MACRO_NAME_WINDIR))
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
  else if (!haveValue && Utils::EqualsIgnoreCase(valueName, CFG_MACRO_NAME_LOCALFONTDIRS))
  {
    value = StringUtil::Flatten(SessionImpl::GetSession()->GetFontDirectories(), PathName::PathNameDelimiter);
    haveValue = true;
  }
  else if (!haveValue && Utils::EqualsIgnoreCase(valueName, CFG_MACRO_NAME_PSFONTDIRS))
  {
    string psFontDirs;
    if (SessionImpl::GetSession()->GetPsFontDirs(psFontDirs))
    {
      value = psFontDirs;
      haveValue = true;
    }
  }
  else if (!haveValue && Utils::EqualsIgnoreCase(valueName, CFG_MACRO_NAME_TTFDIRS))
  {
    string ttfDirs;
    if (SessionImpl::GetSession()->GetTTFDirs(ttfDirs))
    {
      value = ttfDirs;
      haveValue = true;
    }
  }
  else if (!haveValue && Utils::EqualsIgnoreCase(valueName, CFG_MACRO_NAME_OTFDIRS))
  {
    string otfDirs;
    if (SessionImpl::GetSession()->GetOTFDirs(otfDirs))
    {
      value = otfDirs;
      haveValue = true;
    }
  }

#if 1
  // try factory settings
  if (!haveValue)
  {
    const string& factorySettionsKey = "<\\miktex|*|config/>";
    Cfg* cfg = nullptr;
    ConfigurationSettings::iterator it = configurationSettings.find(factorySettionsKey);
    if (it != configurationSettings.end())
    {
      cfg = it->second.get();
    }
    else
    {
      pair<ConfigurationSettings::iterator, bool> p = configurationSettings.insert(ConfigurationSettings::value_type(factorySettionsKey, Cfg::Create()));
      cfg = p.first->second.get();
      memstreambuf buf(miktex_config_ini, sizeof(miktex_config_ini));
      istream reader(&buf);
      cfg->Read(reader);
    }
    if (cfg->TryGetValueAsString(sectionName, valueName, value))
    {
      haveValue = true;
    }
  }
#endif

#if 1
  // expand the value
  if (haveValue)
  {
    string expandedValue = Expand(value, nullptr);
    value = expandedValue;
  }
#endif

  if (trace_values->IsEnabled("core"))
  {
    if (!sectionName.empty())
    {
      trace_values->WriteLine("core", fmt::format("[{}]{} => {}", sectionName, valueName, haveValue ? value : "null"));
    }
    else
    {
      trace_values->WriteLine("core", fmt::format("{} => {}", valueName, haveValue ? value : "null"));
    }
  }

  return haveValue;
}

std::string ConfigValue::GetString() const
{
  switch (type)
  {
  case Type::String:
    return this->s;
  case Type::Int:
    return std::to_string(this->i);
  case Type::Bool:
    return this->b ? "true" : "false";
  case Type::Tri:
    return this->t == TriState::Undetermined ? "undetermined" : this->t == TriState::False ? "false" : "true";
  case Type::Char:
    return std::string(1, this->c);
  case Type::StringArray:
    return StringUtil::Flatten(this->sa, PathName::PathNameDelimiter);
  default:
    MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from type {type} to string."), "type", std::to_string(static_cast<int>(this->type)));
  }
}

int ConfigValue::GetInt() const
{
  switch (type)
  {
  case Type::String:
    return std::stoi(this->s);
  case Type::Int:
    return this->i;
  case Type::Bool:
    return (int)this->b;
  case Type::Tri:
    return (int)this->t;
  case Type::Char:
    return (int)this->c;
  default:
    MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from type {type} to integer."), "type", std::to_string(static_cast<int>(this->type)));
  }
}

bool ConfigValue::GetBool() const
{
  switch (type)
  {
  case Type::String:
    if (this->s == "0"
      || this->s == "disable"
      || this->s == "off"
      || this->s == "f"
      || this->s == "false"
      || this->s == "n"
      || this->s == "no")
    {
      return false;
    }
    else if (this->s == "1"
      || this->s == "enable"
      || this->s == "on"
      || this->s == "t"
      || this->s == "true"
      || this->s == "y"
      || this->s == "yes")
    {
      return true;
    }
    else
    {
      MIKTEX_FATAL_ERROR_2(T_("Configuration error: cannot convert '{s}' to boolean."), "s", this->s);
    }
  case Type::Int:
    if (this->i == 0)
    {
      return false;
    }
    else if (this->i == 1)
    {
      return true;
    }
    else
    {
      MIKTEX_FATAL_ERROR_2(T_("Configuration error: cannot convert {i} to boolean."), "i", std::to_string(this->i));
    }
  case Type::Bool:
    return this->b;
  case Type::Tri:
    if (this->t == TriState::False)
    {
      return false;
    }
    else if (this->t == TriState::True)
    {
      return true;
    }
    else
    {
      MIKTEX_FATAL_ERROR_2(T_("Configuration error: cannot convert {t} to boolean."), "t", std::to_string(static_cast<int>(this->t)));
    }
  case Type::Char:
    if (this->c == '0'
      || this->c == 'f'
      || this->c == 'n')
    {
      return false;
    }
    else if (this->c == '1'
      || this->c == 't'
      || this->c == 'y')
    {
      return true;
    }
    else
    {
      MIKTEX_FATAL_ERROR_2(T_("Configuration error: cannot convert '{c}' to boolean."), "c", string(1, this->c));
    }
  default:
    MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from type {type} to boolean."), "type", std::to_string(static_cast<int>(this->type)));
  }
}

TriState ConfigValue::GetTriState() const
{
  switch (type)
  {
  case Type::String:
    if (this->s == "0"
      || this->s == "disable"
      || this->s == "off"
      || this->s == "f"
      || this->s == "false"
      || this->s == "n"
      || this->s == "no")
    {
      return TriState::False;
    }
    else if (this->s == "1"
      || this->s == "enable"
      || this->s == "on"
      || this->s == "t"
      || this->s == "true"
      || this->s == "y"
      || this->s == "yes")
    {
      return TriState::True;
    }
    else if (this->s == ""
      || this->s == "2"
      || this->s == "?"
      || this->s == "undetermined")
    {
      return TriState::Undetermined;
    }
    else
    {
      MIKTEX_FATAL_ERROR_2(T_("Configuration error: cannot convert '{s}' to tri-state."), "s", this->s);
    }
  case Type::Int:
    if (this->i == 0)
    {
      return TriState::False;
    }
    else if (this->i == 1)
    {
      return TriState::True;
    }
    else if (this->i == 2)
    {
      return TriState::Undetermined;
    }
    else
    {
      MIKTEX_FATAL_ERROR_2(T_("Configuration error: cannot convert {i} to tri-state."), "i", std::to_string(this->i));
    }
  case Type::Bool:
    return this->b ? TriState::True : TriState::False;
  case Type::Tri:
    return this->t;
  case Type::Char:
    if (this->c == '0'
      || this->c == 'f'
      || this->c == 'n')
    {
      return TriState::False;
    }
    else if (this->c == '1'
      || this->c == 't'
      || this->c == 'y')
    {
      return TriState::True;
    }
    else if (this->c == '2'
      || this->c == '?')
    {
      return TriState::Undetermined;
    }
    else
    {
      MIKTEX_FATAL_ERROR_2(T_("Configuration error: cannot convert '{c}' to tri-state."), "c", string(1, this->c));
    }
  default:
    MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from type {type} to tri-state."), "type", std::to_string(static_cast<int>(this->type)));
  }
}

char ConfigValue::GetChar() const
{
  switch (type)
  {
  case Type::String:
    if (this->s.length() != 1)
    {
      MIKTEX_FATAL_ERROR_2(T_("Configuration error: cannot convert '{s}' to character."), "s", this->s);
    }
    return this->s[0];
  case Type::Int:
    if (this->i < CHAR_MIN || this->i > CHAR_MAX)
    {
      MIKTEX_FATAL_ERROR_2(T_("Configuration error: cannot convert {i} to character."), "i", std::to_string(this->i));
    }
    return (char)this->i;
  case Type::Bool:
    return this->b ? 't' : 'f';
  case Type::Tri:
    return this->t == TriState::Undetermined ? '?' : this->t == TriState::False ? 'f' : 't';
  case Type::Char:
    return this->c;
  default:
    MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from type {type} to character."), "type", std::to_string(static_cast<int>(this->type)));
  }
}

vector<string> ConfigValue::GetStringArray() const
{
  switch (type)
  {
  case Type::String:
    return StringUtil::Split(this->s, PathName::PathNameDelimiter);
  case Type::StringArray:
    return this->sa;
  default:
    MIKTEX_FATAL_ERROR_2(T_("Configuration error: no conversion from type {type} to string array."), "type", std::to_string(static_cast<int>(this->type)));
  }
}

bool SessionImpl::TryGetConfigValue(const std::string& sectionName, const string& valueName, string& value)
{
  return GetSessionValue(sectionName, valueName, value);
}

ConfigValue SessionImpl::GetConfigValue(const std::string& sectionName, const string& valueName, const ConfigValue& defaultValue)
{
  string value;
  if (GetSessionValue(sectionName, valueName, value))
  {
    return value;
  }
  else if (defaultValue.GetType() != ConfigValue::Type::None)
  {
    return Expand(defaultValue.GetString(), nullptr);
  }
  else
  {
    return ConfigValue();
  }
}

ConfigValue SessionImpl::GetConfigValue(const std::string& sectionName, const string& valueName)
{
  string value;
  if (GetSessionValue(sectionName, valueName, value))
  {
    return value;
  }
  else
  {
    return ConfigValue();
  }
}

void SessionImpl::SetConfigValue(const std::string& sectionName, const string& valueName, const ConfigValue& value)
{
  PathName pathConfigFile = GetSpecialPath(SpecialPath::ConfigRoot);
  pathConfigFile /= MIKTEX_PATH_MIKTEX_CONFIG_DIR;
  pathConfigFile /= MIKTEX_INI_FILE;

  unique_ptr<Cfg> cfg(Cfg::Create());

  bool haveConfigFile = File::Exists(pathConfigFile);

  if (haveConfigFile)
  {
    cfg->Read(pathConfigFile);
  }

#if defined(MIKTEX_WINDOWS)
  if (!haveConfigFile
    && !IsMiKTeXPortable()
    && !GetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_NO_REGISTRY, NO_REGISTRY ? true : false).GetBool())
  {
    winRegistry::SetRegistryValue(IsAdminMode() ? ConfigurationScope::Common : ConfigurationScope::User, sectionName, valueName, value.GetString());
    string newValue;
    if (GetSessionValue(sectionName, valueName, newValue))
    {
      if (newValue != value.GetString())
      {
        MIKTEX_FATAL_ERROR_2(T_("The configuration value could not be changed. Possible reason: an environment variable definition is in the way."), "valueName", valueName);
      }
    }
    return;
  }
#endif

  cfg->PutValue(sectionName, valueName, value.GetString());
  cfg->Write(pathConfigFile);
  if (!Fndb::FileExists(pathConfigFile))
  {
    Fndb::Add({ { pathConfigFile } });
  }
  configurationSettings.clear();
}

void SessionImpl::SetAdminMode(bool adminMode, bool force)
{
  if (this->adminMode == adminMode)
  {
    return;
  }
  if (adminMode && !force && !IsSharedSetup())
  {
    MIKTEX_FATAL_ERROR(T_("Administrator mode cannot be enabled (makes no sense) because this is not a shared MiKTeX setup."));
  }
  trace_config->WriteLine("core", fmt::format(T_("turning {0} administrator mode"), (adminMode ? "on" : "off")));
  // reinitialize
  fileTypes.clear();
  UnloadFilenameDatabase();
  this->adminMode = adminMode;
  if (!rootDirectories.empty())
  {
    // reinitialize root directories
    InitializeRootDirectories(initStartupConfig, false);
  }
  SetEnvironmentVariables();
}

bool SessionImpl::IsAdminMode()
{
  return adminMode;
}

bool SessionImpl::IsSharedSetup()
{
  if (isSharedSetup == TriState::Undetermined)
  {
    isSharedSetup = GetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_SHARED_SETUP, TriState::Undetermined).GetTriState();
    if (isSharedSetup == TriState::Undetermined)
    {
      string value;
      isSharedSetup = TryGetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_LAST_ADMIN_MAINTENANCE, value) ? TriState::True : TriState::Undetermined;
      if (isSharedSetup == TriState::Undetermined)
      {
#if defined(MIKTEX_WINDOWS)
        isSharedSetup = TriState::False;
#else
        PathName myLoc = GetMyLocation(true);
#if defined(MIKTEX_MACOS_BUNDLE)
        isSharedSetup = Utils::IsParentDirectoryOf("/usr", myLoc) || Utils::IsParentDirectoryOf("/Applications", myLoc) ? TriState::True : TriState::False;
#else
        isSharedSetup = Utils::IsParentDirectoryOf("/usr", myLoc) || Utils::IsParentDirectoryOf("/opt", myLoc) ? TriState::True : TriState::False;
#endif
#endif
      }
    }
  }
  return isSharedSetup == TriState::True;
}

void SessionImpl::ConfigureFile(const PathName& pathRel, HasNamedValues* callback)
{
  PathName pathOut(GetSpecialPath(SpecialPath::ConfigRoot));
  pathOut /= pathRel;
  PathName relPathIn = pathRel;
  relPathIn.AppendExtension(".in");
  PathName pathIn;
  if (!FindFile(relPathIn.ToString(), MIKTEX_PATH_TEXMF_PLACEHOLDER, pathIn))
  {
    MIKTEX_FATAL_ERROR_2(T_("The template file could not be found."), "templateFile", relPathIn.ToString());
  }
  ConfigureFile(pathIn, pathOut, callback);
}

class ConfigureFileCallback : public HasNamedValues
{
public:
  bool TryGetValue(const string& valueName, string& value)
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
  string GetValue(const string& valueName)
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
  ConfigureFileCallback(SessionImpl* session) :
    session(session)
  {
  }
private:
  SessionImpl* session;
};

void SessionImpl::ConfigureFile(const PathName& pathIn, const PathName& pathOut, HasNamedValues* callback)
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
  FileStream streamIn(OpenFile(pathIn.GetData(), FileMode::Open, FileAccess::Read, false));
  FileStream streamOut(OpenFile(pathOut.GetData(), FileMode::Create, FileAccess::Write, false));
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
    Fndb::Add({ {pathOut} });
  }
}

class DefaultCallback :
  public HasNamedValues
{
public:
  DefaultCallback(SessionImpl* session)
  {
  }
public:
  bool TryGetValue(const string& valueName, string& value) override
  {
    if (valueName == "MIKTEX_SYSTEM_TAG")
    {
      value = MIKTEX_SYSTEM_TAG;
      return true;
    }
    else if (valueName == "MIKTEX_EXE_FILE_SUFFIX")
    {
      value = MIKTEX_EXE_FILE_SUFFIX;
      return true;
    }
    else
    {
      return false;
    }
  }
public:
  string GetValue(const string& valueName) override
  {
    UNIMPLEMENTED();
  }
};

std::string SessionImpl::Expand(const string& toBeExpanded)
{
  DefaultCallback callback(this);
  return Expand(toBeExpanded, &callback);
}

std::string SessionImpl::Expand(const string& toBeExpanded, HasNamedValues* callback)
{
  return Expand(toBeExpanded, { ExpandOption::Values }, callback);
}

std::string SessionImpl::Expand(const string& toBeExpanded, ExpandOptionSet options, HasNamedValues* callback)
{
  string result = toBeExpanded;
  if (options[ExpandOption::Braces])
  {
    result = MakeSearchPath(ExpandBraces(result));
  }
  if (options[ExpandOption::Values])
  {
    result = ExpandValues(result, callback);
  }
  if (options[ExpandOption::Braces])
  {
    result = MakeSearchPath(ExpandBraces(result));
  }
  if (options[ExpandOption::PathPatterns])
  {
    result = MakeSearchPath(ExpandPathPatterns(result));
  }
  return result;
}

std::string SessionImpl::ExpandValues(const string& toBeExpanded, HasNamedValues* callback)
{
  const char* lpsz = toBeExpanded.c_str();
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
      else if (lpsz[1] == '{' || isalpha(lpsz[1]) || lpsz[1] == '_')
      {
        const char* lpszBegin = lpsz;
        char endChar = lpsz[1] == '{' ? '}' : 0;
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
          haveValue = TryGetConfigValue("", valueName, value);
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
        expansion += '$';
      }
    }
    else
    {
      expansion += *lpsz;
    }
  }
  return expansion;
}

ShellCommandMode SessionImpl::GetShellCommandMode()
{
  string shellCommandMode = GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_SHELLCOMMANDMODE).GetString();
  if (shellCommandMode == "Forbidden")
  {
    return ShellCommandMode::Forbidden;
  }
  else if (shellCommandMode == "Query")
  {
    return ShellCommandMode::Query;
  }
  else if (shellCommandMode == "Restricted")
  {
    return ShellCommandMode::Restricted;
  }
  else if (shellCommandMode == "Unrestricted")
  {
    return ShellCommandMode::Unrestricted;
  }
  else
  {
    MIKTEX_FATAL_ERROR_2(T_("Invalid configuration: unknown shell command mode."), "shellCommandMode", shellCommandMode);
  }
}

vector<string> SessionImpl::GetAllowedShellCommands()
{
  return GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_ALLOWEDSHELLCOMMANDS).GetStringArray();
}

tuple<Session::ExamineCommandLineResult, string, string> SessionImpl::ExamineCommandLine(const string& commandLine)
{
  Argv argv(commandLine);
  if (argv.GetArgc() == 0 || string(argv[0]).find_first_of(" \t") != string::npos)
  {
    return make_tuple(ExamineCommandLineResult::SyntaxError, "", "");
  }
  PathName argv0(argv[0]);
  vector<string> allowedCommands = GetAllowedShellCommands();
  ExamineCommandLineResult examineResult = std::find_if(allowedCommands.begin(), allowedCommands.end(), [argv0](const string& cmd) { return argv0 == cmd; }) != allowedCommands.end()
    ? ExamineCommandLineResult::ProbablySafe
    : ExamineCommandLineResult::MaybeSafe;
  MIKTEX_ASSERT(argv0.ToString().find_first_of(" \t") == string::npos);
  string toBeExecuted = argv[0];
#if defined(MIKTEX_WINDOWS)
  const char quoteChar = '"';
#else
  const char quoteChar = '\'';
#endif
  for (int idx = 1; idx < argv.GetArgc(); ++idx)
  {
    toBeExecuted += ' ';
    toBeExecuted += quoteChar;
    toBeExecuted += argv[idx];
    toBeExecuted += quoteChar;
  }
  return make_tuple(examineResult, argv[0], toBeExecuted);
}
