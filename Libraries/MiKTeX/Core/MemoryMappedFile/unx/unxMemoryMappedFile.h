/* unxMemoryMappedFile.h:                               -*- C++ -*-

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

#if !defined(D3F1FF54CD2C40CC99A6E4E4DB457FB4)
#define D3F1FF54CD2C40CC99A6E4E4DB457FB4

#include "miktex/Core/MemoryMappedFile.h"
#include "miktex/Core/PathName.h"

BEGIN_INTERNAL_NAMESPACE;

class unxMemoryMappedFile :
  public MiKTeX::Core::MemoryMappedFile
{
public:
  ~unxMemoryMappedFile() override;

public:
  void* Open(const MiKTeX::Core::PathName& path, bool readWrite) override;

public:
  void Close() override;

public:
  void* Resize(size_t newSize) override;

public:
  void* GetPtr() const override
  {
    return ptr;
  }

public:
  std::string GetName() const override
  {
    return name;
  }

public:
  size_t GetSize() const override
  {
    return size;
  }

public:
  void Flush() override;

private:
  void OpenFile();

private:
  void CreateMapping(size_t maximumFileSize);

private:
  void CloseFile();

private:
  void DestroyMapping();

private:
  void* ptr = nullptr;

private:
  bool readWrite = false;

private:
  MiKTeX::Core::PathName path;

private:
  std::string name;

private:
  size_t size = 0;

private:
  int filedes = -1;
};

END_INTERNAL_NAMESPACE;

#endif
