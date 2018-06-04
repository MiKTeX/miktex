/* vi/Runtime.cpp:

   Copyright (C) 1996-2018 Christian Schenk

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
   along with MiKTeX Package Manager; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include "config.h"

#if defined(HAVE_LIBCURL)
#  include <curl/curl.h>
#endif

#include <expat.h>

#include <miktex/Core/vi/Version>
#include <miktex/Extractor/vi/Version>

#include "miktex/PackageManager/vi/Version.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages::vi;

string Runtime::GetName()
{
  return Header::GetName();
}

string Runtime::GetDescription()
{
  return Header::GetDescription();
}

int Runtime::GetInterfaceVersion()
{
  return Header::GetInterfaceVersion();
}

VersionNumber Runtime::GetVersion()
{
  return Header::GetVersion();
}

vector<LibraryVersion> Runtime::GetDependencies()
{
  vector<LibraryVersion> result;
#if defined(HAVE_LIBCURL)
  result.push_back(LibraryVersion("curl", LIBCURL_VERSION, curl_version()));
#endif
  result.push_back(LibraryVersion("expat", VersionNumber(XML_MAJOR_VERSION, XML_MINOR_VERSION, XML_MICRO_VERSION, 0).ToString(), XML_ExpatVersion()));
  result.push_back(MiKTeX::Core::vi::Version::GetLibraryVersion());
  auto deps = MiKTeX::Core::vi::Runtime::GetDependencies();
  result.insert(std::end(result), std::begin(deps), std::end(deps));
  result.push_back(MiKTeX::Extractor::vi::Version::GetLibraryVersion());
  deps = MiKTeX::Extractor::vi::Runtime::GetDependencies();
  result.insert(std::end(result), std::begin(deps), std::end(deps));
  return result;
}
