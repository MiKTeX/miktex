/**
 * @file CoreStopWatch.h
 * @author Christian Schenk
 * @brief CoreStopWatch class
 *
 * @copyright Copyright © 1996-2023 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#pragma once

#include <miktex/Trace/StopWatch>

#include "Session/SessionImpl.h"

CORE_INTERNAL_BEGIN_NAMESPACE;

class CoreStopWatch
{

public:

    CoreStopWatch(const std::string &message) : stopWatch(MiKTeX::Trace::StopWatch::Start(SESSION_IMPL()->trace_stopwatch.get(), "core", message))
    {
    }

    ~CoreStopWatch()
    {
        try
        {
            stopWatch->Stop();
        }
        catch (const std::exception &)
        {
        }
    }

private:

    std::unique_ptr<MiKTeX::Trace::StopWatch> stopWatch;
};

CORE_INTERNAL_END_NAMESPACE;
