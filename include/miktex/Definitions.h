/* miktex/Definitions.h: basic definitions              -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#if defined(_MSC_VER)
#pragma once
#endif

#if !defined(MIKTEX_DEFINITIONS_H)
#define MIKTEX_DEFINITIONS_H

#if !defined(MIKTEX)
#define MIKTEX 1
#endif

#if defined(_WIN64)
#define MIKTEX_WINDOWS 1
#define MIKTEX_WINDOWS_64 1
#elif defined(_WIN32)
#define MIKTEX_WINDOWS 1
#define MIKTEX_WINDOWS_32 1
#else
#define MIKTEX_UNIX 1
#endif

#if defined(_MSC_VER)
#define MIKTEXCALLBACK __cdecl
#define MIKTEXCEECALL __cdecl
#define MIKTEXDEPRECATED __declspec(deprecated)
#define MIKTEXDLLEXPORT __declspec(dllexport)
#define MIKTEXDLLIMPORT __declspec(dllimport)
#define MIKTEXNORETURN __declspec(noreturn)
#define MIKTEXNOVTABLE __declspec(novtable)
#define MIKTEXTHISCALL __thiscall
#elif defined(__GNUC__)
#define MIKTEXCALLBACK
#define MIKTEXCEECALL
#define MIKTEXDEPRECATED __attribute__((deprecated))
#if defined(_WIN32)
#define MIKTEXDLLEXPORT __attribute__((dllexport))
#define MIKTEXDLLIMPORT __attribute__((dllimport))
#else
#define MIKTEXDLLEXPORT __attribute__((visibility("default")))
#define MIKTEXDLLIMPORT __attribute__((visibility("default")))
#endif
#define MIKTEXNORETURN __attribute__((noreturn))
#define MIKTEXNOVTABLE
#define MIKTEXTHISCALL
#endif

#if defined(__cplusplus)
#define MIKTEX_EXTERN_C extern "C"
#define MIKTEX_BEGIN_EXTERN_C_BLOCK extern "C" {
#define MIKTEX_END_EXTERN_C_BLOCK }
#else
#define MIKTEX_EXTERN_C
#define MIKTEX_BEGIN_EXTERN_C_BLOCK
#define MIKTEX_END_EXTERN_C_BLOCK
#endif

#endif
