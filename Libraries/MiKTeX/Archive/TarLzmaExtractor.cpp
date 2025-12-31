/**
 * @file TarLzmaExtractor.cpp
 * @defgroup MiKTeX Archive
 * @author Christian Schenk
 * @brief TarLzmaExtractor implementation for MiKTeX Archive
 *
 * @copyright Copyright © 2001-2025 Christian Schenk
 *
 * This file is part of the MiKTeX Archive Library.
 *
 * MiKTeX Archive Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#include "config.h"

#include <memory>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/LzmaStream>
#include <miktex/Trace/StopWatch>

#include "internal.h"

#include "TarLzmaExtractor.h"

using namespace std;

using namespace MiKTeX::Archive;
using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

void TarLzmaExtractor::Extract(const PathName& path, const PathName& destDir, bool makeDirectories, IExtractCallback* callback, const string& prefix)
{
    unique_ptr<StopWatch> stopWatch = StopWatch::Start(traceStopWatch.get(), TRACE_FACILITY, path.GetFileName().ToString());
    traceStream->WriteLine(TRACE_FACILITY, fmt::format(T_("extracting {0}"), Q_(path)));
    unique_ptr<LzmaStream> lzmaStream = LzmaStream::Create(path, true);
    TarExtractor::Extract(lzmaStream.get(), destDir, makeDirectories, callback, prefix);
}
