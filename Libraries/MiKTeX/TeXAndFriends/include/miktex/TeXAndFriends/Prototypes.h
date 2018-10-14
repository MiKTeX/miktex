/* miktex/TeXAndFriends/Prototypes.h:                   -*- C++ -*-

   Copyright (C) 1996-2018 Christian Schenk

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

#pragma once

#if !defined(EA5A87F23904AC44BC85743B1568911C)
#define EA5A87F23904AC44BC85743B1568911C

#include <miktex/TeXAndFriends/config.h>

#include <miktex/Core/PathName>

MIKTEXMF_BEGIN_NAMESPACE;

const unsigned long ICT_TCX = 1;
const unsigned long ICT_8BIT = 2;

MIKTEXMFCEEAPI(bool) InitializeCharTables(unsigned long flags, const MiKTeX::Core::PathName& fileName, char* chr, char* ord, char* printable);

MIKTEXMFCEEAPI(bool) OpenTFMFile(void* ptr, const MiKTeX::Core::PathName& fileName);

MIKTEXMFCEEAPI(bool) OpenVFFile(void* ptr, const MiKTeX::Core::PathName& fileName);

MIKTEXMFCEEAPI(int) OpenXFMFile(void* ptr, const MiKTeX::Core::PathName& fileName);

MIKTEXMFCEEAPI(int) OpenXVFFile(void* ptr, const MiKTeX::Core::PathName& fileName);

MIKTEXMF_END_NAMESPACE;

#endif
