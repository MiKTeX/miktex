/* miktex/PackageManager/RepositoryInfo.h:              -*- C++ -*-

   Copyright (C) 2001-2022 Christian Schenk

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

#if !defined(AE9D0FE238A0478AB880D039796623E3)
#define AE9D0FE238A0478AB880D039796623E3

#include "config.h"

#include <ctime>

#include <ostream>
#include <string>

#include "definitions.h"

MIKTEX_PACKAGES_BEGIN_NAMESPACE;

/// Package level.
enum class PackageLevel
{
  None = 0,
  /// Essential package set.
  Essential = 10,
  /// Basic package set.
  Basic = 1000,
  /// Advanced package set.
  Advanced = 100000,
  /// Complete package set.
  Complete = 1000000000,
};

inline std::string PackageLevelToString(PackageLevel packageLevel)
{
  switch (packageLevel)
  {
    case PackageLevel::None:
      return "none";
    case PackageLevel::Essential:
      return "essential";
    case PackageLevel::Basic:
      return "basic";
    case PackageLevel::Advanced:
      return "advanced";
    case PackageLevel::Complete:
      return "complete";
    default:
      return "?";
  }
}

inline std::ostream& operator<<(std::ostream& os, const PackageLevel packageLevel)
{
  return os << PackageLevelToString(packageLevel);
}

/// Repository type.
enum class RepositoryType
{
  Unknown,
  MiKTeXDirect,
  /// Local (file system) directory.
  Local,
  /// Remote (Internet) directory.
  Remote,
  MiKTeXInstallation
};

inline std::string RepositoryTypeToString(RepositoryType repositoryType)
{
  switch (repositoryType)
  {
    case RepositoryType::Unknown:
      return "unknown";
    case RepositoryType::MiKTeXDirect:
      return "miktexdirect";
    case RepositoryType::Local:
      return "local";
    case RepositoryType::Remote:
      return "remote";
    case RepositoryType::MiKTeXInstallation:
      return "miktexinstallation";
    default:
      return "?";
  }  
}

inline std::ostream& operator<<(std::ostream& os, const RepositoryType repositoryType)
{
  return os << RepositoryTypeToString(repositoryType);
}

/// Repository status.
enum class RepositoryStatus
{
  Unknown,
  /// Repository is known to be online.
  Online,
  /// Remote repository is known to be offline.
  Offline,
};

inline std::string RepositoryStatusToString(RepositoryStatus repositoryStatus)
{
  switch (repositoryStatus)
  {
    case RepositoryStatus::Unknown:
      return "unknown";
    case RepositoryStatus::Online:
      return "online";
    case RepositoryStatus::Offline:
      return "offline";
    default:
      return "?";
  }  
}

inline std::ostream& operator<<(std::ostream& os, const RepositoryStatus repositoryStatus)
{
  return os << RepositoryStatusToString(repositoryStatus);
}

/// Repository integrrity.
enum class RepositoryIntegrity
{
  Unknown,
  /// The repository is known to be intact.
  Intact,
  /// The repository is known to be corrupted.
  Corrupted,
};

inline std::string RepositoryIntegrityToString(RepositoryIntegrity repositoryIntegrity)
{
  switch (repositoryIntegrity)
  {
    case RepositoryIntegrity::Unknown:
      return "unknown";
    case RepositoryIntegrity::Intact:
      return "intact";
    case RepositoryIntegrity::Corrupted:
      return "corrupted";
    default:
      return "?";
  }  
}

inline std::ostream& operator<<(std::ostream& os, const RepositoryIntegrity repositoryIntegrity)
{
  return os << RepositoryIntegrityToString(repositoryIntegrity);
}

/// Repository release state.
enum class RepositoryReleaseState
{
  Unknown,
  /// Stable channel.
  Stable,
  /// Experimental channel.
  Next,
};

inline std::string RepositoryReleaseStateToString(RepositoryReleaseState repositoryReleaseState)
{
  switch (repositoryReleaseState)
  {
    case RepositoryReleaseState::Unknown:
      return "unknown";
    case RepositoryReleaseState::Stable:
      return "stable";
    case RepositoryReleaseState::Next:
      return "next";
    default:
      return "?";
  }  
}

inline std::ostream& operator<<(std::ostream& os, const RepositoryReleaseState repositoryReleaseState)
{
  return os << RepositoryReleaseStateToString(repositoryReleaseState);
}

/// Package repository info struct.
struct RepositoryInfo
{
  /// Type of repository.
  RepositoryType type;
  /// URL of (or path to) the repository.
  std::string url;
  /// One-line description.
  std::string description;
  /// Country where the repository is resident.
  std::string country;
  /// Town where the repository is resident.
  std::string town;
  /// Package level.
  PackageLevel packageLevel = PackageLevel::None;
  /// Version number.
  unsigned int version = 0;
  /// Time/Date of last update.
  std::time_t timeDate = InvalidTimeT;
  /// Status of the repository.
  RepositoryStatus status = RepositoryStatus::Unknown;
  /// Integrity of the repository.
  RepositoryIntegrity integrity = RepositoryIntegrity::Unknown;
  /// Delay (in days).
  unsigned delay = 0;
  /// Relative delay (in days).
  unsigned relativeDelay = 0;
  /// Repository release State.
  RepositoryReleaseState releaseState = RepositoryReleaseState::Unknown;
  /// Ranking (low is better than high).
  unsigned ranking = 0;
  /// Data transfer rate (bytes/sec).
  double dataTransferRate = 0.0;
  /// Last check time.
  std::time_t lastCheckTime = InvalidTimeT;
  /// Last visit time.
  std::time_t lastVisitTime = InvalidTimeT;
};

inline bool operator==(const RepositoryInfo& lhs, const RepositoryInfo& rhs)
{
  return lhs.type == rhs.type && lhs.url == rhs.url;
}

inline bool operator!=(const RepositoryInfo& lhs, const RepositoryInfo& rhs)
{
  return !(lhs == rhs);
}

MIKTEX_PACKAGES_END_NAMESPACE;

#endif
