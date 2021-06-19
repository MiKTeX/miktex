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

#include <thread>
#include <vector>

#include <miktex/Core/FileSystemWatcher>

CORE_INTERNAL_BEGIN_NAMESPACE;

class winFileSystemWatcher :
  public MiKTeX::Core::FileSystemWatcher
{
public:
  void MIKTEXTHISCALL AddDirectory(const MiKTeX::Util::PathName& dir) override;

public:
  void MIKTEXTHISCALL Start() override;

public:
  void MIKTEXTHISCALL Stop() override;

public:
  winFileSystemWatcher(MiKTeX::Core::FileSystemWatcherCallback* callback);

public:
  virtual MIKTEXTHISCALL ~winFileSystemWatcher();

private:
  void WatchDirectoriesThreadFunction();

private:
  void WatchDirectories();

private:
  void HandleDirectoryChanges(const MiKTeX::Util::PathName& dir, const FILE_NOTIFY_INFORMATION* fni);

private:
  struct DirectoryWatchInfo
  {
    const size_t bufferSize = 1024 * 64;
    DirectoryWatchInfo() = delete;
    DirectoryWatchInfo(const DirectoryWatchInfo& other) = delete;
    DirectoryWatchInfo(DirectoryWatchInfo&& other)
    {
      buffer = other.buffer;
      other.buffer = nullptr;
      directoryHandle = other.directoryHandle;
      other.directoryHandle = INVALID_HANDLE_VALUE;
      overlapped = other.overlapped;
      other.overlapped.hEvent = nullptr;
      path = std::move(other.path);
      pending = other.pending;
      other.pending = false;
    }
    DirectoryWatchInfo& operator=(const DirectoryWatchInfo& other) = delete;
    DirectoryWatchInfo& operator=(DirectoryWatchInfo&& other)
    {
      if (this != &other)
      {
        buffer = other.buffer;
        other.buffer = nullptr;
        directoryHandle = other.directoryHandle;
        other.directoryHandle = INVALID_HANDLE_VALUE;
        overlapped = other.overlapped;
        other.overlapped.hEvent = nullptr;
        path = std::move(other.path);
        pending = other.pending;
        other.pending = false;
      }
      return *this;
    }
    DirectoryWatchInfo(const MiKTeX::Util::PathName& path) :
      buffer(malloc(bufferSize)),
      path(path),
      pending(false)
    {
      memset(&overlapped, 0, sizeof(overlapped));
      overlapped.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
      if (overlapped.hEvent == nullptr)
      {
        MIKTEX_FATAL_WINDOWS_ERROR("CreateEventW");
      }
      DWORD desiredAccess = FILE_LIST_DIRECTORY;
      DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
      DWORD flagsAndAttributes = FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED;
      directoryHandle = CreateFileW(path.ToWideCharString().c_str(), desiredAccess, shareMode, nullptr, OPEN_EXISTING, flagsAndAttributes, nullptr);
      if (directoryHandle == INVALID_HANDLE_VALUE)
      {
        CloseHandle(overlapped.hEvent);
        MIKTEX_FATAL_WINDOWS_ERROR_2("CreateFileW", "path", path.ToString());
      }
    }
    virtual ~DirectoryWatchInfo()
    {
      if (pending)
      {
        CancelIo(directoryHandle);
        DWORD bytesReturned = 0;
        GetOverlappedResult(directoryHandle, &overlapped, &bytesReturned, TRUE);
      }
      if (directoryHandle != INVALID_HANDLE_VALUE)
      {
        CloseHandle(directoryHandle);
      }
      if (overlapped.hEvent != nullptr)
      {
        CloseHandle(overlapped.hEvent);
      }
      if (buffer != nullptr)
      {
        free(buffer);
      }
    }
    void* buffer;
    HANDLE directoryHandle;
    OVERLAPPED overlapped;
    MiKTeX::Util::PathName path;
    bool pending;
  };

private:
  std::vector<DirectoryWatchInfo> directories;

private:
  MiKTeX::Core::FileSystemWatcherCallback* callback;

private:
  HANDLE cancelEvent;

private:
  std::thread watchDirectoriesThread;

private:
  MiKTeX::Core::MiKTeXException threadMiKTeXException;

private:
  bool failure;  
};

CORE_INTERNAL_END_NAMESPACE;
