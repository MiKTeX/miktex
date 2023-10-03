/**
 * @file macFileSystemWatcher.cpp
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

#include "config.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "internal.h"

#include "macFileSystemWatcher.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

unique_ptr<FileSystemWatcher> FileSystemWatcher::Create()
{
    return make_unique<macFileSystemWatcher>();
}

macFileSystemWatcher::~macFileSystemWatcher()
{
    try
    {
        Stop();
    }
    catch (const exception&)
    {
    }
}

void macFileSystemWatcher::AddDirectories(const vector<PathName>& directories)
{
    bool wasRunning = Stop();
    unique_lock l(mutex);
    for (const PathName &p : directories)
    {
        trace_files->WriteLine("core", fmt::format("adding directory to watch list: {0}", Q_(p.ToDisplayString())));
        this->directories.push_back(CFStringCreateWithCString(nullptr, p.ToString().c_str(), kCFStringEncodingUTF8));
    }
    l.unlock();
    if (wasRunning)
    {
        Start();
    }
}

bool macFileSystemWatcher::Start()
{
    bool runningExpected = false;
    if (!running.compare_exchange_strong(runningExpected, true))
    {
        return false;
    }
    context.version = 0;
    context.info = this;
    context.retain = nullptr;
    context.release = nullptr;
    context.copyDescription = nullptr;
    FSEventStreamCreateFlags flags = 0 |
                                     kFSEventStreamCreateFlagFileEvents |
                                     kFSEventStreamCreateFlagNoDefer |
                                     0;
    CFTimeInterval latency = 0.3;
    unique_lock l(mutex);
    if (directories.size() == 0)
    {
        return true;
    }
    auto pathsToWatch = CFArrayCreate(nullptr, reinterpret_cast<const void**> (&directories[0]), directories.size(), &kCFTypeArrayCallBacks);
    l.unlock();
    stream = FSEventStreamCreate(nullptr, &Callback, &context, pathsToWatch, kFSEventStreamEventIdSinceNow, latency, flags);
    if (stream == nullptr)
    {
        MIKTEX_FATAL_ERROR_2("FSEventStreamCreate() failure");
    }
    StartThreads();
    unique_lock l2(runLoopMutex);
    runLoopCondition.wait(l2, [this] { return runLoopRunning; });
    return true;
}

bool macFileSystemWatcher::Stop()
{
    bool runningExpected = true;
    if (!running.compare_exchange_strong(runningExpected, false))
    {
        return false;
    }
    unique_lock l(runLoopMutex);
    if (runLoopRunning)
    {
        CFRunLoopStop(runLoop);
    }
    l.unlock();
    StopThreads();
    if (stream != nullptr)
    {
        FSEventStreamRelease(stream);
        stream = nullptr;
    }
    return true;
}

void macFileSystemWatcher::WatchDirectories()
{
    unique_lock l(runLoopMutex);
    runLoop = CFRunLoopGetCurrent();
    FSEventStreamScheduleWithRunLoop(stream, runLoop, kCFRunLoopDefaultMode);
    if (!FSEventStreamStart(stream))
    {
        MIKTEX_FATAL_ERROR("FSEventStreamStart() failure");
    }
    runLoopRunning = true;
    l.unlock();
    runLoopCondition.notify_all();
    CFRunLoopRun();
    l.lock();
    runLoopRunning = false;
    l.unlock();
    runLoopCondition.notify_all();
    FSEventStreamStop(stream);
    FSEventStreamInvalidate(stream);
    runLoop = nullptr;
}

void macFileSystemWatcher::Callback(ConstFSEventStreamRef streamRef, void* clientCallBackInfo, size_t numEvents, void* eventPaths, const FSEventStreamEventFlags* eventFlags, const FSEventStreamEventId* eventIds)
{
    macFileSystemWatcher* This = reinterpret_cast<macFileSystemWatcher*>(clientCallBackInfo);
    unique_lock l(This->notifyMutex);
    for (size_t idx = 0; idx < numEvents; ++idx)
    {
        This->HandleDirectoryChange(reinterpret_cast<const char**>(eventPaths)[idx], eventFlags[idx]);
    }
    l.unlock();
    This->notifyCondition.notify_all();
}

void macFileSystemWatcher::HandleDirectoryChange(const char* path, FSEventStreamEventFlags flags)
{
    FileSystemChangeEvent ev;
    if ((flags & kFSEventStreamEventFlagItemRemoved) != 0)
    {
        ev.action = FileSystemChangeAction::Removed;
    }
    else if ((flags & kFSEventStreamEventFlagItemInodeMetaMod) != 0)
    {
        ev.action = FileSystemChangeAction::Modified;
    }
    else if ((flags & kFSEventStreamEventFlagItemCreated) != 0)
    {
        ev.action = FileSystemChangeAction::Added;
    }
    else
    {
        return;
    }
    ev.fileName = path;
    pendingNotifications.push_back(ev);
}
