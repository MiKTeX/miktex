/* macFileSystemWatcher.h: file system watcher (macOS specials)

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

#include <CoreServices/CoreServices.h>

#include "../FileSystemWatcherBase.h"

CORE_INTERNAL_BEGIN_NAMESPACE;

class macFileSystemWatcher :
  public FileSystemWatcherBase
{
public:
    virtual MIKTEXTHISCALL ~macFileSystemWatcher();

private:
    void MIKTEXTHISCALL AddDirectories(const std::vector<MiKTeX::Util::PathName>& directories) override;

private:
    bool MIKTEXTHISCALL Start() override;

private:
    bool MIKTEXTHISCALL Stop() override;

private:
    void MIKTEXTHISCALL WatchDirectories() override;

private:
    static void Callback(ConstFSEventStreamRef streamRef, void* clientCallBackInfo, size_t numEvents, void* eventPaths, const FSEventStreamEventFlags* eventFlags, const FSEventStreamEventId* eventIds);

private:
    void HandleDirectoryChange(const char* path, FSEventStreamEventFlags flags);

private:
    FSEventStreamContext context;
    std::vector<CFStringRef> directories;
    std::condition_variable runLoopCondition;
    std::mutex runLoopMutex;
    bool runLoopRunning = false;
    CFRunLoopRef runLoop = nullptr;
    FSEventStreamRef stream = nullptr;
};

CORE_INTERNAL_END_NAMESPACE;
