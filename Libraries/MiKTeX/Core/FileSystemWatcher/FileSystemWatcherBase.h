/* FileSystemWatcherBase.h: file system watcher (base implementation)

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

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <set>
#include <shared_mutex>

#include <miktex/Core/FileSystemWatcher>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>

CORE_INTERNAL_BEGIN_NAMESPACE;

class FileSystemWatcherBase :
  public MiKTeX::Core::FileSystemWatcher
{
public:
  void MIKTEXTHISCALL Subscribe(MiKTeX::Core::FileSystemWatcherCallback* callback) override;

public:
  void MIKTEXTHISCALL Unsubscribe(MiKTeX::Core::FileSystemWatcherCallback* callback) override;

private:
  void NotifySubscribers();

protected:
  void NotifyThreadFunction();

protected:
  void WatchDirectoriesThreadFunction();

protected:
  void StartThreads();

protected:
  void StopThreads();

protected:
  virtual void MIKTEXTHISCALL WatchDirectories() = 0;

protected:
  std::set<MiKTeX::Core::FileSystemWatcherCallback*> callbacks;
  std::atomic_bool done{ false };
  bool failure = false;
  std::shared_mutex mutex;
  std::condition_variable notifyCondition;
  std::mutex notifyMutex;
  std::thread notifyThread;
  std::vector<MiKTeX::Core::FileSystemChangeEvent> pendingNotifications;
  MiKTeX::Core::MiKTeXException threadMiKTeXException;
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_error = MiKTeX::Trace::TraceStream::Open(MIKTEX_TRACE_ERROR);
  std::unique_ptr<MiKTeX::Trace::TraceStream> trace_files = MiKTeX::Trace::TraceStream::Open(MIKTEX_TRACE_FILES);
  std::thread watchDirectoriesThread;
};

CORE_INTERNAL_END_NAMESPACE;
