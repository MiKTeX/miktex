/* miktex/Setup/SetupService.h:                         -*- C++ -*-

   Copyright (C) 2013-2021 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#pragma once

#if !defined(FAA6EA3649584988994D2B7032C040AB)
#define FAA6EA3649584988994D2B7032C040AB

// DLL import/export switch
#if !defined(C09CDC45E4B649EDA745DAF436D18309)
#  if defined(MIKTEX_SETUP_SHARED)
#    define MIKTEXSETUPEXPORT MIKTEXDLLIMPORT
#  else
#    define MIKTEXSETUPEXPORT
#  endif
#endif

// API decoration for exported member functions
#define MIKTEXSETUPCEEAPI(type) MIKTEXSETUPEXPORT type MIKTEXCEECALL

#define MIKTEX_SETUP_BEGIN_NAMESPACE            \
  namespace MiKTeX {                            \
    namespace Setup {

#define MIKTEX_SETUP_END_NAMESPACE              \
    }                                           \
  }

#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include <miktex/Configuration/TriState>

#include <miktex/Util/OptionSet>
#include <miktex/Util/PathName>
#include <miktex/Core/Session>
#include <miktex/Core/TemporaryDirectory>

#include <miktex/PackageManager/PackageManager>

/// @namespace MiKTeX::Setup
/// @brief Setup utilities.
MIKTEX_SETUP_BEGIN_NAMESPACE;

enum class SetupTask
{
  None,
  Download,
  InstallFromCD,
  InstallFromLocalRepository,
  InstallFromRemoteRepository, // <todo/>
  PrepareMiKTeXDirect,
  FinishSetup,
  FinishUpdate,
  CleanUp
};

enum class CleanupOption
{
  Components,
  Links,
  FileTypes,
  LogFiles,
  Path,
  Registry,
  RootDirectories,
  StartMenu
};

typedef MiKTeX::Util::OptionSet<CleanupOption> CleanupOptionSet;

enum class ReportOption
{
  General,
  CurrentUser,
  Environment,
  Processes,
  RootDirectories,
  BrokenPackages
};

typedef MiKTeX::Util::OptionSet<ReportOption> ReportOptionSet;

enum class IssueType
{
  Path,
  AdminUpdateCheckOverdue,
  UserUpdateCheckOverdue,
  RootDirectoryCoverage,
  PackageDamaged
};

enum class IssueSeverity
{
  Critical = 1,
  Major = 6,
  Minor = 15,
  Trivial = 43
};

struct Issue
{
  IssueType type;
  IssueSeverity severity;
  std::string message;
  std::string remedy;
  std::string tag;
  MIKTEXSETUPCEEAPI(std::string) GetUrl() const;
  MIKTEXSETUPCEEAPI(std::string) ToString() const;
};

inline std::ostream& operator<<(std::ostream& os, const Issue& issue)
{
  return os << issue.ToString();
}


struct SetupOptions
{
public:
  SetupTask Task = SetupTask::None;

public:
  std::string Banner;

public:
  std::string Version;

public:
  bool IsDryRun = false;

public:
  bool IsCommonSetup = false;

public:
  bool IsPortable = false;

public:
  bool IsRegisterPathEnabled = false;

public:
  bool IsPrefabricated = false;

#if defined(MIKTEX_WINDOWS)
public:
  bool IsRegistryEnabled = false;
#endif

public:
  CleanupOptionSet CleanupOptions;

public:
  MiKTeX::Configuration::TriState IsInstallOnTheFlyEnabled = MiKTeX::Configuration::TriState::Undetermined;

public:
  MiKTeX::Core::StartupConfig Config;

public:
  MiKTeX::Util::PathName MiKTeXDirectRoot;

public:
  MiKTeX::Util::PathName PortableRoot;

public:
  std::string RemotePackageRepository;

public:
  MiKTeX::Packages::PackageLevel PackageLevel = MiKTeX::Packages::PackageLevel::None;

public:
  MiKTeX::Util::PathName LocalPackageRepository;

#if defined(MIKTEX_WINDOWS)
public:
  MiKTeX::Util::PathName FolderName;
#endif

public:
  std::string PaperSize = "A4";

public:
  MiKTeX::Util::PathName CommonLinkTargetDirectory;

public:
  MiKTeX::Util::PathName UserLinkTargetDirectory;
};

/// Notification enum class.
enum class Notification
{
  None = 0,
  /// Package download is about to start.
  DownloadPackageStart,
  /// Package download has finished.
  DownloadPackageEnd,
  /// File installation is about to start.
  InstallFileStart,
  /// File installation has finished.
  InstallFileEnd,
  /// Package installation is about to start.
  InstallPackageStart,
  /// Package installation has finished.
  InstallPackageEnd,
  /// File removal is about to start.
  RemoveFileStart,
  /// File removal has finished.
  RemoveFileEnd,
  /// Package removal is about to start.
  RemovePackageStart,
  /// Package removal has finished.
  RemovePackageEnd,

  ConfigureBegin,
  ConfigureEnd,
};

/// Callback interface.
class MIKTEXNOVTABLE SetupServiceCallback
{
  /// Reporting. This method is called by the installer if a new message
  /// is available.
  /// @param lpszLine One-line message.
public:
  virtual void MIKTEXTHISCALL ReportLine(const std::string& str) = 0;

  /// Error handling. This method is called by the installer if a
  /// problem was detected which can be remedied by the user.
  /// @param lpszMessage Message to be presented to the user.
  /// @returns Returns true, if the user has fixed the problem.
public:
  virtual bool MIKTEXTHISCALL OnRetryableError(const std::string& message) = 0;

  /// Progress. This method is called by the installer if new
  /// progress information is available. Also gives the opportunity
  /// to abort all activities.
  /// @param nf Notification code.
  /// @returns Returns true, if the installer shall continue.
public:
  virtual bool MIKTEXTHISCALL OnProgress(Notification nf) = 0;

  /// Output function. Called by the Process object if new output text
  /// is available.
  /// @param output Output text bytes.
  /// @param n Number of output text bytes.
  /// @return Returns true, of the Process object shall continue.
public:
  virtual bool MIKTEXTHISCALL OnProcessOutput(const void* output, size_t n) = 0;
};

class MIKTEXNOVTABLE SetupService
{
public:
  virtual MIKTEXTHISCALL ~SetupService() noexcept = 0;

public:
  virtual void MIKTEXTHISCALL Initialize() = 0;

public:
  virtual SetupOptions MIKTEXTHISCALL GetOptions() = 0;

public:
  virtual SetupOptions MIKTEXTHISCALL SetOptions(const SetupOptions& options) = 0;

public:
  virtual void MIKTEXTHISCALL OpenLog() = 0;

public:
  virtual MiKTeX::Util::PathName MIKTEXTHISCALL CloseLog(bool cancel) = 0;

public:
  virtual void MIKTEXCEECALL Log(const std::string& s) = 0;

public:
  virtual void MIKTEXTHISCALL ULogOpen() = 0;

public:
  virtual void MIKTEXTHISCALL ULogClose() = 0;

public:
  virtual MiKTeX::Util::PathName MIKTEXTHISCALL GetULogFileName() = 0;

public:
  virtual void ULogAddFile(const MiKTeX::Util::PathName& path) = 0;

#if defined(MIKTEX_WINDOWS)
public:
  virtual void ULogAddRegValue(HKEY hkey, const std::string& valueName, const std::string& value) = 0;
#endif

  /// Progress info struct.
public:
  struct ProgressInfo
  {
    /// Package ID.
    std::string packageId;

    /// Display name of package.
    std::string displayName;

    /// Path name of current file.
    MiKTeX::Util::PathName fileName;

    /// Number of removed files.
    unsigned long cFilesRemoveCompleted = 0;

    /// Number of files to be removed.
    unsigned long cFilesRemoveTotal = 0;

    /// Number of removed packages.
    unsigned long cPackagesRemoveCompleted = 0;

    /// Number of packages to be removed.
    unsigned long cPackagesRemoveTotal = 0;

    /// Number of received bytes (current package).
    size_t cbPackageDownloadCompleted = 0;

    /// Number of bytes to be received (current package).
    size_t cbPackageDownloadTotal = 0;

    /// Number of bytes received.
    size_t cbDownloadCompleted = 0;

    /// Number of bytes to be received.
    size_t cbDownloadTotal = 0;

    /// Number of installed files (current package).
    size_t cFilesPackageInstallCompleted = 0;

    /// Number of files to be installed (current package).
    unsigned long cFilesPackageInstallTotal = 0;

    /// Number of installed files.
    unsigned long cFilesInstallCompleted = 0;

    /// Number of files to be installed.
    unsigned long cFilesInstallTotal = 0;

    /// Number of installed packages
    unsigned long cPackagesInstallCompleted = 0;

    /// Number of packages to be installed.
    unsigned long cPackagesInstallTotal = 0;

    /// Number of written bytes (current package).
    size_t cbPackageInstallCompleted = 0;

    /// Number of bytes to be written (current package).
    size_t cbPackageInstallTotal = 0;

    /// Number of bytes written.
    size_t cbInstallCompleted = 0;

    /// Number of bytes to be written.
    size_t cbInstallTotal = 0;

    /// Current transfer speed.
    unsigned long bytesPerSecond = 0;

    /// estimated time of arrival (millisecs remaining).
    unsigned long timeRemaining = 0;

    /// Ready flag.
    bool ready = false;

    /// Number of errors.
    unsigned numErrors = 0;

    bool cancelled = false;
  };

  /// Gets progress information.
public:
  virtual ProgressInfo MIKTEXTHISCALL GetProgressInfo() = 0;

  /// Sets the callback interface.
  /// @param callback Pointer to an interface.
public:
  virtual void MIKTEXTHISCALL SetCallback(SetupServiceCallback* callback) = 0;

public:
  virtual void MIKTEXTHISCALL SetCallbacks(std::function<void(const std::string&)> f_ReportLine, std::function<bool(const std::string&)> f_OnRetryableError, std::function<bool(MiKTeX::Setup::Notification)> f_OnProgress, std::function<bool(const void*, size_t)> f_OnProcessOutput) = 0;

public:
  virtual void MIKTEXTHISCALL Run() = 0;

#if defined(MIKTEX_WINDOWS)
public:
  virtual void MIKTEXTHISCALL CreateProgramIcons() = 0;
#endif

public:
  virtual void MIKTEXTHISCALL WriteReport(std::ostream& s, ReportOptionSet options) = 0;

public:
  virtual void MIKTEXTHISCALL WriteReport(std::ostream& s) = 0;

public:
  virtual std::vector<Issue> MIKTEXTHISCALL FindIssues(bool checkPath, bool checkPackageIntegrity) = 0;

public:
  virtual std::vector<Issue> MIKTEXTHISCALL GetIssues() = 0;

public:
  static MIKTEXSETUPCEEAPI(std::unique_ptr<SetupService>) Create();

public:
  static MIKTEXSETUPCEEAPI(std::unique_ptr<MiKTeX::Core::TemporaryDirectory>) CreateSandbox(MiKTeX::Core::StartupConfig& startupConfig);

public:
  static MIKTEXSETUPCEEAPI(MiKTeX::Packages::PackageLevel) TestLocalRepository(const MiKTeX::Util::PathName& pathRepository, MiKTeX::Packages::PackageLevel requestedPackageLevel);

public:
  static MIKTEXSETUPCEEAPI(MiKTeX::Util::PathName) GetDefaultLocalRepository();

public:
  static MIKTEXSETUPCEEAPI(MiKTeX::Packages::PackageLevel) SearchLocalRepository(MiKTeX::Util::PathName& localRepository, MiKTeX::Packages::PackageLevel requestedPackageLevel, bool& prefabricated);

public:
  static MIKTEXSETUPCEEAPI(MiKTeX::Util::PathName) GetDefaultCommonInstallDir();

public:
  static MIKTEXSETUPCEEAPI(MiKTeX::Util::PathName) GetDefaultUserInstallDir();

public:
  static MIKTEXSETUPCEEAPI(MiKTeX::Util::PathName) GetDefaultPortableRoot();

public:
  static MIKTEXSETUPCEEAPI(bool) IsMiKTeXDirect(MiKTeX::Util::PathName& MiKTeXDirectRoot);

public:
  static MIKTEXSETUPCEEAPI(std::unique_ptr<MiKTeX::Core::TemporaryDirectory>) ExtractFiles();
};

MIKTEX_SETUP_END_NAMESPACE;

#undef MIKTEX_SETUP_BEGIN_NAMESPACE
#undef MIKTEX_SETUP_END_NAMESPACE

#endif
