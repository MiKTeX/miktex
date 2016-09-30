/* miktex/PackageManager/RepositoryInfo.h:              -*- C++ -*-

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

#if !defined(AE9D0FE238A0478AB880D039796623E3)
#define AE9D0FE238A0478AB880D039796623E3

#include "config.h"

#include <ctime>

#include <string>

MPM_BEGIN_NAMESPACE;

/// Package level enum class.
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

enum class RepositoryType
{
  Unknown,
  MiKTeXDirect,
  Local,
  Remote,
  MiKTeXInstallation
};
  
enum class RepositoryStatus
{
  Unknown,
  Online,
  Offline,
};

enum class RepositoryIntegrity
{
  Unknown,
  Intact,
  Corrupted,
};

/// Repository release state.
enum class RepositoryReleaseState
{
  Unknown,
  Stable,
  Next,
};

/// Package repository info struct.
struct RepositoryInfo
{
  /// URL of the repository.
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
  time_t timeDate = static_cast<time_t>(0);
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
};

MPM_END_NAMESPACE;

#endif
