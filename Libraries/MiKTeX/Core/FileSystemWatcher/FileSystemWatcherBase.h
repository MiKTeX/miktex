/**
 * @file FileSystemWatcherBase.h
 * @author Christian Schenk
 * @brief File system watcher (base implementation)
 *
 * @copyright Copyright © 2021-2026 Christian Schenk
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
#include <thread>
#include <set>
#include <shared_mutex>

#include <miktex/Core/FileSystemWatcher>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>

CORE_INTERNAL_BEGIN_NAMESPACE;

class FileSystemWatcherBase : public MiKTeX::Core::FileSystemWatcher
{

public:

    void MIKTEXTHISCALL Subscribe(MiKTeX::Core::FileSystemWatcherCallback *callback) override;
    void MIKTEXTHISCALL Unsubscribe(MiKTeX::Core::FileSystemWatcherCallback *callback) override;

protected:

    void NotifyThreadFunction();
    void WatchDirectoriesThreadFunction();
    void StartThreads();
    void StopThreads();
    virtual void MIKTEXTHISCALL WatchDirectories() = 0;

    std::set<MiKTeX::Core::FileSystemWatcherCallback *> callbacks;
    std::atomic_bool done{false};
    bool failure = false;
    std::shared_mutex mutex;
    std::condition_variable notifyCondition;
    std::mutex notifyMutex;
    std::thread notifyThread;
    std::vector<MiKTeX::Core::FileSystemChangeEvent> pendingNotifications;
    std::atomic_bool running{false};
    MiKTeX::Core::MiKTeXException threadMiKTeXException;
    std::unique_ptr<MiKTeX::Trace::TraceStream> trace_error = MiKTeX::Trace::TraceStream::Open(MIKTEX_TRACE_ERROR);
    std::unique_ptr<MiKTeX::Trace::TraceStream> trace_files = MiKTeX::Trace::TraceStream::Open(MIKTEX_TRACE_FILES);
    std::thread watchDirectoriesThread;

private:

    void NotifySubscribers();
};

CORE_INTERNAL_END_NAMESPACE;
