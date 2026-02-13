/**
 * @file miktex/W2C/pre.h
 * @author Christian Schenk
 * @brief Web2C emulation
 *
 * @copyright Copyright © 2013-2026 Christian Schenk
 *
 * This file is part of the MiKTeX W2CEMU Library.
 *
 * MiKTeX W2CEMU Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#pragma once

#if defined(__cplusplus)
#include <miktex/Core/Session>
#else
#include <miktex/Core/c/api.h>
#endif
#include <miktex/Core/Debug>
#include <miktex/Core/IntegerTypes>
#include <miktex/Definitions>
#include <miktex/KPSE/Emulation>
#include <miktex/Version>

// DLL import/export switch
#if !defined(B96BCD894353492A9CF685C84B9AB5E0)
#define MIKTEXW2CEXPORT MIKTEXDLLIMPORT
#endif

// API decoration for exported functions and data
#define MIKTEXW2CCEEAPI(type) MIKTEXW2CEXPORT type MIKTEXCEECALL
#define MIKTEXW2CDATA(type) MIKTEXW2CEXPORT type

#define MIKTEX_WEB2C_BEGIN_NAMESPACE        \
    namespace MiKTeX {                      \
        namespace Web2C {

#define MIKTEX_WEB2C_END_NAMESPACE          \
        }                                   \
    }
