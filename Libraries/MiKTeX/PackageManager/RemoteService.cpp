/* RemoteService.cpp:

   Copyright (C) 2001-2016 Christian Schenk

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

#include "StdAfx.h"

#include "internal.h"

#include "NoRemoteService.h"
#include "RemoteService.h"
#include "RestRemoteService.h"
#include "SoapRemoteService.h"

using namespace MiKTeX::Packages;
using namespace std;

RemoteService::~RemoteService()
{
}

bool endsWith(const string & s, const string & suffix)
{
  return s.length() >= suffix.length() && s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0;
}

unique_ptr<RemoteService> RemoteService::Create(const string & endpoint, const ProxySettings & proxySettings)
{
  if (endpoint.empty() || endpoint == "multiplexor")
  {
    return make_unique<NoRemoteService>({ "http://mirrors.ctan.org/" });
  }
  else if (endsWith(endpoint, ".asmx"))
  {
    return make_unique<SoapRemoteService>(endpoint, proxySettings);
  }
  else
  {
    return make_unique<RestRemoteService>(endpoint, proxySettings);
  }
}
