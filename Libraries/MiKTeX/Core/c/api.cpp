/* api.cpp: C API

   Copyright (C) 1996-2020 Christian Schenk

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

#include "config.h"

#include <miktex/Core/BufferSizes>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Debug>
#include <miktex/Core/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/c/api.h>

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

MIKTEXCORECEEAPI(void) miktex_create_temp_file_name(char* fileName)
{
  MIKTEX_ASSERT_PATH_BUFFER(fileName);
  StringUtil::CopyString(fileName, BufferSizes::MaxPath, PathName().SetToTempFile().GetData());
}

MIKTEXCORECEEAPI(void) miktex_uncompress_file(const char* pathIn, char* pathOut)
{
  PathName temp;
  Utils::UncompressFile(PathName(pathIn), temp);
  StringUtil::CopyString(pathOut, BufferSizes::MaxPath, temp.GetData());
}

MIKTEXCORECEEAPI(void*) miktex_core_malloc(size_t size, const char* fileName, int line)
{
#if defined(MIKTEX_DEBUG)
  return MiKTeX::Debug::Malloc(size, SourceLocation("", fileName == nullptr ? "" : fileName, line));
#else
  return MiKTeX::Debug::Malloc(size, SourceLocation());
#endif
}

MIKTEXCORECEEAPI(void) miktex_core_free(void* ptr, const char* fileName, int line)
{
#if defined(MIKTEX_DEBUG)
  MiKTeX::Debug::Free(ptr, SourceLocation("", fileName == nullptr ? "" : fileName, line));
#else
  MiKTeX::Debug::Free(ptr, SourceLocation());
#endif
}

MIKTEXCORECEEAPI(void*) miktex_core_calloc(size_t num, size_t size, const char* fileName, int line)
{
#if defined(MIKTEX_DEBUG)
  return MiKTeX::Debug::Calloc(num, size, SourceLocation("", fileName == nullptr ? "" : fileName, line));
#else
  return MiKTeX::Debug::Calloc(num, size, SourceLocation());
#endif
}

MIKTEXCORECEEAPI(void*) miktex_core_realloc(void* ptr, size_t size, const char* fileName, int line)
{
#if defined(MIKTEX_DEBUG)
  return MiKTeX::Debug::Realloc(ptr, size, SourceLocation("", fileName == nullptr ? "" : fileName, line));
#else
  return MiKTeX::Debug::Realloc(ptr, size, SourceLocation());
#endif
}

#if defined(_MSC_VER)
#  pragma push_macro("StrDup")
#  undef StrDup
#endif

MIKTEXCORECEEAPI(char*) miktex_core_strdup(const char* lpsz, const char* fileName, int line)
{
#if defined(MIKTEX_DEBUG)
  return MiKTeX::Debug::StrDup(lpsz, SourceLocation("", fileName == nullptr ? "" : fileName, line));
#else
  return MiKTeX::Debug::StrDup(lpsz, SourceLocation());
#endif
}

#if defined(_MSC_VER)
#  pragma pop_macro("StrDup")
#endif

MIKTEXCORECEEAPI(int) miktex_pathcmp(const char* path1, const char* path2)
{
  return PathName::Compare(path1, path2);
}

MIKTEXCOREEXPORT MIKTEXNORETURN void MIKTEXCEECALL miktex_exit(int status)
{
  throw status;
}

MIKTEXCORECEEAPI(void) miktex_core_fatal_error(const char* miktexFunction, const char* message, const char* info, const char* sourceFile, int sourceLine)
{
  Session::FatalMiKTeXError(message, "", "", "", MiKTeXException::KVMAP("", info == nullptr ? "" : info), SourceLocation(miktexFunction == nullptr ? "" : miktexFunction, sourceFile == nullptr ? "" : sourceFile, sourceLine));
}

MIKTEXCORECEEAPI(int) miktex_get_miktex_banner(char* buf, size_t bufSize)
{
  string banner = Utils::GetMiKTeXBannerString();
  StringUtil::CopyString(buf, bufSize, banner.c_str());
  return 1;
}

MIKTEXCORECEEAPI(int) miktex_get_miktex_version_string_ex(char* version, size_t bufSize)
{
  StringUtil::CopyString(version, bufSize, Utils::GetMiKTeXVersionString().c_str());
  return 1;
}

MIKTEXCORECEEAPI(int) miktex_find_file(const char* fileName, const char* pathList, char* path)
{
  MIKTEX_ASSERT_STRING(fileName);
  MIKTEX_ASSERT_STRING(pathList);
  MIKTEX_ASSERT_PATH_BUFFER(path);
  PathName temp;
  if (!SessionImpl::GetSession()->FindFile(fileName, pathList, temp))
  {
    return 0;
  }
  StringUtil::CopyString(path, BufferSizes::MaxPath, temp.GetData());
  return 1;
}

MIKTEXCORECEEAPI(int) miktex_find_tfm_file(const char* fontName, char* path)
{
  MIKTEX_ASSERT_STRING(fontName);
  MIKTEX_ASSERT_PATH_BUFFER(path);
  PathName temp;
  if (!SessionImpl::GetSession()->FindFile(fontName, FileType::TFM, temp))
  {
    return 0;
  }
  StringUtil::CopyString(path, BufferSizes::MaxPath, temp.GetData());
  return 1;
}

MIKTEXCORECEEAPI(int) miktex_find_ttf_file(const char* fontName, char* path)
{
  MIKTEX_ASSERT_STRING(fontName);
  MIKTEX_ASSERT_PATH_BUFFER(path);
  PathName temp;
  if (!SessionImpl::GetSession()->FindFile(fontName, FileType::TTF, temp))
  {
    return 0;
  }
  StringUtil::CopyString(path, BufferSizes::MaxPath, temp.GetData());
  return 1;
}

MIKTEXCORECEEAPI(int) miktex_find_enc_file(const char* fontName, char* path)
{
  MIKTEX_ASSERT_STRING(fontName);
  MIKTEX_ASSERT_PATH_BUFFER(path);
  PathName temp;
  if (!SessionImpl::GetSession()->FindFile(fontName, FileType::ENC, temp))
  {
    return 0;
  }
  StringUtil::CopyString(path, BufferSizes::MaxPath, temp.GetData());
  return 1;
}

MIKTEXCORECEEAPI(int) miktex_find_psheader_file(const char* headerName, char* path)
{
  MIKTEX_ASSERT_STRING(headerName);
  MIKTEX_ASSERT_PATH_BUFFER(path);
  PathName temp;
  if (!SessionImpl::GetSession()->FindFile(headerName, FileType::PSHEADER, temp))
  {
    return 0;
  }
  StringUtil::CopyString(path, BufferSizes::MaxPath, temp.GetData());
  return 1;
}

MIKTEXCORECEEAPI(int) miktex_find_input_file(const char* applicationName, const char* fileName, char* path)
{
  MIKTEX_ASSERT_STRING_OR_NIL(applicationName);
  MIKTEX_ASSERT_STRING(fileName);
  MIKTEX_ASSERT_PATH_BUFFER(path);
  PathName temp;
  if (!SessionImpl::GetSession()->FindFile(fileName, FileType::None, temp))
  {
    if (applicationName == nullptr)
    {
      return 0;
    }
    string searchPath = CURRENT_DIRECTORY;
    searchPath += PATH_DELIMITER;
    searchPath += TEXMF_PLACEHOLDER;
    searchPath += MIKTEX_PATH_DIRECTORY_DELIMITER_STRING;
    searchPath += applicationName;
    searchPath += RECURSION_INDICATOR;
    if (!SessionImpl::GetSession()->FindFile(fileName, searchPath, temp))
    {
      return 0;
    }
  }
  StringUtil::CopyString(path, BufferSizes::MaxPath, temp.GetData());
  return 1;
}

MIKTEXCORECEEAPI(int) miktex_find_hbf_file(const char* fontName, char* path)
{
  MIKTEX_ASSERT_STRING(fontName);
  MIKTEX_ASSERT_PATH_BUFFER(path);
  PathName temp;
  if (!SessionImpl::GetSession()->FindFile(fontName, FileType::HBF, temp))
  {
    return 0;
  }
  StringUtil::CopyString(path, BufferSizes::MaxPath, temp.GetData());
  return 1;
}

MIKTEXCORECEEAPI(int) miktex_find_miktex_executable(const char* exeName, char* exePath)
{
  MIKTEX_ASSERT_STRING(exeName);
  MIKTEX_ASSERT_PATH_BUFFER(exePath);
  PathName temp;
  if (!SessionImpl::GetSession()->FindFile(exeName, FileType::EXE, temp))
  {
    return 0;
  }
  StringUtil::CopyString(exePath, BufferSizes::MaxPath, temp.GetData());
  return 1;
}

MIKTEXCORECEEAPI(unsigned) miktex_get_number_of_texmf_roots()
{
  return SessionImpl::GetSession()->GetNumberOfTEXMFRoots();
}

MIKTEXCORECEEAPI(char*) miktex_get_root_directory(unsigned r, char* path)
{
  MIKTEX_ASSERT_PATH_BUFFER(path);
  StringUtil::CopyString(path, BufferSizes::MaxPath, SessionImpl::GetSession()->GetRootDirectoryPath(r).GetData());
  return path;
}

MIKTEXCORECEEAPI(wchar_t*) miktex_utf8_to_wide_char(const char* utf8, size_t sizeWideChar, wchar_t* wideChar)
{
  StringUtil::CopyString(wideChar, sizeWideChar, utf8);
  return wideChar;
}

MIKTEXCORECEEAPI(char*) miktex_wide_char_to_utf8(const wchar_t* wideChar, size_t sizeUtf8, char* utf8)
{
  StringUtil::CopyString(utf8, sizeUtf8, wideChar);
  return utf8;
}

MIKTEXCORECEEAPI(int) miktex_execute_system_command(const char* command, int* exitCode)
{
  return Process::ExecuteSystemCommand(command, exitCode) ? 1 : 0;
}


MIKTEXCORECEEAPI(void) miktex_start_process(const char* fileName, const char* commandLine, FILE* pFileStandardInput, FILE** ppFileStandardInput, FILE** ppFileStandardOutput, FILE** ppFileStandardError, const char* workingDirectory)
{
  MIKTEX_ASSERT_STRING(commandLine);
  Process::Start(PathName(fileName), Argv(commandLine).ToStringVector(), pFileStandardInput, ppFileStandardInput, ppFileStandardOutput, ppFileStandardError, workingDirectory);
}
