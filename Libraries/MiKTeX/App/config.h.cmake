/**
 * @file config.h
 * @author Christian Schenk
 * @brief Internal configuration
 *
 * @copyright Copyright © 2005-2022 Christian Schenk
 *
 * This file is part of the MiKTeX Application Framework.
 *
 * The MiKTeX Application Framework is licensed under GNU General Public License
 * version 2 or any later version.
 */

#include <miktex/First>

#define MIKTEX_COMP_ID "${MIKTEX_COMP_ID}"

#define BDF6E2537F116547846406B5B2B65949

#if defined(MIKTEX_APP_SHARED)
#   define MIKTEXAPPEXPORT MIKTEXDLLEXPORT
#else
#   define MIKTEXAPPEXPORT
#endif
