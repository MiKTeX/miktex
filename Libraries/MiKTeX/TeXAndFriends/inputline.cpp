/* inputline.cpp:

   Copyright (C) 1996-2017 Christian Schenk

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

class WebAppInputLine::impl
{
public:
  PathName outputDirectory;
public:
  PathName auxDirectory;
public:
  int optBase;
public:
  PathName foundFile;
public:
  PathName foundFileFq;
public:
  bool enablePipes;
public:
  PathName lastInputFileName;
public:
  IInputOutput* inputOutput = nullptr;
public:
  TriState allowInput = TriState::Undetermined;
public:
  TriState allowOutput = TriState::Undetermined;
};

WebAppInputLine::WebAppInputLine() :
  pimpl(make_unique<impl>())
{
}

WebAppInputLine::~WebAppInputLine() noexcept
{
}

void WebAppInputLine::Init(const string& programInvocationName)
{
  WebApp::Init(programInvocationName);
  pimpl->enablePipes = false;
}

void WebAppInputLine::Finalize()
{
  pimpl->foundFile.Clear();
  pimpl->foundFileFq.Clear();
  pimpl->lastInputFileName.Clear();
  pimpl->outputDirectory.Clear();
  pimpl->auxDirectory.Clear();
  WebApp::Finalize();
}

enum {
  OPT_DISABLE_PIPES,
  OPT_ENABLE_PIPES,
};

void WebAppInputLine::AddOptions()
{
  WebApp::AddOptions();
  pimpl->optBase = (int)GetOptions().size();
  AddOption(T_("enable-pipes\0Enable input (output) from (to) processes."), FIRST_OPTION_VAL + pimpl->optBase + OPT_ENABLE_PIPES);
  AddOption(T_("disable-pipes\0Disable input (output) from (to) processes."), FIRST_OPTION_VAL + pimpl->optBase + OPT_DISABLE_PIPES);
}

bool WebAppInputLine::ProcessOption(int opt, const string& optArg)
{
  bool done = true;

  switch (opt - FIRST_OPTION_VAL - pimpl->optBase)
  {

  case OPT_DISABLE_PIPES:
    pimpl->enablePipes = false;
    break;

  case OPT_ENABLE_PIPES:
    pimpl->enablePipes = true;
    break;

  default:
    done = WebApp::ProcessOption(opt, optArg);
    break;
  }

  return done;
}

#if defined(WITH_OMEGA)
PathName WebAppInputLine::MangleNameOfFile(const char* lpszFrom)
{
  PathName ret;
  char* lpszTo = ret.GetData();
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
#endif

#if defined(WITH_OMEGA)
template<typename CharType> static PathName UnmangleNameOfFile_(const CharType* lpszFrom)
{
  PathName ret;
  char* lpszTo = ret.GetData();
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
#endif

#if defined(WITH_OMEGA)
PathName WebAppInputLine::UnmangleNameOfFile(const char* lpszFrom)
{
  return UnmangleNameOfFile_(lpszFrom);
}
#endif

static bool IsOutputFile(const PathName& path)
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

bool WebAppInputLine::AllowFileName(const PathName& fileName, bool forInput)
{
  shared_ptr<Session> session = GetSession();
  bool allow;
  if (forInput)
  {
    if (pimpl->allowInput == TriState::Undetermined)
    {
      allow = session->GetConfigValue("", "AllowUnsafeInputFiles", true).GetBool();
      pimpl->allowInput = allow ? TriState::True : TriState::False;
    }
    else
    {
      allow = pimpl->allowInput == TriState::True ? true : false;
    }
  }
  else
  {
    if (pimpl->allowOutput == TriState::Undetermined)
    {
      allow = session->GetConfigValue("", "AllowUnsafeOutputFiles", false).GetBool();
      pimpl->allowOutput = allow ? TriState::True : TriState::False;
    }
    else
    {
      allow = pimpl->allowOutput == TriState::True ? true : false;
    }
  }
  if (allow)
  {
    return true;
  }
  return Utils::IsSafeFileName(fileName, forInput);
}

bool WebAppInputLine::OpenOutputFile(C4P::FileRoot& f, const PathName& fileName, FileShare share, bool text, PathName& outPath)
{
  const char* lpszPath = fileName.GetData();
#if defined(MIKTEX_WINDOWS)
  string utf8Path;
  if (!Utils::IsUTF8(lpszPath))
  {
    utf8Path = StringUtil::AnsiToUTF8(lpszPath);
    lpszPath = utf8Path.c_str();
  }
#endif
  shared_ptr<Session> session = GetSession();
  FILE* file = nullptr;
  if (pimpl->enablePipes && lpszPath[0] == '|')
  {
    file = session->OpenFile(lpszPath + 1, FileMode::Command, FileAccess::Write, false);
  }
  else
  {
#if defined(WITH_OMEGA)
    PathName unmangled;
    if (AmI("omega"))
    {
      unmangled = UnmangleNameOfFile(lpszPath);
      lpszPath = unmangled.GetData();
    }
#endif
    bool isAuxFile = !IsOutputFile(lpszPath);
    PathName path;
    if (isAuxFile && !pimpl->auxDirectory.Empty())
    {
      path = pimpl->auxDirectory / lpszPath;
      lpszPath = path.GetData();
    }
    else if (!pimpl->outputDirectory.Empty())
    {
      path = pimpl->outputDirectory / lpszPath;
      lpszPath = path.GetData();
    }
    file = session->TryOpenFile(lpszPath, FileMode::Create, FileAccess::Write, text, share);
    if (file != nullptr)
    {
      outPath = lpszPath;
    }
  }
  if (file == nullptr)
  {
    return false;
  }
  f.Attach(file, true);
  return true;
}

bool WebAppInputLine::OpenInputFile(FILE** ppFile, const PathName& fileName)
{
  const char* lpszFileName = fileName.GetData();

#if defined(MIKTEX_WINDOWS)
  string utf8FileName;
  if (!Utils::IsUTF8(lpszFileName))
  {
    utf8FileName = StringUtil::AnsiToUTF8(lpszFileName);
    lpszFileName = utf8FileName.c_str();
  }
#endif

  shared_ptr<Session> session = GetSession();

  if (pimpl->enablePipes && lpszFileName[0] == '|')
  {
    *ppFile = session->OpenFile(lpszFileName + 1, FileMode::Command, FileAccess::Read, false);
    pimpl->foundFile.Clear();
    pimpl->foundFileFq.Clear();
  }
  else
  {
#if defined(WITH_OMEGA)
    PathName unmangled;
    if (AmI("omega"))
    {
      unmangled = UnmangleNameOfFile(lpszFileName);
      lpszFileName = unmangled.GetData();
    }
#endif

    if (!session->FindFile(lpszFileName, GetInputFileType(), pimpl->foundFile))
    {
      return false;
    }

    pimpl->foundFileFq = pimpl->foundFile;
    pimpl->foundFileFq.MakeAbsolute();

#if 1 // 2015-01-15
    if (pimpl->foundFile[0] == '.' && PathName::IsDirectoryDelimiter(pimpl->foundFile[1]))
    {
      PathName temp(pimpl->foundFile.GetData() + 2);
      pimpl->foundFile = temp;
    }
#endif

    try
    {
      if (pimpl->foundFile.HasExtension(".gz"))
      {
        CommandLineBuilder cmd("zcat");
        cmd.AppendArgument(pimpl->foundFile);
        *ppFile = session->OpenFile(cmd.ToString(), FileMode::Command, FileAccess::Read, false);
      }
      else if (pimpl->foundFile.HasExtension(".bz2"))
      {
        CommandLineBuilder cmd("bzcat");
        cmd.AppendArgument(pimpl->foundFile);
        *ppFile = session->OpenFile(cmd.ToString(), FileMode::Command, FileAccess::Read, false);
      }
      else if (pimpl->foundFile.HasExtension(".xz") || pimpl->foundFile.HasExtension(".lzma"))
      {
        CommandLineBuilder cmd("xzcat");
        cmd.AppendArgument(pimpl->foundFile);
        *ppFile = session->OpenFile(cmd.ToString(), FileMode::Command, FileAccess::Read, false);
      }
      else
      {
        FileShare share = FileShare::ReadWrite;
        *ppFile = session->OpenFile(pimpl->foundFile.GetData(), FileMode::Open, FileAccess::Read, false, share);
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

  pimpl->lastInputFileName = lpszFileName;

  return true;
}

bool WebAppInputLine::OpenInputFile(C4P::FileRoot& f, const PathName& fileName)
{
  FILE* pFile = nullptr;

  if (!OpenInputFile(&pFile, fileName))
  {
    return false;
  }

  f.Attach(pFile, true);

#ifdef PASCAL_TEXT_IO
  not_implemented();
  get(f);
#endif

  pimpl->lastInputFileName = fileName;

  return true;
}

void WebAppInputLine::TouchJobOutputFile(FILE *) const
{
}

void WebAppInputLine::SetOutputDirectory(const PathName& path)
{
  pimpl->outputDirectory = path;
}

PathName WebAppInputLine::GetOutputDirectory() const
{
  return pimpl->outputDirectory;
}

void WebAppInputLine::SetAuxDirectory(const PathName& path)
{
  pimpl->auxDirectory = path;
}

PathName WebAppInputLine::GetAuxDirectory() const
{
  return pimpl->auxDirectory;
}

PathName WebAppInputLine::GetFoundFile() const
{
  return pimpl->foundFile;
}

PathName WebAppInputLine::GetFoundFileFq() const
{
  return pimpl->foundFileFq;
}

void WebAppInputLine::EnablePipes(bool f)
{
  pimpl->enablePipes = f;
}

PathName WebAppInputLine::GetLastInputFileName() const
{
  return pimpl->lastInputFileName;
}

void WebAppInputLine::SetInputOutput(IInputOutput* inputOutput)
{
  pimpl->inputOutput = inputOutput;
}

IInputOutput* WebAppInputLine::GetInputOutput() const
{
  return pimpl->inputOutput;
}

void WebAppInputLine::BufferSizeExceeded() const
{
  if (GetFormatIdent() == 0)
  {
    fputs("Buffer size exceeded!", stdout);
    throw new C4P::Exception9999;
  }
  else
  {
    IInputOutput* inputOutput = GetInputOutput();
    inputOutput->loc() = inputOutput->first();
    inputOutput->limit() = inputOutput->last() - 1;
    inputOutput->overflow(256, inputOutput->bufsize());
  }
}

inline int GetCharacter(FILE* file)
{
  MIKTEX_ASSERT(file != nullptr);
  int ch = getc(file);
  if (ch == EOF)
  {
    if (ferror(file) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR("getc");
    }
  }
  return ch;
}

struct Bom
{
public:
  static constexpr int UTF8 = 0xbfbbef;
public:
  static constexpr int UTF8_length = 3;
public:
  static constexpr int UTF8_mask = 0xffffff;
public:
  static constexpr int UTF16le = 0xfeff;
public:
  static constexpr int UTF16be = 0xfffe;
public:
  static constexpr int UTF16_length = 2;
public:
  static constexpr int UTF16_mask = 0xffff;
};

void Seek(FILE* file, int pos)
{
  if (fseek(file, pos, SEEK_SET) < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fseek");
  }
}

void CheckBom(FILE* file)
{
  long filePosition = ftell(file);
  if (filePosition < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("ftell");
  }
  if (filePosition > 0)
  {
    return;
  }
  int val = 0;
  MIKTEX_ASSERT(Bom::UTF8_length >= Bom::UTF16_length);
  int n = fread(&val, 1, Bom::UTF8_length, file);
  if (n < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("fread");
  }
  bool maybeUtf16bom = false;
  if (n == Bom::UTF8_length)
  {
    int bom = val & Bom::UTF8_mask;
    if (bom == Bom::UTF8)
    {
      // TODO: logging
      return;
    }
    else
    {
      maybeUtf16bom = true;
    }
  }
  if (n == Bom::UTF16_length || maybeUtf16bom)
  {
    int bom = val & Bom::UTF16_mask;
    if (bom == Bom::UTF16be || bom == Bom::UTF16le)
    {
      // TODO: logging
      Seek(file, Bom::UTF16_length);
      return;
    }
  }
  Seek(file, 0);
}

bool WebAppInputLine::InputLine(C4P_text& f, C4P_boolean bypassEndOfLine) const
{
  f.AssertValid();

  if (AmI("xetex"))
  {
    MIKTEX_UNEXPECTED();
  }

#if defined(PASCAL_TEXT_IO)
  MIKTEX_UNEXPECTED();
#endif

  if (feof(f) != 0)
  {
    return false;
  }

  CheckBom(f);

  IInputOutput* inputOutput = GetInputOutput();

  const C4P_signed32 first = inputOutput->first();
  C4P_signed32& last = inputOutput->last();
  const C4P_signed32 bufsize = inputOutput->bufsize();

  const char* xord = nullptr;
#if defined(WITH_OMEGA)
  if (!AmI("omega"))
#endif
  {
    xord = GetCharacterConverter()->xord();
  }

  char *buffer = nullptr;
#if defined(WITH_OMEGA)
  char16_t* buffer16 = nullptr;
  if (AmI("omega"))
  {
    buffer16 = inputOutput->buffer16();
  }
  else
#endif
  {
    buffer = inputOutput->buffer();
  }

  last = first;

  int ch = GetCharacter(f);
  if (ch == EOF)
  {
    return false;
  }
  if (ch == '\r')
  {
    ch = GetCharacter(f);
    if (ch == EOF)
    {
      return false;
    }
    if (ch != '\n')
    {
      ungetc(ch, f);
      ch = '\n';
    }
  }

  if (ch == '\n')
  {
    return true;
  }

#if defined(WITH_OMEGA)
  if (AmI("omega"))
  {
    buffer16[last] = ch;
  }
  else
#endif
  {
    buffer[last] = xord[ch & 0xff];
  }
  last += 1;

  while ((ch = GetCharacter(f)) != EOF && last < bufsize)
  {
    if (ch == '\r')
    {
      ch = GetCharacter(f);
      if (ch == EOF)
      {
        break;
      }
      if (ch != '\n')
      {
        ungetc(ch, f);
        ch = '\n';
      }
    }
    if (ch == '\n')
    {
      break;
    }
#if defined(WITH_OMEGA)
    if (AmI("omega"))
    {
      buffer16[last] = ch;
    }
    else
#endif
    {
      buffer[last] = xord[ch & 0xff];
    }
    last += 1;
  }

  if (ch != '\n' && ch != EOF)
  {
    BufferSizeExceeded();
  }

  if (!AmI("bibtex") && last >= inputOutput->maxbufstack())
  {
    inputOutput->maxbufstack() = last + 1;
    if (inputOutput->maxbufstack() >= bufsize)
    {
      BufferSizeExceeded();
    }
  }

#if defined(WITH_OMEGA)
  if (AmI("omega"))
  {
    while (last > first && (buffer16[last - 1] == u' ' || buffer16[last - 1] == u'\t' || buffer16[last - 1] == u'\r'))
    {
      last -= 1;
    }
  }
  else
#endif
  {
    while (last > first && (buffer[last - 1] == ' ' || buffer[last - 1] == '\t' || buffer[last - 1] == '\r'))
    {
      last -= 1;
    }
  }

  return true;
}
