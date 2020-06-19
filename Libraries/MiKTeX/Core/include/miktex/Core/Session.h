/* miktex/Core/Session.h:                               -*- C++ -*-

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

#pragma once

#if !defined(B4CE13E8A2514CC8B46D9F6D49EEDC60)
#define B4CE13E8A2514CC8B46D9F6D49EEDC60

#include <miktex/Core/config.h>

#if defined(MIKTEX_WINDOWS)
#  include <Windows.h>
#endif

#include <cstddef>

#include <chrono>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include <miktex/Trace/TraceCallback>

#include "Exceptions.h"
#include "File.h"
#include "FileType.h"
#include "HasNamedValues.h"
#include "OptionSet.h"
#include "PathName.h"
#include "Process.h"
#include "RootDirectoryInfo.h"
#include "TriState.h"
#include "VersionNumber.h"

/// @namespace MiKTeX::Core
/// @brief The core namespace.
MIKTEX_CORE_BEGIN_NAMESPACE;

/// An invalid TEXMF root index.
const unsigned INVALID_ROOT_INDEX = static_cast<unsigned>(-1);

/// MiKTeX configurations.
enum class MiKTeXConfiguration
{
  None,
  /// Regular MiKTeX.
  Regular,
  /// MiKTeXDirect (CD/DVD).
  Direct,
  /// MiKTeX Portable.
  Portable,
};

inline std::ostream& operator<<(std::ostream& os, const MiKTeXConfiguration& miktexConfiguration)
{
  switch (miktexConfiguration)
  {
  case MiKTeXConfiguration::None: os << "None"; break;
  case MiKTeXConfiguration::Regular: os << "Regular"; break;
  case MiKTeXConfiguration::Direct: os << "Direct"; break;
  case MiKTeXConfiguration::Portable: os << "Portable"; break;
  }
  return os;
}

/// MiKTeX configuration scope.
enum class ConfigurationScope
{
  None,
  /// User configuration.
  User,
  /// Common (system-wide) configuration.
  Common
};

inline std::ostream& operator<<(std::ostream& os, const ConfigurationScope& scope)
{
  switch (scope)
  {
  case ConfigurationScope::None: os << "None"; break;
  case ConfigurationScope::User: os << "User"; break;
  case ConfigurationScope::Common: os << "Common"; break;
  }
  return os;
}

/// Startup configuration.
struct StartupConfig
{
  /// User configuration root directory.
public:
  PathName userConfigRoot;

  /// User data root directory.
public:
  PathName userDataRoot;

  /// User installation root directory.
public:
  PathName userInstallRoot;

  /// Additional TEXMF roots defined by the user.
public:
  std::string userRoots;

  /// User root directories defined by other TeX systems.
public:
  std::string otherUserRoots;

  /// System-wide configuration root directory.
public:
  PathName commonConfigRoot;

  /// System-wide data root directory.
public:
  PathName commonDataRoot;

  /// System-wide installation root directory.
public:
  PathName commonInstallRoot;

  /// Additional TEXMF root defined by the admin.
public:
  std::string commonRoots;

  /// System-wide root directories defined by other TeX systems.
public:
  std::string otherCommonRoots;

  /// The MiKTeX configuration type.
public:
  MiKTeXConfiguration config = MiKTeXConfiguration::None;
};

inline std::ostream& operator<<(std::ostream& os, const StartupConfig& startupConfig)
{
  if (!startupConfig.userConfigRoot.Empty())
  {
    os << "UserConfig=" << startupConfig.userConfigRoot << ",";
  }
  if (!startupConfig.userDataRoot.Empty())
  {
    os << "UserData=" << startupConfig.userDataRoot << ",";
  }
  if (!startupConfig.userInstallRoot.Empty())
  {
    os << "UserInstall=" << startupConfig.userInstallRoot << ",";
  }
  if (!startupConfig.userRoots.empty())
  {
    os << "UserRoots=" << startupConfig.userRoots << ",";
  }
  if (!startupConfig.otherUserRoots.empty())
  {
    os << "OtherUserRoots=" << startupConfig.otherUserRoots << ",";
  }
  if (!startupConfig.commonConfigRoot.Empty())
  {
    os << "CommonConfig=" << startupConfig.commonConfigRoot << ",";
  }
  if (!startupConfig.commonDataRoot.Empty())
  {
    os << "CommonData=" << startupConfig.commonDataRoot << ",";
  }
  if (!startupConfig.commonInstallRoot.Empty())
  {
    os << "CommonInstall=" << startupConfig.commonInstallRoot << ",";
  }
  if (!startupConfig.commonRoots.empty())
  {
    os << "CommonRoots=" << startupConfig.commonRoots << ",";
  }
  if (!startupConfig.otherCommonRoots.empty())
  {
    os << "OtherCommonRoots=" << startupConfig.otherCommonRoots << ",";
  }
  os << "MiKTeXConfiguration=" << startupConfig.config;
  return os;
}

/// Special path names.
enum class SpecialPath
{
  /// User configuration root directory.
  UserConfigRoot,

  /// User data root directory.
  UserDataRoot,

  /// User installation root directory.
  UserInstallRoot,

  /// System-wide configuration root directory.
  CommonConfigRoot,

  /// System-wide data root directory.
  CommonDataRoot,

  /// System-wide installation root directory.
  CommonInstallRoot,

  /// The portable root directory.
  PortableRoot,

  /// The portable mount directory.
  PortableMount,

  /// The root directory of the MiKTeX installation.
  DistRoot,

#if defined(MIKTEX_MACOS_BUNDLE)
  /// The directory which contains the main application file.
  /// Example: `/Applications/MiKTeX\ Console.app/Contents/Macos`
  MacOsDirectory,
#endif

  /// The effective configuration root directory.
  ConfigRoot,

  /// The effective data root directory.
  DataRoot,

  /// The effective installation root directory.
  InstallRoot,

  /// The effective directory for MiKTeX binaries.
  BinDirectory,

  /// The effective directory for internal MiKTeX binaries.
  InternalBinDirectory,

  /// The effective directory in which to create symbolic links to the MiKTeX executables.
  LinkTargetDirectory,

  /// The effective directory for log files.
  LogDirectory,
};

/// Paper size information.
class PaperSizeInfo
{
public:
  static MIKTEXCORECEEAPI(PaperSizeInfo) Parse(const std::string& spec);

  /// Paper name (e.g., `A4`)
public:
  std::string name;

public:
  std::string dvipsName;

  /// Paper width (in 72nds of an inch).
public:
  int width;

  /// Paper height (in 72nds of an inch).
public:
  int height;
};

/// METAFONT mode.
struct MIKTEXMFMODE
{
  std::string mnemonic;
  std::string description;
  int horizontalResolution;
  int verticalResolution;
};

/// File type information.
struct FileTypeInfo
{
  /// The file type.
  FileType fileType = FileType::None;
  /// The file type as a string.
  std::string fileTypeString;
  /// List of file name extensions associated with the file type.
  std::vector<std::string> fileNameExtensions;
  /// Extra list of file name extensions associated with the file type.
  std::vector<std::string> alternateExtensions;
  /// Search path for files.
  std::vector<std::string> searchPath;
  /// List of environment variable names.
  std::vector<std::string> envVarNames;
};

/// File information.
struct FileInfoRecord
{
  std::string fileName;
  std::string packageName;
  FileAccess access = FileAccess::None;
};

/// User information.
struct MiKTeXUserInfo
{
  enum { Developer = 1, Contributor = 2, Sponsor = 4, KnownUser = 8 };
  enum { Individual = 100 };
  std::string userid;
  std::string name;
  std::string organization;
  std::string email;
  int role = 0;
  int level = 0;
  time_t expirationDate = static_cast<time_t>(-1);
  bool IsMember () const
  {
    return level >= Individual && (expirationDate == static_cast<time_t>(-1) || expirationDate >= time(nullptr));
  }
  bool IsDeveloper () const { return IsMember() && (role & Developer) != 0; }
  bool IsContributor () const { return IsMember() && (role & Contributor) != 0; }
  bool IsSponsor () const { return IsMember() && (role & Sponsor) != 0; }
  bool IsKnownUser () const { return IsMember() && (role & KnownUser) != 0; }
};

/// Information about a TeX format.
struct FormatInfo
{
  /// The access key.
  std::string key;
  /// Name of the format.
  std::string name;
  /// One-line description.
  std::string description;
  /// The compiler (engine) which processes the format.
  std::string compiler;
  /// The name of the input file.
  std::string inputFile;
  /// The name of the output file.
  std::string outputFile;
  /// The name of another format which has to be pre-loaded.
  std::string preloaded;
  /// Exclusion flag. If set, the format will be ignored by initexmf.
  bool exclude = false;
  /// NoExecutable flag. If set, no executable will be created by initexmf.
  bool noExecutable = false;
  /// Custom flag. Set, if this format was defined by the user.
  bool custom = false;
  /// Extra engine arguments.
  std::string arguments;
};

/// Language information.
struct LanguageInfo
{
  std::string key;
  std::string synonyms;
  std::string loader;
  std::string patterns;
  std::string hyphenation;
  std::string luaspecial;
  int lefthyphenmin = -1;
  int righthyphenmin = -1;
  bool exclude = false;
  bool custom = false;
};

/// MiKTeX configuration value.
class ConfigValue
{
public:
  ConfigValue()
  {
  };

public:
  ConfigValue(const ConfigValue& other)
  {
    switch (other.type)
    {
    case Type::String:
      new (&this->s) std::string(other.s);
      break;
    case Type::Int:
      this->i = other.i;
      break;
    case Type::Bool:
      this->b = other.b;
      break;
    case Type::Tri:
      this->t = other.t;
      break;
    case Type::Char:
      this->c = other.c;
      break;
    case Type::StringArray:
      new (&this->sa) std::vector<std::string>(other.sa);
      break;
    case Type::None:
      break;
    }
    this->type = other.type;
    this->section = other.section;
    this->description = other.description;
  }

public:
  ConfigValue& operator=(const ConfigValue& other) = delete;

public:
  ConfigValue(ConfigValue&& other) noexcept
  {
    switch (other.type)
    {
    case Type::String:
      new (&this->s) std::string(other.s);
      this->s = std::move(other.s);
      break;
    case Type::Int:
      this->i = other.i;
      break;
    case Type::Bool:
      this->b = other.b;
      break;
    case Type::Tri:
      this->t = other.t;
      break;
    case Type::Char:
      this->c = other.c;
      break;
    case Type::StringArray:
      new (&this->sa) std::vector<std::string>(other.sa);
      this->sa = std::move(other.sa);
      break;
    case Type::None:
      break;
    }
    this->type = other.type;
    this->section = other.section;
  }

public:
  ConfigValue& operator=(ConfigValue&& other) noexcept
  {
    if (this->type == Type::String && other.type != Type::String)
    {
      this->s.~basic_string();
    }
    else if (this->type == Type::StringArray && other.type != Type::StringArray)
    {
      this->sa.~vector();
    }
    switch (other.type)
    {
    case Type::String:
      if (this->type != Type::String)
      {
        new (&this->s) std::string(other.s);
      }
      this->s = std::move(other.s);
      break;
    case Type::Int:
      this->i = other.i;
      break;
    case Type::Bool:
      this->b = other.b;
      break;
    case Type::Tri:
      this->t = other.t;
      break;
    case Type::Char:
      this->c = other.c;
      break;
    case Type::StringArray:
      if (this->type != Type::StringArray)
      {
        new (&this->sa) std::vector<std::string>(other.sa);
      }
      this->sa = std::move(other.sa);
      break;
    case Type::None:
      break;
    }
    this->type = other.type;
    this->section = other.section;
    return *this;
  }

public:
  virtual ~ConfigValue() noexcept
  {
    if (type == Type::String)
    {
      this->s.~basic_string();
    }
    else if (type == Type::StringArray)
    {
      this->sa.~vector();
    }
    type = Type::None;
  }

public:
  explicit ConfigValue(const std::string& s)
  {
    new(&this->s) std::string(s);
    type = Type::String;
  }

public:
  explicit ConfigValue(const char* lpsz)
  {
    new(&this->s) std::string(lpsz == nullptr ? "" : lpsz);
    type = Type::String;
  }

public:
  explicit ConfigValue(int i)
  {
    this->i = i;
    type = Type::Int;
  }

public:
  explicit ConfigValue(bool b)
  {
    this->b = b;
    type = Type::Bool;
  }

public:
  explicit ConfigValue(TriState t)
  {
    this->t = t;
    type = Type::Tri;
  }

public:
  explicit ConfigValue(char c)
  {
    this->c = c;
    type = Type::Char;
  }

public:
  explicit ConfigValue(const std::vector<std::string>& sa)
  {
    new(&this->sa) std::vector<std::string>(sa);
    type = Type::StringArray;
  }

  /// Gets the configuration value as a string.
  /// @return Returns the configuration value.
public:
  MIKTEXCORETHISAPI(std::string) GetString() const;

  /// Gets the configuration value as an integer.
  /// @return Returns the configuration value.
public:
  MIKTEXCORETHISAPI(int) GetInt() const;

  /// Gets the configuration value as a boolean.
  /// @return Returns the configuration value.
public:
  MIKTEXCORETHISAPI(bool) GetBool() const;

  /// Gets the configuration value as a tri-state.
  /// @return Returns the configuration value.
public:
  MIKTEXCORETHISAPI(TriState) GetTriState() const;

  /// Gets the configuration value as a character.
  /// @return Returns the configuration value.
public:
  MIKTEXCORETHISAPI(char) GetChar() const;

  /// Gets the configuration value as a time_t.
  /// @return Returns the configuration value.
public:
  MIKTEXCORETHISAPI(std::time_t) GetTimeT() const;

  /// Gets the configuration value as a string list.
  /// @return Returns the configuration value.
public:
  MIKTEXCORETHISAPI(std::vector<std::string>) GetStringArray() const;

  /// Value type.
public:
  enum class Type
  {
    None,
    String,
    Int,
    Bool,
    Tri,
    Char,
    StringArray
  };

private:
  Type type = Type::None;

  /// Gets the value type.
  /// @return Returns the value type
public:
  Type GetType() const
  {
    return type;
  }

  /// Tests whether the configuration value is defined.
  /// @return Returns `true`, if the configuration value is defined.
public:
  bool HasValue() const
  {
    return type != Type::None;
  }

private:
  std::string section;

  /// Gets the section of the configuration value.
  /// @return Returns the section name.
public:
  std::string GetSection() const
  {
    return section;
  }

private:
  std::string description;

  /// Gets the description of the configuration value.
  /// @return Returns the description.
public:
  std::string GetDescription() const
  {
    return description;
  }

private:
  union
  {
    std::string s;
    int i;
    bool b;
    TriState t;
    char c;
    std::vector<std::string> sa;
  };
};

/// Expansion options.
enum class ExpandOption
{
  /// Expand values.
  Values,
  /// Expand braces.
  Braces,
  /// Expand path patterns.
  PathPatterns
};

typedef OptionSet<ExpandOption> ExpandOptionSet;

/// Root registration options.
enum class RegisterRootDirectoriesOption
{
#if defined(MIKTEX_WINDOWS)
  /// Don't store in registry.
  NoRegistry,
#endif
  /// Only temporary registration.
  Temporary,
  /// Check if the registration is valid.
  Review
};

typedef OptionSet<RegisterRootDirectoriesOption> RegisterRootDirectoriesOptionSet;

/// Shell command mode.
enum class ShellCommandMode
{
  /// Shell commands are forbidden.
  Forbidden,
  /// Only some shell commands are allowed.
  Restricted,
  /// Ask the user.
  Query,
  /// All shell commands are allowed.
  Unrestricted
};

/// Find file callback interface.
class MIKTEXNOVTABLE IFindFileCallback
{
  /// Installs a package.
  /// @param packageId Identifies the package.
  /// @param trigger File name that triggered the auto-installer.
  /// @param[out] installRoot Installation root directory.
  /// @return Returns `true`, if the package has been installed.
public:
  virtual bool MIKTEXTHISCALL InstallPackage(const std::string& packageId, const PathName& trigger, PathName& installRoot) = 0;

  /// Tries to create a file.
  /// @param fileName Name of the file to create.
  /// @param fileType Type of the file.
  /// @return Returns `true`, if the file has been created.
public:
  virtual bool MIKTEXTHISCALL TryCreateFile(const PathName& fileName, FileType fileType) = 0;
};

/// The MiKTeX session interface.
class MIKTEXNOVTABLE Session
{
public:
  /// Find file options.
  enum class FindFileOption
  {
    /// Try to create the file, if it doesn't exist.
    Create,
    /// Renew the file.
    Renew,
    /// Return all files.
    All,
    /// Search the file system.
    SearchFileSystem
  };

public:
  typedef OptionSet<FindFileOption> FindFileOptionSet;

public:
  /// Initialization options.
  enum class InitOption
  {
    /// We are setting up MiKTeX.
    SettingUp,
    /// Don't fix `PATH`.
    NoFixPath,
#if defined(MIKTEX_WINDOWS)
    /// Initialize the COM library.
    InitializeCOM,
#endif
    /// start in administrator mode.
    AdminMode,
  };

  /// Init flags enum.
  typedef OptionSet<InitOption> InitOptionSet;

  /// Extended initializion options.
public:
  class InitInfo
  {
  public:
    InitInfo() = default;

  public:
    InitInfo(const InitInfo& other) = default;

  public:
    InitInfo& operator=(const InitInfo& other) = default;

  public:
    InitInfo(InitInfo&& other) = default;

  public:
    InitInfo& operator=(InitInfo&& other) = default;

  public:
    ~InitInfo() = default;

    /// Constructor.
    /// @param programInvocationName Name of the program.
  public:
    InitInfo(const std::string& programInvocationName) :
      programInvocationName(programInvocationName)
    {
    }

    /// Constructor.
    /// @param programInvocationName Name of the program.
    /// @param flags Initialization options.
  public:
    InitInfo(const std::string& programInvocationName, InitOptionSet options) :
      options(options),
      programInvocationName(programInvocationName)
    {
    }

    /// Sets initialization options.
    /// @param options the options to set.
  public:
    void SetOptions(InitOptionSet options)
    {
      this->options = options;
    }

    /// Gets initialization options.
    /// @return Returns initialization options.
  public:
    InitOptionSet GetOptions() const
    {
      return options;
    }

    /// Add an initializtion option.
    /// @param option The option to add.
  public:
    void AddOption(InitOption option)
    {
      this->options += option;
    }

    /// Sets the name of the program.
    /// @param programInvocationName The name of the program.
  public:
    void SetProgramInvocationName(const std::string& programInvocationName)
    {
      this->programInvocationName = programInvocationName;
    }

    /// Gets the name of the program.
    /// @return Returns the program name.
  public:
    std::string GetProgramInvocationName() const
    {
      return programInvocationName;
    }

    /// Sets the descriptive program name.
    /// @param theNameOfTheGame The descriptive program name.
  public:
    void SetTheNameOfTheGame(const std::string& theNameOfTheGame)
    {
      this->theNameOfTheGame = theNameOfTheGame;
    }

    /// Gets the descriptive program name.
    /// @return Returns the descriptive program name.
  public:
    std::string GetTheNameOfTheGame() const
    {
      return theNameOfTheGame;
    }

    /// Sets the startup options.
    /// @param startupConfig The startup options to set.
  public:
    void SetStartupConfig(const StartupConfig& startupConfig)
    {
      this->startupConfig = startupConfig;
    }

    /// Gets the startup options.
    /// @return Return s the startup options.
  public:
    StartupConfig GetStartupConfig() const
    {
      return startupConfig;
    }

    /// Enables trace streams.
    /// @param The trace streams (by name) to enable.
  public:
    void SetTraceFlags(const std::string& traceFlags)
    {
      this->traceFlags = traceFlags;
    }

    /// Gets enabled trace streams.
    /// @return Returns enabled trace stream names.
  public:
    std::string GetTraceFlags() const
    {
      return traceFlags;
    }

    /// Sets the trace callback interface.
    /// @param callback Pointer to an object implementing the trace callback interface.
  public:
    void SetTraceCallback(MiKTeX::Trace::TraceCallback* callback)
    {
      traceCallback = callback;
    }

    /// Gets the trace callback interface.
    /// @return Returns the pointer to an object implementing the trace callback interface.
  public:
    MiKTeX::Trace::TraceCallback* GetTraceCallback()
    {
      return traceCallback;
    }

  private:
    InitOptionSet options;

  private:
    std::string programInvocationName;

  private:
    std::string theNameOfTheGame;

  private:
    StartupConfig startupConfig;

  private:
    std::string traceFlags;

  private:
    MiKTeX::Trace::TraceCallback* traceCallback = nullptr;
  };

  /// Information about an open file.
public:
  struct OpenFileInfo
  {
    const FILE* file = nullptr;
    std::string fileName;
    FileMode mode = FileMode::Open;
    FileAccess access = FileAccess::None;
  };

  /// Gets the current session object.
  /// @return Returns a smart pointer to the current session object.
public:
  static MIKTEXCORECEEAPI(std::shared_ptr<Session>) Get();

  /// Tries to get the current session object.
  /// @return Returns a smart pointer to the current session object. Can be `nullptr`.
public:
  static MIKTEXCORECEEAPI(std::shared_ptr<Session>) TryGet();

  /// Creates a new session object.
  /// @param initInfo Initializtion options.
  /// @return Returns a smart pointer to the new session object.
public:
  static MIKTEXCORECEEAPI(std::shared_ptr<Session>) Create(const InitInfo& initInfo);

public:
  virtual MIKTEXTHISCALL ~Session() noexcept = 0;

  /// Resets this session object.
public:
  virtual void MIKTEXTHISCALL Reset() = 0;

  /// Adds an application name.
  /// @param name The name to add.
public:
  virtual void MIKTEXTHISCALL PushAppName(const std::string& name) = 0;

  /// Adds an application name with a lower priority.
  /// @param name The name to add.
public:
  virtual void MIKTEXTHISCALL PushBackAppName(const std::string& name) = 0;

  /// Adds an inpit directory.
  /// @param path The file system path to the directory.
  /// @param atEnd Indicates wheter the directory shall be added at the end of the list.
public:
  virtual void MIKTEXTHISCALL AddInputDirectory(const PathName& path, bool atEnd) = 0;

  /// Gets a special file system path.
  /// @param specialPath Identifies the special path.
  /// @return Returns a path name.
public:
  virtual PathName MIKTEXTHISCALL GetSpecialPath(SpecialPath specialPath) = 0;

  /// Gets registered root directories.
  /// @return Returns the list of registered root directories.
public:
  virtual std::vector<RootDirectoryInfo> GetRootDirectories() = 0;

  /// Gets the number of root directories.
  /// @return Returns the number of registered root directories.
public:
  virtual unsigned MIKTEXTHISCALL GetNumberOfTEXMFRoots() = 0;

  /// Gets the path to a root directory.
  /// @param r Identifies the root directory.
  /// @return Returns the file system path to the root directory.
public:
  virtual PathName MIKTEXTHISCALL GetRootDirectoryPath(unsigned r) = 0;

  /// Tests whether a root directory is registered system-wide.
  /// @param r Identifies the root directory.
  /// @return Returns `true`, if this is a system-wide root directory.
public:
  virtual bool MIKTEXTHISCALL IsCommonRootDirectory(unsigned r) = 0;

  /// Tests whether a root directory is from another TeX system.
  /// @param r Identifies the root directory.
  /// @return Returns `true`, if this root directory is from another TeX system.
public:
  virtual bool MIKTEXTHISCALL IsOtherRootDirectory(unsigned r) = 0;

  /// Gets the virtual path to the MPM root directory.
  /// @return Returns the virtual path to the MPM root directory.
public:
  virtual PathName MIKTEXTHISCALL GetMpmRootPath() = 0;

  /// Gets a path to the MPM file name database.
  /// @return Returns the path to the MPM file name database.
public:
  virtual PathName MIKTEXTHISCALL GetMpmDatabasePathName() = 0;

  /// Tries to get a root directory index from a file system path.
  /// @param path The file system path.
  /// @return Returns a root directory index (or `INVALID_ROOT_INDEX`).
public:
  virtual unsigned MIKTEXTHISCALL TryDeriveTEXMFRoot(const PathName& path) = 0;

  /// Get a root directory index from a file system path.
  /// @param path The file system path.
  /// @return Returns a root directory index.
public:
  virtual unsigned MIKTEXTHISCALL DeriveTEXMFRoot(const PathName& path) = 0;

  /// Finds a file name database.
  /// @param r Identifies the root directory.
  /// @param[out] path The file system path to the databsae.
  /// @return Returns `true`, if the database was found.
public:
  virtual bool MIKTEXTHISCALL FindFilenameDatabase(unsigned r, PathName& path) = 0;

  /// Gets the file system path to a file name database file.
  /// @param r Identifies the root directory.
  /// @return Returns the file system path to the databsae.
public:
  virtual PathName MIKTEXTHISCALL GetFilenameDatabasePathName(unsigned r) = 0;

  /// Unloads the file name database.
  /// @return Returns `true`, if the file name database could be unloaded.
  /// Returns `false`, if the database is still in use.
public:
  virtual bool MIKTEXTHISCALL UnloadFilenameDatabase() = 0;

  /// Unloads the file name database.
  /// @param minIdleTime The minimum time the database has not been used.
  /// @return Returns `true`, if the file name database could be unloaded.
  /// Returns `false`, if the database is still in use.
public:
  virtual bool MIKTEXTHISCALL UnloadFilenameDatabase(std::chrono::duration<double> minIdleTime) = 0;

  /// Splits a TEXMF file system path.
  /// @param path The path to split.
  /// @param[out] root The path to the root directory.
  /// @param[out] relative The relative path from the root directory.
  /// @return Returns the root directory index.
public:
  virtual unsigned MIKTEXTHISCALL SplitTEXMFPath(const PathName& path, PathName& root, PathName& relative) = 0;

  /// Registers TEXMF root directories.
  /// @param startupConfig Specifies the root directories to register.
  /// @param options Registration options.
public:
  virtual void MIKTEXTHISCALL RegisterRootDirectories(const StartupConfig& startupConfig, RegisterRootDirectoriesOptionSet options) = 0;

  /// Registers an additional root directory.
  /// @param path The file system path to the root directory.
  /// @param other Indicates wheter the root directory is from another TeX system.
public:
virtual void MIKTEXTHISCALL RegisterRootDirectory(const PathName& path, bool other) = 0;

  /// Unregisters an additional root directory.
  /// @param path The file system path to the root directory.
  /// @param other Indicates wheter the root directory is from another TeX system.
public:
virtual void MIKTEXTHISCALL UnregisterRootDirectory(const PathName& path, bool other) = 0;

  /// Moves a root directory up in the list.
  /// @param r Identifies the root directory.
public:
  virtual void MIKTEXTHISCALL MoveRootDirectoryUp(unsigned r) = 0;

  /// Moves a root directory down in the list.
  /// @param r Identifies the root directory.
public:
  virtual void MIKTEXTHISCALL MoveRootDirectoryDown(unsigned r) = 0;

  /// Tests if this is MiKTeXDirect.
  /// @return Returns `true`, if this is a MiKTeXDirect configuration
public:
  virtual bool MIKTEXTHISCALL IsMiKTeXDirect() = 0;

  /// Tests if this is MiKTeX Portable.
  /// @return Returns `true`, if this is a MiKTeX Portable configuration
public:
  virtual bool MIKTEXTHISCALL IsMiKTeXPortable() = 0;

  /// Gets the next METAFONT mode.
  /// @param The index of the next METAFONT mode.
  /// @param[out] mode The METAFONT mode.
  /// @return Returns `true`, if the METAFONT mode could be retrieved.
public:
  virtual bool MIKTEXTHISCALL GetMETAFONTMode(unsigned idx, MIKTEXMFMODE& mode) = 0;

  /// Gets the METAFONT mode matching a given resolution.
  /// @param dpi The resolution (in dots per inch).
  /// @param[out] mode The METAFONT mode.
  /// @return Returns `true`, if the METAFONT mode could be retrieved.
public:
  virtual bool MIKTEXTHISCALL DetermineMETAFONTMode(unsigned dpi, MIKTEXMFMODE& Mode) = 0;

  /// Tries to get a configuration value.
  /// @param sectionName Identifies the configuration section.
  /// @param valueName Identifies the value within the section.
  /// @param callback The pointer to an object which implements the `HasNamedValue` interface.
  /// @param[out] value The configuration value as a string.
  /// @return Returns `true`, if the value was found.
public:
  virtual bool MIKTEXTHISCALL TryGetConfigValue(const std::string& sectionName, const std::string& valueName, HasNamedValues* callback, std::string& value) = 0;

  /// Tries to get a configuration value.
  /// @param sectionName Identifies the configuration section.
  /// @param valueName Identifies the value within the section.
  /// @param[out] value The configuration value as a string.
  /// @return Returns `true`, if the value was found.
public:
  virtual bool MIKTEXTHISCALL TryGetConfigValue(const std::string& sectionName, const std::string& valueName, std::string& value) = 0;

  /// Gets a configuration value.
  /// @param sectionName Identifies the configuration section.
  /// @param valueName Identifies the value within the section.
  /// @param defaultValue Value to be returned if the requested value was not found.
  /// @param callback The pointer to an object which implements the `HasNamedValue` interface.
  /// @return Returns the configuration value.
  /// @see SetConfigValue
public:
  virtual ConfigValue MIKTEXTHISCALL GetConfigValue(const std::string& sectionName, const std::string& valueName, const ConfigValue& defaultValue, HasNamedValues* callback) = 0;

  /// Gets a configuration value.
  /// @param sectionName Identifies the configuration section.
  /// @param valueName Identifies the value within the section.
  /// @param defaultValue Value to be returned if the requested value was not found.
  /// @return Returns the configuration value.
  /// @see SetConfigValue
public:
  virtual ConfigValue MIKTEXTHISCALL GetConfigValue(const std::string& sectionName, const std::string& valueName, const ConfigValue& defaultValue) = 0;

  /// Gets a configuration value.
  /// @param sectionName Identifies the configuration section.
  /// @param valueName Identifies the value within the section.
  /// @param callback The pointer to an object which implements the `HasNamedValue` interface.
  /// @return Returns the configuration value.
  /// @see SetConfigValue
public:
  virtual ConfigValue MIKTEXTHISCALL GetConfigValue(const std::string& sectionName, const std::string& valueName, HasNamedValues* callback) = 0;

  /// Gets a configuration value.
  /// @param sectionName Identifies the configuration section.
  /// @param valueName Identifies the value within the section.
  /// @return Returns the configuration value.
  /// @see SetConfigValue
public:
  virtual ConfigValue MIKTEXTHISCALL GetConfigValue(const std::string& sectionName, const std::string& valueName) = 0;

  /// Sets a configuration value.
  /// @param sectionName Identifies the configuration section.
  /// @param valueName Identifies the value within the section.
  /// @param The configuration value.
  /// @see GetConfigValue
public:
  virtual void MIKTEXTHISCALL SetConfigValue(const std::string& sectionName, const std::string& valueName, const ConfigValue& value) = 0;

  /// Gets the name of the running engine (e.g., `pdftex`).
  /// @return Returns the engine name.
public:
  virtual std::string MIKTEXTHISCALL GetEngineName() = 0;

  /// Opens a file.
  /// @param path The file system path to the file.
  /// @param mode Specifies how the file is to open.
  /// @param access Specifies how the file will be accessed.
  /// @param isTextFile Specifies if the file should be opened in text mode.
  /// @return Returns the pointer to a `FILE` object.
public:
  virtual FILE* MIKTEXTHISCALL OpenFile(const PathName& path, FileMode mode, FileAccess access, bool isTextFile) = 0;

  /// Tries to open a file.
  /// @param path The file system path to the file.
  /// @param mode Specifies how the file is to open.
  /// @param access Specifies how the file will be accessed.
  /// @param isTextFile Specifies if the file should be opened in text mode.
  /// @return Returns the pointer to a `FILE` object. Returns `nullptr`, if
  /// the file could not be opened.
public:
  virtual FILE* MIKTEXTHISCALL TryOpenFile(const PathName& path, FileMode mode, FileAccess access, bool isTextFile) = 0;

  /// Tries to get information about an open file.
  /// @param file The pointer to the `FILE` object.
  /// @Returns The first return value indicates whether the second value contains the
  /// requested information.
public:
  virtual std::pair<bool, OpenFileInfo> MIKTEXTHISCALL TryGetOpenFileInfo(FILE* file) = 0;

  /// Closes a file.
  /// @param file The pointer to the `FILE` object.
public:
  virtual void MIKTEXTHISCALL CloseFile(FILE* file, int& exitCode) = 0;

  /// Closes a file.
  /// @param file The pointer to the `FILE` object.
public:
  virtual void MIKTEXTHISCALL CloseFile(FILE* file) = 0;


  /// Tests whether a file has been opened for output.
  /// @param file The pointer to the `FILE` object.
  /// @return Returns `true`, if it is an output file.
public:
  virtual bool MIKTEXTHISCALL IsOutputFile(const FILE* file) = 0;

#if defined(MIKTEX_WINDOWS)
  /// Tests if a file as been opened.
  /// @param fileName Name of the file to be checked.
  /// @return Returns true, if the file has been opened.
public:
  virtual bool MIKTEXTHISCALL IsFileAlreadyOpen(const PathName& fileName) = 0;
#endif

  /// Schedules the execution of a shell command when the application terminates.
  /// @param commandLine The shell command to execute.
public:
  virtual void MIKTEXTHISCALL ScheduleSystemCommand(const std::string& commandLine) = 0;

#if defined(MIKTEX_WINDOWS)
  /// Schedules the removal of a file when the application terminates.
  /// @param path The file system path to the file.
public:
  virtual void MIKTEXTHISCALL ScheduleFileRemoval(const PathName& path) = 0;
#endif

  /// Starts recording file names
  /// @return Returns `true`.
public:
  virtual bool MIKTEXTHISCALL StartFileInfoRecorder() = 0;

  /// Starts recording file names
  /// @param recordPackageNames Indicates whether to include package names.
  /// @return Returns `true`.
public:
  virtual bool MIKTEXTHISCALL StartFileInfoRecorder(bool recordPackageNames) = 0;

  /// Sets the file name recorder log file.
  /// @param path The file system path to the log file.
public:
  virtual void MIKTEXTHISCALL SetRecorderPath(const PathName& path) = 0;

  /// Adds a file name record to the log file.
  /// @param The file system file to the file.
  /// @param How the file is accessed.
public:
  virtual void MIKTEXTHISCALL RecordFileInfo(const PathName& path, FileAccess access) = 0;

  /// Gets the recorded file names.
  /// @return Returns the recorded file names.
public:
  virtual std::vector<FileInfoRecord> MIKTEXTHISCALL GetFileInfoRecords() = 0;

  /// Gets the file type from a given file name.
  /// @param fileName The file name.
  /// @return Returns the derived file type.
public:
  virtual FileType MIKTEXTHISCALL DeriveFileType(const PathName& fileName) = 0;

  /// Searches a file.
  /// @param fileName The name of the file to search.
  /// @param pathList The search path.
  /// @param options Search options.
  /// @param[out] result The result of the search.
  /// @return Return `true`, if the file was found.
public:
  virtual bool MIKTEXTHISCALL FindFile(const std::string& fileName, const std::string& pathList, FindFileOptionSet options, std::vector<PathName>& result) = 0;

  /// Searches a file.
  /// @param fileName The name of the file to search.
  /// @param pathList The search path.
  /// @param[out] result The result of the search.
  /// @return Return `true`, if the file was found.
public:
  virtual bool MIKTEXTHISCALL FindFile(const std::string& fileName, const std::string& pathList, std::vector<PathName>& result) = 0;

  /// Searches a file.
  /// @param fileName The name of the file to search.
  /// @param pathList The search path.
  /// @param options Search options.
  /// @param[out] result The result of the search.
  /// @return Return `true`, if the file was found.
public:
  virtual bool MIKTEXTHISCALL FindFile(const std::string& fileName, const std::string& pathList, FindFileOptionSet options, PathName& result) = 0;

  /// Searches a file.
  /// @param fileName The name of the file to search.
  /// @param pathList The search path.
  /// @param[out] result The result of the search.
  /// @return Return `true`, if the file was found.
public:
  virtual bool MIKTEXTHISCALL FindFile(const std::string& fileName, const std::string& pathList, PathName& result) = 0;

  /// Searches a file.
  /// @param fileName The name of the file to search.
  /// @param fileType The file type to search for.
  /// @param options Search options.
  /// @param[out] result The result of the search.
  /// @return Return `true`, if the file was found.
public:
  virtual bool MIKTEXTHISCALL FindFile(const std::string& fileName, FileType fileType, FindFileOptionSet options, std::vector<PathName>& result) = 0;

  /// Searches a file.
  /// @param fileName The name of the file to search.
  /// @param fileType The file type to search for.
  /// @param[out] result The result of the search.
  /// @return Return `true`, if the file was found.
public:
  virtual bool MIKTEXTHISCALL FindFile(const std::string& fileName, FileType fileType, std::vector<PathName>& result) = 0;

  /// Searches a file.
  /// @param fileName The name of the file to search.
  /// @param fileType The file type to search for.
  /// @param options Search options.
  /// @param[out] result The result of the search.
  /// @return Return `true`, if the file was found.
public:
  virtual bool MIKTEXTHISCALL FindFile(const std::string& fileName, FileType fileType, FindFileOptionSet options, PathName& result) = 0;

  /// Searches a file.
  /// @param fileName The name of the file to search.
  /// @param fileType The file type to search for.
  /// @param[out] result The result of the search.
  /// @return Return `true`, if the file was found.
public:
  virtual bool MIKTEXTHISCALL FindFile(const std::string& fileName, FileType fileType, PathName& result) = 0;

  /// Searches a PK font file.
  /// @param fontName The name of the font to search.
  /// @param mfMode The METAFONT mode.
  /// @param dpi The resolution (in dots per inch) of the requested PK font.
  /// @param[out] result The result of the search.
  /// @return Return `true`, if the file was found.
public:
  virtual bool MIKTEXTHISCALL FindPkFile(const std::string& fontName, const std::string& mfMode, int dpi, PathName& result) = 0;

  /// Searches a TFM file.
  /// @param fontName The name of the font to search.
  /// @param[out] result The result of the search.
  /// @param create Indicates, if the TFM file should be created if it was not found.
  /// @return Return `true`, if the file was found.
public:
  virtual bool MIKTEXTHISCALL FindTfmFile(const std::string& fontName, PathName& result, bool create) = 0;

  /// Sets the callback interface for file search.
  /// @param callback The pointer to an object which implements the interface.
public:
  virtual void MIKTEXTHISCALL SetFindFileCallback(IFindFileCallback* callback) = 0;

  /// Splits the file system path of a font file.
  /// @param fontPath The file system path to the font file.
  /// @param[out] fontType The font type.
  /// @param[out] supplier The font supplier.
  /// @param[out] typeface The typeface.
  /// @param[out] fontName The font name.
  /// @param[oit] pointSize The size of the font.
public:
  virtual void MIKTEXTHISCALL SplitFontPath(const PathName& fontPath, std::string* fontType, std::string* supplier, std::string* typeface, std::string* fontName, std::string* pointSize) = 0;

  /// Searches a font file.
  /// @param fontName The name of the font to search.
  /// @param[out] The supplier of the font.
  /// @param[out] The typeface.
  /// @param[out] genSize The size of the font.
  /// @return Returns `true`, if the font was found.
public:
  virtual bool MIKTEXTHISCALL GetFontInfo(const std::string& fontName, std::string& supplier, std::string& typeface, double* genSize) = 0;

  /// Searches the Ghostscript program.
  /// @param[out] versionNumber The Ghostscript version number
  /// @return Returns the file system path to the Ghostscript program file.
public:
  virtual PathName MIKTEXTHISCALL GetGhostscript(unsigned long* versionNumber) = 0;

  /// Gets the seach path for a file type.
  /// @param fileType The file type.
  /// @return Returns the expanded search path.
public:
  virtual std::string MIKTEXTHISCALL GetExpandedSearchPath(FileType fileType) = 0;

  /// Converts a graphics file into a Bitmap file.
  /// @param sourceFileName The file system path to the source file.
  /// @param[out] destFileName The file system path to the destination file.
  /// @param callback The pointer to an object implementing the `IRunProcessCallback` callback.
  /// @return Returns `true`, if the conversion was successful.
public:
  virtual bool MIKTEXTHISCALL ConvertToBitmapFile(const PathName& sourceFileName, PathName& destFileName, IRunProcessCallback* callback) = 0;

  /// Enables or disables automatic font file creation.
  /// @param enable Indicates whether automatic font file creation is enabled.
  /// @return Returns `true`, if automatic font creation file was enabled.
public:
  virtual bool MIKTEXTHISCALL EnableFontMaker(bool enable) = 0;

  /// Tests whether automatic font file creation is enabled.
  /// @return Returns `true`, if automatic font creation file is enabled.
public:
  virtual bool MIKTEXTHISCALL GetMakeFontsFlag() = 0;

  /// Build a command-line for `makepk`.
  /// @param fontName The Name of the font.
  /// @param dpi The requested resolution (in dots per inch).
  /// @param bdpi The base resolution (in dots per inch).
  /// @param mfMode The METAFONT mode.
  /// @param[out] fileName The name of the PK font file.
  /// @param enableInstaller Indicates whether automatic package installation is allowed.
  /// @return Returns the `makepk` command-line arguments.
public:
  virtual std::vector<std::string> MIKTEXTHISCALL MakeMakePkCommandLine(const std::string& fontName, int dpi, int baseDpi, const std::string& mfMode, PathName& fileName, TriState enableInstaller) = 0;

#if defined(MIKTEX_WINDOWS)
  /// Executes a Windows batch script.
  /// @param argc Number of command-line arguments.
  /// @param argv Pointers to command-line arguments. Must be terminated with `nullptr`.
  /// @return Returns the exit code of the batch script.
public:
  virtual int MIKTEXTHISCALL RunBatch(int argc, const char** argv) = 0;
#endif

  /// Runs an executable.
  /// @param argc Number of command-line arguments.
  /// @param argv Pointers to command-line arguments. Must be terminated with `nullptr`.
  /// @return Returns the exit code.
public:
  virtual int MIKTEXTHISCALL RunExe(int argc, const char** argv) = 0;

  /// Runs Ghostscript.
  /// @param argc Number of command-line arguments.
  /// @param argv Pointers to command-line arguments. Must be terminated with `nullptr`.
  /// @return Returns the exit code.
public:
  virtual int MIKTEXTHISCALL RunGhostscript(int argc, const char** argv) = 0;

#if !defined(MIKTEX_WINDOWS)
  /// Executes a shell script.
  /// @param argc Number of command-line arguments.
  /// @param argv Pointers to command-line arguments. Must be terminated with `nullptr`.
  /// @return Returns the exit code of the shell script.
public:
  virtual int MIKTEXTHISCALL RunSh(int argc, const char** argv) = 0;
#endif

  /// Executes a Perl script.
  /// @param argc Number of command-line arguments.
  /// @param argv Pointers to command-line arguments. Must be terminated with `nullptr`.
  /// @return Returns the exit code of the Perl script.
public:
  virtual int MIKTEXTHISCALL RunPerl(int argc, const char** argv) = 0;

  /// Executes a Python script.
  /// @param argc Number of command-line arguments.
  /// @param argv Pointers to command-line arguments. Must be terminated with `nullptr`.
  /// @return Returns the exit code of the Python script.
public:
  virtual int MIKTEXTHISCALL RunPython(int argc, const char** argv) = 0;

  /// Executes a Java program.
  /// @param argc Number of command-line arguments.
  /// @param argv Pointers to command-line arguments. Must be terminated with `nullptr`.
  /// @return Returns the exit code of the Perl script.
public:
  virtual int MIKTEXTHISCALL RunJava(int argc, const char** argv) = 0;

#if defined(MIKTEX_WINDOWS)
  /// Shows a help topic.
  /// @param hWnd The handle of a parent window.
  /// @param topic Identifies the topic.
  /// @return Returns `true`, if the topic was shown.
public:
  virtual bool MIKTEXTHISCALL ShowManualPageAndWait(HWND hWnd, unsigned long topic) = 0;
#endif

  /// Gets all file types.
  /// @return Returns all file types.
public:
  virtual std::vector<FileTypeInfo> MIKTEXTHISCALL GetFileTypes() = 0;

  /// Gets all formats.
  /// @return Returns all formats.
public:
  virtual std::vector<FormatInfo> MIKTEXTHISCALL GetFormats() = 0;

  /// Gets a format.
  /// @param key Identifies the format.
  /// @return Returns the format.
public:
  virtual FormatInfo MIKTEXTHISCALL GetFormatInfo(const std::string& key) = 0;

  /// Gets a format.
  /// @param key Identifies the format.
  /// @param[out] formatInfo The format.
  /// @return Returns `true`, if the format was found.
public:
  virtual bool MIKTEXTHISCALL TryGetFormatInfo(const std::string& key, FormatInfo& formatInfo) = 0;

  /// Removes a format.
  /// @param key Identifies the format.
public:
  virtual void MIKTEXTHISCALL DeleteFormatInfo(const std::string& key) = 0;

  /// Updates a format.
  /// @param formatInfo The format.
public:
  virtual void MIKTEXTHISCALL SetFormatInfo(const FormatInfo& formatInfo) = 0;

  /// Gets all languages.
  /// @return Returns all languages.
public:
  virtual std::vector<LanguageInfo> MIKTEXTHISCALL GetLanguages() = 0;

  /// Gets the file system path to the running program file.
  /// @param canonicalized Indicates wheter symbolic links should be followed.
  /// @return Returns the file system path to the running program file.
public:
  virtual PathName MIKTEXTHISCALL GetMyProgramFile(bool canonicalized) = 0;

  /// Gets the file system path to the directory of the running program file.
  /// @param canonicalized Indicates wheter symbolic links should be followed.
  /// @return Returns the file system path to the directory of running program file.
public:
  virtual PathName MIKTEXTHISCALL GetMyLocation(bool canonicalized) = 0;

  /// Gets the file system path to the prefix directory of the running program file.
  /// @param canonicalized Indicates wheter symbolic links should be followed.
  /// @return Returns the file system path to the prefix directory.
public:
  virtual PathName MIKTEXTHISCALL GetMyPrefix(bool canonicalized) = 0;

  /// Tests whether the program runs with elevated privileges.
  /// @return Returns `true`, if the program is running with elevated privileges.
public:
  virtual bool MIKTEXTHISCALL RunningAsAdministrator() = 0;

  /// Turns admin mode on or off.
  /// @param adminMode Indicates whether admin mode shall be turned on.
  /// @param force Enforces admin mode even for a private installation.
public:
  virtual void MIKTEXTHISCALL SetAdminMode(bool adminMode, bool force = false) = 0;

  /// Tests whether the program is running in admin mode.
  /// @return Returns `true`, if the program is running in admin mode.
public:
  virtual bool MIKTEXTHISCALL IsAdminMode() = 0;

  /// Tests whether this is a system-wide installation.
  /// @return Returns `true`, if this is a system-wide installation.
public:
  virtual bool MIKTEXTHISCALL IsSharedSetup() = 0;

public:
  /// Get the version number of the initial setup.
  /// @return Returns the version number of the initial setup.
  virtual VersionNumber MIKTEXTHISCALL GetSetupVersionNumber() = 0;

public:
  /// Set the version number of the initial setup.
  /// @param The version number of the initial setup.
  virtual void MIKTEXTHISCALL SetSetupVersionNumber(VersionNumber setupVersion) = 0;

  /// Gets the next paper size.
  /// @param idx Index of the next entry in the paper size table.
  /// @param[out] The paper size.
  /// @return Return `true`, if the next paper size entry could be retrieved.
public:
  virtual bool MIKTEXTHISCALL GetPaperSizeInfo(int idx, PaperSizeInfo& paperSize) = 0;

  /// Gets a paper size.
  /// @param dvipsName Identifies the paper size.
  /// @return Returns the paper size.
public:
  virtual PaperSizeInfo MIKTEXTHISCALL GetPaperSizeInfo(const std::string& dvipsName) = 0;

  /// Sets the default paper size.
  /// @param dvipsName Identifies the paper size.
public:
  virtual void MIKTEXTHISCALL SetDefaultPaperSize(const std::string& dvipsName) = 0;

  /// Tries to create a file by its template.
  /// @param The file system path to the file.
  /// @return Returns `true`, if the file has been created.
public:
  virtual bool MIKTEXTHISCALL TryCreateFromTemplate(const PathName& path) = 0;

  /// Tests whether the current user is an administrator.
  /// @return Returns `true`, if the current user is an administrator.
public:
  virtual bool MIKTEXTHISCALL IsUserAnAdministrator() = 0;

  /// Configures a file.
  /// @param pathIn The file system path to the source file.
  /// @param pathOut The file system path to the destination file.
  /// @param callback The pointer to an object which implements the `HasNamedValue` interface.
public:
  virtual void MIKTEXTHISCALL ConfigureFile(const PathName& pathIn, const PathName& pathOut, HasNamedValues* callback = nullptr) = 0;

  /// Configures a file.
  /// @param pathRel The relative file system path to the destination file.
  /// @param callback The pointer to an object which implements the `HasNamedValue` interface.
public:
  virtual void MIKTEXTHISCALL ConfigureFile(const PathName& pathRel, HasNamedValues* callback = nullptr) = 0;

  /// Sets the descriptive name of the running program.
  /// @param name The descriptive name.
public:
  virtual void MIKTEXTHISCALL SetTheNameOfTheGame(const std::string& name) = 0;

  /// Get the system font directories.
  /// @return Returns the system font directories.
public:
  virtual std::vector<std::string> MIKTEXTHISCALL GetFontDirectories() = 0;

  /// Gets file type data.
  /// @param filetype Identifies the file type.
  /// @return Returns the file type data.
public:
  virtual FileTypeInfo MIKTEXTHISCALL GetFileTypeInfo(FileType fileType) = 0;

  /// Expands valuus references in a string.
  /// @param toBeExpanded The string to expand.
  /// @return Returns the expanded string.
public:
  virtual std::string Expand(const std::string& toBeExpanded) = 0;

  /// Expands value references in a string.
  /// @param toBeExpanded The string to expand.
  /// @param callback The pointer to an object which implements the `HasNamedValues` interface.
  /// @return Returns the expanded string.
public:
  virtual std::string Expand(const std::string& toBeExpanded, HasNamedValues* callback) = 0;

  /// Expands a string.
  /// @param toBeExpanded The string to expand.
  /// @param options What to expand.
  /// @param callback The pointer to an object which implements the `HasNamedValues` interface.
  /// @return Returns the expanded string.
public:
  virtual std::string Expand(const std::string& toBeExpanded, ExpandOptionSet options, HasNamedValues* callback) = 0;

  /// Updates language data.
  /// @param languageInfo The language data.
public:
  virtual void MIKTEXTHISCALL SetLanguageInfo(const LanguageInfo& languageInfo) = 0;

#if HAVE_MIKTEX_USER_INFO
  /// Registers a MiKTeX user.
  /// @param info Registration data.
  /// @return Returns the registration data.
public:
  virtual MiKTeXUserInfo RegisterMiKTeXUser(const MiKTeXUserInfo& info) = 0;
#endif

#if HAVE_MIKTEX_USER_INFO
  /// Tries to get registration data
  /// @param[out] info Registration data.
  /// @return Returns `true`, if registration data is avaialable.
public:
  virtual bool TryGetMiKTeXUserInfo(MiKTeXUserInfo& info) = 0;
#endif

  /// Gets the configured shell command mode.
  /// @return Returns the configured shell command mode.
public:
  virtual ShellCommandMode GetShellCommandMode() = 0;

  /// Gets allowed shell commands.
  /// @return Returns allowed shell commands.
public:
  virtual std::vector<std::string> GetAllowedShellCommands() = 0;

  /// Result of a command-line examination.
public:
  enum class ExamineCommandLineResult {
    /// It's probably safe to run the command.
    ProbablySafe,
    /// It's probably unsafe to run the command.
    MaybeSafe,
    /// It's not safe to run the command.
    NotSafe,
    /// The command-line is not well formed.
    SyntaxError
  };

  /// Examines a command-line.
  /// @param commandLine The command-line to examine.
  /// @returns Returns three values:
  /// 1. The result of the examination.
  /// 2. The command (`argv[0]`) to be executed.
  /// 3. The safe command-line.
public:
  virtual std::tuple<ExamineCommandLineResult, std::string, std::string> ExamineCommandLine(const std::string& commandLine) = 0;

  /// Throws a C/C++-runtime exception.
  /// @param functionName The name of the C/C++-runtime function.
  /// @param errorCode The `errno` value.
  /// @param info Additional information.
  /// @param sourceLocation The source code location.
public:
  static MIKTEXCOREEXPORT MIKTEXNORETURN void MIKTEXCEECALL FatalCrtError(const std::string& functionName, int errorCode, const MiKTeXException::KVMAP& info, const SourceLocation& sourceLocation);

  /// Throws a MiKTeX exception.
  /// @param message The short error message.
  /// @param description A more detailed description of the error.
  /// @param remedy A user friendly recipe to remedy the error.
  /// @param tag An error tag.
  /// @param info Additional information.
  /// @param sourceLocation The source code location.
public:
  static MIKTEXCOREEXPORT MIKTEXNORETURN void MIKTEXCEECALL FatalMiKTeXError(const std::string& message, const std::string& description, const std::string& remedy, const std::string& tag, const MiKTeXException::KVMAP& info, const SourceLocation& sourceLocation);

#if defined(MIKTEX_WINDOWS)
  /// Throws a Windows exception.
  /// @param functionName The name of the Windows function.
  /// @param errorCode The last Windows error value.
  /// @param description A more detailed description of the error.
  /// @param remedy A user friendly recipe to remedy the error.
  /// @param tag An error tag.
  /// @param info Additional information.
  /// @param sourceLocation The source code location.
public:
  static MIKTEXCOREEXPORT MIKTEXNORETURN void MIKTEXCEECALL FatalWindowsError(const std::string& functionName, unsigned long errorCode, const std::string& description, const std::string& remedy, const std::string& tag, const MiKTeXException::KVMAP& info, const SourceLocation& sourceLocation);
#endif
};

MIKTEX_CORE_END_NAMESPACE;

#define MIKTEX_FATAL_ERROR(message) \
  MiKTeX::Core::Session::FatalMiKTeXError(message, "", "", "", MiKTeX::Core::MiKTeXException::KVMAP(), MIKTEX_SOURCE_LOCATION())

#define MIKTEX_FATAL_ERROR_2(message, ...) \
  MiKTeX::Core::Session::FatalMiKTeXError(message, "", "", "", MiKTeX::Core::MiKTeXException::KVMAP(__VA_ARGS__), MIKTEX_SOURCE_LOCATION())

#define MIKTEX_FATAL_ERROR_3(message, description, ...) \
  MiKTeX::Core::Session::FatalMiKTeXError(message, description, "", "", MiKTeX::Core::MiKTeXException::KVMAP(__VA_ARGS__), MIKTEX_SOURCE_LOCATION())

#define MIKTEX_FATAL_ERROR_4(message, description, remedy, ...) \
  MiKTeX::Core::Session::FatalMiKTeXError(message, description, remedy, "", MiKTeX::Core::MiKTeXException::KVMAP(__VA_ARGS__), MIKTEX_SOURCE_LOCATION())

#define MIKTEX_FATAL_ERROR_5(message, description, remedy, tag, ...) \
  MiKTeX::Core::Session::FatalMiKTeXError(message, description, remedy, tag, MiKTeX::Core::MiKTeXException::KVMAP(__VA_ARGS__), MIKTEX_SOURCE_LOCATION())

#define MIKTEX_INTERNAL_ERROR() MIKTEX_FATAL_ERROR(MIKTEXTEXT("MiKTeX encountered an internal error."))

#define MIKTEX_UNEXPECTED() MIKTEX_INTERNAL_ERROR()

#define MIKTEX_EXPECT(p) if (!(p)) { MIKTEX_UNEXPECTED(); }

#define MIKTEX_FATAL_CRT_ERROR(functionName)                            \
  {                                                                     \
    int errorCode = errno;                                              \
    MiKTeX::Core::Session::FatalCrtError(functionName, errorCode, MiKTeX::Core::MiKTeXException::KVMAP(), MIKTEX_SOURCE_LOCATION()); \
  }

#define MIKTEX_FATAL_CRT_ERROR_2(functionName, ...)                     \
  {                                                                     \
    int errorCode = errno;                                              \
    MiKTeX::Core::Session::FatalCrtError(functionName, errorCode, MiKTeX::Core::MiKTeXException::KVMAP(__VA_ARGS__), MIKTEX_SOURCE_LOCATION()); \
  }

#define MIKTEX_FATAL_CRT_RESULT(functionName, errorCode) \
  MiKTeX::Core::Session::FatalCrtError(functionName, errorCode, MiKTeX::Core::MiKTeXException::KVMAP(), MIKTEX_SOURCE_LOCATION());

#define MIKTEX_FATAL_CRT_RESULT_2(functionName, errorCode, ...) \
  MiKTeX::Core::Session::FatalCrtError(functionName, errorCode, MiKTeX::Core::MiKTeXException::KVMAP(__VA_ARGS__), MIKTEX_SOURCE_LOCATION());

#if defined(MIKTEX_WINDOWS)

#define MIKTEX_FATAL_WINDOWS_ERROR(functionName)                        \
  {                                                                     \
    unsigned long errorCode = GetLastError();                           \
    MiKTeX::Core::Session::FatalWindowsError(functionName, errorCode, "", "", "", MiKTeX::Core::MiKTeXException::KVMAP(), MIKTEX_SOURCE_LOCATION()); \
  }

#define MIKTEX_FATAL_WINDOWS_ERROR_2(functionName, ...)                 \
  {                                                                     \
    unsigned long errorCode = GetLastError();                           \
    MiKTeX::Core::Session::FatalWindowsError(functionName, errorCode, "", "", "", MiKTeX::Core::MiKTeXException::KVMAP(__VA_ARGS__), MIKTEX_SOURCE_LOCATION()); \
  }

#define MIKTEX_FATAL_WINDOWS_ERROR_3(functionName, description, ...)   \
  {                                                                     \
    unsigned long errorCode = GetLastError();                           \
    MiKTeX::Core::Session::FatalWindowsError(functionName, errorCode, description, "", "", MiKTeX::Core::MiKTeXException::KVMAP(__VA_ARGS__), MIKTEX_SOURCE_LOCATION()); \
  }

#define MIKTEX_FATAL_WINDOWS_ERROR_4(functionName, description, remedy, ...)   \
  {                                                                     \
    unsigned long errorCode = GetLastError();                           \
    MiKTeX::Core::Session::FatalWindowsError(functionName, errorCode, description, remedy, "", MiKTeX::Core::MiKTeXException::KVMAP(__VA_ARGS__), MIKTEX_SOURCE_LOCATION()); \
  }

#define MIKTEX_FATAL_WINDOWS_ERROR_5(functionName, description, remedy, tag, ...)   \
  {                                                                     \
    unsigned long errorCode = GetLastError();                           \
    MiKTeX::Core::Session::FatalWindowsError(functionName, errorCode, description, remedy, tag, MiKTeX::Core::MiKTeXException::KVMAP(__VA_ARGS__), MIKTEX_SOURCE_LOCATION()); \
  }

#define MIKTEX_FATAL_WINDOWS_RESULT(functionName, errorCode) \
  MiKTeX::Core::Session::FatalWindowsError(functionName, errorCode, "", "", "", MiKTeX::Core::MiKTeXException::KVMAP(), MIKTEX_SOURCE_LOCATION())

#define MIKTEX_FATAL_WINDOWS_RESULT_2(functionName, errorCode, ...) \
  MiKTeX::Core::Session::FatalWindowsError(functionName, errorCode, "", "", "", MiKTeX::Core::MiKTeXException::KVMAP(__VA_ARGS__), MIKTEX_SOURCE_LOCATION())

#define MIKTEX_FATAL_WINDOWS_RESULT_3(functionName, errorCode, description, ...) \
  MiKTeX::Core::Session::FatalWindowsError(functionName, errorCode, description, "", "", MiKTeX::Core::MiKTeXException::KVMAP(__VA_ARGS__), MIKTEX_SOURCE_LOCATION())

#define MIKTEX_FATAL_WINDOWS_RESULT_4(functionName, errorCode, description, remedy, ...) \
  MiKTeX::Core::Session::FatalWindowsError(functionName, errorCode, description, remedy, "", MiKTeX::Core::MiKTeXException::KVMAP(__VA_ARGS__), MIKTEX_SOURCE_LOCATION())

#define MIKTEX_FATAL_WINDOWS_RESULT_5(functionName, errorCode, description, remedy, tag, ...) \
  MiKTeX::Core::Session::FatalWindowsError(functionName, errorCode, description, remedy, tag, MiKTeX::Core::MiKTeXException::KVMAP(__VA_ARGS__), MIKTEX_SOURCE_LOCATION())

#endif

#endif
