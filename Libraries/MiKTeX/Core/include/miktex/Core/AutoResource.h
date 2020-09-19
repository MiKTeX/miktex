/* miktex/Core/AutoResource.h:                          -*- C++ -*-

   Copyright (C) 1996-2020 Christian Schenk

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

#if !defined(A089FEF06254514BA063DED44B70E66F)
#define A089FEF06254514BA063DED44B70E66F

#include <miktex/Core/config.h>

#include <cstdio>
#include <cstdlib>

#include <exception>

#include <miktex/Helpers>

#include "Session.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

template<class HandleType> HandleType InvalidHandleValue()
{
  return 0;
}

template<class HandleType, class Destroyer> class AutoResource
{
public:
  AutoResource() = default;

public:
  AutoResource(const AutoResource& other) = delete;

public:
  AutoResource& operator=(const AutoResource& other) = delete;

public:
  AutoResource(AutoResource&& other) = delete;

public:
  AutoResource& operator=(AutoResource&& other) = delete;

public:
  virtual ~AutoResource() noexcept
  {
    try
    {
      Reset();
    }
    catch (const std::exception&)
    {
    }
  }

public:
  AutoResource(HandleType handle) :
    handle(handle)
  {
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
  HandleType* operator&()
  {
    return &handle;
  }

private:
  HandleType handle = InvalidHandleValue<HandleType>();
};

template<class HandleType1, class HandleType2, class Destroyer> class AutoResource2
{
public:
  AutoResource2() = default;

public:
  AutoResource2(const AutoResource2& other) = delete;

public:
  AutoResource2& operator=(const AutoResource2& other) = delete;

public:
  AutoResource2(AutoResource2&& other) = delete;

public:
  AutoResource2& operator=(AutoResource2&& other) = delete;

public:
  virtual ~AutoResource2()
  {
    try
    {
      Reset();
    }
    catch (const std::exception&)
    {
    }
  }

public:
  AutoResource2(HandleType1 handle1, HandleType2 handle2 = InvalidHandleValue<HandleType2>()) :
    handle1(handle1),
    handle2(handle2)
  {
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
  HandleType2* operator&() const
  {
    return &handle2;
  }

private:
  HandleType1 handle1 = InvalidHandleValue<HandleType1>();

private:
  HandleType2 handle2 = InvalidHandleValue<HandleType2>();
};

struct fclose_
{
public:
  void operator()(FILE* file) const
  {
    if (fclose(file) != 0)
    {
      MIKTEX_FATAL_CRT_ERROR("fclose");
    }
  }
};

typedef AutoResource<FILE*, fclose_> AutoFILE;

struct free_
{
public:
  void operator()(void* ptr) const
  {
    free(ptr);
  }
};

typedef AutoResource<void*, free_> AutoMemoryPointer;

template<class Func> struct AutoFunc
{
public:
  AutoFunc() = default;

public:
  AutoFunc(const AutoFunc& other) = default;

public:
  AutoFunc& operator=(const AutoFunc& other) = default;

public:
  AutoFunc(AutoFunc&& other) = default;

public:
  AutoFunc& operator=(AutoFunc&& other) = default;

public:
  AutoFunc(Func func) :
    func(func)
  {
  }

public:
  ~AutoFunc()
  {
    try
    {
      func();
    }
    catch (const std::exception&)
    {
    }
  }

private:
  Func func;
};

template<class Func> AutoFunc<Func> CreateAutoFunc(Func func)
{
  return AutoFunc<Func>(func);
}

#define MIKTEX_AUTO(x) auto MIKTEX_UNIQUE(bbf09dd97db04f579f1c386c16f7fd38) = MiKTeX::Core::CreateAutoFunc([&](){x;})

MIKTEX_CORE_END_NAMESPACE;

#endif
