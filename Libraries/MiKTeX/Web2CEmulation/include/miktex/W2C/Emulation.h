/* miktex/W2C/Emulation.h:                              -*- C++ -*-

   Copyright (C) 2010-2019 Christian Schenk

   This file is part of the MiKTeX W2CEMU Library.

   The MiKTeX W2CEMU Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX W2CEMU Library is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX W2CEMU Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

/// @file miktex/W2c/Emulation.h

/// @defgroup W2C Web2C emulation
/// 
/// @brief Utilities for emulating Web2c.
///
/// In order to support programs relying on Web2c, we implement an
/// emulation library.

/// @{

#pragma once

#if !defined(AA9CDF05506A4A07827B1144C4755C06)
/// @cond
#define AA9CDF05506A4A07827B1144C4755C06
/// @endcond

#if defined(__cplusplus)
#else
#include <miktex/Core/c/api.h>
#endif
#include <miktex/Definitions>
#include <miktex/Core/Debug>
#include <miktex/Core/IntegerTypes>
#include <miktex/Version>
#include <miktex/KPSE/Emulation>

#include "pre.h"

#if defined(__cplusplus)
MIKTEX_WEB2C_BEGIN_NAMESPACE;

MIKTEXW2CCEEAPI(char*) GetCurrentFileName();
MIKTEXW2CCEEAPI(void) GetSecondsAndMicros(int* seconds, int* micros);
MIKTEXW2CCEEAPI(int) RunSystemCommand(const char* cmd);

MIKTEX_WEB2C_END_NAMESPACE;
#endif


/* _________________________________________________________________________
 *
 *  c-auto.h
 *
 */

#include "../../c-auto.h"

/* _________________________________________________________________________
 *
 * w2c/config.h
 *
 */

#include "../../w2c/config.h"

/* _________________________________________________________________________
 *
 * lib/lib.h
 *
 */

#include "../../lib/lib.h"

/* _________________________________________________________________________
 *
 * texmfmp.h
 *
 */

/// @name texmfmp.h
/// Stuff from `texmfmp.h`.
/// @{

#if defined(__cplusplus) 
inline char* generic_synctex_get_current_name()
{
  return MiKTeX::Web2C::GetCurrentFileName();
}
#endif

#if defined(__cplusplus)
inline int runsystem(const char* cmd)
{
  return MiKTeX::Web2C::RunSystemCommand(cmd);
}
#endif

#if defined(__cplusplus)
inline void secondsandmicros(integer& s, integer& m)
{
  MiKTeX::Web2C::GetSecondsAndMicros(&s, &m);
}
#endif

/// @}

/* _________________________________________________________________________
 *
 * cpascal.h
 *
 */

/// @name cpascal.h
/// Stuff from `cpascal.h`.
/// @{

#if defined(__cplusplus)
template<class T> T* addressof(T& x)
{
  return &x;
}
#else
#define addressof(x) (&(x))
#endif

#if defined(__cplusplus)
template<class T> const char* conststringcast(T* s)
{
  MIKTEX_ASSERT(sizeof(s[0]) == sizeof(char));
  return (const char*)s;
}
#endif

#define decr(x) --(x)

#define incr(x) ++(x)

#define halfp(i) ((i) >> 1)

#define ISDIRSEP IS_DIR_SEP

#define libcfree free

#define odd(x) ((x) & 1)

#define round(x) miktex_zround((double) (x))

#define xmallocarray(type, size) \
  ((type*)xmalloc(((size) + 1) * sizeof(type)))

#define xreallocarray(ptr, type, size) \
  ((type*)xrealloc(ptr, ((size) + 1) * sizeof(type)))

#if defined(__cplusplus)
#  define nil nullptr
#else
#  define nil ((void*)0)
#endif

#define kpsevarvalue kpse_var_value

#define kpseinitprog kpse_init_prog

#define kpsesetprogramenabled kpse_set_program_enabled

#define kpsepkformat kpse_pk_format

#define kpsesrccmdline kpse_src_cmdline

#define kpsemaketexdiscarderrors kpse_make_tex_discard_errors

typedef double real;

#define intcast(x) ((integer)(x))

#define stringcast(x) ((char *) (x))

#define ucharcast(x) ((unsigned char)(x))

#if defined(MIKTEX_WINDOWS)
#  define promptfilenamehelpmsg "(Press Enter to retry, or Control-Z to exit"
#else
#  define promptfilenamehelpmsg "(Press Enter to retry, or Control-D to exit"
#endif

/// @}

/* _________________________________________________________________________ */

#if defined(__cplusplus)
MIKTEX_BEGIN_EXTERN_C_BLOCK
#endif

MIKTEXW2CCEEAPI(integer) miktex_zround(double r);

MIKTEX_END_EXTERN_C_BLOCK

#endif

/// @}
