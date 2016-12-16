/* miktex/TeXAndFriends/WebAppInputLine.h:              -*- C++ -*-

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
#  pragma once
#endif

#if !defined(CCBF075DC1F84F54BFB0FC72972CB3E4)
#define CCBF075DC1F84F54BFB0FC72972CB3E4

#include <miktex/TeXAndFriends/config.h>

#include <memory>
#include <string>
#include <iostream>

#include <miktex/Core/BufferSizes>
#include <miktex/Core/File>
#include <miktex/Core/FileType>
#include <miktex/Core/PathName>

#include <miktex/Util/StringUtil>

#include "WebApp.h"

#define MIKTEXMF_BEGIN_NAMESPACE                \
  namespace MiKTeX {                            \
    namespace TeXAndFriends {

#define MIKTEXMF_END_NAMESPACE                  \
    }                                           \
  }

MIKTEXMF_BEGIN_NAMESPACE;

class MIKTEXMFTYPEAPI(WebAppInputLine) :
  public WebApp
{
public:
  MIKTEXMFEXPORT MIKTEXTHISCALL WebAppInputLine();

public:
  WebAppInputLine(const WebAppInputLine& other) = delete;

public:
  WebAppInputLine& operator=(const WebAppInputLine& other) = delete;

public:
  WebAppInputLine(WebAppInputLine&& other) = delete;

public:
  WebAppInputLine& operator=(WebAppInputLine&& other) = delete;

public:
  virtual MIKTEXMFEXPORT MIKTEXTHISCALL ~WebAppInputLine() noexcept;

public:
  MIKTEXMFTHISAPI(void) Init(const std::string& programInvocationName) override;

public:
  MIKTEXMFTHISAPI(void) Finalize() override;

protected:
  MIKTEXMFTHISAPI(void) AddOptions() override;

protected:
  MIKTEXMFTHISAPI(bool) ProcessOption(int opt, const std::string& optArg) override;

public:
  virtual int GetFormatIdent() const
  {
    MIKTEX_UNEXPECTED();
  }

protected:
  virtual MIKTEXMFTHISAPI(void) TouchJobOutputFile(FILE*) const;

#ifdef THEDATA
private:
  void BufferSizeExceeded()
  {
#if defined(MIKTEX_BIBTEX)
    std::cout << "Sorry---you've exceeded BibTeX's buffer size";
    THEDATA(history) = 3;
    c4pthrow(9998);
#else
    if (GetFormatIdent() == 0)
    {
      fputs("Buffer size exceeded!", THEDATA(termout));
      throw (new C4P::Exception9999);
    }
    else
    {
      THEDATA(curinput).locfield = THEDATA(first);
      THEDATA(curinput).limitfield = THEDATA(last) - 1;
#if defined(bufsize)
      overflow (256, bufsize);
#else
      overflow(256, THEDATA(bufsize));
#endif
    }
#endif // ifndef MIKTEX_BIBTEX
  }
#endif // ifdef THEDATA

public:
  template<class FileType> void CloseFile(FileType& f)
  {
    f.AssertValid();
    TouchJobOutputFile(f);
    GetSession()->CloseFile(f);
  }

private:
  int GetCharacter(FILE* file) const
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
#if 0
    const int e_o_f = 0x1a; // ^Z
    if (ch == e_o_f)
    {
      ch = EOF;               // -1
      HandleEof(file);
    }
#endif
    return ch;
  }

public:
  MIKTEXMFTHISAPI(MiKTeX::Core::PathName) GetFoundFile() const;

public:
  MIKTEXMFTHISAPI(MiKTeX::Core::PathName) GetFoundFileFq() const;

#if defined(THEDATA)
public:
  MiKTeX::Core::PathName GetNameOfFile() const
  {
#if defined(MIKTEX_XETEX)
    MIKTEX_ASSERT (sizeof(THEDATA(nameoffile)[0]) == sizeof(char));
    const char* lpsz = reinterpret_cast<const char*>(THEDATA(nameoffile));
    return MiKTeX::Util::StringUtil::UTF8ToWideChar(lpsz);
#else
    return THEDATA(nameoffile);
#endif
  }
#endif

#if defined(THEDATA)
public:
  void SetNameOfFile(const MiKTeX::Core::PathName& fileName)
  {
#if defined(MIKTEX_XETEX)
    MiKTeX::Util::StringUtil::CopyString(reinterpret_cast<char*>(THEDATA(nameoffile)), MiKTeX::Core::BufferSizes::MaxPath + 1, fileName.GetData());
#else
    MiKTeX::Util::StringUtil::CopyString(THEDATA(nameoffile), MiKTeX::Core::BufferSizes::MaxPath + 1, fileName.GetData());
#endif
    THEDATA(namelength) = fileName.GetLength();
  }
#endif

public:
  MIKTEXMFTHISAPI(void) SetOutputDirectory(const MiKTeX::Core::PathName & path);

public:
  MIKTEXMFTHISAPI(MiKTeX::Core::PathName) GetOutputDirectory() const;

public:
  MIKTEXMFTHISAPI(void) SetAuxDirectory(const MiKTeX::Core::PathName & path);

public:
  MIKTEXMFTHISAPI(MiKTeX::Core::PathName) GetAuxDirectory() const;

#if 0  
private:
  MIKTEXMFTHISAPI(void) HandleEof(FILE* file) const;
#endif

#if defined(THEDATA) && !defined(MIKTEX_XETEX)
public:
  bool InputLine(C4P::C4P_text& f, C4P::C4P_boolean bypassEndOfLine)
  {
    f.AssertValid();

#if defined(PASCAL_TEXT_IO)
    not_implemented ();
    if (bypassEndOfLine && feof(f) == 0)
    {
      MIKTEX_ASSERT((*f)() == '\n');
      c4pgetc(f);
    }
#endif

#if defined(MIKTEX_BIBTEX)
    const unsigned long first = 0;
#else
    const unsigned long first = THEDATA(first);
#ifndef bufsize
    const unsigned long bufsize = THEDATA(bufsize);
#endif
#endif

    THEDATA(last) = first;

    if (feof(f) != 0)
    {
      return false;
    }

    int ch;

#if defined(PASCAL_TEXT_IO)
    not_implemented ();
    ch = (*f)();
#else
    ch = GetCharacter(f);
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
#endif // not Pascal Text I/O

    if (ch == '\n')
    {
      return true;
    }

#if defined(MIKTEX_OMEGA)
    THEDATA(buffer)[ THEDATA(last)++ ] = ch;
#else
    THEDATA(buffer)[THEDATA(last)++] = THEDATA(xord)[ch & 0xff];
#endif

    while ((ch = GetCharacter(f)) != EOF && THEDATA(last) < bufsize)
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
#if defined(MIKTEX_OMEGA)
      THEDATA(buffer)[ THEDATA(last)++ ] = ch;
#else
      THEDATA(buffer)[THEDATA(last)++] = THEDATA(xord)[ch & 0xff];
#endif
    }

    if (ch != '\n' && ch != EOF)
    {
      BufferSizeExceeded();
    }

#if !defined(MIKTEX_BIBTEX)
    if (THEDATA(last) >= THEDATA(maxbufstack))
    {
      THEDATA(maxbufstack) = THEDATA(last) + 1;
      if (THEDATA(maxbufstack) >= bufsize)
      {
        BufferSizeExceeded();
      }
    }
#endif // ifndef MIKTEX_BIBTEX

    while (THEDATA(last) > first
      && (THEDATA(buffer)[THEDATA(last) - 1] == ' '
        || THEDATA(buffer)[THEDATA(last) - 1] == '\t'
        || THEDATA(buffer)[THEDATA(last) - 1] == '\r'))
    {
      THEDATA(last)--;
    }

#if defined(PASCAL_TEXT_IO)
    not_implemented ();
    f() = '\n';
#endif

    return true;
  }
#endif

public:
  MIKTEXMFTHISAPI(bool) OpenInputFile(FILE** ppFile, const MiKTeX::Core::PathName& fileName);

public:
  MIKTEXMFTHISAPI(bool) OpenInputFile(C4P::FileRoot& f, const MiKTeX::Core::PathName& fileName);

public:
  MIKTEXMFTHISAPI(bool) OpenOutputFile(C4P::FileRoot& f, const MiKTeX::Core::PathName& fileName, MiKTeX::Core::FileShare share, bool text, MiKTeX::Core::PathName& outPath);

public:
  MIKTEXMFTHISAPI(bool) AllowFileName(const MiKTeX::Core::PathName& fileName, bool forInput);

protected:
  MIKTEXMFTHISAPI(void) EnablePipes(bool f);

protected:
  MIKTEXMFTHISAPI(MiKTeX::Core::PathName) GetLastInputFileName() const;

public:
  static MIKTEXMFCEEAPI(MiKTeX::Core::PathName) MangleNameOfFile(const char* fileName);

public:
  static MIKTEXMFCEEAPI(MiKTeX::Core::PathName) UnmangleNameOfFile(const char* fileName);

public:
  static MIKTEXMFCEEAPI(MiKTeX::Core::PathName) UnmangleNameOfFile(const wchar_t* fileName);

private:
  class impl;
  std::unique_ptr<impl> pimpl;
};

MIKTEXMF_END_NAMESPACE;

#endif
