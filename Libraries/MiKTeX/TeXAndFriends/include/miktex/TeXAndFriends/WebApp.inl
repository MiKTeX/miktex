/* miktex/TeXAndFriends/WebApp.inl:                     -*- C++ -*-

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

#if !defined(EE6ABF547DB7484EA8E02C572BC7086C)
#define EE6ABF547DB7484EA8E02C572BC7086C

#include <miktex/TeXAndFriends/config.h>

MIKTEXMF_BEGIN_NAMESPACE;

#if defined(THEDATA)
inline bool miktexgetquietflag()
{
  return THEAPP.GetQuietFlag();
}
#endif

#if defined(THEAPP) && defined(IMPLEMENT_TCX)
inline void miktexinitializechartables()
{
  THEAPP.InitializeCharTables();
}
#endif

#if defined(THEAPP) && defined(IMPLEMENT_TCX)
inline bool miktexhavetcxfilename()
{
  return THEAPP.GetTcxFileName().GetLength() > 0;
}
#endif

#if defined(THEAPP) && defined(IMPLEMENT_TCX)
inline bool miktexenableeightbitcharsp()
{
  return THEAPP.Enable8BitCharsP();
}
#endif

#if defined(THEDATA)
inline void miktexprocesscommandlineoptions()
{
  THEAPP.ProcessCommandLineOptions();
}
#endif

MIKTEXMF_END_NAMESPACE;

#endif
