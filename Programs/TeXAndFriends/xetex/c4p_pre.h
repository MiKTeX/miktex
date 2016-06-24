/* c4p_pre.h:                                           -*- C++ -*-
   
   Copyright (C) 2007-2016 Christian Schenk

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

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif

#include <cstdio>

/* _________________________________________________________________________

   texmfmp.h stuff
   _________________________________________________________________________ */

#define XETEX_UNICODE_FILE_DEFINED      1
typedef struct {
  FILE *f;
  long  savedChar;
  short skipNextLF;
  short encodingMode;
  void *conversionData;
} UFILE;
typedef UFILE* unicodefile;

typedef void* voidpointer;

#if _MSC_VER == 1800 || _MSC_VER == 1900
// workaround bug #2371 mathchoice in pdftex broken 
#  define C4P_NOOPT_mlisttohlist
#endif
