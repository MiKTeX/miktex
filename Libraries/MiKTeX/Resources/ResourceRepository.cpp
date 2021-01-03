/* ResourceRepository.cpp:

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

#include "config.h"

#include <map>
#include <mutex>
#include <string>

#include "miktex/Resources/ResourceRepository.h"

using namespace std;

using namespace MiKTeX::Resources;

class ResourceRepository::impl
{
public:
  map<string, Resource> resources;
public:
  std::once_flag initFlag;
};

void ResourceRepository::AddResource(const char* resourceId, const Resource& resource)
{
  pimpl->resources[resourceId] = resource;
}

const Resource& ResourceRepository::GetResource(const char* resourceId)
{
  std::call_once(pimpl->initFlag, [this]() { Init(); });
  auto it = pimpl->resources.find(resourceId);
  if (it == pimpl->resources.end())
  {
    static Resource nullResource;
    return nullResource;
  }
  return it->second;
}

ResourceRepository::ResourceRepository()
  : pimpl(new ResourceRepository::impl)
{
}

ResourceRepository::~ResourceRepository()
{
  delete pimpl;
}
