/* miktex/Core/FileSystemWatcher.h:

   Copyright (C) 2021-2022 Christian Schenk

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

#include <miktex/Core/config.h>

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include <miktex/Util/PathName>

MIKTEX_CORE_BEGIN_NAMESPACE;

enum class FileSystemChangeAction
{
  Added,
  Modified,
  Removed,
};

inline std::ostream& operator<<(std::ostream& os, const FileSystemChangeAction& action)
{
  switch (action)
  {
    case FileSystemChangeAction::Added:
      return os << "added";
    case FileSystemChangeAction::Modified:
      return os << "modified";
    case FileSystemChangeAction::Removed:
      return os << "removed";
    default:
      return os << "?";
  }
}

struct FileSystemChangeEvent
{
  FileSystemChangeAction action;
  MiKTeX::Util::PathName fileName;
};

class MIKTEXNOVTABLE FileSystemWatcherCallback
{
public:
  virtual void MIKTEXTHISCALL OnChange(const FileSystemChangeEvent& ev) = 0;
};

class MIKTEXNOVTABLE FileSystemWatcher
{
public:
  virtual MIKTEXTHISCALL ~FileSystemWatcher() noexcept = 0;

public:
  virtual void MIKTEXTHISCALL AddDirectories(const std::vector<MiKTeX::Util::PathName>& directories) = 0;

public:
  virtual bool MIKTEXTHISCALL Start() = 0;

public:
  virtual bool MIKTEXTHISCALL Stop() = 0;

public:
  virtual void MIKTEXTHISCALL Subscribe(FileSystemWatcherCallback* callback) = 0;

public:
  virtual void MIKTEXTHISCALL Unsubscribe(FileSystemWatcherCallback* callback) = 0;

public:
  static MIKTEXCORECEEAPI(std::unique_ptr<FileSystemWatcher>) Create();
};

MIKTEX_CORE_END_NAMESPACE;
