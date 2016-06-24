/* miktex/TeXAndFriends/Prototypes.h:                   -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX TeXMF Library.

   The MiKTeX TeXMF Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX TeXMF Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX TeXMF Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(EA5A87F23904AC44BC85743B1568911C)
#define EA5A87F23904AC44BC85743B1568911C

#include <miktex/TeXAndFriends/config.h>

MIKTEXMF_BEGIN_NAMESPACE;

const unsigned long ICT_TCX = 1;
const unsigned long ICT_8BIT = 2;

MIKTEXMFCEEAPI(bool) InitializeCharTables(unsigned long flags, const char * lpszFileName, void * pChar, void * pOrd, void * pPrintable);

MIKTEXMFCEEAPI(bool) OpenMAPFile(void * p, const char * lpszName);

MIKTEXMFCEEAPI(bool) OpenMETAFONTFile(void * p, const char * lpszName);

MIKTEXMFCEEAPI(bool) OpenTFMFile(void * p, const char * lpszName);

MIKTEXMFCEEAPI(bool) OpenVFFile(void * p, const char * lpszName);

MIKTEXMFCEEAPI(int) OpenXFMFile(void * p, const char * lpszName);

MIKTEXMFCEEAPI(int) OpenXVFFile(void * p, const char * lpszName);

MIKTEXMFCEEAPI(bool) Write18(const char * lpszCommand, unsigned long * lpExitCode = 0);

#if defined(MIKTEX_WINDOWS_32)
#  define MIKTEXMFAPI_USE_ASM 1
#endif

#if defined(MIKTEXMFAPI_USE_ASM)
MIKTEXMFAPI_(int, __stdcall) TakeFraction(int p, int q, bool & arithError);
#endif

#if defined(MIKTEXMFAPI_USE_ASM)
MIKTEXMFAPI_(int, __stdcall) TakeScaled(int p, int q, bool & arithError);
#endif

#if defined(MIKTEXMFAPI_USE_ASM)
MIKTEXMFAPI_(int, __stdcall) MakeFraction(int p, int q, bool & arithError);
#endif

#if defined(MIKTEXMFAPI_USE_ASM)
MIKTEXMFAPI_(int, __stdcall) MakeScaled(int p, int q, bool & arithError);
#endif

MIKTEXMF_END_NAMESPACE;

#endif
