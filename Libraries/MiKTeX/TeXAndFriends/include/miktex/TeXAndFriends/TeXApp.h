/* miktex/TeXAndFriends/TeXApp.h:                       -*- C++ -*-

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

#if !defined(EC8B1ACFE8E843EA960CDDFE7DD89AC9)
#define EC8B1ACFE8E843EA960CDDFE7DD89AC9

#include <miktex/TeXAndFriends/config.h>

#include <cstddef>

#include <memory>
#include <string>

#include <miktex/Core/BufferSizes>
#include <miktex/Core/FileType>
#include <miktex/Core/PathName>

#include <miktex/Util/CharBuffer>
#include <miktex/Util/inliners.h>

#include "TeXMFApp.h"

MIKTEX_TEXMF_BEGIN_NAMESPACE;

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

public:
  static TeXApp* GetTeXApp()
  {
    MIKTEX_ASSERT(dynamic_cast<TeXApp*>(Application::GetApplication()) != nullptr);
    return (TeXApp*)Application::GetApplication();
  }

protected:
  MIKTEXMFTHISAPI(void) Init(std::vector<char*>& args) override;

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
  MIKTEXMFTHISAPI(MiKTeX::Core::ShellCommandMode) GetWrite18Mode() const;

public:
  MIKTEXMFTHISAPI(bool) Write18P() const;

public:
  MIKTEXMFTHISAPI(bool) IsSourceSpecialOn(SourceSpecial s) const;

public:
  MIKTEXMFTHISAPI(bool) IsNewSource(int sourceFileName, int line) const;

public:
  MIKTEXMFTHISAPI(int) MakeSrcSpecial(int sourceFileName, int line) const;

public:
  MIKTEXMFTHISAPI(void) RememberSourceInfo(int sourceFileName, int line) const;

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
  MIKTEXMFTHISAPI(void) SetFormatHandler(IFormatHandler* formatHandler);

public:
  MIKTEXMFTHISAPI(IFormatHandler*) GetFormatHandler() const;

private:
  class impl;
  std::unique_ptr<impl> pimpl;
};

inline void miktexallocatememory()
{
  TeXApp::GetTeXApp()->AllocateMemory();
}

template<class FileType> void miktexclosedvifile(FileType& f)
{
  TeXApp::GetTeXApp()->CloseFile(f);
}

template<class FileType> void miktexclosepdffile(FileType& f)
{
  TeXApp::GetTeXApp()->CloseFile(f);
}

inline void miktexfreememory()
{
  TeXApp::GetTeXApp()->FreeMemory();
}

inline bool miktexinsertsrcspecialauto()
{
  return TeXApp::GetTeXApp()->IsSourceSpecialOn(SourceSpecial::Auto);
}

inline bool miktexinsertsrcspecialeverycr()
{
  return TeXApp::GetTeXApp()->IsSourceSpecialOn(SourceSpecial::CarriageReturn);
}

inline bool miktexinsertsrcspecialeverydisplay()
{
  return TeXApp::GetTeXApp()->IsSourceSpecialOn(SourceSpecial::Display);
}

inline bool miktexinsertsrcspecialeveryhbox()
{
  return TeXApp::GetTeXApp()->IsSourceSpecialOn(SourceSpecial::HorizontalBox);
}

inline bool miktexinsertsrcspecialeverymath()
{
  return TeXApp::GetTeXApp()->IsSourceSpecialOn(SourceSpecial::Math);
}

inline bool miktexinsertsrcspecialeverypar()
{
  return TeXApp::GetTeXApp()->IsSourceSpecialOn(SourceSpecial::Paragraph);
}

inline bool miktexinsertsrcspecialeveryparend()
{
  return TeXApp::GetTeXApp()->IsSourceSpecialOn(SourceSpecial::ParagraphEnd);
}

inline bool miktexinsertsrcspecialeveryvbox()
{
  return TeXApp::GetTeXApp()->IsSourceSpecialOn(SourceSpecial::VerticalBox);
}

inline bool miktexisnewsource(int fileName, int lineNo)
{
  return TeXApp::GetTeXApp()->IsNewSource(fileName, lineNo);
}

inline int miktexmakesrcspecial(int fileName, int lineNo)
{
  return TeXApp::GetTeXApp()->MakeSrcSpecial(fileName, lineNo);
}

template<class FileType> inline bool miktexopendvifile(FileType& f)
{
  MiKTeX::Core::PathName outPath;
  bool done = TeXApp::GetTeXApp()->OpenOutputFile(*reinterpret_cast<C4P::FileRoot*>(&f), TeXApp::GetTeXApp()->GetNameOfFile(), false, outPath);
  if (done)
  {
    TeXApp::GetTeXApp()->SetNameOfFile(outPath);
  }
  return done;
}

template<class FileType> inline bool miktexopenpdffile(FileType& f)
{
  MiKTeX::Core::PathName outPath;
  bool done = TeXApp::GetTeXApp()->OpenOutputFile(*reinterpret_cast<C4P::FileRoot*>(&f), TeXApp::GetTeXApp()->GetNameOfFile(), false, outPath);
  if (done)
  {
    TeXApp::GetTeXApp()->SetNameOfFile(outPath);
  }
  return done;
}

template<class FileType> inline bool miktexopenformatfile(FileType& f, bool renew = false)
{
  return TeXApp::GetTeXApp()->OpenMemoryDumpFile(f, renew);
}

inline void miktexremembersourceinfo(int fileName, int lineNo)
{
  TeXApp::GetTeXApp()->RememberSourceInfo(fileName, lineNo);
}

inline bool miktexwrite18p()
{
  return TeXApp::GetTeXApp()->Write18P();
}

inline bool miktexenctexp()
{
  return TeXApp::GetTeXApp()->EncTeXP();
}

inline bool miktexmltexp()
{
  return TeXApp::GetTeXApp()->MLTeXP();
}

inline int miktexgetsynchronizationoptions()
{
  return TeXApp::GetTeXApp()->GetSynchronizationOptions();
}

inline bool restrictedshell()
{
  return TeXApp::GetTeXApp()->GetWrite18Mode() == MiKTeX::Core::ShellCommandMode::Restricted || TeXApp::GetTeXApp()->GetWrite18Mode() == MiKTeX::Core::ShellCommandMode::Query;
}

inline bool shellenabledp()
{
  return miktexwrite18p();
}

MIKTEX_TEXMF_END_NAMESPACE;

#endif
