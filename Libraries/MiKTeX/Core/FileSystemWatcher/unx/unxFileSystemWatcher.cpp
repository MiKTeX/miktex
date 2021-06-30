/* unxFileSystemWatcher.cpp: file system watcher (Unx specials)

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

#include <unistd.h>
#include <sys/inotify.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "internal.h"

#include "unxFileSystemWatcher.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

unique_ptr<FileSystemWatcher> FileSystemWatcher::Create()
{
  return make_unique<unxFileSystemWatcher>();
}

unxFileSystemWatcher::~unxFileSystemWatcher()
{
  try
  {
    Stop();
  }
  catch (const exception&)
  {
  }
}

void unxFileSystemWatcher::AddDirectory(const MiKTeX::Util::PathName& dir)
{
  lock_guard l(mutex);
  for (auto& d : directories)
  {
    if (d.second == dir)
    {
      return;
    }
  }
  if (watchFd < 0)
  {
    MIKTEX_FATAL_ERROR("File system watcher is not running.");
  }
  int wd = inotify_add_watch(watchFd, dir.GetData(), IN_ALL_EVENTS);
  if (wd < 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("inotify_add_Watch", "path", dir.ToString());
  }
  trace_files->WriteLine("core", fmt::format("watching directory: {0}", dir));
  directories[wd] = dir;
  if (restartEventPipe[1] >= 0)
  {
    char buf[1];
    if (write(restartEventPipe[1], buf, 1) < 0)
    {
      MIKTEX_FATAL_CRT_ERROR("write");
    }
  }
}

void unxFileSystemWatcher::Start()
{
  watchFd = inotify_init();
  if (watchFd < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("inotify_init");
  }
  if (pipe(cancelEventPipe) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("pipe");
  }
  if (pipe(restartEventPipe) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("pipe");
  }
  StartThreads();
}

void unxFileSystemWatcher::Stop()
{
  if (cancelEventPipe[1] >= 0)
  {
    char buf[1];
    if (write(cancelEventPipe[1], buf, 1) < 0)
    {
      MIKTEX_FATAL_CRT_ERROR("write");
    }
  }
  StopThreads();
  if (watchFd >= 0)
  {
    if (close(watchFd) < 0)
    {
      MIKTEX_FATAL_CRT_ERROR("close");
    }
    watchFd = -1;
  }
  if (cancelEventPipe[1] >= 0)
  {
    if (close(cancelEventPipe[0]) < 0)
    {
      MIKTEX_FATAL_CRT_ERROR("clode");
    }
    if (close(cancelEventPipe[1]) < 0)
    {
      MIKTEX_FATAL_CRT_ERROR("clode");
    }
  }
  if (restartEventPipe[1] >= 0)
  {
    if (close(restartEventPipe[0]) < 0)
    {
      MIKTEX_FATAL_CRT_ERROR("clode");
    }
    if (close(restartEventPipe[1]) < 0)
    {
      MIKTEX_FATAL_CRT_ERROR("clode");
    }
  }
}

void unxFileSystemWatcher::WatchDirectories()
{
  vector<unsigned char> buffer;
  buffer.reserve(4096);
  while (true)
  {
    int maxFd = -1;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(cancelEventPipe[0], &readfds);
    if (cancelEventPipe[0] > maxFd)
    {
      maxFd = cancelEventPipe[0];
    }
    FD_SET(restartEventPipe[0], &readfds);
    if (restartEventPipe[0] > maxFd)
    {
      maxFd = restartEventPipe[0];
    }
    FD_SET(watchFd, &readfds);
    if (watchFd > maxFd)
    {
      maxFd = watchFd;
    }
    if (select(maxFd + 1, &readfds, nullptr, nullptr, nullptr) < 0)
    {
      MIKTEX_FATAL_CRT_ERROR("select");
    }
    if (FD_ISSET(watchFd, &readfds))
    {
      auto n = read(watchFd, &buffer[0], buffer.size());
      if (n < 0)
      {
        MIKTEX_FATAL_CRT_ERROR("read");
      }
      for (size_t idx = 0; idx < n;)
      {
	const struct inotify_event* evt = reinterpret_cast<const struct inotify_event*>(&buffer[idx]);
	HandleDirectoryChange(evt);
	idx += sizeof(struct inotify_event) + evt->len;
      }
      notifyCondition.notify_all();
    }
    if (FD_ISSET(cancelEventPipe[0], &readfds))
    {
      return;
    }
    if (FD_ISSET(restartEventPipe[0], &readfds))
    {
      if (read(restartEventPipe[0], &buffer[0], buffer.size()) < 0)
      {
	MIKTEX_FATAL_CRT_ERROR("read");
      }
    }
  }
}

void unxFileSystemWatcher::HandleDirectoryChange(const inotify_event* evt)
{
  // TODO
}
