/* RemoteService.cpp:

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

#include "config.h"

#include <memory>
#include <string>

#include "miktex/PackageManager/PackageManager.h"

#include "NoRemoteService.h"
#include "RemoteService.h"
#include "RestRemoteService.h"

using namespace std;

using namespace MiKTeX::Packages;

using namespace MiKTeX::Packages::D6AAD62216146D44B580E92711724B78;

RemoteService::~RemoteService()
{
}

bool endsWith(const string& s, const string& suffix)
{
  return s.length() >= suffix.length() && s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0;
}

unique_ptr<RemoteService> RemoteService::Create(const string& endpoint, const ProxySettings& proxySettings)
{
  if (endpoint.empty() || endpoint == "multiplexor")
  {
    return make_unique<NoRemoteService>(vector<string>({ "http://mirrors.ctan.org/" }));
  }
  else
  {
    return make_unique<RestRemoteService>(endpoint, proxySettings);
  }
}
