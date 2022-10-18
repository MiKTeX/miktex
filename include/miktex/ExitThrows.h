/**
 * @file miktex/ExitThrows.h
 * @author Christian Schenk
 * @brief Redefine exit (deprecated)
 *
 * @copyright Copyright © 2018-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives
 * unlimited permission to copy and/or distribute it, with or
 * without modifications, as long as this notice is preserved.
 */

#pragma once

#define exit miktex_hidden_exit
#include <cstdlib>
#if defined(_WIN32)
#   include <process.h>
#endif
#undef exit

#if defined(_WIN32)
#   define MIKTEX_NORETURN_TMP_ __declspec(noreturn)
#   define MIKTEX_CDECL_TMP_ __cdecl
#else
#   define MIKTEX_NORETURN_TMP_
#   define MIKTEX_CDECL_TMP_ cdecl
#endif

inline MIKTEX_NORETURN_TMP_ void MIKTEX_CDECL_TMP_ exit(int code)
{
    throw code;
}

#undef MIKTEX_NORETURN_TMP_
#undef MIKTEX_CDECL_TMP_
