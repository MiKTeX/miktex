/* miktex/PackageManager/PackageInstaller.h:            -*- C++ -*-

   Copyright (C) 2001-2020 Christian Schenk

   This file is part of MiKTeX Package Manager.

   MiKTeX Package Manager is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Package Manager is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Package Manager; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#if !defined(E22949C2ACEF4F1DB1F4008919909456)
#define E22949C2ACEF4F1DB1F4008919909456

#include "config.h"

#include <cstddef>
#include <ctime>

#include <string>
#include <vector>

#include <miktex/Core/OptionSet>
#include <miktex/Core/PathName>

#include "RepositoryInfo.h"
#include "definitions.h"

MIKTEX_PACKAGES_BEGIN_NAMESPACE;

/// UpdateDb options.
enum class UpdateDbOption
{
  FromCache,
};

typedef MiKTeX::Core::OptionSet<UpdateDbOption> UpdateDbOptionSet;

/// Installer notifications.
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
};

/// Callback interface.
class MIKTEXNOVTABLE PackageInstallerCallback
{
  /// Reporting. This method is called by the installer if a new message
  /// is available.
  /// @param str One-line message.
public:
  virtual void MIKTEXTHISCALL ReportLine(const std::string& str) = 0;

  /// Error handling. This method is called by the installer if a
  /// problem was detected which can be remedied by the user.
  /// @param message Message to be presented to the user.
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
};

class MIKTEXNOVTABLE PackageInstaller
{
public:
  virtual MIKTEXTHISCALL ~PackageInstaller() noexcept = 0;

  /// Releases all resources used by this package installer object.
public:
  virtual void MIKTEXTHISCALL Dispose() = 0;

  /// Sets the package repository for this package installer object.
  /// @param repository The location of the package repository. Either an URL or
  /// an absolute path name.
public:
  virtual void MIKTEXTHISCALL SetRepository(const std::string& repository) = 0;

  /// Sets the download directory for this package installer.
  /// @param directory The path to the download directory.
public:
  virtual void MIKTEXTHISCALL SetDownloadDirectory(const MiKTeX::Core::PathName& directory) = 0;

  /// Synchronizes the package database.
public:
  virtual void MIKTEXTHISCALL UpdateDb(UpdateDbOptionSet options) = 0;

  /// Starts the package database synchronization thread.
  /// @see WaitForCompletion
public:
  virtual void MIKTEXTHISCALL UpdateDbAsync() = 0;

  /// Checks for updates.
public:
  virtual void MIKTEXTHISCALL FindUpdates() = 0;

  /// Checks for updates in a secondary thread.
  /// @see WaitForCompletion
public:
  virtual void MIKTEXTHISCALL FindUpdatesAsync() = 0;

  /// Update information record.
public:
  struct UpdateInfo
  {
    /// Package ID.
    std::string packageId;

    /// Date/Time the package was created.
    std::time_t timePackaged = InvalidTimeT;

    /// Informal version informtion.
    std::string version;

    /// Update actions.
    enum Action {
      None,
      /// Unused
      /// @todo: remove
      Keep,
      /// No permission to remove.
      KeepAdmin,
      /// Should be removed.
      KeepObsolete,
      /// Optional update.
      Update,
      /// Required update.
      ForceUpdate, 
      /// Required deletion.
      ForceRemove,
      /// Package has been tampered with.
      Repair,
      /// Release channel has changed.
      ReleaseStateChange
    };

    /// The action to take.
    Action action = None;
  };

  /// Gets updateable packages.
  /// @return Returns collected update information records.
public:
  virtual std::vector<UpdateInfo> MIKTEXTHISCALL GetUpdates() = 0;

  /// Checks for upgradeable packages.
  /// @param packageLevel The requested package level.
public:
  virtual void MIKTEXTHISCALL FindUpgrades(PackageLevel packageLevel) = 0;

  /// Checks for upgrades in a secondary thread.
  /// @see WaitForCompletion
public:
  virtual void MIKTEXTHISCALL FindUpgradesAsync(PackageLevel packageLevel) = 0;

  /// Upgrade information record.
public:
  struct UpgradeInfo
  {
    /// ID of an upgradaeble package.
    std::string packageId;
    /// Packaging timestamp.
    std::time_t timePackaged = InvalidTimeT;
    /// Informal version information.
    std::string version;
  };

  /// Gets upgradeable packages.
  /// @return Returns upgrade information records.
public:
  virtual std::vector<UpgradeInfo> MIKTEXTHISCALL GetUpgrades() = 0;

  /// Installation role.
public:
  enum class Role
  {
    /// Regular application.
    Application,
    /// Installer (setup) program.
    Installer,
    /// Updater.
    Updater
  };

  /// Installs/removes packages.
  /// @param role The installation role.
public:
  virtual void MIKTEXTHISCALL InstallRemove(Role role) = 0;

  /// Installs/removes packages in a secondary thread.
  /// @param role The installation role.
  /// @see WaitForCompletion
public:
  virtual void MIKTEXTHISCALL InstallRemoveAsync(Role role) = 0;

  /// Waits for the started thread to complete.
public:
  virtual void MIKTEXTHISCALL WaitForCompletion() = 0;

  /// Downloads packages.
  /// @see SetDownloadDirectory
  /// @see SetPackageLevel
public:
  virtual void MIKTEXTHISCALL Download() = 0;

  /// Downloads packages in a secondary thread.
  /// @see SetDownloadDirectory
  /// @see SetPackageLevel
  /// @see WaitForCompletion
public:
  virtual void MIKTEXTHISCALL DownloadAsync() = 0;

  /// Progress information.
public:
  struct ProgressInfo
  {
    /// ID of the current package.
    std::string packageId;

    /// Display name of package.
    std::string displayName;

    /// Path name of current file.
    MiKTeX::Core::PathName fileName;

    /// Number of removed files.
    unsigned long cFilesRemoveCompleted = 0;

    /// Number of files to be removed.
    unsigned long cFilesRemoveTotal = 0;

    /// Number of removed packages.
    unsigned long cPackagesRemoveCompleted = 0;

    /// Number of packages to be removed.
    unsigned long cPackagesRemoveTotal = 0;

    /// Number of received bytes (current package).
    std::size_t cbPackageDownloadCompleted = 0;

    /// Number of bytes to be received (current package).
    std::size_t cbPackageDownloadTotal = 0;

    /// Number of bytes received.
    std::size_t cbDownloadCompleted = 0;
    
    /// Number of bytes to be received.
    std::size_t cbDownloadTotal = 0;

    /// Number of installed files (current package).
    std::size_t cFilesPackageInstallCompleted = 0;

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
    std::size_t cbPackageInstallCompleted = 0;

    /// Number of bytes to be written (current package).
    std::size_t cbPackageInstallTotal = 0;

    /// Number of bytes written.
    std::size_t cbInstallCompleted = 0;

    /// Number of bytes to be written.
    std::size_t cbInstallTotal = 0;

    /// Current transfer speed.
    unsigned long bytesPerSecond = 0;

    /// Estimated time of arrival (millisecs remaining).
    unsigned long timeRemaining = 0;
    
    /// Ready flag.
    bool ready = false;

    /// Number of errors.
    unsigned numErrors = 0;

    /// Indicates whether the current operation has been cancelled.
    bool cancelled = false;
  };

  /// Gets progress information.
  /// @return Returns progress information.
public:
  virtual ProgressInfo MIKTEXTHISCALL GetProgressInfo() = 0;

  /// Sets the callback interface.
  /// @param callback Pointer to object implementing the interface.
public:
  virtual void MIKTEXTHISCALL SetCallback(PackageInstallerCallback* callback) = 0;

  /// Sets the package lists.
  /// @param toBeInstalled Packages to be installed.
  /// @param toBeRemoved Packages to be removed.
public:
  virtual void MIKTEXTHISCALL SetFileLists(const std::vector<std::string>& toBeInstalled, const std::vector<std::string>& toBeRemoved) = 0;

  /// Sets the package level.
  /// @param packageLevel The package level to set.
public:
  virtual void MIKTEXTHISCALL SetPackageLevel(PackageLevel packageLevel) = 0;

  /// Sets the package lists.
  /// @param toBeInstalled Packages to be installed.
public:
  virtual void MIKTEXTHISCALL SetFileList(const std::vector<std::string>& toBeInstalled) = 0;

  /// Tests if the secondary thread is running.
  /// @return Returns `true`, if the installer is running.
public:
  virtual bool MIKTEXTHISCALL IsRunning() const = 0;

  /// Performs post-processing tasks.
  /// @param postInstall Indicates whether this is post-install or post-uninstall.
public:
  virtual void MIKTEXTHISCALL RegisterComponents(bool postInstall) = 0;

  /// Initialization options.
public:
  struct InitInfo
  {
    InitInfo() = default;
    InitInfo(const InitInfo& other) = default;
    InitInfo& operator=(const InitInfo& other) = default;
    InitInfo(InitInfo&& other) = default;
    InitInfo& operator=(InitInfo&& other) = default;
    ~InitInfo() = default;
    /// Pointer to an object implementing the callback interface.
    PackageInstallerCallback* callback = nullptr;
    /// @brief Indicates whether to operate in unattended mode.
    ///
    /// Setting this value to `true` will disable the Windows elevation prompt (UAC).
    bool unattended = false;
    /// Indicates whether to enable or disable post-processing.
    bool enablePostProcessing = true;
  };
};

MIKTEX_PACKAGES_END_NAMESPACE;

#endif
