/* winFileSystemWatcher.h: file system watcher (Windows specials)

   Copyright (C) 2021 Christian Schenk

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

#include <vector>

#include "../FileSystemWatcherBase.h"

CORE_INTERNAL_BEGIN_NAMESPACE;

class winFileSystemWatcher :
  public FileSystemWatcherBase
{
public:
  virtual MIKTEXTHISCALL ~winFileSystemWatcher();

public:
  void MIKTEXTHISCALL AddDirectory(const MiKTeX::Util::PathName& dir) override;

public:
  void MIKTEXTHISCALL Start() override;

public:
  void MIKTEXTHISCALL Stop() override;

private:
  void MIKTEXTHISCALL WatchDirectories() override;

private:
  void HandleDirectoryChanges(const MiKTeX::Util::PathName& dir, const FILE_NOTIFY_INFORMATION* fni);

private:
  void HandleDirectoryChange(const MiKTeX::Util::PathName& dir, const FILE_NOTIFY_INFORMATION* fni);

private:
  struct DirectoryWatchInfo
  {
    const size_t bufferSize = 1024 * 64;
    DirectoryWatchInfo() = delete;
    DirectoryWatchInfo(const DirectoryWatchInfo& other) = delete;
    DirectoryWatchInfo(DirectoryWatchInfo&& other);
    DirectoryWatchInfo& operator=(const DirectoryWatchInfo& other) = delete;
    DirectoryWatchInfo& operator=(DirectoryWatchInfo&& other);
    DirectoryWatchInfo(const MiKTeX::Util::PathName& path);
    virtual ~DirectoryWatchInfo();
    void* buffer;
    HANDLE directoryHandle;
    OVERLAPPED overlapped;
    MiKTeX::Util::PathName path;
    bool pending;
  };

private:
  HANDLE cancelEvent = nullptr;
  std::vector<DirectoryWatchInfo> directories;
  HANDLE restartEvent = nullptr;
};

CORE_INTERNAL_END_NAMESPACE;
