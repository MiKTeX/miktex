/**
 * @file macFileSystemWatcher.h
 * @author Christian Schenk
 * @brief File system watcher (macOS)
 *
 * @copyright Copyright © 2021-2023 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

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
    bool MIKTEXTHISCALL Start() override;
    bool MIKTEXTHISCALL Stop() override;
    void MIKTEXTHISCALL WatchDirectories() override;
    void HandleDirectoryChange(const char* path, FSEventStreamEventFlags flags);

    static void Callback(ConstFSEventStreamRef streamRef, void* clientCallBackInfo, size_t numEvents, void* eventPaths, const FSEventStreamEventFlags* eventFlags, const FSEventStreamEventId* eventIds);

    FSEventStreamContext context;
    std::vector<CFStringRef> directories;
    std::condition_variable runLoopCondition;
    std::mutex runLoopMutex;
    CFRunLoopRef runLoop = nullptr;
    bool runLoopRunning = false;
    FSEventStreamRef stream = nullptr;
};

CORE_INTERNAL_END_NAMESPACE;
