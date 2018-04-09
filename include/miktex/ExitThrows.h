/* miktex/ExitThrows.h:                                 -*- C++ -*-

   Copyright (C) 2018 Christian Schenk

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

#if !defined(FDF71B81770F4CF8BC7FF1C14C9B9A46)
#define FDF71B81770F4CF8BC7FF1C14C9B9A46

#define exit miktex_hidden_exit
#include <cstdlib>
#if defined(_WIN32)
#include <process.h>
#endif
#undef exit

#if defined(_WIN32)
#define MIKTEX_NORETURN_TMP_ __declspec(noreturn)
#define MIKTEX_CDECL_TMP_ __cdecl
#else
#define MIKTEX_NORETURN_
#define MIKTEX_CDECL_ cdecl
#endif

inline MIKTEX_NORETURN_TMP_ void MIKTEX_CDECL_TMP_ exit(int code)
{
  throw code;
}

#undef MIKTEX_NORETURN_TMP_
#undef MIKTEX_CDECL_TMP_

#endif
