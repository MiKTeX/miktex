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

unique_ptr<FileSystemWatcher> FileSystemWatcher::Create(FileSystemWatcherCallback* callback)
{
    return make_unique<winFileSystemWatcher>(callback);
}

winFileSystemWatcher::winFileSystemWatcher(FileSystemWatcherCallback* callback) :
  callback(callback),
  failure(false)
{
}

winFileSystemWatcher::~winFileSystemWatcher()
{
  try
  {
    Stop();
    if (cancelEvent != nullptr)
    {
      if (!CloseHandle(cancelEvent))
      {
        MIKTEX_FATAL_WINDOWS_ERROR("CloseHandle");
      }
    }
  }
  catch (const exception&)
  {
  }
}

void winFileSystemWatcher::AddDirectory(const PathName& dir)
{
  directories.push_back(dir);
}

void winFileSystemWatcher::Start()
{
  cancelEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
  if (cancelEvent == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CreateEventW");
  }
  watchDirectoriesThread = std::thread(&winFileSystemWatcher::WatchDirectoriesThreadFunction, this);
}

void winFileSystemWatcher::Stop()
{
  if (watchDirectoriesThread.joinable())
  {
    if (!SetEvent(cancelEvent))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("SetEvent");
    }
    watchDirectoriesThread.join();
  }
  if (failure)
  {
    throw threadMiKTeXException;
  }
}

void winFileSystemWatcher::WatchDirectoriesThreadFunction()
{
  try
  {
    WatchDirectories();
  }
  catch (const MiKTeX::Core::MiKTeXException& e)
  {
    threadMiKTeXException = e;
    failure = true;
  }
  catch (const std::exception& e)
  {
    threadMiKTeXException = MiKTeX::Core::MiKTeXException(e.what());
    failure = true;
  }
}

void winFileSystemWatcher::WatchDirectories()
{
  while (true)
  {
    const DWORD notifyFilter = 0 |
                               FILE_NOTIFY_CHANGE_ATTRIBUTES |
                               FILE_NOTIFY_CHANGE_CREATION |
                               FILE_NOTIFY_CHANGE_DIR_NAME |
                               FILE_NOTIFY_CHANGE_FILE_NAME |
                               FILE_NOTIFY_CHANGE_LAST_ACCESS |
                               FILE_NOTIFY_CHANGE_LAST_WRITE |
                               FILE_NOTIFY_CHANGE_SECURITY |
                               FILE_NOTIFY_CHANGE_SIZE |
                               0;
    vector<HANDLE> handles = { cancelEvent };
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
    auto ev = WaitForMultipleObjects(static_cast<DWORD>(handles.size()), handles.data(), FALSE, INFINITE);
    if (ev == WAIT_OBJECT_0)
    {
      return;
    }
    else if (ev == WAIT_FAILED)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("WaitForMultipleObjects");
    }
    auto idx = ev - WAIT_OBJECT_0 - 1;
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
      HandleDirectoryChanges(dwi.path, reinterpret_cast<FILE_NOTIFY_INFORMATION*>(dwi.buffer));
    }
  }
}

void winFileSystemWatcher::HandleDirectoryChanges(const PathName& dir, const FILE_NOTIFY_INFORMATION* fni)
{
  while (true)
  {
    string fileName = StringUtil::WideCharToUTF8(wstring(fni->FileName, fni->FileNameLength));
    FileSystemChangeEvent ev;
    ev.fileName = dir;
    ev.fileName /= fileName;
    callback->OnChange(ev);
    if (fni->NextEntryOffset == 0)
    {
      return;
    }
    fni = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(reinterpret_cast<const unsigned char*>(fni) + fni->NextEntryOffset);
  }
}
