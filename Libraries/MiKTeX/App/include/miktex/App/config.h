/**
 * @file miktex/App/config.h
 * @author Christian Schenk
 * @brief Framework configuration
 *
 * @copyright Copyright © 2005-2022 Christian Schenk
 *
 * This file is part of the MiKTeX Application Framework.
 *
 * The MiKTeX Application Framework is licensed under GNU General Public License
 * version 2 or any later version.
 */

#pragma once

#include <miktex/First>
#include <miktex/Definitions>

// DLL import/export switch
#if !defined(BDF6E2537F116547846406B5B2B65949)
#   if defined(MIKTEX_APP_SHARED)
#       define MIKTEXAPPEXPORT MIKTEXDLLIMPORT
#   else
#       define MIKTEXAPPEXPORT
#   endif
#endif

// API decoration for exported member functions
#define MIKTEXAPPTHISAPI(type) MIKTEXAPPEXPORT type MIKTEXTHISCALL
#define MIKTEXAPPCEEAPI(type) MIKTEXAPPEXPORT type MIKTEXCEECALL

#if defined(__GNUC__)
#   define MIKTEXAPPTYPEAPI(type) MIKTEXAPPEXPORT type
#else
#   define MIKTEXAPPTYPEAPI(type) type
#endif

#define MIKTEX_APP_BEGIN_NAMESPACE              \
    namespace MiKTeX {                          \
        namespace App {

#define MIKTEX_APP_END_NAMESPACE                \
        }                                       \
    }
