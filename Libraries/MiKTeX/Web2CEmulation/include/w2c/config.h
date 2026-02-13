/**
 * @file w2c/config.h
 * @author Christian Schenk
 * @brief Web2C emulation
 *
 * @copyright Copyright © 2010-2026 Christian Schenk
 *
 * This file is part of the MiKTeX W2CEMU Library.
 *
 * MiKTeX W2CEMU Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#pragma once

#include "../miktex/W2C/pre.h"

#include "c-auto.h"

#include <math.h>

MIKTEX_BEGIN_EXTERN_C_BLOCK

#if !defined(INTEGER_TYPE)
#define INTEGER_TYPE int
typedef INTEGER_TYPE integer;
#endif

typedef MIKTEX_INT64 longinteger;
typedef long long LONGINTEGER_TYPE;
#define LONGINTEGER_PRI "ll"

#if defined(MIKTEX_WINDOWS)
typedef __int64 integer64;
#else
typedef int64_t integer64;
#endif

MIKTEXW2CEXPORT MIKTEXNORETURN void MIKTEXCEECALL miktex_uexit(int status);
MIKTEXW2CEXPORT MIKTEXNORETURN void MIKTEXCEECALL miktex_usagehelp(const char** lines, const char* bugEmail);

MIKTEX_END_EXTERN_C_BLOCK

#if defined(__cplusplus)
inline void uexit(int status)
{
    miktex_uexit(status);
}
#else
static inline void uexit(int status)
{
    miktex_uexit(status);
}
#endif

#if defined(__cplusplus)
inline void usagehelp(const char** lines, const char* bugEmail)
{
    miktex_usagehelp(lines, bugEmail);
}
#else
static inline void usagehelp(const char** lines, const char* bugEmail)
{
    miktex_usagehelp(lines, bugEmail);
}
#endif
