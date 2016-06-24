/* miktex/TeXAndFriends/TeXApp.inl:                     -*- C++ -*-

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

#if !defined(C3A59E4996D60046B59B00D793120222)
#define C3A59E4996D60046B59B00D793120222

#include <miktex/TeXAndFriends/config.h>

#include <miktex/Core/File>
#include <miktex/Core/PathName>

#include "TeXMFApp.inl"

#if !defined(THEAPP)
#  error THEAPP not defined
#endif

#if !defined(THEDATA)
#  error THEDATA not defined
#endif

MIKTEXMF_BEGIN_NAMESPACE;

inline void miktexallocatememory()
{
  THEAPP.AllocateMemory();
}

inline void miktexclosedvifile(bytefile & f)
{
  THEAPP.CloseFile(f);
}

inline void miktexclosepdffile(bytefile & f)
{
  THEAPP.CloseFile(f);
}

inline void miktexfreememory()
{
  THEAPP.FreeMemory();
}

inline bool miktexinsertsrcspecialauto()
{
  return THEAPP.IsSourceSpecialOn(SourceSpecial::Auto);
}

inline bool miktexinsertsrcspecialeverycr()
{
  return THEAPP.IsSourceSpecialOn(SourceSpecial::CarriageReturn);
}

inline bool miktexinsertsrcspecialeverydisplay()
{
  return THEAPP.IsSourceSpecialOn(SourceSpecial::Display);
}

inline bool miktexinsertsrcspecialeveryhbox()
{
  return THEAPP.IsSourceSpecialOn(SourceSpecial::HorizontalBox);
}

inline bool miktexinsertsrcspecialeverymath()
{
  return THEAPP.IsSourceSpecialOn(SourceSpecial::Math);
}

inline bool miktexinsertsrcspecialeverypar()
{
  return THEAPP.IsSourceSpecialOn(SourceSpecial::Paragraph);
}

inline bool miktexinsertsrcspecialeveryparend()
{
  return THEAPP.IsSourceSpecialOn(SourceSpecial::ParagraphEnd);
}

inline bool miktexinsertsrcspecialeveryvbox()
{
  return THEAPP.IsSourceSpecialOn(SourceSpecial::VerticalBox);
}

inline bool miktexisnewsource(int fileName, int lineNo)
{
  return THEAPP.IsNewSource(fileName, lineNo);
}

inline int miktexmakesrcspecial(int fileName, int lineNo)
{
  return THEAPP.MakeSrcSpecial(fileName, lineNo);
}

template<class FileType> inline bool miktexopendvifile(FileType & f)
{
  MiKTeX::Core::PathName outPath;
  bool done = THEAPP.OpenOutputFile(*reinterpret_cast<C4P::FileRoot*>(&f), THEAPP.GetNameOfFile().Get(), MiKTeX::Core::FileShare::Read, false, outPath);
  if (done)
  {
    THEAPP.SetNameOfFile(THEAPP.MangleNameOfFile(outPath.Get()));
  }
  return done;
}

template<class FileType> inline bool miktexopenpdffile(FileType & f)
{
  MiKTeX::Core::PathName outPath;
  bool done = THEAPP.OpenOutputFile(*reinterpret_cast<C4P::FileRoot*>(&f), THEAPP.GetNameOfFile().Get(), MiKTeX::Core::FileShare::Read, false, outPath);
  if (done)
  {
    THEAPP.SetNameOfFile(THEAPP.MangleNameOfFile(outPath.Get()));
  }
  return done;
}

template<class FileType> inline bool miktexopenformatfile(FileType & f, bool renew = false)
{
  return THEAPP.OpenMemoryDumpFile(f, renew);
}

inline void miktexremembersourceinfo(int fileName, int lineNo)
{
  THEAPP.RememberSourceInfo(fileName, lineNo);
}

inline int miktexwrite18(const TEXMFCHAR * lpszCommand)
{
  int exitCode;
  return (int)THEAPP.Write18(lpszCommand, exitCode);
}

inline bool miktexwrite18p()
{
  return THEAPP.Write18P();
}

inline bool miktexenctexp()
{
  return THEAPP.EncTeXP();
}

inline bool miktexmltexp()
{
  return THEAPP.MLTeXP();
}

inline int miktexgetsynchronizationoptions()
{
  return THEAPP.GetSynchronizationOptions();
}

#if 1

inline bool restrictedshell()
{
  return (THEAPP.GetWrite18Mode() == TeXApp::Write18Mode::PartiallyEnabled
    || THEAPP.GetWrite18Mode() == TeXApp::Write18Mode::Query);
}

inline bool shellenabledp()
{
  return miktexwrite18p();
}

template<class CharType> int runsystem(const CharType * lpszCommand)
{
  return miktexwrite18(reinterpret_cast<const TEXMFCHAR *>(lpszCommand));
}

template<class CharType> const CharType * conststringcast(CharType * lpsz)
{
  return const_cast<const CharType *>(lpsz);
}

#endif

MIKTEXMF_END_NAMESPACE;

#endif
