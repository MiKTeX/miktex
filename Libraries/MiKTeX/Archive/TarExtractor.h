/**
 * @file TarExtractor.h
 * @defgroup MiKTeX Archive
 * @author Christian Schenk
 * @brief TarExtractor implementation for MiKTeX Archive
 *
 * @copyright Copyright © 2001-2026 Christian Schenk
 *
 * This file is part of the MiKTeX Archive Library.
 *
 * MiKTeX Archive Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#pragma once

#include <miktex/Trace/TraceStream>

#include "miktex/Archive/Extractor"

#include "internal.h"

BEGIN_INTERNAL_NAMESPACE;

class TarExtractor : public MiKTeX::Archive::Extractor
{

public:

    TarExtractor();
    MIKTEXTHISCALL ~TarExtractor() override;

    void MIKTEXTHISCALL Extract(const MiKTeX::Util::PathName& path, const MiKTeX::Util::PathName& destDir, bool makeDirectories, IExtractCallback* callback, const std::string& prefix) override;
    void MIKTEXTHISCALL Extract(MiKTeX::Core::Stream* stream, const MiKTeX::Util::PathName& destDir, bool makeDirectories, IExtractCallback* callback, const std::string& prefix) override;

protected:

    size_t Read(void* data, size_t numBytes)
    {
        size_t n = streamIn->Read(data, numBytes);
        totalBytesRead += n;
        return n;
    }

    void ReadBlock(void* data);
    void Skip(size_t bytes);

    std::unique_ptr<MiKTeX::Trace::TraceStream> traceStream;
    std::unique_ptr<MiKTeX::Trace::TraceStream> traceStopWatch;

    bool haveLongName;
    MiKTeX::Util::PathName longName;
    size_t totalBytesRead;
    MiKTeX::Core::Stream* streamIn = nullptr;
};

END_INTERNAL_NAMESPACE;
