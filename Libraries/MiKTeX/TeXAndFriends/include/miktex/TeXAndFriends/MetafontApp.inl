/* miktex/TeXAndFriends/MetafontApp.inl:                -*- C++ -*-

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

#if !defined(DE57699544C3AA4398A72098F31ED9CD)
#define DE57699544C3AA4398A72098F31ED9CD

#include <miktex/TeXAndFriends/config.h>

#include "TeXMFApp.inl"

MIKTEXMF_BEGIN_NAMESPACE;

inline void miktexallocatememory()
{
  THEAPP.AllocateMemory();
}

inline void miktexfreememory()
{
  THEAPP.FreeMemory();
}

inline C4P::C4P_integer makefraction(C4P::C4P_integer p, C4P::C4P_integer q)
{
#if defined(MIKTEXMFAPI_USE_ASM)
  return MakeFraction(p, q, THEDATA(aritherror));
#else
  return MFPROG.makefractionorig(p, q);
#endif
}

inline C4P_integer makescaled(C4P::C4P_integer p, C4P::C4P_integer q)
{
#if 0 && defined(MIKTEXMFAPI_USE_ASM)
  return MakeScaled(p, q, THEDATA(aritherror));
#else
  return MFPROG.makescaledorig(p, q);
#endif
}

inline C4P_integer takefraction(C4P::C4P_integer p, C4P::C4P_integer q)
{
#if defined(MIKTEXMFAPI_USE_ASM)
  return TakeFraction(p, q, THEDATA(aritherror));
#else
  return MFPROG.takefractionorig(p, q);
#endif
}

inline C4P_integer takescaled(C4P_integer p, C4P_integer q)
{
#if defined(MIKTEXMFAPI_USE_ASM)
  return TakeScaled(p, q, THEDATA(aritherror));
#else
  return MFPROG.takescaledorig(p, q);
#endif
}

MIKTEXMF_END_NAMESPACE;

#endif
