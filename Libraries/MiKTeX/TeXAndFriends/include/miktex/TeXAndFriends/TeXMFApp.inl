/* miktex/TeXAndFriends/TeXMFApp.inl:                   -*- C++ -*-

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

#if !defined(AFD860C2031C4658874212BDF371CFCA)
#define AFD860C2031C4658874212BDF371CFCA

#include <miktex/TeXAndFriends/config.h>

#include <cstddef>

#include "WebAppInputLine.inl"

#if !defined(THEAPP)
#  error THEAPP not defined
#endif

#if !defined(THEDATA)
#  error THEDATA not defined
#endif

MIKTEXMF_BEGIN_NAMESPACE;

inline bool miktexcstyleerrormessagesp()
{
  return THEAPP.CStyleErrorMessagesP();
}

inline void miktexgetdefaultdumpfilename(char * lpszPath)
{
  THEAPP.GetDefaultMemoryDumpFileName(lpszPath);
}

inline int miktexgetinteraction()
{
  return THEAPP.GetInteraction();
}

inline int miktexgetjobname()
{
  return THEAPP.GetJobName();
}

inline const TEXMFCHAR * miktexgetstringat(int idx)
{
  MIKTEX_ASSERT(sizeof(TEXMFCHAR) == sizeof(THEDATA(strpool)[idx]));
  return reinterpret_cast<TEXMFCHAR *>(&(THEDATA(strpool)[idx]));
}

inline bool miktexhaltonerrorp()
{
  return THEAPP.HaltOnErrorP();
}

inline void miktexinitializebuffer()
{
  THEDATA(last) = THEAPP.InitializeBuffer(THEDATA(buffer));
}

inline void miktexinvokeeditor(int editFileName, int editFileNameLength, int editLineNumber, int transcriptFileName, int transcriptFileNameLength)
{
  THEAPP.InvokeEditor(editFileName, editFileNameLength, editLineNumber, transcriptFileName, transcriptFileNameLength);
}

inline void miktexinvokeeditor(int editFileName, int editFileNameLength, int editLineNumber)
{
  THEAPP.InvokeEditor(editFileName, editFileNameLength, editLineNumber, 0, 0);
}

inline bool miktexisinitprogram()
{
  return THEAPP.IsInitProgram();
}

inline int miktexmakefullnamestring()
{
  return THEAPP.MakeFullNameString();
}

inline void miktexontexmffinishjob()
{
  THEAPP.OnTeXMFFinishJob();
}

inline void miktexontexmfinitialize()
{
  THEAPP.OnTeXMFInitialize();
}

inline void miktexontexmfstartjob()
{
  THEAPP.OnTeXMFStartJob();
}

#define miktexreallocate(p, n) \
  THEAPP.Reallocate(#p, p, n, __FILE__, __LINE__)

template<typename FileType, typename EleType> inline void miktexdump(FileType & f, const EleType & e, std::size_t n)
{
  THEAPP.Dump(f, e, n);
}

template<typename FileType, typename EleType> inline void miktexdump(FileType & f, const EleType & e)
{
  THEAPP.Dump(f, e);
}

template<typename FileType> inline void miktexdumpint(FileType & f, int val)
{
  miktexdump(f, val);
}

template<typename FileType, typename EleType> inline void miktexundump(FileType & f, EleType & e, std::size_t n)
{
  THEAPP.Undump(f, e, n);
}

template<typename FileType, typename EleType> inline void miktexundump(FileType & f, EleType & e)
{
  THEAPP.Undump(f, e);
}

template<typename FileType, typename LowType, typename HighType, typename EleType> inline void miktexundump(FileType & f, LowType low, HighType high, EleType & e, std::size_t n)
{
  THEAPP.Undump(f, static_cast<EleType>(low), static_cast<EleType>(high), e, n);
}

template<typename FileType, typename HighType, typename EleType> inline void miktexundump(FileType & f, HighType high, EleType & e, std::size_t n)
{
  THEAPP.Undump(f, static_cast<EleType>(high), e, n);
}

template<typename FileType> inline void miktexundumpint(FileType & f, int & val)
{
  miktexundump(f, val);
}

inline void miktexcheckmemoryifdebug()
{
#if defined(MIKTEX_DEBUG)
  THEAPP.CheckMemory();
#endif
}

MIKTEXMF_END_NAMESPACE;

#endif
