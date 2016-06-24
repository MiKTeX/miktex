/* miktex/PackageManager/PackageInstaller.h:            -*- C++ -*-

   Copyright (C) 2001-2016 Christian Schenk

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

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(E22949C2ACEF4F1DB1F4008919909456)
#define E22949C2ACEF4F1DB1F4008919909456

#include "config.h"

#include <cstddef>
#include <ctime>

#include <string>
#include <vector>

#include <miktex/Core/PathName>

#include "RepositoryInfo.h"

MPM_BEGIN_NAMESPACE;

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
};

/// Callback interface.
class MIKTEXNOVTABLE PackageInstallerCallback
{
  /// Reporting. This method is called by the installer if a new message
  /// is available.
  /// @param str One-line message.
public:
  virtual void MIKTEXTHISCALL ReportLine(const std::string & str) = 0;

  /// Error handling. This method is called by the installer if a
  /// problem was detected which can be remedied by the user.
  /// @param message Message to be presented to the user.
  /// @returns Returns true, if the user has fixed the problem.
public:
  virtual bool MIKTEXTHISCALL OnRetryableError(const std::string & message) = 0;

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
  /// Destructor.
public:
  virtual MIKTEXTHISCALL ~PackageInstaller() = 0;

  /// Releases all resources used by this package installer object.
public:
  virtual void MIKTEXTHISCALL Dispose() = 0;

  /// Sets the package repository for this package installer object.
  /// @param The location of the package repository. Either an URL or
  /// an absolute path name.
public:
  virtual void MIKTEXTHISCALL SetRepository(const std::string & repository) = 0;

  /// Sets the download directory for this package installer.
public:
  virtual void MIKTEXTHISCALL SetDownloadDirectory(const MiKTeX::Core::PathName & directory) = 0;

public:
  virtual void MIKTEXTHISCALL UpdateDb() = 0;

  /// Starts the package database synchronization thread.
public:
  virtual void MIKTEXTHISCALL UpdateDbAsync() = 0;

public:
  virtual void MIKTEXTHISCALL FindUpdates() = 0;

public:
  virtual void MIKTEXTHISCALL FindUpdatesAsync() = 0;

  /// Update info struct.
public:
  struct UpdateInfo
  {
    /// Package deployment name.
    std::string deploymentName;

    /// Date/Time the package was created.
    time_t timePackaged = static_cast<time_t>(0);

    std::string version;

    enum Action {
      None,
      Keep,
      KeepAdmin,
      KeepObsolete,
      Update,
      ForceUpdate, 
      ForceRemove,
      Repair,
      ReleaseStateChange
    };

    Action action = None;
  };

public:
  virtual std::vector<UpdateInfo> MIKTEXTHISCALL GetUpdates() = 0;

public:
  virtual void MIKTEXTHISCALL InstallRemove() = 0;

  /// Starts the installer background thread.
public:
  virtual void MIKTEXTHISCALL InstallRemoveAsync() = 0;

public:
  virtual void MIKTEXTHISCALL WaitForCompletion() = 0;

public:
  virtual void MIKTEXTHISCALL Download() = 0;

  /// Starts the downloader.
public:
  virtual void MIKTEXTHISCALL DownloadAsync() = 0;

  /// Progress info struct.
public:
  struct ProgressInfo
  {
    /// Deployment name of package.
    std::string deploymentName;

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
  virtual void MIKTEXTHISCALL SetCallback(PackageInstallerCallback * callback) = 0;

  /// Sets the package lists.
  /// @param toBeInstalled Packages to be installed.
  /// @param toBeRemoved Packages to be removed.
public:
  virtual void MIKTEXTHISCALL SetFileLists(const std::vector<std::string> & toBeInstalled, const std::vector<std::string> & toBeRemoved) = 0;

  /// Sets the package level.
public:
  virtual void MIKTEXTHISCALL SetPackageLevel(PackageLevel packageLevel) = 0;

  /// Sets the file lists.
  /// @param toBeInstalled Packages to be installed.
public:
  virtual void MIKTEXTHISCALL SetFileList(const std::vector<std::string> & toBeInstalled) = 0;

  /// Tests if the installer is running.
public:
  virtual bool MIKTEXTHISCALL IsRunning() const = 0;

public:
  virtual void MIKTEXTHISCALL RegisterComponents(bool doRegister) = 0;

public:
  virtual void MIKTEXTHISCALL SetNoPostProcessing(bool noPostProcessing) = 0;

public:
  virtual void MIKTEXTHISCALL SetNoLocalServer(bool noLocalServer) = 0;
};

MPM_END_NAMESPACE;

#endif
