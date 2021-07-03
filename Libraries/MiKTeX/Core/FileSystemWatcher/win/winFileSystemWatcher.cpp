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

#include <fmt/format.h>
#include <fmt/ostream.h>

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

void winFileSystemWatcher::AddDirectories(const vector<PathName>& directories)
{
  bool wasRunning = Stop();
  vector<PathName> newDirectories;
  unique_lock l(mutex);
  for (const auto& dir : directories)
  {
    bool dup = false;
    for (auto& d : this->directories)
      if (d.path == dir)
      {
        dup = true;
        break;
      }
      if (!dup)
      {
        newDirectories.push_back(dir);
      }
  }
  for (const auto& dir : newDirectories)
  {
    trace_files->WriteLine("core", fmt::format("watching directory: {0}", dir));
    this->directories.push_back(dir);
  }
  l.unlock();
  if (wasRunning)
  {
    Start();
  }
}

bool winFileSystemWatcher::Start()
{
  bool runningExpected = false;
  if (!running.compare_exchange_strong(runningExpected, true))
  {
    return false;
  }
  cancelEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
  if (cancelEvent == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CreateEventW");
  }
  StartThreads();
  return true;
}

bool winFileSystemWatcher::Stop()
{
  bool runningExpected = true;
  if (!running.compare_exchange_strong(runningExpected, false))
  {
    return false;
  }
  if (!SetEvent(cancelEvent))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("SetEvent");
  }
  StopThreads();
  if (!CloseHandle(cancelEvent))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CloseHandle");
  }
  unique_lock l(mutex);
  for (auto& d : this->directories)
  {
    if (d.pending)
    {
      d.CancelIo();
    }
  }
  l.unlock();
  return true;
}

void winFileSystemWatcher::WatchDirectories()
{
  while (true)
  {
    const DWORD notifyFilter = 0 |
                               FILE_NOTIFY_CHANGE_DIR_NAME |
                               FILE_NOTIFY_CHANGE_FILE_NAME |
                               FILE_NOTIFY_CHANGE_LAST_WRITE |
                               0;
    vector<HANDLE> handles = {cancelEvent};
    unique_lock l(mutex);
    for (auto& dwi : directories)
    {
      handles.push_back(dwi.overlapped->hEvent);
      if (!dwi.pending)
      {
        if (!ReadDirectoryChangesW(dwi.directoryHandle, dwi.buffer, static_cast<DWORD>(dwi.bufferSize), FALSE, notifyFilter, nullptr, dwi.overlapped, nullptr))
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
      trace_files->WriteLine("core", fmt::format("cancaling directory watch loop"));
      return;
    }
    else if (ev == WAIT_FAILED)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("WaitForMultipleObjects");
    }
    auto idx = ev - WAIT_OBJECT_0 - 1;
    l.lock();
    auto& dwi = directories[idx];
    dwi.pending = false;
    DWORD bytesReturned = 0;
    if (!GetOverlappedResult(dwi.directoryHandle, dwi.overlapped, &bytesReturned, TRUE))
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("GetOverlappedResult", "path", dwi.path.ToString());
    }
    if (bytesReturned == 0)
    {
      trace_error->WriteLine("core", MiKTeX::Trace::TraceLevel::Error, fmt::format("event buffer overflow while watching: {0}", dwi.path));
      continue;
    }
    FILE_NOTIFY_INFORMATION *fni = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(dwi.buffer);
    l.unlock();
    HandleDirectoryChanges(dwi.path, fni);
    notifyCondition.notify_all();
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
  case FILE_ACTION_ADDED:
    ev.action = FileSystemChangeAction::Added;
    break;
  case FILE_ACTION_MODIFIED:
    ev.action = FileSystemChangeAction::Modified;
    break;
  case FILE_ACTION_REMOVED:
    ev.action = FileSystemChangeAction::Removed;
    break;
  default:
    return;
  }
  string fileName = StringUtil::WideCharToUTF8(wstring(fni->FileName, fni->FileNameLength / sizeof(WCHAR)));
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
  other.overlapped = nullptr;
  path = std::move(other.path);
  pending = other.pending;
  other.pending = false;
}

winFileSystemWatcher::DirectoryWatchInfo& winFileSystemWatcher::DirectoryWatchInfo::operator=(winFileSystemWatcher::DirectoryWatchInfo&& other)
{
  if (this != &other)
  {
    buffer = other.buffer;
    other.buffer = nullptr;
    directoryHandle = other.directoryHandle;
    other.directoryHandle = INVALID_HANDLE_VALUE;
    overlapped = other.overlapped;
    other.overlapped = nullptr;
    path = std::move(other.path);
    pending = other.pending;
    other.pending = false;
  }
  return *this;
}

winFileSystemWatcher::DirectoryWatchInfo::DirectoryWatchInfo(const PathName& path)
  : buffer(malloc(bufferSize)),
    path(path),
    pending(false)
{
  overlapped = new OVERLAPPED;
  memset(overlapped, 0, sizeof(*overlapped));
  overlapped->hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
  if (overlapped->hEvent == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CreateEventW");
  }
  DWORD desiredAccess = FILE_LIST_DIRECTORY;
  DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
  DWORD flagsAndAttributes = FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED;
  directoryHandle = CreateFileW(path.ToWideCharString().c_str(), desiredAccess, shareMode, nullptr, OPEN_EXISTING, flagsAndAttributes, nullptr);
  if (directoryHandle == INVALID_HANDLE_VALUE)
  {
    CloseHandle(overlapped->hEvent);
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateFileW", "path", path.ToString());
  }
}

void winFileSystemWatcher::DirectoryWatchInfo::CancelIo()
{
  if (pending)
  {
    if (!::CancelIo(directoryHandle))
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("CancelIo", "path", path.ToString());
    }
    DWORD bytesReturned = 0;
    GetOverlappedResult(directoryHandle, overlapped, &bytesReturned, TRUE);
    pending = false;
  }
}

winFileSystemWatcher::DirectoryWatchInfo::~DirectoryWatchInfo()
{
  CancelIo();
  if (directoryHandle != INVALID_HANDLE_VALUE)
  {
    CloseHandle(directoryHandle);
    directoryHandle = nullptr;
  }
  if (overlapped != nullptr && overlapped->hEvent != nullptr)
  {
    CloseHandle(overlapped->hEvent);
    overlapped->hEvent = nullptr;
  }
  if (overlapped != nullptr)
  {
    delete overlapped;
    overlapped = nullptr;
  }
  if (buffer != nullptr)
  {
    free(buffer);
    buffer = nullptr;
  }
}
