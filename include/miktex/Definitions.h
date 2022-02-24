/**
 * @file miktex/Definitions.h
 * @author Christian Schenk
 * @brief Basic definitions
 *
 * @copyright Copyright © 1996-2022 Christian Schenk
 *
 * This file is free software; the copyright holder gives
 * unlimited permission to copy and/or distribute it, with or
 * without modifications, as long as this notice is preserved.
 */

#pragma once

#define MIKTEX_DEFINITIONS_H

#if !defined(MIKTEX)
#   define MIKTEX 1
#endif

#if defined(_WIN64)
#   define MIKTEX_WINDOWS 1
#   define MIKTEX_WINDOWS_64 1
#elif defined(_WIN32)
#   define MIKTEX_WINDOWS 1
#   define MIKTEX_WINDOWS_32 1
#else
#   define MIKTEX_UNIX 1
#   if defined(__APPLE__)
#       define MIKTEX_MACOS 1
#   endif
#   if defined(__linux__)
#       define MIKTEX_LINUX 1
#   endif
#   if defined(__FreeBSD__)
#       define MIKTEX_FREEBSD 1
#   endif
#endif

#if defined(_MSC_VER)
#   define MIKTEXCALLBACK __cdecl
#   define MIKTEXCEECALL __cdecl
#   define MIKTEXDEPRECATED __declspec(deprecated)
#   define MIKTEXDLLEXPORT __declspec(dllexport)
#   define MIKTEXDLLIMPORT __declspec(dllimport)
#   define MIKTEXNORETURN __declspec(noreturn)
#   define MIKTEXNOVTABLE __declspec(novtable)
#   define MIKTEXTHISCALL __thiscall
#elif defined(__GNUC__)
#   define MIKTEXCALLBACK
#   define MIKTEXCEECALL
#   define MIKTEXDEPRECATED __attribute__((deprecated))
#   if defined(_WIN32)
#       define MIKTEXDLLEXPORT __attribute__((dllexport))
#       define MIKTEXDLLIMPORT __attribute__((dllimport))
#   else
#       define MIKTEXDLLEXPORT __attribute__((visibility("default")))
#       define MIKTEXDLLIMPORT __attribute__((visibility("default")))
#   endif
#   define MIKTEXNORETURN __attribute__((noreturn))
#   define MIKTEXNOVTABLE
#   define MIKTEXTHISCALL
#endif

#if defined(__cplusplus)
#   define MIKTEX_EXTERN_C extern "C"
#   define MIKTEX_BEGIN_EXTERN_C_BLOCK extern "C" {
#   define MIKTEX_END_EXTERN_C_BLOCK }
#else
#   define MIKTEX_EXTERN_C
#   define MIKTEX_BEGIN_EXTERN_C_BLOCK
#   define MIKTEX_END_EXTERN_C_BLOCK
#endif
