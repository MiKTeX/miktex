/* WebFile.h:                                           -*- C++ -*-

   Copyright (C) 2001-2018 Christian Schenk

   This file is part of MiKTeX Package Manager.

   MiKTeX Package Manager is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Package Manager is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Package Manager; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if !defined(FDC3B537D4484567B7577B2803B60F36)
#define FDC3B537D4484567B7577B2803B60F36

BEGIN_INTERNAL_NAMESPACE;

class MIKTEXNOVTABLE WebFile
{
public:
  virtual ~WebFile() = 0;

public:
  virtual size_t Read(void* pBuffer, size_t n) = 0;

public:
  virtual void Close() = 0;
};

END_INTERNAL_NAMESPACE;

#endif
