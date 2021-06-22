/* winFileSystemWatcher.cpp: file system watcher (Windows specials)

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

#include "config.h"

#include <vector>

#include <miktex/Core/AutoResource>
#include <miktex/Core/Directory>

#include "internal.h"

#include "winFileSystemWatcher.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

unique_ptr<FileSystemWatcher> FileSystemWatcher::Create()
{
    return make_unique<winFileSystemWatcher>();
}

winFileSystemWatcher::~winFileSystemWatcher()
{
  try
  {
    Stop();
  }
  catch (const exception&)
  {
  }
}

void winFileSystemWatcher::AddDirectory(const MiKTeX::Util::PathName &dir)
{
  lock_guard l(mutex);
  for (auto& d : directories)
  {
    if (d.path == dir)
    {
      return;
    }
  }
  directories.push_back(dir);
  if (restartEvent != nullptr)
  {
    if (!SetEvent(restartEvent))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("SetEvent");
    }
  }
}

void winFileSystemWatcher::Start()
{
  cancelEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
  if (cancelEvent == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CreateEventW");
  }
  restartEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
  if (restartEvent == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CreateEventW");
  }
  StartThreads();
}

void winFileSystemWatcher::Stop()
{
  if (cancelEvent != nullptr)
  {
    if (!SetEvent(cancelEvent))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("SetEvent");
    }
  }
  StopThreads();
  if (cancelEvent != nullptr)
  {
    if (!CloseHandle(cancelEvent))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CloseHandle");
    }
    cancelEvent = nullptr;
  }
  if (restartEvent != nullptr)
  {
    if (!CloseHandle(restartEvent))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CloseHandle");
    }
    restartEvent = nullptr;
  }
}

void winFileSystemWatcher::WatchDirectories()
{
  while (true)
  {
    const DWORD notifyFilter = 0 |
                               FILE_NOTIFY_CHANGE_CREATION |
                               FILE_NOTIFY_CHANGE_DIR_NAME |
                               FILE_NOTIFY_CHANGE_FILE_NAME |
                               FILE_NOTIFY_CHANGE_LAST_WRITE |
                               0;
    vector<HANDLE> handles = { cancelEvent, restartEvent };
    unique_lock l(mutex);
    for (auto& dwi : directories)
    {
      handles.push_back(dwi.overlapped.hEvent);
      if (!dwi.pending)
      {
        if (!ReadDirectoryChangesW(dwi.directoryHandle, dwi.buffer, static_cast<DWORD>(dwi.bufferSize), FALSE, notifyFilter, nullptr, &dwi.overlapped, nullptr))
        {
          MIKTEX_FATAL_WINDOWS_ERROR_2("ReadDirectoryChangesW", "path", dwi.path.ToString());
        }
        dwi.pending = true;
      }
    }
    l.unlock();
    auto ev = WaitForMultipleObjects(static_cast<DWORD>(handles.size()), handles.data(), FALSE, INFINITE);
    if (ev == WAIT_OBJECT_0)
    {
      return;
    }
    else if (ev == WAIT_OBJECT_0 + 1)
    {
      continue;
    }
    else if (ev == WAIT_FAILED)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("WaitForMultipleObjects");
    }
    auto idx = ev - WAIT_OBJECT_0 - 2;
    l.lock();
    auto& dwi = directories[idx];
    dwi.pending = false;
    DWORD bytesReturned = 0;
    if (!GetOverlappedResult(dwi.directoryHandle, &dwi.overlapped, &bytesReturned, TRUE))
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("GetOverlappedResult", "path", dwi.path.ToString());
    }
    if (bytesReturned == 0)
    {
      directories.erase(directories.begin() + idx);
    }
    else
    {
      FILE_NOTIFY_INFORMATION* fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(dwi.buffer);
      l.unlock();
      HandleDirectoryChanges(dwi.path, fni);
      notifyCondition.notify_all();
    }
  }
}

void winFileSystemWatcher::HandleDirectoryChanges(const PathName& dir, const FILE_NOTIFY_INFORMATION* fni)
{
  lock_guard l(notifyMutex);
  while (true)
  {
    HandleDirectoryChange(dir, fni);
    if (fni->NextEntryOffset == 0)
    {
      return;
    }
    fni = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(reinterpret_cast<const unsigned char*>(fni) + fni->NextEntryOffset);
  }
}

void winFileSystemWatcher::HandleDirectoryChange(const PathName& dir, const FILE_NOTIFY_INFORMATION* fni)
{
    FileSystemChangeEvent ev;
    switch (fni->Action)
    {
      case FILE_ACTION_ADDED: ev.action = FileSystemChangeAction::Added; break;
      case FILE_ACTION_MODIFIED: ev.action = FileSystemChangeAction::Modified; break;
      case FILE_ACTION_REMOVED: ev.action = FileSystemChangeAction::Removed; break;
      default:
        return;
    }
    string fileName = StringUtil::WideCharToUTF8(wstring(fni->FileName, fni->FileNameLength));
    ev.fileName = dir;
    ev.fileName /= fileName;
    pendingNotifications.push_back(ev);
}

winFileSystemWatcher::DirectoryWatchInfo::DirectoryWatchInfo(DirectoryWatchInfo&& other)
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

winFileSystemWatcher::DirectoryWatchInfo &winFileSystemWatcher::DirectoryWatchInfo::operator=(winFileSystemWatcher::DirectoryWatchInfo &&other)
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

winFileSystemWatcher::DirectoryWatchInfo::DirectoryWatchInfo(const PathName& path) :
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

winFileSystemWatcher::DirectoryWatchInfo::~DirectoryWatchInfo()
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
