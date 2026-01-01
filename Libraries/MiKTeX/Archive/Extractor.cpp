/**
 * @file Extractor.cpp
 * @defgroup MiKTeX Archive
 * @author Christian Schenk
 * @brief Extractor interface for MiKTeX Archive
 *
 * @copyright Copyright © 2001-2026 Christian Schenk
 *
 * This file is part of the MiKTeX Archive Library.
 *
 * MiKTeX Archive Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#include "config.h"

#include <memory>

#include "internal.h"

#include "CabExtractor.h"
#include "TarBzip2Extractor.h"
#include "TarLzmaExtractor.h"

#if defined (MIKTEX_WINDOWS) && defined(ENABLE_WINDOWS_CAB_EXTRACTOR)
#  include "win/winCabExtractor.h"
#endif

using namespace std;

using namespace MiKTeX::Archive;

Extractor::~Extractor() noexcept
{
}

unique_ptr<Extractor> Extractor::New(ArchiveFileType archiveFileType)
{
    switch (archiveFileType)
    {
    case ArchiveFileType::MSCab:
#if defined (MIKTEX_WINDOWS) && defined(ENABLE_WINDOWS_CAB_EXTRACTOR)
        static bool USE_WINDOWS_CABEXTRACTOR = false;
        if (USE_WINDOWS_CABEXTRACTOR)
        {
            return new winCabExtractor;
        }
#endif
        return make_unique<CabExtractor>();
    case ArchiveFileType::TarBzip2:
        return make_unique<TarBzip2Extractor>();
    case ArchiveFileType::TarLzma:
    case ArchiveFileType::TarXz:
        return make_unique<TarLzmaExtractor>();
    case ArchiveFileType::Tar:
        return make_unique<TarExtractor>();
    default:
        UNIMPLEMENTED();
    }
}
