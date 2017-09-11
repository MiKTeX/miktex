/* miktex/First.h:                                      -*- C++ -*-

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
#pragma once
#endif

#if !defined(MIKTEX_FIRST_H)
#define MIKTEX_FIRST_H

#if defined(_MSC_VER)
#include <crtdbg.h>
#endif

#if defined(_MSC_VER) && defined(__cplusplus)

namespace MiKTeX {
  namespace Debug {
    void
#if defined(EAD86981C92C904D808A5E6CEC64B90E)
    __declspec(dllexport)
#else
    __declspec(dllimport)
#endif
    __cdecl OnThrowStdException ();
  }
}

#endif /* Microsoft C++ */

#endif
