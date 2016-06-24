/* inputline.cpp:

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX TeXMF Library.

   The MiKTeX TeXMF Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX TeXMF Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX TeXMF Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"

#include "internal.h"

BEGIN_INTERNAL_NAMESPACE;

bool IsNameManglingEnabled = false;

END_INTERNAL_NAMESPACE;

WebAppInputLine::WebAppInputLine() :
  inputFileType(FileType::PROGRAMTEXTFILE)
{
}

void WebAppInputLine::Init(const char * lpszProgramInvocationName)
{
  WebApp::Init(lpszProgramInvocationName);
  enablePipes = false;
}

void WebAppInputLine::Finalize()
{
  auxDirectory.Clear();
  foundFile.Clear();
  foundFileFq.Clear();
  lastInputFileName.Clear();
  outputDirectory.Clear();
  WebApp::Finalize();
}

enum {
  OPT_DISABLE_PIPES,
  OPT_ENABLE_PIPES,
};

void WebAppInputLine::AddOptions()
{
  WebApp::AddOptions();
  optBase = static_cast<int>(GetOptions().size());
  AddOption(T_("enable-pipes\0Enable input (output) from (to) processes."), FIRST_OPTION_VAL + optBase + OPT_ENABLE_PIPES);
  AddOption(T_("disable-pipes\0Disable input (output) from (to) processes."), FIRST_OPTION_VAL + optBase + OPT_DISABLE_PIPES);
}

bool WebAppInputLine::ProcessOption(int opt, const string & optArg)
{
  bool done = true;

  switch (opt - FIRST_OPTION_VAL - optBase)
  {

  case OPT_DISABLE_PIPES:
    enablePipes = false;
    break;

  case OPT_ENABLE_PIPES:
    enablePipes = true;
    break;

  default:
    done = WebApp::ProcessOption(opt, optArg);
    break;
  }

  return done;
}

PathName WebAppInputLine::MangleNameOfFile(const char * lpszFrom)
{
  if (!IsNameManglingEnabled)
  {
    return PathName(lpszFrom).ToUnix();
  }
  else
  {
    PathName ret;
    char * lpszTo = ret.GetData();
    MIKTEX_ASSERT_STRING(lpszFrom);
    size_t len = StrLen(lpszFrom);
    if (len >= ret.GetCapacity())
    {
      MIKTEX_UNEXPECTED();
    }
    size_t idx;
    for (idx = 0; idx < len; ++idx)
    {
      if (lpszFrom[idx] == ' ')
      {
        lpszTo[idx] = '*';
      }
      else if (lpszFrom[idx] == '~')
      {
        lpszTo[idx] = '?';
      }
      else if (lpszFrom[idx] == '\\')
      {
        lpszTo[idx] = '/';
      }
      else
      {
        lpszTo[idx] = lpszFrom[idx];
      }
    }
    lpszTo[idx] = 0;
    return ret;
  }
}

template<typename CharType> static PathName UnmangleNameOfFile_(const CharType * lpszFrom)
{
  if (!IsNameManglingEnabled)
  {
    return PathName(lpszFrom);
  }
  else
  {
    PathName ret;
    char * lpszTo = ret.GetData();
    MIKTEX_ASSERT_STRING(lpszFrom);
    size_t len = StrLen(lpszFrom);
    if (len >= ret.GetCapacity())
    {
      MIKTEX_UNEXPECTED();
    }
    size_t idx;
    for (idx = 0; idx < len; ++idx)
    {
      if (lpszFrom[idx] == '*')
      {
        lpszTo[idx] = ' ';
      }
      else if (lpszFrom[idx] == '?')
      {
        lpszTo[idx] = '~';
      }
      else
      {
        lpszTo[idx] = lpszFrom[idx];
      }
    }
    lpszTo[idx] = 0;
    return ret;
  }
}

PathName WebAppInputLine::UnmangleNameOfFile(const char * lpszFrom)
{
  return UnmangleNameOfFile_(lpszFrom);
}

PathName WebAppInputLine::UnmangleNameOfFile(const wchar_t * lpszFrom)
{
  return UnmangleNameOfFile_(lpszFrom);
}

static bool IsOutputFile(const PathName & path)
{
  PathName path_(path);
  if (path_.HasExtension(".gz"))
  {
    path_.SetExtension(nullptr);
  }
  return path_.HasExtension(".dvi")
    || path_.HasExtension(".pdf")
    || path_.HasExtension(".synctex");
}

bool WebAppInputLine::AllowFileName(const char * lpszPath, bool forInput)
{
  bool allow;
  if (forInput)
  {
    static Core::TriState allowInput = TriState::Undetermined;
    if (allowInput == TriState::Undetermined)
    {
      allow = session->GetConfigValue(nullptr, "AllowUnsafeInputFiles", true);
      allowInput = allow ? TriState::True : TriState::False;
    }
    else
    {
      allow = allowInput == TriState::True ? true : false;
    }
  }
  else
  {
    static Core::TriState allowOutput = TriState::Undetermined;
    if (allowOutput == TriState::Undetermined)
    {
      allow = session->GetConfigValue(nullptr, "AllowUnsafeOutputFiles", false);
      allowOutput = allow ? TriState::True : TriState::False;
    }
    else
    {
      allow = allowOutput == TriState::True ? true : false;
    }
  }
  if (allow)
  {
    return true;
  }
  return Core::Utils::IsSafeFileName(lpszPath, forInput);
}

bool WebAppInputLine::OpenOutputFile(C4P::FileRoot & f, const char * lpszPath, FileShare share, bool text, PathName & outPath)
{
  MIKTEX_ASSERT_STRING(lpszPath);
#if defined(MIKTEX_WINDOWS)
  string utf8Path;
  if (!Utils::IsUTF8(lpszPath))
  {
    utf8Path = StringUtil::AnsiToUTF8(lpszPath);
    lpszPath = utf8Path.c_str();
  }
#endif
  FILE * pfile = nullptr;
  if (enablePipes && lpszPath[0] == '|')
  {
    pfile = session->OpenFile(lpszPath + 1, FileMode::Command, FileAccess::Write, false);
  }
  else
  {
    PathName unmangled = UnmangleNameOfFile(lpszPath);
    bool isAuxFile = !IsOutputFile(unmangled);
    PathName path;
    if (isAuxFile && !auxDirectory.Empty())
    {
      path.Set(auxDirectory, unmangled);
      lpszPath = path.Get();
    }
    else if (!outputDirectory.Empty())
    {
      path.Set(outputDirectory, unmangled);
      lpszPath = path.Get();
    }
    else
    {
      lpszPath = unmangled.Get();
    }
    pfile = session->TryOpenFile(lpszPath, FileMode::Create, FileAccess::Write, text, share);
    if (pfile != nullptr)
    {
      outPath = lpszPath;
    }
  }
  if (pfile == nullptr)
  {
    return false;
  }
  f.Attach(pfile, true);
  return true;
}

bool WebAppInputLine::OpenInputFile(FILE ** ppFile, const char * lpszFileName)
{
  MIKTEX_ASSERT_STRING(lpszFileName);

#if defined(MIKTEX_WINDOWS)
  string utf8FileName;
  if (!Utils::IsUTF8(lpszFileName))
  {
    utf8FileName = StringUtil::AnsiToUTF8(lpszFileName);
    lpszFileName = utf8FileName.c_str();
  }
#endif

  if (enablePipes && lpszFileName[0] == '|')
  {
    *ppFile = session->OpenFile(lpszFileName + 1, FileMode::Command, FileAccess::Read, false);
    foundFile.Clear();
    foundFileFq.Clear();
  }
  else
  {
    if (!session->FindFile(UnmangleNameOfFile(lpszFileName).Get(), GetInputFileType(), foundFile))
    {
      return false;
    }

    foundFileFq = foundFile;
    foundFileFq.MakeAbsolute();

#if 1 // 2015-01-15
    if (foundFile[0] == '.' && PathName::IsDirectoryDelimiter(foundFile[1]))
    {
      PathName temp(foundFile.Get() + 2);
      foundFile = temp;
    }
#endif

    try
    {
      if (foundFile.HasExtension(".gz"))
      {
        CommandLineBuilder cmd("zcat");
        cmd.AppendArgument(foundFile);
        *ppFile = session->OpenFile(cmd.ToString(), FileMode::Command, FileAccess::Read, false);
      }
      else if (foundFile.HasExtension(".bz2"))
      {
        CommandLineBuilder cmd("bzcat");
        cmd.AppendArgument(foundFile);
        *ppFile = session->OpenFile(cmd.ToString(), FileMode::Command, FileAccess::Read, false);
      }
      else if (foundFile.HasExtension(".xz") || foundFile.HasExtension(".lzma"))
      {
        CommandLineBuilder cmd("xzcat");
        cmd.AppendArgument(foundFile);
        *ppFile = session->OpenFile(cmd.ToString(), FileMode::Command, FileAccess::Read, false);
      }
      else
      {
#if 0
        FileShare share = FileShare::Read;
#else
        FileShare share = FileShare::ReadWrite;
#endif
        *ppFile = session->OpenFile(foundFile.Get(), FileMode::Open, FileAccess::Read, false, share);
      }
    }
#if defined(MIKTEX_WINDOWS)
    catch (const SharingViolationException &)
    {
    }
#endif
    catch (const UnauthorizedAccessException &)
    {
    }
    catch (const FileNotFoundException &)
    {
    }
  }

  if (*ppFile == nullptr)
  {
    return false;
  }

  lastInputFileName = lpszFileName;

  return true;
}

bool WebAppInputLine::OpenInputFile(C4P::FileRoot & f, const char * lpszFileName)
{
  FILE * pFile = nullptr;

  if (!OpenInputFile(&pFile, lpszFileName))
  {
    return false;
  }

  f.Attach(pFile, true);

#ifdef PASCAL_TEXT_IO
  not_implemented();
  get(f);
#endif

  lastInputFileName = lpszFileName;

  return true;
}

#if 0
void WebAppInputLine::HandleEof(FILE * pfile) const
{
  MIKTEX_ASSERT(pfile != nullptr);
#if 1
  while (GetC(pfile) != EOF)
  {
    ;
  }
#else
  fseek(pfile, SEEK_END, 0);
#endif
}
#endif

void WebAppInputLine::TouchJobOutputFile(FILE *) const
{
}
