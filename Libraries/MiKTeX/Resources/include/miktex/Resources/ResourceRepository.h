/* miktex/Resources/ResourceRepository.h:

   Copyright (C) 2020-2021 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#pragma once

#include "config.h"

#include <cstddef>

MIKTEX_RES_BEGIN_NAMESPACE;

struct Resource
{
  const void* data = nullptr;
  std::size_t len = 0;
};

class MIKTEXRESTYPEAPI(ResourceRepository)
{
public:
   MIKTEXRESEXPORT MIKTEXTHISCALL ResourceRepository();

public:
  ResourceRepository(const ResourceRepository& other) = delete;

public:
  ResourceRepository& operator=(const ResourceRepository& other) = delete;

public:
  ResourceRepository(const ResourceRepository&& other) = delete;

public:
  ResourceRepository& operator=(const ResourceRepository&& other) = delete;

public:
  virtual MIKTEXRESEXPORT MIKTEXTHISCALL ~ResourceRepository() noexcept;

public:
   MIKTEXRESTHISAPI(const Resource&) GetResource(const char* resourceId);

protected:
  virtual  MIKTEXRESTHISAPI(void) Init() = 0;

protected:
   MIKTEXRESTHISAPI(void) AddResource(const char* resourceId, const Resource& resource);

private:
  class impl;
  impl* pimpl;
};

MIKTEX_RES_END_NAMESPACE;
