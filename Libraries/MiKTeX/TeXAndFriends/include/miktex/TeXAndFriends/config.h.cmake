/**
 * @file miktex/TeXAndFriends/config.h
 * @author Christian Schenk
 * @brief TeXMF compile-time configuration
 *
 * @copyright Copyright © 2009-2022 Christian Schenk
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
#if !defined(B8C7815676699B4EA2DE96F0BD727276)
#   if defined(MIKTEX_TEXMF_SHARED)
#       define MIKTEXMFEXPORT MIKTEXDLLIMPORT
#   else
#       define MIKTEXMFEXPORT
#  endif
#endif

#define MIKTEXMFAPI_(type, cc) MIKTEXMFEXPORT type cc

// API decoration for exported member functions
#define MIKTEXMFTHISAPI(type) MIKTEXMFEXPORT type MIKTEXTHISCALL

// API decoration for exported functions
#define MIKTEXMFCEEAPI(type) MIKTEXMFEXPORT type MIKTEXCEECALL

// API decoration for exported types
#if defined(__GNUC__)
#   define MIKTEXMFTYPEAPI(type) MIKTEXMFEXPORT type
#else
#   define MIKTEXMFTYPEAPI(type) type
#endif

// API decoration for exported data
#define MIKTEXMFDATA(type) MIKTEXMFEXPORT type

#define MIKTEX_TEXMF_BEGIN_NAMESPACE        \
namespace MiKTeX {                          \
    namespace TeXAndFriends {

#define MIKTEX_TEXMF_END_NAMESPACE          \
    }                                       \
}

#cmakedefine WITH_PDFTEX 1
#cmakedefine WITH_SYNCTEX 1

#if defined(MIKTEX_TEX) || defined(MIKTEX_TRIPTEX)
#   define MIKTEX_TEX_COMPILER 1
#endif

#if defined(MIKTEX_PDFTEX)
#   define MIKTEX_TEX_COMPILER 1
#endif

#if defined(MIKTEX_PTEX) || defined(MIKTEX_EPTEX) || defined(MIKTEX_UPTEX) || defined(MIKTEX_EUPTEX)
#   define MIKTEX_PTEX_FAMILY 1
#   define MIKTEX_TEX_COMPILER 1
#endif

#if defined(MIKTEX_XETEX)
#   define MIKTEX_TEX_COMPILER 1
#endif

#if defined(MIKTEX_METAFONT)
#   define MIKTEX_META_COMPILER 1
#   define HAVE_MAIN_MEMORY 1
#endif

#if defined(MIKTEX_TEX_COMPILER)
#   define HAVE_EXTRA_MEM_BOT 1
#   define HAVE_EXTRA_MEM_TOP 1
#   define HAVE_MAIN_MEMORY 1
#   define HAVE_POOL_FREE 1
#   define HAVE_STRINGS_FREE 1
#endif
