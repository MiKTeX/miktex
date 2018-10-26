/* miktex/Core/Session.h:                               -*- C++ -*-

   Copyright (C) 1996-2018 Christian Schenk

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

#include <memory>
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

MIKTEX_CORE_BEGIN_NAMESPACE;

const unsigned INVALID_ROOT_INDEX = static_cast<unsigned>(-1);

enum class MiKTeXConfiguration
{
  None,
  Regular,
  Direct,
  Portable,
};

struct StartupConfig
{
public:
  std::string userRoots;

public:
  PathName userInstallRoot;

public:
  PathName userDataRoot;

public:
  PathName userConfigRoot;

public:
  std::string commonRoots;

public:
  PathName commonDataRoot;

public:
  PathName commonConfigRoot;

public:
  PathName commonInstallRoot;

public:
  std::string otherUserRoots;

public:
  std::string otherCommonRoots;

public:
  MiKTeXConfiguration config = MiKTeXConfiguration::None;
};

/// Special path enum class.
enum class SpecialPath
{
  /// The system-wide installation root directory.
  CommonInstallRoot,

  // The user's installation root directory.
  UserInstallRoot,

  /// The system-wide data root directory.
  CommonDataRoot,

  /// The user's data root directory.
  UserDataRoot,

  /// The system-wide configuration root directory.
  CommonConfigRoot,

  /// The user's configuration root directory.
  UserConfigRoot,

  /// The effective installation root directory.
  InstallRoot,

  /// The effective data root directory.
  DataRoot,

  /// The effective configuration root directory.
  ConfigRoot,

  /// The effective directory for MiKTeX binaries.
  BinDirectory,

  /// The directory for internal MiKTeX binaries.
  InternalBinDirectory,

  /// The portable root directory.
  PortableRoot,

  /// The portable mount directory.
  PortableMount,

  /// The effective directory for symbolic links to MiKTeX binaries.
  LocalBinDirectory,

  /// The effective directory for log files.
  LogDirectory,

  /// The root directory of the MiKTeX installation.
  DistRoot,

#if defined(MIKTEX_MACOS_BUNDLE)
  /// The directory which contains the main application file.
  /// Example: /Applications/MiKTeX\ Console.app/Contents/Macos
  MacOsDirectory
#endif
};

/// Paper size info.
class PaperSizeInfo
{
public:
  static MIKTEXCORECEEAPI(PaperSizeInfo) Parse(const std::string& spec);

  /// Paper name (e.g., "A4")
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

struct MIKTEXMFMODE
{
  std::string mnemonic;
  std::string description;
  int horizontalResolution;
  int verticalResolution;
};

struct FileTypeInfo
{
  FileType fileType = FileType::None;
  std::string fileTypeString;
  std::vector<std::string> fileNameExtensions;
  std::vector<std::string> alternateExtensions;
  std::vector<std::string> searchPath;
  std::vector<std::string> envVarNames;
};

struct FileInfoRecord
{
  std::string fileName;
  std::string packageName;
  FileAccess access = FileAccess::None;
};

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

  std::string arguments;
};

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

class ConfigValue
{
public:
  ConfigValue()
  {
  }

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
  ConfigValue(ConfigValue&& other)
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
  ConfigValue& operator=(ConfigValue&& other)
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
  ConfigValue(const std::string& s)
  {
    new(&this->s) std::string(s);
    type = Type::String;
  }

public:
  ConfigValue(const char* lpsz)
  {
    new(&this->s) std::string(lpsz == nullptr ? "" : lpsz);
    type = Type::String;
  }

public:
  ConfigValue(int i)
  {
    this->i = i;
    type = Type::Int;
  }

public:
  ConfigValue(bool b)
  {
    this->b = b;
    type = Type::Bool;
  }

public:
  ConfigValue(TriState t)
  {
    this->t = t;
    type = Type::Tri;
  }

public:
  ConfigValue(char c)
  {
    this->c = c;
    type = Type::Char;
  }

public:
  ConfigValue(const std::vector<std::string>& sa)
  {
    new(&this->sa) std::vector<std::string>(sa);
    type = Type::StringArray;
  }

public:
  MIKTEXCORETHISAPI(std::string) GetString() const;

public:
  MIKTEXCORETHISAPI(int) GetInt() const;

public:
  MIKTEXCORETHISAPI(bool) GetBool() const;

public:
  MIKTEXCORETHISAPI(TriState) GetTriState() const;

public:
  MIKTEXCORETHISAPI(char) GetChar() const;

public:
  MIKTEXCORETHISAPI(std::vector<std::string>) GetStringArray() const;

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

public:
  Type GetType() const
  {
    return type;
  }

public:
  bool HasValue() const
  {
    return type != Type::None;
  }

private:
  std::string section;

public:
  std::string GetSection() const
  {
    return section;
  }

private:
  std::string description;

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

enum class ExpandOption
{
  Values,
  Braces,
  PathPatterns
};

typedef OptionSet<ExpandOption> ExpandOptionSet;

enum class RegisterRootDirectoriesOption
{
#if defined(MIKTEX_WINDOWS)
  NoRegistry,
#endif
  Temporary,
  Review
};

typedef OptionSet<RegisterRootDirectoriesOption> RegisterRootDirectoriesOptionSet;

enum class ShellCommandMode
{
  Forbidden,
  Restricted,
  Query,
  Unrestricted
};

/// Find file callback interface
class MIKTEXNOVTABLE IFindFileCallback
{
  /// Request to install package. Called by the FindFile machinery.
  /// @param deploymentName Package to be installed.
  /// @param trigger File name that triggered the auto-installer.
  /// @param[out] installRoot Installation root directory.
  /// @return Returns true, if the package has been installed.
public:
  virtual bool MIKTEXTHISCALL InstallPackage(const std::string& deploymentName, const PathName& trigger, PathName& installRoot) = 0;

public:
  virtual bool MIKTEXTHISCALL TryCreateFile(const PathName& fileName, FileType fileType) = 0;
};

/// The MiKTeX session interface.
class MIKTEXNOVTABLE Session
{
public:
  enum class FindFileOption
  {
    Create,
    Renew,
    All,
    TryHard
  };

public:
  typedef OptionSet<FindFileOption> FindFileOptionSet;

public:
  /// Init flags enum class.
  enum class InitOption
  {
    /// No config files will be loaded.
    NoConfigFiles,
    /// Don't fix PATH.
    NoFixPath,
#if defined(MIKTEX_WINDOWS)
    /// Initialize the COM library.
    InitializeCOM,
#endif
    /// start in administrator mode
    AdminMode,
  };

  /// Init flags enum.
  typedef OptionSet<InitOption> InitOptionSet;

  /// Init info struct.
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

    /// Initializes a new init info struct.
    /// @param programInvocationName Name of the invoked program.
  public:
    InitInfo(const std::string& programInvocationName) :
      programInvocationName(programInvocationName)
    {
    }

    /// Initializes a new init info struct.
    /// @param programInvocationName Name of the invoked program.
    /// @param flags Init flags.
  public:
    InitInfo(const std::string& programInvocationName, InitOptionSet options) :
      options(options),
      programInvocationName(programInvocationName)
    {
    }

    /// Sets init flags.
  public:
    void SetOptions(InitOptionSet options)
    {
      this->options = options;
    }

    /// Gets init flags.
  public:
    InitOptionSet GetOptions() const
    {
      return options;
    }

  public:
    void AddOption(InitOption option)
    {
      this->options += option;
    }

    /// Sets the name of the invoked program.
  public:
    void SetProgramInvocationName(const std::string& programInvocationName)
    {
      this->programInvocationName = programInvocationName;
    }

    /// Gets the name of the invoked program.
  public:
    std::string GetProgramInvocationName() const
    {
      return programInvocationName;
    }

    /// Sets the name of the game.
  public:
    void SetTheNameOfTheGame(const std::string& theNameOfTheGame)
    {
      this->theNameOfTheGame = theNameOfTheGame;
    }

    /// Gets the name of the game.
  public:
    std::string GetTheNameOfTheGame() const
    {
      return theNameOfTheGame;
    }

  public:
    void SetStartupConfig(const StartupConfig& startupConfig)
    {
      this->startupConfig = startupConfig;
    }

  public:
    StartupConfig GetStartupConfig() const
    {
      return startupConfig;
    }

    /// Enables trace streams.
  public:
    void SetTraceFlags(const std::string& traceFlags)
    {
      this->traceFlags = traceFlags;
    }

    /// Gets enabled trace streams.
  public:
    std::string GetTraceFlags() const
    {
      return traceFlags;
    }

  public:
    void SetTraceCallback(MiKTeX::Trace::TraceCallback* callback)
    {
      traceCallback = callback;
    }

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

public:
  struct OpenFileInfo
  {
    const FILE* file = nullptr;
    std::string fileName;
    FileMode mode = FileMode::Open;
    FileAccess access = FileAccess::None;
  };

public:
  static MIKTEXCORECEEAPI(std::shared_ptr<Session>) Get();

public:
  static MIKTEXCORECEEAPI(std::shared_ptr<Session>) TryGet();

public:
  static MIKTEXCORECEEAPI(std::shared_ptr<Session>) Create(const InitInfo& initInfo);

public:
  virtual MIKTEXTHISCALL ~Session() noexcept = 0;

public:
  virtual void MIKTEXTHISCALL Reset() = 0;

public:
  virtual void MIKTEXTHISCALL PushAppName(const std::string& name) = 0;

public:
  virtual void MIKTEXTHISCALL PushBackAppName(const std::string& name) = 0;

public:
  virtual void MIKTEXTHISCALL AddInputDirectory(const PathName& path, bool atEnd) = 0;

  /// Gets a special path.
  /// @param specialPath Specifies special path.
  /// @return Returns a path name.
public:
  virtual PathName MIKTEXTHISCALL GetSpecialPath(SpecialPath specialPath) = 0;

public:
  virtual std::vector<RootDirectoryInfo> GetRootDirectories() = 0;

  /// Gets the number of root directories.
public:
  virtual unsigned MIKTEXTHISCALL GetNumberOfTEXMFRoots() = 0;

public:
  virtual PathName MIKTEXTHISCALL GetRootDirectoryPath(unsigned r) = 0;

public:
  virtual bool MIKTEXTHISCALL IsCommonRootDirectory(unsigned r) = 0;

public:
  virtual bool MIKTEXTHISCALL IsOtherRootDirectory(unsigned r) = 0;

  /// Gets the path name of the virtual MPM TEXMF root.
public:
  virtual PathName MIKTEXTHISCALL GetMpmRootPath() = 0;

  /// Gets the path name of the MPM file name database.
public:
  virtual PathName MIKTEXTHISCALL GetMpmDatabasePathName() = 0;

public:
  virtual unsigned MIKTEXTHISCALL TryDeriveTEXMFRoot(const PathName& path) = 0;

public:
  virtual unsigned MIKTEXTHISCALL DeriveTEXMFRoot(const PathName& path) = 0;

  /// Finds a file name database file.
  /// @param r File name database ID.
  /// @param path Path name object buffer to be filled.
public:
  virtual bool MIKTEXTHISCALL FindFilenameDatabase(unsigned r, PathName& path) = 0;

  /// Gets the path name of a file name database file.
  /// @param r File name database ID.
  /// @param path Path name object buffer to be filled.
public:
  virtual PathName MIKTEXTHISCALL GetFilenameDatabasePathName(unsigned r) = 0;

  /// Unloads the file name database.
  /// @return Returns true, if the file name database could be unloaded.
  /// Returns false, if the database is still in use.
public:
  virtual bool MIKTEXTHISCALL UnloadFilenameDatabase() = 0;

  /// Splits a TEXMF path name.
  /// @param path Path name to split.
  /// @param root String buffer to be filled with the root directory.
  /// @param relative String buffer to be filled with the relative path.
  /// @return Returns the TEXMF ID.
public:
  virtual unsigned MIKTEXTHISCALL SplitTEXMFPath(const PathName& path, PathName& root, PathName& relative) = 0;

public:
  virtual void MIKTEXTHISCALL RegisterRootDirectories(const std::string& roots, bool other) = 0;

public:
  virtual void MIKTEXTHISCALL RegisterRootDirectories(const StartupConfig& startupConfig, RegisterRootDirectoriesOptionSet options) = 0;

public:
  virtual void MIKTEXTHISCALL RegisterRootDirectory(const PathName& path) = 0;

public:
  virtual void MIKTEXTHISCALL UnregisterRootDirectory(const PathName& path) = 0;

public:
  virtual void MIKTEXTHISCALL MoveRootDirectoryUp(unsigned r) = 0;

public:
  virtual void MIKTEXTHISCALL MoveRootDirectoryDown(unsigned r) = 0;

public:
  virtual bool MIKTEXTHISCALL IsMiKTeXDirect() = 0;

public:
  virtual bool MIKTEXTHISCALL IsMiKTeXPortable() = 0;

public:
  virtual bool MIKTEXTHISCALL GetMETAFONTMode(unsigned idx, MIKTEXMFMODE& mode) = 0;

public:
  virtual bool MIKTEXTHISCALL DetermineMETAFONTMode(unsigned dpi, MIKTEXMFMODE& Mode) = 0;

public:
  virtual bool MIKTEXTHISCALL TryGetConfigValue(const std::string& sectionName, const std::string& valueName, std::string& value) = 0;

public:
  virtual ConfigValue MIKTEXTHISCALL GetConfigValue(const std::string& sectionName, const std::string& valueName, const ConfigValue& defaultValue) = 0;

public:
  virtual ConfigValue MIKTEXTHISCALL GetConfigValue(const std::string& sectionName, const std::string& valueName) = 0;

public:
  virtual void MIKTEXTHISCALL SetConfigValue(const std::string& sectionName, const std::string& valueName, const ConfigValue& value) = 0;

public:
  virtual std::string MIKTEXTHISCALL GetEngineName() = 0;

public:
  virtual FILE* MIKTEXTHISCALL OpenFile(const PathName& path, FileMode mode, FileAccess access, bool isTextFile) = 0;

public:
  virtual FILE* MIKTEXTHISCALL TryOpenFile(const PathName& path, FileMode mode, FileAccess access, bool isTextFile) = 0;

public:
  virtual FILE* MIKTEXTHISCALL OpenFile(const PathName& path, FileMode mode, FileAccess access, bool isTextFile, FileShare share) = 0;

public:
  virtual FILE* MIKTEXTHISCALL TryOpenFile(const PathName& path, FileMode mode, FileAccess access, bool isTextFile, FileShare share) = 0;

public:
  virtual std::pair<bool, OpenFileInfo> MIKTEXTHISCALL TryGetOpenFileInfo(FILE* file) = 0;

public:
  virtual void MIKTEXTHISCALL CloseFile(FILE* file) = 0;

public:
  virtual bool MIKTEXTHISCALL IsOutputFile(const FILE* file) = 0;

#if defined(MIKTEX_WINDOWS)
public:
  virtual bool MIKTEXTHISCALL IsFileAlreadyOpen(const PathName& fileName) = 0;
#endif

public:
  virtual void MIKTEXTHISCALL ScheduleSystemCommand(const std::string& commandLine) = 0;

#if defined(MIKTEX_WINDOWS)
public:
  virtual void MIKTEXTHISCALL ScheduleFileRemoval(const PathName& fileName) = 0;
#endif

public:
  virtual bool MIKTEXTHISCALL StartFileInfoRecorder() = 0;

public:
  virtual bool MIKTEXTHISCALL StartFileInfoRecorder(bool recordPackageNames) = 0;

public:
  virtual void MIKTEXTHISCALL SetRecorderPath(const PathName& path) = 0;

public:
  virtual void MIKTEXTHISCALL RecordFileInfo(const PathName& path, FileAccess access) = 0;

public:
  virtual std::vector<FileInfoRecord> MIKTEXTHISCALL GetFileInfoRecords() = 0;

public:
  virtual FileType MIKTEXTHISCALL DeriveFileType(const PathName& fileName) = 0;

public:
  virtual bool MIKTEXTHISCALL FindFile(const std::string& fileName, const std::string& pathList, FindFileOptionSet options, std::vector<PathName>& result) = 0;

public:
  virtual bool MIKTEXTHISCALL FindFile(const std::string& fileName, const std::string& pathList, std::vector<PathName>& result) = 0;

public:
  virtual bool MIKTEXTHISCALL FindFile(const std::string& fileName, const std::string& pathList, FindFileOptionSet options, PathName& result) = 0;

public:
  virtual bool MIKTEXTHISCALL FindFile(const std::string& fileName, const std::string& pathList, PathName& result) = 0;

public:
  virtual bool MIKTEXTHISCALL FindFile(const std::string& fileName, FileType fileType, FindFileOptionSet options, std::vector<PathName>& result) = 0;

public:
  virtual bool MIKTEXTHISCALL FindFile(const std::string& fileName, FileType fileType, std::vector<PathName>& result) = 0;

public:
  virtual bool MIKTEXTHISCALL FindFile(const std::string& fileName, FileType fileType, FindFileOptionSet options, PathName& result) = 0;

public:
  virtual bool MIKTEXTHISCALL FindFile(const std::string& fileName, FileType fileType, PathName& result) = 0;

public:
  virtual bool MIKTEXTHISCALL FindPkFile(const std::string& fontName, const std::string& mfMode, int dpi, PathName& result) = 0;

public:
  virtual bool MIKTEXTHISCALL FindTfmFile(const std::string& fontName, PathName& result, bool create) = 0;

public:
  virtual void MIKTEXTHISCALL SetFindFileCallback(IFindFileCallback* pCallback) = 0;

public:
  virtual void MIKTEXTHISCALL SplitFontPath(const PathName& fontPath, std::string* fontType, std::string* supplier, std::string* typeface, std::string* fontName, std::string* pointSize) = 0;

public:
  virtual bool MIKTEXTHISCALL GetFontInfo(const std::string& fontName, std::string& supplier, std::string& typeface, double* genSize) = 0;

public:
  virtual PathName MIKTEXTHISCALL GetGhostscript(unsigned long* versionNumber) = 0;

public:
  virtual std::string MIKTEXTHISCALL GetExpandedSearchPath(FileType fileType) = 0;

public:
  virtual bool MIKTEXTHISCALL ConvertToBitmapFile(const PathName& sourceFileName, PathName& destFileName, IRunProcessCallback* callback) = 0;

public:
  virtual bool MIKTEXTHISCALL EnableFontMaker(bool enable) = 0;

public:
  virtual bool MIKTEXTHISCALL GetMakeFontsFlag() = 0;

public:
  virtual std::vector<std::string> MIKTEXTHISCALL MakeMakePkCommandLine(const std::string& fontName, int dpi, int baseDpi, const std::string& mfMode, PathName& fileName, TriState enableInstaller) = 0;

#if defined(MIKTEX_WINDOWS)
public:
  virtual int MIKTEXTHISCALL RunBatch(int argc, const char** argv) = 0;
#endif

#if !defined(MIKTEX_WINDOWS)
public:
  virtual int MIKTEXTHISCALL RunSh(int argc, const char** argv) = 0;
#endif

public:
  virtual int MIKTEXTHISCALL RunPerl(int argc, const char** argv) = 0;

public:
  virtual int MIKTEXTHISCALL RunPython(int argc, const char** argv) = 0;

public:
  virtual int MIKTEXTHISCALL RunJava(int argc, const char** argv) = 0;

#if defined(MIKTEX_WINDOWS)
public:
  virtual bool MIKTEXTHISCALL ShowManualPageAndWait(HWND hWnd, unsigned long topic) = 0;
#endif

public:
  virtual std::vector<FileTypeInfo> MIKTEXTHISCALL GetFileTypes() = 0;

public:
  virtual std::vector<FormatInfo> MIKTEXTHISCALL GetFormats() = 0;

public:
  virtual FormatInfo MIKTEXTHISCALL GetFormatInfo(const std::string& key) = 0;

public:
  virtual bool MIKTEXTHISCALL TryGetFormatInfo(const std::string& key, FormatInfo& formatInfo) = 0;

public:
  virtual void MIKTEXTHISCALL DeleteFormatInfo(const std::string& key) = 0;

public:
  virtual void MIKTEXTHISCALL SetFormatInfo(const FormatInfo& formatInfo) = 0;

public:
  virtual std::vector<LanguageInfo> MIKTEXTHISCALL GetLanguages() = 0;

public:
  virtual PathName MIKTEXTHISCALL GetMyProgramFile(bool canonicalized) = 0;

public:
  virtual PathName MIKTEXTHISCALL GetMyLocation(bool canonicalized) = 0;

public:
  virtual PathName MIKTEXTHISCALL GetMyPrefix(bool canonicalized) = 0;

public:
  virtual bool MIKTEXTHISCALL RunningAsAdministrator() = 0;

public:
  virtual void MIKTEXTHISCALL SetAdminMode(bool adminMode, bool force = false) = 0;

public:
  virtual bool MIKTEXTHISCALL IsAdminMode() = 0;

public:
  virtual bool MIKTEXTHISCALL IsSharedSetup() = 0;

public:
  virtual bool MIKTEXTHISCALL GetPaperSizeInfo(int idx, PaperSizeInfo& paperSize) = 0;

public:
  virtual PaperSizeInfo MIKTEXTHISCALL GetPaperSizeInfo(const std::string& dvipsName) = 0;

public:
  virtual void MIKTEXTHISCALL SetDefaultPaperSize(const std::string& dvipsName) = 0;

public:
  virtual bool MIKTEXTHISCALL TryCreateFromTemplate(const PathName& path) = 0;

#if defined(MIKTEX_WINDOWS)
public:
  virtual bool MIKTEXTHISCALL RunningAsPowerUser() = 0;
#endif

public:
  virtual bool MIKTEXTHISCALL IsUserAnAdministrator() = 0;

#if defined(MIKTEX_WINDOWS)
public:
  virtual bool MIKTEXTHISCALL IsUserAPowerUser() = 0;
#endif

public:
  virtual void MIKTEXTHISCALL ConfigureFile(const PathName& pathIn, const PathName& pathOut, HasNamedValues* callback = nullptr) = 0;

public:
  virtual void MIKTEXTHISCALL ConfigureFile(const PathName& pathRel, HasNamedValues* callback = nullptr) = 0;

public:
  virtual void MIKTEXTHISCALL SetTheNameOfTheGame(const std::string& name) = 0;

public:
  virtual std::vector<std::string> MIKTEXTHISCALL GetFontDirectories() = 0;

public:
  virtual FileTypeInfo MIKTEXTHISCALL GetFileTypeInfo(FileType fileType) = 0;

public:
  virtual std::string Expand(const std::string& toBeExpanded) = 0;

public:
  virtual std::string Expand(const std::string& toBeExpanded, HasNamedValues* callback) = 0;

public:
  virtual std::string Expand(const std::string& toBeExpanded, ExpandOptionSet options, HasNamedValues* callback) = 0;

public:
  virtual void MIKTEXTHISCALL SetLanguageInfo(const LanguageInfo& languageInfo) = 0;

#if HAVE_MIKTEX_USER_INFO
public:
  virtual MiKTeXUserInfo RegisterMiKTeXUser(const MiKTeXUserInfo& info) = 0;
#endif

#if HAVE_MIKTEX_USER_INFO
public:
  virtual bool TryGetMiKTeXUserInfo(MiKTeXUserInfo& info) = 0;
#endif

public:
  virtual ShellCommandMode GetShellCommandMode() = 0;

public:
  virtual std::vector<std::string> GetAllowedShellCommands() = 0;

public:
  enum class ExamineCommandLineResult {
    ProbablySafe,
    MaybeSafe,
    NotSafe,
    SyntaxError
  };

public:
  virtual std::tuple<ExamineCommandLineResult, std::string, std::string> ExamineCommandLine(const std::string& commandLine) = 0;
    
public:
  static MIKTEXCOREEXPORT MIKTEXNORETURN void MIKTEXCEECALL FatalCrtError(const std::string& functionName, int errorCode, const MiKTeXException::KVMAP& info, const SourceLocation& sourceLocation);

public:
  static MIKTEXCOREEXPORT MIKTEXNORETURN void MIKTEXCEECALL FatalMiKTeXError(const std::string& message, const std::string& description, const std::string& remedy, const std::string& tag, const MiKTeXException::KVMAP& info, const SourceLocation& sourceLocation);

#if defined(MIKTEX_WINDOWS)
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
