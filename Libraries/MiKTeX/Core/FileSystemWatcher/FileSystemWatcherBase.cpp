/* FileSystemWatcherBase.cpp: file system watcher (base implementation)

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

#include "FileSystemWatcherBase.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

void FileSystemWatcherBase::Subscribe(MiKTeX::Core::FileSystemWatcherCallback *callback)
{
  lock_guard<shared_mutex> l(mutex);
  callbacks.insert(callback);
}

void FileSystemWatcherBase::Unsubscribe(MiKTeX::Core::FileSystemWatcherCallback *callback)
{
  lock_guard<shared_mutex> l(mutex);
  auto it = callbacks.find(callback);
  if (it != callbacks.end())
  {
    callbacks.erase(it);
  }
}

void FileSystemWatcherBase::StartThreads()
{
  done = false;
  notifyThread = std::thread(&FileSystemWatcherBase::NotifyThreadFunction, this);
  watchDirectoriesThread = std::thread(&FileSystemWatcherBase::WatchDirectoriesThreadFunction, this);
}

void FileSystemWatcherBase::StopThreads()
{
  done = true;
  notifyCondition.notify_all();
  if (notifyThread.joinable())
  {
    notifyThread.join();
  }
  if (watchDirectoriesThread.joinable())
  {
    watchDirectoriesThread.join();
  }
  if (failure)
  {
    throw threadMiKTeXException;
  }
}

void FileSystemWatcherBase::NotifyThreadFunction()
{
  try
  {
    NotifySubscribers();
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

void FileSystemWatcherBase::WatchDirectoriesThreadFunction()
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

void FileSystemWatcherBase::NotifySubscribers()
{
  while (!done)
  {
    unique_lock<std::mutex> l(notifyMutex);
    notifyCondition.wait(l, [this] { return done || !pendingNotifications.empty(); });
    vector<FileSystemChangeEvent> notifications;
    std::swap(notifications, pendingNotifications);
    l.unlock();
    for (const auto &ev : notifications)
    {
      for (auto &c : callbacks)
      {
        c->OnChange(ev);
      }
    }
  }
}
