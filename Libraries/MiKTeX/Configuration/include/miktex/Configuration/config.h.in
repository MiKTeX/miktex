/**
 * @file miktex/Configuration/config.h
 * @author Christian Schenk
 * @brief Library configuration
 *
 * @copyright Copyright © 2017-2022 Christian Schenk
 *
 * This file is part of the MiKTeX Configuration Library.
 *
 * The MiKTeX Configuration Library is licensed under GNU General Public License
 * version 2 or any later version.
 */

#pragma once

#include <miktex/First>
#include <miktex/Definitions>

#if !defined(B967F942274040EE9C705A791BAED737)
#   if defined(MIKTEX_CONFIG_SHARED)
#       define MIKTEXCONFIGEXPORT MIKTEXDLLIMPORT
#   else
#       define MIKTEXCONFIGEXPORT
#  endif
#endif

#define MIKTEXCONFIGTHISAPI(type) MIKTEXCONFIGEXPORT type MIKTEXTHISCALL
#define MIKTEXCONFIGCEEAPI(type) MIKTEXCONFIGEXPORT type MIKTEXCEECALL

#if defined(__GNUC__)
#   define MIKTEXCONFIGTYPEAPI(type) MIKTEXCONFIGEXPORT type
#else
#   define MIKTEXCONFIGTYPEAPI(type) type
#endif

#define MIKTEX_CONFIG_BEGIN_NAMESPACE           \
    namespace MiKTeX {                          \
        namespace Configuration {

#define MIKTEX_CONFIG_END_NAMESPACE             \
        }                                       \
    }
