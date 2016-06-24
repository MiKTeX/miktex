/* miktex/Core/c/api.h: MiKTeX core C API               -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

/* Support utilities which cannot be compiled with a C++ compiler. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(A4210C159BBA4B268A6BB2E90069621A)
#define A4210C159BBA4B268A6BB2E90069621A

#include <miktex/Core/config.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

MIKTEX_BEGIN_EXTERN_C_BLOCK;

MIKTEXCORECEEAPI(void) miktex_create_temp_file_name(char * lpszFileName);

MIKTEXCOREEXPORT MIKTEXNORETURN void MIKTEXCEECALL miktex_exit(int status);

#if defined(USE_MIKTEX_EXIT)
#  define exit(status) miktex_exit(status)
#endif

MIKTEXCORECEEAPI(int) miktex_execute_system_command(const char * command, int * exitCode);

MIKTEXCORECEEAPI(int) miktex_find_input_file(const char * lpszApplicationNames, const char * lpszFileName, char * lpszPath);

MIKTEXCORECEEAPI(int) miktex_find_enc_file(const char * lpszFontName, char * lpszPath);

MIKTEXCORECEEAPI(int) miktex_find_file(const char * lpszFileName, const char * lpszPathList, char * lpszPath);

MIKTEXCORECEEAPI(int) miktex_find_hbf_file(const char * lpszFileName, char * lpszPath);

MIKTEXCORECEEAPI(int) miktex_find_miktex_executable(const char * lpszExeName, char * lpszExePath);

MIKTEXCORECEEAPI(int) miktex_find_psheader_file(const char * lpszHeaderName, char * lpszPath);

MIKTEXCORECEEAPI(int) miktex_find_tfm_file(const char * lpszFontName, char * lpszPath);

MIKTEXCORECEEAPI(int) miktex_find_ttf_file(const char * lpszFontName, char * lpszPath);

MIKTEXCORECEEAPI(unsigned) miktex_get_number_of_texmf_roots();

MIKTEXCORECEEAPI(int) miktex_get_miktex_version_string_ex(char * lpszVersion, size_t maxsize);

MIKTEXCORECEEAPI(char *) miktex_get_root_directory(unsigned r, char * lpszPath);

MIKTEXCORECEEAPI(int) miktex_pathcmp(const char * lpszPath1, const char * lpszPath2);

MIKTEXCORECEEAPI(void) miktex_replace_string(char * lpszBuf, size_t * lpSzeDest, const char * lpszSource, const char * lpszString1, const char * lpszString2);

MIKTEXCORECEEAPI(void) miktex_start_process(const char * lpszFileName, const char * lpszArguments, FILE * pFileStandardInput, FILE ** ppFileStandardInput, FILE ** ppFileStandardOutput, FILE ** ppFileStandardError, const char * lpszWorkingDirectory);

MIKTEXCORECEEAPI(void) miktex_uncompress_file(const char * lpszPathIn, char * lpszPathOut);

MIKTEXCORECEEAPI(wchar_t *) miktex_utf8_to_wide_char(const char * lpszUtf8, size_t sizeWideChar, wchar_t * lpszWideChar);

MIKTEXCORECEEAPI(char*) miktex_wide_char_to_utf8(const wchar_t * lpszWideChar, size_t sizeUtf8, char * lpszUtf8);

MIKTEX_END_EXTERN_C_BLOCK;

#endif
