/* lib/lib.h:                                           -*- C++ -*-

   Copyright (C) 2013-2016 Christian Schenk

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

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(CD8636162364443B94377CDABD5ABD8F)
#define CD8636162364443B94377CDABD5ABD8F

#include "../miktex/W2C/pre.h"

#if defined(__cplusplus)
#define open_input(f_ptr, filefmt, fopen_mode) \
  MiKTeX::Web2C::OpenInput(nameoffile + 1, f_ptr, filefmt, fopen_mode)
#else
#define open_input(f_ptr, filefmt, fopen_mode) \
  UNIMPLEMENTED_miktex_web2c_open_input(f_ptr, filefmt, fopen_mode)
#endif

#define close_file(f) ((f) != 0 ? (void)fclose(f) : (void)0)

#define versionstring miktex_web2c_version_string

#if defined(__cplusplus)
#define recorder_change_filename(new_name) \
  MiKTeX::Web2C::ChangeRecorderFileName(new_name)
#else
#define recorder_change_filename(new_name) \
  miktex_web2c_change_recorder_file_name(new_name)
#endif

#if defined(__cplusplus)
#define recorder_record_input(fname) \
  MiKTeX::Web2C::RecordFileName(fname, MiKTeX::Core::FileAccess::Read)
#else
#define recorder_record_input(fname) \
  miktex_web2c_record_file_name(fname, 1)
#endif

#if defined(__cplusplus)
#define recorder_record_output(fname) \
  MiKTeX::Web2C::RecordFileName(fname, MiKTeX::Core::FileAccess::Write)
#else
#define recorder_record_output(fname) \
  miktex_web2c_record_file_name(fname, 0)
#endif

#define recorder_enabled miktex_web2c_recorder_enabled

#if defined(THEAPP)
#  define output_directory (THEAPP.GetOutputDirectory().Empty() ? 0 : THEAPP.GetOutputDirectory().Get())
#elif defined(__cplusplus)
#  define output_directory (MiKTeX::Web2C::GetOutputDirectory().Empty() ? 0 : MiKTeX::Web2C::GetOutputDirectory().Get())
#else
#  define output_directory miktex_web2c_get_output_directory()
#endif

#define fullnameoffile miktex_web2c_fullnameoffile

#define setupboundvariable(var, var_name, dflt) \
  miktex_setupboundvariable(var, var_name, dflt)

#if defined(__cplusplus)
MIKTEXWEB2C_BEGIN_NAMESPACE;

MIKTEXW2CCEEAPI(void)
ChangeRecorderFileName(const char * lpszName);

MIKTEXW2CCEEAPI(int) OpenInput(char * lpszFileName, FILE ** ppfile, kpse_file_format_type format, const char * lpszMode);

MIKTEXW2CCEEAPI(void) RecordFileName(const char * lpszPath, MiKTeX::Core::FileAccess access);

MIKTEXW2CCEEAPI(MiKTeX::Core::PathName) GetOutputDirectory();

MIKTEXW2CCEEAPI(void) SetOutputDirectory(const MiKTeX::Core::PathName & path);

MIKTEXWEB2C_END_NAMESPACE;
#endif

MIKTEX_BEGIN_EXTERN_C_BLOCK

MIKTEXW2CCEEAPI(void) miktex_web2c_change_recorder_file_name(const char * lpszPath);

MIKTEXW2CCEEAPI(void) miktex_web2c_record_file_name(const char * lpszPath, int reading);

MIKTEXW2CCEEAPI(integer) miktex_zround(double r);

MIKTEXW2CCEEAPI(void) miktex_setupboundvariable(integer * pVar, const char * lpszVarName, integer dflt);

extern MIKTEXW2CDATA(string) miktex_web2c_fullnameoffile;

MIKTEXW2CCEEAPI(const char *) miktex_web2c_get_output_directory();

MIKTEXW2CCEEAPI(void) miktex_web2c_set_output_directory(const char * lpszPath);

extern MIKTEXW2CDATA(boolean) miktex_web2c_recorder_enabled;

extern MIKTEXW2CDATA(const_string) miktex_web2c_version_string;

MIKTEX_END_EXTERN_C_BLOCK

#endif
