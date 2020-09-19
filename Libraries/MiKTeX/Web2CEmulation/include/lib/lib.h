/* lib/lib.h:                                           -*- C++ -*-

   Copyright (C) 2013-2020 Christian Schenk

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

/// @file lib/lib.h

/// @addtogroup W2C

/// @{

#pragma once

#if !defined(CD8636162364443B94377CDABD5ABD8F)
/// @cond
#define CD8636162364443B94377CDABD5ABD8F
/// @endcond

#include "../miktex/W2C/pre.h"

#if defined(__cplusplus)
/// @namespace MiKTeX::Web2C
/// @brief MiKTeX Web2C emulation utilities.
MIKTEX_WEB2C_BEGIN_NAMESPACE;

MIKTEXW2CCEEAPI(void) ChangeRecorderFileName(const char* fileName);
MIKTEXW2CCEEAPI(int) OpenInput(FILE** ppfile, kpse_file_format_type format, const char* modeString);
MIKTEXW2CCEEAPI(void) RecordFileName(const char* path, MiKTeX::Core::FileAccess access);
MIKTEXW2CCEEAPI(MiKTeX::Core::PathName) GetOutputDirectory();
MIKTEXW2CCEEAPI(void) SetOutputDirectory(const MiKTeX::Core::PathName& path);

MIKTEX_WEB2C_END_NAMESPACE;
#endif

MIKTEX_BEGIN_EXTERN_C_BLOCK

MIKTEXW2CCEEAPI(void) miktex_web2c_change_recorder_file_name(const char* path);
MIKTEXW2CCEEAPI(void) miktex_web2c_record_file_name(const char* path, int reading);
MIKTEXW2CCEEAPI(integer) miktex_zround(double r);
MIKTEXW2CCEEAPI(void) miktex_setupboundvariable(integer* pVar, const char* lpszVarName, integer dflt);
MIKTEXW2CCEEAPI(const char *) miktex_web2c_get_output_directory();
MIKTEXW2CCEEAPI(void) miktex_web2c_set_output_directory(const char* path);
extern MIKTEXW2CDATA(boolean) miktex_web2c_recorder_enabled;
extern MIKTEXW2CDATA(const_string) miktex_web2c_version_string;
extern MIKTEXW2CDATA(string) miktex_web2c_fullnameoffile;

/// @name lib.h
/// Stuff from `lib.h`.
/// @{

MIKTEX_END_EXTERN_C_BLOCK

#if defined(__cplusplus)
inline int open_input(FILE** f_ptr, kpse_file_format_type filefmt, const char* fopen_mode)
{
  return MiKTeX::Web2C::OpenInput(f_ptr, filefmt, fopen_mode);
}
#endif

static inline void close_file(FILE* f)
{
  if (f != 0)
  {
    fclose(f);
  }
}

#if defined(__cplusplus)
inline void recorder_change_filename(const char* new_name)
{
  MiKTeX::Web2C::ChangeRecorderFileName(new_name);
}
#else
static inline void recorder_change_filename(const char* new_name)
{
  miktex_web2c_change_recorder_file_name(new_name);
}
#endif

#if defined(__cplusplus)
inline void recorder_record_input(const char* fname)
{
  MiKTeX::Web2C::RecordFileName(fname, MiKTeX::Core::FileAccess::Read);
}
#else
static inline void recorder_record_input(const char* fname)
{
  miktex_web2c_record_file_name(fname, 1);
}
#endif

#if defined(__cplusplus)
inline void recorder_record_output(const char* fname)
{
  MiKTeX::Web2C::RecordFileName(fname, MiKTeX::Core::FileAccess::Write);
}
#else
static inline void recorder_record_output(const char* fname)
{
  miktex_web2c_record_file_name(fname, 0);
}
#endif

#if defined(__cplusplus)
#  define output_directory (MiKTeX::Web2C::GetOutputDirectory().Empty() ? nullptr : MiKTeX::Web2C::GetOutputDirectory().GetData())
#else
#  define output_directory miktex_web2c_get_output_directory()
#endif

#if defined(__cplusplus)
#else
static inline void setupboundvariable(integer* var, const char* var_name, integer dflt)
{
  miktex_setupboundvariable(var, var_name, dflt);
}
#endif

static inline integer zround(double r)
{
  return miktex_zround(r);
}

#define recorder_enabled miktex_web2c_recorder_enabled
#define fullnameoffile miktex_web2c_fullnameoffile

/// @}

#endif

/// @}
