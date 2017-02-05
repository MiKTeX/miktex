/* miktex/TeXAndFriends/TeXApp.h:                       -*- C++ -*-

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

#if !defined(EC8B1ACFE8E843EA960CDDFE7DD89AC9)
#define EC8B1ACFE8E843EA960CDDFE7DD89AC9

#include <miktex/TeXAndFriends/config.h>

#include <cstddef>

#include <memory>
#include <string>

#include <miktex/Core/BufferSizes>
#include <miktex/Core/FileType>
#include <miktex/Core/PathName>

#include <miktex/Util/inliners.h>

#include "TeXMFApp.h"

namespace texapp {
#include <miktex/texapp.defaults.h>
}

MIKTEXMF_BEGIN_NAMESPACE;

class IFormatHandler
{
public:
  virtual C4P::C4P_signed32& formatident() = 0;
};

enum class SourceSpecial
{
  Auto,
  CarriageReturn,
  Display,
  HorizontalBox,
  Math,
  Paragraph,
  ParagraphEnd,
  VerticalBox,
};

class MIKTEXMFTYPEAPI(TeXApp) :
  public TeXMFApp
{
public:
  MIKTEXMFEXPORT MIKTEXTHISCALL TeXApp();

public:
  TeXApp(const TeXApp& other) = delete;

public:
  TeXApp& operator=(const TeXApp& other) = delete;

public:
  TeXApp(TeXApp&& other) = delete;

public:
  TeXApp& operator=(TeXApp&& other) = delete;

public:
  virtual MIKTEXMFEXPORT MIKTEXTHISCALL ~TeXApp() noexcept;

protected:
  MIKTEXMFTHISAPI(void) Init(const std::string& programInvocationName) override;

public:
  MIKTEXMFTHISAPI(void) Finalize() override;

protected:
  MIKTEXMFTHISAPI(void) AddOptions() override;

public:
  MiKTeX::Core::FileType GetInputFileType() const override
  {
    return MiKTeX::Core::FileType::TEX;
  }

protected:
  MIKTEXMFTHISAPI(bool) ProcessOption(int c, const std::string& optArg) override;

public:
  int GetFormatIdent() const override
  {
    return GetFormatHandler()->formatident();
  }

public:
  std::string GetMemoryDumpFileExtension() const override
  {
    return ".fmt";
  }

public:
  MiKTeX::Core::FileType GetMemoryDumpFileType() const override
  {
    return MiKTeX::Core::FileType::FMT;
  }

public:
  MIKTEXMFTHISAPI(void) OnTeXMFStartJob() override;

public:
  MIKTEXMFTHISAPI(bool) MLTeXP() const;

public:
  MIKTEXMFTHISAPI(int) GetSynchronizationOptions() const;

public:
  MIKTEXMFTHISAPI(bool) EncTeXP() const;

public:
  enum class Write18Mode
  {
    Enabled = 't',
    Disabled = 'f',
    PartiallyEnabled = 'p',
    Query = 'q',
  };

public:
  MIKTEXMFTHISAPI(Write18Mode) GetWrite18Mode() const;

public:
  MIKTEXMFTHISAPI(bool) Write18P() const;

public:
  MIKTEXMFTHISAPI(bool) IsSourceSpecialOn(SourceSpecial s) const;

public:
  bool IsNewSource(int sourceFileName, int line) const
  {
    TEXMFCHAR fileName[MiKTeX::Core::BufferSizes::MaxPath];
    GetTeXString(fileName, sourceFileName, MiKTeX::Core::BufferSizes::MaxPath);
    return lastSourceFilename != fileName || lastLineNum != line;
  }

public:
  int MakeSrcSpecial(int sourceFileName, int line)
  {
    IStringHandler* stringHandler = GetStringHandler();
    C4P::C4P_integer oldpoolptr = stringHandler->poolptr();
    TEXMFCHAR szFileName[MiKTeX::Core::BufferSizes::MaxPath];
    GetTeXString(szFileName, sourceFileName, MiKTeX::Core::BufferSizes::MaxPath);
    MiKTeX::Core::PathName fileName = UnmangleNameOfFile(szFileName);
    const std::size_t BUFSIZE = MiKTeX::Core::BufferSizes::MaxPath + 100;
    char szBuf[BUFSIZE];
#if _MSC_VER >= 1400
    sprintf_s(szBuf, BUFSIZE, "src:%d%s%s", line, isdigit(fileName[0]) ? " " : "", fileName.GetData());
#else
    sprintf(szBuf, "src:%d%s%s", line, isdigit(fileName[0]) ? " " : "", fileName.GetData());
#endif
    std::size_t len = MiKTeX::Util::StrLen(szBuf);
    CheckPoolPointer(stringHandler->poolptr(), len);
    char* lpsz = szBuf;
    while (*lpsz != 0)
    {
      stringHandler->strpool()[stringHandler->poolptr()] = *lpsz++;
      stringHandler->poolptr() += 1;
    }
    return oldpoolptr;
  }

public:
  void RememberSourceInfo(int sourceFileName, int line)
  {
    TEXMFCHAR szFileName[MiKTeX::Core::BufferSizes::MaxPath];
    GetTeXString(szFileName, sourceFileName, MiKTeX::Core::BufferSizes::MaxPath);
    lastSourceFilename = szFileName;
    lastLineNum = line;
  }

public:
  enum class Write18Result
  {
    QuotationError = -1,
    Disallowed = 0,
    Executed = 1,
    ExecutedAllowed = 2
  };

public:
  MIKTEXMFTHISAPI(Write18Result) Write18(const std::string& command, int& exitCode) const;

public:
  MIKTEXMFTHISAPI(Write18Result) Write18(const std::wstring& command, int& exitCode) const;

private:
  int lastLineNum;

private:
  MiKTeX::Core::PathName lastSourceFilename;

public:
  MIKTEXMFTHISAPI(void) SetFormatHandler(IFormatHandler* formatHandler);

public:
  MIKTEXMFTHISAPI(IFormatHandler*) GetFormatHandler() const;

private:
  class impl;
  std::unique_ptr<impl> pimpl;
};

MIKTEXMF_END_NAMESPACE;

#endif
