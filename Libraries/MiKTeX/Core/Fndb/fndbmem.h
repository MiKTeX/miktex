/* fndbmem.h: fndb file format                          -*- C++ -*-

   Copyright (C) 1996-2018 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#if !defined(A0FEBF8A9A7A419BB1230D6A7C07C5FA)
#define A0FEBF8A9A7A419BB1230D6A7C07C5FA

#include <miktex/Core/Debug>

CORE_INTERNAL_BEGIN_NAMESPACE;

typedef uint32_t FndbWord;
typedef FndbWord FndbByteOffset;

struct FileNameDatabaseHeader
{
  static const FndbWord Signature = 0x42444e46; // 'FNDB' (the x86 way)
  static const FndbWord Version = MIKTEX_FNDB_VERSION;

  // signature of fndb file
  FndbWord signature;

  // format version number
  FndbWord version;

  // flag word (bits see above)
  FndbWord flags;

  // pointer to string pool
  FndbByteOffset foStrings;

  // pointer to first record
  FndbByteOffset foTable;

  // number of directories
  FndbWord numDirs;

  // number of files (records)
  FndbWord numFiles;

  // max directory depth
  FndbWord depth;

  // size (in bytes) of fndb; includes header size
  FndbWord size;
  
  FndbWord reserved;

  void Init()
  {
    MIKTEX_ASSERT(sizeof(*this) % 8 == 0);
    signature = Signature;
    version = Version;
    flags = 0;
    size = sizeof(*this);
  }
};

struct FileNameDatabaseRecord
{
  FndbByteOffset foFileName;
  FndbByteOffset foDirectory;
  FndbByteOffset foInfo;
  FndbByteOffset reserved = 0;
};

CORE_INTERNAL_END_NAMESPACE;

#endif
