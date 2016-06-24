/* SessionImpl.h: Session impl class                    -*- C++ -*-

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

#if !defined(INTERNAL_CORE_SESSION_SESSIONIMPL_H)
#define INTERNAL_CORE_SESSION_SESSIONIMPL_H

#include "miktex/Core/Cfg.h"
#include "miktex/Core/Session.h"
#include "miktex/Core/Stream.h"
#include "miktex/Core/StreamWriter.h"
#include "miktex/Core/equal_icase.h"
#include "miktex/Core/hash_icase.h"

#include "core-version.h"
#include "Fndb/FileNameDatabase.h"
#include "RootDirectory.h"

BEGIN_INTERNAL_NAMESPACE;

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
namespace MiKTeXSessionLib = MAKE_CURVER_ID(MiKTeXSession);
#endif

struct FormatInfo_ : public MiKTeX::Core::FormatInfo
{
public:
  FormatInfo_()
  {
  }

public:
  FormatInfo_(const MiKTeX::Core::FormatInfo & other) :
    MiKTeX::Core::FormatInfo(other)
  {
  }

public:
  MiKTeX::Core::PathName cfgFile;
};

struct LanguageInfo_ : public MiKTeX::Core::LanguageInfo
{
public:
  LanguageInfo_()
  {
  }

public:
  LanguageInfo_(const MiKTeX::Core::LanguageInfo & other) :
    MiKTeX::Core::LanguageInfo(other)
  {
  }

public:
  MiKTeX::Core::PathName cfgFile;
};

inline bool operator< (const LanguageInfo_ & lhs, const LanguageInfo_ & rhs)
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

struct InternalFileTypeInfo : public MiKTeX::Core::FileTypeInfo
{
public:
  std::vector<MiKTeX::Core::PathName> searchVec;
public:
  std::string alternateExtensions;
};

class DvipsPaperSizeInfo : public MiKTeX::Core::PaperSizeInfo
{
public:
  std::vector<std::string> definition;
};

class SessionImpl : public MiKTeX::Core::Session
{
public:
  SessionImpl();

public:
  virtual ~SessionImpl();

private:
  void Initialize(const InitInfo & initInfo);

private:
  void Uninitialize();

public:
  void PushAppName(const std::string & name) override;

public:
  void PushBackAppName(const std::string & name) override;

public:
  void AddInputDirectory(const char * lpszPath, bool atEnd) override;

public:
  MiKTeX::Core::PathName GetSpecialPath(MiKTeX::Core::SpecialPath specialPath) override;

public:
  unsigned GetNumberOfTEXMFRoots() override;

public:
  MiKTeX::Core::PathName GetRootDirectory(unsigned r) override;

public:
  bool IsCommonRootDirectory(unsigned r) override;

public:
  MiKTeX::Core::PathName GetMpmRootPath() override;

public:
  MiKTeX::Core::PathName GetMpmDatabasePathName() override;

public:
  unsigned TryDeriveTEXMFRoot(const MiKTeX::Core::PathName & path) override;

public:
  unsigned DeriveTEXMFRoot(const MiKTeX::Core::PathName & path) override;

public:
  bool FindFilenameDatabase(unsigned r, MiKTeX::Core::PathName & path) override;

public:
  MiKTeX::Core::PathName GetFilenameDatabasePathName(unsigned r) override;

public:
  bool UnloadFilenameDatabase() override;

public:
  unsigned SplitTEXMFPath(const MiKTeX::Core::PathName & path, MiKTeX::Core::PathName & root, MiKTeX::Core::PathName & relative) override;

public:
  void RegisterRootDirectories(const std::string & roots) override;

public:
  void RegisterRootDirectories(const MiKTeX::Core::StartupConfig & startupConfig, MiKTeX::Core::RegisterRootDirectoriesOptionSet options) override;

public:
  bool IsMiKTeXDirect() override;

public:
  bool IsMiKTeXPortable() override;

public:
  bool GetMETAFONTMode(unsigned idx, MiKTeX::Core::MIKTEXMFMODE * pMode) override;

public:
  bool DetermineMETAFONTMode(unsigned dpi, MiKTeX::Core::MIKTEXMFMODE * pMode) override;

public:
  bool TryGetConfigValue(const char * lpszSectionName, const char * lpszValueName, std::string & value) override;

public:
  std::string GetConfigValue(const char * lpszSectionName, const char * lpszValueName, const char * lpszDefaultValue) override;

public:
  int GetConfigValue(const char * lpszSectionName, const char * lpszValueName, int defaultValue) override;

public:
  bool GetConfigValue(const char * lpszSectionName, const char * lpszValueName, bool defaultValue) override;

public:
  MiKTeX::Core::TriState GetConfigValue(const char * lpszSectionName, const char * lpszValueName, MiKTeX::Core::TriState defaultValue) override;

public:
  char GetConfigValue(const char * lpszSectionName, const char * lpszValueName, char defaultValue) override;

public:
  void SetConfigValue(const char * lpszSectionName, const char * lpszValueName, const char * lpszValue) override;

public:
  void SetConfigValue(const char * lpszSectionName, const char * lpszValueName, bool value) override;

public:
  void SetConfigValue(const char * lpszSectionName, const char * lpszValueName, int value) override;

public:
  FILE * OpenFile(const MiKTeX::Core::PathName & path, MiKTeX::Core::FileMode mode, MiKTeX::Core::FileAccess access, bool isTextFile) override;

public:
  FILE * TryOpenFile(const MiKTeX::Core::PathName & path, MiKTeX::Core::FileMode mode, MiKTeX::Core::FileAccess access, bool isTextFile) override;

public:
  FILE * OpenFile(const MiKTeX::Core::PathName & path, MiKTeX::Core::FileMode mode, MiKTeX::Core::FileAccess access, bool isTextFile, MiKTeX::Core::FileShare share) override;

public:
  FILE * TryOpenFile(const MiKTeX::Core::PathName & path, MiKTeX::Core::FileMode mode, MiKTeX::Core::FileAccess access, bool isTextFile, MiKTeX::Core::FileShare share) override;

public:
  void CloseFile(FILE * pFile) override;

public:
  bool IsOutputFile(const FILE * pFile) override;

#if defined(MIKTEX_WINDOWS)
public:
  bool IsFileAlreadyOpen(const char * lpszFileName) override;
#endif

#if defined(MIKTEX_WINDOWS)
public:
  void ScheduleFileRemoval(const char * lpszFileName) override;
#endif

public:
  bool StartFileInfoRecorder() override;

public:
  bool StartFileInfoRecorder(bool RecordPackageNames) override;

public:
  void SetRecorderPath(const MiKTeX::Core::PathName & path) override;

public:
  void RecordFileInfo(const MiKTeX::Core::PathName & path, MiKTeX::Core::FileAccess access) override;

public:
  std::vector<MiKTeX::Core::FileInfoRecord> GetFileInfoRecords() override;

public:
  MiKTeX::Core::FileType DeriveFileType(const char * lpszPath) override;

public:
  bool FindFile(const char * lpszFileName, const char * lpszPathList, FindFileOptionSet options, std::vector<MiKTeX::Core::PathName> & result) override;

public:
  bool FindFile(const char * lpszFileName, const char * lpszPathList, std::vector<MiKTeX::Core::PathName> & result) override
  {
    return FindFile(lpszFileName, lpszPathList, { FindFileOption::All }, result);
  }

public:
  bool FindFile(const char * lpszFileName, const char * lpszPathList, FindFileOptionSet options, MiKTeX::Core::PathName & result) override;

public:
  bool FindFile(const char * lpszFileName, const char * lpszPathList, MiKTeX::Core::PathName & result) override
  {
    return FindFile(lpszFileName, lpszPathList, {}, result);
  }

public:
  bool FindFile(const char * lpszFileName, MiKTeX::Core::FileType fileType, FindFileOptionSet options, std::vector<MiKTeX::Core::PathName> & result) override;

public:
  bool FindFile(const char * lpszFileName, MiKTeX::Core::FileType fileType, std::vector<MiKTeX::Core::PathName> & result) override
  {
    return FindFile(lpszFileName, fileType, { FindFileOption::All }, result);
  }

public:
  bool FindFile(const char * lpszFileName, MiKTeX::Core::FileType fileType, FindFileOptionSet options, MiKTeX::Core::PathName & result) override;

public:
  bool FindFile(const char * lpszFileName, MiKTeX::Core::FileType fileType, MiKTeX::Core::PathName & result) override
  {
    return FindFile(lpszFileName, fileType, {}, result);
  }

public:
  bool FindPkFile(const char * lpszFontName, const char * lpszMode, int dpi, MiKTeX::Core::PathName & result) override;

public:
  bool FindTfmFile(const char * lpszFontName, MiKTeX::Core::PathName & result, bool create) override
  {
    return FindFile(lpszFontName, MiKTeX::Core::FileType::TFM, (create ? FindFileOptionSet({ FindFileOption::Create }) : FindFileOptionSet()), result);
  }

public:
  void SetFindFileCallback(MiKTeX::Core::IFindFileCallback * callback) override;

public:
  void SplitFontPath(const char * lpszFontPath, char * lpszFontType, char * lpszSupplier, char * lpszTypeface, char * lpszFontName, char * lpszPointSize) override;

public:
  bool GetFontInfo(const char * lpszFontName, char * lpszSupplier, char * lpszTypeface, double * lpGenSize) override;

public:
  void GetGhostscript(char * lpszPath, unsigned long * pVersionNumber) override;

public:
  std::string GetExpandedSearchPath(MiKTeX::Core::FileType fileType) override;

public:
  bool FindGraphicsRule(const char * lpszFrom, const char * lpszTo, char * lpszRule, size_t bufSize) override;

public:
  bool ConvertToBitmapFile(const char * lpszSourceFileName, char * lpszDestFileName, MiKTeX::Core::IRunProcessCallback * pCallback) override;

public:
  bool EnableFontMaker(bool enable) override;

public:
  bool GetMakeFontsFlag() override;

public:
  std::string MakeMakePkCommandLine(const char * lpszFontName, int dpi, int baseDpi, const char * lpszMfMode, MiKTeX::Core::PathName & fileName, MiKTeX::Core::TriState enableInstaller) override;

#if defined(MIKTEX_WINDOWS)
public:
  int RunBatch(int argc, const char ** argv) override;
#endif

public:
  int RunPerl(int argc, const char ** argv) override;

public:
  int RunPython(int argc, const char ** argv) override;

public:
  int RunJava(int argc, const char ** argv) override;

#if defined(MIKTEX_WINDOWS)
public:
  bool ShowManualPageAndWait(HWND hWnd, unsigned long topic) override;
#endif

public:
  std::vector<MiKTeX::Core::FileTypeInfo> GetFileTypes() override;

public:
  std::vector<MiKTeX::Core::FormatInfo> GetFormats() override;

public:
  MiKTeX::Core::FormatInfo GetFormatInfo(const char * lpszKey) override;

public:
  bool TryGetFormatInfo(const char * lpszKey, MiKTeX::Core::FormatInfo & formatInfo) override;

public:
  void DeleteFormatInfo(const char * lpszKey) override;

public:
  void SetFormatInfo(const MiKTeX::Core::FormatInfo & formatInfo) override;

public:
  std::vector<MiKTeX::Core::LanguageInfo> GetLanguages() override;

public:
  MiKTeX::Core::PathName GetMyLocation(bool canonicalized) override;

public:
  MiKTeX::Core::PathName GetMyPrefix() override;

public:
  bool RunningAsAdministrator() override;

public:
  void SetAdminMode(bool adminMode, bool isSetup) override;

public:
  bool IsAdminMode() override;

public:
  bool IsSharedSetup() override;

public:
  bool GetPaperSizeInfo(int idx, MiKTeX::Core::PaperSizeInfo & paperSize) override;

public:
  MiKTeX::Core::PaperSizeInfo GetPaperSizeInfo(const char * lpszName) override;

public:
  void SetDefaultPaperSize(const char * lpszDvipsName) override;

public:
  bool TryCreateFromTemplate(const MiKTeX::Core::PathName & path) override;

#if defined(MIKTEX_WINDOWS)
public:
  bool RunningAsPowerUser() override;
#endif

public:
  bool IsUserAnAdministrator() override;

#if defined(MIKTEX_WINDOWS)
public:
  bool IsUserAPowerUser() override;
#endif

public:
  void ConfigureFile(const MiKTeX::Core::PathName & pathIn, const MiKTeX::Core::PathName & pathOut, MiKTeX::Core::HasNamedValues * callback) override;

public:
  void ConfigureFile(const MiKTeX::Core::PathName & pathRel, MiKTeX::Core::HasNamedValues * callback) override;

public:
  void SetTheNameOfTheGame(const char * lpszTheNameOfTheGame) override;

public:
  std::string GetLocalFontDirectories() override;

public:
  MiKTeX::Core::FileTypeInfo GetFileTypeInfo(MiKTeX::Core::FileType fileType) override;

public:
  std::string Expand(const char * lpszToBeExpanded) override;

public:
  std::string Expand(const char * lpszToBeExpanded, MiKTeX::Core::HasNamedValues * callback) override;

public:
  std::string Expand(const char * lpszToBeExpanded, MiKTeX::Core::ExpandOptionSet options, MiKTeX::Core::HasNamedValues * callback) override;

public:
  void SetLanguageInfo(const MiKTeX::Core::LanguageInfo & languageInfo) override;

#if HAVE_MIKTEX_USER_INFO
public:
  MiKTeX::Core::MiKTeXUserInfo RegisterMiKTeXUser(const MiKTeX::Core::MiKTeXUserInfo & info) override;
#endif

#if HAVE_MIKTEX_USER_INFO
public:
  bool TryGetMiKTeXUserInfo(MiKTeX::Core::MiKTeXUserInfo & info) override;
#endif

public:
  bool IsTEXMFFile(const MiKTeX::Core::PathName & path, MiKTeX::Core::PathName & relPath, unsigned & rootIndex);

public:
  bool IsTEXMFFile(const MiKTeX::Core::PathName & path, MiKTeX::Core::PathName & relPath)
  {
    unsigned rootIndex;
    return IsTEXMFFile(path, relPath, rootIndex);
  }

public:
  bool IsTEXMFFile(const MiKTeX::Core::PathName & path)
  {
    MiKTeX::Core::PathName relPath;
    unsigned rootIndex;
    return IsTEXMFFile(path, relPath, rootIndex);
  }

public:
  bool UnloadFilenameDatabaseInternal(unsigned r, bool bRemove);

private:
  bool UnloadFilenameDatabaseInternal_nolock(unsigned r, bool bRemove);

public:
  std::shared_ptr<FileNameDatabase> GetFileNameDatabase(unsigned r, MiKTeX::Core::TriState triReadOnly);

public:
  std::shared_ptr<FileNameDatabase> GetFileNameDatabase(const char * lpszPath);

public:
  MiKTeX::Core::PathName GetTempDirectory();

  // FIXME: make this an API
public:
  MiKTeX::Core::PathName GetMyProgramFile(bool canonicalized);

#if defined(MIKTEX_WINDOWS) && defined(MIKTEX_CORE_SHARED)
public:
  MiKTeX::Core::PathName GetDllPathName(bool canonicalized);
#endif

public:
  bool GetPsFontDirs(std::string & psFontDirs);

public:
  bool GetOTFDirs(std::string & otfDirs);

public:
  bool GetTTFDirs(std::string & ttfDirs);

private:
  void ReadDvipsPaperSizes();

private:
  void WriteDvipsPaperSizes();

private:
  void WriteDvipdfmPaperSize();

private:
  void WriteDvipdfmxPaperSize();

private:
  void WritePdfTeXPaperSize();

private:
  void AddDvipsPaperSize(const DvipsPaperSizeInfo & dvipsPaperSizeInfo);

#if defined(MIKTEX_WINDOWS)
private:
  bool GetAcrobatFontDir(MiKTeX::Core::PathName & path);
#endif

#if defined(MIKTEX_WINDOWS)
private:
  bool GetATMFontDir(MiKTeX::Core::PathName & path);
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
  int RunScript(const std::string & scriptEngine, const std::string & scriptEngineArgument, int argc, const char ** argv);

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
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_packages;
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_process;
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_tempfile;
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_time;
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_values;

public:
  std::locale defaultLocale;

private:
  MiKTeX::Core::PathName startDirectory;

#if defined(MIKTEX_WINDOWS) && defined(MIKTEX_CORE_SHARED)
public:
  static HINSTANCE hinstDLL;
#endif

#if defined(MIKTEX_WINDOWS) && defined(MIKTEX_CORE_SHARED)
public:
  static MiKTeX::Core::TriState dynamicLoad;
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

public:
  static std::shared_ptr<SessionImpl> GetSession()
  {
    if (theSession.expired())
    {
      MIKTEX_UNEXPECTED();
    }
    return theSession.lock();
  }

public:
  static std::locale & GetDefaultLocale()
  {
    return GetSession()->defaultLocale;
  }

public:
  MiKTeX::Core::PathName GetRelativeFilenameDatabasePathName(unsigned r);

#if defined(HAVE_ATLBASE_H) && defined(MIKTEX_CORE_SHARED)
public:
  static BOOL AtlDllMain(DWORD reason, LPVOID lpReserved);
#endif

private:
  enum class InternalFlag
  {
    CachedLocalFontDirs,
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
  std::string localFontDirs;

private:
  std::string psFontDirs;

private:
  std::string ttfDirs;

private:
  std::string otfDirs;

#if defined(MIKTEX_WINDOWS)
private:
  MiKTeX::Core::PathName acrobatFontDir;
#endif

#if defined(MIKTEX_WINDOWS)
private:
  MiKTeX::Core::PathName atmFontDir;
#endif

private:
  void SetCWDEnv();

private:
  bool GetWorkingDirectory(unsigned n, MiKTeX::Core::PathName & path);

private:
  std::vector<MiKTeX::Core::PathName> ConstructSearchVector(MiKTeX::Core::FileType fileType);

private:
  void TraceSearchVector(const char * lpszKey, const std::vector<MiKTeX::Core::PathName> & pathvec);

private:
  void RegisterLibraryTraceStreams();

private:
  void UnregisterLibraryTraceStreams();

private:
  bool FindInTypefaceMap(const char * lpszFontName, char * lpszTypeface);

private:
  bool FindInSupplierMap(const char * lpszFontName, char * lpszSupplier, char * lpszTypeface);

private:
  bool FindInSpecialMap(const char * lpszFontName, char * lpszSupplier, char * lpszTypeface);

private:
  bool InternalGetFontInfo(const char * lpszFontName, char * lpszSupplier, char * lpszTypeface);


private:
  bool MakePkFileName(MiKTeX::Core::PathName & pkFileName, const char * lpszFontName, int dpi);

private:
  bool FindFileInternal(const char * lpszFileName, const std::vector<MiKTeX::Core::PathName> & vec, bool firstMatchOnly, bool useFndb, bool searchFileSystem, std::vector<MiKTeX::Core::PathName> & result);

private:
  bool FindFileInternal(const char * lpszFileName, MiKTeX::Core::FileType fileType, bool firstMatchOnly, bool tryHard, bool create, bool renew, std::vector<MiKTeX::Core::PathName> & result);

private:
  bool SearchFileSystem(const char * lpszRelPath, const char * lpszDirPath, bool firstMatchOnly, std::vector<MiKTeX::Core::PathName> & result);

private:
  bool CheckCandidate(MiKTeX::Core::PathName & path, const char * lpszFileInfo);

private:
  bool GetSessionValue(const char * lpszSectionName, const char * lpszValueName, std::string & value, const char * lpszDefaultValue);

private:
  void ReadAllConfigFiles(const std::string & baseName, MiKTeX::Core::Cfg & cfg);

private:
  std::deque<MiKTeX::Core::PathName> inputDirectories;

public:
  void SetEnvironmentVariables();

private:
  std::vector<MiKTeX::Core::PathName> GetFilenameDatabasePathNames(unsigned r);

private:
  unsigned GetMpmRoot();

private:
  bool IsMpmFile(const char * lpszPath);

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
  unsigned GetDistRoot();

private:
  bool IsManagedRoot(unsigned root);

private:
  unsigned RegisterRootDirectory(const MiKTeX::Core::PathName & root, bool common);

private:
  bool FindStartupConfigFile(bool common, MiKTeX::Core::PathName & path);

private:
  MiKTeX::Core::StartupConfig ReadStartupConfigFile(bool common, const MiKTeX::Core::PathName & path);

private:
  void WriteStartupConfigFile(bool common, const MiKTeX::Core::StartupConfig & startupConfig);

private:
  MiKTeX::Core::StartupConfig ReadEnvironment(bool common);

#if defined(MIKTEX_WINDOWS)
private:
  MiKTeX::Core::StartupConfig ReadRegistry(bool common);
#endif

#if defined(MIKTEX_WINDOWS)
private:
  void WriteRegistry(bool common, const MiKTeX::Core::StartupConfig & startupConfig);
#endif

private:
  MiKTeX::Core::StartupConfig DefaultConfig(MiKTeX::Core::MiKTeXConfiguration config, const MiKTeX::Core::PathName & commonPrefix, const MiKTeX::Core::PathName & userPrefix);

private:
  MiKTeX::Core::StartupConfig DefaultConfig()
  {
    return DefaultConfig(startupConfig.config, "", "");
  }

private:
  void DoStartupConfig();

private:
  void InitializeRootDirectories(const MiKTeX::Core::StartupConfig & startupConfig);

private:
  void InitializeRootDirectories();

private:
#if defined(MIKTEX_WINDOWS)
  void SaveRootDirectories(bool noRegistry);
#else
  void SaveRootDirectories();
#endif

private:
  bool IsTeXMFReadOnly(unsigned r);

private:
  std::shared_ptr<FileNameDatabase> GetFileNameDatabase(unsigned r);

private:
  MiKTeX::Core::PathName GetBinDirectory();

private:
  void AppendToSearchPath(std::string & strSearchPath, const std::string & strSearchPath2);

private:
  void SplitSearchPath(std::vector<MiKTeX::Core::PathName> & pathvec, const char * lpszSearchPath);

private:
  std::vector<MiKTeX::Core::PathName> SplitSearchPath(const char * lpszSearchPath);

private:
  void PushBackPath(std::vector<MiKTeX::Core::PathName> & pathvec, const MiKTeX::Core::PathName & path);

private:
  void ReadFormatsIni();

private:
  void ReadFormatsIni(const MiKTeX::Core::PathName & cfgFile);

private:
  void ReadLanguagesIni();

private:
  void ReadLanguagesIni(const MiKTeX::Core::PathName & cfgFile);

private:
  void WriteLanguagesIni();

private:
  void WriteFormatsIni();

private:
  void ReadMetafontModes();

private:
  bool FindMETAFONTMode(const char * lpszMnemonic, MiKTeX::Core::MIKTEXMFMODE * pMode);

private:
  FILE * InitiateProcessPipe(const std::string & command, MiKTeX::Core::FileAccess access, MiKTeX::Core::FileMode & mode);

private:
  FILE * OpenFileOnStream(std::unique_ptr<MiKTeX::Core::Stream> stream);

private:
  void CheckOpenFiles();

private:
  void WritePackageHistory();

private:
  std::string ExpandValues(const char * lpszToBeExpanded, MiKTeX::Core::HasNamedValues * callback);

private:
  void DirectoryWalk(const MiKTeX::Core::PathName & directory, const MiKTeX::Core::PathName & pathPattern, std::vector<MiKTeX::Core::PathName> & paths);

private:
  void ExpandBraces(const char * lpszToBeExpanded, std::vector<MiKTeX::Core::PathName> & paths);

private:
  std::vector<MiKTeX::Core::PathName> ExpandBraces(const char * lpszToBeExpanded);

private:
  void ExpandRootDirectories(const char * lpszToBeExpanded, std::vector<MiKTeX::Core::PathName> & paths);

private:
  std::vector<MiKTeX::Core::PathName> ExpandRootDirectories(const char * lpszToBeExpanded);

private:
  void ExpandPathPattern(const MiKTeX::Core::PathName & directory, const MiKTeX::Core::PathName & pathPattern, std::vector<MiKTeX::Core::PathName> & paths);

private:
  std::vector<MiKTeX::Core::PathName> ExpandPathPatterns(const char * lpszToBeExpanded);

private:
  void RegisterFileType(MiKTeX::Core::FileType fileType, const char * lpszFileType, const char * lpszApplication, const char * lpszFileNameExtensions, const char * lpszAlternateExtensions, const char * lpszDefaultSearchPath, const char * lpszEnvVarNames);

private:
  void RegisterFileType(MiKTeX::Core::FileType fileType, const char * lpszFileType, const char * lpszApplication, const char * lpszFileNameExtensions, const char * lpszDefaultSearchPath, const char * lpszEnvVarNames)
  {
    RegisterFileType(fileType, lpszFileType, lpszApplication, lpszFileNameExtensions, nullptr, lpszDefaultSearchPath, lpszEnvVarNames);
  }

private:
  void RegisterFileType(MiKTeX::Core::FileType fileType);

private:
  void RegisterFileTypes();

private:
  InternalFileTypeInfo * GetInternalFileTypeInfo(MiKTeX::Core::FileType fileType);

private:
  void ClearSearchVectors();

private:
  MiKTeX::Core::PathName pathGsExe;

private:
  MiKTeX::Core::VersionNumber gsVersion;

private:
  MiKTeX::Core::IFindFileCallback * findFileCallback = nullptr;

private:
  std::vector<InternalFileTypeInfo> fileTypes;

private:
  std::vector<MiKTeX::Core::MIKTEXMFMODE> metafontModes;

  // info about open file
private:
  struct OpenFileInfo
  {
    const FILE * pFile = nullptr;
    std::string fileName;
    MiKTeX::Core::FileMode mode = MiKTeX::Core::FileMode::Open;
    MiKTeX::Core::FileAccess access = MiKTeX::Core::FileAccess::None;
  };

  // caching open files
private:
  std::map<const FILE *, OpenFileInfo> openFilesMap;

  // caching path patterns
private:
  typedef std::unordered_map<std::string, std::vector<MiKTeX::Core::PathName>> SearchPathDictionary;

private:
  SearchPathDictionary expandedPathPatterns;

  // file access history
private:
  std::vector<MiKTeX::Core::FileInfoRecord> fileInfoRecords;

  // true, if we record a file history
private:
  bool recordingFileNames = false;

  // true, if we record a package history
private:
  bool recordingPackageNames = false;

private:
  MiKTeX::Core::StreamWriter fileNameRecorderStream;

  // package history file
private:
  std::string packageHistoryFile;

private:
  bool makeFonts = true;

private:
  typedef std::unordered_map<std::string, std::unique_ptr<MiKTeX::Core::Cfg>, MiKTeX::Core::hash_icase, MiKTeX::Core::equal_icase> ConfigurationSettings;

private:
  ConfigurationSettings configurationSettings;

private:
  std::vector<FormatInfo_> formats;

private:
  std::vector<LanguageInfo_> languages;

private:
  MiKTeX::Core::StartupConfig startupConfig;

private:
  MiKTeX::Core::Session::InitInfo initInfo;

private:
  bool adminMode = false;

private:
  MiKTeX::Core::TriState runningAsAdministrator = MiKTeX::Core::TriState::Undetermined;

private:
  MiKTeX::Core::TriState isUserAnAdministrator = MiKTeX::Core::TriState::Undetermined;

#if defined(MIKTEX_WINDOWS)
private:
  MiKTeX::Core::TriState runningAsPowerUser = MiKTeX::Core::TriState::Undetermined;
#endif

#if defined(MIKTEX_WINDOWS)
private:
  MiKTeX::Core::TriState isUserAPowerUser = MiKTeX::Core::TriState::Undetermined;
#endif

private:
  MiKTeX::Core::TriState isSharedSetup = MiKTeX::Core::TriState::Undetermined;

private:
  std::string applicationNames;

private:
  const std::string & get_ApplicationNames() const
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
    for(char & ch : engine)
    {
      if (ch >= 'A' && ch <= 'Z')
      {
        ch = ch - 'A' + 'a';
      }
    }
    return engine;
  }

  // registered root directories
private:
  std::vector<RootDirectory> rootDirectories;

private:
  std::vector<RootDirectory> & get_RootDirectories()
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

#if defined(MIKTEX_WINDOWS)
private:
  void MyCoInitialize();
#endif

#if defined(MIKTEX_WINDOWS)
private:
  void MyCoUninitialize();
#endif

  // index of common data root
private:
  unsigned commonDataRootIndex = MiKTeX::Core::INVALID_ROOT_INDEX;

  // index of user data root
private:
  unsigned userDataRootIndex = MiKTeX::Core::INVALID_ROOT_INDEX;

  // index of common install root
private:
  unsigned commonInstallRootIndex = MiKTeX::Core::INVALID_ROOT_INDEX;

  // index of user install root
private:
  unsigned userInstallRootIndex = MiKTeX::Core::INVALID_ROOT_INDEX;

  // index of common config root
private:
  unsigned commonConfigRootIndex = MiKTeX::Core::INVALID_ROOT_INDEX;

  // index of user config root
private:
  unsigned userConfigRootIndex = MiKTeX::Core::INVALID_ROOT_INDEX;

private:
  std::vector<DvipsPaperSizeInfo> dvipsPaperSizes;

private:
  std::set<std::string> valuesBeingExpanded;

  // the MiKTeXDirect flag:
  // TriState::True if running from a MiKTeXDirect medium
  // TriState::False if not running from a MiKTeXDirect medium
  // TriState::Undetermined otherwise
private:
 MiKTeX::Core:: TriState triMiKTeXDirect = MiKTeX::Core::TriState::Undetermined;

  // fully qualified path to the running application file
private:
  MiKTeX::Core::PathName myProgramFile;
  MiKTeX::Core::PathName myProgramFileCanon;

private:
  MiKTeX::Core::PathName dllPathName;
  MiKTeX::Core::PathName dllPathNameCanon;

private:
  bool initialized = false;

#if defined(MIKTEX_WINDOWS)
private:
  int numCoInitialize = 0;
#endif

#if defined(MIKTEX_WINDOWS) && USE_LOCAL_SERVER
private:
  struct
  {
    ATL::CComQIPtr<MiKTeXSessionLib::ISession> pSession;
  } localServer;
#endif

private:
  friend class MiKTeX::Core::Session;
};

END_INTERNAL_NAMESPACE;

#endif
