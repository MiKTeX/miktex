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

unique_ptr<FileSystemWatcher> FileSystemWatcher::Start(const PathName& path, FileSystemWatcherCallback* callback)
{
    return make_unique<winFileSystemWatcher>(path, callback);
}

winFileSystemWatcher::winFileSystemWatcher(const PathName& path, FileSystemWatcherCallback* callback) :
  callback(callback)
{
  if (Directory::Exists(path))
  {
    directory = path;
  }
  else
  {
    directory = path.GetDirectoryName();
    if (directory.Empty() || !Directory::Exists(directory))
    {
      MIKTEX_FATAL_ERROR_2(T_("Directory does not exist."), "path", path.ToString());
    }
    fileName = path.GetFileName().ToString();
  }
  cancelEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
  if (cancelEvent == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateEventW", "path", directory.ToString());
  }
  watchDirectoryThread = std::thread(&winFileSystemWatcher::WatchDirectoryThreadFunction, this);
}

winFileSystemWatcher::~winFileSystemWatcher()
{
  try
  {
    MIKTEX_AUTO(CloseHandle(cancelEvent));
    Stop();
  }
  catch (const exception&)
  {
  }
}

void winFileSystemWatcher::Stop()
{
  if (watchDirectoryThread.joinable())
  {
    if (!SetEvent(cancelEvent))
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("SetEvent", "path", directory.ToString());
    }
    watchDirectoryThread.join();
  }
  if (failure)
  {
    throw threadMiKTeXException;
  }
}

void winFileSystemWatcher::WatchDirectoryThreadFunction()
{
  try
  {
    WatchDirectory();
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

void winFileSystemWatcher::WatchDirectory()
{
  DWORD desiredAccess = FILE_LIST_DIRECTORY;
  DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
  DWORD flagsAndAttributes = FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED;
  HANDLE directoryHandle = CreateFileW(directory.ToWideCharString().c_str(), desiredAccess, shareMode, nullptr, OPEN_EXISTING, flagsAndAttributes, nullptr);
  if (directoryHandle == INVALID_HANDLE_VALUE)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateFileW", "path", directory.ToString());
  }
  MIKTEX_AUTO(CloseHandle(directoryHandle));
  vector<unsigned char> buffer(1024 * 64);
  OVERLAPPED overlapped{0};
  overlapped.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
  if (overlapped.hEvent == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateEventW", "path", directory.ToString());
  }
  MIKTEX_AUTO(CloseHandle(overlapped.hEvent));
  vector<HANDLE> handles = { overlapped.hEvent, cancelEvent };
  bool mustCancelIo = false;
  AutoFunc cleanUp([&](){
    if (mustCancelIo)
    {
      CancelIo(directoryHandle);
      DWORD bytesReturned = 0;
      GetOverlappedResult(directoryHandle, &overlapped, &bytesReturned, TRUE);
    }
  });
  while (true)
  {
    const DWORD notifyFilter = 0 |
                               FILE_NOTIFY_CHANGE_CREATION |
                               FILE_NOTIFY_CHANGE_FILE_NAME |
                               FILE_NOTIFY_CHANGE_LAST_WRITE |
                               0;
    DWORD bytesReturned = 0;
    BOOL done = ReadDirectoryChangesW(directoryHandle, &buffer[0], static_cast<DWORD>(buffer.size()), FALSE, notifyFilter, &bytesReturned, &overlapped, nullptr);
    if (!done)
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("ReadDirectoryChangesW", "path", directory.ToString());
    }
    mustCancelIo = true;
    switch (WaitForMultipleObjects(handles.size(), &handles[0], FALSE, INFINITE))
    {
      case WAIT_OBJECT_0:
        mustCancelIo = false;
        if (!GetOverlappedResult(directoryHandle, &overlapped, &bytesReturned, TRUE))
        {
          MIKTEX_FATAL_WINDOWS_ERROR_2("GetOverlappedResult", "path", directory.ToString());
        }
        if (bytesReturned == 0)
        {
          return;
        }
        HandleDirectoryChanges(reinterpret_cast<FILE_NOTIFY_INFORMATION*>(&buffer[0]));
        break;
      case WAIT_OBJECT_0 + 1:
        return;
      case WAIT_FAILED:
        MIKTEX_FATAL_WINDOWS_ERROR_2("WaitForMultipleObjects", "path", directory.ToString());
    }
  }
}

void winFileSystemWatcher::HandleDirectoryChanges(const FILE_NOTIFY_INFORMATION* fni)
{
  while (true)
  {
    string fileName = StringUtil::WideCharToUTF8(wstring(fni->FileName, fni->FileNameLength));
    if (this->fileName.empty() || PathName::Compare(this->fileName, fileName) == 0)
    {
      FileSystemChangeEvent ev;
      ev.fileName = fileName;
      callback->OnChange(ev);
    }
    if (fni->NextEntryOffset == 0)
    {
      return;
    }
    fni = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(reinterpret_cast<const unsigned char*>(fni) + fni->NextEntryOffset);
  }
}
