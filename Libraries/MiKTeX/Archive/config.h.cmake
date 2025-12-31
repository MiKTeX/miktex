/**
 * @file config.h
 * @author Christian Schenk
 * @brief Library configuration
 *
 * @copyright Copyright © 2001-2025 Christian Schenk
 *
 * This file is part of the MiKTeX Archive Library.
 *
 * MiKTeX Archive Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#include <miktex/First>
#include <miktex/Definitions>

#define TRACE_FACILITY "archive"

#if defined(MIKTEX_ARCHIVE_SHARED)
#   define MIKTEXARCHIVEEXPORT MIKTEXDLLEXPORT
#else
#   define MIKTEXARCHIVEEXPORT
#endif

#define DAA6476494C144C8BED9A9E8810BAABA
