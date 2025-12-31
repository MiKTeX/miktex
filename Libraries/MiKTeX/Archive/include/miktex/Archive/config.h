/**
 * @file miktex/Archive/config.h
 * @defgroup MiKTeX Archive
 * @author Christian Schenk
 * @brief Library configuration for MiKTeX Archive
 *
 * @copyright Copyright © 2008-2025 Christian Schenk
 *
 * This file is part of the MiKTeX Archive Library.
 *
 * MiKTeX Archive Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#pragma once

#include <miktex/First>
#include <miktex/Definitions>

// DLL import/export switch
#if !defined(DAA6476494C144C8BED9A9E8810BAABA)
#  if defined(MIKTEX_ARCHIVE_SHARED)
#    define MIKTEXARCHIVEEXPORT MIKTEXDLLIMPORT
#  else
#    define MIKTEXARCHIVEEXPORT
#  endif
#endif

// API decoration for exported member functions
#define MIKTEXARCHIVECEEAPI(type) MIKTEXARCHIVEEXPORT type MIKTEXCEECALL

#define MIKTEX_ARCHIVE_BEGIN_NAMESPACE  \
    namespace MiKTeX {                  \
        namespace Archive {

#define MIKTEX_ARCHIVE_END_NAMESPACE    \
        }                               \
    }
