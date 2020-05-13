/* miktex/TeXAndFriends/WebAppInputLine.h:              -*- C++ -*-

   Copyright (C) 1996-2020 Christian Schenk

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

#pragma once

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

#define MIKTEX_TEXMF_BEGIN_NAMESPACE                \
  namespace MiKTeX {                            \
    namespace TeXAndFriends {

#define MIKTEX_TEXMF_END_NAMESPACE                  \
    }                                           \
  }

MIKTEX_TEXMF_BEGIN_NAMESPACE;

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
  virtual C4P::C4P_signed32 bufsize() = 0;
public:
  virtual char*& nameoffile() = 0;
public:
  virtual C4P::C4P_signed32& namelength() = 0;
public:
  virtual char* buffer() = 0;
public:
  virtual char16_t* buffer16() = 0;
public:
  virtual char32_t* buffer32() = 0;
public:
  virtual C4P::C4P_signed32& maxbufstack() = 0;
public:
  virtual void overflow(C4P::C4P_signed32 s, C4P::C4P_integer n) = 0;
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
  static WebAppInputLine* GetWebAppInputLine()
  {
    MIKTEX_ASSERT(dynamic_cast<WebAppInputLine*>(Application::GetApplication()) != nullptr);
    return (WebAppInputLine*)Application::GetApplication();
  }

public:
  MIKTEXMFTHISAPI(void) Init(std::vector<char*>& args) override;

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
  virtual MIKTEXMFTHISAPI(void) BufferSizeExceeded() const;

public:
  template<class FileType> void CloseFile(FileType& f)
  {
    f.AssertValid();
    TouchJobOutputFile(f);
    GetSession()->CloseFile(f);
  }

public:
  MIKTEXMFTHISAPI(MiKTeX::Core::PathName) GetFoundFile() const;

public:
  MIKTEXMFTHISAPI(MiKTeX::Core::PathName) GetFoundFileFq() const;

public:
  MiKTeX::Core::PathName GetNameOfFile() const
  {
    IInputOutput* inputOutput = GetInputOutput();
    return MiKTeX::Core::PathName(inputOutput->nameoffile());
  }

public:
  virtual void SetNameOfFile(const MiKTeX::Core::PathName& fileName) const
  {
    IInputOutput* inputOutput = GetInputOutput();
    MiKTeX::Util::StringUtil::CopyString(inputOutput->nameoffile(), MiKTeX::Core::BufferSizes::MaxPath + 1, fileName.GetData());
    inputOutput->namelength() = static_cast<C4P::C4P_signed16>(fileName.GetLength());
  }

public:
  MIKTEXMFTHISAPI(void) SetOutputDirectory(const MiKTeX::Core::PathName & path);

public:
  MIKTEXMFTHISAPI(MiKTeX::Core::PathName) GetOutputDirectory() const;

public:
  MIKTEXMFTHISAPI(void) SetAuxDirectory(const MiKTeX::Core::PathName & path);

public:
  MIKTEXMFTHISAPI(MiKTeX::Core::PathName) GetAuxDirectory() const;

public:
  MIKTEXMFTHISAPI(bool) InputLine(C4P::C4P_text& f, C4P::C4P_boolean bypassEndOfLine) const;

public:
  MIKTEXMFTHISAPI(bool) OpenInputFile(FILE** ppFile, const MiKTeX::Core::PathName& fileName);

public:
  MIKTEXMFTHISAPI(bool) OpenInputFile(C4P::FileRoot& f, const MiKTeX::Core::PathName& fileName);

public:
  MIKTEXMFTHISAPI(bool) OpenOutputFile(C4P::FileRoot& f, const MiKTeX::Core::PathName& fileName, bool isTextFile_deprecated, MiKTeX::Core::PathName& outPath);

public:
  MIKTEXMFTHISAPI(bool) AllowFileName(const MiKTeX::Core::PathName& fileName, bool forInput);

protected:
  MIKTEXMFTHISAPI(void) EnableShellCommands(MiKTeX::Core::ShellCommandMode mode);

protected:
  MIKTEXMFTHISAPI(MiKTeX::Core::ShellCommandMode) GetShellCommandMode() const;

protected:
  MIKTEXMFTHISAPI(void) EnablePipes(bool f);

protected:
  MIKTEXMFTHISAPI(MiKTeX::Core::PathName) GetLastInputFileName() const;

#if defined(WITH_OMEGA)
public:
  static MIKTEXMFCEEAPI(MiKTeX::Core::PathName) MangleNameOfFile(const char* fileName);
#endif

#if defined(WITH_OMEGA)
public:
  static MIKTEXMFCEEAPI(MiKTeX::Core::PathName) UnmangleNameOfFile(const char* fileName);
#endif

public:
  MIKTEXMFTHISAPI(void) SetInputOutput(IInputOutput* inputOutput);

public:
  MIKTEXMFTHISAPI(IInputOutput*) GetInputOutput() const;

private:
  class impl;
  std::unique_ptr<impl> pimpl;
};

template<class FileType> inline bool inputln(FileType& f, C4P::C4P_boolean bypassEndOfLine = true)
{
  return WebAppInputLine::GetWebAppInputLine()->InputLine(f, bypassEndOfLine);
}

template<class FileType> inline void miktexclosefile(FileType& f)
{
  WebAppInputLine::GetWebAppInputLine()->CloseFile(f);
}

template<class FileType> inline bool miktexopeninputfile(FileType& f)
{
  bool done = WebAppInputLine::GetWebAppInputLine()->OpenInputFile(*static_cast<C4P::FileRoot*>(&f), WebAppInputLine::GetWebAppInputLine()->GetNameOfFile());
  if (done)
  {
#if defined(MIKTEX_OMEGA)
    WebAppInputLine::GetWebAppInputLine()->SetNameOfFile(WebAppInputLine::GetWebAppInputLine()->MangleNameOfFile(WebAppInputLine::GetWebAppInputLine()->GetFoundFileFq().GetData()));
#else
    WebAppInputLine::GetWebAppInputLine()->SetNameOfFile(WebAppInputLine::GetWebAppInputLine()->GetFoundFileFq());
#endif
  }
  return done;
}

inline bool miktexallownameoffile(C4P::C4P_boolean forInput)
{
  return WebAppInputLine::GetWebAppInputLine()->AllowFileName(WebAppInputLine::GetWebAppInputLine()->GetNameOfFile(), forInput);
}

template<class FileType> inline bool miktexopenoutputfile(FileType& f, C4P::C4P_boolean isTextFile_deprecated)
{
  // must open with read/write sharing flags
  // cf. bug 2006511
  MiKTeX::Core::PathName outPath;
  bool done = WebAppInputLine::GetWebAppInputLine()->OpenOutputFile(*static_cast<C4P::FileRoot*>(&f), WebAppInputLine::GetWebAppInputLine()->GetNameOfFile(), isTextFile_deprecated, outPath);
  if (done)
  {
#if defined(MIKTEX_OMEGA)
    WebAppInputLine::GetWebAppInputLine()->SetNameOfFile(WebAppInputLine::GetWebAppInputLine()->MangleNameOfFile(outPath.GetData()));
#else
    WebAppInputLine::GetWebAppInputLine()->SetNameOfFile(outPath);
#endif
  }
  return done;
}

MIKTEX_TEXMF_END_NAMESPACE;

#endif
