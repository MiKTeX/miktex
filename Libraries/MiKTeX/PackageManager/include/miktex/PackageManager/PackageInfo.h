/* miktex/PackageManager/PackageInfo.h:                 -*- C++ -*-

   Copyright (C) 2001-2019 Christian Schenk

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

#if !defined(C47620F83A5C47D2B408E1D3C4EF0E48)
#define C47620F83A5C47D2B408E1D3C4EF0E48

#include "config.h"

#include <cstddef>
#include <ctime>

#include <string>
#include <vector>

#include <miktex/Core/MD5>

#include "definitions.h"
#include "RepositoryInfo.h"

MIKTEX_PACKAGES_BEGIN_NAMESPACE;

/// Package record.
struct PackageInfo
{
  /// Package ID.
  std::string id;

  /// Display name.
  std::string displayName;

  /// One-line package description.
  std::string title;

  /// Informal version information.
  std::string version;

  /// Target system.
  std::string targetSystem;

  /// Multi-line package description.
  std::string description;

  /// Name of creator (packer).
  std::string creator;

  /// Accumulated file sizes of the run file list.
  std::size_t sizeRunFiles = 0;

  /// Accumulated file sizes of the doc file list.
  std::size_t sizeDocFiles = 0;

  /// Accumulated file sizes of the source file list.
  std::size_t sizeSourceFiles = 0;

  /// The run file list.
  std::vector<std::string> runFiles;

  /// The doc file list.
  std::vector<std::string> docFiles;

  /// The source file list.
  std::vector<std::string> sourceFiles;

  /// List of required packages.
  std::vector<std::string> requiredPackages;

  /// List of dependants.
  std::vector<std::string> requiredBy;

  /// Date/time when the package was created.
  std::time_t timePackaged = InvalidTimeT;

  /// Date/time when the package was installed.
  std::time_t timeInstalled = InvalidTimeT;

  /// Date/time when the package was installed by the user.
  std::time_t timeInstalledByUser = InvalidTimeT;

  /// Date/time when the package was installed by the administrator (for all users).
  std::time_t timeInstalledByAdmin = InvalidTimeT;

  /// Size of the archive file.
  std::size_t archiveFileSize = 0;

  /// MD5 of the package.
  MiKTeX::Core::MD5 digest;

  /// `true`, if the package can be removed.
  bool isRemovable = false;

  /// `true`, if the package is obsolete.
  bool isObsolete = false;

  /// The release state of the package.
  RepositoryReleaseState releaseState = RepositoryReleaseState::Unknown;

  /// Relative path to the package directory on a CTAN mirror.
  std::string ctanPath;

  /// The license type of the package.
  std::string licenseType;

  /// The copyright owner of the package.
  std::string copyrightOwner;

  /// Copyright year
  std::string copyrightYear;

  /// The version/date of the package.
  std::string versionDate;

  /// Gets the total number of files in the package.
  /// @return Returns the number of files.
  unsigned long GetNumFiles() const
  {
    return static_cast<unsigned long>(runFiles.size() + docFiles.size() + sourceFiles.size());
  }

  /// Gets the total size (in bytes) of all files in the package.
  /// @return Returns the package size.
  std::size_t GetSize() const
  {
    return sizeRunFiles + sizeDocFiles + sizeSourceFiles;
  }

  /// Tests for a pure container package.
  /// @return Returns true, if this is a pure container package.
  bool IsPureContainer() const
  {
    return IsContainer() && GetNumFiles() <= 1;
  }

  /// Tests for a container package.
  bool IsContainer() const
  {
    return !requiredPackages.empty();
  }

  /// Checks to see if this package has dependants.
  bool IsContained() const
  {
    return GetRefCount() > 0;
  }

  /// Checks to see whether the package is installed.
  bool IsInstalled() const
  {
    return IsValidTimeT(timeInstalled);
  }

  /// Gets the number of dependants.
  unsigned long GetRefCount() const
  {
    return static_cast<unsigned long>(requiredBy.size());
  }
};

inline bool operator==(const PackageInfo& lhs, const PackageInfo& rhs)
{
  return
    lhs.id == rhs.id
    && lhs.digest == rhs.digest
    && lhs.displayName == rhs.displayName
    && lhs.title == rhs.title
    && lhs.version == rhs.version
    && lhs.targetSystem == rhs.targetSystem
    && lhs.description == rhs.description
    && lhs.creator == rhs.creator
    && lhs.sizeRunFiles == rhs.sizeRunFiles
    && lhs.sizeDocFiles == rhs.sizeDocFiles
    && lhs.sizeSourceFiles == rhs.sizeSourceFiles
    && lhs.requiredPackages == rhs.requiredPackages
    && lhs.timePackaged == rhs.timePackaged
    && lhs.ctanPath == rhs.ctanPath
    && lhs.licenseType == rhs.licenseType
    && lhs.copyrightOwner == rhs.copyrightOwner
    && lhs.copyrightYear == rhs.copyrightYear
    && lhs.versionDate == rhs.versionDate
    ;
}

MIKTEX_PACKAGES_END_NAMESPACE;

#endif
