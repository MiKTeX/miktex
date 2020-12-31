/* ResourceRepository.h:

   Copyright (C) 2020 Christian Schenk

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

#include <map>
#include <string>

#include "ResourceRepository.h"
#include "resdata.h"

using namespace std;

class ResourceRepository::impl
{
public:
  map<string, Resource> resources;
};

void ResourceRepository::addResource(const char* resourceId, const Resource& resource)
{
  pimpl->resources[resourceId] = resource;
}

const Resource& ResourceRepository::GetResource(const char* resourceId) const
{
    return pimpl->resources[resourceId];
}

ResourceRepository::ResourceRepository()
  :pimpl(new ResourceRepository::impl)
{
}

ResourceRepository::~ResourceRepository()
{
  delete pimpl;
}
