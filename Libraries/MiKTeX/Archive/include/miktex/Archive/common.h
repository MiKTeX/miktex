/**
 * @file miktex/Archive/common.h
 * @defgroup MiKTeX Archive
 * @author Christian Schenk
 * @brief Common definitions for MiKTeX Archive
 *
 * @copyright Copyright © 2026 Christian Schenk
 *
 * This file is part of the MiKTeX Archive Library.
 *
 * MiKTeX Archive Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#pragma once

#include "config.h"

MIKTEX_ARCHIVE_BEGIN_NAMESPACE;

enum class ArchiveFileType
{
    None, MSCab, TarBzip2, Zip, Tar, TarLzma, TarXz
};

MIKTEX_ARCHIVE_END_NAMESPACE;
