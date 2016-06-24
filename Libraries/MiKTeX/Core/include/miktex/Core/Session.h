/* miktex/Core/Session.h:                               -*- C++ -*-

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

#if defined(_MSC_VER)
#  pragma once
#endif

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
#include "PathName.h"
#include "OptionSet.h"
#include "Process.h"
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
  MiKTeXConfiguration config = MiKTeXConfiguration::None;
};

/// Special path enum class.
enum class SpecialPath
{
  /// The common installation root directory.
  CommonInstallRoot,

  // The installation directory of the user.
  UserInstallRoot,

  /// The common data root directory.
  CommonDataRoot,

  /// The data root directory of the user.
  UserDataRoot,

  CommonConfigRoot,
  UserConfigRoot,
  InstallRoot,

  /// The prefered data root directory. For a common MiKTeX setup
  /// this is equivalent to the common data directory. For a private
  /// MiKTeX setup this is equivalent to the user data directory.
  DataRoot,

  ConfigRoot,

  /// The bin directory.
  BinDirectory,

  /// The internal bin directory.
  InternalBinDirectory,

  /// The portable root directory.
  PortableRoot,

  /// The portable mount directory.
  PortableMount,

  DistRoot,
};

/// Paper size info.
class PaperSizeInfo
{
public:
  static MIKTEXCORECEEAPI(PaperSizeInfo) Parse(const char * s);

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
  char szMnemonic[32];
  char szDescription[128];
  int iHorzRes;
  int iVertRes;
};

struct FileTypeInfo
{
  FileType fileType = FileType::None;
  std::string fileTypeString;
  std::string fileNameExtensions;
  std::string applicationName;
  std::string searchPath;
  std::string envVarNames;
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
  std::string id;
  std::string name;
  std::string organization;
  std::string email;
  int role = 0;
  int level = 0;
  time_t expirationDate = static_cast<time_t>(-1);
  bool IsMember () const
  {
    return level >= Individual && expirationDate != static_cast<time_t>(-1) && expirationDate >= time(0);
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
  Temporary
};

typedef OptionSet<RegisterRootDirectoriesOption> RegisterRootDirectoriesOptionSet;

/// Find file callback interface
class MIKTEXNOVTABLE IFindFileCallback
{
  /// Request to install package. Called by the FindFile machinery.
  /// @param lpszPackageName Package to be installed.
  /// @param lpszTrigger File name that triggered the auto-installer.
  /// @param[out] installRoot Installation root directory.
  /// @return Returns true, if the package has been installed.
public:
  virtual bool MIKTEXTHISCALL InstallPackage(const std::string & deploymentName, const PathName & trigger, PathName & installRoot) = 0;

public:
  virtual bool MIKTEXTHISCALL TryCreateFile(const PathName & fileName, FileType fileType) = 0;
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
#if defined(MIKTEX_WINDOWS)
    /// Initialize the COM library.
    InitializeCOM,
#endif
    /// start in administration mode
    AdminMode,
  };

  /// Init flags enum.
  typedef OptionSet<InitOption> InitOptionSet;

  /// Init info struct.
public:
  class InitInfo
  {
    /// Initializes a new init info struct.
  public:
    InitInfo()
    {
    }

    /// Initializes a new init info struct.
    /// @param lpszProgramInvocationName Name of the invoked program.
  public:
    InitInfo(const std::string & programInvocationName) :
      programInvocationName(programInvocationName)
    {
    }

    /// Initializes a new init info struct.
    /// @param lpszProgramInvocationName Name of the invoked program.
    /// @param flags Init flags.
  public:
    InitInfo(const std::string & programInvocationName, InitOptionSet options) :
      programInvocationName(programInvocationName),
      options(options)
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
    void SetProgramInvocationName(const std::string & programInvocationName)
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
    void SetTheNameOfTheGame(const std::string & theNameOfTheGame)
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
    void SetStartupConfig(const StartupConfig & startupConfig)
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
    void SetTraceFlags(const std::string & traceFlags)
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
    void SetTraceCallback(MiKTeX::Trace::TraceCallback * callback)
    {
      traceCallback = callback;
    }

  public:
    MiKTeX::Trace::TraceCallback * GetTraceCallback()
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
    MiKTeX::Trace::TraceCallback * traceCallback = nullptr;
  };

public:
  static MIKTEXCORECEEAPI(std::shared_ptr<Session>) Get();

public:
  static MIKTEXCORECEEAPI(std::shared_ptr<Session>) TryGet();

public:
  static MIKTEXCORECEEAPI(std::shared_ptr<Session>) Create(const InitInfo & initInfo);

public:
  virtual MIKTEXTHISCALL ~Session() = 0;

public:
  virtual void MIKTEXTHISCALL PushAppName(const std::string & name) = 0;

public:
  virtual void MIKTEXTHISCALL PushBackAppName(const std::string & name) = 0;

public:
  virtual void MIKTEXTHISCALL AddInputDirectory(const char * lpszPath, bool atEnd) = 0;

  /// Gets a special path.
  /// @param specialPath Specifies special path.
  /// @return Returns a path name.
public:
  virtual PathName MIKTEXTHISCALL GetSpecialPath(SpecialPath specialPath) = 0;

  /// Gets the number of root directories.
public:
  virtual unsigned MIKTEXTHISCALL GetNumberOfTEXMFRoots() = 0;

public:
  virtual PathName MIKTEXTHISCALL GetRootDirectory(unsigned r) = 0;

public:
  virtual bool MIKTEXTHISCALL IsCommonRootDirectory(unsigned r) = 0;

  /// Gets the path name of the virtual MPM TEXMF root.
public:
  virtual PathName MIKTEXTHISCALL GetMpmRootPath() = 0;

  /// Gets the path name of the MPM file name database.
public:
  virtual PathName MIKTEXTHISCALL GetMpmDatabasePathName() = 0;

public:
  virtual unsigned MIKTEXTHISCALL TryDeriveTEXMFRoot(const PathName & path) = 0;

public:
  virtual unsigned MIKTEXTHISCALL DeriveTEXMFRoot(const PathName & path) = 0;

  /// Finds a file name database file.
  /// @param r File name database ID.
  /// @param path Path name object buffer to be filled.
public:
  virtual bool MIKTEXTHISCALL FindFilenameDatabase(unsigned r, PathName & path) = 0;

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
  /// @param lpszPath Path name to split.
  /// @param lpszRoot String buffer to be filled with the root directory.
  /// @param lpszRelativ String buffer to be filled with the relative path.
  /// @return Returns the TEXMF ID.
public:
  virtual unsigned MIKTEXTHISCALL SplitTEXMFPath(const PathName & path, PathName & root, PathName & relative) = 0;

public:
  virtual void MIKTEXTHISCALL RegisterRootDirectories(const std::string & roots) = 0;

public:
  virtual void MIKTEXTHISCALL RegisterRootDirectories(const StartupConfig & startupConfig, RegisterRootDirectoriesOptionSet options) = 0;

public:
  virtual bool MIKTEXTHISCALL IsMiKTeXDirect() = 0;

public:
  virtual bool MIKTEXTHISCALL IsMiKTeXPortable() = 0;

public:
  virtual bool MIKTEXTHISCALL GetMETAFONTMode(unsigned idx, MIKTEXMFMODE * pMode) = 0;

public:
  virtual bool MIKTEXTHISCALL DetermineMETAFONTMode(unsigned dpi, MIKTEXMFMODE * pMode) = 0;

public:
  virtual bool MIKTEXTHISCALL TryGetConfigValue(const char * lpszSectionName, const char * lpszValueName, std::string &  value) = 0;

public:
  virtual std::string MIKTEXTHISCALL GetConfigValue(const char * lpszSectionName, const char * lpszValueName, const char * lpszDefaultValue) = 0;

public:
  virtual int MIKTEXTHISCALL GetConfigValue(const char * lpszSectionName, const char * lpszValueName, int defaultValue) = 0;

public:
  virtual bool MIKTEXTHISCALL GetConfigValue(const char * lpszSectionName, const char * lpszValueName, bool defaultValue) = 0;

public:
  virtual TriState MIKTEXTHISCALL GetConfigValue(const char * lpszSectionName, const char * lpszValueName, TriState defaultValue) = 0;

public:
  virtual char MIKTEXTHISCALL GetConfigValue(const char * lpszSectionName, const char * lpszValueName, char defaultValue) = 0;

public:
  virtual void MIKTEXTHISCALL SetConfigValue(const char * lpszSectionName, const char * lpszValueName, const char * lpszValue) = 0;

public:
  virtual void MIKTEXTHISCALL SetConfigValue(const char * lpszSectionName, const char * lpszValueName, bool value) = 0;

public:
  virtual void MIKTEXTHISCALL SetConfigValue(const char * lpszSectionName, const char * lpszValueName, int value) = 0;

public:
  virtual std::string MIKTEXTHISCALL GetEngineName() = 0;

public:
  virtual FILE * MIKTEXTHISCALL OpenFile(const PathName & path, FileMode mode, FileAccess access, bool isTextFile) = 0;

public:
  virtual FILE * MIKTEXTHISCALL TryOpenFile(const PathName & path, FileMode mode, FileAccess access, bool isTextFile) = 0;

public:
  virtual FILE * MIKTEXTHISCALL OpenFile(const PathName & path, FileMode mode, FileAccess access, bool isTextFile, FileShare share) = 0;

public:
  virtual FILE * MIKTEXTHISCALL TryOpenFile(const PathName & path, FileMode mode, FileAccess access, bool isTextFile, FileShare share) = 0;

public:
  virtual void MIKTEXTHISCALL CloseFile(FILE * pFile) = 0;

public:
  virtual bool MIKTEXTHISCALL IsOutputFile(const FILE * pFile) = 0;

#if defined(MIKTEX_WINDOWS)
public:
  virtual bool MIKTEXTHISCALL IsFileAlreadyOpen(const char * lpszFileName) = 0;
#endif

#if defined(MIKTEX_WINDOWS)
public:
  virtual void MIKTEXTHISCALL ScheduleFileRemoval(const char * lpszFileName) = 0;
#endif

public:
  virtual bool MIKTEXTHISCALL StartFileInfoRecorder() = 0;

public:
  virtual bool MIKTEXTHISCALL StartFileInfoRecorder(bool recordPackageNames) = 0;

public:
  virtual void MIKTEXTHISCALL SetRecorderPath(const PathName & path) = 0;

public:
  virtual void MIKTEXTHISCALL RecordFileInfo(const PathName & path, FileAccess access) = 0;

public:
  virtual std::vector<FileInfoRecord> MIKTEXTHISCALL GetFileInfoRecords() = 0;

public:
  virtual FileType MIKTEXTHISCALL DeriveFileType(const char * lpszPath) = 0;

public:
  virtual bool MIKTEXTHISCALL FindFile(const char * lpszFileName, const char * lpszPathList, FindFileOptionSet options, std::vector<PathName> & result) = 0;

public:
  virtual bool MIKTEXTHISCALL FindFile(const char * lpszFileName, const char * lpszPathList, std::vector<PathName> & result) = 0;

public:
  virtual bool MIKTEXTHISCALL FindFile(const char * lpszFileName, const char * lpszPathList, FindFileOptionSet options, PathName & result) = 0;

public:
  virtual bool MIKTEXTHISCALL FindFile(const char * lpszFileName, const char * lpszPathList, PathName & result) = 0;

public:
  virtual bool MIKTEXTHISCALL FindFile(const char * lpszFileName, FileType fileType, FindFileOptionSet options, std::vector<PathName> & result) = 0;

public:
  virtual bool MIKTEXTHISCALL FindFile(const char * lpszFileName, FileType fileType, std::vector<PathName> & result) = 0;

public:
  virtual bool MIKTEXTHISCALL FindFile(const char * lpszFileName, FileType fileType, FindFileOptionSet options, PathName & result) = 0;

public:
  virtual bool MIKTEXTHISCALL FindFile(const char * lpszFileName, FileType fileType, PathName & result) = 0;

public:
  virtual bool MIKTEXTHISCALL FindPkFile(const char * lpszFontName, const char * lpszMode, int dpi, PathName & result) = 0;

public:
  virtual bool MIKTEXTHISCALL FindTfmFile(const char * lpszFontName, PathName & result, bool create) = 0;

public:
  virtual void MIKTEXTHISCALL SetFindFileCallback(IFindFileCallback * pCallback) = 0;

public:
  virtual void MIKTEXTHISCALL SplitFontPath(const char * lpszFontPath, char * lpszFontType, char * lpszSupplier, char * lpszTypeface, char * lpszFontName, char * lpszPointSize) = 0;

public:
  virtual bool MIKTEXTHISCALL GetFontInfo(const char * lpszFontName, char * lpszSupplier, char * lpszTypeface, double * lpGenSize) = 0;

public:
  virtual void MIKTEXTHISCALL GetGhostscript(char * lpszPath, unsigned long * pVersionNumber) = 0;

public:
  virtual std::string MIKTEXTHISCALL GetExpandedSearchPath(FileType fileType) = 0;

public:
  virtual bool MIKTEXTHISCALL FindGraphicsRule(const char * lpszFrom, const char * lpszTo, char * lpszRule, std::size_t bufSize) = 0;

public:
  virtual bool MIKTEXTHISCALL ConvertToBitmapFile(const char * lpszSourceFileName, char * lpszDestFileName, IRunProcessCallback * pCallback) = 0;

public:
  virtual bool MIKTEXTHISCALL EnableFontMaker(bool enable) = 0;

public:
  virtual bool MIKTEXTHISCALL GetMakeFontsFlag() = 0;

public:
  virtual std::string MIKTEXTHISCALL MakeMakePkCommandLine(const char * lpszFontName, int dpi, int baseDpi, const char * lpszMfMode, PathName & fileName, TriState enableInstaller) = 0;

#if defined(MIKTEX_WINDOWS)
public:
  virtual int MIKTEXTHISCALL RunBatch(int argc, const char ** argv) = 0;
#endif

public:
  virtual int MIKTEXTHISCALL RunPerl(int argc, const char ** argv) = 0;

public:
  virtual int MIKTEXTHISCALL RunPython(int argc, const char ** argv) = 0;

public:
  virtual int MIKTEXTHISCALL RunJava(int argc, const char ** argv) = 0;

#if defined(MIKTEX_WINDOWS)
public:
  virtual bool MIKTEXTHISCALL ShowManualPageAndWait(HWND hWnd, unsigned long topic) = 0;
#endif

public:
  virtual std::vector<FileTypeInfo> MIKTEXTHISCALL GetFileTypes() = 0;

public:
  virtual std::vector<FormatInfo> MIKTEXTHISCALL GetFormats() = 0;

public:
  virtual FormatInfo MIKTEXTHISCALL GetFormatInfo(const char * lpszKey) = 0;

public:
  virtual bool MIKTEXTHISCALL TryGetFormatInfo(const char * lpszKey, FormatInfo & formatInfo) = 0;

public:
  virtual void MIKTEXTHISCALL DeleteFormatInfo(const char * lpszKey) = 0;

public:
  virtual void MIKTEXTHISCALL SetFormatInfo(const FormatInfo & formatInfo) = 0;

public:
  virtual std::vector<LanguageInfo> MIKTEXTHISCALL GetLanguages() = 0;

public:
  virtual PathName MIKTEXTHISCALL GetMyLocation(bool canonicalized) = 0;

public:
  PathName GetMyLocation()
  {
    return GetMyLocation(false);
  }

public:
  virtual PathName MIKTEXTHISCALL GetMyPrefix() = 0;

public:
  virtual bool MIKTEXTHISCALL RunningAsAdministrator() = 0;

public:
  virtual void MIKTEXTHISCALL SetAdminMode(bool adminMode, bool isSetup) = 0;

public:
  void SetAdminMode(bool adminMode)
  {
    SetAdminMode(adminMode, false);
  }

public:
  virtual bool MIKTEXTHISCALL IsAdminMode() = 0;

public:
  virtual bool MIKTEXTHISCALL IsSharedSetup() = 0;

public:
  virtual bool MIKTEXTHISCALL GetPaperSizeInfo(int idx, PaperSizeInfo & paperSize) = 0;

public:
  virtual PaperSizeInfo MIKTEXTHISCALL GetPaperSizeInfo(const char * lpszDvipsName) = 0;

public:
  virtual void MIKTEXTHISCALL SetDefaultPaperSize(const char * lpszDvipsName) = 0;

public:
  virtual bool MIKTEXTHISCALL TryCreateFromTemplate(const PathName & path) = 0;

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
  virtual void MIKTEXTHISCALL ConfigureFile(const PathName & pathIn, const PathName & pathOut, HasNamedValues * callback) = 0;

public:
  void ConfigureFile(const PathName & pathIn, const PathName & pathOut)
  {
    ConfigureFile(pathIn, pathOut, nullptr);
  }

public:
  virtual void MIKTEXTHISCALL ConfigureFile(const PathName & pathRel, HasNamedValues * callback) = 0;

public:
  void ConfigureFile(const PathName & pathRel)
  {
    ConfigureFile(pathRel, nullptr);
  }
  
public:
  virtual void MIKTEXTHISCALL SetTheNameOfTheGame(const char * lpszTheNameOfTheGame) = 0;

public:
  virtual std::string MIKTEXTHISCALL GetLocalFontDirectories() = 0;

public:
  virtual FileTypeInfo MIKTEXTHISCALL GetFileTypeInfo(FileType fileType) = 0;

public:
  virtual std::string Expand(const char * lpszToBeExpanded) = 0;

public:
  virtual std::string Expand(const char * lpszToBeExpanded, HasNamedValues * callback) = 0;

public:
  virtual std::string Expand(const char * lpszToBeExpanded, ExpandOptionSet options, HasNamedValues * callback) = 0;

public:
  virtual void MIKTEXTHISCALL SetLanguageInfo(const LanguageInfo & languageInfo) = 0;

#if HAVE_MIKTEX_USER_INFO
public:
  virtual MiKTeXUserInfo RegisterMiKTeXUser(const MiKTeXUserInfo & info) = 0;
#endif

#if HAVE_MIKTEX_USER_INFO
public:
  virtual bool TryGetMiKTeXUserInfo(MiKTeXUserInfo & info) = 0;
#endif

public:
  static MIKTEXCOREEXPORT MIKTEXNORETURN void MIKTEXCEECALL FatalCrtError(const std::string & functionName, int errorCode, const MiKTeXException::KVMAP & info, const SourceLocation & sourceLocation);

public:
  static MIKTEXCOREEXPORT MIKTEXNORETURN void MIKTEXCEECALL FatalMiKTeXError(const std::string & message, const MiKTeXException::KVMAP & info, const SourceLocation & sourceLocation);

#if defined(MIKTEX_WINDOWS)
public:
  static MIKTEXCOREEXPORT MIKTEXNORETURN void MIKTEXCEECALL FatalWindowsError(const std::string & functionName, unsigned long errorCode, const MiKTeXException::KVMAP & info, const SourceLocation & sourceLocation);
#endif
};

MIKTEX_CORE_END_NAMESPACE;

#define MIKTEX_FATAL_ERROR(message) \
  MiKTeX::Core::Session::FatalMiKTeXError(message, MiKTeX::Core::MiKTeXException::KVMAP(), MIKTEX_SOURCE_LOCATION())

#define MIKTEX_FATAL_ERROR_2(message, ...) \
  MiKTeX::Core::Session::FatalMiKTeXError(message, MiKTeX::Core::MiKTeXException::KVMAP(__VA_ARGS__), MIKTEX_SOURCE_LOCATION())

#define MIKTEX_INTERNAL_ERROR() MIKTEX_FATAL_ERROR(MIKTEXTEXT("MiKTeX encountered an internal error."))

#define MIKTEX_UNEXPECTED() MIKTEX_INTERNAL_ERROR()

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
    MiKTeX::Core::Session::FatalWindowsError(functionName, errorCode, MiKTeX::Core::MiKTeXException::KVMAP(), MIKTEX_SOURCE_LOCATION()); \
  }

#define MIKTEX_FATAL_WINDOWS_ERROR_2(functionName, ...)                 \
  {                                                                     \
    unsigned long errorCode = GetLastError();                           \
    MiKTeX::Core::Session::FatalWindowsError(functionName, errorCode, MiKTeX::Core::MiKTeXException::KVMAP(__VA_ARGS__), MIKTEX_SOURCE_LOCATION()); \
  }

#define MIKTEX_FATAL_WINDOWS_RESULT(functionName, errorCode) \
  MiKTeX::Core::Session::FatalWindowsError(functionName, errorCode, MiKTeX::Core::MiKTeXException::KVMAP(), MIKTEX_SOURCE_LOCATION())

#define MIKTEX_FATAL_WINDOWS_RESULT_2(functionName, errorCode, ...) \
  MiKTeX::Core::Session::FatalWindowsError(functionName, errorCode, MiKTeX::Core::MiKTeXException::KVMAP(__VA_ARGS__), MIKTEX_SOURCE_LOCATION())

#endif

#endif
