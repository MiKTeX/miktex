/**
 * @file unxFileSystemWatcher.h
 * @author Christian Schenk
 * @brief File system watcher (Linux)
 *
 * @copyright Copyright © 2021-2023 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#pragma once

#include <sys/inotify.h>

#include <unordered_map>

#include "../FileSystemWatcherBase.h"

CORE_INTERNAL_BEGIN_NAMESPACE;

class unxFileSystemWatcher :
  public FileSystemWatcherBase
{

public:

    unxFileSystemWatcher();

    virtual MIKTEXTHISCALL ~unxFileSystemWatcher();

private:

    void MIKTEXTHISCALL AddDirectories(const std::vector<MiKTeX::Util::PathName>& directories) override;

    bool MIKTEXTHISCALL Start() override;

    bool MIKTEXTHISCALL Stop() override;

    void MIKTEXTHISCALL WatchDirectories() override;

    void HandleDirectoryChange(const struct inotify_event* evt);

    int cancelEventPipe[2];
    std::unordered_map<int, MiKTeX::Util::PathName> directories;
    int watchFd;
};

CORE_INTERNAL_END_NAMESPACE;
