/* miktex/TeXAndFriends/WebAppInputLine.h:              -*- C++ -*-

   Copyright (C) 1996-2017 Christian Schenk

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

class IInputOutput
{
public:
  virtual C4P::C4P_signed32& loc() = 0;
public:
  virtual C4P::C4P_signed32& limit() = 0;
public:
  virtual C4P::C4P_signed32 first() = 0;
public:
  virtual C4P::C4P_signed32& last() = 0;
public:
  virtual C4P::C4P_signed32 bufsize_get() = 0;
public:
  virtual char* nameoffile() = 0;
public:
  virtual C4P::C4P_signed16& namelength() = 0;
public:
  virtual char* buffer() = 0;
public:
  virtual C4P::C4P_signed32& maxbufstack() = 0;
public:
  virtual void overflow(C4P::C4P_signed32 s, C4P::C4P_integer n) = 0;
};

template<class PROGRAM_CLASS> class InputOutputImpl :
  public IInputOutput
{
public:
  InputOutputImpl(PROGRAM_CLASS& program) :
    program(program)
  {
  }
private:
  PROGRAM_CLASS& program;
public:
  C4P::C4P_signed32& loc() override
  {
#if defined(MIKTEX_BIBTEX)
    MIKTEX_UNEXPECTED();
#else
    return program.curinput.locfield;
#endif
  }
public:
  C4P::C4P_signed32& limit()
  {
#if defined(MIKTEX_BIBTEX)
    MIKTEX_UNEXPECTED();
#else
    return program.curinput.limitfield;
#endif
  }
public:
  C4P::C4P_signed32 first() override
  {
#if defined(MIKTEX_BIBTEX)
    return 0;
#else
    return program.first;
#endif
  }
public:
  C4P::C4P_signed32& last() override
  {
    return program.last;
  }
public:
  C4P::C4P_signed32 bufsize_get() override
  {
#if defined(bufsize)
    return bufsize;
#else
    return program.bufsize;
#endif
  }
public:
  char* nameoffile() override
  {
    return program.nameoffile;
  }
public:
  C4P::C4P_signed16& namelength() override
  {
    return program.namelength;
  }
public:
  char* buffer() override
  {
    return (char*)program.buffer;
  }
public:
  C4P::C4P_signed32& maxbufstack() override
  {
#if defined(MIKTEX_BIBTEX)
    MIKTEX_UNEXPECTED();
#else
    return program.maxbufstack;
#endif
  }
public:
  void overflow(C4P::C4P_signed32 s, C4P::C4P_integer n) override
  {
#if defined(MIKTEX_BIBTEX)
    MIKTEX_UNEXPECTED();
#else
    program.overflow(s, n);
#endif
  }
};

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

private:
  void BufferSizeExceeded()
  {
#if defined(MIKTEX_BIBTEX)
    std::cout << "Sorry---you've exceeded BibTeX's buffer size";
    GetInitFinalize()->history() = 3;
    c4pthrow(9998);
#else
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
      inputOutput->overflow(256, inputOutput->bufsize_get());
    }
#endif
  }

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

public:
  MiKTeX::Core::PathName GetNameOfFile() const
  {
    IInputOutput* inputOutput = GetInputOutput();
#if defined(MIKTEX_XETEX)
    return MiKTeX::Util::StringUtil::UTF8ToWideChar(inputOutput->nameoffile());
#else
    return inputOutput->nameoffile();
#endif
  }

public:
  void SetNameOfFile(const MiKTeX::Core::PathName& fileName)
  {
    IInputOutput* inputOutput = GetInputOutput();
    MiKTeX::Util::StringUtil::CopyString(inputOutput->nameoffile(), MiKTeX::Core::BufferSizes::MaxPath + 1, fileName.GetData());
    inputOutput->namelength() = fileName.GetLength();
  }

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

#if !defined(MIKTEX_XETEX)
public:
  bool InputLine(C4P::C4P_text& f, C4P::C4P_boolean bypassEndOfLine)
  {
    f.AssertValid();

#if defined(PASCAL_TEXT_IO)
    MIKTEX_UNEXPECTED();
#endif

    IInputOutput* inputOutput = GetInputOutput();

    inputOutput->last() = inputOutput->first();

    if (feof(f) != 0)
    {
      return false;
    }

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

#if defined(MIKTEX_OMEGA)
    inputOutput->buffer()[inputOutput->last()] = ch;
#else
    inputOutput->buffer()[inputOutput->last()] = GetCharacterConverter()->xord()[ch & 0xff];
#endif
    inputOutput->last() += 1;

    while ((ch = GetCharacter(f)) != EOF && inputOutput->last() < inputOutput->bufsize_get())
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
      inputOutput->buffer()[inputOutput->last()] = ch;
#else
      inputOutput->buffer()[inputOutput->last()] = GetCharacterConverter()->xord()[ch & 0xff];
#endif
      inputOutput->last() += 1;
    }

    if (ch != '\n' && ch != EOF)
    {
      BufferSizeExceeded();
    }

#if !defined(MIKTEX_BIBTEX)
    if (inputOutput->last() >= inputOutput->maxbufstack())
    {
      inputOutput->maxbufstack() = inputOutput->last() + 1;
      if (inputOutput->maxbufstack() >= inputOutput->bufsize_get())
      {
        BufferSizeExceeded();
      }
    }
#endif

    while (inputOutput->last() > inputOutput->first()
      && (inputOutput->buffer()[inputOutput->last() - 1] == ' '
        || inputOutput->buffer()[inputOutput->last() - 1] == '\t'
        || inputOutput->buffer()[inputOutput->last() - 1] == '\r'))
    {
      inputOutput->last() -= 1;
    }

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

public:
  MIKTEXMFTHISAPI(void) SetInputOutput(IInputOutput* inputOutput);

public:
  MIKTEXMFTHISAPI(IInputOutput*) GetInputOutput() const;

private:
  class impl;
  std::unique_ptr<impl> pimpl;
};

MIKTEXMF_END_NAMESPACE;

#endif
