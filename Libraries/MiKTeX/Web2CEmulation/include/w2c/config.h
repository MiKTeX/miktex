/* w2c/config.h: Web2C emulation                        -*- C++ -*-

   Copyright (C) 2010-2016 Christian Schenk

   This file is part of the MiKTeX W2CEMU Library.

   The MiKTeX W2CEMU Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX W2CEMU Library is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX W2CEMU Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if ! defined(ED87D0F8C292441A8B1032D6D24136E5)
#define ED87D0F8C292441A8B1032D6D24136E5

#include "../miktex/W2C/pre.h"

#include "c-auto.h"

#include <math.h>

MIKTEX_BEGIN_EXTERN_C_BLOCK

typedef int integer;

typedef MIKTEX_INT64 longinteger;

#define LONGINTEGER_TYPE long long
#define LONGINTEGER_PRI "ll"

MIKTEXW2CEXPORT MIKTEXNORETURN void MIKTEXCEECALL miktex_uexit(int status);

#define uexit miktex_uexit

MIKTEXW2CEXPORT MIKTEXNORETURN void MIKTEXCEECALL miktex_usagehelp(const char ** lpszLines, const char * lpszBugEmail);

#define usagehelp miktex_usagehelp

MIKTEX_END_EXTERN_C_BLOCK

#endif
