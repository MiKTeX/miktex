#pragma once

#include "LibLsp/JsonRpc/MessageIssue.h"
#include <memory>

class ParentProcessWatcher
{
public:
    struct ParentProcessWatcherData;

    ParentProcessWatcher(lsp::Log& log, int pid, std::function<void()> const&& callback, uint32_t poll_delay_secs = 10);

    ~ParentProcessWatcher();

    std::shared_ptr<ParentProcessWatcherData> d_ptr;
};
