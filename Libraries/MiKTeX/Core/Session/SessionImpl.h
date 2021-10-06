/* SessionImpl.h: Session impl class

   Copyright (C) 1996-2021 Christian Schenk

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

#if !defined(INTERNAL_CORE_SESSION_SESSIONIMPL_H)
#define INTERNAL_CORE_SESSION_SESSIONIMPL_H

#include <deque>
#include <fstream>
#include <map>
#include <set>

#if defined(HAVE_ATLBASE_H)
#  define _ATL_FREE_THREADED
#  define _ATL_NO_AUTOMATIC_NAMESPACE
#  define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#  include <atlbase.h>
#  include <ATLComTime.h>
#  include <atlcom.h>
#endif

// FIXME: must come first
#include "core-version.h"

#include <miktex/Core/Cfg>
#include <miktex/Core/FileSystemWatcher>
#include <miktex/Core/Session>
#include <miktex/Core/Stream>
#include <miktex/Core/equal_icase>
#include <miktex/Core/hash_icase>

#if defined(MIKTEX_WINDOWS)
#include <miktex/Core/win/COMInitializer>
#endif

#include "Fndb/FileNameDatabase.h"
#include "RootDirectoryInternals.h"

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
#  import MIKTEX_SESSION_TLB raw_interfaces_only
#endif

CORE_INTERNAL_BEGIN_NAMESPACE;

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
namespace MiKTeXSessionLib = MAKE_CURVER_ID(MiKTeXSession);
#endif

struct FormatInfo_ :
  public MiKTeX::Core::FormatInfo
{
  FormatInfo_()
  {
  }

  FormatInfo_(const MiKTeX::Core::FormatInfo& other) :
    MiKTeX::Core::FormatInfo(other)
  {
  }

  MiKTeX::Util::PathName cfgFile;
};

struct LanguageInfo_ :
  public MiKTeX::Core::LanguageInfo
{
  LanguageInfo_()
  {
  }

  LanguageInfo_(const MiKTeX::Core::LanguageInfo &other) :
    MiKTeX::Core::LanguageInfo(other)
  {
  }

  MiKTeX::Util::PathName cfgFile;
};

inline bool operator<(const LanguageInfo_& lhs, const LanguageInfo_& rhs)
{
  if (lhs.key == "english")
  {
    return true;
  }
  if (rhs.key == "english")
  {
    return false;
  }
  return lhs.key < rhs.key;
}

struct InternalFileTypeInfo :
  public MiKTeX::Core::FileTypeInfo
{
  std::vector<MiKTeX::Util::PathName> pathPatterns;
};

struct DvipsPaperSizeInfo :
  public MiKTeX::Core::PaperSizeInfo
{
  std::vector<std::string> definition;
};

struct InternalStartupConfig :
  public MiKTeX::Core::StartupConfig
{
  InternalStartupConfig()
  {
  }

  InternalStartupConfig(const MiKTeX::Core::StartupConfig& parent) :
    StartupConfig(parent)
  {
  }
  
  /// Indicates whether the installation is shared by all users.
  MiKTeX::Configuration::TriState isSharedSetup = MiKTeX::Configuration::TriState::Undetermined;

  /// MiKTeX setup version.
  MiKTeX::Core::VersionNumber setupVersion;
};

class SessionImpl :
  public MiKTeX::Core::Session
{
public:
  SessionImpl();

public:
  virtual ~SessionImpl();

private:
  void Initialize(const InitInfo& initInfo);

private:
  void Uninitialize();

public:
  void Close() override;

public:
  void Reset() override;

public:
  void PushAppName(const std::string& name) override;

public:
  void PushBackAppName(const std::string& name) override;

public:
  void AddInputDirectory(const MiKTeX::Util::PathName& path, bool atEnd) override;

public:
  MiKTeX::Util::PathName GetSpecialPath(MiKTeX::Configuration::SpecialPath specialPath) override;

public:
  std::vector<MiKTeX::Core::RootDirectoryInfo> GetRootDirectories() override;

public:
  unsigned GetNumberOfTEXMFRoots() override;

public:
  MiKTeX::Util::PathName GetRootDirectoryPath(unsigned r) override;

public:
  bool IsCommonRootDirectory(unsigned r) override;

public:
  bool IsOtherRootDirectory(unsigned r) override;

public:
  MiKTeX::Util::PathName GetMpmRootPath() override;

public:
  MiKTeX::Util::PathName GetMpmDatabasePathName() override;

public:
  unsigned TryDeriveTEXMFRoot(const MiKTeX::Util::PathName& path) override;

public:
  unsigned DeriveTEXMFRoot(const MiKTeX::Util::PathName& path) override;

public:
  bool FindFilenameDatabase(unsigned r, MiKTeX::Util::PathName& path) override;

public:
  MiKTeX::Util::PathName GetFilenameDatabasePathName(unsigned r) override;

public:
  bool UnloadFilenameDatabase() override
  {
    return UnloadFilenameDatabase(std::chrono::seconds(0));
  }

public:
  bool UnloadFilenameDatabase(std::chrono::duration<double> minIdleTime) override;

public:
  unsigned SplitTEXMFPath(const MiKTeX::Util::PathName& path, MiKTeX::Util::PathName& root, MiKTeX::Util::PathName& relative) override;

public:
  void RegisterRootDirectories(const MiKTeX::Core::StartupConfig& startupConfig, MiKTeX::Core::RegisterRootDirectoriesOptionSet options) override;

public:
  void RegisterRootDirectory(const MiKTeX::Util::PathName& path, bool other) override;

public:
  void UnregisterRootDirectory(const MiKTeX::Util::PathName& path, bool other) override;

public:
  void MoveRootDirectoryUp(unsigned r) override;

public:
  void MoveRootDirectoryDown(unsigned r) override;

public:
  bool IsMiKTeXDirect() override;

public:
  bool IsMiKTeXPortable() override;

public:
  bool GetMETAFONTMode(unsigned idx, MiKTeX::Core::MIKTEXMFMODE& mode) override;

public:
  bool DetermineMETAFONTMode(unsigned dpi, MiKTeX::Core::MIKTEXMFMODE& mode) override;

public:
  bool TryGetConfigValue(const std::string& sectionName, const std::string& valueName, MiKTeX::Configuration::HasNamedValues* callback, std::string& value) override;

public:
  bool TryGetConfigValue(const std::string& sectionName, const std::string& valueName, std::string& value) override
  {
    return TryGetConfigValue(sectionName, valueName, nullptr, value);
  }

public:
  MiKTeX::Configuration::ConfigValue GetConfigValue(const std::string& sectionName, const std::string& valueName, const MiKTeX::Configuration::ConfigValue& defaultValue, MiKTeX::Configuration::HasNamedValues* callback) override;

public:
  MiKTeX::Configuration::ConfigValue GetConfigValue(const std::string& sectionName, const std::string& valueName, const MiKTeX::Configuration::ConfigValue& defaultValue) override
  {
    return GetConfigValue(sectionName, valueName, defaultValue, nullptr);
  }

public:
  MiKTeX::Configuration::ConfigValue GetConfigValue(const std::string& sectionName, const std::string& valueName, MiKTeX::Configuration::HasNamedValues* callback) override;

public:
  MiKTeX::Configuration::ConfigValue GetConfigValue(const std::string& sectionName, const std::string& valueName) override
  {
    return GetConfigValue(sectionName, valueName, nullptr);
  }

public:
  void SetConfigValue(const std::string& sectionName, const std::string& valueName, const MiKTeX::Configuration::ConfigValue& value) override;

public:
  FILE* OpenFile(const MiKTeX::Util::PathName& path, MiKTeX::Core::FileMode mode, MiKTeX::Core::FileAccess access, bool isTextFile) override;

public:
  FILE* TryOpenFile(const MiKTeX::Util::PathName& path, MiKTeX::Core::FileMode mode, MiKTeX::Core::FileAccess access, bool isTextFile) override;

public:
  std::pair<bool, OpenFileInfo> TryGetOpenFileInfo(FILE* file) override;

public:
  void CloseFile(FILE* file, int& exitCode) override;

public:
  void CloseFile(FILE* file) override
  {
    int exitCode;
    CloseFile(file, exitCode);
  }

public:
  bool IsOutputFile(const FILE* file) override;

#if defined(MIKTEX_WINDOWS)
public:
  bool IsFileAlreadyOpen(const MiKTeX::Util::PathName& fileName) override;
#endif

public:
  void ScheduleSystemCommand(const std::string& commandLine) override;

#if defined(MIKTEX_WINDOWS)
public:
  void ScheduleFileRemoval(const MiKTeX::Util::PathName& fileName) override;
#endif

public:
  bool StartFileInfoRecorder() override;

public:
  bool StartFileInfoRecorder(bool recordPackageNames) override;

public:
  void SetRecorderPath(const MiKTeX::Util::PathName& path) override;

public:
  void RecordFileInfo(const MiKTeX::Util::PathName& path, MiKTeX::Core::FileAccess access) override;

public:
  std::vector<MiKTeX::Core::FileInfoRecord> GetFileInfoRecords() override;

public:
  MiKTeX::Core::FileType DeriveFileType(const MiKTeX::Util::PathName& fileName) override;

public:
  MiKTeX::Core::LocateResult MIKTEXTHISCALL Locate(const std::string& fileName, const MiKTeX::Core::LocateOptions& options) override;

public:
  bool FindFile(const std::string& fileName, const std::string& searchPath, FindFileOptionSet options, std::vector<MiKTeX::Util::PathName>& result) override;

public:
  bool FindFile(const std::string& fileName, const std::string& searchPath, std::vector<MiKTeX::Util::PathName>& result) override
  {
    MiKTeX::Core::LocateOptions locateOptions;
    locateOptions.all = true;
    locateOptions.searchPath = searchPath;
    result = Locate(fileName, locateOptions).pathNames;
    return !result.empty();
  }

public:
  bool FindFile(const std::string& fileName, const std::string& searchPath, FindFileOptionSet options, MiKTeX::Util::PathName& result) override;

public:
  bool FindFile(const std::string& fileName, const std::string& searchPath, MiKTeX::Util::PathName& result) override
  {
    MiKTeX::Core::LocateOptions locateOptions;
    locateOptions.searchPath = searchPath;
    if (auto locateResult = Locate(fileName, locateOptions); !locateResult.pathNames.empty())
    {
      result = locateResult.pathNames[0];
      return true;
    }
    return false;
  }

public:
  bool FindFile(const std::string& fileName, MiKTeX::Core::FileType fileType, FindFileOptionSet options, std::vector<MiKTeX::Util::PathName>& result) override;

public:
  bool FindFile(const std::string& fileName, MiKTeX::Core::FileType fileType, std::vector<MiKTeX::Util::PathName>& result) override
  {
    MiKTeX::Core::LocateOptions locateOptions;
    locateOptions.all = true;
    locateOptions.fileType = fileType;
    result = Locate(fileName, locateOptions).pathNames;
    return !result.empty();
  }

public:
  bool FindFile(const std::string& fileName, MiKTeX::Core::FileType fileType, FindFileOptionSet options, MiKTeX::Util::PathName& result) override;

public:
  bool FindFile(const std::string& fileName, MiKTeX::Core::FileType fileType, MiKTeX::Util::PathName& result) override
  {
    MiKTeX::Core::LocateOptions locateOptions;
    locateOptions.fileType = fileType;
    if (auto locateResult = Locate(fileName, locateOptions); !locateResult.pathNames.empty())
    {
      result = locateResult.pathNames[0];
      return true;
    }
    return false;
  }

public:
  bool FindPkFile(const std::string& fontName, const std::string& mfMode, int dpi, MiKTeX::Util::PathName& result) override;

public:
  bool FindTfmFile(const std::string& fontName, MiKTeX::Util::PathName& result, bool create) override
  {
    MiKTeX::Core::LocateOptions locateOptions;
    locateOptions.create = create;
    locateOptions.fileType = MiKTeX::Core::FileType::TFM;
    if (auto locateResult = Locate(fontName, locateOptions); !locateResult.pathNames.empty())
    {
      result = locateResult.pathNames[0];
      return true;
    }
    return false;
  }

public:
  void SetFindFileCallback(MiKTeX::Core::IFindFileCallback* callback) override;

public:
  void SplitFontPath(const MiKTeX::Util::PathName& fontPath, std::string* fontType, std::string* supplier, std::string* typeface, std::string* fontName, std::string* pointSize) override;

public:
  bool GetFontInfo(const std::string& fontName, std::string& supplier, std::string& typeface, double* genSize) override;

public:
  MiKTeX::Util::PathName GetGhostscript(unsigned long* versionNumber) override;

public:
  std::string GetExpandedSearchPath(MiKTeX::Core::FileType fileType) override;

private:
  bool FindGraphicsRule(const std::string& fromExt, const std::string& toext, std::string& rule);

public:
  bool ConvertToBitmapFile(const MiKTeX::Util::PathName& sourceFileName, MiKTeX::Util::PathName& destFileName, MiKTeX::Core::IRunProcessCallback* callback) override;

public:
  bool EnableFontMaker(bool enable) override;

public:
  bool GetMakeFontsFlag() override;

public:
  std::vector<std::string> MakeMakePkCommandLine(const std::string& fontName, int dpi, int baseDpi, const std::string& mfMode, MiKTeX::Util::PathName& fileName, MiKTeX::Configuration::TriState enableInstaller) override;

#if defined(MIKTEX_WINDOWS)
public:
  int RunBatch(int argc, const char** argv) override;
#endif

public:
  int RunExe(int argc, const char** argv) override;

public:
  int RunGhostscript(int argc, const char** argv) override;

#if !defined(MIKTEX_WINDOWS)
public:
  int RunSh(int argc, const char** argv) override;
#endif

public:
  int RunPerl(int argc, const char** argv) override;

public:
  int RunPython(int argc, const char** argv) override;

public:
  int RunJava(int argc, const char** argv) override;

#if defined(MIKTEX_WINDOWS)
public:
  bool ShowManualPageAndWait(HWND hWnd, unsigned long topic) override;
#endif

public:
  std::vector<MiKTeX::Core::FileTypeInfo> GetFileTypes() override;

public:
  std::vector<MiKTeX::Core::FormatInfo> GetFormats() override;

public:
  MiKTeX::Core::FormatInfo GetFormatInfo(const std::string& key) override;

public:
  bool TryGetFormatInfo(const std::string& key, MiKTeX::Core::FormatInfo& formatInfo) override;

public:
  void DeleteFormatInfo(const std::string& key) override;

public:
  void SetFormatInfo(const MiKTeX::Core::FormatInfo& formatInfo) override;

public:
  std::vector<MiKTeX::Core::LanguageInfo> GetLanguages() override;

public:
  MiKTeX::Util::PathName GetMyProgramFile(bool canonicalized) override;

public:
  MiKTeX::Util::PathName GetMyLocation(bool canonicalized) override;

public:
  MiKTeX::Util::PathName GetMyPrefix(bool canonicalized) override;

public:
  bool RunningAsAdministrator() override;

public:
  void SetAdminMode(bool isAdminMode, bool force) override;

public:
  bool IsAdminMode() override;

public:
  bool IsSharedSetup() override;

public:
  MiKTeX::Core::SetupConfig GetSetupConfig() override;

public:
  bool GetPaperSizeInfo(int idx, MiKTeX::Core::PaperSizeInfo& paperSize) override;

public:
  MiKTeX::Core::PaperSizeInfo GetPaperSizeInfo(const std::string& dvipsName) override;

public:
  void SetDefaultPaperSize(const std::string& dvipsName) override;

public:
  bool TryCreateFromTemplate(const MiKTeX::Util::PathName& path) override;

public:
  bool IsUserAnAdministrator() override;

public:
  void ConfigureFile(const MiKTeX::Util::PathName& pathIn, const MiKTeX::Util::PathName& pathOut, MiKTeX::Configuration::HasNamedValues* callback) override;

public:
  void ConfigureFile(const MiKTeX::Util::PathName& pathRel, MiKTeX::Configuration::HasNamedValues* callback) override;

public:
  void SetTheNameOfTheGame(const std::string& name) override;

public:
  std::vector<std::string> GetFontDirectories() override;

public:
  MiKTeX::Core::FileTypeInfo GetFileTypeInfo(MiKTeX::Core::FileType fileType) override;

public:
  std::string Expand(const std::string& toBeExpanded) override;

public:
  std::string Expand(const std::string& toBeExpanded, MiKTeX::Configuration::HasNamedValues* callback) override;

public:
  std::string Expand(const std::string& toBeExpanded, MiKTeX::Core::ExpandOptionSet options, MiKTeX::Configuration::HasNamedValues* callback) override;

public:
  void SetLanguageInfo(const MiKTeX::Core::LanguageInfo& languageInfo) override;

#if HAVE_MIKTEX_USER_INFO
public:
  MiKTeX::Core::MiKTeXUserInfo RegisterMiKTeXUser(const MiKTeX::Core::MiKTeXUserInfo& info) override;
#endif

#if HAVE_MIKTEX_USER_INFO
public:
  bool TryGetMiKTeXUserInfo(MiKTeX::Core::MiKTeXUserInfo& info) override;
#endif

public:
  MiKTeX::Core::ShellCommandMode GetShellCommandMode() override;

public:
  std::vector<std::string> GetAllowedShellCommands() override;

public:
  std::tuple<ExamineCommandLineResult, std::string, std::string> ExamineCommandLine(const std::string& commandLine) override;

public:
  InitInfo GetInitInfo() const
  {
    return initInfo;
  }
  
public:
  bool IsTEXMFFile(const MiKTeX::Util::PathName& path, MiKTeX::Util::PathName& relPath, unsigned& rootIndex);

public:
  bool IsTEXMFFile(const MiKTeX::Util::PathName& path, MiKTeX::Util::PathName& relPath)
  {
    unsigned rootIndex;
    return IsTEXMFFile(path, relPath, rootIndex);
  }

public:
  bool IsTEXMFFile(const MiKTeX::Util::PathName& path)
  {
    MiKTeX::Util::PathName relPath;
    unsigned rootIndex;
    return IsTEXMFFile(path, relPath, rootIndex);
  }

public:
  bool UnloadFilenameDatabaseInternal(unsigned r, std::chrono::duration<double> minIdleTime);

private:
  bool UnloadFilenameDatabaseInternal_nolock(unsigned r, std::chrono::duration<double> minIdleTime);

public:
  std::shared_ptr<FileNameDatabase> GetFileNameDatabase(unsigned r);

public:
  std::shared_ptr<FileNameDatabase> GetFileNameDatabase(const char* path);

public:
  MiKTeX::Util::PathName GetTempDirectory();

#if defined(MIKTEX_WINDOWS) && defined(MIKTEX_CORE_SHARED)
public:
  MiKTeX::Util::PathName GetDllPathName(bool canonicalized);
#endif

public:
  bool GetPsFontDirs(std::string& psFontDirs);

public:
  bool GetOTFDirs(std::string& otfDirs);

public:
  bool GetTTFDirs(std::string& ttfDirs);

public:
  void RecordMaintenance();

private:
  void ReadDvipsPaperSizes();

private:
  void WriteDvipsPaperSizes();

private:
  void WriteDvipdfmxPaperSize();

private:
  void WritePdfTeXPaperSize();

private:
  void AddDvipsPaperSize(const DvipsPaperSizeInfo& dvipsPaperSizeInfo);

#if defined(MIKTEX_WINDOWS)
private:
  bool GetAcrobatFontDir(MiKTeX::Util::PathName& path);
#endif

#if defined(MIKTEX_WINDOWS)
private:
  bool GetATMFontDir(MiKTeX::Util::PathName& path);
#endif

#if defined(MIKTEX_WINDOWS)
private:
  bool IsUserMemberOfGroup(DWORD localGroup);
#endif

#if defined(MIKTEX_WINDOWS)
private:
  bool RunningElevated();
#endif

private:
  std::tuple<MiKTeX::Util::PathName, std::vector<std::string>> GetScript(const std::string& scriptEngine, const std::string& name);

private:
  int RunScript(const std::string& scriptEngine, const std::string& scriptEngineArgument, int argc, const char** argv);

public:
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_access;
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_config;
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_core;
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_env;
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_error;
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_files;
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_filesearch;
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_fndb;
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_fonts;
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mem;
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_mmap;
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_process;
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_stopwatch;
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_tempfile;
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_values;

public:
  std::locale defaultLocale;

private:
  MiKTeX::Util::PathName startDirectory;

#if defined(MIKTEX_WINDOWS) && defined(MIKTEX_CORE_SHARED)
public:
  static HINSTANCE hinstDLL;
#endif

#if defined(MIKTEX_WINDOWS) && defined(MIKTEX_CORE_SHARED)
public:
  static MiKTeX::Configuration::TriState dynamicLoad;
#endif

public:
  static std::weak_ptr<SessionImpl> theSession;

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
public:
  static bool runningAsLocalServer;
#endif

public:
  static std::shared_ptr<SessionImpl> TryGetSession()
  {
    return theSession.lock();
  }

#define SESSION_IMPL() (MiKTeX::ABF3880A6239B84E87DC7E727A8BBFD4::SessionImpl::theSession.expired() ? (MIKTEX_UNEXPECTED(), nullptr) : MiKTeX::ABF3880A6239B84E87DC7E727A8BBFD4::SessionImpl::theSession.lock())

public:
  MiKTeX::Util::PathName GetRelativeFilenameDatabasePathName(unsigned r);

#if defined(HAVE_ATLBASE_H) && defined(MIKTEX_CORE_SHARED)
public:
  static BOOL AtlDllMain(DWORD reason, LPVOID lpReserved);
#endif

private:
  enum class InternalFlag
  {
    CachedSystemFontDirs,
    CachedPsFontDirs,
    CachedTtfDirs,
    CachedOtfDirs,
#if defined(MIKTEX_WINDOWS)
    CachedAcrobatFontDir,
#endif
#if defined(MIKTEX_WINDOWS)
    CachedAtmFontDir,
#endif
  };

private:
  std::bitset<32> flags;

private:
  std::vector<std::string> systemFontDirs;

private:
  std::string psFontDirs;

private:
  std::string ttfDirs;

private:
  std::string otfDirs;

#if defined(MIKTEX_WINDOWS)
private:
  MiKTeX::Util::PathName acrobatFontDir;
#endif

#if defined(MIKTEX_WINDOWS)
private:
  MiKTeX::Util::PathName atmFontDir;
#endif

private:
  bool GetWorkingDirectory(unsigned n, MiKTeX::Util::PathName& path);

private:
  std::vector<MiKTeX::Util::PathName> GetDirectoryPatterns(MiKTeX::Core::FileType fileType);

private:
  void TraceDirectoryPatterns(const std::string& fileType, const std::vector<MiKTeX::Util::PathName>& pathPatterns);

private:
  void RegisterLibraryTraceStreams();

private:
  void UnregisterLibraryTraceStreams();

private:
  bool FindInTypefaceMap(const std::string& fontName, std::string& typeface);

private:
  bool FindInSupplierMap(const std::string& fontName, std::string& supplier, std::string& typeface);

private:
  bool FindInSpecialMap(const std::string& fontName, std::string& supplier, std::string& typeface);

private:
  bool InternalGetFontInfo(const std::string& fontName, std::string& supplier, std::string& typeface);

private:
  bool MakePkFileName(MiKTeX::Util::PathName& pkFileName, const std::string& fontName, int dpi);

private:
  bool FindFileInDirectories(const std::string& fileName, const std::vector<MiKTeX::Util::PathName>& pathPatterns, bool all, bool useFndb, bool searchFileSystem, std::vector<MiKTeX::Util::PathName>& result, MiKTeX::Core::IFindFileCallback* callback);

private:
  bool FindFileByType(const std::string& fileName, MiKTeX::Core::FileType fileType, bool all, bool tryHard, bool create, bool renew, std::vector<MiKTeX::Util::PathName>& result, MiKTeX::Core::IFindFileCallback* callback);

private:
  bool SearchFileSystem(const std::string& fileName, const char* dirPath, bool all, std::vector<MiKTeX::Util::PathName>& result, MiKTeX::Core::IFindFileCallback* callback);

private:
  bool CheckCandidate(MiKTeX::Util::PathName& path, const char* fileInfo, MiKTeX::Core::IFindFileCallback* callback);

private:
  bool GetSessionValue(const std::string& sectionName, const std::string& valueName, std::string& value, MiKTeX::Configuration::HasNamedValues* callback);

private:
  void ReadAllConfigFiles(const std::string& baseName, MiKTeX::Core::Cfg& cfg);

private:
  std::deque<MiKTeX::Util::PathName> inputDirectories;

public:
  std::unordered_map<std::string, std::string> CreateChildEnvironment(bool changeDirectory);

private:
  std::vector<MiKTeX::Util::PathName> GetFilenameDatabasePathNames(unsigned r);

private:
  unsigned GetMpmRoot();

private:
  bool IsMpmFile(const char* path);

public:
  unsigned GetDataRoot();

public:
  unsigned GetCommonDataRoot();

public:
  unsigned GetUserDataRoot();

public:
  unsigned GetConfigRoot();

public:
  unsigned GetCommonConfigRoot();

public:
  unsigned GetUserConfigRoot();

public:
  unsigned GetInstallRoot();

public:
  unsigned GetCommonInstallRoot();

public:
  unsigned GetUserInstallRoot();

public:
  MiKTeX::Util::PathName GetDistRootDirectory();

public:
  std::pair<bool, MiKTeX::Util::PathName> TryGetDistRootDirectory();

private:
  bool IsManagedRoot(unsigned root);

private:
  void MoveRootDirectory(unsigned r, int dir);

private:
  void ReregisterRootDirectories(const std::string& roots, bool other);

private:
  unsigned RegisterRootDirectory(const MiKTeX::Util::PathName& root, MiKTeX::Core::RootDirectoryInfo::Purpose purpose, MiKTeX::Core::ConfigurationScope scope, bool other, bool review);

private:
  bool FindBinRelative(const MiKTeX::Util::PathName& relPath, MiKTeX::Util::PathName& path);

private:
  bool FindStartupConfigFile(MiKTeX::Core::ConfigurationScope scope, MiKTeX::Util::PathName& path);

private:
  InternalStartupConfig ReadStartupConfigFile(MiKTeX::Core::ConfigurationScope scope, const MiKTeX::Util::PathName& path);

private:
  InternalStartupConfig ReadMiKTeXConfig(const MiKTeX::Util::PathName& path);

private:
  MiKTeX::Util::PathName GetStartupConfigFile(MiKTeX::Core::ConfigurationScope scope, MiKTeX::Core::MiKTeXConfiguration config, MiKTeX::Core::VersionNumber version);

private:
  void WriteStartupConfigFile(MiKTeX::Core::ConfigurationScope scope, const InternalStartupConfig& startupConfig);

private:
  InternalStartupConfig ReadEnvironment(MiKTeX::Core::ConfigurationScope scope);

#if defined(MIKTEX_WINDOWS)
private:
  InternalStartupConfig ReadRegistry(MiKTeX::Core::ConfigurationScope scope);
#endif

#if defined(MIKTEX_WINDOWS)
private:
  void WriteRegistry(MiKTeX::Core::ConfigurationScope scope, const InternalStartupConfig& startupConfig);
#endif

private:
  InternalStartupConfig DefaultConfig(MiKTeX::Core::MiKTeXConfiguration config, MiKTeX::Core::VersionNumber setupVersion, const MiKTeX::Util::PathName& commonPrefix, const MiKTeX::Util::PathName& userPrefix);

private:
  InternalStartupConfig DefaultConfig()
  {
    return DefaultConfig(initStartupConfig.config, initStartupConfig.setupVersion, MiKTeX::Util::PathName(), MiKTeX::Util::PathName());
  }

private:
  void InitializeStartupConfig();

private:
  void MergeStartupConfig(InternalStartupConfig& startupConfig, const InternalStartupConfig& defaults);

private:
  void InitializeRootDirectories(const InternalStartupConfig& startupConfig, bool review);

private:
  void SaveStartupConfig(const InternalStartupConfig& startupConfig, MiKTeX::Core::RegisterRootDirectoriesOptionSet options);

private:
  bool IsTeXMFReadOnly(unsigned r);

private:
  std::pair<bool, MiKTeX::Util::PathName> TryGetBinDirectory(bool canonicalized);

private:
  MiKTeX::Util::PathName GetBinDirectory(bool canonicalized);

private:
  std::vector<MiKTeX::Util::PathName> SplitSearchPath(const std::string& searchPath);

private:
  void PushBackPath(std::vector<MiKTeX::Util::PathName>& pathvec, const MiKTeX::Util::PathName& path);

private:
  void ReadFormatsIni();

private:
  void ReadFormatsIni(const MiKTeX::Util::PathName& cfgFile);

private:
  void ReadLanguagesIni();

private:
  void ReadLanguagesIni(const MiKTeX::Util::PathName& cfgFile);

private:
  void WriteLanguagesIni();

private:
  void WriteFormatsIni();

private:
  void ReadMetafontModes();

private:
  bool FindMETAFONTMode(const char* mnemonic, MiKTeX::Core::MIKTEXMFMODE* mfMode);

private:
  std::tuple<std::unique_ptr<MiKTeX::Core::Process>, FILE*> InitiateProcessPipe(const std::string& command, MiKTeX::Core::FileAccess access, MiKTeX::Core::FileMode& mode);

private:
  int CloseProcessPipe(MiKTeX::Core::Process* process, FILE* file);

private:
  FILE* OpenFileOnStream(std::unique_ptr<MiKTeX::Core::Stream> stream);

private:
  void CheckOpenFiles();

private:
  void WritePackageHistory();

private:
  std::string ExpandValues(const std::string& toBeExpanded, MiKTeX::Configuration::HasNamedValues* callback);

private:
  void DirectoryWalk(const MiKTeX::Util::PathName& directory, const MiKTeX::Util::PathName& pathPattern, std::vector<MiKTeX::Util::PathName>& paths);

private:
  void ExpandBraces(const std::string& toBeExpanded, std::vector<MiKTeX::Util::PathName>& paths);

private:
  std::vector<MiKTeX::Util::PathName> ExpandBraces(const std::string& toBeExpanded);

private:
  void ExpandRootDirectories(const std::string& toBeExpanded, std::vector<MiKTeX::Util::PathName>& paths);

private:
  std::vector<MiKTeX::Util::PathName> ExpandRootDirectories(const std::string& toBeExpanded);

private:
  void ExpandPathPattern(const MiKTeX::Util::PathName& directory, const MiKTeX::Util::PathName& pathPattern, std::vector<MiKTeX::Util::PathName>& paths);

private:
  std::vector<MiKTeX::Util::PathName> ExpandPathPatterns(const std::string& toBeExpanded);

private:
  void RegisterFileType(MiKTeX::Core::FileType fileType);

private:
  void RegisterFileTypes();

private:
  InternalFileTypeInfo* GetInternalFileTypeInfo(MiKTeX::Core::FileType fileType);

private:
  void ClearSearchVectors();

private:
  MiKTeX::Util::PathName pathGsExe;

private:
  MiKTeX::Core::VersionNumber gsVersion;

private:
  MiKTeX::Core::IFindFileCallback* findFileCallback = nullptr;

private:
  std::vector<InternalFileTypeInfo> fileTypes;

private:
  std::vector<MiKTeX::Core::MIKTEXMFMODE> metafontModes;

private:
  struct InternalOpenFileInfo :
    public MiKTeX::Core::Session::OpenFileInfo
  {
    std::unique_ptr<MiKTeX::Core::Process> process;
  };

private:
  // caching open files
  std::map<const FILE*, InternalOpenFileInfo> openFilesMap;

private:
  // caching path patterns
  typedef std::unordered_map<std::string, std::vector<MiKTeX::Util::PathName>> SearchPathDictionary;

private:
  SearchPathDictionary expandedPathPatterns;

private:
  // file access history
  std::vector<MiKTeX::Core::FileInfoRecord> fileInfoRecords;

private:
  // true, if we record a file history
  bool recordingFileNames = false;

private:
  // true, if we record a package history
  bool recordingPackageNames = false;

private:
  std::ofstream fileNameRecorderStream;

private:
  // package history file
  std::string packageHistoryFile;

private:
  bool makeFonts = true;

private:
  typedef std::unordered_map<std::string, std::unique_ptr<MiKTeX::Core::Cfg>> ConfigurationSettings;

private:
  ConfigurationSettings configurationSettings;

private:
  std::vector<FormatInfo_> formats;

private:
  std::vector<LanguageInfo_> languages;

private:
  InternalStartupConfig initStartupConfig;

private:
  // session initialization options provided by caller
  MiKTeX::Core::Session::InitInfo initInfo;

private:
  bool adminMode = false;

private:
  MiKTeX::Configuration::TriState runningAsAdministrator = MiKTeX::Configuration::TriState::Undetermined;

private:
  MiKTeX::Configuration::TriState isUserAnAdministrator = MiKTeX::Configuration::TriState::Undetermined;

private:
  MiKTeX::Configuration::TriState isSharedSetup = MiKTeX::Configuration::TriState::Undetermined;

private:
  std::string applicationNames;

private:
  const std::string& get_ApplicationNames() const
  {
    return applicationNames;
  }

private:
  std::string theNameOfTheGame;

public:
  std::string GetEngineName() override
  {
    std::string engine = theNameOfTheGame;
    if (engine.empty())
    {
      if (!GetEnvironmentString("engine", engine))
      {
        engine = "engine-not-set";
      }
    }
    for(char& ch : engine)
    {
      if (ch >= 'A' && ch <= 'Z')
      {
        ch = ch - 'A' + 'a';
      }
    }
    return engine;
  }

private:
  // registered root directories
  std::vector<RootDirectoryInternals> rootDirectories;

private:
  std::vector<RootDirectoryInternals>& get_RootDirectories()
  {
    return rootDirectories;
  }

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
private:
  bool UseLocalServer();
#endif

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
private:
  void ConnectToServer();
#endif

private:
  // index of common data root
  unsigned commonDataRootIndex = MiKTeX::Core::INVALID_ROOT_INDEX;

private:
  // index of user data root
  unsigned userDataRootIndex = MiKTeX::Core::INVALID_ROOT_INDEX;

private:
  // index of common install root
  unsigned commonInstallRootIndex = MiKTeX::Core::INVALID_ROOT_INDEX;

private:
  // index of user install root
  unsigned userInstallRootIndex = MiKTeX::Core::INVALID_ROOT_INDEX;

private:
  // index of common config root
  unsigned commonConfigRootIndex = MiKTeX::Core::INVALID_ROOT_INDEX;

private:
  // index of user config root
  unsigned userConfigRootIndex = MiKTeX::Core::INVALID_ROOT_INDEX;

private:
  std::vector<DvipsPaperSizeInfo> dvipsPaperSizes;

private:
  std::set<std::string> valuesBeingExpanded;

private:
  // the MiKTeXDirect flag:
  // TriState::True if running from a MiKTeXDirect medium
  // TriState::False if not running from a MiKTeXDirect medium
  // TriState::Undetermined otherwise
 MiKTeX::Configuration::TriState triMiKTeXDirect = MiKTeX::Configuration::TriState::Undetermined;

private:
  // fully qualified path to the running application file
  MiKTeX::Util::PathName myProgramFile;
  MiKTeX::Util::PathName myProgramFileCanon;

private:
  MiKTeX::Util::PathName dllPathName;
  MiKTeX::Util::PathName dllPathNameCanon;

private:
  std::vector<std::string> onFinishScript;

private:
  void StartFinishScript(int delay);

private:
  std::shared_ptr<MiKTeX::Core::FileSystemWatcher> fsWatcher;

private:
  // indicates whether the session has been initialized
  bool initialized = false;

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
private:
  struct LocalServer :
    public MiKTeX::Core::COMInitializer
  {
    ATL::CComQIPtr<MiKTeXSessionLib::ISession> pSession;
  };
  std::unique_ptr<LocalServer> localServer;
#endif

private:
  friend class MiKTeX::Core::Session;
};

CORE_INTERNAL_END_NAMESPACE;

#endif
