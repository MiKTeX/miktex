/* Dib.h:                                               -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX DVI Library.

   The MiKTeX DVI Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2, or (at your option) any later version.

   The MiKTeX DVI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the MiKTeX DVI Library; if not, write to the
   Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
   USA.  */

#pragma once

class Dib
{
public:
  Dib();

public:
  Dib(const PathName & path);

public:
  ~Dib();

public:
  void GetSize(long * pcx, long * pcy) const;

public:
  void Render(HDC hdc, int x, int y, int cx, int cy);

private:
  void AttachFile(const char * lpszFileName);

private:
  const BITMAPINFO * GetBitmapInfo() const
  {
    MIKTEX_ASSERT(pBitmapFileHeader != nullptr);
    return reinterpret_cast<const BITMAPINFO *>(reinterpret_cast<const BYTE *>(pBitmapFileHeader) + sizeof(*pBitmapFileHeader));
  }

private:
  unsigned long GetWidth() const
  {
    return GetBitmapInfo()->bmiHeader.biWidth;
  }

private:
  unsigned long GetHeight() const
  {
    return GetBitmapInfo()->bmiHeader.biHeight;
  }

private:
  const void * GetBits() const
  {
    MIKTEX_ASSERT(pBitmapFileHeader != nullptr);
    return reinterpret_cast<const BYTE *>(pBitmapFileHeader) + pBitmapFileHeader->bfOffBits;
  }

private:
  void Clear();

private:
  HANDLE hFile = INVALID_HANDLE_VALUE;

private:
  HANDLE hMap = nullptr;

private:
  BITMAPFILEHEADER * pBitmapFileHeader = nullptr;
};
