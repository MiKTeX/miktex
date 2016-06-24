/* api.cpp: C API

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/BufferSizes.h"
#include "miktex/Core/Debug.h"
#include "miktex/Core/PathName.h"
#include "miktex/Core/Paths.h"
#include "miktex/Core/c/api.h"

#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

MIKTEXCORECEEAPI(void) miktex_create_temp_file_name(char * lpszFileName)
{
  C_FUNC_BEGIN();
  MIKTEX_ASSERT_PATH_BUFFER(lpszFileName);
  StringUtil::CopyString(lpszFileName, BufferSizes::MaxPath, PathName().SetToTempFile().Get());
  C_FUNC_END();
}

MIKTEXCORECEEAPI(void) miktex_uncompress_file(const char * lpszPathIn, char * lpszPathOut)
{
  C_FUNC_BEGIN();
  PathName temp;
  Utils::UncompressFile(lpszPathIn, temp);
  StringUtil::CopyString(lpszPathOut, BufferSizes::MaxPath, temp.Get());
  C_FUNC_END();
}

MIKTEXCORECEEAPI(void) miktex_replace_string(char * lpszBuf, size_t * lpSizeDest, const char * lpszSource, const char * lpszString1, const char * lpszString2)
{
  C_FUNC_BEGIN();
  MIKTEX_ASSERT(lpSizeDest != 0);
  StringUtil::ReplaceString(lpszBuf, *lpSizeDest, lpszSource, lpszString1, lpszString2);
  C_FUNC_END();
}

MIKTEXCORECEEAPI(void *) miktex_core_malloc(size_t size, const char * lpszFileName, int line)
{
  C_FUNC_BEGIN();
  return MiKTeX::Debug::Malloc(size, lpszFileName, line);
  C_FUNC_END();
}

MIKTEXCORECEEAPI(void) miktex_core_free(void * ptr, const char * lpszFileName, int line)
{
  C_FUNC_BEGIN();
  MiKTeX::Debug::Free(ptr, lpszFileName, line);
  C_FUNC_END();
}

MIKTEXCORECEEAPI(void *) miktex_core_calloc(size_t num, size_t size, const char * lpszFileName, int line)
{
  C_FUNC_BEGIN();
  return MiKTeX::Debug::Calloc(num, size, lpszFileName, line);
  C_FUNC_END();
}

MIKTEXCORECEEAPI(void *) miktex_core_realloc(void * ptr, size_t size, const char * lpszFileName, int line)
{
  C_FUNC_BEGIN();
  return MiKTeX::Debug::Realloc(ptr, size, lpszFileName, line);
  C_FUNC_END();
}

MIKTEXCORECEEAPI(char *) miktex_core_strdup(const char * lpsz, const char * lpszFileName, int line)
{
  C_FUNC_BEGIN();
  return MiKTeX::Debug::StrDup(lpsz, lpszFileName, line);
  C_FUNC_END();
}

MIKTEXCORECEEAPI(int) miktex_pathcmp(const char * lpszPath1, const char * lpszPath2)
{
  C_FUNC_BEGIN();
  return PathName::Compare(lpszPath1, lpszPath2);
  C_FUNC_END();
}


MIKTEXCOREEXPORT MIKTEXNORETURN void MIKTEXCEECALL miktex_exit(int status)
{
  throw status;
}

MIKTEXCORECEEAPI(void) miktex_core_fatal_error(const char * lpszMiktexFunction, const char * lpszMessage, const char * lpszInfo, const char * lpszSourceFile, int sourceLine)
{
  C_FUNC_BEGIN();
  Session::FatalMiKTeXError(lpszMessage, MiKTeXException::KVMAP("", lpszInfo == nullptr ? "" : lpszInfo), SourceLocation(lpszMiktexFunction == nullptr ? "" : lpszMiktexFunction, lpszSourceFile == nullptr ? "" : lpszSourceFile, sourceLine));
  C_FUNC_END();
}

MIKTEXCORECEEAPI(int) miktex_get_miktex_version_string_ex(char * lpszVersion, size_t bufSize)
{
  C_FUNC_BEGIN();
  string version = Utils::GetMiKTeXVersionString();
  StringUtil::CopyString(lpszVersion, bufSize, version.c_str());
  return 1;
  C_FUNC_END();
}

MIKTEXCORECEEAPI(int) miktex_find_file(const char * lpszFileName, const char * lpszPathList, char * lpszPath)
{
  C_FUNC_BEGIN();
  MIKTEX_ASSERT_STRING(lpszFileName);
  MIKTEX_ASSERT_STRING(lpszPathList);
  MIKTEX_ASSERT_PATH_BUFFER(lpszPath);
  PathName temp;
  if (!SessionImpl::GetSession()->FindFile(lpszFileName, lpszPathList, temp))
  {
    return 0;
  }
  StringUtil::CopyString(lpszPath, BufferSizes::MaxPath, temp.Get());
  return 1;
  C_FUNC_END();
}

MIKTEXCORECEEAPI(int) miktex_find_tfm_file(const char * lpszFontName, char * lpszPath)
{
  C_FUNC_BEGIN();
  MIKTEX_ASSERT_STRING(lpszFontName);
  MIKTEX_ASSERT_PATH_BUFFER(lpszPath);
  PathName temp;
  if (!SessionImpl::GetSession()->FindFile(lpszFontName, FileType::TFM, temp))
  {
    return 0;
  }
  StringUtil::CopyString(lpszPath, BufferSizes::MaxPath, temp.Get());
  return 1;
  C_FUNC_END();
}

MIKTEXCORECEEAPI(int) miktex_find_ttf_file(const char * lpszFontName, char * lpszPath)
{
  C_FUNC_BEGIN();
  MIKTEX_ASSERT_STRING(lpszFontName);
  MIKTEX_ASSERT_PATH_BUFFER(lpszPath);
  PathName temp;
  if (!SessionImpl::GetSession()->FindFile(lpszFontName, FileType::TTF, temp))
  {
    return 0;
  }
  StringUtil::CopyString(lpszPath, BufferSizes::MaxPath, temp.Get());
  return 1;
  C_FUNC_END();
}

MIKTEXCORECEEAPI(int) miktex_find_enc_file(const char * lpszFontName, char * lpszPath)
{
  C_FUNC_BEGIN();
  MIKTEX_ASSERT_STRING(lpszFontName);
  MIKTEX_ASSERT_PATH_BUFFER(lpszPath);
  PathName temp;
  if (!SessionImpl::GetSession()->FindFile(lpszFontName, FileType::ENC, temp))
  {
    return 0;
  }
  StringUtil::CopyString(lpszPath, BufferSizes::MaxPath, temp.Get());
  return 1;
  C_FUNC_END();
}

MIKTEXCORECEEAPI(int) miktex_find_psheader_file(const char * lpszHeaderName, char * lpszPath)
{
  C_FUNC_BEGIN();
  MIKTEX_ASSERT_STRING(lpszHeaderName);
  MIKTEX_ASSERT_PATH_BUFFER(lpszPath);
  PathName temp;
  if (!SessionImpl::GetSession()->FindFile(lpszHeaderName, FileType::PSHEADER, temp))
  {
    return 0;
  }
  StringUtil::CopyString(lpszPath, BufferSizes::MaxPath, temp.Get());
  return 1;
  C_FUNC_END();
}

MIKTEXCORECEEAPI(int) miktex_find_input_file(const char * lpszApplicationName, const char * lpszFileName, char * lpszPath)
{
  C_FUNC_BEGIN();
  MIKTEX_ASSERT_STRING_OR_NIL(lpszApplicationName);
  MIKTEX_ASSERT_STRING(lpszFileName);
  MIKTEX_ASSERT_PATH_BUFFER(lpszPath);
  PathName temp;
  if (!SessionImpl::GetSession()->FindFile(lpszFileName, FileType::None, temp))
  {
    if (lpszApplicationName == nullptr)
    {
      return 0;
    }
    string searchPath = CURRENT_DIRECTORY;
    searchPath += PATH_DELIMITER;
    searchPath += TEXMF_PLACEHOLDER;
    searchPath += MIKTEX_PATH_DIRECTORY_DELIMITER_STRING;
    searchPath += lpszApplicationName;
    searchPath += RECURSION_INDICATOR;
    if (!SessionImpl::GetSession()->FindFile(lpszFileName, searchPath.c_str(), temp))
    {
      return 0;
    }
  }
  StringUtil::CopyString(lpszPath, BufferSizes::MaxPath, temp.Get());
  return 1;
  C_FUNC_END();
}

MIKTEXCORECEEAPI(int) miktex_find_hbf_file(const char * lpszFontName, char * lpszPath)
{
  C_FUNC_BEGIN();
  MIKTEX_ASSERT_STRING(lpszFontName);
  MIKTEX_ASSERT_PATH_BUFFER(lpszPath);
  PathName temp;
  if (!SessionImpl::GetSession()->FindFile(lpszFontName, FileType::HBF, temp))
  {
    return 0;
  }
  StringUtil::CopyString(lpszPath, BufferSizes::MaxPath, temp.Get());
  return 1;
  C_FUNC_END();
}

MIKTEXCORECEEAPI(int) miktex_find_miktex_executable(const char * lpszExeName, char * lpszExePath)
{
  C_FUNC_BEGIN();
  MIKTEX_ASSERT_STRING(lpszExeName);
  MIKTEX_ASSERT_PATH_BUFFER(lpszExePath);
  PathName temp;
  if (!SessionImpl::GetSession()->FindFile(lpszExeName, FileType::EXE, temp))
  {
    return 0;
  }
  StringUtil::CopyString(lpszExePath, BufferSizes::MaxPath, temp.Get());
  return 1;
  C_FUNC_END();
}

MIKTEXCORECEEAPI(unsigned) miktex_get_number_of_texmf_roots()
{
  C_FUNC_BEGIN();
  return SessionImpl::GetSession()->GetNumberOfTEXMFRoots();
  C_FUNC_END();
}

MIKTEXCORECEEAPI(char *) miktex_get_root_directory(unsigned r, char * lpszPath)
{
  C_FUNC_BEGIN();
  MIKTEX_ASSERT_PATH_BUFFER(lpszPath);
  StringUtil::CopyString(lpszPath, BufferSizes::MaxPath, SessionImpl::GetSession()->GetRootDirectory(r).Get());
  return lpszPath;
  C_FUNC_END();
}

MIKTEXCORECEEAPI(wchar_t*) miktex_utf8_to_wide_char(const char * lpszUtf8, size_t sizeWideChar, wchar_t * lpszWideChar)
{
  C_FUNC_BEGIN();
  StringUtil::CopyString(lpszWideChar, sizeWideChar, lpszUtf8);
  return lpszWideChar;
  C_FUNC_END();
}

MIKTEXCORECEEAPI(char*) miktex_wide_char_to_utf8(const wchar_t * lpszWideChar, size_t sizeUtf8, char * lpszUtf8)
{
  C_FUNC_BEGIN();
  StringUtil::CopyString(lpszUtf8, sizeUtf8, lpszWideChar);
  return lpszUtf8;
  C_FUNC_END();
}

MIKTEXCORECEEAPI(int) miktex_execute_system_command(const char * command, int * exitCode)
{
  C_FUNC_BEGIN();
  return Process::ExecuteSystemCommand(command, exitCode) ? 1 : 0;
  C_FUNC_END();
}


MIKTEXCORECEEAPI(void) miktex_start_process(const char * lpszFileName, const char * lpszArguments, FILE * pFileStandardInput, FILE ** ppFileStandardInput, FILE ** ppFileStandardOutput, FILE ** ppFileStandardError, const char * lpszWorkingDirectory)
{
  C_FUNC_BEGIN();
  Process::Start(lpszFileName, lpszArguments == nullptr ? "" : lpszArguments, pFileStandardInput, ppFileStandardInput, ppFileStandardOutput, ppFileStandardError, lpszWorkingDirectory);
  C_FUNC_END();
}
