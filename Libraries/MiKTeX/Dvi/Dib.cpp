/* Dib.cpp:

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
   Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "StdAfx.h"

#include "internal.h"

Dib::Dib()
{
}

Dib::Dib(const PathName & path)
{
  AttachFile(path.GetData());
}

Dib::~Dib()
{
  try
  {
    Clear();
  }
  catch (const exception &)
  {
  }
}

void Dib::Clear()
{
  if (hMap != nullptr)
  {
    CloseHandle(hMap);
    hMap = nullptr;
  }
  if (hFile != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;
  }
  if (pBitmapFileHeader != nullptr)
  {
    UnmapViewOfFile(pBitmapFileHeader);
    pBitmapFileHeader = nullptr;
  }
}

void Dib::GetSize(long * pcx, long * pcy) const
{
  *pcx = static_cast<LONG>(GetBitmapInfo()->bmiHeader.biWidth);
  *pcy = static_cast<LONG>(GetBitmapInfo()->bmiHeader.biHeight);
}

void Dib::Render(HDC hdc, int x, int y, int cx, int cy)
{
  if (cx < 0)
  {
    cx = GetWidth();
  }
  if (cy < 0)
  {
    cy = GetHeight();
  }
  SetStretchBltMode(hdc, COLORONCOLOR);
  if (StretchDIBits(hdc, x, y, cx, cy, 0, 0, GetWidth(), GetHeight(), GetBits(), GetBitmapInfo(), DIB_RGB_COLORS, SRCCOPY) == GDI_ERROR)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("StretchDIBits");
  }
}

void Dib::AttachFile(const char * lpszFileName)
{
  hFile = CreateFileW(UW_(lpszFileName), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, nullptr);
  if (hFile == INVALID_HANDLE_VALUE)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateFileW", "path", lpszFileName);
  }
  hMap = CreateFileMappingW(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
  if (hMap == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("CreateFileMappingW", "path", lpszFileName);
  }
  void * pv = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
  if (pv == nullptr)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("MapViewOfFile", "path", lpszFileName);
  }
  pBitmapFileHeader = reinterpret_cast<BITMAPFILEHEADER *>(pv);
  if (pBitmapFileHeader->bfType != 0x4d42)
  {
    MIKTEX_FATAL_ERROR_2(T_("Not a Windows Bitmap file."), "path", lpszFileName);
  }
}
