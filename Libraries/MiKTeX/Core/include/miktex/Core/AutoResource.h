/* miktex/Core/AutoResource.h:                          -*- C++ -*-

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

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(A089FEF06254514BA063DED44B70E66F)
#define A089FEF06254514BA063DED44B70E66F

#include <miktex/Core/config.h>

#include <cstdio>
#include <cstdlib>

#include <exception>

#include "Session.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

template<class HandleType> HandleType InvalidHandleValue()
{
  return 0;
}

template<class HandleType, class Destroyer> class AutoResource
{
public:
  AutoResource(HandleType handle = InvalidHandleValue<HandleType>()) :
    handle(handle)
  {
  }

public:
  virtual ~AutoResource()
  {
    try
    {
      Reset();
    }
    catch (const std::exception &)
    {
    }
  }

public:
  HandleType Get() const
  {
    return handle;
  }

public:
  void Attach(HandleType handle)
  {
    this->handle = handle;
  }

public:
  HandleType Detach()
  {
    HandleType handle = Get();
    Attach(InvalidHandleValue<HandleType>());
    return handle;
  }

public:
  void Reset(HandleType handle)
  {
    if (Get() != InvalidHandleValue<HandleType>())
    {
      Destroyer() (Detach());
    }
    Attach(handle);
  }

public:
  void Reset()
  {
    Reset(InvalidHandleValue<HandleType>());
  }

public:
  HandleType * operator & ()
  {
    return &handle;
  }

private:
  HandleType handle;
};

template<class HandleType1, class HandleType2, class Destroyer> class AutoResource2
{
public:
  AutoResource2(HandleType1 handle1 = InvalidHandleValue<HandleType1>(), HandleType2 handle2 = InvalidHandleValue<HandleType2>()) :
    handle1(handle1),
    handle2(handle2)
  {
  }

public:
  virtual ~AutoResource2()
  {
    try
    {
      Reset();
    }
    catch (const std::exception &)
    {
    }
  }

public:
  HandleType2 Get() const
  {
    return handle2;
  }

public:
  void Attach(HandleType1 handle1, HandleType2 handle2)
  {
    this->handle1 = handle1;
    this->handle2 = handle2;
  }

public:
  HandleType2 Detach()
  {
    HandleType2 handle2 = Get();
    Attach(InvalidHandleValue<HandleType1>(), InvalidHandleValue<HandleType2>());
    return handle2;
  }

public:
  void Reset()
  {
    if (Get() != InvalidHandleValue<HandleType2>())
    {
      HandleType1 handle1 = this->handle1;
      HandleType2 handle2 = this->handle2;
      this->handle1 = InvalidHandleValue<HandleType1>();
      this->handle2 = InvalidHandleValue<HandleType2>();
      Destroyer() (handle1, handle2);
    }
  }

public:
  HandleType2 * operator & () const
  {
    return &handle2;
  }

private:
  HandleType1 handle1;

private:
  HandleType2 handle2;
};

struct fclose_
{
public:
  void operator() (FILE * file) const
  {
    if (fclose(file) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR("fclose");
    }
  }
};

typedef AutoResource<FILE *, fclose_> AutoFILE;

struct free_
{
public:
  void operator() (void * ptr) const
  {
    free(ptr);
  }
};

typedef AutoResource<void *, free_> AutoMemoryPointer;

MIKTEX_CORE_END_NAMESPACE;

#endif
