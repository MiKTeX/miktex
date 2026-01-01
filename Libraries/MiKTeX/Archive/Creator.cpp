/**
 * @file Creator.cpp
 * @defgroup MiKTeX Archive
 * @author Christian Schenk
 * @brief Creator interface for MiKTeX Archive
 *
 * @copyright Copyright © 2026 Christian Schenk
 *
 * This file is part of the MiKTeX Archive Library.
 *
 * MiKTeX Archive Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#include "config.h"

#include <memory>

#include "internal.h"

#include "TarBzip2Creator.h"
#include "TarCreator.h"

using namespace std;

using namespace MiKTeX::Archive;

Creator::~Creator() noexcept
{
}

unique_ptr<Creator> Creator::New(ArchiveFileType archiveFileType)
{
    switch (archiveFileType)
    {
    case ArchiveFileType::TarBzip2:
        return make_unique<TarBzip2Creator>();
    case ArchiveFileType::Tar:
        return make_unique<TarCreator>();
    default:
        UNIMPLEMENTED();
    }
}
