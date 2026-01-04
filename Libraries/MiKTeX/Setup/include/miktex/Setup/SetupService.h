/**
 * @file SetupService
 * @author Christian Schenk
 * @brief MiKTeX Setup service interface
 *
 * @copyright Copyright © 2013-2026 Christian Schenk
 *
 * This file is part of the MiKTeX Setup Library.
 *
 * The MiKTeX Setup Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#pragma once

// DLL import/export switch
#if !defined(C09CDC45E4B649EDA745DAF436D18309)
#if defined(MIKTEX_SETUP_SHARED)
#define MIKTEXSETUPEXPORT MIKTEXDLLIMPORT
#else
#define MIKTEXSETUPEXPORT
#endif
#endif

// API decoration for exported member functions
#define MIKTEXSETUPCEEAPI(type) MIKTEXSETUPEXPORT type MIKTEXCEECALL

#define MIKTEX_SETUP_BEGIN_NAMESPACE            \
    namespace MiKTeX {                          \
        namespace Setup {

#define MIKTEX_SETUP_END_NAMESPACE              \
        }                                       \
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
    PackageDamaged,
    Windows32bit,
    UnsupportedPlatform,
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

    std::string Banner;
    CleanupOptionSet CleanupOptions;
    MiKTeX::Util::PathName CommonLinkTargetDirectory;
    MiKTeX::Core::StartupConfig Config;
#if defined(MIKTEX_WINDOWS)
    MiKTeX::Util::PathName FolderName;
#endif
    bool IsCommonSetup = false;
    bool IsDryRun = false;
    MiKTeX::Configuration::TriState IsInstallOnTheFlyEnabled = MiKTeX::Configuration::TriState::Undetermined;
    bool IsPortable = false;
    bool IsPrefabricated = false;
    bool IsRegisterPathEnabled = false;
#if defined(MIKTEX_WINDOWS)
    bool IsRegistryEnabled = false;
#endif
    MiKTeX::Util::PathName LocalPackageRepository;
    MiKTeX::Util::PathName MiKTeXDirectRoot;
    MiKTeX::Packages::PackageLevel PackageLevel = MiKTeX::Packages::PackageLevel::None;
    std::string PaperSize = "A4";
    MiKTeX::Util::PathName PortableRoot;
    std::string RemotePackageRepository;
    SetupTask Task = SetupTask::None;
    MiKTeX::Util::PathName UserLinkTargetDirectory;
    std::string Version;
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

public:

    /// Reporting. This method is called by the installer if a new message
    /// is available.
    /// @param lpszLine One-line message.
    virtual void MIKTEXTHISCALL ReportLine(const std::string& str) = 0;

    /// Error handling. This method is called by the installer if a
    /// problem was detected which can be remedied by the user.
    /// @param lpszMessage Message to be presented to the user.
    /// @returns Returns true, if the user has fixed the problem.
    virtual bool MIKTEXTHISCALL OnRetryableError(const std::string& message) = 0;

    /// Progress. This method is called by the installer if new
    /// progress information is available. Also gives the opportunity
    /// to abort all activities.
    /// @param nf Notification code.
    /// @returns Returns true, if the installer shall continue.
    virtual bool MIKTEXTHISCALL OnProgress(Notification nf) = 0;

    /// Output function. Called by the Process object if new output text
    /// is available.
    /// @param output Output text bytes.
    /// @param n Number of output text bytes.
    /// @return Returns true, of the Process object shall continue.
    virtual bool MIKTEXTHISCALL OnProcessOutput(const void* output, size_t n) = 0;
};

class MIKTEXNOVTABLE SetupService
{

public:

    /// Progress info struct.
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

    virtual MIKTEXTHISCALL ~SetupService() noexcept = 0;

    /// Gets progress information.
    virtual ProgressInfo MIKTEXTHISCALL GetProgressInfo() = 0;

    /// Sets the callback interface.
    /// @param callback Pointer to an interface.
    virtual void MIKTEXTHISCALL SetCallback(SetupServiceCallback* callback) = 0;

    virtual void MIKTEXTHISCALL Initialize() = 0;
    virtual SetupOptions MIKTEXTHISCALL GetOptions() = 0;
    virtual SetupOptions MIKTEXTHISCALL SetOptions(const SetupOptions& options) = 0;
    virtual void MIKTEXTHISCALL OpenLog() = 0;
    virtual MiKTeX::Util::PathName MIKTEXTHISCALL CloseLog(bool cancel) = 0;
    virtual void MIKTEXTHISCALL Log(const std::string& s) = 0;
    virtual void MIKTEXTHISCALL ULogOpen() = 0;
    virtual void MIKTEXTHISCALL ULogClose() = 0;
    virtual MiKTeX::Util::PathName MIKTEXTHISCALL GetULogFileName() = 0;
    virtual void ULogAddFile(const MiKTeX::Util::PathName& path) = 0;
    virtual void MIKTEXTHISCALL SetCallbacks(std::function<void(const std::string&)> f_ReportLine, std::function<bool(const std::string&)> f_OnRetryableError, std::function<bool(MiKTeX::Setup::Notification)> f_OnProgress, std::function<bool(const void*, size_t)> f_OnProcessOutput) = 0;
    virtual void MIKTEXTHISCALL Run() = 0;
    virtual void MIKTEXTHISCALL WriteReport(std::ostream& s, ReportOptionSet options) = 0;
    virtual void MIKTEXTHISCALL WriteReport(std::ostream& s) = 0;
    virtual void MIKTEXTHISCALL CollectDiagnosticInfo(const MiKTeX::Util::PathName& outputFileName) = 0;
    virtual std::vector<Issue> MIKTEXTHISCALL FindIssues(bool checkPath, bool checkPackageIntegrity) = 0;
    virtual std::vector<Issue> MIKTEXTHISCALL GetIssues() = 0;

#if defined(MIKTEX_WINDOWS)
    virtual void MIKTEXTHISCALL CreateProgramIcons() = 0;
    virtual void ULogAddRegValue(HKEY hkey, const std::string& valueName, const std::string& value) = 0;
#endif

    static MIKTEXSETUPCEEAPI(std::unique_ptr<SetupService>) Create();
    static MIKTEXSETUPCEEAPI(std::unique_ptr<MiKTeX::Core::TemporaryDirectory>) CreateSandbox(MiKTeX::Core::StartupConfig& startupConfig);
    static MIKTEXSETUPCEEAPI(MiKTeX::Packages::PackageLevel) TestLocalRepository(const MiKTeX::Util::PathName& pathRepository, MiKTeX::Packages::PackageLevel requestedPackageLevel);
    static MIKTEXSETUPCEEAPI(MiKTeX::Util::PathName) GetDefaultLocalRepository();
    static MIKTEXSETUPCEEAPI(MiKTeX::Packages::PackageLevel) SearchLocalRepository(MiKTeX::Util::PathName& localRepository, MiKTeX::Packages::PackageLevel requestedPackageLevel, bool& prefabricated);
    static MIKTEXSETUPCEEAPI(MiKTeX::Util::PathName) GetDefaultCommonInstallDir();
    static MIKTEXSETUPCEEAPI(MiKTeX::Util::PathName) GetDefaultUserInstallDir();
    static MIKTEXSETUPCEEAPI(MiKTeX::Util::PathName) GetDefaultPortableRoot();
    static MIKTEXSETUPCEEAPI(bool) IsMiKTeXDirect(MiKTeX::Util::PathName& MiKTeXDirectRoot);
    static MIKTEXSETUPCEEAPI(std::unique_ptr<MiKTeX::Core::TemporaryDirectory>) ExtractFiles();
};

MIKTEX_SETUP_END_NAMESPACE;

#undef MIKTEX_SETUP_BEGIN_NAMESPACE
#undef MIKTEX_SETUP_END_NAMESPACE
