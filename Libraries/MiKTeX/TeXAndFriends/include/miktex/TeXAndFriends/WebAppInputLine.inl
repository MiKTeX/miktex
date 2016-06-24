/* miktex/TeXAndFriends/WebAppInputLine.inl:            -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Publstrcmpilic
   License along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(ABC4B02244924C66A78EB0460822CA84)
#define ABC4B02244924C66A78EB0460822CA84

#include <miktex/TeXAndFriends/config.h>

#include <miktex/Core/File>
#include <miktex/Core/PathName>

#include "WebApp.inl"

#if !defined(THEAPP)
#  error THEAPP not defined
#endif

#if !defined(THEDATA)
#  error THEDATA not defined
#endif

MIKTEXMF_BEGIN_NAMESPACE;

#if !defined(MIKTEX_XETEX)
template<class FileType> inline bool inputln(FileType & f, C4P::C4P_boolean bypassEndOfLine = true)
{
  return THEAPP.InputLine(f, bypassEndOfLine);
}
#endif

template<class FileType> inline void miktexclosefile(FileType & f)
{
  THEAPP.CloseFile(f);
}

template<class FileType> inline bool miktexopeninputfile(FileType & f)
{
  bool done = (THEAPP.OpenInputFile(*static_cast<C4P::FileRoot*>(&f), THEAPP.GetNameOfFile().Get()));
  if (done)
  {
    THEAPP.SetNameOfFile(THEAPP.MangleNameOfFile(THEAPP.GetFoundFile()));
  }
  return done;
}

inline bool miktexallownameoffile(C4P::C4P_boolean forInput)
{
  return THEAPP.AllowFileName(THEAPP.GetNameOfFile().Get(), forInput);
}

template<class FileType> inline bool miktexopenoutputfile(FileType & f, C4P::C4P_boolean text)
{
  // must open with read/write sharing flags
  // cf. bug 2006511
  MiKTeX::Core::FileShare share = MiKTeX::Core::FileShare::ReadWrite;
  MiKTeX::Core::PathName outPath;
  bool done = THEAPP.OpenOutputFile(*static_cast<C4P::FileRoot*>(&f), THEAPP.GetNameOfFile().Get(), share, text, outPath);
  if (done)
  {
    THEAPP.SetNameOfFile(THEAPP.MangleNameOfFile(outPath.Get()));
  }
  return done;
}

MIKTEXMF_END_NAMESPACE;

#endif
