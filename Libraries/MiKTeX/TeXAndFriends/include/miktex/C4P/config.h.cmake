/**
 * @file miktex/C4P/config.h
 * @author Christian Schenk
 * @brief C4P compile-time configuration
 *
 * @copyright Copyright © 2008-2022 Christian Schenk
 *
 * This file is part of the MiKTeX TeXMF Framework.
 *
 * The MiKTeX TeXMF Framework is licensed under GNU General Public License
 * version 2 or any later version.
 */

#pragma once

#include <miktex/First>
#include <miktex/Definitions>

// DLL import/export switch
#if !defined(C1F0C63F01D5114A90DDF8FC10FF410B)
#   if defined(MIKTEX_TEXMF_SHARED)
#       define C4PEXPORT MIKTEXDLLIMPORT
#   else
#       define C4PEXPORT
#   endif
#endif

// API decoration for exported member functions
#define C4PTHISAPI(type) C4PEXPORT type MIKTEXTHISCALL

// API decoration for exported functions
#define C4PCEEAPI(type) C4PEXPORT type MIKTEXCEECALL

// API decoration for exported types
#if defined(__GNUC__)
#   define C4PTYPEAPI(type) C4PEXPORT type
#else
#   define C4PTYPEAPI(type) type
#endif

#define C4PCEECALL MIKTEXCEECALL

#define C4P_BEGIN_NAMESPACE namespace C4P {
#define C4P_END_NAMESPACE }

#cmakedefine HAVE_ROUND 1
#cmakedefine HAVE_TRUNC 1

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wdangling-else"
#pragma clang diagnostic ignored "-Wparentheses-equality"
#endif
