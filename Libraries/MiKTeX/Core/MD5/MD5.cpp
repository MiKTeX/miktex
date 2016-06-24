/* MD5.cpp: MD5 calculations

   Copyright (C) 1996-2016 Christian Schenk

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/File.h"
#include "miktex/Core/MD5.h"
#include "miktex/Core/MemoryMappedFile.h"
#include "miktex/Core/PathName.h"

using namespace MiKTeX::Core;
using namespace std;

MD5 MD5::FromFile(const PathName & path)
{
  MD5Builder md5Builder;
  md5Builder.Init();
  size_t size = File::GetSize(path);
  if (size > 0)
  {
    unique_ptr<MemoryMappedFile> mmapFile(MemoryMappedFile::Create());
    const void * ptr = mmapFile->Open(path, false);
    md5Builder.Update(ptr, size);
  }
  md5Builder.Final();
  return md5Builder.GetMD5();
}

MIKTEXSTATICFUNC(int) Unhex(char x)
{
  if (x >= '0' && x <= '9')
  {
    return x - '0';
  }
  else if (x >= 'A' && x <= 'F')
  {
    return x - 'A' + 10;
  }
  else if (x >= 'a' && x <= 'f')
  {
    return x - 'a' + 10;
  }
  else
  {
    MIKTEX_UNEXPECTED();
  }
}

MD5 MD5::Parse(const string & hexString)
{
  size_t l = hexString.length();
  if (l != 32)
  {
    MIKTEX_UNEXPECTED();
  }
  MD5 md5;
  for (size_t i = 0; i < 16; ++i)
  {
    int x1 = Unhex(hexString[i * 2]);
    int x2 = Unhex(hexString[i * 2 + 1]);
    md5[i] = static_cast<md5_byte_t>(x1 << 4);
    md5[i] |= x2;
  }
  return md5;
}

